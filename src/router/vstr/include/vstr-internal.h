#ifndef VSTR__INTERNAL_HEADER_H
#define VSTR__INTERNAL_HEADER_H

/*
 *  Copyright (C) 1999-2006  James Antill
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

#ifdef __GNUC__
/* debug call */
# define D(x, args...) \
    fprintf(stderr, "DBG: %d:%s <" x ">\n", __LINE__, __FILE__ , ## args)
#endif

#ifdef __GNUC__
# define VSTR__ATTR_UNUSED(x) vstr__UNUSED_ ## x __attribute__((unused)) 
#elif defined(__LCLINT__)
# define VSTR__ATTR_UNUSED(x) /*@unused@*/ vstr__UNUSED_ ## x
#else
# define VSTR__ATTR_UNUSED(x) vstr__UNUSED_ ## x
#endif

#ifdef __GNUC__
# define VSTR__ATTR_USED() __attribute__((used))
#else
# define VSTR__ATTR_USED() /* do nothing */
#endif

#if defined(HAVE_ATTRIB_DEPRECATED)
# define VSTR__ATTR_D() __attribute__((deprecated))
#else
# define VSTR__ATTR_D()
#endif

#define VSTR__USE_INTERNAL_SYMBOLS ( \
      defined(HAVE_ATTRIB_ALIAS) && \
      defined(HAVE_ATTRIB_VISIBILITY) && \
      defined(HAVE___TYPEOF))

#if VSTR__USE_INTERNAL_SYMBOLS
/* only do visibility on symbols if we can alias the valid exports */
# define VSTR__ATTR_A(x) __attribute__((alias(x)))
# define VSTR__ATTR_H() __attribute__((visibility("hidden")))
# define VSTR__ATTR_I() __attribute__((visibility("internal")))

