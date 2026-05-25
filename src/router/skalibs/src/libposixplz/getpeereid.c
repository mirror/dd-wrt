/* ISC license. */

#include <skalibs/sysdeps.h>

#ifdef SKALIBS_HASGETPEEREID
/* syscall exists - do nothing */

#else

#ifdef SKALIBS_HASSOPEERCRED
/* implementation with SO_PEERCRED */

#include <skalibs/nonposix.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <skalibs/posixplz.h>

int getpeereid (int s, uid_t *u, gid_t *g)
{
  struct ucred blah ;
  socklen_t len = sizeof(blah) ;

  if (getsockopt(s, SOL_SOCKET, SO_PEERCRED, &blah, &len) == -1)
    return -1 ;
  *u = blah.uid ;
  *g = blah.gid ;
  return 0 ;
}

#else

#ifdef SKALIBS_HASGETPEERUCRED
/* implementation with getpeerucred() */

#include <skalibs/nonposix.h>
#include <ucred.h>
#include <skalibs/posixplz.h>

int getpeereid (int s, uid_t *u, gid_t *g)
{
  ucred_t *cred = 0 ;
  if (getpeerucred(s, &cred) == -1) return -1 ;
  *u = ucred_geteuid(cred) ;
  *g = ucred_getegid(cred) ;
  ucred_free(cred) ;
  return 0 ;
}

#else

/* can't find a real implementation, make a stub */

#include <errno.h>
#include <skalibs/posixplz.h>

int getpeereid (int s, uid_t *uid, gid_t *gid)
{
  (void)s ;
  *uid = *gid = -1 ;
  errno = ENOSYS ;
  return -1 ;
}

#endif
#endif
#endif
