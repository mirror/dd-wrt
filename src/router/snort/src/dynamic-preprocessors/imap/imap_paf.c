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
#include "imap_paf.h"
#include "spp_imap.h"
#include "sf_dynamic_preprocessor.h"
#include "stream_api.h"
#include "snort_imap.h"
#include "imap_config.h"
#include "file_api.h"


extern const IMAPToken imap_resps[];

static uint8_t imap_paf_id = 0;

typedef struct _ImapDataInfo
{
    int paren_cnt;        /* The open parentheses count in fetch */
    bool found_len;
    bool esc_nxt_char;    /* true if the next charachter has been escaped */
    char *next_letter;    /* The current command in fetch */
    uint32_t length;
} ImapDataInfo;


/* State tracker for SMTP PAF */
typedef enum _ImapPafState
{
    IMAP_PAF_REG_STATE,       /* default state. eat until LF */
    IMAP_PAF_DATA_HEAD_STATE, /* parses the fetch header */
    IMAP_PAF_DATA_LEN_STATE,  /* parse the literal length */
    IMAP_PAF_DATA_STATE,      /* search for and flush on MIME boundaries */
    IMAP_PAF_FLUSH_STATE,     /* flush if a termination sequence is found */
    IMAP_PAF_CMD_IDENTIFIER,  /* determine the line identifier ('+', '*', tag) */
    IMAP_PAF_CMD_TAG,         /* currently analyzing tag . identifier */
    IMAP_PAF_CMD_STATUS,      /* currently parsing second argument */
    IMAP_PAF_CMD_SEARCH       /* currently searching data for a command */
} ImapPafState;

typedef enum _ImapDataEnd
{
    IMAP_PAF_DATA_END_UNKNOWN,
    IMAP_PAF_DATA_END_PAREN
} ImapDataEnd;

/* State tracker for POP PAF */
typedef struct _ImapPafData
{
    MimeDataPafInfo mime_info;    /* Mime response information */
    ImapPafState imap_state;      /* The current IMAP paf stat */
    bool end_of_data;
    ImapDataInfo imap_data_info;  /* Used for parsing data */
    ImapDataEnd data_end_state;
//    uint32_t length;            TODO -- parse and add literal length
} ImapPafData;

static inline void reset_data_states(ImapPafData *pfdata)
{
    // reset MIME info
    _dpd.fileAPI->reset_mime_paf_state(&(pfdata->mime_info));

    // reset server info
    pfdata->imap_state = IMAP_PAF_CMD_IDENTIFIER;

    // reset fetch data information information
    pfdata->imap_data_info.paren_cnt = 0;
    pfdata->imap_data_info.next_letter = 0;
    pfdata->imap_data_info.length = 0;
}

static inline bool is_untagged(const uint8_t ch)
{
    return (ch == '*' || ch == '+');
}

static bool parse_literal_length(const uint8_t ch, uint32_t *len)
{
    uint32_t length = *len;

    if (isdigit(ch))
    {
        uint64_t tmp_len = (10 * length)  + (ch - '0');
        if (tmp_len < UINT32_MAX)
        {
            *len = (uint32_t) tmp_len;
            return false;
        }
        else
        {
            *len = 0;
        }
    } 
    else if (ch != '}')
        *len = 0;  //  ALERT!!  charachter should be a digit or ''}'' 

    return true;  
}

static void parse_fetch_header(const uint8_t ch, ImapPafData *pfdata)
{
    if (pfdata->imap_data_info.esc_nxt_char)
    {
        pfdata->imap_data_info.esc_nxt_char = false;
    }
    else
    {
        switch(ch)
        {
        case '{':
            pfdata->imap_state = IMAP_PAF_DATA_LEN_STATE;
            break;
        case '(':
            pfdata->imap_data_info.paren_cnt++;
            break;

        case ')':
            if (pfdata->imap_data_info.paren_cnt > 0)
                pfdata->imap_data_info.paren_cnt--;
            break;

        case '\n':
            if (pfdata->imap_data_info.paren_cnt)
            {
                pfdata->imap_state = IMAP_PAF_DATA_STATE;
            }
            else
            {
                reset_data_states(pfdata);
            }
            break;

        case '\\':
            pfdata->imap_data_info.esc_nxt_char = true;
            break;

        default:
            break;
        }
    }
}

/* 
 * Statefully search for the single line termination sequence LF ("\n").
 *
 * PARAMS:
 *        const uint8_t ch - the next character to analyze.
 *        ImapPafData *pfdata - the struct containing all imap paf information
 *
 * RETURNS:
 *        false - if termination sequence not found 
 *        true - if termination sequence found
 */
