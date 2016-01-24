#if defined(__i386__)
# include "autoconf.h" /* ASM_GLOBAL_DIRECTIVE */

# ifndef ASM_GLOBAL_DIRECTIVE
#  error "No ASM_GLOBAL_DIRECTIVE"
# endif

#define HAVE_ELF 1
#define NO_UNDERSCORES 1
#define __ASSEMBLER__ 1
#ifndef  __BOUNDED_POINTERS__
# define __BOUNDED_POINTERS__ 0
#endif

#ifdef HAVE_ATTRIB_VISIBILITY
# define HAVE_HIDDEN 1
#endif

#ifdef __PIC__
# ifndef  PIC
#  define PIC __PIC__ /* glibc uses PIC -- gcc3 gives it, but not gcc2 */
# endif
#endif

# include "vstr_dbl/glibc/sysdeps/i386/sysdep.h"
# include "vstr_dbl/glibc/sysdeps/i386/asm-syntax.h"
# include "vstr_dbl/glibc/sysdeps/generic/bp-sym.h"
# include "vstr_dbl/glibc/sysdeps/i386/bp-asm.h"

/* from libc-symbols.h -- just need this */
# ifndef C_SYMBOL_NAME
#  ifdef NO_UNDERSCORES
#   define C_SYMBOL_NAME(name) name
#  else
#   define C_SYMBOL_NAME(name) _##name
#  endif
# endif


# define __mpn_add_n    vstr__fmt_dbl_mpn_add_n
# define __mpn_addmul_1 vstr__fmt_dbl_mpn_addmul_1
# define __mpn_mul_1    vstr__fmt_dbl_mpn_mul_1
# define __mpn_sub_n    vstr__fmt_dbl_mpn_sub_n
# define __mpn_submul_1 vstr__fmt_dbl_mpn_submul_1
# define __mpn_lshift   vstr__fmt_dbl_mpn_lshift
# define __mpn_rshift   vstr__fmt_dbl_mpn_rshift

#ifdef HAVE_ELF /* code for L define */
#define VSTR__ASM_L(name)         .L##name
#else
#define VSTR__ASM_L(name)         name
#endif

/* #define HAVE_i686 1 */
/* TODO: Maybe bother to set this up ...
 * do 3 libraries and get the dynamic linker to do the cool stuff ?
 * ... it's correct for i386, so I don't really care
 */
# if   defined(HAVE_i686)
#  undef  PARMS
#  undef  SIZE
#  undef  L
#  define L(x) VSTR__ASM_L(add_n ## x)
#  include "vstr_dbl/glibc/sysdeps/i386/i686/add_n.S"
#  undef  PARMS
#  undef  SIZE
#  undef  L
#  define L(x) VSTR__ASM_L(addmul_1 ## x)
#  include "vstr_dbl/glibc/sysdeps/i386/i586/addmul_1.S"
#  undef  PARMS
#  undef  SIZE
#  undef  L
#  define L(x) VSTR__ASM_L(mul_1 ## x)
#  include "vstr_dbl/glibc/sysdeps/i386/i586/mul_1.S"
#  undef  PARMS
#  undef  SIZE
#  undef  L
#  define L(x) VSTR__ASM_L(sub_n ## x)
#  include "vstr_dbl/glibc/sysdeps/i386/i586/sub_n.S"
#  undef  PARMS
#  undef  SIZE
#  undef  L
#  define L(x) VSTR__ASM_L(submul_1 ## x)
#  include "vstr_dbl/glibc/sysdeps/i386/i586/submul_1.S"
#  undef  PARMS
#  undef  SIZE
#  undef  L
#  define L(x) VSTR__ASM_L(lshift ## x)
#  include "vstr_dbl/glibc/sysdeps/i386/i586/lshift.S"
#  undef  PARMS
#  undef  SIZE
#  undef  L
#  define L(x) VSTR__ASM_L(rshift ## x)
#  include "vstr_dbl/glibc/sysdeps/i386/i586/rshift.S"
# elif defined(HAVE_i586)
#  undef  PARMS
#  undef  SIZE
#  undef  L
#  define L(x) VSTR__ASM_L(add_n ## x)
#  include "vstr_dbl/glibc/sysdeps/i386/i586/add_n.S"
#  undef  PARMS
#  undef  SIZE
#  undef  L
#  define L(x) VSTR__ASM_L(addmul_1 ## x)
#  include "vstr_dbl/glibc/sysdeps/i386/i586/addmul_1.S"
#  undef  PARMS
#  undef  SIZE
#  undef  L
#  define L(x) VSTR__ASM_L(mul_1 ## x)
#  include "vstr_dbl/glibc/sysdeps/i386/i586/mul_1.S"
#  undef  PARMS
#  undef  SIZE
#  undef  L
#  define L(x) VSTR__ASM_L(sub_n ## x)
#  include "vstr_dbl/glibc/sysdeps/i386/i586/sub_n.S"
#  undef  PARMS
#  undef  SIZE
#  undef  L
#  define L(x) VSTR__ASM_L(submul_1 ## x)
#  include "vstr_dbl/glibc/sysdeps/i386/i586/submul_1.S"
#  undef  PARMS
#  undef  SIZE
#  undef  L
#  define L(x) VSTR__ASM_L(lshift ## x)
#  include "vstr_dbl/glibc/sysdeps/i386/i586/lshift.S"
#  undef  PARMS
#  undef  SIZE
#  undef  L
#  define L(x) VSTR__ASM_L(rshift ## x)
#  include "vstr_dbl/glibc/sysdeps/i386/i586/rshift.S"
# else
#  undef  PARMS
#  undef  SIZE
#  undef  L
#  define L(x) VSTR__ASM_L(add_n ## x)
#  include "vstr_dbl/glibc/sysdeps/i386/add_n.S"
#  undef  PARMS
#  undef  SIZE
#  undef  L
#  define L(x) VSTR__ASM_L(addmul_1 ## x)
#  include "vstr_dbl/glibc/sysdeps/i386/addmul_1.S"
#  undef  PARMS
#  undef  SIZE
#  undef  L
#  define L(x) VSTR__ASM_L(mul_1 ## x)
#  include "vstr_dbl/glibc/sysdeps/i386/mul_1.S"
#  undef  PARMS
#  undef  SIZE
#  undef  L
#  define L(x) VSTR__ASM_L(sub_n ## x)
#  include "vstr_dbl/glibc/sysdeps/i386/sub_n.S"
#  undef  PARMS
#  undef  SIZE
#  undef  L
#  define L(x) VSTR__ASM_L(submul_1 ## x)
#  include "vstr_dbl/glibc/sysdeps/i386/submul_1.S"
#  undef  PARMS
#  undef  SIZE
#  undef  L
#  define L(x) VSTR__ASM_L(lshift ## x)
#  include "vstr_dbl/glibc/sysdeps/i386/lshift.S"
#  undef  PARMS
#  undef  SIZE
#  undef  L
#  define L(x) VSTR__ASM_L(rshift ## x)
#  include "vstr_dbl/glibc/sysdeps/i386/rshift.S"
# endif

#else
# error "No asm files for arch"
#endif
