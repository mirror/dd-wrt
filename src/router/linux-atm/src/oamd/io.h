/* io.h - I/O operations */
 
/* Written 1995-1999 by Werner Almesberger, EPFL-LRC/ICA */
 

#ifndef IO_H
#define IO_H

#include <stdint.h>
#include <sys/socket.h> /* for struct sockaddr */
#include <atm.h> /* for struct sockaddr_atmsvc */
#include <atmd.h>
#include <linux/atmoam.h>

void open_kernel(void);
void recv_kernel(void);
int send_kernel(struct atmoam_ctrl *);
void close_kernel(void);
void poll_loop(void);
#endif
