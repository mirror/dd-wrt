#define VSTR_CMP_C
/*
 *  Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004  James Antill
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
/* functions for comparing vstrs */
#include "main.h"

#define VSTR__CMP_DOUBLE_ITER_BEG(b1, p1, l1, i1, b2, p2, l2, i2) do { \
  int r1 = vstr_iter_fwd_beg(b1, p1, l1, i1); \
  int r2 = vstr_iter_fwd_beg(b2, p2, l2, i2); \
  if (!r1 && !r2) \
    return (0); \
  else if (!r1) \
    return (-1); \
  else if (!r2) \
    return (1); \
  \
  ASSERT((i1)->node && (i2)->node); \
  \
 } while (FALSE)


/* compare 2 vector strings */
int vstr_cmp(const Vstr_base *base_1, size_t pos_1, size_t len_1,
             const Vstr_base *base_2, size_t pos_2, size_t len_2)
{
  Vstr_iter iter1[1];
  Vstr_iter iter2[1];

  VSTR__CMP_DOUBLE_ITER_BEG(base_1, pos_1, len_1, iter1,
                            base_2, pos_2, len_2, iter2);

  do
  {
    size_t tmp = iter1->len;
    if (tmp > iter2->len)
      tmp = iter2->len;

    assert(iter1->node && iter2->node);

    if ((iter1->node->type != VSTR_TYPE_NODE_NON) &&
        (iter2->node->type != VSTR_TYPE_NODE_NON))
    {
      int ret = vstr_wrap_memcmp(iter1->ptr, iter2->ptr, tmp);
      if (ret)
        return (ret);
      iter1->ptr += tmp;
      iter2->ptr += tmp;
    }
    else if (iter1->node->type != VSTR_TYPE_NODE_NON)
      return (1);
    else if (iter2->node->type != VSTR_TYPE_NODE_NON)
      return (-1);

    iter1->len -= tmp;
    iter2->len -= tmp;

    assert(!iter1->len || !iter2->len);

  } while ((iter1->len || vstr_iter_fwd_nxt(iter1)) &&
           (iter2->len || vstr_iter_fwd_nxt(iter2)) &&
           TRUE);

  if (iter1->node)
    return (1);
  if (vstr_iter_len(iter2))
    return (-1);

  return (0);
}

/* compare with a "normal" C string */
int vstr_cmp_buf(const Vstr_base *base, size_t pos, size_t len,
                 const void *buf, size_t buf_len)
{
  Vstr_iter iter[1];

  if (!vstr_iter_fwd_beg(base, pos, len, iter) && !buf_len)
    return (0);
  if (!iter->node)
    return (-1);
  if (!buf_len)
    return (1);

  do
  {
    int ret = 0;

    if (iter->len > buf_len)
    {
      iter->len = buf_len;
      /* make sure we know iter is bigger than buf */
      iter->remaining += 1;
    }

    if ((iter->node->type == VSTR_TYPE_NODE_NON) &&  buf)
      return (-1);
    if ((iter->node->type != VSTR_TYPE_NODE_NON) && !buf)
      return (1);

    if (buf)
    {
      if ((ret = vstr_wrap_memcmp(iter->ptr, buf, iter->len)))
        return (ret);
      buf = ((char *)buf) + iter->len;
    }

    buf_len -= iter->len;
  } while (buf_len && vstr_iter_fwd_nxt(iter));

  if (iter->remaining)
    return (1);
  if (buf_len)
    return (-1);

  return (0);
}

/* only do ASCII/binary case comparisons -- regardless of the OS/compiler
 * char set default */
static int vstr__cmp_memcasecmp(const char *str1, const char *str2, size_t len)
{
  while (len)
  {
    unsigned char a = *str1;
    unsigned char b = *str2;

    if (VSTR__IS_ASCII_UPPER(a))
      a = VSTR__TO_ASCII_LOWER(a);
    if (VSTR__IS_ASCII_UPPER(b))
      b = VSTR__TO_ASCII_LOWER(b);

    if (a - b)
      return (a - b);

    ++str1;
    ++str2;
    --len;
  }

  return (0);
}

