/* 
 * net-snmp configuration header file
 */
/* Portions of this file are subject to the following copyright(s).  See
 * the Net-SNMP's COPYING file for more details and other copyrights
 * that may apply:
 */
/*
 * Portions of this file are copyrighted by:
 * Copyright © 2003 Sun Microsystems, Inc. All rights reserved.
 * Use is subject to license terms specified in the COPYING file
 * distributed with the Net-SNMP package.
 */

#ifndef NET_SNMP_CONFIG_H
#define NET_SNMP_CONFIG_H


/* ********* NETSNMP_MARK_BEGIN_AUTOCONF_DEFINITIONS ********* */
/*
 * put all autoconf-specific definitions below here
 *
 */
#ifndef NETSNMP_NO_AUTOCONF_DEFINITIONS

/* define if you have type int32_t */
#undef HAVE_INT32_T

/* define if you have type uint32_t */
#undef HAVE_UINT32_T

/* define if you have type u_int32_t */
#undef HAVE_U_INT32_T

/* define if you have type int64_t */
#undef HAVE_INT64_T

/* define if you have type uint64_t */
#undef HAVE_UINT64_T

/* define if you have type u_int64_t */
#undef HAVE_U_INT64_T

/* define if you have type intptr_t */
#undef HAVE_INTPTR_T

/* define if you have type uintptr_t */
#undef HAVE_UINTPTR_T

/* got socklen_t? */
#undef HAVE_SOCKLEN_T

/* got in_addr_t? */
#undef HAVE_IN_ADDR_T

/* got ssize_t? */
#undef HAVE_SSIZE_T

/* define if you have getdevs() */
#undef HAVE_GETDEVS

/* define if you have devstat_getdevs() */
#undef HAVE_DEVSTAT_GETDEVS

/* define if you have <netinet/in_pcb.h> */
#undef HAVE_NETINET_IN_PCB_H

/* define if you have <sys/disklabel.h> */
#undef HAVE_SYS_DISKLABEL_H

/* define if your compiler (processor) defines __FUNCTION__ for you */
#undef HAVE_CPP_UNDERBAR_FUNCTION_DEFINED

/* on aix, if you have perfstat */
#undef HAVE_PERFSTAT

/* define if you have libdb, libnm or librpm, respectively */
#undef HAVE_LIBDB
#undef HAVE_LIBNM
#undef HAVE_LIBRPM

/* define if you have pkginfo */
#undef HAVE_PKGINFO

/* define if you have gethostbyname */
#undef HAVE_GETHOSTBYNAME

/* define if you have getservbyname */
#undef HAVE_GETSERVBYNAME

/* define if you have the perl_eval_pv() function */
#undef HAVE_PERL_EVAL_PV_LC

/* define if you have the Perl_eval_pv() function */
#undef HAVE_PERL_EVAL_PV_UC

/* Define to 1 if you have the `perl_eval_pv' function. */
#undef HAVE_PERL_EVAL_PV

/* printing system */
#undef HAVE_LPSTAT
#undef LPSTAT_PATH
#undef HAVE_PRINTCAP
/* Use dmalloc to do malloc debugging? */
#undef HAVE_DMALLOC_H

/* location of UNIX kernel */
#define KERNEL_LOC "/vmunix"

/* location of mount table list */
#define ETC_MNTTAB "/etc/mnttab"

/* location of swap device (ok if not found) */
#undef DMEM_LOC

/* Command to generate ps output, the final column must be the process
   name withOUT arguments */
#define PSCMD "/bin/ps"

/* Where is the uname command */
#define UNAMEPROG "/bin/uname"

/* define if you are using linux and /proc/net/dev has the compressed
   field, which exists in linux kernels 2.2 and greater. */
#undef PROC_NET_DEV_HAS_COMPRESSED

/* define rtentry to ortentry on SYSV machines (alphas) */
#undef RTENTRY

/* Use BSD 4.4 routing table entries? */
#undef RTENTRY_4_4

/* Does struct sigaction have a sa_sigaction field? */
#undef STRUCT_SIGACTION_HAS_SA_SIGACTION

/* Does struct tm have a tm_gmtoff field? */
#undef STRUCT_TM_HAS_TM_GMTOFFF

/* Does struct sockaddr have a sa_len field? */
#undef STRUCT_SOCKADDR_HAS_SA_LEN

/* Does struct sockaddr have a sa_family2 field? */
#undef STRUCT_SOCKADDR_HAS_SA_UNION_SA_GENERIC_SA_FAMILY2

/* Does struct sockaddr_storage have a ss_family field? */
#undef STRUCT_SOCKADDR_STORAGE_HAS_SS_FAMILY

/* Does struct sockaddr_storage have a __ss_family field? */
#undef STRUCT_SOCKADDR_STORAGE_HAS___SS_FAMILY

/* Does struct in6_addr have a s6_un.sa6_ladd field? */
#undef STRUCT_IN6_ADDR_HAS_S6_UN_SA6_LADDR

/* rtentry structure tests */
#undef RTENTRY_RT_NEXT
#undef STRUCT_RTENTRY_HAS_RT_DST
#undef STRUCT_RTENTRY_HAS_RT_UNIT
#undef STRUCT_RTENTRY_HAS_RT_USE
#undef STRUCT_RTENTRY_HAS_RT_REFCNT
#undef STRUCT_RTENTRY_HAS_RT_HASH

/* ifnet structure tests */
#undef STRUCT_IFNET_HAS_IF_BAUDRATE
#undef STRUCT_IFNET_HAS_IF_BAUDRATE_IFS_VALUE
#undef STRUCT_IFNET_HAS_IF_SPEED
#undef STRUCT_IFNET_HAS_IF_TYPE
#undef STRUCT_IFNET_HAS_IF_IMCASTS
#undef STRUCT_IFNET_HAS_IF_IQDROPS
#undef STRUCT_IFNET_HAS_IF_LASTCHANGE_TV_SEC
#undef STRUCT_IFNET_HAS_IF_NOPROTO
#undef STRUCT_IFNET_HAS_IF_OMCASTS
#undef STRUCT_IFNET_HAS_IF_XNAME
#undef STRUCT_IFNET_HAS_IF_OBYTES
#undef STRUCT_IFNET_HAS_IF_IBYTES
#undef STRUCT_IFNET_HAS_IF_ADDRLIST

