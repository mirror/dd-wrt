AUTOMAKE_OPTIONS = foreign

ACLOCAL_AMFLAGS = -I m4

lib_LTLIBRARIES = libsfbpf.la

include_HEADERS = sfbpf.h sfbpf_dlt.h

libsfbpf_la_SOURCES = \
arcnet.h \
atmuni31.h \
bittypes.h \
ethertype.h \
gencode.h \
ieee80211.h \
IP6_misc.h \
ipnet.h \
llc.h \
namedb.h \
nlpid.h \
ppp.h \
sfbpf.h \
sfbpf_dlt.h \
sf-redefines.h \
sf_bpf_filter.c \
sf_bpf_printer.c \
sf_gencode.c \
sf_nametoaddr.c \
sf_optimize.c \
sfbpf-int.c \
sll.h \
sunatmpos.h

EXTRA_DIST = grammar.y scanner.l \
win32-stdinc.h \
sfbpf-int.h \
sfbpf-int.c \
runlex.sh

nodist_libsfbpf_la_SOURCES = sf_grammar.c sf_scanner.c tokdefs.h

libsfbpf_la_CFLAGS = -Dyylval=sfbpf_lval
libsfbpf_la_LDFLAGS = -version-info 0:1:0 @XCCFLAGS@

# use of $@ and $< here is a GNU idiom that borks BSD
sf_scanner.c: $(srcdir)/scanner.l
	@rm -f $(srcdir)/sf_scanner.c
	$(srcdir)/runlex.sh $(V_LEX) -osf_scanner.c $(srcdir)/scanner.l

sf_scanner.o: sf_scanner.c tokdefs.h

tokdefs.h: sf_grammar.c
sf_grammar.c: $(srcdir)/grammar.y
	@rm -f sf_grammar.c tokdefs.h
	$(V_YACC) -d $(srcdir)/grammar.y
	mv y.tab.c sf_grammar.c
	mv y.tab.h tokdefs.h

CLEANFILES = sf_scanner.c sf_grammar.c tokdefs.h sf_scanner.h
