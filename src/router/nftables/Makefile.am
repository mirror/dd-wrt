# This is _NOT_ the library release version, it's an API version.
# Extracted from Chapter 6 "Library interface versions" of the libtool docs.
#
# <snippet>
# Here are a set of rules to help you update your library version information:
#
# 1. Start with version information of `0:0:0' for each libtool library.
# 2. Update the version information only immediately before a public release
# of your software. More frequent updates are unnecessary, and only guarantee
# that the current interface number gets larger faster.
# 3. If the library source code has changed at all since the last update,
# then increment revision (`c:r:a' becomes `c:r+1:a').
# 4. If any interfaces have been added, removed, or changed since the last
# update, increment current, and set revision to 0.
# 5. If any interfaces have been added since the last public release, then
# increment age.
# 6. If any interfaces have been removed since the last public release, then
# set age to 0.
# </snippet>
#
libnftables_LIBVERSION = 2:0:1

###############################################################################

ACLOCAL_AMFLAGS = -I m4
AM_DISTCHECK_CONFIGURE_FLAGS = --enable-distcheck

EXTRA_DIST =
BUILT_SOURCES =
LDADD =
lib_LTLIBRARIES =
noinst_LTLIBRARIES =
sbin_PROGRAMS =
noinst_PROGRAMS =
check_PROGRAMS =
dist_man_MANS =
CLEANFILES =
DISTCLEANFILES =

###############################################################################

pkginclude_HEADERS = \
	include/nftables/libnftables.h \
	$(NULL)

noinst_HEADERS = \
	\
	include/linux/netfilter.h \
	include/linux/netfilter/nf_conntrack_common.h \
	include/linux/netfilter/nf_conntrack_tuple_common.h \
	include/linux/netfilter/nf_log.h \
	include/linux/netfilter/nf_nat.h \
	include/linux/netfilter/nf_synproxy.h \
	include/linux/netfilter/nf_tables.h \
	include/linux/netfilter/nf_tables_compat.h \
	include/linux/netfilter/nfnetlink.h \
	include/linux/netfilter/nfnetlink_hook.h \
	include/linux/netfilter/nfnetlink_osf.h \
	include/linux/netfilter_arp.h \
	include/linux/netfilter_arp/arp_tables.h \
	include/linux/netfilter_bridge.h \
	include/linux/netfilter_bridge/ebtables.h \
	include/linux/netfilter_ipv4.h \
	include/linux/netfilter_ipv4/ip_tables.h \
	include/linux/netfilter_ipv6.h \
	include/linux/netfilter_ipv6/ip6_tables.h \
	\
	include/afl++.h	\
	include/cache.h \
	include/cli.h \
	include/cmd.h \
	include/ct.h \
	include/datatype.h \
	include/dccpopt.h \
	include/erec.h \
	include/expression.h \
	include/exthdr.h \
	include/fib.h \
	include/gmputil.h \
	include/hash.h \
	include/headers.h \
	include/iface.h \
	include/intervals.h \
	include/ipopt.h \
	include/json.h \
	include/list.h \
	include/meta.h \
	include/mini-gmp.h \
	include/misspell.h \
	include/mnl.h \
	include/netlink.h \
	include/nft.h \
	include/nftables.h \
	include/numgen.h \
	include/osf.h \
	include/owner.h \
	include/parser.h \
	include/payload.h \
	include/proto.h \
	include/rt.h \
	include/rule.h \
	include/sctp_chunk.h \
	include/socket.h \
	include/statement.h \
	include/tcpopt.h \
	include/trace.h \
	include/tunnel.h \
	include/utils.h \
	include/xfrm.h \
	include/xt.h \
	\
	$(NULL)

DISTCLEANFILES += nftversion.h

###############################################################################

AM_CPPFLAGS = \
	"-I$(srcdir)/include" \
	"-DDEFAULT_INCLUDE_PATH=\"${sysconfdir}\"" \
	$(LIBMNL_CFLAGS) \
	$(LIBNFTNL_CFLAGS) \
	$(NULL)

if BUILD_DEBUG
AM_CPPFLAGS += -g -DDEBUG
endif
if BUILD_XTABLES
AM_CPPFLAGS += $(XTABLES_CFLAGS)
endif
if BUILD_MINIGMP
AM_CPPFLAGS += -DHAVE_MINIGMP
endif
if BUILD_JSON
AM_CPPFLAGS += -DHAVE_JSON
endif
if BUILD_XTABLES
AM_CPPFLAGS += -DHAVE_XTABLES
endif

