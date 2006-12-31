/* io.h - I/O operations */
 
/* Written 1995-1999 by Werner Almesberger, EPFL-LRC/ICA */
 

#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <atm.h>
#include <linux/atmsvc.h>

#include "proto.h"


int open_all(void);
void open_unix(const char *name);
void init_current_time(void);
void poll_loop(void);
void close_all(void);

void to_kernel(struct atmsvc_msg *msg);
void to_net(SIG_ENTITY *sig,void *msg,int size);

int get_addr(int itf,LOCAL_ADDR *local_addr);

int get_pvc(int itf,int *vci);

int get_link_rate(int itf);

#endif
