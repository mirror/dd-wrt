/***************************************************************************
 *
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
 ***************************************************************************/

/**
 *****************************************************************************
 * @file lac_common.h Common macros
 *
 * @defgroup  Lac   Look Aside Crypto LLD Doc
 *
 *****************************************************************************/

/**
 *****************************************************************************
 * @mainpage "LLD Introduction"
 *
 * @section LacIntro Browsing the LLD
 * The LLD for LAC is embedded in the source code and is generated through
 * doxygen. The LLD can be browsed using the menu on the left, specifically
 * the "Modules" tree. To browse to the Lac LLD Doc
 *  -# Expand the Modules Menu Tree menu on the left
 *  -# Browse to the \ref Lac "Look Aside Crypto LLD Doc" module. There are two 
 *     options to navigate the LLD Doc.
 *    - Expand the tree until the desired sub-module is found and select it.
 *      This menu can be expanded through a number of levels.
 *    - Select the Look Aside Crypto LLD Doc module in the menu on the left. 
 *      In the main window the sub components can be navigated through the 
 *      collaboration diagram which links directly to the sub-component.
 * 
 * @section LacContent LLD Content 
 *  - The main LLD content for a subcomponent is outlined in the table titled
 *    "LLD Info". Some components are trivial and do not have a dedicated 
 *    "LLD Info" section but have their interfaces documented in detail.
 *  - Complex components contain sequence diagrams such as \ref LacHash "Hash"
 *    and \ref LacCipher "Cipher". The function in the sequence diagram can
 *    be selected and this links to the documentation for that function.
 *    The sequence diagrams are embedded in the code using
 *    syntax that is interpreted through doxygen using a plug-in called mscgen
 *  - The "Related pages" section contains information on Performance 
 *    Considerations, Memory Usage, Locking and Reference Counting. There is
 *    also a todo and bug section which are displayed here if there are any.
 *  - The following graphs are automatically generated
 *     - Call-graphs and caller-graphs for functions. These are visible
 *       under the function documentation for a module.
 *     - For .c files: Include dependency graph of header files. These are 
 *       visible through the "Files" section of a module or through the 
 *       "File List" menu on the left. 
 *     - For .h files: Include dependency graph of header files and graph of 
 *       files that directly or indirectly include this file. These are only
 *       through the "File List" menu on the left.
 *      
 * @section LacDeployment Deployment View of Lac Code
 * At a high level the LAC source code is divided from the the top level 
 * directory /look_aside/crypto into the following subsections:
 * - <b>/src</b> Contains all Production code
 * - <b>/include</b> Externally visible API which are exposed to other 
 *                   Platform level subsystems. This API is not to be used by 
 *                   our end client directly. Any API headers targeted at 
 *                   the end client will reside in 
 *                   /vobs/external_interfaces/api/lac
 * - <b>/unit_test</b> All unit test code for the LAC feature resides here.
 *                     Unit tests treat the LAC feature as a "white box" model 
 *                     for testing.
 * - <b>/integ_test</b> All integration test code resides here. Integration
 *                      testing treats the LAC feature as a "black box" and 
 *                      will use the external interface in the same way as our 
 *                      customers
 * - <b>/doc</b> Customer visible documentation was archived here (Release
 *               Notes, GSG etc.). Now it contains LLD Material
 * 
 *
 * Within the source directory the production code is further broken out as 
 * follows:
 * - <b>/common</b> All code which is common between OS's.
 *        - <b>/asym</b> Asymmetric crypto features
 *        - <b>/sym</b> Symmetric crypto features
 *        - <b>/include</b> Contains all API which are visible across 
 *                          components at this level of the hierarchy. Each 
 *                          level of the hierarchy can have an include which
 *                          is used to share it's API with other components
 *                          at the same level.
 *        - <b>/init</b> Initialisation and Shutdown code for all LAC 
 *                       sub-components.
 *        - <b>/lac_common</b> Functions common across LAC (stats show etc.)
 *        - <b>/utils</b> Generic utilities applicable to all components in 
 *                        LAC.
 * - <b>/linux</b> Any code which is required to be Linux specific. The kernel
 *                 module API resides here.
 *
 * The source code is broken up in this way to allow each feature to be 
 * treated as a sub-component and only expose out the API which can be called
 * by other components (thus using the directory structure to help indicate 
 * which APIs should be visible). More detail on each feature can be accessed 
 * through navigating down to the relevant sub-component or browsing the LLD
 * content present in each feature.
 *
 * @image html lacTree.png "Lac Directory structure"
 *
 *****************************************************************************/

