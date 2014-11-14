/**
 * @file IxParityENAccCodelet.h
 *
 * @author Intel Corporation
 *
 * @date 23 December 2004
 *
 * @brief This is the header file for Intel (R) IXP400 Software Parity Error Notifier 
 *	  Access Codelet
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

#ifndef IXPARITYENACCCODELET_H
#define IXPARITYENACCCODELET_H

#if defined (__ixp46X) || defined(__ixp43X)

/**
 * @ingroup Codelets
 *
 * @defgroup IxParityENAccCodelet Intel (R) IXP400 Software Parity Error Notifier Access Codelet 
 *
 * @brief Intel (R) IXP400 Software Parity Error Notifier Access Codelet 
 *
 * This codelet shows how to integrate Parity Error Notifier to client 
 * application. It demonstrates the followings: 
 *
 * <PRE>
 *	- how to initialize ParityEN. 
 *	
 *	- how to configure ParityEN or modify ParityEN configuration. 
 *
 *	- how to register callback with ParityEN. 
 *
 *	- how to register data abort handler with kernel (only for VxWorks*).
 *	
 *	- how to inject ECC error. 
 *
 *	- how to spawn a task to initiate SDRAM memory scan. 
 *	
 *	- how to scrub memory to correct single bit ECC error.
 *
 *	- how to handle various parity errors reported by ParityENAcc
 *		e.g. scrub memory to correct single bit ECC error,
 *		     reboot the board after detecting multi-bit ECC error, etc. 
 *	
 *	- how to determine whether the data abort is due to multi bit ECC 
 *	  error initiated when the Intel XScale(R) processor accesses SDRAM.
 *
 * </PRE>
 *
 * <B> VxWorks* User Guide </B><BR>
 * (1) <B> ixParityENAccCodeletMain </B><BR>
 * This function is the main function for ParityENAcc
 * codelet. This function will perform the followings:
 *
 *	- initialize ParityENACC
 *
 *	- register callback with ParityENAcc to handle parity error
 *
 *	- configure ParityENAcc
 *
 *	- spawn "shut down" task to reboot the board when reboot
 *	  is requested
 *
 *	- register callback with VxWorks* kernel to handle data abort
 *
 *	- generate bad ECC on allocated SDRAM memory
 *
 *	- spawn a task to start SDRAM memory scan
 *
 * <PRE>
 *  <I> Usage :
 *      -> ixParityENAccCodeletMain "multiBit", "injectNow"
 *
 *	where 
 *	"multiBit" specifies the ECC error type. 
 *		0 - single bit ECC error (DEFAULT)
 *		1 - multi bit ECC error
 *
 *	"injectNow" specifies when ECC error will occur.
 *		0  - do not cause ECC error until the memory is read later (DEFAULT)
 *		1  - read the memory now to cause ECC error
 *  </I>
 * </PRE>
 *
 *
 * (2) <B> ixParityENAccCodeletQuit </B><BR>
 * This function terminates SDRAM memory scan. 
 *
 * <PRE>
 *  <I> Usage :
 * 	-> ixParityENAccCodeletQuit
 *
 *  </I>
 * </PRE>
 *
 * <B> Linux* User Guide </B><BR>
 * (1) <B> ixParityENAccCodeletMain </B><BR>
 * This function is the main function for ParityENAcc
 * codelet. This function will perform the followings:
 *
 *	- initialize ParityENACC
 *
 *	- register callback with ParityENAcc to handle parity error
 *
 *	- configure ParityENAcc
 *
 *	- spawn "shut down" task to reboot the board when reboot
 *	  is requested
 *
 *	- generate bad ECC on allocated SDRAM memory
 *
 *	- spawn a task to start SDRAM memory scan
 *
 * This function will be invoked and executed when the user loads the
 * ParityENACc codelet module using 'insmod' command.
 * 
 * <PRE>
 *  <I> Usage :
 *      prompt> insmod ixp400_codelets_parityENAcc.o multiBit=x injectNow=y
 *
 *	where x = 0 - single bit ECC error (DEFAULT)
 *		    1 - multi bit ECC error
 *
 *	and y = 0 - do not cause ECC error until the memory is read later (DEFAULT)
 *		  1 - read the memory now to cause ECC error
 *  </I>
 * </PRE>
 *
 * (2) <B> ixParityENAccCodeletQuit </B><BR>
 * This function terminates SDRAM memory scan. This function will
 * be executed when user terminates parityENAcc codelet
 * execution using 'rmmod' command. 
 *   
 * <PRE>
 *  <I> Usage :
 * 	prompt> rmmod ixp400_codelets_parityENAcc
 *
 *  </I>
 * </PRE>
 *
 * @{
 */