static bool find_data_end_single_line(const uint8_t ch, ImapPafData *pfdata)
{
    if (ch == '\n')
    {
        reset_data_states(pfdata);
        return true;
    }
    return false;
}

/* Flush based on data length*/
static inline bool literal_complete(ImapPafData *pfdata)
{
    if (pfdata->imap_data_info.length)
    {
        pfdata->imap_data_info.length--;
        if (pfdata->imap_data_info.length)
            return false;
    }

    return true;
}

static bool check_imap_data_end(ImapDataEnd *data_end_state,  uint8_t val)
{
    switch(*data_end_state)
    {

    
    case IMAP_PAF_DATA_END_UNKNOWN:
        if (val == ')')
            *data_end_state = (ImapDataEnd) IMAP_PAF_DATA_END_PAREN;
        break;

    case IMAP_PAF_DATA_END_PAREN:
        if (val == '\n')
        {
            *data_end_state = (ImapDataEnd) PAF_DATA_END_UNKNOWN;
            return true;
        }
        else if (val != '\r')
        {
            *data_end_state = (ImapDataEnd) PAF_DATA_END_UNKNOWN;
        }
        break;
    
    default:
        break;
    }

    return false;
}

/* 
 * Statefully search for the data termination sequence or a MIME boundary.
 *
 * PARAMS:
 *        const uint8_t ch - the next character to analyze.
 *        ImapPafData *pfdata - the struct containing all imap paf information
 *
 * RETURNS:
 *        false - if termination sequence not found 
 *        true - if termination sequence found
 */
static bool find_data_end_mime_data(const uint8_t ch, ImapPafData *pfdata)
{
    if (literal_complete(pfdata)
         && check_imap_data_end(&(pfdata->data_end_state), ch))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "IMAP PAF: End of Data!\n"););
        reset_data_states(pfdata);
        return true;
    }


    // check for mime flush point
    if (_dpd.fileAPI->process_mime_paf_data(&(pfdata->mime_info), ch))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "IMAP PAF: Mime Boundary found."
                    " Flushing data!\n"););
        return true;
	}

    return false;
}


/*
 * Initial command processing function.  Determine if this command
 * may be analyzed irregularly ( which currently means if emails
 * and email attachments need to be analyzed).
 * 
 * PARAMS: 
 *        const uint8_t ch - the next character to analyze.
 *        ImapPafData *pfdata - the struct containing all imap paf information
 */
static inline void init_command_search(const uint8_t ch, ImapPafData *pfdata)
{
    switch(ch)
    {
    case 'F':
    case 'f':
        // may be a FETCH response
        pfdata->imap_data_info.next_letter = &(imap_resps[RESP_FETCH].name[1]);
        break;

    default:
        // this is not a data command. Search for regular end of line.
        pfdata->imap_state = IMAP_PAF_REG_STATE;
    }
}


/*
 * Confirms every character in the current sequence is part of the expected 
 * command. After confirmation is complete, IMAP PAF will begin searching
 * for data. If any character is unexpected, searches for the default 
 * termination sequence. 
 * 
 * PARAMS: 
 *        const uint8_t ch - the next character to analyze.
 *        ImapPafData *pfdata - the struct containing all imap paf information
 */
static inline void parse_command(const uint8_t ch, ImapPafData *pfdata)
{
    char val = *(pfdata->imap_data_info.next_letter);

    if (val == '\0' && isblank(ch))
        pfdata->imap_state = IMAP_PAF_DATA_HEAD_STATE;

    else if (toupper(ch) == toupper(val))
        pfdata->imap_data_info.next_letter++;

    else
        pfdata->imap_state = IMAP_PAF_REG_STATE;
}


/*
 * Wrapper function for the command parser.  Determines whether this is the
 * first letter being processed and calls the appropriate processing 
 * function.
 * 
 * PARAMS: 
 *        const uint8_t ch - the next character to analyze.
 *        ImapPafData *pfdata - the struct containing all imap paf information
 */
static inline void process_command(const uint8_t ch, ImapPafData *pfdata)
{
    if (pfdata->imap_data_info.next_letter)
        parse_command(ch, pfdata);
    else
        init_command_search(ch, pfdata);
}

/*
 * This function only does something when the character is a blank or a CR/LF.
 * In those specific cases, this function will set the appropriate next
 * state information
 *
 * PARAMS:
 *        const uint8_t ch - the next character to analyze.
 *        ImapPafData *pfdata - the struct containing all imap paf information
 *        ImapPafData base_state - if a space is not found, revert to this state
 *        ImapPafData next_state - if a space is found, go to this state
 * RETURNS:
 *        true - if the status has been eaten
 *        false - if a CR or LF has been found
 */