AM_CFLAGS = \
	-Wall \
	\
	-Waggregate-return \
	-Wbad-function-cast \
	-Wcast-align \
	-Wdeclaration-after-statement \
	-Wformat-nonliteral \
	-Wformat-security \
	-Winit-self \
	-Wmissing-declarations \
	-Wmissing-format-attribute \
	-Wmissing-prototypes \
	-Wsign-compare \
	-Wstrict-prototypes \
	-Wundef \
	-Wunused \
	-Wwrite-strings \
	\
	$(GCC_FVISIBILITY_HIDDEN) \
	\
	-DMAKE_STAMP=$(MAKE_STAMP) \
	\
	$(NULL)

AM_YFLAGS = -d -Wno-yacc

###############################################################################

BUILT_SOURCES += src/parser_bison.h

# yacc and lex generate dirty code
noinst_LTLIBRARIES += src/libparser.la

src_libparser_la_SOURCES = \
	src/parser_bison.y \
	src/scanner.l \
	$(NULL)

src_libparser_la_CFLAGS = \
	$(AM_CFLAGS) \
	-Wno-missing-declarations \
	-Wno-missing-prototypes \
	-Wno-nested-externs \
	-Wno-redundant-decls \
	-Wno-undef \
	-Wno-unused-but-set-variable \
	$(NULL)

###############################################################################

if BUILD_MINIGMP

noinst_LTLIBRARIES += src/libminigmp.la

src_libminigmp_la_SOURCES = src/mini-gmp.c

src_libminigmp_la_CFLAGS = \
	$(AM_CFLAGS) \
	-Wno-sign-compare \
	$(NULL)

endif

###############################################################################

lib_LTLIBRARIES += src/libnftables.la

src_libnftables_la_SOURCES = \
	src/libnftables.map \
	\
	src/cache.c \
	src/cmd.c \
	src/ct.c \
	src/datatype.c \
	src/dccpopt.c \
	src/erec.c \
	src/evaluate.c \
	src/expression.c \
	src/exthdr.c \
	src/fib.c \
	src/gmputil.c \
	src/hash.c \
	src/iface.c \
	src/intervals.c \
	src/ipopt.c \
	src/libnftables.c \
	src/mergesort.c \
	src/meta.c \
	src/misspell.c \
	src/mnl.c \
	src/monitor.c \
	src/trace.c \
	src/netlink.c \
	src/netlink_delinearize.c \
	src/netlink_linearize.c \
	src/nfnl_osf.c \
	src/nftutils.c \
	src/nftutils.h \
	src/numgen.c \
	src/optimize.c \
	src/osf.c \
	src/owner.c \
	src/payload.c \
	src/preprocess.c \
	src/print.c \
	src/proto.c \
	src/rt.c \
	src/rule.c \
	src/sctp_chunk.c \
	src/segtree.c \
	src/socket.c \
	src/statement.c \
	src/tcpopt.c \
	src/tunnel.c \
	src/utils.c \
	src/xfrm.c \
	$(NULL)

src_libnftables_la_SOURCES += src/xt.c

if BUILD_JSON
src_libnftables_la_SOURCES += \
	src/json.c \
	src/parser_json.c \
	$(NULL)
endif

src_libnftables_la_LDFLAGS = \
	-version-info "${libnftables_LIBVERSION}" \
	-Wl,--version-script="$(srcdir)/src//libnftables.map" \
	$(NULL)

src_libnftables_la_LIBADD = \
	$(LIBMNL_LIBS) \
	$(LIBNFTNL_LIBS) \
	src/libparser.la \
	$(NULL)

if BUILD_MINIGMP
src_libnftables_la_LIBADD += src/libminigmp.la
endif

if BUILD_XTABLES
src_libnftables_la_LIBADD += $(XTABLES_LIBS)
endif

if BUILD_JSON
src_libnftables_la_LIBADD += $(JANSSON_LIBS)
endif

###############################################################################

sbin_PROGRAMS += src/nft

src_nft_SOURCES = src/main.c

if BUILD_CLI
src_nft_SOURCES += src/cli.c
endif

src_nft_LDADD = src/libnftables.la

###############################################################################

check_PROGRAMS += examples/nft-buffer

examples_nft_buffer_AM_CPPFLAGS = -I$(srcdir)/include
examples_nft_buffer_LDADD = src/libnftables.la

