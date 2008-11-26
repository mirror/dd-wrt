#if !defined(COM_UTIL_H)
#define COM_UTIL_H

#include "com_type.h"

#define MV_ZeroMemory(buffer, byte_count)				memset(buffer, 0, byte_count)
#define MV_FillMemory(buffer, byte_count, pattern)		memset(buffer, pattern, byte_count)
#define MV_CopyMemory(dest, source, byte_count)			memcpy(dest, source, byte_count)

void MV_ZeroMvRequest(PMV_Request pReq);
void MV_CopySGTable(PMV_SG_Table pTargetSGTable, PMV_SG_Table pSourceSGTable);

MV_BOOLEAN MV_Equals(MV_PU8	des, MV_PU8	src, MV_U8 len);
/* The following definition is for Intel big endian cpu. */
#define CPU_TO_BIG_ENDIAN_32(x)		\
	( ((MV_U32)((MV_U8)(x)))<<24 | ((MV_U32)((MV_U8)((x)>>8)))<<16 | ((MV_U32)((MV_U8)((x)>>16)))<<8 | ((MV_U32)((MV_U8)((x)>>24))) )

//#define CPU_TO_BIG_ENDIAN_64(x)
//#define CPU_TO_BIG_ENDIAN_16(x)

/* -------- Added by xxp --------*/
#ifndef _OS_BIOS
#define CPU_TO_BIG_ENDIAN_64(x)		\
	( ((_MV_U64)(CPU_TO_BIG_ENDIAN_32(x.low))) << 32 |	\
	  CPU_TO_BIG_ENDIAN_32(x.high) )
#else
MV_U64 CPU_TO_BIG_ENDIAN_64(MV_U64 x);

#endif

#define CPU_TO_BIG_ENDIAN_16(x)		\
	( ((MV_U16)((MV_U8)(x))) << 8 | (MV_U16)((MV_U8)((x)>>8)) )
/* -------- End -------- */

void SGTable_Init(
	OUT PMV_SG_Table pSGTable,
	IN MV_U8 flag
	);

void SGTable_Append(
	OUT PMV_SG_Table pSGTable,
	MV_U32 address,
	MV_U32 addressHigh,
	MV_U32 size
	);

MV_BOOLEAN SGTable_Available(
	IN PMV_SG_Table pSGTable
	);

void MV_DumpRequest(PMV_Request pReq, MV_BOOLEAN detail);
#ifdef SUPPORT_RAID6
void MV_DumpXORRequest(PMV_XOR_Request pXORReq, MV_BOOLEAN detail);
#endif /* SUPPORT_RAID6 */
void MV_DumpSGTable(PMV_SG_Table pSGTable);
const char* MV_DumpSenseKey(MV_U8 sense);

MV_U32 MV_CRC(
	IN	MV_PU8		pData, 
	IN	MV_U16		len
);

#define MV_MOD_ADD(value, mod)	do {	\
	(value)++;							\
	if ((value)>=(mod)) (value)=0;		\
	} while (0);

#endif

