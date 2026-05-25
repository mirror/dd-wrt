/* ISC license. */

#include <skalibs/sysdeps.h>

#ifdef SKALIBS_HASPOSIXSPAWN
#undef SKALIBS_POSIXSPAWNEARLYRETURN_WORKAROUND
#if defined(SKALIBS_HASPOSIXSPAWNEARLYRETURN) && defined(SKALIBS_HASWAITID)
# define SKALIBS_POSIXSPAWNEARLYRETURN_WORKAROUND
#endif 
#include <skalibs/nonposix.h>
#endif

#include <errno.h>
#include <unistd.h>

#include <skalibs/allreadwrite.h>
#include <skalibs/sig.h>
#include <skalibs/posixplz.h>
#include <skalibs/djbunix.h>
#include <skalibs/selfpipe.h>
#include <skalibs/exec.h>
#include <skalibs/cspawn.h>

static inline void cspawn_child_exec (char const *prog, char const *const *argv, char const *const *envp, uint16_t flags, cspawn_fileaction const *fa, size_t n)
{
  if (flags & CSPAWN_FLAGS_SELFPIPE_FINISH) selfpipe_finish() ;
  if (flags & CSPAWN_FLAGS_SIGBLOCKNONE) sig_blocknone() ;
  if (flags & CSPAWN_FLAGS_SETSID) setsid() ;

  for (size_t i = 0 ; i < n ; i++)
  {
    switch (fa[i].type)
    {
      case CSPAWN_FA_CLOSE : fd_close(fa[i].x.fd) ; break ;
      case CSPAWN_FA_COPY :
        if (fd_copy(fa[i].x.fd2[0], fa[i].x.fd2[1]) == -1) return ;
        break ;
      case CSPAWN_FA_MOVE :
        if (fd_move(fa[i].x.fd2[0], fa[i].x.fd2[1]) == -1) return ;
        if (fa[i].x.fd2[0] == fa[i].x.fd2[1] && uncoe(fa[i].x.fd2[0]) == -1) return ;
        break ;
      case CSPAWN_FA_OPEN :
      {
        int fd = open3(fa[i].x.openinfo.file, fa[i].x.openinfo.oflag, fa[i].x.openinfo.mode) ;
        if (fd == -1) return ;
        if (fd_move(fa[i].x.openinfo.fd, fd) == -1) return ;
        break ;
      }
      case CSPAWN_FA_CHDIR :
        if (chdir(fa[i].x.path) == -1) return ;
        break ;
      case CSPAWN_FA_FCHDIR :
        if (fchdir(fa[i].x.fd) == -1) return ;
        break ;
      default :
        errno = EINVAL ; return ;
    }
  }

  exec_ae(prog, argv, envp) ;
}

static inline pid_t cspawn_fork (char const *prog, char const *const *argv, char const *const *envp, uint16_t flags, cspawn_fileaction const *fa, size_t n)
{
  pid_t pid ;
  int p[2] ;
  char c ;

  if (pipecoe(p) == -1) return 0 ;
  pid = flags & CSPAWN_FLAGS_NEWPIDNS ? fork_newpid() : fork() ;
  if (pid == -1)
  {
    fd_close(p[1]) ;
    fd_close(p[0]) ;
    return 0 ;
  }
  if (!pid)
  {
    cspawn_child_exec(prog, argv, envp, flags & ~CSPAWN_FLAGS_NEWPIDNS, fa, n) ;
    c = errno ;
    fd_write(p[1], &c, 1) ;
    _exit(127) ;
  }

  fd_close(p[1]) ;
  p[1] = fd_read(p[0], &c, 1) ;
  if (p[1] < 0)
  {
    fd_close(p[0]) ;
    return 0 ;
  }
  fd_close(p[0]) ;
  if (p[1])
  {
    wait_pid(pid, &p[0]) ;
    errno = (unsigned char)c ;
    return 0 ;
  }
  return pid ;
}

