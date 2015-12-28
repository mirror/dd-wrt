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

#include "rtl_multicast_types.h"

#ifndef RTL_MULTICAST_SNOOPING_GLUE
#define RTL_MULTICAST_SNOOPING_GLUE

#define MII_PORT_MASK 0x20 /*should be modified by user*/

#ifdef RTL_MULTICAST_SNOOPING_TEST
 uint32 cpuPortNumer;
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

#ifndef RTL_MULTICAST_SNOOPING_TEST
void rtl_glueNicSend(uint8 *macFrame, uint32 macFrameLen);
void rtl_multicastSnoopingV2TimeUpdate(void);

#endif

#ifdef RTL_MULTICAST_SNOOPING_TEST
void   rtl_glueInit(void);
int32  rtl8306_addMuticastMacAddress(uint8 *macAddr,uint32 isAuth, uint32 portMask);
int32  rtl8306_deleteMacAddress(uint8 * macAddr);
uint8  rtl8306sdm_forward(uint8* macFrame);
#endif


#endif



