AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS = file_server

docdir = ${datadir}/doc/${PACKAGE}

file_server_SOURCES = file_server.c
file_server_CFLAGS = @CFLAGS@ $(AM_CFLAGS)

INCLUDES = @INCLUDES@ @extra_incl@

dist_doc_DATA = README.file_server