/**
 *****************************************************************************
 * @defgroup  LacCommon   LAC Common
 * Common code for Lac which includes init/shutdown, memory, logging and
 * hooks.
 *
 * @ingroup Lac
 *
 *****************************************************************************/



/***************************************************************************/

#ifndef LAC_COMMON_H
#define LAC_COMMON_H

/*
******************************************************************************
* Include public/global header files
******************************************************************************
*/

#include "cpa.h"
#include "lac_log.h"
#include "cpa_cy_common.h"
#include "qat_comms.h"

#include "IxOsal.h"
/*
******************************************************************************
* Some common settings for this version of the API
******************************************************************************
*/
#define LAC_QAT_NUM_INSTANCES       1 
/**< @ingroup LacCommon
 * Number of QAT instances in on this platform */
#define LAC_QAT_INSTANCE_NAME       "Intel(R) EP80579 with Intel(R) " \
                                    "QuickAssist Technology"
/**< @ingroup LacCommon
 * Name of the QAT platform */

#define LAC_QAT_SERVICES            (CPA_INSTANCE_TYPE_CRYPTO)
/**< @ingroup LacCommon
 * List of service supported by the QAT */


/*
********************************************************************************
* Debug Macros and settings
********************************************************************************
*/

/* DEBUG  */
#ifdef ICP_DEBUG

#define STATIC
/**< @ingroup LacCommon
 * When DEBUG is set STATIC evaluates to nothing. */

#else

#define STATIC static
/**< @ingroup LacCommon
  * otherwise it evaluates to the static keyword */

#endif



#define SEPARATOR "+--------------------------------------------+\n"
/**< @ingroup LacCommon
 * seperator used for printing stats to standard output*/

#define BORDER "|"
/**< @ingroup LacCommon
 * seperator used for printing stats to standard output*/


/**
*****************************************************************************
 * @ingroup LacCommon
 *      Component state
 *
 * @description
 *      This enum is used to indicate the state that the component is in. Its
 *      purpose is to prevent components from being initialised or shutdown
 *      incorrectly.
 *
 *****************************************************************************/
typedef enum {
    LAC_COMP_SHUT_DOWN = 0,
    /**< Component in the Shut Down state */
    LAC_COMP_SHUTTING_DOWN,
    /**< Component in the Process of Shutting down */
    LAC_COMP_INITIALISING,
    /**< Component in the Process of being initialised */
    LAC_COMP_INITIALISED,
    /**< Component in the initialised state */
}lac_comp_state_t;


/**
*******************************************************************************
 * @ingroup LacCommon
 *      Check to see if the LAC component has been initialised
 *
 * @description
 *      This function checks the state of LAC to see if it has being
 *      initialised
 *
 * @retval CPA_TRUE   Initialised
 * @retval CPA_FALSE  Not Initialised.
 *
 *****************************************************************************/
CpaBoolean
Lac_IsInitialised(void);

/**
*******************************************************************************
 * @ingroup LacCommon
 *      Check to see if the LAC component is in the running state
 *
 * @description
 *      This function checks the state of LAC to see if it is in the 
 *      Initialised or shutting down state
 *
 * @retval CPA_TRUE   Initialised or Shutting down
 * @retval CPA_FALSE  Not Initialised or Shutting down.
 *
 *****************************************************************************/
CpaBoolean
Lac_IsRunning(void);

/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro checks if Lac is initilised. An error message is Logged if it
 *      has not been initialised.
 *
 * @return CPA_STATUS_FAIL   Lac has not been initialised
 * @return void         Lac has been successfully initialised
 ******************************************************************************/
#define LAC_INITIALISED_CHECK()                                     \
do {                                                                \
    if ( CPA_TRUE != Lac_IsInitialised() )                          \
    {                                                               \
        LAC_LOG_ERROR("LAC API called before LAC was initialised"); \
        return CPA_STATUS_FAIL;                                     \
    }                                                               \
} while(0)

/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro checks if Lac is initilised. An error message is Logged if it
 *      has not been initialised.
 *
 * @return CPA_STATUS_FAIL   Lac has not been initialised
 * @return void         Lac has been successfully initialised
 ******************************************************************************/
#define LAC_RUNNING_CHECK()                                             \
do {                                                                    \
    if ( CPA_TRUE != Lac_IsRunning() )                                  \
    {                                                                   \
        LAC_LOG_ERROR("LAC API called with LAC not in a Running state");\
        return CPA_STATUS_FAIL;                                         \
    }                                                                   \
} while(0)


