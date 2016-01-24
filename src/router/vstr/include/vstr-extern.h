
#ifndef VSTR__HEADER_H
# error " You must _just_ #include <vstr.h>"
#endif
/*
 *  Copyright (C) 1999, 2000, 2001, 2002, 2003, 2004, 2005  James Antill
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

/* == macro functions == */
#define VSTR_SECTS_INIT(sects, p_sz, p_ptr, p_free_ptr) do { \
  (sects)->num = 0; \
  (sects)->sz = (p_sz); \
 \
  (sects)->malloc_bad = 0; \
  (sects)->free_ptr =   !!(p_free_ptr); \
  (sects)->can_add_sz = !!(p_free_ptr); \
  (sects)->can_del_sz = 0; \
 \
  (sects)->alloc_double = 1; \
 \
 \
  (sects)->ptr = (p_ptr); \
 } while (0)

#define VSTR__SECTS_DECL_INIT(p_sz) { 0, (p_sz), \
 0, 0, 0, 0, \
 \
 0, 0, 0, 0, \
 0, 0, 0, 0, \
 0, 0, 0, 0, \
 0, 0, 0, 0, \
 0, 0, 0, 0, \
 0, 0, 0, 0, \
 0, 0, 0, 0, \
 (Vstr_sect_node *)0}

/* This is more disgusting than it should be because C89 doesn't say you
 * can init part of a struct to another variable.
 *
 * It basically does the same thing as...
 *
 * struct Vstr_sects name[1] = VSTR__SECTS_DECL_INIT(sz);
 * struct Vstr_sect_node name[sz];
 *
 * ...asssuming that C allowed that, with the meaning of tack the second
 * declaration on the end of the first.
 *
 */
/* The allocation is calculated by working out the of the difference in structs
 * and then working out, from that, how many Vstr_sects nodes are wasted.
 * This wasted value is then taken from the number passed, to come up with how
 * many should be used.
 * NOTE: the wasted number is rounded down, so we waste at most
 * sizeof(Vstr_sects) bytes.
 */
#define VSTR_SECTS_EXTERN_DECL(name, sz) \
 struct Vstr_sects name[1 + \
   (((unsigned int) (sz)) - (((sizeof(struct Vstr_sects) - \
              sizeof(struct Vstr_sect_node)) * ((unsigned int) (sz))) / \
            sizeof(struct Vstr_sects)))]

#define VSTR_SECTS_DECL(name, sz) \
 VSTR_SECTS_EXTERN_DECL(name, sz) = {VSTR__SECTS_DECL_INIT(sz)}

#define VSTR_SECTS_DECL_INIT(sects) \
 ((void)((sects)->ptr = (sects)->integrated_objs))

#define VSTR_SECTS_NUM(sects, pos) (&((sects)->ptr[((unsigned int) (pos)) - 1]))

#define VSTR_FMT_CB_ARG_PTR(spec, num) \
 (       (spec)->data_ptr[(size_t) (num)])
#define VSTR_FMT_CB_ARG_VAL(spec, T, num) \
 (*(T *)((spec)->data_ptr[(size_t) (num)]))

#define VSTR_SC_FMT_ADD(conf, func, pb, nchk, pe)              \
    (func (conf, pb                  pe) &&                    \
     func (conf, pb ":%"        nchk pe) &&                    \
     func (conf, pb ":%" "*"    nchk pe) &&                    \
     func (conf, pb ":%"  ".*"  nchk pe) &&                    \
     func (conf, pb ":%" "*.*"  nchk pe) &&                    \
     func (conf, pb ":%"  "d%"  nchk pe) &&                    \
     func (conf, pb ":%" "d%d%" nchk pe))

#if VSTR_COMPILE_MACRO_FUNCTIONS
/* these are also inline functions... */
#define VSTR_SC_POSDIFF(bp, ep) (((ep) - (bp)) + 1)
#define VSTR_SC_POSLAST(bp, l)  ( (bp) + ((l)  - 1))
#define VSTR_CMP_EQ(x, p1, l1, y, p2, l2) (((l1) == (l2)) && \
 !vstr_cmp(x, p1, l1, y, p2, l1))
#define VSTR_CMP_BUF_EQ(x, p1, l1, y, l2) (((l1) == (l2)) && \
 !vstr_cmp_buf(x, p1, l1, y, l1))
#define VSTR_CMP_CSTR(x, p1, l1, y) \
  vstr_cmp_buf(x, p1, l1, y, strlen(y))
#define VSTR_CMP_CSTR_EQ(x, p1, l1, y) (((l1) == strlen(y)) && \
 !vstr_cmp_buf(x, p1, l1, y, l1))
#define VSTR_CMP_CASE_EQ(x, p1, l1, y, p2, l2) (((l1) == (l2)) && \
 !vstr_cmp_case(x, p1, l1, y, p2, l1))
#define VSTR_CMP_CASE_BUF_EQ(x, p1, l1, y, l2) (((l1) == (l2)) && \
 !vstr_cmp_case_buf(x, p1, l1, y, l1))
#define VSTR_CMP_CASE_CSTR(x, p1, l1, y) \
  vstr_cmp_case_buf(x, p1, l1, y, strlen(y))
#define VSTR_CMP_CASE_CSTR_EQ(x, p1, l1, y) (((l1) == strlen(y)) && \
 !vstr_cmp_case_buf(x, p1, l1, y, l1))
#define VSTR_CMP_VERS_EQ(x, p1, l1, y, p2, l2) (((l1) == (l2)) && \
 !vstr_cmp(x, p1, l1, y, p2, l1))
#define VSTR_CMP_VERS_BUF_EQ(x, p1, l1, y, l2) (((l1) == (l2)) && \
 !vstr_cmp_buf(x, p1, l1, y, l1))
#define VSTR_CMP_VERS_CSTR(x, p1, l1, y) \
  vstr_cmp_vers_buf(x, p1, l1, y, strlen(y))
