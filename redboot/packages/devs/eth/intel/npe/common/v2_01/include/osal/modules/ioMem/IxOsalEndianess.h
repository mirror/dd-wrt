/** 
 * @file IxOsalEndianess.h (Obsolete file) 
 * 
 * @brief Header file for determining system endianess and OS
 * 
 * @par
 * @version $Revision: 1.1
 * 
 * @par
 * IXP400 SW Release version  2.0
 * 
 * -- Intel Copyright Notice --
 * 
 * @par
 * Copyright 2002-2005 Intel Corporation All Rights Reserved.
 * 
 * @par
 * The source code contained or described herein and all documents
 * related to the source code ("Material") are owned by Intel Corporation
 * or its suppliers or licensors.  Title to the Material remains with
 * Intel Corporation or its suppliers and licensors.
 * 
 * @par
 * The Material is protected by worldwide copyright and trade secret laws
 * and treaty provisions. No part of the Material may be used, copied,
 * reproduced, modified, published, uploaded, posted, transmitted,
 * distributed, or disclosed in any way except in accordance with the
 * applicable license agreement .
 * 
 * @par
 * No license under any patent, copyright, trade secret or other
 * intellectual property right is granted to or conferred upon you by
 * disclosure or delivery of the Materials, either expressly, by
 * implication, inducement, estoppel, except in accordance with the
 * applicable license agreement.
 * 
 * @par
 * Unless otherwise agreed by Intel in writing, you may not remove or
 * alter this notice or any other notice embedded in Materials by Intel
 * or Intel's suppliers or licensors in any way.
 * 
 * @par
 * For further details, please see the file README.TXT distributed with
 * this software.
 * 
 * @par
 * -- End Intel Copyright Notice --
 */


#ifndef IxOsalEndianess_H
#define IxOsalEndianess_H

#if defined (__vxworks) || defined (__linux)

/* get ntohl/ntohs/htohl/htons macros and CPU definitions for VxWorks */
/* #include <netinet/in.h>  */

#elif defined (__wince)

/* get ntohl/ntohs/htohl/htons macros definitions for WinCE */
#include <Winsock2.h>

#elif defined(__ECOS)

#include <pkgconf/system.h>

#ifdef CYGPKG_REDBOOT
#include <redboot.h>
#include <net/net.h>
#else
#error "Need something for non-RedBoot case!"
#endif

#else

#error Unknown OS, please add a section with the include file for htonl/htons/ntohl/ntohs

#endif /* vxworks or linux or wince */

/* Compiler specific endianness selector - WARNING this works only with arm gcc, use appropriate defines with diab */

#ifndef __wince

#if defined (__ARMEL__)

#ifndef __LITTLE_ENDIAN

#define __LITTLE_ENDIAN

#endif /* _LITTLE_ENDIAN */

#elif defined (__ARMEB__) || CPU == SIMSPARCSOLARIS

#ifndef __BIG_ENDIAN

#define __BIG_ENDIAN

#endif /* __BIG_ENDIAN */

#else

#error Error, could not identify target endianness

#endif /* endianness selector no WinCE OSes */

#else /* ndef __wince */

#define __LITTLE_ENDIAN

#endif /* def __wince */


/* OS mode selector */
#if defined (__vxworks) && defined (__LITTLE_ENDIAN)

#define IX_OSAL_VXWORKS_LE

#elif defined (__vxworks) && defined (__BIG_ENDIAN)

#define IX_OSAL_VXWORKS_BE

#elif defined (__linux) && defined (__BIG_ENDIAN)

#define IX_OSAL_LINUX_BE

#elif defined (__linux) && defined (__LITTLE_ENDIAN)

#define IX_OSAL_LINUX_LE

#elif defined (BOOTLOADER_BLD) && defined (__LITTLE_ENDIAN)

#define IX_OSAL_EBOOT_LE

#elif defined (__wince) && defined (__LITTLE_ENDIAN)

#define IX_OSAL_WINCE_LE

#elif defined (__ECOS) && defined (__BIG_ENDIAN)

#define IX_OSAL_ECOS_BE

#elif defined (__ECOS) && defined (__LITTLE_ENDIAN)

#define IX_OSAL_ECOS_LE

#else

#error Unknown OS/Endianess combination - only vxWorks BE LE, Linux BE LE, WinCE BE LE are supported

#endif /* mode selector */



#endif /* IxOsalEndianess_H */
