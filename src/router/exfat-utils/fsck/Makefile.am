AM_CFLAGS = -Wall -Wextra -include $(top_builddir)/config.h -I$(top_srcdir)/include -fno-common
fsck_exfat_LDADD = $(top_builddir)/lib/libexfat.a

sbin_PROGRAMS = fsck.exfat

fsck_exfat_SOURCES = fsck.c repair.c fsck.h repair.h
