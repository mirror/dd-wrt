#
# Makefile for the 802.11 NET80211 module.
#


COMPAT=	drivers/net/wireless/net80211/compat

EXTRA_CFLAGS+= -DATH_SUPERG_FF=1
EXTRA_CFLAGS+= -DATH_SUPERG_DYNTURBO=1
EXTRA_CFLAGS+= -DATH_TURBO_SCAN=1

EXTRA_CFLAGS+=	-include ${COMPAT}/compat.h -I${COMPAT} -I${src}/.. -DAH_BYTE_ORDER=AH_LITTLE_ENDIAN

#
# There are two authenticator mechanisms: an in-kernel implementation
# (wlan_auth+wlan_radius) and an external implementation (wlan_xauth) that
# requires a user process to manage the authentication process.  By default
# the external authenticator is used.  ieee80211_proto.c has a table of module
# names that defines the default module to auto-load for each authentication
# scheme; to get the in-kernel authenticator by default modify it to load
# wlan_auth instead of wlan_xauth or manually load wlan_auth prior to use.
#
MOD_AUTH	:= wlan_xauth.o
#
# Scanning policy is split into modules.  The default policy modules
# separate station-mode scanning support from ap-mode scanning support.
#
MOD_SCAN	:= wlan_scan_sta.o wlan_scan_ap.o

obj-$(CONFIG_NET80211) += wlan.o wlan_wep.o wlan_tkip.o wlan_ccmp.o wlan_acl.o \
		   $(MOD_AUTH) $(MOD_SCAN)

wlan-objs	:= if_media.o \
		   ieee80211.o ieee80211_beacon.o ieee80211_crypto.o \
		   ieee80211_crypto_none.o ieee80211_input.o ieee80211_node.o \
		   ieee80211_output.o ieee80211_proto.o ieee80211_power.o \
		   ieee80211_scan.o ieee80211_wireless.o ieee80211_linux.o \
		   ieee80211_monitor.o
wlan_wep-objs	:= ieee80211_crypto_wep.o
wlan_tkip-objs	:= ieee80211_crypto_tkip.o
wlan_ccmp-objs	:= ieee80211_crypto_ccmp.o
wlan_xauth-objs	:= ieee80211_xauth.o
wlan_acl-objs	:= ieee80211_acl.o
wlan_scan_sta-objs := ieee80211_scan_sta.o
wlan_scan_ap-objs  := ieee80211_scan_ap.o



