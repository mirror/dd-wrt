/* 
 * @file:    IxQMgrHwQIf.c
 *
 * @author Intel Corporation
 * @date     26-Jan-2006
 *
 * @brief    This component provides a set of functions for
 * perfoming I/O on the HwQ hardware.
 * 
 * Design Notes: 
 *              These functions are intended to be as fast as possible
 * and as a result perform NO PARAMETER CHECKING.
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

/*
 * Inlines are compiled as function when this is defined.
 * N.B. Must be placed before #include of "IxQMgrHwQIf_p.h
 */
#ifndef IXQMGRHWQIF_P_H
#    define IXQMGRHWQIF_C
#else
#    error
#endif

/*
 * User defined include files.
 */
#include "IxOsal.h"
#include "IxQMgr_sp.h"
#include "IxQMgrHwQIfIxp400_p.h"
#include "IxQMgrLog_p.h"


/*
 * #defines and macros used in this file.
 */

/* These defines are the bit offsets of the various fields of
 * the queue configuration register
 */
#define IX_QMGR_Q_CONFIG_WRPTR_OFFSET       0x00
#define IX_QMGR_Q_CONFIG_RDPTR_OFFSET       0x07
#define IX_QMGR_Q_CONFIG_BADDR_OFFSET       0x0E
#define IX_QMGR_Q_CONFIG_ESIZE_OFFSET       0x16
#define IX_QMGR_Q_CONFIG_BSIZE_OFFSET       0x18
#define IX_QMGR_Q_CONFIG_NE_OFFSET          0x1A
#define IX_QMGR_Q_CONFIG_NF_OFFSET          0x1D

#define IX_QMGR_BASE_ADDR_16_WORD_ALIGN     0x40
#define IX_QMGR_BASE_ADDR_16_WORD_SHIFT     0x6

#define IX_QMGR_NE_NF_CLEAR_MASK            0x03FFFFFF
#define IX_QMGR_NE_MASK                     0x7
#define IX_QMGR_NF_MASK                     0x7
#define IX_QMGR_SIZE_MASK                   0x3
#define IX_QMGR_ENTRY_SIZE_MASK             0x3
#define IX_QMGR_BADDR_MASK                  0x003FC000
#define IX_QMGR_RDPTR_MASK                  0x7F
#define IX_QMGR_WRPTR_MASK                  0x7F
#define IX_QMGR_RDWRPTR_MASK                0x00003FFF

#define IX_QMGR_HWQ_ADDRESS_SPACE_SIZE_IN_WORDS 0x1000

/* Base address of HwQ SRAM */
#define IX_QMGR_HWQ_INTERNAL_SRAM_BASE_ADDRESS_OFFSET \
((IX_QMGR_QUECONFIG0_BASE_OFFSET) + (IX_QMGR_QUECONFIG_SIZE))

/* Min buffer size used for generating buffer size in QUECONFIG */
#define IX_QMGR_MIN_BUFFER_SIZE 16

/* Reset values of QMgr hardware registers */
#define IX_QMGR_QUELOWSTAT_RESET_VALUE    0x33333333
#define IX_QMGR_QUEUOSTAT_RESET_VALUE     0x00000000
#define IX_QMGR_QUEUPPSTAT0_RESET_VALUE   0xFFFFFFFF
#define IX_QMGR_QUEUPPSTAT1_RESET_VALUE   0x00000000
#define IX_QMGR_INT0SRCSELREG_RESET_VALUE 0x00000000
#define IX_QMGR_QUEIEREG_RESET_VALUE      0x00000000
#define IX_QMGR_QINTREG_RESET_VALUE       0xFFFFFFFF
#define IX_QMGR_QUECONFIG_RESET_VALUE     0x00000000

#define IX_QMGR_PHYSICAL_HWQ_BASE_ADDRESS IX_OSAL_IXP400_QMGR_PHYS_BASE

#define IX_QMGR_QUELOWSTAT_BITS_PER_Q (BITS_PER_WORD/IX_QMGR_QUELOWSTAT_NUM_QUE_PER_WORD)

#define IX_QMGR_QUELOWSTAT_QID_MASK 0x7

#define IX_QMGR_ENTRY1_OFFSET 0
#define IX_QMGR_ENTRY2_OFFSET 1
#define IX_QMGR_ENTRY4_OFFSET 3

/*
 * Variable declarations global to this file. Externs are followed by
 * statics.
 */
UINT32 hwQBaseAddress = 0;

/*
 * This flag indicates to the dispatcher that sticky interrupts are currently enabled.
 */
extern BOOL stickyEnabled;

/*
 * This flag is used to check whether the call is being made for Config or Reconfig
 * purpose 
 */
extern BOOL reCfgFlag;

/* Store addresses and bit-masks for certain queue access and status registers.
 * This is to facilitate inlining of QRead, QWrite and QStatusGet functions
 * in IxQMgr,h
 */
