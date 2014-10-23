sbin_PROGRAMS   = sstpc
sstpc_CFLAGS	= -I$(top_srcdir)/include -DSSTP_RUNTIME_DIR='"$(SSTP_RUNTIME_DIR)"'
sstpc_LDADD	    =       \
    libsstp-log/libsstp_log.la \
    libsstp-api/libsstp_api.la \
    libsstp-compat/libsstp_compat.la

utest_task_SOURCES  = sstp-task.c
utest_task_CFLAGS   = -I$(top_srcdir)/include -D__SSTP_UNIT_TEST_TASK=1
utest_cmac_SOURCES  = sstp-cmac.c
utest_cmac_CFLAGS   = -I$(top_srcdir)/include -D__SSTP_UNIT_TEST_CMAC=1
utest_chap_SOURCES  = sstp-chap.c
utest_chap_CFLAGS   = -I$(top_srcdir)/include -D__SSTP_UNIT_TEST_CHAP=1
utest_fcs_SOURCES   = sstp-fcs.c
utest_fcs_CFLAGS    = -I$(top_srcdir)/include -D__SSTP_UNIT_TEST_FCS=1
utest_route_SOURCES = sstp-route.c
utest_route_CFLAGS  = -I$(top_srcdir)/include -D__SSTP_UNIT_TEST_ROUTE=1

check_PROGRAMS      =   \
    utest_task          \
    utest_cmac          \
    utest_chap          \
    utest_fcs           \
    utest_route

TESTS= $(check_PROGRAMS)

sstpc_SOURCES =         \
    sstp-client.c       \
    sstp-option.c       \
    sstp-stream.c       \
    sstp-packet.c       \
    sstp-pppd.c         \
    sstp-util.c         \
    sstp-cmac.c         \
    sstp-buff.c         \
    sstp-http.c         \
    sstp-task.c         \
    sstp-event.c        \
    sstp-state.c        \
    sstp-chap.c         \
    sstp-route.c        \
    sstp-fcs.c

noinst_HEADERS  =       \
    sstp-buff.h         \
    sstp-client.h       \
    sstp-chap.h         \
    sstp-cmac.h         \
    sstp-event.h        \
    sstp-fcs.h          \
    sstp-http.h         \
    sstp-option.h       \
    sstp-packet.h       \
    sstp-pppd.h         \
    sstp-private.h      \
    sstp-route.h        \
    sstp-state.h        \
    sstp-stream.h       \
    sstp-task.h         \
    sstp-util.h
