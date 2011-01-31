#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <atm.h>
#include <linux/atmdev.h>

#define BHLI_MAGIC	"FORE_ATM"
#define	NUMPDUS		50000

static void usage(const char *name)
{
	fprintf(stderr, "usage: %s [<nsap> [<selector> [<qos>]]]\n", name);
	exit(1);
}

int main(int argc, char **argv)
{
	struct sockaddr_atmsvc addr;
	struct sockaddr_atmsvc *my_addr;
	struct atm_qos qos;
	struct atm_sap sap;
	int s, i, errcount = 0, fd, rv;
	int selector = 0;
	int one = 1;
	socklen_t len;

	char *outbuf = (char *) malloc(65535);
	if (outbuf == NULL)
		return 1;

	if ((s = socket(PF_ATMSVC, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return 1;
	}

	memset(&addr, 0, sizeof(addr));
	memset(&qos, 0, sizeof(qos));
	memset(&my_addr, 0, sizeof(my_addr));

	qos.aal = ATM_AAL5;
	qos.txtp.traffic_class = ATM_NONE;
	qos.rxtp.traffic_class = ATM_UBR;
	qos.rxtp.max_sdu = 65535;

	/* Usage: svc_recv [local_host_name [selector [qos]]] */
	if (argc > 1) {
		int answer;
		my_addr =
		    (struct sockaddr_atmsvc *)
		    malloc(sizeof(struct sockaddr_atmsvc));

		answer =
		    text2atm(argv[1], (struct sockaddr *) my_addr,
			     sizeof(struct sockaddr_atmsvc),
			     T2A_SVC | T2A_NAME);

		if (-1 == answer) {
			fprintf(stderr, "Bad host name: %s\n", argv[1]);
			usage(argv[0]);
		}

		if (argc > 2) {

			selector = atoi(argv[2]);
			my_addr->sas_addr.prv[ATM_ESA_LEN - 1] =
			    (char) selector;

			if (argc > 3) {
				if (text2qos(argv[3], &qos, 0) < 0) {
					fprintf(stderr, "Invalid qos.\n");
					usage(argv[0]);
				}
			}
		}
	}

	memset(&sap, 0, sizeof(sap));
	sap.bhli.hl_type = ATM_HL_USER;
	sap.bhli.hl_length = 8;
	memcpy(&sap.bhli.hl_info, BHLI_MAGIC, 8);

	if (setsockopt(s, SOL_ATM, SO_ATMQOS, &qos, sizeof(qos)) < 0) {
		perror("setsockopt SO_ATMQOS (1)");
		return 1;
	}

	if (setsockopt(s, SOL_ATM, SO_ATMSAP, &sap, sizeof(sap)) < 0) {
		perror("setsockopt SO_ATMSAP");
		return 1;
	}

	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0) {
		perror("setsockopt SO_REUSEADDR");
		return 1;
	}

	if (NULL != my_addr) {
		int ret;
		char atm2textbuf[MAX_ATM_ADDR_LEN + 1];

		ret =
		    atm2text(atm2textbuf, sizeof(atm2textbuf),
			     (struct sockaddr *) my_addr,
			     A2T_NAME | A2T_PRETTY | A2T_LOCAL);

		if (-1 == ret) {
			fprintf(stderr, "Bad host name: %s\n", argv[1]);
			return 1;
		}

		fprintf(stderr, "binding to addr %s\n", atm2textbuf);

		ret =
		    bind(s, (struct sockaddr *) my_addr,
			 sizeof(struct sockaddr_atmsvc));

		if (ret < 0) {
			perror("bind() said");
			return 1;
		}
	}

	if (listen(s, 1) < 0) {
		perror("listen");
		return 1;
	}

	len = sizeof(addr);
	if ((fd = accept(s, (struct sockaddr *) &addr, &len)) < 0) {
		perror("accept");
		return 1;
	}

	fprintf(stderr, "receiving %d PDUs\n", NUMPDUS);
	for (i = 0, errcount = 0, rv = -1; i < NUMPDUS && rv != 0; i++)
		if ((rv = read(fd, outbuf, 65535)) < 0)
			errcount++;

	if (errcount > 0)
		fprintf(stderr, "read() failed %d out of %d times\n",
			errcount, NUMPDUS);
	fprintf(stderr, "received %d PDUs\n", i - 1);

	if (close(s) < 0)
		perror("close s");
	if (close(fd) < 0)
		perror("close fd");
	exit(0);
}
