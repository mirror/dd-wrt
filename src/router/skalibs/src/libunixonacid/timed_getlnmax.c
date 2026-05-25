/* ISC license. */

#include <sys/types.h>
#include <skalibs/allreadwrite.h>
#include <skalibs/buffer.h>
#include <skalibs/functypes.h>
#include <skalibs/skamisc.h>
#include <skalibs/unix-timed.h>

struct blah_s
{
  buffer *b ;
  char *d ;
  size_t max ;
  size_t w ;
  char sep ;
} ;

static int getfd (struct blah_s *blah)
{
  return buffer_fd(blah->b) ;
}

static ssize_t get (struct blah_s *blah)
{
  return sanitize_read(getlnmax(blah->b, blah->d, blah->max, &blah->w, blah->sep)) ;
}

ssize_t timed_getlnmax (buffer *b, char *d, size_t max, size_t *w, char sep, tain const *deadline, tain *stamp)
{
  struct blah_s blah = { .b = b, .d = d, .max = max, .w = *w, .sep = sep } ;
  ssize_t r = timed_get(&blah, (init_func_ref)&getfd, (get_func_ref)&get, deadline, stamp) ;
  *w = blah.w ;
  return r ;
}
