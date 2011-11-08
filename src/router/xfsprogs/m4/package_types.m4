# 
# Check if we have a type for the pointer's size integer (__psint_t)
# 
AC_DEFUN([AC_TYPE_PSINT],
  [ AC_MSG_CHECKING([for __psint_t ])
    AC_TRY_COMPILE([
#include <sys/types.h>
#include <stdlib.h>
#include <stddef.h>
    ], [
         __psint_t  psint;
    ], AC_DEFINE([HAVE___PSINT_T], [1], [Define if __psint_t exists]) AC_MSG_RESULT(yes) , AC_MSG_RESULT(no))
  ])

# 
# Check if we have a type for the pointer's size unsigned (__psunsigned_t)
# 
AC_DEFUN([AC_TYPE_PSUNSIGNED],
  [ AC_MSG_CHECKING([for __psunsigned_t ])
    AC_TRY_COMPILE([
#include <sys/types.h>
#include <stdlib.h>
#include <stddef.h>
    ], [
        __psunsigned_t  psuint;
    ], AC_DEFINE([HAVE___PSUNSIGNED_T], [1], [Define if __psunsigned_t exists]) AC_MSG_RESULT(yes) , AC_MSG_RESULT(no))
  ])

# 
# Check if we have a type for __u32
# 
AC_DEFUN([AC_TYPE_U32],
  [ AC_MSG_CHECKING([for __u32 ])
    AC_TRY_COMPILE([
#include <asm/types.h>
#include <stdlib.h>
#include <stddef.h>
    ], [
         __u32  u32;
    ], AC_DEFINE([HAVE___U32], [1], [Define if __u32 exists]) AC_MSG_RESULT(yes) , AC_MSG_RESULT(no))
  ])

# 
# Check type sizes
# 
AC_DEFUN([AC_SIZEOF_POINTERS_AND_LONG],
  [ if test "$cross_compiling" = yes -a -z "$ac_cv_sizeof_long"; then
      AC_MSG_WARN([Cross compiling; assuming 32bit long and 32bit pointers])
    fi
    AC_CHECK_SIZEOF(long, 4)
    AC_CHECK_SIZEOF(char *, 4)
    if test $ac_cv_sizeof_long -eq 4 -o $ac_cv_sizeof_long -eq 0; then
      AC_DEFINE([HAVE_32BIT_LONG], [1], [Define if long is 32bit])
    fi
    if test $ac_cv_sizeof_long -eq 8; then
      AC_DEFINE([HAVE_64BIT_LONG], [1], [Define if long is 64bit])
    fi
    if test $ac_cv_sizeof_char_p -eq 4 -o $ac_cv_sizeof_char_p -eq 0; then
      AC_DEFINE([HAVE_32BIT_PTR], [1], [Define if char* is 32bit])
    fi
    if test $ac_cv_sizeof_char_p -eq 8; then
      AC_DEFINE([HAVE_64BIT_PTR], [1], [Define if char* is 64bit])
    fi
  ])
