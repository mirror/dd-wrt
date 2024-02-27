# SYNOPSIS
#
#   MHD_CHECK_FUNC_GETTIMEOFDAY([ACTION-IF-AVAILABLE],
#                               [ACTION-IF-NOT-AVAILABLE])
#
# DESCRIPTION
#
#   This macro checks for presence of gettimeofday() function.
#   If function is available macro HAVE_GETTIMEOFDAY is defined
#   automatically.
#
#   Example usage:
#
#     MHD_CHECK_FUNC_GETTIMEOFDAY([var_use_gettimeofday='yes'])
#
#   The cache variable used in check so if any test will not work
#   correctly on some platform, user may simply fix it by giving cache
#   variable in configure parameters, for example:
#
#     ./configure mhd_cv_func_memmem_have=no
#
#   This simplifies building from source on exotic platforms as patching
#   of configure.ac is not required to change results of tests.
#
# LICENSE
#
#   Copyright (c) 2019-2023 Karlson2k (Evgeny Grin) <k2k@narod.ru>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 1

AC_DEFUN([MHD_CHECK_FUNC_GETTIMEOFDAY],[dnl
AC_CHECK_HEADERS([sys/time.h time.h])dnl
MHD_CHECK_FUNC([[gettimeofday]],
  [[
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif /* HAVE_SYS_TIME_H */
#ifdef HAVE_TIME_H
#include <time.h>
#endif /* HAVE_TIME_H */
  ]],
  [[
  struct timeval tv;
  if (0 != gettimeofday (&tv, (void*) 0))
    return 1;
  ]],[$1],[$2]
)
])dnl AC_DEFUN MHD_CHECK_FUNC_GETTIMEOFDAY
