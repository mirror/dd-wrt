/* ISC license. */

#include <sys/types.h>
#include <skalibs/disize.h>
#include <skalibs/stralloc.h>
#include <skalibs/genalloc.h>
#include <skalibs/djbunix.h>
#include <skalibs/unixmessage.h>

void unixmessage_sender_free (unixmessage_sender *b)
{
  size_t n = genalloc_len(int, &b->fds) ;
  if (n)
  {
    size_t i = genalloc_s(disize, &b->offsets)[b->head].right ;
    for (; i < n ; i++)
    {
      int fd = genalloc_s(int, &b->fds)[i] ;
      if (fd < 0) (*b->closecb)(-(fd+1), b->closecbdata) ;
    }
  }
  genalloc_free(disize, &b->offsets) ;
  genalloc_free(int, &b->fds) ;
  stralloc_free(&b->data) ;
  *b = unixmessage_sender_zero ;
}
