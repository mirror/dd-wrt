#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <signal.h>
#include <string.h>
#include <syslog.h>
#include <atm.h>
#include <linux/atmdev.h>
#include <linux/atmbr2684.h>
#ifndef BR2684_FLAG_ROUTED
#warning "Kernel missing routed support for br2684"
#define BR2684_FLAG_ROUTED    (1<<16) /* payload is routed, not bridged */
#endif

/* Written by Marcell GAL <cell@sch.bme.hu> to make use of the */
/* ioctls defined in the br2684... kernel patch */
/* Compile with cc -o br2684ctl br2684ctl.c -latm */

/*
  Modified feb 2001 by Stephen Aaskov (saa@lasat.com)
  - Added daemonization code
  - Added syslog
  
  TODO: Delete interfaces after exit?
*/


#define LOG_NAME       "br2684ctl"
#define LOG_OPTION     LOG_PERROR|LOG_PID
#define LOG_FACILITY   LOG_LOCAL2

struct br2684_params {
  int itfnum;
  int encap;
  int sndbuf;
  int payload;
  char *astr; /* temporary */
  struct atm_qos reqqos;
};


int lastsock, lastitf;


void fatal(const char *str, int err)
{
  syslog (LOG_ERR,"Fatal: %s; %s", str, strerror(err));
  exit(-2);
};


void exitFunc(void)
{
  syslog (LOG_NOTICE,"Daemon terminated");
}


void int_signal(int dummy)
{
  syslog (LOG_INFO,"Killed by a signal");
  exit(0);
}

int create_pidfile(int num)
{
  FILE *pidfile = NULL;
  char name[32];

  if (num < 0) return -1;

  snprintf(name, 32, "/var/run/br2684ctl-nas%d.pid", num);
  pidfile = fopen(name, "w");
  if (pidfile == NULL) return -1;
  fprintf(pidfile, "%d", getpid());
  fclose(pidfile);

  return 0;
}

int create_br(int itfnum, int payload)
{
  int err;
  
  if(lastsock<0) {
    lastsock = socket(PF_ATMPVC, SOCK_DGRAM, ATM_AAL5);
  }
  if (lastsock<0) {
    syslog(LOG_ERR, "socket creation failed: %s",strerror(errno));
  } else {
    /* create the device with ioctl: */
    if( itfnum>=0 && itfnum<1234567890){
      struct atm_newif_br2684 ni;
      ni.backend_num = ATM_BACKEND_BR2684;
      ni.media = BR2684_MEDIA_ETHERNET;
#ifdef BR2684_FLAG_ROUTED
      if (payload == 0)
        ni.media |= BR2684_FLAG_ROUTED;
#endif
      ni.mtu = 1500;
      sprintf(ni.ifname, "nas%d", itfnum);
      err=ioctl (lastsock, ATM_NEWBACKENDIF, &ni);
  
      if (err == 0)
	syslog(LOG_NOTICE, "Interface \"%s\" created sucessfully",ni.ifname);
      else
	syslog(LOG_INFO, "Interface \"%s\" could not be created, reason: %s",
	       ni.ifname,
	       strerror(errno));
      lastitf=itfnum;	/* even if we didn't create, because existed, 
			assign_vcc wil want to know it! */
    } else {
      syslog(LOG_ERR,"err: strange interface number %d", itfnum );
    }
  }
  return 0;
}


int assign_vcc(char *astr, int encap, int payload,
               int bufsize, struct atm_qos qos)
{
    int err;
    struct sockaddr_atmpvc addr;
    int fd;
    struct atm_backend_br2684 be;

    memset(&addr, 0, sizeof(addr));
    err=text2atm(astr,(struct sockaddr *)(&addr), sizeof(addr), T2A_PVC);
    if (err!=0)
      syslog(LOG_ERR,"Could not parse ATM parameters (error=%d)",err);
    
    syslog(LOG_NOTICE,"Communicating over ATM %d.%d.%d, encapsulation: %s",
	   addr.sap_addr.itf,
	   addr.sap_addr.vpi,
	   addr.sap_addr.vci,
	   encap?"VC mux":"LLC");
    
    if ((fd = socket(PF_ATMPVC, SOCK_DGRAM, ATM_AAL5)) < 0)
      syslog(LOG_ERR,"failed to create socket %d, reason: %s", 
	     errno,strerror(errno));
    
    if (qos.aal == 0) {
      qos.aal                     = ATM_AAL5;
      qos.txtp.traffic_class      = ATM_UBR;
      qos.txtp.max_sdu            = 1524;
      qos.txtp.pcr                = ATM_MAX_PCR;
      qos.rxtp = qos.txtp;
    }

    if ( (err=setsockopt(fd,SOL_SOCKET,SO_SNDBUF, &bufsize ,sizeof(bufsize))) )
      syslog(LOG_ERR,"setsockopt SO_SNDBUF: (%d) %s",err, strerror(err));
    
    if (setsockopt(fd, SOL_ATM, SO_ATMQOS, &qos, sizeof(qos)) < 0)
      syslog(LOG_ERR,"setsockopt SO_ATMQOS %d", errno);

    err = connect(fd, (struct sockaddr*)&addr, sizeof(struct sockaddr_atmpvc));
    
    if (err < 0)
      fatal("failed to connect on socket", errno);
    
    /* attach the vcc to device: */
    
    be.backend_num = ATM_BACKEND_BR2684;
    be.ifspec.method = BR2684_FIND_BYIFNAME;
    sprintf(be.ifspec.spec.ifname, "nas%d", lastitf);
    be.fcs_in = BR2684_FCSIN_NO;
    be.fcs_out = BR2684_FCSOUT_NO;
    be.fcs_auto = 0;
    be.encaps = encap ? BR2684_ENCAPS_VC : BR2684_ENCAPS_LLC;
    be.has_vpiid = 0;
    be.send_padding = 0;
    be.min_size = 0;
    err=ioctl (fd, ATM_SETBACKEND, &be);
    if (err == 0)
      syslog (LOG_INFO,"Interface configured");
    else {
      syslog (LOG_ERR,"Could not configure interface:%s",strerror(errno));
      exit(2);
    }
    return fd ;
}

