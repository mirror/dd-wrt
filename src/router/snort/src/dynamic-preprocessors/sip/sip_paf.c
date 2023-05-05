/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
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
 ****************************************************************************/


#include <sys/types.h>
#include "sf_types.h"
#include "sf_dynamic_preprocessor.h"
#include "stream_api.h"
#include "sip_paf.h"
#include <string.h>

/* State tracker for SIP PAF */
typedef enum _SIPPafState
{
    SIP_PAF_START_STATE,     /* default state. continue until LF */
    SIP_PAF_CONTENT_LEN_CMD, /* searching Content-Length header */
    SIP_PAF_CONTENT_LEN_CONVERT,   /* parse the literal content length */
    SIP_PAF_BODY_SEARCH,    /* Check SIP body start */
    SIP_PAF_FLUSH_STATE     /* flush if Content-Length is reached */
} SIPPafState;

/* State tracker for SIP Content Length */
typedef enum _SIPPafDataLenStatus
{
    SIP_PAF_LENGTH_INVALID,
    SIP_PAF_LENGTH_CONTINUE,
    SIP_PAF_LENGTH_DONE
} SIPPafDataLenStatus;

/* State tracker for SIP Body Boundary*/
typedef enum _SIPPafBodyStatus
{
    SIP_PAF_BODY_UNKNOWN,
    SIP_PAF_BODY_START_FIRST_CR,   /* Check SIP body start - first CR */
    SIP_PAF_BODY_START_FIRST_LF,   /* Check SIP body start - first LF */
    SIP_PAF_BODY_START_SECOND_CR,  /* Check SIP body start - second CR */
    SIP_PAF_BODY_START_SECOND_LF  /* Check SIP body start - second LF */
} SIPPafBodyStatus;

/* State tracker for POP PAF */
typedef struct _sipPafData
{
    SIPPafState sip_state;    /* The current sip paf state */
    SIPPafBodyStatus body_state; /* State to find sip body */
    char *next_letter;        /* The current character in Content-Length */
    bool found_len;
    uint32_t length;
} SIPPafData;

static char* content_len_key = "Content-Length";
static char* content_len_key_compact = "l";

#define UNKNOWN_CONTENT_LENGTH         UINT32_MAX

static uint8_t sip_paf_id = 0;

static inline void reset_data_states(SIPPafData *pfdata)
{
    /* reset data information information */
    pfdata->next_letter = 0;
    pfdata->length = UNKNOWN_CONTENT_LENGTH;
    pfdata->sip_state = SIP_PAF_START_STATE;
    pfdata->body_state = SIP_PAF_BODY_UNKNOWN;
}

/*
 * Search for the single line termination sequence LF ("\n").
 *
 * PARAMS:
 *        const uint8_t ch - the next character to analyze.
 *        SIPPafData *pfdata - the struct containing all sip paf information
 *
 * RETURNS:
 *        false - if termination sequence not found
 *        true - if termination sequence found
 */
static bool find_data_end_single_line(const uint8_t ch, SIPPafData *pfdata)
{
    if (ch == '\n')
    {
        pfdata->sip_state = SIP_PAF_CONTENT_LEN_CMD;
        return true;
    }
    return false;
}
/*
 * Confirms every character in the current sequence is part of the expected
 * header. After confirmation is complete, SIP PAF will begin searching
 * for content length. If any character is unexpected, searches for the default
 * termination sequence.
 *
 * PARAMS:
 *        const uint8_t ch - the next character to analyze.
 *        SIPPafData *pfdata - the struct containing all sip paf information
 */
