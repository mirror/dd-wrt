AC_DEFUN([AC_PACKAGE_WANT_ATTRIBUTES_H],
  [
    AC_CHECK_HEADERS(attr/attributes.h)
  ])

#
# Check if we have a ATTR_ROOT flag and libattr structures
#
AC_DEFUN([AC_HAVE_LIBATTR],
  [ AC_MSG_CHECKING([for struct attrlist_cursor])
    AC_COMPILE_IFELSE(
    [	AC_LANG_PROGRAM([[
#include <sys/types.h>
#include <attr/attributes.h>
	]], [[
struct attrlist_cursor *cur;
struct attrlist *list;
struct attrlist_ent *ent;
int flags = ATTR_ROOT;
	]])
    ], have_libattr=yes
          AC_MSG_RESULT(yes),
          AC_MSG_RESULT(no))
    AC_SUBST(have_libattr)
  ])
