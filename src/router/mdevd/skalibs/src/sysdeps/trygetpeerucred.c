/* ISC license. */

#undef _POSIX_C_SOURCE
#undef _XOPEN_SOURCE

#include <sys/types.h>
#include <ucred.h>

int main (void)
{
  ucred_t *cred ;
  uid_t uid ;
  gid_t gid ;
  int s = 0 ;
  getpeerucred(s, &cred) ;
  uid = ucred_geteuid(cred) ;
  gid = ucred_getegid(cred) ;
  ucred_free(cred) ;
  return 0 ;
}