static inline void process_command(const uint8_t ch, SIPPafData *pfdata)
{
    char val;

    if (!pfdata->next_letter)
    {
        /* avoid leading space */
        if (isspace(ch))
            return;
        if (toupper(ch) == toupper(content_len_key[0]))
        {
            pfdata->next_letter = &content_len_key[1];
            return;
        }
        else
            pfdata->next_letter = content_len_key_compact;
    }

    val = *(pfdata->next_letter);

    if (val == '\0')
    {
        /* end of content header */
        if (ch == ':')
            pfdata->sip_state = SIP_PAF_CONTENT_LEN_CONVERT;
        else if (!isblank(ch))
        {
            reset_data_states(pfdata);
            find_data_end_single_line(ch, pfdata);
        }
    }
    else if (toupper(ch) == toupper(val))
        pfdata->next_letter++;
    else
    {
        reset_data_states(pfdata);
        find_data_end_single_line(ch, pfdata);
    }
}

/* Get the length of SIP body from Content-Length header */
static SIPPafDataLenStatus get_length(char c, uint32_t *len )
{
    uint64_t length = *len;

    if (isspace(c))
    {
        if (length != UNKNOWN_CONTENT_LENGTH)
        {
            *len = length;
            return SIP_PAF_LENGTH_DONE;
        }
    }
    else if (isdigit(c))
    {
        uint64_t tmp_len;
        if (length == UNKNOWN_CONTENT_LENGTH)
            length = 0;
        tmp_len = (10 * length)  + (c - '0');
        if (tmp_len < UINT32_MAX)
            length = (uint32_t) tmp_len;
        else
        {
            *len = 0;
            return SIP_PAF_LENGTH_INVALID;
        }
    }
    else
    {
        *len = 0;
        return SIP_PAF_LENGTH_INVALID;
    }

    *len = length;
    return SIP_PAF_LENGTH_CONTINUE;
}

/*
 * Find the start of SIP body, "\r\n\r\n" or "\n\n"
 *
 * PARAMS:
 *        const uint8_t ch - the next character to analyze.
 *        SIPPafData *pfdata - the struct containing all sip paf information
 * Returns:
 *        true: found body;
 *        false: not found yet
 */
static inline bool find_body(const uint8_t ch, SIPPafData *pfdata)
{
    switch (pfdata->body_state)
    {
    case SIP_PAF_BODY_UNKNOWN:
        if (ch == '\r')
            pfdata->body_state = SIP_PAF_BODY_START_FIRST_CR;
        else if (ch == '\n')
            pfdata->body_state = SIP_PAF_BODY_START_FIRST_LF;
        break;
    case SIP_PAF_BODY_START_FIRST_CR:
        if (ch == '\n')
            pfdata->body_state = SIP_PAF_BODY_START_SECOND_CR;
        else if (ch != '\r')
            pfdata->body_state = SIP_PAF_BODY_UNKNOWN;
        break;
    case SIP_PAF_BODY_START_FIRST_LF:
        if (ch == '\n')
            return true;
        else if (ch == '\r')
            pfdata->body_state = SIP_PAF_BODY_START_FIRST_CR;
        else
            pfdata->body_state = SIP_PAF_BODY_UNKNOWN;
        break;
    case SIP_PAF_BODY_START_SECOND_CR:
        if (ch == '\r')
            pfdata->body_state = SIP_PAF_BODY_START_SECOND_LF;
        else if (ch == '\n')
            return true;
        else
            pfdata->body_state = SIP_PAF_BODY_UNKNOWN;
        break;
    case SIP_PAF_BODY_START_SECOND_LF:
        if (ch == '\n')
            return true;
        else if (ch == '\r')
            pfdata->body_state = SIP_PAF_BODY_START_FIRST_CR;
        else
            pfdata->body_state = SIP_PAF_BODY_UNKNOWN;
        break;
    }
    return false;
}

/* Function: sip_paf()
 *
 *  Purpose: sip PAF callback.
 *           Inspects sip traffic.
 *
 *  Arguments:
 *    void *ssn - stream session pointer
 *    void **ps - sipPaf state tracking structure
 *    const uint8_t *data - payload data to inspect
 *    uint32_t len - length of payload data
 *    uint32_t flags - flags to check whether client or server
 *    uint32_t * fp - pointer to set flush point
 *    uint32_t * fp_eoh - pointer to set header flush point
 *
 * Returns:
 *   PAF_Status - PAF_FLUSH if flush point found, PAF_SEARCH otherwise
 */