/* tcpstat.tcps_rcvmemdrop */
#undef STRUCT_TCPSTAT_HAS_TCPS_RCVMEMDROP

/* udpstat.udps_discard */
#undef STRUCT_UDPSTAT_HAS_UDPS_DISCARD

/* udpstat.udps_discard */
#undef STRUCT_UDPSTAT_HAS_UDPS_NOPORT

/* udpstat.udps_discard */
#undef STRUCT_UDPSTAT_HAS_UDPS_NOPORTBCAST

/* udpstat.udps_discard */
#undef STRUCT_UDPSTAT_HAS_UDPS_FULLSOCK

/* arphd.at_next */
#undef STRUCT_ARPHD_HAS_AT_NEXT

/* ifaddr.ifa_next */
#undef STRUCT_IFADDR_HAS_IFA_NEXT

/* ifnet.if_mtu */
#undef STRUCT_IFNET_HAS_IF_MTU

/* swdevt.sw_nblksenabled */
#undef STRUCT_SWDEVT_HAS_SW_NBLKSENABLED

/* nlist.n_value */
#undef STRUCT_NLIST_HAS_N_VALUE

/* nlist64.n_value */
#undef STRUCT_NLIST64_HAS_N_VALUE

/* ipstat structure tests */
#undef STRUCT_IPSTAT_HAS_IPS_CANTFORWARD
#undef STRUCT_IPSTAT_HAS_IPS_CANTFRAG
#undef STRUCT_IPSTAT_HAS_IPS_DELIVERED
#undef STRUCT_IPSTAT_HAS_IPS_FRAGDROPPED
#undef STRUCT_IPSTAT_HAS_IPS_FRAGTIMEOUT
#undef STRUCT_IPSTAT_HAS_IPS_LOCALOUT
#undef STRUCT_IPSTAT_HAS_IPS_NOPROTO
#undef STRUCT_IPSTAT_HAS_IPS_NOROUTE
#undef STRUCT_IPSTAT_HAS_IPS_ODROPPED
#undef STRUCT_IPSTAT_HAS_IPS_OFRAGMENTS
#undef STRUCT_IPSTAT_HAS_IPS_REASSEMBLED

/* vfsstat.f_frsize */
#undef STRUCT_STATVFS_HAS_F_FRSIZE

/* vfsstat.f_files */
#undef STRUCT_STATVFS_HAS_F_FILES

/* statfs inode structure tests*/
#undef STRUCT_STATFS_HAS_F_FILES
#undef STRUCT_STATFS_HAS_F_FFREE
#undef STRUCT_STATFS_HAS_F_FAVAIL

/* des_ks_struct.weak_key */
#undef STRUCT_DES_KS_STRUCT_HAS_WEAK_KEY

/* ifnet needs to have _KERNEL defined */
#undef IFNET_NEEDS_KERNEL

/* sysctl works to get boottime, etc... */
#undef NETSNMP_CAN_USE_SYSCTL

/* define if SIOCGIFADDR exists in sys/ioctl.h */
#undef SYS_IOCTL_H_HAS_SIOCGIFADDR

/* Define if statfs takes 2 args and the second argument has
   type struct fs_data. [Ultrix] */
#undef STAT_STATFS_FS_DATA

/* Define if the TCP timer constants in <netinet/tcp_timer.h>
   depend on the integer variable `hz'.  [FreeBSD 4.x] */
#undef TCPTV_NEEDS_HZ

/* Not-to-be-compiled macros for use by configure only */
#define config_require(x)
#define config_exclude(x)
#define config_arch_require(x,y)
#define config_parse_dot_conf(w,x,y,z)
#define config_add_mib(x)
#define config_belongs_in(x)
#define config_error(x)
#define config_warning(x)
  
#if defined (WIN32) || defined (mingw32) || defined (cygwin)
#define ENV_SEPARATOR ";"
#define ENV_SEPARATOR_CHAR ';'
#else
#define ENV_SEPARATOR ":"
#define ENV_SEPARATOR_CHAR ':'
#endif

/* definitions added by configure on-the-fly */
@TOP@
@BOTTOM@

/* end of definitions added by configure on-the-fly */

#ifndef HAVE_STRCHR
#ifdef HAVE_INDEX
# define strchr index
# define strrchr rindex
#endif
#endif

#ifndef HAVE_INDEX
#ifdef HAVE_STRCHR
#ifdef mingw32
# define index(a,b) strchr(a,b)
# define rindex(a,b) strrchr(a,b)
#else
# define index strchr
# define rindex strrchr
#endif
#endif
#endif

#ifndef HAVE_MEMCPY
#ifdef HAVE_BCOPY
# define memcpy(d, s, n) bcopy ((s), (d), (n))
# define memmove(d, s, n) bcopy ((s), (d), (n))
# define memcmp bcmp
#endif
#endif

#ifndef HAVE_MEMMOVE
#ifdef HAVE_MEMCPY
# define memmove memcpy
#endif
#endif

#ifndef HAVE_BCOPY
#ifdef HAVE_MEMCPY
# define bcopy(s, d, n) memcpy ((d), (s), (n))
# define bzero(p,n) memset((p),(0),(n))
# define bcmp memcmp
#endif
#endif

/* If you have openssl 0.9.7 or above, you likely have AES support. */
#undef NETSNMP_USE_OPENSSL
#if defined(NETSNMP_USE_OPENSSL) && defined(HAVE_OPENSSL_AES_H) && defined(HAVE_AES_CFB128_ENCRYPT)
#define HAVE_AES 1
#endif

/* define random functions */

