/* atmdiag.c - ATM diagnostics */

/* Written 1995-2000 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>

#include <atm.h>
#include <linux/atmdev.h>


#define BUF_LEN 1024 /* ugly */


static void usage(const char *name)
{
    fprintf(stderr,"usage: %s [ -z ]  [ itf ... ]\n",name);
    exit(1);
}


static void display(int s,int itf,int zero)
{
    static int first = 1;
    struct atmif_sioc req;
    struct atm_dev_stats stats;

    req.number = itf;
    req.arg = &stats;
    req.length = sizeof(stats);
    if (ioctl(s,zero ? ATM_GETSTATZ : ATM_GETSTAT,&req) < 0) {
	perror(zero ? "ioctl ATM_GETSTATZ" : "ioctl ATM_GETSTAT");
	exit(1);
    }
    if (first) {
	printf("Itf       TX_okay   TX_err    RX_okay   RX_err    RX_drop\n"
	  );
	first = 0;
    }
    printf("%3d AAL0  %8d  %8d  %8d  %8d  %8d\n",itf,stats.aal0.tx,
      stats.aal0.tx_err,stats.aal0.rx,stats.aal0.rx_err,stats.aal0.rx_drop);
    printf("%3s AAL5  %8d  %8d  %8d  %8d  %8d\n","",stats.aal5.tx,
      stats.aal5.tx_err,stats.aal5.rx,stats.aal5.rx_err,stats.aal5.rx_drop);
}


int main(int argc,const char *const *argv)
{
    int s,zero;

    zero = 0;
    if (argc > 1 && *argv[1] == '-') {
	if (strcmp(argv[1],"-z")) usage(argv[0]);
	else {
	    zero = 1;
	    argv++;
	    argc--;
	}
    }
    if ((s = socket(PF_ATMPVC,SOCK_DGRAM,0)) < 0) {
	perror("socket");
	return 1;
    }
    if (argc > 1)
	while (argc-- > 1) {
	    char *end;
	    int itf;

	    itf = strtol(*++argv,&end,0);
	    if (*end || itf < 0) {
		fprintf(stderr,"invalid interface number: %s\n",*argv);
		return 1;
	    }
	    display(s,itf,zero);
	}
    else {
	struct atm_iobuf req;
	int buf[BUF_LEN];
	int i;

	req.length = BUF_LEN*sizeof(int);
	req.buffer = buf;
	if (ioctl(s,ATM_GETNAMES,&req) < 0) {
	    perror("ioctl ATM_GETNAMES");
	    return 1;
	}
	for (i = 0; i < req.length/sizeof(int); i++) display(s,buf[i],zero);
    }
    return 0;
}
