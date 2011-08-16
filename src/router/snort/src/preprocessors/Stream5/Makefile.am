## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

noinst_LIBRARIES = libstream5.a

libstream5_a_SOURCES = \
snort_stream5_tcp.c \
snort_stream5_tcp.h \
snort_stream5_udp.c \
snort_stream5_udp.h \
snort_stream5_icmp.c \
snort_stream5_icmp.h \
snort_stream5_session.c \
snort_stream5_session.h \
stream5_common.c \
stream5_common.h 

libstream5_a_LIBADD = \
snort_stream5_tcp.o \
snort_stream5_udp.o \
snort_stream5_icmp.o \
snort_stream5_session.o \
stream5_common.o

INCLUDES = @INCLUDES@
