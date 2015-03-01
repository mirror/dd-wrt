AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS = snort_control snort_dump_packets_control

docdir = ${datadir}/doc/${PACKAGE}

snort_control_SOURCES = sfcontrol.c
snort_control_CFLAGS = @CFLAGS@ $(AM_CFLAGS)

snort_dump_packets_control_SOURCES = snort_dump_packets.c
snort_dump_packets_control_CFLAGS = @CFLAGS@ $(AM_CFLAGS)

INCLUDES = @INCLUDES@ @extra_incl@

dist_doc_DATA = README.snort_control README.snort_dump_packets_control

