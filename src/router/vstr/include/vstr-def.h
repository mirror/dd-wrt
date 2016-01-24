#ifndef VSTR__HEADER_H
# error " You must _just_ #include <vstr.h>"
#endif
/*
 *  Copyright (C) 1999-2004, 2006  James Antill
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

VSTR__DECL_TYPEDEF1(struct Vstr_ref)
{
 void (*func) (struct Vstr_ref *); /* public/read|write */
 void *ptr; /* public/read|write */
 unsigned int ref; /* public/read|write */
}  VSTR__DECL_TYPEDEF2(Vstr_ref);

#define VSTR__DEF_BITFLAG_1_4(x) \
 unsigned int unused1_ ## x : 1; \
 unsigned int unused2_ ## x : 1; \
 unsigned int unused3_ ## x : 1; \
 unsigned int unused4_ ## x : 1

VSTR__DECL_TYPEDEF1(struct Vstr_node)
{
 struct Vstr_node *next; /* private */

 unsigned int len : 28; /* private */

 unsigned int type : 4; /* private */
} VSTR__DECL_TYPEDEF2(Vstr_node);

VSTR__DECL_TYPEDEF1(struct Vstr_node_buf)
{
 struct Vstr_node s; /* private */
 char VSTR__STRUCT_HACK_ARRAY(buf); /* private */
} VSTR__DECL_TYPEDEF2(Vstr_node_buf);

VSTR__DECL_TYPEDEF1(struct Vstr_node_non)
{
 struct Vstr_node s; /* private */
} VSTR__DECL_TYPEDEF2(Vstr_node_non);

VSTR__DECL_TYPEDEF1(struct Vstr_node_ptr)
{
 struct Vstr_node s; /* private */
 void *ptr; /* private */
} VSTR__DECL_TYPEDEF2(Vstr_node_ptr);

VSTR__DECL_TYPEDEF1(struct Vstr_node_ref)
{
 struct Vstr_node s; /* private */
 struct Vstr_ref *ref; /* private */
 unsigned int off;
} VSTR__DECL_TYPEDEF2(Vstr_node_ref);

VSTR__DECL_TYPEDEF1(struct Vstr_fmt_spec)
{
  size_t vstr_orig_len; /* public/read|write */

  unsigned int obj_precision; /* public/read|write */
  unsigned int obj_field_width; /* public/read|write */

  unsigned int fmt_precision     : 1; /* public/read|write */
  unsigned int fmt_field_width   : 1; /* public/read|write */

  unsigned int fmt_minus         : 1; /* public/read|write */
  unsigned int fmt_plus          : 1; /* public/read|write */
  unsigned int fmt_space         : 1; /* public/read|write */
  unsigned int fmt_hash          : 1; /* public/read|write */
  unsigned int fmt_zero          : 1; /* public/read|write */
  unsigned int fmt_quote         : 1; /* public/read|write */
  unsigned int fmt_I             : 1; /* public/read|write */

  VSTR__DEF_BITFLAG_1_4(1); /* private */
  VSTR__DEF_BITFLAG_1_4(2); /* private */
  VSTR__DEF_BITFLAG_1_4(3); /* private */
  VSTR__DEF_BITFLAG_1_4(4); /* private */
  VSTR__DEF_BITFLAG_1_4(5); /* private */

  const char *name; /* public/read|write */

  void *VSTR__STRUCT_HACK_ARRAY(data_ptr); /* public/read|write */
} VSTR__DECL_TYPEDEF2(Vstr_fmt_spec);

VSTR__DECL_TYPEDEF1(struct Vstr_locale_num_base)
{
  unsigned int num_base; /* private */
  struct Vstr_locale_num_base *next; /* private */
  struct Vstr_ref *decimal_point_ref; /* private */
  struct Vstr_ref *thousands_sep_ref; /* private */
  struct Vstr_ref *grouping; /* private */
  size_t decimal_point_len; /* private */
  size_t thousands_sep_len; /* private */
} VSTR__DECL_TYPEDEF2(Vstr_locale_num_base);

VSTR__DECL_TYPEDEF1(struct Vstr_locale)
{
  struct Vstr_ref *name_lc_numeric_ref; /* private */
  size_t name_lc_numeric_len; /* private */
  struct Vstr_locale_num_base *num_beg; /* private */
  
  struct Vstr_ref *null_ref; /* private */
  size_t null_len; /* private */
} VSTR__DECL_TYPEDEF2(Vstr_locale);

