#include <atm.h>
#include <linux/atmdev.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "ilmid.h"

static const char *ilmiver[4] = { "", "unsupported", "4.0", "undefined" };
static const char *univer[6] = { "", "", "3.0", "3.1", "4.0", "undefined" };
const char *struni(int ver){
	switch(ver){
		case 2: return univer[2];
		case 3: return univer[3];
		case 4: return univer[4];
	}
	return univer[5];
}

const char *strilmi(int ver){
	switch(ver){
		case 1: return ilmiver[1];
		case 2: return ilmiver[2];
	}
	return ilmiver[3];
}

int main(int argc, char **argv){
	int fd;
	struct ilmi_state ic;
	struct sockaddr_un addr;
	struct sockaddr_atmpvc vc;

	if(argc > 2){
		fprintf(stderr, "usage: %s [<pvc>]\n", argv[0]);
		fprintf(stderr, "  Default pvc is 0.0.16\n");
		return 1;
	}
	if (argc == 1)
		text2atm("0.0.16", (struct sockaddr *)&vc, sizeof(vc), T2A_PVC);
	else
		if (text2atm(argv[1], (struct sockaddr *)&vc,
		             sizeof(vc), T2A_PVC) < 0) {
			fprintf(stderr, "invalid pvc\n");
			return 1;
		}

	fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(fd < 0){
		fprintf(stderr, "Could not open socket.\n");
		return 1;
	}

	memset(&addr, 0, sizeof(addr));
	snprintf(addr.sun_path, sizeof(addr.sun_path) - 1,
	         "%s/%d.%d.%d",
	         ILMIDIAG_DIR,
	         vc.sap_addr.itf,
	         vc.sap_addr.vpi,
	         vc.sap_addr.vci);
	addr.sun_family = AF_UNIX;

	if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		fprintf(stderr, "unable to connect (ilmid running?)\n");
		return 1;
	}

	if (read(fd, &ic, sizeof(ic)) < sizeof(ic)) {
		fprintf(stderr, "read failed\n");
		return 1;
	}

	printf("\n");
	printf("%-15.15s%-15.15s%-15.15s%-15.15s%-15.15s\n",
	       "Ilmi", "Uni", "VPI", "VCI", "Ilmi");
	printf("%-15.15s%-15.15s%-15.15s%-15.15s%-15.15s\n",
	       "Version", "Version", "Bits", "Bits", "State");
	printf("---------------------------------------"
	       "------------------------------------\n");
	printf("%-15s%-15s%-15d%-15d%-15s\n",
	       strilmi(ic.ilmi_version), struni(ic.uni_version),
	       ic.vpi_bits, ic.vci_bits,
	       (ic.state == 1) ? "Up" : "Down");
	printf("\n");

	printf("%-15.15s%-15.15s%-15.15s%-15.15s\n",
	       "Remote", "Remote", "Remote", "Remote");
	printf("%-15.15s%-15.15s%-15.15s%-15.15s\n",
	       "Port Id", "Port Name", "Sysname", "Contact");
	printf("---------------------------------------"
	       "------------------------------------\n");
	printf("%-15d%-15.15s%-15.15s%-30.30s\n",
	       ic.remote_portid, ic.remote_portname,
	       ic.remote_sysname, ic.remote_contact);
	printf("\n");

	return 0;
}

