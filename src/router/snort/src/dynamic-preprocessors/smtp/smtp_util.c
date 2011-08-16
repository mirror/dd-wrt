/*
 * smtp_util.c
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Copyright (C) 2005-2011 Sourcefire, Inc.
 *
 * Author: Andy  Mullican
 *
 * Description:
 *
 * This file contains SMTP helper functions.
 *
 * Entry point functions:
 *
 *    safe_strchr()
 *    safe_strstr()
 *    copy_to_space()
 *    safe_sscanf()
 *
 *
 */

#include <sys/types.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#include "debug.h"
#include "bounds.h"

#include "snort_smtp.h"
#include "smtp_util.h"
#include "sf_dynamic_preprocessor.h"
#include "sf_snort_packet.h"
#include "sf_base64decode.h"

extern DynamicPreprocessorData _dpd;
extern SMTP *smtp_ssn;
extern char smtp_normalizing;

void SMTP_GetEOL(const uint8_t *ptr, const uint8_t *end,
                 const uint8_t **eol, const uint8_t **eolm)
{
    const uint8_t *tmp_eol;
    const uint8_t *tmp_eolm;

    /* XXX maybe should fatal error here since none of these 
     * pointers should be NULL */
    if (ptr == NULL || end == NULL || eol == NULL || eolm == NULL)
        return;

    tmp_eol = (uint8_t *)memchr(ptr, '\n', end - ptr);
    if (tmp_eol == NULL)
    {
        tmp_eol = end;
        tmp_eolm = end;
    }
    else
    {
        /* end of line marker (eolm) should point to marker and 
         * end of line (eol) should point to end of marker */
        if ((tmp_eol > ptr) && (*(tmp_eol - 1) == '\r'))
        {
            tmp_eolm = tmp_eol - 1;
        }
        else
        {
            tmp_eolm = tmp_eol;
        }

        /* move past newline */
        tmp_eol++;
    }

    *eol = tmp_eol;
    *eolm = tmp_eolm;
}

int SMTP_CopyToAltBuffer(SFSnortPacket *p, const uint8_t *start, int length)
{
    uint8_t *alt_buf;
    int alt_size;
    uint16_t *alt_len;
    int ret;

    /* if we make a call to this it means we want to use the alt buffer
     * regardless of whether we copy any data into it or not - barring a failure */
    p->flags |= FLAG_ALT_DECODE;
    smtp_normalizing = 1;

    /* if start and end the same, nothing to copy */
    if (length == 0)
        return 0;

    alt_buf = _dpd.altBuffer->data;
    alt_size = sizeof(_dpd.altBuffer->data);
    alt_len = &_dpd.altBuffer->len;

    ret = SafeMemcpy(alt_buf + *alt_len, start, length, alt_buf, alt_buf + alt_size);

    if (ret != SAFEMEM_SUCCESS)
    {
        ResetAltBuffer(p);
        smtp_normalizing = 0;
        return -1;
    }

    *alt_len += length;

    return 0;
}

int SMTP_IsBase64Data(const char *start, int length)
{
    const char *tmp = NULL;
    
    tmp = _dpd.SnortStrnStr(start, length, "base64");

    if( tmp == NULL )
        return -1;

    return 0;
}

void SMTP_Base64Decode(const uint8_t *start, const uint8_t *end)
{
    uint32_t encode_avail = 0, decode_avail = 0 ;
    uint8_t *encode_buf, *decode_buf;
    uint32_t act_encode_size = 0, act_decode_size = 0;
    uint32_t prev_bytes = 0;
    int i = 0;

    if (smtp_ssn == NULL || smtp_ssn->decode_state == NULL )
        return;

    
    encode_avail = (smtp_ssn->decode_state->encode_depth - smtp_ssn->decode_state->encode_bytes_read);
    decode_avail = (smtp_ssn->decode_state->decode_depth - smtp_ssn->decode_state->decode_bytes_read);
    encode_buf = (uint8_t *)smtp_ssn->decode_state->encodeBuf;
    decode_buf = (uint8_t *)smtp_ssn->decode_state->decodeBuf;

    /* 1. Stop decoding when we have reached either the decode depth or encode depth.
     * 2. Stop decoding when we are out of memory */
    if(encode_avail ==0 || decode_avail ==0 ||
            (!encode_buf) || (!decode_buf))
    {
        ResetDecodeState(smtp_ssn->decode_state);
        return;
    }
  
   /*The non decoded encoded data in the previous packet is required for successful decoding
    * in case of base64 data spanned across packets*/
    if( smtp_ssn->decode_state->prev_encoded_bytes && 
            (smtp_ssn->decode_state->prev_encoded_bytes <= (int) encode_avail))
    {
        if(smtp_ssn->decode_state->prev_encoded_buf)
        {
            prev_bytes = smtp_ssn->decode_state->prev_encoded_bytes;
            while(smtp_ssn->decode_state->prev_encoded_bytes)
            {
                /* Since this data cannot be more than 3 bytes*/
                encode_buf[i] = smtp_ssn->decode_state->prev_encoded_buf[i];
                i++;
                smtp_ssn->decode_state->prev_encoded_bytes--;
            }
        }
    }

    if(sf_unfold_smtp(start, (end-start), encode_buf + prev_bytes, encode_avail, &act_encode_size) != 0)
    {
        ResetDecodeState(smtp_ssn->decode_state);
        return;
    }

    act_encode_size = act_encode_size + prev_bytes;

    i = (act_encode_size)%4 ;

    /* Encoded data should be in multiples of 4. Then we need to wait for the remainder encoded data to
     * successfully decode the base64 data. This happens when base64 data is spanned across packets*/
    if(i)
    {
        smtp_ssn->decode_state->prev_encoded_bytes = i;
        act_encode_size = act_encode_size - i;
        smtp_ssn->decode_state->prev_encoded_buf = encode_buf + act_encode_size;
    }
    
    smtp_ssn->decode_state->encode_bytes_read += act_encode_size;

    if(sf_base64decode(encode_buf, act_encode_size, decode_buf, decode_avail, &act_decode_size) != 0)
    {
        ResetDecodeState(smtp_ssn->decode_state);
        return;
    }


    
    smtp_ssn->decode_state->decode_present = 1;
    smtp_ssn->decode_state->decoded_bytes = act_decode_size;
    smtp_ssn->decode_state->decode_bytes_read += act_decode_size;


    return;
}

#ifdef DEBUG
char smtp_print_buffer[65537];

const char * SMTP_PrintBuffer(SFSnortPacket *p)
{
    const uint8_t *ptr = NULL;
    int len = 0;
    int iorig, inew;

    if (smtp_normalizing)
    {
        ptr = _dpd.altBuffer->data;
        len = _dpd.altBuffer->len;
    }
    else
    {
        ptr = p->payload;
        len = p->payload_size;
    }

    for (iorig = 0, inew = 0; iorig < len; iorig++, inew++)
    {
        if ((isascii((int)ptr[iorig]) && isprint((int)ptr[iorig])) || (ptr[iorig] == '\n'))
        {
            smtp_print_buffer[inew] = ptr[iorig];
        }
        else if (ptr[iorig] == '\r' &&
                 ((iorig + 1) < len) && (ptr[iorig + 1] == '\n'))
        {
            iorig++;
            smtp_print_buffer[inew] = '\n';
        }
        else if (isspace((int)ptr[iorig]))
        {
            smtp_print_buffer[inew] = ' ';
        }
        else
        {
            smtp_print_buffer[inew] = '.';
        }
    }

    smtp_print_buffer[inew] = '\0';

    return &smtp_print_buffer[0];
}
#endif

