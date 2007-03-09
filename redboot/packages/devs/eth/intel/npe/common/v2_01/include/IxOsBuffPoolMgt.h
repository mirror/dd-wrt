/**
 * @file IxOsBuffPoolMgt.h (Replaced by OSAL)
 *
 * @date 9 Oct 2002
 *
 * @brief This file contains the mbuf pool implementation API
 *
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
 * This module contains the implementation of the OS Services buffer pool
 * management service.  This module provides routines for creating pools
 * of buffers for exchange of network data, getting and returning buffers
 * from and to the pool, and some other utility functions. 
 * <P>
 * Currently, the pool has 2 underlying implementations - one for the vxWorks
 * OS, and another which attempts to be OS-agnostic so that it can be used on
 * other OS's such as Linux.  The API is largely the same for all OS's,
 * but there are some differences to be aware of.  These are documented
 * in the API descriptions below.
 * <P>
 * The most significant difference is this: when this module is used with
 * the WindRiver VxWorks OS, it will create a pool of vxWorks "MBufs".
 * These can be used directly with the vxWorks "netBufLib" OS Library.
 * For other OS's, it will create a pool of generic buffers.  These may need
 * to be converted into other buffer types (sk_buff's in Linux, for example)
 * before being used with any built-in OS routines available for
 * manipulating network data buffers.
 *
 * @sa IxOsBuffMgt.h
 */

#ifndef IXOSBUFFPOOLMGT_H
#define IXOSBUFFPOOLMGT_H

#include "IxOsalBackward.h"

#endif  /* IXOSBUFFPOOLMGT_H */

