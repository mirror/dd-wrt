dnl ### Checking for builtin bool type.

# serial 1

AC_DEFUN(mfx_CHECK_BOOL,
[
        AC_MSG_CHECKING([for bool])
        AC_CACHE_VAL(mfx_cv_have_bool,
        [
                AC_LANG_CPLUSPLUS
                AC_TRY_COMPILE([],
                 [bool a = true; bool b = false;],
                 [mfx_cv_have_bool="yes"],
                 [mfx_cv_have_bool="no"])
        ]) dnl end AC_CHECK_VAL
        AC_MSG_RESULT($mfx_cv_have_bool)
        if test "X$mfx_cv_have_bool" = "Xyes"; then
                AC_DEFINE(HAVE_BOOL)
        fi
])
