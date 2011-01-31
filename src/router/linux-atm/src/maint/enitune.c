/* enitune.c - ENI buffer size tuning */

/* Written 2000 by Werner Almesberger, EPFL ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <atm.h>
#include <linux/atm_eni.h>


static void usage(const char *name)
{
    fprintf(stderr,"usage: %s [ -t tx_mult ] [ -r rx_mult ] itf\n",name);
    fprintf(stderr,"    multipliers are in percent and must be > 100\n");
    exit(1);
}


int main(int argc,char **argv)
{
    char *name,*end;
    struct atmif_sioc sioc;
    struct eni_multipliers mult;
    int c,s;

    name = argv[0];
    mult.tx = mult.rx = 0;
    while ((c = getopt(argc,argv,"t:r:")) != EOF)
	switch (c) {
	    case 't':
		mult.tx = strtol(optarg,&end,0);
		if (*end || mult.tx <= 100) usage(name);
		break;
	    case 'r':
		mult.rx = strtol(optarg,&end,0);
		if (*end || mult.rx <= 100) usage(name);
		break;
	}
    if (argc != optind+1) usage(name);
    sioc.number = strtol(argv[optind],&end,0);
    if (*end || sioc.number < 0) usage(name);
    if ((s = socket(PF_ATMPVC,SOCK_DGRAM,0)) < 0) {
	perror("socket");
	return 1;
    }
    sioc.arg = &mult;
    sioc.length = sizeof(mult);
    if (ioctl(s,ENI_SETMULT,&sioc) < 0) {
	perror("ioctl ENI_SETMULT");
	return 1;
    }
    return 0;
}
