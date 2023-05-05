## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

INCLUDES = -I$(top_srcdir)/src/dynamic-plugins -I$(top_srcdir)/src/dynamic-plugins/sf_engine -I$(top_builddir)/src/dynamic-plugins -I$(top_builddir)/src/dynamic-plugins/sf_engine

lib_LTLIBRARIES = libsf_sorules.la

libsf_sorules_la_LDFLAGS = -export-dynamic -module @XCCFLAGS@

libsf_sorules_la_SOURCES = \
sfsnort_dynamic_detection_lib.c \
sfsnort_dynamic_detection_lib.h \
detection_lib_meta.h \
rules.c \
imap_base64_decode.c \
imap_base64_decode.h \
web-misc_base64_decode.c \
web-misc_base64_decode.h \
misc_ber.c \
misc_ber.h \
bug35218.c \
1000.c \
1001.c \
109.c \
12759.c \
1915.c \
1939.c \
2044.c \
2313.c \
2404.c \
2486.c \
2527.c \
2528.c \
2570.c \
3036.c \
3052.c \
3099.c \
3665.c \
3682.c \
593.c \
637.c \
652.c \
bug25673.c \
bug26266.c \
bug29818.c \
bug30013.c \
bug31842.c \
misc_mozilla_sslv2_cmk.c \
misc_mysql_com_table_dump.c \
rule_abcxyz.c \
rule_storeandforward.c \
rule_storeandforward2.c \
rule_contentcontent.c \
rule_content_notcontent.c \
rule_ftpbounce.c \
rule_http_buffers.c \
rule_relative_contents.c \
sid1026.c \
sid1902.c \
sid1970.c \
sid2257.c \
sid2389.c \
sid2489.c \
sid2519.c \
sid2578.c \
sid272.c \
sid2922.c \
sid9999.c \
testsid.c \
bug34427.c \
bug31159.c \
32694.c \
34408.c \
36734.c \
36733.c \
web-client_test.c \
dce_so_rule.c \
flowbits_test.c \
bug109059-builtin.c \
bug109059-custom.c