struct Vstr_base; /* fwd declaration for callbacks */

VSTR__DECL_TYPEDEF1(struct Vstr_cache_cb)
{
  const char *name; /* private */
  void *(*cb_func)(const struct Vstr_base *, size_t, size_t,
                   unsigned int, void *); /* private */
} VSTR__DECL_TYPEDEF2(Vstr_cache_cb);

VSTR__DECL_TYPEDEF1(struct Vstr_data_usr)
{
  const char *name; /* private */
  struct Vstr_ref *data; /* private */
} VSTR__DECL_TYPEDEF2(Vstr_data_usr);

VSTR__DECL_TYPEDEF1(struct Vstr_ref_grp_ptr)
{
 unsigned char make_num; /* private */
 unsigned char free_num; /* private */
 unsigned char flags; /* private */
 
 void (*func)(struct Vstr_ref *); /* private */
 
 struct Vstr_ref VSTR__STRUCT_HACK_ARRAY(refs); /* private */
} VSTR__DECL_TYPEDEF2(Vstr_ref_grp_ptr);

VSTR__DECL_TYPEDEF1(struct Vstr_conf)
{
  unsigned int spare_buf_num; /* private */
  struct Vstr_node_buf *spare_buf_beg; /* private */

  unsigned int spare_non_num; /* private */
  struct Vstr_node_non *spare_non_beg; /* private */

  unsigned int spare_ptr_num; /* private */
  struct Vstr_node_ptr *spare_ptr_beg; /* private */

  unsigned int spare_ref_num; /* private */
  struct Vstr_node_ref *spare_ref_beg; /* private */

  struct Vstr_locale *loc; /* private */

  unsigned int iov_min_alloc; /* private */
  unsigned int iov_min_offset; /* private */

  unsigned int buf_sz; /* private */

  struct Vstr_cache_cb *cache_cbs_ents; /* private */
  unsigned int cache_cbs_sz; /* private */

  unsigned int cache_pos_cb_pos; /* private */
  unsigned int cache_pos_cb_iovec; /* private */
  unsigned int cache_pos_cb_cstr; /* private */
  unsigned int cache_pos_cb_sects; /* private */

  unsigned char fmt_usr_escape; /* private */
  struct Vstr__fmt_usr_name_node *fmt_usr_names; /* private */
  size_t fmt_name_max; /* private */

  struct Vstr__fmt_spec *vstr__fmt_spec_make; /* private */
  struct Vstr__fmt_spec *vstr__fmt_spec_list_beg; /* private */
  struct Vstr__fmt_spec *vstr__fmt_spec_list_end; /* private */

  int ref; /* private */
  int user_ref; /* private */

  struct Vstr__conf_ref_linked *ref_link; /* private */

  unsigned int free_do : 1; /* private */
  unsigned int malloc_bad : 1; /* public/read|write */
  unsigned int iovec_auto_update : 1; /* private */
  unsigned int split_buf_del : 1; /* private */

  unsigned int no_cache : 1; /* private */
  unsigned int fmt_usr_curly_braces : 1; /* private */
  unsigned int atomic_ops : 1; /* private */

  unsigned int grpalloc_cache : 3; /* private */

  VSTR__DEF_BITFLAG_1_4(4); /* private */
  VSTR__DEF_BITFLAG_1_4(5); /* private */
  VSTR__DEF_BITFLAG_1_4(6); /* private */
  VSTR__DEF_BITFLAG_1_4(7); /* private */
  VSTR__DEF_BITFLAG_1_4(8); /* private */

  /* ABI compat... */
  unsigned int spare_base_num; /* private */
  struct Vstr_base *spare_base_beg; /* private */
  
  struct Vstr_data_usr *data_usr_ents; /* private */
  unsigned int data_usr_len; /* private */
  unsigned int data_usr_sz; /* private */

  struct Vstr_ref_grp_ptr *ref_grp_ptr; /* private */
  struct Vstr_ref_grp_ptr *ref_grp_buf2ref; /* private */
  
  struct Vstr__fmt_usr_name_node *fmt_usr_name_hash[37]; /* private */
} VSTR__DECL_TYPEDEF2(Vstr_conf);

