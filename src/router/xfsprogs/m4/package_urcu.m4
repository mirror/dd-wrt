AC_DEFUN([AC_PACKAGE_NEED_URCU_H],
  [ AC_CHECK_HEADERS(urcu.h)
    if test $ac_cv_header_urcu_h = no; then
       AC_CHECK_HEADERS(urcu.h,, [
       echo
       echo 'FATAL ERROR: could not find a valid urcu header.'
       exit 1])
    fi
  ])

AC_DEFUN([AC_PACKAGE_NEED_RCU_INIT],
  [ AC_MSG_CHECKING([for liburcu])
    AC_COMPILE_IFELSE(
    [	AC_LANG_PROGRAM([[
#define _GNU_SOURCE
#include <urcu.h>
	]], [[
rcu_init();
	]])
    ], liburcu=-lurcu
       AC_MSG_RESULT(yes),
       AC_MSG_RESULT(no))
    AC_SUBST(liburcu)
  ])

#
# Make sure that calling uatomic_inc on a 64-bit integer doesn't cause a link
# error on _uatomic_link_error, which is how liburcu signals that it doesn't
# support atomic operations on 64-bit data types for its generic
# implementation (which relies on compiler builtins). For certain archs
# where liburcu carries its own implementation (such as x86_32), it
# signals lack of support during runtime by emitting an illegal
# instruction, so we also need to check CAA_BITS_PER_LONG to detect that.
#
AC_DEFUN([AC_HAVE_LIBURCU_ATOMIC64],
  [ AC_MSG_CHECKING([for atomic64_t support in liburcu])
    AC_LINK_IFELSE(
    [	AC_LANG_PROGRAM([[
#define _GNU_SOURCE
#include <urcu.h>
#define BUILD_BUG_ON(condition) ((void)sizeof(char[1 - 2*!!(condition)]))
	]], [[
long long f = 3;

BUILD_BUG_ON(CAA_BITS_PER_LONG < 64);
uatomic_inc(&f);
	]])
    ], have_liburcu_atomic64=yes
       AC_MSG_RESULT(yes),
       AC_MSG_RESULT(no))
    AC_SUBST(have_liburcu_atomic64)
  ])
