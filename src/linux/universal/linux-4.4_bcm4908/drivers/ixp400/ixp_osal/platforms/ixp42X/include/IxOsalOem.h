/**
 * @file IxOsalOem.h
 *
 * @brief this file contains platform-specific defines.
 * 
 * 
 * @par
 * IXP400 SW Release version 2.4
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2007, Intel Corporation.
 * All rights reserved.
 * 
 * @par
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Intel Corporation nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * 
 * @par
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * 
 * @par
 * -- End of Copyright Notice --
 */

#ifndef IxOsalOem_H
#define IxOsalOem_H

/* This might cause a problem. Try including after OsOem.h in case 
 * of issues */
#include "IxOsalTypes.h"

/* OS-specific header for Platform package */
#include "IxOsalOsOem.h"

/*
 * Platform Name
 */
#define IX_OSAL_PLATFORM_NAME ixp400

/*
 * Cache line size
 */
#define IX_OSAL_CACHE_LINE_SIZE (32)

/* Compiler specific endianness selector - WARNING this works only with arm gcc, use appropriate defines with diab */
#ifndef __wince
 
#if defined (__ARMEL__)
    #ifndef __LITTLE_ENDIAN
        #define __LITTLE_ENDIAN
    #endif /* _LITTLE_ENDIAN */
#elif defined (__ARMEB__)   ||  (CPU == SIMSPARCSOLARIS) || (CPU == SIMLINUX)
    #ifndef __BIG_ENDIAN
        #define __BIG_ENDIAN
    #endif /* __BIG_ENDIAN */
#else
    #error Error, could not identify target endianness
#endif /* endianness selector no WinCE OSes */
													  
#else /* ndef __wince */
													   
#define __LITTLE_ENDIAN
													    
#endif /* def __wince */

#ifndef __wince

/* From IxOsalIoMem.h */
#ifdef __XSCALE__

#ifdef _DIAB_TOOL
 
__asm volatile UINT32 ixOsalCoreWordSwap (UINT32 wordIn)
{
% reg wordIn;
! "r0", "r1"
 
   eor r1, wordIn, wordIn, ror #16; /* R1 = A^C, B^D, C^A, D^B */
   bic r1, r1, #0x00ff0000;         /* R1 = A^C, 0  , C^A, D^B */
   mov r0, wordIn, ror #8;          /* wordOut(R0) = D,   A,   B,   C   */
   eor r0, r0, r1, lsr #8;          /* wordOut(R0) = D,   C,   B,   A   */
}  /* return value is returned through register R0 */
 
#else /* _DIAB_TOOL not defined */
 
static __inline__ UINT32
ixOsalCoreWordSwap (UINT32 wordIn)
{
    /*
     * Storage for the swapped word 
     */
    UINT32 wordOut;
 
    /*
     * wordIn = A, B, C, D 
     */
    __asm__ (" eor r1, %1, %1, ror #16;"        /* R1 =      A^C, B^D, C^A, D^B */
             " bic r1, r1, #0x00ff0000;"        /* R1 =      A^C, 0  , C^A, D^B */
             " mov %0, %1, ror #8;"     /* wordOut = D,   A,   B,   C   */
             " eor %0, %0, r1, lsr #8;" /* wordOut = D,   C,   B,   A   */
  : "=r" (wordOut): "r" (wordIn):"r1");
 
    return wordOut;
} 
 
#endif /* #ifdef _DIAB_TOOL */

#define IX_OSAL_OEM_SWAP_LONG(wData)          (ixOsalCoreWordSwap(wData))

#else

#define IX_OSAL_OEM_SWAP_LONG(wData)	  ((wData >> 24) | (((wData >> 16) & 0xFF) << 8) | (((wData >> 8) & 0xFF) << 16) | ((wData & 0xFF) << 24))

#endif  /* __XSCALE__ */

/* Various default module inclusions */

/* Memory related */
#if defined (IX_OSAL_LINUX_LE)
    #define IX_SDRAM_LE_DATA_COHERENT
#endif