static PAF_Status sip_paf(void* ssn, void** ps, const uint8_t* data,
        uint32_t len, uint64_t *flags, uint32_t* fp, uint32_t* fp_eoh)
{
    uint32_t i;
    SIPPafData *pfdata = *(SIPPafData **)ps;

    if (pfdata == NULL)
    {
        pfdata = _dpd.snortAlloc(1, sizeof(*pfdata),
                                 PP_SIP, PP_MEM_CATEGORY_SESSION);
        if (pfdata == NULL)
        {
            return PAF_ABORT;
        }

        reset_data_states(pfdata);
        *ps = pfdata;
    }

    for (i = 0; i < len; i++)
    {
        uint8_t ch = data[i];
        const uint8_t *next;
        SIPPafDataLenStatus status;
        switch(pfdata->sip_state)
        {
        case SIP_PAF_START_STATE:
            next = (const uint8_t *) memchr(&data[i], '\n', (len - i));
            if (next)
            {
                i = (uint32_t)(next - data);
                pfdata->sip_state = SIP_PAF_CONTENT_LEN_CMD;
            }
            else
                return PAF_SEARCH;

            break;
        case SIP_PAF_CONTENT_LEN_CMD:
            process_command(ch, pfdata);
            break;
        case SIP_PAF_CONTENT_LEN_CONVERT:
            status = get_length(ch, &pfdata->length);
            if ( status == SIP_PAF_LENGTH_DONE)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_SIP, "Find data length: %d\n",
                        pfdata->length););
                pfdata->sip_state = SIP_PAF_BODY_SEARCH;
                find_body(ch, pfdata);
            }
            else if (status == SIP_PAF_LENGTH_INVALID)
            {
                reset_data_states(pfdata);
                find_data_end_single_line(ch, pfdata);
            }
            break;
        case SIP_PAF_BODY_SEARCH:
            if (find_body(ch, pfdata))
            {
                pfdata->sip_state = SIP_PAF_FLUSH_STATE;
            }
            else
                break;
        case SIP_PAF_FLUSH_STATE:
            if (pfdata->length == 0)
            {
                DEBUG_WRAP(DebugMessage(DEBUG_SIP, "SIP PAF: flushing data!\n"););
                *fp = i + 1;
                reset_data_states(pfdata);
                return PAF_FLUSH;
            }
            pfdata->length--;
            break;
        }
    }

    return PAF_SEARCH;
}

static void sip_paf_cleanup(void *pafData)
{
    if (pafData)
        _dpd.snortFree(pafData, sizeof(SIPPafData), PP_SIP,
                       PP_MEM_CATEGORY_SESSION);
}

#ifdef TARGET_BASED
void register_sip_paf_service (struct _SnortConfig *sc, int16_t app, tSfPolicyId policy)
{
    if (_dpd.isPafEnabled())
    {
        sip_paf_id = _dpd.streamAPI->register_paf_service(sc, policy, app, true, sip_paf, true);
        sip_paf_id = _dpd.streamAPI->register_paf_service(sc, policy, app, false, sip_paf, true);
        _dpd.streamAPI->register_paf_free(sip_paf_id, sip_paf_cleanup);
    }
}
#endif


void register_sip_paf_port(struct _SnortConfig *sc, unsigned int i, tSfPolicyId policy)
{
    if (_dpd.isPafEnabled())
    {
        sip_paf_id = _dpd.streamAPI->register_paf_port(sc, policy, (uint16_t)i, true, sip_paf, true);
        sip_paf_id = _dpd.streamAPI->register_paf_port(sc, policy, (uint16_t)i, false, sip_paf, true);
    }
}
