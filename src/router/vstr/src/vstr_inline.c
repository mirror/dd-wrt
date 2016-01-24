#define VSTR_INLINE_C
/*
 *  Copyright (C) 2002, 2003  James Antill
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
/* functions which are inlined */

#undef  VSTR_COMPILE_INLINE /* gcc3 SEGVs if it sees the inline's twice */
#define VSTR_COMPILE_INLINE 0

#include "main.h"

/* everything is done in vstr-inline.h with magic added vstr-internal.h */
/* This works with gcc */
# undef extern
# define extern /* nothing */
# undef inline
# define inline /* nothing */

/* debugging stuff comes from vstr-internal */

# include "vstr-inline.h"
# include "vstr-nx-inline.h"
#include "internal_syms_generated/vstr-cpp-symbols_rev.h"
VSTR__SYM_ALIAS(add_buf);
VSTR__SYM_ALIAS(add_cstr_buf);
VSTR__SYM_ALIAS(add_cstr_ptr);
VSTR__SYM_ALIAS(add_cstr_ref);
VSTR__SYM_ALIAS(add_rep_chr);
VSTR__SYM_ALIAS(base__pos);
VSTR__SYM_ALIAS(cache_get);
VSTR__SYM_ALIAS(cache__pos);
VSTR__SYM_ALIAS(cmp_bod);
VSTR__SYM_ALIAS(cmp_bod_buf);
VSTR__SYM_ALIAS(cmp_bod_buf_eq);
VSTR__SYM_ALIAS(cmp_bod_cstr);
VSTR__SYM_ALIAS(cmp_bod_cstr_eq);
VSTR__SYM_ALIAS(cmp_bod_eq);
VSTR__SYM_ALIAS(cmp_buf_eq);
VSTR__SYM_ALIAS(cmp_case_bod);
VSTR__SYM_ALIAS(cmp_case_bod_buf);
VSTR__SYM_ALIAS(cmp_case_bod_buf_eq);
VSTR__SYM_ALIAS(cmp_case_bod_cstr);
VSTR__SYM_ALIAS(cmp_case_bod_cstr_eq);
VSTR__SYM_ALIAS(cmp_case_bod_eq);
VSTR__SYM_ALIAS(cmp_case_buf_eq);
VSTR__SYM_ALIAS(cmp_case_cstr);
VSTR__SYM_ALIAS(cmp_case_cstr_eq);
VSTR__SYM_ALIAS(cmp_case_eod);
VSTR__SYM_ALIAS(cmp_case_eod_buf);
VSTR__SYM_ALIAS(cmp_case_eod_buf_eq);
VSTR__SYM_ALIAS(cmp_case_eod_cstr);
VSTR__SYM_ALIAS(cmp_case_eod_cstr_eq);
VSTR__SYM_ALIAS(cmp_case_eod_eq);
VSTR__SYM_ALIAS(cmp_case_eq);
VSTR__SYM_ALIAS(cmp_cstr);
VSTR__SYM_ALIAS(cmp_cstr_eq);
VSTR__SYM_ALIAS(cmp_eod);
VSTR__SYM_ALIAS(cmp_eod_buf);
VSTR__SYM_ALIAS(cmp_eod_buf_eq);
VSTR__SYM_ALIAS(cmp_eod_cstr);
VSTR__SYM_ALIAS(cmp_eod_cstr_eq);
VSTR__SYM_ALIAS(cmp_eod_eq);
VSTR__SYM_ALIAS(cmp_eq);
VSTR__SYM_ALIAS(cmp_fast);
VSTR__SYM_ALIAS(cmp_fast_buf);
VSTR__SYM_ALIAS(cmp_fast_cstr);
VSTR__SYM_ALIAS(cmp_vers_bod);
VSTR__SYM_ALIAS(cmp_vers_bod_buf);
VSTR__SYM_ALIAS(cmp_vers_bod_buf_eq);
VSTR__SYM_ALIAS(cmp_vers_bod_cstr);
VSTR__SYM_ALIAS(cmp_vers_bod_cstr_eq);
VSTR__SYM_ALIAS(cmp_vers_bod_eq);
VSTR__SYM_ALIAS(cmp_vers_buf_eq);
VSTR__SYM_ALIAS(cmp_vers_cstr);
VSTR__SYM_ALIAS(cmp_vers_cstr_eq);
VSTR__SYM_ALIAS(cmp_vers_eod);
VSTR__SYM_ALIAS(cmp_vers_eod_buf);
VSTR__SYM_ALIAS(cmp_vers_eod_buf_eq);
VSTR__SYM_ALIAS(cmp_vers_eod_cstr);
VSTR__SYM_ALIAS(cmp_vers_eod_cstr_eq);
VSTR__SYM_ALIAS(cmp_vers_eod_eq);
VSTR__SYM_ALIAS(cmp_vers_eq);
VSTR__SYM_ALIAS(cspn_cstr_chrs_fwd);
VSTR__SYM_ALIAS(cspn_cstr_chrs_rev);
VSTR__SYM_ALIAS(csrch_cstr_chrs_fwd);
VSTR__SYM_ALIAS(csrch_cstr_chrs_rev);
VSTR__SYM_ALIAS(data_get);
VSTR__SYM_ALIAS(data_set);
VSTR__SYM_ALIAS(del);
VSTR__SYM_ALIAS(dup_cstr_buf);
VSTR__SYM_ALIAS(dup_cstr_ptr);
VSTR__SYM_ALIAS(dup_cstr_ref);
VSTR__SYM_ALIAS(export_chr);
VSTR__SYM_ALIAS(export__node_ptr);
VSTR__SYM_ALIAS(iter_fwd_beg);
VSTR__SYM_ALIAS(iter_fwd_buf);
VSTR__SYM_ALIAS(iter_fwd_chr);
VSTR__SYM_ALIAS(iter_fwd_cstr);
VSTR__SYM_ALIAS(iter_fwd_nxt);
VSTR__SYM_ALIAS(iter_len);
VSTR__SYM_ALIAS(iter_pos);
VSTR__SYM_ALIAS(num);
VSTR__SYM_ALIAS(ref_add);
VSTR__SYM_ALIAS(ref_del);
VSTR__SYM_ALIAS(ref_make_strdup);
VSTR__SYM_ALIAS(sc_add_b_uint16);
VSTR__SYM_ALIAS(sc_add_b_uint32);
VSTR__SYM_ALIAS(sc_add_cstr_grpbasenum_buf);
VSTR__SYM_ALIAS(sc_add_cstr_grpbasenum_ptr);
VSTR__SYM_ALIAS(sc_add_cstr_grpbasenum_ref);
VSTR__SYM_ALIAS(sc_add_cstr_grpnum_buf);
VSTR__SYM_ALIAS(sc_add_grpnum_buf);
VSTR__SYM_ALIAS(sc_bmap_init_eq_spn_buf);
VSTR__SYM_ALIAS(sc_bmap_init_eq_spn_cstr);
VSTR__SYM_ALIAS(sc_bmap_init_or_spn_buf);
VSTR__SYM_ALIAS(sc_bmap_init_or_spn_cstr);
VSTR__SYM_ALIAS(sc_parse_b_uint16);
VSTR__SYM_ALIAS(sc_parse_b_uint32);
VSTR__SYM_ALIAS(sc_posdiff);
VSTR__SYM_ALIAS(sc_poslast);
VSTR__SYM_ALIAS(sc_reduce);
VSTR__SYM_ALIAS(sc_sub_b_uint16);
VSTR__SYM_ALIAS(sc_sub_b_uint32);
VSTR__SYM_ALIAS(sects_add);
VSTR__SYM_ALIAS(split_cstr_buf);
VSTR__SYM_ALIAS(split_cstr_chrs);
VSTR__SYM_ALIAS(spn_cstr_chrs_fwd);
VSTR__SYM_ALIAS(spn_cstr_chrs_rev);
VSTR__SYM_ALIAS(srch_case_cstr_buf_fwd);
VSTR__SYM_ALIAS(srch_case_cstr_buf_rev);
VSTR__SYM_ALIAS(srch_cstr_buf_fwd);
VSTR__SYM_ALIAS(srch_cstr_buf_rev);
VSTR__SYM_ALIAS(srch_cstr_chrs_fwd);
VSTR__SYM_ALIAS(srch_cstr_chrs_rev);
VSTR__SYM_ALIAS(sub_cstr_buf);
VSTR__SYM_ALIAS(sub_cstr_ptr);
VSTR__SYM_ALIAS(sub_cstr_ref);