/* Memory related */
#define IX_OSAL_READ_LONG_BE(wAddr)          IX_OSAL_BE_BUSTOXSL(IX_OSAL_READ_LONG_IO((volatile UINT32 *) (wAddr) ))
#define IX_OSAL_READ_SHORT_BE(sAddr)         IX_OSAL_BE_BUSTOXSS(IX_OSAL_READ_SHORT_IO((volatile UINT16 *) (sAddr) ))
#define IX_OSAL_READ_BYTE_BE(bAddr)          IX_OSAL_BE_BUSTOXSB(IX_OSAL_READ_BYTE_IO((volatile UINT8 *) (bAddr) ))
#define IX_OSAL_WRITE_LONG_BE(wAddr, wData)  IX_OSAL_WRITE_LONG_IO((volatile UINT32 *) (wAddr), IX_OSAL_BE_XSTOBUSL((UINT32) (wData) ))
#define IX_OSAL_WRITE_SHORT_BE(sAddr, sData) IX_OSAL_WRITE_SHORT_IO((volatile UINT16 *) (sAddr), IX_OSAL_BE_XSTOBUSS((UINT16) (sData) ))
#define IX_OSAL_WRITE_BYTE_BE(bAddr, bData)  IX_OSAL_WRITE_BYTE_IO((volatile UINT8 *) (bAddr), IX_OSAL_BE_XSTOBUSB((UINT8) (bData) ))

#endif /* def __wince */
 
 /* Define LE AC macros */
#define IX_OSAL_READ_LONG_LE_AC(wAddr)          IX_OSAL_READ_LONG_IO((volatile UINT32 *) IX_OSAL_LE_AC_BUSTOXSL((UINT32) (wAddr) )) 
#define IX_OSAL_READ_SHORT_LE_AC(sAddr)         IX_OSAL_READ_SHORT_IO((volatile UINT16 *) IX_OSAL_LE_AC_BUSTOXSS((UINT32) (sAddr) )) 
#define IX_OSAL_READ_BYTE_LE_AC(bAddr)          IX_OSAL_READ_BYTE_IO((volatile UINT8 *) IX_OSAL_LE_AC_BUSTOXSB((UINT32) (bAddr) ))
#define IX_OSAL_WRITE_LONG_LE_AC(wAddr, wData)  IX_OSAL_WRITE_LONG_IO((volatile UINT32 *) IX_OSAL_LE_AC_XSTOBUSL((UINT32) (wAddr) ), (UINT32) (wData))
#define IX_OSAL_WRITE_SHORT_LE_AC(sAddr, sData) IX_OSAL_WRITE_SHORT_IO((volatile UINT16 *) IX_OSAL_LE_AC_XSTOBUSS((UINT32) (sAddr) ), (UINT16) (sData))
#define IX_OSAL_WRITE_BYTE_LE_AC(bAddr, bData)  IX_OSAL_WRITE_BYTE_IO((volatile UINT8 *) IX_OSAL_LE_AC_XSTOBUSB((UINT32) (bAddr) ), (UINT8) (bData))

/* Define LE DC macros */
 
 #define IX_OSAL_READ_LONG_LE_DC(wAddr)          ixOsalDataCoherentLongReadSwap((volatile UINT32 *) (wAddr) )
 #define IX_OSAL_READ_SHORT_LE_DC(sAddr)         ixOsalDataCoherentShortReadSwap((volatile UINT16 *) (sAddr) )
 #define IX_OSAL_READ_BYTE_LE_DC(bAddr)          IX_OSAL_LE_DC_BUSTOXSB(IX_OSAL_READ_BYTE_IO((volatile UINT8 *) (bAddr) ))
 #define IX_OSAL_WRITE_LONG_LE_DC(wAddr, wData)  ixOsalDataCoherentLongWriteSwap((volatile UINT32 *) (wAddr), (UINT32) (wData))
 #define IX_OSAL_WRITE_SHORT_LE_DC(sAddr, sData) ixOsalDataCoherentShortWriteSwap((volatile UINT16 *) (sAddr), (UINT16) (sData))
 #define IX_OSAL_WRITE_BYTE_LE_DC(bAddr, bData)  IX_OSAL_WRITE_BYTE_IO((volatile UINT8 *) (bAddr), IX_OSAL_LE_DC_XSTOBUSB((UINT8) (bData)))



