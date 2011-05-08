dnl $Id: config.m4 258935 2008-05-02 23:05:05Z tony2001 $
dnl config.m4 for extension reflection

AC_DEFINE(HAVE_REFLECTION, 1, [Whether Reflection is enabled])
PHP_NEW_EXTENSION(reflection, php_reflection.c, no)