static inline void eat_character(const uint8_t ch, ImapPafData *pfdata, 
                ImapPafState base_state, ImapPafState next_state)
{
    switch(ch)
    {
    case ' ':
    case '\t':
        pfdata->imap_state = next_state;
        break;;

    case '\r': 
    case '\n':
        pfdata->imap_state = base_state;
        break;
    }

}


/*
 *  defined above in the eat_character function
 *
 * Keeping the next two functions to ease any future development 
 * where these cases will no longer be simple or identical
 */
static inline void eat_second_argument(const uint8_t ch, ImapPafData *pfdata)
{
    eat_character(ch, pfdata, IMAP_PAF_REG_STATE, IMAP_PAF_CMD_SEARCH);
}

/* explanation in 'eat_second_argument' above */
static inline void eat_response_identifier(const uint8_t ch, ImapPafData *pfdata)
{
    eat_character(ch, pfdata, IMAP_PAF_REG_STATE, IMAP_PAF_CMD_STATUS);
}


/*
 * Analyzes the current data for a correct flush point.  Flushes when 
 * a command is complete or a MIME boundary is found.
 *
 * PARAMS:
 *    ImapPafData *pfdata - ImapPaf state tracking structure
 *    const uint8_t *data - payload data to inspect
 *    uint32_t len - length of payload data
 *    uint32_t * fp- pointer to set flush point
 *
 * RETURNS:
 *    PAF_Status - PAF_FLUSH if flush point found, 
 *    PAF_SEARCH otherwise
 */
static PAF_Status imap_paf_server(ImapPafData *pfdata,
        const uint8_t* data, uint32_t len, uint32_t* fp)
{
    uint32_t i;
    uint32_t flush_len = 0;
    uint32_t boundary_start = 0;

    pfdata->end_of_data = false;

    for (i = 0; i < len; i++)
    {
        uint8_t ch = data[i];
        switch(pfdata->imap_state)
        {
        case IMAP_PAF_CMD_IDENTIFIER:
            // can be '+', '*', or a tag
            if (is_untagged(ch))
            {
                // continue checking for fetch command
                pfdata->imap_state = IMAP_PAF_CMD_TAG;
            }
            else
            {
                // end of a command.  flush at end of line.
                pfdata->imap_state = IMAP_PAF_FLUSH_STATE;
            }
            break;

        case IMAP_PAF_CMD_TAG:
            eat_response_identifier(ch, pfdata);
            break;

        case IMAP_PAF_CMD_STATUS:
            // can be a command name, msg sequence number, msg count, etc...
            // since we are only interested in fetch, eat this argument
            eat_second_argument(ch, pfdata);
            break;

        case IMAP_PAF_CMD_SEARCH:
            process_command(ch, pfdata);
            find_data_end_single_line(ch, pfdata);
            break;

        case IMAP_PAF_REG_STATE:
            find_data_end_single_line(ch, pfdata); // data reset when end of line hit
            break;

        case IMAP_PAF_DATA_HEAD_STATE:
            parse_fetch_header(ch, pfdata); // function will change state
            break;

        case IMAP_PAF_DATA_LEN_STATE:
            if (parse_literal_length(ch, &(pfdata->imap_data_info.length)))
            {
                pfdata->imap_state = IMAP_PAF_DATA_HEAD_STATE;
            }
            break;

        case IMAP_PAF_DATA_STATE:
            if (find_data_end_mime_data(ch, pfdata))
            {
                // if not a boundary, wait for end of
                // the server's response before flushing
                if (pfdata->imap_state == IMAP_PAF_DATA_STATE)
                {
                    *fp = i + 1;
                    return PAF_FLUSH;
                }
            }
            if (pfdata->mime_info.boundary_state == MIME_PAF_BOUNDARY_UNKNOWN)
                boundary_start = i;
            break;

        case IMAP_PAF_FLUSH_STATE:
            if (find_data_end_single_line(ch, pfdata))
            {
                flush_len = i +1;
            }
            break;
        }

    }

    if (flush_len)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "IMAP PAF: flushing data!\n"););

        // flush at the final termination sequence
        *fp = flush_len;
        return PAF_FLUSH;
    }

    if ( scanning_boundary(&pfdata->mime_info, boundary_start, fp) )
        return PAF_LIMIT;

    return PAF_SEARCH;
}

