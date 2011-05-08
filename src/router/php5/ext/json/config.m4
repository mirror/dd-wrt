dnl
dnl $Id: config.m4 282645 2009-06-23 13:09:34Z johannes $
dnl

PHP_ARG_ENABLE(json, whether to enable JavaScript Object Serialization support,
[  --disable-json          Disable JavaScript Object Serialization support], yes)

if test "$PHP_JSON" != "no"; then
  AC_DEFINE([HAVE_JSON],1 ,[whether to enable JavaScript Object Serialization support])
  AC_HEADER_STDC

  PHP_NEW_EXTENSION(json, json.c utf8_to_utf16.c utf8_decode.c JSON_parser.c, $ext_shared)
  PHP_INSTALL_HEADERS([ext/json], [php_json.h])
  PHP_SUBST(JSON_SHARED_LIBADD)
fi
