dnl $Id: config.m4 242949 2007-09-26 15:44:16Z cvs2svn $
PHP_ARG_ENABLE(shmop, whether to enable shmop support, 
[  --enable-shmop          Enable shmop support])

if test "$PHP_SHMOP" != "no"; then
  AC_DEFINE(HAVE_SHMOP, 1, [ ])
  PHP_NEW_EXTENSION(shmop, shmop.c, $ext_shared)
fi
