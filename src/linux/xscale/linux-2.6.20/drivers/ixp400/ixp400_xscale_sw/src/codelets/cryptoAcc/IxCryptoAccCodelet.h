/**
 * @file IxCryptoAccCodelet.h
 *
 * @date April-20-2005
 *
 * @brief This is the header file for the Crypto Access Component Codelet.
 *
 * 
 * @par
 * IXP400 SW Release Crypto version 2.3
 * 
 * -- Copyright Notice --
 * 
 * @par
 * Copyright (c) 2001-2005, Intel Corporation.
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

/**
 * @defgroup Codelets Intel (R) IXP400 Software Codelets
 *
 * @brief Intel (R) IXP400 Software codelets
 *
 * @{
 */

/**
 * @ingroup Codelets
 *
 * @defgroup IxCryptoAccCodelet Intel (R) IXP400 Software Crypto Access Codelet
 *           (IxCryptoAccCodelet) API
 *
 * @brief Intel (R) IXP400 Software Crypto Access Codelet API
 *
 * The codelet demonstrates how the Security Hardware Accelerator API
 * can be used for performing various cryptographic operations.
 * For demonstration purposes, various cryptographic operations (called services
 * in this codelet) are preconfigured in an array. Users can specify
 * a particular service with a "service index" from the list below. 
 * Various services shown are:
 *
 * <PRE>
 *    - Encryption and decryption, using DES ECB
 *    - Authentication calculation and check, using MD5
 *    - Combined encryption and decryption, using DES CBC and SHA1
 *    - CCM for 802.11i
 *    - ARC4 with WEP CRC computations on XScale
 *    - ARC4 with WEP CRC computations on NPE
 *    - Pseudo-random number generator (RNG) on PKE Crypto Engine
 *    - Hashing (SHA1) on PKE Crypto Engine
 *    - Arithmetic operations (modular exponential, big number modular reduction,
 *      addition, subtraction, and multiplication) on PKE Crypto Engine.
 * </PRE>
 *
 * The codelet shows how to initialize the required components (like Qmgr, NPE's)
 * and the Security Hardware Acclerator. This initialization
 * is done only once for the very first request. Every invocation of the codelet
 * for a specific service results in the context registration followed by
 * execution of the operation. Once the operation completes, the contexts 
 * are un-registered. If the operation completes successfully performance 
 * statistics are printed. If there is an error, statistics are not printed
 * and an error message is displayed.
 * The codelet shows the following :
 *
 * <PRE>
 *       - The codelet demonstrates how the Network Processor Engine (NPE) 
 * is initialised, how the NPE image is downloaded to the NPE, and 
 * and how the NPE is started.
 *
 *       - The codelet demonstrates how the Queue Manager is initialised to
 * polling or interrupt mode.
 *
 *       - The codelet shows how to set up the callback functions and 
 * how to register crypto contexts with cryptoAcc access component.
 *
 *       - The codelet demonstrates how the registration should be done for
 * the different operations using the register API.
 *
 *       - The codelet demonstrates how the perform API can be used after
 * successful registration.
 *
 *       - The codelet demonstrates how to use XScaleWepPerform function
 * and NpeWepPerform function
 *
 *       - The codelet demonstrates the use of the unregister API in the
 * event of re-starting the above mentioned operation.
 *
 *       - The codelet demonstrates how to use PKE API for RNG, SHA, and EAU and 
 * how to setup callback functions.
 *
 *       - The codelet measures the performance of the Security Hardware
 * Accelerator for each of the operations listed above.
 * 
 * </PRE>
 * 
 * Configuration parameters for Performance benchmarking are
 *
 * <B>Default Performance Sampling Size</B>
 * <PRE>
 *    - Sampling rate            : 1,000 (100 for EAU)
 *    - Number of Samples        : 20
 *    - Total Samples            : 20,000 (2,000 for EAU)
 * </PRE>
 *
 * <B>Default mbuf Pool Setting</B>
 * <PRE>
 *    - Number of mbufs in pool     : 20
 *    - Min of mbufs needed in pool : 2 (for crypto context registration 
 *                                    purpose)
 * </PRE>
 *
 * User may choose to run different operation with different data/operand lengths. 
 * The performance rate of a selected operation with the selected length 
 * will be displayed after throughput rate has been captured for a predefined 
 * performance sampling size.If an error occurs any time in the codelet, the  
 * performance numbers will not be displayed and error message will be
 * printed.
 *
 * <B>Notes for Performance Benchmarking:</B>
 * <PRE>
 *       - Data are generated in XScale with random payload. No explicit
 * verification of payload will be carried out in the codelet.
 *
 *       - Encrypted data are sent to NPE again for decryption and same
 * applies to authentication operation. Data payload is verified implicitly
 * through this feedback system.
 *
 *       - Throughput rate is captured from the point where data is sent to
 * NPE for processing to the point where NPE completes the encryption and/or
 * authentication operation and notifies XScale through callback.
 *
 *       - No external data generator or benchmarking devices involved in this
 * codelet. Throughput rate is captured using XScale timestamp.
 *
 *       - Performance of PKE functions are captured from the time the function
 * is called until the PKE has finished the operations and callback is invoked.
 *
 *       - Time taken for codelet to complete the operation depends on the
 * length selected. Larger data will take more time to complete.
 * </PRE>
 *
 * <B> VxWorks* User Guide </B><BR>
 * ixCryptoAccCodeletMain() function is used as the entry point of execution
 * for cryptoAcc Codelet. <BR>
 * It allows user to enter selection for various operations described below
 * with different data/operand length.
 *
 * <PRE>
 *  <I> Usage :
 *      >ixCryptoAccCodeletMain (serviceIndex, dataBLenOrOprWLen)
 *      Where serviceIndex should be one of the following:
 *
 *            0 : Lists the set of services demonstrated.
 *            1 : Encryption and Decryption using DES(ECB)
 *            2 : Encryption and Decryption using DES(CBC)
 *            3 : Encryption and Decryption using 3DES(ECB)
 *            4 : Encryption and Decryption using 3DES(CBC)
 *            5 : Encryption and Decryption using AES(ECB)
 *            6 : Encryption and Decryption using AES(CBC)
 *            7 : Encryption and Decryption using AES(CTR)
 *            8 : Encryption and Decryption using AES-CCM
 *            9 : Encryption and Decryption using ARC4 on XScale
 *            10 : Encryption and Decryption using ARC4 on WAN-NPE
 *            11 : Authentication calculation and verification using MD5
 *            12 : Authentication calculation and verification using SHA1
 *            13 : Authentication calculation and verification WEP CRC on XScale
 *            14 : Authentication calculation and verification WEP CRC on WAN-NPE
 *            15 : A combined mode of operation using DES(ECB) + MD5
 *            16 : A combined mode of operation using DES(CBC) + MD5
 *            17 : A combined mode of operation using DES(ECB) + SHA1
 *            18 : A combined mode of operation using DES(CBC) + SHA1
 *            19 : A combined mode of operation using 3DES(ECB) + MD5
 *            20 : A combined mode of operation using 3DES(CBC) + MD5
 *            21 : A combined mode of operation using 3DES(ECB) + SHA1
 *            22 : A combined mode of operation using 3DES(CBC) + SHA1
 *            23 : A combined mode of operation using AES(ECB) + MD5
 *            24 : A combined mode of operation using AES(CBC) + MD5
 *            25 : A combined mode of operation using AES(CTR) + MD5
 *            26 : A combined mode of operation using AES(ECB) + SHA1
 *            27 : A combined mode of operation using AES(CBC) + SHA1
 *            28 : A combined mode of operation using AES(CTR) + SHA1
 *            29 : A combined mode of operation using ARC4 + WEP CRC on XScale
 *            30 : A combined mode of operation using ARC4 + WEP CRC on WAN-NPE
 *            31 : PKE RNG pseudo-random number
 *            32 : PKE SHA1 hashing operation
 *            33 : PKE EAU modular exponential operation
 *            34 : PKE EAU large number modular reduction operation
 *            35 : PKE EAU large number addition operation
 *            36 : PKE EAU large number subtraction operation
 *            37 : PKE EAU large number multiplication operation
 *
 *      Where dataBLenOrOprWLen (data byte length or operand word length):
 *           Data length ranges from 64 bytes to 65456 bytes, if cipher
 *           algorithm is DES/3DES, data length must be multiple of 8-byte
 *           (cipher block length); while AES algorithm must have data length
 *           that is multiple of 16-byte in size. For ARC4 algorithm block length
 *           there is no such restriction (because block size is 1 byte).
 *           If cipher mode is CCM, there is no restriction on the data length
 *           being a multiple of cipher block length.
 *
 *           For PKE EAU, RNG and SHA operands' range, please refer to the
 *           IxCryptoAcc.h file.
 *
 *           For EAU operation, each operand length allocates 8 bits. All others
 *           operations take the whole word of dataBLenOrOprWLen.
 *           For example: WLen = 0x00ccbbaa. 
 *           modular exponential: aa is M, bb is N, and cc is e
 *           large number modular reduction: aa is A, bb is N
 *           large number add/sub/mul: aa is A, bb is B
 *           others: 0x00ccbbaa
 *
 *
 * </I>
 * </PRE>
 *
 * <B> Linux* User Guide </B><BR>
 * The ixCryptoAccCodeletMain() is the entry point of cryptoAcc codelet. <BR>
 * The selected operation will be executed when user issues 'insmod' at command
 * prompt.
 *
 * <PRE>
 * <I>  Usage :
 *      >insmod ixp400_codelets_cryptoAcc.o serviceIndex=<x> dataBLenOrOprWLen=<y>
 *      Where x should be one of the following numbers:
 *
 *             0 : Lists all set of services demonstrated.
 *            1 : Encryption and Decryption using DES(ECB)
 *            2 : Encryption and Decryption using DES(CBC)
 *            3 : Encryption and Decryption using 3DES(ECB)
 *            4 : Encryption and Decryption using 3DES(CBC)
 *            5 : Encryption and Decryption using AES(ECB)
 *            6 : Encryption and Decryption using AES(CBC)
 *            7 : Encryption and Decryption using AES(CTR)
 *            8 : Encryption and Decryption using AES-CCM
 *            9 : Encryption and Decryption using ARC4 on XScale
 *            10 : Encryption and Decryption using ARC4 on WAN-NPE
 *            11 : Authentication calculation and verification using MD5
 *            12 : Authentication calculation and verification using SHA1
 *            13 : Authentication calculation and verification WEP CRC on XScale
 *            14 : Authentication calculation and verification WEP CRC on WAN-NPE
 *            15 : A combined mode of operation using DES(ECB) + MD5
 *            16 : A combined mode of operation using DES(CBC) + MD5
 *            17 : A combined mode of operation using DES(ECB) + SHA1
 *            18 : A combined mode of operation using DES(CBC) + SHA1
 *            19 : A combined mode of operation using 3DES(ECB) + MD5
 *            20 : A combined mode of operation using 3DES(CBC) + MD5
 *            21 : A combined mode of operation using 3DES(ECB) + SHA1
 *            22 : A combined mode of operation using 3DES(CBC) + SHA1
 *            23 : A combined mode of operation using AES(ECB) + MD5
 *            24 : A combined mode of operation using AES(CBC) + MD5
 *            25 : A combined mode of operation using AES(CTR) + MD5
 *            26 : A combined mode of operation using AES(ECB) + SHA1
 *            27 : A combined mode of operation using AES(CBC) + SHA1
 *            28 : A combined mode of operation using AES(CTR) + SHA1
 *            29 : A combined mode of operation using ARC4 + WEP CRC on XScale
 *            30 : A combined mode of operation using ARC4 + WEP CRC on WAN-NPE
 *            31 : PKE RNG pseudo-random number
 *            32 : PKE SHA1 hashing operation
 *            33 : PKE EAU modular exponential operation
 *            34 : PKE EAU large number modular reduction operation
 *            35 : PKE EAU large number addition operation
 *            36 : PKE EAU large number subtraction operation
 *            37 : PKE EAU large number multiplication operation
 *
 *      Where dataBLenOrOprWLen (data byte length or operand word length):
 *           Data length ranges from 64 bytes to 65456 bytes, if cipher
 *           algorithm is DES/3DES, data length must be multiple of 8-byte
 *           (cipher block length); while AES algorithm must have data length
 *           that is multiple of 16-byte in size. For ARC4 algorithm block length
 *           there is no such restriction (because block size is 1 byte).
 *           If cipher mode is CCM, there is no restriction on the data length
 *           being a multiple of cipher block length.
 *
 *           For PKE EAU, RNG and SHA operands' range, please refer to the
 *           IxCryptoAcc.h file.
 *
 *           For EAU operation, each operand length allocates 8 bits. All other
 *           operations take the whole word of dataBLenOrOprWLen.
 *           For example: WLen = 0x00ccbbaa. 
 *           modular exponential: aa is M, bb is N, and cc is e
 *           large number modular reduction: aa is A, bb is N
 *           large number add/sub/mul: aa is A, bb is B
 *           others: 0x00ccbbaa                  
 *
 * </I>
 * </PRE>
 *
 * @{
 */
