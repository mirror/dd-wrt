#
# Check if we have umode_t
#
AH_TEMPLATE([HAVE_UMODE_T], [Whether you have umode_t])
AC_DEFUN([AC_TYPE_UMODE_T],
  [ AC_MSG_CHECKING([for umode_t])
    AC_TRY_COMPILE([
#include <asm/types.h>
    ], [
         umode_t umode;
    ], AC_DEFINE(HAVE_UMODE_T) AC_MSG_RESULT(yes) , AC_MSG_RESULT(no))
  ])
