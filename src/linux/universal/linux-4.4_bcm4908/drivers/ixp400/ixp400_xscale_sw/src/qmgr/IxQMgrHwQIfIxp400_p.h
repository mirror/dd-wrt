/**
 * @file    IxQMgrHwQIf_p.h
 *
 * @author Intel Corporation
 * @date    26-Jan-2006
 *
 * @brief   The IxQMgrHwQIf sub-component provides a number of inline
 * functions for performing I/O on the HWQ. 
 *
 * Because  some functions contained in this module are inline and are
 * used in other modules (within the QMgr component) the definitions are
 * contained in this header file. The "normal" use of inline functions
 * is to use the inline functions in the module in which they are
 * defined. In this case these inline functions are used in external
 * modules and therefore the use of "inline extern". What this means
 * is as follows: if a function foo is declared as "inline extern"this
 * definition is only used for inlining, in no case is the function
 * compiled on its own. If the compiler cannot inline the function it
 * becomes an external reference. Therefore in IxQMgrHwQIf.c all
 * inline functions are defined without the "inline extern" specifier
 * and so define the external references. In all other modules these
 * funtions are defined as "inline extern".
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

#ifndef IXQMGRHWQIF_P_H
#define IXQMGRHWQIF_P_H

#include "IxOsal.h"

/*
 * inline definition
 */
 
#ifdef IX_OSAL_INLINE_ALL
/* If IX_OSAL_INLINE_ALL is set then each inlineable API functions will be defined as
   inline functions */
#   ifdef _DIAB_TOOL
#       define IX_QMGR_HWQIF_INLINE IX_OSAL_INLINE
#   else
#       define IX_QMGR_HWQIF_INLINE IX_OSAL_INLINE_EXTERN
#   endif
#else   
#ifdef IXQMGRHWQIF_C
#ifndef IX_QMGR_HWQIF_INLINE
#define IX_QMGR_HWQIF_INLINE
#endif
#else  
#ifndef IX_QMGR_HWQIF_INLINE
#   ifdef _DIAB_TOOL
        /* DIAB does not allow both the funtion prototype and
         * defintion to use extern
         */
#       define IX_QMGR_HWQIF_INLINE IX_OSAL_INLINE
#   else
#       define IX_QMGR_HWQIF_INLINE IX_OSAL_INLINE_EXTERN
#   endif
#endif
#endif /* IXQMGRHWQIF_C */
#endif /* IX_OSAL_INLINE */


/*
 * User defined include files.
 */
#include "IxQMgr_sp.h"
#include "IxQMgrLog_p.h"
#include "IxQMgrQCfg_p.h"

/* Because this file contains inline functions which will be compiled into
 * other components, we need to ensure that the IX_COMPONENT_NAME define
 * is set to ix_qmgr while this code is being compiled.  This will ensure
 * that the correct implementation is provided for the memory access macros
 * IX_OSAL_READ_LONG and IX_OSAL_WRITE_LONG which are used in this file.
 * This must be done before including "IxOsalMemAccess.h"
 */
#define IX_QMGR_HWQIF_SAVED_COMPONENT_NAME IX_COMPONENT_NAME
#undef  IX_COMPONENT_NAME
#define IX_COMPONENT_NAME ix_qmgr
#include "IxOsal.h" 

/*
 * #defines and macros used in this file.
 */

/* Number of bytes per word */
#define IX_QMGR_NUM_BYTES_PER_WORD 4

/* Size of memory map block in words */
#define IX_QMGR_MEM_MAP_BLOCK_SIZE 0x10000
 
/* Underflow bit mask  */
#define IX_QMGR_UNDERFLOW_BIT_OFFSET    0x0

/* Overflow bit mask */
#define IX_QMGR_OVERFLOW_BIT_OFFSET     0x1

/* Queue access register, queue 0 */
#define IX_QMGR_QUEACC0_OFFSET      0x0000

/* Size of queue access register in words */
#define IX_QMGR_QUEACC_SIZE         0x4/*words*/

/* Queue status register, queues 0-7 */
#define IX_QMGR_QUELOWSTAT0_OFFSET  (IX_QMGR_QUEACC0_OFFSET +\
(IX_QMGR_MAX_NUM_QUEUES * IX_QMGR_QUEACC_SIZE * IX_QMGR_NUM_BYTES_PER_WORD))

