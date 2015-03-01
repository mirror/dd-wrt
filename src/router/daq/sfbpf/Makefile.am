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

nodist_libsfbpf_la_SOURCES = ${builddir}/sf_grammar.c ${builddir}/sf_scanner.c ${builddir}/tokdefs.h

libsfbpf_la_CFLAGS = -Dyylval=sfbpf_lval
libsfbpf_la_LDFLAGS = -version-info 0:1:0 @XCCFLAGS@

# use of $@ and $< here is a GNU idiom that borks BSD
${builddir}/sf_scanner.c: ${srcdir}/scanner.l ${builddir}/tokdefs.h
	@rm -f ${builddir}/sf_scanner.c
	${srcdir}/runlex.sh $(V_LEX) -osf_scanner.c ${srcdir}/scanner.l

${builddir}/sf_grammar.c: ${srcdir}/grammar.y
	@rm -f ${builddir}/sf_grammar.c ${builddir}/tokdefs.h
	$(V_YACC) -d ${srcdir}/grammar.y
	mv y.tab.c sf_grammar.c
	mv y.tab.h tokdefs.h

${builddir}/tokdefs.h: ${builddir}/sf_grammar.c ; @true

CLEANFILES = ${builddir}/sf_scanner.c ${builddir}/sf_grammar.c ${builddir}/tokdefs.h ${builddir}/sf_scanner.h
