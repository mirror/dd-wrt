# -*- Makefile -*-

AM_CFLAGS        = ${regular_CFLAGS}
AM_CPPFLAGS      = ${regular_CPPFLAGS} -I${top_builddir}/include -I${top_srcdir}/include -I${top_srcdir} ${kinclude_CPPFLAGS} ${libmnl_CFLAGS} ${libnftnl_CFLAGS} ${libnetfilter_conntrack_CFLAGS}

BUILT_SOURCES =

xtables_legacy_multi_SOURCES  = xtables-legacy-multi.c iptables-xml.c
xtables_legacy_multi_CFLAGS   = ${AM_CFLAGS}
xtables_legacy_multi_LDADD    = ../extensions/libext.a
if ENABLE_STATIC
xtables_legacy_multi_CFLAGS  += -DALL_INCLUSIVE
endif
if ENABLE_IPV4
xtables_legacy_multi_SOURCES += iptables-standalone.c iptables.c
xtables_legacy_multi_CFLAGS  += -DENABLE_IPV4
xtables_legacy_multi_LDADD   += ../libiptc/libip4tc.la ../extensions/libext4.a
endif
if ENABLE_IPV6
xtables_legacy_multi_SOURCES += ip6tables-standalone.c ip6tables.c
xtables_legacy_multi_CFLAGS  += -DENABLE_IPV6
xtables_legacy_multi_LDADD   += ../libiptc/libip6tc.la ../extensions/libext6.a
endif
xtables_legacy_multi_SOURCES += xshared.c iptables-restore.c iptables-save.c
xtables_legacy_multi_LDADD   += ../libxtables/libxtables.la -lm

# iptables using nf_tables api
if ENABLE_NFTABLES
xtables_nft_multi_SOURCES  = xtables-nft-multi.c iptables-xml.c
xtables_nft_multi_CFLAGS   = ${AM_CFLAGS}
xtables_nft_multi_LDADD    = ../extensions/libext.a ../extensions/libext_ebt.a
if ENABLE_STATIC
xtables_nft_multi_CFLAGS  += -DALL_INCLUSIVE
endif
xtables_nft_multi_CFLAGS  += -DENABLE_NFTABLES -DENABLE_IPV4 -DENABLE_IPV6
xtables_nft_multi_SOURCES += xtables-save.c xtables-restore.c \
				xtables-standalone.c xtables.c nft.c \
				nft-shared.c nft-ipv4.c nft-ipv6.c nft-arp.c \
				xtables-monitor.c nft-cache.c \
				xtables-arp-standalone.c xtables-arp.c \
				nft-bridge.c nft-cmd.c \
				xtables-eb-standalone.c xtables-eb.c \
				xtables-eb-translate.c \
				xtables-translate.c
xtables_nft_multi_LDADD   += ${libmnl_LIBS} ${libnftnl_LIBS} ${libnetfilter_conntrack_LIBS} ../extensions/libext4.a ../extensions/libext6.a ../extensions/libext_ebt.a ../extensions/libext_arpt.a
xtables_nft_multi_SOURCES += xshared.c
xtables_nft_multi_LDADD   += ../libxtables/libxtables.la -lm
endif

sbin_PROGRAMS    = xtables-legacy-multi
if ENABLE_NFTABLES
sbin_PROGRAMS	+= xtables-nft-multi
endif
man_MANS         = iptables.8 iptables-restore.8 iptables-save.8 \
                   iptables-xml.1 ip6tables.8 ip6tables-restore.8 \
                   ip6tables-save.8 iptables-extensions.8 \
                   iptables-apply.8 ip6tables-apply.8

sbin_SCRIPT      = iptables-apply

if ENABLE_NFTABLES
man_MANS	+= xtables-nft.8 xtables-translate.8 xtables-legacy.8 \
                   iptables-translate.8 ip6tables-translate.8 \
		   iptables-restore-translate.8 ip6tables-restore-translate.8 \
                   xtables-monitor.8 \
                   arptables-nft.8 arptables-nft-restore.8 arptables-nft-save.8 \
                   ebtables-nft.8
endif
CLEANFILES       = iptables.8 xtables-monitor.8 \
		   iptables-translate.8 ip6tables-translate.8

vx_bin_links   = iptables-xml
if ENABLE_IPV4
v4_sbin_links  = iptables-legacy iptables-legacy-restore iptables-legacy-save \
		 iptables iptables-restore iptables-save
endif
if ENABLE_IPV6
v6_sbin_links  = ip6tables-legacy ip6tables-legacy-restore ip6tables-legacy-save \
		 ip6tables ip6tables-restore ip6tables-save
endif
if ENABLE_NFTABLES
x_sbin_links  = iptables-nft iptables-nft-restore iptables-nft-save \
		ip6tables-nft ip6tables-nft-restore ip6tables-nft-save \
		iptables-translate ip6tables-translate \
		iptables-restore-translate ip6tables-restore-translate \
		arptables-nft arptables \
		arptables-nft-restore arptables-restore \
		arptables-nft-save arptables-save \
		ebtables-nft ebtables \
		ebtables-nft-restore ebtables-restore \
		ebtables-nft-save ebtables-save \
		xtables-monitor
endif

iptables-extensions.8: iptables-extensions.8.tmpl ../extensions/matches.man ../extensions/targets.man
	${AM_VERBOSE_GEN} sed \
		-e '/@MATCH@/ r ../extensions/matches.man' \
		-e '/@TARGET@/ r ../extensions/targets.man' $< >$@;

iptables-translate.8 ip6tables-translate.8 iptables-restore-translate.8 ip6tables-restore-translate.8:
	${AM_VERBOSE_GEN} echo '.so man8/xtables-translate.8' >$@

pkgconfig_DATA = xtables.pc

# Using if..fi avoids an ugly "error (ignored)" message :)
install-exec-hook:
	-if test -z "${DESTDIR}"; then /sbin/ldconfig; fi;
	${INSTALL} -dm0755 "${DESTDIR}${bindir}";
	for i in ${vx_bin_links}; do ${LN_S} -f "${sbindir}/xtables-legacy-multi" "${DESTDIR}${bindir}/$$i"; done;
	for i in ${v4_sbin_links}; do ${LN_S} -f xtables-legacy-multi "${DESTDIR}${sbindir}/$$i"; done;
	for i in ${v6_sbin_links}; do ${LN_S} -f xtables-legacy-multi "${DESTDIR}${sbindir}/$$i"; done;
	for i in ${x_sbin_links}; do ${LN_S} -f xtables-nft-multi "${DESTDIR}${sbindir}/$$i"; done;
	${LN_S} -f iptables-apply "${DESTDIR}${sbindir}/ip6tables-apply"