/* Queue status register, queues 8-15 */
#define IX_QMGR_QUELOWSTAT1_OFFSET  (IX_QMGR_QUELOWSTAT0_OFFSET +\
                                     IX_QMGR_NUM_BYTES_PER_WORD)

/* Queue status register, queues 16-23 */
#define IX_QMGR_QUELOWSTAT2_OFFSET  (IX_QMGR_QUELOWSTAT1_OFFSET +\
                                     IX_QMGR_NUM_BYTES_PER_WORD)

/* Queue status register, queues 24-31 */
#define IX_QMGR_QUELOWSTAT3_OFFSET  (IX_QMGR_QUELOWSTAT2_OFFSET +\
                                     IX_QMGR_NUM_BYTES_PER_WORD)

/* Queue status register Q status bits mask */
#define IX_QMGR_QUELOWSTAT_QUE_STS_BITS_MASK 0xF

/* Size of queue 0-31 status register */
#define IX_QMGR_QUELOWSTAT_SIZE     0x4 /*words*/

/* The number of queues' status specified per word */
#define IX_QMGR_QUELOWSTAT_NUM_QUE_PER_WORD 0x8

/* Queue UF/OF status register queues 0-15  */
#define IX_QMGR_QUEUOSTAT0_OFFSET   (IX_QMGR_QUELOWSTAT3_OFFSET +\
                                     IX_QMGR_NUM_BYTES_PER_WORD)
/* Queue UF/OF status register queues 16-31 */
#define IX_QMGR_QUEUOSTAT1_OFFSET   (IX_QMGR_QUEUOSTAT0_OFFSET +\
                                     IX_QMGR_NUM_BYTES_PER_WORD)

/* The number of queues' underflow/overflow status specified per word */
#define IX_QMGR_QUEUOSTAT_NUM_QUE_PER_WORD 0x10

/* Queue NE status register, queues 32-63   */
#define IX_QMGR_QUEUPPSTAT0_OFFSET  (IX_QMGR_QUEUOSTAT1_OFFSET +\
                                     IX_QMGR_NUM_BYTES_PER_WORD)

/* Queue F status register, queues 32-63    */
#define IX_QMGR_QUEUPPSTAT1_OFFSET  (IX_QMGR_QUEUPPSTAT0_OFFSET +\
                                     IX_QMGR_NUM_BYTES_PER_WORD)

/* Size of queue 32-63 status register */
#define IX_QMGR_QUEUPPSTAT_SIZE     0x2 /*words*/

/* The number of queues' status specified per word */
#define IX_QMGR_QUEUPPSTAT_NUM_QUE_PER_WORD 0x20

/* Queue INT source select register, queues 0-7   */
#define IX_QMGR_INT0SRCSELREG0_OFFSET (IX_QMGR_QUEUPPSTAT1_OFFSET   +\
                                       IX_QMGR_NUM_BYTES_PER_WORD)

/* Queue INT source select register, queues 8-15  */
#define IX_QMGR_INT0SRCSELREG1_OFFSET (IX_QMGR_INT0SRCSELREG0_OFFSET+\
                                       IX_QMGR_NUM_BYTES_PER_WORD)

/* Queue INT source select register, queues 16-23 */
#define IX_QMGR_INT0SRCSELREG2_OFFSET (IX_QMGR_INT0SRCSELREG1_OFFSET+\
                                       IX_QMGR_NUM_BYTES_PER_WORD)

/* Queue INT source select register, queues 24-31 */
#define IX_QMGR_INT0SRCSELREG3_OFFSET (IX_QMGR_INT0SRCSELREG2_OFFSET+\
                                       IX_QMGR_NUM_BYTES_PER_WORD)

/* Size of interrupt source select reegister */
#define IX_QMGR_INT0SRCSELREG_SIZE  0x4 /*words*/

/* The number of queues' interrupt source select specified per word*/
#define IX_QMGR_INTSRC_NUM_QUE_PER_WORD 0x8

/* Queue INT enable register, queues 0-31  */
#define IX_QMGR_QUEIEREG0_OFFSET    (IX_QMGR_INT0SRCSELREG3_OFFSET +\
                                     IX_QMGR_NUM_BYTES_PER_WORD)

