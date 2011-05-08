dnl
dnl $Id: config.m4 72968 2002-03-12 16:44:00Z sas $
dnl

PHP_ARG_ENABLE(ctype, whether to enable ctype functions,
[  --disable-ctype         Disable ctype functions], yes)

if test "$PHP_CTYPE" != "no"; then
  AC_DEFINE(HAVE_CTYPE, 1, [ ])
  PHP_NEW_EXTENSION(ctype, ctype.c, $ext_shared)
fi