#ifndef HAVE_RANDOM
#ifdef HAVE_LRAND48
#define random lrand48
#define srandom(s) srand48(s)
#else
#ifdef HAVE_RAND
#define random rand
#define srandom(s) srand(s)
#endif
#endif
#endif

/* define signal if DNE */

#ifndef HAVE_SIGNAL
#ifdef HAVE_SIGSET
#define signal(a,b) sigset(a,b)
#endif
#endif

#if HAVE_DMALLOC_H
#define DMALLOC_FUNC_CHECK
#endif

#endif /* NETSNMP_NO_AUTOCONF_DEFINITIONS */




/* ********* NETSNMP_MARK_BEGIN_CLEAN_NAMESPACE ********* */
/* 
 * put all new net-snmp-specific definitions here
 *
 * all definitions MUST have a NETSNMP_ prefix
 *
 */

/* Default (SNMP) version number for the tools to use */
#define NETSNMP_DEFAULT_SNMP_VERSION 3

/* don't change these values! */
#define NETSNMP_SNMPV1      0xAAAA       /* readable by anyone */
#define NETSNMP_SNMPV2ANY   0xA000       /* V2 Any type (includes NoAuth) */
#define NETSNMP_SNMPV2AUTH  0x8000       /* V2 Authenticated requests only */

/* default list of mibs to load */
#define NETSNMP_DEFAULT_MIBS "IP-MIB:IF-MIB:TCP-MIB:UDP-MIB:SNMPv2-MIB:RFC1213-MIB"

/* default location to look for mibs to load using the above tokens
   and/or those in the MIBS envrionment variable*/
#undef NETSNMP_DEFAULT_MIBDIRS

/* default mib files to load, specified by path. */
#undef NETSNMP_DEFAULT_MIBFILES

/* should we compile to use special opaque types: float, double,
   counter64, i64, ui64, union? */
#undef NETSNMP_WITH_OPAQUE_SPECIAL_TYPES

/* define if you want to compile support for both authentication and
   privacy support. */
#undef NETSNMP_ENABLE_SCAPI_AUTHPRIV

/* define if you are using the MD5 code ...*/
#undef NETSNMP_USE_INTERNAL_MD5

/* define if you are using the codeS11 library ...*/
#undef NETSNMP_USE_PKCS11

/* debugging stuff */
/* if defined, we optimize the code to exclude all debugging calls. */
#undef NETSNMP_NO_DEBUGGING
/* ignore the -D flag and always print debugging information */
#define NETSNMP_ALWAYS_DEBUG 0

/* reverse encoding BER packets is both faster and more efficient in space. */
#define NETSNMP_USE_REVERSE_ASNENCODING       1
#define NETSNMP_DEFAULT_ASNENCODING_DIRECTION 1 /* 1 = reverse, 0 = forwards */

/* PERSISTENT_DIRECTORY: If defined, the library is capabile of saving
   persisant information to this directory in the form of configuration
   lines: PERSISTENT_DIRECTORY/NAME.persistent.conf */
#define NETSNMP_PERSISTENT_DIRECTORY "/var/snmp"

/* PERSISTENT_MASK: the umask permissions to set up persistent files with */
#define NETSNMP_PERSISTENT_MASK 077

/* AGENT_DIRECTORY_MODE: the mode the agents should use to create
   directories with. Since the data stored here is probably sensitive, it
   probably should be read-only by root/administrator. */
#define NETSNMP_AGENT_DIRECTORY_MODE 0700

/* MAX_PERSISTENT_BACKUPS:
 *   The maximum number of persistent backups the library will try to
 *   read from the persistent cache directory.  If an application fails to
 *   close down successfully more than this number of times, data will be lost.
 */
#define NETSNMP_MAX_PERSISTENT_BACKUPS 10

/* define the system type include file here */
#define NETSNMP_SYSTEM_INCLUDE_FILE <net-snmp/system/generic.h>

/* define the machine (cpu) type include file here */
#define NETSNMP_MACHINE_INCLUDE_FILE <net-snmp/machine/generic.h>

/* define the UDP buffer defaults undefined means use the OS buffers
 * by default */
#undef NETSNMP_DEFAULT_SERVER_SEND_BUF
#undef NETSNMP_DEFAULT_SERVER_RECV_BUF
#undef NETSNMP_DEFAULT_CLIENT_SEND_BUF
#undef NETSNMP_DEFAULT_CLIENT_RECV_BUF

/* net-snmp's major path names */
#undef SNMPLIBPATH
#undef SNMPSHAREPATH
#undef SNMPCONFPATH
#undef SNMPDLMODPATH

/* NETSNMP_LOGFILE:  If defined it closes stdout/err/in and opens this in 
   out/err's place.  (stdin is closed so that sh scripts won't wait for it) */
#undef NETSNMP_LOGFILE

/* default system contact */
#undef NETSNMP_SYS_CONTACT

/* system location */
#undef NETSNMP_SYS_LOC

/* Use libwrap to handle allow/deny hosts? */
#undef NETSNMP_USE_LIBWRAP

/* testing code sections. */
#undef NETSNMP_ENABLE_TESTING_CODE 

/* If you don't have root access don't exit upon kmem errors */
#undef NETSNMP_NO_ROOT_ACCESS

/* If we don't want to use kmem. */
#undef NETSNMP_NO_KMEM_USAGE

/* If you don't want the agent to report on variables it doesn't have data for */
#undef NETSNMP_NO_DUMMY_VALUES


/* Mib-2 tree Info */
/* These are the system information variables. */

#define NETSNMP_VERS_DESC   "unknown"             /* overridden at run time */
#define NETSNMP_SYS_NAME    "unknown"             /* overridden at run time */

/* comment out the second define to turn off functionality for any of
   these: (See README for details) */

/*   proc PROCESSNAME [MAX] [MIN] */
#define NETSNMP_PROCMIBNUM 2

/*   exec/shell NAME COMMAND      */
#define NETSNMP_SHELLMIBNUM 8

/*   swap MIN                     */
#define NETSNMP_MEMMIBNUM 4

