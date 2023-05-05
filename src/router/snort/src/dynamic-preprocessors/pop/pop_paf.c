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
#include "pop_paf.h"
#include "spp_pop.h"
#include "sf_dynamic_preprocessor.h"
#include "stream_api.h"
#include "snort_pop.h"
#include "pop_config.h"
#include "file_api.h"
#include "pop_util.h"

static uint8_t pop_paf_id = 0;

// global variables defined in snort_pop.c
extern const POPToken pop_known_cmds[];

/* Structure used to record expected server termination sequence */
typedef enum _PopExpectedResp
{
    POP_PAF_SINGLE_LINE_STATE,  /* server response will end with \r\n */
    POP_PAF_MULTI_LINE_STATE,   /* server response will end with \r\n.\r\n */
    POP_PAF_DATA_STATE,         /* Indicated MIME will be contained in response */
    POP_PAF_HAS_ARG             /* Intermediate state when parsing LIST */
} PopExpectedResp;


typedef enum _PopParseCmdState
{
    POP_CMD_SEARCH,     /* Search for Command */
    POP_CMD_FIN,        /* Found space. Finished parsing Command */
    POP_CMD_ARG         /* Parsing command with multi-line response iff arg given */
} PopParseCmdState;


/* saves data when parsing client commands */
typedef struct _PopPafParseCmd
{
    char *next_letter;          /* a pointer to the current commands data */
    PopExpectedResp exp_resp;   /* the expected termination sequence for this command */
    PopParseCmdState status;    /* whether the current has already been found */
} PopPafParseCmd;


/* State tracker for POP PAF */
typedef struct _PopPafData
{
    PopExpectedResp pop_state;   /* The current POP PAF state. */
    DataEndState end_state;   /* Current termination sequence state */
    PopPafParseCmd cmd_state;    /* all of the command parsing data */
    MimeDataPafInfo data_info;   /* Mime Information */
    bool cmd_continued;          /* data continued from previous packet? */
    bool end_of_data;
//    uint32_t length;           /* TODO --> use length in top and RETR */
} PopPafData;



/*
 *  read process_command() description below
 */
static bool search_for_command(PopPafData *pfdata, const uint8_t ch)
{
    char val = *(pfdata->cmd_state.next_letter);

    // if end of command && data contains a space or newline
    if (val == '\0'  && (isblank(ch) || ch == '\r' || ch == '\n'))
    {
        if (pfdata->cmd_state.exp_resp == POP_PAF_HAS_ARG)
        {
            pfdata->cmd_state.status = POP_CMD_ARG;
        }
        else 
        {
            pfdata->cmd_state.status = POP_CMD_FIN;
            pfdata->pop_state = pfdata->cmd_state.exp_resp;
            return true;
        }
    }
    else if (toupper(ch) == toupper(val) )
    {
        pfdata->cmd_state.next_letter++;
    }
    else
    {
        pfdata->cmd_state.status = POP_CMD_FIN;
    }

    return false;
}


/*
 *  read process_command() description below
 */
static bool init_command_search(PopPafData *pfdata, const uint8_t ch)
{
    pfdata->pop_state = POP_PAF_SINGLE_LINE_STATE;

    switch(ch)
    {
    case 'c':
    case 'C':
        pfdata->cmd_state.exp_resp = POP_PAF_MULTI_LINE_STATE;
        pfdata->cmd_state.next_letter = &(pop_known_cmds[CMD_CAPA].name[1]);
        break;
    case 'l':
    case 'L':
        pfdata->cmd_state.exp_resp = POP_PAF_HAS_ARG;
        pfdata->cmd_state.next_letter = &(pop_known_cmds[CMD_LIST].name[1]);
        break;
    case 'r':
    case 'R':
        pfdata->cmd_state.exp_resp = POP_PAF_DATA_STATE;
        pfdata->cmd_state.next_letter = &(pop_known_cmds[CMD_RETR].name[1]);
        break;
    case 't':
    case 'T':
        pfdata->cmd_state.exp_resp = POP_PAF_DATA_STATE;
        pfdata->cmd_state.next_letter = &(pop_known_cmds[CMD_TOP].name[1]);
        break;
    case 'u':
    case 'U':
        pfdata->cmd_state.exp_resp = POP_PAF_HAS_ARG;
        pfdata->cmd_state.next_letter = &(pop_known_cmds[CMD_UIDL].name[1]);
        break;
    default:
        pfdata->cmd_state.status = POP_CMD_FIN;
    }

    return false;
}

/*
 * Attempts to determine the current command based upon the given character
 * If another character is required to determine the current command,
 * sets the function pointer to the correct next state
 *
 * PARAMS:
 *         pop_cmd - a pointer to the struct containing all of the 
 *                     relevant parsing info
 *         ch      - the first character from the clients command
 * RETURNS
 *         true  - if the expected response is NOT a single line
 *         false - otherwise
 */