/* Queue INT enable register, queues 32-63 */
#define IX_QMGR_QUEIEREG1_OFFSET    (IX_QMGR_QUEIEREG0_OFFSET      +\
                                     IX_QMGR_NUM_BYTES_PER_WORD)

/* Queue INT register, queues 0-31  */
#define IX_QMGR_QINTREG0_OFFSET     (IX_QMGR_QUEIEREG1_OFFSET +\
                                     IX_QMGR_NUM_BYTES_PER_WORD)

/* Queue INT register, queues 32-63 */
#define IX_QMGR_QINTREG1_OFFSET     (IX_QMGR_QINTREG0_OFFSET  +\
                                     IX_QMGR_NUM_BYTES_PER_WORD)

/* Size of interrupt register */
#define IX_QMGR_QINTREG_SIZE        0x2 /*words*/

/* Number of queues' status specified per word */
#define IX_QMGR_QINTREG_NUM_QUE_PER_WORD 0x20

/* Number of bits per queue interrupt status */
#define IX_QMGR_QINTREG_BITS_PER_QUEUE 0x1
#define IX_QMGR_QINTREG_BIT_OFFSET 0x1

/* Size of address space not used by HwQ */
#define IX_QMGR_HWQ_UNUSED_ADDRESS_SPACE_SIZE_IN_BYTES  0x1bC0

/* Queue config register, queue 0 */
#define IX_QMGR_QUECONFIG0_BASE_OFFSET (IX_QMGR_QINTREG1_OFFSET +\
                             IX_QMGR_NUM_BYTES_PER_WORD +\
                             IX_QMGR_HWQ_UNUSED_ADDRESS_SPACE_SIZE_IN_BYTES)			   

/* Total size of configuration words */
#define IX_QMGR_QUECONFIG_SIZE      0x100

/* Start of queue buffer space */
#define IX_QMGR_QUEBUFFER0_SPACE_OFFSET (IX_QMGR_QUECONFIG0_BASE_OFFSET +\
                                 IX_QMGR_MAX_NUM_QUEUES * IX_QMGR_NUM_BYTES_PER_WORD)

/* Total bits in a word */
#define BITS_PER_WORD 32

/* Size of queue buffer space */
#define IX_QMGR_QUE_BUFFER_SPACE_SIZE 0x1F00

/*
 * This macro will return the address of the access register for the
 * queue  specified by qId
 */
#define IX_QMGR_Q_ACCESS_ADDR_GET(qId)\
        (((qId) * (IX_QMGR_QUEACC_SIZE * IX_QMGR_NUM_BYTES_PER_WORD))\
	 + IX_QMGR_QUEACC0_OFFSET)

/*
 * This macro will return the address of the configuration register for the
 * queue  specified by qId
 */	 
#define IX_QMGR_Q_CONFIG_ADDR_GET(qId)\
        (((qId) * IX_QMGR_NUM_BYTES_PER_WORD) +\
                  IX_QMGR_QUECONFIG0_BASE_OFFSET)

/* 
 * Bit location of bit-3 of INT0SRCSELREG0 register to enabled
 * sticky interrupt register.
 */
#define IX_QMGR_INT0SRCSELREG0_BIT3 3

/*
 * Number of queues in a group
 */
#define IX_QMGR_NUM_QUEUES_PER_GROUP 32

/*
 * Number of queues maintained in a qmgr memory map block
 */
#define IX_QMGR_NUM_QUEUES_PER_MEM_MAP_BLOCK 64

/*
 * Variable declerations global to this file. Externs are followed by
 * statics.
 */
extern UINT32 hwQBaseAddress;

/*
 * Function declarations.
 */
void
ixQMgrHwQIfInit (void);

void
ixQMgrHwQIfUninit (void);

unsigned
ixQMgrHwQIfLog2 (unsigned number);

void
ixQMgrHwQIfQRegisterBitsWrite (IxQMgrQId qId, 
			       UINT32 registerBaseAddrOffset,
			       unsigned queuesPerRegWord,
			       UINT32 value);

void
ixQMgrHwQIfQStatusCheckValsCalc (IxQMgrQId qId,
				 IxQMgrSourceId srcSel,
				 unsigned int *statusWordOffset,
				 UINT32 *checkValue,
				 UINT32 *mask);
