## --------------------------------------------------------- ##
## Check for working memcmp.                                 ##
## Adapted from AC_FUNC_MEMCMP.                              ##
## --------------------------------------------------------- ##

# serial 1

AC_DEFUN(mfx_FUNC_MEMCMP,
[AC_CACHE_CHECK([for working memcmp], mfx_cv_func_memcmp,
[AC_RUN_IFELSE([AC_LANG_PROGRAM([[
#include <string.h>
 ]], [[
  unsigned char c0 = 0x40, c1 = 0x80, c2 = 0x81;
  char aa[21], bb[21];
  int i, j;
  if (memcmp(&c0, &c2, 1) >= 0 || memcmp(&c1, &c2, 1) >= 0)
    exit(1);
  for (i = 0; i < 4; i++) {
    for (j = 0; j < 4; j++) {
      char *a = aa + i; char *b = bb + j;
      strcpy(a, "0000000012222222");
      strcpy(b, "0000000021111111");
      if (memcmp(a, b, 16) >= 0)
        exit(1);
    }
  }
]])],
[mfx_cv_func_memcmp=yes],
[mfx_cv_func_memcmp=no],
[mfx_cv_func_memcmp=unknown])])
if test "$mfx_cv_func_memcmp" = no; then
  AC_DEFINE(NO_MEMCMP)
fi
])
