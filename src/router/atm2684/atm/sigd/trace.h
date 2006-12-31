/* trace.h - Support functions for message tracing */
 
/* Written 1996,1998 by Werner Almesberger, EPFL-LRC */
 

#ifndef TRACE_H
#define TRACE_H

#include <linux/atmsvc.h>

#include "proto.h"


extern int trace_size;

void trace_msg(const char *msg);
void trace_uni(const char *comment,const SIG_ENTITY *sig,const void *msg,
  int size);
void trace_kernel(const char *comment,const struct atmsvc_msg *msg);
char *get_trace(void);

#endif
