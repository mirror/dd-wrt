## Makefile.am

AM_CPPFLAGS = ${libuuid_CFLAGS} -I$(top_srcdir)/include
AM_CFLAGS = -Wall
sbin_PROGRAMS = fsck.f2fs
fsck_f2fs_SOURCES = main.c fsck.c dump.c mount.c defrag.c f2fs.h fsck.h $(top_srcdir)/include/f2fs_fs.h	\
		resize.c										\
		node.c segment.c dir.c sload.c xattr.c
fsck_f2fs_LDADD = ${libselinux_LIBS} ${libuuid_LIBS} $(top_builddir)/lib/libf2fs.la

install-data-hook:
	ln -sf fsck.f2fs $(DESTDIR)/$(sbindir)/dump.f2fs
	ln -sf fsck.f2fs $(DESTDIR)/$(sbindir)/defrag.f2fs
	ln -sf fsck.f2fs $(DESTDIR)/$(sbindir)/resize.f2fs
	ln -sf fsck.f2fs $(DESTDIR)/$(sbindir)/sload.f2fs