/*   disk DISK MINSIZE            */
#define NETSNMP_DISKMIBNUM 9

/*   load 1 5 15                  */
#define NETSNMP_LOADAVEMIBNUM 10

/* which version are you using? This mibloc will tell you */
#define NETSNMP_VERSIONMIBNUM 100

/* Reports errors the agent runs into */
/* (typically its "can't fork, no mem" problems) */
#define NETSNMP_ERRORMIBNUM 101

/* The sub id of EXTENSIBLEMIB returned to queries of
   .iso.org.dod.internet.mgmt.mib-2.system.sysObjectID.0 */
#define NETSNMP_AGENTID 250

/* This ID is returned after the AGENTID above.  IE, the resulting
   value returned by a query to sysObjectID is
   EXTENSIBLEMIB.AGENTID.???, where ??? is defined below by OSTYPE */

#define NETSNMP_HPUX9ID 1
#define NETSNMP_SUNOS4ID 2 
#define NETSNMP_SOLARISID 3
#define NETSNMP_OSFID 4
#define NETSNMP_ULTRIXID 5
#define NETSNMP_HPUX10ID 6
#define NETSNMP_NETBSD1ID 7
#define NETSNMP_FREEBSDID 8
#define NETSNMP_IRIXID 9
#define NETSNMP_LINUXID 10
#define NETSNMP_BSDIID 11
#define NETSNMP_OPENBSDID 12
#define NETSNMP_WIN32ID 13
#define NETSNMP_HPUX11ID 14
#define NETSNMP_AIXID 15
#define NETSNMP_MACOSXID 16
#define NETSNMP_UNKNOWNID 255

#ifdef hpux9
#define NETSNMP_OSTYPE NETSNMP_HPUX9ID
#endif
#ifdef hpux10
#define NETSNMP_OSTYPE NETSNMP_HPUX10ID
#endif
#ifdef hpux11
#define NETSNMP_OSTYPE NETSNMP_HPUX11ID
#endif
#ifdef sunos4
#define NETSNMP_OSTYPE NETSNMP_SUNOS4ID
#endif
#ifdef solaris2
#define NETSNMP_OSTYPE NETSNMP_SOLARISID
#endif
#if defined(osf3) || defined(osf4) || defined(osf5)
#define NETSNMP_OSTYPE NETSNMP_OSFID
#endif
#ifdef ultrix4
#define NETSNMP_OSTYPE NETSNMP_ULTRIXID
#endif
#if defined(netbsd1) || defined(netbsd2)
#define NETSNMP_OSTYPE NETSNMP_NETBSD1ID
#endif
#if defined(__FreeBSD__)
#define NETSNMP_OSTYPE NETSNMP_FREEBSDID
#endif
#if defined(irix6) || defined(irix5)
#define NETSNMP_OSTYPE NETSNMP_IRIXID
#endif
#ifdef linux
#define NETSNMP_OSTYPE NETSNMP_LINUXID
#endif
#if defined(bsdi2) || defined(bsdi3) || defined(bsdi4)
#define NETSNMP_OSTYPE NETSNMP_BSDIID
#endif
#if defined(openbsd2) || defined(openbsd3) || defined(openbsd4)
#define NETSNMP_OSTYPE NETSNMP_OPENBSDID
#endif
#ifdef WIN32
#define NETSNMP_OSTYPE NETSNMP_WIN32ID
#endif
#if defined(aix3) || defined(aix4) || defined(aix5)
#define NETSNMP_OSTYPE NETSNMP_AIXID
#endif
#if defined(darwin) && (darwin >= 8)
#define NETSNMP_OSTYPE NETSNMP_MACOSXID
#endif
/* unknown */
#ifndef NETSNMP_OSTYPE
#define NETSNMP_OSTYPE NETSNMP_UNKNOWNID
#endif

/* The enterprise number has been assigned by the IANA group.   */
/* Optionally, this may point to the location in the tree your  */
/* company/organization has been allocated.                     */
/* The assigned enterprise number for the NET_SNMP MIB modules. */
#define NETSNMP_ENTERPRISE_OID			8072
#define NETSNMP_ENTERPRISE_MIB			1,3,6,1,4,1,8072
#define NETSNMP_ENTERPRISE_DOT_MIB		1.3.6.1.4.1.8072
#define NETSNMP_ENTERPRISE_DOT_MIB_LENGTH	7

/* The assigned enterprise number for sysObjectID. */
#define NETSNMP_SYSTEM_MIB		1,3,6,1,4,1,8072,3,2,NETSNMP_OSTYPE
#define NETSNMP_SYSTEM_DOT_MIB		1.3.6.1.4.1.8072.3.2.NETSNMP_OSTYPE
#define NETSNMP_SYSTEM_DOT_MIB_LENGTH	10

/* The assigned enterprise number for notifications. */
#define NETSNMP_NOTIFICATION_MIB		1,3,6,1,4,1,8072,4
#define NETSNMP_NOTIFICATION_DOT_MIB		1.3.6.1.4.1.8072.4
#define NETSNMP_NOTIFICATION_DOT_MIB_LENGTH	8

/* this is the location of the ucdavis mib tree.  It shouldn't be
   changed, as the places it is used are expected to be constant
   values or are directly tied to the UCD-SNMP-MIB. */
#define NETSNMP_UCDAVIS_OID		2021
#define NETSNMP_UCDAVIS_MIB		1,3,6,1,4,1,2021
#define NETSNMP_UCDAVIS_DOT_MIB		1.3.6.1.4.1.2021
#define NETSNMP_UCDAVIS_DOT_MIB_LENGTH	7

/* how long to wait (seconds) for error querys before reseting the error trap.*/
#define NETSNMP_ERRORTIMELENGTH 600 

/* Exec command to fix PROC problems */
/* %s will be replaced by the process name in error */

/* #define NETSNMP_PROCFIXCMD "/usr/bin/perl /local/scripts/fixproc %s" */

/* Exec command to fix EXEC problems */
/* %s will be replaced by the exec/script name in error */

