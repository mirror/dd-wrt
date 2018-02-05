# Check if the platform has librt
#

AC_DEFUN([AC_RT],
[
  if test "$enable_librt" = "yes"; then
    librt="-lrt"
  else
    librt=""
  fi
  AC_SUBST(librt)
])