/*
 * The Xscale software allways deals with logical addresses and so the
 * base address of the HwQ memory space is not a hardcoded value. This
 * function must be called before any other function in this component.
 * NO CHECKING is performed to ensure that the base address has been
 * set.
 */
void
ixQMgrHwQIfBaseAddressSet (UINT32 address);

/*
 * Get the base address of the HwQ memory space.
 */
void
ixQMgrHwQIfBaseAddressGet (UINT32 *address);

/*
 * Get the base address of internal memory for a particular block. 
 * Note: In IXP4XX, hardware contains just one memory map block
 */
void
ixQMgrHwQIfInternalSramBaseAddressGet (IxQMgrMemMapBlock block, UINT32 *address);

/*
 * Read a queue status
 */
void
ixQMgrHwQIfQueStatRead (IxQMgrQId qId,
			IxQMgrQStatus* status);


/*
 *   Set INT0SRCSELREG0 Bit3 
 */ 
void ixQMgrHwQIfIntSrcSelReg0Bit3Set (void);


/*
 * Set the interrupt source
 */
void
ixQMgrHwQIfIntSrcSelWrite (IxQMgrQId qId,
			   IxQMgrSourceId sourceId);

/*
 * Enable interruptson a queue
 */
void
ixQMgrHwQIfQInterruptEnable (IxQMgrQId qId);

/*
 * Disable interrupt on a quee
 */
void
ixQMgrHwQIfQInterruptDisable (IxQMgrQId qId);

/*
 * Write the config register of the specified queue
 */
void
ixQMgrHwQIfQueCfgWrite (IxQMgrQId qId,
			IxQMgrQSizeInWords qSizeInWords,
			IxQMgrQEntrySizeInWords entrySizeInWords,
			UINT32 *freeSRAMAddress);

/*
 * read fields from the config of the specified queue.
 */
void
ixQMgrHwQIfQueCfgRead (IxQMgrQId qId,
		       UINT32 numEntries,
		       UINT32 *baseAddress,
		       UINT32 *ne,
		       UINT32 *nf,
		       UINT32 *readPtr,
		       UINT32 *writePtr);

/*
 * Set the ne and nf watermark level on a queue.
 */
void
ixQMgrHwQIfWatermarkSet (IxQMgrQId qId,
			 unsigned ne,
			 unsigned nf);

/* Inspect an entry without moving the read pointer */
IX_STATUS
ixQMgrHwQIfQPeek (IxQMgrQId qId,
		  unsigned int entryIndex,
		  unsigned int *entry);

/* Modify an entry without moving the write pointer */
IX_STATUS
ixQMgrHwQIfQPoke (IxQMgrQId qId,
		  unsigned int entryIndex,
		  unsigned int *entry);

/*
 * Function prototype for inline functions. For description refers to 
 * the functions defintion below.
 */
IX_QMGR_HWQIF_INLINE void
ixQMgrHwQIfWordWrite (VUINT32 *address,
		      UINT32 word);

IX_QMGR_HWQIF_INLINE void
ixQMgrHwQIfWordRead (VUINT32 *address,
		     UINT32 *word);

IX_QMGR_HWQIF_INLINE void
ixQMgrHwQIfQPop (IxQMgrQId qId,
		 IxQMgrQEntrySizeInWords numWords,
		 UINT32 *entry);

IX_QMGR_HWQIF_INLINE void
ixQMgrHwQIfQPush (IxQMgrQId qId,
		  IxQMgrQEntrySizeInWords numWords,
		  UINT32 *entry);

IX_QMGR_HWQIF_INLINE void
ixQMgrHwQIfQStatusRegsRead (IxQMgrDispatchGroup group, 
			    UINT32 *qStatusWords);

IX_QMGR_HWQIF_INLINE BOOL
ixQMgrHwQIfQStatusCheck (UINT32 *oldQStatusWords,
			 UINT32 *newQStatusWords,
			 unsigned int statusWordOffset,			 
			 UINT32 checkValue,
			 UINT32 mask);

IX_QMGR_HWQIF_INLINE BOOL
ixQMgrHwQIfRegisterBitCheck (IxQMgrQId qId, 
			     UINT32 registerBaseAddrOffset,
			     unsigned queuesPerRegWord,
			     unsigned relativeBitOffset,
			     BOOL reset);

IX_QMGR_HWQIF_INLINE BOOL
ixQMgrHwQIfUnderflowCheck (IxQMgrQId qId);