#define VSTR_CMP_VERS_CSTR_EQ(x, p1, l1, y) (((l1) == strlen(y)) && \
 !vstr_cmp_vers_buf(x, p1, l1, y, l1))


#define VSTR_ADD_CSTR_BUF(x, y, buf) vstr_add_buf(x, y, buf, strlen(buf))
#define VSTR_ADD_CSTR_PTR(x, y, ptr) vstr_add_ptr(x, y, ptr, strlen(ptr))
#define VSTR_ADD_CSTR_REF(x, y, ref, off) \
 vstr_add_ref(x, y, ref, off, strlen(((const char *)(ref)->ptr) + (off)))


#define VSTR_DUP_CSTR_BUF(x, buf) vstr_dup_buf(x, buf, strlen(buf))
#define VSTR_DUP_CSTR_PTR(x, ptr) vstr_dup_ptr(x, ptr, strlen(ptr))
#define VSTR_DUP_CSTR_REF(x, ref, off) \
 vstr_dup_ref(x, ref, off, strlen(((const char *)(ref)->ptr) + (off)))


#define VSTR_SUB_CSTR_BUF(x, y, z, buf) \
 vstr_sub_buf(x, y, z, buf, strlen(buf))
#define VSTR_SUB_CSTR_PTR(x, y, z, ptr) \
 vstr_sub_ptr(x, y, z, ptr, strlen(ptr))
#define VSTR_SUB_CSTR_REF(x, y, z, ref, off) \
 vstr_sub_ref(x, y, z, ref, off, strlen(((const char *)(ref)->ptr) + (off)))


#define VSTR_SRCH_CSTR_BUF_FWD(x, y, z, buf) \
 vstr_srch_buf_fwd(x, y, z, buf, strlen(buf))
#define VSTR_SRCH_CSTR_BUF_REV(x, y, z, buf) \
 vstr_srch_buf_rev(x, y, z, buf, strlen(buf))

#define VSTR_SRCH_CSTR_CHRS_FWD(x, y, z, chrs) \
 vstr_srch_chrs_fwd(x, y, z, chrs, strlen(chrs))
#define VSTR_SRCH_CSTR_CHRS_REV(x, y, z, chrs) \
 vstr_srch_chrs_rev(x, y, z, chrs, strlen(chrs))

#define VSTR_CSRCH_CSTR_CHRS_FWD(x, y, z, chrs) \
 vstr_csrch_chrs_fwd(x, y, z, chrs, strlen(chrs))
#define VSTR_CSRCH_CSTR_CHRS_REV(x, y, z, chrs) \
 vstr_csrch_chrs_rev(x, y, z, chrs, strlen(chrs))

#define VSTR_SRCH_CASE_CSTR_BUF_FWD(x, y, z, buf) \
 vstr_srch_case_buf_fwd(x, y, z, buf, strlen(buf))
#define VSTR_SRCH_CASE_CSTR_BUF_REV(x, y, z, buf) \
 vstr_srch_case_buf_rev(x, y, z, buf, strlen(buf))


#define VSTR_SPN_CSTR_CHRS_FWD(x, y, z, chrs) \
 vstr_spn_chrs_fwd(x, y, z, chrs, strlen(chrs))
#define VSTR_SPN_CSTR_CHRS_REV(x, y, z, chrs) \
 vstr_spn_chrs_rev(x, y, z, chrs, strlen(chrs))

#define VSTR_CSPN_CSTR_CHRS_FWD(x, y, z, chrs) \
 vstr_cspn_chrs_fwd(x, y, z, chrs, strlen(chrs))
#define VSTR_CSPN_CSTR_CHRS_REV(x, y, z, chrs) \
 vstr_cspn_chrs_rev(x, y, z, chrs, strlen(chrs))


#define VSTR_SPLIT_CSTR_BUF(x, y, z, buf, sect, lim, flags) \
 vstr_split_buf(x, y, z, buf, strlen(buf), sect, lim, flags)
#define VSTR_SPLIT_CSTR_CHRS(x, y, z, chrs, sect, lim, flags) \
 vstr_split_chrs(x, y, z, chrs, strlen(chrs), sect, lim, flags)

#define VSTR_REF_MAKE_STRDUP(x) vstr_ref_make_memdup((x), strlen(x) + 1)

#define VSTR_SC_ADD_CSTR_GRPBASENUM_BUF(x, y, z, buf) vstr_sc_add_grpbasenum_buf(x, y, z, buf, strlen(buf))
#define VSTR_SC_ADD_CSTR_GRPBASENUM_PTR(x, y, z, ptr) vstr_sc_add_grpbasenum_ptr(x, y, z, ptr, strlen(ptr))
#define VSTR_SC_ADD_CSTR_GRPBASENUM_REF(x, y, z, ref, off) vstr_sc_add_grpbasenum_ref(x, y, z, ref, off, strlen(((char *)(ref)->ptr) + (off)))
#define VSTR_SC_ADD_CSTR_GRPNUM_BUF(x, y, buf) vstr_sc_add_grpnum_buf(x, y, buf, strlen(buf))

#endif

#define VSTR__CACHE(x) ((struct Vstr__base_cache *)(x))->cache

/* == real functions == */

/* not really vectored string functions ... just stuff needed */
extern void vstr_ref_cb_free_nothing(struct Vstr_ref *);
extern void vstr_ref_cb_free_ref(struct Vstr_ref *);
extern void vstr_ref_cb_free_ptr(struct Vstr_ref *);
extern void vstr_ref_cb_free_ptr_ref(struct Vstr_ref *);

