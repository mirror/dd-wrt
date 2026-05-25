/* ISC license. */

#include <errno.h>
#include <sys/stat.h>

#include <skalibs/djbunix.h>

static unsigned char cclass (char c)
{
  switch (c)
  {
    case 0 : return 0 ;
    case '/' : return 1 ;
    case '.' : return 2 ;
    default : return 3 ;
  }
}

 /* out must be at least strlen(in) + 2 bytes */

size_t path_canonicalize (char *out, char const *in, int check)
{
  static unsigned char const table[4][4] =
  {
    { 0x04, 0x00, 0x12, 0x11 },
    { 0x04, 0x50, 0x11, 0x11 },
    { 0x24, 0x20, 0x13, 0x11 },
    { 0xa4, 0xa0, 0x11, 0x11 }
  } ;
  int isabsolute = in[0] == '/' ;
  size_t j = 0 ;
  unsigned int depth = 0 ;
  unsigned char state = 0 ;

  if (isabsolute) *out++ = *in++ ;
  while (state < 4)
  {
    char c = *in++ ;
    unsigned char what = table[state][cclass(c)] ;
    state = what & 0x07 ;
    if (what & 0x80)
    {
      if (depth)
      {
        depth-- ;
        j -= 3 ;
        if (check)
        {
          struct stat st ;
          out[j] = 0 ;
          if (stat(out - isabsolute, &st) < 0) return 0 ;
          if (!S_ISDIR(st.st_mode)) return (errno = ENOTDIR, 0) ;
        }
      }
      else if (!isabsolute)
      {
        out[j++] = '/' ;
        out[j++] = '.' ;
      }
    }
    if (what & 0x40) depth++ ;
    if (what & 0x20) while (j && out[j-1] != '/') j-- ;
    if (what & 0x10) out[j++] = c ;
  }
  if (j && out[j-1] == '/') j-- ;
  if (!j && !isabsolute) out[j++] = '.' ;
  out[j] = 0 ;
  return j + isabsolute ;
}
