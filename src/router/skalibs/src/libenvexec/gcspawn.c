/* ISC license. */

#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

#include <skalibs/types.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/djbunix.h>
#include <skalibs/cspawn.h>

pid_t gcspawn (char const *prog, char const *const *argv, char const *const *envp, uint16_t flags, cspawn_fileaction const *fa, size_t n)
{
  pid_t pid = 0 ;
  int wstat ;
  int p[2] ;
  char pack[PID_PACK] ;
  if (pipecoe(p) == -1) return 0 ;
  pid = fork() ;
  switch (pid)
  {
    case -1:
    {
      fd_close(p[1]) ;
      fd_close(p[0]) ;
      return 0 ;
    }
    case 0:
    {
      fd_close(p[0]) ;
      pid = cspawn(prog, argv, envp, flags, fa, n) ;
      if (!pid) _exit(errno) ;
      pid_pack_big(pack, pid) ;
      _exit(fd_write(p[1], pack, PID_PACK) < PID_PACK ? errno : 0) ;
    }
  }
  fd_close(p[1]) ;
  if (fd_read(p[0], pack, PID_PACK) < PID_PACK) goto err ;
  fd_close(p[0]) ;
  wait_pid(pid, &wstat) ;
  pid_unpack_big(pack, &pid) ;
  return pid ;

 err:
  fd_close(p[0]) ;
  wait_pid(pid, &wstat) ;
  return (errno = WIFSIGNALED(wstat) ? EINTR : WEXITSTATUS(wstat), 0) ;
}
