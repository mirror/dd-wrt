/* esi.c - Get or set End System Identifier (ESI) */

/* Written 1997-2000 by Werner Almesberger, EPFL LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <linux/version.h>


#if LINUX_VERSION_CODE < 0x20100


#include <stdio.h>


int main(int argc,const char **argv)
{
    fprintf(stderr,"%s: not supported in this version\n",*argv);
    return 1;
}


#else


#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

#include <atm.h>
#include <linux/atmdev.h>


static void usage(const char *name)
{
    fprintf(stderr,"usage: %s [ [ -f ] esi ] [ itf ]\n",name);
    fprintf(stderr,"%6s %s -V\n","",name);
    exit(1);
}


int main(int argc,char **argv)
{
    struct atmif_sioc req;
    const char *esi;
    unsigned char esi_buf[ESI_LEN];
    char *end;
    int op,c,s;

    op = ATM_SETESI;
    memset(&req,0,sizeof(req));
    esi = NULL;
    while ((c = getopt(argc,argv,"fV")) != EOF)
	switch (c) {
	    case 'f':
		op = ATM_SETESIF;
		break;
	    case 'V':
		printf("%s\n",VERSION);
		return 0;
	    default:
		usage(*argv);
	}
    if (argc > optind+2) usage(*argv);
    if (argc == optind+2) {
	req.number = strtoul(argv[optind+1],&end,0);
	if (*end) usage(*argv);
	esi = argv[optind];
    }
    if (argc == optind+1) {
	if (strlen(argv[optind]) == ESI_LEN*2) esi = argv[optind];
	else {
	    req.number = strtoul(argv[optind],&end,0);
	    if (*end) usage(*argv);
	}
    }
    if (op == ATM_SETESIF && !esi) usage(*argv);
    if (esi) {
	int i,byte;

	for (i = 0; i < ESI_LEN; i++) {
	    if (sscanf(esi,"%2x",&byte) != 1) usage(*argv);
	    esi_buf[i] = byte;
	    esi += 2;
	}
    }
    req.arg = esi_buf;
    req.length = ESI_LEN;
    if ((s = socket(PF_ATMPVC,SOCK_DGRAM,0)) < 0) {
	perror("socket");
	return 1;
    }
    if (ioctl(s,esi ? op : ATM_GETESI,&req) < 0) {
	perror(esi ? op == ATM_SETESI ? "ioctl ATM_SETESI" :
	  "ioctl ATM_SETESIF" : "ioctl ATM_GETESI");
	return 1;
    }
    if (!esi) {
	int i;

	for (i = 0; i < ESI_LEN; i++) printf("%02X",esi_buf[i]);
	putchar('\n');
    }
    return 0;
}


#endif