VSTR__DECL_TYPEDEF1(struct Vstr_base)
{
  size_t len; /* public/read -- bytes in vstr */

  struct Vstr_node *beg; /* private */
  struct Vstr_node *end; /* private */

  unsigned int num; /* private */

  struct Vstr_conf *conf; /* public/read */

  unsigned int used : 16; /* private */

  unsigned int free_do : 1; /* private */
  unsigned int iovec_upto_date : 1; /* private */
  unsigned int cache_available : 1; /* private */
  unsigned int cache_internal : 1; /* private */

  unsigned int node_buf_used : 1; /* private */
  unsigned int node_non_used : 1; /* private */
  unsigned int node_ptr_used : 1; /* private */
  unsigned int node_ref_used : 1; /* private */

  unsigned int grpalloc_cache : 3; /* private */

  unsigned int unused7_4 : 1; /* private */
  
  VSTR__DEF_BITFLAG_1_4(8); /* private */
} VSTR__DECL_TYPEDEF2(Vstr_base);

VSTR__DECL_TYPEDEF1(struct Vstr_sect_node)
{
  size_t pos;
  size_t len;
} VSTR__DECL_TYPEDEF2(Vstr_sect_node);

VSTR__DECL_TYPEDEF1(struct Vstr_sects)
{
  size_t num; /* public/read|write */
  size_t sz; /* public/read|write */

  unsigned int malloc_bad : 1; /* public/read|write */
  unsigned int free_ptr : 1; /* public/read|write */
  unsigned int can_add_sz : 1; /* public/read|write */
  unsigned int can_del_sz : 1; /* public/read|write */

  unsigned int alloc_double : 1; /* public/read|write */
  unsigned int unused02 : 1; /* private */
  unsigned int unused03 : 1; /* private */
  unsigned int unused04 : 1; /* private */

  VSTR__DEF_BITFLAG_1_4(3); /* private */
  VSTR__DEF_BITFLAG_1_4(4); /* private */
  VSTR__DEF_BITFLAG_1_4(5); /* private */
  VSTR__DEF_BITFLAG_1_4(6); /* private */
  VSTR__DEF_BITFLAG_1_4(7); /* private */
  VSTR__DEF_BITFLAG_1_4(8); /* private */

  struct Vstr_sect_node *ptr; /* public/read|write */
  struct Vstr_sect_node VSTR__STRUCT_ARRAY_HACK_ARRAY(integrated_objs); /* priavte */
} VSTR__DECL_TYPEDEF2(Vstr_sects);

VSTR__DECL_TYPEDEF1(struct Vstr_iter)
{
  const char *ptr; /* public/read|write */
  size_t len; /* public/read|write */
  unsigned int num; /* public/read */
  struct Vstr_node *node; /* private */
  size_t remaining; /* private */
} VSTR__DECL_TYPEDEF2(Vstr_iter);

/* internal defines ... */

struct Vstr__cache_data_pos
{
 size_t pos;
 unsigned int num;
 struct Vstr_node *node;
};

struct Vstr__cache_data_iovec
{
 struct iovec *v;
 unsigned char *t;
 /* num == base->num */
 unsigned int off;
 unsigned int sz;
};

struct Vstr__cache_data_cstr
{
 size_t pos;
 size_t len;
 struct Vstr_ref *ref;
 size_t sz; /* FIXME: ABI: order must be this, for 1.0.x ABI */
 size_t off;
};

struct Vstr__cache
{
 unsigned int sz;

 struct Vstr__cache_data_iovec *vec;

 void *VSTR__STRUCT_HACK_ARRAY(data);
};

struct Vstr__base_cache
{
 struct Vstr_base base;
 struct Vstr__cache *cache;
};

struct Vstr__base_p_cache
{
 struct Vstr__base_cache s;
 struct Vstr__cache_data_pos   real_pos;
};
struct Vstr__base_pi_cache
{
 struct Vstr__base_p_cache s;
 struct Vstr__cache_data_iovec real_iov;
};
struct Vstr__base_pic_cache
{
 struct Vstr__base_pi_cache s;
 struct Vstr__cache_data_cstr  real_cstr;
};