#if defined(SKALIBS_HASPOSIXSPAWN) && (!defined(SKALIBS_HASPOSIXSPAWNEARLYRETURN) || defined(SKALIBS_POSIXSPAWNEARLYRETURN_WORKAROUND))

#include <signal.h>
#include <stdlib.h>
#include <spawn.h>

#include <skalibs/config.h>
#include <skalibs/djbunix.h>

#ifdef SKALIBS_POSIXSPAWNEARLYRETURN_WORKAROUND

#include <sys/wait.h>

static inline pid_t cspawn_workaround (pid_t pid, int const *p)
{
  siginfo_t si ;
  int e ;
  ssize_t r ;
  char c ;

  fd_close(p[1]) ;
  r = fd_read(p[0], &c, 1) ;
  fd_close(p[0]) ;
  if (r == -1) return 0 ;
  if (r) return (errno = EILSEQ, 0) ;  /* child wrote, wtf */

  int se = errno ;
  do e = waitid(P_PID, pid, &si, WEXITED | WNOHANG | WNOWAIT) ;
  while (e == -1 && errno == EINTR) ;
  errno = se ;
  if (e == -1) return pid ;  /* we're in trouble, but don't leak a child */
  if (!si.si_pid) return pid ;  /* child is running */
  if (si.si_code != CLD_EXITED || si.si_status != 127) return pid ; /* child died after execve(), let caller handle it */
  /*
    child exited 127, so either execve() failed, which is what we want to catch,
    or it raced like a mofo, execve()d and then exited 127 on its own, in which
    case, tough luck, it never existed.
  */
  wait_pid(pid, 0) ;
  return (errno = 0, 0) ;
}

#endif

