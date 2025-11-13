# See if we need to provide obstacks.

dnl Copyright 1996-2022 Free Software Foundation, Inc.
dnl This file is free software; the Free Software Foundation
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

dnl Autoconf's AC_FUNC_OBSTACK is marked obsolete since version 2.70.
dnl We provide our own macro here.

AC_DEFUN([gl_FUNC_OBSTACK],
[
  AC_CACHE_CHECK([for obstacks that work with any size object],
    [gl_cv_func_obstack],
    [AC_LINK_IFELSE(
       [AC_LANG_PROGRAM(
          [[#include "obstack.h"
            void *obstack_chunk_alloc (size_t n) { return 0; }
            void obstack_chunk_free (void *p) { }
            /* Check that an internal function returns size_t, not int.  */
            size_t _obstack_memory_used (struct obstack *);
           ]],
          [[struct obstack mem;
            obstack_init (&mem);
            obstack_free (&mem, 0);
          ]])],
       [gl_cv_func_obstack=yes],
       [gl_cv_func_obstack=no])])
  if test "$gl_cv_func_obstack" = yes; then
    AC_DEFINE([HAVE_OBSTACK], 1,
      [Define to 1 if the system has obstacks that work with any size object.])
  fi
])
