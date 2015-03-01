## $Id
AUTOMAKE_OPTIONS=foreign no-dependencies

noinst_LIBRARIES = libstream6.a

libstream6_a_SOURCES = \
snort_stream_tcp.c \
snort_stream_tcp.h \
snort_stream_udp.c \
snort_stream_udp.h \
snort_stream_icmp.c \
snort_stream_icmp.h \
snort_stream_ip.c \
snort_stream_ip.h \
stream_paf.c \
stream_paf.h \
stream_common.c \
stream_common.h 

libstream6_a_LIBADD = \
snort_stream_tcp.o \
snort_stream_udp.o \
snort_stream_icmp.o \
snort_stream_ip.o \
stream_paf.o \
stream_common.o

INCLUDES = @INCLUDES@
