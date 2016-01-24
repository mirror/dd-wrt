#ifndef VSTR__HEADER_H
# error " You must _just_ #include <vstr.h>"
#endif
/*
 *  Copyright (C) 2002, 2003, 2004, 2005  James Antill
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  email: james@and.org
 */
/* exported functions which are inlined */
/* NOTE: this implementation can change when the ABI changes ... DO NOT use
 * undocumented knowledge from here */

#ifndef VSTR__ASSERT
#  define VSTR__ASSERT(x) /* do nothing */
#endif

#ifndef VSTR__ASSERT_RET
#  define VSTR__ASSERT_RET(x, y)   do { if (x) {} else return (y); } while (0)
#endif

#ifndef VSTR__ASSERT_RET_VOID
#  define VSTR__ASSERT_RET_VOID(x) do { if (x) {} else return;     } while (0)
#endif

#ifndef VSTR__ASSERT_NO_SWITCH_DEF
#  define VSTR__ASSERT_NO_SWITCH_DEF() break; default: break
#endif

/* cast so it warns for ptrs */
#undef  VSTR__TRUE
#define VSTR__TRUE  ((int)1)
#undef  VSTR__FALSE
#define VSTR__FALSE ((int)0)

#ifdef VSTR_AUTOCONF_USE_WRAP_MEMCPY
extern inline void *vstr_wrap_memcpy(void *passed_s1, const void *passed_s2,
                                     size_t n)
{
  unsigned char *s1 = passed_s1;
  const unsigned char *s2 = passed_s2;

  if ((n > 16) || VSTR__AT_COMPILE_CONST_P(n))
    memcpy(passed_s1, passed_s2, n);
  else switch (n)
  {
    case 16:  s1[15] = s2[15];
    case 15:  s1[14] = s2[14];
    case 14:  s1[13] = s2[13];
    case 13:  s1[12] = s2[12];
    case 12:  s1[11] = s2[11];
    case 11:  s1[10] = s2[10];
    case 10:  s1[ 9] = s2[ 9];
    case  9:  s1[ 8] = s2[ 8];
    case  8:  s1[ 7] = s2[ 7];
    case  7:  s1[ 6] = s2[ 6];
    case  6:  s1[ 5] = s2[ 5];
    case  5:  s1[ 4] = s2[ 4];
    case  4:  s1[ 3] = s2[ 3];
    case  3:  s1[ 2] = s2[ 2];
    case  2:  s1[ 1] = s2[ 1];
    case  1:  s1[ 0] = s2[ 0];
    case  0:
      break;
  }

  return (passed_s1);
}
#else
# define vstr_wrap_memcpy(x, y, z)  memcpy(x, y, z)
#endif

#ifdef VSTR_AUTOCONF_USE_WRAP_MEMCMP
extern inline int vstr_wrap_memcmp(const void *passed_s1,
                                   const void *passed_s2, size_t n)
{
  const unsigned char *s1 = passed_s1;
  const unsigned char *s2 = passed_s2;
  int ret = 0;
  int tmp = 0;

  if ((n > 16) || VSTR__AT_COMPILE_CONST_P(n))
    ret = memcmp(passed_s1, passed_s2, n);
  else switch (n)
  {
    case 16:  tmp = s1[15] - s2[15]; if (tmp) ret = tmp;
    case 15:  tmp = s1[14] - s2[14]; if (tmp) ret = tmp;
    case 14:  tmp = s1[13] - s2[13]; if (tmp) ret = tmp;
    case 13:  tmp = s1[12] - s2[12]; if (tmp) ret = tmp;
    case 12:  tmp = s1[11] - s2[11]; if (tmp) ret = tmp;
    case 11:  tmp = s1[10] - s2[10]; if (tmp) ret = tmp;
    case 10:  tmp = s1[ 9] - s2[ 9]; if (tmp) ret = tmp;
    case  9:  tmp = s1[ 8] - s2[ 8]; if (tmp) ret = tmp;
    case  8:  tmp = s1[ 7] - s2[ 7]; if (tmp) ret = tmp;
    case  7:  tmp = s1[ 6] - s2[ 6]; if (tmp) ret = tmp;
    case  6:  tmp = s1[ 5] - s2[ 5]; if (tmp) ret = tmp;
    case  5:  tmp = s1[ 4] - s2[ 4]; if (tmp) ret = tmp;
    case  4:  tmp = s1[ 3] - s2[ 3]; if (tmp) ret = tmp;
    case  3:  tmp = s1[ 2] - s2[ 2]; if (tmp) ret = tmp;
    case  2:  tmp = s1[ 1] - s2[ 1]; if (tmp) ret = tmp;
    case  1:  tmp = s1[ 0] - s2[ 0]; if (tmp) ret = tmp;
    case  0:
      break;
  }

  return (ret);
}
#else
# define vstr_wrap_memcmp(x, y, z)  memcmp(x, y, z)
#endif

#ifdef VSTR_AUTOCONF_USE_WRAP_MEMCHR
extern inline void *vstr_wrap_memchr(const void *passed_s1, int c, size_t n)
{
  const unsigned char *s1 = passed_s1;
  const void *ret = 0;
  int tmp = 0;

  switch (n)
  {
    case 16:  tmp = s1[15] == c; if (tmp) ret = s1 + 15;
    case 15:  tmp = s1[14] == c; if (tmp) ret = s1 + 14;
    case 14:  tmp = s1[13] == c; if (tmp) ret = s1 + 13;
    case 13:  tmp = s1[12] == c; if (tmp) ret = s1 + 12;
    case 12:  tmp = s1[11] == c; if (tmp) ret = s1 + 11;
    case 11:  tmp = s1[10] == c; if (tmp) ret = s1 + 10;
    case 10:  tmp = s1[ 9] == c; if (tmp) ret = s1 +  9;
    case  9:  tmp = s1[ 8] == c; if (tmp) ret = s1 +  8;
    case  8:  tmp = s1[ 7] == c; if (tmp) ret = s1 +  7;
    case  7:  tmp = s1[ 6] == c; if (tmp) ret = s1 +  6;
    case  6:  tmp = s1[ 5] == c; if (tmp) ret = s1 +  5;
    case  5:  tmp = s1[ 4] == c; if (tmp) ret = s1 +  4;
    case  4:  tmp = s1[ 3] == c; if (tmp) ret = s1 +  3;
    case  3:  tmp = s1[ 2] == c; if (tmp) ret = s1 +  2;
    case  2:  tmp = s1[ 1] == c; if (tmp) ret = s1 +  1;
    case  1:  tmp = s1[ 0] == c; if (tmp) ret = s1 +  0;
    case  0:
      break;
    default: ret = memchr(s1, c, n);
      break;
  }

  return ((void *)ret);
}
#else
# define vstr_wrap_memchr(x, y, z)  memchr(x, y, z)
#endif

