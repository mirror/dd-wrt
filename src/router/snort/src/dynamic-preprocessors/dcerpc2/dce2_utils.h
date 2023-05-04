/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2008-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************
 *
 ****************************************************************************/

#ifndef _DCE2_UTILS_H_
#define _DCE2_UTILS_H_

#include "dce2_debug.h"
#include "dce2_memory.h"
#include "dcerpc.h"
#include "sf_types.h"
#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"
#include "snort_debug.h"
#include "snort_bounds.h"

/********************************************************************
 * Macros
 ********************************************************************/
#define DCE2_SENTINEL -1

/********************************************************************
 * Enumerations
 ********************************************************************/
typedef enum _DCE2_Ret
{
    DCE2_RET__SUCCESS = 0,
    DCE2_RET__ERROR,
    DCE2_RET__MEMCAP,
    DCE2_RET__NOT_INSPECTED,
    DCE2_RET__INSPECTED,
    DCE2_RET__REASSEMBLE,
    DCE2_RET__SEG,
    DCE2_RET__FULL,
    DCE2_RET__FRAG,
    DCE2_RET__ALERTED,
    DCE2_RET__IGNORE,
    DCE2_RET__DUPLICATE

} DCE2_Ret;

typedef enum _DCE2_TransType
{
    DCE2_TRANS_TYPE__NONE = 0,
    DCE2_TRANS_TYPE__SMB,
    DCE2_TRANS_TYPE__TCP,
    DCE2_TRANS_TYPE__UDP,
    DCE2_TRANS_TYPE__HTTP_PROXY,
    DCE2_TRANS_TYPE__HTTP_SERVER,
    DCE2_TRANS_TYPE__MAX

} DCE2_TransType;

typedef enum _DCE2_BufferMinAddFlag
{
    DCE2_BUFFER_MIN_ADD_FLAG__USE,
    DCE2_BUFFER_MIN_ADD_FLAG__IGNORE

} DCE2_BufferMinAddFlag;

typedef enum _DCE2_BufType
{
    DCE2_BUF_TYPE__NULL,
    DCE2_BUF_TYPE__SEG,
    DCE2_BUF_TYPE__FRAG

} DCE2_BufType;

typedef enum _DCE2_LogType
{
    DCE2_LOG_TYPE__LOG,
    DCE2_LOG_TYPE__WARN,
    DCE2_LOG_TYPE__ERROR

} DCE2_LogType;

/********************************************************************
 * Structures
 ********************************************************************/
typedef struct _DCE2_Buffer
{
    uint8_t *data;
    uint32_t len;
    uint32_t size;
    DCE2_MemType mtype;
    uint32_t min_add_size;
    uint32_t offset;

} DCE2_Buffer;

/********************************************************************
 * Inline function prototypes
 ********************************************************************/
#define DCE2_MOVE(data_ptr, data_len, amount) \
    { data_len -= (amount); data_ptr = (uint8_t *)data_ptr + (amount); }

static inline int DCE2_BufferIsEmpty(DCE2_Buffer *);
static inline void DCE2_BufferEmpty(DCE2_Buffer *);
static inline uint32_t DCE2_BufferSize(DCE2_Buffer *);
static inline uint32_t DCE2_BufferLength(DCE2_Buffer *);
static inline void DCE2_BufferSetLength(DCE2_Buffer *, uint32_t);
static inline uint8_t * DCE2_BufferData(DCE2_Buffer *);
static inline uint32_t DCE2_BufferMinAllocSize(DCE2_Buffer *);
static inline void DCE2_BufferSetMinAllocSize(DCE2_Buffer *, uint32_t);

static inline char * DCE2_PruneWhiteSpace(char *);
static inline int DCE2_IsEmptyStr(char *);
static inline DCE2_Ret DCE2_Memcpy(void *, const void *, uint32_t, const void *, const void *);
static inline DCE2_Ret DCE2_Memmove(void *, const void *, uint32_t, const void *, const void *);
static inline int DCE2_UuidCompare(const void *, const void *);
static inline void DCE2_CopyUuid(Uuid *, const Uuid *, const DceRpcBoFlag);

/********************************************************************
 * Public function prototypes
 ********************************************************************/
DCE2_Buffer * DCE2_BufferNew(uint32_t, uint32_t, DCE2_MemType);
DCE2_Ret DCE2_BufferAddData(DCE2_Buffer *, const uint8_t *,
        uint32_t, uint32_t, DCE2_BufferMinAddFlag);
DCE2_Ret DCE2_BufferMoveData(DCE2_Buffer *, uint32_t, const uint8_t *, uint32_t);
void DCE2_BufferDestroy(DCE2_Buffer *);

DCE2_Ret DCE2_HandleSegmentation(DCE2_Buffer *, const uint8_t *,
        uint16_t, uint32_t, uint16_t *);