/*********************************************************************
 *	System include files
 *********************************************************************/

#include "IxOsal.h"

#ifdef __vxworks
    	#include "sysLib.h"
    	#include "rebootLib.h"
    	#include "excLib.h"
#elif defined(__linux)
    	#include <linux/reboot.h>
    	#include <linux/mm.h>
#else
    	#error unsupported OS 
#endif


/*********************************************************************
 *	include files 
 *********************************************************************/

#include "IxOsCacheMMU.h"
#include "IxParityENAcc.h"

/*********************************************************************
 *	Macro constants and function definition 
 *********************************************************************/

/**
 * @def IX_PARITYENACC_CODELET_QWORD_ALIGNED_MASK
 *
 * @brief mask value that moves an address to its Q-word aligned
 *	  location. 
 */
#define IX_PARITYENACC_CODELET_QWORD_ALIGNED_MASK (0xFFFFFFF0)          

/**
 * @def IX_PARITYENACC_CODELET_SCAN_SEGMENT_SIZE
 *
 * @brief segment size for memory scan. One segment is 256 Kb.  
 *
 */
#define IX_PARITYENACC_CODELET_SCAN_SEGMENT_SIZE (0x40000)          

/**
 * @def IX_PARITYENACC_CODELET_DISPLAY_INTERVAL
 *
 * @brief display interval for memory scan. The progress of
 *	  scanning will be shown after scanning 32 Mb.  
 *
 */
#define IX_PARITYENACC_CODELET_DISPLAY_INTERVAL (0x2000000)          

/**
 * @def IX_PARITYENACC_CODELET_ECC_TEST_REG_OFFSET
 *
 * @brief byte offset for Memory Controller's ECC Test Register  
 *	  from ECC Control Register. ECC Test Register is used to  
 *	  generate bad ECC for ECC testing.
 */
#define IX_PARITYENACC_CODELET_ECC_TEST_REG_OFFSET (0x14)

/**
 * @def IX_PARITYENACC_CODELET_SINGLE_BIT_ERROR_BIT_0_SYNDROME
 *
 * @brief single bit error syndrome value at bit 0. 
 */
#define IX_PARITYENACC_CODELET_SINGLE_BIT_ERROR_BIT_0_SYNDROME (0xC1)

/**
 * @def IX_PARITYENACC_CODELET_MULTI_BIT_ERROR_SYNDROME
 *
 * @brief multi bit error syndrome value. 
 */
#define IX_PARITYENACC_CODELET_MULTI_BIT_ERROR_SYNDROME (0xFF)

/**
 * @def IX_PARITYENACC_CODELET_EXTERNAL_DATA_ABORT
 *
 * @brief Fault Status Register's imprecise external data abort value
 */
#define IX_PARITYENACC_CODELET_EXTERNAL_DATA_ABORT (0x00000406)

/**
 * @def IX_PARITYENACC_CODELET_DATA_CACHE_PARITY_ERR
 *
 * @brief Fault Status Register's data cache parity error value
 */
#define IX_PARITYENACC_CODELET_DATA_CACHE_PARITY_ERR (0x00000408)

/**
 * @def IX_PARITYENACC_CODELET_BIT_MASK_CHECK(data, mask)
 *
 * @brief check if the result of bit mask operation on 'data' 
 *	  and 'mask' match the 'mask'.
 *
 * @param data [in] - the data that will be masked and then compared 
 *			with the mask.
 *
 * @param mask [in] - the mask value.
 *
 * @return BOOL 
 *         @li TRUE  - bit mask operation result match the mask  
 *         @li FALSE - bit mask operation result does not match the mask 
 */
#define IX_PARITYENACC_CODELET_BIT_MASK_CHECK(data, mask) \
		(((data) & (mask)) == (mask))


#ifdef __vxworks

	/**
 	 * @def IX_PARITYENACC_CODELET_REBOOT()
 	 *
 	 * @brief map to reboot function. reboot() is not fully implemented, and it
	 * 	  would cause the board to hang when it is executed. In the mean time,
	 *	  sysToMonitorColdReboot() which reboots the board using the watchdog 
	 *	  timer reset functionality, will be used to reboot the board.
 	 */
	#define IX_PARITYENACC_CODELET_REBOOT()  (reboot (BOOT_QUICK_AUTOBOOT))

	/**
 	 * @def IX_PARITYENACC_CODELET_SDRAM_BASE_ADDR
 	 *
	 * @brief SDRAM base address.  
 	 */
	#define IX_PARITYENACC_CODELET_SDRAM_BASE_ADDR	(IXP400_SDRAM_BASE) 

