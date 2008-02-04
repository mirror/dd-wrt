#include <Copyright.h>
/********************************************************************************
* msSample.h
*
* DESCRIPTION:
*       Types definitions for Sample program
*
* DEPENDENCIES:   Platform.
*
* FILE REVISION NUMBER:
*
*******************************************************************************/

#ifndef __pfTesth
#define __pfTesth

#ifdef _VXWORKS
#include "vxWorks.h"
#include "logLib.h"
#endif
#include "stdio.h"
#include "stdarg.h"
#include "stdlib.h"
#include "time.h"
#include "string.h"

#include "msApi.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef GT_U32 (*GT_API_VOID) (GT_QD_DEV*);
typedef int (*GT_CMP_FUNC) (void*, int, int);

typedef GT_STATUS (*GT_API_SET_BOOL) (GT_QD_DEV*, GT_BOOL);
typedef GT_STATUS (*GT_API_GET_BOOL) (GT_QD_DEV*, GT_BOOL*);

typedef GT_STATUS (*GT_API_SET_U16) (GT_QD_DEV*, GT_U16);
typedef GT_STATUS (*GT_API_GET_U16) (GT_QD_DEV*, GT_U16*);

typedef GT_STATUS (*GT_API_SET_U32) (GT_QD_DEV*, GT_U32);
typedef GT_STATUS (*GT_API_GET_U32) (GT_QD_DEV*, GT_U32*);

typedef GT_STATUS (*GT_API_MAC_ADDR) (GT_QD_DEV*, GT_ETHERADDR*);

typedef GT_STATUS (*GT_API_SET_PORT_BOOL) (GT_QD_DEV*, GT_LPORT,GT_BOOL);
typedef GT_STATUS (*GT_API_GET_PORT_BOOL) (GT_QD_DEV*, GT_LPORT,GT_BOOL*);

typedef GT_STATUS (*GT_API_SET_PORT_U16) (GT_QD_DEV*, GT_LPORT,GT_U16);
typedef GT_STATUS (*GT_API_GET_PORT_U16) (GT_QD_DEV*, GT_LPORT,GT_U16*);

typedef GT_STATUS (*GT_API_SET_PORT_U32) (GT_QD_DEV*, GT_LPORT,GT_U32);
typedef GT_STATUS (*GT_API_GET_PORT_U32) (GT_QD_DEV*, GT_LPORT,GT_U32*);

typedef GT_STATUS (*GT_API_SET_PORT_U8) (GT_QD_DEV*, GT_LPORT,GT_U8);
typedef GT_STATUS (*GT_API_GET_PORT_U8) (GT_QD_DEV*, GT_LPORT,GT_U8*);

typedef struct _TEST_API
{
	union
	{
		GT_API_SET_BOOL bool;
		GT_API_SET_U16 u16;
		GT_API_SET_U32 u32;
		GT_API_MAC_ADDR mac;
		GT_API_SET_PORT_BOOL port_bool;
		GT_API_SET_PORT_U8 port_u8;
		GT_API_SET_PORT_U16 port_u16;
		GT_API_SET_PORT_U32 port_u32;
	} setFunc;

	union
	{
		GT_API_GET_BOOL bool;
		GT_API_GET_U16 u16;
		GT_API_GET_U32 u32;
		GT_API_MAC_ADDR mac;
		GT_API_GET_PORT_BOOL port_bool;
		GT_API_GET_PORT_U8 port_u8;
		GT_API_GET_PORT_U16 port_u16;
		GT_API_GET_PORT_U32 port_u32;
	} getFunc;

}TEST_API;

typedef struct _TEST_STRUCT
{
	char strTest[16];
	GT_API_VOID testFunc;
	GT_U32 testResults;
} TEST_STRUCT;

#define MSG_PRINT(x) testPrint x

#define MAX_MAC_ENTRIES	    8192
#define TEST_MAC_ENTRIES 	64
#define MAX_BUCKET_SIZE		2048

