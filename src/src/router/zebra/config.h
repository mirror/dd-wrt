/* config.h.  Generated automatically by configure.  */
/* config.h.in.  Generated automatically from configure.in by autoheader.  */

/* Define if on AIX 3.
   System headers sometimes define this.
   We just want to avoid a redefinition error message.  */
#ifndef _ALL_SOURCE
/* #undef _ALL_SOURCE */
#endif

/* Define to empty if the keyword does not work.  */
/* #undef const */

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX).  */
/* #undef WORDS_BIGENDIAN */

/* Solaris on x86. */
/* #undef SOLARIS_X86 */

/* Define if host is GNU/Linux */
#define GNU_LINUX 1

/* Define if you have the AF_ROUTE socket.  */
/* #undef HAVE_AF_ROUTE */

/* Define if you have the inet_aton function.  */
#define HAVE_INET_ATON 1

/* Define if you have the inet_ntop function.  */
#define HAVE_INET_NTOP 1

/* Define if you have the inet_pton function.  */
#define HAVE_INET_PTON 1

/* Define if you have ipv6 stack.  */
/* #undef HAVE_IPV6 */

/* whether system has GNU regex */
#define HAVE_GNU_REGEX 1

/* whether system has SNMP library */
/* #undef HAVE_SNMP */

/* whether sockaddr has a sa_len field */
/* #undef HAVE_SA_LEN */

/* whether sockaddr_in has a sin_len field */
/* #undef HAVE_SIN_LEN */

/* whether sockaddr_un has a sun_len field */
/* #undef HAVE_SUN_LEN */

/* whether sockaddr_in6 has a sin6_scope_id field */
/* #undef HAVE_SIN6_SCOPE_ID */

/* Define if there is socklen_t. */
#define HAVE_SOCKLEN_T 1

/* Define if there is sockaddr_dl structure. */
/* #undef HAVE_SOCKADDR_DL */

/* Define if there is ifaliasreq structure. */
/* #undef HAVE_IFALIASREQ */

/* Define if there is in6_aliasreq structure. */
/* #undef HAVE_IN6_ALIASREQ */

/* Define if there is rt_addrinfo structure. */
/* #undef HAVE_RT_ADDRINFO */

/* Define if there is in_pktinfo structure. */
#define HAVE_INPKTINFO 1

/* Define if /proc/net/dev exists. */
#define HAVE_PROC_NET_DEV 1

/* Define if /proc/net/if_inet6 exists. */
/* #undef HAVE_PROC_NET_IF_INET6 */

/* Define if NET_RT_IFLIST exists in sys/socket.h. */
/* #undef HAVE_NET_RT_IFLIST */

/* Define if you have INRIA ipv6 stack.  */
/* #undef INRIA_IPV6 */

/* Define if you have KAME project ipv6 stack.  */
/* #undef KAME */

/* Define if you have Linux ipv6 stack.  */
/* #undef LINUX_IPV6 */

/* Define if you have NRL ipv6 stack.  */
/* #undef NRL */

/* Define if you have BSDI NRL IPv6 stack. */
/* #undef BSDI_NRL */

/* Define if one-vty option is specified. */
/* #undef VTYSH */

/* Define if interface aliases don't have distinct indeces */
/* #undef HAVE_BROKEN_ALIASES */

/* Define if disable-bgp-announce option is specified. */
/* #undef DISABLE_BGP_ANNOUNCE */

/* PAM support */
/* #undef USE_PAM */

/* TCP/IP communication between zebra and protocol daemon. */
/* #undef HAVE_TCP_ZEBRA */

/* OSPF NSSA option */
/* #undef HAVE_NSSA */

/* Linux netlink. */
#define HAVE_NETLINK 1

/* PATHS */
#define PATH_ZEBRA_PID "/var/run/zebra.pid"
#define PATH_RIPD_PID "/var/run/ripd.pid"
#define PATH_RIPNGD_PID "/var/run/ripngd.pid"
#define PATH_BGPD_PID "/var/run/bgpd.pid"
#define PATH_OSPFD_PID "/var/run/ospfd.pid"
#define PATH_OSPF6D_PID "/var/run/ospf6d.pid"

/* Define if Solaris */
/* #undef SUNOS_5 */

/* Define if FreeBSD 3.2 */
/* #undef FREEBSD_32 */

/* Define if OpenBSD */
/* #undef OPEN_BSD */

#ifdef HAVE_IPV6
#ifdef KAME
#ifndef INET6
#define INET6
#endif /* INET6 */
#endif /* KAME */
#endif /* HAVE_IPV6 */

#ifdef SUNOS_5
typedef unsigned int u_int32_t; 
typedef unsigned short u_int16_t; 
typedef unsigned short u_int8_t; 
#endif /* SUNOS_5 */

#ifndef HAVE_SOCKLEN_T
typedef int socklen_t;
#endif /* HAVE_SOCKLEN_T */

