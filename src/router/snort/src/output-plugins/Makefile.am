## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

noinst_LIBRARIES = libspo.a

libspo_a_SOURCES = spo_alert_fast.c spo_alert_fast.h \
spo_alert_full.c spo_alert_full.h \
spo_alert_syslog.c spo_alert_syslog.h \
spo_alert_unixsock.c spo_alert_unixsock.h \
spo_csv.c spo_csv.h  \
spo_log_null.c spo_log_null.h \
spo_log_tcpdump.c spo_log_tcpdump.h \
spo_unified2.c spo_unified2.h \
spo_log_ascii.c spo_log_ascii.h \
spo_alert_sf_socket.h spo_alert_sf_socket.c \
spo_alert_test.c spo_alert_test.h

if BUILD_BUFFER_DUMP
libspo_a_SOURCES += \
spo_log_buffer_dump.c spo_log_buffer_dump.h
endif

INCLUDES = @INCLUDES@