NORETURN void DCE2_Die(const char *, ...);
void DCE2_Log(DCE2_LogType, const char *, ...);
const char * DCE2_UuidToStr(const Uuid *, DceRpcBoFlag);
void DCE2_PrintPktData(const uint8_t *, const uint16_t);

/*********************************************************************
 * Function: DCE2_BufferIsEmpty()
 *
 * Determines whether or not a buffer should be considered empty.
 * Buffer is considered empty if it is NULL, it's data is NULL
 * or the length of the data is zero.
 *
 * Arguments:
 *  DCE2_Buffer *
 *      Pointer to buffer object.
 *
 * Returns:
 *  1 if considered empty
 *  0 if not considered empty
 *
 *********************************************************************/
static inline int DCE2_BufferIsEmpty(DCE2_Buffer *buf)
{
    if (buf == NULL) return 1;
    if ((buf->data == NULL) || (buf->len == 0)) return 1;
    return 0;
}

/*********************************************************************
 * Function: DCE2_BufferEmpty()
 *
 * Sets the buffer's data length to zero.  Essentially says that
 * any copied data is not important anymore.
 *
 * Arguments:
 *  DCE2_Buffer *
 *      Pointer to buffer object.
 *
 * Returns: None
 *
 *********************************************************************/
static inline void DCE2_BufferEmpty(DCE2_Buffer *buf)
{
    if (buf == NULL) return;
    buf->len = 0;
}

/*********************************************************************
 * Function: DCE2_BufferSize()
 *
 * Returns the size of the data currently allocated for storing data.
 *
 * Arguments:
 *  DCE2_Buffer *
 *      Pointer to buffer object.
 *
 * Returns:
 *  uint32_t
 *      The size of the allocated data or zero if buffer
 *      object is NULL.
 *
 *********************************************************************/
static inline uint32_t DCE2_BufferSize(DCE2_Buffer *buf)
{
    if (buf == NULL) return 0;
    return buf->size;
}

/*********************************************************************
 * Function: DCE2_BufferLength()
 *
 * Returns the length of the data copied into the buffer.  This will
 * always be less than or equal to the size of the data allocated.
 *
 * Arguments:
 *  DCE2_Buffer *
 *      Pointer to buffer object.
 *
 * Returns:
 *  uint32_t
 *      The length of the data copied into the buffer or zero
 *      if buffer object is NULL.
 *
 *********************************************************************/
static inline uint32_t DCE2_BufferLength(DCE2_Buffer *buf)
{
    if (buf == NULL) return 0;
    return buf->len;
}

/*********************************************************************
 * Function: DCE2_BufferOffset()
 *
 * Returns the offset of the data copied into the buffer.
 *
 * Arguments:
 *  DCE2_Buffer *
 *      Pointer to buffer object.
 *
 * Returns:
 *  uint32_t
 *      The length of the data copied into the buffer or zero
 *      if buffer object is NULL.
 *
 *********************************************************************/
static inline uint32_t DCE2_BufferOffset(DCE2_Buffer *buf)
{
    if (buf == NULL) return 0;
    return buf->offset;
}


/*********************************************************************
 * Function: DCE2_BufferSetLength()
 *
 * Sets the length of the buffer up to the buffer size.
 *
 * Arguments:
 *  DCE2_Buffer *
 *      Pointer to buffer object.
 *  uint32_t len
 *      The length to set the amount of data in the buffer to
 *
 * Returns: None
 *
 *********************************************************************/
static inline void DCE2_BufferSetLength(DCE2_Buffer *buf, uint32_t len)
{
    if (buf == NULL) return;
    if (len > buf->size) buf->len = buf->size;
    else buf->len = len;
}

/*********************************************************************
 * Function: DCE2_BufferData()
 *
 * Returns a pointer to the allocated data.  Note that this could
 * be NULL if not data has been allocated yet.
 *
 * Arguments:
 *  DCE2_Buffer *
 *      Pointer to buffer object.
 *
 * Returns:
 *  uint8_t *
 *      Pointer to the buffer data or NULL if none allocated or
 *      buffer object is NULL.
 *
 *********************************************************************/
static inline uint8_t * DCE2_BufferData(DCE2_Buffer *buf)
{
    if (buf == NULL) return NULL;
    return buf->data;
}

/*********************************************************************
 * Function: DCE2_BufferMinAllocSize()
 *
 * Returns the minimum allocation size for the buffer.
 *
 * Arguments:
 *  DCE2_Buffer *
 *      Pointer to buffer object.
 *
 * Returns:
 *  uint32_t
 *      The minimum allocation size.
 *
 *********************************************************************/
static inline uint32_t DCE2_BufferMinAllocSize(DCE2_Buffer *buf)
{
    if (buf == NULL) return 0;
    return buf->min_add_size;
}

/*********************************************************************
 * Function: DCE2_BufferSetMinAllocSize()
 *
 * Sets the minimum allocation size for the buffer.
 *
 * Arguments:
 *  DCE2_Buffer *
 *      Pointer to buffer object.
 *  uint32_t
 *      Size to set the minimum allocation size to.
 *
 * Returns: None
 *
 *********************************************************************/
