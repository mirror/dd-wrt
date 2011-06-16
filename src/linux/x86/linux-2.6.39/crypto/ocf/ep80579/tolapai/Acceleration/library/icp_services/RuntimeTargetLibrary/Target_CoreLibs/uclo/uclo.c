/**
 **************************************************************************
 * @file uclo.c
 *
 * @description
 *      This file provides Ucode Object File Loader facilities
 *
 * @par 
 * This file is provided under a dual BSD/GPLv2 license.  When using or 
 *   redistributing this file, you may do so under either license.
 * 
 *   GPL LICENSE SUMMARY
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify 
 *   it under the terms of version 2 of the GNU General Public License as
 *   published by the Free Software Foundation.
 * 
 *   This program is distributed in the hope that it will be useful, but 
 *   WITHOUT ANY WARRANTY; without even the implied warranty of 
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 *   General Public License for more details.
 * 
 *   You should have received a copy of the GNU General Public License 
 *   along with this program; if not, write to the Free Software 
 *   Foundation, Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 *   The full GNU General Public License is included in this distribution 
 *   in the file called LICENSE.GPL.
 * 
 *   Contact Information:
 *   Intel Corporation
 * 
 *   BSD LICENSE 
 * 
 *   Copyright(c) 2007,2008,2009 Intel Corporation. All rights reserved.
 *   All rights reserved.
 * 
 *   Redistribution and use in source and binary forms, with or without 
 *   modification, are permitted provided that the following conditions 
 *   are met:
 * 
 *     * Redistributions of source code must retain the above copyright 
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright 
 *       notice, this list of conditions and the following disclaimer in 
 *       the documentation and/or other materials provided with the 
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its 
 *       contributors may be used to endorse or promote products derived 
 *       from this software without specific prior written permission.
 * 
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * 
 *  version: Security.L.1.0.3-98
 *
 **************************************************************************/ 

#include "uclo_platform.h"
#include "core_io.h"
#include "uclo_dev.h"
#include "uclo.h"
#include "uclo_helper.h"
#include "uof_dbg.h"
#include "uof_prf.h"

#include "hal_sram.h"
#include "hal_dram.h"
#include "hal_scratch.h"
#include "hal_ae.h"
#include "hal_global.h"
#include "IxOsal.h"
#include "halAeApi.h"
#include "uclo_overlay.h"

#define USTORE_8K        0x2000            /* 8 kbytes ustore size */    
#define UOF_CFID_CC      0xCC              /* .uof file ID */
#define UOF_CFID_C3      0xC3              /* .uof file ID */
#define PRE_CUR_MAJVER   1                 /* .uof major version */ 
#define PRE_CUR_MINVER   2                 /* .uof minor version */
#define SHRAM_SIZE       0x8000            /* 32 kbytes shared ram size */

#define UCLO_ALLAE       0xff00ff          /* all AE mask */
#define ICP_UWORD_MASK   0x3ffffffffffull  /* micro-word mask without parity */

#define CHAN0    0                         /* DRAM channel 0 - NCDRAM */
#define CHAN1    1                         /* DRAM channel 1 - CDRAM */

#define BAD_EP80579_HWAE_MASK     0xfffffff0

#define UWORD_CPYBUF_SIZE 1024             /* micro-store copy buffer size(bytes) */

#define UENGINE_ID "__uengine_id"          /* micro engine ID */  
#define USTORE_DRAM_BASE_SYM "__USTORE_DRAM_BASE"  /* micro-store dram base address symbol */

extern int UcloExp_evalPostfix(uclo_objHandle_T *pObjHandle, 
                               unsigned char swAe,
                               char *pExp, 
                               int *pRes);

unsigned int UcLo_getSwAe(unsigned int prodType, 
                          unsigned char aeNum);
unsigned int UcLo_getHwAe(unsigned int prodType, 
                          unsigned char aeNum);
int UcLo_WriteNumUword(void *handle, 
                       unsigned int aeMask, 
                       unsigned int phyUaddr,
                       unsigned int numWords, 
                       uword_T *uWord);
char *UcLo_getString(uof_strTab_T *strTable, 
                     unsigned int strOffset);
int halAe_getMMScheme(void);
static void UcLo_wrScratchBytes(unsigned int addr, 
                                unsigned int *val,
                                unsigned int numBytes);
uword_T Uclo_getUofUword(uclo_objHandle_T *objHandle, 
                         uof_encapPage_T *page, 
                         unsigned int addr);
int UcLo_computeFreeUstore(uclo_objHandle_T *objHandle);
static int UcLo_fixupExpr(uclo_objHandle_T *objHandle, 
                          uof_encapPage_T *page,
                          unsigned char swAe);
uof_encapAe_T *UcLo_getEncapImage(uclo_objHandle_T *objHandle, 
                                  char *ucodeImageName);
int UcLo_GetMeSliceImageName(void *handle, 
                             unsigned char hwAe, unsigned int slice,
                             ucLo_ImageAssign_T *imageAssign);
void UcLo_wrScratch(unsigned int byteAddr, 
                    unsigned int *val, 
                    unsigned int count);                   
int UcLo_fixupLocals(uclo_objHandle_T *objHandle, 
                     uof_encapPage_T *page,
                     unsigned char swAe);
static int UcLo_fixupLocalNeighReg(uclo_objHandle_T *objHandle, 
                                   uof_encapPage_T *page,
                                   unsigned char swAe);
static int UcLo_fixupGlobals(uclo_objHandle_T *objHandle, 
                             uof_encapPage_T *page);
static int UcLo_initGlobals(uclo_objHandle_T *objHandle);
static int UcLo_initReg(unsigned char hwAe, 
                        unsigned char ctxMask, 
                        icp_RegType_T regType,
                        unsigned short regAddr, 
                        unsigned int value);
static int UcLo_initSym(uclo_objHandle_T *objHandle, 
                        unsigned char hwAe, 
                        char *symName,
                        unsigned int offset, 
                        unsigned int value);
int UcLo_bindIvd(void *objHandle);
static int UcLo_initRegSymExpr(uclo_objHandle_T *objHandle, 
                               unsigned char hwAe, 
                               uof_encapAe_T *encapAe);
void UcLo_ProcessHalCallbacks(Hal_UcloCallReason_T reason,
                              unsigned int         arg0,
                              unsigned int*        arg1,
                              void*                user_data);

void halAe_setUofChecksum(unsigned int uofChecksum);
                               
static Hal_SysMemInfo_T SysMemInfo;
static unsigned int LmemByteSize=(MAX_LMEM_REG << 2);    /* 2560 bytes */
static unsigned int BadHwAeMask = BAD_EP80579_HWAE_MASK;

uclo_objHandle_T *UcLo_getObjHandle(void);
int UcLo_overlayObj(uclo_objHandle_T *objHandle, 
                    int readOnly);

int UcLo_isProfileSupported(void);

/*-----------------------------------------------------------------------------
   Function:    UcLo_getSwAe
   Description: return the AE number as used by this software.
                EP80579 AE numbering is 0x00-0x03
                Internal AE numbering is 0x00-0x0f
   Returns:     UCLO_BADAE, or SW AE number
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
unsigned int 
UcLo_getSwAe(unsigned int prodType, 
             unsigned char aeNum)
{
    if(((unsigned int)aeNum >= UCLO_MAX_AES) || ((1 << (unsigned int)aeNum) & BadHwAeMask)) 
    {
        return (UCLO_BADAE);
    }    

    if(prodType & (EP80579_CPU_TYPE)) 
    {
        aeNum = aeNum & 0x3;
    }    
    else 
    { 
        return (UCLO_BADAE);
    }
    return (aeNum);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_getHwAe
   Description: return the AE number as used by the hadrware.
                EP80579 AE numbering is 0x00-0x03
                Internal AE numbering is 0x00-0x0f
   Returns:     UCLO_BADAE, or the HW AE number
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
unsigned int 
UcLo_getHwAe(unsigned int prodType, 
             unsigned char aeNum)
{
    if(prodType & (EP80579_CPU_TYPE)) 
    {
        aeNum = aeNum & 0x3;
    }    
    else 
    {
        return (UCLO_BADAE);
    }
    return (aeNum);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_getString
   Description: Gets string from the string table.
   Returns:     pointer to the string or NULL if not found
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
char *
UcLo_getString(uof_strTab_T *strTable, 
               unsigned int strOffset)
{
    if((!strTable->tableLen) || (strOffset > strTable->tableLen)) 
    {
        return (NULL);
    }    
    return ((char *)(strTable->strings + strOffset));
}

/*-----------------------------------------------------------------------------
   Function:     UcLo_verifyVersion
   Description: Set the file format version in the handle.
   Returns:        UCLO_SUCCESS, UCLO_UOFVERINCOMPAT
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_verifyVersion(short *minVer, 
                   short *majVer)
{
    if((*majVer == UOF_CFID_C3) && (*minVer == UOF_CFID_CC))
    {
        /* pre UOF v2.0 supported versions... Releases 1.2, & 1.3 */
        *minVer = PRE_CUR_MINVER;
        *majVer = PRE_CUR_MAJVER;
        return (UCLO_UOFVERINCOMPAT);
    }
    else
    { 
      if((*minVer != UOF_MINVER) || (*majVer != UOF_MAJVER)) 
      {
          return (UCLO_UOFVERINCOMPAT);
      }
    }
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_verifyFile
   Description: Verify whether the file is an UOF. If it is, then the UOF format
                version of the file will be returned in the minVer, and majVer
                arguments.
   Returns:     UCLO_FAILURE, UCLO_SUCCESS, UCLO_UOFINCOMPAT, UCLO_UOFVERINCOMPAT
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_verifyFile(char *fileHdr, 
                short *minVer, 
                short *majVer)
{
    uof_fileHdr_T    fHdr;

    /* copy fileHdr so we can modified it without any reprecussion */
    memmove(&fHdr, fileHdr, sizeof(uof_fileHdr_T));

    /* check new format proper, endian */
    if(fHdr.fileId != UOF_FID) 
    {
        return (UCLO_FAILURE);
    }
    
    *minVer = fHdr.minVer & 0xff; /* The valid data of minVer is 8 LSBs */
    *majVer = fHdr.majVer & 0xff; /* The valid data of majVer is 8 LSBs */
    return (UcLo_verifyVersion(minVer, majVer));
}

/*-----------------------------------------------------------------------------
   Function: UcLo_wrSramBytes    
   Description: Write a specified number of bytes to SRAM.
   Returns: 
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static void 
UcLo_wrSramBytes(unsigned int addr, 
                 unsigned int *val, 
                 unsigned int numBytes)
{
    unsigned int  outVal, cpSize=4;
    unsigned char *chPtr=(unsigned char *)val;

    while(numBytes)
    {
        if(numBytes < 4)
        {
            cpSize = numBytes;
            outVal = SRAM_READ(addr);
        }
        memmove(&outVal, chPtr, cpSize);
        SRAM_WRITE(addr, outVal);
        numBytes -= cpSize;
        chPtr += cpSize;
        addr += cpSize;
    }
    return;
}

/*-----------------------------------------------------------------------------
   Function: UcLo_wrDramBytes    
   Description: Write a specified number of bytes to DRAM.
   Returns: 
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static void 
UcLo_wrDramBytes(unsigned int channel, 
                 unsigned int addr,
                 unsigned int *val, 
                 unsigned int numBytes)
{
    unsigned int  outVal, cpSize=4;
    unsigned char *chPtr=(unsigned char *)val;

    while(numBytes)
    {
        if(numBytes < 4)
        {
            cpSize = numBytes;
            if(channel == 0) 
            {
                 outVal = DRAM_READ_CH0(addr);
            }    
            else
            {
                 outVal = DRAM_READ_CH1(addr);
            }     
        }
        memmove(&outVal, chPtr, cpSize);
        DRAM_WRITE_XA(channel, addr, outVal);
        numBytes -= cpSize;
        chPtr += cpSize;
        addr += cpSize;
    }
    return;
}

/*-----------------------------------------------------------------------------
   Function: UcLo_wrScratchBytes    
   Description: Write a specified number of bytes to SCRATCH.
   Returns: 
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static void 
UcLo_wrScratchBytes(unsigned int addr, 
                    unsigned int *val,
                    unsigned int numBytes)
{
    unsigned int  outVal, cpSize=4;
    unsigned char *chPtr=(unsigned char *)val;

    while(numBytes)
    {
        if(numBytes < 4)
        {
            cpSize = numBytes;
            outVal = SCRATCH_READ(addr);
        }
        memmove(&outVal, chPtr, cpSize);
        SCRATCH_WRITE(addr, outVal);
        numBytes -= cpSize;
        chPtr += cpSize;
        addr += cpSize;
    }
    return;
}

/*-----------------------------------------------------------------------------
   Function: UcLo_wrLocalMemBytes    
   Description: Write a specified number of bytes to LOCAL-MEM.
   Returns: 
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static void 
UcLo_wrLocalMemBytes(unsigned char hwAe, 
                     unsigned int addr,
                     unsigned int *val, 
                     unsigned int numBytes)
{
    unsigned int  outVal, cpSize=4;
    unsigned char *chPtr=(unsigned char *)val;
    
    addr >>= 2;

    while(numBytes)
    {
        if(numBytes < 4)
        {
            cpSize = numBytes;
            halAe_GetLM(hwAe, (unsigned short)addr, &outVal);
        }
        memmove(&outVal, chPtr, cpSize);
        halAe_PutLM(hwAe, addr++, outVal);
        numBytes -= cpSize;
        chPtr += cpSize;
    }

    return;
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_wrUstoreMemBytes    
   Description: Write a specified number of bytes to USTORE-MEM starting at the
                byte-address specified by addr.
   Returns: 
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static void 
UcLo_wrUstoreMemBytes(unsigned char hwAe, 
                      unsigned int addr,
                      unsigned int *val, 
                      unsigned int numBytes)
{
    unsigned int  outVal, cpSize=4;
    unsigned char *chPtr=(unsigned char *)val;
    
    addr >>= 2; /* convert to uword address */

    while(numBytes)
    {
        if(numBytes < 4)
        {
            cpSize = numBytes;
            halAe_GetUmem(hwAe, addr, 1, &outVal);
        }
        memmove(&outVal, chPtr, cpSize);
        halAe_PutUmem(hwAe, addr++, 1, &outVal);
        numBytes -= cpSize;
        chPtr += cpSize;
    }

    return;
}