IX_QMGR_HWQIF_INLINE BOOL
ixQMgrHwQIfOverflowCheck (IxQMgrQId qId);

IX_QMGR_HWQIF_INLINE UINT32
ixQMgrHwQIfQRegisterBitsRead (IxQMgrQId qId, 
			      UINT32 registerBaseAddrOffset,
			      unsigned queuesPerRegWord);
IX_QMGR_HWQIF_INLINE void
ixQMgrHwQIfQInterruptRegWrite (IxQMgrDispatchGroup group, 
			       UINT32 reg);
IX_QMGR_HWQIF_INLINE void
ixQMgrHwQIfQInterruptRegRead (IxQMgrDispatchGroup group, 
			      UINT32 *regVal);

IX_QMGR_HWQIF_INLINE void
ixQMgrHwQIfQueLowStatRead (IxQMgrQId qId,
			   IxQMgrQStatus *status);

IX_QMGR_HWQIF_INLINE void
ixQMgrHwQIfQueUppStatRead (IxQMgrQId qId,
			   IxQMgrQStatus *status);

IX_QMGR_HWQIF_INLINE void
ixQMgrHwQIfQueStatRead (IxQMgrQId qId, 
			IxQMgrQStatus *qStatus);

IX_QMGR_HWQIF_INLINE unsigned
ixQMgrHwQIfPow2NumDivide (unsigned numerator, 
			  unsigned denominator);

IX_QMGR_HWQIF_INLINE void
ixQMgrHwQIfQInterruptEnableRegRead (IxQMgrDispatchGroup group, 
			            UINT32 *regVal);
/*
 * Inline functions
 */

/*
 * This inline function is used by other QMgr components to write one
 * word to the specified address.
 */
IX_QMGR_HWQIF_INLINE void
ixQMgrHwQIfWordWrite (VUINT32 *address,
		      UINT32 word)
{
    IX_OSAL_WRITE_LONG(address, word);
}

/*
 * This inline function is used by other QMgr components to read a
 * word from the specified address.
 */
IX_QMGR_HWQIF_INLINE void
ixQMgrHwQIfWordRead (VUINT32 *address,
		     UINT32 *word)
{
    *word = IX_OSAL_READ_LONG(address);
}


/*
 * This inline function is used by other QMgr components to pop an
 * entry off the specified queue.
 */
IX_QMGR_HWQIF_INLINE void
ixQMgrHwQIfQPop (IxQMgrQId qId,
		 IxQMgrQEntrySizeInWords numWords,
		 UINT32 *entry)
{
    volatile UINT32 *accRegAddr;

    accRegAddr = (UINT32*)(hwQBaseAddress +
    			   IX_QMGR_Q_ACCESS_ADDR_GET(qId));

    switch (numWords)
    {
	case IX_QMGR_Q_ENTRY_SIZE1:
	    ixQMgrHwQIfWordRead (accRegAddr, entry);
	    break;
	case IX_QMGR_Q_ENTRY_SIZE2:
	    ixQMgrHwQIfWordRead (accRegAddr++, entry++);
	    ixQMgrHwQIfWordRead (accRegAddr, entry);
	    break;
	case IX_QMGR_Q_ENTRY_SIZE4:
	    ixQMgrHwQIfWordRead (accRegAddr++, entry++);
	    ixQMgrHwQIfWordRead (accRegAddr++, entry++);
	    ixQMgrHwQIfWordRead (accRegAddr++, entry++);
	    ixQMgrHwQIfWordRead (accRegAddr, entry);
	    break;
	default:
	    IX_QMGR_LOG_ERROR0("Invalid Q Entry size passed to ixQMgrHwQIfQPop");
	    break;
    }
}

/*
 * This inline function is used by other QMgr components to push an
 * entry to the specified queue.
 */
