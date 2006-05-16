## --------------------------------------------------------- ##
## Check the byte order.                                     ##
## Adapted from AC_C_BIGENDIAN.                              ##
## --------------------------------------------------------- ##

# serial 1

AC_DEFUN(mfx_C_BYTE_ORDER,
[AC_CACHE_CHECK([the byte order], mfx_cv_c_byte_order,
[AC_RUN_IFELSE([AC_LANG_PROGRAM([], [[
  union { long l; char c[sizeof(long)]; } u;
  u.l = 1; exit(u.c[0] == 1);
]])],
[mfx_cv_c_byte_order=4321],
[mfx_cv_c_byte_order=1234],
[mfx_cv_c_byte_order=unknown])])
if test "X$mfx_cv_c_byte_order" = X1234; then
  AC_DEFINE_UNQUOTED(MFX_BYTE_ORDER,1234)
fi
if test "X$mfx_cv_c_byte_order" = X4321; then
  AC_DEFINE_UNQUOTED(MFX_BYTE_ORDER,4321)
fi
])