static inline pid_t cspawn_pspawn (char const *prog, char const *const *argv, char const *const *envp, uint16_t flags, cspawn_fileaction const *fa, size_t n)
{
  pid_t pid ;
  posix_spawnattr_t attr ;
  posix_spawn_file_actions_t actions ;
  int e ;
  int nopath = !getenv("PATH") ;
#ifdef SKALIBS_POSIXSPAWNEARLYRETURN_WORKAROUND
  int p[2] ;
  if (pipecoe(p) == -1) return 0 ;
#endif

  if (flags)
  {
    short pfff = 0 ;
    e = posix_spawnattr_init(&attr) ;
    if (e) goto err ;
    if (flags & (CSPAWN_FLAGS_SIGBLOCKNONE | CSPAWN_FLAGS_SELFPIPE_FINISH))
    {
      sigset_t set ;
      sigemptyset(&set) ;
      e = posix_spawnattr_setsigmask(&attr, &set) ;
      if (e) goto errattr ;
      pfff |= POSIX_SPAWN_SETSIGMASK ;
    }
#ifdef SKALIBS_HASPOSIXSPAWNSETSID
    if (flags & CSPAWN_FLAGS_SETSID) pfff |= POSIX_SPAWN_SETSID ;
#else
#ifdef SKALIBS_HASPOSIXSPAWNSETSIDNP
    if (flags & CSPAWN_FLAGS_SETSID) pfff |= POSIX_SPAWN_SETSID_NP ;
#endif
#endif
    e = posix_spawnattr_setflags(&attr, pfff) ;
    if (e) goto errattr ;
  }

  if (n)
  {
    e = posix_spawn_file_actions_init(&actions) ;
    if (e) goto errattr ;
    for (size_t i = 0 ; i < n ; i++)
    {
      switch (fa[i].type)
      {
        case CSPAWN_FA_CLOSE :
          e = posix_spawn_file_actions_addclose(&actions, fa[i].x.fd) ;
          if (e) goto erractions ;
          break ;
        case CSPAWN_FA_COPY :
          e = posix_spawn_file_actions_adddup2(&actions, fa[i].x.fd2[1], fa[i].x.fd2[0]) ;
          if (e) goto erractions ;
          break ;
        case CSPAWN_FA_MOVE :
          e = posix_spawn_file_actions_adddup2(&actions, fa[i].x.fd2[1], fa[i].x.fd2[0]) ;
          if (e) goto erractions ;
          if (fa[i].x.fd2[0] != fa[i].x.fd2[1])
          {
            e = posix_spawn_file_actions_addclose(&actions, fa[i].x.fd2[1]) ;
            if (e) goto erractions ;
          }
          break ;
        case CSPAWN_FA_OPEN :
          e = posix_spawn_file_actions_addopen(&actions, fa[i].x.openinfo.fd, fa[i].x.openinfo.file, fa[i].x.openinfo.oflag, fa[i].x.openinfo.mode) ;
          if (e) goto erractions ;
          break ;
#ifdef SKALIBS_HASPOSIXSPAWNCHDIR
        case CSPAWN_FA_CHDIR :
          e = posix_spawn_file_actions_addchdir(&actions, fa[i].x.path) ;
          if (e) goto erractions ;
          break ;
        case CSPAWN_FA_FCHDIR :
          e = posix_spawn_file_actions_addfchdir(&actions, fa[i].x.fd) ;
          if (e) goto erractions ;
          break ;
#else
#ifdef SKALIBS_HASPOSIXSPAWNCHDIRNP
        case CSPAWN_FA_CHDIR :
          e = posix_spawn_file_actions_addchdir_np(&actions, fa[i].x.path) ;
          if (e) goto erractions ;
          break ;
        case CSPAWN_FA_FCHDIR :
          e = posix_spawn_file_actions_addfchdir_np(&actions, fa[i].x.fd) ;
          if (e) goto erractions ;
          break ;
#endif
#endif
        default :
          e = EINVAL ;
          goto erractions ;
      }
    }
  }

  if (nopath && (setenv("PATH", SKALIBS_DEFAULTPATH, 0) == -1)) { e = errno ; goto erractions ; }
  e = posix_spawnp(&pid, prog, n ? &actions : 0, flags ? &attr : 0, (char *const *)argv, (char *const *)envp) ;
  if (nopath) unsetenv("PATH") ;
  if (e) goto erractions ;

  if (n) posix_spawn_file_actions_destroy(&actions) ;
  if (flags) posix_spawnattr_destroy(&attr) ;
#ifdef SKALIBS_POSIXSPAWNEARLYRETURN_WORKAROUND
  return cspawn_workaround(pid, p) ;
#else
  return pid ;
#endif

 erractions:
  if (n) posix_spawn_file_actions_destroy(&actions) ;
 errattr:
  if (flags) posix_spawnattr_destroy(&attr) ;
 err:
#ifdef SKALIBS_POSIXSPAWNEARLYRETURN_WORKAROUND
  fd_close(p[1]) ;
  fd_close(p[0]) ;
#endif
  errno = e ;
  return 0 ;
}

pid_t cspawn (char const *prog, char const *const *argv, char const *const *envp, uint16_t flags, cspawn_fileaction const *fa, size_t n)
{
  if (flags & CSPAWN_FLAGS_NEWPIDNS) goto dofork ;
#if !defined(SKALIBS_HASPOSIXSPAWNSETSID) && !defined(SKALIBS_HASPOSIXSPAWNSETSIDNP)
  if (flags & CSPAWN_FLAGS_SETSID) goto dofork ;
#endif
#if !defined(SKALIBS_HASPOSIXSPAWNCHDIR) && !defined(SKALIBS_HASPOSIXSPAWNCHDIRNP)
  for (size_t i = 0 ; i < n ; i++)
    if (fa[i].type == CSPAWN_FA_CHDIR || fa[i].type == CSPAWN_FA_FCHDIR)
      goto dofork ;
#endif
  return cspawn_pspawn(prog, argv, envp, flags, fa, n) ;

 dofork:
  return cspawn_fork(prog, argv, envp, flags, fa, n) ;
}

#else

pid_t cspawn (char const *prog, char const *const *argv, char const *const *envp, uint16_t flags, cspawn_fileaction const *fa, size_t n)
{
  return cspawn_fork(prog, argv, envp, flags, fa, n) ;
}

#endif