static inline bool process_command(PopPafData *pfdata, const uint8_t ch)
{
    if (pfdata->cmd_state.next_letter)
        return search_for_command(pfdata, ch);
    else
        return init_command_search(pfdata, ch);
}


static inline void reset_data_states(PopPafData *pfdata)
{
    // reset MIME info
    _dpd.fileAPI->reset_mime_paf_state(&(pfdata->data_info));

    // reset general pop fields
    pfdata->cmd_continued = false;
    pfdata->end_state = PAF_DATA_END_UNKNOWN;
    pfdata->pop_state = POP_PAF_SINGLE_LINE_STATE;
}


/*
 *  Checks if the current data is a valid response.
 *  According to RFC 1939, every response begins with either 
 *     +OK
 *     -ERR.
 *  
 *  RETURNS:
 *           true - if the character is a +
 *           false - if the character is anything else
 */ 
static inline int valid_response(const uint8_t data)
{
    return (data == '+');
}

/*
 * Client PAF calls this command to set the server's state.  This is the
 * function which ensure's the server know the correct expected
 * DATA
 */
static inline void set_server_state(void *ssn, PopExpectedResp state)
{
    PopPafData *server_data = *(_dpd.streamAPI->get_paf_user_data(ssn, 0, pop_paf_id));

    // ERROR IF SERVER DATA DOES NOT EXIST!! SHOULD NOT BE POSSIBLE!!
    if (server_data)
    {
        reset_data_states(server_data);
        server_data->end_of_data = false;
        server_data->pop_state = state;
    }
}

/*
 * A helper function to reset the client's command parsing 
 * information 
 */
static inline void reset_client_cmd_info(PopPafData *pfdata)
{
    pfdata->cmd_state.next_letter = NULL;
    pfdata->cmd_state.status = POP_CMD_SEARCH;
}

/*
 * Statefully search for the termination sequence CRCL.CRLF ("\r\n.\r\n").  
 *
 * PARAMS:
 *        mime_data : true if this is mime_data.
 *
 * RETURNS:
 *         0 - if termination sequence not found 
 *         1 - if termination sequence found
 */
static bool find_data_end_multi_line(PopPafData *pfdata, const uint8_t ch, bool mime_data)
{
    // TODO:  This will currently flush on MIME boundary, and one line later at end of PDU

    if (_dpd.fileAPI->check_data_end(&(pfdata->end_state), ch))
    {
        DEBUG_WRAP(DebugMessage(DEBUG_POP, "End of Multi-line response found\n"););
        pfdata->end_of_data = true;
        pfdata->pop_state = POP_PAF_SINGLE_LINE_STATE;
        reset_data_states(pfdata);
        return true;
    }

    // if this is a data command, search for MIME ending
    if (mime_data)
    {
        if (_dpd.fileAPI->process_mime_paf_data(&(pfdata->data_info), ch))
        {
            DEBUG_WRAP(DebugMessage(DEBUG_POP, "Mime Boundary found.  Flushing data!\n"););
            pfdata->cmd_continued = true;
            return true;
        }
    }


    return false;
}

/*
 * Statefully search for the termination sequence LF ("\n").  Will also
 * set the correct response state.
 *
 * PARAMS:
 *
 * RETURNS:
 *         0 - if terminatino sequence not found 
 *         1 - if termination sequence found
 */
static inline bool find_data_end_single_line(PopPafData *pfdata, const uint8_t ch, bool client)
{
    if (ch == '\n')
    {
        // reset the correct information
        if (client)
            reset_client_cmd_info(pfdata);
        else
            reset_data_states(pfdata);

        DEBUG_WRAP(DebugMessage(DEBUG_POP, "End of single-line response "
                "found.  Flushing data!\n"););
        return true;
    }

    return false;
}


static PAF_Status pop_paf_server(PopPafData *pfdata,
        const uint8_t* data, uint32_t len, uint32_t* fp)
{
    uint32_t i;
    uint32_t boundary_start = 0;

    // if a negative response was received, it will be a one line response.
    if (!pfdata->cmd_continued && !valid_response(*data))
        pfdata->pop_state = POP_PAF_SINGLE_LINE_STATE;


    for (i = 0; i < len; i++)
    {
        uint8_t ch = data[i];

        // find the termination sequence based upon the current state
        switch (pfdata->pop_state)
        {
        case POP_PAF_MULTI_LINE_STATE:
            if( find_data_end_multi_line(pfdata, ch, false) )
            {
                *fp = i + 1;
                return PAF_FLUSH;
            }
            break;
        
        case POP_PAF_DATA_STATE:
            // TODO --> statefully get length
            if ( find_data_end_multi_line(pfdata, ch, true) )
            {
                *fp = i + 1;
                return PAF_FLUSH;
            }

            if (pfdata->data_info.boundary_state == MIME_PAF_BOUNDARY_UNKNOWN)
                boundary_start = i;

            break;

        case POP_PAF_SINGLE_LINE_STATE:
        default:
            if ( find_data_end_single_line(pfdata, ch, false) )
            {
                *fp = i + 1;
                return PAF_FLUSH;
            }
            break;
        }
    }

    pfdata->cmd_continued = true;    


    if ( scanning_boundary(&pfdata->data_info, boundary_start, fp) )
        return PAF_LIMIT;

    return PAF_SEARCH;
}


