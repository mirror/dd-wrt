
PHP_ARG_WITH(system-tzdata, for use of system timezone data,
[  --with-system-tzdata[=DIR]      to specify use of system timezone data],
no, no)

if test "$PHP_SYSTEM_TZDATA" != "no"; then
   AC_DEFINE(HAVE_SYSTEM_TZDATA, 1, [Define if system timezone data is used])

   if test "$PHP_SYSTEM_TZDATA" != "yes"; then
      AC_DEFINE_UNQUOTED(HAVE_SYSTEM_TZDATA_PREFIX, "$PHP_SYSTEM_TZDATA",
                         [Define for location of system timezone data])
   fi
fi
