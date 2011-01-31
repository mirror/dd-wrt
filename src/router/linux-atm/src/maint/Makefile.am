BOOTPGMS=atmaddr esi
SYSPGMS=atmloop atmtcp enitune zntune hediag # nstune
USRPGMS=atmdiag atmdump sonetdiag saaldump

INCLUDES=-I$(srcdir)/../q2931 -I$(srcdir)/../saal -I.
LDADD = $(top_builddir)/src/lib/libatm.la

sbin_PROGRAMS = $(BOOTPGMS) $(SYSPGMS)
bin_PROGRAMS = $(USRPGMS)

atmaddr_SOURCES = atmaddr.c
esi_SOURCES = esi.c

atmloop_SOURCES = atmloop.c
atmtcp_SOURCES = atmtcp.c
enitune_SOURCES = enitune.c
zntune_SOURCES = zntune.c
#nstune_SOURCES = nstune.c

atmdiag_SOURCES = atmdiag.c
atmdump_SOURCES = atmdump.c
sonetdiag_SOURCES = sonetdiag.c
saaldump_SOURCES = saaldump.c
saaldump_LDADD = $(top_builddir)/src/q2931/qd.dump.o $(top_builddir)/src/saal/libsaal.a $(LDADD)
saaldump_DEPENDENCIES = $(saaldump_LDADD)

man_MANS = atmaddr.8 atmdiag.8 atmdump.8 atmloop.8 atmtcp.8 esi.8

#EXTRA_DIST = $(man_MANS) README.nstune
EXTRA_DIST = $(man_MANS) README.nstune nstune.c

