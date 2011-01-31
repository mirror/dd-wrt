# Remove the # for more hardware-specific debugging programs.
# I'll need those only if you're fiddling with the guts of drivers.
noinst_PROGRAMS = delay svctor # ed dnstest znth

LDADD = $(top_builddir)/src/lib/libatm.la

delay_SOURCES = delay.c
svctor_SOURCES = svctor.c
#ed_SOURCES = ed.c
#dnstest_SOURCES = dnstest.c
#znth_SOURCES = znth.c

#EXTRA_DIST = peek.pl
EXTRA_DIST = peek.pl ed.c dnstest.c znth.c

