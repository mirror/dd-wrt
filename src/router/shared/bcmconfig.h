/*
 * Automatically generated C config: don't edit
 */

/*
 * Base Features
 */
#define __CONFIG_RC__ 1
#define __CONFIG_NVRAM__ 1
#define __CONFIG_SHARED__ 1
#define __CONFIG_BUSYBOX__ 1
#define __CONFIG_LIBBCM__ 1
#define __CONFIG_WLCONF__ 1
#define __CONFIG_BRIDGE__ 1
#undef __CONFIG_VLAN__
#define __CONFIG_HTTPD__ 1
#define __CONFIG_WWW__ 1
#undef __CONFIG_OPENSSL__
#undef __CONFIG_MATRIXSSL__
#define __CONFIG_NOSSL__ 1
#define __CONFIG_EBTABLES__ 1
#undef __CONFIG_NOCAT__
#undef __CONFIG_HOTSPOT__
#define __CONFIG_VPND__ 1
#define __CONFIG_PPTPD__ 1

/*
 * IPSec
 */
#undef __CONFIG_IPSEC__
#undef __CONFIG_RADIUSPLUGIN__
#undef __CONFIG_PPPSTATS__
#define __CONFIG_L7__ 1
#undef __CONFIG_ZEBRA__
#define __CONFIG_BIRD__ 1
#undef __CONFIG_WSHAPER__
#define __CONFIG_SVQOS__ 1
#undef __CONFIG_FROTTLE__
#undef __CONFIG_PARPROUTED__
#define __CONFIG_WOL__ 1
#undef __CONFIG_SNMP__
#undef __CONFIG_RADVD__
#undef __CONFIG_RFLOW__
#define __CONFIG_NETCONF__ 1
#undef __CONFIG_IPTABLES__
#define __CONFIG_LIBIPT__ 1
#undef __CONFIG_GLIBC__
#define __CONFIG_UCLIBC__ 1
#define __CONFIG_LIBOPT__ 1
#define __CONFIG_ADM6996__ 1

/*
 * GUI Styles
 */
#define __CONFIG_KROMO__ 1
#define __CONFIG_XIRIAN__ 1
#define __CONFIG_BRAINSLAYER__ 1
#define __CONFIG_WIKAR__ 1
#undef __CONFIG_FREEBIRD__
#undef __CONFIG_DLS__
#undef __CONFIG_OMNI__
#undef __CONFIG_WTS__
#undef __CONFIG_FBN__

/*
 * Special DD-WRT Features
 */
#undef __CONFIG_MSSID__
#undef __CONFIG_MADWIFI__
#undef __CONFIG_E2FSPROGS__
#undef __CONFIG_NOTRANS__
#undef __CONFIG_AQOS__
#undef __CONFIG_ROBOCFG__
#undef __CONFIG_OPENVPN__
#define __CONFIG_TELNET__ 1
#undef __CONFIG_MASKMAC__
#undef __CONFIG_MMC__
#undef __CONFIG_SAMBA__
#undef __CONFIG_KAID__
#undef __CONFIG_ZEROIP__
#define __CONFIG_RADAUTH__ 1
#define __CONFIG_DHCPFORWARD__ 1
#undef __CONFIG_DHCPRELAY__

/*
 * SIPATH
 */
#undef __CONFIG_RTPPROXY__
#undef __CONFIG_SER__

/*
 * Options
 */
#define __CONFIG_VENDOR__ "broadcom"
#define __CONFIG_UDHCPD__ 1
#define __CONFIG_PPP__ 1
#undef __CONFIG_PPPOESERVER__
#define __CONFIG_PPPOERELAY__ 1
#undef __CONFIG_PPPOESNIFF__
#define __CONFIG_UPNP__ 1
#define __CONFIG_NAS__ 1
#undef __CONFIG_SES__
#define __CONFIG_DROPBEAR_SSHD__ 1
#define __CONFIG_BOOT_WAIT_ON__ 1
#define __CONFIG_IPROUTE2__ 1
#define __CONFIG_NTP__ 1
#define __CONFIG_DNSMASQ__ 1
#define __CONFIG_UTILS__ 1
#undef __CONFIG_ETC__
#define __CONFIG_SPUTNIK_APD__ 1
#define __CONFIG_BCMWPA2__ 1

/*
 * Additional C libraries
 */
#define __CONFIG_LIBCRYPT__ 1
#undef __CONFIG_LIBPCAP__
#define __CONFIG_LIBDL__ 1
#define __CONFIG_LIBM__ 1
#define __CONFIG_LIBNSL__ 1
#define __CONFIG_LIBPTHREAD__ 1
#define __CONFIG_LIBRESOLV__ 1
#define __CONFIG_LIBUTIL__ 1

/*
 * Environment
 */
#define __PLATFORM__ "mipsel"
#define __LINUXDIR__ "$(SRCBASE)/linux/linux"
#define __LIBDIR__ "$(TOOLCHAIN)/lib"
#define __USRLIBDIR__ "$(TOOLCHAIN)/usr/lib"

/*
 * Internal Options
 */
