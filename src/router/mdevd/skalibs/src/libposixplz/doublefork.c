/* ISC license. */

#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include <skalibs/uint64.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/djbunix.h>
#include <skalibs/posixplz.h>

pid_t doublefork ()
{
  char pack[8] ;
  int fd[2] ;
  pid_t child ;
  if (pipe(fd) == -1) return -1 ;
  child = fork() ;
  switch (child)
  {
    case -1:
    {
      fd_close(fd[1]) ;
      fd_close(fd[0]) ;
      return -1 ;
    }
    case 0:
    {
      pid_t pid ;
      fd_close(fd[0]) ;
      pid = fork() ;
      switch (pid)
      {
        case -1: _exit(errno) ;
        case 0: fd_close(fd[1]) ; return 0 ;
      }
      uint64_pack_big(pack, pid) ;
      _exit((allwrite(fd[1], pack, 8) < 8) ? errno : 0) ;
    }
  }
  fd_close(fd[1]) ;
  {
    uint64_t grandchild = 0 ;
    int wstat ;
    if (allread(fd[0], pack, 8) < 8) grandchild = 1 ;
    fd_close(fd[0]) ;
    wait_pid(child, &wstat) ;
    if (grandchild) return (errno = WIFSIGNALED(wstat) ? EINTR : WEXITSTATUS(wstat), -1) ;
    uint64_unpack_big(pack, &grandchild) ;
    return (pid_t)grandchild ;
  }
}
