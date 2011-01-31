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
	fprintf(stderr, "usage: %s <nsap> [<nsap> [...]]\n", name);
	exit(1);
}

int main(int argc, char **argv)
{
	struct sockaddr_atmsvc addr, leaf;
	struct atm_qos qos;
	struct atm_sap sap;
	int s, i, ep_ref, errcount = 0, leaves = 0;
	int one = 1;

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
	qos.aal = ATM_AAL5;
	qos.txtp.traffic_class = ATM_UBR;
	qos.rxtp.traffic_class = ATM_NONE;
	qos.txtp.max_sdu = 8192;

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

	if (setsockopt(s, SOL_ATM, SO_MULTIPOINT, &one, sizeof(one)) < 0) {
		perror("setsockopt SO_MULTIPOINT");
		return 1;
	}

	if (connect(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		/* return here if connect fails.  The ADDPARTY ioctl can't
		 * be called until connect succeeds.
		 */
		perror("connect");
		return 1;
	}
	fprintf(stderr, "added leaf 0 at %s\n", argv[1]);
	leaves = 1;

	if (argc > 2)
		for (i = 2; i < argc; i++) {
			if (text2atm(argv[i], (struct sockaddr *) &leaf,
				     sizeof(leaf), T2A_SVC) >= 0) {
				if ((ep_ref =
				     ioctl(s, ATM_ADDPARTY, &leaf)) < 0)
					perror("ATM_ADDPARTY");
				else {
					fprintf(stderr, "added leaf %d at %s\n",
						ep_ref, argv[i]);
					leaves++;
				}
			} else
				fprintf(stderr, "invalid leaf address\n");
		}

	fprintf(stderr, "sending %d PDUs\n", NUMPDUS);
	for (i = 0, errcount = 0; i < NUMPDUS; i++)
		if (write(s, outbuf, 8192) < 0)
			errcount++;
	write(s, NULL, 0);

	if (errcount > 0)
		fprintf(stderr, "write() failed %d out of %d times\n",
			errcount, NUMPDUS);

	ep_ref = leaves - 1;
	while (ep_ref >= 0) {
		i = ioctl(s, ATM_DROPPARTY, &ep_ref);
		if (i < 0)
			perror("ioctl(ATM_DROPPARTY)");
		else
			fprintf(stderr, "dropped leaf %d\n", ep_ref);
		ep_ref--;
	}

	if (close(s) < 0)
		perror("close s");
	exit(0);
}