# define VSTR__SYM(x) \
 extern __typeof(vstr_nx_ ## x) vstr_nx_ ## x \
    VSTR__ATTR_H() ;

# define VSTR__SYM_ALIAS(x)                             \
 extern __typeof(vstr_nx_ ## x) vstr_ ## x              \
    VSTR__ATTR_A("vstr_nx_" #x ) VSTR__ATTR_D()

#else
# define VSTR__ATTR_H()
# define VSTR__ATTR_I()
# define VSTR__SYM_ALIAS(x) extern int vstr__does_not_exist(void)
#endif

#define VSTR_TYPE_FMT_UCHAR 1
#define VSTR_TYPE_FMT_USHORT 2

#define VSTR_TYPE_CACHE_NOTHING 0

/* ISO C magic, converts a ptr to ->next into ->next->prev
 *   (even though ->prev doesn't exist) */
#define VSTR__CONV_PTR_NEXT_PREV(x) \
 ((Vstr_node *)(((char *)(x)) - offsetof(Vstr_node, next)))

#define VSTR__UC(x) ((unsigned char)(x))
#define VSTR__IS_ASCII_LOWER(x) ((VSTR__UC(x) >= 0x61) && (VSTR__UC(x) <= 0x7A))
#define VSTR__IS_ASCII_UPPER(x) ((VSTR__UC(x) >= 0x41) && (VSTR__UC(x) <= 0x5A))
#define VSTR__IS_ASCII_DIGIT(x) ((VSTR__UC(x) >= 0x30) && (VSTR__UC(x) <= 0x39))
#define VSTR__IS_ASCII_ALPHA(x) (VSTR__IS_ASCII_LOWER(x) || \
                                 VSTR__IS_ASCII_UPPER(x))

#define VSTR__TO_ASCII_LOWER(x) (VSTR__UC(x) + 0x20) /* must be IS_ASCII_U */
#define VSTR__TO_ASCII_UPPER(x) (VSTR__UC(x) - 0x20) /* must be IS_ASCII_L */

#define VSTR__ASCII_DIGIT_a() (0x61)
#define VSTR__ASCII_DIGIT_0() (0x30)
#define VSTR__ASCII_COLON()   (0x3A)
#define VSTR__ASCII_COMMA()   (0x2C)

#define VSTR__MK(sz) \
   malloc_check_malloc(sz, __FILE__, __LINE__)
#define VSTR__MV(ptr, tmp, sz) \
   (((tmp) = malloc_check_realloc(ptr, sz, __FILE__, __LINE__)) && \
    ((ptr) = (tmp)))
#define VSTR__F(ptr) \
   malloc_check_free(ptr)

/* iteration is this node all of the rest of the iteration,
 * Ie. vstr_iter_fwd_nxt() will return FALSE ... called at the start of an
 * iteration to see if the iteration consists of one node. */
/* NOTE: see tst_sub_ptr for a copy of this... so it gets tested */
#define VSTR__ITER_EQ_ALL_NODE(base, iter) \
  (!(iter)->remaining && \
   (((base)->beg != (iter)->node) ? \
    \
    (((iter)->len == (iter)->node->len) && \
     ((iter)->ptr == (vstr_export__node_ptr((iter)->node)))) : \
    \
    (((iter)->len == (size_t)((iter)->node->len - (base)->used)) && \
     ((iter)->ptr == (vstr_export__node_ptr((iter)->node) + (base)->used)))))

/* print a number backwards (any base)... */
/* chrs_base, buf_beg, buf all need to exist */
#define VSTR__ADD_FMT_NUM(type, passed_num, num_base) do { \
 type num = passed_num; \
 \
 while (num) \
 { \
  unsigned int chr_offset = (num % num_base); \
  \
  ASSERT(buf > buf_beg); \
  \
  num /= num_base; \
  *--buf = chrs_base[chr_offset]; \
 } \
} while (FALSE)

#define VSTR__CACHE_INTERNAL_POS_MAX 2

#ifndef USE_MALLOC_CHECK
# ifdef NDEBUG
#  define USE_MALLOC_CHECK 0
# else
#  define USE_MALLOC_CHECK 1
# endif
#endif

#ifndef USE_FD_CLOSE_CHECK
# ifdef NDEBUG
#  define USE_FD_CLOSE_CHECK 0
# else
#  define USE_FD_CLOSE_CHECK 1
# endif
#endif

#define MALLOC_CHECK_PRINT 0 /* needs to be zero'd for coverage testing */
#define MALLOC_CHECK_STORE vstr__malloc_check_store
#define MALLOC_CHECK__ATTR_H()      VSTR__ATTR_H()
#define MALLOC_CHECK__ATTR_USED()   VSTR__ATTR_USED()
#define MALLOC_CHECK__ATTR_MALLOC() VSTR__COMPILE_ATTR_MALLOC()
#include "malloc-check.h"
#if !(USE_MALLOC_CHECK)
# define VSTR__CONF_REF_LINKED_SZ (UINT_MAX / 2)
# define VSTR__SECTS_SZ 8
# define VSTR__STACK_BUF_SZ 64
# define VSTR__FMT_USR_SZ 8
# define VSTR__REF_GRP_MAKE_SZ 42 /* makes 512 bytes ia32: 4 + 4 + 12*x */
# define VSTR__SC_VEC_SZ 32
#else
# define VSTR__CONF_REF_LINKED_SZ 2
# define VSTR__SECTS_SZ 2
# define VSTR__STACK_BUF_SZ 2
# define VSTR__FMT_USR_SZ 3
# define VSTR__REF_GRP_MAKE_SZ 2
# define VSTR__SC_VEC_SZ 2
#endif

typedef struct Vstr__options
{
 Vstr_conf *def; /* define conf */

 unsigned int mmap_count; /* mmap debugging */
 
 unsigned int  fd_count; /* fd debugging */
 unsigned long fd_close_fail_num;
 } Vstr__options;

typedef struct Vstr__cache_data_iovec Vstr__cache_data_iovec;

typedef struct Vstr__cache_data_cstr Vstr__cache_data_cstr;

typedef struct Vstr__cache_data_pos Vstr__cache_data_pos;

typedef struct Vstr__cache Vstr__cache;

typedef struct Vstr__base_cache Vstr__base_cache;

typedef struct Vstr__buf_ref
{
 Vstr_ref ref;
 char VSTR__STRUCT_HACK_ARRAY(buf);
} Vstr__buf_ref;

typedef struct Vstr__sc_mmap_ref
{
 Vstr_ref ref;
 size_t mmap_len;
} Vstr__sc_mmap_ref;

typedef struct Vstr__fmt_usr_name_node
{
 struct Vstr__fmt_usr_name_node *next;

 const char *name_str;
 size_t      name_len;
 int (*func)(Vstr_base *, size_t, struct Vstr_fmt_spec *);

 unsigned int sz;
 unsigned int VSTR__STRUCT_HACK_ARRAY(types);
} Vstr__fmt_usr_name_node;

#ifdef HAVE_LONG_LONG
typedef unsigned long long Vstr__unsigned_long_long;
typedef          long long Vstr__long_long;
#else
typedef unsigned      long Vstr__unsigned_long_long;
typedef               long Vstr__long_long;
#endif

#ifdef USE_RESTRICTED_HEADERS /* always use C locale */
# define setlocale(x, y) NULL
# define localeconv()    NULL
# define SYS_LOC(x) ""
# undef  USE_WIDE_CHAR_T
# define USE_WIDE_CHAR_T 0
#else
# define SYS_LOC(x) ((sys_loc)->x)
#endif

extern Vstr__options VSTR__ATTR_H() vstr__options;

/* the size of ULONG_MAX converted to a string */
#ifndef VSTR_AUTOCONF_ULONG_MAX_LEN
extern size_t vstr__netstr2_ULONG_MAX_len;
#define VSTR__ULONG_MAX_LEN vstr__netstr2_ULONG_MAX_len
#define VSTR__ULONG_MAX_SET_LEN(x) (vstr__netstr2_ULONG_MAX_len = (x))
#else
#define VSTR__ULONG_MAX_LEN VSTR_AUTOCONF_ULONG_MAX_LEN
#define VSTR__ULONG_MAX_SET_LEN(x) /* do nothing */
#endif

#ifndef NDEBUG
extern int vstr__check_real_nodes(const Vstr_base *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET()
    VSTR__ATTR_I();
extern int vstr__check_spare_nodes(const Vstr_conf *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET()
    VSTR__ATTR_I();
#endif

extern void vstr__add_user_conf(Vstr_conf *) VSTR__ATTR_I();
extern void vstr__add_base_conf(Vstr_base *, Vstr_conf *) VSTR__ATTR_I();
/* vstr__del_user_user == nx_free_conf */
extern void vstr__del_conf(Vstr_conf *) VSTR__ATTR_I();

extern Vstr_node *vstr__add_setup_pos(Vstr_base *, size_t *, unsigned int *,
                                      size_t *)
    VSTR__COMPILE_ATTR_NONNULL_L((1, 2, 3)) VSTR__COMPILE_ATTR_WARN_UNUSED_RET()
    VSTR__ATTR_I();

extern void vstr__base_zero_used(Vstr_base *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__ATTR_I();
extern Vstr_node *vstr__base_split_node(Vstr_base *, Vstr_node *, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET()
    VSTR__ATTR_I();

extern Vstr_node **vstr__base_ptr_pos(const Vstr_base *, size_t *,
                                      unsigned int *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET()
    VSTR__ATTR_I();
extern int vstr__base_scan_rev_beg(const Vstr_base *, size_t, size_t *,
                                   unsigned int *, unsigned int *,
                                   char **, size_t *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET()
    VSTR__ATTR_I();
extern int vstr__base_scan_rev_nxt(const Vstr_base *, size_t *,
                                   unsigned int *, unsigned int *,
                                   char **, size_t *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET()
    VSTR__ATTR_I();

extern void vstr__relink_nodes(Vstr_conf *, Vstr_node *, Vstr_node **,
                               unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__ATTR_I();
extern int vstr__chg_node_buf_ref(const Vstr_base *, Vstr_node **,
                                  unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__ATTR_I();
extern void vstr__swap_node_X_X(const Vstr_base *, size_t, Vstr_node *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__ATTR_I();

extern int vstr__cache_iovec_alloc(const Vstr_base *, unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET()
    VSTR__ATTR_I();
extern void vstr__cache_iovec_add_node_end(Vstr_base *, unsigned int,
                                           unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__ATTR_I();
extern void vstr__cache_iovec_reset_node(const Vstr_base *base, Vstr_node *node,
                                        unsigned int num)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__ATTR_I();
extern int vstr__cache_iovec_valid(Vstr_base *) /* makes it valid */
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET()
    VSTR__ATTR_I();

extern void vstr__cache_free_cstr(const Vstr_base *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__ATTR_I();
extern void vstr__cache_del(const Vstr_base *, size_t, size_t) VSTR__ATTR_I();
extern void vstr__cache_add(const Vstr_base *, size_t, size_t) VSTR__ATTR_I();
extern void vstr__cache_cstr_cpy(const Vstr_base *, size_t, size_t,
                                 const Vstr_base *, size_t) VSTR__ATTR_I();
extern int  vstr__make_cache(const Vstr_base *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__ATTR_I();
extern void vstr__free_cache(const Vstr_base *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__ATTR_I();
extern int  vstr__cache_conf_init(Vstr_conf *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET()
    VSTR__ATTR_I();
extern int  vstr__cache_subset_cbs(Vstr_conf *, Vstr_conf *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET()
    VSTR__ATTR_I();
extern int  vstr__cache_dup_cbs(Vstr_conf *, Vstr_conf *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__ATTR_I();

extern int  vstr__data_conf_init(Vstr_conf *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET()
    VSTR__ATTR_I();
extern void vstr__data_conf_free(Vstr_conf *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__ATTR_I();

extern unsigned int vstr__add_fmt_grouping_mod(const char *, unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET()
    VSTR__ATTR_I();
extern size_t vstr__add_fmt_grouping_num_sz(Vstr_base *, unsigned int, size_t)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET()
    VSTR__ATTR_I();

extern Vstr_locale_num_base *vstr__loc_num_srch(Vstr_locale *,
                                                unsigned int, int)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET()
    VSTR__COMPILE_ATTR_PURE() VSTR__ATTR_I();
extern const char *
vstr__loc_num_grouping(Vstr_locale *, unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET()
    VSTR__COMPILE_ATTR_PURE() VSTR__ATTR_I();
extern const char *
vstr__loc_num_sep_ptr(Vstr_locale *, unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET()
    VSTR__COMPILE_ATTR_PURE() VSTR__ATTR_I();
extern size_t
vstr__loc_num_sep_len(Vstr_locale *, unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET()
    VSTR__COMPILE_ATTR_PURE() VSTR__ATTR_I();
extern const char *
vstr__loc_num_pnt_ptr(Vstr_locale *, unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET()
    VSTR__COMPILE_ATTR_PURE() VSTR__ATTR_I();
extern size_t
vstr__loc_num_pnt_len(Vstr_locale *, unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__COMPILE_ATTR_WARN_UNUSED_RET()
    VSTR__COMPILE_ATTR_PURE() VSTR__ATTR_I();

extern void vstr__del_grpalloc(Vstr_conf *, unsigned int)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__ATTR_I();

extern int vstr__sc_fmt_add_posix(Vstr_conf *conf)
   VSTR__COMPILE_ATTR_WARN_UNUSED_RET() VSTR__ATTR_I();

extern void *vstr_wrap_memrchr(const void *, int, size_t)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__ATTR_I();

#define VSTR__FLAG_REF_GRP_REF (1U<<6)
extern Vstr_ref_grp_ptr *vstr__ref_grp_make(void (*func) (struct Vstr_ref *),
                                            unsigned int flags)
   VSTR__COMPILE_ATTR_NONNULL_A() VSTR__ATTR_I();
extern Vstr_ref *vstr__ref_grp_add(Vstr_ref_grp_ptr **parent, const void *ptr)
   VSTR__COMPILE_ATTR_NONNULL_L((1)) VSTR__ATTR_I();
extern void vstr__ref_grp_free(Vstr_ref_grp_ptr *parent)
   VSTR__ATTR_I();

extern size_t vstr__loc_thou_grp_strlen(const char *)
    VSTR__COMPILE_ATTR_PURE() VSTR__COMPILE_ATTR_NONNULL_A() VSTR__ATTR_I();
extern int vstr__make_conf_loc_numeric(Vstr_conf *, const char *)
    VSTR__ATTR_I();

extern void vstr__add_fmt_free_conf(Vstr_conf *) VSTR__ATTR_I();

extern Vstr__fmt_usr_name_node *vstr__fmt_usr_match(Vstr_conf *, const char *)
    VSTR__COMPILE_ATTR_NONNULL_A() VSTR__ATTR_I();

/* so the linker-script does the right thing */
extern void vstr_version_func(void);

#if  VSTR__USE_INTERNAL_SYMBOLS
# include "internal_syms_generated/vstr-cpp-symbols_fwd.h"
#endif

#include "vstr-extern.h"

#ifndef NDEBUG /* inline debugging stuff... */
# ifdef USE_ASSERT_LOOP
#  define VSTR__ASSERT(x) do { \
 if (x) {} else \
  vstr__assert_loop(#x, __FILE__, __LINE__, __func__); } \
 while (FALSE)
#  define VSTR__ASSERT_RET(x, y)  do { \
 if (x) {} else \
  vstr__assert_loop(#x, __FILE__, __LINE__, __func__); } \
 while (FALSE)
# else
#  define VSTR__ASSERT(x) do { \
 if (x) {} else { \
  fprintf(stderr, " -=> ASSERT (%s) failed in (%s) from %d %s.\n", \
          #x , __func__, __LINE__, __FILE__); \
  abort(); } } \
 while (FALSE)
#  define VSTR__ASSERT_RET(x, y)  do { \
 if (x) {} else { \
  fprintf(stderr, " -=> ASSERT (%s) failed in (%s) from %d %s.\n", \
          #x , __func__, __LINE__, __FILE__); \
  abort(); } } \
 while (FALSE)
#  define VSTR__ASSERT_RET_VOID(x)  do { \
 if (x) {} else { \
  fprintf(stderr, " -=> ASSERT (%s) failed in (%s) from %d %s.\n", \
          #x , __func__, __LINE__, __FILE__); \
  abort(); } } \
 while (FALSE)
# endif
# define VSTR__ASSERT_NO_SWITCH_DEF() break; default: VSTR__ASSERT(!"default label")
#endif

#if defined(VSTR_AUTOCONF_HAVE_INLINE) && VSTR_COMPILE_INLINE
# include "vstr-inline.h"
# include "vstr-nx-inline.h"
#else
# ifndef VSTR_AUTOCONF_USE_WRAP_MEMCPY
#  define vstr_wrap_memcpy(x, y, z)  memcpy(x, y, z)
# endif
# ifndef VSTR_AUTOCONF_USE_WRAP_MEMCMP
#  define vstr_wrap_memcmp(x, y, z)  memcmp(x, y, z)
# endif
# ifndef VSTR_AUTOCONF_USE_WRAP_MEMCHR
#  define vstr_wrap_memchr(x, y, z)  memchr(x, y, z)
# endif
# ifndef VSTR_AUTOCONF_USE_WRAP_MEMRCHR
#  define vstr_wrap_memrchr(x, y, z) memrchr(x, y, z)
# endif
# ifndef VSTR_AUTOCONF_USE_WRAP_MEMSET
#  define vstr_wrap_memset(x, y, z)  memset(x, y, z)
# endif
# ifndef VSTR_AUTOCONF_USE_WRAP_MEMMOVE
#  define vstr_wrap_memmove(x, y, z) memmove(x, y, z)
# endif
#endif

#if  VSTR__USE_INTERNAL_SYMBOLS

# include "internal_syms_generated/vstr-cpp-symbols_rev.h"

# include "internal_syms_generated/vstr-alias-symbols.h"

# include "internal_syms_generated/vstr-cpp-symbols_fwd.h"

#endif

#endif