#ifdef VSTR_AUTOCONF_USE_WRAP_MEMSET
extern inline void *vstr_wrap_memset(void *passed_s1, int c, size_t n)
{
  unsigned char *s1 = passed_s1;

  if ((n > 16) || VSTR__AT_COMPILE_CONST_P(n))
    memset(passed_s1, c, n);
  else switch (n)
  {
    case 16:  s1[15] = c;
    case 15:  s1[14] = c;
    case 14:  s1[13] = c;
    case 13:  s1[12] = c;
    case 12:  s1[11] = c;
    case 11:  s1[10] = c;
    case 10:  s1[ 9] = c;
    case  9:  s1[ 8] = c;
    case  8:  s1[ 7] = c;
    case  7:  s1[ 6] = c;
    case  6:  s1[ 5] = c;
    case  5:  s1[ 4] = c;
    case  4:  s1[ 3] = c;
    case  3:  s1[ 2] = c;
    case  2:  s1[ 1] = c;
    case  1:  s1[ 0] = c;
    case  0:
      break;
  }

  return (passed_s1);
}
#else
# define vstr_wrap_memset(x, y, z)  memset(x, y, z)
#endif

#ifdef VSTR_AUTOCONF_USE_WRAP_MEMMOVE
extern inline void *vstr_wrap_memmove(void *s1, const void *s2, size_t n)
{
  if (n < 16)
  {
    unsigned char tmp[16];
    vstr_wrap_memcpy(tmp,  s2, n);
    vstr_wrap_memcpy(s1, tmp, n);
    return (s1);
  }

  return memmove(s1, s2, n);
}
#else
# define vstr_wrap_memmove(x, y, z) memmove(x, y, z)
#endif

/* needed at the top so vstr_del() etc. can use it */
extern inline void vstr_ref_del(struct Vstr_ref *tmp)
{
  if (!tmp)
    return; /* std. free semantics */

  if (!--tmp->ref)
    (*tmp->func)(tmp);
}

extern inline struct Vstr_ref *vstr_ref_add(struct Vstr_ref *tmp)
{
  ++tmp->ref;

  return (tmp);
}

extern inline void *vstr_cache_get(const struct Vstr_base *base,
                                   unsigned int pos)
{
  if (!pos)
    return (NULL);

  if (!base->cache_available)
    return (NULL);

  VSTR__ASSERT(VSTR__CACHE(base));
  
  --pos;

  if (pos >= VSTR__CACHE(base)->sz)
    return (NULL);

  return (VSTR__CACHE(base)->data[pos]);
}

extern inline void *vstr_data_get(struct Vstr_conf *conf,
                                  unsigned int pos)
{
  struct Vstr_ref *data = NULL;
  
  if (!conf)
    return (vstr_extern_inline_data_get(pos));
  
  VSTR__ASSERT_RET(pos && (pos <= conf->data_usr_len), NULL);

  if (!(data = conf->data_usr_ents[pos - 1].data))
    return (NULL);
  
  return (data->ptr);
}

extern inline void vstr_data_set(struct Vstr_conf *conf,
                                 unsigned int pos,  struct Vstr_ref *ref)
{
  if (!conf)
  {
    vstr_extern_inline_data_set(pos, ref);
    return;
  }
  
  VSTR__ASSERT_RET_VOID(pos && (pos <= conf->data_usr_len));

  vstr_ref_del(conf->data_usr_ents[pos - 1].data);
  conf->data_usr_ents[pos - 1].data = ref ? vstr_ref_add(ref) : NULL;
}

extern inline
int vstr_cache__pos(const struct Vstr_base *base,
                    struct Vstr_node *node, size_t pos, unsigned int num)
{
  struct Vstr__cache_data_pos *data = NULL;

  if (!base->cache_available)
    return (VSTR__FALSE);

  data = vstr_cache_get(base, 1);
  VSTR__ASSERT(data);

  data->node = node;
  data->pos = pos;
  data->num = num;

  return (VSTR__TRUE);
}

extern inline size_t vstr_sc_posdiff(size_t beg_pos, size_t end_pos)
{
  return ((end_pos - beg_pos) + 1);
}

extern inline size_t vstr_sc_poslast(size_t beg_pos, size_t diff_pos)
{
  return (beg_pos + (diff_pos - 1));
}

extern inline
struct Vstr_node *vstr_base__pos(const struct Vstr_base *base,
                                 size_t *pos, unsigned int *num, int cache)
{
  size_t orig_pos = *pos;
  struct Vstr_node *scan = base->beg;
  struct Vstr__cache_data_pos *data = NULL;
  unsigned int dummy_num = 0;

  if (!num) num = &dummy_num;

  *pos += base->used;
  *num = 1;

  if (*pos <= base->beg->len)
    return (base->beg);

  /* must be more than one node */

  if (orig_pos > (base->len - base->end->len))
  {
    *pos = orig_pos - (base->len - base->end->len);
    *num = base->num;
    return (base->end);
  }

  if ((data = vstr_cache_get(base, 1)) && data->node && (data->pos <= orig_pos))
  {
    scan = data->node;
    *num = data->num;
    *pos = (orig_pos - data->pos) + 1;
  }

  while (*pos > scan->len)
  {
    *pos -= scan->len;

    VSTR__ASSERT(scan->next);
    scan = scan->next;
    ++*num;
  }

  if (cache)
    vstr_cache__pos(base, scan, (orig_pos - *pos) + 1, *num);

  return (scan);
}

extern inline char *vstr_export__node_ptr(const struct Vstr_node *node)
{
  switch (node->type)
  {
    case VSTR_TYPE_NODE_BUF:
      return (((struct Vstr_node_buf *)node)->buf);
    case VSTR_TYPE_NODE_PTR:
      return (((const struct Vstr_node_ptr *)node)->ptr);
    case VSTR_TYPE_NODE_REF:
      return (((char *)((const struct Vstr_node_ref *)node)->ref->ptr) +
              ((const struct Vstr_node_ref *)node)->off);
    case VSTR_TYPE_NODE_NON:
      VSTR__ASSERT_NO_SWITCH_DEF();
  }

  return (NULL);
}

extern inline char vstr_export_chr(const struct Vstr_base *base, size_t pos)
{
  struct Vstr_node *node = NULL;
  const char *tmp = NULL;
  
  node = vstr_base__pos(base, &pos, NULL, VSTR__TRUE);
  VSTR__ASSERT(pos);
  
  /* errors, requests for data from NON nodes and real data are all == 0 */
  if (!node) return (0);

  if (!(tmp = vstr_export__node_ptr(node)))
    return (0);
  
  return (*(tmp + --pos));
}

