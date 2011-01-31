#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/utsname.h>
#include <getopt.h>
#include <time.h>
#include <sys/ioctl.h>
#include <atm.h>
#include <linux/types.h>
#include <linux/atmdev.h>
#include <linux/atmmpc.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <atm.h>
#include <linux/if_ether.h>
#include <signal.h>
#include "packets.h"
#include "io.h"
#include "k_interf.h"
#include "get_vars.h"
#include "lecs.h"

/*
 * Global struct containing sockets addresses and parameters.
 */
struct mpc_control mpc_control;

static void create_kernel_socket(int itf);
static int listen_to_MPS( struct sockaddr_atmsvc ctrl_listen_addr );
static int set_mps_mac_addr(void);
static void usage(const char *progname);

static void signal_handler(int sig){
  struct k_message msg;
  memset(&msg,0,sizeof(struct k_message));
  if (sig == SIGHUP) msg.type = RELOAD;
  else msg.type = CLEAN_UP_AND_EXIT;
  send_to_kernel(&msg);
  printf("mpcd: main.c: signal_handler() signal %d\n", sig);
  return;
}

/*
 * Initialize our listen addresses for
 * the incoming/outgoing MPOA connections
 */
static void init_default_addresses(struct sockaddr_atmsvc *ctrl, struct sockaddr_atmsvc *data)
{
        struct sockaddr_atmsvc sa;
        struct atmif_sioc req;
        int fd;
	unsigned char wellknown_lecs[ATM_ESA_LEN];
	memset(wellknown_lecs,0,ATM_ESA_LEN);
	wellknown_lecs[0] = 0x47;
	wellknown_lecs[2] = 0x79;
	wellknown_lecs[14] = 0xa0;
	wellknown_lecs[15] = 0x3e;
	wellknown_lecs[18] = 0x01;
        fd = get_socket(NULL);
        req.number=0;
        req.arg=&sa;
        req.length=sizeof(struct sockaddr_atmsvc);
        if ( ioctl(fd, ATM_GETADDR, &req) <0 ){
                perror("mpcd: main.c: ioctl(ATM_GETADDR)");
                exit(-1);
        }
        ctrl->sas_family = AF_ATMSVC;       
        data->sas_family = AF_ATMSVC;
	mpc_control.lecs_address.sas_family = AF_ATMSVC;
        /* ATM address for the incoming/outgoing MPOA control connections */
        sa.sas_addr.prv[ATM_ESA_LEN-1] = 50;
        memcpy(ctrl->sas_addr.prv, sa.sas_addr.prv, ATM_ESA_LEN);
        memcpy(mpc_control.OWN_ATM_ADDRESS, ctrl->sas_addr.prv, ATM_ESA_LEN);
	memcpy(mpc_control.lecs_address.sas_addr.prv,wellknown_lecs, ATM_ESA_LEN);
        /* ATM address for the incoming/outgoing MPOA shortcuts */
        sa.sas_addr.prv[ATM_ESA_LEN-1] = 51;
        memcpy(data->sas_addr.prv, sa.sas_addr.prv, ATM_ESA_LEN);
        close(fd);
	
        return;
}

