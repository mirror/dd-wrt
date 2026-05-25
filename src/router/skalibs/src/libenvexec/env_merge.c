/* ISC license. */

#include <string.h>
#include <errno.h>

#include <skalibs/bytestr.h>
#include <skalibs/env.h>

size_t env_merge (char const **v, size_t vmax, char const *const *envp, size_t envlen, char const *modifs, size_t modiflen)
{
  size_t vlen = envlen ;
  size_t i = 0 ;
  if (envlen >= vmax) return 0 ;
  for (; i < envlen ; i++) v[i] = envp[i] ;
  for (i = 0 ; i < modiflen ; i += strlen(modifs + i) + 1)
  {
    size_t split = str_chr(modifs + i, '=') ;
    size_t j = 0 ;
    for (; j < vlen ; j++)
      if (!strncmp(modifs + i, v[j], split) && (v[j][split] == '=')) break ;
    if (j < vlen) v[j] = v[--vlen] ;
    if (modifs[i + split])
    {
      if (vlen >= vmax) return 0 ;
      v[vlen++] = modifs + i ;
    }
  }
  if (vlen >= vmax) return 0 ;
  v[vlen++] = 0 ;
  return vlen ;
}