extern IxQMgrQInlinedReadWriteInfo ixQMgrQInlinedReadWriteInfo[];
UINT32 * ixQMgrHwQIfQueAccRegAddr[IX_QMGR_MAX_NUM_QUEUES];
UINT32 ixQMgrHwQIfQueLowStatRegAddr[IX_QMGR_NUM_QUEUES_PER_GROUP];
UINT32 ixQMgrHwQIfQueLowStatBitsOffset[IX_QMGR_NUM_QUEUES_PER_GROUP];
UINT32 ixQMgrHwQIfQueLowStatBitsMask;
UINT32 ixQMgrHwQIfQueUppStat0RegAddr;
UINT32 ixQMgrHwQIfQueUppStat1RegAddr;
UINT32 ixQMgrHwQIfQueUppStat0BitMask[IX_QMGR_NUM_QUEUES_PER_GROUP];
UINT32 ixQMgrHwQIfQueUppStat1BitMask[IX_QMGR_NUM_QUEUES_PER_GROUP];

/* 
 * Fast mutexes, one for each queue, used to protect peek & poke functions
 */
IxOsalFastMutex ixQMgrHwQIfPeekPokeFastMutex[IX_QMGR_MAX_NUM_QUEUES];

/*
 * Function prototypes
 */
PRIVATE unsigned
ixQMgrHwQIfWatermarkToHwQWatermarkConvert (IxQMgrWMLevel watermark );

PRIVATE unsigned
ixQMgrHwQIfEntrySizeToHwQEntrySizeConvert (IxQMgrQEntrySizeInWords entrySize);

PRIVATE unsigned
ixQMgrHwQIfBufferSizeToHwQBufferSizeConvert (unsigned bufferSizeInWords);

PRIVATE void
ixQMgrHwQIfRegistersReset (void);

PRIVATE void
ixQMgrHwQIfEntryAddressGet (unsigned int entryIndex,
			    UINT32 configRegWord,
			    unsigned int qEntrySizeInwords,
			    unsigned int qSizeInWords,
			    UINT32 **address);
/*
 * Function definitions
 */
void
ixQMgrHwQIfInit (void)
{
    UINT32 hwQVirtualAddr;
    int i;

    /* The value of hwQBaseAddress depends on the logical address
     * assigned by the MMU.
     */
    hwQVirtualAddr =
	(UINT32) IX_OSAL_MEM_MAP(IX_QMGR_PHYSICAL_HWQ_BASE_ADDRESS,
				    IX_OSAL_IXP400_QMGR_MAP_SIZE);
    IX_OSAL_ASSERT (hwQVirtualAddr);
    
    ixQMgrHwQIfBaseAddressSet (hwQVirtualAddr);

    ixQMgrHwQIfRegistersReset ();

    for (i = 0; i< IX_QMGR_MAX_NUM_QUEUES; i++)
    {
	ixOsalFastMutexInit(&ixQMgrHwQIfPeekPokeFastMutex[i]);

	/********************************************************************
	 * Register addresses and bit masks are calculated and stored here to
	 * facilitate inlining of QRead, QWrite and QStatusGet functions in
	 * IxQMgr.h.
	 * These calculations are normally performed dynamically in inlined
	 * functions in IxQMgrHwQIf_p.h, and their semantics are reused here.
	 */

	/* HwQ Queue access reg addresses, per queue */
   	ixQMgrHwQIfQueAccRegAddr[i] = 
	    (UINT32 *)(hwQBaseAddress + IX_QMGR_Q_ACCESS_ADDR_GET(i));
	ixQMgrQInlinedReadWriteInfo[i].qAccRegAddr = 
	    (volatile UINT32 *)(hwQBaseAddress + IX_QMGR_Q_ACCESS_ADDR_GET(i));


	ixQMgrQInlinedReadWriteInfo[i].qConfigRegAddr = 
	    (volatile UINT32 *)(hwQBaseAddress + IX_QMGR_Q_CONFIG_ADDR_GET(i));

	/* HwQ Queue lower-group (0-31), only */
	if (i < IX_QMGR_MIN_QUE_2ND_GROUP_QID)
	{
	    /* HwQ Q underflow/overflow status register addresses, per queue */
	    ixQMgrQInlinedReadWriteInfo[i].qUOStatRegAddr = 
		(volatile UINT32 *)(hwQBaseAddress +
		IX_QMGR_QUEUOSTAT0_OFFSET +
		((i / IX_QMGR_QUEUOSTAT_NUM_QUE_PER_WORD) *
		 IX_QMGR_NUM_BYTES_PER_WORD));

	    /* HwQ Q underflow status bit masks for status register per queue */
	    ixQMgrQInlinedReadWriteInfo[i].qUflowStatBitMask = 
		(IX_QMGR_UNDERFLOW_BIT_OFFSET + 1) <<
		((i & (IX_QMGR_QUEUOSTAT_NUM_QUE_PER_WORD - 1)) *
		 (BITS_PER_WORD / IX_QMGR_QUEUOSTAT_NUM_QUE_PER_WORD));

	    /* HwQ Q overflow status bit masks for status register, per queue */
	    ixQMgrQInlinedReadWriteInfo[i].qOflowStatBitMask = 
		(IX_QMGR_OVERFLOW_BIT_OFFSET + 1) <<
		((i & (IX_QMGR_QUEUOSTAT_NUM_QUE_PER_WORD - 1)) *
		 (BITS_PER_WORD / IX_QMGR_QUEUOSTAT_NUM_QUE_PER_WORD));

	    /* HwQ Q lower-group (0-31) status register addresses, per queue */
	    ixQMgrHwQIfQueLowStatRegAddr[i] = hwQBaseAddress +
		IX_QMGR_QUELOWSTAT0_OFFSET +
		((i / IX_QMGR_QUELOWSTAT_NUM_QUE_PER_WORD) *
		 IX_QMGR_NUM_BYTES_PER_WORD);

	    /* HwQ Q lower-group (0-31) status register bit offset */
	    ixQMgrHwQIfQueLowStatBitsOffset[i] =
		(i & (IX_QMGR_QUELOWSTAT_NUM_QUE_PER_WORD - 1)) * 
		(BITS_PER_WORD / IX_QMGR_QUELOWSTAT_NUM_QUE_PER_WORD);
	}
	else /* HwQ Q upper-group (32-63), only */
	{
	    /* HwQ Q upper-group (32-63) Nearly Empty status reg bit masks */
	    ixQMgrHwQIfQueUppStat0BitMask[i - IX_QMGR_MIN_QUE_2ND_GROUP_QID] =
		(1 << (i - IX_QMGR_MIN_QUE_2ND_GROUP_QID));

	    /* HwQ Q upper-group (32-63) Full status register bit masks */
	    ixQMgrHwQIfQueUppStat1BitMask[i - IX_QMGR_MIN_QUE_2ND_GROUP_QID] =
		(1 << (i - IX_QMGR_MIN_QUE_2ND_GROUP_QID));
	}
    }

    /* HwQ Q lower-group (0-31) status register bit mask */
    ixQMgrHwQIfQueLowStatBitsMask = (1 <<
				    (BITS_PER_WORD /
				     IX_QMGR_QUELOWSTAT_NUM_QUE_PER_WORD)) - 1;

    /* HwQ Q upper-group (32-63) Nearly Empty status register address */
    ixQMgrHwQIfQueUppStat0RegAddr = hwQBaseAddress + IX_QMGR_QUEUPPSTAT0_OFFSET;
    
    /* HwQ Q upper-group (32-63) Full status register address */
    ixQMgrHwQIfQueUppStat1RegAddr = hwQBaseAddress + IX_QMGR_QUEUPPSTAT1_OFFSET;
}

