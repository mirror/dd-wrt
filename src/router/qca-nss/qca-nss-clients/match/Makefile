# Makefile for match
ccflags-y := -I$(obj)/../exports -I$(obj)/.. \
	-DNSS_CLIENT_BUILD_ID="$(BUILD_ID)" -DNSS_MATCH_DEBUG_LEVEL=2 -Wall -Werror

obj-m += qca-nss-match.o
qca-nss-match-objs := nss_match.o
qca-nss-match-objs += nss_match_cmd.o
qca-nss-match-objs += nss_match_db.o
qca-nss-match-objs += nss_match_l2.o
qca-nss-match-objs += nss_match_stats.o
qca-nss-match-objs += nss_match_vow.o
