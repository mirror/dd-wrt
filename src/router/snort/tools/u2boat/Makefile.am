AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS = u2boat

docdir = ${datadir}/doc/${PACKAGE}

u2boat_SOURCES = u2boat.c u2boat.h
u2boat_CFLAGS = @CFLAGS@ $(AM_CFLAGS)
u2boat_LDADD = -lpcap

INCLUDES = @INCLUDES@ @extra_incl@

dist_doc_DATA = README.u2boat

