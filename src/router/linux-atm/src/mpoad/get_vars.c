#if HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdint.h>
#include <atm.h>
#include <linux/atmdev.h>
#include <sys/ioctl.h>  
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h> 
#include <net/if.h>
#include <arpa/inet.h>
#include <errno.h>
#include "packets.h"
#include "get_vars.h" 
#include "io.h"

extern struct mpc_control mpc_control; /* From main.c */

/* Returns the Time To Live value. */
int get_ttl(){
  int optvalue = 1;
  unsigned optlength = sizeof(optvalue);
  int sockfd = socket( AF_INET, SOCK_STREAM, IPPROTO_IP );
  getsockopt( sockfd, IPPROTO_IP, IP_TTL, &optvalue, &optlength );
  close(sockfd);
  return optvalue;
}

/* 
 * Returns clients own IP-address. According to interface number.
 *
 */ 
uint32_t get_own_ip_addr(int iface_nmbr ){
  struct ifreq req;
  int fd;
  char * addr;
  uint32_t address;
  char name[6];
  sprintf(name, "lec%d", iface_nmbr );
  memcpy(req.ifr_ifrn.ifrn_name,name,sizeof(name));
  fd =  socket( AF_INET, SOCK_STREAM, IPPROTO_IP );
  if(fd < 0){
    printf("mpcd: get_vars.c: socket creation failed.\n");
    exit(1);
  }
  if(ioctl(fd,SIOCGIFADDR,&req)<0){
    printf("mpcd: get_vars.c: ioctl failed: %s\n", strerror(errno));
    exit(1);
  }
  addr = req.ifr_ifru.ifru_addr.sa_data;
  address = ((unsigned char)addr[2] << 24) |
            ((unsigned char)addr[3] << 16) |
            ((unsigned char)addr[4] << 8 ) |
             (unsigned char)addr[5] ;
  close(fd);
  return address;
} 


int get_own_atm_addr(unsigned char  * address){
  memcpy(address,mpc_control.data_listen_addr.sas_addr.prv,ATM_ESA_LEN);
  return 1;
}