/*
 * Uninitialise the HwQIf module by unmapping memory, etc
 */
void
ixQMgrHwQIfUninit (void)
{
    UINT32 virtAddr;

    ixQMgrHwQIfBaseAddressGet (&virtAddr);
    IX_OSAL_MEM_UNMAP (virtAddr);
    ixQMgrHwQIfBaseAddressSet (0);
}

/*
 * Set the the logical base address of HwQ
 */
void
ixQMgrHwQIfBaseAddressSet (UINT32 address)
{
    hwQBaseAddress = address;
}

/*
 * Get the logical base address of HwQ
 */
void
ixQMgrHwQIfBaseAddressGet (UINT32 *address)
{
    *address = hwQBaseAddress;
}

/*
 * Get the logical base address of HwQ internal sram block
 */
void
ixQMgrHwQIfInternalSramBaseAddressGet (IxQMgrMemMapBlock block, UINT32 *address)
{
    *address = hwQBaseAddress                +
	IX_QMGR_HWQ_INTERNAL_SRAM_BASE_ADDRESS_OFFSET;
}

/*
 * This function will write the status bits of a queue
 * specified by qId.
 */
void
ixQMgrHwQIfQRegisterBitsWrite (IxQMgrQId qId, 
			       UINT32 registerBaseAddrOffset,
			       unsigned queuesPerRegWord,
			       UINT32 value)
{
    volatile UINT32 *registerAddress;
    UINT32 registerWord;
    UINT32 statusBitsMask;
    UINT32 bitsPerQueue;

    bitsPerQueue = BITS_PER_WORD / queuesPerRegWord;

    /*
     * Calculate the registerAddress
     * multiple queues split accross registers
     */
    registerAddress = (UINT32*)(hwQBaseAddress +
				registerBaseAddrOffset +
				((qId / queuesPerRegWord) *
				 IX_QMGR_NUM_BYTES_PER_WORD));    

    /* Read the current data */
    ixQMgrHwQIfWordRead (registerAddress, &registerWord);


    if( (registerBaseAddrOffset == IX_QMGR_INT0SRCSELREG0_OFFSET) &&
        (qId == IX_QMGR_QUEUE_0) )
    {
      statusBitsMask = 0x7 ;   

      /* Queue 0 at INT0SRCSELREG should not corrupt the value bit-3  */
      value &=  0x7 ;        
    }
    else
    {     
      /* Calculate the mask for the status bits for this queue. */
      statusBitsMask = ((1 << bitsPerQueue) - 1);
      statusBitsMask <<= ((qId & (queuesPerRegWord - 1)) * bitsPerQueue);

      /* Mask out bits in value that would overwrite other q data */
      value <<= ((qId & (queuesPerRegWord - 1)) * bitsPerQueue);
      value &= statusBitsMask;
    }

    /* Mask out bits to write to */
    registerWord &= ~statusBitsMask;
    

    /* Set the write bits */
    registerWord |= value;

    /*
     * Write the data
     */
    ixQMgrHwQIfWordWrite (registerAddress, registerWord);
}