void start_interface(struct br2684_params* params)
{
  if (params->astr==NULL) {
    syslog(LOG_ERR, "Required ATM parameters not specified.");
    exit(1);
  }

  create_br(params->itfnum, params->payload);
  assign_vcc(params->astr, params->encap, params->payload, params->sndbuf,
	     params->reqqos);
}


void usage(char *s)
{
  printf("usage: %s [-b] [[-c number] [-e 0|1] [-s sndbuf] [-q qos] [-p 0|1] "
        "[-a [itf.]vpi.vci]*]*\n"
           " -b               = run in background (daemonize)\n"
           " -c <num>         = use interface nas<num>\n"
           " -e 0|1           = encapsulation (0=LLC, 1=VC Mux)\n"
           " -p 0|1           = payload type (0=routed,1=bridged)\n"
           " -s <num>         = set sndbuf (send buffer) size (default 8192)\n"
           " -a [itf.]vpi.vci = ATM interface no, VPI, VCI\n",
           s);
  exit(1);
}



int main (int argc, char **argv)
{
  int c, background=0;

  struct br2684_params params;
  params.itfnum=-1;
  params.encap=0;
  params.sndbuf=8192;
  params.payload=1;
  params.astr=NULL;
  memset(&params.reqqos, 0, sizeof(params.reqqos));
  
  lastsock=-1;
  lastitf=0;
  
  /* st qos to 0 */

  openlog (LOG_NAME,LOG_OPTION,LOG_FACILITY);
  if (argc>1)
    while ((c = getopt(argc, argv,"q:a:bc:e:s:p:?h")) !=EOF)
      switch (c) {
      case 'q':
	printf ("optarg : %s",optarg);
	if (text2qos(optarg,&params.reqqos,0))
	  fprintf(stderr,"QOS parameter invalid\n");
	break;
      case 'a':
	params.astr=optarg;
	break;
      case 'b':
	background=1;
	break;
      case 'c':
	/* temporary, to make it work with multiple interfaces: */
	if (params.itfnum>=0) start_interface(&params);
	params.itfnum= atoi(optarg);
	break;
      case 'e':
	params.encap=(atoi(optarg));
	if(params.encap<0){
	  syslog (LOG_ERR, "invalid encapsulation: %s:",optarg);
	  params.encap=0;
	}
	break;
      case 's':
	params.sndbuf=(atoi(optarg));
	if(params.sndbuf<0){
	  syslog(LOG_ERR, "Invalid sndbuf: %s, using size of 8192 instead", 
		 optarg);
	  params.sndbuf=8192;
	}
	break;
      case 'p':	/* payload type: routed (0) or bridged (1) */
#ifdef BR2684_FLAG_ROUTED
	params.payload = atoi(optarg);
	break;
#else
	syslog(LOG_ERR, "payload option not supported.");
#endif
      case '?':
      case 'h':
      default:
	usage(argv[0]);
      }
  else
    usage(argv[0]);

  if (argc != optind) usage(argv[0]);
  
  start_interface(&params);  

  if(lastsock>=0) close(lastsock);
  
  if (background) {
    pid_t pid;
    
    pid=fork();
    if (pid < 0) {
      fprintf(stderr,"Error detaching\n");
      exit(2);
    } else if (pid) 
      exit(0); // This is the parent
    
    // Become a process group and session group leader
    if (setsid()<0) {
      fprintf (stderr,"Could not set process group\n");
      exit(2);
    }
    
    // Fork again to let process group leader exit
    pid = fork();
    if (pid < 0) {
      fprintf(stderr,"Error detaching during second fork\n");
      exit(2);
    } else if (pid)
      exit(0); // This is the parent
    
    // Now we're ready for buisness
    chdir("/");            // Don't keep directories in use
    close(0); close(1); close(2);  // Close stdin, -out and -error
    /*
      Note that this implementation does not keep an open 
      stdout/err.
      If we need them they can be opened now
    */
    
  }
  
  create_pidfile(params.itfnum);
  signal(SIGINT, int_signal);
  signal(SIGTERM, int_signal);

  syslog (LOG_INFO, "RFC 1483/2684 bridge daemon started");
  atexit (exitFunc);
  
  while (1) pause();	/* to keep the sockets... */
  return 0;
}

