/* swc.c - User switch control */

/* Written 1998 by Werner Almesberger, EPFL ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <atm.h>
#include <atmd.h>

#include "swc.h"


static void dialog(int s,SWC_MSG *msg)
{
    int size;

    size = write(s,msg,sizeof(*msg));
    if (size < 0) {
	perror("write");
	exit(1);
    }
    if (size != sizeof(*msg)) {
	fprintf(stderr,"bad write: %d != %d\n",size,sizeof(*msg));
	exit(1);
    }
    size = read(s,msg,sizeof(*msg));
    if (size < 0) {
	perror("read");
	exit(1);
    }
    if (size != sizeof(*msg)) {
	fprintf(stderr,"bad read: %d != %d\n",size,sizeof(*msg));
	exit(1);
    }
}


static void usage(const char *name)
{
    fprintf(stderr,"usage: %s <socket> <command>\n",name);
    fprintf(stderr,"  commands: show\n");
    fprintf(stderr,"            add <in_pvc> <out_pvc> [<qos>]\n");
    fprintf(stderr,"            del <in_pvc> <out_pvc>\n");
    exit(1);
}


int main(int argc,const char **argv)
{
    char buffer[MAX_ATM_ADDR_LEN+1];
    SWC_MSG msg;
    int s;

    if (argc < 3) usage(*argv);
    s = un_attach(argv[1]);
    if (s < 0) {
	perror(argv[1]);
	return 1;
    }
    memset(&msg,0,sizeof(msg));
    if (!strcmp(argv[2],"show")) {
	if (argc != 3) usage(*argv);
	msg.type = smt_get;
	msg.n = 0;
	while (1) {
	    dialog(s,&msg);
	    if (msg.type != smt_get) {
		fprintf(stderr,"unexpeced message type %d != %d\n",msg.type,
		  smt_get);
	    }
	    if (msg.n < 0) return 0;
	    if (msg.in.sap_addr.vci != ATM_VCI_UNSPEC) printf("VC ");
	    else {
		printf("VP ");
		msg.out.sap_addr.vci = ATM_VCI_UNSPEC;
	    }
	    if (atm2text(buffer,sizeof(buffer),(struct sockaddr *) &msg.in,
	      A2T_PRETTY) < 0) strcpy(buffer,"<invalid>");
	    printf("%s %c-%c ",buffer,msg.qos.rxtp.traffic_class ? '<' : '-',
	      msg.qos.txtp.traffic_class ? '>' : '-');
	    if (atm2text(buffer,sizeof(buffer),(struct sockaddr *) &msg.out,
	      A2T_PRETTY) < 0) strcpy(buffer,"<invalid>");
	    printf("%s\n",buffer);
	    msg.n++;
	}
    }
    if (!strcmp(argv[2],"add")) {
	msg.type = smt_add;
	msg.qos.txtp.traffic_class = msg.qos.rxtp.traffic_class = ATM_UBR;
	msg.qos.aal = ATM_AAL5;
	if (argc == 6) {
	    if (text2qos(argv[5],&msg.qos,0) < 0) {
		fprintf(stderr,"invalid QOS specification: %s\n",argv[5]);
		return 1;
	    }
	}
	else if (argc != 5) usage(*argv);
    }
    else if (!strcmp(argv[2],"del")) {
	    if (argc != 5) usage(*argv);
	    msg.type = smt_del;
	}
        else usage(*argv);
    if (text2atm(argv[3],(struct sockaddr *) &msg.in,sizeof(msg.in),
      T2A_PVC | T2A_UNSPEC | T2A_NAME) < 0) {
	fprintf(stderr,"invalid PVC address: %s\n",argv[3]);
	return 1;
    }
    if (text2atm(argv[4],(struct sockaddr *) &msg.out,sizeof(msg.out),
      T2A_PVC | T2A_UNSPEC | T2A_NAME) < 0) {
	fprintf(stderr,"invalid PVC address: %s\n",argv[4]);
	return 1;
    }
    dialog(s,&msg);
    if (msg.n < 0) {
	fprintf(stderr,"%s\n",strerror(-msg.n));
	return 1;
    }
    return 0;
}
