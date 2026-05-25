/* ISC license. */

#include <sys/types.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/buffer.h>
#include <skalibs/functypes.h>
#include <skalibs/skamisc.h>
#include <skalibs/stralloc.h>
#include <skalibs/unix-timed.h>

struct blah_s
{
  buffer *b ;
  stralloc *sa ;
  char sep ;
} ;

static int getfd (struct blah_s *blah)
{
  return buffer_fd(blah->b) ;
}

static ssize_t get (struct blah_s *blah)
{
  return sanitize_read(skagetln(blah->b, blah->sa, blah->sep)) ;
}

int timed_getln (buffer *b, stralloc *sa, char sep, tain const *deadline, tain *stamp)
{
  struct blah_s blah = { .b = b, .sa = sa, .sep = sep } ;
  return timed_get(&blah, (init_func_ref)&getfd, (get_func_ref)&get, deadline, stamp) ;
}