/* #define NETSNMP_EXECFIXCMD "/usr/bin/perl /local/scripts/fixproc %s" */

/* Should exec output Cashing be used (speeds up things greatly), and
   if so, After how many seconds should the cache re-newed?  Note:
   Don't define CASHETIME to disable cashing completely */

#define NETSNMP_EXCACHETIME 30
#define NETSNMP_CACHEFILE ".snmp-exec-cache"
#define NETSNMP_MAXCACHESIZE (200*80)   /* roughly 200 lines max */

/* misc defaults */

/* default of 100 meg minimum if the minimum size is not specified in
   the config file */
#define NETSNMP_DEFDISKMINIMUMSPACE 100000

/* default maximum load average before error */
#define NETSNMP_DEFMAXLOADAVE 12.0

/* max times to loop reading output from execs. */
/* Because of sleep(1)s, this will also be time to wait (in seconds) for exec
   to finish */
#define NETSNMP_MAXREADCOUNT 100

/* Set if snmpgets should block and never timeout */
/* The original CMU code had this hardcoded as = 1 */
#define NETSNMP_SNMPBLOCK 1

/* How long to wait before restarting the agent after a snmpset to
   EXTENSIBLEMIB.VERSIONMIBNUM.VERRESTARTAGENT.  This is
   necessary to finish the snmpset reply before restarting. */
#define NETSNMP_RESTARTSLEEP 5

/* UNdefine to allow specifying zero-length community string */
/* #define NETSNMP_NO_ZEROLENGTH_COMMUNITY 1 */

/* define to exit the agent on a bad kernel read */
/* #define NETSNMP_EXIT_ON_BAD_KLREAD  */

/* Number of community strings to store */
#define NETSNMP_NUM_COMMUNITIES	5

/* internal define */
#define NETSNMP_LASTFIELD -1

/* configure options specified */
#define NETSNMP_CONFIGURE_OPTIONS ""

/*  Pluggable transports.  */

/*  This is defined if support for the UDP/IP transport domain is
    available.   */
#undef NETSNMP_TRANSPORT_UDP_DOMAIN

/*  This is defined if support for the "callback" transport domain is
    available.   */
#undef NETSNMP_TRANSPORT_CALLBACK_DOMAIN

/*  This is defined if support for the TCP/IP transport domain is
    available.  */
#undef NETSNMP_TRANSPORT_TCP_DOMAIN

/*  This is defined if support for the Unix transport domain
    (a.k.a. "local IPC") is available.  */
#undef NETSNMP_TRANSPORT_UNIX_DOMAIN

/*  This is defined if support for the AAL5 PVC transport domain is
    available.  */
#undef NETSNMP_TRANSPORT_AAL5PVC_DOMAIN

/*  This is defined if support for the IPX transport domain is
    available.  */
#undef NETSNMP_TRANSPORT_IPX_DOMAIN

/*  This is defined if support for the UDP/IPv6 transport domain is
    available.  */
#undef NETSNMP_TRANSPORT_UDPIPV6_DOMAIN

/*  This is defined if support for the TCP/IPv6 transport domain is
    available.  */
#undef NETSNMP_TRANSPORT_TCPIPV6_DOMAIN

/*  This is defined if support for the UDP/IP transport domain is
    available.   */
#undef NETSNMP_TRANSPORT_TLS_DOMAIN

/*  This is defined if support for stdin/out transport domain is available.   */
#undef NETSNMP_TRANSPORT_STD_DOMAIN

/* define this if the USM security module is available */
#undef NETSNMP_SECMOD_USM

/* define this if the KSM (kerberos based snmp) security module is available */
#undef NETSNMP_SECMOD_KSM

/* define this if the local security module is available */
#undef NETSNMP_SECMOD_LOCALSM

/* define if you want to build with reentrant/threaded code (incomplete)*/
#undef NETSNMP_REENTRANT

/* define if configured as a "mini-agent" */
#undef NETSNMP_MINI_AGENT

/* define if you are embedding perl in the main agent */
#undef NETSNMP_EMBEDDED_PERL

/* define if you want to build MFD module rewrites*/
#undef NETSNMP_ENABLE_MFD_REWRITES

/* this is the location of the net-snmp mib tree.  It shouldn't be
   changed, as the places it is used are expected to be constant
   values or are directly tied to the UCD-SNMP-MIB. */
#define NETSNMP_OID		8072
#define NETSNMP_MIB		1,3,6,1,4,1,8072
#define NETSNMP_DOT_MIB		1.3.6.1.4.1.8072
#define NETSNMP_DOT_MIB_LENGTH	7

/* pattern for temporary file names */
#define NETSNMP_TEMP_FILE_PATTERN "/tmp/snmpdXXXXXX"

/*
 * this must be before the system/machine includes, to allow them to
 * override and turn off inlining. To do so, they should do the
 * following:
 *
 *    #undef NETSNMP_ENABLE_INLINE
 *    #define NETSNMP_ENABLE_INLINE 0
 *
 * A user having problems with their compiler can also turn off
 * the use of inline by defining NETSNMP_NO_INLINE via their cflags:
 *
 *    -DNETSNMP_NO_INLINE
 *
 * Header and source files should only test against NETSNMP_USE_INLINE:
 *
 *   #ifdef NETSNMP_USE_INLINE
 *   NETSNMP_INLINE function(int parm) { return parm -1; }
 *   #endif
 *
 * Functions which should be static, regardless of whether or not inline
 * is available or enabled should use the NETSNMP_STATIC_INLINE macro,
 * like so:
 *
 *    NETSNMP_STATIC_INLINE function(int parm) { return parm -1; }
 *
 * NOT like this:
 *
 *    static NETSNMP_INLINE function(int parm) { return parm -1; }
 *
 */
#undef NETSNMP_BROKEN_INLINE
#ifdef NETSNMP_BROKEN_INLINE
#   define NETSNMP_ENABLE_INLINE 0
#else
#   define NETSNMP_ENABLE_INLINE 1
#endif

