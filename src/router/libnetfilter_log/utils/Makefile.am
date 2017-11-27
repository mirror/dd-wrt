include ${top_srcdir}/Make_global.am

check_PROGRAMS = nfulnl_test ulog_test

nfulnl_test_SOURCES = nfulnl_test.c
nfulnl_test_LDADD = ../src/libnetfilter_log.la
nfulnl_test_LDFLAGS = -dynamic

ulog_test_SOURCES = ulog_test.c
ulog_test_LDADD = ../src/libnetfilter_log_libipulog.la ../src/libnetfilter_log.la
ulog_test_LDFLAGS = -dynamic