#ifndef IxCryptoAccCodelet_H
#define IxCryptoAccCodelet_H

#include "IxOsal.h"
#include "IxNpeDl.h"

#define INLINE __inline__

/**************************************************************************
 *        USER SETTING - CryptoAcc Configuration and NPE BUILD ID's.
 **************************************************************************/

/**
 * @ingroup IxCryptoAccCodelet
 *
 * @def  IX_CRYPTOACC_CODELET_CRYPTOACC_CFG
 *
 * @brief Selects the interface's (to the NPE) to be initialized by the codelet.
 *        Use one of the values defined in IxCryptoAccCfg.
 *        IX_CRYPTO_ACC_CFG_CRYPTO_WEP_NPE_ACC_EN: Enable's access to the Crypto
 *        NPE and WEP NPE.
 */
#define IX_CRYPTOACC_CODELET_CRYPTOACC_CFG  (IX_CRYPTO_ACC_CFG_CRYPTO_WEP_NPE_ACC_EN)


/**
 * @ingroup IxCryptoAccCodelet
 *
 * @def IX_CRYPTOACC_CODELET_NPEC_IMAGE_ID
 *
 * @brief The build ID for NPE C image, default is
 *        IX_NPEDL_NPEIMAGE_NPEC_CRYPTO_AES_CCM_ETH 
 *
 * @note Please refer to IxNpeDl.h for NPE image definition
 *
 */