/*
 * Searches through the current data for a LF.  All client
 * commands end with this termination sequence
 *
 * PARAMS:
 *    ImapPafData *pfdata - ImapPaf state tracking structure
 *    const uint8_t *data - payload data to inspect
 *    uint32_t len - length of payload data
 *    uint32_t * fp- pointer to set flush point
 *
 * RETURNS:
 *    PAF_Status - PAF_FLUSH if flush point found, 
 *    PAF_SEARCH otherwise
 */
static PAF_Status imap_paf_client(ImapPafData *pfdata,
        const uint8_t* data, uint32_t len, uint32_t* fp)
{
    const char *pch;

    pch = memchr (data, '\n', len);

    if (pch != NULL)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "IMAP PAF: Flushing client"
                                    " data!\n"););
        *fp = (uint32_t)(pch - (const char*)data) + 1;
        return PAF_FLUSH;
    }
    return PAF_SEARCH;
}

/* Function: imap_paf()
 *
 *  Purpose: IMAP PAF callback.
 *           Inspects imap traffic.  All client traffic will be flushed after 
 *           every single line termination sequence (LF == '\n'). When 
 *           analyzing server traffic, will flush after the final termination
 *           sequence in the current PDU.  However, if the server is returning
 *           an email, a flush will occur after every MIME boundary and at the 
 *           end of the email.
 *
 *  Arguments:
 *    void *ssn - stream5 session pointer
 *    void **ps - ImapPaf state tracking structure
 *    const uint8_t *data - payload data to inspect
 *    uint32_t len - length of payload data
 *    uint32_t flags- flags to check whether client or server
 *    uint32_t * fp- pointer to set flush point
 *    uint32_t * fp_eoh - pointer to set header flush point
 *
 * Returns:
 *   PAF_Status - PAF_FLUSH if flush point found, PAF_SEARCH otherwise
 */

static PAF_Status imap_paf(void* ssn, void** ps, const uint8_t* data,
        uint32_t len, uint64_t *flags, uint32_t* fp, uint32_t* fp_eoh)
{

    ImapPafData *pfdata = *(ImapPafData **)ps;

    if (pfdata == NULL)
    {
        /* IMAP has long running sessions, avoid the paf check*/
        //if (_dpd.fileAPI->check_paf_abort(ssn))
          //  return PAF_ABORT;

        pfdata = _dpd.snortAlloc(1, sizeof(*pfdata), PP_IMAP, PP_MEM_CATEGORY_SESSION);
        if (pfdata == NULL)
        {
            return PAF_ABORT;
        }

        reset_data_states(pfdata);
        *ps = pfdata;
    }


    if (*flags & FLAG_FROM_SERVER)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "IMAP PAF: From server.\n"););
        return imap_paf_server(pfdata, data, len, fp);
    }
    else
    {
    	DEBUG_WRAP(DebugMessage(DEBUG_IMAP, "IMAP PAF: From client.\n"););
        return imap_paf_client(pfdata, data, len, fp);
    }
}

bool is_data_end (void* ssn)
{
    if ( ssn )
    {
        ImapPafData** s = (ImapPafData **)_dpd.streamAPI->get_paf_user_data(ssn, 1, imap_paf_id);

        if ( s && (*s) )
            return ((*s)->end_of_data);
    }

    return false;
}

void imap_paf_cleanup(void *pafData)
{
   if(pafData) {
      _dpd.snortFree(pafData, sizeof(ImapPafData), PP_IMAP, PP_MEM_CATEGORY_SESSION);
   }
}

#ifdef TARGET_BASED
void register_imap_paf_service (struct _SnortConfig *sc, int16_t app, tSfPolicyId policy)
{
    if (_dpd.isPafEnabled())
    {
        imap_paf_id = _dpd.streamAPI->register_paf_service(sc, policy, app, true, imap_paf, true);
        imap_paf_id = _dpd.streamAPI->register_paf_service(sc, policy, app, false,imap_paf, true);
        _dpd.streamAPI->register_paf_free(imap_paf_id, imap_paf_cleanup);
    }
}
#endif


void register_imap_paf_port(struct _SnortConfig *sc, unsigned int i, tSfPolicyId policy)
{
    if (_dpd.isPafEnabled())
    {
        imap_paf_id = _dpd.streamAPI->register_paf_port(sc, policy, (uint16_t)i, true, imap_paf, true);
        imap_paf_id = _dpd.streamAPI->register_paf_port(sc, policy, (uint16_t)i, false, imap_paf, true);
        _dpd.streamAPI->register_paf_free(imap_paf_id, imap_paf_cleanup);
    }
}
