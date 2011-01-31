sbin_PROGRAMS = @atmoamd@

EXTRA_PROGRAMS = atmoamd

atmoamd_SOURCES = atmoamd.c io.c io.h oam.c oam.h crc10.c crc10.h

atmoamd_LDADD = $(top_builddir)/src/lib/libatm.la
atmoamd_DEPENDENCIES = $(zeppelin_LDADD)