#define IX_CRYPTOACC_CODELET_NPEC_IMAGE_ID IX_NPEDL_NPEIMAGE_NPEC_CRYPTO_AES_CCM_ETH


/**
 * @ingroup IxCryptoAccCodelet
 *
 * @def IX_CRYPTOACC_CODELET_NPEA_IMAGE_ID
 * 
 * @brief The build ID for NPE A image, default is IX_NPEDL_NPEIMAGE_NPEA_WEP
 *
 * @note Please refer to IxNpeDl.h for NPE image definition
 *
 */
#define IX_CRYPTOACC_CODELET_NPEA_IMAGE_ID  IX_NPEDL_NPEIMAGE_NPEA_WEP


/**************************************************************************
 *                 PERFORMANCE BENCHMARKING SETTING
 **************************************************************************/

/**
 * @ingroup IxCryptoAccCodelet
 *
 * @def PERFORMANCE_WINDOW_SIZE
 *
 * @brief Number of crypto packets/requests need to be sent in for performance 
 *        benchmarking (sampling size), 1 unit represents 1000 of requests 
 *        (100 requests for EAU)
 */
#define PERFORMANCE_WINDOW_SIZE                        20

/**
 * @ingroup IxCryptoAccCodelet
 *
 * @def IX_CRYPTOACC_CODELET_CRYPTO_MBUF_POOL_SIZE
 *
 * @brief Size of MBuf pool, ie number of buffers to circulate 
 *
 */
