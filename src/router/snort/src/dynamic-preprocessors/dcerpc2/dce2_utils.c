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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "snort_debug.h"
#include "dce2_utils.h"
#include "dce2_debug.h"
#include "dce2_config.h"
#include "snort_dce2.h"
#include "sf_types.h"
#include "snort_bounds.h"
#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"
#include <stdarg.h>

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
DCE2_Buffer * DCE2_BufferNew(uint32_t initial_size, uint32_t min_add_size, DCE2_MemType mem_type)
{
    DCE2_Buffer *buf = (DCE2_Buffer *)DCE2_Alloc(sizeof(DCE2_Buffer), mem_type);

    if (buf == NULL)
        return NULL;

    if (initial_size != 0)
    {
        buf->data = (uint8_t *)DCE2_Alloc(initial_size, mem_type);
        if (buf->data == NULL)
        {
            DCE2_Free((void *)buf, sizeof(DCE2_Buffer), mem_type);
            return NULL;
        }
    }

    buf->size = initial_size;
    buf->len = 0;
    buf->mtype = mem_type;
    buf->min_add_size = min_add_size;
    buf->offset = 0;

    return buf;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
DCE2_Ret DCE2_BufferAddData(DCE2_Buffer *buf, const uint8_t *data,
    uint32_t data_len, uint32_t data_offset, DCE2_BufferMinAddFlag mflag)
{
    DCE2_Ret status;

    if ((buf == NULL) || (data == NULL))
        return DCE2_RET__ERROR;

    /* Return success for this since ultimately nothing _was_ added */
    if (data_len == 0)
        return DCE2_RET__SUCCESS;

    if (buf->data == NULL)
    {
        uint32_t size = data_offset + data_len;

        if ((size < buf->min_add_size) && (mflag == DCE2_BUFFER_MIN_ADD_FLAG__USE))
            size = buf->min_add_size;

        buf->data = (uint8_t *)DCE2_Alloc(size, buf->mtype);
        if (buf->data == NULL)
            return DCE2_RET__ERROR;

        buf->size = size;
    }
    else if ((data_offset + data_len) > buf->size)
    {
        uint8_t *tmp;
        uint32_t new_size = data_offset + data_len;

        if (((new_size - buf->size) < buf->min_add_size) && (mflag == DCE2_BUFFER_MIN_ADD_FLAG__USE))
            new_size = buf->size + buf->min_add_size;

        tmp = (uint8_t *)DCE2_ReAlloc(buf->data, buf->size, new_size, buf->mtype);
        if (tmp == NULL)
            return DCE2_RET__ERROR;

        buf->data = tmp;
        buf->size = new_size;
    }

    status = DCE2_Memcpy(buf->data + data_offset, data, data_len, buf->data, buf->data + buf->size);
    if (status != DCE2_RET__SUCCESS)
    {
        DCE2_Log(DCE2_LOG_TYPE__ERROR,
                 "%s(%d) Failed to copy data into buffer.", __FILE__, __LINE__);
        return DCE2_RET__ERROR;
    }

    if ((data_offset + data_len) > buf->len)
        buf->len = data_offset + data_len;

    return DCE2_RET__SUCCESS;
}

/********************************************************************
 * Function:
 *
 * Must have allocated data in buffer and data_len must fit in
 * buffer.
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
DCE2_Ret DCE2_BufferMoveData(DCE2_Buffer *buf, uint32_t data_offset,
                             const uint8_t *move, uint32_t move_len)
{
    DCE2_Ret status;
    uint8_t *offset, *end;

    if ((buf == NULL) || (buf->data == NULL) || (move == NULL))
        return DCE2_RET__ERROR;

    /* Return success for this since ultimately nothing _was_ moved */
    if (move_len == 0)
        return DCE2_RET__SUCCESS;

    offset = buf->data + data_offset;
    end = buf->data + buf->len;

    /* Moved data must be within current data */
    if ((move < buf->data) || ((move + move_len) > end))
        return DCE2_RET__ERROR;

    /* No move required */
    if (move == offset)
        return DCE2_RET__SUCCESS;

    /* Would have to do two moves.  One for the data and one to realign data
     * with start of moved data.  Don't want to succeed on the first and fail
     * on the second and leave the buffer in a bad state.  Don't want to use
     * an offset in data buffer because want to keep the size the same. */
    if (move == buf->data)
    {
        uint32_t tmp_size = buf->len;
        uint8_t *tmp = (uint8_t *)DCE2_Alloc(tmp_size, buf->mtype);
        uint8_t *tmp_offset, *tmp_end;
        uint32_t new_len;

        if (tmp == NULL)
            return DCE2_RET__ERROR;

        tmp_offset = tmp + data_offset;
        tmp_end = tmp + tmp_size;

        status = DCE2_Memcpy(tmp, buf->data, buf->len, tmp, tmp_end);
        if (status != DCE2_RET__SUCCESS)
        {
            DCE2_Free((void *)tmp, tmp_size, buf->mtype);
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Failed to move data in buffer.", __FILE__, __LINE__);
            return DCE2_RET__ERROR;
        }

        status = DCE2_Memmove(tmp_offset, tmp, move_len, tmp_offset, tmp_end);
        if (status != DCE2_RET__SUCCESS)
        {
            DCE2_Free((void *)tmp, tmp_size, buf->mtype);
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Failed to move data in buffer.", __FILE__, __LINE__);
            return DCE2_RET__ERROR;
        }

        if (tmp_offset > (tmp + move_len))
            tmp_offset = tmp + move_len;

        new_len = tmp_end - tmp_offset;

        status = DCE2_Memcpy(buf->data, tmp_offset, new_len, buf->data, end);
        if (status != DCE2_RET__SUCCESS)
        {
            DCE2_Free((void *)tmp, tmp_size, buf->mtype);
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Failed to move data in buffer.", __FILE__, __LINE__);
            return DCE2_RET__ERROR;
        }

        buf->len = new_len;

        DCE2_Free((void *)tmp, tmp_size, buf->mtype);
    }
    else
    {
        status = DCE2_Memmove(offset, move, move_len, offset, end);
        if (status != DCE2_RET__SUCCESS)
        {
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                     "%s(%d) Failed to move data in buffer", __FILE__, __LINE__);
            return DCE2_RET__ERROR;
        }

        /* If we have a new end of data, adjust length */
        if ((move + move_len) == end)
            buf->len = data_offset + move_len;
    }

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
 *
 ********************************************************************/
