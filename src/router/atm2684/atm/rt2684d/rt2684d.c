#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/atmclip.h>

#include "atm.h"
#include "atmd.h"

#define CONFIG_MIPS_BRCM
#include "atmdev.h"
#include "atmrt2684.h"

#define COMPONENT "RT2684D"

#define ATF_NULL        0x1000  /* use NULL encapsulation */
#define ATF_ARPSRV      0x2000  /* entry describes ARP server */
#define ATF_NOVC        0x4000  /* query only; do not create a VC */



/* Global Variables */
static char local_interface[32]="";
static char local_addr[32]="";
static char local_netmask[32]="";
static char local_ip[32]="";
#if 0
static char remote_ip[32]="";
#endif
static int vcc_encap=0;

static int pretty = A2T_PRETTY | A2T_NAME | A2T_LOCAL;

static int init_done=0;
static int setup_done=0;

static void usage(const char *name)
{
    fprintf(stderr,"usage: %s [ -i interface ] [ -a addr ] [ -o local ip ]"
    " [ -t local netmask ] [ -b ] [ -l logfile ] [ -n ] [ -V ]\n",name);
    fprintf(stderr,"%6s %s -V\n","",name);
    exit(1);
}

static int connect_vcc(struct sockaddr *remote,const struct atm_qos *qos,int sndbuf,
  int timeout)
{
    int fd,error,flags;
    struct atm_backend_rt2684 be;

    if (remote->sa_family == AF_ATMSVC) return -EUNATCH;

    if ((fd = socket(remote->sa_family,SOCK_DGRAM,0)) < 0) {
	error = -errno;
	diag(COMPONENT,DIAG_ERROR,"socket: %s",strerror(errno));
	return error;
    }
    if (setsockopt(fd,SOL_ATM,SO_ATMQOS,qos,sizeof(*qos)) < 0) {
	error = -errno;
	diag(COMPONENT,DIAG_ERROR,"setsockopt SO_ATMQOS: %s",strerror(errno));
	return error;
    }
    if (sndbuf)
	if (setsockopt(fd,SOL_SOCKET,SO_SNDBUF,&sndbuf,sizeof(sndbuf)) < 0) {
	    error = -errno;
	    diag(COMPONENT,DIAG_ERROR,"setsockopt SO_SNDBUF: %s",
	      strerror(errno));
	    return error;
	}
    if ((flags = fcntl(fd,F_GETFL)) < 0) {
	error = -errno;
	diag(COMPONENT,DIAG_ERROR,"fcntl F_GETFL: %s",strerror(errno));
	return error;
    }
    flags |= O_NONBLOCK;
    if (fcntl(fd,F_SETFL,flags) < 0) {
	error = -errno;
	diag(COMPONENT,DIAG_ERROR,"fcntl F_GETFL: %s",strerror(errno));
	return error;
    }

    /* PVC connect never blocks */
    if (connect(fd,remote,remote->sa_family == AF_ATMPVC ?
      sizeof(struct sockaddr_atmpvc) : sizeof(struct sockaddr_atmsvc)) < 0) {
	if (errno != EINPROGRESS) {
	    error = -errno;
	    diag(COMPONENT,DIAG_ERROR,"connect: %s",strerror(errno));
	    return error;
	}
	return fd;
    }

    /* attach the vcc to device: */
    be.backend_num = ATM_BACKEND_RT2684;
    be.ifspec.method = RT2684_FIND_BYIFNAME;
    sprintf(be.ifspec.spec.ifname, "atm%d", atoi(local_interface));
    be.encaps = vcc_encap;
    if (ioctl (fd, ATM_SETBACKEND, &be) < 0) {
	error = -errno;
	diag(COMPONENT,DIAG_ERROR,"ioctl : ATM_SETBACKEND %s",strerror(errno));
	return error;
    }

    return fd;
}

static void adjust_qos(struct atm_qos *qos,int null_encap)
{
    if (!qos->txtp.max_sdu)
	qos->txtp.max_sdu = RFC1626_MTU+(null_encap ? 0 : RFC1483LLC_LEN);
    if (!qos->rxtp.max_sdu)
	qos->rxtp.max_sdu = RFC1626_MTU+(null_encap ? 0 : RFC1483LLC_LEN);
}