#define IX_CRYPTOACC_CODELET_CRYPTO_MBUF_POOL_SIZE     20


/**************************************************************************
*                           SYSTEM SETTING 
**************************************************************************/

/**
 * @ingroup IxCryptoAccCodelet
 *  
 * @def   IX_CRYPTOACC_CODELET_BATCH_LEN
 *
 * @brief Number of packets/requests per sampling for performance benchmarking 
 *
 */
#define IX_CRYPTOACC_CODELET_BATCH_LEN              1000

/**
 * @ingroup IxCryptoAccCodelet
 * 
 * @def   IX_CRYPTOACC_CODELET_QMGR_DISPATCHER_PRIORITY
 *
 * @brief Recommended priority of queue manager dispatch loop 
 *
 */
#define IX_CRYPTOACC_CODELET_QMGR_DISPATCHER_PRIORITY    150 

/**
 * @ingroup IxCryptoAccCodelet
 * 
 * @def   IX_CRYPTOACC_CODELET_QMGR_DISPATCHER_THREAD_STACK_SIZE
 *
 * @brief Recommended stack size for queue manager dispatcher thread 
 *
 */
#define IX_CRYPTOACC_CODELET_QMGR_DISPATCHER_THREAD_STACK_SIZE 10240

/**
 * @ingroup IxCryptoAccCodelet
 *
 * @def   IX_CRYPTOACC_CODELET_XSCALE_TICK
 *
 * @brief The XScale tick is 66 MHz. The ixOsalOsIxp400TimestampResolutionGet()
 *        function returns 66666666, thus need to divide by a million.
 *
 */
