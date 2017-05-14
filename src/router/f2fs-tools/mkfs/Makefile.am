## Makefile.am

AM_CPPFLAGS = ${libuuid_CFLAGS} -I$(top_srcdir)/include
AM_CFLAGS = -Wall -DWITH_BLKDISCARD
sbin_PROGRAMS = mkfs.f2fs
mkfs_f2fs_SOURCES = f2fs_format_main.c f2fs_format.c f2fs_format_utils.c f2fs_format_utils.h $(top_srcdir)/include/f2fs_fs.h
mkfs_f2fs_LDADD = ${libuuid_LIBS} $(top_builddir)/lib/libf2fs.la

lib_LTLIBRARIES = libf2fs_format.la
libf2fs_format_la_SOURCES = f2fs_format_main.c f2fs_format.c f2fs_format_utils.c
libf2fs_format_la_CFLAGS = -DWITH_BLKDISCARD
libf2fs_format_la_CPPFLAGS = -I$(top_srcdir)/include
libf2fs_format_la_LDFLAGS = -luuid -L$(top_srcdir)/lib -lf2fs \
	-version-info $(FMT_CURRENT):$(FMT_REVISION):$(FMT_AGE)
