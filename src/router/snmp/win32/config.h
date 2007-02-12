/* config.h:  a general config file */

#ifdef __cplusplus
extern "C" {
#endif

/* UC-Davis' IANA-assigned enterprise number is 2021 */
#define ENTERPRISE_NUMBER 2021

/* don't change these values! */
#define SNMPV1      0xAAAA       /* readable by anyone */
#define SNMPV2ANY   0xA000       /* V2 Any type (includes NoAuth) */
#define SNMPV2AUTH  0x8000       /* V2 Authenticated requests only */

/* default list of mibs to load */
#define DEFAULT_MIBS "IP-MIB;IF-MIB;TCP-MIB;UDP-MIB;SNMPv2-MIB;RFC1213-MIB;UCD-SNMP-MIB;UCD-DEMO-MIB;SNMP-TARGET-MIB;SNMP-VIEW-BASED-ACM-MIB;SNMP-COMMUNITY-MIB;UCD-DLMOD-MIB;SNMP-FRAMEWORK-MIB;SNMP-MPD-MIB;SNMP-USER-BASED-SM-MIB;SNMP-NOTIFICATION-MIB;SNMPv2-TM"

/* default location to look for mibs to load using the above tokens
   and/or those in the MIBS envrionment variable*/

#define DEFAULT_MIBDIRS "/USR/MIBS"

/* default mib files to load, specified by path. */

#undef DEFAULT_MIBFILES

/* should we compile to use special opaque types: float, double,
   counter64, i64, ui64, union? */
#define OPAQUE_SPECIAL_TYPES 1

/* comment the next line if you are compiling with libsnmp.h
   and are not using the UC-Davis SNMP library. */
#define UCD_SNMP_LIBRARY 1

/* define if you want to compile support for both authentication and
   privacy support. */
#define SCAPI_AUTHPRIV 1

/* define if you are using the internal MD5 code */
#define USE_INTERNAL_MD5 1

/* add in recent CMU library extensions (not complete) */
#define CMU_COMPATIBLE 1

/* should "--" comments in mibs be a comment till the end of the line
   or also until another "--", the latter being the technically
   correct. */
#undef MIB_COMMENT_IS_EOL_TERMINATED

/* debugging stuff */
#undef SNMP_NO_DEBUGGING           /* if defined, we optimize the code
                                      to exclude all debugging calls. */
#define SNMP_ALWAYS_DEBUG 0        /* Always print debugging information and
                                      ignore the -D flag passed to the cmds */

/* Define if using alloca.c.  */
#undef C_ALLOCA

/* Define to one of _getb67, GETB67, getb67 for Cray-2 and Cray-YMP systems.
   This function is required for alloca.c support on those systems.  */
#undef CRAY_STACKSEG_END

/* Define if you have alloca, as a function or macro.  */
#undef HAVE_ALLOCA

/* Define if you have <alloca.h> and it should be used (not on Ultrix).  */
#undef HAVE_ALLOCA_H

/* Define if you have the getmntent function.  */
#undef HAVE_GETMNTENT

/* Define if you have <sys/wait.h> that is POSIX.1 compatible.  */
#undef HAVE_SYS_WAIT_H

/* Define to `long' if <sys/types.h> doesn't define.  */
#undef off_t

/* Define to `int' if <sys/types.h> doesn't define.  */
#undef pid_t

/* Define as the return type of signal handlers (int or void).  */
#define RETSIGTYPE void

/* Define if you have raise() instead of alarm() */

#define HAVE_RAISE 1

/* If using the C implementation of alloca, define if you know the
   direction of stack growth for your system; otherwise it will be
   automatically deduced at run-time.
 STACK_DIRECTION > 0 => grows toward higher addresses
 STACK_DIRECTION < 0 => grows toward lower addresses
 STACK_DIRECTION = 0 => direction of growth unknown
 */
#undef STACK_DIRECTION

/* Define if you have the ANSI C header files.  */
#define STDC_HEADERS 1

/* Define if you can safely include both <sys/time.h> and <time.h>.  */
#define TIME_WITH_SYS_TIME 1

/* Define if you have the gettimeofday function.  */
#undef HAVE_GETTIMEOFDAY

/* Define if you have the <sys/time.h> header file.  */
#undef HAVE_SYS_TIME_H

/* Define if your processor stores words with the most significant
   byte first (like Motorola and SPARC, unlike Intel and VAX).  */
#undef WORDS_BIGENDIAN

#define SNMPDLMODPATH "/USR/LIB/DLMOD"
#define SNMPLIBPATH "/USR/LIB"
#define SNMPSHAREPATH "/USR/SHARE/SNMP"
#define SNMPCONFPATH "/USR"

/* SNMPPATH contains (more) important files */

#undef SNMPPATH

/* LOGFILE:  If defined it closes stdout/err/in and opens this in out/err's
   place.  (stdin is closed so that sh scripts won't wait for it) */

#undef LOGFILE

/* PERSISTENT_DIRECTORY: If defined, the library is capabile of saving
   persisant information to this directory in the form of configuration
   lines: PERSISTENT_DIRECTORY/NAME.persistent.conf */
#define PERSISTENT_DIRECTORY "/USR/SNMP/PERSIST"

#define MAX_PERSISTENT_BACKUPS 10
/* default system contact */
#define SYS_CONTACT "unknown"

/* system location */
#define SYS_LOC "unknown"

/* location of UNIX kernel */
#define KERNEL_LOC "unknown"

/* location of mount table list */
#define ETC_MNTTAB "unknown"

/* location of swap device (ok if not found) */
#undef DMEM_LOC

/* define rtentry to ortentry on SYSV machines (alphas) */
#define RTENTRY rtentry;

/* Use BSD 4.4 routing table entries? */
#undef RTENTRY_4_4

/* Does the rtentry structure have a rt_next node */
#undef RTENTRY_RT_NEXT

#define PSCMD "/bin/ps"

/* Where is the uname command */
#define UNAMEPROG "/bin/uname"

/* testing code sections. */
/* #undef SNMP_TESTING_CODE */

/* If you don't have root access don't exit upon kmem errors */
#undef NO_ROOT_ACCESS

/* Define if you have the bcopy function.  */
#undef HAVE_BCOPY

/* Define if you have the gethostbyname function.  */
#define HAVE_GETHOSTBYNAME 1

/* Define if you have the gethostname function.  */
#define HAVE_GETHOSTNAME 1

/* Define if you have the getloadavg function.  */
#undef HAVE_GETLOADAVG

/* Define if you have the getservbyname function.  */
#define HAVE_GETSERVBYNAME 1

/* Define if you have the setenv function.  */
#undef HAVE_SETENV

/* Define if you have the strtoul function.  */
#define HAVE_STRTOUL 1

/* Define if you have the vsnprintf function.  */
#define HAVE_VSNPRINTF 1
#define HAVE_SNPRINTF 1

/* Define if you have the index function.  */
#undef HAVE_INDEX

/* Define if you have the kvm_openfiles function.  */
#undef HAVE_KVM_OPENFILES

/* Define if you have the lrand48 function.  */
#undef HAVE_LRAND48

/* Define if you have the memcpy function.  */
#define HAVE_MEMCPY 1

/* Define if you have the memmove function.  */
#define HAVE_MEMMOVE 1

/* Define if you have the rand function.  */
#define HAVE_RAND 1

/* Define if you have the random function.  */
#undef HAVE_RANDOM

/* Define if you have the select function.  */
#undef HAVE_SELECT

/* Define if you have the setmntent function.  */
#undef HAVE_SETMNTENT

/* Define if you have the sigblock function.  */
#undef HAVE_SIGBLOCK

/* Define if you have the sighold function.  */
#undef HAVE_SIGHOLD

/* Define if you have the signal function.  */
#undef HAVE_SIGNAL

/* Define if you have the sigset function.  */
#undef HAVE_SIGSET

/* Define if you have the socket function.  */
#define HAVE_SOCKET 1

/* Define if you have <winsock.h> header file. */
#define HAVE_WINSOCK_H 1

/* Define if you have the closesocket function.  */
#define HAVE_CLOSESOCKET 1

/* Define if you have the statfs function.  */
#undef HAVE_STATFS

/* Define if you have the statvfs function.  */
#undef HAVE_STATVFS

/* Define if you have the strchr function.  */
#define HAVE_STRCHR 1

/* use win32 strdup */
#define strdup _strdup

/* Define if you have the strtol function.  */
#define HAVE_STRTOL 1

/* Define if you have the tcgetattr function.  */
/* #define HAVE_TCGETATTR 1 */

/* Define if you have the uname function.  */
#undef HAVE_UNAME

/* Define if you have <io.h> header file. */
#define HAVE_IO_H 1

/* Define if you have <process.h> header file. (Win32-getpid) */
#define HAVE_PROCESS_H 1

/* Define if you have the <arpa/inet.h> header file.  */
#undef HAVE_ARPA_INET_H

/* Define if you have the <dirent.h> header file.  */
#undef HAVE_DIRENT_H

/* Define if you have the <err.h> header file.  */
#undef HAVE_ERR_H

/* Define if you have the <fcntl.h> header file.  */
#define HAVE_FCNTL_H 1

/* Define if you have the <fstab.h> header file.  */
#undef HAVE_FSTAB_H

/* Define if you have the <inet/mib2.h> header file.  */
#undef HAVE_INET_MIB2_H

/* Define if you have the <kstat.h> header file.  */
#undef HAVE_KSTAT_H

/* Define if you have the <kvm.h> header file.  */
#undef HAVE_KVM_H

/* Define if you have the <limits.h> header file.  */
#define HAVE_LIMITS_H 1

/* Define if you have the <machine/param.h> header file.  */
#undef HAVE_MACHINE_PARAM_H

/* Define if you have the <machine/pte.h> header file.  */
#undef HAVE_MACHINE_PTE_H

/* Define if you have the <malloc.h> header file.  */
#undef HAVE_MALLOC_H

/* Define if you have the <mntent.h> header file.  */
#undef HAVE_MNTENT_H

/* Define if you have the <mtab.h> header file.  */
#undef HAVE_MTAB_H

/* Define if you have the <ndir.h> header file.  */
#undef HAVE_NDIR_H

/* Define if you have the <net/if_dl.h> header file.  */
#undef HAVE_NET_IF_DL_H

/* Define if you have the <net/if_types.h> header file.  */
#undef HAVE_NET_IF_TYPES_H

/* Define if you have the <netinet/icmp_var.h> header file.  */
#undef HAVE_NETINET_ICMP_VAR_H

/* Define if you have the <netinet/if_ether.h> header file.  */
#undef HAVE_NETINET_IF_ETHER_H

/* Define if you have the <netinet/in.h> header file.  */
#undef HAVE_NETINET_IN_H

/* Define if you have the <netinet/in_pcb.h> header file.  */
#undef HAVE_NETINET_IN_PCB_H

/* Define if you have the <netinet/in_var.h> header file.  */
#undef HAVE_NETINET_IN_VAR_H

/* Define if you have the <netinet/ip_var.h> header file.  */
#undef HAVE_NETINET_IP_VAR_H

/* Define if you have the <netinet/tcp_fsm.h> header file.  */
#undef HAVE_NETINET_TCP_FSM_H

/* Define if you have the <netinet/tcp_timer.h> header file.  */
#undef HAVE_NETINET_TCP_TIMER_H

/* Define if you have the <netinet/tcp_var.h> header file.  */
#undef HAVE_NETINET_TCP_VAR_H

/* Define if you have the <netinet/tcpip.h> header file.  */
#undef HAVE_NETINET_TCPIP_H

/* Define if you have the <netinet/udp_var.h> header file.  */
#undef HAVE_NETINET_UDP_VAR_H

/* Define if you have the <sgtty.h> header file.  */
#undef HAVE_SGTTY_H

/* Define if you have the <stdarg.h> header file.  */
#define HAVE_STDARG_H 1

/* Define if you have the <stdlib.h> header file.  */
#define HAVE_STDLIB_H 1

/* Define if you have the <string.h> header file.  */
#define HAVE_STRING_H 1

/* Define if you have the <sys/conf.h> header file.  */
#undef HAVE_SYS_CONF_H

/* Define if you have the <sys/dir.h> header file.  */
#undef HAVE_SYS_DIR_H

/* Define if you have the <sys/dmap.h> header file.  */
#undef HAVE_SYS_DMAP_H

/* Define if you have the <sys/file.h> header file.  */
#undef HAVE_SYS_FILE_H

/* Define if you have the <sys/filio.h> header file.  */
#undef HAVE_SYS_FILIO_H

/* Define if you have the <sys/fixpoint.h> header file.  */
#undef HAVE_SYS_FIXPOINT_H

/* Define if you have the <sys/fs.h> header file.  */
#undef HAVE_SYS_FS_H

/* Define if you have the <sys/stat.h> header file.  */
#define HAVE_SYS_STAT_H 1

/* Define if you have the <sys/hashing.h> header file.  */
#undef HAVE_SYS_HASHING_H

/* Define if you have the <sys/ioctl.h> header file.  */
#undef HAVE_SYS_IOCTL_H

/* Define if you have the <sys/mbuf.h> header file.  */
#undef HAVE_SYS_MBUF_H

/* Define if you have the <sys/mnttab.h> header file.  */
#undef HAVE_SYS_MNTTAB_H

/* Define if you have the <sys/mount.h> header file.  */
#undef HAVE_SYS_MOUNT_H

/* Define if you have the <sys/ndir.h> header file.  */
#undef HAVE_SYS_NDIR_H

/* Define if you have the <sys/param.h> header file.  */
#undef HAVE_SYS_PARAM_H

/* Define if you have the <sys/proc.h> header file.  */
#undef HAVE_SYS_PROC_H

/* Define if you have the <sys/protosw.h> header file.  */
#undef HAVE_SYS_PROTOSW_H

/* Define if you have the <sys/select.h> header file.  */
#undef HAVE_SYS_SELECT_H

/* Define if you have the <sys/sockio.h> header file.  */
#undef HAVE_SYS_SOCKIO_H

/* Define if you have the <sys/statvfs.h> header file.  */
#undef HAVE_SYS_STATVFS_H

/* Define if you have the <sys/swap.h> header file.  */
#undef HAVE_SYS_SWAP_H

/* Define if you have the <sys/sysctl.h> header file.  */
#undef HAVE_SYS_SYSCTL_H

/* Define if you have the <sys/tcpipstats.h> header file.  */
#undef HAVE_SYS_TCPIPSTATS_H

/* Define if you have the <sys/user.h> header file.  */
#undef HAVE_SYS_USER_H

/* Define if you have the <sys/utsname.h> header file.  */
#undef HAVE_SYS_UTSNAME_H

/* Define if you have the <sys/vfs.h> header file.  */
#undef HAVE_SYS_VFS_H

/* Define if you have the <sys/vm.h> header file.  */
#undef HAVE_SYS_VM_H

/* Define if you have the <sys/vmmac.h> header file.  */
#undef HAVE_SYS_VMMAC_H

/* Define if you have the <sys/vmmeter.h> header file.  */
#undef HAVE_SYS_VMMETER_H

/* Define if you have the <sys/vmparam.h> header file.  */
#undef HAVE_SYS_VMPARAM_H

/* Define if you have the <sys/vmsystm.h> header file.  */
#undef HAVE_SYS_VMSYSTM_H

/* Define if you have the <syslog.h> header file.  */
#undef HAVE_SYSLOG_H

#ifndef LOG_DAEMON
#define       LOG_DAEMON      (3<<3)  /* system daemons */
#endif

/* Define if you have the <ufs/ffs/fs.h> header file.  */
#undef HAVE_UFS_FFS_FS_H

/* Define if you have the <ufs/fs.h> header file.  */
#undef HAVE_UFS_FS_H

/* Define if you have the <ufs/ufs/dinode.h> header file.  */
#undef HAVE_UFS_UFS_DINODE_H

/* Define if you have the <unistd.h> header file.  */
#undef HAVE_UNISTD_H

/* Define if you have the <utsname.h> header file.  */
#undef HAVE_UTSNAME_H

/* Define if you have the <vm/swap_pager.h> header file.  */
#undef HAVE_VM_SWAP_PAGER_H

/* Define if you have the <vm/vm.h> header file.  */
#undef HAVE_VM_VM_H

/* Define if you have the <xti.h> header file.  */
#undef HAVE_XTI_H

/* Define if you have the crypto library (-lcrypto).  */
#undef HAVE_LIBCRYPTO

/* Define if you have the elf library (-lelf).  */
#undef HAVE_LIBELF

/* Define if you have the kstat library (-lkstat).  */
#undef HAVE_LIBKSTAT

/* Define if you have the kvm library (-lkvm).  */
#undef HAVE_LIBKVM

/* Define if you have the m library (-lm).  */
#undef HAVE_LIBM

/* Define if you have the mld library (-lmld).  */
#undef HAVE_LIBMLD

/* Define if you have the nsl library (-lnsl).  */
#undef HAVE_LIBNSL

/* Define if you have the socket library (-lsocket).  */
#undef HAVE_LIBSOCKET

/* ifnet structure tests */
#undef STRUCT_IFNET_HAS_IF_BAUDRATE
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

/* arphd.at_next */
#undef STRUCT_ARPHD_HAS_AT_NEXT

/* ifaddr.ifa_next */
#undef STRUCT_IFADDR_HAS_IFA_NEXT

/* ifnet.if_mtu */
#undef STRUCT_IFNET_HAS_IF_MTU

/* ifnet needs to have _KERNEL defined */
#undef IFNET_NEEDS_KERNEL

/* sysctl works to get boottime, etc... */
#undef CAN_USE_SYSCTL

/* define if your compiler (processor) defines __FUNCTION__ for you */
#undef HAVE_CPP_UNDERBAR_FUNCTION_DEFINED

/* mib pointer to the top of the extensible tree.  This has been
 assigned to UCDavis by the iana group.  Optionally, point this to the
 location in the tree your company/organization has been allocated. */

/* location of the extensible mib tree */
#define EXTENSIBLEMIB 1,3,6,1,4,1,2021
/* location of the extensible mib tree */
#define EXTENSIBLEDOTMIB 1.3.6.1.4.1.2021
/* count the above numbers */
#define EXTENSIBLENUM 7

/* the ErrorFlag is V1 accessable because HP Openview does not support
   V2.  You can make this list of pairs as long as you want, just make
   sure to end it in -1.*/

#define SECURITYEXCEPTIONS {100,SNMPV1,-1} /* the ErrorFlag is V1 */

/* Mib-2 tree Info */
/* These are the system information variables. */

#define VERS_DESC   "unknown"             /* overridden at run time */
#define SYS_NAME    "unknown"             /* overridden at run time */

/* comment out the second define to turn off functionality for any of
   these: (See README for details) */

/*   proc PROCESSNAME [MAX] [MIN] */
#define PROCMIBNUM 2
#define USEPROCMIB

/*   exec/shell NAME COMMAND      */
#define SHELLMIBNUM 8
#define USESHELLMIB

/*   swap MIN                     */
#define MEMMIBNUM 4
#if defined(hpux9) || defined(bsdi2)
#define USEMEMMIB
#endif

/*   disk DISK MINSIZE            */
#define DISKMIBNUM 9
#if (HAVE_FSTAB_H || HAVE_SYS_STATVFS_H)
#define USEDISKMIB
#endif

/*   load 1 5 15                  */
#define LOADAVEMIBNUM 10
#define USELOADAVEMIB

/*   pass MIBOID command */
#define USEPASSMIB

/* which version are you using? This mibloc will tell you */
#define VERSIONMIBNUM 100
#define USEVERSIONMIB

/* Reports errors the agent runs into */
/* (typically its "can't fork, no mem" problems) */
#define ERRORMIBNUM 101
#define USEERRORMIB

/* The sub id of EXENSIBLEMIB returned to queries of
   .iso.org.dod.internet.mgmt.mib-2.system.sysObjectID.0 */
#define AGENTID 250

/* This ID is returned after the AGENTID above.  IE, the resulting
   value returned by a query to sysObjectID is
   EXTENSIBLEMIB.AGENTID.???, where ??? is defined below by OSTYPE */

#define HPUX9ID 1
#define SUNOS4ID 2
#define SOLARISID 3
#define OSFID 4
#define ULTRIXID 5
#define HPUX10ID 6
#define NETBSD1ID 7
#define FREEBSDID 8
#define IRIXID 9
#define LINUXID 10
#define BSDIID 11
#define OPENBSDID 12
#define UNKNOWNID 255

#ifdef hpux9
#define OSTYPE HPUX9ID
#endif
#ifdef hpux10
#define OSTYPE HPUX10ID
#endif
#ifdef sunos4
#define OSTYPE SUNOS4ID
#endif
#ifdef solaris2
#define OSTYPE SOLARISID
#endif
#if defined(osf3) || defined(osf4)
#define OSTYPE OSFID
#endif
#ifdef ultrix4
#define OSTYPE ULTRIXID
#endif
#ifdef netbsd1
#define OSTYPE NETBSD1ID
#endif
#ifdef freebsd2
#define OSTYPE FREEBSDID
#endif
#if defined(irix6) || defined(irix5)
#define OSTYPE IRIXID
#endif
#ifdef linux
#define OSTYPE LINUXID
#endif
#if defined(bsdi2) || defined(bsdi3)
#define OSTYPE BSDIID
#endif
#ifdef openbsd2
#define OSTYPE OPENBSDID
#endif
/* unknown */
#ifndef OSTYPE
#define OSTYPE UNKNOWNID
#endif

/* The enterprise number has been assigned by the IANA group.   */
/* Optionally, this may point to the location in the tree your  */
/* company/organization has been allocated.                     */
/* The assigned enterprise number for the NET_SNMP MIB modules. */
#define ENTERPRISE_OID			8072
#define ENTERPRISE_MIB			1,3,6,1,4,1,8072
#define ENTERPRISE_DOT_MIB		1.3.6.1.4.1.8072
#define ENTERPRISE_DOT_MIB_LENGTH	7

/* The assigned enterprise number for sysObjectID. */
#define SYSTEM_MIB		1,3,6,1,4,1,2021,AGENTID,OSTYPE
#define SYSTEM_DOT_MIB		1.3.6.1.4.1.2021.AGENTID.OSTYPE
#define SYSTEM_DOT_MIB_LENGTH	9

/* The assigned enterprise number for notifications. */
#define NOTIFICATION_MIB		1,3,6,1,4,1,2021,251
#define NOTIFICATION_DOT_MIB		1.3.6.1.4.1.2021.251
#define NOTIFICATION_DOT_MIB_LENGTH	8

/* this is the location of the ucdavis mib tree.  It shouldn't be
   changed, as the places it is used are expected to be constant
   values or are directly tied to the UCD-SNMP-MIB. */
#define UCDAVIS_OID		2021
#define UCDAVIS_MIB		1,3,6,1,4,1,2021
#define UCDAVIS_DOT_MIB		1.3.6.1.4.1.2021
#define UCDAVIS_DOT_MIB_LENGTH	7

/* how long to wait (seconds) for error querys before reseting the error trap.*/
#define ERRORTIMELENGTH 600

/* Exec command to fix PROC problems */
/* %s will be replaced by the process name in error */

#define PROCFIXCMD "/usr/local/bin/perl /local/scripts/fixproc %s"

/* Exec command to fix EXEC problems */
/* %s will be replaced by the exec/script name in error */

#define EXECFIXCMD "/usr/local/bin/perl /local/scripts/fixproc %s"

/* Should exec output Cashing be used (speeds up things greatly), and
   if so, After how many seconds should the cache re-newed?  Note:
   Don't define CASHETIME to disable cashing completely */

#define EXCACHETIME 30
#define CACHEFILE ".snmp-exec-cache"
#define MAXCACHESIZE (200*80)   /* roughly 200 lines max */

#define MAXDISKS 10                      /* can't scan more than this number */

/* misc defaults */

/* default of 100 meg minimum if the minimum size is not specified in
   the config file */
#define DEFDISKMINIMUMSPACE 100000

#define DEFMAXLOADAVE 12.0      /* default maximum load average before error */

#define MAXREADCOUNT 20   /* max times to loop reading output from
                             execs.  Because of sleep(1)s, this will also
                             be time to wait (in seconds) for exec to finish */

#define SNMPBLOCK 1       /* Set to 1 if you want snmpgets to block and never
                             timeout.  Original CMU code had this
                             hardcoded into the code as = 1 */

#define RESTARTSLEEP 5    /* How long to wait after a snmpset to
                             EXTENSIBLEMIB.VERSIONMIBNUM.VERRESTARTAGENT
                             before restarting the agent.  This is
                             necessary to finish the snmpset reply
                             before restarting. */

/* Number of community strings to store */
#define NUM_COMMUNITIES	5

/* UNdefine to allow specifying zero-length community string */
/* #define NO_ZEROLENGTH_COMMUNITY 1 */

/* #define EXIT_ON_BAD_KLREAD  */
/* define to exit the agent on a bad kernel read */

#define LASTFIELD -1      /* internal define */

/* Define if you have the strdup function.  */
#define HAVE_STRDUP 1

/* Define if you have the strerror function.  */
#define HAVE_STRERROR 1

#ifndef HAVE_STRCHR
#ifdef HAVE_INDEX
# define strchr index
# define strrchr rindex
#endif
#endif

#ifndef HAVE_INDEX
#ifdef HAVE_STRCHR
# define index strchr
# define rindex strrchr
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

/* Not-to-be-compiled macros for use by configure only */
#define config_require(x)
#define config_arch_require(x,y)
#define config_parse_dot_conf(w,x,y,z)
#define config_add_mib(x)

#ifndef DONT_INC_STRUCTS
/*#include "agent/mibgroup/struct.h" */
#endif

#ifndef linux
#ifndef solaris2
#define bsdlike
#endif
#endif

#ifdef WIN32

#define HAVE_GETPID 1

int strcasecmp(const char *s1, const char *s2);
#define vsnprintf _vsnprintf
#define snprintf  _snprintf

#define EADDRINUSE	WSAEADDRINUSE

#define ENV_SEPARATOR ";"
#define ENV_SEPARATOR_CHAR ';'


#else

#define ENV_SEPARATOR ":"
#define ENV_SEPARATOR_CHAR ':'

#endif

typedef unsigned short mode_t;

#define AGENT_DIRECTORY_MODE 0700

/* reverse encoding BER packets is both faster and more efficient in space. */
#define USE_REVERSE_ASNENCODING       1
#define DEFAULT_ASNENCODING_DIRECTION 1 /* 1 = reverse, 0 = forwards */

#ifndef NETSNMP_INLINE
#   define NETSNMP_NO_INLINE
#   define NETSNMP_INLINE
#endif

#ifdef __cplusplus
}
#endif