#define IX_CRYPTOACC_CODELET_XSCALE_TICK \
        (ixOsalOsIxp400TimestampResolutionGet()/1000000)

/**
 * @ingroup IxCryptoAccCodelet
 * 
 * @def   IX_CRYPTOACC_CODELET_REGISTER_WAIT_TIME
 *
 * @brief The codelet register wait time (in ms) in a for loop 
 *
 */
#define IX_CRYPTOACC_CODELET_REGISTER_WAIT_TIME     500

/**
 * @ingroup IxCryptoAccCodelet
 *
 * @def   IX_CRYPTOACC_CODELET_PERFORM_WAIT_TIME
 * 
 * @brief The codelet perform wait time (in ms) in a for loop 
 *
 */
#define IX_CRYPTOACC_CODELET_PERFORM_WAIT_TIME      500

/**
 * @ingroup IxCryptoAccCodelet
 * 
 * @def   IX_CRYPTOACC_CODELET_MAX_TIME_WAIT
 *
 * @brief Maximum time (in ms) to wait before exiting the program
 *
 */
#define IX_CRYPTOACC_CODELET_MAX_TIME_WAIT          200

/**
 * @ingroup IxCryptoAccCodelet
 * 
 * @def   IX_CRYPTO_ACC_CODELET_PKE_PERFORM_WAIT_TIME
 *
 * @brief Maximum time (in ms) to wait before exiting the program
 *
 */
#define IX_CRYPTO_ACC_CODELET_PKE_PERFORM_WAIT_TIME 100