extern inline size_t vstr_iter_len(struct Vstr_iter *iter)
{
  return (iter->len + iter->remaining);
}
extern inline size_t vstr_iter_pos(struct Vstr_iter *iter,
                                   size_t pos, size_t len)
{
  VSTR__ASSERT_RET((len >= (iter->len + iter->remaining)), 0);
    
  return (pos + (len - vstr_iter_len(iter)));
}

extern inline int vstr_iter_fwd_beg(const struct Vstr_base *base,
                                    size_t pos, size_t len,
                                    struct Vstr_iter *iter)
{
  VSTR__ASSERT_RET(base && iter, 0);

  iter->node = NULL;

  VSTR__ASSERT_RET(pos && (((pos <= base->len) &&
                            (vstr_sc_poslast(pos, len) <= base->len)) || !len),
                   VSTR__FALSE);

  if (!len)
    return (VSTR__FALSE);
  
  iter->node = vstr_base__pos(base, &pos, &iter->num, VSTR__TRUE);
  --pos;

  iter->len = iter->node->len - pos;
  if (iter->len > len)
    iter->len = len;
  len -= iter->len;

  iter->remaining = len;

  iter->ptr = NULL;
  if (iter->node->type != VSTR_TYPE_NODE_NON)
    iter->ptr = vstr_export__node_ptr(iter->node) + pos;

  return (VSTR__TRUE);
}

extern inline int vstr_iter_fwd_nxt(struct Vstr_iter *iter)
{
  if (!iter->remaining)
  {
    iter->len  = 0;
    iter->node = NULL;
    return (VSTR__FALSE);
  }

  iter->node = iter->node->next;
  ++iter->num;

  iter->len = iter->node->len;

  VSTR__ASSERT(iter->len);

  if (iter->len > iter->remaining)
    iter->len = iter->remaining;
  iter->remaining -= iter->len;

  iter->ptr = NULL;
  if (iter->node->type != VSTR_TYPE_NODE_NON)
    iter->ptr = vstr_export__node_ptr(iter->node);

  return (VSTR__TRUE);
}

extern inline char vstr_iter_fwd_chr(struct Vstr_iter *iter, unsigned int *ern)
{
  unsigned int dummy_ern;
  
  if (!ern)
    ern = &dummy_ern;

  if (!iter->len && !vstr_iter_fwd_nxt(iter))
  {
    *ern = VSTR_TYPE_ITER_END;
    return (0);
  }
  
  VSTR__ASSERT(iter->len);
  --iter->len;
  
  VSTR__ASSERT(iter->node);
  if (iter->node->type == VSTR_TYPE_NODE_NON)
  {
    *ern = VSTR_TYPE_ITER_NON;
    return (0);
  }
  
  *ern = VSTR_TYPE_ITER_DEF;
  return (*iter->ptr++);
}

extern inline size_t vstr_iter_fwd_buf(struct Vstr_iter *iter, size_t len,
                                       void *passed_buf, size_t buf_len,
                                       unsigned int *ern)
{
  size_t orig_len = len;
  char *buf = passed_buf;
  unsigned int dummy_ern;
  
  VSTR__ASSERT(buf || !buf_len);

  if (!ern)
    ern = &dummy_ern;

  if (!iter->len && !vstr_iter_fwd_nxt(iter))
  {
    *ern = VSTR_TYPE_ITER_END;
    return (0);
  }

  *ern = VSTR_TYPE_ITER_DEF;
  while ((iter->len || vstr_iter_fwd_nxt(iter)) && len)
  {
    size_t tmp = len;
    size_t tmp_buf_len = 0;
    
    VSTR__ASSERT(iter->len);
    VSTR__ASSERT(iter->node);

    if (tmp > iter->len)
      tmp = iter->len;
    
    tmp_buf_len = tmp;
    if (tmp_buf_len > buf_len)
      tmp_buf_len = buf_len;
    
    len       -= tmp;
    iter->len -= tmp;
    
    if (tmp_buf_len && (iter->node->type != VSTR_TYPE_NODE_NON))
      vstr_wrap_memcpy(buf, iter->ptr, tmp_buf_len);
    if (tmp_buf_len)
    {
      buf       += tmp_buf_len;
      buf_len   -= tmp_buf_len;
    }
    if (iter->node->type != VSTR_TYPE_NODE_NON)
      iter->ptr += tmp;
  }

  return (orig_len - len);
}

extern inline size_t vstr_iter_fwd_cstr(struct Vstr_iter *iter, size_t len,
                                        char *buf, size_t buf_len,
                                        unsigned int *ern)
{
  size_t ret = 0;
  
  VSTR__ASSERT(buf);

  if (!buf_len)
    return (0);
  
  --buf_len;
  buf[buf_len] = 0;
  ret = vstr_iter_fwd_buf(iter, len, buf, buf_len, ern);
  if (ret < buf_len)
    buf[ret] = 0;
  
  return (ret);
}

extern inline unsigned int vstr_num(const struct Vstr_base *base,
                                    size_t pos, size_t len)
{
  struct Vstr_iter dummy_iter;
  struct Vstr_iter *iter = &dummy_iter;
  unsigned int beg_num = 0;

  VSTR__ASSERT_RET(base, VSTR__FALSE);
  
  if (pos == 1 && len == base->len)
    return (base->num);

  if (!vstr_iter_fwd_beg(base, pos, len, iter))
    return (0);

  beg_num = iter->num;
  while (vstr_iter_fwd_nxt(iter))
  { /* do nothing */; }

  return ((iter->num - beg_num) + 1);
}

extern inline int vstr_add_buf(struct Vstr_base *base, size_t pos,
                               const void *buffer, size_t len)
{
  VSTR__ASSERT_RET(!(!base || !buffer || (pos > base->len)), VSTR__FALSE);

  if (!len) return (VSTR__TRUE);

  if (base->len && (pos == base->len) &&
      (base->end->type == VSTR_TYPE_NODE_BUF) &&
      (len <= (base->conf->buf_sz - base->end->len)) &&
      (!base->cache_available || base->cache_internal))
  {
    struct Vstr_node *scan = base->end;

    VSTR__ASSERT(vstr__check_spare_nodes(base->conf));
    VSTR__ASSERT(vstr__check_real_nodes(base));

    vstr_wrap_memcpy(((struct Vstr_node_buf *)scan)->buf + scan->len,
                     buffer, len);
    scan->len += len;
    base->len += len;

    if (base->iovec_upto_date)
    {
      unsigned int num = base->num + VSTR__CACHE(base)->vec->off - 1;
      VSTR__CACHE(base)->vec->v[num].iov_len += len;
    }

    VSTR__ASSERT(vstr__check_spare_nodes(base->conf));
    VSTR__ASSERT(vstr__check_real_nodes(base));

    return (VSTR__TRUE);
  }

  return (vstr_extern_inline_add_buf(base, pos, buffer, len));
}