static inline void DCE2_BufferSetMinAllocSize(DCE2_Buffer *buf, uint32_t size)
{
    if (buf == NULL) return;
    buf->min_add_size = size;
}

/********************************************************************
 * Function: DCE2_PruneWhiteSpace()
 *
 * Prunes whitespace surrounding string.
 * String must be 0 terminated.
 *
 * Arguments:
 *  char *
 *      NULL terminated string to prune.
 *  int
 *      length of string
 *
 * Returns:
 *  char * - Pointer to the pruned string.  Note that the pointer
 *           still points within the original string.
 *
 * Side effects: Spaces at the end of the string passed in as an
 *               argument are replaced by NULL bytes.
 *
 ********************************************************************/
static inline char * DCE2_PruneWhiteSpace(char *str)
{
    char *end;

    if (str == NULL)
        return NULL;

    /* Put end a char before NULL byte */
    end = str + (strlen(str) - 1);

    while (isspace((int)*str))
        str++;

    while ((end > str) && isspace((int)*end))
    {
        *end = '\0';
        end--;
    }

    return str;
}

/********************************************************************
 * Function: DCE2_IsEmptyStr()
 *
 * Checks if string is NULL, empty or just spaces.
 * String must be 0 terminated.
 *
 * Arguments: None
 *  char * - string to check
 *
 * Returns:
 *  1  if string is NULL, empty or just spaces
 *  0  otherwise
 *
 ********************************************************************/
static inline int DCE2_IsEmptyStr(char *str)
{
    char *end;

    if (str == NULL)
        return 1;

    end = str + strlen(str);

    while ((str < end) && isspace((int)*str))
        str++;

    if (str == end)
        return 1;

    return 0;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *  DCE2_RET__ERROR - memcpy failed
 *  DCE2_RET__SUCCESS - memcpy succeeded
 *
 ********************************************************************/
static inline DCE2_Ret DCE2_Memcpy(void *dst, const void *src, uint32_t len,
                                   const void *dst_start, const void *dst_end)
{
    if (SafeMemcpy(dst, src, (size_t)len, dst_start, dst_end) != SAFEMEM_SUCCESS)
        return DCE2_RET__ERROR;

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *  DCE2_RET__ERROR - memmove failed
 *  DCE2_RET__SUCCESS - memmove succeeded
 *
 ********************************************************************/
static inline DCE2_Ret DCE2_Memmove(void *dst, const void *src, uint32_t len,
                                    const void *dst_start, const void *dst_end)
{
    if (SafeMemmove(dst, src, (size_t)len, dst_start, dst_end) != SAFEMEM_SUCCESS)
        return DCE2_RET__ERROR;

    return DCE2_RET__SUCCESS;
}

/*********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 *********************************************************************/
static inline int DCE2_UuidCompare(const void *data1, const void *data2)
{
    const Uuid *uuid1 = (Uuid *)data1;
    const Uuid *uuid2 = (Uuid *)data2;

    if ((uuid1 == NULL) || (uuid2 == NULL))
        return -1;

    if ((uuid1->time_low == uuid2->time_low) &&
        (uuid1->time_mid == uuid2->time_mid) &&
        (uuid1->time_high_and_version == uuid2->time_high_and_version) &&
        (uuid1->clock_seq_and_reserved == uuid2->clock_seq_and_reserved) &&
        (uuid1->clock_seq_low == uuid2->clock_seq_low) &&
        (memcmp(uuid1->node, uuid2->node, sizeof(uuid1->node)) == 0))
    {
        return 0;
    }

    /* Just return something other than 0 */
    return -1;
}

/*********************************************************************
 * Function: DCE2_CopyUuid()
 *
 * Copies a src uuid to a dst uuid based on the byte
 * order specified.
 *
 * Arguments:
 *  Uuid *
 *      Pointer to uuid to copy to.
 *  Uuid *
 *      Pointer to uuid to copy from.
 *  const int
 *      The byte order to use.
 *
 * Returns: None
 *
 *********************************************************************/
static inline void DCE2_CopyUuid(Uuid *dst_uuid, const Uuid *pkt_uuid, const DceRpcBoFlag byte_order)
{
    dst_uuid->time_low = DceRpcNtohl(&pkt_uuid->time_low, byte_order);
    dst_uuid->time_mid = DceRpcNtohs(&pkt_uuid->time_mid, byte_order);
    dst_uuid->time_high_and_version = DceRpcNtohs(&pkt_uuid->time_high_and_version, byte_order);
    dst_uuid->clock_seq_and_reserved = pkt_uuid->clock_seq_and_reserved;
    dst_uuid->clock_seq_low = pkt_uuid->clock_seq_low;
    memcpy(dst_uuid->node, pkt_uuid->node, sizeof(dst_uuid->node));
}


#endif  /* _DCE2_UTILS_H_ */

