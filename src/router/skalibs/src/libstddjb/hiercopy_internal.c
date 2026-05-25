/* ISC license. */

#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include <skalibs/stralloc.h>
#include <skalibs/direntry.h>
#include <skalibs/fcntl.h>
#include <skalibs/djbunix.h>

static int dircopy (char const *src, char const *dst, mode_t mode, stralloc *tmp, unsigned int options)
{
  size_t tmpbase = tmp->len ;
  size_t maxlen = 0 ;
  if (sals(src, tmp, &maxlen) == -1) return 0 ;

  if (mkdir(dst, S_IRWXU) < 0)
  {
    struct stat st ;
    if (errno != EEXIST) goto err ;
    if (stat(dst, &st) < 0) goto err ;
    if (!S_ISDIR(st.st_mode)) { errno = ENOTDIR ; goto err ; }
  }
  {
    size_t srclen = strlen(src) ;
    size_t dstlen = strlen(dst) ;
    size_t i = tmpbase ;
    char srcbuf[srclen + maxlen + 2] ;
    char dstbuf[dstlen + maxlen + 2] ;
    memcpy(srcbuf, src, srclen) ;
    memcpy(dstbuf, dst, dstlen) ;
    srcbuf[srclen] = '/' ;
    dstbuf[dstlen] = '/' ;
    while (i < tmp->len)
    {
      size_t n = strlen(tmp->s + i) + 1 ;
      memcpy(srcbuf + srclen + 1, tmp->s + i, n) ;
      memcpy(dstbuf + dstlen + 1, tmp->s + i, n) ;
      i += n ;
      hiercopy_internal(srcbuf, dstbuf, tmp, 0) ;
    }
  }
  if (chmod(dst, mode) < 0 && !(options & 1)) goto err ;
  tmp->len = tmpbase ;
  return 1 ;
err:
  tmp->len = tmpbase ;
  return 0 ;
}

int hiercopy_internal (char const *src, char const *dst, stralloc *tmp, unsigned int options)
{
  struct stat st ;
  if (lstat(src, &st) < 0) return 0 ;
  if (S_ISREG(st.st_mode))
  {
    if (!filecopy_unsafe(src, dst, st.st_mode)) return 0 ;
  }
  else if (S_ISDIR(st.st_mode))
  {
    if (!dircopy(src, dst, st.st_mode, tmp, options)) return 0 ;
  }
  else if (S_ISFIFO(st.st_mode))
  {
    if (mkfifo(dst, st.st_mode) < 0) return 0 ;
  }
  else if (S_ISLNK(st.st_mode))
  {
    size_t tmpbase = tmp->len ;
    if (sareadlink(tmp, src) < 0) return 0 ;
    if (!stralloc_0(tmp) || symlink(tmp->s + tmpbase, dst) < 0)
    {
      tmp->len = tmpbase ;
      return 0 ;
    }
    tmp->len = tmpbase ;
  }
  else if (S_ISCHR(st.st_mode) || S_ISBLK(st.st_mode) || S_ISSOCK(st.st_mode))
  {
    if (mknod(dst, st.st_mode, st.st_rdev) < 0) return 0 ;
  }
  else return (errno = ENOTSUP, 0) ;
  lchown(dst, st.st_uid, st.st_gid) ;
  if (!S_ISLNK(st.st_mode)) chmod(dst, st.st_mode) ;
  return 1 ;
}
