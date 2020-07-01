AM_CFLAGS = -Wall -include $(top_builddir)/config.h -I$(top_srcdir)/include -fno-common
mkfs_exfat_LDADD = $(top_builddir)/lib/libexfat.a

sbin_PROGRAMS = mkfs.exfat

mkfs_exfat_SOURCES = mkfs.c upcase.c mkfs.h
