AC_DEFUN(mfx_CHECK_CROSS,
[
AC_REQUIRE([AC_PROG_CC])

if test "X$cross_compiling" = Xyes; then
  if test "X$build" = "X$host"; then
    AC_MSG_ERROR([you are cross compiling - please use the \`--host=' option])
  fi
fi

])
