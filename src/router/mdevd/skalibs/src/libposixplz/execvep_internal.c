/* ISC license. */

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <skalibs/bytestr.h>
#include <skalibs/posixplz.h>

void execvep_internal (char const *file, char const *const *argv, char const *const *envp, char const *path)
{
  if (!path) errno = EINVAL ;
  else
  {
    size_t pathlen = strlen(path) + 1 ;
    size_t filelen = strlen(file) ;
    int savederrno = 0 ;
    while (pathlen)
    {
      size_t split = byte_chr(path, pathlen - 1, ':') ;
      if (split)
      {
        char tmp[split + 2 + filelen] ;
        memcpy(tmp, path, split) ;
        tmp[split] = '/' ;
        memcpy(tmp + split + 1, file, filelen + 1) ;
        execve(tmp, (char *const *)argv, (char *const *)envp) ;
        if (errno != ENOENT)
        {
          savederrno = errno ;
          if ((errno != EACCES) && (errno != EPERM) && (errno != EISDIR) && (errno != ENOTDIR)) break ;
        }
      }
      path += split+1 ; pathlen -= split+1 ;
    }
    if (savederrno) errno = savederrno ;
  }
}
