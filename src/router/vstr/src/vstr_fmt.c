#define VSTR_ADD_FMT_C
/*
 *  Copyright (C) 2002, 2003, 2004, 2006  James Antill
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
/* Registration/deregistration of custom format specifiers */
#include "main.h"


static Vstr__fmt_usr_name_node **vstr__fmt_beg(Vstr_conf *conf, char pval)
{
  unsigned char val = pval;
  
  ASSERT(conf->fmt_usr_curly_braces);
  
  if (VSTR__IS_ASCII_DIGIT(val))
    return &conf->fmt_usr_name_hash[val - VSTR__ASCII_DIGIT_0()];

  if (!VSTR__IS_ASCII_ALPHA(val))
    return &conf->fmt_usr_name_hash[36];

  if (VSTR__IS_ASCII_UPPER(val))
    val = VSTR__TO_ASCII_LOWER(val);
  
  return &conf->fmt_usr_name_hash[10 + val - VSTR__ASCII_DIGIT_a()];
}

#define VSTR__FMT_ADD_Q(name, len, b1, b2)                      \
    (((name)[0] == (b1)) &&                                     \
     ((name)[(len) - 1] == (b2)) &&                             \
     (((len) == 2) || (((len) > 2) &&                           \
                       !memchr((name) + 1, (b1), (len) - 2) &&  \
                       !memchr((name) + 1, (b2), (len) - 2))))

static
Vstr__fmt_usr_name_node **
vstr__fmt_usr_srch(Vstr_conf *conf, const char *name)
{
  Vstr__fmt_usr_name_node **scan = &conf->fmt_usr_names;
  size_t len = strlen(name);

  if (conf->fmt_usr_curly_braces)
  {
    ASSERT(!*scan);
    
    if (!VSTR__FMT_ADD_Q(name, len, '{', '}') &&
        !VSTR__FMT_ADD_Q(name, len, '[', ']') &&
        !VSTR__FMT_ADD_Q(name, len, '<', '>') &&
        !VSTR__FMT_ADD_Q(name, len, '(', ')'))
      return (NULL);

    scan = vstr__fmt_beg(conf, name[1]);
  }
  
  while (*scan)
  {
    assert(!(*scan)->next || ((*scan)->name_len <= (*scan)->next->name_len));

    if (((*scan)->name_len == len) &&
        !vstr_wrap_memcmp((*scan)->name_str, name, len))
      return (scan);

    scan = &(*scan)->next;
  }

  return (NULL);
}

static void vstr__fmt_insert(Vstr__fmt_usr_name_node **scan,
                             Vstr__fmt_usr_name_node *node)
{
  while (*scan)
  {
    if ((*scan)->name_len >= node->name_len)
      break;

    scan = &(*scan)->next;
  }

  node->next = *scan;
  *scan = node;
}

static void vstr__fmt_flatten_hash(Vstr_conf *conf)
{
  unsigned int num = 0;

  ASSERT(!conf->fmt_usr_names);
  
  while (num < 37)
  {
    Vstr__fmt_usr_name_node *tmp = conf->fmt_usr_name_hash[num];

    conf->fmt_usr_name_hash[num] = NULL;

    while (tmp)
    {
      Vstr__fmt_usr_name_node *tmp_next = tmp->next;

      tmp->next = NULL;
      
      vstr__fmt_insert(&conf->fmt_usr_names, tmp);

      tmp = tmp_next;
    }

    ++num;
  }
}

/* like srch, but matches in a format (Ie. not zero terminated) */
Vstr__fmt_usr_name_node *vstr__fmt_usr_match(Vstr_conf *conf, const char *fmt)
{
  Vstr__fmt_usr_name_node *scan = conf->fmt_usr_names;
  size_t fmt_max_len = 0;

  if (conf->fmt_usr_curly_braces)
  { /* we know they follow a format of one of...
       "{" [^}]* "}"
       "[" [^]]* "]"
       "<" [^>]* ">"
       "(" [^)]* ")"
     * so we can find the length */
    char *ptr = NULL;
    size_t len = 0;
    
    ASSERT(!scan);
    
    switch (*fmt)
    {
      case '{': ptr = strchr(fmt, '}'); break;
      case '[': ptr = strchr(fmt, ']'); break;
      case '<': ptr = strchr(fmt, '>'); break;
      case '(': ptr = strchr(fmt, ')');
        ASSERT_NO_SWITCH_DEF();
    }
    
    if (!ptr)
      return (NULL);

    scan = *vstr__fmt_beg(conf, fmt[1]);
    
    len = (ptr - fmt) + 1;
    while (scan)
    {
      assert(!scan->next || (scan->name_len <= scan->next->name_len));

      if ((scan->name_len == len) &&
          !vstr_wrap_memcmp(scan->name_str, fmt, len))
        break;

      ASSERT_RET(scan->name_len <= len, NULL);

      scan = scan->next;
    }

    return (scan);
  }

  if (!conf->fmt_name_max)
  {
    while (scan)
    {
      if (conf->fmt_name_max < scan->name_len)
        conf->fmt_name_max = scan->name_len;

      scan = scan->next;
    }

    scan = conf->fmt_usr_names;
  }

  fmt_max_len = strnlen(fmt, conf->fmt_name_max);
  while (scan && (fmt_max_len >= scan->name_len))
  {
    assert(!scan->next || (scan->name_len <= scan->next->name_len));

    if (!vstr_wrap_memcmp(fmt, scan->name_str, scan->name_len))
      return (scan);

    scan = scan->next;
  }

  return (NULL);
}