/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro checks if a paramater is NULL
 *
 * @param[in] param                 Paramater
 *
 * @return CPA_STATUS_INVALID_PARAM Paramater is NULL
 * @return void                     Paramater is not NULL
 ******************************************************************************/
#define LAC_CHECK_NULL_PARAM(param)               \
do {                                              \
    if (NULL == (param))                          \
    {                                             \
        LAC_INVALID_PARAM_LOG(#param " is NULL"); \
        return CPA_STATUS_INVALID_PARAM;          \
    }                                             \
} while(0)


/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro returns the size of the buffer list structure given the
 *      number of elements in the buffer list - note: only the sizeof the
 *      buffer list structure is returned.
 *
 * @param[in] numBuffers    The number of flatbuffers in a buffer list
 *
 * @return size of the buffer list structure
 ******************************************************************************/
#define LAC_BUFFER_LIST_SIZE_GET(numBuffers) \
    (sizeof(CpaBufferList) + \
    (numBuffers * sizeof(CpaFlatBuffer)))

/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro checks that a flatbuffer is valid i.e. that it is not
 *      null and the data it points to is not null
 *
 * @param[in] pFlatBuffer           Pointer to flatbuffer
 *
 * @return CPA_STATUS_INVALID_PARAM Invalid flatbuffer pointer
 * @return void                     flatbuffer is ok
 ******************************************************************************/
#define LAC_CHECK_FLAT_BUFFER(pFlatBuffer)     \
do {                                           \
    LAC_CHECK_NULL_PARAM((pFlatBuffer));       \
    LAC_CHECK_NULL_PARAM((pFlatBuffer)->pData);\
} while(0)


/**
 *******************************************************************************
 * @ingroup LacCommon
 *   This macro verifies that the status is ok i.e. equal to CPA_STATUS_SUCCESS
 *
 * @param[in] status    status we are checking
 *
 * @return void         status is ok (CPA_STATUS_SUCCESS)
 * @return status       The value in the status paramater is an error one
 *
 ******************************************************************************/
#define LAC_CHECK_STATUS(status)    \
do {                                \
    if (CPA_STATUS_SUCCESS != (status)) \
    {                               \
        return status;              \
    }                               \
} while(0)

/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro verifies that the Instance Handle is valid.
 *
 * @param[in] instanceHandle    Instance Handle
 *
 * @return void                 Instance Handle is ok
 *
 ******************************************************************************/
#define LAC_CHECK_INSTANCE_HANDLE(instanceHandle)


/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro converts between LAC and QAT priority enums
 *
 * @param[in] lac_priority      LAC Priority Value
 *
 * @return QAT Priority Value
 *
 ******************************************************************************/
#define LAC_TO_QAT_PRIORITY(lac_priority)       \
    ((CPA_CY_PRIORITY_HIGH == lac_priority) ?   \
     QAT_COMMS_PRIORITY_HIGH : QAT_COMMS_PRIORITY_NORMAL)


/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro copies a string from one location to another
 *
 * @param[out] pDestinationBuffer   Pointer to destination buffer
 * @param[in] pSource               Pointer to source buffer
 *
 ******************************************************************************/
#define LAC_COPY_STRING(pDestinationBuffer, pSource)            \
do {                                                            \
    memcpy(pDestinationBuffer, pSource, (sizeof(pSource)-1));   \
    pDestinationBuffer[(sizeof(pSource)-1)] = '\0';             \
} while(0)


/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro fills a memory zone with ZEROES
 *
 * @param[in] pBuffer               Pointer to buffer
 * @param[in] count                 Buffer length
 *
 * @return void
 *
 ******************************************************************************/
#define LAC_OS_BZERO(pBuffer, count)            \
    ixOsalMemSet(pBuffer, 0, count);


/*
********************************************************************************
* Alignment and Bit Operation Macros
********************************************************************************
*/

#define LAC_NUM_BITS_IN_BYTE    (8)
/**< @ingroup LacCommon
 * Number of bits in a byte */

#define LAC_IA_WORD_SIZE_IN_BYTES   (4)
/**< @ingroup LacCommon
 * Number of bytes in an IA word */

#define LAC_QUAD_WORD_IN_BYTES  (8)
/**< @ingroup LacCommon
 * Number of bytes in a QUAD word */

#define LAC_8BYTE_ALIGNMENT_SHIFT   (3)
/**< @ingroup LacCommon
 * 8 byte alignment to a power of 2 */

#define LAC_16BYTE_ALIGNMENT_SHIFT  (4)
/**< @ingroup LacCommon
 * 16 byte alignment to a power of 2 */

#define LAC_64BYTE_ALIGNMENT_SHIFT  (6)
/**< @ingroup LacCommon
 * 64 byte alignment to a power of 2 */

#define LAC_1BYTE_ALIGNMENT (1)
/**< @ingroup LacCommon
 * 1 byte alignment */

#define LAC_8BYTE_ALIGNMENT (8)
/**< @ingroup LacCommon
 * 8 byte alignment */

#define LAC_64BYTE_ALIGNMENT (64)
/**< @ingroup LacCommon
 * 64 byte alignment */

#define LAC_OPTIMAL_ALIGNMENT_SHIFT LAC_64BYTE_ALIGNMENT_SHIFT
/**< @ingroup LacCommon
 * optimal alignment to a power of 2 */

#define LAC_MAX_16_BIT_VALUE    ((1 << 16) - 1)
/**< @ingroup LacCommon
 * maximum value a 16 bit type can hold */

/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro checks if an address is aligned to the specified power of 2
 *      Returns 0 if alignment is ok, or non-zero otherwise
 *
 * @param[in] address   the address we are checking
 *
 * @param[in] alignment the byte alignment to check (specified as power of 2)
 *
 ******************************************************************************/
#define LAC_ADDRESS_ALIGNED(address, alignment)\
    (!((Cpa32U)(address) & ((1 << (alignment)) - 1)))


/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro rounds up a number to a be a multiple of the alignment when
 *      the alignment is a power of 2.
 *
 * @param[in] num   Number
 * @param[in] align Alignement (must be a power of 2)
 *
 ******************************************************************************/
#define LAC_ALIGN_POW2_ROUNDUP(num, align) \
    ( ((num) + (align) -1) & ~((align) - 1) )


/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro generates a bit mask to select a particular bit
 *
 * @param[in] bitPos    Bit position to select
 *
 ******************************************************************************/
#define LAC_BIT(bitPos)       (0x1 << (bitPos))

/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro converts a size in bits to the equivalent size in bytes,
 *      using a bit shift to divide by 8
 *
 * @param[in] x     size in bits
 *
 ******************************************************************************/
#define LAC_BITS_TO_BYTES(x) ((x) >> 3)

/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro converts a size in bytes to the equivalent size in bits,
 *      using a bit shift to multiply by 8
 *
 * @param[in] x     size in bytes
 *
 ******************************************************************************/
#define LAC_BYTES_TO_BITS(x) ((x) << 3)

/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro converts a size in bytes to the equivalent size in longwords,
 *      using a bit shift to divide by 4
 *
 * @param[in] x     size in bytes
 *
 ******************************************************************************/
#define LAC_BYTES_TO_LONGWORDS(x) ((x) >> 2)


/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro converts a size in longwords to the equivalent size in bytes,
 *      using a bit shift to multiply by 4
 *
 * @param[in] x     size in long words
 *
 ******************************************************************************/
#define LAC_LONGWORDS_TO_BYTES(x) ((x) << 2)

/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro converts a size in bytes to the equivalent size in quadwords,
 *      using a bit shift to divide by 8
 *
 * @param[in] x     size in bytes
 *
 ******************************************************************************/
#define LAC_BYTES_TO_QUADWORDS(x) ((x) >> 3)


/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro converts a size in quadwords to the equivalent size in bytes,
 *      using a bit shift to multiply by 8
 *
 * @param[in] x     size in quad words
 *
 ******************************************************************************/
#define LAC_QUADWORDS_TO_BYTES(x) ((x) << 3)

/*
*******************************************************************************
* Assert Macros
*******************************************************************************
*/

/* LAC_ASSERT usage: call with a boolean condition and text to be logged if
 * that condition fails */

/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro checks a condition and if the condition is false, logs an
 *      error message for debug releases but gets compiled out for non-debug
 *      releases
 *
 * @param[in] condition     condition to check
 * @param[in] errorText     Text to printed if condition is false
 *
 ******************************************************************************/
#define LAC_ASSERT(condition, errorText)\
    IX_OSAL_ENSURE((condition), (int)(errorText))

/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro checks a condition and if the condition is false, logs an
 *      error message for debug releases but gets compiled out for non-debug
 *      releases
 *
 * @param[in] condition     condition to check
 * @param[in] errorText     Text to printed if condition is false
 *
 ******************************************************************************/
#define LAC_ENSURE(condition, errorText) \
    LAC_ASSERT((condition), (errorText))

/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This assert checks if an address is not null.
 *      This will be compiled out for non debug releases
 *
 * @param[in] address the address we are checking
 *
 ******************************************************************************/
#define LAC_ASSERT_NOT_NULL(address)\
    LAC_ASSERT((NULL != (address)), "Parameter " #address " is null.\n")

/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This assert checks if an address is not null.
 *      This will be compiled out for non debug releases
 *
 * @param[in] address the address we are checking
 *
 ******************************************************************************/
#define LAC_ENSURE_NOT_NULL(address)\
    LAC_ASSERT_NOT_NULL(address)

/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This assert checks if an address is quad word aligned and is not null.
 *      This will be compiled out for non debug releases
 *
 * @param[in] address the address we are checking
 *
 ******************************************************************************/
#define LAC_QAT_ALIGNED_ASSERT(address)\
    LAC_ASSERT(((NULL != (address))  && \
    (LAC_ADDRESS_ALIGNED((address),LAC_8BYTE_ALIGNMENT_SHIFT))),\
        "Parameter " #address " is not correctly aligned for QAT.\n")

/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This assert checks if an address is quad word aligned and is not null.
 *      This will be compiled out for non debug releases
 *
 * @param[in] address the address we are checking
 *
 ******************************************************************************/
#define LAC_QAT_ALIGNED_ENSURE(address)\
    LAC_QAT_ALIGNED_ASSERT(address)

/******************************************************************************/

/*
*******************************************************************************
* Mutex Macros
*******************************************************************************
*/

/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro tries to acquire a mutex and returns the status
 *
 * @param[in] pLock             Pointer to Lock
 * @param[in] timeout           Timeout
 *
 * @retval CPA_STATUS_SUCCESS   Function executed successfully.
 * @retval CPA_STATUS_RESOURCE  Error with Mutex
 ******************************************************************************/
#define LAC_LOCK_MUTEX(pLock, timeout)                   \
    ( (IX_SUCCESS != ixOsalMutexLock((pLock), (timeout)) \
      ) ? CPA_STATUS_RESOURCE : CPA_STATUS_SUCCESS)

/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro unlocks a mutex and returns the status
 *
 * @param[in] pLock             Pointer to Lock
 *
 * @retval CPA_STATUS_SUCCESS   Function executed successfully.
 * @retval CPA_STATUS_RESOURCE  Error with Mutex
 ******************************************************************************/
#define LAC_UNLOCK_MUTEX(pLock) \
    ( (IX_SUCCESS != ixOsalMutexUnlock((pLock)) \
      ) ? CPA_STATUS_RESOURCE : CPA_STATUS_SUCCESS)


/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro initialises a mutex and returns the status
 *
 * @param[in] pLock             Pointer to Lock
 *
 * @retval CPA_STATUS_SUCCESS   Function executed successfully.
 * @retval CPA_STATUS_RESOURCE  Error with Mutex
 ******************************************************************************/
#define LAC_INIT_MUTEX(pLock) \
    ( (IX_SUCCESS != ixOsalMutexInit((pLock)) \
      ) ? CPA_STATUS_RESOURCE : CPA_STATUS_SUCCESS)


/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro destroys a mutex and returns the status
 *
 * @param[in] pLock             Pointer to Lock
 *
 * @retval CPA_STATUS_SUCCESS   Function executed successfully.
 * @retval CPA_STATUS_RESOURCE  Error with Mutex
 ******************************************************************************/
#define LAC_DESTROY_MUTEX(pLock) \
    ( (IX_SUCCESS != ixOsalMutexDestroy((pLock)) \
      ) ? CPA_STATUS_RESOURCE : CPA_STATUS_SUCCESS)


/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro calls a trylock on a mutex
 *
 * @param[in] pLock             Pointer to Lock
 *
 * @retval CPA_STATUS_SUCCESS   Function executed successfully.
 * @retval CPA_STATUS_RESOURCE  Error with Mutex
 ******************************************************************************/
#define LAC_TRYLOCK_MUTEX(pLock) \
    ( (IX_SUCCESS != ixOsalMutexTryLock((pLock)) \
      ) ? CPA_STATUS_RESOURCE : CPA_STATUS_SUCCESS)


/*
*******************************************************************************
* Semaphore Macros
*******************************************************************************
*/


/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro waits on a semaphore and returns the status
 *
 * @param[in] sid               The semaphore
 * @param[in] timeout           Timeout
 *
 * @retval CPA_STATUS_SUCCESS   Function executed successfully.
 * @retval CPA_STATUS_RESOURCE  Error with semaphore
 ******************************************************************************/
#define LAC_WAIT_SEMAPHORE(sid, timeout)                   \
    ( (IX_SUCCESS != ixOsalSemaphoreWait(&sid, (timeout)) \
      ) ? CPA_STATUS_RESOURCE : CPA_STATUS_SUCCESS)

/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro post a semaphore and returns the status
 *
 * @param[in] sid               The semaphore
 *
 * @retval CPA_STATUS_SUCCESS   Function executed successfully.
 * @retval CPA_STATUS_RESOURCE  Error with semaphore
 ******************************************************************************/
#define LAC_POST_SEMAPHORE(sid) \
    ( (IX_SUCCESS != ixOsalSemaphorePost(&sid) \
      ) ? CPA_STATUS_RESOURCE : CPA_STATUS_SUCCESS)


/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro initialises a semaphore and returns the status
 *
 * @param[in] sid               The semaphore
 * @param[in] semValue          Initial semaphore value
 *
 * @retval CPA_STATUS_SUCCESS   Function executed successfully.
 * @retval CPA_STATUS_RESOURCE  Error with semaphore
 ******************************************************************************/
#define LAC_INIT_SEMAPHORE(sid, semValue) \
    ( (IX_SUCCESS != ixOsalSemaphoreInit(&sid, semValue) \
      ) ? CPA_STATUS_RESOURCE : CPA_STATUS_SUCCESS)


/**
 *******************************************************************************
 * @ingroup LacCommon
 *      This macro destroys a semaphore and returns the status
 *
 * @param[in] sid               The semaphore
 *
 * @retval CPA_STATUS_SUCCESS   Function executed successfully.
 * @retval CPA_STATUS_RESOURCE  Error with semaphore
 ******************************************************************************/
#define LAC_DESTROY_SEMAPHORE(sid) \
    ( (IX_SUCCESS != ixOsalSemaphoreDestroy(&sid) \
      ) ? CPA_STATUS_RESOURCE : CPA_STATUS_SUCCESS)


/*
*******************************************************************************
* Spinlock Macros
*******************************************************************************
*/

#ifdef ENABLE_SPINLOCK
typedef IxOsalSpinLock lac_lock_t;

#define LAC_SPINLOCK_INIT(lock) (void)ixOsalSpinLockInit(lock, TYPE_IGNORE)
#if defined(__linux)
#define LAC_SPINLOCK_IS_INITIALISED(lock) (0 == 1)
#define LAC_SPINLOCK(lock)      (void)ixOsalSpinLockLockBh(lock)
#define LAC_SPINUNLOCK(lock)    (void)ixOsalSpinLockUnlockBh(lock)
#define LAC_SPINLOCK_DESTROY(lock)
#elif defined(__freebsd)
#define LAC_SPINLOCK_IS_INITIALISED(lock) (lock != NULL)
#define LAC_SPINLOCK(lock)      (void)ixOsalSpinLockLock(lock)
#define LAC_SPINUNLOCK(lock)    (void)ixOsalSpinLockUnlock(lock)
#define LAC_SPINLOCK_DESTROY(lock)  (void)ixOsalSpinLockDestroy(lock)
#endif

#else

typedef IxOsalMutex lac_lock_t;

#define LAC_SPINLOCK_INIT   LAC_INIT_MUTEX      
#define LAC_SPINLOCK(lock)  LAC_LOCK_MUTEX(lock, IX_OSAL_WAIT_FOREVER)
#define LAC_SPINUNLOCK      LAC_UNLOCK_MUTEX
#define LAC_SPINLOCK_DESTROY(lock)
#define LAC_SPINLOCK_IS_INITIALISED(lock) (0 == 1)

#endif

#define LAC_CONST_PTR_CAST(castee) \
                 IX_OSAL_CONST_PTR_CAST(castee)

#define LAC_CONST_VOLATILE_PTR_CAST(castee) \
                 IX_OSAL_CONST_VOLATILE_PTR_CAST(castee)


CpaStatus
cpaCyStartInstance(CpaInstanceHandle instanceHandle);

CpaStatus
cpaCyStopInstance(CpaInstanceHandle instanceHandle);

CpaStatus
LacInit_ResponsesPending(CpaInstanceHandle instanceHandle,
              CpaBoolean *pNumRespPending);

#endif /* LAC_COMMON_H */