/*
 * This function generates the parameters that can be used to
 * check if a Qs status matches the specified source select.
 * It calculates which status word to check (statusWordOffset),
 * the value to check the status against (checkValue) and the
 * mask (mask) to mask out all but the bits to check in the status word.
 */
void
ixQMgrHwQIfQStatusCheckValsCalc (IxQMgrQId qId,
				 IxQMgrSourceId srcSel,
				 unsigned int *statusWordOffset,
				 UINT32 *checkValue,
				 UINT32 *mask)
{
    UINT32 shiftVal;
   
    if (qId < IX_QMGR_MIN_QUE_2ND_GROUP_QID)
    {
	switch (srcSel)
	{
	    case IX_QMGR_Q_SOURCE_ID_E:
		*checkValue = IX_QMGR_Q_STATUS_E_BIT_MASK;
		*mask = IX_QMGR_Q_STATUS_E_BIT_MASK;
		break;
	    case IX_QMGR_Q_SOURCE_ID_NE:
		*checkValue = IX_QMGR_Q_STATUS_NE_BIT_MASK;
		*mask = IX_QMGR_Q_STATUS_NE_BIT_MASK;
		break;
	    case IX_QMGR_Q_SOURCE_ID_NF:
		*checkValue = IX_QMGR_Q_STATUS_NF_BIT_MASK;
		*mask = IX_QMGR_Q_STATUS_NF_BIT_MASK;
		break;
	    case IX_QMGR_Q_SOURCE_ID_F:
		*checkValue = IX_QMGR_Q_STATUS_F_BIT_MASK;
		*mask = IX_QMGR_Q_STATUS_F_BIT_MASK;
		break;
	    case IX_QMGR_Q_SOURCE_ID_NOT_E:
		*checkValue = 0;
		*mask = IX_QMGR_Q_STATUS_E_BIT_MASK;
		break;
	    case IX_QMGR_Q_SOURCE_ID_NOT_NE:
		*checkValue = 0;
		*mask = IX_QMGR_Q_STATUS_NE_BIT_MASK;
		break;
	    case IX_QMGR_Q_SOURCE_ID_NOT_NF:
		*checkValue = 0;
		*mask = IX_QMGR_Q_STATUS_NF_BIT_MASK;
		break;
	    case IX_QMGR_Q_SOURCE_ID_NOT_F:
		*checkValue = 0;
		*mask = IX_QMGR_Q_STATUS_F_BIT_MASK;
		break;
	    default:
		/* Should never hit */
		IX_OSAL_ASSERT(0);
		break;
	}

	/* One nibble of status per queue so need to shift the
	 * check value and mask out to the correct position.
	 */
	shiftVal = (qId % IX_QMGR_QUELOWSTAT_NUM_QUE_PER_WORD) * 
	    IX_QMGR_QUELOWSTAT_BITS_PER_Q;

	/* Calculate the which status word to check from the qId,
	 * 8 Qs status per word
	 */
	*statusWordOffset = qId / IX_QMGR_QUELOWSTAT_NUM_QUE_PER_WORD;

	*checkValue <<= shiftVal;
	*mask <<= shiftVal;
    }
    else
    {
	/* One status word */
	*statusWordOffset = 0;
	/* Single bits per queue and int source bit hardwired  NE,
	 * Qs start at 32.
	 */
	*mask = 1 << (qId - IX_QMGR_MIN_QUE_2ND_GROUP_QID);
	*checkValue = *mask;
    }
}

void
ixQMgrHwQIfQInterruptEnable (IxQMgrQId qId)
{
    volatile UINT32 *registerAddress;
    UINT32 registerWord;
    UINT32 actualBitOffset;
    
    if (qId < IX_QMGR_MIN_QUE_2ND_GROUP_QID)
    {    
	registerAddress = (UINT32*)(hwQBaseAddress + IX_QMGR_QUEIEREG0_OFFSET);
    }
    else
    {
	registerAddress = (UINT32*)(hwQBaseAddress + IX_QMGR_QUEIEREG1_OFFSET);
    }

    actualBitOffset = 1 << (qId % IX_QMGR_MIN_QUE_2ND_GROUP_QID);

    ixQMgrHwQIfWordRead (registerAddress, &registerWord);
    ixQMgrHwQIfWordWrite (registerAddress, (registerWord | actualBitOffset));
}

void
ixQMgrHwQIfQInterruptDisable (IxQMgrQId qId)
{
    volatile UINT32 *registerAddress;
    UINT32 registerWord;
    UINT32 actualBitOffset;

    if (qId < IX_QMGR_MIN_QUE_2ND_GROUP_QID)
    {    
	registerAddress = (UINT32*)(hwQBaseAddress + IX_QMGR_QUEIEREG0_OFFSET);
    }
    else
    {
	registerAddress = (UINT32*)(hwQBaseAddress + IX_QMGR_QUEIEREG1_OFFSET);
    }

    actualBitOffset = 1 << (qId % IX_QMGR_MIN_QUE_2ND_GROUP_QID);

    ixQMgrHwQIfWordRead (registerAddress, &registerWord);
    ixQMgrHwQIfWordWrite (registerAddress, registerWord & (~actualBitOffset));
}

