#ifndef _RTK_TYPES_H_
#define _RTK_TYPES_H_


#ifndef _RTL_TYPES_H

#ifdef EMBEDDED_SUPPORT

#include <common_types.h>

#define CONST_T               far const

#else
typedef unsigned long long    uint64;
typedef long long				      int64;
typedef unsigned int			    uint32;
typedef int                   int32;
typedef unsigned short        uint16;
typedef short                 int16;
typedef unsigned char         uint8;
typedef char                  int8;

#define CONST_T               const

#endif

typedef uint32               ipaddr_t;
typedef uint32				 memaddr;	

#ifndef ETHER_ADDR_LEN
#define ETHER_ADDR_LEN		6
#endif

typedef struct ether_addr_s {
	uint8 octet[ETHER_ADDR_LEN];
} ether_addr_t;

#define swapl32(x)\
        ((((x) & 0xff000000U) >> 24) | \
         (((x) & 0x00ff0000U) >>  8) | \
         (((x) & 0x0000ff00U) <<  8) | \
         (((x) & 0x000000ffU) << 24))
#define swaps16(x)        \
        ((((x) & 0xff00) >> 8) | \
         (((x) & 0x00ff) << 8))  


#ifdef _LITTLE_ENDIAN
	#define ntohs(x)   (swaps16(x))
	#define ntohl(x)   (swapl32(x))
	#define htons(x)   (swaps16(x))
	#define htonl(x)   (swapl32(x))
#else
	#define ntohs(x)	(x)
	#define ntohl(x)	(x)
	#define htons(x)	(x)
	#define htonl(x)	(x)
#endif


#ifdef _LITTLE_ENDIAN
	#define MEM16(x)		(x)
#else
	#define MEM16(x)		(swaps16(x)) 
#endif
#endif /*_RTL_TYPES_H*/

/* type abstraction */
#ifdef EMBEDDED_SUPPORT

typedef int16                   rtk_api_ret_t;
typedef int16                   ret_t;
typedef uint32                  rtk_u_long_t;

#else

typedef int32                   rtk_api_ret_t;
typedef int32                   ret_t;
typedef uint64                  rtk_u_long_t;

#endif

#ifndef NULL
#define NULL 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef FAILED
#define FAILED -1
#endif

#ifdef __KERNEL__
#define rtlglue_printf printk
#else
#define rtlglue_printf printf
#endif
#define PRINT			rtlglue_printf



#endif /* _RTL8370_TYPES_H_ */
