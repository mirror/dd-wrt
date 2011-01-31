/* ed.c - eni memory dump */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <atm.h>
#include <linux/atmdev.h>
#include <linux/sonet.h>
#include <linux/atm_eni.h>


static void usage(const char *name)
{
    fprintf(stderr,"usage: %s itf\n",name);
    exit(1);
}


int main(int argc,char **argv)
{
    struct atmif_sioc req;
    int s;

    if (argc != 2) usage(argv[0]);
    if ((s = socket(PF_ATMPVC,SOCK_DGRAM,ATM_AAL5)) < 0) {
	perror("socket");
	return 1;
    }
    req.number = atoi(argv[1]);
    if (ioctl(s,ENI_MEMDUMP,&req) < 0) {
	perror("ioctl ENI_MEMDUMP");
	return 1;
    }
    return 0;
}
