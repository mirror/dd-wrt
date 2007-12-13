 /**
 * @file IxDmaAccCodelet.h
 *
 * @date 18 November 2002
 *
 * @brief This file contains the interface for the Dma Access Codelet.
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

#ifndef IXDMAACCCODELET_H
#define IXDMAACCCODELET_H

#include "IxOsal.h"
#include "IxDmaAcc.h"

/**
 * @defgroup Codelets Intel (R) IXP400 Software Codelets
 *
 * @brief Intel (R) IXP400 Software Codelets
 *
 * @{
 */

/**
 * @ingroup Codelets
 *
 * @defgroup IxDmaAccCodelet Intel (R) IXP400 Software DMA Access Codelet (IxDmaAccCodelet) API
 *
 * @brief Intel (R) IXP400 Software DMA Access component API
 *
 * This file contains a main interface of the Dma Access Codelet that 
 * initialises the DmaAcc codelet and execute Dma transfer using 
 * ixDmaAccCodeletTestPerform() function for various DMA transfer mode, 
 * addressing mode and transfer width. The block size used in this codelet
 * are 8,1024,16384,32768,65528 bytes. For each Dma configuration, the 
 * performance will be measured and the average rate (in Mbps) will be
 * displayed
 *
 * <b> VxWorks* User Guide </b><br>
 *  <pre>
 *  <i> Usage :
 *      -> ixDmaAccCodeletMain()
 *
 * Note:
 * 1. Once the function is executed, the codelet will display the results <br>
 * 2. The formulae to calculate the rate is: <br>
 *    Rate (in Mbps)  = ( (length * 8) / (ticks / 66) ) 
 *
 * </i>
 * </pre>     
 *
 * <b> Linux* User Guide </b><br>
 * <pre>
 * <i>  Usage :
 *      # insmod ixp400_codelets_dmaAcc.o
 *
 * Note: <br>
 * 1. Once the function is executed, the codelet will display the results <br>
 * 2. The formulae to calculate the rate is: <br>
 *    Rate (in Mbps)  = ( (length * 8) / (ticks / 66) ) 
 *
 * </i>
 * </pre>   
 *
 * <b> DmaAcc Codelet Features </b>
 *
 * The API ixDmaAccCodeletTestPerform() allows the user to perform a
 * Dma transfer of block size 0 to 65535 bytes between two locations
 * in the SRAM. The user can specify any combination of the following modes.   
 *
 *            DMA Transfer Modes 
 *	         1. Copy
 *	      	 2. Copy and Clear Source
 *	      	 3. Copy with Bytes Swap
 *	      	 4. Copy with Bytes Reversed
 *
 *	      DMA Addressing Modes
 *	      	 1. Incremental Source to Incremental Destination Addressess
 *	         2. Fixed Source to Incremental Destination Addressess
 *	      	 3. Incremental Source to Fixed Destination Addressess
 *
 *	      DMA Transfer Widths
 *	       	 1. 32-bit Transfer
 *	      	 2. 16-bit Transfer
 *	      	 3. 8-bit Transfer
 *	      	 4. Burst Transfer
 *
 *  NOTE : The user must initialise the system with ixDmaAccCodeletInit prior to 
 *         calling the function ixDmaAccCodeletiTestPerform()
 *
 * Performance will execute PERFORMANCE_NUM_LOOP (i.e. 100 runs) in order to 
 * calculate the average rate for each Dma transfer configuration
 *
 * @{
 */

/*
 * Defines
 */

/**
 * @ingroup IxDmaAccCodelet
 *
 * @def IX_DMA_CODELET_TRANSFER_LENGTH
 *
 * @brief The length of the transfer size if 128 bytes.
 * 
 * It can be changed for Dma transfer. The range is between 1-65535 bytes
 */
#define IX_DMA_CODELET_TRANSFER_LENGTH 128

/*
 * Prototypes for interface functions.
 */

/**
 * @ingroup IxDmaAccCodelet
 *
 * @fn void ixDmaAccCodeletMain(void)
 *
 * @brief This function is the entry point to the Dma Access codelet.
 * It will initialise the Dma codelet which in turn initialises the necessary
 * components.
 *
 * Once it has successfully initialise the Dma Codelet, this function will
 * continue to perform valid DMA transfer using IxDmaAccCodeletTestPerform()
 *
 * @param none
 *
 * @return none
 */
IX_STATUS ixDmaAccCodeletMain(void);

#endif /* IXDMAACCCODELET_H */

/** @} defgroup IxDmaAccCodelet*/

/** @} defgroup Codelet*/