void
ixQMgrHwQIfQueCfgWrite (IxQMgrQId qId,
		       IxQMgrQSizeInWords qSizeInWords,
		       IxQMgrQEntrySizeInWords entrySizeInWords,
		       UINT32 *freeSRAMAddress)
{
    volatile UINT32 *cfgAddress = NULL;
    UINT32 qCfg = 0;
    UINT32 baseAddress = 0;
    unsigned hwQEntrySize = 0;
    unsigned hwQBufferSize = 0;

    /* Build config register */
    hwQEntrySize = ixQMgrHwQIfEntrySizeToHwQEntrySizeConvert (entrySizeInWords);
    qCfg |= (hwQEntrySize&IX_QMGR_ENTRY_SIZE_MASK) <<
	IX_QMGR_Q_CONFIG_ESIZE_OFFSET;

    hwQBufferSize = ixQMgrHwQIfBufferSizeToHwQBufferSizeConvert (qSizeInWords);
    qCfg |= (hwQBufferSize&IX_QMGR_SIZE_MASK) << IX_QMGR_Q_CONFIG_BSIZE_OFFSET;

    /* baseAddress, calculated relative to hwQBaseAddress and start address  */
    if(reCfgFlag)
    {
        baseAddress = *freeSRAMAddress -
            (hwQBaseAddress + IX_QMGR_QUECONFIG0_BASE_OFFSET);
    }
    else
    {
        baseAddress = freeSRAMAddress[qId / IX_QMGR_NUM_QUEUES_PER_MEM_MAP_BLOCK] -
	    (hwQBaseAddress + IX_QMGR_QUECONFIG0_BASE_OFFSET);
    }

    /* Verify base address aligned to a 16 word boundary */
    if ((baseAddress % IX_QMGR_BASE_ADDR_16_WORD_ALIGN) != 0)
    {
	IX_QMGR_LOG_ERROR0("ixQMgrHwQIfQueCfgWrite () address is not on 16 word boundary\n");
    }
    /* Now convert it to a 16 word pointer as required by QUECONFIG register */
    baseAddress >>= IX_QMGR_BASE_ADDR_16_WORD_SHIFT;
    
    qCfg |= (baseAddress << IX_QMGR_Q_CONFIG_BADDR_OFFSET);

    cfgAddress = (UINT32*)(hwQBaseAddress +
			IX_QMGR_Q_CONFIG_ADDR_GET(qId));

    /* NOTE: High and Low watermarks are set to zero */
    ixQMgrHwQIfWordWrite (cfgAddress, qCfg);

    /* Reset the reconfig flag */
    reCfgFlag = FALSE;
}

void
ixQMgrHwQIfQueCfgRead (IxQMgrQId qId,
		       UINT32 numEntries,
		       UINT32 *baseAddress,
		       UINT32 *ne,
		       UINT32 *nf,
		       UINT32 *readPtr,
		       UINT32 *writePtr)
{
    UINT32 qcfg;
    UINT32 *cfgAddress = (UINT32*)(hwQBaseAddress + IX_QMGR_Q_CONFIG_ADDR_GET(qId));
    unsigned int qEntrySizeInwords;
    unsigned int qSizeInWords;
    UINT32 *readPtr_ = NULL;
	
    /* Read the queue configuration register */
    ixQMgrHwQIfWordRead (cfgAddress, &qcfg);
    
    /* Extract the base address */
    *baseAddress = (UINT32)((qcfg & IX_QMGR_BADDR_MASK) >>
			    (IX_QMGR_Q_CONFIG_BADDR_OFFSET));

    /* Base address is a 16 word pointer from the start of HwQ SRAM.
     * Convert to absolute word address.
     */
    *baseAddress <<= IX_QMGR_BASE_ADDR_16_WORD_SHIFT;
    *baseAddress += (UINT32)IX_QMGR_QUECONFIG0_BASE_OFFSET;

    /*
     * Extract the watermarks. 0->0 entries, 1->1 entries, 2->2 entries, 3->4 entries......
     * If ne > 0 ==> neInEntries = 2^(ne - 1)
     * If ne == 0 ==> neInEntries = 0
     * The same applies.
     */
    *ne = ((qcfg) >> (IX_QMGR_Q_CONFIG_NE_OFFSET)) & IX_QMGR_NE_MASK;
    *nf = ((qcfg) >> (IX_QMGR_Q_CONFIG_NF_OFFSET)) & IX_QMGR_NF_MASK;

    if (0 != *ne)
    {
	*ne = 1 << (*ne - 1);	
    }
    if (0 != *nf)
    {
	*nf = 1 << (*nf - 1);
    }

    /* Get the queue entry size in words */
    qEntrySizeInwords = ixQMgrQEntrySizeInWordsGet (qId);

    /* Get the queue size in words */
    qSizeInWords = ixQMgrQSizeInWordsGet (qId);

    ixQMgrHwQIfEntryAddressGet (0/* Entry 0. i.e the readPtr*/,
				qcfg,
				qEntrySizeInwords,
				qSizeInWords,
				&readPtr_);
    *readPtr = (UINT32)readPtr_;
    *readPtr -= (UINT32)hwQBaseAddress;/* Offset, not absolute address */

    *writePtr = (qcfg >> IX_QMGR_Q_CONFIG_WRPTR_OFFSET) & IX_QMGR_WRPTR_MASK;
    *writePtr = *baseAddress + (*writePtr * (IX_QMGR_NUM_BYTES_PER_WORD));
    return;
}

