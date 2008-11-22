#ifndef __SYSDEP_H__
#define __SYSDEP_H__

/*
 * Different systems define different macros.
 * For vpnc, this list should be used as
 * reference:
 *
 * __linux__ 
 * __NetBSD__
 * __OpenBSD__
 * __FreeBSD__
 * __DragonFly__
 * __APPLE__      Darwin / MacOS X
 * __sun__        SunOS / Solaris
 * __CYGWIN__
 * __SKYOS__
 *
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#if !defined(__CYGWIN__)
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/if_ether.h>
#endif

#include "config.h"

int tun_open(char *dev, enum if_mode_enum mode);
int tun_close(int fd, char *dev);
int tun_write(int fd, unsigned char *buf, int len);
int tun_read(int fd, unsigned char *buf, int len);
int tun_get_hwaddr(int fd, char *dev, uint8_t *hwaddr);

/***************************************************************************/
#if defined(__linux__) || defined(__GLIBC__)
#include <error.h>

#define HAVE_VASPRINTF 1
#define HAVE_ASPRINTF  1
#define HAVE_ERROR     1
#define HAVE_GETLINE   1
#define HAVE_UNSETENV  1
#define HAVE_SETENV    1
#endif

/***************************************************************************/
#if defined(__NetBSD__)
#define HAVE_SA_LEN 1

#define HAVE_VASPRINTF 1
#define HAVE_ASPRINTF  1
#define HAVE_FGETLN    1
#define HAVE_UNSETENV  1
#define HAVE_SETENV    1
#endif

/***************************************************************************/
#if defined(__OpenBSD__)
#define HAVE_SA_LEN 1
#define NEED_IPLEN_FIX 1
#define NEW_TUN 1

#define HAVE_VASPRINTF 1
#define HAVE_ASPRINTF  1
#define HAVE_FGETLN    1
#define HAVE_UNSETENV  1
#define HAVE_SETENV    1
#endif

/***************************************************************************/
#if defined(__FreeBSD_kernel__)
#define HAVE_SA_LEN 1
#endif

/***************************************************************************/
#if defined(__FreeBSD__)
#define HAVE_SA_LEN 1

#define HAVE_VASPRINTF 1
#define HAVE_ASPRINTF  1
#define HAVE_FGETLN    1
#define HAVE_UNSETENV  1
#define HAVE_SETENV    1
#endif

/***************************************************************************/
#if defined(__DragonFly__)
#define HAVE_SA_LEN 1

#define HAVE_VASPRINTF 1
#define HAVE_ASPRINTF  1
#define HAVE_FGETLN    1
#define HAVE_UNSETENV  1
#define HAVE_SETENV    1
#endif

/***************************************************************************/
#if defined(__APPLE__)
#define HAVE_SA_LEN 1
#define NEED_IPLEN_FIX 1

#define HAVE_VASPRINTF 1
#define HAVE_ASPRINTF  1
#define HAVE_FGETLN    1
#define HAVE_UNSETENV  1
#define HAVE_SETENV    1
#endif

/***************************************************************************/
#if defined(__sun__)
#define NEED_IPLEN_FIX 1

#ifndef IPPROTO_ESP
#define IPPROTO_ESP 50
#endif

#define getpass(prompt) getpassphrase(prompt)

/* where is this defined? */
#include <sys/socket.h>
const char *inet_ntop(int af, const void *src, char *dst, size_t cnt);
#endif
/***************************************************************************/
#if defined (__SKYOS__)
#define HAVE_UNSETENV  1

#ifndef IPPROTO_ENCAP
#define IPPROTO_ENCAP 4
#endif

#ifndef IPPROTO_ESP
#define IPPROTO_ESP 50
#endif
#endif
/***************************************************************************/
#if defined (__CYGWIN__)
#define HAVE_VASPRINTF 1
#define HAVE_ASPRINTF  1
#define HAVE_GETLINE   1
#define HAVE_FGETLN    1
#define HAVE_UNSETENV  1
#define HAVE_SETENV    1

#ifndef IPPROTO_ESP
#define IPPROTO_ESP 50
#endif

#ifndef IPPROTO_ENCAP
#define IPPROTO_ENCAP 4
#endif

#ifdef IFNAMSIZ
#undef IFNAMSIZ
#endif
#define IFNAMSIZ 256

/*
 * At the moment the Cygwin environment does not have header files
 * for raw ethernet access, hence we need to define here what
 * is usually found in net/ethernet.h and netinet/if_ether.h
 */

#define ETH_ALEN 6

/* Ethernet header */
struct ether_header
{
	unsigned char ether_dhost[ETH_ALEN]; /* destination eth addr */
	unsigned char ether_shost[ETH_ALEN]; /* source ether addr    */
	unsigned short ether_type;           /* packet type ID field */
} __attribute__ ((__packed__));

#define ETHERTYPE_IP  0x0800 /* IP  */
#define ETHERTYPE_ARP 0x0806 /* ARP */

/* Common ARP header */
struct arphdr {
	unsigned short ar_hrd; /* format of hardware address   */
	unsigned short ar_pro; /* format of protocol address   */
	unsigned char  ar_hln; /* length of hardware address   */
	unsigned char  ar_pln; /* length of protocol address   */
	unsigned short ar_op;  /* ARP opcode (command)         */
};

/* Ethernet ARP header */
struct ether_arp {
	struct arphdr ea_hdr;            /* fixed-size header */
	unsigned char arp_sha[ETH_ALEN]; /* sender hardware address */
	unsigned char arp_spa[4];        /* sender protocol address */
	unsigned char arp_tha[ETH_ALEN]; /* target hardware address */
	unsigned char arp_tpa[4];        /* target protocol address */
};
#define arp_hrd ea_hdr.ar_hrd
#define arp_pro ea_hdr.ar_pro
#define arp_hln ea_hdr.ar_hln
#define arp_pln ea_hdr.ar_pln
#define arp_op  ea_hdr.ar_op

#define ARPHRD_ETHER 1 /* Ethernet */

#define ARPOP_REQUEST 1 /* ARP request */
#define ARPOP_REPLY   2 /* ARP reply   */

#endif
/***************************************************************************/


#ifndef IPDEFTTL
#define IPDEFTTL 64 /* default ttl, from RFC 1340 */
#endif

#ifndef IPPROTO_IPIP
#define IPPROTO_IPIP IPPROTO_ENCAP
#endif

#ifndef ETH_HLEN
#define ETH_HLEN (sizeof(struct ether_header))
#endif

#ifndef ETH_ALEN
#define ETH_ALEN (sizeof(struct ether_addr))
#endif

#ifndef HAVE_ERROR
extern void error(int fd, int errorno, const char *fmt, ...);
#endif
#ifndef HAVE_GETLINE
extern int getline(char **line, size_t * length, FILE * stream);
#endif
#ifndef HAVE_VASPRINTF
#include <stdarg.h>
extern int vasprintf(char **strp, const char *fmt, va_list ap);
#endif
#ifndef HAVE_ASPRINTF
extern int asprintf(char **strp, const char *fmt, ...);
#endif
#ifndef HAVE_SETENV
extern int setenv(const char *name, const char *value, int overwrite);
#endif
#ifndef HAVE_UNSETENV
extern int unsetenv(const char *name);
#endif


#endif