#elif defined(__linux)
	/**
 	 * @def IX_PARITYENACC_CODELET_REBOOT()
 	 * New Timesys kernel does not support machine_restart()
 	 * @brief map to reboot function
 	 */
	 /* #define IX_PARITYENACC_CODELET_REBOOT() (machine_restart(NULL))*/
	#define IX_PARITYENACC_CODELET_REBOOT()	 

	/**
 	 * @def IX_PARITYENACC_CODELET_SDRAM_SCAN_START_OFFSET
 	 *
	 * @brief memory scan start address offset 
 	 */
	#define IX_PARITYENACC_CODELET_SDRAM_SCAN_START_OFFSET	 (4)	

	/**
 	 * @def IX_PARITYENACC_CODELET_SDRAM_BASE_ADDR
 	 *
	 * @brief SDRAM base address, adjust base address to start at byte #4
 	 */
	#define IX_PARITYENACC_CODELET_SDRAM_BASE_ADDR	(IX_PARITYENACC_CODELET_SDRAM_SCAN_START_OFFSET)
										
#else
	#error unsupported OS
#endif


/*********************************************************************
 *	Function Prototype 
 *********************************************************************/
        
/**
 * @fn ixParityENAccCodeletMain (BOOL multiBit, BOOL injectNow)	
 *
 * @brief  This is the main function for Parity Error Notifier Access
 *	   Component (ParityENAcc) Codelet. 
 *
 * This function will first initialize ParityENAcc. Then, it registers 
 * the callback with ParityENAcc to notify the codelet whenever parity
 * error is detected. Next, this function will configure ParityENAcc 
 * to enable parity error detection. By default, parity error detection 
 * will only enabled on MCU. 
 *
 * Next, this function will spawn "shut down" task. This task will
 * sleep all the time. It will only be invoked when board reboot is
 * requested. This task is mainly used to reboot the board. 
 *
 * In VxWorks*, a data abort handler is hooked up to DATA ABORT 
 * exception vector. This data abort handler will be called when
 * data abort occurs. This data abort handler will determine whether
 * the data abort is triggered by multi-bit ECC error. The board will 
 * be rebooted after data abort is processed. Linux* does not have the  
 * hook up capability.
 *
 * After that, this function will generate bad ECC on allocated SDRAM
 * memory. The user needs to provide two parameters to specify how the
 * ECC error should be generated. First, the user has to specify the  
 * type of ECC error injection - single bit or multi bit. Second, the  
 * user has to tell this function when to cause ECC error after bad 
 * ECC generation. By default, bad single bit ECC will be injected,  
 * and the error will be discovered later when the memory is accessed 
 * by the memory scan. 
 *
 * Finally, this function will initiate SDRAM memory scan. Any  
 * single bit ECC error found during the scan will be scrubbed and  
 * corrected. If multi-bit ECC error is detected during the scan,
 * the address of memory with bad ECC will be printed, then the board
 * will be rebooted.
 *
 * @param 
 * multiBit BOOL [in] - type of ECC error injection.
 *	 - FALSE : Single bit ECC error.
 *	 - TRUE  : Multi-bit ECC error.
 * 
 * @param 
 * injectNow BOOL [in] - preference for when to generate 
 *		ECC error.
 *	 - FALSE : This function will only generate bad ECC 
 * 		     on allocated memory. ECC error will only occur
 *		     when the memory is read later.
 *	 - TRUE : After generating bad ECC, this function will  
 *		    immediately read the memory to cause ECC error.
 * 
 * @return  IX_STATUS
 *          @li IX_SUCCESS - start codelet successfully
 *          @li IX_FAIL    - fail to start codelet
 */
PUBLIC IX_STATUS ixParityENAccCodeletMain (
	BOOL multiBit,		
	BOOL injectNow);

/**
 * @fn ixParityENAccCodeletQuit ()
 *
 * @brief  ixParityENAccCodeletQuit function terminates SDRAM scan.
 *
 * @return void 
 *
 */
PUBLIC void ixParityENAccCodeletQuit (void);

/** @} */

/*********************************************************************
 *	extern functions 
 *********************************************************************/

#ifdef __vxworks
	extern void reboot (int);
	extern char *sysMemTop (void);
#endif
	
#endif  /* __ixp46X || __ixp43X */
#endif  /* end of IXPARITYENACCCODELET_H */
