dnl $Id: config.m4,v 1.4.2.3.2.1 2006/08/23 09:47:21 tony2001 Exp $
dnl config.m4 for extension reflection

PHP_ARG_ENABLE(reflection, whether to enable reflection support,
[  --disable-reflection    Disable reflection support], yes, no)

if test "$PHP_REFLECTION" != "no"; then
  AC_DEFINE(HAVE_REFLECTION, 1, [Whether Reflection is enabled])
  PHP_NEW_EXTENSION(reflection, php_reflection.c, no)
fi