IX_QMGR_HWQIF_INLINE void
ixQMgrHwQIfQPush (IxQMgrQId qId,
		  IxQMgrQEntrySizeInWords numWords,
		  UINT32 *entry)
{
    volatile UINT32 *accRegAddr;

    accRegAddr = (UINT32*)(hwQBaseAddress +
    			   IX_QMGR_Q_ACCESS_ADDR_GET(qId));
    
    switch (numWords)
    {
	case IX_QMGR_Q_ENTRY_SIZE1:
	    ixQMgrHwQIfWordWrite (accRegAddr, *entry);
	    break;
	case IX_QMGR_Q_ENTRY_SIZE2:
	    ixQMgrHwQIfWordWrite (accRegAddr++, *entry++);
	    ixQMgrHwQIfWordWrite (accRegAddr, *entry);
	    break;
	case IX_QMGR_Q_ENTRY_SIZE4:
	    ixQMgrHwQIfWordWrite (accRegAddr++, *entry++);
	    ixQMgrHwQIfWordWrite (accRegAddr++, *entry++);
	    ixQMgrHwQIfWordWrite (accRegAddr++, *entry++);
	    ixQMgrHwQIfWordWrite (accRegAddr, *entry);
	    break;
	default:
	    IX_QMGR_LOG_ERROR0("Invalid Q Entry size passed to ixQMgrHwQIfQPush");
	    break;
    }
}

/*
 * The HwQ interrupt registers contains a bit for each HwQ queue
 * specifying the queue (s) that cause an interrupt to fire. This
 * function is called by IxQMGrDispatcher component.
 */
IX_QMGR_HWQIF_INLINE void
ixQMgrHwQIfQStatusRegsRead (IxQMgrDispatchGroup group, 
			    UINT32 *qStatusWords)
{
    volatile UINT32 *regAddress = NULL;

    if (group == IX_QMGR_GROUP_Q0_TO_Q31)
    {
	regAddress = (UINT32*)(hwQBaseAddress +
			       IX_QMGR_QUELOWSTAT0_OFFSET);

	ixQMgrHwQIfWordRead (regAddress++, qStatusWords++);
	ixQMgrHwQIfWordRead (regAddress++, qStatusWords++);
	ixQMgrHwQIfWordRead (regAddress++, qStatusWords++);
	ixQMgrHwQIfWordRead (regAddress, qStatusWords);
    }
    else /* We have the upper queues */
    {
       /* Only need to read the Nearly Empty status register for
	* queues 32-63 as for therse queues the interrtupt source
	* condition is fixed to Nearly Empty
	*/
	regAddress = (UINT32*)(hwQBaseAddress +
			       IX_QMGR_QUEUPPSTAT0_OFFSET);
	ixQMgrHwQIfWordRead (regAddress, qStatusWords);
    }
}


/*
 * This function check if the status for a queue has changed between
 * 2 snapshots and if it has, that the status matches a particular
 * value after masking.
 */
IX_QMGR_HWQIF_INLINE BOOL
ixQMgrHwQIfQStatusCheck (UINT32 *oldQStatusWords,
			 UINT32 *newQStatusWords,
			 unsigned int statusWordOffset,			 
			 UINT32 checkValue,
			 UINT32 mask)
{
    if (((oldQStatusWords[statusWordOffset] & mask) != 
	 (newQStatusWords[statusWordOffset] & mask)) &&
	((newQStatusWords[statusWordOffset] & mask) == checkValue))
    {
	return TRUE;
    }

    return FALSE;
}

/*
 * The HwQ interrupt register contains a bit for each HwQ queue
 * specifying the queue (s) that cause an interrupt to fire. This
 * function is called by IxQMgrDispatcher component.
 */
IX_QMGR_HWQIF_INLINE void
ixQMgrHwQIfQInterruptRegRead (IxQMgrDispatchGroup group, 
			      UINT32 *regVal)
{
    volatile UINT32 *regAddress;

    if (group == IX_QMGR_GROUP_Q0_TO_Q31)
    {
	regAddress = (UINT32*)(hwQBaseAddress +
			       IX_QMGR_QINTREG0_OFFSET);
    }
    else
    {
	regAddress = (UINT32*)(hwQBaseAddress +
			       IX_QMGR_QINTREG1_OFFSET);
    }

    ixQMgrHwQIfWordRead (regAddress, regVal);
}

/*
 * The HwQ interrupt enable register contains a bit for each HwQ queue.
 * This function reads the interrupt enable register. This
 * function is called by IxQMgrDispatcher component.
 */