extern inline int vstr_add_rep_chr(struct Vstr_base *base, size_t pos,
                                   char chr, size_t len)
{ /* almost embarassingly similar to add_buf */
  VSTR__ASSERT_RET(!(!base || (pos > base->len)), VSTR__FALSE);

  if (!len) return (VSTR__TRUE);

  if (base->len && (pos == base->len) &&
      (base->end->type == VSTR_TYPE_NODE_BUF) &&
      (len <= (base->conf->buf_sz - base->end->len)) &&
      (!base->cache_available || base->cache_internal))
  {
    struct Vstr_node *scan = base->end;

    VSTR__ASSERT(vstr__check_spare_nodes(base->conf));
    VSTR__ASSERT(vstr__check_real_nodes(base));

    vstr_wrap_memset(((struct Vstr_node_buf *)scan)->buf + scan->len, chr, len);
    scan->len += len;
    base->len += len;

    if (base->iovec_upto_date)
    {
      unsigned int num = base->num + VSTR__CACHE(base)->vec->off - 1;
      VSTR__CACHE(base)->vec->v[num].iov_len += len;
    }

    VSTR__ASSERT(vstr__check_spare_nodes(base->conf));
    VSTR__ASSERT(vstr__check_real_nodes(base));

    return (VSTR__TRUE);
  }

  return (vstr_extern_inline_add_rep_chr(base, pos, chr, len));
}

extern inline int vstr_del(struct Vstr_base *base, size_t pos, size_t len)
{
  VSTR__ASSERT_RET(!(!base || ((pos > base->len) && len)), VSTR__FALSE);

  if (!len) return (VSTR__TRUE);

  if (!base->cache_available || base->cache_internal)
  {
    size_t end_len = 0;

    if ((pos == 1) && ((len + base->used) < base->beg->len))
    { /* delete from beginning, in one node */
      struct Vstr_node *scan = base->beg;
      struct Vstr__cache_data_cstr *cdata = NULL;
      struct Vstr__cache_data_pos  *pdata = NULL;

      VSTR__ASSERT(vstr__check_spare_nodes(base->conf));
      VSTR__ASSERT(vstr__check_real_nodes(base));

      base->len -= len;

      switch (scan->type)
      {
        case VSTR_TYPE_NODE_BUF:
          base->used += len;
          break;
        case VSTR_TYPE_NODE_NON:
          scan->len -= len;
          break;
        case VSTR_TYPE_NODE_PTR:
        {
          char *tmp = ((struct Vstr_node_ptr *)scan)->ptr;
          ((struct Vstr_node_ptr *)scan)->ptr = tmp + len;
          scan->len -= len;
        }
        break;
        case VSTR_TYPE_NODE_REF:
          ((struct Vstr_node_ref *)scan)->off += len;
          scan->len -= len;
          break;
      }

      if ((cdata = vstr_cache_get(base, 3)) && cdata->ref && cdata->len)
      {
        size_t data_end_pos = vstr_sc_poslast(cdata->pos, cdata->len);
        size_t end_pos = vstr_sc_poslast(1, len);

        if (cdata->pos > end_pos)
          cdata->pos -= len;
        else if (data_end_pos <= end_pos)
          cdata->len = 0;
        else
        {
          cdata->len -= vstr_sc_posdiff(cdata->pos, end_pos);
          cdata->off += vstr_sc_posdiff(cdata->pos, end_pos);
          
          cdata->pos = pos;
        }
      }
      if (base->iovec_upto_date)
      {
        unsigned int num = VSTR__CACHE(base)->vec->off;

        if (scan->type != VSTR_TYPE_NODE_NON)
        {
          char *tmp = VSTR__CACHE(base)->vec->v[num].iov_base;
          VSTR__CACHE(base)->vec->v[num].iov_base = tmp + len;
        }
        VSTR__CACHE(base)->vec->v[num].iov_len -= len;
      }
      if ((pdata = vstr_cache_get(base, 1)))
        pdata->node = NULL;

      VSTR__ASSERT(vstr__check_spare_nodes(base->conf));
      VSTR__ASSERT(vstr__check_real_nodes(base));

      return (VSTR__TRUE);
    }

    end_len = base->end->len;
    if (base->beg == base->end)
    {
      VSTR__ASSERT(base->num == 1);
      end_len += base->used;
    }

    if ((pos > (base->len - (end_len - 1))) &&
        (len == vstr_sc_posdiff(pos, base->len)))
    { /* delete from end, in one node */
      struct Vstr_node *scan = base->end;
      struct Vstr__cache_data_cstr *cdata = NULL;
      struct Vstr__cache_data_pos  *pdata = NULL;
      
      VSTR__ASSERT(vstr__check_spare_nodes(base->conf));
      VSTR__ASSERT(vstr__check_real_nodes(base));

      base->len -= len;
      scan->len -= len;

      if ((cdata = vstr_cache_get(base, 3)) && cdata->ref && cdata->len)
      {
        size_t data_end_pos = vstr_sc_poslast(cdata->pos, cdata->len);

        if (data_end_pos >= pos)
          cdata->len = 0;
      }
      if (base->iovec_upto_date)
      {
        unsigned int num = base->num + VSTR__CACHE(base)->vec->off - 1;

        VSTR__CACHE(base)->vec->v[num].iov_len -= len;
      }
      if ((pdata = vstr_cache_get(base, 1)))
        pdata->node = NULL;

      VSTR__ASSERT(vstr__check_spare_nodes(base->conf));
      VSTR__ASSERT(vstr__check_real_nodes(base));

      return (VSTR__TRUE);
    }
  }

  return (vstr_extern_inline_del(base, pos, len));
}

extern inline int vstr_sc_reduce(struct Vstr_base *base,
                                 size_t pos, size_t len, size_t reduce)
{
  VSTR__ASSERT_RET(len >= reduce, 0);

  if (!len) return (VSTR__TRUE);

  return (vstr_del(base, pos + (len - reduce), reduce));
}

extern inline int vstr_sects_add(struct Vstr_sects *sects,
                                 size_t pos, size_t len)
{
  if (!sects->sz || (sects->num >= sects->sz))
  {
    if (!sects->can_add_sz)
      return (VSTR__FALSE);

    if (!vstr_extern_inline_sects_add(sects, pos, len))
      return (VSTR__FALSE);
  }

  sects->ptr[sects->num].pos = pos;
  sects->ptr[sects->num].len = len;
  ++sects->num;

  return (VSTR__TRUE);
}

extern inline void vstr_sc_bmap_init_eq_spn_buf(unsigned char bmap[256], 
                                                const void *pbuf, size_t len,
                                                unsigned char val)
{
  const unsigned char *buf = pbuf;
  
  while (len)
    bmap[0xFF & buf[--len]] = val;
}

extern inline void vstr_sc_bmap_init_or_spn_buf(unsigned char bmap[256], 
                                                const void *pbuf, size_t len,
                                                unsigned char val)
{
  const unsigned char *buf = pbuf;
  
  while (len)
    bmap[0xFF & buf[--len]] |= val;
}

