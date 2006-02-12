AM_CFLAGS = @CFLAGS@ @GLIB_CFLAGS@ -Wall
LIBS = @LIBS@ @GLIB_LIBS@ 
sbin_PROGRAMS = splashd @GHTTPD_PROGRAM@
EXTRA_PROGRAMS = ghttpd
splashd_SOURCES = splashd.c
EXTRA_splashd_SOURCES = open.c passive.c base64.c
splashd_LDADD = @SPLASHD_MODE@ libsplash.a 
ghttpd_SOURCES = ghttpd.c
ghttpd_LDADD = libsplash.a
noinst_LIBRARIES = libsplash.a
noinst_HEADERS = conf.h firewall.h gateway.h http.h mime.h util.h
libsplash_a_SOURCES = \
    conf.c defaults.c firewall.c http.c \
    util.c gateway.c linux.c