unsigned
ixQMgrHwQIfLog2 (unsigned number)
{
    unsigned count = 0;

    /*
     * N.B. this function will return 0
     * for ixQMgrHwQIfLog2 (0)
     */
    while (number/2)
    {
	number /=2;
	count++;	
    }

    return count;
}

void ixQMgrHwQIfIntSrcSelReg0Bit3Set (void)
{

    volatile UINT32 *registerAddress;
    UINT32 registerWord; 

    /*
     * Calculate the registerAddress
     * multiple queues split accross registers
     */
    registerAddress = (UINT32*)(hwQBaseAddress +
				IX_QMGR_INT0SRCSELREG0_OFFSET);    

    /* Read the current data */
    ixQMgrHwQIfWordRead (registerAddress, &registerWord);

    /* Set the write bits */
    registerWord |= (1<<IX_QMGR_INT0SRCSELREG0_BIT3) ;

    /*
     * Write the data
     */
    ixQMgrHwQIfWordWrite (registerAddress, registerWord);
}  


void
ixQMgrHwQIfIntSrcSelWrite (IxQMgrQId qId,
			  IxQMgrSourceId sourceId)
{
    ixQMgrHwQIfQRegisterBitsWrite (qId,
				   IX_QMGR_INT0SRCSELREG0_OFFSET,
				   IX_QMGR_INTSRC_NUM_QUE_PER_WORD,
				   sourceId);
}



void
ixQMgrHwQIfWatermarkSet (IxQMgrQId qId,
			unsigned ne,
			unsigned nf)
{
    volatile UINT32 *address = 0;
    UINT32 value = 0;
    unsigned hwQNeWatermark = 0;
    unsigned hwQNfWatermark = 0;

    address = (UINT32*)(hwQBaseAddress +
		     IX_QMGR_Q_CONFIG_ADDR_GET(qId));

    hwQNeWatermark = ixQMgrHwQIfWatermarkToHwQWatermarkConvert (ne);
    hwQNfWatermark = ixQMgrHwQIfWatermarkToHwQWatermarkConvert (nf);

    /* Read the current watermarks */
    ixQMgrHwQIfWordRead (address, &value);

    /* Clear out the old watermarks */
    value &=  IX_QMGR_NE_NF_CLEAR_MASK;
    
    /* Generate the value to write */
    value |= (hwQNeWatermark << IX_QMGR_Q_CONFIG_NE_OFFSET) |
	(hwQNfWatermark << IX_QMGR_Q_CONFIG_NF_OFFSET); 

    ixQMgrHwQIfWordWrite (address, value);

}

PRIVATE void
ixQMgrHwQIfEntryAddressGet (unsigned int entryIndex,
			    UINT32 configRegWord,
			    unsigned int qEntrySizeInwords,
			    unsigned int qSizeInWords,
			    UINT32 **address)
{
    UINT32 readPtr;
    UINT32 baseAddress;
    UINT32 *topOfHwQSram;

    topOfHwQSram = ((UINT32 *)hwQBaseAddress + IX_QMGR_HWQ_ADDRESS_SPACE_SIZE_IN_WORDS);

    /* Extract the base address */
    baseAddress = (UINT32)((configRegWord & IX_QMGR_BADDR_MASK) >>
			   (IX_QMGR_Q_CONFIG_BADDR_OFFSET));

    /* Base address is a 16 word pointer from the start of HwQ SRAM.
     * Convert to absolute word address.
     */
    baseAddress <<= IX_QMGR_BASE_ADDR_16_WORD_SHIFT;
    baseAddress += ((UINT32)hwQBaseAddress + (UINT32)IX_QMGR_QUECONFIG0_BASE_OFFSET);

    /* Extract the read pointer. Read pointer is a word pointer */
    readPtr = (UINT32)((configRegWord >>
			IX_QMGR_Q_CONFIG_RDPTR_OFFSET)&IX_QMGR_RDPTR_MASK);

    /* Read/Write pointers(word pointers)  are offsets from the queue buffer space base address.
     * Calculate the absolute read pointer address. NOTE: Queues are circular buffers.
     */
    readPtr  = (readPtr + (entryIndex * qEntrySizeInwords)) & (qSizeInWords - 1); /* Mask by queue size */
    *address = (UINT32 *)(baseAddress + (readPtr * (IX_QMGR_NUM_BYTES_PER_WORD)));

    switch (qEntrySizeInwords)
    {
	case IX_QMGR_Q_ENTRY_SIZE1:
	    IX_OSAL_ASSERT((*address + IX_QMGR_ENTRY1_OFFSET) < topOfHwQSram);	    
	    break;
	case IX_QMGR_Q_ENTRY_SIZE2:
	    IX_OSAL_ASSERT((*address + IX_QMGR_ENTRY2_OFFSET) < topOfHwQSram);
	    break;
	case IX_QMGR_Q_ENTRY_SIZE4:
	    IX_OSAL_ASSERT((*address + IX_QMGR_ENTRY4_OFFSET) < topOfHwQSram);
	    break;
	default:
	    IX_QMGR_LOG_ERROR0("Invalid Q Entry size passed to ixQMgrHwQIfEntryAddressGet");
	    break;
    }
    
}