#include NETSNMP_SYSTEM_INCLUDE_FILE
#include NETSNMP_MACHINE_INCLUDE_FILE

#if NETSNMP_ENABLE_INLINE && !defined(NETSNMP_NO_INLINE)
#   define NETSNMP_USE_INLINE 1
#   ifndef NETSNMP_INLINE
#      define NETSNMP_INLINE inline
#   endif
#   ifndef NETSNMP_STATIC_INLINE
#      define NETSNMP_STATIC_INLINE static inline
#   endif
#else
#   define NETSNMP_INLINE 
#   define NETSNMP_STATIC_INLINE static
#endif

#ifndef NETSNMP_IMPORT
#  define NETSNMP_IMPORT extern
#endif

/* define if you want to enable IPv6 support */
#undef NETSNMP_ENABLE_IPV6

/* define if you want to restrict SMUX connections to localhost by default */
#undef NETSNMP_ENABLE_LOCAL_SMUX

/* define if agentx transport is to use domain sockets only */
#undef NETSNMP_AGENTX_DOM_SOCK_ONLY

/* define if you do not want snmptrapd to register as an AgentX subagent */
#undef NETSNMP_SNMPTRAPD_DISABLE_AGENTX

/* define this if we're using the new MIT crypto API */
#undef NETSNMP_USE_KERBEROS_MIT

/* define this if you're using Heimdal Kerberos */
#undef NETSNMP_USE_KERBEROS_HEIMDAL

/* comment the next line if you are compiling with libsnmp.h 
   and are not using the UC-Davis SNMP library. */
#define UCD_SNMP_LIBRARY 1

/* add in recent CMU library extensions (not complete) */
#undef CMU_COMPATIBLE

/* final conclusion on nlist usage */
#if defined(HAVE_NLIST) && defined(STRUCT_NLIST_HAS_N_VALUE) && !defined(NETSNMP_DONT_USE_NLIST) && !defined(NETSNMP_NO_KMEM_USAGE)
#define NETSNMP_CAN_USE_NLIST
#endif


/* ********* NETSNMP_MARK_BEGIN_LEGACY_DEFINITIONS *********/
/* 
 * existing definitions prior to Net-SNMP 5.4
 *
 * do not add anything new here
 *
 */

#ifndef NETSNMP_NO_LEGACY_DEFINITIONS

#ifdef NETSNMP_DEFAULT_SNMP_VERSION
# define DEFAULT_SNMP_VERSION NETSNMP_DEFAULT_SNMP_VERSION
#endif

#ifdef NETSNMP_SNMPV1
# define SNMPV1 NETSNMP_SNMPV1
#endif

#ifdef NETSNMP_SNMPV2ANY
# define SNMPV2ANY NETSNMP_SNMPV2ANY
#endif

#ifdef NETSNMP_SNMPV2AUTH
# define SNMPV2AUTH NETSNMP_SNMPV2AUTH
#endif

#ifdef NETSNMP_DEFAULT_MIBS
# define DEFAULT_MIBS NETSNMP_DEFAULT_MIBS
#endif

#ifdef NETSNMP_DEFAULT_MIBDIRS
# define DEFAULT_MIBDIRS NETSNMP_DEFAULT_MIBDIRS
#endif

#ifdef NETSNMP_DEFAULT_MIBFILES
# define DEFAULT_MIBFILES NETSNMP_DEFAULT_MIBFILES
#endif

#ifdef NETSNMP_WITH_OPAQUE_SPECIAL_TYPES
# define OPAQUE_SPECIAL_TYPES NETSNMP_WITH_OPAQUE_SPECIAL_TYPES
#endif

#ifdef NETSNMP_ENABLE_SCAPI_AUTHPRIV
# define SCAPI_AUTHPRIV NETSNMP_ENABLE_SCAPI_AUTHPRIV
#endif

#ifdef NETSNMP_USE_INTERNAL_MD5
# define USE_INTERNAL_MD5 NETSNMP_USE_INTERNAL_MD5
#endif

#ifdef NETSNMP_USE_PKCS11
# define USE_PKCS NETSNMP_USE_PKCS11
#endif

#ifdef NETSNMP_USE_OPENSSL
# define USE_OPENSSL NETSNMP_USE_OPENSSL
#endif

#ifdef NETSNMP_NO_DEBUGGING
# define SNMP_NO_DEBUGGING NETSNMP_NO_DEBUGGING
#endif

#ifdef NETSNMP_ALWAYS_DEBUG
# define SNMP_ALWAYS_DEBUG NETSNMP_ALWAYS_DEBUG
#endif

#ifdef NETSNMP_USE_REVERSE_ASNENCODING
# define USE_REVERSE_ASNENCODING NETSNMP_USE_REVERSE_ASNENCODING
#endif
#ifdef NETSNMP_DEFAULT_ASNENCODING_DIRECTION
# define DEFAULT_ASNENCODING_DIRECTION NETSNMP_DEFAULT_ASNENCODING_DIRECTION
#endif

#define PERSISTENT_DIRECTORY NETSNMP_PERSISTENT_DIRECTORY
#define PERSISTENT_MASK NETSNMP_PERSISTENT_MASK
#define AGENT_DIRECTORY_MODE NETSNMP_AGENT_DIRECTORY_MODE
#define MAX_PERSISTENT_BACKUPS NETSNMP_MAX_PERSISTENT_BACKUPS
#define SYSTEM_INCLUDE_FILE NETSNMP_SYSTEM_INCLUDE_FILE
#define MACHINE_INCLUDE_FILE NETSNMP_MACHINE_INCLUDE_FILE

