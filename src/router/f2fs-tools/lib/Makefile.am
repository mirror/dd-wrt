## Makefile.am

lib_LTLIBRARIES = libf2fs.la

libf2fs_la_SOURCES = libf2fs.c libf2fs_io.c libf2fs_zoned.c
libf2fs_la_CFLAGS = -Wall
libf2fs_la_CPPFLAGS = -I$(top_srcdir)/include
libf2fs_la_LDFLAGS = -version-info $(LIBF2FS_CURRENT):$(LIBF2FS_REVISION):$(LIBF2FS_AGE)
