AM_CFLAGS = -I$(top_srcdir)/include $(GLIB_CFLAGS) $(LIBNL_CFLAGS) -fno-common
LIBS = $(GLIB_LIBS)
ksmbd_control_LDADD = $(top_builddir)/lib/libksmbdtools.a

sbin_PROGRAMS = ksmbd.control

ksmbd_control_SOURCES = control.c
