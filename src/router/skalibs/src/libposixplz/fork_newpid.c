/* ISC license. */

#include <skalibs/sysdeps.h>

#ifdef SKALIBS_HASCLONENEWPID

#include <skalibs/nonposix.h>

#include <sys/syscall.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <errno.h>

#include <skalibs/uint64.h>

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

pid_t fork_newpid (void)
{
  pid_t pid ;
  pid_t settid = 1 ;
  struct clone_args args =
  {
    .flags = CLONE_NEWPID | CLONE_PARENT_SETTID,
    .pidfd = 0,
    .child_tid = 0,
    .parent_tid = (uint64_t)&pid,
    .exit_signal = SIGCHLD,
    .stack = 0,
    .stack_size = 0,
    .tls = 0,
    .set_tid = (uint64_t)&settid,
    .set_tid_size = 1,
    .cgroup = 0
  } ;
  long r = syscall(SYS_clone3, &args, sizeof(args)) ;
  if (r < 0) return (errno = -r, -1) ;
  return r ? pid : 0 ;
}

#else

#include <unistd.h>
#include <errno.h>

pid_t fork_newpid (void)
{
  errno = ENOSYS ;
  return -1 ;
}

#endif
