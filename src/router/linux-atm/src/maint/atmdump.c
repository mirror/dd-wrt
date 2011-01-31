/* atmdump.c - ATM raw cell dumper */

/* Written 1995-1998 by Werner Almesberger, EPFL-LRC/ICA */

#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h> /* for htonl and ntohl */
#include <atm.h>


static const char *pti[] = { "Data SDU 0","Data SDU 1","Data SDU 0, CE",
  "Data SDU 1, CE","Segment OAM F5","End-to-end OAM F5","Reserved (RM)",
  "Reserved" };

static int interval = 0; /* display absolute time by default */


#define GET(item) ((*cell & ATM_HDR_##item##_MASK) >> ATM_HDR_##item##_SHIFT)


static void analyze(unsigned long *cell,struct timeval stamp)
{
    static struct timeval last;
    static int first = 1;
    int i;

    if (first || !interval) {
	printf("%2d:%02d:%02d",(int) ((stamp.tv_sec/3600) % 24),
	  (int) ((stamp.tv_sec/60) % 60),(int) (stamp.tv_sec % 60));
	if (interval) {
	    first = 0;
	    last = stamp;
	}
    }
    else {
	struct timeval diff;

	diff.tv_sec = stamp.tv_sec-last.tv_sec;
	diff.tv_usec = stamp.tv_usec-last.tv_usec;
	while (diff.tv_usec < 0) {
	    diff.tv_usec += 1000000;
	    diff.tv_sec--;
	}
	last = stamp;
	printf("%8ld",(long) diff.tv_sec);
    }
    printf(".%06ld: VPI=%ld VCI=%ld, GFC=0x%lx, CLP=%ld, %s (PTI %ld)\n",
      (long) stamp.tv_usec,GET(VPI),GET(VCI),GET(GFC),*cell & ATM_HDR_CLP,
      pti[GET(PTI)],GET(PTI));
    for (i = 0; i < ATM_CELL_PAYLOAD; i++) {
	if (!(i & 15)) printf("   ");
	printf("%02x ",((unsigned char *) cell)[i+4]);
	if ((i & 15) == 15) putchar('\n');
    }
}


static void usage(const char *name)
{
    int i;

    fprintf(stderr,"usage: %s [ -i ] [ -t type [ -g gfc ] [ -c ] ] "
      "[itf.]vpi.vci\n",name);
    for (i = 0; i < 8; i++)
	fprintf(stderr,"  %-6s %d %s\n",i ? "" : "type",i,pti[i]);
    fprintf(stderr,"  gfc    0-15, default 0\n");
    exit(1);
}


int main(int argc,char **argv)
{
    unsigned long buf[13]; /* ugly */
    struct sockaddr_atmpvc addr;
    struct atm_qos qos;
    char *name,*end;
    int type,gfc,clp;
    int c,s,size;

    type = -1;
    gfc = clp = 0; /* for GCC */
    name = argv[0];
    while ((c = getopt(argc,argv,"t:g:c")) != EOF)
	switch (c) {
	    case 'i':
		interval = 1;
		break;
	    case 't':
		type = strtol(optarg,&end,0);
		if (*end || type < 0 || type > 7) usage(name);
		break;
	    case 'g':
		gfc = strtol(optarg,&end,0);
		if (*end || gfc < 0 || gfc > 15) usage(name);
		break;
	    case 'c':
		clp = 1;
		break;
	    default:
		usage(name);
	}
    if (argc != optind+1) usage(name);
    if ((s = socket(PF_ATMPVC,SOCK_DGRAM,0)) < 0) {
	perror("socket");
	return 1;
    }
    memset(&addr,0,sizeof(addr));
    if (text2atm(argv[optind],(struct sockaddr *) &addr,sizeof(addr),T2A_PVC)
      < 0) usage(name);
    memset(&qos,0,sizeof(qos));
    qos.aal = ATM_AAL0;
    if (type == -1) {
	qos.rxtp.traffic_class = ATM_UBR;
	qos.rxtp.max_sdu = 52;
    }
    else {
	qos.txtp.traffic_class = ATM_UBR;
	qos.txtp.max_sdu = 52;
    }
    if (setsockopt(s,SOL_ATM,SO_ATMQOS,&qos,sizeof(qos)) < 0) {
	perror("setsockopt SO_ATMQOS");
	return 1;
    }
    if (bind(s,(struct sockaddr *) &addr,sizeof(addr)) < 0) {
	perror("bind");
	return 1;
    }
    if (type == -1) {
	while ((size = read(s,buf,52)) == 52) {
	    struct timeval stamp;

	    if (ioctl(s,SIOCGSTAMP,&stamp) < 0) {
		perror("ioctl SIOCGSTAMP");
		return 1;
	    }
	    analyze(buf,stamp);
	    fflush(stdout);
	}
	if (size < 0) perror("read");
	else fprintf(stderr,"short read (%d bytes)\n",size);
	return 1;
    }
    if ((size = read(0,buf+1,ATM_CELL_PAYLOAD)) < 0) {
	perror("read stdin");
	return 1;
    }
    if (size < ATM_CELL_PAYLOAD)
	memset((unsigned char *) (buf+1)+size,0,ATM_CELL_PAYLOAD-size);
    *buf = (gfc << ATM_HDR_GFC_SHIFT) |
      (addr.sap_addr.vpi << ATM_HDR_VPI_SHIFT) |
      (addr.sap_addr.vci << ATM_HDR_VCI_SHIFT) |
      (type << ATM_HDR_PTI_SHIFT) | clp;
    if ((size = write(s,buf,ATM_CELL_SIZE-1)) < 0) {
	perror("write to network");
	return 1;
    }
    if (size < ATM_CELL_SIZE-1) {
	fprintf(stderr,"short write (%d bytes)\n",size);
	return 1;
    }
    return 0;
}