IX_QMGR_HWQIF_INLINE void
ixQMgrHwQIfQInterruptEnableRegRead (IxQMgrDispatchGroup group, 
			            UINT32 *regVal)
{
    volatile UINT32 *regAddress;

    if (group == IX_QMGR_GROUP_Q0_TO_Q31)
    {
	regAddress = (UINT32*)(hwQBaseAddress +
			       IX_QMGR_QUEIEREG0_OFFSET);
    }
    else
    {
	regAddress = (UINT32*)(hwQBaseAddress +
			       IX_QMGR_QUEIEREG1_OFFSET);
    }

    ixQMgrHwQIfWordRead (regAddress, regVal);
}


/*
 * This inline function will read the status bit of a queue
 * specified by qId. If reset is TRUE the bit is cleared.
 */
IX_QMGR_HWQIF_INLINE BOOL
ixQMgrHwQIfRegisterBitCheck (IxQMgrQId qId, 
			     UINT32 registerBaseAddrOffset,
			     unsigned queuesPerRegWord,
			     unsigned relativeBitOffset,
			     BOOL reset)
{
    UINT32 actualBitOffset;
    volatile UINT32 *registerAddress;
    UINT32 registerWord;

    /*
     * Calculate the registerAddress
     * multiple queues split accross registers
     */
    registerAddress = (UINT32*)(hwQBaseAddress +
    				registerBaseAddrOffset +
    				((qId / queuesPerRegWord) *
    				 IX_QMGR_NUM_BYTES_PER_WORD));

    /*
     * Get the status word
     */
    ixQMgrHwQIfWordRead (registerAddress, &registerWord);
    
    /*
     * Calculate the actualBitOffset
     * status for multiple queues stored in one register
     */
    actualBitOffset = (relativeBitOffset + 1) <<
	((qId & (queuesPerRegWord - 1)) * (BITS_PER_WORD / queuesPerRegWord));

    /* Check if the status bit is set */
    if (registerWord & actualBitOffset)
    {
	/* Clear the bit if reset */
	if (reset)
	{
	    ixQMgrHwQIfWordWrite (registerAddress, registerWord & (~actualBitOffset));
	}
	return TRUE;
    }

    /* Bit not set */
    return FALSE;
}


/*
 * @ingroup IxQmgrHwQIfAPI
 *
 * @brief Read the underflow status of a queue 
 *
 * This inline function will read the underflow status of a queue
 * specified by qId.
 * 
 */
IX_QMGR_HWQIF_INLINE BOOL
ixQMgrHwQIfUnderflowCheck (IxQMgrQId qId)
{
    if (qId < IX_QMGR_MIN_QUE_2ND_GROUP_QID)
    {
	return (ixQMgrHwQIfRegisterBitCheck (qId,
					     IX_QMGR_QUEUOSTAT0_OFFSET,
					     IX_QMGR_QUEUOSTAT_NUM_QUE_PER_WORD,
					     IX_QMGR_UNDERFLOW_BIT_OFFSET,
					     TRUE/*reset*/));
    }
    else
    {
	/* Qs 32-63 have no underflow status */
	return FALSE;
    }
}

/*
 * This inline function will read the overflow status of a queue
 * specified by qId.
 */
IX_QMGR_HWQIF_INLINE BOOL
ixQMgrHwQIfOverflowCheck (IxQMgrQId qId)
{
    if (qId < IX_QMGR_MIN_QUE_2ND_GROUP_QID)
    {
	return (ixQMgrHwQIfRegisterBitCheck (qId,
					     IX_QMGR_QUEUOSTAT0_OFFSET,
					     IX_QMGR_QUEUOSTAT_NUM_QUE_PER_WORD,
					     IX_QMGR_OVERFLOW_BIT_OFFSET,
					     TRUE/*reset*/));
    }
    else
    {
	/* Qs 32-63 have no overflow status */
	return FALSE;
    }
}

/*
 * This inline function will read the status bits of a queue
 * specified by qId.
 */
IX_QMGR_HWQIF_INLINE UINT32
ixQMgrHwQIfQRegisterBitsRead (IxQMgrQId qId, 
			      UINT32 registerBaseAddrOffset,
			      unsigned queuesPerRegWord)
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
    /*
     * Read the status word
     */
    ixQMgrHwQIfWordRead (registerAddress, &registerWord);
    

    /*
     * Calculate the mask for the status bits for this queue.
     */
    statusBitsMask = ((1 << bitsPerQueue) - 1);

    /*
     * Shift the status word so it is right justified
     */    
    registerWord >>= ((qId & (queuesPerRegWord - 1)) * bitsPerQueue);

    /*
     * Mask out all bar the status bits for this queue
     */
    return (registerWord &= statusBitsMask);
}

