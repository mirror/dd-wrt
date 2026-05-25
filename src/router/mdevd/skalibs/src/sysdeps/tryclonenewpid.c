/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sched.h>
#include <sys/syscall.h>
#include <stdint.h>
#include <unistd.h>

struct clone_args
{
  uint64_t flags ;
  uint64_t pidfd ;
  uint64_t child_tid ;
  uint64_t parent_tid ;
  uint64_t exit_signal ;
  uint64_t stack ;
  uint64_t stack_size ;
  uint64_t tls ;
  uint64_t set_tid ;
  uint64_t set_tid_size ;
  uint64_t cgroup ;
} ;

int main (void)
{
  struct clone_args args = { 0 } ;
  args.flags = CLONE_NEWPID ;
  syscall(SYS_clone3, &args, sizeof(struct clone_args)) ;
  return 0 ;
}
