/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell 
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.


********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or 
modify this File in accordance with the terms and conditions of the General 
Public License Version 2, June 1991 (the "GPL License"), a copy of which is 
available along with the File in the license.txt file or by writing to the Free 
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or 
on the worldwide web at http://www.gnu.org/licenses/gpl.txt. 

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED 
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY 
DISCLAIMED.  The GPL License provides additional details about this warranty 
disclaimer.
*******************************************************************************/
/*******************************************************************************
* mvOsCpuArchLib.c - Marvell CPU architecture library
*
* DESCRIPTION:
*       This library introduce Marvell API for OS dependent CPU architecture 
*       APIs. This library introduce single CPU architecture services APKI 
*       cross OS.
*
* DEPENDENCIES:
*       None.
*
*******************************************************************************/


#ifndef __INCmvLinuxPpch
#define __INCmvLinuxPpch

#include "mvCommon.h"

#define CPU_PHY_MEM(x)			    (MV_U32)x
#define CPU_MEMIO_CACHED_ADDR(x)    (void*)x
#define CPU_MEMIO_UNCACHED_ADDR(x)	(void*)x


/* CPU architecture dependent 32, 16, 8 bit read/write IO addresses */
#define MV_MEMIO32_WRITE(addr, data)	\
    ((*((volatile unsigned int*)(addr))) = ((unsigned int)(data)))

#define MV_MEMIO32_READ(addr)       	\
    ((*((volatile unsigned int*)(addr))))

#define MV_MEMIO16_WRITE(addr, data)	\
    ((*((volatile unsigned short*)(addr))) = ((unsigned short)(data)))

#define MV_MEMIO16_READ(addr)       	\
    ((*((volatile unsigned short*)(addr))))

#define MV_MEMIO8_WRITE(addr, data) 	\
    ((*((volatile unsigned char*)(addr))) = ((unsigned char)(data)))

#define MV_MEMIO8_READ(addr)        	\
    ((*((volatile unsigned char*)(addr))))


/* No Fast Swap implementation (in assembler) for ARM */
#define MV_32BIT_LE_FAST(val)            MV_32BIT_LE(val)
#define MV_16BIT_LE_FAST(val)            MV_16BIT_LE(val)
#define MV_32BIT_BE_FAST(val)            MV_32BIT_BE(val)
#define MV_16BIT_BE_FAST(val)            MV_16BIT_BE(val)
    
/* 32 and 16 bit read/write in big/little endian mode */

/* 16bit write in little endian mode */
#define MV_MEMIO_LE16_WRITE(addr, data) \
        MV_MEMIO16_WRITE(addr, MV_16BIT_LE_FAST(data))

/* 16bit read in little endian mode */
static __inline MV_U16 MV_MEMIO_LE16_READ(MV_U32 addr)
{
	MV_U16 data;

	data= (MV_U16)MV_MEMIO16_READ(addr);

	return (MV_U16)MV_16BIT_LE_FAST(data);
}

/* 32bit write in little endian mode */
#define MV_MEMIO_LE32_WRITE(addr, data) \
        MV_MEMIO32_WRITE(addr, MV_32BIT_LE_FAST(data))

/* 32bit read in little endian mode */
static __inline MV_U32 MV_MEMIO_LE32_READ(MV_U32 addr)
{
	MV_U32 data;

	data= (MV_U32)MV_MEMIO32_READ(addr);

	return (MV_U32)MV_32BIT_LE_FAST(data);
}


/* Flash APIs */
#define MV_FL_8_READ            MV_MEMIO8_READ
#define MV_FL_16_READ           MV_MEMIO_LE16_READ
#define MV_FL_32_READ           MV_MEMIO_LE32_READ
#define MV_FL_8_DATA_READ       MV_MEMIO8_READ
#define MV_FL_16_DATA_READ      MV_MEMIO16_READ
#define MV_FL_32_DATA_READ      MV_MEMIO32_READ
#define MV_FL_8_WRITE           MV_MEMIO8_WRITE
#define MV_FL_16_WRITE          MV_MEMIO_LE16_WRITE
#define MV_FL_32_WRITE          MV_MEMIO_LE32_WRITE
#define MV_FL_8_DATA_WRITE      MV_MEMIO8_WRITE
#define MV_FL_16_DATA_WRITE     MV_MEMIO16_WRITE
#define MV_FL_32_DATA_WRITE     MV_MEMIO32_WRITE


/* CPU cache information */
#define CPU_I_CACHE_LINE_SIZE   32    /* 2do: replace 32 with linux core macro */
#define CPU_D_CACHE_LINE_SIZE   32    /* 2do: replace 32 with linux core macro */



/* Data cache flush one line */
#define mvOsCacheLineFlushInv(addr)                                                             \
{                                                               \
  __asm__ __volatile__ ("mcr p15, 0, %0, c7, c14, 1" : : "r" (addr));\
}
 
#define mvOsCacheLineInv(addr)                                                                  \
{                                                               \
  __asm__ __volatile__ ("mcr p15, 0, %0, c7, c6, 1" : : "r" (addr)); \
}


/* Flush CPU pipe */
#define CPU_PIPE_FLUSH

/* ARM architecture APIs */


MV_U32  mvOsCpuRevGet (MV_VOID);
MV_U32  mvOsCpuPartGet (MV_VOID);
MV_U32  mvOsCpuArchGet (MV_VOID);
MV_U32  mvOsCpuVarGet (MV_VOID);
MV_U32  mvOsCpuAsciiGet (MV_VOID);


#endif /* __INCmvLinuxPpch */