/* don't include ASCII case when comparing */
int vstr_cmp_case(const Vstr_base *base_1, size_t pos_1, size_t len_1,
                  const Vstr_base *base_2, size_t pos_2, size_t len_2)
{
  Vstr_iter iter1[1];
  Vstr_iter iter2[1];

  VSTR__CMP_DOUBLE_ITER_BEG(base_1, pos_1, len_1, iter1,
                            base_2, pos_2, len_2, iter2);

  do
  {
    size_t tmp = iter1->len;
    if (tmp > iter2->len)
      tmp = iter2->len;

    assert(iter1->node && iter2->node);

    if ((iter1->node->type != VSTR_TYPE_NODE_NON) &&
        (iter2->node->type != VSTR_TYPE_NODE_NON))
    {
      int ret = vstr__cmp_memcasecmp(iter1->ptr, iter2->ptr, tmp);
      if (ret)
        return (ret);
      iter1->ptr += tmp;
      iter2->ptr += tmp;
    }
    else if (iter1->node->type != VSTR_TYPE_NODE_NON)
      return (1);
    else if (iter2->node->type != VSTR_TYPE_NODE_NON)
      return (-1);

    iter1->len -= tmp;
    iter2->len -= tmp;

    assert(!iter1->len || !iter2->len);

  } while ((iter1->len || vstr_iter_fwd_nxt(iter1)) &&
           (iter2->len || vstr_iter_fwd_nxt(iter2)) &&
           TRUE);

  if (iter1->node)
    return (1);
  if (vstr_iter_len(iter2))
    return (-1);

  return (0);
}

int vstr_cmp_case_buf(const Vstr_base *base, size_t pos, size_t len,
                      const char *buf, size_t buf_len)
{
  Vstr_iter iter[1];

  if (!vstr_iter_fwd_beg(base, pos, len, iter) && !buf_len)
    return (0);
  if (!iter->node)
    return (-1);
  if (!buf_len)
    return (1);

  do
  {
    int ret = 0;

    if (iter->len > buf_len)
    {
      iter->len = buf_len;
      /* make sure we know iter is bigger than buf */
      iter->remaining += 1;
    }

    if ((iter->node->type == VSTR_TYPE_NODE_NON) &&  buf)
      return (-1);
    if ((iter->node->type != VSTR_TYPE_NODE_NON) && !buf)
      return (1);

    if (buf)
    {
      if ((ret = vstr__cmp_memcasecmp(iter->ptr, buf, iter->len)))
        return (ret);
      buf += iter->len;
    }

    buf_len -= iter->len;
  } while (buf_len && vstr_iter_fwd_nxt(iter));

  if (iter->remaining)
    return (1);
  if (buf_len)
    return (-1);

  return (0);
}

#define VSTR__CMP_BAD (-1)

#define VSTR__CMP_NORM 0
#define VSTR__CMP_NUMB 1
#define VSTR__CMP_FRAC 2

#define VSTR__CMP_NON_MAIN_LOOP 3

#define VSTR__CMP_LEN_POS 4 /* return positive if scan_1 length is longer */
#define VSTR__CMP_LEN_NEG 8 /* return negative if scan_1 length is longer */
#define VSTR__CMP_DONE 9