IX_STATUS
ixQMgrHwQIfQPeek (IxQMgrQId qId,
		  unsigned int entryIndex,
		  unsigned int *entry)
{
    UINT32 *cfgRegAddress = (UINT32*)(hwQBaseAddress + IX_QMGR_Q_CONFIG_ADDR_GET(qId));
    UINT32 *entryAddress = NULL;
    UINT32 configRegWordOnEntry;
    UINT32 configRegWordOnExit;
    unsigned int qEntrySizeInwords;
    unsigned int qSizeInWords;

    /* Get the queue entry size in words */
    qEntrySizeInwords = ixQMgrQEntrySizeInWordsGet (qId);

    /* Get the queue size in words */
    qSizeInWords = ixQMgrQSizeInWordsGet (qId);

    /* Read the config register */
    ixQMgrHwQIfWordRead (cfgRegAddress, &configRegWordOnEntry);

    /* Get the entry address */
    ixQMgrHwQIfEntryAddressGet (entryIndex,
				configRegWordOnEntry,
				qEntrySizeInwords,
				qSizeInWords,
				&entryAddress);

    /* Get the lock or return busy */
    if (IX_SUCCESS != ixOsalFastMutexTryLock(&ixQMgrHwQIfPeekPokeFastMutex[qId]))
    {
	return IX_FAIL;
    }

    while(qEntrySizeInwords--)
    {
	ixQMgrHwQIfWordRead (entryAddress++, entry++);
    }

    /* Release the lock */
    ixOsalFastMutexUnlock(&ixQMgrHwQIfPeekPokeFastMutex[qId]);

    /* Read the config register */
    ixQMgrHwQIfWordRead (cfgRegAddress, &configRegWordOnExit);

    /* Check that the read and write pointers have not changed */
    if (configRegWordOnEntry != configRegWordOnExit)
    {
	return IX_FAIL;
    }

    return IX_SUCCESS;
}

IX_STATUS
ixQMgrHwQIfQPoke (IxQMgrQId qId,
		  unsigned entryIndex,
		  unsigned int *entry)
{
    UINT32 *cfgRegAddress = (UINT32*)(hwQBaseAddress + IX_QMGR_Q_CONFIG_ADDR_GET(qId));
    UINT32 *entryAddress = NULL;
    UINT32 configRegWordOnEntry;
    UINT32 configRegWordOnExit;
    unsigned int qEntrySizeInwords;
    unsigned int qSizeInWords;
    
    /* Get the queue entry size in words */
    qEntrySizeInwords = ixQMgrQEntrySizeInWordsGet (qId);

    /* Get the queue size in words */
    qSizeInWords = ixQMgrQSizeInWordsGet (qId);

    /* Read the config register */
    ixQMgrHwQIfWordRead (cfgRegAddress, &configRegWordOnEntry);

    /* Get the entry address */
    ixQMgrHwQIfEntryAddressGet (entryIndex,
				configRegWordOnEntry,
				qEntrySizeInwords,
				qSizeInWords,
				&entryAddress);

    /* Get the lock or return busy */
    if (IX_SUCCESS != ixOsalFastMutexTryLock(&ixQMgrHwQIfPeekPokeFastMutex[qId]))
    {
	return IX_FAIL;
    }

    /* Else read the entry directly from SRAM. This will not move the read pointer */
    while(qEntrySizeInwords--)
    {
	ixQMgrHwQIfWordWrite (entryAddress++, *entry++);
    }

    /* Release the lock */
    ixOsalFastMutexUnlock(&ixQMgrHwQIfPeekPokeFastMutex[qId]);

    /* Read the config register */
    ixQMgrHwQIfWordRead (cfgRegAddress, &configRegWordOnExit);

    /* Check that the read and write pointers have not changed */
    if (configRegWordOnEntry != configRegWordOnExit)
    {
	return IX_FAIL;
    }

    return IX_SUCCESS;
}

PRIVATE unsigned
ixQMgrHwQIfWatermarkToHwQWatermarkConvert (IxQMgrWMLevel watermark )
{
    unsigned hwQWatermark = 0;

    /*
     * Watermarks 0("000"),1("001"),2("010"),4("011"),
     * 8("100"),16("101"),32("110"),64("111")
     */
    hwQWatermark = ixQMgrHwQIfLog2 (watermark * 2);
    
    return hwQWatermark;
}

PRIVATE unsigned
ixQMgrHwQIfEntrySizeToHwQEntrySizeConvert (IxQMgrQEntrySizeInWords entrySize)
{
    /* entrySize  1("00"),2("01"),4("10") */
    return (ixQMgrHwQIfLog2 (entrySize));
}

PRIVATE unsigned
ixQMgrHwQIfBufferSizeToHwQBufferSizeConvert (unsigned bufferSizeInWords)
{
    /* bufferSize 16("00"),32("01),64("10"),128("11") */
    return (ixQMgrHwQIfLog2 (bufferSizeInWords / IX_QMGR_MIN_BUFFER_SIZE));
}