#ifdef NETSNMP_DEFAULT_SERVER_SEND_BUF
# define DEFAULT_SERVER_SEND_BUF NETSNMP_DEFAULT_SERVER_SEND_BUF
#endif
#ifdef NETSNMP_DEFAULT_SERVER_RECV_BUF
# define DEFAULT_SERVER_RECV_BUF NETSNMP_DEFAULT_SERVER_RECV_BUF
#endif
#ifdef NETSNMP_DEFAULT_CLIENT_SEND_BUF
# define DEFAULT_CLIENT_SEND_BUF NETSNMP_DEFAULT_CLIENT_SEND_BUF
#endif
#ifdef NETSNMP_DEFAULT_CLIENT_RECV_BUF
# define DEFAULT_CLIENT_RECV_BUF NETSNMP_DEFAULT_CLIENT_RECV_BUF
#endif

#ifdef NETSNMP_LOGFILE
# define LOGFILE NETSNMP_LOGFILE
#endif

#ifdef NETSNMP_SYS_CONTACT
# define SYS_CONTACT NETSNMP_SYS_CONTACT
#endif

#ifdef NETSNMP_SYS_LOC
# define SYS_LOC NETSNMP_SYS_LOC
#endif

#ifdef NETSNMP_USE_LIBWRAP
# define USE_LIBWRAP NETSNMP_USE_LIBWRAP
#endif

#ifdef NETSNMP_ENABLE_TESTING_CODE 
# define SNMP_TESTING_CODE NETSNMP_ENABLE_TESTING_CODE
#endif

#ifdef NETSNMP_NO_ROOT_ACCESS
# define NO_ROOT_ACCESS NETSNMP_NO_ROOT_ACCESS
#endif

#ifdef NETSNMP_NO_KMEM_USAGE
# define NO_KMEM_USAGE NETSNMP_NO_KMEM_USAGE
#endif

#ifdef NETSNMP_NO_DUMMY_VALUES
# define NO_DUMMY_VALUES NETSNMP_NO_DUMMY_VALUES
#endif

#define VERS_DESC     NETSNMP_VERS_DESC
#define SYS_NAME      NETSNMP_SYS_NAME

#define PROCMIBNUM    NETSNMP_PROCMIBNUM
#define SHELLMIBNUM   NETSNMP_SHELLMIBNUM
#define MEMMIBNUM     NETSNMP_MEMMIBNUM
#define DISKMIBNUM    NETSNMP_DISKMIBNUM

#define LOADAVEMIBNUM NETSNMP_LOADAVEMIBNUM
#define VERSIONMIBNUM NETSNMP_VERSIONMIBNUM
#define ERRORMIBNUM   NETSNMP_ERRORMIBNUM
#define AGENTID       NETSNMP_AGENTID

#define HPUX9ID       NETSNMP_HPUX9ID
#define SUNOS4ID      NETSNMP_SUNOS4ID
#define SOLARISID     NETSNMP_SOLARISID
#define OSFID         NETSNMP_OSFID
#define ULTRIXID      NETSNMP_ULTRIXID
#define HPUX10ID      NETSNMP_HPUX10ID
#define NETBSD1ID     NETSNMP_NETBSD1ID
#define FREEBSDID     NETSNMP_FREEBSDID
#define IRIXID        NETSNMP_IRIXID
#define LINUXID       NETSNMP_LINUXID
#define BSDIID        NETSNMP_BSDIID
#define OPENBSDID     NETSNMP_OPENBSDID
#define WIN32ID       NETSNMP_WIN32ID
#define HPUX11ID      NETSNMP_HPUX11ID
#define AIXID         NETSNMP_AIXID
#define MACOSXID      NETSNMP_MACOSXID
#define UNKNOWNID     NETSNMP_UNKNOWNID

#define ENTERPRISE_OID            NETSNMP_ENTERPRISE_OID
#define ENTERPRISE_MIB            NETSNMP_ENTERPRISE_MIB
#define ENTERPRISE_DOT_MIB        NETSNMP_ENTERPRISE_DOT_MIB
#define ENTERPRISE_DOT_MIB_LENGTH NETSNMP_ENTERPRISE_DOT_MIB_LENGTH

#define SYSTEM_MIB		  NETSNMP_SYSTEM_MIB
#define SYSTEM_DOT_MIB		  NETSNMP_SYSTEM_DOT_MIB
#define SYSTEM_DOT_MIB_LENGTH	  NETSNMP_SYSTEM_DOT_MIB_LENGTH

#define NOTIFICATION_MIB	    NETSNMP_NOTIFICATION_MIB	
#define NOTIFICATION_DOT_MIB	    NETSNMP_NOTIFICATION_DOT_MIB
#define NOTIFICATION_DOT_MIB_LENGTH NETSNMP_NOTIFICATION_DOT_MIB_LENGTH

#define UCDAVIS_OID		  NETSNMP_UCDAVIS_OID
#define UCDAVIS_MIB		  NETSNMP_UCDAVIS_MIB
#define UCDAVIS_DOT_MIB		  NETSNMP_UCDAVIS_DOT_MIB
#define UCDAVIS_DOT_MIB_LENGTH	  NETSNMP_UCDAVIS_DOT_MIB_LENGTH

#define ERRORTIMELENGTH NETSNMP_ERRORTIMELENGTH

#ifdef NETSNMP_PROCFIXCMD
# define PROCFIXCMD NETSNMP_PROCFIXCMD
#endif

#ifdef NETSNMP_EXECFIXCMD
# define EXECFIXCMD NETSNMP_EXECFIXCMD
#endif

#define EXCACHETIME  NETSNMP_EXCACHETIME
#define CACHEFILE    NETSNMP_CACHEFILE
#define MAXCACHESIZE NETSNMP_MAXCACHESIZE

#define DEFDISKMINIMUMSPACE NETSNMP_DEFDISKMINIMUMSPACE
#define DEFMAXLOADAVE NETSNMP_DEFMAXLOADAVE
#define MAXREADCOUNT NETSNMP_MAXREADCOUNT

#define SNMPBLOCK NETSNMP_SNMPBLOCK
#define RESTARTSLEEP NETSNMP_RESTARTSLEEP

#define NUM_COMMUNITIES	NETSNMP_NUM_COMMUNITIES

#ifdef NETSNMP_NO_ZEROLENGTH_COMMUNITY
# define NO_ZEROLENGTH_COMMUNITY NETSNMP_NO_ZEROLENGTH_COMMUNITY
#endif