/* do inline versions of macro/simple functions */
/* cmp */
extern inline int vstr_cmp_eq(const struct Vstr_base *s1, size_t p1, size_t l1,
                              const struct Vstr_base *s2, size_t p2, size_t l2)
{ return ((l1 == l2) && !vstr_cmp(s1, p1, l1, s2, p2, l1)); }
extern inline int vstr_cmp_buf_eq(const struct Vstr_base *s1,
                                  size_t p1, size_t l1,
                                  const void *buf, size_t buf_len)
{ return ((l1 == buf_len) && !vstr_cmp_buf(s1, p1, l1, buf, buf_len)); }
extern inline int vstr_cmp_cstr(const struct Vstr_base *s1,
                                size_t p1, size_t l1,
                                const char *buf)
{ return (vstr_cmp_buf(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }
extern inline int vstr_cmp_cstr_eq(const struct Vstr_base *s1,
                                   size_t p1, size_t l1,
                                   const char *buf)
{ return (vstr_cmp_buf_eq(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }

/* cmp case */
extern inline int vstr_cmp_case_eq(const struct Vstr_base *s1,
                                   size_t p1, size_t l1,
                                   const struct Vstr_base *s2,
                                   size_t p2, size_t l2)
{ return ((l1 == l2) && !vstr_cmp_case(s1, p1, l1, s2, p2, l1)); }
extern inline int vstr_cmp_case_buf_eq(const struct Vstr_base *s1,
                                       size_t p1, size_t l1,
                                       const char *buf, size_t buf_len)
{ return ((l1 == buf_len) && !vstr_cmp_case_buf(s1, p1, l1, buf, buf_len)); }
extern inline int vstr_cmp_case_cstr(const struct Vstr_base *s1,
                                     size_t p1, size_t l1,
                                     const char *buf)
{ return (vstr_cmp_case_buf(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }
extern inline int vstr_cmp_case_cstr_eq(const struct Vstr_base *s1,
                                        size_t p1, size_t l1, const char *buf)
{ return (vstr_cmp_case_buf_eq(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }

/* cmp vers */
extern inline int vstr_cmp_vers_eq(const struct Vstr_base *s1,
                                   size_t p1, size_t l1,
                                   const struct Vstr_base *s2,
                                   size_t p2, size_t l2)
{ return ((l1 == l2) && !vstr_cmp_vers(s1, p1, l1, s2, p2, l1)); }
extern inline int vstr_cmp_vers_buf_eq(const struct Vstr_base *s1,
                                       size_t p1, size_t l1,
                                       const char *buf, size_t buf_len)
{ return ((l1 == buf_len) && !vstr_cmp_vers_buf(s1, p1, l1, buf, buf_len)); }
extern inline int vstr_cmp_vers_cstr(const struct Vstr_base *s1,
                                     size_t p1, size_t l1, const char *buf)
{ return (vstr_cmp_vers_buf(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }
extern inline int vstr_cmp_vers_cstr_eq(const struct Vstr_base *s1,
                                        size_t p1, size_t l1, const char *buf)
{ return (vstr_cmp_vers_buf_eq(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }

/* cmp fast */
extern inline int vstr_cmp_fast(const struct Vstr_base *s1,
                                size_t p1, size_t l1,
                                const struct Vstr_base *s2,
                                size_t p2, size_t l2)
{ if (l1 != l2) return (l2 - l1);
  return (-vstr_cmp(s1, p1, l1, s2, p2, l1)); }
extern inline int vstr_cmp_fast_buf(const struct Vstr_base *s1,
                                       size_t p1, size_t l1,
                                       const char *buf, size_t buf_len)
{ if (l1 != buf_len) return (buf_len - l1);
  return (-vstr_cmp_buf(s1, p1, l1, buf, l1)); }
extern inline int vstr_cmp_fast_cstr(const struct Vstr_base *s1,
                                     size_t p1, size_t l1, const char *buf)
{ return (vstr_cmp_fast_buf(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }


/* add */
extern inline int vstr_add_cstr_buf(struct Vstr_base *s1, size_t pa1,
                                    const char *buf)
{ return (vstr_add_buf(s1, pa1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }
extern inline int vstr_add_cstr_ptr(struct Vstr_base *s1, size_t pa1,
                                    const char *ptr)
{ return (vstr_add_ptr(s1, pa1, ptr, VSTR__AT_COMPILE_STRLEN(ptr))); }
extern inline int vstr_add_cstr_ref(struct Vstr_base *s1, size_t pa1,
                                    struct Vstr_ref *ref, size_t off)
{ return (vstr_add_ref(s1, pa1, ref, off,
                       VSTR__AT_COMPILE_STRLEN(((const char *)ref->ptr) + off))); }

/* dup */
extern inline struct Vstr_base *vstr_dup_cstr_buf(struct Vstr_conf *conf,
                                                  const char *buf)
{ return (vstr_dup_buf(conf, buf, VSTR__AT_COMPILE_STRLEN(buf))); }
extern inline struct Vstr_base *vstr_dup_cstr_ptr(struct Vstr_conf *conf,
                                                  const char *ptr)
{ return (vstr_dup_ptr(conf, ptr, VSTR__AT_COMPILE_STRLEN(ptr))); }
extern inline struct Vstr_base *vstr_dup_cstr_ref(struct Vstr_conf *conf,
                                                  struct Vstr_ref *ref,
                                                  size_t off)
{ return (vstr_dup_ref(conf, ref, off,
                       VSTR__AT_COMPILE_STRLEN(((const char *)ref->ptr) + off))); }

/* sub */
extern inline int vstr_sub_cstr_buf(struct Vstr_base *s1, size_t p1, size_t l1,
                                    const char *buf)
{ return (vstr_sub_buf(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }
extern inline int vstr_sub_cstr_ptr(struct Vstr_base *s1, size_t p1, size_t l1,
                                    const char *ptr)
{ return (vstr_sub_ptr(s1, p1, l1, ptr, VSTR__AT_COMPILE_STRLEN(ptr))); }
extern inline int vstr_sub_cstr_ref(struct Vstr_base *s1, size_t p1, size_t l1,
                                    struct Vstr_ref *ref, size_t off)
{ return (vstr_sub_ref(s1, p1, l1, ref, off,
                       VSTR__AT_COMPILE_STRLEN(((const char *)ref->ptr) + off))); }

/* srch */
extern inline size_t vstr_srch_cstr_buf_fwd(const struct Vstr_base *s1,
                                            size_t p1, size_t l1,
                                            const char *buf)
{ return (vstr_srch_buf_fwd(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }
extern inline size_t vstr_srch_cstr_buf_rev(const struct Vstr_base *s1,
                                            size_t p1, size_t l1,
                                            const char *buf)
{ return (vstr_srch_buf_rev(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }

extern inline size_t vstr_srch_cstr_chrs_fwd(const struct Vstr_base *s1,
                                             size_t p1, size_t l1,
                                             const char *chrs)
{ return (vstr_srch_chrs_fwd(s1, p1, l1, chrs, VSTR__AT_COMPILE_STRLEN(chrs))); }
extern inline size_t vstr_srch_cstr_chrs_rev(const struct Vstr_base *s1,
                                             size_t p1, size_t l1,
                                             const char *chrs)
{ return (vstr_srch_chrs_rev(s1, p1, l1, chrs, VSTR__AT_COMPILE_STRLEN(chrs))); }

extern inline size_t vstr_csrch_cstr_chrs_fwd(const struct Vstr_base *s1,
                                              size_t p1, size_t l1,
                                              const char *chrs)
{ return (vstr_csrch_chrs_fwd(s1, p1, l1, chrs, VSTR__AT_COMPILE_STRLEN(chrs))); }
extern inline size_t vstr_csrch_cstr_chrs_rev(const struct Vstr_base *s1,
                                              size_t p1, size_t l1,
                                              const char *chrs)
{ return (vstr_csrch_chrs_rev(s1, p1, l1, chrs, VSTR__AT_COMPILE_STRLEN(chrs))); }

extern inline size_t vstr_srch_case_cstr_buf_fwd(const struct Vstr_base *s1,
                                                 size_t p1, size_t l1,
                                                 const char *buf)
{ return (vstr_srch_case_buf_fwd(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }
extern inline size_t vstr_srch_case_cstr_buf_rev(const struct Vstr_base *s1,
                                                 size_t p1, size_t l1,
                                                 const char *buf)
{ return (vstr_srch_case_buf_rev(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }

/* spn */
extern inline size_t vstr_spn_cstr_chrs_fwd(const struct Vstr_base *s1,
                                            size_t p1, size_t l1,
                                            const char *chrs)
{ return (vstr_spn_chrs_fwd(s1, p1, l1, chrs, VSTR__AT_COMPILE_STRLEN(chrs))); }
extern inline size_t vstr_spn_cstr_chrs_rev(const struct Vstr_base *s1,
                                            size_t p1, size_t l1,
                                            const char *chrs)
{ return (vstr_spn_chrs_rev(s1, p1, l1, chrs, VSTR__AT_COMPILE_STRLEN(chrs))); }

extern inline size_t vstr_cspn_cstr_chrs_fwd(const struct Vstr_base *s1,
                                             size_t p1, size_t l1,
                                             const char *chrs)
{ return (vstr_cspn_chrs_fwd(s1, p1, l1, chrs, VSTR__AT_COMPILE_STRLEN(chrs))); }
extern inline size_t vstr_cspn_cstr_chrs_rev(const struct Vstr_base *s1,
                                             size_t p1, size_t l1,
                                             const char *chrs)
{ return (vstr_cspn_chrs_rev(s1, p1, l1, chrs, VSTR__AT_COMPILE_STRLEN(chrs))); }

/* split */
extern inline size_t vstr_split_cstr_buf(const struct Vstr_base *s1,
                                         size_t p1, size_t l1,
                                         const char *buf,
                                         struct Vstr_sects *sect,
                                         unsigned int lim, unsigned int flags)
{  return (vstr_split_buf(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf), sect, lim, flags)); }
extern inline size_t vstr_split_cstr_chrs(const struct Vstr_base *s1,
                                          size_t p1, size_t l1,
                                          const char *chrs,
                                          struct Vstr_sects *sect,
                                          unsigned int lim, unsigned int flags)
{  return (vstr_split_chrs(s1, p1, l1, chrs, VSTR__AT_COMPILE_STRLEN(chrs), sect, lim, flags)); }


/* simple inlines that can't easily be macros... */
/* cmp */
extern inline int vstr_cmp_bod(const struct Vstr_base *s1, size_t p1, size_t l1,
                               const struct Vstr_base *s2, size_t p2, size_t l2)
{
  size_t tmp = l1; if (tmp > l2) tmp = l2;
  return (vstr_cmp(s1, p1, tmp, s2, p2, tmp));
}
extern inline int vstr_cmp_eod(const struct Vstr_base *s1, size_t p1, size_t l1,
                               const struct Vstr_base *s2, size_t p2, size_t l2)
{
  size_t tmp = l1; if (tmp > l2) tmp = l2;
  p1 += (l1 - tmp);
  p2 += (l2 - tmp);
  return (vstr_cmp(s1, p1, tmp, s2, p2, tmp));
}
extern inline int vstr_cmp_bod_eq(const struct Vstr_base *s1,
                                  size_t p1, size_t l1,
                                  const struct Vstr_base *s2,
                                  size_t p2, size_t l2)
{ return (!vstr_cmp_bod(s1, p1, l1, s2, p2, l2)); }
extern inline int vstr_cmp_eod_eq(const struct Vstr_base *s1,
                                  size_t p1, size_t l1,
                                  const struct Vstr_base *s2,
                                  size_t p2, size_t l2)
{ return (!vstr_cmp_eod(s1, p1, l1, s2, p2, l2)); }

/* cmp buf */
extern inline int vstr_cmp_bod_buf(const struct Vstr_base *s1,
                                   size_t p1, size_t l1,
                                   const void *buf, size_t buf_len)
{
  size_t tmp = l1; if (tmp > buf_len) tmp = buf_len;
  return (vstr_cmp_buf(s1, p1, tmp, buf, tmp));
}
extern inline int vstr_cmp_eod_buf(const struct Vstr_base *s1,
                                   size_t p1, size_t l1,
                                   const void *pbuf, size_t buf_len)
{
  const char *buf = (const char *)pbuf;
  size_t tmp = l1; if (tmp > buf_len) tmp = buf_len;
  p1  += (l1 - tmp);
  buf += (buf_len - tmp);
  return (vstr_cmp_buf(s1, p1, tmp, buf, tmp));
}
extern inline int vstr_cmp_bod_buf_eq(const struct Vstr_base *s1,
                                      size_t p1, size_t l1,
                                      const void *pbuf, size_t buf_len)
{ return (!vstr_cmp_bod_buf(s1, p1, l1, pbuf, buf_len)); }
extern inline int vstr_cmp_eod_buf_eq(const struct Vstr_base *s1,
                                      size_t p1, size_t l1,
                                      const void *pbuf, size_t buf_len)
{ return (!vstr_cmp_eod_buf(s1, p1, l1, pbuf, buf_len)); }

/* cmp cstr */
extern inline int vstr_cmp_bod_cstr(const struct Vstr_base *s1,
                                    size_t p1, size_t l1,
                                    const char *buf)
{ return (vstr_cmp_bod_buf(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }
extern inline int vstr_cmp_eod_cstr(const struct Vstr_base *s1,
                                    size_t p1, size_t l1,
                                    const char *buf)
{ return (vstr_cmp_eod_buf(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }
extern inline int vstr_cmp_bod_cstr_eq(const struct Vstr_base *s1,
                                       size_t p1, size_t l1,
                                       const char *buf)
{ return (vstr_cmp_bod_buf_eq(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }
extern inline int vstr_cmp_eod_cstr_eq(const struct Vstr_base *s1,
                                       size_t p1, size_t l1,
                                       const char *buf)
{ return (vstr_cmp_eod_buf_eq(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }

/* cmp case */
extern inline int vstr_cmp_case_bod(const struct Vstr_base *s1,
                                    size_t p1, size_t l1,
                                    const struct Vstr_base *s2,
                                    size_t p2, size_t l2)
{
  size_t tmp = l1; if (tmp > l2) tmp = l2;
  return (vstr_cmp_case(s1, p1, tmp, s2, p2, tmp));
}
extern inline int vstr_cmp_case_eod(const struct Vstr_base *s1,
                                    size_t p1, size_t l1,
                                    const struct Vstr_base *s2,
                                    size_t p2, size_t l2)
{
  size_t tmp = l1; if (tmp > l2) tmp = l2;
  p1 += (l1 - tmp);
  p2 += (l2 - tmp);
  return (vstr_cmp_case(s1, p1, tmp, s2, p2, tmp));
}
extern inline int vstr_cmp_case_bod_eq(const struct Vstr_base *s1,
                                  size_t p1, size_t l1,
                                  const struct Vstr_base *s2,
                                  size_t p2, size_t l2)
{ return (!vstr_cmp_case_bod(s1, p1, l1, s2, p2, l2)); }
extern inline int vstr_cmp_case_eod_eq(const struct Vstr_base *s1,
                                  size_t p1, size_t l1,
                                  const struct Vstr_base *s2,
                                  size_t p2, size_t l2)
{ return (!vstr_cmp_case_eod(s1, p1, l1, s2, p2, l2)); }

/* cmp case buf */
extern inline int vstr_cmp_case_bod_buf(const struct Vstr_base *s1,
                                        size_t p1, size_t l1,
                                        const char *buf, size_t buf_len)
{
  size_t tmp = l1; if (tmp > buf_len) tmp = buf_len;
  return (vstr_cmp_case_buf(s1, p1, tmp, buf, tmp));
}
extern inline int vstr_cmp_case_eod_buf(const struct Vstr_base *s1,
                                        size_t p1, size_t l1,
                                        const char *buf, size_t buf_len)
{
  size_t tmp = l1; if (tmp > buf_len) tmp = buf_len;
  p1  += (l1 - tmp);
  buf += (buf_len - tmp);
  return (vstr_cmp_case_buf(s1, p1, tmp, buf, tmp));
}
extern inline int vstr_cmp_case_bod_buf_eq(const struct Vstr_base *s1,
                                           size_t p1, size_t l1,
                                           const char *buf, size_t buf_len)
{ return (!vstr_cmp_case_bod_buf(s1, p1, l1, buf, buf_len)); }
extern inline int vstr_cmp_case_eod_buf_eq(const struct Vstr_base *s1,
                                           size_t p1, size_t l1,
                                           const char *buf, size_t buf_len)
{ return (!vstr_cmp_case_eod_buf(s1, p1, l1, buf, buf_len)); }

/* cmp case cstr */
extern inline int vstr_cmp_case_bod_cstr(const struct Vstr_base *s1,
                                         size_t p1, size_t l1,
                                         const char *buf)
{ return (vstr_cmp_case_bod_buf(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }
extern inline int vstr_cmp_case_eod_cstr(const struct Vstr_base *s1,
                                         size_t p1, size_t l1,
                                         const char *buf)
{ return (vstr_cmp_case_eod_buf(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }
extern inline int vstr_cmp_case_bod_cstr_eq(const struct Vstr_base *s1,
                                            size_t p1, size_t l1,
                                            const char *buf)
{ return (vstr_cmp_case_bod_buf_eq(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }
extern inline int vstr_cmp_case_eod_cstr_eq(const struct Vstr_base *s1,
                                            size_t p1, size_t l1,
                                            const char *buf)
{ return (vstr_cmp_case_eod_buf_eq(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }

/* cmp vers */
extern inline int vstr_cmp_vers_bod(const struct Vstr_base *s1,
                                    size_t p1, size_t l1,
                                    const struct Vstr_base *s2,
                                    size_t p2, size_t l2)
{
  size_t tmp = l1; if (tmp > l2) tmp = l2;
  return (vstr_cmp_vers(s1, p1, tmp, s2, p2, tmp));
}
extern inline int vstr_cmp_vers_eod(const struct Vstr_base *s1,
                                    size_t p1, size_t l1,
                                    const struct Vstr_base *s2,
                                    size_t p2, size_t l2)
{
  size_t tmp = l1; if (tmp > l2) tmp = l2;
  p1 += (l1 - tmp);
  p2 += (l2 - tmp);
  return (vstr_cmp_vers(s1, p1, tmp, s2, p2, tmp));
}
extern inline int vstr_cmp_vers_bod_eq(const struct Vstr_base *s1,
                                  size_t p1, size_t l1,
                                  const struct Vstr_base *s2,
                                  size_t p2, size_t l2)
{ return (!vstr_cmp_vers_bod(s1, p1, l1, s2, p2, l2)); }
extern inline int vstr_cmp_vers_eod_eq(const struct Vstr_base *s1,
                                  size_t p1, size_t l1,
                                  const struct Vstr_base *s2,
                                  size_t p2, size_t l2)
{ return (!vstr_cmp_vers_eod(s1, p1, l1, s2, p2, l2)); }

/* cmp vers buf */
extern inline int vstr_cmp_vers_bod_buf(const struct Vstr_base *s1,
                                        size_t p1, size_t l1,
                                        const char *buf, size_t buf_len)
{
  size_t tmp = l1; if (tmp > buf_len) tmp = buf_len;
  return (vstr_cmp_vers_buf(s1, p1, tmp, buf, tmp));
}
extern inline int vstr_cmp_vers_eod_buf(const struct Vstr_base *s1,
                                        size_t p1, size_t l1,
                                        const char *buf, size_t buf_len)
{
  size_t tmp = l1; if (tmp > buf_len) tmp = buf_len;
  p1  += (l1 - tmp);
  buf += (buf_len - tmp);
  return (vstr_cmp_vers_buf(s1, p1, tmp, buf, tmp));
}
extern inline int vstr_cmp_vers_bod_buf_eq(const struct Vstr_base *s1,
                                           size_t p1, size_t l1,
                                           const char *buf, size_t buf_len)
{ return (!vstr_cmp_vers_bod_buf(s1, p1, l1, buf, buf_len)); }
extern inline int vstr_cmp_vers_eod_buf_eq(const struct Vstr_base *s1,
                                           size_t p1, size_t l1,
                                           const char *buf, size_t buf_len)
{ return (!vstr_cmp_vers_eod_buf(s1, p1, l1, buf, buf_len)); }

/* cmp vers cstr */
extern inline int vstr_cmp_vers_bod_cstr(const struct Vstr_base *s1,
                                         size_t p1, size_t l1,
                                         const char *buf)
{ return (vstr_cmp_vers_bod_buf(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }
extern inline int vstr_cmp_vers_eod_cstr(const struct Vstr_base *s1,
                                         size_t p1, size_t l1,
                                         const char *buf)
{ return (vstr_cmp_vers_eod_buf(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }
extern inline int vstr_cmp_vers_bod_cstr_eq(const struct Vstr_base *s1,
                                            size_t p1, size_t l1,
                                            const char *buf)
{ return (vstr_cmp_vers_bod_buf_eq(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }
extern inline int vstr_cmp_vers_eod_cstr_eq(const struct Vstr_base *s1,
                                            size_t p1, size_t l1,
                                            const char *buf)
{ return (vstr_cmp_vers_eod_buf_eq(s1, p1, l1, buf, VSTR__AT_COMPILE_STRLEN(buf))); }

/* ref */
extern inline struct Vstr_ref *vstr_ref_make_strdup(const char *val)
{ return (vstr_ref_make_memdup(val, VSTR__AT_COMPILE_STRLEN(val) + 1)); }

/* sc */
extern inline int vstr_sc_add_cstr_grpbasenum_buf(struct Vstr_base *s1,
                                                  size_t p1, unsigned int nb,
                                                  const char *val)
{ return (vstr_sc_add_grpbasenum_buf(s1, p1, nb, val, VSTR__AT_COMPILE_STRLEN(val))); }
extern inline int vstr_sc_add_cstr_grpbasenum_ptr(struct Vstr_base *s1,
                                                  size_t p1, unsigned int nb,
                                                  const char *val)
{ return (vstr_sc_add_grpbasenum_ptr(s1, p1, nb, val, VSTR__AT_COMPILE_STRLEN(val))); }
extern inline int vstr_sc_add_cstr_grpbasenum_ref(struct Vstr_base *s1,
                                                  size_t p1, unsigned int nb,
                                                  struct Vstr_ref *ref,
                                                  size_t off)
{ return (vstr_sc_add_grpbasenum_ref(s1, p1, nb, ref, off,
                                     VSTR__AT_COMPILE_STRLEN(((const char *)ref->ptr) + off))); }
extern inline int vstr_sc_add_grpnum_buf(struct Vstr_base *s1, size_t p1,
                                         const void *val, size_t len)
{ return (vstr_sc_add_grpbasenum_buf(s1, p1, 0, val, len)); }
extern inline int vstr_sc_add_cstr_grpnum_buf(struct Vstr_base *s1, size_t p1,
                                              const char *val)
{ return (vstr_sc_add_grpnum_buf(s1, p1, val, VSTR__AT_COMPILE_STRLEN(val))); }
extern inline void vstr_sc_bmap_init_eq_spn_cstr(unsigned char bmap[256], 
                                                 const char *buf,
                                                 unsigned char val)
{ vstr_sc_bmap_init_eq_spn_buf(bmap, buf, VSTR__AT_COMPILE_STRLEN(buf), val); }
extern inline void vstr_sc_bmap_init_or_spn_cstr(unsigned char bmap[256], 
                                                 const char *buf,
                                                 unsigned char val)
{ vstr_sc_bmap_init_or_spn_buf(bmap, buf, VSTR__AT_COMPILE_STRLEN(buf), val); }

/* binary */
extern inline int vstr_sc_add_b_uint16(struct Vstr_base *s1, size_t p1,
                                       VSTR_AUTOCONF_uint_least16_t data)
{
  unsigned char buf[2];

  buf[1] = data & 0xFF; data >>= 8;
  buf[0] = data & 0xFF;

  return (vstr_add_buf(s1, p1, buf, sizeof(buf)));
}
extern inline int vstr_sc_add_b_uint32(struct Vstr_base *s1, size_t p1,
                                       VSTR_AUTOCONF_uint_least32_t data)
{
  unsigned char buf[4];

  buf[3] = data & 0xFF; data >>= 8;
  buf[2] = data & 0xFF; data >>= 8;
  buf[1] = data & 0xFF; data >>= 8;
  buf[0] = data & 0xFF;

  return (vstr_add_buf(s1, p1, buf, sizeof(buf)));
}

extern inline int vstr_sc_sub_b_uint16(struct Vstr_base *s1,
                                       size_t p1, size_t l1,
                                       VSTR_AUTOCONF_uint_least16_t data)
{
  unsigned char buf[2];

  buf[1] = data & 0xFF; data >>= 8;
  buf[0] = data & 0xFF;

  return (vstr_sub_buf(s1, p1, l1, buf, sizeof(buf)));
}
extern inline int vstr_sc_sub_b_uint32(struct Vstr_base *s1,
                                       size_t p1, size_t l1,
                                       VSTR_AUTOCONF_uint_least32_t data)
{
  unsigned char buf[4];

  buf[3] = data & 0xFF; data >>= 8;
  buf[2] = data & 0xFF; data >>= 8;
  buf[1] = data & 0xFF; data >>= 8;
  buf[0] = data & 0xFF;

  return (vstr_sub_buf(s1, p1, l1, buf, sizeof(buf)));
}

extern inline VSTR_AUTOCONF_uint_least16_t
vstr_sc_parse_b_uint16(struct Vstr_base *s1, size_t p1)
{
  unsigned char buf[2];
  VSTR_AUTOCONF_uint_least16_t num = 0;
  
  if (!vstr_export_buf(s1, p1, sizeof(buf), buf, sizeof(buf)))
    return (0);

  num += buf[0]; num <<= 8;
  num += buf[1];
  
  return (num);
}
extern inline VSTR_AUTOCONF_uint_least32_t
vstr_sc_parse_b_uint32(struct Vstr_base *s1, size_t p1)
{
  unsigned char buf[4];
  VSTR_AUTOCONF_uint_least32_t num = 0;

  if (!vstr_export_buf(s1, p1, sizeof(buf), buf, sizeof(buf)))
    return (0);

  num += buf[0]; num <<= 8;
  num += buf[1]; num <<= 8;
  num += buf[2]; num <<= 8;
  num += buf[3];
  
  return (num);
}

#undef VSTR__ASSERT
#undef VSTR__ASSERT_RET
#undef VSTR__ASSERT_NO_SWITCH_DEF
#undef VSTR__TRUE
#undef VSTR__FALSE