/**
 * @ingroup IxCryptoAccCodelet
 * 
 * @def   IX_CRYPTO_ACC_CODELET_MAX_COUNT
 *
 * @brief Maximum counter to wait before exiting the program
 *
 */
#define IX_CRYPTO_ACC_CODELET_MAX_COUNT             100       

/**
 * @ingroup IxCryptoAccCodelet
 * 
 * @def   IX_CRYPTO_ACC_CODELET_PKE_EAU_TOTAL_OP
 *
 * @brief Total Pke Eau operation for one window size
 *
 */
#define IX_CRYPTO_ACC_CODELET_PKE_EAU_TOTAL_OP      100

/**
 * @ingroup IxCryptoAccCodelet
 * 
 * @def   IX_CRYPTO_ACC_CODELET_PKE_TOTAL_OP
 *
 * @brief Total Pke RNG/SHA operation for one window size
 *
 */
#define IX_CRYPTO_ACC_CODELET_PKE_TOTAL_OP          1000

/**
 * @ingroup IxCryptoAccCodelet
 * 
 * @def   IX_CRYPTO_ACC_CODELET_SLEEP_WAIT
 *
 * @brief Delay time (in ms) for printf to avoid rpc error
 *
 */
#define IX_CRYPTO_ACC_CODELET_SLEEP_WAIT            5

/**
 * @ingroup IxCryptoAccCodelet
 *
 * @def   IX_CRYPTOACC_CODELET_MIN_CRYPTO_PKT_SIZE
 *
 * @brief The minimum packet/data size 
 *
 */
#define IX_CRYPTOACC_CODELET_MIN_CRYPTO_PKT_SIZE    64

/**
 * @ingroup IxCryptoAccCodelet
 *
 * @def   IX_CRYPTOACC_CODELET_MAX_CRYPTO_PKT_SIZE
 *
 * @brief The maximum packet/data size 
 *
 */
#define IX_CRYPTOACC_CODELET_MAX_CRYPTO_PKT_SIZE    65456

/**
 * @ingroup IxCryptoAccCodelet
 *
 * @def IX_CRYPTOACC_CODELET_QMGR_DISPATCHER_MODE
 * 
 * @brief QMgr Dispatcher Mode, whether in interrupt (TRUE) or poll (FALSE) mode
 *
 * Note: <BR>
 * 1. QMgr dispatcher can be run in both poll mode and interrupt mode in 
 *    vxWorks platform. 
 *
 * 2. In linux platform (with Linux* kernel 2.4 and below), QMgr Dispatcher is 
 *    NOT advisible to run in poll mode. Task scheduling in poll mode runs
 *    tasks sequentially (non-preemptive) that cause the performance rate to  
 *    drop tremendously. Thus the performance rate displayed in Linux* platform  
 *    using poll mode is not accurate. Performance rate is more accurate when
 *    QMgr dispatcher is running in interrupt mode.
 */
#ifdef __linux
/* use interrupts for performances */
#define IX_CRYPTOACC_CODELET_QMGR_DISPATCHER_MODE          (TRUE)
#elif __wince
#define IX_CRYPTOACC_CODELET_QMGR_DISPATCHER_MODE          (TRUE)
#else
/* use polled mode for performances */
#define IX_CRYPTOACC_CODELET_QMGR_DISPATCHER_MODE          (FALSE)
#endif

/**
 * @ingroup IxCryptoAccCodelet
 *
 * @def IX_CRYPTOACC_PKE_EAU_EXP_CONFIG_FE_MODE
 * 
 * Set the PKE EAU modular exponential configuration for fast exponent
 *
 */
#define IX_CRYPTOACC_PKE_EAU_EXP_CONFIG_FE_MODE             (FALSE)

/**
 * @ingroup IxCryptoAccCodelet
 *
 * @def IX_CRYPTOACC_PKE_EAU_EXP_CONFIG_SE_MODE
 * 
 * Set the PKE EAU modular exponential configuration for short exponent
 *
 */
