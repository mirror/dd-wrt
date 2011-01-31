SUBDIRS = asn1

INCLUDES = -I$(srcdir)/../ilmid/asn1

sbin_PROGRAMS = ilmid ilmidiag

ilmid_SOURCES = rfc1157_snmp.c rfc1157_snmp.h rfc1155_smi.c rfc1155_smi.h \
			util.c util.h io.c io.h message.c message.h \
			atmf_uni.c atmf_uni.h mib.c mib.h \
			sysgroup.c sysgroup.h ilmid.c ilmid.h \
			wait.c wait.h states.c states.h actions.c actions.h
ilmid_LDADD = $(top_builddir)/src/lib/libatm.la \
		$(top_builddir)/src/ilmid/asn1/libasn1.a
ilmid_DEPENDENCIES = $(ilmid_LDADD)

ilmidiag_SOURCES = ilmidiag.c
ilmidiag_LDADD = $(top_builddir)/src/lib/libatm.la
ilmidiag_DEPENDENCIES = $(ilmid_LDADD)

man_MANS = ilmid.8

EXTRA_DIST = COPYRIGHT ilmid.8

