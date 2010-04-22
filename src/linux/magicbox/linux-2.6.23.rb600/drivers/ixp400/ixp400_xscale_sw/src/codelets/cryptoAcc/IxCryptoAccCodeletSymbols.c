/**
 * @file IxCryptoAccCodeletSymbols.c
 *
 * @date 25-Apr-2005
 *
 * @brief This file declares exported symbols for linux kernel module builds.
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

#ifdef __linux

#include <linux/autoconf.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <asm/semaphore.h>

#include "IxTypes.h"
#include "IxCryptoAccCodelet.h"
#include "IxCryptoAcc.h"

EXPORT_SYMBOL(ixCryptoAccCodeletMain);

UINT32 serviceIndex;
UINT32 dataBLenOrOprWLen;


MODULE_PARM(serviceIndex, "i");
MODULE_PARM_DESC(serviceIndex,
                 "0 : Lists all set of services demonstrated."
                 "1 : Encryption and Decryption using DES(ECB)"
                 "2 : Encryption and Decryption using DES(CBC)"
                 "3 : Encryption and Decryption using 3DES(ECB)"
                 "4 : Encryption and Decryption using 3DES(CBC)"
                 "5 : Encryption and Decryption using AES(ECB)"
                 "6 : Encryption and Decryption using AES(CBC)"
                 "7 : Encryption and Decryption using AES(CTR)"
                 "8 : Encryption and Decryption using AES-CCM"
                 "9 : Encryption and Decryption using ARC4 on XScale"
                 "10 : Encryption and Decryption using ARC4 on WAN-NPE"
                 "11 : Authentication calculation and verification using MD5"
                 "12 : Authentication calculation and verification using SHA1"
                 "13 : Authentication calculation and verification WEP CRC on XScale"
                 "14 : Authentication calculation and verification WEP CRC on WAN-NPE"
                 "15 : A combined mode of operation using DES(ECB) + MD5"
                 "16 : A combined mode of operation using DES(CBC) + MD5"
                 "17 : A combined mode of operation using DES(ECB) + SHA1"
                 "18 : A combined mode of operation using DES(CBC) + SHA1"
                 "19 : A combined mode of operation using 3DES(ECB) + MD5"
                 "20 : A combined mode of operation using 3DES(CBC) + MD5"
                 "21 : A combined mode of operation using 3DES(ECB) + SHA1"
                 "22 : A combined mode of operation using 3DES(CBC) + SHA1"
                 "23 : A combined mode of operation using AES(ECB) + MD5"
                 "24 : A combined mode of operation using AES(CBC) + MD5"
                 "25 : A combined mode of operation using AES(CTR) + MD5"
                 "26 : A combined mode of operation using AES(ECB) + SHA1"
                 "27 : A combined mode of operation using AES(CBC) + SHA1"
                 "28 : A combined mode of operation using AES(CTR) + SHA1"
                 "29 : A combined mode of operation using ARC4 + WEP CRC on XScale"
                 "30 : A combined mode of operation using ARC4 + WEP CRC on WAN-NPE"
                 "31 : PKE RNG pseudo-random number"
                 "32 : PKE SHA1 hashing operation"
                 "33 : PKE EAU modular exponential operation"
                 "34 : PKE EAU large number modular reduction operation"
                 "35 : PKE EAU large number addition operation"
                 "36 : PKE EAU large number subtraction operation"
                 "37 : PKE EAU large number multiplication operation");

MODULE_PARM(dataBLenOrOprWLen, "i");
MODULE_PARM_DESC(dataBLenOrOprWLen,         
        "Data length ranges from 64 bytes to 65456 bytes, if cipher algorithm\n"
        "is DES/3DES, data length must be multiple of 8-byte (cipher block\n"
        "length); while AES algorithm must have data length that is multiple\n"
        "of 16-byte in size. For ARC4 algorithm block length there is no such\n"
        "restriction (because block size is 1 byte). If cipher mode is CCM,\n"
        "there is no restriction on the data length being a multiple of cipher\n"
        "block length.\n\n"
        "For PKE EAU, RNG and SHA operands' range, please refer to the\n"
        "IxCryptoAcc.h file.\n\n"
        "For EAU operation, each operand length allocates 8 bits. All others\n"
        "operations take the whole word of dataBLenOrOprWLen.\n"
        "For example: WLen = 0x00ccbbaa.\n"
        "modular exponential: aa is M, bb is N, and cc is e\n"
        "large number modular reduction: aa is A, bb is N\n"
        "large number add/sub/mul: aa is A, bb is B\n"
        "others: 0x00ccbbaa\n");
         
static int __init CryptoInitModule(void)
{
    printk("Loading CryptoAcc Codelet...\n");
    if (IX_SUCCESS != ixCryptoAccCodeletMain (serviceIndex, dataBLenOrOprWLen))
        return -1;
    else
        return 0;
}

static void __exit CryptoExitModule(void)
{
    printk("Unloading CryptoAcc Codelet\n");
}

module_init(CryptoInitModule);
module_exit(CryptoExitModule);


#endif
