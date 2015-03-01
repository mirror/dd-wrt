AUTOMAKE_OPTIONS=foreign no-dependencies

#ACLOCAL_AMFLAGS = -I m4

#INCLUDES = -I ../include -I ../api

noinst_LIBRARIES = liboutput.a
#liboutput_a_LIBADD = ../libs/liboutput_lib.a
liboutput_a_CFLAGS = -fPIC 
liboutput_a_SOURCES = \
output_base.c \
output_plugin.c \
output.h \
output_api.h \
output_common.h \
output_lib.h

INCLUDES = @INCLUDES@