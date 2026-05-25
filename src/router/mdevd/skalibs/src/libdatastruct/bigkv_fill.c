/* ISC license. */

#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <skalibs/bytestr.h>
#include <skalibs/stralloc.h>
#include <skalibs/genalloc.h>
#include <skalibs/avltree.h>
#include <skalibs/bigkv.h>

static void *bigkv_dtok (uint32_t d, void *p)
{
  bigkv *b = p ;
  return b->storage.s + genalloc_s(bigkv_node, &b->nodes)[d].k ;
}

static int bigkv_cmp (void const *a, void const *b, void *p)
{
  (void)p ;
  return strcmp((char const *)a, (char const *)b) ;
}

int bigkv_fill (bigkv *b, char const *const *argv, char delim, char const *prefix, char const *stop, uint32_t options)
{
  int i = 0 ;
  size_t prefixlen = prefix ? strlen(prefix) : 0 ;
  avltree_init(&b->map, 3, 3, 8, &bigkv_dtok, &bigkv_cmp, b) ;
  for (; argv[i] && !(stop && !strcmp(argv[i], stop)) ; i++)
  {
    bigkv_node node = { .k = b->storage.len } ;
    char const *s = argv[i] ;
    size_t len = strlen(s) ;
    size_t pos ;
    int isdup ;
    uint32_t d ;
    if (prefixlen)
    {
      if (!strncmp(s, prefix, prefixlen)) return i+1 ;
      s += prefixlen ;
      len -= prefixlen ;
    }
    pos = byte_chr(s, len, delim) ;
    if (!stralloc_catb(&b->storage, s, pos+1)) goto err ;
    b->storage.s[pos] = 0 ;
    isdup = avltree_search(&b->map, s, &d) ;
    if (isdup)
    {
      if (options & BIGKV_OPTIONS_NODUP) goto invalid ;
      b->storage.len = node.k ;
    }
    if (pos < len)
    {
      node.v = b->storage.len ;
      if (!stralloc_catb(&b->storage, s + pos + 1, len - pos)) goto err ;
    }
    else node.v = b->storage.len - 1 ;
    if (isdup) genalloc_s(bigkv_node, &b->nodes)[d].v = node.v ;
    else
    {
      d = genalloc_len(bigkv_node, &b->nodes) ;
      if (!genalloc_append(bigkv_node, &b->nodes, &node)) goto err ;
      if (!avltree_insert(&b->map, d)) goto err ;
    }
  }
  return i ;

 invalid:
   errno = EINVAL ;
 err:
  bigkv_free(b) ;
  return -1 ;
}
