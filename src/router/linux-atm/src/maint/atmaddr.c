/* atmaddr.c - Get/set local ATM adresses */

/* Written 1995-2000 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <atm.h>
#include <linux/atmdev.h>


#define MAX_ADDR 1024 /* that should do it for now ... */


static void usage(const char *name)
{
    fprintf(stderr,"usage: %s [-n] [itf]\n",name);
    fprintf(stderr,"%6s %s -r [itf]\n","",name);
    fprintf(stderr,"%6s %s -a [itf] atm_addr\n","",name);
    fprintf(stderr,"%6s %s -d [itf] atm_addr\n","",name);
    fprintf(stderr,"%6s %s -V\n","",name);
    exit(1);
}


int main(int argc,char **argv)
{
    struct atmif_sioc req;
    struct sockaddr_atmsvc addr[MAX_ADDR];
    char buffer[MAX_ATM_ADDR_LEN+1];
    const char *itf,*name;
    char *end;
    int pretty;
    int cmd,s,i;

    req.number = 0;
    cmd = 0; /* for gcc */
    itf = NULL;
    pretty = A2T_PRETTY | A2T_NAME;
    name = *argv;
    if (argc == 2 && !strcmp(argv[1],"-V")) {
	printf("%s\n",VERSION);
	return 0;
    }
    if (argc > 1 && !strcmp(argv[1],"-n")) {
	pretty = A2T_PRETTY;
	argc--;
	argv++;
    }
    if (argc < 2) cmd = ATM_GETADDR;
    else if (*argv[1] != '-') {
	    if (argc != 2) usage(name);
	    cmd = ATM_GETADDR;
	    itf = argv[1];
	}
	else {
	    if (!argv[1][1] || argv[1][2]) usage(name);
	    switch (argv[1][1]) {
		case 'r':
		    if (argc < 2 || argc > 3) usage(name);
		    if (argc == 3) itf = argv[2];
		    cmd = ATM_RSTADDR;
		    break;
		case 'a':
		case 'd':
		    if (argc < 3 || argc > 4) usage(name);
		    if (argc == 4) itf = argv[2];
		    cmd = argv[1][1] == 'a' ? ATM_ADDADDR : ATM_DELADDR;
		    memset(&addr,0,sizeof(addr));
		    if (text2atm(argv[argc-1],(struct sockaddr *) addr,
		      sizeof(*addr),T2A_SVC | T2A_NAME) < 0) usage(name);
		    break;
		default:
		    usage(name);
	    }
	}
    if (itf) {
	req.number = strtoul(itf,&end,10);
	if (*end) usage(name);
    }
    if ((s = socket(PF_ATMSVC,SOCK_DGRAM,0)) < 0) {
	perror("socket");
	return 1;
    }
    req.arg = addr;
    req.length = cmd == ATM_GETADDR ? sizeof(addr) : sizeof(*addr);
    if (ioctl(s,cmd,&req) < 0) {
	perror("ioctl ATM_xxxADDR");
	return 1;
    }
    if (cmd != ATM_GETADDR) return 0;
    if (req.length % sizeof(*addr)) {
	fprintf(stderr,"internal error: len %d %% %d\n",req.length,
	  sizeof(*addr));
	return 1;
    }
    for (i = 0; i < req.length/sizeof(*addr); i++) {
	if (atm2text(buffer,MAX_ATM_ADDR_LEN+1,(struct sockaddr *) &addr[i],
	  pretty) < 0) {
	    fprintf(stderr,"bad address (internal error)\n");
	    return 1;
	}
	printf("%s\n",buffer);
    }
    return 0;
}
