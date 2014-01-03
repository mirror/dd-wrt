/*
* Copyright c                  Realsil Semiconductor Corporation, 2006
* All rights reserved.
* 
* Program :  IGMP glue file
* Abstract : 
* Author :qinjunjie 
* Email:qinjunjie1980@hotmail.com
*
*/
#ifndef RTL_MULTICAST_SNOOPING_GLUE
#define RTL_MULTICAST_SNOOPING_GLUE

#include "rtl_multicast_types.h"


/* supported chip: CONFIG_RTL8366S/CONFIG_RTL8306SDM */
/*#define CONFIG_RTL8306SDM	1*/
#define CONFIG_RTL8366S		1



#ifdef RTL865X_OVER_LINUX
#include <linux/config.h>
 #if defined(CONFIG_RTL8306SDM)
	#define MII_PORT_MASK		0x20
 #elif defined(CONFIG_RTL8366S)
	#include "rtl8366s_cpu.h"
 #endif
#endif

#if defined(RTL_MULTICAST_SNOOPING_TEST)
#define RTL8306_TBLBAK
uint32	 cpuPortNumer;
#elif defined(CONFIG_RTL8306SDM)
#define RTL8306_TBLBAK
#elif defined(CONFIG_RTL8366S)
#endif


#define swapl32(x)\
        ((((x) & 0xff000000U) >> 24) | \
         (((x) & 0x00ff0000U) >>  8) | \
         (((x) & 0x0000ff00U) <<  8) | \
         (((x) & 0x000000ffU) << 24))
#define swaps16(x)        \
        ((((x) & 0xff00) >> 8) | \
         (((x) & 0x00ff) << 8))

#ifdef _LITTLE_ENDIAN

#ifndef ntohs
	#define ntohs(x)   (swaps16(x))
#endif 

#ifndef ntohl
	#define ntohl(x)   (swapl32(x))
#endif

#ifndef htons
	#define htons(x)   (swaps16(x))
#endif

#ifndef htonl
	#define htonl(x)   (swapl32(x))
#endif

#else

#ifndef ntohs
	#define ntohs(x)	(x)
#endif 

#ifndef ntohl
	#define ntohl(x)	(x)
#endif

#ifndef htons
	#define htons(x)	(x)
#endif

#ifndef htonl
	#define htonl(x)	(x)
#endif

#endif

#ifdef __KERNEL__
	#define rtl_gluePrintf printk
#else
	#define rtl_gluePrintf printf 
#endif


#ifndef RTL_MULTICAST_SNOOPING_TEST
extern int32 rtl_glueMutexLock(void);
extern int32 rtl_glueMutexUnlock(void);
#else
extern int testdrvMutex;

#define rtl_glueMutexLock()\
	do { \
		testdrvMutex ++;\
	} while (0)
	
#define rtl_glueMutexUnlock()\
	do{\
		testdrvMutex --;\
		if (testdrvMutex < 0)\
		{\
		printf("Error: Driver Mutex Lock/Unlcok is not balance");\
		}\
	}while (0)


#endif



void *rtl_glueMalloc(uint32 NBYTES);
void rtl_glueFree(void *memblock);

#ifdef RTL_MULTICAST_SNOOPING_TEST
void   rtl_glueInit(void);
int32  Software_addMuticastMacAddress(uint8 *macAddr,uint32 isAuth, uint32 portMask);
int32  Software_deleteMacAddress(uint8 *macAddr);
uint8  Software_forward(uint8 *macFrame);
#else
void rtl_glueNicSend(uint8 *macFrame, uint32 macFrameLen);
void rtl_multicastSnoopingV2TimeUpdate(void);
#endif


#endif