/* Create atmxxx interface */
void rt2684_init(void)
{
    int reply,kernel;
    char cmd[128];
    struct atm_newif_rt2684 ni;

    // Create atmxxx interface
    kernel = socket(PF_ATMPVC, SOCK_DGRAM, ATM_AAL5);
    if (kernel<0) {
        diag(COMPONENT,DIAG_INFO,"socket: %s",strerror(errno));
        return;
    }

    ni.backend_num = ATM_BACKEND_RT2684;
    sprintf(ni.ifname, "atm%d", atoi(local_interface));

    reply = ioctl(kernel,ATM_NEWBACKENDIF,&ni);
    if (reply < 0) {
        diag(COMPONENT,DIAG_INFO,"create: %s",strerror(errno));
        return;
    }
    sleep(1);

    sprintf(cmd, "ifconfig atm%s %s netmask %s", local_interface, local_ip, local_netmask);
    system(cmd);

    close(kernel);
    init_done=1;
}

/* Assign vcc to a atmxxx device and connect to the ATM socket */
void rt2684_setup(void)
{
    struct atm_qos qos;
    struct sockaddr_atmpvc addr;
    int flags,sndbuf;
    int fd;
    char buffer[MAX_ATM_ADDR_LEN+1];

    text2atm(local_addr, (struct sockaddr *) &addr, sizeof(addr),T2A_PVC); // "0.0.40"
    if (!vcc_encap)
        flags = ATF_NULL;
    memset(&qos, 0, sizeof(qos));
    qos.aal = ATM_AAL5;
    qos.txtp.traffic_class = ATM_UBR;
    qos.txtp.max_sdu = RFC1483LLC_LEN+RFC1626_MTU;
    qos.rxtp = qos.txtp;
    sndbuf = 0;

    if (atm2text(buffer,MAX_ATM_ADDR_LEN+1,
      (struct sockaddr *) &addr,pretty) < 0) {
	diag(COMPONENT,DIAG_ERROR,"rt2684_setup: atm2text fails");
	return;
    }

    if (addr.sap_family != AF_ATMPVC) {
        diag(COMPONENT,DIAG_ERROR,"rt2684_setup: bad HA AF 0x%x",
	      addr.sap_family);
        return;
    }

    adjust_qos(&qos,flags & ATF_NULL);

    if ((fd = connect_vcc((struct sockaddr *)&addr,&qos,sndbuf,0)) < 0)
	return;
#if 0
    if (flags & ATF_NULL) {
	if ((result = set_encap(fd,0)) < 0) return;
	flags |= ATF_PERM;
    }
#endif

    setup_done=1;
}



#ifdef BUILD_STATIC
int rt2684d_main(int argc,char **argv)
#else
int main(int argc,char **argv)
#endif
{
    int c,background;

    background = 0;
    while ((c = getopt(argc,argv,"i:a:t:o:eb:l:nV")) != EOF)
	switch (c) {
	    case 'i':
		strcpy(local_interface, optarg);
		break;
	    case 'a':
		strcpy(local_addr, optarg);
		break;
	    case 't':
		strcpy(local_netmask, optarg);
		break;
	    case 'o':
		strcpy(local_ip, optarg);
		break;
#if 0
	    case 'r':
		strcpy(remote_ip, optarg);
		break;
#endif		
	    case 'e':
                vcc_encap=1;
		break;

	    case 'b':
		background = 1;
		break;
	    case 'l':
		set_logfile(optarg);
		break;
	    case 'n': /* @@@ was planned for NSAP matching */
		pretty = A2T_PRETTY;
		break;
	    case 'V':
		printf("%s\n",VERSION);
		return 0;
	    default:
		usage(argv[0]);
	}
    if (argc > optind || argc < 2) usage(argv[0]);

    if (!init_done)  rt2684_init();
    if (!setup_done) rt2684_setup();

    if (background) {
    	pid_t pid;

	pid = fork();
	if (pid < 0) diag(COMPONENT,DIAG_FATAL,"fork: %s",strerror(errno));
	if (pid) {
	    diag(COMPONENT,DIAG_DEBUG,"Backgrounding (PID %d)",pid);
	    exit(0);
	}
    }

    while (1) sleep(30);	/* to keep the sockets... */
    return 0;
}
