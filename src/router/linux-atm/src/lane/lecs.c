/*
 *
 * LECS main code
 *
 * $Id: lecs.c,v 1.2 2001/10/09 22:33:07 paulsch Exp $
 *
 */

#if HAVE_CONFIG_H
#include <config.h>
#endif

/* Standard includes*/
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>

/* ATM includes */
#include <atm.h>

/* Local includes */
#include "lecs_load.h"
#include "ldb.h"
#include "mem_lecs.h"
#include "lecs.h"
#include "atm_lecs.h"

/* Protos */
static void sig_reset(int foobar);
static void sig_kill(int foobar);
static void usage(const char *progname);
int send_response(int fd, unsigned char *buffer, int len);

/* Local data */
#define COMP_NAME "MAIN"
#define MAX_FD 32
#define P_SIZE 1024
static int stay_alive = 1;
static int reset = 0;

#define DUMP_PACKETS 0

static void
usage(const char *progname)
{
  printf("Usage: %s [-f configuration_file][-l listen_address][-d]\n",
	 progname);
}

static void
sig_reset(int foobar)
{
  reset = 1;
}

static void
sig_kill(int foobar)
{
  stay_alive = 0;
}

int 
send_response(int fd, unsigned char *buffer, int len)
{
  LaneControl_t *dp;
  Elan_t *elan;
  unsigned short response;
#ifdef DUMP_PACKETS
  int i;
#endif

  if (len < sizeof(LaneControl_t))
    return -1;

  dp = (LaneControl_t *)buffer;

  if (dp->marker != htons(LE_MARKER) ||
      dp->protocol != LE_PROTOCOL ||
      dp->version != LE_VERSION ||
      dp->opcode != htons(LE_CONFIGURE_REQUEST)) {
    return -1;
  }
  dp->opcode = htons(LE_CONFIGURE_RESPONSE);
  elan = find_elan(dp->source_atm, dp->lan_type, dp->max_frame,
		   (char*)dp->elan_name, dp->elan_name_size, &response);
  if (!elan) {
    dp->status = htons(response);
  } else {
    dp->status = htons(LE_STATUS_SUCCESS);
    dp->lan_type = elan->type;
    dp->max_frame = elan->max_frame;
    memcpy(dp->elan_name, elan->elan_name, elan->elan_name_size);
    dp->elan_name_size = elan->elan_name_size;
    memcpy(dp->target_atm, elan->les_addr, ATM_ESA_LEN);
  }
#ifdef DUMP_PACKETS
  for(i=0;i<len;i++) {
    printf("%2.2x ",0xff&buffer[i]);
  }
  printf("\n");
#endif
  return write(fd, buffer, len);
}

int main(int argc, char **argv)
{
  int i =0;
  char *config_file =NULL;
  char *listen_addr = NULL;
  int fd_arr[MAX_FD];
  int no_fds=1;
  int just_dump=0;
  fd_set fds;
  struct sockaddr_atmsvc client;
  socklen_t len;
  unsigned char buffer[P_SIZE];

  while(i!=-1) {
    i = getopt(argc, argv, "f:l:d");
    switch(i) {
    case 'd':
      printf("Dumping databasefile\n");
      just_dump=1;
      break;
    case 'f':
      if (config_file) {
	usage(argv[0]);
	exit(-1);
      }
      config_file = (char*)mem_alloc(COMP_NAME, strlen(optarg)+1);
      if (!config_file) {
	exit(-1);
      }
      memcpy(config_file, optarg, strlen(optarg)+1);
      break;
    case 'l':
      if (listen_addr) {
	usage(argv[0]);
	exit(-1);
      }
      listen_addr = (char*)mem_alloc(COMP_NAME, strlen(optarg)+1);
      if (!listen_addr)
	exit(-1);
      memcpy(listen_addr, optarg, strlen(optarg)+1);
      break;
    case -1:
      break;
    default:
      usage(argv[0]);
      exit(-1);
    }
  }
  if (argc != optind) {
    usage(argv[0]);
    exit(-1);
  }
  /* Following gets run in the beginning or when lecs is restarted */
  while (stay_alive) {

    /* Read configuration file */
    if (config_file) {
      if (load_db(config_file)<0)
	exit(-1);
    } else {
      if (load_db(DEFAULT_CONFIG)<0)
	exit(-1);
    }
    if (just_dump) {
      dump_db(NULL);
      exit(0);
    }

    /* Reserve signals */
    signal(SIGHUP, sig_reset);
    signal(SIGINT, sig_kill);
    signal(SIGQUIT, sig_kill);
    signal(SIGABRT, sig_kill);
    signal(SIGTERM, sig_kill);
    signal(SIGSEGV, sig_kill);
    
    /* CHANGE: First parameter, then configuration file! */
    fd_arr[0] = atm_create_socket(CONFIGURATION_DIRECT, 
				  get_lecs_addr());
    no_fds=1;
    if (fd_arr[0] <0) {
      stay_alive=0; /* No need to go on */
    }
    while(!reset && stay_alive) {
      FD_ZERO(&fds);
      for(i=0;i<no_fds;i++) {
	FD_SET(fd_arr[i],&fds);
      }
      
      if (select(MAX_FD, &fds, NULL, NULL, NULL)<0) {
	perror("select(MAX_FD,...)");
	stay_alive=0;
      } else {
	if (FD_ISSET(fd_arr[0],&fds)) { /* Incoming call */
	  if (no_fds == MAX_FD) {
	    close(fd_arr[1]); /* Oldest */
	    memmove(&fd_arr[1], &fd_arr[2], sizeof(int)*(MAX_FD-2));
	    no_fds--;
	  }
	  len = sizeof(client);
	  fd_arr[no_fds] = accept(fd_arr[0], (struct sockaddr*)&client,
				  &len);
	  if (fd_arr[no_fds]<0) {
	    if (errno==ENETRESET)
	      reset=1;
	    if (errno==EUNATCH)
	      stay_alive=1;
	  } else {
	    no_fds++;
	  }
	}
	for(i=1;i<no_fds;i++) {
	  if (FD_ISSET(fd_arr[i],&fds)) {
	    len = read(fd_arr[i], buffer, P_SIZE);
	    if (len <0 && (errno == ENETRESET || errno == EUNATCH)) {
	      reset=0;
	    }
	    if (len<=0) {
	      close(fd_arr[i]);
	      memmove(&fd_arr[i], &fd_arr[i+1], sizeof(int)*(--no_fds -i));
	      i--;
	    } else {
	      if(send_response(fd_arr[i], buffer, len)<0) {
		close(fd_arr[i]);
		memmove(&fd_arr[i], &fd_arr[i+1], sizeof(int)*(--no_fds -i));
	      }
	    }
	  }
	}
      }
    }
    /* This gets done if a signal has been caught, or if
       network resets/becomes unavailable */
    reset=0;
    for(i=0;i<no_fds;i++)
      close(fd_arr[i]);
    no_fds=0;
    reset_db();
  }
  return 0;
}
