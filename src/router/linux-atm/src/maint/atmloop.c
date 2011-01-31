/* atmloop.c - get/set loopback mode of ATM interfaces */

/* Written 2000 by Werner Almesberger, EPFL ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <atm.h>
#include <linux/atmdev.h>


static void usage(const char *name)
{
    fprintf(stderr,"usage: %s -s [ -l level ] [ -r level ] [ itf ]\n",name);
    fprintf(stderr,"%7s%s [ itf ]\n","",name);
    fprintf(stderr,"%7s%s -q [ itf ]\n","",name);
    fprintf(stderr,"%7s%s -V\n","",name);
    fprintf(stderr,"  levels: aal = AAL PDU, atm = ATM cell,\n");
    fprintf(stderr,"%10sphy = line (digital), analog = line (analog)\n","");
    exit(1);
}


static int text2level(const char *name,const char *s)
{
    if (!strcmp(s,"aal")) return __ATM_LM_AAL;
    if (!strcmp(s,"atm")) return __ATM_LM_ATM;
    if (!strcmp(s,"phy")) return __ATM_LM_PHY;
    if (!strcmp(s,"analog")) return __ATM_LM_ANALOG;
    usage(name);
    return 0; /* uh ... */
}


static const char *level2text(int level)
{
    switch (level) {
	case __ATM_LM_AAL:
	    return "aal";
	case __ATM_LM_ATM:
	    return "atm";
	case __ATM_LM_PHY:
	    return "phy";
	case __ATM_LM_ANALOG:
	    return "analog";
	default:
	    return "???";
    }
}


static void print_levels(int levels,int show_none)
{
    int mask;

    if (!levels) {
	if (show_none) printf(" (none)");
	return;
    }
    for (mask = 1; levels; mask += mask) {
	if (!(mask & levels)) continue;
	printf(" %s",level2text(__ATM_LM_XTLOC(mask)));
	levels &= ~mask;
    }
}


int main(int argc,char **argv)
{
    int local = ATM_LM_NONE,remote = ATM_LM_NONE,mode;
    int set = 0,query = 0;
    struct atmif_sioc req;
    int s,c;

    req.number = 0;
    while ((c = getopt(argc,argv,"l:qr:sV")) != EOF)
        switch (c) {
            case 'l':
		if (local) usage(*argv);
                local = text2level(*argv,optarg);
                break;
	    case 'q':
		query = 1;
		break;
            case 'r':
		if (remote) usage(*argv);
                remote = text2level(*argv,optarg);
                break;
	    case 's':
		set = 1;
		break;
	    case 'V':
		printf("%s\n",VERSION);
		return 0;
            default:
                usage(*argv);
        }
    if (argc > optind+1) usage(*argv);
    if (argc == optind+1) {
	char *end;

	req.number = strtoul(argv[optind],&end,0);
	if (*end) usage(*argv);
    }
    mode = __ATM_LM_MKLOC(local) | __ATM_LM_MKRMT(remote);
    if ((mode && !set) || (set && query)) usage(*argv);
    s = socket(PF_ATMPVC,SOCK_DGRAM,0);
    if (s < 0) {
	perror("socket");
	return 1;
    }
    if (set) {
	req.arg = (void *) mode;
	req.length = 0;
	if (ioctl(s,ATM_SETLOOP,&req) < 0) {
	    perror("ioctl ATM_SETLOOP");
	    return 1;
	}
	return 0;
    }
    req.arg = &mode;
    req.length = sizeof(mode);
    if (ioctl(s,query ? ATM_QUERYLOOP : ATM_GETLOOP,&req) < 0) {
	perror(query ? "ioctl ATM_QUERYLOOP" : "ioctl ATM_GETLOOP");
	return 1;
    }
    printf("local: ");
    print_levels(__ATM_LM_XTLOC(mode),query);
    printf("\nremote:");
    print_levels(__ATM_LM_XTRMT(mode),query);
    printf("\n");
    return 0;
}
