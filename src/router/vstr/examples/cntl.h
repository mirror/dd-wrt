#ifndef CNTL_H
#define CNTL_H

#include "evnt.h"

extern void cntl_make_file(Vlg *, const char *);

extern void cntl_child_make(unsigned int);
extern void cntl_child_free(void);

extern void cntl_child_pid(pid_t, int);

extern void cntl_sc_multiproc(Vlg *, unsigned int, int, int);

#endif
