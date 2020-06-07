noinst_LIBRARIES = libnatpmp.a

AM_CFLAGS = @PTHREAD_CFLAGS@ -DENABLE_STRNATPMPERR

libnatpmp_a_SOURCES = \
    getgateway.c \
    natpmp.c \
    wingettimeofday.c

noinst_HEADERS = \
    getgateway.h \
    natpmp.h \
    natpmp_declspec.h \
    wingettimeofday.h

EXTRA_DIST = \
    CMakeLists.txt \
    README \
    LICENSE