check_PROGRAMS += examples/nft-json-file

examples_nft_json_file_AM_CPPFLAGS = -I$(srcdir)/include
examples_nft_json_file_LDADD = src/libnftables.la

###############################################################################

if BUILD_AFL
noinst_PROGRAMS += tools/nft-afl

tools_nft_afl_SOURCES = tools/nft-afl.c
tools_nft_afl_LDADD = src/libnftables.la
endif

###############################################################################

if BUILD_MAN

dist_man_MANS += \
	doc/nft.8 \
	doc/libnftables-json.5 \
	doc/libnftables.3 \
	$(NULL)

A2X_OPTS_MANPAGE = \
	-L \
	--doctype manpage \
	--format manpage \
	-D "${builddir}/doc" \
	$(NULL)

ASCIIDOC_MAIN = doc/nft.txt

ASCIIDOC_INCLUDES = \
	doc/additional-commands.txt \
	doc/data-types.txt \
	doc/payload-expression.txt \
	doc/primary-expression.txt \
	doc/stateful-objects.txt \
	doc/statements.txt \
	$(NULL)

ASCIIDOCS = \
	$(ASCIIDOC_MAIN) \
	$(ASCIIDOC_INCLUDES) \
	$(NULL)

EXTRA_DIST += \
	$(ASCIIDOCS) \
	doc/libnftables-json.adoc \
	doc/libnftables.adoc \
	$(NULL)

CLEANFILES += doc/*~

doc/nft.8: $(ASCIIDOCS)
	mkdir -p ${builddir}/doc
	$(AM_V_GEN)$(A2X) $(A2X_OPTS_MANPAGE) $<

.adoc.3:
	$(AM_V_GEN)$(A2X) $(A2X_OPTS_MANPAGE) $<

.adoc.5:
	$(AM_V_GEN)$(A2X) $(A2X_OPTS_MANPAGE) $<

MAINTAINERCLEANFILES = ${dist_man_MANS}

endif

###############################################################################

dist_pkgdata_DATA = \
	files/nftables/all-in-one.nft \
	files/nftables/arp-filter.nft \
	files/nftables/bridge-filter.nft \
	files/nftables/inet-filter.nft \
	files/nftables/inet-nat.nft \
	files/nftables/ipv4-filter.nft \
	files/nftables/ipv4-mangle.nft \
	files/nftables/ipv4-nat.nft \
	files/nftables/ipv4-raw.nft \
	files/nftables/ipv6-filter.nft \
	files/nftables/ipv6-mangle.nft \
	files/nftables/ipv6-nat.nft \
	files/nftables/ipv6-raw.nft \
	files/nftables/netdev-ingress.nft \
	$(NULL)

exampledir = ${docdir}/examples

dist_example_SCRIPTS = \
	files/examples/ct_helpers.nft \
	files/examples/load_balancing.nft \
	files/examples/secmark.nft \
	files/examples/sets_and_maps.nft \
	$(NULL)

pkgsysconfdir = ${sysconfdir}/${PACKAGE}
osfdir = ${pkgsysconfdir}/osf

dist_osf_DATA = \
	files/osf/pf.os \
	$(NULL)

###############################################################################

EXTRA_DIST += \
	py/pyproject.toml \
	py/setup.cfg \
	py/setup.py \
	py/src/__init__.py \
	py/src/nftables.py \
	py/src/schema.json \
	$(NULL)

###############################################################################

EXTRA_DIST += \
	files \
	tests \
	tools/nftables.service.8 \
	tools/nftables.service.in \
	$(NULL)

if BUILD_SERVICE
CLEANFILES += tools/nftables.service
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libnftables.pc

if BUILD_SERVICE
unit_DATA = tools/nftables.service
man_MANS = tools/nftables.service.8
doc_DATA = files/nftables/main.nft

tools/nftables.service: tools/nftables.service.in ${top_builddir}/config.status
	${AM_V_GEN}${MKDIR_P} tools
	${AM_V_at}sed -e 's|@''sbindir''@|${sbindir}|g;s|@''pkgsysconfdir''@|${pkgsysconfdir}|g' <${srcdir}/tools/nftables.service.in >$@
endif

if !BUILD_DISTCHECK
TESTS = tests/build/run-tests.sh \
	tests/json_echo/run-test.py \
	tests/monitor/run-tests.sh \
	tests/py/nft-test.py \
	tests/shell/run-tests.sh
endif
