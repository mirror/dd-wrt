/* ISC license. */

#ifndef SKALIBS_PROG_H
#define SKALIBS_PROG_H

#include <stddef.h>

#include <skalibs/types.h>

extern char const *PROG ;

#define PROG_pid_len(name) (sizeof(name) + 5 + PID_FMT)

#define PROG_pid_fill(buf, name) prog_pid_fill(buf, (name), sizeof(name)-1)
#define PROG_pid_set(buf, name) do { PROG_pid_fill(buf, name) ; PROG = buf ; } while (0)

extern void prog_pid_fill (char *, char const *, size_t) ;

#endif