/*
 * Determine the Client's command and set the response state.
 * Flush data when "\r\n" is received
 */
static PAF_Status pop_paf_client(void *ssn, PopPafData *pfdata,
        const uint8_t* data, uint32_t len, uint32_t* fp)
{
    uint32_t i;

    // TODO ... ensure current command is smaller than max command length

    for (i = 0; i < len; i++)
    {
        uint8_t ch = data[i];

        switch (pfdata->cmd_state.status)
        {
        case POP_CMD_SEARCH:
            if (process_command(pfdata, ch) )
            {
                set_server_state(ssn, pfdata->pop_state);
            }
            
            //break;  DO NOT UNCOMMENT!!  both cases should check for a LF.
        
        case POP_CMD_FIN:
            if (find_data_end_single_line(pfdata, ch, true) )
            {
                // reset command parsing data
                *fp = i + 1;
                return PAF_FLUSH;
            }
            break;

        case POP_CMD_ARG:
            if (find_data_end_single_line(pfdata, ch, true))
            {
                set_server_state(ssn, POP_PAF_MULTI_LINE_STATE);
                *fp = i + 1;
                return PAF_FLUSH;
            }
            else if (isdigit(ch))
            {
                pfdata->cmd_state.status = POP_CMD_FIN;
            }
        }
    }

    return PAF_SEARCH;
}

/* Function: pop_paf()

   Purpose: POP PAF callback.
            Inspects pop traffic.  Checks client traffic for the current command
            and sets correct server termination sequence. Client side data will
            flush after receiving CRLF ("\r\n").  Server data flushes after 
            finding set termination sequence.

   Arguments:
     void * - stream5 session pointer
     void ** - DNP3 state tracking structure
     const uint8_t * - payload data to inspect
     uint32_t - length of payload data
     uint32_t - flags to check whether client or server
     uint32_t * - pointer to set flush point
     uint32_t * - pointer to set header flush point

   Returns:
    PAF_Status - PAF_FLUSH if flush point found, PAF_SEARCH otherwise
*/

static PAF_Status pop_paf(void* ssn, void** ps, const uint8_t* data,
        uint32_t len, uint64_t *flags, uint32_t* fp, uint32_t* fp_eoh)
{

    PopPafData *pfdata = *(PopPafData **)ps;

    if (pfdata == NULL)
    {
        if (_dpd.fileAPI->check_paf_abort(ssn))
            return PAF_ABORT;
        
        pfdata = _dpd.snortAlloc(1, sizeof(*pfdata), PP_POP, 0);
        
        if (pfdata == NULL)
        {
            return PAF_ABORT;
        }

        reset_data_states(pfdata);
        *ps = pfdata;
    }


    if (*flags & FLAG_FROM_SERVER)
    {
       DEBUG_WRAP(DebugMessage(DEBUG_POP, "PAF: From server.\n"););
        return pop_paf_server(pfdata, data, len, fp);
    }
    else
    {
        DEBUG_WRAP(DebugMessage(DEBUG_POP, "PAF: From client.\n"););
        return pop_paf_client(ssn, pfdata, data, len, fp);
    }
}

bool is_data_end (void* ssn)
{
    if ( ssn )
    {
        PopPafData** s = (PopPafData **)_dpd.streamAPI->get_paf_user_data(ssn, 0, pop_paf_id);

        if ( s && (*s) )
            return ((*s)->end_of_data);
    }

    return false;
}

void pop_paf_cleanup(void *pafData)
{
   if (pafData) {
      _dpd.snortFree(pafData, sizeof(PopPafData), PP_POP, 0);
   }
}

#ifdef TARGET_BASED
void register_pop_paf_service (struct _SnortConfig *sc, int16_t app, tSfPolicyId policy)
{
    if (_dpd.isPafEnabled())
    {
       pop_paf_id = _dpd.streamAPI->register_paf_service(sc, policy, app, true, pop_paf, true);
       pop_paf_id = _dpd.streamAPI->register_paf_service(sc, policy, app, false,pop_paf, true);
       _dpd.streamAPI->register_paf_free(pop_paf_id, pop_paf_cleanup);
    }
}
#endif


void register_pop_paf_port(struct _SnortConfig *sc, unsigned int i, tSfPolicyId policy)
{
    if (_dpd.isPafEnabled())
    {
        pop_paf_id = _dpd.streamAPI->register_paf_port(sc, policy, (uint16_t)i, true, pop_paf, true);
        pop_paf_id = _dpd.streamAPI->register_paf_port(sc, policy, (uint16_t)i, false, pop_paf, true);
	_dpd.streamAPI->register_paf_free(pop_paf_id, pop_paf_cleanup);
    }
}