static int vstr__cmp_vers(const char *scan_str_1,
                          const char *scan_str_2, size_t len,
                          int state, int *difference)
{
  int diff = 0;

  while ((state < VSTR__CMP_NON_MAIN_LOOP) &&
         len && !(diff = *scan_str_1 - *scan_str_2))
  {
    switch (state)
    {
      case VSTR__CMP_NORM:
        if (VSTR__IS_ASCII_DIGIT(*scan_str_1))
          state = VSTR__CMP_NUMB;
        if (*scan_str_1 == VSTR__ASCII_DIGIT_0())
        {
          assert(state == VSTR__CMP_NUMB);
          ++state;
          assert(state == VSTR__CMP_FRAC);
        }
        break;
      case VSTR__CMP_FRAC:
        if (VSTR__IS_ASCII_DIGIT(*scan_str_1) &&
            (*scan_str_1 != VSTR__ASCII_DIGIT_0()))
          state = VSTR__CMP_NUMB;
      case VSTR__CMP_NUMB:
        if (!VSTR__IS_ASCII_DIGIT(*scan_str_1))
          state = VSTR__CMP_NORM;

        ASSERT_NO_SWITCH_DEF();
    }

    ++scan_str_1;
    ++scan_str_2;

    --len;
  }

  if (diff)
  {
    int new_state = VSTR__CMP_BAD;

    assert(len);

    *difference = diff;

    switch (state)
    {
      case VSTR__CMP_NORM:
        if (VSTR__IS_ASCII_DIGIT(*scan_str_1) &&
            (*scan_str_1 != VSTR__ASCII_DIGIT_0()) &&
            VSTR__IS_ASCII_DIGIT(*scan_str_2) &&
            (*scan_str_2 != VSTR__ASCII_DIGIT_0()))
          state = VSTR__CMP_NUMB;
        break;
      case VSTR__CMP_FRAC:
      case VSTR__CMP_NUMB:
        if (!VSTR__IS_ASCII_DIGIT(*scan_str_1) &&
            !VSTR__IS_ASCII_DIGIT(*scan_str_2))
          state = VSTR__CMP_NORM;
        
        ASSERT_NO_SWITCH_DEF();
    }

    if (state == VSTR__CMP_NORM)
      return (VSTR__CMP_DONE);

    assert((state == VSTR__CMP_NUMB) ||
           (state == VSTR__CMP_FRAC));

    /* if a string is longer return positive or negative ignoring difference */
    new_state = state << 2;

    assert(((state == VSTR__CMP_NUMB) && (new_state == VSTR__CMP_LEN_POS)) ||
           ((state == VSTR__CMP_FRAC) && (new_state == VSTR__CMP_LEN_NEG)) ||
           FALSE);

    state = new_state;
  }

  if (state >= VSTR__CMP_NON_MAIN_LOOP)
  {
    assert((state == VSTR__CMP_LEN_POS) ||
           (state == VSTR__CMP_LEN_NEG));

    while (len &&
           VSTR__IS_ASCII_DIGIT(*scan_str_1) &&
           VSTR__IS_ASCII_DIGIT(*scan_str_2))
    {
      ++scan_str_1;
      ++scan_str_2;

      --len;
    }

    if (len)
    {
      assert((VSTR__CMP_LEN_POS + 1) < VSTR__CMP_LEN_NEG);

      if (VSTR__IS_ASCII_DIGIT(*scan_str_1))
        *difference = ((-state) + VSTR__CMP_LEN_POS + 1);
      if (VSTR__IS_ASCII_DIGIT(*scan_str_2))
        *difference = (state - VSTR__CMP_LEN_POS - 1);
      /* if both are the same length then use the initial stored difference */

      return (VSTR__CMP_DONE);
    }
  }

  return (state);
}

/* Compare strings while treating digits characters numerically. *
 * However digits starting with a 0 are clasified as fractional (Ie. 0.x)
 */