/*
 * This function is called by IxQMgrDispatcher to set the contents of
 * the HwQ interrupt register.
 */
IX_QMGR_HWQIF_INLINE void
ixQMgrHwQIfQInterruptRegWrite (IxQMgrDispatchGroup group, 
			       UINT32 reg)
{
    volatile UINT32 *address;

    if (group == IX_QMGR_GROUP_Q0_TO_Q31)
    {
	address = (UINT32*)(hwQBaseAddress +
			    IX_QMGR_QINTREG0_OFFSET);
    }
    else
    {
	address = (UINT32*)(hwQBaseAddress +
			    IX_QMGR_QINTREG1_OFFSET);
    }

    ixQMgrHwQIfWordWrite (address, reg);
}

/*
 * Read the status of a queue in the range 0-31.
 *
 * This function is used by other QMgr components to read the
 * status of the queue specified by qId.
 */
IX_QMGR_HWQIF_INLINE void
ixQMgrHwQIfQueLowStatRead (IxQMgrQId qId,
			   IxQMgrQStatus *status)
{
    /* Read the general status bits */
    *status = ixQMgrHwQIfQRegisterBitsRead (qId,
					    IX_QMGR_QUELOWSTAT0_OFFSET,
					    IX_QMGR_QUELOWSTAT_NUM_QUE_PER_WORD);
}

/*
 * This function will read the status of the queue specified
 * by qId.
 */
IX_QMGR_HWQIF_INLINE void
ixQMgrHwQIfQueUppStatRead (IxQMgrQId qId,
			   IxQMgrQStatus *status)
{
    /* Reset the status bits */
    *status = 0;

    /* 
     * Check if the queue is nearly empty,
     * N.b. QUPP stat register contains status for regs 32-63 at each
     *      bit position so subtract 32 to get bit offset
     */
    if (ixQMgrHwQIfRegisterBitCheck ((qId - IX_QMGR_MIN_QUE_2ND_GROUP_QID),
				     IX_QMGR_QUEUPPSTAT0_OFFSET,
				     IX_QMGR_QUEUPPSTAT_NUM_QUE_PER_WORD,
				     0/*relativeBitOffset*/,
				     FALSE/*!reset*/))
    {
	*status |= IX_QMGR_Q_STATUS_NE_BIT_MASK;
    }

    /* 
     * Check if the queue is full,
     * N.b. QUPP stat register contains status for regs 32-63 at each
     *      bit position so subtract 32 to get bit offset
     */
    if (ixQMgrHwQIfRegisterBitCheck ((qId - IX_QMGR_MIN_QUE_2ND_GROUP_QID),
				     IX_QMGR_QUEUPPSTAT1_OFFSET,
				     IX_QMGR_QUEUPPSTAT_NUM_QUE_PER_WORD,
				     0/*relativeBitOffset*/,
				     FALSE/*!reset*/))
    {
	*status |= IX_QMGR_Q_STATUS_F_BIT_MASK;
    }
}

/*
 * This function is used by other QMgr components to read the
 * status of the queue specified by qId.
 */
IX_QMGR_HWQIF_INLINE void
ixQMgrHwQIfQueStatRead (IxQMgrQId qId, 
			IxQMgrQStatus *qStatus)
{
    if (qId < IX_QMGR_MIN_QUE_2ND_GROUP_QID)
    {
	ixQMgrHwQIfQueLowStatRead (qId, qStatus);
    }
    else
    {
	ixQMgrHwQIfQueUppStatRead (qId, qStatus);
    }
}


/*
 * This function performs a mod division
 */
IX_QMGR_HWQIF_INLINE unsigned
ixQMgrHwQIfPow2NumDivide (unsigned numerator, 
			  unsigned denominator)
{
    /* Number is evenly divisable by 2 */
    return (numerator >> ixQMgrHwQIfLog2 (denominator));
}

/* Restore IX_COMPONENT_NAME */
#undef IX_COMPONENT_NAME
#define IX_COMPONENT_NAME IX_QMGR_HWQIF_SAVED_COMPONENT_NAME

#endif/*IXQMGRHWQIF_P_H*/
