/* io.h - I/O operations */
 
/* Written 1995-1999 by Werner Almesberger, EPFL-LRC/ICA */
 

#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <sys/socket.h> /* for struct sockaddr */
#include <atm.h> /* for struct sockaddr_atmsvc */
#include <atmd.h>

#include "table.h"


void open_all(void);
void close_all(void);
void notify(const UN_CTX *ctx,uint32_t ip,const ENTRY *entry);
int do_close(int fd);
void poll_loop(void);
int connect_vcc(struct sockaddr *remote,const struct atm_qos *qos,int sndbuf,
  int timeout);
int set_ip(int fd,int ip);
int set_encap(int fd,int mode);
void set_sndbuf(VCC *vcc);
void send_packet(int fd,void *data,int length);
int ip_itf_info(int number,uint32_t *ip,uint32_t *netmask,int *mtu);
int get_local(int fd,struct sockaddr_atmsvc *addr);

#endif
