/** 
 * This file is intended to provide backward 
 * compatibility for main osService/OSSL 
 * APIs. 
 *
 * It shall be phased out gradually and users
 * are strongly recommended to use IX_OSAL API.
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

#ifndef IX_OSAL_BACKWARD_BUFFER_MGT_H
#define IX_OSAL_BACKWARD_BUFFER_MGT_H

typedef IX_OSAL_MBUF IX_MBUF;

typedef IX_OSAL_MBUF_POOL IX_MBUF_POOL;


#define IX_MBUF_NEXT_BUFFER_IN_PKT_PTR(m_blk_ptr)  \
		IX_OSAL_MBUF_NEXT_BUFFER_IN_PKT_PTR(m_blk_ptr)


#define IX_MBUF_NEXT_PKT_IN_CHAIN_PTR(m_blk_ptr)  \
		IX_OSAL_MBUF_NEXT_PKT_IN_CHAIN_PTR(m_blk_ptr)


#define IX_MBUF_MDATA(m_blk_ptr)  \
		IX_OSAL_MBUF_MDATA(m_blk_ptr)


#define IX_MBUF_MLEN(m_blk_ptr) \
		IX_OSAL_MBUF_MLEN(m_blk_ptr)


#define IX_MBUF_TYPE(m_blk_ptr) \
		IX_OSAL_MBUF_MTYPE(m_blk_ptr)

/* Same as IX_MBUF_TYPE */
#define IX_MBUF_MTYPE(m_blk_ptr) \
                IX_OSAL_MBUF_MTYPE(m_blk_ptr)

#define IX_MBUF_FLAGS(m_blk_ptr)   	\
		IX_OSAL_MBUF_FLAGS(m_blk_ptr)


#define IX_MBUF_NET_POOL(m_blk_ptr)	\
		IX_OSAL_MBUF_NET_POOL(m_blk_ptr)


#define IX_MBUF_PKT_LEN(m_blk_ptr)	\
		IX_OSAL_MBUF_PKT_LEN(m_blk_ptr)


#define IX_MBUF_PRIV(m_blk_ptr)		\
		IX_OSAL_MBUF_PRIV(m_blk_ptr)


#define IX_MBUF_ALLOCATED_BUFF_LEN(m_blk_ptr)  \
		IX_OSAL_MBUF_ALLOCATED_BUFF_LEN(m_blk_ptr)


#define IX_MBUF_ALLOCATED_BUFF_DATA(m_blk_ptr)  \
		IX_OSAL_MBUF_ALLOCATED_BUFF_DATA(m_blk_ptr)


#define IX_MBUF_POOL_SIZE_ALIGN(size)   \
		IX_OSAL_MBUF_POOL_SIZE_ALIGN(size)


#define IX_MBUF_POOL_MBUF_AREA_SIZE_ALIGNED(count)	\
		IX_OSAL_MBUF_POOL_MBUF_AREA_SIZE_ALIGNED(count)


#define IX_MBUF_POOL_DATA_AREA_SIZE_ALIGNED(count, size) \
		IX_OSAL_MBUF_POOL_DATA_AREA_SIZE_ALIGNED(count, size)


#define IX_MBUF_POOL_MBUF_AREA_ALLOC(count, memAreaSize) \
		IX_OSAL_MBUF_POOL_MBUF_AREA_ALLOC(count, memAreaSize)


#define IX_MBUF_POOL_DATA_AREA_ALLOC(count, size, memAreaSize) \
		IX_OSAL_MBUF_POOL_DATA_AREA_ALLOC(count, size, memAreaSize)

IX_STATUS
ixOsalOsIxp400BackwardPoolInit (IX_OSAL_MBUF_POOL ** poolPtrPtr,
				UINT32 count, UINT32 size, const char *name);


/* This one needs extra steps*/
#define IX_MBUF_POOL_INIT(poolPtr, count, size, name) \
		ixOsalOsIxp400BackwardPoolInit( poolPtr, count,  size, name)


#define IX_MBUF_POOL_INIT_NO_ALLOC(poolPtrPtr, bufPtr, dataPtr, count, size, name) \
		(*poolPtrPtr = IX_OSAL_MBUF_NO_ALLOC_POOL_INIT(bufPtr, dataPtr, count, size, name))


IX_STATUS
ixOsalOsIxp400BackwardMbufPoolGet (IX_OSAL_MBUF_POOL * poolPtr,
				   IX_OSAL_MBUF ** newBufPtrPtr);

#define IX_MBUF_POOL_GET(poolPtr, bufPtrPtr) \
		ixOsalOsIxp400BackwardMbufPoolGet(poolPtr, bufPtrPtr)


#define IX_MBUF_POOL_PUT(bufPtr) \
		IX_OSAL_MBUF_POOL_PUT(bufPtr)


#define IX_MBUF_POOL_PUT_CHAIN(bufPtr) \
		IX_OSAL_MBUF_POOL_PUT_CHAIN(bufPtr)


#define IX_MBUF_POOL_SHOW(poolPtr) \
		IX_OSAL_MBUF_POOL_SHOW(poolPtr)


#define IX_MBUF_POOL_MDATA_RESET(bufPtr) \
		IX_OSAL_MBUF_POOL_MDATA_RESET(bufPtr)

#endif /* IX_OSAL_BACKWARD_BUFFER_MGT_H */