#define IX_CRYPTOACC_PKE_EAU_EXP_CONFIG_SE_MODE             (FALSE)


/**************************************************************************
 *                           Function Prototype 
 **************************************************************************/
        
/**
 * @ingroup IxCryptoAccCodelet
 *
 * @fn ixCryptoAccCodeletMain (
           INT32  srvIndex,
           UINT32 dataBLenOrOprWLen)
 *
 * @brief  This is the entry point function to the codelet to choose the 
 *         service for the codelet and data/operand length to be used.
 *         This is the main function of the codelet where crypto contexts
 *         registration and crypto perform services are done. Based on the
 *         selected operation and length, data/operands are sent to
 *         cryptoAcc for processing.
 *
 * @param srvIndex INT32 [in] - type of service to be invoked
 *                   choose from one of the following pre-configured values.
 * <PRE>
 *       0 : Lists all set of services demonstrated.
 *       1 : Encryption and Decryption using DES(ECB)
 *       2 : Encryption and Decryption using DES(CBC)
 *       3 : Encryption and Decryption using 3DES(ECB)
 *       4 : Encryption and Decryption using 3DES(CBC)
 *       5 : Encryption and Decryption using AES(ECB)
 *       6 : Encryption and Decryption using AES(CBC)
 *       7 : Encryption and Decryption using AES(CTR)
 *       8 : Encryption and Decryption using AES-CCM
 *       9 : Encryption and Decryption using ARC4 on XScale
 *       10 : Encryption and Decryption using ARC4 on WAN-NPE
 *       11 : Authentication calculation and verification using MD5
 *       12 : Authentication calculation and verification using SHA1
 *       13 : Authentication calculation and verification WEP CRC on XScale
 *       14 : Authentication calculation and verification WEP CRC on WAN-NPE
 *       15 : A combined mode of operation using DES(ECB) + MD5
 *       16 : A combined mode of operation using DES(CBC) + MD5
 *       17 : A combined mode of operation using DES(ECB) + SHA1
 *       18 : A combined mode of operation using DES(CBC) + SHA1
 *       19 : A combined mode of operation using 3DES(ECB) + MD5
 *       20 : A combined mode of operation using 3DES(CBC) + MD5
 *       21 : A combined mode of operation using 3DES(ECB) + SHA1
 *       22 : A combined mode of operation using 3DES(CBC) + SHA1
 *       23 : A combined mode of operation using AES(ECB) + MD5
 *       24 : A combined mode of operation using AES(CBC) + MD5
 *       25 : A combined mode of operation using AES(CTR) + MD5
 *       26 : A combined mode of operation using AES(ECB) + SHA1
 *       27 : A combined mode of operation using AES(CBC) + SHA1
 *       28 : A combined mode of operation using AES(CTR) + SHA1
 *       29 : A combined mode of operation using ARC4 + WEP CRC on XScale
 *       30 : A combined mode of operation using ARC4 + WEP CRC on WAN-NPE
 *       31 : PKE RNG pseudo-random number
 *       32 : PKE SHA1 hashing operation
 *       33 : PKE EAU modular exponential operation
 *       34 : PKE EAU large number modular reduction operation
 *       35 : PKE EAU large number addition operation
 *       36 : PKE EAU large number subtraction operation
 *       37 : PKE EAU large number multiplication operation
 * </PRE>
 * @param dataBLenOrOprWLen UINT32 [in] - data length in bytes or operand length
 *                                        in words
 *
 * @return  IX_STATUS
 *          @li IX_SUCCESS - codelet runs successfully
 *          @li IX_FAIL    - codelet fails
 */
IX_STATUS ixCryptoAccCodeletMain (
               INT32  srvIndex,
               UINT32 dataBLenOrOprWLen);

#endif /* IxCryptoAccCodelet_H */

/** @} defgroup IxCryptoAccCodelet*/

/** @} defgroup Codelet*/

