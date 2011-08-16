## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

noinst_LIBRARIES = libspo.a

if BUILD_PRELUDE
PRELUDE_CODE = spo_alert_prelude.c spo_alert_prelude.h
endif

libspo_a_SOURCES = spo_alert_fast.c spo_alert_fast.h \
spo_alert_full.c spo_alert_full.h \
spo_alert_syslog.c spo_alert_syslog.h spo_alert_unixsock.c     \
spo_alert_unixsock.h spo_csv.c spo_csv.h spo_database.c spo_database.h         \
spo_log_null.c spo_log_null.h spo_log_tcpdump.c \
spo_log_tcpdump.h spo_unified.c spo_unified2.c spo_unified.h spo_unified2.h \
spo_log_ascii.c spo_log_ascii.h spo_alert_sf_socket.h spo_alert_sf_socket.c \
$(PRELUDE_CODE) spo_alert_arubaaction.c spo_alert_arubaaction.h \
spo_alert_test.c spo_alert_test.h

INCLUDES = @INCLUDES@