#define IX_OSAL_BE_XSTOBUSL(wData)  (wData)
#define IX_OSAL_BE_XSTOBUSS(sData)  (sData)
#define IX_OSAL_BE_XSTOBUSB(bData)  (bData)
#define IX_OSAL_BE_BUSTOXSL(wData)  (wData)
#define IX_OSAL_BE_BUSTOXSS(sData)  (sData)
#define IX_OSAL_BE_BUSTOXSB(bData)  (bData)
 
#define IX_OSAL_LE_AC_XSTOBUSL(wAddr) (wAddr)
#define IX_OSAL_LE_AC_XSTOBUSS(sAddr) IX_OSAL_SWAP_SHORT_ADDRESS(sAddr)
#define IX_OSAL_LE_AC_XSTOBUSB(bAddr) IX_OSAL_SWAP_BYTE_ADDRESS(bAddr)
#define IX_OSAL_LE_AC_BUSTOXSL(wAddr) (wAddr)
#define IX_OSAL_LE_AC_BUSTOXSS(sAddr) IX_OSAL_SWAP_SHORT_ADDRESS(sAddr)
#define IX_OSAL_LE_AC_BUSTOXSB(bAddr) IX_OSAL_SWAP_BYTE_ADDRESS(bAddr)
  
#define IX_OSAL_LE_DC_XSTOBUSL(wData) IX_OSAL_SWAP_LONG(wData)
#define IX_OSAL_LE_DC_XSTOBUSS(sData) IX_OSAL_SWAP_SHORT(sData)
#define IX_OSAL_LE_DC_XSTOBUSB(bData) (bData)
#define IX_OSAL_LE_DC_BUSTOXSL(wData) IX_OSAL_SWAP_LONG(wData)
#define IX_OSAL_LE_DC_BUSTOXSS(sData) IX_OSAL_SWAP_SHORT(sData)
#define IX_OSAL_LE_DC_BUSTOXSB(bData) (bData)


/* Inline functions are required here to avoid reading the same I/O location 2 or 4 times for the byte swap */
static __inline__ UINT32
ixOsalDataCoherentLongReadSwap (volatile UINT32 * wAddr)
{
    UINT32 wData = IX_OSAL_READ_LONG_IO (wAddr);
    return IX_OSAL_LE_DC_BUSTOXSL (wData);
}
		 
static __inline__ UINT16
ixOsalDataCoherentShortReadSwap (volatile UINT16 * sAddr)
{
    UINT16 sData = IX_OSAL_READ_SHORT_IO (sAddr);
    return IX_OSAL_LE_DC_BUSTOXSS (sData);
}
				  
static __inline__ void
ixOsalDataCoherentLongWriteSwap (volatile UINT32 * wAddr, UINT32 wData)
{
    wData = IX_OSAL_LE_DC_XSTOBUSL (wData);
	IX_OSAL_WRITE_LONG_IO (wAddr, wData);
}
						   
static __inline__ void
ixOsalDataCoherentShortWriteSwap (volatile UINT16 * sAddr, UINT16 sData)
{
	sData = IX_OSAL_LE_DC_XSTOBUSS (sData);
    IX_OSAL_WRITE_SHORT_IO (sAddr, sData);
}


/* Platform-specific fastmutex implementation */
PUBLIC IX_STATUS ixOsalOemFastMutexTryLock (IxOsalFastMutex * mutex);

/* Platform-specific init */
PUBLIC IX_STATUS
ixOsalOemInit (void);

/* Platform-specific unload */
PUBLIC void
ixOsalOemUnload (void);


/* Memory mapping table init. This function is public within OSAL only */
PUBLIC IX_STATUS
ixOsalMemMapInit (IxOsalMemoryMap *map, UINT32 numElement);

/* IX_OSAL_LE is an invalid endianness type for IXP4XX platforms */
#define IX_OSAL_OEM_COMPONENT_COHERENCY_MODE_CHECK(fun_name, return_exp) \
		ixOsalLog(IX_OSAL_LOG_LVL_ERROR, IX_OSAL_LOG_DEV_STDOUT, #fun_name": Please specify component coherency mode to use MEM functions \n", 0, 0, 0, 0, 0, 0); \
		return_exp;

#endif /* IxOsalOem_H */
