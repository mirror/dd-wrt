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
	fprintf(stderr, "usage: %s <nsap> [<qos>]\n", name);
	exit(1);
}

int main(int argc, char **argv)
{
	struct sockaddr_atmsvc addr;
	struct atm_qos qos;
	struct atm_sap sap;
	int s, i, errcount = 0;
	unsigned long one = 1;

	char *outbuf = (char *) malloc(8192);
	if (outbuf == NULL)
		return 1;

	if (argc < 2) {
		fprintf(stderr, "not enough arguments\n");
		usage(argv[0]);
	}
	if ((s = socket(PF_ATMSVC, SOCK_DGRAM, 0)) < 0) {
		perror("socket");
		return 1;
	}

	memset(&addr, 0, sizeof(addr));
	if (text2atm(argv[1], (struct sockaddr *) &addr, sizeof(addr),
		     T2A_SVC) < 0)
		usage(argv[0]);
	memset(&qos, 0, sizeof(qos));
	memset(&sap, 0, sizeof(sap));
	sap.bhli.hl_type = ATM_HL_USER;
	sap.bhli.hl_length = 8;
	memcpy(&sap.bhli.hl_info, BHLI_MAGIC, 8);

	if (argc > 2) {
		if (text2qos(argv[2], &qos, 0) < 0) {
			fprintf(stderr, "Invalid qos.\n");
			return 1;
		}
	} else {
		qos.aal = ATM_AAL5;
		qos.txtp.traffic_class = ATM_UBR;
		qos.rxtp.traffic_class = ATM_NONE;
		qos.txtp.max_sdu = 8192;
	}

	if (setsockopt(s, SOL_ATM, SO_ATMQOS, &qos, sizeof(qos)) < 0) {
		perror("setsockopt SO_ATMQOS (1)");
		return 1;
	}

	if (setsockopt(s, SOL_ATM, SO_ATMSAP, &sap, sizeof(sap)) < 0) {
		perror("setsockopt SO_ATMSAP");
		return 1;
	}

	if (connect(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("connect");
		return 1;
	}

	fprintf(stderr, "sending %d PDUs of length %d\n",
		NUMPDUS, qos.txtp.max_sdu);
	for (i = 0, errcount = 0; i < NUMPDUS; i++)
		if (write(s, outbuf, qos.txtp.max_sdu) < 0)
			errcount++;
	write(s, NULL, 0);

	if (errcount > 0)
		fprintf(stderr, "write() failed %d out of %d times\n",
			errcount, NUMPDUS);

	if (close(s) < 0)
		perror("close s");
	exit(0);
}
