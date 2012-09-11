/* obj/sysdep/autoconf.h.  Generated automatically by configure.  */
/*
 *	This file contains all system parameters automatically
 *	discovered by the configure script.
 */

/* System configuration file */
#define SYSCONF_INCLUDE "sysdep/cf/linux.h"

/* Include debugging code */
/* #undef DEBUGGING */

/* 8-bit integer type */
#define INTEGER_8 char

/* 16-bit integer type */
#define INTEGER_16 short int

/* 32-bit integer type */
#define INTEGER_32 int

#define INTEGER_64 long long

/* CPU endianity */
#include <endian.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN  /* nbd fix for swapped version and length field */
#define CPU_LITTLE_ENDIAN 1
#define _LITTLE_ENDIAN 1
#elif __BYTE_ORDER == __BIG_ENDIAN
#define CPU_BIG_ENDIAN 1
#define _BIG_ENDIAN 1
#else
#error "cannot detect endianess"
#endif


/* Usual alignment for structures */
#define CPU_STRUCT_ALIGN 8

/* Characteristics of time_t */
/* #undef TIME_T_IS_64BIT */
#define TIME_T_IS_SIGNED 1

/* We have struct ip_mreqn in <netinet/in.h> */
#define HAVE_STRUCT_IP_MREQN 1

/* Protocols compiled in */
#define CONFIG_STATIC 1
#define CONFIG_RIP 1
#define CONFIG_BGP 1
#define CONFIG_OSPF 1

/* We have <syslog.h> and syslog() */
#define HAVE_SYSLOG 1

/* Are we using dmalloc? */
/* #undef HAVE_LIBDMALLOC */
