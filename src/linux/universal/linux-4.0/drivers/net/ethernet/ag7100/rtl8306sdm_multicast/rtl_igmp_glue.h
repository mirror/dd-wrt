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

#include "rtl_igmp_types.h"
#ifndef RTL_IGMP_SNOOPING_TEST
#define IGMP_DEBUG	/*should be removed when release code*/
#define RTL8651B_SDK  /*should be removed when release code*/
#define  RTL_CPU_HW_FWD /*should be removed when release code*/
#endif

#ifdef RTL_IGMP_SNOOPING_TEST
 uint32 cpuPortNumer;
#endif

#define MII_PORT_MASK 0x20 /*should be modified by user*/

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
	#define rtl_igmpGluePrintf printk
#else
	#define rtl_igmpGluePrintf printf 
#endif


#ifndef RTL_IGMP_SNOOPING_TEST
extern int32 rtl_igmpGlueMutexLock(void);
extern int32 rtl_igmpGlueMutexUnlock(void);
#else
extern int testdrvMutex;

#define rtl_igmpGlueMutexLock()\
	do { \
		testdrvMutex ++;\
	} while (0)
	
#define rtl_igmpGlueMutexUnlock()\
	do {\
		testdrvMutex --;\
		if (testdrvMutex < 0)\
		{\
			printf("%s (%d) Error: Driver Mutex Lock/Unlcok is not balance (%d).\n", __FUNCTION__, __LINE__, testdrvMutex);\
		}\
	} while (0)

#endif



void *rtl_igmpGlueMalloc(uint32 NBYTES);
void rtl_igmpGlueFree(void *memblock);

#ifndef RTL_IGMP_SNOOPING_TEST
void rtl_igmpGlueNicSend(uint8 *macFrame, uint32 macFrameLen, uint8 fwdPortMask);
void rtl_multicastSnoopingV1TimeUpdate(void);

#endif

#ifdef RTL_CPU_HW_FWD
void rtl_igmpDisCpuHwFwd(uint32 groupAddress);
#endif

#ifdef RTL_IGMP_SNOOPING_TEST
void   rtl_igmpGlueInit(void);
int32  rtl8306_addMuticastMacAddress(uint8 *macAddr,uint32 isAuth, uint32 portMask, uint32 *entryaddr);
int32  rtl8306_deleteMacAddress(uint8 * macAddr, uint32 *entryaddr);
uint8  rtl8306_forward(uint8* macFrame);
#endif