/* Define if you have the bcopy function.  */
#define HAVE_BCOPY 1

/* Define if you have the bzero function.  */
#define HAVE_BZERO 1

/* Define if you have the daemon function.  */
#define HAVE_DAEMON 1

/* Define if you have the getaddrinfo function.  */
#define HAVE_GETADDRINFO 1

/* Define if you have the getifaddrs function.  */
/* #undef HAVE_GETIFADDRS */

/* Define if you have the if_indextoname function.  */
#define HAVE_IF_INDEXTONAME 1

/* Define if you have the if_nametoindex function.  */
#define HAVE_IF_NAMETOINDEX 1

/* Define if you have the inet_aton function.  */
#define HAVE_INET_ATON 1

/* Define if you have the setproctitle function.  */
/* #undef HAVE_SETPROCTITLE */

/* Define if you have the snprintf function.  */
#define HAVE_SNPRINTF 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

/* Define if you have the strlcat function.  */
/* #undef HAVE_STRLCAT */

/* Define if you have the strlcpy function.  */
/* #undef HAVE_STRLCPY */

/* Define if you have the vsnprintf function.  */
#define HAVE_VSNPRINTF 1

/* Define if you have the <asm/types.h> header file.  */
#define HAVE_ASM_TYPES_H 1

/* Define if you have the <inet/nd.h> header file.  */
/* #undef HAVE_INET_ND_H */

/* Define if you have the <kvm.h> header file.  */
/* #undef HAVE_KVM_H */

/* Define if you have the <libutil.h> header file.  */
/* #undef HAVE_LIBUTIL_H */

/* Define if you have the <linux/rtnetlink.h> header file.  */
#define HAVE_LINUX_RTNETLINK_H 1

/* Define if you have the <linux/version.h> header file.  */
#define HAVE_LINUX_VERSION_H 1

/* Define if you have the <net/if_dl.h> header file.  */
/* #undef HAVE_NET_IF_DL_H */

/* Define if you have the <net/if_var.h> header file.  */
/* #undef HAVE_NET_IF_VAR_H */

/* Define if you have the <net/netopt.h> header file.  */
/* #undef HAVE_NET_NETOPT_H */

/* Define if you have the <netdb.h> header file.  */
#define HAVE_NETDB_H 1

/* Define if you have the <netinet/icmp6.h> header file.  */
#define HAVE_NETINET_ICMP6_H 1

/* Define if you have the <netinet/in.h> header file.  */
#define HAVE_NETINET_IN_H 1

/* Define if you have the <netinet/in6.h> header file.  */
/* #undef HAVE_NETINET_IN6_H */

/* Define if you have the <netinet/in6_var.h> header file.  */
/* #undef HAVE_NETINET_IN6_VAR_H */

/* Define if you have the <netinet/in_var.h> header file.  */
/* #undef HAVE_NETINET_IN_VAR_H */

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <stropts.h> header file.  */
#define HAVE_STROPTS_H 1

/* Define if you have the <sys/conf.h> header file.  */
/* #undef HAVE_SYS_CONF_H */

/* Define if you have the <sys/ksym.h> header file.  */
/* #undef HAVE_SYS_KSYM_H */

/* Define if you have the <sys/select.h> header file.  */
#define HAVE_SYS_SELECT_H 1

/* Define if you have the <sys/sockio.h> header file.  */
/* #undef HAVE_SYS_SOCKIO_H */

/* Define if you have the <sys/sysctl.h> header file.  */
#define HAVE_SYS_SYSCTL_H 1

/* Define if you have the <sys/time.h> header file.  */
#define HAVE_SYS_TIME_H 1

/* Define if you have the <sys/times.h> header file.  */
#define HAVE_SYS_TIMES_H 1

/* Define if you have the <sys/types.h> header file.  */
#define HAVE_SYS_TYPES_H 1

/* Define if you have the crypt library (-lcrypt).  */
#define HAVE_LIBCRYPT 1

/* Define if you have the kvm library (-lkvm).  */
/* #undef HAVE_LIBKVM */

/* Define if you have the m library (-lm).  */
#define HAVE_LIBM 1

/* Define if you have the nsl library (-lnsl).  */
/* #undef HAVE_LIBNSL */

/* Define if you have the readline library (-lreadline).  */
/* #undef HAVE_LIBREADLINE */

/* Define if you have the resolv library (-lresolv).  */
/* #undef HAVE_LIBRESOLV */

/* Define if you have the socket library (-lsocket).  */
/* #undef HAVE_LIBSOCKET */

/* Define if you have the tinfo library (-ltinfo).  */
/* #undef HAVE_LIBTINFO */

/* Define if you have the xnet library (-lxnet).  */
/* #undef HAVE_LIBXNET */

/* Name of package */
#define PACKAGE "zebra"

/* Version number of package */
#define VERSION "0.89"

