#define VSTR_CNTL_C
/*
 *  Copyright (C) 1999, 2000, 2001, 2002, 2003  James Antill
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
/* functions for the "misc" control stuff. like reading struct values etc. */
#include "main.h"


Vstr__options vstr__options =
{
 NULL, /* def */

 0, /* mmap count */

 0, /* fd count */
 0, /* fd close fail num */
};

int vstr_cntl_opt(int option, ...)
{
  int ret = 0;

  va_list ap;

  va_start(ap, option);

  switch (option)
  {
    case VSTR_CNTL_OPT_GET_CONF:
    {
      Vstr_conf **val = va_arg(ap, Vstr_conf **);

      vstr__add_user_conf(*val = vstr__options.def);

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_OPT_SET_CONF:
    {
      Vstr_conf *val = va_arg(ap, Vstr_conf *);

      ASSERT(val);
      
      if (vstr__options.def != val)
      {
        vstr_free_conf(vstr__options.def);
        vstr__add_user_conf(vstr__options.def = val);
      }

      ret = TRUE;
    }
    
#if USE_MALLOC_CHECK || USE_FD_CLOSE_CHECK
    break;
    
    case 666:
    {
      unsigned long valT = va_arg(ap, unsigned long);
      unsigned long valV = va_arg(ap, unsigned long);

      ASSERT((valT == 0x0F0F) || (valT == 0xF0F0));
      
      if (0) {}
      else if (USE_FD_CLOSE_CHECK && (valT == 0x0F0F))
      { vstr__options.fd_close_fail_num = valV; ret = TRUE; }
      else if (USE_MALLOC_CHECK   && (valT == 0xF0F0))
      { MALLOC_CHECK_FAIL_IN(valV); ret = TRUE; }
    }
#endif

    ASSERT_NO_SWITCH_DEF();
  }

  va_end(ap);

  return (ret);
}

int vstr_cntl_base(Vstr_base *base, int option, ...)
{
  int ret = 0;

  va_list ap;

  va_start(ap, option);

  switch (option)
  {
    case VSTR_CNTL_BASE_GET_CONF:
    {
      Vstr_conf **val = va_arg(ap, Vstr_conf **);

      vstr__add_user_conf(*val = base->conf);

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_BASE_SET_CONF:
    {
      Vstr_conf *val = va_arg(ap, Vstr_conf *);

      if (!val)
        val = vstr__options.def;

      if (base->conf == val)
        ret = TRUE;
      else if (((val->buf_sz == base->conf->buf_sz) || !base->len) &&
               vstr__cache_subset_cbs(val, base->conf))
      {
        vstr__del_conf(base->conf);
        vstr__add_base_conf(base, val);

        ret = TRUE;
      }
    }
    break;

    case VSTR_CNTL_BASE_GET_FLAG_HAVE_CACHE:
    {
      int *val = va_arg(ap, int *);

      *val = !!base->cache_available;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_BASE_GET_TYPE_GRPALLOC_CACHE:
    {
      unsigned int *val = va_arg(ap, unsigned int *);

      *val = base->grpalloc_cache;

      ret = TRUE;
    }

    ASSERT_NO_SWITCH_DEF();
  }

  va_end(ap);

  return (ret);
}

int vstr_cntl_conf(Vstr_conf *passed_conf, int option, ...)
{
  Vstr_conf *conf = passed_conf ? passed_conf : vstr__options.def;
  int ret = 0;
  va_list ap;
  
  assert(conf->user_ref <= conf->ref);

  va_start(ap, option);

  switch (option)
  {
    case VSTR_CNTL_CONF_GET_NUM_REF:
    {
      int *val = va_arg(ap, int *);

      *val = conf->ref;
      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_GET_NUM_IOV_MIN_ALLOC:
    {
      int *val = va_arg(ap, int *);

      *val = conf->iov_min_alloc;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_SET_NUM_IOV_MIN_ALLOC:
    {
      int val = va_arg(ap, int);

      conf->iov_min_alloc = val;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_GET_NUM_IOV_MIN_OFFSET:
    {
      int *val = va_arg(ap, int *);

      *val = conf->iov_min_offset;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_SET_NUM_IOV_MIN_OFFSET:
    {
      int val = va_arg(ap, int);

      conf->iov_min_offset = val;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_GET_NUM_BUF_SZ:
    {
      unsigned int *val = va_arg(ap, unsigned int *);

      *val = conf->buf_sz;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_SET_NUM_BUF_SZ:
    {
      unsigned int val = va_arg(ap, unsigned int);

      if (val > VSTR_MAX_NODE_BUF)
        val = VSTR_MAX_NODE_BUF;

      /* this is too restrictive, but getting it "right" would require too much
       * bookkeeping. */
      if (!conf->spare_buf_num && (conf->user_ref == conf->ref))
      {
        conf->buf_sz = val;

        ret = TRUE;
      }
    }
    break;

    case VSTR_CNTL_CONF_SET_LOC_CSTR_AUTO_NAME_NUMERIC:
    {
      const char *val = va_arg(ap, const char *);

      ret = vstr__make_conf_loc_numeric(conf, val);
    }
    break;

    case VSTR_CNTL_CONF_GET_LOC_CSTR_NAME_NUMERIC:
    {
      const char **val = va_arg(ap, const char **);

      *val = conf->loc->name_lc_numeric_ref->ptr;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_SET_LOC_CSTR_NAME_NUMERIC:
    {
      const char *val = va_arg(ap, const char *);
      Vstr_ref *tmp = NULL;
      
      if (!(tmp = vstr_ref_make_strdup(val)))
        break;

      vstr_ref_del(conf->loc->name_lc_numeric_ref);
      conf->loc->name_lc_numeric_ref = tmp;
      conf->loc->name_lc_numeric_len = strlen(tmp->ptr);

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_GET_LOC_CSTR_DEC_POINT:
    {
      const char **val = va_arg(ap, const char **);
      Vstr_locale_num_base *srch = vstr__loc_num_srch(conf->loc, 0, FALSE);

      *val = srch->decimal_point_ref->ptr;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_SET_LOC_CSTR_DEC_POINT:
    {
      const char *val = va_arg(ap, const char *);
      Vstr_ref *tmp = NULL;
      Vstr_locale_num_base *srch = vstr__loc_num_srch(conf->loc, 0, FALSE);
      
      if (!(tmp = vstr_ref_make_strdup(val)))
        break;

      vstr_ref_del(srch->decimal_point_ref);
      srch->decimal_point_ref = tmp;
      srch->decimal_point_len = strlen(tmp->ptr);

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_GET_LOC_CSTR_THOU_SEP:
    {
      const char **val = va_arg(ap, const char **);
      Vstr_locale_num_base *srch = vstr__loc_num_srch(conf->loc, 0, FALSE);

      *val = srch->thousands_sep_ref->ptr;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_SET_LOC_CSTR_THOU_SEP:
    {
      const char *val = va_arg(ap, const char *);
      Vstr_ref *tmp = NULL;
      Vstr_locale_num_base *srch = vstr__loc_num_srch(conf->loc, 0, FALSE);

      if (!(tmp = vstr_ref_make_strdup(val)))
        break;

      vstr_ref_del(srch->thousands_sep_ref);
      srch->thousands_sep_ref = tmp;
      srch->thousands_sep_len = strlen(tmp->ptr);

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_GET_LOC_CSTR_THOU_GRP:
    {
      const char **val = va_arg(ap, const char **);
      Vstr_locale_num_base *srch = vstr__loc_num_srch(conf->loc, 0, FALSE);

      *val = srch->grouping->ptr;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_SET_LOC_CSTR_THOU_GRP:
    {
      const char *val = va_arg(ap, const char *);
      Vstr_ref *tmp = NULL;
      size_t len = vstr__loc_thou_grp_strlen(val);
      Vstr_locale_num_base *srch = vstr__loc_num_srch(conf->loc, 0, FALSE);

      if (!(tmp = vstr_ref_make_malloc(len + 1)))
        break;

      vstr_ref_del(srch->grouping);
      if (len)
        vstr_wrap_memcpy(tmp->ptr, val, len);
      ((char *)tmp->ptr)[len] = 0;
      srch->grouping = tmp;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_GET_FLAG_IOV_UPDATE:
    {
      int *val = va_arg(ap, int *);

      *val = conf->iovec_auto_update;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_SET_FLAG_IOV_UPDATE:
    {
      int val = va_arg(ap, int);

      assert(val == !!val);

      conf->iovec_auto_update = val;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_GET_FLAG_DEL_SPLIT:
    {
      int *val = va_arg(ap, int *);

      *val = conf->split_buf_del;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_SET_FLAG_DEL_SPLIT:
    {
      int val = va_arg(ap, int);

      assert(val == !!val);

      conf->split_buf_del = val;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_GET_FLAG_ALLOC_CACHE:
    {
      int *val = va_arg(ap, int *);

      *val = !conf->no_cache;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_SET_FLAG_ALLOC_CACHE:
    {
      int val = va_arg(ap, int);

      assert(val == !!val);

      ret = TRUE;

      if (conf->no_cache == !val)
        break;

      vstr__del_grpalloc(conf, conf->spare_base_num);
      
      conf->no_cache = !val;
      if (conf->no_cache)
        conf->grpalloc_cache = VSTR_TYPE_CNTL_CONF_GRPALLOC_NONE;
      else
        conf->grpalloc_cache = VSTR_TYPE_CNTL_CONF_GRPALLOC_POS;
    }
    break;

    case VSTR_CNTL_CONF_GET_FMT_CHAR_ESC:
    {
      char *val = va_arg(ap, char *);

      *val = conf->fmt_usr_escape;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_SET_FMT_CHAR_ESC:
    {
      int val = va_arg(ap, int);

      conf->fmt_usr_escape = val;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_GET_NUM_SPARE_BUF:
    case VSTR_CNTL_CONF_GET_NUM_SPARE_NON:
    case VSTR_CNTL_CONF_GET_NUM_SPARE_PTR:
    case VSTR_CNTL_CONF_GET_NUM_SPARE_REF:
    {
      unsigned int *val = va_arg(ap, unsigned int *);

      switch (option)
      {
        case VSTR_CNTL_CONF_GET_NUM_SPARE_BUF: *val= conf->spare_buf_num; break;
        case VSTR_CNTL_CONF_GET_NUM_SPARE_NON: *val= conf->spare_non_num; break;
        case VSTR_CNTL_CONF_GET_NUM_SPARE_PTR: *val= conf->spare_ptr_num; break;
        case VSTR_CNTL_CONF_GET_NUM_SPARE_REF: *val= conf->spare_ref_num; break;
      }

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_SET_NUM_SPARE_BUF:
    case VSTR_CNTL_CONF_SET_NUM_SPARE_NON:
    case VSTR_CNTL_CONF_SET_NUM_SPARE_PTR:
    case VSTR_CNTL_CONF_SET_NUM_SPARE_REF:
    {
      unsigned int val = va_arg(ap, unsigned int);
      unsigned int type = 0;
      unsigned int spare_num = 0;

      switch (option)
      {
        case VSTR_CNTL_CONF_SET_NUM_SPARE_BUF:
          type      = VSTR_TYPE_NODE_BUF;
          spare_num = conf->spare_buf_num;
          break;
        case VSTR_CNTL_CONF_SET_NUM_SPARE_NON:
          type      = VSTR_TYPE_NODE_NON;
          spare_num = conf->spare_non_num;
          break;
        case VSTR_CNTL_CONF_SET_NUM_SPARE_PTR:
          type      = VSTR_TYPE_NODE_PTR;
          spare_num = conf->spare_ptr_num;
          break;
        case VSTR_CNTL_CONF_SET_NUM_SPARE_REF:
          type      = VSTR_TYPE_NODE_REF;
          spare_num = conf->spare_ref_num;
          break;
      }

      if (val == spare_num)
      { /* do nothing */ }
      else if (val > spare_num)
      {
        unsigned int num = 0;

        num = vstr_make_spare_nodes(conf, type, val - spare_num);
        if (num != (val - spare_num))
        {
          assert(ret == FALSE);
          break;
        }
      }
      else if (val < spare_num)
      {
        unsigned int num = 0;

        num = vstr_free_spare_nodes(conf, type, spare_num - val);
        ASSERT(num == (spare_num - val));
      }

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_BUF:
    case VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_NON:
    case VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_PTR:
    case VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_REF:
    {
      unsigned int val_min = va_arg(ap, unsigned int);
      unsigned int val_max = va_arg(ap, unsigned int);
      unsigned int type = 0;
      unsigned int spare_num = 0;

      ASSERT(val_min <= val_max);
      
      switch (option)
      {
        case VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_BUF:
          type      = VSTR_CNTL_CONF_SET_NUM_SPARE_BUF;
          spare_num = conf->spare_buf_num;
          break;
        case VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_NON:
          type      = VSTR_CNTL_CONF_SET_NUM_SPARE_NON;
          spare_num = conf->spare_non_num;
          break;
        case VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_PTR:
          type      = VSTR_CNTL_CONF_SET_NUM_SPARE_PTR;
          spare_num = conf->spare_ptr_num;
          break;
        case VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_REF:
          type      = VSTR_CNTL_CONF_SET_NUM_SPARE_REF;
          spare_num = conf->spare_ref_num;
          
          ASSERT_NO_SWITCH_DEF();
      }

      if (0)
      { ASSERT(FALSE); }
      else if (val_min > spare_num)
        return (vstr_cntl_conf(conf, type, val_min));
      else if (val_max < spare_num)
        return (vstr_cntl_conf(conf, type, val_max));

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_GET_FLAG_ATOMIC_OPS:
    {
      int *val = va_arg(ap, int *);

      *val = conf->atomic_ops;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_SET_FLAG_ATOMIC_OPS:
    {
      int val = va_arg(ap, int);

      assert(val == !!val);

      conf->atomic_ops = val;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_GET_TYPE_GRPALLOC_CACHE:
    {
      unsigned int *val = va_arg(ap, unsigned int *);

      *val = conf->grpalloc_cache;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_SET_TYPE_GRPALLOC_CACHE:
    {
      unsigned int val = va_arg(ap, unsigned int);

      ASSERT_RET(((val == VSTR_TYPE_CNTL_CONF_GRPALLOC_NONE) ||
                  (val == VSTR_TYPE_CNTL_CONF_GRPALLOC_POS) ||
                  (val == VSTR_TYPE_CNTL_CONF_GRPALLOC_IOVEC) ||
                  (val == VSTR_TYPE_CNTL_CONF_GRPALLOC_CSTR) ||
                  FALSE), FALSE);

      ret = TRUE;
      
      if (conf->grpalloc_cache == val)
        break;
      
      vstr__del_grpalloc(conf, conf->spare_base_num);

      conf->grpalloc_cache = val;
      if (val == VSTR_TYPE_CNTL_CONF_GRPALLOC_NONE)
        conf->no_cache = TRUE;
      else
        conf->no_cache = FALSE;        
    }
    break;
    
    case VSTR_CNTL_CONF_GET_NUM_SPARE_BASE:
    {
      unsigned int *val = va_arg(ap, unsigned int *);

      *val = conf->spare_base_num;
      
      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_SET_NUM_SPARE_BASE:
    {
      unsigned int val = va_arg(ap, unsigned int);
      unsigned int spare_num = conf->spare_base_num;
      
      if (val == spare_num)
      { /* do nothing */ }
      else if (val > spare_num)
      {
        Vstr_base *tmp = NULL;
        Vstr_base *old_beg = NULL;
        unsigned int old_num = 0;

        while (conf->spare_base_num < val)
        {
          old_beg = conf->spare_base_beg;
          old_num = conf->spare_base_num;
          
          conf->spare_base_beg = NULL;
          conf->spare_base_num = 0;
          
          tmp = vstr_make_base(conf);
          
          conf->spare_base_beg = old_beg;
          conf->spare_base_num = old_num;
          
          if (!tmp)
            return (FALSE);

          vstr_free_base(tmp);
        }
      }
      else if (val < spare_num)
        vstr__del_grpalloc(conf, spare_num - val);
      
      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_SET_NUM_RANGE_SPARE_BASE:
    {
      unsigned int val_min = va_arg(ap, unsigned int);
      unsigned int val_max = va_arg(ap, unsigned int);
      unsigned int spare_num = conf->spare_base_num;
      
      ASSERT(val_min <= val_max);
      
      if (0)
      { ASSERT(FALSE); }
      else if (val_min > spare_num)
        return (vstr_cntl_conf(conf,
                               VSTR_CNTL_CONF_SET_NUM_SPARE_BASE, val_min));
      else if (val_max < spare_num)
        return (vstr_cntl_conf(conf,
                               VSTR_CNTL_CONF_SET_NUM_SPARE_BASE, val_max));
      
      ret = TRUE;
    }
    break;
    
    case VSTR_CNTL_CONF_GET_LOC_REF_NAME_NUMERIC:
    {
      Vstr_ref **val_ref = va_arg(ap, Vstr_ref **);
      size_t *val_len = va_arg(ap, size_t *);
      
      *val_ref = vstr_ref_add(conf->loc->name_lc_numeric_ref);
      *val_len =              conf->loc->name_lc_numeric_len;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_SET_LOC_REF_NAME_NUMERIC:
    {
      Vstr_ref *tmp = va_arg(ap, Vstr_ref *);
      size_t len = va_arg(ap, size_t);

      vstr_ref_del(conf->loc->name_lc_numeric_ref);
      
      conf->loc->name_lc_numeric_ref = vstr_ref_add(tmp);
      conf->loc->name_lc_numeric_len = len;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_GET_LOC_REF_DEC_POINT:
    {
      unsigned int nb = va_arg(ap, unsigned int);
      Vstr_ref **val_ref = va_arg(ap, Vstr_ref **);
      size_t *val_len = va_arg(ap, size_t *);
      Vstr_locale_num_base *srch = vstr__loc_num_srch(conf->loc, nb, FALSE);
      
      *val_ref = vstr_ref_add(srch->decimal_point_ref);
      *val_len =              srch->decimal_point_len;
      
      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_SET_LOC_REF_DEC_POINT:
    {
      unsigned int nb = va_arg(ap, unsigned int);
      Vstr_ref *tmp = va_arg(ap, Vstr_ref *);
      size_t len = va_arg(ap, size_t);
      Vstr_locale_num_base *srch = vstr__loc_num_srch(conf->loc, nb, TRUE);

      if (!srch) break;
      
      vstr_ref_del(srch->decimal_point_ref);

      srch->decimal_point_ref = vstr_ref_add(tmp);
      srch->decimal_point_len = len;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_GET_LOC_REF_THOU_SEP:
    {
      unsigned int nb = va_arg(ap, unsigned int);
      Vstr_ref **val_ref = va_arg(ap, Vstr_ref **);
      size_t *val_len = va_arg(ap, size_t *);
      Vstr_locale_num_base *srch = vstr__loc_num_srch(conf->loc, nb, FALSE);

      *val_ref = vstr_ref_add(srch->thousands_sep_ref);
      *val_len =              srch->thousands_sep_len;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_SET_LOC_REF_THOU_SEP:
    {
      unsigned int nb = va_arg(ap, unsigned int);
      Vstr_ref *tmp = va_arg(ap, Vstr_ref *);
      size_t len = va_arg(ap, size_t);
      Vstr_locale_num_base *srch = vstr__loc_num_srch(conf->loc, nb, TRUE);

      if (!srch) break;
      
      vstr_ref_del(srch->thousands_sep_ref);

      srch->thousands_sep_ref = vstr_ref_add(tmp);
      srch->thousands_sep_len = len;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_GET_LOC_REF_THOU_GRP:
    {
      unsigned int nb = va_arg(ap, unsigned int);
      Vstr_ref **val_ref = va_arg(ap, Vstr_ref **);
      Vstr_locale_num_base *srch = vstr__loc_num_srch(conf->loc, nb, FALSE);

      *val_ref = vstr_ref_add(srch->grouping);

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_SET_LOC_REF_THOU_GRP:
    {
      unsigned int nb = va_arg(ap, unsigned int);
      Vstr_ref *tmp = va_arg(ap, Vstr_ref *);
      Vstr_locale_num_base *srch = vstr__loc_num_srch(conf->loc, nb, TRUE);

      if (!srch) break;
      
      vstr_ref_del(srch->grouping);

      srch->grouping = vstr_ref_add(tmp);

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_GET_LOC_REF_NULL_PTR:
    {
      Vstr_ref **val_ref = va_arg(ap, Vstr_ref **);
      size_t *val_len = va_arg(ap, size_t *);
      
      *val_ref = vstr_ref_add(conf->loc->null_ref);
      *val_len =              conf->loc->null_len;

      ret = TRUE;
    }
    break;

    case VSTR_CNTL_CONF_SET_LOC_REF_NULL_PTR:
    {
      Vstr_ref *tmp = va_arg(ap, Vstr_ref *);
      size_t len = va_arg(ap, size_t);

      vstr_ref_del(conf->loc->null_ref);
      
      conf->loc->null_ref = vstr_ref_add(tmp);
      conf->loc->null_len = len;

      ret = TRUE;
    }
    /* break in assert */

   ASSERT_NO_SWITCH_DEF();
  }

  va_end(ap);

  return (ret);
}
#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(cntl_base);
VSTR__SYM_ALIAS(cntl_conf);
VSTR__SYM_ALIAS(cntl_opt);
