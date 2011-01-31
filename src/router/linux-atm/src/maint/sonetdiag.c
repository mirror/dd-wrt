/* sonetdiag.c - SONET diagnostics */

/* Written 1995-1999 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <atm.h>
#include <linux/atmdev.h>
#include <linux/sonet.h>


struct opts {
    const char *name;
    int value;
} options[] = {
    { "sbip", SONET_INS_SBIP  }, { "lbip", SONET_INS_LBIP  },
    { "pbip", SONET_INS_PBIP  }, { "frame",SONET_INS_FRAME },
    { "los",  SONET_INS_LOS   }, { "lais", SONET_INS_LAIS  },
    { "pais", SONET_INS_PAIS  }, { "hcs",  SONET_INS_HCS   },
    { NULL,   0 }
};


static void usage(const char *name)
{
    fprintf(stderr,"usage: %s [ -z ] [ itf ] [ [-]error ...]\n",name);
    fprintf(stderr,"  errors: sbip  lbip  pbip  frame\n");
    fprintf(stderr,"          los   lais  pais  hcs\n");
    exit(1);
}


int main(int argc,char **argv)
{
    struct atmif_sioc req;
    struct sonet_stats stats;
    struct opts *walk;
    const char *name;
    char *opt,*end;
    int zero,s,set,clear,error,minus;

    zero = 0;
    name = argv[0];
    if (argc > 1 && argv[1][0] == '-') {
	if (strcmp(argv[1],"-z")) usage(name);
	zero = 1;
	argc--;
	argv++;
    }
    if ((s = socket(PF_ATMPVC,SOCK_DGRAM,0)) < 0) {
	perror("socket");
	return 1;
    }
    if (argc == 1) req.number = 0;
    else {
	req.number = strtol(argv[1],&end,10);
	if (*end) req.number = 0;
	else {
	    if (req.number < 0) usage(name);
	    argc--;
	    argv++;
	}
    }
    argc--;
    argv++;
    set = clear = error = 0;
    while (argc--) {
	minus = *(opt = *argv++) == '-';
	if (minus) opt++;
	for (walk = options; walk->name; walk++)
	    if (!strcmp(walk->name,opt)) break;
	if (walk->name)
	    if (minus) clear |= walk->value;
	    else set |= walk->value;
	else {
	    fprintf(stderr,"unrecognized option: %s\n",opt);
	    error = 1;
	}
    }
    if (error) return 1;
    if (!set && !clear) {
	req.arg = &stats;
	req.length = sizeof(stats);
	if (ioctl(s,zero ? SONET_GETSTATZ : SONET_GETSTAT,&req) < 0) {
	    perror(zero ? "ioctl SONET_GETSTATZ" : "ioctl SONET_GETSTAT");
	    return 1;
	}
	req.arg = &set;
	req.length = sizeof(set);
	if (ioctl(s,SONET_GETDIAG,&req) < 0)
	    if (errno != EINVAL) perror("ioctl SONET_GETDIAG");
	if (stats.section_bip != -1)
	    printf("Section BIP errors: %10d\n",stats.section_bip);
	if (stats.line_bip != -1)
	    printf("Line BIP errors:    %10d\n",stats.line_bip);
	if (stats.path_bip != -1)
	    printf("Path BIP errors:    %10d\n",stats.path_bip);
	if (stats.line_febe != -1)
	    printf("Line FEBE:          %10d\n",stats.line_febe);
	if (stats.path_febe != -1)
	    printf("Path FEBE:          %10d\n",stats.path_febe);
	if (stats.corr_hcs != -1)
	    printf("Correctable HCS:    %10d\n",stats.corr_hcs);
	if (stats.uncorr_hcs != -1)
	    printf("Uncorrectable HCS:  %10d\n",stats.uncorr_hcs);
	if (stats.tx_cells != -1)
	    printf("TX cells:           %10d\n",stats.tx_cells);
	if (stats.rx_cells != -1)
	    printf("RX cells:           %10d\n",stats.rx_cells);
	if (set) {
	    int i;

	    printf("\nDiagnostics:");
	    for (i = 0; options[i].name; i++)
		if (set & options[i].value) printf(" %s",options[i].name);
	    putchar('\n');
	}
    }
    else {
	if (set) {
	    req.arg = &set;
	    req.length = sizeof(set);
	    if (ioctl(s,SONET_SETDIAG,&req) < 0)
		perror("ioctl SONET_SETDIAG");
	}
	if (clear) {
	    req.arg = &clear;
	    req.length = sizeof(clear);
	    if (ioctl(s,SONET_CLRDIAG,&req) < 0)
		perror("ioctl SONET_SETDIAG");
	}
    }
    return 0;
}