/*
 * Reset HwQ registers to default values.
 */
PRIVATE void
ixQMgrHwQIfRegistersReset (void)
{
    volatile UINT32 *qConfigWordAddress = NULL;
    unsigned int i;

    /*
     * Need to initialize HwQ hardware registers to an initial
     * value as init may have been called as a result of a soft
     * reset. i.e. soft reset does not reset hardware registers.
     */

    /* Reset queues 0..31 status registers 0..3 */
    ixQMgrHwQIfWordWrite((UINT32 *)(hwQBaseAddress + IX_QMGR_QUELOWSTAT0_OFFSET), 
			 IX_QMGR_QUELOWSTAT_RESET_VALUE);
    ixQMgrHwQIfWordWrite((UINT32 *)(hwQBaseAddress + IX_QMGR_QUELOWSTAT1_OFFSET), 
			 IX_QMGR_QUELOWSTAT_RESET_VALUE);
    ixQMgrHwQIfWordWrite((UINT32 *)(hwQBaseAddress + IX_QMGR_QUELOWSTAT2_OFFSET), 
			 IX_QMGR_QUELOWSTAT_RESET_VALUE);
    ixQMgrHwQIfWordWrite((UINT32 *)(hwQBaseAddress + IX_QMGR_QUELOWSTAT3_OFFSET), 
			 IX_QMGR_QUELOWSTAT_RESET_VALUE);

    /* Reset underflow/overflow status registers 0..1 */
    ixQMgrHwQIfWordWrite((UINT32 *)(hwQBaseAddress + IX_QMGR_QUEUOSTAT0_OFFSET), 
			 IX_QMGR_QUEUOSTAT_RESET_VALUE);
    ixQMgrHwQIfWordWrite((UINT32 *)(hwQBaseAddress + IX_QMGR_QUEUOSTAT1_OFFSET), 
			 IX_QMGR_QUEUOSTAT_RESET_VALUE);
    
    /* Reset queues 32..63 nearly empty status registers */
    ixQMgrHwQIfWordWrite((UINT32 *)(hwQBaseAddress + IX_QMGR_QUEUPPSTAT0_OFFSET),
			 IX_QMGR_QUEUPPSTAT0_RESET_VALUE);

    /* Reset queues 32..63 full status registers */
    ixQMgrHwQIfWordWrite((UINT32 *)(hwQBaseAddress + IX_QMGR_QUEUPPSTAT1_OFFSET),
			 IX_QMGR_QUEUPPSTAT1_RESET_VALUE);

    /* since we are resetting the INT0SRCSELREG0, sticky interrupts are disabled */
    stickyEnabled = FALSE;

    /* Reset int0 status flag source select registers 0..3 */
    ixQMgrHwQIfWordWrite((UINT32 *)(hwQBaseAddress + IX_QMGR_INT0SRCSELREG0_OFFSET),
			 IX_QMGR_INT0SRCSELREG_RESET_VALUE);
    ixQMgrHwQIfWordWrite((UINT32 *)(hwQBaseAddress + IX_QMGR_INT0SRCSELREG1_OFFSET),
			 IX_QMGR_INT0SRCSELREG_RESET_VALUE);
    ixQMgrHwQIfWordWrite((UINT32 *)(hwQBaseAddress + IX_QMGR_INT0SRCSELREG2_OFFSET),
			 IX_QMGR_INT0SRCSELREG_RESET_VALUE);
    ixQMgrHwQIfWordWrite((UINT32 *)(hwQBaseAddress + IX_QMGR_INT0SRCSELREG3_OFFSET),
			 IX_QMGR_INT0SRCSELREG_RESET_VALUE);
	 
    /* Reset queue interrupt enable register 0..1 */
    ixQMgrHwQIfWordWrite((UINT32 *)(hwQBaseAddress + IX_QMGR_QUEIEREG0_OFFSET),
			 IX_QMGR_QUEIEREG_RESET_VALUE);
    ixQMgrHwQIfWordWrite((UINT32 *)(hwQBaseAddress + IX_QMGR_QUEIEREG1_OFFSET),
			 IX_QMGR_QUEIEREG_RESET_VALUE);

    /* Reset queue interrupt register 0..1 */
    ixQMgrHwQIfWordWrite((UINT32 *)(hwQBaseAddress + IX_QMGR_QINTREG0_OFFSET),
			 IX_QMGR_QINTREG_RESET_VALUE);
    ixQMgrHwQIfWordWrite((UINT32 *)(hwQBaseAddress + IX_QMGR_QINTREG1_OFFSET),
			 IX_QMGR_QINTREG_RESET_VALUE);

    /* Reset queue configuration words 0..63 */
    qConfigWordAddress = (UINT32 *)(hwQBaseAddress + IX_QMGR_QUECONFIG0_BASE_OFFSET);
    for (i = 0; i < (IX_QMGR_QUECONFIG_SIZE / sizeof(UINT32)); i++)
    {
	ixQMgrHwQIfWordWrite(qConfigWordAddress,
			     IX_QMGR_QUECONFIG_RESET_VALUE);
	/* Next word */
	qConfigWordAddress++;
    }
}

