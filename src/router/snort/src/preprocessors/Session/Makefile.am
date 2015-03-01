## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

noinst_LIBRARIES = libsession.a

libsession_a_SOURCES = \
snort_session.c \
snort_session.h \
session_common.c \
session_common.h \
session_expect.c \
session_expect.h

libsession_a_LIBADD = \
session_common.o \
session_expect.o

if BUILD_HA
libsession_a_SOURCES += \
stream5_ha.c \
stream5_ha.h
libsession_a_LIBADD += \
stream5_ha.o
endif

INCLUDES = @INCLUDES@