extern struct Vstr_ref *vstr_ref_add(struct Vstr_ref *)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern void vstr_ref_del(struct Vstr_ref *);
extern struct Vstr_ref *vstr_ref_make_ptr(const void *,
                                          void (*)(struct Vstr_ref *))
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_MALLOC()
   VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern struct Vstr_ref *vstr_ref_make_malloc(size_t)
    VSTR__COMPILE_ATTR_MALLOC() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern struct Vstr_ref *vstr_ref_make_memdup(const void *, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_MALLOC() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern struct Vstr_ref *vstr_ref_make_strdup(const char *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_MALLOC() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern struct Vstr_ref *vstr_ref_make_vstr_base(struct Vstr_base *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_MALLOC() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern struct Vstr_ref *vstr_ref_make_vstr_conf(struct Vstr_conf *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_MALLOC() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern struct Vstr_ref *vstr_ref_make_vstr_sects(struct Vstr_sects *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_MALLOC() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();


/* real start of vector string functions */

extern int  vstr_init(void) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern void vstr_exit(void);

extern struct Vstr_conf *vstr_make_conf(void)
    VSTR__COMPILE_ATTR_MALLOC() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern void vstr_free_conf(struct Vstr_conf *);
extern int vstr_swap_conf(struct Vstr_base *, struct Vstr_conf **)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();

extern struct Vstr_base *vstr_make_base(struct Vstr_conf *)
    VSTR__COMPILE_ATTR_MALLOC() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern void vstr_free_base(struct Vstr_base *);

extern unsigned int vstr_make_spare_nodes(struct Vstr_conf *,
                                          unsigned int,
                                          unsigned int);
extern unsigned int vstr_free_spare_nodes(struct Vstr_conf *,
                                          unsigned int,
                                          unsigned int);

/* add data functions */
extern int vstr_add_buf(struct Vstr_base *, size_t, const void *, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_add_ptr(struct Vstr_base *, size_t, const void *, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_add_non(struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_add_ref(struct Vstr_base *, size_t,
                        struct Vstr_ref *, size_t, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_add_vstr(struct Vstr_base *, size_t,
                         const struct Vstr_base *, size_t, size_t,
                         unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_add_rep_chr(struct Vstr_base *, size_t, char, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();


extern struct Vstr_base *vstr_dup_buf(struct Vstr_conf *, const void *, size_t)
    VSTR__COMPILE_ATTR_NONNULL_L((2)) VSTR__COMPILE_ATTR_MALLOC() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern struct Vstr_base *vstr_dup_ptr(struct Vstr_conf *, const void *, size_t)
    VSTR__COMPILE_ATTR_NONNULL_L((2)) VSTR__COMPILE_ATTR_MALLOC() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern struct Vstr_base *vstr_dup_non(struct Vstr_conf *, size_t)
    VSTR__COMPILE_ATTR_MALLOC() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern struct Vstr_base *vstr_dup_ref(struct Vstr_conf *,
                                      struct Vstr_ref *ref, size_t, size_t)
    VSTR__COMPILE_ATTR_NONNULL_L((2)) VSTR__COMPILE_ATTR_MALLOC() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern struct Vstr_base *vstr_dup_vstr(struct Vstr_conf *,
                                       const struct Vstr_base *, size_t, size_t,
                                       unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_L((2)) VSTR__COMPILE_ATTR_MALLOC() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern struct Vstr_base *vstr_dup_rep_chr(struct Vstr_conf *, char, size_t)
    VSTR__COMPILE_ATTR_MALLOC() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();

extern size_t vstr_add_vfmt(struct Vstr_base *, size_t, const char *, va_list)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_FMT(3, 0);
extern size_t vstr_add_fmt(struct Vstr_base *, size_t, const char *, ...)
    VSTR__COMPILE_ATTR_NONNULL_L((1, 3)) VSTR__COMPILE_ATTR_FMT(3, 4);
extern size_t vstr_add_vsysfmt(struct Vstr_base *, size_t, const char *,va_list)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_FMT(3, 0);
extern size_t vstr_add_sysfmt(struct Vstr_base *, size_t, const char *, ...)
    VSTR__COMPILE_ATTR_NONNULL_L((1, 3)) VSTR__COMPILE_ATTR_FMT(3, 4);

extern size_t vstr_add_iovec_buf_beg(struct Vstr_base *, size_t,
                                     unsigned int, unsigned int,
                                     struct iovec **,
                                     unsigned int *)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern void vstr_add_iovec_buf_end(struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();

/* NOTE: netstr2 allows leading '0' s, netstr doesn't */
extern size_t vstr_add_netstr_beg(struct Vstr_base *, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_add_netstr_end(struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern size_t vstr_add_netstr2_beg(struct Vstr_base *, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_add_netstr2_end(struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();

extern int vstr_add_cstr_buf(struct Vstr_base *, size_t,
                             const char *)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_add_cstr_ptr(struct Vstr_base *, size_t,
                             const char *)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_add_cstr_ref(struct Vstr_base *, size_t,
                             struct Vstr_ref *, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();

extern struct Vstr_base *vstr_dup_cstr_buf(struct Vstr_conf *,
                                           const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2)) VSTR__COMPILE_ATTR_MALLOC() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern struct Vstr_base *vstr_dup_cstr_ptr(struct Vstr_conf *,
                                           const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2)) VSTR__COMPILE_ATTR_MALLOC() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern struct Vstr_base *vstr_dup_cstr_ref(struct Vstr_conf *,
                                           struct Vstr_ref *, size_t)
    VSTR__COMPILE_ATTR_NONNULL_L((2)) VSTR__COMPILE_ATTR_MALLOC() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();

/* delete functions */
extern int vstr_del(struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();

/* substitution functions */
extern int vstr_sub_buf(struct Vstr_base *, size_t, size_t,
                        const void *, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_sub_ptr(struct Vstr_base *, size_t, size_t,
                        const void *, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_sub_non(struct Vstr_base *, size_t, size_t, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_sub_ref(struct Vstr_base *, size_t, size_t,
                        struct Vstr_ref *, size_t, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_sub_vstr(struct Vstr_base *, size_t, size_t,
                         const struct Vstr_base *, size_t, size_t,
                         unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_sub_rep_chr(struct Vstr_base *, size_t, size_t,
                            char, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();

extern int vstr_sub_cstr_buf(struct Vstr_base *, size_t, size_t, const char *)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_sub_cstr_ptr(struct Vstr_base *, size_t, size_t, const char *)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_sub_cstr_ref(struct Vstr_base *, size_t, size_t,
                             struct Vstr_ref *, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();

/* convertion functions */
extern int vstr_conv_lowercase(struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_conv_uppercase(struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_conv_unprintable_chr(struct Vstr_base *, size_t, size_t,
                                     unsigned int, char)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_conv_unprintable_del(struct Vstr_base *, size_t, size_t,
                                     unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_conv_encode_uri(struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_conv_decode_uri(struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();


/* move functions */
extern int vstr_mov(struct Vstr_base *, size_t,
                    struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();

/* control misc stuff */
extern int vstr_cntl_opt(int, ...);
extern int vstr_cntl_base(struct Vstr_base *, int, ...)
    VSTR__COMPILE_ATTR_NONNULL_L((1));
extern int vstr_cntl_conf(struct Vstr_conf *, int, ...);

/* --------------------------------------------------------------------- */
/* constant base functions below here */
/* --------------------------------------------------------------------- */

/* data about the vstr */
extern unsigned int vstr_num(const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();

/* comparing functions */
extern int vstr_cmp(const struct Vstr_base *, size_t, size_t,
                    const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_buf(const struct Vstr_base *, size_t, size_t,
                        const void *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_case(const struct Vstr_base *, size_t, size_t,
                         const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_case_buf(const struct Vstr_base *, size_t, size_t,
                             const char *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_vers(const struct Vstr_base *, size_t, size_t,
                         const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_vers_buf(const struct Vstr_base *, size_t, size_t,
                             const char *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_eq(const struct Vstr_base *, size_t, size_t,
                       const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_buf_eq(const struct Vstr_base *, size_t, size_t,
                           const void *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_cstr(const struct Vstr_base *, size_t, size_t,
                         const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_cstr_eq(const struct Vstr_base *, size_t, size_t,
                            const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_case_eq(const struct Vstr_base *, size_t, size_t,
                            const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_case_buf_eq(const struct Vstr_base *, size_t, size_t,
                                const char *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_case_cstr(const struct Vstr_base *, size_t, size_t,
                              const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_case_cstr_eq(const struct Vstr_base *,
                                 size_t, size_t, const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_vers_eq(const struct Vstr_base *, size_t, size_t,
                            const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_vers_buf_eq(const struct Vstr_base *, size_t, size_t,
                                const char *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_vers_cstr(const struct Vstr_base *,
                              size_t, size_t, const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_vers_cstr_eq(const struct Vstr_base *,
                                 size_t, size_t, const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();

extern int vstr_cmp_fast(const struct Vstr_base *, size_t, size_t,
                         const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_fast_buf(const struct Vstr_base *, size_t, size_t,
                             const char *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_fast_cstr(const struct Vstr_base *, size_t, size_t,
                              const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();

extern int vstr_cmp_bod(const struct Vstr_base *, size_t, size_t,
                        const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_eod(const struct Vstr_base *, size_t, size_t,
                        const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_bod_eq(const struct Vstr_base *, size_t, size_t,
                           const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_eod_eq(const struct Vstr_base *, size_t, size_t,
                           const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_bod_buf(const struct Vstr_base *, size_t, size_t,
                            const void *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_eod_buf(const struct Vstr_base *, size_t, size_t,
                            const void *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_bod_buf_eq(const struct Vstr_base *, size_t, size_t,
                               const void *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_eod_buf_eq(const struct Vstr_base *, size_t, size_t,
                               const void *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_bod_cstr(const struct Vstr_base *, size_t, size_t,
                             const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_eod_cstr(const struct Vstr_base *, size_t, size_t,
                             const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_bod_cstr_eq(const struct Vstr_base *, size_t, size_t,
                                const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_eod_cstr_eq(const struct Vstr_base *, size_t, size_t,
                                const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_case_bod(const struct Vstr_base *, size_t, size_t,
                             const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_case_eod(const struct Vstr_base *, size_t, size_t,
                             const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_case_bod_eq(const struct Vstr_base *, size_t, size_t,
                                const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_case_eod_eq(const struct Vstr_base *, size_t, size_t,
                                  const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_case_bod_buf(const struct Vstr_base *, size_t, size_t,
                                 const char *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_case_eod_buf(const struct Vstr_base *, size_t, size_t,
                                 const char *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_case_bod_buf_eq(const struct Vstr_base *, size_t, size_t,
                                    const char *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_case_eod_buf_eq(const struct Vstr_base *, size_t, size_t,
                                    const char *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_case_bod_cstr(const struct Vstr_base *, size_t, size_t,
                                  const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_case_eod_cstr(const struct Vstr_base *, size_t, size_t,
                                  const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_case_bod_cstr_eq(const struct Vstr_base *, size_t, size_t,
                                     const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_case_eod_cstr_eq(const struct Vstr_base *, size_t, size_t,
                                     const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_vers_bod(const struct Vstr_base *, size_t, size_t,
                             const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_vers_eod(const struct Vstr_base *, size_t, size_t,
                             const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_vers_bod_eq(const struct Vstr_base *, size_t, size_t,
                                const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_vers_eod_eq(const struct Vstr_base *, size_t, size_t,
                                const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_vers_bod_buf(const struct Vstr_base *, size_t, size_t,
                                 const char *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_vers_eod_buf(const struct Vstr_base *, size_t, size_t,
                                 const char *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_vers_bod_buf_eq(const struct Vstr_base *, size_t, size_t,
                                    const char *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_vers_eod_buf_eq(const struct Vstr_base *, size_t, size_t,
                                    const char *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_vers_bod_cstr(const struct Vstr_base *, size_t, size_t,
                                  const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_vers_eod_cstr(const struct Vstr_base *, size_t, size_t,
                                  const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_vers_bod_cstr_eq(const struct Vstr_base *, size_t, size_t,
                                     const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_cmp_vers_eod_cstr_eq(const struct Vstr_base *, size_t, size_t,
                                     const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();



/* search functions */
extern size_t vstr_srch_chr_fwd(const struct Vstr_base *, size_t, size_t, char)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_srch_chr_rev(const struct Vstr_base *, size_t, size_t, char)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();

extern size_t vstr_srch_chrs_fwd(const struct Vstr_base *, size_t, size_t,
                                 const char *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_srch_chrs_rev(const struct Vstr_base *, size_t, size_t,
                                 const char *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_csrch_chrs_fwd(const struct Vstr_base *, size_t, size_t,
                                  const char *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_csrch_chrs_rev(const struct Vstr_base *, size_t, size_t,
                                  const char *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();

extern size_t vstr_srch_buf_fwd(const struct Vstr_base *, size_t, size_t,
                                const void *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_srch_buf_rev(const struct Vstr_base *, size_t, size_t,
                                const void *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();

extern size_t vstr_srch_vstr_fwd(const struct Vstr_base *, size_t, size_t,
                                 const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_srch_vstr_rev(const struct Vstr_base *, size_t, size_t,
                                 const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();

extern size_t vstr_srch_case_chr_fwd(const struct Vstr_base *, size_t, size_t,
                                     char)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_srch_case_chr_rev(const struct Vstr_base *, size_t, size_t,
                                     char)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();

extern size_t vstr_srch_case_buf_fwd(const struct Vstr_base *, size_t, size_t,
                                     const void *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_srch_case_buf_rev(const struct Vstr_base *, size_t, size_t,
                                     const void *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();

extern size_t vstr_srch_case_vstr_fwd(const struct Vstr_base *, size_t, size_t,
                                      const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_srch_case_vstr_rev(const struct Vstr_base *, size_t, size_t,
                                      const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();

extern size_t vstr_srch_cstr_buf_fwd(const struct Vstr_base *, size_t, size_t,
                                     const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_srch_cstr_buf_rev(const struct Vstr_base *, size_t, size_t,
                                     const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_srch_cstr_chrs_fwd(const struct Vstr_base *, size_t, size_t,
                                      const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_srch_cstr_chrs_rev(const struct Vstr_base *, size_t, size_t,
                                      const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_csrch_cstr_chrs_fwd(const struct Vstr_base *, size_t, size_t,
                                       const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_csrch_cstr_chrs_rev(const struct Vstr_base *, size_t, size_t,
                                       const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_srch_case_cstr_buf_fwd(const struct Vstr_base *,
                                          size_t, size_t, const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_srch_case_cstr_buf_rev(const struct Vstr_base *,
                                          size_t, size_t, const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();

/* spanning and compliment spanning */
extern size_t vstr_spn_chrs_fwd(const struct Vstr_base *, size_t, size_t,
                                const char *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_spn_chrs_rev(const struct Vstr_base *, size_t, size_t,
                                const char *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_cspn_chrs_fwd(const struct Vstr_base *, size_t, size_t,
                                 const char *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_cspn_chrs_rev(const struct Vstr_base *, size_t, size_t,
                                 const char *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();

extern size_t vstr_spn_cstr_chrs_fwd(const struct Vstr_base *, size_t, size_t,
                                     const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_spn_cstr_chrs_rev(const struct Vstr_base *, size_t, size_t,
                                     const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_cspn_cstr_chrs_fwd(const struct Vstr_base *, size_t, size_t,
                                      const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_cspn_cstr_chrs_rev(const struct Vstr_base *, size_t, size_t,
                                      const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_spn_bmap_eq_fwd(const struct Vstr_base *, size_t, size_t,
                                   const unsigned char[VSTR__COMPILE_STATIC_ARRAY() 256], unsigned char)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_spn_bmap_eq_rev(const struct Vstr_base *, size_t, size_t,
                                   const unsigned char[VSTR__COMPILE_STATIC_ARRAY() 256], unsigned char)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_spn_bmap_and_fwd(const struct Vstr_base *, size_t, size_t,
                                   const unsigned char[VSTR__COMPILE_STATIC_ARRAY() 256], unsigned char)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_spn_bmap_and_rev(const struct Vstr_base *, size_t, size_t,
                                   const unsigned char[VSTR__COMPILE_STATIC_ARRAY() 256], unsigned char)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_cspn_bmap_eq_fwd(const struct Vstr_base *, size_t, size_t,
                                    const unsigned char[VSTR__COMPILE_STATIC_ARRAY() 256], unsigned char)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_cspn_bmap_eq_rev(const struct Vstr_base *, size_t, size_t,
                                    const unsigned char[VSTR__COMPILE_STATIC_ARRAY() 256], unsigned char)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_cspn_bmap_and_fwd(const struct Vstr_base *, size_t, size_t,
                                    const unsigned char[VSTR__COMPILE_STATIC_ARRAY() 256], unsigned char)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_cspn_bmap_and_rev(const struct Vstr_base *, size_t, size_t,
                                    const unsigned char[VSTR__COMPILE_STATIC_ARRAY() 256], unsigned char)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();


/* export functions */
extern size_t vstr_export_iovec_ptr_all(const struct Vstr_base *,
                                        struct iovec **,
                                        unsigned int *)
    VSTR__COMPILE_ATTR_NONNULL_L((1));
extern size_t vstr_export_iovec_cpy_buf(const struct Vstr_base *,
                                        size_t, size_t,
                                        struct iovec *, unsigned int,
                                        unsigned int *)
    VSTR__COMPILE_ATTR_NONNULL_L((1, 4));
extern size_t vstr_export_iovec_cpy_ptr(const struct Vstr_base *,
                                        size_t, size_t,
                                        struct iovec *, unsigned int,
                                        unsigned int *)
    VSTR__COMPILE_ATTR_NONNULL_L((1, 4));
extern size_t vstr_export_buf(const struct Vstr_base *, size_t, size_t,
                              void *, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern char vstr_export_chr(const struct Vstr_base *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A();

extern struct Vstr_ref *vstr_export_ref(const struct Vstr_base *,
                                        size_t, size_t, size_t *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();

extern const char *vstr_export_cstr_ptr(const struct Vstr_base *,
                                        size_t, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern char *vstr_export_cstr_malloc(const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_MALLOC() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_export_cstr_buf(const struct Vstr_base *, size_t, size_t,
                                   void *, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern struct Vstr_ref *vstr_export_cstr_ref(const struct Vstr_base *,
                                             size_t, size_t, size_t *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();

/* functions for parsing data out of a vstr */
extern void *vstr_parse_num(const struct Vstr_base *, size_t, size_t,
                            unsigned int, size_t *,
                            unsigned int *,
                            void *(*)(unsigned int, int,
                                      unsigned int *, void *),
                            void *)
   /* don't specify warn_unused_ret as they may not use it that way */
   VSTR__COMPILE_ATTR_NONNULL_L((1, 7, 8));

extern short vstr_parse_short(const struct Vstr_base *,
                              size_t, size_t, unsigned int, size_t *,
                              unsigned int *)
    VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern unsigned short vstr_parse_ushort(const struct Vstr_base *,
                                        size_t, size_t,
                                        unsigned int, size_t *,
                                        unsigned int *)
    VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_parse_int(const struct Vstr_base *,
                          size_t, size_t, unsigned int, size_t *,
                          unsigned int *)
    VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern unsigned int vstr_parse_uint(const struct Vstr_base *,
                                    size_t, size_t, unsigned int, size_t *,
                                    unsigned int *)
    VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern long vstr_parse_long(const struct Vstr_base *,
                            size_t, size_t, unsigned int, size_t *,
                            unsigned int *)
    VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern unsigned long vstr_parse_ulong(const struct Vstr_base *,
                                      size_t, size_t, unsigned int, size_t *,
                                      unsigned int *)
    VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern VSTR_AUTOCONF_intmax_t vstr_parse_intmax(const struct Vstr_base *,
                                                size_t, size_t, unsigned int,
                                                size_t *, unsigned int *)
    VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern VSTR_AUTOCONF_uintmax_t vstr_parse_uintmax(const struct Vstr_base *,
                                                  size_t, size_t,
                                                  unsigned int, size_t *,
                                                  unsigned int *)
    VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();

extern int vstr_parse_ipv4(const struct Vstr_base *,
                           size_t, size_t,
                           unsigned char *, unsigned int *, unsigned int,
                           size_t *, unsigned int *)
    VSTR__COMPILE_ATTR_NONNULL_L((1, 4));
extern int vstr_parse_ipv6(const struct Vstr_base *,
                           size_t, size_t,
                           unsigned int *, unsigned int *, unsigned int,
                           size_t *, unsigned int *)
    VSTR__COMPILE_ATTR_NONNULL_L((1, 4));

extern size_t vstr_parse_netstr(const struct Vstr_base *, size_t, size_t,
                                size_t *, size_t *)
    VSTR__COMPILE_ATTR_NONNULL_L((1));
extern size_t vstr_parse_netstr2(const struct Vstr_base *, size_t, size_t,
                                 size_t *, size_t *)
    VSTR__COMPILE_ATTR_NONNULL_L((1));

/* section functions -- seperate namespace */
extern struct Vstr_sects *vstr_sects_make(unsigned int)
    VSTR__COMPILE_ATTR_MALLOC() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern void vstr_sects_free(struct Vstr_sects *);

extern int vstr_sects_add(struct Vstr_sects *, size_t, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_sects_del(struct Vstr_sects *, unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A();

extern unsigned int vstr_sects_srch(struct Vstr_sects *, size_t, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();

extern int vstr_sects_foreach(const struct Vstr_base *, struct Vstr_sects *,
                              unsigned int,
                              unsigned int (*) (const struct Vstr_base *,
                                                size_t, size_t, void *),
                              void *)
    VSTR__COMPILE_ATTR_NONNULL_L((1, 2, 4));

extern int vstr_sects_update_add(const struct Vstr_base *,
                                 struct Vstr_sects *)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_sects_update_del(const struct Vstr_base *,
                                 struct Vstr_sects *)
    VSTR__COMPILE_ATTR_NONNULL_L((1));


/* split functions */
extern unsigned int vstr_split_buf(const struct Vstr_base *, size_t, size_t,
                                   const void *, size_t, struct Vstr_sects *,
                                   unsigned int, unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_L((1, 6));
extern unsigned int vstr_split_chrs(const struct Vstr_base *, size_t, size_t,
                                    const char *, size_t, struct Vstr_sects *,
                                    unsigned int, unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_L((1, 6));

extern size_t vstr_split_cstr_buf(const struct Vstr_base *, size_t, size_t,
                                  const char *, struct Vstr_sects *,
                                  unsigned int, unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern size_t vstr_split_cstr_chrs(const struct Vstr_base *, size_t, size_t,
                                   const char *, struct Vstr_sects *sect,
                                   unsigned int, unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A();


/* cache functions */
extern unsigned int vstr_cache_add(struct Vstr_conf *, const char *,
                                   void *(*)(const struct Vstr_base *,
                                             size_t, size_t,
                                             unsigned int, void *))
    VSTR__COMPILE_ATTR_NONNULL_L((2, 3));
extern unsigned int vstr_cache_srch(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern void *vstr_cache_get(const struct Vstr_base *, unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_cache_set(const struct Vstr_base *, unsigned int, void *)
    VSTR__COMPILE_ATTR_NONNULL_L((1));

extern void vstr_cache_cb_sub(const struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern void vstr_cache_cb_free(const struct Vstr_base *, unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A();


/* configuration data functions... */
extern unsigned int vstr_data_add(struct Vstr_conf *,
                                  const char *, struct Vstr_ref *)
   VSTR__COMPILE_ATTR_NONNULL_L((2)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern unsigned int vstr_data_srch(struct Vstr_conf *, const char *)
   VSTR__COMPILE_ATTR_NONNULL_L((2)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern void vstr_data_del(struct Vstr_conf *, unsigned int);
extern void *vstr_data_get(struct Vstr_conf *, unsigned int)
   VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern void  vstr_data_set(struct Vstr_conf *, unsigned int, struct Vstr_ref *);


/* custom format specifier registration functions */
extern int vstr_fmt_add(struct Vstr_conf *, const char *,
                        int (*)(struct Vstr_base *, size_t,
                                struct Vstr_fmt_spec *), ...)
    VSTR__COMPILE_ATTR_NONNULL_L((2, 3));
extern void vstr_fmt_del(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_fmt_srch(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();

/* scan the vstr in iteration functions */

extern int vstr_iter_fwd_beg(const struct Vstr_base *, size_t, size_t,
                             struct Vstr_iter *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_iter_fwd_nxt(struct Vstr_iter *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern char vstr_iter_fwd_chr(struct Vstr_iter *, unsigned int *)
   VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_iter_fwd_buf(struct Vstr_iter *, size_t,
                                void *, size_t, unsigned int *)
   VSTR__COMPILE_ATTR_NONNULL_L((1));
extern size_t vstr_iter_fwd_cstr(struct Vstr_iter *, size_t,
                                char *, size_t, unsigned int *)
   VSTR__COMPILE_ATTR_NONNULL_L((1, 3));
extern size_t vstr_iter_pos(struct Vstr_iter *, size_t, size_t)
   VSTR__COMPILE_ATTR_PURE()
   VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_iter_len(struct Vstr_iter *)
   VSTR__COMPILE_ATTR_PURE()
   VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();


/* shortcut functions ... */
extern size_t vstr_sc_posdiff(size_t, size_t)
    VSTR__COMPILE_ATTR_CONST() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern size_t vstr_sc_poslast(size_t, size_t)
    VSTR__COMPILE_ATTR_CONST() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern int vstr_sc_reduce(struct Vstr_base *, size_t, size_t, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();

extern int vstr_sc_fmt_cb_beg(struct Vstr_base *, size_t *,
                              struct Vstr_fmt_spec *, size_t *, unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_sc_fmt_cb_end(struct Vstr_base *, size_t,
                              struct Vstr_fmt_spec *, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_sc_fmt_add_vstr(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_buf(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_ptr(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_non(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_ref(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_rep_chr(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_bkmg_Byte_uint(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_bkmg_bit_uint(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_bkmg_Bytes_uint(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_bkmg_bits_uint(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_bkmg_Byte_uintmax(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_bkmg_bit_uintmax(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_bkmg_Bytes_uintmax(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_bkmg_bits_uintmax(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_ipv4_vec(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_ipv6_vec(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_ipv4_vec_cidr(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_ipv6_vec_cidr(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));

extern int vstr_sc_fmt_add_upper_base2_uint(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_upper_base2_ulong(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_upper_base2_size(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_upper_base2_uintmax(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_lower_base2_uint(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_lower_base2_ulong(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_lower_base2_size(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_lower_base2_uintmax(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));

extern int vstr_sc_fmt_add_all(struct Vstr_conf *);

extern int vstr_sc_fmt_add_ipv4_ptr(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));
extern int vstr_sc_fmt_add_ipv6_ptr(struct Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_L((2));

extern int vstr_sc_read_iov_fd(struct Vstr_base *, size_t, int,
                               unsigned int, unsigned int,
                               unsigned int *)
    VSTR__COMPILE_ATTR_NONNULL_L((1));
extern int vstr_sc_read_len_fd(struct Vstr_base *, size_t, int,
                               size_t, unsigned int *)
    VSTR__COMPILE_ATTR_NONNULL_L((1));
extern int vstr_sc_write_fd(struct Vstr_base *, size_t, size_t, int,
                            unsigned int *)
    VSTR__COMPILE_ATTR_NONNULL_L((1));

#ifndef VSTR__COMPILE_SKIP_BROKEN_LFS
extern int vstr_sc_mmap_fd(struct Vstr_base *, size_t, int,
                           VSTR_AUTOCONF_off64_t, size_t, unsigned int *)
    VSTR__COMPILE_ATTR_NONNULL_L((1));
extern int vstr_sc_mmap_file(struct Vstr_base *, size_t,
                             const char *, VSTR_AUTOCONF_off64_t, size_t,
                             unsigned int *)
    VSTR__COMPILE_ATTR_NONNULL_L((1, 3));
extern int vstr_sc_read_iov_file(struct Vstr_base *, size_t,
           	                 const char *, VSTR_AUTOCONF_off64_t,
                                 unsigned int, unsigned int,
                                 unsigned int *)
    VSTR__COMPILE_ATTR_NONNULL_L((1, 3));
extern int vstr_sc_read_len_file(struct Vstr_base *, size_t,
           	                 const char *, VSTR_AUTOCONF_off64_t, size_t,
                                 unsigned int *)
    VSTR__COMPILE_ATTR_NONNULL_L((1, 3));
extern int vstr_sc_write_file(struct Vstr_base *, size_t, size_t,
                              const char *, int, VSTR_AUTOCONF_mode_t,
                              VSTR_AUTOCONF_off64_t, unsigned int *)
    VSTR__COMPILE_ATTR_NONNULL_L((1, 4));
#endif

extern void vstr_sc_basename(const struct Vstr_base *, size_t, size_t,
                             size_t *, size_t *)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern void vstr_sc_dirname(const struct Vstr_base *, size_t, size_t,
                            size_t *)
    VSTR__COMPILE_ATTR_NONNULL_A();

extern int vstr_sc_add_grpbasenum_buf(struct Vstr_base *, size_t, unsigned int,
                                      const void *, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_sc_add_cstr_grpbasenum_buf(struct Vstr_base *, size_t,
                                           unsigned int, const char *)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_sc_add_grpbasenum_ptr(struct Vstr_base *, size_t, unsigned int,
                                      const void *, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_sc_add_cstr_grpbasenum_ptr(struct Vstr_base *, size_t,
                                           unsigned int, const char *)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_sc_add_grpbasenum_ref(struct Vstr_base *, size_t, unsigned int,
                                      struct Vstr_ref *, size_t, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_sc_add_cstr_grpbasenum_ref(struct Vstr_base *, size_t,
                                           unsigned int,
                                           struct Vstr_ref *, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_sc_add_grpnum_buf(struct Vstr_base *, size_t,
                                  const void *, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_sc_add_cstr_grpnum_buf(struct Vstr_base *, size_t,
                                       const char *)
    VSTR__COMPILE_ATTR_NONNULL_A();

/* these should be somewhere else ... but where ?
 * needed for number based custom formatters */
extern size_t vstr_sc_conv_num_uint(char *, size_t, unsigned int,
                                    const char *, unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern size_t vstr_sc_conv_num10_uint(char *, size_t, unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern size_t vstr_sc_conv_num_ulong(char *, size_t, unsigned long,
                                     const char *, unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern size_t vstr_sc_conv_num10_ulong(char *, size_t, unsigned long)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern size_t vstr_sc_conv_num_size(char *, size_t, size_t,
                                    const char *, unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern size_t vstr_sc_conv_num10_size(char *, size_t, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern size_t vstr_sc_conv_num_uintmax(char *, size_t,
                                       VSTR_AUTOCONF_uintmax_t,
                                       const char *, unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern size_t vstr_sc_conv_num10_uintmax(char *, size_t,
                                         VSTR_AUTOCONF_uintmax_t)
    VSTR__COMPILE_ATTR_NONNULL_A();

extern int vstr_sc_add_b_uint16(struct Vstr_base *, size_t,
                                VSTR_AUTOCONF_uint_least16_t)
   VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_sc_add_b_uint32(struct Vstr_base *, size_t,
                                VSTR_AUTOCONF_uint_least32_t)
   VSTR__COMPILE_ATTR_NONNULL_A();

extern int vstr_sc_sub_b_uint16(struct Vstr_base *, size_t, size_t,
                                VSTR_AUTOCONF_uint_least16_t)
   VSTR__COMPILE_ATTR_NONNULL_A();
extern int vstr_sc_sub_b_uint32(struct Vstr_base *, size_t, size_t,
                                VSTR_AUTOCONF_uint_least32_t)
   VSTR__COMPILE_ATTR_NONNULL_A();

extern VSTR_AUTOCONF_uint_least16_t vstr_sc_parse_b_uint16(struct Vstr_base *,
                                                           size_t)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
extern VSTR_AUTOCONF_uint_least32_t vstr_sc_parse_b_uint32(struct Vstr_base *,
                                                           size_t)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();

extern void vstr_sc_bmap_init_eq_spn_buf(unsigned char [VSTR__COMPILE_STATIC_ARRAY() 256], 
                                         const void *, size_t,
                                         unsigned char)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern void vstr_sc_bmap_init_or_spn_buf(unsigned char [VSTR__COMPILE_STATIC_ARRAY() 256], 
                                         const void *, size_t,
                                         unsigned char)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern void vstr_sc_bmap_init_eq_spn_cstr(unsigned char [VSTR__COMPILE_STATIC_ARRAY() 256], 
                                          const char *, unsigned char)
    VSTR__COMPILE_ATTR_NONNULL_A();
extern void vstr_sc_bmap_init_or_spn_cstr(unsigned char [VSTR__COMPILE_STATIC_ARRAY() 256], 
                                          const char *, unsigned char)
    VSTR__COMPILE_ATTR_NONNULL_A();


/* == inline helper functions == */
/* indented because they aren't a documented part of the API ...
 * just used from the inline calls */
 extern int vstr_extern_inline_add_buf(struct Vstr_base *, size_t,
                                       const void *, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
 extern int vstr_extern_inline_del(struct Vstr_base *, size_t, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
 extern int vstr_extern_inline_sects_add(struct Vstr_sects *, size_t, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
 extern int vstr_extern_inline_add_rep_chr(struct Vstr_base *, size_t,
                                           char, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
 extern void *vstr_extern_inline_data_get(unsigned int)
    VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
 extern void  vstr_extern_inline_data_set(unsigned int, struct Vstr_ref *);

 extern void *vstr_wrap_memcpy(void *, const void *, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
 extern int   vstr_wrap_memcmp(const void *, const void *, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A()
    VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
 extern void *vstr_wrap_memchr(const void *, int, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A()
    VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
 extern void *vstr_wrap_memset(void *, int, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();
 extern void *vstr_wrap_memmove(void *, const void *, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A();

 extern int vstr_cache__pos(const struct Vstr_base *,
                            struct Vstr_node *, size_t, unsigned int);
 extern struct Vstr_node *vstr_base__pos(const struct Vstr_base *,
                                         size_t *, unsigned int *, int);
 extern char *vstr_export__node_ptr(const struct Vstr_node *)
    VSTR__COMPILE_ATTR_WARN_UNUSED_RET();