int vstr_cmp_vers(const Vstr_base *base_1, size_t pos_1, size_t len_1,
                  const Vstr_base *base_2, size_t pos_2, size_t len_2)
{
  Vstr_iter iter1[1];
  Vstr_iter iter2[1];
  int state = VSTR__CMP_NORM;
  int ret = 0;

  VSTR__CMP_DOUBLE_ITER_BEG(base_1, pos_1, len_1, iter1,
                            base_2, pos_2, len_2, iter2);

  do
  {
    size_t tmp = iter1->len;
    if (tmp > iter2->len)
      tmp = iter2->len;

    assert(iter1->node && iter2->node);

    if ((iter1->node->type != VSTR_TYPE_NODE_NON) &&
        (iter2->node->type != VSTR_TYPE_NODE_NON))
    {
      state = vstr__cmp_vers(iter1->ptr, iter2->ptr, tmp, state, &ret);
      if (state == VSTR__CMP_DONE)
        return (ret);
      iter1->ptr += tmp;
      iter2->ptr += tmp;
    }
    else if (iter1->node->type != VSTR_TYPE_NODE_NON)
      goto scan_1_longer;
    else if (iter2->node->type != VSTR_TYPE_NODE_NON)
      goto scan_2_longer;

    iter1->len -= tmp;
    iter2->len -= tmp;

    assert(!iter1->len || !iter2->len);
  } while ((iter1->len || vstr_iter_fwd_nxt(iter1)) &&
           (iter2->len || vstr_iter_fwd_nxt(iter2)) &&
           TRUE);

  if (iter1->node)
    goto scan_1_longer;
  if (vstr_iter_len(iter2))
    goto scan_2_longer;

  return (ret); /* same length, might have been different at a previous point */

 scan_1_longer:
  if ((state == VSTR__CMP_FRAC) || (state == VSTR__CMP_LEN_NEG))
    return (-1);

  assert((state == VSTR__CMP_NORM) || (state == VSTR__CMP_NUMB) ||
         (state == VSTR__CMP_LEN_POS));
  return (1);

 scan_2_longer:
  if ((state == VSTR__CMP_FRAC) || (state == VSTR__CMP_LEN_NEG))
    return (1);

  assert((state == VSTR__CMP_NORM) || (state == VSTR__CMP_NUMB) ||
         (state == VSTR__CMP_LEN_POS));
  return (-1);
}

int vstr_cmp_vers_buf(const Vstr_base *base, size_t pos, size_t len,
                      const char *buf, size_t buf_len)
{
  Vstr_iter iter[1];
  int state = VSTR__CMP_NORM;
  int ret = 0;

  if (!vstr_iter_fwd_beg(base, pos, len, iter) && !buf_len)
    return (0);
  if (!iter->node)
    return (-1);
  if (!buf_len)
    return (1);

  do
  {
    if (iter->len > buf_len)
    {
      iter->len = buf_len;
      /* make sure we know iter is bigger than buf */
      iter->remaining += 1;
    }

    if ((iter->node->type == VSTR_TYPE_NODE_NON) &&  buf)
      goto scan_2_longer;
    if ((iter->node->type != VSTR_TYPE_NODE_NON) && !buf)
      goto scan_1_longer;

    if (buf)
    {
      state = vstr__cmp_vers(iter->ptr, buf, iter->len, state, &ret);
      if (state == VSTR__CMP_DONE)
        return (ret);
      buf += iter->len;
    }

    buf_len -= iter->len;
  } while (buf_len && vstr_iter_fwd_nxt(iter));

  if (iter->remaining)
    goto scan_1_longer;
  if (buf_len)
    goto scan_2_longer;

  return (ret);

 scan_1_longer:
  if ((state == VSTR__CMP_FRAC) || (state == VSTR__CMP_LEN_NEG))
    return (-1);

  assert((state == VSTR__CMP_NORM) || (state == VSTR__CMP_NUMB) ||
         (state == VSTR__CMP_LEN_POS));
  return (1);

 scan_2_longer:
  if ((state == VSTR__CMP_FRAC) || (state == VSTR__CMP_LEN_NEG))
    return (1);

  assert((state == VSTR__CMP_NORM) || (state == VSTR__CMP_NUMB) ||
         (state == VSTR__CMP_LEN_POS));
  return (-1);
}

#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(cmp);
VSTR__SYM_ALIAS(cmp_buf);
VSTR__SYM_ALIAS(cmp_case);
VSTR__SYM_ALIAS(cmp_case_buf);
VSTR__SYM_ALIAS(cmp_vers);
VSTR__SYM_ALIAS(cmp_vers_buf);
