dnl Checks for Kerberos
dnl NOTE: while we intend to do generic gss-api, currently we
dnl have a requirement to get an initial Kerberos machine
dnl credential.  Thus, the requirement for Kerberos.
dnl The Kerberos gssapi library will be dynamically loaded?
AC_DEFUN([AC_KERBEROS_V5],[
  AC_MSG_CHECKING(for Kerberos v5)
  AC_ARG_WITH(krb5,
  [AS_HELP_STRING([--with-krb5=DIR],[use Kerberos v5 installation in DIR])],
  [ case "$withval" in
    yes|no)
       krb5_with=""
       ;;
    *)
       krb5_with="$withval"
       ;;
    esac ]
  )
  gssapi_lib="gssapi_krb5"
  dnl Now check for functions within gssapi library
  AC_CHECK_LIB($gssapi_lib, gss_krb5_export_lucid_sec_context,
    AC_DEFINE(HAVE_LUCID_CONTEXT_SUPPORT, 1, [Define this if the Kerberos GSS library supports gss_krb5_export_lucid_sec_context]), ,$KRBLIBS)
  AC_CHECK_LIB($gssapi_lib, gss_krb5_set_allowable_enctypes,
    AC_DEFINE(HAVE_SET_ALLOWABLE_ENCTYPES, 1, [Define this if the Kerberos GSS library supports gss_krb5_set_allowable_enctypes]), ,$KRBLIBS)
  AC_CHECK_LIB($gssapi_lib, gss_krb5_free_lucid_sec_context,
    AC_DEFINE(HAVE_GSS_KRB5_FREE_LUCID_SEC_CONTEXT, 1, [Define this if the Kerberos GSS library supports gss_krb5_free_lucid_sec_context]), ,$KRBLIBS)

  dnl Check for newer error message facility
  AC_CHECK_LIB($gssapi_lib, krb5_get_error_message,
    AC_DEFINE(HAVE_KRB5_GET_ERROR_MESSAGE, 1, [Define this if the function krb5_get_error_message is available]), ,$KRBLIBS)

  dnl Check for function to specify addressless tickets
  AC_CHECK_LIB($gssapi_lib, krb5_get_init_creds_opt_set_addressless,
    AC_DEFINE(HAVE_KRB5_GET_INIT_CREDS_OPT_SET_ADDRESSLESS, 1, [Define this if the function krb5_get_init_creds_opt_set_addressless is available]), ,$KRBLIBS)

  dnl If they specified a directory and it didn't work, give them a warning
  if test "x$krb5_with" != "x" -a "$krb5_with" != "$KRBDIR"; then
    AC_MSG_WARN(Using $KRBDIR instead of requested value of $krb5_with for Kerberos!)
  fi

  AC_DEFINE(HAVE_KRB5, 1, [Define this if you have MIT Kerberos libraries])
  AC_SUBST([KRBDIR])
  AC_SUBST([KRBLIBS])
  AC_SUBST([KRBCFLAGS])
  AC_SUBST([KRBLDFLAGS])
  AC_SUBST([K5VERS])
  AC_SUBST([GSSKRB_CFLAGS])
  AC_SUBST([GSSKRB_LIBS])

])