/*-----------------------------------------------------------------------------
   Function: UcLo_wrSram    
   Description: Write a specified number of longwords to SRAM
   Returns: 
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static void 
UcLo_wrSram(unsigned int addr, 
            unsigned int *val, 
            unsigned int count)
{
    unsigned int i=0;

    for(i = 0; i < count; i++)
    {
        SRAM_WRITE(addr+(i<<2), val[i]);
    }
    return;
}

/*-----------------------------------------------------------------------------
   Function: UcLo_wrDram    
   Description: Write a specified number of longwords to DRAM
   Returns: 
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static void 
UcLo_wrDram(unsigned int channel, 
            unsigned int addr, 
            unsigned int *val, 
            unsigned int count)
{
    unsigned int i=0;

    for(i = 0; i < count; i++)
    {
        DRAM_WRITE_XA(channel, addr+(i<<2), val[i]);
    }
    return;
}

/*-----------------------------------------------------------------------------
   Function: UcLo_wrScratch    
   Description: Write a specified number of longwords to SCRATCH PAD MEMORY
   Returns: 
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
void 
UcLo_wrScratch(unsigned int byteAddr, 
               unsigned int *val, \
               unsigned int count)
{
    unsigned int i = 0;

    for(i = 0; i < count; i++)
    {
        SCRATCH_WRITE(byteAddr+(i<<2), val[i]);
    }
    return;
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_wrLocalMem    
   Description: Write a specified number of longwords to LOCAL-MEMORY
   Returns: 
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static void 
UcLo_wrLocalMem(unsigned char hwAe, 
                unsigned int byteAddr,
                unsigned int *val, 
                unsigned int count)
{
    unsigned int i = 0;

    byteAddr >>= 2;
    for(i = 0; i < count; i++)
    {
        halAe_PutLM(hwAe, byteAddr+i, val[i]);
    }
    return;
}

/*-----------------------------------------------------------------------------
   Function:     UcLo_findMemSym
   Description:  Locates a memory-resident symbol
   Returns:      Pointer to the symbol, or NULL
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
uof_initMem_T *
UcLo_findMemSym(uclo_objHandle_T *objHandle, 
                char *symName)
{
    unsigned int i;
    uof_initMem_T *initMem;
    char          *str;

    if(!objHandle) {
        return (NULL);
    }    

    initMem = objHandle->initMemTab.initMem;
    for(i = 0; i < objHandle->initMemTab.numEntries; i++)
    {
        str = UcLo_getString(&objHandle->strTable, initMem->symName);
        if(!str)
        {
            continue;
        }    
        if(!strcmp(str, symName)) 
        {
            return (initMem);
        }    
        initMem = (uof_initMem_T *)((unsigned int)((unsigned int)initMem + sizeof(uof_initMem_T)) +
                  (sizeof(uof_memValAttr_T) * initMem->numValAttr));
    }
    return (NULL);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_sramSet
   Description: Init a sram region to a specific value
   Returns:     UCLO_SUCCESS
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_sramSet(unsigned int addr, 
             unsigned int value,
             unsigned int numBytes)
{
    unsigned int wrSize=4;

    while(numBytes)
    {
        if(numBytes < 4) 
        {
           wrSize = numBytes;
        }   
        UcLo_wrSramBytes(addr, &value, wrSize);
        addr += wrSize;
        numBytes -= wrSize;
    }

    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:     UcLo_dramSet
   Description: Init a dram region to a specific value
   Returns:     UCLO_SUCCESS
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_dramSet(unsigned int chan, 
             unsigned int addr,
             unsigned int value,
             unsigned int numBytes)
{
    unsigned int wrSize=4;

    while(numBytes)
    {
        if(numBytes < 4) 
        {
           wrSize = numBytes;
        }   
        UcLo_wrDramBytes(chan, addr, &value, wrSize);
        addr += wrSize;
        numBytes -= wrSize;
    }

    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    zeroMemRegion
   Description: Initialize a memory-resident symbol to zero
   Returns:     UCLO_BADOBJ, UCLO_SUCCESS
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
zeroMemRegion(uclo_objHandle_T *objHandle, 
              uof_initMem_T *initMem)
{
    unsigned int wrSize=4, zero=0, addr, hwAe, numBytes;
    unsigned int ustoreSize;
    unsigned int ScratchOffset, SramOffset, NCDramOffset, CDramOffset;

    numBytes = initMem->numBytes;
    addr = initMem->addr;

    halAe_GetMemoryStartOffset(&ScratchOffset, &SramOffset, &NCDramOffset, &CDramOffset); 

    switch(initMem->region)
    {
    case SRAM0_REGION:
        if((addr + SramOffset + numBytes) > (SysMemInfo.sramChan[0].sramSize)) 
        {
            return (UCLO_EXCDSRAM0SIZE);
        }    
        UcLo_sramSet(addr+SramOffset, zero, numBytes);
        break;
    case DRAM_REGION:
        /* offset into the non-coherent dram region */
        if((addr < SysMemInfo.dramDesc[0].aeDramOffset) || 
           ((addr + NCDramOffset + numBytes) > (SysMemInfo.dramDesc[0].aeDramOffset + SysMemInfo.dramDesc[0].aeDramSize)))
        {
            return (UCLO_EXCDDRAMSIZE);
        }    
        UcLo_dramSet(0, addr+NCDramOffset, zero, numBytes);
        break;
    case DRAM1_REGION:
            /* remap to channel zero */
        if((addr < SysMemInfo.dramDesc[1].aeDramOffset) ||
           ((addr + CDramOffset + numBytes) > (SysMemInfo.dramDesc[1].aeDramOffset + SysMemInfo.dramDesc[1].aeDramSize))) 
        {
            return (UCLO_EXCDDRAM1SIZE);
        }    
        UcLo_dramSet(1, addr+CDramOffset, zero, numBytes);
        break;
    case SCRATCH_REGION:
        if((addr + ScratchOffset + numBytes) > SCRATCH_SIZE) 
        {
            return (UCLO_EXCDSCRTHSIZE);
        }    
        while(numBytes)
        {
            if(numBytes < 4) 
            {
               wrSize = numBytes;
            }   
            UcLo_wrScratchBytes(addr+ScratchOffset, &zero, wrSize);
            addr += wrSize;
            numBytes -= wrSize;
        }
        break;
    case LMEM_REGION:
        if((addr + numBytes) > LmemByteSize) 
        {
            return (UCLO_EXCDLMEMSIZE);
        }    
        if(UcLo_parseNum(UcLo_getString(&objHandle->strTable, initMem->symName), (int *)(&hwAe))) 
        {
            return (UCLO_BADOBJ);
        }    
        if(hwAe >= MAX_AE) 
        {
            return (UCLO_BADOBJ);                
        }    
        while(numBytes)
        {
            if(numBytes < 4) 
            { 
               wrSize = numBytes;
            }   
            UcLo_wrLocalMemBytes((char)hwAe, addr, &zero, wrSize);
            addr += wrSize;
            numBytes -= wrSize;
        }
        break;
    case UMEM_REGION:
        if(UcLo_parseNum(UcLo_getString(&objHandle->strTable, initMem->symName), (int *)(&hwAe))) 
        {
            return (UCLO_BADOBJ);
        }    
        if(hwAe >= MAX_AE) 
        {
            return (UCLO_BADOBJ);                
        }    
        ustoreSize = objHandle->aeData[objHandle->swAe[hwAe]].effectUstoreSize;
        if((addr + numBytes) > (ustoreSize << 2)) 
        {
            return (UCLO_EXCDUMEMSIZE);
        }    
        while(numBytes)
        {
            if(numBytes < 4) 
            {
               wrSize = numBytes;
            }   
            UcLo_wrUstoreMemBytes((char)hwAe, addr, &zero, wrSize);
            addr += wrSize;
            numBytes -= wrSize;
        }
        break;
    case SHRAM_REGION:
        /* the loader is not supposed to initialize the shram region and this code won't be executed
          in case if we want to support it in future, just add code to zero the shram region */
        if((addr + numBytes) > SHRAM_SIZE) 
        {
            return (UCLO_EXCDSHRAMSIZE);
        }    
        break;
    default: return (UCLO_BADOBJ);
    }
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    initMemory
   Description: Init the memory of a symbol
   Returns:     UCLO_EXCDSRAM0SIZE, UCLO_EXCDDRAMSIZE, UCLO_EXCDSCRTHSIZE,
                UCLO_EXCDUMEMSIZE, UCLO_FAILURE, UCLO_BADOBJ, UCLO_SUCCESS
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
initMemory(uclo_objHandle_T *objHandle, 
           uof_initMem_T *initMem)
{
    unsigned int i, size=4, hwAe, swAe, uAddr, s;
    unsigned int ustoreSize;
    uof_memValAttr_T *memValAttr;
    unsigned int ScratchOffset, SramOffset, NCDramOffset, CDramOffset;

    memValAttr = (uof_memValAttr_T *)((unsigned int)initMem + sizeof(uof_initMem_T));
    halAe_GetMemoryStartOffset(&ScratchOffset, &SramOffset, &NCDramOffset, &CDramOffset); 

    switch(initMem->region)
    {
    /* write the bytes values to the appropriate memory region.
       The values are stored as bytes and should be written as such to avoid
       system-endian conversion */
    case SRAM0_REGION:
        if((SramOffset + initMem->addr + initMem->numBytes) > (SysMemInfo.sramChan[0].sramSize)) 
        {
            return (UCLO_EXCDSRAM0SIZE);
        }    
        for(i=0; i < initMem->numValAttr; i++)
        {
            UcLo_wrSramBytes(SramOffset + initMem->addr + memValAttr->byteOffset,
                &memValAttr->value, size);
            memValAttr++;
        }
        break;
    case DRAM_REGION:
            /* offset into the non-coherent dram region */
        if((initMem->addr < SysMemInfo.dramDesc[0].aeDramOffset) || 
           ((NCDramOffset + initMem->addr + initMem->numBytes) > (SysMemInfo.dramDesc[0].aeDramOffset + SysMemInfo.dramDesc[0].aeDramSize))) 
        {
            return (UCLO_EXCDDRAMSIZE);
        }    
        for(i=0; i < initMem->numValAttr; i++)
        {
            UcLo_wrDramBytes(CHAN0, NCDramOffset + initMem->addr + memValAttr->byteOffset, &memValAttr->value, size);
            memValAttr++;
        }
        break;
    case DRAM1_REGION:
        /* offset into the coherent dram region */
        if((initMem->addr < SysMemInfo.dramDesc[1].aeDramOffset) ||
           ((CDramOffset + initMem->addr + initMem->numBytes) > (SysMemInfo.dramDesc[1].aeDramOffset + SysMemInfo.dramDesc[1].aeDramSize))) 
        {
            return (UCLO_EXCDDRAM1SIZE);
        }    
        for(i=0; i < initMem->numValAttr; i++)
        {
            UcLo_wrDramBytes(CHAN1, CDramOffset + initMem->addr + memValAttr->byteOffset, &memValAttr->value, size);
            memValAttr++;
        }
        break;
    case SCRATCH_REGION:
        if((ScratchOffset + initMem->addr + initMem->numBytes) > SCRATCH_SIZE) 
        {
            return (UCLO_EXCDSCRTHSIZE);
        }    
        for(i=0; i < initMem->numValAttr; i++)
        {
            UcLo_wrScratchBytes(ScratchOffset + initMem->addr + memValAttr->byteOffset,
                &memValAttr->value, size);
            memValAttr++;
        }
        break;
    case LMEM_REGION:
        if((initMem->addr + initMem->numBytes) > LmemByteSize) 
        {
            return (UCLO_EXCDLMEMSIZE);
        }    
        if(initMem->scope != LOCAL_SCOPE) 
        {
            return (UCLO_BADOBJ);
        }    
        if(UcLo_parseNum(UcLo_getString(&objHandle->strTable, initMem->symName), (int *)(&hwAe))) 
        {
            return (UCLO_BADOBJ);
        }    
        if(hwAe >= MAX_AE) 
        {
            return (UCLO_BADOBJ);                
        }    
        for(i=0; i < initMem->numValAttr; i++)
        {
            UcLo_wrLocalMemBytes((char)hwAe, initMem->addr + memValAttr->byteOffset,
                &memValAttr->value, size);
            memValAttr++;
        }
        break;
    case UMEM_REGION:
        if(initMem->scope != LOCAL_SCOPE)
        {
            return (UCLO_BADOBJ);
        }    
        if(UcLo_parseNum(UcLo_getString(&objHandle->strTable, initMem->symName), (int *)(&hwAe))) 
        {
            return (UCLO_BADOBJ);
        }    
        if(hwAe >= MAX_AE) 
        {
            return (UCLO_BADOBJ);
        }    

        ustoreSize = objHandle->aeData[objHandle->swAe[hwAe]].effectUstoreSize;
        if((initMem->addr + initMem->numBytes) > (ustoreSize << 2)) 
        {
            return (UCLO_EXCDUMEMSIZE);
        }
        
        for(i=0; i < initMem->numValAttr; i++)
        {
            UcLo_wrUstoreMemBytes((char)hwAe, initMem->addr + memValAttr->byteOffset,
                &memValAttr->value, size);
            memValAttr++;
        }
        /* set the highest ustore address referenced */
        if((swAe = UcLo_getSwAe(objHandle->prodType, (unsigned char)hwAe)) == UCLO_BADAE) 
        {
            return (UCLO_BADARG);
        }    
        uAddr = (initMem->addr + initMem->numBytes) >> 2;
        for(s=0; s < objHandle->aeData[swAe].numSlices; s++)
        {
            if(objHandle->aeData[swAe].aeSlice[s].encapImage->numUwordsUsed < uAddr) 
            {
                objHandle->aeData[swAe].aeSlice[s].encapImage->numUwordsUsed = uAddr;
            }    
        }
        break;
    case SHRAM_REGION:
        /* the loader is not supposed to initialize the shram region and this code won't be executed
           in case if we want to support it in future, just add code to initialize the shram region */
        if((initMem->addr + initMem->numBytes) > SHRAM_SIZE) 
        {
            return (UCLO_EXCDSHRAMSIZE);
        }    
        break;
    default: return (UCLO_BADOBJ);
    }
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:     UcLo_initMemory
   Description:  Initialize allocated memory
   Returns:      UCLO_SUCCESS, UCLO_BADARG, UCLO_FAILURE, UCLO_EXCDLMEMSIZE,
                 UCLO_EXCDUMEMSIZE, UCLO_EXCDSRAM0SIZE, UCLO_EXCDDRAMSIZE, 
                 UCLO_EXCDSCRTHSIZE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_initMemory(uclo_objHandle_T *objHandle)
{
    unsigned int    i;
    int             status = UCLO_SUCCESS;
    uof_initMem_T   *initMem;
    unsigned int NCDramBaseAddr, CDramBaseAddr;
    const unsigned int SramBaseAddr = 0xFFFC0000;
    unsigned int ScratchOffset, SramOffset, NCDramOffset, CDramOffset;

    if(!objHandle) 
    {
        return (UCLO_BADARG);
    } 

    /* Verify that the offsets meet the alignment requirements of the memory regions */
    if((halAe_GetNCDramBaseAddr(&NCDramBaseAddr) != HALAE_SUCCESS) ||
       (halAe_GetCDramBaseAddr(&CDramBaseAddr) != HALAE_SUCCESS))
    {
        return (UCLO_FAILURE);
    }
    halAe_GetMemoryStartOffset(&ScratchOffset, &SramOffset, &NCDramOffset, &CDramOffset); 
    if ((ScratchOffset % objHandle->encapUofObj.varMemSeg->scratchAlignment) != 0 ||
        ((SramBaseAddr + SramOffset) % objHandle->encapUofObj.varMemSeg->sram0Alignment) != 0 ||
        ((NCDramBaseAddr + NCDramOffset) % objHandle->encapUofObj.varMemSeg->sdramAlignment) != 0 ||
        ((CDramBaseAddr + CDramOffset) % objHandle->encapUofObj.varMemSeg->sdram1Alignment) != 0)
    {
        return (UCLO_FAILURE);
    }

    /* First zero memory blocks, just in case there is any overlap of the symbol regions.  Otherwise
       the zeroing of one symbol could clobber the initialized values of the overlapping region. */
    initMem = objHandle->initMemTab.initMem;
    for(i = 0; i < objHandle->initMemTab.numEntries; i++)
    {
        if(initMem->numBytes) 
        { 
            if((status = zeroMemRegion(objHandle, initMem))) 
            {
                return (status);
            }    
        }    

        initMem = (uof_initMem_T *)((unsigned int)((unsigned int)initMem + sizeof(uof_initMem_T)) +
                  (sizeof(uof_memValAttr_T) * initMem->numValAttr));
    }

    /* Now that we have zeroed memory in all symbol regions above, we can perform initializations. */
    initMem = objHandle->initMemTab.initMem;
    for(i = 0; i < objHandle->initMemTab.numEntries; i++)
    {
        if(initMem->numBytes) 
        {
            if((status = initMemory(objHandle, initMem))) 
            {
                return (status);
            }    
        }
        initMem = (uof_initMem_T *)((unsigned int)((unsigned int)initMem + sizeof(uof_initMem_T)) +
                  (sizeof(uof_memValAttr_T) * initMem->numValAttr));
    }
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:     UcLo_findChunk
   Description: Locates UOF chunk header matching the type specified by the
                input parameter 'chunkId' and excluding the current chunk.
   Returns: pointer to the chunk header or NULL if not found
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
void *
UcLo_findChunk(uof_objHdr_T *objHdr, 
               char *chunkId, 
               void *cur)
{
    int i;
    uof_chunkHdr_T *chunkHdr = (uof_chunkHdr_T *)((unsigned int)objHdr + sizeof(uof_objHdr_T));

    for(i = 0; i < objHdr->numChunks; i++) 
    {
      if((cur < (void *)&chunkHdr[i]) && !(strncmp(chunkHdr[i].chunkId, chunkId, UOF_OBJID_LEN))) 
      {
          return (&chunkHdr[i]);
      }    
    }  
    return (NULL);
}

/*-----------------------------------------------------------------------------
   Function:     Uclo_getUofUword
   Description: Given a valid uword address, return the associated uword from
                the UOF. The function will fail if uAddr is an invalid or unused
                address. NOTE: the "addr" parameter is the address relative to the
                start of the page.
   Returns:     uword value, or INVLD_UWORD
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
uword_T 
Uclo_getUofUword(uclo_objHandle_T *objHandle, 
                 uof_encapPage_T *page, 
                 unsigned int addr)
{
    uword_T uwrd=0;
    unsigned int i;

    if(!page) 
    {
        return (INVLD_UWORD);
    }    

    /* find the block */
    for(i = 0; i < page->numUwBlocks; i++)
    {
        if((addr >= page->uwBlocks[i].startAddr) && (addr <= (page->uwBlocks[i].startAddr + (page->uwBlocks[i].numWords-1))))
        {
            /* unpack n bytes and assigned to the 64-bit uword value.
            note: the microwords are stored as packed bytes.
            */
            addr -= page->uwBlocks[i].startAddr;
            addr *= objHandle->uWordBytes;
            memmove(&uwrd, &page->uwBlocks[i].microWords[addr], objHandle->uWordBytes);
            uwrd = uwrd & ICP_UWORD_MASK;

            return (uwrd);
        }
    }

    return (INVLD_UWORD);
}

/*-----------------------------------------------------------------------------
   Function:    Uclo_setUofUword
   Description: Given a valid uword address, set the associated uword in
                the UOF. The function will fail if uAddr is an invalid or unused
                address.
   Returns:     0 = success, -1 = failure
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
Uclo_setUofUword(uclo_objHandle_T *objHandle, 
                 uof_encapPage_T *page,
                 unsigned int addr, 
                 uword_T uwrd)
{
    unsigned int i;

    if(!page) 
    {
        return (-1);
    }    

    /* find the block */
    for(i = 0; i < page->numUwBlocks; i++)
    {
        if((addr >= page->uwBlocks[i].startAddr) && (addr <= (page->uwBlocks[i].startAddr + (page->uwBlocks[i].numWords-1))))
        {
            /* assigned n bytes to microwords -- stored as packed bytes. */
            addr -= page->uwBlocks[i].startAddr;
            addr *= objHandle->uWordBytes;

            memmove(&page->uwBlocks[i].microWords[addr], &uwrd, objHandle->uWordBytes);
            return (0);
        }
    }
    return (-1);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_mapFileChunk
   Description: Maps the file header -- must be the first entry in the buffer.
   Returns:     Pointer to the chunk or NULL
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static uclo_objHdr_T *
UcLo_mapFileChunk(char *buf, 
                  uof_fileHdr_T *fileHdr, 
                  char *chunkId)
{
    int                    i;
    uof_fileChunkHdr_T  *fileChunk;
    void                *chunk;
    uclo_objHdr_T        *objHdr;

    fileChunk = (uof_fileChunkHdr_T *)(buf + sizeof(uof_fileHdr_T));
    /* find the chunk, verify it, return a pointer to it */
    for(i = 0; i < fileHdr->numChunks; i++)
    {
        if(!(strncmp(fileChunk->chunkId, chunkId, UOF_OBJID_LEN)))
        {

            /* verify chunk checksum */
            chunk = buf + fileChunk->offset;
            if(fileChunk->checksum != UcLo_strChecksum((char*)chunk, fileChunk->size)) 
            {
                break;
            }
            if(!(objHdr = ixOsalMemAlloc(sizeof(uclo_objHdr_T)))) 
            {
                break;
            }
            objHdr->fBuf = chunk;
            objHdr->checksum = fileChunk->checksum;
            objHdr->size = fileChunk->size;
            return (objHdr);
        }
        fileChunk++;
    }
    return (NULL);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_mapImage
   Description: Map the uof image
   Returns:     number of images, otherwise 0 if none or failure
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int UcLo_mapImage(uclo_objHandle_T *objHandle,
                         uof_encapAe_T *uniqueAeImage,
                         int maxImages)
{
    int                        a = 0, p, i;
    uof_chunkHdr_T            *chunkHdr = NULL;
    uof_Image_T                *image;
    uof_codePage_T            *codePage;
    uof_aeRegTab_T            *aeRegTab;
    uof_initRegSymTab_T     *initRegSymTab;
    uof_sbreakTab_T            *sbreakTab;
    uof_encapUofObj_T        *encapUofObj = &objHandle->encapUofObj;

    for(i = 0; i < maxImages; i++)
    {
        uniqueAeImage[i].imagePtr = NULL;
        uniqueAeImage[i].pages = NULL;
        uniqueAeImage[i].aeReg = NULL;
        uniqueAeImage[i].numAeReg = 0;
        uniqueAeImage[i].initRegSym = NULL;
        uniqueAeImage[i].numInitRegSym = 0;
        uniqueAeImage[i].numSbreak = 0;
    }

    for(a = 0; a < maxImages; a++)
    {
        /* get the next IMAGE chunk header */
        if(!(chunkHdr = UcLo_findChunk(encapUofObj->objHdr, UOF_IMAG, (void *)chunkHdr))) 
        {
            break;
        }    

        image = (uof_Image_T *)(encapUofObj->begUof + chunkHdr->offset);

        aeRegTab = (uof_aeRegTab_T *)(image->regTabOffset + objHandle->objHdr->fBuf);
        uniqueAeImage[a].numAeReg = aeRegTab->numEntries;

        uniqueAeImage[a].aeReg = (uof_aeReg_T *)(((char *)aeRegTab) + sizeof(uof_aeRegTab_T));

        initRegSymTab = (uof_initRegSymTab_T *)(image->initRegSymTab + objHandle->objHdr->fBuf);
        uniqueAeImage[a].numInitRegSym = initRegSymTab->numEntries;

        uniqueAeImage[a].initRegSym = (uof_initRegSym_T *)(((char *)initRegSymTab) + sizeof(uof_initRegSymTab_T));
        
        sbreakTab = (uof_sbreakTab_T *)(image->sbreakTab + objHandle->objHdr->fBuf);
        uniqueAeImage[a].numSbreak = sbreakTab->numEntries;
	
        uniqueAeImage[a].sbreak = (uof_sbreak_T *)(((char *)sbreakTab) + sizeof(uof_sbreakTab_T));

        uniqueAeImage[a].imagePtr = image;
        codePage = (uof_codePage_T *)((char *)image + sizeof(uof_Image_T));

        if(!(uniqueAeImage[a].pages = (uof_encapPage_T *)ixOsalMemAlloc(image->numOfPages * sizeof(uof_encapPage_T))))
        {
            return (0);
        }
        ixOsalMemSet(uniqueAeImage[a].pages, 0, image->numOfPages * sizeof(uof_encapPage_T));

        /* assign the pages */
        for(p = 0; p < image->numOfPages; p++)
        {
            uof_ucVarTab_T      *ucVarTab;
            uof_impVarTab_T     *impVarTab;
            uof_impExprTab_T    *impExprTab;
            uof_neighRegTab_T   *neighRegTab;
            uof_codeArea_T      *codeArea;
            uof_uwordBlockTab_T *uwBlockTab;
            uof_uWordBlock_T    *uwBlocks;

            uniqueAeImage[a].pages[p].pageNum = codePage[p].pageNum;
            uniqueAeImage[a].pages[p].defPage = codePage[p].defPage;
            uniqueAeImage[a].pages[p].pageRegion = codePage[p].pageRegion;
            uniqueAeImage[a].pages[p].begVirtAddr = codePage[p].begVirtAddr;
            uniqueAeImage[a].pages[p].begPhyAddr = codePage[p].begPhyAddr;

            /* assign uC variables */
            ucVarTab = (uof_ucVarTab_T *)(encapUofObj->begUof + codePage[p].ucVarTabOffset);
            uniqueAeImage[a].pages[p].numUcVar = ucVarTab->numEntries;
	    
            uniqueAeImage[a].pages[p].ucVar = (uof_ucVar_T *)((char *)ucVarTab + sizeof(uof_ucVarTab_T));

            /* assign import variables */
            impVarTab = (uof_impVarTab_T *)(encapUofObj->begUof + codePage[p].impVarTabOffset);
            uniqueAeImage[a].pages[p].numImpVar = impVarTab->numEntries;
	    
            uniqueAeImage[a].pages[p].impVar = (uof_importVar_T *)((char *)impVarTab + sizeof(uof_impVarTab_T));
            
            /* assign import expression variables */
            impExprTab = (uof_impExprTab_T *)(encapUofObj->begUof + codePage[p].impExprTabOffset);
            uniqueAeImage[a].pages[p].numImpExpr = impExprTab->numEntries;
	    
            uniqueAeImage[a].pages[p].impExpr = (uof_impExpr_T *)((char *)impExprTab + sizeof(uof_impExprTab_T));

            /* assign neighbour */
            neighRegTab = (uof_neighRegTab_T *)(encapUofObj->begUof + codePage[p].neighRegTabOffset);
            uniqueAeImage[a].pages[p].numNeighReg = neighRegTab->numEntries;
	    
            uniqueAeImage[a].pages[p].neighReg = (uof_neighReg_T *)((char *)neighRegTab + sizeof(uof_neighRegTab_T));
            
            /* assign code area */
            codeArea = (uof_codeArea_T *)(encapUofObj->begUof + codePage[p].codeAreaOffset);
            uniqueAeImage[a].pages[p].numMicroWords = codeArea->numMicroWords;
            uwBlockTab = (uof_uwordBlockTab_T *)(encapUofObj->begUof + codeArea->uwordBlockTab);
            uniqueAeImage[a].pages[p].numUwBlocks = uwBlockTab->numEntries;
            uwBlocks = (uof_uWordBlock_T *)((char *)uwBlockTab + sizeof(uof_uwordBlockTab_T));
	    
            uniqueAeImage[a].pages[p].uwBlocks = (uof_encapUwBlock_T *)uwBlocks;
            
            for(i=0; (unsigned int)i < uwBlockTab->numEntries; i++)
            {
                uniqueAeImage[a].pages[p].uwBlocks[i].microWords = (char *)(encapUofObj->begUof + uwBlocks[i].uwordOffset);
            }
        }
    }

   return (a);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_mapAe
   Description: Maps Uengines in UOF to the handle
   Returns:     UCLO_SUCCESS
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_mapAe(uclo_objHandle_T *objHandle, 
           int maxMe)
{
    int imageNum, swAe;
    int status = UCLO_SUCCESS;

    for(swAe = 0; swAe < maxMe; swAe++)
    {
        UcLo_ClearAeData(&objHandle->aeData[swAe]);

        /* traverse the images and check if AE is assigned */
        for(imageNum = 0; imageNum < objHandle->numUniqueImage; imageNum++)
        {
            int hwAe = UcLo_getHwAe(objHandle->prodType, (unsigned char)swAe);
            if(isBitSet(objHandle->uniqueAeImage[imageNum].imagePtr->aeAssigned, hwAe))
            {
                UcLo_InitAeData(objHandle, swAe, imageNum);
            }
        }
        if((status = UcLo_AssignHalPages(&objHandle->aeData[swAe], objHandle->hwAeNum[swAe])))
        {
            return (status);
        }    
    }

    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_mapStrTable
   Description: Maps the UOF string table
   Returns:     pointer to the string table or NULL
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static uof_strTab_T *
UcLo_mapStrTable(uclo_objHdr_T *objHdr, 
                 char *tabName, 
                 uof_strTab_T *strTable)
{
    uof_chunkHdr_T *chunkHdr = NULL;

    /* get the first string table -- should be only one */
    if((chunkHdr = UcLo_findChunk((uof_objHdr_T *)objHdr->fBuf, tabName, NULL)))
    {
        memmove(&strTable->tableLen, (objHdr->fBuf + chunkHdr->offset), sizeof(strTable->tableLen));
        strTable->strings = (char *)(objHdr->fBuf + chunkHdr->offset + sizeof(strTable->tableLen));
    }
    return (strTable);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_mapInitMemTable
   Description: Maps the UOF init memory table
   Returns:     pointer to the initMem table or NULL
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static uof_initMemTab_T *
UcLo_mapInitMemTable(uof_encapUofObj_T *encapUofObj, 
                     uof_initMemTab_T *initMemTab)
{
    uof_chunkHdr_T *chunkHdr = NULL;

    /* get ustore mem inits table -- should be only one */
    if((chunkHdr = UcLo_findChunk(encapUofObj->objHdr, UOF_IMEM, NULL)))
    {
        memmove(&initMemTab->numEntries, (encapUofObj->begUof + chunkHdr->offset), sizeof(unsigned int));
        if(initMemTab->numEntries) 
        {
            initMemTab->initMem = (uof_initMem_T *)(encapUofObj->begUof + chunkHdr->offset + sizeof(unsigned int));
        }    
    }
    else
    {
        initMemTab->numEntries = 0;
        initMemTab->initMem = NULL;
    }
    return (initMemTab);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_mapVarMemSeg
   Description: Maps the uC memory segments
   Returns:     pointer to the uof_uCmemSeg_T or NULL
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static uof_varMemSeg_T *
UcLo_mapVarMemSeg(uof_encapUofObj_T *encapUofObj)
{
    uof_chunkHdr_T *chunkHdr = NULL;
    uof_varMemSeg_T *varMemSeg = NULL;
    unsigned int numEntries;

    /* get first memory segment header -- should only be one in the UOF */
    if((chunkHdr = UcLo_findChunk(encapUofObj->objHdr, UOF_MSEG, NULL)))
    {
        /* numEntries should be one  -- allows us to entend the number of uof_varMemSeg_T without
        affecting the UOF format */
        memmove(&numEntries, (encapUofObj->begUof + chunkHdr->offset), sizeof(unsigned int));
        varMemSeg = (uof_varMemSeg_T *)(encapUofObj->begUof + chunkHdr->offset + sizeof(numEntries));
    }
    return (varMemSeg);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_setValue
   Description: Assigns 'value' to a group of bits, (sub-field), as described
                by the 'field' input parameter.  Unspecified bits will remain
                unaffected, so, this function should work for 32bit-uwords if
                the field doesn't extend beyond bit31. 
   Returns:     the updated word
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static uword_T 
UcLo_setValue(uword_T uword, 
              char *field, 
              unsigned int value)
{
    int i;

    /* validate the sub-fields */
    for(i=0; i < 12;)
    {
        unsigned int val;
        unsigned char msb, lsb, begBitPos, len, rShtValue;

        if((msb = field[i++]) >= 64)
        {
           break;
        }   
        lsb = field[i++];
        begBitPos = lsb;
        len = (msb - lsb + 1);

        rShtValue = field[i++];
        val = value >> rShtValue; 
        uword = UcLo_setField64(uword, begBitPos, len, val);
    }
    return (uword);
}

/*-----------------------------------------------------------------------------
   Function:     UcLo_getObjHandle
   Description:  Creates an UOF object handle
   Returns:      pointer to handle, otherwise NULL if failed
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
uclo_objHandle_T *
UcLo_getObjHandle(void)
{
    uclo_objHandle_T *objHandle;

    if(!(objHandle = (uclo_objHandle_T*)ixOsalMemAlloc(sizeof(uclo_objHandle_T)))) 
    {
        return (NULL);
    }    
    ixOsalMemSet(objHandle, 0, sizeof(uclo_objHandle_T));

    objHandle->readOnly = 1;
    return (objHandle);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_getEncapImage
   Description: Locates an UOF image by name as specified by the 'ucodeImageName'
                parameter.
   Returns:     pointer to the encapsulated image or NULL
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
uof_encapAe_T *
UcLo_getEncapImage(uclo_objHandle_T *objHandle, 
                   char *ucodeImageName)
{
    int i;
    char          *str;    

    if(!ucodeImageName || !objHandle || !objHandle->objHdr->fBuf)
    {
        return (NULL);
    }    

    /* find the image */
    for(i = 0; i < objHandle->numUniqueImage; i++) 
    {
        str = UcLo_getString(&objHandle->strTable, objHandle->uniqueAeImage[i].imagePtr->imageName);   
        if(!str) 
        { 
           continue;
        }   
        if(!UcLo_strcmp(ucodeImageName, str, objHandle->uniqueAeImage[i].imagePtr->sensitivity)) 
        {
            return (&objHandle->uniqueAeImage[i]);
        }    
    }        

    return (NULL);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_GetMeSliceImageName
   Description: Return the image-name(s) and ctxAssignments of the specified AE.
                The return value to the image-name must not be modified.
   Returns:     UCLO_SUCCESS, UCLO_BADARG, UCLO_BADAE, UCLO_NOOBJ,UCLO_IMGNOTFND
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_GetMeSliceImageName(void *handle, 
                         unsigned char hwAe, 
                         unsigned int slice,
                         ucLo_ImageAssign_T *imageAssign)
{
    uclo_objHandle_T *objHandle = (uclo_objHandle_T *)handle;
    unsigned int swAe;

    if(!imageAssign) 
    {
        return (UCLO_BADARG);
    }    
    if(!objHandle || !objHandle->objHdr->fBuf) 
    {
        return (UCLO_NOOBJ);
    }    
    if((swAe = UcLo_getSwAe(objHandle->prodType, hwAe)) == UCLO_BADAE) 
    {
        return (UCLO_BADARG);
    }    

    if(slice > objHandle->aeData[swAe].numSlices) 
    {
        return (UCLO_IMGNOTFND);
    }    
    if((!objHandle->aeData[swAe].aeSlice[slice].encapImage) || (!objHandle->aeData[swAe].aeSlice[slice].encapImage->imagePtr)) 
    {
        return (UCLO_IMGNOTFND);
    }    
    if(objHandle->aeData[swAe].aeSlice[slice].encapImage->imagePtr->imageName == 0xffffffff) 
    {
        return (UCLO_IMGNOTFND);
    }    
    imageAssign->imageName = UcLo_getString(&objHandle->strTable, objHandle->aeData[swAe].aeSlice[slice].encapImage->imagePtr->imageName);
    imageAssign->assignedCtxMask = objHandle->aeData[swAe].aeSlice[slice].assignedCtxMask;
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_GetAeImageName
   Description: Return the image-name by name of the specified AE.  The return
                value to the image-name must not be modified.
   Returns:     pointer to the image-name or NULL.
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
char *
UcLo_GetAeImageName(void *handle, 
                    unsigned char hwAe)
{
    uclo_objHandle_T *objHandle = (uclo_objHandle_T *)handle;
    unsigned int swAe;

    if(!objHandle || !objHandle->objHdr->fBuf) 
    {
        return (NULL);
    }    
    if((swAe = UcLo_getSwAe(objHandle->prodType, hwAe)) == UCLO_BADAE) 
    {
        return (NULL);
    }    

    if((!objHandle->aeData[swAe].aeSlice[0].encapImage) || (!objHandle->aeData[swAe].aeSlice[0].encapImage->imagePtr)) 
    {
        return (NULL);
    }    
    if(objHandle->aeData[swAe].aeSlice[0].encapImage->imagePtr->imageName == 0xffffffff) 
    {
        return (NULL);
    }    
    return (UcLo_getString(&objHandle->strTable, objHandle->aeData[swAe].aeSlice[0].encapImage->imagePtr->imageName));
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_checkCompat
   Description: Check if the UOF objects are compatible with the chip.
   Returns:     UCLO_SUCCESS, or UCLO_UOFINCOMPAT
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_checkCompat(uclo_objHandle_T *objHandle)
{
    unsigned int majVer, prodType=objHandle->prodType;

    /* check is chip is compatable with the UOF */
    if(!(prodType & objHandle->encapUofObj.objHdr->cpuType)) 
    {
        return (UCLO_UOFINCOMPAT);
    }    

    majVer = objHandle->prodRev & 0xff;
    /* check the min and max cpu version required against the chip's */
    if((objHandle->encapUofObj.objHdr->maxCpuVer < majVer) ||
       (objHandle->encapUofObj.objHdr->minCpuVer > majVer)) 
    {
        return (UCLO_UOFINCOMPAT);
    }    

    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_fixupGlobalConst
   Description: Set the __CHIP_ID, __CHIP_REVISION constants to the appropriate
                values in the micro-instructions.
   Returns:     UCLO_SUCCESS, or UCLO_FAILURE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_fixupGlobalConst(uclo_objHandle_T *objHandle, 
                      uof_encapPage_T *page)
{
    unsigned int    iv;
    char            *varName;
    char            *chipRev = "__chip_revision";
    char            *chipId = "__chip_id";
    uof_importVar_T    *impVar;


    for(iv = 0; iv < page->numImpVar; iv++)
    {
        varName = UcLo_getString(&objHandle->strTable, page->impVar[iv].name);
        if(varName == NULL) 
        {
           continue;       
        }   
        if(!strncmp(varName, chipRev, strlen(chipRev)))
        {
            impVar = &page->impVar[iv];
            impVar->value = objHandle->prodRev;
            SET_FIXUP_ASSIGN(impVar->valueAttrs, 1);
        }
        else if(!strncmp(varName, chipId, strlen(chipId)))
        {
            impVar = &page->impVar[iv];
            impVar->value = objHandle->prodType;
            SET_FIXUP_ASSIGN(impVar->valueAttrs, 1);
        }
        else
        {
            continue;
        }    
    }

    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_fixupExpr
   Description: Evaluate import_expression and fixup the micro-instruction.
   Returns:     UCLO_SUCCESS, UCLO_UNINITVAR, or UCLO_FAILURE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_fixupExpr(uclo_objHandle_T *objHandle, 
               uof_encapPage_T *page,
               unsigned char swAe)
{
    uword_T            uword, uwrdBefore;
    unsigned int    iv;
    char            *fields;
    uof_impExpr_T    *impExpr;
    int                expRes, retVal=UCLO_SUCCESS, status = UCLO_SUCCESS;
    char          *str;    

    for(iv = 0; iv < page->numImpExpr; iv++)
    {
        impExpr = &page->impExpr[iv];

        if(impExpr->valType == EXPR_VAL)
        {
            str = UcLo_getString(&objHandle->strTable,impExpr->exprValue);
            if(!str) 
            {
                continue;
            }    
            if((status = UcloExp_evalPostfix(objHandle, swAe, str, &expRes)))
            {
                if(status != UCLO_UNINITVAR) 
                {  
                   return (status);
                }   
                retVal = UCLO_UNINITVAR;
            }
        }
        else
        {
            if(!GET_FIXUP_ASSIGN(impExpr->valueAttrs)) 
            {  
                retVal = UCLO_UNINITVAR;
            }    
        }

        fields = impExpr->fieldAttrs;

        /* bind the global constant */
        if((uword = Uclo_getUofUword(objHandle, page, impExpr->uwordAddress)) == INVLD_UWORD) 
        {
            return (UCLO_FAILURE);
        }    
        uwrdBefore = uword;
        uword = UcLo_setValue(uword, fields, expRes);
        Uclo_setUofUword(objHandle, page, impExpr->uwordAddress, uword);
        SET_FIXUP_ASSIGN(impExpr->valueAttrs, 1);
        
    }

    return (retVal);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_fixupLocalConst
   Description: Set the __UENGINE_ID, and __USTORE_DRAM_BASE constants to the appropriate
                values in the micro-instructions.
   Returns:     UCLO_SUCCESS, UCLO_UNINITVAR, or UCLO_FAILURE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_fixupLocalConst(uclo_objHandle_T *objHandle, 
                     uof_encapPage_T *page,
                     unsigned char swAe)
{
    unsigned int    i;
    char            *varName;
    uof_importVar_T *impVar;
    int             status = UCLO_SUCCESS;

    if(swAe >= UOF_MAX_NUM_OF_AE)
    {
        return (UCLO_BADARG);
    }

    for(i = 0; i < page->numImpVar; i++)
    {
        impVar = &page->impVar[i];

        varName = UcLo_getString(&objHandle->strTable, impVar->name);
        if(varName == NULL) 
        {
           continue;
        }   
        
        if(!strncmp(varName, UENGINE_ID, strlen(UENGINE_ID)))
        {
            impVar->value = UcLo_getHwAe(objHandle->prodType, swAe);
            SET_FIXUP_ASSIGN(impVar->valueAttrs, 1);
            
        }
        else
        {
            if(!strncmp(varName, USTORE_DRAM_BASE_SYM, strlen(USTORE_DRAM_BASE_SYM)))
            {
                if(objHandle->aeData[swAe].relocUstoreDram != -1)
                {   
                    impVar->value = (unsigned int)objHandle->aeData[swAe].relocUstoreDram;
                    SET_FIXUP_ASSIGN(impVar->valueAttrs, 1);
               }
            }
        }
    }

    return (status);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_fixupLocalUcVar
   Description: Fixup the local variables for the specified uengine in the UOF
   Returns:     UCLO_SUCCESS, or UCLO_FAILURE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_fixupLocalUcVar(uclo_objHandle_T *objHandle, 
                     uof_encapPage_T *page,
                     char swAe)
{
    uword_T uword, uwrdBefore;
    unsigned int ucv, uaddr;
    char *fields;
    char prefix[12];
    char          *str;    

    LOCAL_NAME(prefix, "", UcLo_getHwAe(objHandle->prodType, swAe));    /* format the local variable prefix */
    for(ucv = 0; ucv < page->numUcVar; ucv++)
    {
        str = UcLo_getString(&objHandle->strTable, page->ucVar[ucv].name);
        if(!str) 
        {
           continue;
        }   
        if((GET_FIXUP_SCOPE(page->ucVar[ucv].valueAttrs) == LOCAL_SCOPE) &&
                !strncmp(str, prefix, strlen(prefix)))
        {
            fields = page->ucVar[ucv].fieldAttrs;
            uaddr = page->ucVar[ucv].uwordAddress;

            /* bind the local variable */
            if((uword = Uclo_getUofUword(objHandle, page, uaddr)) == INVLD_UWORD) 
            {
                return (UCLO_FAILURE);
            }    
            uwrdBefore = uword;
            uword = UcLo_setValue(uword, fields, page->ucVar[ucv].exprValue);
            Uclo_setUofUword(objHandle, page, uaddr, uword);
            
        }
    }

   return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_fixupGlobalUcVar
   Description: Fixup the global variables in the UOF
   Returns:     UCLO_SUCCESS, or UCLO_FAILURE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_fixupGlobalUcVar(uclo_objHandle_T *objHandle, 
                      uof_encapPage_T *page)
{
    uword_T uword, uwrdBefore;
    unsigned int uaddr, ucv;
    char *fields;

    for(ucv = 0; ucv < page->numUcVar; ucv++)
    {

        if(GET_FIXUP_SCOPE(page->ucVar[ucv].valueAttrs) == GLOBAL_SCOPE)
        {
            fields = page->ucVar[ucv].fieldAttrs;
            uaddr = page->ucVar[ucv].uwordAddress;

            /* bind the global variable */
            if((uword = Uclo_getUofUword(objHandle, page, uaddr)) == INVLD_UWORD) 
            {
                return (UCLO_FAILURE);
            }    
            uwrdBefore = uword;
            uword = UcLo_setValue(uword, fields, page->ucVar[ucv].exprValue);
            Uclo_setUofUword(objHandle, page, uaddr, uword);
            
        }
    }

   return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_getImportVar
   Description: Locate an import variable by name
   Returns:     pointer to the import-variable, or NULL
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
uof_importVar_T *
UcLo_getImportVar(uclo_objHandle_T *objHandle,
                  unsigned char swAe,
                  char *varName)
{
    unsigned int i, p, s;
    char          *str;    

    for(s=0; s < objHandle->aeData[swAe].numSlices; s++)
    {
        if(!objHandle->aeData[swAe].aeSlice[s].encapImage) 
        {
            continue;
        }    
        for(p = 0; p < objHandle->aeData[swAe].aeSlice[s].encapImage->imagePtr->numOfPages; p++)
        {
            for(i = 0; i < objHandle->aeData[swAe].aeSlice[s].encapImage->pages[p].numImpVar; i++)
            {
                str = UcLo_getString(&objHandle->strTable, objHandle->aeData[swAe].aeSlice[s].encapImage->pages[p].impVar[i].name);
                if(!str) 
                {
                    continue;
                }    
                if(!UcLo_strcmp(str, varName, objHandle->aeData[swAe].aeSlice[s].encapImage->imagePtr->sensitivity)) 
                {
                    return (&objHandle->aeData[swAe].aeSlice[s].encapImage->pages[p].impVar[i]);
                }    
            }
        }
    }
    return (NULL);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_getAeRegName
   Description: Locate a AE register by name and type
   Returns:     pointer to the register, or NULL
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
uof_aeReg_T *
UcLo_getAeRegName(uclo_objHandle_T *objHandle, 
                  uof_encapAe_T *encapAeImage,
                  char *regName, 
                  unsigned int type)
{
    unsigned int    i, regIndx;
    char            *strPtr, *target;
    uof_aeReg_T        *reg=NULL;
    char          *str;
    
    if((strPtr = strstr(regName, "[")))
    {
        regIndx = atoi(strPtr+1);
        *strPtr = '\0';
        target = strPtr;
    }
    else
    {
        target = regName;
    }    

    for(i = 0; i < encapAeImage->numAeReg; i++)
    {
        str = UcLo_getString(&objHandle->strTable, encapAeImage->aeReg[i].visName);
        if(!str) 
        {
            continue;
        }    
        if(!UcLo_strcmp(str, regName, encapAeImage->imagePtr->sensitivity) &&
                ((type == ICP_ANY_REG) || (encapAeImage->aeReg[i].type == type)))
        {
            reg = &encapAeImage->aeReg[i];
            break;
        }
    }

    if(strPtr && reg)
    {
        for(i = 0; i < encapAeImage->numAeReg; i++)
        {
            if((encapAeImage->aeReg[i].xoId == reg->xoId) &&
                    (encapAeImage->aeReg[i].refCount == reg->refCount) &&
                    (encapAeImage->aeReg[i].type == reg->type))
            {
                reg = &encapAeImage->aeReg[i];
                break;
            }
        }
        *strPtr = '[';
    }

    return (reg);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_findMeReg
   Description: Locate a AE register by string-table-name-offset and type
   Returns:     pointer to the register, or NULL
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static uof_aeReg_T *
UcLo_findMeReg(uof_encapAe_T *encapAeImage, 
               unsigned int regNameOffset,
               unsigned int type)
{
    unsigned int i;
    if(!encapAeImage)
    {
        return (NULL);
    }    
    for(i = 0; i < encapAeImage->numAeReg; i++)
    {
        if((regNameOffset == encapAeImage->aeReg[i].visName) &&
            ((type == ICP_ANY_REG) || (encapAeImage->aeReg[i].type == type))) 
        {
            return (&encapAeImage->aeReg[i]);
        }    
    }
    return (NULL);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_getSharedUstoreNeigh
   Description: 
   Returns:
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_getSharedUstoreNeigh(uclo_objHandle_T *objHandle, 
                          unsigned int hwAeNum)
{
    if(hwAeNum & 0x1)
    {
        if(UCLO_AE_NEIGHBOR(objHandle->swAe[hwAeNum-1], objHandle->numAEs) == UCLO_BADAE) 
        {
            return (UCLO_BADAE);
        }    
        return (hwAeNum-1);
    }
    return (UCLO_AE_NEIGHBOR(objHandle->swAe[hwAeNum], objHandle->numAEs));
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_fixupLocalNeighReg
   Description: Fixup the neighbour-reg definitions for the specified uengine
                in the UOF
   Returns:     UCLO_SUCCESS, UCLO_NEIGHNOTFND, or UCLO_FAILURE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_fixupLocalNeighReg(uclo_objHandle_T *objHandle, 
                        uof_encapPage_T *page,
                        unsigned char swAe)
{
    uword_T         uword, uwrdBefore;
    unsigned int    i, uaddr, meNeighbor, s;
    char            *fields;
    uof_aeReg_T     *aeReg = 0;

    if(page->numNeighReg == 0) 
    {
       return (UCLO_SUCCESS);
    }   
    meNeighbor = UCLO_AE_NEIGHBOR(swAe, objHandle->numAEs);
    if(meNeighbor == UCLO_BADAE) 
    {
       return (UCLO_NEIGHNOTFND);
    }   

    for(i = 0; i < page->numNeighReg; i++)
    {
        fields = page->neighReg[i].fieldAttrs;
        uaddr = page->neighReg[i].uwordAddress;

        /* get the address of corresponding neigh-reg of the neighboring AE */
        for(s=0; s < objHandle->aeData[meNeighbor].numSlices; s++)
        {
            if(!(aeReg = UcLo_findMeReg(objHandle->aeData[meNeighbor].aeSlice[s].encapImage,
                                        page->neighReg[i].name, ICP_NEIGH_REL)))
            {
                 break;
            }     
        }
        if(!aeReg)
        {
            return (UCLO_FAILURE);
        }

        /* bind the local variable */
        if((uword = Uclo_getUofUword(objHandle, page, uaddr)) == INVLD_UWORD) 
        {
            return (UCLO_FAILURE);
        }    
        uwrdBefore = uword;
        uword = UcLo_setValue(uword, fields, aeReg->addr);
        Uclo_setUofUword(objHandle, page, uaddr, uword);
        SET_FIXUP_ASSIGN(page->neighReg[i].valueAttrs, 1);
        
    }
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_fixupLocals
   Description: Fixup local variables and constants in the UOF for the specified
                accelEngine.
   Returns:     UCLO_SUCCESS, UCLO_UNINITVAR, or UCLO_FAILURE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_fixupLocals(uclo_objHandle_T *objHandle, 
                 uof_encapPage_T *page,
                 unsigned char swAe)
{
    int status = UCLO_SUCCESS, retVal = UCLO_SUCCESS;

    if(UcLo_fixupLocalUcVar(objHandle, page, swAe)) 
    {
        return (UCLO_FAILURE);
    } 
    if(swAe < (objHandle->numAEs-1)) 
    { /* last AE has no neighbor */
        if((status = UcLo_fixupLocalNeighReg(objHandle, page, swAe)) != UCLO_SUCCESS)
        {
            if(status != UCLO_UNINITVAR) 
            {
                return (UCLO_FAILURE);
            }    
            else 
            {
                retVal = status;
            }    
        }
    }
    if((status = UcLo_fixupLocalConst(objHandle, page, swAe)) != UCLO_SUCCESS)
    {
        if(status != UCLO_UNINITVAR) 
        {
            return (status);
        }    
    }

    if((status = UcLo_fixupExpr(objHandle, page, swAe)) != UCLO_SUCCESS)
    {
        if(status != UCLO_UNINITVAR) 
        {
            return (status);
        }    
        else
        {
            retVal = status;
        }    
    }
    return (retVal);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_fixupGlobals
   Description: Fixup global variables and constants in the UOF for the specified
                accelEngine.
   Returns:     UCLO_SUCCESS, or UCLO_FAILURE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_fixupGlobals(uclo_objHandle_T *objHandle, 
                  uof_encapPage_T *page)
{
    if(page->numUcVar) 
    {
        if(UcLo_fixupGlobalUcVar(objHandle, page))
        {
           return (UCLO_FAILURE);
        }   
    }    
    if(UcLo_fixupGlobalConst(objHandle, page)) 
    {
        return (UCLO_FAILURE);
    }    
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_initGlobals
   Description: Initialize global variables and constants in memory and the UOF.
   Returns:     UCLO_IVDWARN, UCLO_SUCCESS, UCLO_BADARG, UCLO_FAILURE,
                UCLO_EXCDLMEMSIZE, UCLO_EXCDUMEMSIZE, UCLO_EXCDSRAM0SIZE, 
                UCLO_EXCDDRAMSIZE, UCLO_EXCDSCRTHSIZE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_initGlobals(uclo_objHandle_T *objHandle)
{
    int retVal=UCLO_SUCCESS;

    if(!objHandle->globalInited)
    {
        int i, stat;
        unsigned int j, s;

        /* initialize the memory segments */
        if(objHandle->initMemTab.numEntries) 
        {
            if((stat = UcLo_initMemory(objHandle))) 
            {
                return (stat);
            }    
        }    

        /* bind import variables with ivd values */
        if((stat = UcLo_bindIvd(objHandle)))
        {
            if((stat == UCLO_IMGNOTFND) || (stat == UCLO_SYMNOTFND))    /* ignore these failures and allow load to continue */
            {
                retVal = UCLO_IVDWARN;
            }    
            else
            {
                return (stat);
            }    
        }

        /* bind the uC global variables -- local variables will done on-the-fly */
        for(i = 0; i < objHandle->numUniqueImage; i++)
        {
            int p;

            for(p = 0; p < objHandle->uniqueAeImage[i].imagePtr->numOfPages; p++)
            {
                if(objHandle->uniqueAeImage[i].pages[p].numUwBlocks)
                {
                    if((stat = UcLo_fixupGlobals(objHandle, &objHandle->uniqueAeImage[i].pages[p])) !=
                        UCLO_SUCCESS) 
                    {
                        return (stat);
                    }    
                }
            }
        }

        /* init reg and sym */
        for(j=0; (j < objHandle->numAEs) && (j < UOF_MAX_NUM_OF_AE); j++)
        {
            for(s=0; s < objHandle->aeData[j].numSlices; s++)
            {
                if(objHandle->aeData[j].aeSlice[s].encapImage)
                {
                    if((stat = UcLo_initRegSymExpr(objHandle, (unsigned char)UcLo_getHwAe(objHandle->prodType, (unsigned char)j),
                            objHandle->aeData[j].aeSlice[s].encapImage))) 
                    {
                        return (stat);
                    }    
                }
            }
        }

        objHandle->globalInited=1;
    }
    return (retVal);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_initReg
   Description: Initialize AE register.
   Returns:     UCLO_SUCCESS, or UCLO_FAILURE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_initReg(unsigned char hwAe, 
             unsigned char ctxMask, 
             icp_RegType_T regType,
             unsigned short regAddr, 
             unsigned int value)
{
    unsigned char ctx;

    switch(regType){
    case ICP_GPA_REL:
    case ICP_GPB_REL:
    case ICP_SR_REL:
    case ICP_SR_RD_REL:
    case ICP_SR_WR_REL:
    case ICP_DR_REL:
    case ICP_DR_RD_REL:
    case ICP_DR_WR_REL:
    case ICP_NEIGH_REL:
        /* init for all valid ctx */
        for(ctx=0; ctx < MAX_CTX; ctx++)
        {
            if(!IS_BIT_SET(ctxMask, ctx)) 
            {
                continue;
            }    
            if(halAe_PutRelDataReg(hwAe, ctx, regType, regAddr, value)) 
            {
               return (UCLO_FAILURE);
            }   
        }
        break;
    case ICP_GPA_ABS:
    case ICP_GPB_ABS:
    case ICP_SR_ABS:
    case ICP_SR_RD_ABS:
    case ICP_SR_WR_ABS:
    case ICP_DR_ABS:
    case ICP_DR_RD_ABS:
    case ICP_DR_WR_ABS:
        if(halAe_PutAbsDataReg(hwAe, regType, regAddr, value)) 
        {
           return (UCLO_FAILURE);
        }   
        break;
    default: return (UCLO_FAILURE);
    }
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_initSym
   Description: Init symbol in memory.
   Returns:     UCLO_SUCCESS, or UCLO_BADOBJ
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_initSym(uclo_objHandle_T *objHandle, 
             unsigned char hwAe, 
             char *symName,
             unsigned int offset, 
             unsigned int value)
{
    uof_initMem_T *initMem[2];
    const unsigned int numWords=1;
    char localName[MAX_VARNAME];
    int i;
    unsigned int ScratchOffset, SramOffset, NCDramOffset, CDramOffset;

    halAe_GetMemoryStartOffset(&ScratchOffset, &SramOffset, &NCDramOffset, &CDramOffset);

    initMem[0] = UcLo_findMemSym(objHandle, symName);
    LOCAL_NAME(localName, symName, hwAe);    /* format the local variable name */
    initMem[1] = UcLo_findMemSym(objHandle, localName);

    for(i=0; i < 2; i++)
    {
        if(initMem[i])
        {
            switch(initMem[i]->region)
            {
            case SRAM0_REGION:
                UcLo_wrSram(SramOffset + initMem[i]->addr+offset, &value, numWords);
                break;
            case DRAM_REGION:
                UcLo_wrDram(CHAN0, NCDramOffset + initMem[i]->addr+offset, &value, numWords);
                break;
            case DRAM1_REGION:
                UcLo_wrDram(CHAN1, CDramOffset + initMem[i]->addr+offset, &value, numWords);
                break;
            case SCRATCH_REGION:
                UcLo_wrScratch(ScratchOffset + initMem[i]->addr+offset, &value, numWords);
                break;
            case LMEM_REGION:
                UcLo_wrLocalMem((unsigned char)hwAe, initMem[i]->addr+offset, &value, numWords);
                break;
            case SHRAM_REGION:
                /* not support init for shram now, add initialization code here if want to support it in future */
                break;
            default: return (UCLO_BADOBJ);
            }
        }
    }
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_initRegSymExpr
   Description: Fixup register, or sym with value from an expression.
   Returns:     UCLO_SUCCESS, UCLO_INVLDCTX, or UCLO_FAILURE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_initRegSymExpr(uclo_objHandle_T *objHandle, 
                    unsigned char hwAe,
                    uof_encapAe_T *encapAe)
{
    unsigned int        i, expRes, swAe;
    unsigned char        ctxMask;
    uof_initRegSym_T    *initRegSym;
    char          *str;
    
    if((swAe = UcLo_getSwAe(objHandle->prodType, hwAe)) == UCLO_BADAE) 
    {
        return (UCLO_FAILURE);
    }
    for(i=0; i < encapAe->numInitRegSym; i++)
    {
        initRegSym = &encapAe->initRegSym[i];
        if(initRegSym->valueType == EXPR_VAL)
        {
            str = UcLo_getString(&objHandle->strTable, initRegSym->value);
            if(!str) 
            {
                continue;
            }    
            if(UcloExp_evalPostfix(objHandle, (unsigned char)swAe, str, (int *)(&expRes)))
            {                    
                return (UCLO_FAILURE);
            }
        }
        else
        {
            expRes = initRegSym->value;
        }

        switch(initRegSym->initType)
        {
        case INIT_EXPR: /* init sym */                
            str = UcLo_getString(&objHandle->strTable, initRegSym->symName); 
            if(str)
            {
               UcLo_initSym(objHandle, hwAe, str, initRegSym->regAddrOrOffset, expRes);
            }   
            break;
        case INIT_REG: /* init register */
            if(CTX_MODE(encapAe->imagePtr->aeMode) == MAX_CTX) 
            {
                ctxMask = 0xff; /* eight ctx-mode */
            }    
            else
            {
                ctxMask = 0x55; /* four ctx-mode */
            }    

            UcLo_initReg(hwAe, ctxMask,
                (icp_RegType_T)initRegSym->regType,
                (unsigned short)initRegSym->regAddrOrOffset, expRes);
            break;
        case INIT_REG_CTX: /* init ctx relative register */
            if(CTX_MODE(encapAe->imagePtr->aeMode) == MAX_CTX) 
            {
                ctxMask = 0xff; /* eight ctx-mode */
            }    
            else
            {
                ctxMask = 0x55; /* four ctx-mode */
            }    

            /* check if ctx is appropriate for the ctxMode */
            if(!((1 << initRegSym->ctx) & ctxMask)) 
            {
                return (UCLO_INVLDCTX);
            }    

            UcLo_initReg(hwAe, (unsigned char)(1 << initRegSym->ctx),
                (icp_RegType_T)initRegSym->regType,
                (unsigned short)initRegSym->regAddrOrOffset, expRes);
            break;
        case INIT_EXPR_ENDIAN_SWAP: /* init sym with endian_swap*/
            expRes = ENDIAN_SWAP32(expRes);
            str = UcLo_getString(&objHandle->strTable, initRegSym->symName);
            if(str) 
            {
                UcLo_initSym(objHandle, hwAe, str, initRegSym->regAddrOrOffset, expRes);
            }    
            break;
        default: break;
        }
    }
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_getAeMode
   Description: Get AE number of contexts mode.
   Returns:     aeMode
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
unsigned int 
UcLo_getAeMode(uclo_objHandle_T *objHandle, 
               int ae)
{
    uof_Image_T *image = objHandle->aeData[ae].aeSlice[0].encapImage->imagePtr;
    return image->aeMode;
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_setAeMode
   Description: Set AE number of contexts mode.
   Returns:     UCLO_SUCCESS, UCLO_FAILURE, or UCLO_BADOBJ
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int  
UcLo_setAeMode(uclo_objHandle_T *objHandle)
{
    unsigned char i, hwAeNum, nnMode, s;
    uof_Image_T      *uofImage;
    UcLo_AeData_t    *aeData;

    for(i = 0; (i < objHandle->numAEs) && (i < UOF_MAX_NUM_OF_AE); i++)
    {
        aeData = &objHandle->aeData[i];
        hwAeNum = objHandle->hwAeNum[i];
        for(s=0; (s < aeData->numSlices) && (s < MAX_CONTEXTS); s++)
        {
            if(objHandle->aeData[i].aeSlice[s].encapImage)
            {
                uofImage = aeData->aeSlice[s].encapImage->imagePtr;
                if(halAe_PutAeCtxMode(hwAeNum, (char)CTX_MODE(uofImage->aeMode))) 
                {
                    return (UCLO_FAILURE);
                }
                /* ignore if UOF_NN_MODE_DONTCARE -- this module don't care about the NNmode; the mode will be set
                by another module or the default value will be used. */
                if((nnMode = NN_MODE(uofImage->aeMode)) != UOF_NN_MODE_DONTCARE) 
                {
                    if(halAe_PutAeNnMode(hwAeNum, nnMode)) 
                    {
                        return (UCLO_FAILURE);
                    }
                }        

                if(halAe_PutAeLmMode(hwAeNum, ICP_LMEM0, (char)LOC_MEM0_MODE(uofImage->aeMode))) 
                {
                    return (UCLO_FAILURE);
                }
                if(halAe_PutAeLmMode(hwAeNum, ICP_LMEM1, (char)LOC_MEM1_MODE(uofImage->aeMode))) 
                {
                    return (UCLO_FAILURE);
                }
                /* set the share control store mode on/off */
                if(halAe_PutAeSharedCsMode(hwAeNum, (unsigned char)SHARED_USTORE_MODE(uofImage->aeMode))) 
                {
                    return (UCLO_FAILURE);
                }
                /* set relodable mode and assign ustore-dram if neccessary */
                if(halAe_SetReloadUstore(hwAeNum, uofImage->reloadableSize, RELOADABLE_CTX_SHARED_MODE(uofImage->aeMode),
                                            aeData->relocUstoreDram)) 
                {
                   return (UCLO_FAILURE);
                }   
 
            }
        }
    }
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_initUstore
   Description: Fill ustore with specified pattern
   Returns:     UCLO_SUCCESS, UCLO_FAILURE
-----------------------------------------------------------------------------*/
static int 
UcLo_initUstore(uclo_objHandle_T *objHandle)
{

    /* For simplicity, we initialize all of ustore. Later, if we want to
       get fancy, we could limit our initialization to not include words
       in pages being loaded by default */
    int a, p;
    unsigned int i;
    uof_encapAe_T *image;
    uof_encapPage_T *page;
    uof_Image_T *uofImage;
    unsigned int doNotInit;
    unsigned int start, end;
    unsigned char swAe, hwAeNum;
    unsigned int numUwordsUsed, tmp, ustoreSize;

    uword_T *fill_data = NULL;
    fill_data = (uword_T *)ixOsalMemAlloc(ICP_MAX_USTORE*8);
    if(fill_data == NULL) 
    {
        return (UCLO_FAILURE);
    }        
    
    for(a = 0; a < objHandle->numUniqueImage; a++)
    {
        image = &objHandle->uniqueAeImage[a];
        uofImage = image->imagePtr;

        for (i=0; i<ICP_MAX_USTORE; i++) 
        {
            memmove(&fill_data[i], &uofImage->fillPattern, sizeof(uword_T));
        }

        /* Compute doNotInit value as a value that will not be equal to
           fill_data when cast to an int */
        doNotInit = 0;
        if (doNotInit == (unsigned int) fill_data[0]) 
        { 
            doNotInit = 0xFFFFFFFF;
        } 
        numUwordsUsed = 0;

        for (p=0; p<uofImage->numOfPages; p++) 
        {
            page = &image->pages[p];

            /* update numUwordsUsed */
            tmp = page->begPhyAddr+page->numMicroWords;
            if (tmp > numUwordsUsed) 
            {
                numUwordsUsed = tmp;
            }
            /* If this is not a default page, do not mark it as doNotInit */
            if (!page->defPage) 
            {
                continue;
            }    

            for (i=page->begPhyAddr; i<page->begPhyAddr+page->numMicroWords; i++)
            {
                fill_data[i] = (uword_T)doNotInit;
            } /* end for i */
        } /* end for p */

        image->numUwordsUsed = numUwordsUsed;

        for(swAe = 0; (swAe < objHandle->numAEs) && (swAe < UOF_MAX_NUM_OF_AE); swAe++)
        {
            hwAeNum = (unsigned char)UcLo_getHwAe(objHandle->prodType, swAe);
            if(!IS_BIT_SET(uofImage->aeAssigned, hwAeNum)) 
            {
                continue;
            }  
            /* if odd numbered AE, then assume that the even numbered AE already fill the ustore */
            if(objHandle->aeData[swAe].shareableUstore && (hwAeNum & 1))
            {
                unsigned int hwNeighNum;
                hwNeighNum = UcLo_getSharedUstoreNeigh(objHandle, hwAeNum);
                if (hwNeighNum == UCLO_BADAE)
                {
		    ixOsalMemFree(fill_data);
                    return (UCLO_NEIGHNOTFND);
                }
                if(IS_BIT_SET(uofImage->aeAssigned, hwNeighNum)) 
                {
                    continue;
                }    
            }

            ustoreSize = objHandle->aeData[swAe].effectUstoreSize;

            /* initialize the areas not going to be overwritten */
            end = (unsigned int)-1;
            do 
            {
                /* find next uword that needs to be initialized */
                for (start = end+1; start < ustoreSize; start++) 
                {
                    if (((unsigned int)fill_data[start]) != doNotInit)
                    {
                        break;
                    }    
                }
                /* see if there are no more such uwords */
                if (start >= ustoreSize) 
                {
                    break;
                }    
                for (end = start+1; end < ustoreSize; end++) 
                {
                    if (((unsigned int)fill_data[end]) == doNotInit) 
                    {
                        break;
                    }    
                }
                /* we need to fill start - (end-1) */
                if(objHandle->aeData[swAe].shareableUstore)
                {
                    if(halAe_PutCoalesceUwords((unsigned char)hwAeNum, start, end-start, &fill_data[start]) != HALAE_SUCCESS) 
                    {
                        ixOsalMemFree(fill_data);
                        return (UCLO_FAILURE);
                    }
                }
                else
                { 
                    if(halAe_PutUwords(hwAeNum, start, end-start, &fill_data[start]) != HALAE_SUCCESS)  
                    {
                        ixOsalMemFree(fill_data);
                        return (UCLO_FAILURE);
                    }
                }
            } while (end < ustoreSize);
        } /* end for swAe */

    } /* end for a */

    ixOsalMemFree(fill_data);

    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_initUstoreDramBase
   Description: 
   Returns:     UCLO_SUCCESS
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_initUstoreDramBase(uclo_objHandle_T *objHandle)
{
    uof_initMem_T *initMem;
    char          symName[MAX_VARNAME];
    unsigned int  swAe;

    for (swAe=0; swAe<objHandle->numAEs; swAe++) 
    {
        LOCAL_NAME(symName, USTORE_DRAM_BASE_SYM, objHandle->hwAeNum[swAe]);
        if((initMem = UcLo_findMemSym(objHandle, symName))) 
        { 
            objHandle->aeData[swAe].relocUstoreDram = initMem->addr;
        }    
        else 
        {
            objHandle->aeData[swAe].relocUstoreDram = -1;
        }    
    }
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_overlayObj
   Description: Overlays the UOF objects
   Returns:     UCLO_SUCCESS, UCLO_UOFINCOMPAT, UCLO_INVLDCTX, UCLO_BADOBJ, UCLO_FAILURE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_overlayObj(uclo_objHandle_T *objHandle, 
                int readOnly)
{
    int stat;
    unsigned int swAe, hwAeNum;

    /* set the UOF object offset */
    objHandle->encapUofObj.begUof = objHandle->objHdr->fBuf;
    objHandle->encapUofObj.objHdr = (uof_objHdr_T *)objHandle->objHdr->fBuf;
    objHandle->encapUofObj.chunkHdr = (uof_chunkHdr_T *)(objHandle->objHdr->fBuf + sizeof(uof_objHdr_T));
    objHandle->readOnly = readOnly;


    objHandle->uWordBytes = AEV2_PACKED_UWORD_BYTES;
    objHandle->numCtxPerAe = MAX_AE2_CTX;

    /* get the cpu type and rev */
    halAe_GetProdInfo(&objHandle->prodType, &objHandle->prodRev);
    switch(objHandle->prodType)
    {
    case (EP80579_CPU_TYPE):
        objHandle->numAEs = 4;
        objHandle->ustorePhySize = USTORE_8K;
        LmemByteSize = (MAX_ICP_LMEM_REG << 2);
        BadHwAeMask = BAD_EP80579_HWAE_MASK;  
        break;
    default:
        return (UCLO_BADOBJ);
    }

    /* Set up swAe <-> hwAeNum mapping table */
    for (swAe=0; swAe < objHandle->numAEs; swAe++) 
    {
        hwAeNum = UcLo_getHwAe(objHandle->prodType, (unsigned char)swAe);
        objHandle->hwAeNum[swAe] = (unsigned char)hwAeNum;
        objHandle->swAe[hwAeNum] = (unsigned char)swAe;
    }


    /* map the dbg string-table */
    UcLo_mapStrTable(objHandle->objHdr, UOF_STRT, &objHandle->strTable);
    if(objHandle->dbgObjHdr) 
    {
        UcLo_mapStrTable(objHandle->dbgObjHdr, DBG_STRT, &objHandle->dbgObjStrTable);
    }    

    if (UcLo_isProfileSupported())
    {
        /* map the prf string-table */
        if(objHandle->prfObjHdr) 
        {
            UcLo_mapStrTable(objHandle->prfObjHdr, PRF_STRT, &objHandle->prfObjStrTable);
        }    
    }

    /* map the images */
    if((objHandle->numUniqueImage = UcLo_mapImage(objHandle,
            objHandle->uniqueAeImage, objHandle->numAEs * MAX_CONTEXTS)))
    {
        if(UcLo_checkCompat(objHandle)) 
        {
           return (UCLO_UOFINCOMPAT);
        }   
        if(UcLo_mapAe(objHandle, objHandle->numAEs)) 
        {
           return (UCLO_BADOBJ);
        }   
    }

    /* Get memory sizes */
    halAe_GetSysMemInfo(&SysMemInfo);
    
    /* This must be called before initMemory, otherwise it will clobber any
       data being initialized into ustore */
    if ((stat = UcLo_initUstore(objHandle))) 
    {
        return (stat);
    }    

    /* map memory initialization table and memory segments table */
    UcLo_mapInitMemTable(&objHandle->encapUofObj, &objHandle->initMemTab);
    objHandle->encapUofObj.varMemSeg = UcLo_mapVarMemSeg(&objHandle->encapUofObj);

    /* init the reloadable-ustore dram base */
    UcLo_initUstoreDramBase(objHandle);

    /* set the AE number of ctx mode -- ctxMode */
    if(UcLo_setAeMode(objHandle)) 
    {
       return (UCLO_FAILURE);
    }   

    objHandle->pausingAeMask = 0;
    ixOsalMutexInit(&objHandle->overlayMutex);
    
    halAe_DefineUcloCallback(UcLo_ProcessHalCallbacks, (void*) objHandle);

    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_WriteNumUword
   Description: Writes a word to the specified uEngine
   Returns:     UCLO_SUCCESS, UCLO_BADARG, UCLO_FAILURE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_WriteNumUword(void *handle, 
                   unsigned int aeMask, 
                   unsigned int phyUaddr,
                   unsigned int numWords, 
                   uword_T *uWord)
{
    uclo_objHandle_T *objHandle = (uclo_objHandle_T *)handle;
    unsigned char swAe, hwAeNum;
    unsigned int copyCnt=numWords, i=0;
    if((objHandle == NULL) || (uWord == NULL)
      || (phyUaddr >= objHandle->ustorePhySize) || ((phyUaddr + numWords) > objHandle->ustorePhySize)) 
    {
        return (UCLO_BADARG);
    }    

    while(numWords)
    {
        if(numWords < UWORD_CPYBUF_SIZE) 
        {
           copyCnt = numWords;
        }   
        else
        {
           copyCnt = UWORD_CPYBUF_SIZE;
        }   

        for(swAe = 0; swAe < objHandle->numAEs; swAe++)
        {
            hwAeNum = (unsigned char)UcLo_getHwAe(objHandle->prodType, swAe);
            if(IS_BIT_SET(aeMask, hwAeNum))
            {
                if(halAe_PutUwords((unsigned char)hwAeNum, phyUaddr+i, copyCnt, &uWord[i]) != HALAE_SUCCESS) 
                {
                    return (UCLO_FAILURE);
                }    
            }
        }
        i += copyCnt;
        numWords -= copyCnt;
    }
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_WriteUword
   Description: Writes a word to the specified uEngine
   Returns:     UCLO_SUCCESS, UCLO_BADARG, UCLO_FAILURE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_WriteUword(void *handle, 
                unsigned int aeMask, 
                unsigned int phyUaddr, 
                uword_T uWord)
{
    uclo_objHandle_T *objHandle = (uclo_objHandle_T *)handle;
    unsigned char swAe, hwAeNum;
    if((objHandle == NULL) || (phyUaddr >= objHandle->ustorePhySize)) 
    {
        return (UCLO_BADARG);
    }    

    for(swAe = 0; swAe < objHandle->numAEs; swAe++)
    {
        hwAeNum = (unsigned char)UcLo_getHwAe(objHandle->prodType, swAe);
        if(IS_BIT_SET(aeMask, hwAeNum))
        {
            if(halAe_PutUwords((unsigned char)hwAeNum, phyUaddr, 1, &uWord) != HALAE_SUCCESS) 
            {
                return (UCLO_FAILURE);
            }    
        }
    }
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_ReadUword
   Description: Reads a word from the specified uEngine
   Returns:     UCLO_SUCCESS, UCLO_BADARG, UCLO_FAILURE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_ReadUword(void *handle, 
               unsigned char hwAe, 
               unsigned int phyUaddr, 
               uword_T *uWord)
{
    uclo_objHandle_T *objHandle = (uclo_objHandle_T *)handle;

    if((uWord == NULL) || (objHandle == NULL)) 
    {
        return (UCLO_BADARG);
    }    
    if(((unsigned int)hwAe >= UCLO_MAX_AES) || ((1 << (unsigned int)hwAe) & BadHwAeMask)) 
    {
        return (UCLO_BADAE);
    }    
    if(phyUaddr >= objHandle->ustorePhySize) 
    {
       return (UCLO_BADARG);
    }

    *uWord = 0;
    if(halAe_GetUwords(hwAe, phyUaddr, 1, uWord) != HALAE_SUCCESS) 
    {
       return (UCLO_FAILURE);
    }   
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    isPageLoaded
   Description: Checks to see if the specified page is currently loaded
   Returns:     0 if not loaded,
-----------------------------------------------------------------------------*/
static int 
isPageLoaded(uclo_objHandle_T *objHandle, 
             unsigned int sliceNum,
             int pageNum, 
             unsigned char swAe)
{
    UcLo_AeData_t *aeData;
    UcLo_Page_t   *page;
    UcLo_Region_t *region;

    aeData = &objHandle->aeData[swAe];
    page = &aeData->aeSlice[sliceNum].pages[pageNum];
    region = page->region;
    return (region->loaded == page);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_verifyEncapUimage
   Description: Compares the encapsulated images to images that are loaded in
                the Uengines.
   Returns:     UCLO_SUCCESS, UCLO_FAILURE, or UCLO_BADARG, 
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_verifyEncapUimage(uclo_objHandle_T *objHandle, 
                       uof_encapAe_T *encapImage,
                       unsigned char swAe)
{
    uof_encapPage_T      *pages;
    uof_Image_T          *image;
    int                  p;
    unsigned char        hwAeNum;
    unsigned int         p_offset, s;

    if((!encapImage) || (swAe >= UOF_MAX_NUM_OF_AE))
    {
        return (UCLO_BADARG);
    }    

    image = encapImage->imagePtr;
    pages = encapImage->pages;
    
    /* find the slice to which the image is assigned */
    for(s=0; s < objHandle->aeData[swAe].numSlices; s++)
    {
        if(objHandle->aeData[swAe].aeSlice[s].assignedCtxMask & image->ctxAssigned) 
        {
           break;
        }   
    }
    if(s >= objHandle->aeData[swAe].numSlices) 
    {
        return (UCLO_FAILURE);
    }    

    hwAeNum = (unsigned char)UcLo_getHwAe(objHandle->prodType, swAe);

    /* traverse the pages */
    for(p = 0; p < image->numOfPages; p++)
    {
        unsigned int addr;
        uword_T fEngUword = 0, uword = 0;

        if(s >= MAX_CONTEXTS) 
        { 
           return (UCLO_FAILURE);
        }    
        if (!isPageLoaded(objHandle, s, p, swAe)) 
        {
            continue;
        }
        if(UcLo_fixupLocals(objHandle, &pages[p], swAe)) 
        {
           return (UCLO_FAILURE);
        }   

        p_offset = pages[p].begPhyAddr;

        for(addr = 0; addr < (unsigned int)pages[p].numMicroWords; addr++)
        {
             if(objHandle->aeData[swAe].shareableUstore) 
             {
                /* For shareable ustore, should use halAe_GetCoalesceUwords to get uwords from ustore, 
                * instead of halAe_GetUwords. */
                if(halAe_GetCoalesceUwords(hwAeNum, addr + p_offset, 1, &fEngUword) != HALAE_SUCCESS) 
                {
                    return (UCLO_FAILURE);
                }    
                /* For shared ctrl store, in uof file, the page relative address in the next AE will 
                 * be added by the physical offset of the page in the previous AE. So here, we need to pass the address 
                 * to Uclo_getUofUword after adding in the physical offset of the page in the previous AE. */
                if((uword = Uclo_getUofUword(objHandle, &pages[p], addr + p_offset)) == INVLD_UWORD) 
                {
                    return (UCLO_FAILURE);
                }    
            } 
            else 
            {
                if(halAe_GetUwords(hwAeNum, addr + p_offset, 1, &fEngUword) != HALAE_SUCCESS) 
                {
                    return (UCLO_FAILURE);
                }    
                /* Uclo_getUofUword() takes an address that is relative to the start of the page.
                   So we don't need to add in the physical offset of the page. */
                if((uword = Uclo_getUofUword(objHandle, &pages[p], addr)) == INVLD_UWORD) 
                {
                    return (UCLO_FAILURE);
                }    
            }

            /* ignore parity */
            fEngUword &= ICP_UWORD_MASK;
            uword &= ICP_UWORD_MASK;   

            if(fEngUword != uword) 
            {
                return (UCLO_FAILURE);
            }    
        }
    }

    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_MapObjAddr
   Description: Maps the UOF objects from the memory location described by the
                'addrPtr', and 'memSize' input parameters.  Reference to the
                objects is returned in the 'handle' parameter. The 'readOnly'
                parameter indicates that the mapped memory is read-only and any
                object that has to be updated will be copied by the this library.
   Returns:     UCLO_SUCCESS, UCLO_BADARG, UCLO_MEMFAIL, UCLO_BADOBJ, UCLO_UOFINCOMPAT
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_MapObjAddr(void **handle, 
                void *addrPtr, 
                int memSize, 
                int readOnly)
{
    uof_fileHdr_T       *fileHdr;
    uof_fileChunkHdr_T  *fileChunk;
    uclo_objHandle_T    *objHdl;
    void                *objAddr = addrPtr;
    int                 status = UCLO_SUCCESS;

    if(!handle || !objAddr || memSize < UCLO_MIN_UOF_SIZE) 
    {
        return (UCLO_BADARG);
    }    
    if(!(objHdl = UcLo_getObjHandle())) 
    {
        return (UCLO_MEMFAIL);
    }    
    
    if(readOnly)
    {
        if(!(objHdl->objBuf = ixOsalMemAlloc(memSize))) 
        {
            return (UCLO_MEMFAIL);
        }
        memmove(objHdl->objBuf,
               addrPtr,
               memSize);
        objAddr = objHdl->objBuf;
    }

    /* map the file header */
    if(!(fileHdr = (uof_fileHdr_T *)objAddr)) 
    {
        if(readOnly && objHdl->objBuf)
        {
            ixOsalMemFree(objHdl->objBuf);
        }
        return (UCLO_BADARG);
    }    

   /* verify file id */
    if((status = UcLo_verifyFile((char *)fileHdr, &objHdl->uofMinVer, &objHdl->uofMajVer)) 
                 != UCLO_SUCCESS)
    {
        if(readOnly && objHdl->objBuf)
        {
            ixOsalMemFree(objHdl->objBuf);
        }
        ixOsalMemFree(objHdl);
        return (status);
    }
    
    fileChunk = (uof_fileChunkHdr_T *)((char *)objAddr + sizeof(uof_fileHdr_T));

    objHdl->objCopied = 0;
    
    /* get the UOF object chunk  */
    if(!(objHdl->objHdr = UcLo_mapFileChunk((char *)objAddr, fileHdr, UOF_OBJS)))
    {
        if(readOnly && objHdl->objBuf)
        {
            ixOsalMemFree(objHdl->objBuf);
        }
        ixOsalMemFree(objHdl);
        return (UCLO_BADOBJ);
    }
    /* get the UOF-dbgObj file header  */
    objHdl->dbgObjHdr = UcLo_mapFileChunk((char *)objAddr, fileHdr, DBG_OBJS);

    if (UcLo_isProfileSupported())
    {
        /* get the UOF-prfObj file header  */
        objHdl->prfObjHdr = UcLo_mapFileChunk((char *)objAddr, fileHdr, PRF_OBJS);
    }

    if((status = UcLo_overlayObj(objHdl, readOnly)))
    {
        ixOsalMemFree(objHdl->objHdr);
        if(objHdl->dbgObjHdr) 
        {
           ixOsalMemFree(objHdl->dbgObjHdr); 
        }   

        if (UcLo_isProfileSupported())
        {
            if(objHdl->prfObjHdr) 
            {
               ixOsalMemFree(objHdl->prfObjHdr);
            }   
        }

        if(readOnly && objHdl->objBuf)
        {
            ixOsalMemFree(objHdl->objBuf);
        }
        ixOsalMemFree(objHdl);

        return (status);
    }

    *handle = (void *)objHdl;

    return (status);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_DeleObj
   Description: Removes and free all resources references associated with the handle.
   Returns:     UCLO_SUCCESS, UCLO_NOOBJ
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_DeleObj(void *handle)
{
    int          a;
    uclo_objHandle_T     *objHandle = (uclo_objHandle_T *)handle;

    /* check if an object is loaded or mapped */
    if(!objHandle || !objHandle->objHdr || !objHandle->objHdr->fBuf) 
    {
        return (UCLO_NOOBJ);
    }    
    
    halAe_DeleteUcloCallback(UcLo_ProcessHalCallbacks, (void*) objHandle);

    MUTEX_DESTORY(objHandle->overlayMutex);

    if (objHandle->bkptCallbackHandle) 
    {
        halAe_TerminateCallbackThd(objHandle->bkptCallbackHandle);

        objHandle->bkptCallbackHandle = NULL;
    }

    /* free resource that were allocated by the library */
    for(a = 0; a < objHandle->numUniqueImage; a++)
    {
        /* free encapsulated pages */
        if(objHandle->uniqueAeImage[a].pages) 
        {
            ixOsalMemFree(objHandle->uniqueAeImage[a].pages);
        }    
    }

    for(a = 0; a < (int)objHandle->numAEs; a++)
    {
        UcLo_FreeAeData(&objHandle->aeData[a]);
    }

    if(objHandle->objCopied)
    {
        if(objHandle->objHdr)
        {
            ixOsalMemFree(objHandle->objHdr->fBuf);
            ixOsalMemFree(objHandle->objHdr);
        }
        if(objHandle->dbgObjHdr)
        {
            ixOsalMemFree(objHandle->dbgObjHdr->fBuf);
            ixOsalMemFree(objHandle->dbgObjHdr);
        }
        
        if (UcLo_isProfileSupported())
        {
            if(objHandle->prfObjHdr)
            {
                ixOsalMemFree(objHandle->prfObjHdr->fBuf);
                ixOsalMemFree(objHandle->prfObjHdr);
            }
        }
    }
    else
    {
        if(objHandle->objHdr)
        {
            ixOsalMemFree(objHandle->objHdr);
        }        
    }
 
    if((objHandle->readOnly) && (objHandle->objBuf))
    {
        ixOsalMemFree(objHandle->objBuf);
    }

    ixOsalMemFree(objHandle);

    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_listUninitSym
   Description: List the uninitialized ucode symbols in the UOF image
   Returns:     UCLO_SUCCESS, UCLO_NOOBJ, UCLO_UNINITVAR, UCLO_IMGNOTFND
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_listUninitSym(void *handle, 
                   char *ucodeImageName)
{
    int                 p, status=UCLO_SUCCESS;
    uof_Image_T         *image;
    uof_encapPage_T     *pages;
    uclo_objHandle_T    *objHandle=(uclo_objHandle_T *)handle;
    unsigned int        lastVarName=0;
    uof_encapAe_T       *encapImage;

    if(!objHandle || !objHandle->objHdr->fBuf) 
    {
        return (UCLO_NOOBJ);
    }    
    if(!(encapImage = UcLo_getEncapImage(objHandle, ucodeImageName))) 
    {
        return (UCLO_IMGNOTFND);
    }    

    /* the image exists */
    if(!(image = encapImage->imagePtr)) 
    {
        return (UCLO_NOOBJ);
    }    
    if(!(pages = encapImage->pages)) 
    {
        return (UCLO_NOOBJ);
    }    

   /* traverse the pages */
    for(p = 0; p < image->numOfPages; p++)
    {
        uof_importVar_T   *impVar;
        unsigned int v;

        /* check all instance of the variable */
        for(v = 0; v < pages[p].numImpVar; v++)
        {
            impVar = &pages[p].impVar[v];

            if((!GET_FIXUP_ASSIGN(impVar->valueAttrs)))
            {
                /* attempt to print only one instance of the uninitialized symbol; this works because
                   the instances are normally grouped */
                if(impVar->name != lastVarName)
                {
                    UCLO_PRNWARN("WARNING: Uninitialized symbol '%s' in image '%s', default value=0x%x\n", UcLo_getString(&objHandle->strTable, impVar->name),
                            UcLo_getString(&objHandle->strTable, image->imageName), impVar->value);
                    lastVarName = impVar->name;
                }
                status = UCLO_UNINITVAR;
            }
        }
    }
    return (status);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_bindAllImagesSym
   Description: Binds a value to the Ucode symbol in all of the UOF images
   Returns:     UCLO_SUCCESS, UCLO_NOOBJ, UCLO_BADARG, UCLO_SYMNOTFND
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_bindAllImagesSym(void *handle, 
                      char *ucodeSymName, 
                      int value)
{
    int                 i, p, validSym=0;
    uof_Image_T         *image;
    uof_encapPage_T     *pages;
    uclo_objHandle_T    *objHandle=(uclo_objHandle_T *)handle;
    char                *str;

    if(!ucodeSymName) 
    {
        return (UCLO_BADARG);
    }    
    if(!objHandle || !objHandle->objHdr->fBuf) 
    {
        return (UCLO_NOOBJ);
    }    

    /* bind the symbol in all the images */
    for(i = 0; i < objHandle->numUniqueImage; i++)
    {
        if(!(image = objHandle->uniqueAeImage[i].imagePtr)) 
        {
             continue;
        }     
        if(!(pages = objHandle->uniqueAeImage[i].pages)) 
        {
             continue;
        }     

        /* traverse the pages */
        for(p = 0; p < image->numOfPages; p++)
        {
            uof_importVar_T   *impVar;
            unsigned int v;

            /* update all instance of the variable */
            for(v = 0; v < pages[p].numImpVar; v++)
            {
                impVar = &pages[p].impVar[v];
                str = UcLo_getString(&objHandle->strTable, impVar->name);
                if(!str) 
                {
                    continue;
                }    
                if(!UcLo_strcmp(ucodeSymName, str,
                                image->sensitivity))
                {
                    impVar->value = value;
                    SET_FIXUP_ASSIGN(impVar->valueAttrs, 1);
                    validSym = 1;
                }
            }
        }
    }
    if(!validSym)
    {
        UCLO_PRNWARN("WARNING: Symbol '%s' is not found in any of the UOF image(s)\n", ucodeSymName);
        return (UCLO_SYMNOTFND);
    }
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_bindSym
   Description: Binds a value to the Ucode symbol in the UOF image
   Returns:     UCLO_SUCCESS, UCLO_NOOBJ, UCLO_BADARG, UCLO_SYMNOTFND
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_bindSym(void *handle, 
             uof_encapAe_T *encapImage,
             char *ucodeSymName, 
             int value)
{
    int                    p, validSym=0;
    uof_Image_T            *image;
    uof_encapPage_T        *pages;
    uclo_objHandle_T    *objHandle=(uclo_objHandle_T *)handle;
    char                   *str;

    if(!ucodeSymName) 
    {
        return (UCLO_BADARG);
    }    
    if(!objHandle || !objHandle->objHdr->fBuf || !encapImage) 
    {
        return (UCLO_NOOBJ);
    }    

    /* the image exists */
    if(!(image = encapImage->imagePtr)) 
    {
        return (UCLO_NOOBJ);
    }    
    if(!(pages = encapImage->pages)) 
    {
        return (UCLO_NOOBJ);
    }    

   /* traverse the pages */
    for(p = 0; p < image->numOfPages; p++)
    {
        uof_importVar_T   *impVar;
        unsigned int v;

        /* update all instance of the variable */
        for(v = 0; v < pages[p].numImpVar; v++)
        {
            impVar = &pages[p].impVar[v];
            str = UcLo_getString(&objHandle->strTable, impVar->name);
            if(!str) 
            {
                continue;
            }    
            if(!UcLo_strcmp(ucodeSymName, str,
                            image->sensitivity))
            {
                impVar->value = value;
                SET_FIXUP_ASSIGN(impVar->valueAttrs, 1);
                validSym = 1;
            }
        }
    }
    if(!validSym) 
    {
        return (UCLO_SYMNOTFND);
    }    
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_BindSymbol
   Description: Binds a value to the Ucode symbol in the UOF image
   Returns:     UCLO_SUCCESS, UCLO_NOOBJ, UCLO_BADARG, UCLO_IMGNOTFND, UCLO_SYMNOTFND
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_BindSymbol(void *handle, 
                char *ucodeImageName, 
                char *ucodeSymName, 
                int value)
{
    uclo_objHandle_T    *objHandle=(uclo_objHandle_T *)handle;
    uof_encapAe_T        *encapImage;

    if(!ucodeSymName || !ucodeImageName) 
    {
        return (UCLO_BADARG);
    }    
    if(!objHandle || !objHandle->objHdr->fBuf) 
    {
        return (UCLO_NOOBJ);
    }    
    if(!(strcmp(ucodeImageName, "*")))
    {
        return (UcLo_bindAllImagesSym(objHandle, ucodeSymName, value));
    }

    if(!(encapImage = UcLo_getEncapImage(objHandle, ucodeImageName))) 
    {
      return (UCLO_IMGNOTFND);
    }  

    return (UcLo_bindSym(objHandle, encapImage, ucodeSymName, value));
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_AeBindSymbol
   Description: Binds a value to the Ucode symbol in the UOF image. Because
                multiple AEs may be assigned to the same UOF image, the result
                of this function will not only apply to the specified AE, but also
                to the other assigned AEs.
   Returns:     UCLO_SUCCESS, UCLO_NOOBJ, UCLO_BADARG, UCLO_IMGNOTFND, UCLO_SYMNOTFND
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_AeBindSymbol(void *handle, 
                  unsigned char hwAe, 
                  char *ucodeSymName, 
                  int value)
{
    unsigned int swAe, s;
    int status=UCLO_SUCCESS, stat = UCLO_SUCCESS;

    uclo_objHandle_T *objHandle = (uclo_objHandle_T *)handle;

    if(!objHandle || !objHandle->objHdr->fBuf) 
    {
        return (UCLO_BADARG);
    }    
    if((swAe = UcLo_getSwAe(objHandle->prodType, hwAe)) == UCLO_BADAE) 
    {
        return (UCLO_BADARG);
    }    
    if(!objHandle->aeData[swAe].numSlices) 
    {
        return (UCLO_SYMNOTFND);
    }    

    for(s=0; s < objHandle->aeData[swAe].numSlices; s++) 
    {
        if((stat = UcLo_bindSym(objHandle, objHandle->aeData[swAe].aeSlice[s].encapImage, ucodeSymName, value)))
        {
            if(stat == UCLO_SYMNOTFND) 
            {
                status = stat;
            }    
            else 
            {
                if(stat != UCLO_SUCCESS) 
                {
                     return (stat);  /* return UCLO_NOOBJ or UCLO_BADARG */
                }
            }    
        }
    }    
    return (status);  /* return UCLO_SUCCESS or UCLO_SYMNOTFND */
}


/*-----------------------------------------------------------------------------
   Function:    UcLo_writeUimagePageRaw
   Description: Writes the specified page in the UOF image to the assigned
                uEngines. This only does the writing of ustore.
   Returns:     UCLO_SUCCESS, UCLO_*
-----------------------------------------------------------------------------*/
int 
UcLo_writeUimagePageRaw(uclo_objHandle_T *objHandle,
                        uof_encapPage_T  *encapPage, 
                        unsigned int swAe)
{
    unsigned int     hwAeNum;
    unsigned int     uPhysicalAddr, uRelativeAddr, i, numWords, cpyLen;
    int              status = UCLO_SUCCESS;
    
    uword_T          *uwords, fillPat;
    uwords = (uword_T *)ixOsalMemAlloc(UWORD_CPYBUF_SIZE*8);
    if(uwords == NULL) {
        return (UCLO_FAILURE);
    }

    hwAeNum = objHandle->hwAeNum[swAe];
    /* load the page starting at appropriate ustore address */
    if((status = UcLo_fixupLocals(objHandle, encapPage, (unsigned char)swAe)) == UCLO_FAILURE) 
    {
        ixOsalMemFree(uwords);
        return (UCLO_FAILURE);
    }

    /* get fill-pattern from an image -- they are all the same */
    memmove(&fillPat, objHandle->uniqueAeImage[0].imagePtr->fillPattern, sizeof(uword_T));
    
    uPhysicalAddr = encapPage->begPhyAddr;
    uRelativeAddr = 0;
    numWords = encapPage->numMicroWords;
    while(numWords)
    {
        if(numWords < UWORD_CPYBUF_SIZE) 
        {
           cpyLen = numWords;
        }               
        else 
        {
           cpyLen = UWORD_CPYBUF_SIZE;
        }   

        /* load the buffer */
        for(i=0; i < cpyLen; i++)
        {
            /* keep below code structure in case there are different handling for shared secnarios */
            if(!objHandle->aeData[swAe].shareableUstore) 
            {
                /* Uclo_getUofUword() takes an address that is relative to the start of the page.
                   So we don't need to add in the physical offset of the page. */
                if(encapPage->pageRegion !=0 ) 
                {
                    if((uwords[i] = Uclo_getUofUword(objHandle, encapPage, uRelativeAddr+i)) == INVLD_UWORD)
                    {
                        uwords[i] = fillPat;   /* fill hole in the uof */
                    }    
                } 
                else
                {
                    /* for mixing case, it should take physical address */
                    if((uwords[i] = Uclo_getUofUword(objHandle, encapPage, uPhysicalAddr+i)) == INVLD_UWORD)
                    {
                        uwords[i] = fillPat;   /* fill hole in the uof */
                    }    
                } 
            } 
            else 
            {
                /* For shared ctrl store, in uof file, the page relative address in the next AE will be added  
                 * by the physical offset of the page in the previous AE. So here, we need to pass the address
                   to Uclo_getUofUword after adding in the physical offset of the page in the previous AE. */ 
                if(encapPage->pageRegion !=0 ) 
                {
                    if((uwords[i] = Uclo_getUofUword(objHandle, encapPage, uRelativeAddr+i)) == INVLD_UWORD) 
                    {
                        uwords[i] = fillPat;   /* fill hole in the uof */
                    }    
                } 
                else
                {
                    if((uwords[i] = Uclo_getUofUword(objHandle, encapPage, uPhysicalAddr+i)) == INVLD_UWORD) 
                    {
                        uwords[i] = fillPat;   /* fill hole in the uof */
                    }    
                }
            }
        }

        /* copy the buffer to ustore */
        if(objHandle->aeData[swAe].shareableUstore)
        {
            if(halAe_PutCoalesceUwords((unsigned char)hwAeNum, uPhysicalAddr, cpyLen, uwords) != HALAE_SUCCESS) 
            {
                ixOsalMemFree(uwords);
                return (UCLO_FAILURE);
            }
        }
        else 
        {
            if(halAe_PutUwords((unsigned char)hwAeNum, uPhysicalAddr, cpyLen, uwords) != HALAE_SUCCESS)
            {
                ixOsalMemFree(uwords);
                return (UCLO_FAILURE);
            }
        }
        uPhysicalAddr += cpyLen;
        uRelativeAddr += cpyLen;
        numWords -= cpyLen;
    }

    ixOsalMemFree(uwords);

    return (status);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_writeUimagePages
   Description: Writes the specified page in the UOF image to the assigned uEngines.
   Returns:     UCLO_SUCCESS, UCLO_NOOBJ, UCLO_IMGNOTFND, UCLO_BADARG, UCLO_UNINITVAR,
                UCLO_IVDWARN, UCLO_FAILURE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
static int 
UcLo_writeUimagePages(uclo_objHandle_T *objHandle,
                      char *ucodeImageName, 
                      int *startingPageChange)
{
    unsigned int        mex, ctxMask, s;
    uof_encapAe_T       *encapImage;
    UcLo_Page_t         *page, *oldPage=NULL;
    uof_Image_T         *image;
    unsigned char        swAe, hwAeNum, shareableUstore;
    unsigned int         hwNeighNum;
    int                  retval = UCLO_SUCCESS, status = UCLO_SUCCESS;
    int                  pageNum, ctx;
    if(!objHandle || !objHandle->objHdr->fBuf) 
    {
        return (UCLO_NOOBJ);
    }    
    if((retval = UcLo_initGlobals(objHandle))) 
    {
        if(retval != UCLO_IVDWARN) {
           return (retval);
        }   
    }    
    if(!(encapImage = UcLo_getEncapImage(objHandle, ucodeImageName))) 
    {
        return (UCLO_IMGNOTFND);
    }    

    image = encapImage->imagePtr;

    if(CTX_MODE(image->aeMode) == MAX_CTX) 
    {
        ctxMask = 0xff; /* eight ctx-mode */
    }    
    else
    {
        ctxMask = 0x55; /* four ctx-mode */
    }    

    /* load the default page and set assigned CTX PC to the entrypoint address */
    for(swAe = 0; (swAe < objHandle->numAEs) && (swAe < UOF_MAX_NUM_OF_AE); swAe++)
    {
        if((mex = UcLo_getHwAe(objHandle->prodType, swAe)) == UCLO_BADAE) 
        {
            return (UCLO_FAILURE);
        }        
        hwAeNum = (unsigned char)mex;

        shareableUstore = (unsigned char)(objHandle->aeData[swAe].shareableUstore);
        if(shareableUstore) 
        {
            hwNeighNum = UcLo_getSharedUstoreNeigh(objHandle, hwAeNum);
            if (hwNeighNum == UCLO_BADAE)
            {
                return (UCLO_NEIGHNOTFND);
            }
        }    
        else
        {
            hwNeighNum = 0;  /* unused */
        }    

        if(isBitSet(image->aeAssigned, hwAeNum))
        {
            /* find the slice to which this image is assigned */
            for (s=0; s < objHandle->aeData[swAe].numSlices; s++)
            {
                if(image->ctxAssigned & objHandle->aeData[swAe].aeSlice[s].assignedCtxMask) 
                {
                    break;
                }    
            }
            if (s >= objHandle->aeData[swAe].numSlices) 
            { 
                continue;
            }    

            for (pageNum = 0; pageNum < image->numOfPages; pageNum++) 
            {
                page = &objHandle->aeData[swAe].aeSlice[s].pages[pageNum];
                /* Only load pages loaded by default */
                if (! page->encapPage->defPage) 
                { 
                    continue;
                }    

                if((status = UcLo_doPageIn(objHandle, swAe, page, oldPage, startingPageChange))) 
                {
                    if (status == UCLO_UNINITVAR) 
                    {
                        retval = status;
                    }    
                    else
                    { 
                        return (status);
                    }    
                }
            } /* end for pageNum */

            /* Assume starting page is page 0 */
            page = &objHandle->aeData[swAe].aeSlice[s].pages[0];
            for (ctx=0; ctx<MAX_CONTEXTS; ctx++) 
            {
                if (ctxMask & (1<<ctx)) 
                {
                    objHandle->aeData[swAe].aeSlice[s].currentPage[ctx] = page;
                }    
                else
                {
                    objHandle->aeData[swAe].aeSlice[s].currentPage[ctx] = NULL;
                }    
            }

            /* set the live context */
            if(halAe_SetLiveCtx(hwAeNum, image->ctxAssigned)) 
            {
               return (UCLO_FAILURE);
            }   

            /* set context PC to the image entrypoint address */
            if(halAe_PutPC(hwAeNum, image->ctxAssigned, image->entryAddress)) 
            {
               return (UCLO_FAILURE);
            }   
        }
    }

    /* store the checksum in the HAL for convenience */
    halAe_setUofChecksum(objHandle->objHdr->checksum);

    if ((! objHandle->bkptCallbackHandle) && (image->numOfPages > 1)) 
    {
        halAe_SpawnIntrCallbackThdEx(HALAE_INTR_ATTN_BKPT_MASK,
                                     UcLo_IntrCallback,
                                     (void*)objHandle,
                                     IX_OSAL_DEFAULT_THREAD_PRIORITY,
                                     20, /* callback priority */
                                     &objHandle->bkptCallbackHandle);
    }

    return (retval);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_InitLibUeng
   Description: Initializes the Loader Library. Uengines with
                their corresponding bit set in aeMask will be set to the
                powerup default state. The library will assume that those
                uengs that are not specified are in reset. 
   Returns: 
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
void 
UcLo_InitLibUeng(unsigned int aeMask)
{
    static int onlyOnce = 0;

    if(onlyOnce) 
    {
       return;
    }   
    onlyOnce = 1;
 
    halAe_Init(aeMask);
    return;
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_InitLib
   Description: Initializes the Loader Library
   Returns: 
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
void 
UcLo_InitLib(void)
{
    halAe_Init(UCLO_ALLAE);
    return;
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_WriteUimage
   Description: Writes the default pages of the specified UOF image to the
                assigned uEngines.
   Returns:     UCLO_SUCCESS, UCLO_NOOBJ, UCLO_IMGNOTFND, UCLO_BADARG,
                UCLO_UNINITVAR, UCLO_FAILURE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_WriteUimage(void *handle, 
                 char *ucodeImageName)
{
    int stat;
    int startingPageChange = 0;
    if(!handle)
    {
        return (UCLO_NOOBJ);
    }    
    if((stat = UcLo_writeUimagePages((uclo_objHandle_T *)handle,
                                     ucodeImageName,
                                     &startingPageChange)) == UCLO_UNINITVAR) 
    {
        UcLo_listUninitSym(handle, ucodeImageName);
    }    
    if (startingPageChange) 
    {
        halAe_CallNewPageCallback(END_OF_PAGE_CHANGE, 0, 0, 0);
    }

    return (stat);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_computeFreeUstore
   Description: Computes the amount of free ustore in each of the assigned uEngines
                and informs HAL of the results.
   Returns:     UCLO_SUCCESS, UCLO_FAILURE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_computeFreeUstore(uclo_objHandle_T *objHandle)
{
    /* We have to wait until all images are written before we can calculate the amount 
       of free ustore in each AE. This is because there may be multiple images loaded 
       into different slices in the same AE and/or shareable ustore may be enabled.
    */
    UcLo_AeData_t  *aeData;
    unsigned int    sliceNum, totalUwords;
    int             imageNum;
    unsigned char   swAe, hwAeNum, swNeighMe, shareableUstore;
    unsigned int    hwNeighNum;
    uof_Image_T    *imagePtr;
    unsigned int     mex;

    for (swAe = 0; (swAe < objHandle->numAEs) && (swAe < UOF_MAX_NUM_OF_AE); swAe++)
    {
        if ((mex = UcLo_getHwAe(objHandle->prodType, swAe)) == UCLO_BADAE) 
        {
            return (UCLO_FAILURE);
        }    
        hwAeNum = (unsigned char)mex;

        totalUwords = 0;

        /* get the AE data for the AE */
        aeData = &objHandle->aeData[swAe];

        /* determine whether this AE is sharing ustore with its neighbor AE. If
           sharing is enabled and both AEs have images loaded into them, then both 
           will have their shareableUstore flag set. However, if one of the AEs 
           doesn't have an image loaded into it, then its flag won't be set. So in
           that case we need to check the paired AE's flag. */
        hwNeighNum = UcLo_getSharedUstoreNeigh(objHandle, hwAeNum);
        if (hwNeighNum == UCLO_BADAE)
        {
            return (UCLO_NEIGHNOTFND);
        }
        swNeighMe = objHandle->swAe[hwNeighNum];
        if (swNeighMe >= UOF_MAX_NUM_OF_AE)
        {
            return (UCLO_NEIGHNOTFND);
        }
        if (aeData->numSlices == 0)
        {
            /* no images loaded into the AE, so use the paired AE's flag. */
            shareableUstore = (unsigned char)(objHandle->aeData[swNeighMe].shareableUstore);
        }
        else
        {
            /* AE has an image loaded, so use its flag. */
            shareableUstore = (unsigned char)(objHandle->aeData[swAe].shareableUstore);
        }

        /* if ustore is being shared and the neighbor AE number is lower than
           the current AE number, then skip processing this AE because it
           was already processed when its neighbor was processed. */
        if (shareableUstore && (swNeighMe < swAe)) 
        {
            continue;
        }    

        /* the # of used uwords for each slice has already been computed taking into
           account the offset required if other images are mixed in the same ustore or
           if ustore is shared. so traverse all slices and find the maximum uwords
           used. */
        for (sliceNum = 0; sliceNum < aeData->numSlices; sliceNum++)
        {
            if (aeData->aeSlice[sliceNum].assignedCtxMask != 0)
            {
                imagePtr = aeData->aeSlice[sliceNum].encapImage->imagePtr;
                for (imageNum = 0; imageNum < objHandle->numUniqueImage; imageNum++)
                {
                    if (objHandle->uniqueAeImage[imageNum].imagePtr == imagePtr)
                    {
                        /* Found image that is loaded into this slice. Update the uwords
                           used if necessary. */
                        if (objHandle->uniqueAeImage[imageNum].numUwordsUsed > totalUwords) 
                        {
                            totalUwords = objHandle->uniqueAeImage[imageNum].numUwordsUsed;
                        }    
                    }
                }
            }
        }

        /* if sharing ustore then we need to consider the neighbor. Else, just
           treat this AE independently */
        if (shareableUstore)
        {
            /* sharing ustore */
            /* get the AE data for the neighbor AE */
            aeData = &objHandle->aeData[swNeighMe];
            for (sliceNum = 0; sliceNum < aeData->numSlices; sliceNum++)
            {
                if (aeData->aeSlice[sliceNum].assignedCtxMask != 0)
                {
                    imagePtr = aeData->aeSlice[sliceNum].encapImage->imagePtr;
                    for (imageNum = 0; imageNum < objHandle->numUniqueImage; imageNum++)
                    {
                        if (objHandle->uniqueAeImage[imageNum].imagePtr == imagePtr)
                        {
                            /* Found image that is loaded into this slice. Update the uwords
                               used if necessary. */
                            if (objHandle->uniqueAeImage[imageNum].numUwordsUsed > totalUwords) 
                            {
                                totalUwords = objHandle->uniqueAeImage[imageNum].numUwordsUsed;
                            }    
                        }
                    }
                }
            }

            /* two shared AE ustore have the same free ustore view */
            if (halAe_SetUstoreFreeMem(hwAeNum, totalUwords, objHandle->ustorePhySize*2 - totalUwords) != HALAE_SUCCESS) 
            {
                return (UCLO_FAILURE);
            }    
            if (halAe_SetUstoreFreeMem((unsigned char)hwNeighNum, totalUwords, objHandle->ustorePhySize*2 - totalUwords) != HALAE_SUCCESS) 
            {
                return (UCLO_FAILURE);
            }    
        }
        else
        {
            /* not sharing ustore. let the ae lib know the number of useful micro words */
            if (halAe_SetUstoreFreeMem(hwAeNum, totalUwords, objHandle->ustorePhySize - totalUwords) != HALAE_SUCCESS) 
            {
                return (UCLO_FAILURE);
            }    
        }
    }
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_WriteUimageAll
   Description: Writes all of the UOF images to the assigned uEngines.
   Returns:     UCLO_SUCCESS, UCLO_NOOBJ, UCLO_IMGNOTFND, UCLO_BADARG,
                UCLO_UNINITVAR, UCLO_SYMNOTFND, UCLO_FAILURE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_WriteUimageAll(void *handle)
{
    int              i, stat, retVal = UCLO_SUCCESS;
    uclo_objHandle_T *objHandle = (uclo_objHandle_T *)handle;
    char             *imageName;
    int              startingPageChange = 0, bailOut=0;

    if (!objHandle || !objHandle->objHdr->fBuf) 
    {
        return (UCLO_NOOBJ);
    }    

    for (i = 0; (i < objHandle->numUniqueImage) && !bailOut; i++)
    {
        imageName = UcLo_getString(&objHandle->strTable, objHandle->uniqueAeImage[i].imagePtr->imageName);
        if ((stat = UcLo_writeUimagePages(objHandle, imageName, &startingPageChange)) != UCLO_SUCCESS)
        {
            switch(stat)
            {
            case UCLO_UNINITVAR:
                retVal= stat;       /* warning -- uninitialized value in image */
                UcLo_listUninitSym(handle, imageName);
                break;
            case UCLO_IVDWARN:
                retVal= stat;       /* warning -- unknown image or symbol in IVD */
                break;
            default:
               retVal = stat;
               bailOut=1;
               break;
            }
        }
    }

    /* We have to wait until all images are written before we can calculate the amount 
       of free ustore in each AE. This is because there may be multiple images loaded 
       into different slices in the same AE and/or shareable ustore may be enabled. */
    stat = UcLo_computeFreeUstore(objHandle);

    /* if we previously detected an error, then let it stand. Else, use the new return status */
    if (retVal == UCLO_SUCCESS) 
    {
        retVal = stat;
    }    

    if (startingPageChange) 
    {
        halAe_CallNewPageCallback(END_OF_PAGE_CHANGE, 0, 0, 0);
    }    

    return (retVal);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_VerifyUimage
   Description: Compares the UOF image to the content of the assigned uEngines
   Returns:     UCLO_SUCCESS, UCLO_NOOBJ, UCLO_IMGNOTFND, UCLO_FAILURE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_VerifyUimage(void *handle, char *ucodeImageName)
{
    uof_encapAe_T     *encapImage;
    uof_Image_T         *image;
    unsigned char        swAe;
    uclo_objHandle_T     *objHandle = (uclo_objHandle_T *)handle;

    if(!objHandle || !objHandle->objHdr->fBuf) 
    {
        return (UCLO_NOOBJ);
    }    

    if(!(encapImage = UcLo_getEncapImage(objHandle, ucodeImageName))) 
    {
        return (UCLO_IMGNOTFND);
    }    

    /* Don't compare ustore if it is not normal scheme */
    if(halAe_getMMScheme()) 
    {
        return (UCLO_SUCCESS);
    }   

    /* compare all assigned uEngine images */
    image = encapImage->imagePtr;
    for(swAe = 0; (swAe < objHandle->numAEs) && (swAe < UOF_MAX_NUM_OF_AE); swAe++)
    {
        if(isBitSet(image->aeAssigned, UcLo_getHwAe(objHandle->prodType, swAe)))
        {
            if(UcLo_verifyEncapUimage(objHandle, encapImage, swAe)) 
            {
               return (UCLO_FAILURE);
            }   
        }
    }

    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_VerifyUengine
   Description: Compares the content of the specified uEngine to the UOF image
   Returns:     UCLO_SUCCESS, UCLO_NOOBJ, UCLO_IMGNOTFND, UCLO_BADARG, UCLO_FAILURE
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_VerifyUengine(void *handle, 
                   unsigned char hwAe)
{
    uclo_objHandle_T     *objHandle = (uclo_objHandle_T *)handle;
    unsigned int swAe, s;
    int status = UCLO_SUCCESS;

    if(!objHandle || !objHandle->objHdr || !objHandle->objHdr->fBuf) 
    {
        return (UCLO_NOOBJ);
    }    
    swAe = UcLo_getSwAe(objHandle->prodType, hwAe);
    if((swAe == UCLO_BADAE) || (swAe >= UOF_MAX_NUM_OF_AE)) 
    {
        return (UCLO_BADARG);
    }    

    if(objHandle->aeData[swAe].numSlices == 0) 
    {
        return (UCLO_IMGNOTFND);
    }    

    /* Don't compare ustore if it is not normal scheme */
    if(halAe_getMMScheme()) 
    {
        return (UCLO_SUCCESS);
    }   

    for(s=0; s < objHandle->aeData[swAe].numSlices; s++)
    {
        if(!objHandle->aeData[swAe].aeSlice[s].encapImage ||
           !objHandle->aeData[swAe].aeSlice[s].encapImage->pages ||
           !objHandle->aeData[swAe].aeSlice[s].encapImage->imagePtr) 
        {
            return (UCLO_IMGNOTFND);
        }      
        if((status = UcLo_verifyEncapUimage(objHandle, objHandle->aeData[swAe].aeSlice[s].encapImage,
            (unsigned char)swAe))) 
        {
            break;
        }    
    }
    return (status);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_GetVarMemSegs
   Description: Get the memory segment usage.
   Returns:     UCLO_SUCCESS, UCLO_NOOBJ, UCLO_BADARG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_GetVarMemSegs(void *handle, 
                   UcLo_VarMemSeg_T *varMemSegs)
{
    unsigned int hwAe, swAe, s;
    uclo_objHandle_T     *objHandle = (uclo_objHandle_T *)handle;

    if(!objHandle || !objHandle->objHdr->fBuf) 
    {
        return (UCLO_NOOBJ);
    }    
    if(!varMemSegs) 
    {
        return (UCLO_BADARG);
    }    

    varMemSegs->sram0BaseAddr = objHandle->encapUofObj.varMemSeg->sram0Base;
    varMemSegs->sram0Size = objHandle->encapUofObj.varMemSeg->sram0Size;
    varMemSegs->sram0Alignment = objHandle->encapUofObj.varMemSeg->sram0Alignment;

    varMemSegs->sdramBaseAddr = objHandle->encapUofObj.varMemSeg->sdramBase;
    varMemSegs->sdramSize = objHandle->encapUofObj.varMemSeg->sdramSize;
    varMemSegs->sdramAlignment = objHandle->encapUofObj.varMemSeg->sdramAlignment;

    varMemSegs->sdram1BaseAddr = objHandle->encapUofObj.varMemSeg->sdram1Base;
    varMemSegs->sdram1Size = objHandle->encapUofObj.varMemSeg->sdram1Size;
    varMemSegs->sdram1Alignment = objHandle->encapUofObj.varMemSeg->sdram1Alignment;

    varMemSegs->scratchBaseAddr = objHandle->encapUofObj.varMemSeg->scratchBase;
    varMemSegs->scratchSize = objHandle->encapUofObj.varMemSeg->scratchSize;
    varMemSegs->scratchAlignment = objHandle->encapUofObj.varMemSeg->scratchAlignment;

    ixOsalMemSet(varMemSegs->meUmemSeg, 0, objHandle->numAEs * sizeof(UcLo_VarUmemSeg_T));

    for(swAe = 0; (swAe < objHandle->numAEs) && (swAe < UOF_MAX_NUM_OF_AE); swAe++)
    {
        if((hwAe = UcLo_getHwAe(objHandle->prodType, (unsigned char)swAe)) == UCLO_BADAE) 
        {
            return (UCLO_FAILURE);
        }        

        for(s=0; s < objHandle->aeData[swAe].numSlices; s++)
        {
            /* check if a page is assigned */
            if((!objHandle->aeData[swAe].aeSlice[s].encapImage) || (!objHandle->aeData[swAe].aeSlice[s].encapImage->pages)) 
            {
                continue;
            }    

            /* get the number of ustore used -- assuming that umem starts at the end of the ucode through the
            last highest uaddr referenced. */
            if(varMemSegs->meUmemSeg[hwAe].umemSize < objHandle->aeData[swAe].aeSlice[s].encapImage->numUwordsUsed) 
            {
                varMemSegs->meUmemSeg[hwAe].umemSize = objHandle->aeData[swAe].aeSlice[s].encapImage->numUwordsUsed;
            }    
        }
    }

    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_GetChecksum
   Description: Return the object's CRC checksum
   Returns:     UCLO_SUCCESS, UCLO_NOOBJ, UCLO_BADARG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_GetChecksum(void *handle, 
                 unsigned int *checksum)
{
    uclo_objHandle_T     *objHandle = (uclo_objHandle_T *)handle;

    if(!checksum) 
    {
        return (UCLO_BADARG);
    }    
    if(!objHandle || !objHandle->objHdr->fBuf) 
    {
        return (UCLO_NOOBJ);
    }    
    *checksum = objHandle->objHdr->checksum;

    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_GetAssignedAEs
   Description: Get the all the AEs that are assigned to an image in the UOF.
   Returns:     Mask of the assigned AEs
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
unsigned int 
UcLo_GetAssignedAEs(void *handle)
{
    int i;
    unsigned int aeMask=0;
    uclo_objHandle_T *objHandle = (uclo_objHandle_T *)handle;

    if(objHandle && objHandle->objHdr->fBuf)
    {
        /* traverse the images and set aeMask */
        for(i = 0; i < objHandle->numUniqueImage; i++){
            aeMask |= objHandle->uniqueAeImage[i].imagePtr->aeAssigned;
        }
    }

    return (aeMask);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_GetAssignedCtxs
   Description: Get the contexts that are assigned to the AE in the UOF.
   Returns:     Mask of the assigned contexts.
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
unsigned int 
UcLo_GetAssignedCtxs(void *handle, 
                     unsigned char hwAe)
{
    unsigned int ctxMask=0, swAe, s;
    uclo_objHandle_T *objHandle = (uclo_objHandle_T *)handle;

    if(objHandle && objHandle->objHdr->fBuf)
    {
        if((swAe = UcLo_getSwAe(objHandle->prodType, hwAe)) == UCLO_BADAE) 
        {
            return (ctxMask);
        }    

        for(s=0; s < objHandle->aeData[swAe].numSlices; s++) 
        {
            ctxMask |= objHandle->aeData[swAe].aeSlice[s].assignedCtxMask;
        }    
    }
    return (ctxMask);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_deleDbgInfo
   Description: Delete resource the contents of a dbgAeInfo_Image_T structure.
   Returns:     pointer to the chunk header or NULL if not found
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
void 
UcLo_deleDbgInfo(dbgAeInfo_Image_T *image)
{
    if(!image) 
    {
        return;
    }    
    while(image->numImage--)
    {
        if(image->srcImage[image->numImage].numSrcLine) 
        {
            ixOsalMemFree(image->srcImage[image->numImage].srcLine);
        }    
        if(image->srcImage[image->numImage].label) 
        {
            ixOsalMemFree(image->srcImage[image->numImage].label);
        }    
    }
    return;
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_getDbgInfo
   Description: Retrieve the udbug information from debug section of an uof.
   Returns:     UCLO_SUCCESS, UCLO_FAILURE, UCLO_BADARG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_getDbgInfo(uof_objHdr_T *dbgObjHdr,
                dbgAeInfo_Image_T *image,
                uof_strTab_T *strTable) /* strTable may be NULL */
{
    unsigned int         numDbgInfo = 0;
    unsigned int         i, ii, lastAddr;
    void                *cur = NULL;
    dbg_Image_T         *dbgImage;
    dbg_Source_T        *dbgSrc;
    dbgAeInfo_SrcLine_T *srcLine;
    uof_chunkHdr_T      *chunk;
    dgb_ObjTable_T      *objTab;
    dbg_Label_T         *dbgLbl;
    dbgAeInfo_Label_T   *dbgInfoLabel;

    if(!dbgObjHdr || !image) 
    {
        return (UCLO_BADARG);
    }    

    image->numImage = 0;

    while((chunk = (uof_chunkHdr_T *)UcLo_findChunk(dbgObjHdr, DBG_IMAG, cur)))
    {
        cur = chunk;
        dbgImage = (dbg_Image_T *)(chunk->offset + (char *)dbgObjHdr);

        objTab = (dgb_ObjTable_T *)((char *)dbgObjHdr + dbgImage->srcTabOffset);
        dbgSrc = (dbg_Source_T *)((char *)objTab + sizeof(dgb_ObjTable_T));
        image->srcImage[image->numImage].aeAssigned = dbgImage->aeAssigned;
        image->srcImage[image->numImage].ctxAssigned = dbgImage->ctxAssigned;

        /* count the number of useful dbg info */
        lastAddr = 0xffffffff;
        numDbgInfo = 0;
        for(i = 0; i < objTab->numEntries; i++)
        {
            /* skip src lines that are associated with the same uword */
            if(lastAddr == dbgSrc[i].addr) 
            {
                continue;
            }    
            else
            {
                lastAddr = dbgSrc[i].addr;
            }    
            numDbgInfo++;
        }

        if(!(srcLine = (dbgAeInfo_SrcLine_T *)ixOsalMemAlloc(sizeof(dbgAeInfo_SrcLine_T) * numDbgInfo)))
        {             
            UcLo_deleDbgInfo(image);
            return (UCLO_FAILURE);
        }

        ii = 0;
        lastAddr = 0xffffffff;
        for(i = 0; i < objTab->numEntries; i++)
        {
            /* store only the last src line info that is associated with uword */
            if((lastAddr != 0xffffffff) && (lastAddr != dbgSrc[i].addr)) 
            {
                ii++;
            }    

            lastAddr = dbgSrc[i].addr;
            srcLine[ii].uAddr = dbgSrc[i].addr;
            srcLine[ii].brkPtAllowed = dbgSrc[i].validBkPt;
            srcLine[ii].deferCount = dbgSrc[i].deferCount;
            srcLine[ii].brAddr = dbgSrc[i].brAddr;
            srcLine[ii].regAddr = dbgSrc[i].regAddr;
            srcLine[ii].regType = dbgSrc[i].regType;
            srcLine[ii].ctxArbKill = dbgSrc[i].ctxArbKill;
            srcLine[ii].reserved1 = 0;
            if (strTable) 
            {
                srcLine[ii].text = UcLo_getString(strTable, dbgSrc[i].lines);
            }    
            else
            {
                srcLine[ii].text = "";
            }    
        }

        image->srcImage[image->numImage].numSrcLine = (unsigned short)numDbgInfo;
        image->srcImage[image->numImage].srcLine = srcLine;

        /* Look up label info */
        if (strTable) 
        {
            objTab = (dgb_ObjTable_T *)((char *)dbgObjHdr + dbgImage->lblTabOffset);
            dbgLbl = (dbg_Label_T*)((char *)objTab + sizeof(dgb_ObjTable_T));
            dbgInfoLabel = (dbgAeInfo_Label_T*)ixOsalMemAlloc(sizeof(dbgAeInfo_Label_T) * objTab->numEntries);
            if (dbgInfoLabel == NULL) 
            {
                ixOsalMemFree(srcLine);
                UcLo_deleDbgInfo(image);
                return (UCLO_FAILURE);
            }
            for (i=0; i<objTab->numEntries; i++) 
            {
                dbgInfoLabel[i].name = UcLo_getString(strTable, dbgLbl[i].name);
                dbgInfoLabel[i].addr = dbgLbl[i].addr;
            }

            image->srcImage[image->numImage].numLabels = (unsigned short)objTab->numEntries;
            image->srcImage[image->numImage].label     = dbgInfoLabel;
        } 
        else 
        {
            image->srcImage[image->numImage].numLabels = 0;
            image->srcImage[image->numImage].label     = NULL;
        }

        image->numImage++;
    }
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_GetDebugInfo
   Description: Retrieve the udbug information from debug section of an uof.
   Returns:     UCLO_SUCCESS, UCLO_FAILURE, UCLO_BADARG, UCLO_NOOBJ
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_GetDebugInfo(void *handle, 
                  dbgAeInfo_Image_T *image)
{
    uclo_objHandle_T *objHandle = (uclo_objHandle_T *)handle;

    if(!objHandle || !objHandle->dbgObjHdr || !objHandle->dbgObjHdr->fBuf) 
    {
        return (UCLO_NOOBJ);
    }    
    return (UcLo_getDbgInfo((uof_objHdr_T *)objHandle->dbgObjHdr->fBuf,
                           image, &objHandle->dbgObjStrTable));
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_GetLocalSymbolInfo
   Description: Retrieve the storage information of a local symbol.
   Returns:     UCLO_SUCCESS, UCLO_FAILURE, UCLO_BADARG, UCLO_NOOBJ
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_GetLocalSymbolInfo(void *handle, 
                        unsigned int hwAe, 
                        char *symName, 
                        UcLo_SymbolInfo_T *symInfo)
{
    uof_initMem_T *initMem;
    char localName[MAX_VARNAME];
    uclo_objHandle_T *objHandle = (uclo_objHandle_T *)handle;

    if(!symInfo) 
    {
        return (UCLO_BADARG);
    }    
    if(!objHandle || !objHandle->objHdr->fBuf) 
    {
        return (UCLO_NOOBJ);
    }    

    LOCAL_NAME(localName, symName, hwAe);                /* format the local variable name */
    if(!(initMem = UcLo_findMemSym(objHandle, localName))) 
    {
        return (UCLO_SYMNOTFND);
    }    
    if(initMem->scope != LOCAL_SCOPE) 
    {
        return (UCLO_SYMNOTFND);
    }    

    symInfo->memType = initMem->region;
    symInfo->baseAddr = initMem->addr;
    symInfo->allocSize = initMem->numBytes;

    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_GetGlobalSymbolInfo
   Description: Retrieve the storage information of a global symbol.
   Returns:     UCLO_SUCCESS, UCLO_FAILURE, UCLO_BADARG, UCLO_NOOBJ
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_GetGlobalSymbolInfo(void *handle, 
                         char *symName, 
                         UcLo_SymbolInfo_T *symInfo)
{
    uof_initMem_T *initMem;
    uclo_objHandle_T *objHandle = (uclo_objHandle_T *)handle;

    if(!symInfo) 
    {
        return (UCLO_BADARG);
    }    
    if(!objHandle || !objHandle->objHdr->fBuf) 
    {
        return (UCLO_NOOBJ);
    }    

    if(!(initMem = UcLo_findMemSym(objHandle, symName))) 
    {
        return (UCLO_SYMNOTFND);
    }    
    if(initMem->scope != GLOBAL_SCOPE) 
    {
        return (UCLO_SYMNOTFND);
    }    

    symInfo->memType = initMem->region;
    symInfo->baseAddr = initMem->addr;
    symInfo->allocSize = initMem->numBytes;
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_GetAppMetaData
   Description: Retrieve the application meta-data asociated with the AE or any
                AE if hwAe is UCLO_BADAE.
   Returns:     appMetaData, or NULL
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
char *
UcLo_GetAppMetaData(void *handle, 
                    unsigned int hwAe)
{
    unsigned int swAe, appMetadata = 0xffffffff, s;
    int i;
    uclo_objHandle_T *objHandle = (uclo_objHandle_T *)handle;

    if(!objHandle || !objHandle->objHdr->fBuf) 
    {
        return (NULL);
    }    

    if(hwAe == UCLO_BADAE)
    {
        /* find any image that has a meta-data */
        for(i = 0; i < objHandle->numUniqueImage; i++)
        {
            if(objHandle->uniqueAeImage[i].imagePtr->appMetadata == 0xffffffff)
            {
                continue;
            }    
            appMetadata = objHandle->uniqueAeImage[i].imagePtr->appMetadata;
            break;
        }
    }
    else
    {
        if((swAe = UcLo_getSwAe(objHandle->prodType, (unsigned char)hwAe)) == UCLO_BADAE) 
        {
            return (NULL);
        }    
        for(s=0; s < objHandle->aeData[swAe].numSlices; s++)
        { 
            if((!objHandle->aeData[swAe].aeSlice[s].encapImage) || (!objHandle->aeData[swAe].aeSlice[s].encapImage->imagePtr)) 
            {
                continue;
            }    
            if((appMetadata = objHandle->aeData[swAe].aeSlice[s].encapImage->imagePtr->appMetadata) == 0xffffffff) 
            {
                continue;
            }    
        }
    }
    if(appMetadata == 0xffffffff) 
    {
        return (NULL);
    }    
    return (UcLo_getString(&objHandle->strTable,    appMetadata));
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_GetPhyUaddr
   Description: Retrieve the physical ustore address for the specified virtual
                ustore address.
   Returns:     UCLO_SUCCESS, UCLO_ADDRNOTFOUND, UCLO_BADARG, UCLO_NOOBJ
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_GetPhyUaddr(void *handle, 
                 unsigned int hwAe, 
                 unsigned int virtUaddr,
                 unsigned int*pageNum, 
                 unsigned int *phyUaddr)
{
    unsigned int swAe, p, s;
    uof_encapPage_T *encapPage;
    uclo_objHandle_T *objHandle = (uclo_objHandle_T *)handle;

    if((!pageNum) || (!phyUaddr)) 
    {
        return (UCLO_BADARG);
    }    
    if(!objHandle || !objHandle->objHdr->fBuf) 
    {
        return (UCLO_NOOBJ);
    }    

    if((swAe = UcLo_getSwAe(objHandle->prodType, (unsigned char)hwAe)) == UCLO_BADAE) 
    {
        return (UCLO_BADARG);
    }    
    for(s=0; s < objHandle->aeData[swAe].numSlices; s++)
    {
        if(!objHandle->aeData[swAe].aeSlice[s].encapImage) 
        {
            continue;
        }    
        for(p=0; p < objHandle->aeData[swAe].aeSlice[s].encapImage->imagePtr->numOfPages; p++)
        {
            encapPage = &objHandle->aeData[swAe].aeSlice[s].encapImage->pages[p];
            if((virtUaddr >= encapPage->begVirtAddr) && 
                (virtUaddr < (encapPage->begVirtAddr + encapPage->numMicroWords)))
                {
                    *pageNum = encapPage->pageNum;
                    *phyUaddr = (virtUaddr - encapPage->begVirtAddr) +  encapPage->begPhyAddr;
                    return (UCLO_SUCCESS);
            }
        }
    }

    return (UCLO_ADDRNOTFOUND);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_GetImportVal
   Description: Locate an import variable and return its initial value
   Returns:     UCLO_SUCCESS, UCLO_FAILURE, UCLO_NOOBJ, UCLO_BADARG
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_GetImportVal(void *handle, 
                  unsigned int hwAe, 
                  char *varName, 
                  uclo_ImportValue_T *importVal)
{
    uof_importVar_T *impVar;
    uclo_objHandle_T *objHandle = (uclo_objHandle_T *)handle;
    unsigned int swAe;

    if(!objHandle || !objHandle->objHdr->fBuf) 
    {
        return (UCLO_NOOBJ);
    }    
    if((!importVal) || (!varName)) 
    {
        return (UCLO_BADARG);
    }    
    if(((unsigned int)hwAe >= UCLO_MAX_AES) || ((1 << (unsigned int)hwAe) & BadHwAeMask) || (objHandle->swAe[hwAe] >= UOF_MAX_NUM_OF_AE)) 
    {
        return (UCLO_BADAE);
    }    

    if(!(impVar = UcLo_getImportVar(objHandle, objHandle->swAe[hwAe], varName)))
    {
        for(swAe = 0; (swAe < objHandle->numAEs) && (swAe < UOF_MAX_NUM_OF_AE); swAe++) 
        {
            if((impVar = UcLo_getImportVar(objHandle, (unsigned char)swAe, varName)) != NULL) 
            {
                break;
            }    
        }
    }
        
    if(!impVar) 
    {
        return (UCLO_SYMNOTFND);
    }    

    importVal->value = impVar->value;
    importVal->valueAttrs = impVar->valueAttrs;
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_Get_MENCBASE
   Description: Get the none-coherent dram base address
   Returns:    UCLO_SUCCESS
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
unsigned int 
UcLo_Get_MENCBASE(void) 
{ 
    unsigned int baseAddr;
    if(halAe_GetNCDramBaseAddr(&baseAddr) == HALAE_SUCCESS) 
    {
        return (baseAddr);
    }    
    else
    {
        return (0); 
    }    
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_SetMemoryStartOffset
   Description: Set the SRAM/NCDRAM/CDRAM allocated base address
   Returns:    UCLO_SUCCESS
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
int 
UcLo_SetMemoryStartOffset(unsigned int scratchOffset, 
                          unsigned int sramOffset, 
                          unsigned int ncdramOffset, 
                          unsigned int cdramOffset) 
{ 
    if(halAe_SetMemoryStartOffset(scratchOffset, sramOffset, ncdramOffset, cdramOffset)) 
    {
        return (UCLO_FAILURE);    
    }    
    return (UCLO_SUCCESS);
}

/*-----------------------------------------------------------------------------
   Function:    UcLo_GetErrorStr
   Description: Get the error string for provided error code
   Returns:    
   Uses:
   Modifies:
-----------------------------------------------------------------------------*/
char *
UcLo_GetErrorStr(int errCode)
{
    switch(errCode)
    {
    case UCLO_SUCCESS:
        return ("The operation is successful.");
    case UCLO_FAILURE:
        return ("The operation is failed.");
    case UCLO_FILEFAIL:
        return ("The file operation is failed.");        
    case UCLO_BADOBJ:
        return ("Bad object.");        
    case UCLO_MEMFAIL:
        return ("The memory operation is failed.");        
    case UCLO_BADARG:
        return ("Bad function argument passed.");        
    case UCLO_NOOBJ:
        return ("Bad object handler used.");        
    case UCLO_IMGNOTFND:
        return ("Can not find specified image.");        
    case UCLO_SYMNOTFND:
        return ("Can not find specified symbol.");        
    case UCLO_NEIGHNOTFND:
        return ("Can not find neighbouring Acceleration Engine.");        
    case UCLO_UOFINCOMPAT:
        return (".uof file is incompatible with loader.");        
    case UCLO_UOFVERINCOMPAT:
        return (".uof file version is incompatible with loader.");        
    case UCLO_UNINITVAR:
        return ("There is uninitialized import variables.");        
    case UCLO_EXPRFAIL:
        return ("Incorrect expression specified.");        
    case UCLO_EXCDDRAMSIZE:
        return ("Excede none-coherent dram size.");        
    case UCLO_EXCDDRAM1SIZE:
        return ("Excede coherent dram size.");        
    case UCLO_EXCDSRAM0SIZE:
        return ("Excede sram size.");        
    case UCLO_EXCDSCRTHSIZE:
        return ("Excede scratch size.");        
    case UCLO_EXCDLMEMSIZE:
        return ("Excede local memory size.");        
    case UCLO_INVLDCTX:
        return ("Invalid context mode.");        
    case UCLO_EXCDUMEMSIZE:
        return ("Excede micro-ustore size.");        
    case UCLO_ADDRNOTFOUND:
        return ("Can not find specified address.");        
    case UCLO_PAGENOTFOUND:
        return ("Can not find specified microcode page.");        
    case UCLO_IVDWARN:
        return ("Unknown image or symbol defined in IVD.");        
    case UCLO_EXCDSHRAMSIZE:
        return ("Excede shared ram size.");     
    default:
        return (NULL);
    }        
}
