AM_CPPFLAGS = -I$(top_srcdir)/src/include/

bin_PROGRAMS = OpenDPI_demo
OpenDPI_demo_SOURCES = linux_compat.h \
						OpenDPI_demo.c

OpenDPI_demo_LDADD = $(top_builddir)/src/lib/libopendpi.la -lpcap