void DCE2_BufferDestroy(DCE2_Buffer *buf)
{
    if (buf == NULL)
        return;

    if (buf->data != NULL)
        DCE2_Free((void *)buf->data, buf->size, buf->mtype);

    DCE2_Free((void *)buf, sizeof(DCE2_Buffer), buf->mtype);
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
DCE2_Ret DCE2_HandleSegmentation(DCE2_Buffer *seg_buf, const uint8_t *data_ptr,
    uint16_t data_len, uint32_t need_len, uint16_t *data_used)
{
    uint32_t copy_len;
    DCE2_Ret status;

    /* Initialize in case we return early without adding
     * any data to the buffer */
    *data_used = 0;

    if (seg_buf == NULL)
        return DCE2_RET__ERROR;

    /* Don't need anything - call it desegmented.  Really return
     * an error - this shouldn't happen */
    if (need_len == 0)
        return DCE2_RET__ERROR;

    /* Already have enough data for need */
    if (DCE2_BufferLength(seg_buf) >= need_len)
        return DCE2_RET__SUCCESS;

    /* No data and need length > 0 - must still be segmented */
    if (data_len == 0)
        return DCE2_RET__SEG;

    /* Already know that need length is greater than buffer length */
    copy_len = need_len - DCE2_BufferLength(seg_buf);
    if (copy_len > data_len)
        copy_len = data_len;

    status = DCE2_BufferAddData(seg_buf, data_ptr, copy_len,
            DCE2_BufferLength(seg_buf), DCE2_BUFFER_MIN_ADD_FLAG__USE);

    if (status != DCE2_RET__SUCCESS)
        return DCE2_RET__ERROR;

    /* copy_len <= data_len <= UINT16_MAX */
    *data_used = (uint16_t)copy_len;

    if (DCE2_BufferLength(seg_buf) == need_len)
        return DCE2_RET__SUCCESS;

    return DCE2_RET__SEG;
}

/********************************************************************
 * Function: DCE2_Die()
 *
 * Purpose: Fatal errors.  Calls DynamicPreprocessorFatalMessage.
 *          It's just quicker to type.
 *
 * Arguments: None
 *  const char * - format string
 *  ... - format arguments
 *
 * Returns: None
 *
 ********************************************************************/
NORETURN void DCE2_Die(const char *format, ...)
{
    char buf[1024];
    va_list ap;

    DCE2_FreeGlobals();

    if (format == NULL)
    {
        _dpd.errMsg("ERROR: %s(%d) => %s: format is NULL.\n",
                    __FILE__, __LINE__, DCE2_GNAME);

        DynamicPreprocessorFatalMessage("%s: Dieing.\n", DCE2_GNAME, buf);
    }

    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);

    buf[sizeof(buf) - 1] = '\0';

    DynamicPreprocessorFatalMessage("%s: %s\n", DCE2_GNAME, buf);
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
void DCE2_Log(DCE2_LogType ltype, const char *format, ...)
{
    char buf[1024];
    va_list ap;

    if (format == NULL)
    {
        _dpd.errMsg("ERROR: %s(%d) => %s: format is NULL.\n",
                    __FILE__, __LINE__, DCE2_GNAME);
        return;
    }

    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);

    buf[sizeof(buf) - 1] = '\0';

    switch (ltype)
    {
        case DCE2_LOG_TYPE__LOG:
            _dpd.logMsg("LOG: %s: %s\n", DCE2_GNAME, buf);
            break;
        case DCE2_LOG_TYPE__WARN:
            _dpd.errMsg("WARN: %s: %s\n", DCE2_GNAME, buf);
            break;
        case DCE2_LOG_TYPE__ERROR:
            _dpd.errMsg("ERROR: %s: %s\n", DCE2_GNAME, buf);
            break;
        default:
            _dpd.errMsg("ERROR: %s(%d) => %s: Invalid log type: %d.\n",
                        __FILE__, __LINE__, DCE2_GNAME, ltype);
            break;
    }
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
const char * DCE2_UuidToStr(const Uuid *uuid, DceRpcBoFlag byte_order)
{
#define UUID_BUF_SIZE  50
    static char uuid_buf1[UUID_BUF_SIZE];
    static char uuid_buf2[UUID_BUF_SIZE];
    static int buf_num = 0;
    char *uuid_buf;

    if (buf_num == 0)
    {
        uuid_buf = uuid_buf1;
        buf_num = 1;
    }
    else
    {
        uuid_buf = uuid_buf2;
        buf_num = 0;
    }

    snprintf(uuid_buf, UUID_BUF_SIZE,
             "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
             DceRpcHtonl(&uuid->time_low, byte_order),
             DceRpcHtons(&uuid->time_mid, byte_order),
             DceRpcHtons(&uuid->time_high_and_version, byte_order),
             uuid->clock_seq_and_reserved, uuid->clock_seq_low,
             uuid->node[0], uuid->node[1], uuid->node[2],
             uuid->node[3], uuid->node[4], uuid->node[5]);

    uuid_buf[UUID_BUF_SIZE - 1] = '\0';

    return uuid_buf;
}

#ifdef DEBUG_MSGS
/********************************************************************
 * Function: DCE2_PrintPktData()
 *
 * Purpose: Prints packet data in hex and ascii.
 *
 * Arguments: None
 *  const uint8_t * - pointer to data to print
 *  const uint16_t  - size of data
 *
 * Returns: None
 *
 ********************************************************************/
void DCE2_PrintPktData(const uint8_t *data, const uint16_t len)
{
    unsigned int i, j = 0, line_len = 0;
    uint8_t hex_buf[16];
    uint8_t char_buf[16];

    for (i = 0; i < len; i++)
    {
        hex_buf[j] = data[i];

        if (isascii((int)data[i]) && isprint((int)data[i]))
            char_buf[j] = data[i];
        else
            char_buf[j] = '.';

        if (line_len == 15)
        {
            unsigned int k, sub_line_len = 0;
            for (k = 0; k <= j; k++)
            {
                printf("%02x ", hex_buf[k]);
                if (sub_line_len >= 7)
                {
                    printf(" ");
                    sub_line_len = 0;
                }
                else
                {
                    sub_line_len++;
                }
            }

            printf(" ");

            sub_line_len = 0;
            for (k = 0; k <= j; k++)
            {
                printf("%c", char_buf[k]);
                if (sub_line_len >= 7)
                {
                    printf(" ");
                    sub_line_len = 0;
                }
                else
                {
                    sub_line_len++;
                }
            }

            printf("\n");

            j = line_len = 0;
        }
        else
        {
            j++;
            line_len++;
        }
    }

    if (line_len > 0)
    {
        unsigned int k, sub_line_len = 0;
        for (k = 0; k < j; k++)
        {
            printf("%02x ", hex_buf[k]);
            if (sub_line_len >= 7)
            {
                printf(" ");
                sub_line_len = 0;
            }
            else
            {
                sub_line_len++;
            }
        }

        if (k < 8)
            printf("   ");
        else
            printf("  ");

        while (k < 16)
        {
            printf("   ");
            k++;
        }

        sub_line_len = 0;
        for (k = 0; k < j; k++)
        {
            printf("%c", char_buf[k]);
            if (sub_line_len >= 7)
            {
                printf(" ");
                sub_line_len = 0;
            }
            else
            {
                sub_line_len++;
            }
        }
    }

    printf("\n");
}
#endif