int vstr_fmt_add(Vstr_conf *passed_conf, const char *name,
                 int (*func)(Vstr_base *, size_t, Vstr_fmt_spec *), ...)
{
  Vstr_conf *conf = passed_conf ? passed_conf : vstr__options.def;
  Vstr__fmt_usr_name_node **scan = &conf->fmt_usr_names;
  va_list ap;
  unsigned int count = 1;
  unsigned int scan_type = 0;
  Vstr__fmt_usr_name_node *node = NULL;

  if (vstr__fmt_usr_srch(conf, name))
    return (FALSE);

  node = VSTR__MK(sizeof(Vstr__fmt_usr_name_node) +
                  (sizeof(unsigned int) * count));

  if (!node)
  {
    conf->malloc_bad = TRUE;
    return (FALSE);
  }

  node->name_str = name;
  node->name_len = strlen(name);
  node->func = func;

  if (conf->fmt_usr_curly_braces &&
      !VSTR__FMT_ADD_Q(name, node->name_len, '{', '}') &&
      !VSTR__FMT_ADD_Q(name, node->name_len, '[', ']') &&
      !VSTR__FMT_ADD_Q(name, node->name_len, '<', '>') &&
      !VSTR__FMT_ADD_Q(name, node->name_len, '(', ')'))
  {
    conf->fmt_usr_curly_braces = FALSE;
    vstr__fmt_flatten_hash(conf);
  }

  if (conf->fmt_usr_curly_braces)
  {
    ASSERT(!*scan);
    scan = vstr__fmt_beg(conf, name[1]);
  }
  
  va_start(ap, func);
  while ((scan_type = va_arg(ap, unsigned int)))
  {
    Vstr__fmt_usr_name_node *tmp_node = NULL;

    ++count;
    if (!VSTR__MV(node, tmp_node, (sizeof(Vstr__fmt_usr_name_node) +
                                   (sizeof(unsigned int) * count))))
    {
      conf->malloc_bad = TRUE;
      VSTR__F(node);
      va_end(ap);
      return (FALSE);
    }

    assert(FALSE ||
           (scan_type == VSTR_TYPE_FMT_INT) ||
           (scan_type == VSTR_TYPE_FMT_UINT) ||
           (scan_type == VSTR_TYPE_FMT_LONG) ||
           (scan_type == VSTR_TYPE_FMT_ULONG) ||
           (scan_type == VSTR_TYPE_FMT_LONG_LONG) ||
           (scan_type == VSTR_TYPE_FMT_ULONG_LONG) ||
           (scan_type == VSTR_TYPE_FMT_SSIZE_T) ||
           (scan_type == VSTR_TYPE_FMT_SIZE_T) ||
           (scan_type == VSTR_TYPE_FMT_PTRDIFF_T) ||
           (scan_type == VSTR_TYPE_FMT_INTMAX_T) ||
           (scan_type == VSTR_TYPE_FMT_UINTMAX_T) ||
           (scan_type == VSTR_TYPE_FMT_DOUBLE) ||
           (scan_type == VSTR_TYPE_FMT_DOUBLE_LONG) ||
           (scan_type == VSTR_TYPE_FMT_PTR_VOID) ||
           (scan_type == VSTR_TYPE_FMT_PTR_CHAR) ||
           (scan_type == VSTR_TYPE_FMT_PTR_WCHAR_T) ||
           (scan_type == VSTR_TYPE_FMT_ERRNO) ||
           (scan_type == VSTR_TYPE_FMT_PTR_SIGNED_CHAR) ||
           (scan_type == VSTR_TYPE_FMT_PTR_SHORT) ||
           (scan_type == VSTR_TYPE_FMT_PTR_INT) ||
           (scan_type == VSTR_TYPE_FMT_PTR_LONG) ||
           (scan_type == VSTR_TYPE_FMT_PTR_LONG_LONG) ||
           (scan_type == VSTR_TYPE_FMT_PTR_SSIZE_T) ||
           (scan_type == VSTR_TYPE_FMT_PTR_PTRDIFF_T) ||
           (scan_type == VSTR_TYPE_FMT_PTR_INTMAX_T) ||
           FALSE);

    node->types[count - 2] = scan_type;
  }
  assert(count >= 1);
  node->types[count - 1] = scan_type;
  node->sz = count;

  va_end(ap);

  if (!*scan || (conf->fmt_name_max && (conf->fmt_name_max < node->name_len)))
    conf->fmt_name_max = node->name_len;

  vstr__fmt_insert(scan, node);

  ASSERT(vstr__fmt_usr_srch(conf, name));

  return (TRUE);
}

void vstr_fmt_del(Vstr_conf *passed_conf, const char *name)
{
  Vstr_conf *conf = passed_conf ? passed_conf : vstr__options.def;
  Vstr__fmt_usr_name_node **scan = vstr__fmt_usr_srch(conf, name);

  if (scan)
  {
    Vstr__fmt_usr_name_node *tmp = *scan;

    assert(tmp);

    *scan = tmp->next;

    if (tmp->name_len == conf->fmt_name_max)
      conf->fmt_name_max = 0;

    VSTR__F(tmp);
  }

  if (!conf->fmt_usr_curly_braces && !conf->fmt_usr_names)
    conf->fmt_usr_curly_braces = TRUE;
}

int vstr_fmt_srch(Vstr_conf *passed_conf, const char *name)
{
  Vstr_conf *conf = passed_conf ? passed_conf : vstr__options.def;
  return (!!vstr__fmt_usr_srch(conf, name));
}
#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(fmt_add);
VSTR__SYM_ALIAS(fmt_del);
VSTR__SYM_ALIAS(fmt_srch);