#ifdef NETSNMP_EXIT_ON_BAD_KLREAD
# define EXIT_ON_BAD_KLREAD NETSNMP_EXIT_ON_BAD_KLREAD
#endif

#define LASTFIELD NETSNMP_LASTFIELD

#define CONFIGURE_OPTIONS NETSNMP_CONFIGURE_OPTIONS

#ifdef NETSNMP_TRANSPORT_UDP_DOMAIN
# define SNMP_TRANSPORT_UDP_DOMAIN NETSNMP_TRANSPORT_UDP_DOMAIN
#endif

#ifdef NETSNMP_TRANSPORT_CALLBACK_DOMAIN
# define SNMP_TRANSPORT_CALLBACK_DOMAIN NETSNMP_TRANSPORT_CALLBACK_DOMAIN
#endif

#ifdef NETSNMP_TRANSPORT_TCP_DOMAIN
# define SNMP_TRANSPORT_TCP_DOMAIN NETSNMP_TRANSPORT_TCP_DOMAIN
#endif

#ifdef NETSNMP_TRANSPORT_UNIX_DOMAIN
# define SNMP_TRANSPORT_UNIX_DOMAIN NETSNMP_TRANSPORT_UNIX_DOMAIN
#endif

#ifdef NETSNMP_TRANSPORT_AAL5PVC_DOMAIN
# define SNMP_TRANSPORT_AAL5PVC_DOMAIN NETSNMP_TRANSPORT_AAL5PVC_DOMAIN
#endif

#ifdef NETSNMP_TRANSPORT_IPX_DOMAIN
# define SNMP_TRANSPORT_IPX_DOMAIN NETSNMP_TRANSPORT_IPX_DOMAIN
#endif

#ifdef NETSNMP_TRANSPORT_UDPIPV6_DOMAIN
# define SNMP_TRANSPORT_UDPIPV6_DOMAIN NETSNMP_TRANSPORT_UDPIPV6_DOMAIN
#endif

#ifdef NETSNMP_TRANSPORT_TCPIPV6_DOMAIN
# define SNMP_TRANSPORT_TCPIPV6_DOMAIN NETSNMP_TRANSPORT_TCPIPV6_DOMAIN
#endif

#ifdef NETSNMP_TRANSPORT_TLS_DOMAIN
# define SNMP_TRANSPORT_TLS_DOMAIN NETSNMP_TRANSPORT_TLS_DOMAIN
#endif

#ifdef NETSNMP_TRANSPORT_STD_DOMAIN
# define SNMP_TRANSPORT_STD_DOMAIN NETSNMP_TRANSPORT_STD_DOMAIN
#endif

#ifdef NETSNMP_SECMOD_USM
# define SNMP_SECMOD_USM NETSNMP_SECMOD_USM
#endif

#ifdef NETSNMP_SECMOD_KSM
# define SNMP_SECMOD_KSM NETSNMP_SECMOD_KSM
#endif

#ifdef NETSNMP_SECMOD_LOCALSM 
# define SNMP_SECMOD_LOCALSM NETSNMP_SECMOD_LOCALSM
#endif

#ifdef NETSNMP_REENTRANT
# define NS_REENTRANT NETSNMP_REENTRANT
#endif

#ifdef NETSNMP_ENABLE_IPV6
# define INET6 NETSNMP_ENABLE_IPV6
#endif

#ifdef NETSNMP_ENABLE_LOCAL_SMUX
# define LOCAL_SMUX NETSNMP_ENABLE_LOCAL_SMUX
#endif

#ifdef NETSNMP_AGENTX_DOM_SOCK_ONLY
# define AGENTX_DOM_SOCK_ONLY NETSNMP_AGENTX_DOM_SOCK_ONLY
#endif

#ifdef NETSNMP_SNMPTRAPD_DISABLE_AGENTX
# define SNMPTRAPD_DISABLE_AGENTX
#endif

#ifdef NETSNMP_USE_KERBEROS_MIT
# define MIT_NEW_CRYPTO NETSNMP_USE_KERBEROS_MIT
#endif

#ifdef NETSNMP_USE_KERBEROS_HEIMDAL
# define HEIMDAL NETSNMP_USE_KERBEROS_HEIMDAL
#endif

#ifdef NETSNMP_AGENTX_SOCKET
# define AGENTX_SOCKET NETSNMP_AGENTX_SOCKET
#endif

#ifdef NETSNMP_DISABLE_MIB_LOADING
# define DISABLE_MIB_LOADING NETSNMP_DISABLE_MIB_LOADING
#endif

#ifdef NETSNMP_DISABLE_SNMPV1
# define DISABLE_SNMPV1 NETSNMP_DISABLE_SNMPV1
#endif

#ifdef NETSNMP_DISABLE_SNMPV2C
# define DISABLE_SNMPV2C NETSNMP_DISABLE_SNMPV2C
#endif

#ifdef NETSNMP_DISABLE_SET_SUPPORT
# define DISABLE_SET_SUPPORT NETSNMP_DISABLE_SET_SUPPORT
#endif

#ifdef NETSNMP_DISABLE_DES
# define DISABLE_DES NETSNMP_DISABLE_DES
#endif

#ifdef NETSNMP_DISABLE_MD5
# define DISABLE_MD5 NETSNMP_DISABLE_MD5
#endif

#ifdef NETSNMP_DONT_USE_NLIST
# define DONT_USE_NLIST NETSNMP_DONT_USE_NLIST
#endif

#ifdef NETSNMP_CAN_USE_NLIST
# define CAN_USE_NLIST NETSNMP_CAN_USE_NLIST
#endif

#ifdef NETSNMP_CAN_USE_SYSCTL
# define CAN_USE_SYSCTL NETSNMP_CAN_USE_SYSCTL
#endif

#endif /* NETSNMP_NO_LEGACY_DEFINITIONS */


#endif /* NET_SNMP_CONFIG_H */