typedef struct _TEST_ATU_ENTRY
{
	GT_ATU_ENTRY atuEntry[TEST_MAC_ENTRIES];
}TEST_ATU_ENTRY;

typedef struct _ATU_ENTRY_INFO
{
	GT_ATU_ENTRY atuEntry;
	GT_U16	hash;
	GT_U16	bucket;
} ATU_ENTRY_INFO;

extern GT_SYS_CONFIG   pfTestSysCfg;
extern ATU_ENTRY_INFO *gAtuEntry;
extern GT_QD_DEV       *dev;

GT_STATUS qdStart(int,int,int);
GT_STATUS qdSimSetPhyInt(unsigned int portNumber, unsigned short u16Data);
GT_STATUS qdSimSetGlobalInt(unsigned short u16Data);

GT_STATUS testAll(GT_QD_DEV*);
void testPrint(char* format, ...);
GT_U32 testATU(GT_QD_DEV *dev);
GT_STATUS testATUStress(GT_QD_DEV *dev);
GT_U32 testVTU(GT_QD_DEV *dev);

extern FGT_INT_HANDLER qdIntHandler;

int vtuEntryCmpFunc(void* buf, int a, int b);
int atuEntryCmpFunc(void* buf, int a, int b);
GT_STATUS gtSort(int list[], GT_CMP_FUNC cmpFunc, void* buf, GT_U32 len);
GT_U16 createATUList(GT_QD_DEV *dev, TEST_ATU_ENTRY atuEntry[], GT_U16 entrySize, GT_U16 dbNumSize, 
					GT_U16 sameMacsInEachDb, GT_U16 bSize);
GT_STATUS testFillUpAtu(GT_QD_DEV *dev, ATU_ENTRY_INFO *atuEntry, GT_U8 atuSize, 
					GT_U32 dbNum, GT_U16 first2Bytes, GT_ATU_UC_STATE state);
GT_U16 runQDHash(GT_U8* eaddr, GT_U16 dbNum, int bSize, GT_U16* pHash, 
					GT_U16* preBucket, GT_U16* posBucket);
GT_STATUS testDisplayATUList(GT_QD_DEV *dev);

#ifdef USE_SEMAPHORE
GT_SEM osSemCreate(GT_SEM_BEGIN_STATE state);
GT_STATUS osSemDelete(GT_SEM smid);
GT_STATUS osSemWait(GT_SEM smid, GT_U32 timeOut);
GT_STATUS osSemSignal(GT_SEM smid);
#endif

GT_BOOL gtBspReadMii ( GT_QD_DEV* dev, unsigned int portNumber , unsigned int MIIReg,
                      unsigned int* value);
GT_BOOL gtBspWriteMii ( GT_QD_DEV* dev, unsigned int portNumber , unsigned int MIIReg,
                       unsigned int value);
void gtBspMiiInit(GT_QD_DEV* dev);

GT_BOOL qdSimRead (GT_QD_DEV* dev,unsigned int portNumber , unsigned int miiReg, unsigned int* value);
GT_BOOL qdSimWrite (GT_QD_DEV* dev,unsigned int portNumber , unsigned int miiReg, unsigned int value);
void qdSimInit(GT_DEVICE devId, int baseAddr);


#if 1 /* This is for testing */

#define TEST_MASK		0
#define TEST_MASK_0		0
#define TEST_MASK_1		1
#define TEST_MASK_2		2
#define TEST_MASK_3		3
#define TEST_MASK_4		4
#define TEST_MASK_5		5
#define TEST_MASK_6		6
#define TEST_MASK_7		7

#define GET_TEST_MASK(_index, _mask)	\
		((_index == 0)?_mask##_0:		\
		(_index == 1)?_mask##_1:		\
		(_index == 2)?_mask##_2:		\
		(_index == 3)?_mask##_3:		\
		(_index == 4)?_mask##_4:		\
		(_index == 5)?_mask##_5:		\
		(_index == 6)?_mask##_6:		\
		(_index == 7)?_mask##_7:0)

#endif


#ifdef __cplusplus
}
#endif

#endif   /* __pfTesth */