int main(int argc, char **argv){
  int listen_socket;
  int opt_ret = 0;
  struct k_message msg;
  struct sockaddr_atmsvc control_listen_addr;
  struct sockaddr_atmsvc mps_ctrl_addr;
  struct sockaddr_atmsvc lec_addr;

  memset(&control_listen_addr,0,sizeof(struct sockaddr_atmsvc));
  memset(&mpc_control.data_listen_addr,0,sizeof(struct sockaddr_atmsvc));
  memset(&lec_addr,0,sizeof(struct sockaddr_atmsvc));
  memset(&mps_ctrl_addr,0,sizeof(struct sockaddr_atmsvc));
  memset(&msg,0,sizeof(struct k_message));
  memset(&mpc_control,0,sizeof(mpc_control));
  mpc_control.elan_name[32] = '\0';
  init_default_addresses(&control_listen_addr, &mpc_control.data_listen_addr);
    
  while( opt_ret != -1 ){
    opt_ret = getopt(argc, argv, "h:s:l:c:L:n:C:i:m:");
    switch(opt_ret) {
    case 'h':
      usage(argv[0]);
      exit(0);
      break;
    case 's':
      if(text2atm(optarg,(struct sockaddr *)&control_listen_addr,
		  sizeof(struct sockaddr_atmsvc),T2A_SVC | T2A_NAME)<0){
	printf("mpcd: main.c: text2atm failed.\n");
	usage(argv[0]);
        exit(1);
      }
      memcpy(mpc_control.OWN_ATM_ADDRESS,control_listen_addr.sas_addr.prv, ATM_ESA_LEN);
      break;
    case 'l':
      if(text2atm(optarg,(struct sockaddr *)&mpc_control.data_listen_addr,
		  sizeof(struct sockaddr_atmsvc),T2A_SVC | T2A_NAME)<0){
	printf("mpcd: main.c: text2atm failed.\n");
	usage(argv[0]);
	exit(1);
      }
      break;
    case 'c':
      if(text2atm(optarg,(struct sockaddr *)&mps_ctrl_addr,
		  sizeof(struct sockaddr_atmsvc),T2A_SVC | T2A_NAME)<0){
	printf("mpcd: main.c: text2atm failed.\n");
	usage(argv[0]);
	exit(1);
      }
      memcpy(mpc_control.MPS_CTRL_ATM_ADDR,mps_ctrl_addr.sas_addr.prv,ATM_ESA_LEN);
      mpc_control.mps_ctrl_addr_set = 1;
      break;
    case 'L':
      if(text2atm(optarg,(struct sockaddr *)&lec_addr,
		  sizeof(struct sockaddr_atmsvc),T2A_SVC | T2A_NAME)<0){
	printf("mpcd: main.c: text2atm failed.\n");
	usage(argv[0]);
	exit(1);
      }
      memcpy(mpc_control.LEC_ADDRESS,lec_addr.sas_addr.prv,ATM_ESA_LEN);
      mpc_control.use_lecs = 1;
      break;
    case 'n':
      strncpy(mpc_control.elan_name,optarg,33);
      break;
    case 'C':
      if(text2atm(optarg,(struct sockaddr *)&mpc_control.lecs_address,
		  sizeof(struct sockaddr_atmsvc),T2A_SVC | T2A_NAME)<0){
	printf("mpcd: main.c: text2atm failed.\n");
	usage(argv[0]);
	exit(1);
      }
      break;
    case 'm':
      strncpy(mpc_control.MPS_MAC_ADDRESS,optarg,13);
      mpc_control.mps_mac_addr_set = 1;
      break;
    case 'i':
      mpc_control.INTERFACE_NUMBER = atoi(optarg);
      break;
    }
  }
  if (argc != optind) {
    usage(argv[0]);
    exit(1);
  }
  while(1){
    create_kernel_socket(mpc_control.INTERFACE_NUMBER);
    if(mpc_control.use_lecs){
      get_mpc_config(&mpc_control.lecs_address, mpc_control.LEC_ADDRESS, mpc_control.elan_name);
    }
    msg.type = SET_MPC_CTRL_ADDR; 
    memcpy(msg.MPS_ctrl,mpc_control.OWN_ATM_ADDRESS,ATM_ESA_LEN);
    if (send_to_kernel(&msg) < 0) {
      printf("mpcd: main.c: send_to_kernel(SET_MPC_CTRL_ADDR) failed\n");
      exit(1);
    }
    if(mpc_control.mps_mac_addr_set)
      set_mps_mac_addr();
    listen_to_MPS( control_listen_addr );
    if ( (listen_socket = get_listen_socket(&mpc_control.data_listen_addr)) < 0) 
      {
      printf("mpcd: main.c: listen_socket creation failed\n");
      exit (1);
      }
    
    signal(SIGHUP, signal_handler);
    signal(SIGINT, signal_handler);
    signal(SIGQUIT, signal_handler);
    signal(SIGABRT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    main_loop(listen_socket);
    sleep(5);
    printf("mpcd: main.c: going back to main loop...\n");
  }
  return 0;
}

static void create_kernel_socket(int itf)
{
        mpc_control.kernel_socket = socket(PF_ATMSVC, SOCK_DGRAM, 0);
        if (mpc_control.kernel_socket < 0) {
                printf("mpcd: main.c: kernel socket creation failed: %s\n", strerror(errno));
                exit (1);
        }
        if ( ioctl(mpc_control.kernel_socket, ATMMPC_CTRL, itf) < 0) {
                printf("mpcd: main.c: kernel socket ioctl(ATMMPC_CTRL) failed: %s\n", strerror(errno));
                exit (1);
        }
	return;
}

static int listen_to_MPS( struct sockaddr_atmsvc ctrl_listen_addr ){
        /* soketti, joka kuuntelee MPC:n Control ATM-osoitteessa */
        mpc_control.MPS_listen_socket = get_listen_socket(&ctrl_listen_addr); 
        if (mpc_control.MPS_listen_socket < 0)
	        return -1;
	return 0;
}

static int set_mps_mac_addr(){
     char *string = mpc_control.MPS_MAC_ADDRESS;
     struct k_message msg;
     unsigned char mac_addr[ETH_ALEN];
     int tmp;
     int i = strlen(string);
     memset(&msg,0,sizeof(struct k_message));
     if (i != 12){
       printf("mpcd: main.c: incorrect mac address.\n");
       exit(1);
     }
     for(i=0;i<6;i++) {
             sscanf(&string[i*2],"%2x",&tmp);
	     mac_addr[i]=(unsigned char)tmp;
     }
     msg.type = SET_MPS_MAC_ADDR;
     memcpy(&msg.MPS_ctrl,&mac_addr,ETH_ALEN);
     send_to_kernel(&msg);
     return 0;
}

static void usage( const char * progname ){
        printf("Usage: %s [-s our_control_listen_ATM_Address]  [-l our_data_listen_address]\n"
               "              [-c MPS_control_ATM_Address] [-i interface_number]\n"
	       "              [-m MPS_MAC_address]\n"
	       "              [-L lec_address [-n elan_name [-C lecs_address]]]\n",
               progname);
        return;
}
