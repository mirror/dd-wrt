/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2012-2013 Sourcefire, Inc.
 **
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License Version 2 as
 ** published by the Free Software Foundation.  You may not use, modify or
 ** distribute this program under any other version of the GNU General
 ** Public License.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **
 **  Author(s):  Hui Cao <hcao@sourcefire.com>
 **
 **  NOTES
 **  9.25.2012 - Initial Source Code. Hcao
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "file_mail_common.h"
#include "file_mime_process.h"
#include "mempool.h"
#include "file_api.h"
#include "snort_bounds.h"
#include "util.h"
#include "str_search.h"
#include "decode.h"
#include "detection_util.h"

#include "memory_stats.h"
#include "stream_api.h"
#include "reg_test.h"
#include <file_lib.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

/* state flags */
#define MIME_FLAG_FOLDING                    0x00000001
#define MIME_FLAG_IN_CONTENT_TYPE            0x00000002
#define MIME_FLAG_GOT_BOUNDARY               0x00000004
#define MIME_FLAG_DATA_HEADER_CONT           0x00000008
#define MIME_FLAG_IN_CONT_TRANS_ENC          0x00000010
#define MIME_FLAG_EMAIL_ATTACH               0x00000020
#define MIME_FLAG_MULTIPLE_EMAIL_ATTACH      0x00000040
#define MIME_FLAG_MIME_END                   0x00000080
#define MIME_FLAG_IN_CONT_DISP               0x00000200
#define MIME_FLAG_IN_CONT_DISP_CONT          0x00000400

#define STATE_DATA_INIT    0
#define STATE_DATA_HEADER  1    /* Data header section of data state */
#define STATE_DATA_BODY    2    /* Data body section of data state */
#define STATE_MIME_HEADER  3    /* MIME header section within data section */
#define STATE_DATA_UNKNOWN 4

/* Maximum length of header chars before colon, based on Exim 4.32 exploit */
#define MAX_HEADER_NAME_LEN 64

typedef struct _MimeToken
{
    char *name;
    int   name_len;
    int   search_id;

} MimeToken;

typedef enum _MimeHdrEnum
{
    HDR_CONTENT_TYPE = 0,
    HDR_CONT_TRANS_ENC,
    HDR_CONT_DISP,
    HDR_LAST

} MimeHdrEnum;

const MimeToken mime_hdrs[] =
{
        {"Content-type:", 13, HDR_CONTENT_TYPE},
        {"Content-Transfer-Encoding:", 26, HDR_CONT_TRANS_ENC},
        {"Content-Disposition:", 20, HDR_CONT_DISP},
        {NULL,             0, 0}
};

typedef struct _MIMESearch
{
    char *name;
    int   name_len;

} MIMESearch;

typedef struct _MIMESearchInfo
{
    int id;
    int index;
    int length;

} MIMESearchInfo;

MIMESearchInfo mime_search_info;

void *mime_hdr_search_mpse = NULL;
MIMESearch mime_hdr_search[HDR_LAST];
MIMESearch *mime_current_search = NULL;
static const char *boundary_str = "boundary=";
static char *preprocessor = NULL;

/* Extract the filename from the header */
static inline int extract_file_name(const char **start, int length, bool *disp_cont)
{
    const char *tmp = NULL;
    const char *end = *start+length;

    if (length <= 0)
        return -1;


    if (!(*disp_cont))
    {
        tmp = SnortStrcasestr(*start, length, "filename");

        if( tmp == NULL )
            return -1;

        tmp = tmp + 8;
        while( (tmp < end) && ((isspace(*tmp)) || (*tmp == '=') ))
        {
            tmp++;
        }
    }
    else
        tmp = *start;

    if(tmp < end)
    {
        if(*tmp == '"' || (*disp_cont))
        {
            if(*tmp == '"')
            {
                if(*disp_cont)
                {
                    *disp_cont = false;
                    return (tmp - *start);
                }
                tmp++;

            }
            *start = tmp;
            tmp = SnortStrnPbrk(*start ,(end - tmp),"\"");
            if(tmp == NULL )
            {
                if ((end - tmp) > 0 )
                {
                    tmp = end;
                    *disp_cont = true;
                }
                else
                    return -1;
            }
            else
                *disp_cont = false;
            end = tmp;
        }
        else
        {
            *start = tmp;
        }
        return (end - *start);
    }
    else
    {
        return -1;
    }

}

/* accumulate MIME attachment filenames. The filenames are appended by commas
 * start - If extract_fname is true,start is ptr in MIME header
 *         to extract file names from. Else, if extract_fname
 *         is false, start is the filename
 *  length - If extract_fname is false , length is the strlen of
 *          filename contained in start
 *
 * log_state - file log state
 * disp_cont
 * extract_fname - false, when filename is already extracted
 *                        and passed to the first argument
 */

int log_file_name(const uint8_t *start, int length, FILE_LogState *log_state, bool *disp_cont, bool extract_fname)
{
    uint8_t *alt_buf;
    int alt_size;
    uint16_t *alt_len;
    int ret=0;
    int cont =0;
    int log_avail = 0;

    if ( extract_fname )
    {
        if(!start || (length <= 0))
        {
            *disp_cont = false;
            return -1;
        }

        if(*disp_cont)
            cont = 1;

        ret = extract_file_name((const char **)(&start), length, disp_cont);
    }
    else
    {
        if(*disp_cont)
            cont = 1;
         /* Since the file name is already passed as parameter and the length as well,
          * just set ret here*/
         ret = length;
    }

    if (ret == -1)
        return ret;

    length = ret;

    alt_buf = log_state->filenames;
    alt_size =  MAX_FILE;
    alt_len = &(log_state->file_logged);
    log_avail = alt_size - *alt_len;

    if(!alt_buf || (log_avail <= 0))
        return -1;


    if ( *alt_len > 0 && ((*alt_len + 1) < alt_size))
    {
        if(!cont)
        {
            alt_buf[*alt_len] = ',';
            *alt_len = *alt_len + 1;
        }
    }

    ret = SafeMemcpy(alt_buf + *alt_len, start, length, alt_buf, alt_buf + alt_size);

    if (ret != SAFEMEM_SUCCESS)
    {
        if(*alt_len != 0)
            *alt_len = *alt_len - 1;
        return -1;
    }

    log_state->file_current = *alt_len;
    *alt_len += length;

    return 0;
}

static void update_file_name(MAIL_LogState *log_state)
{
    if (log_state)
        log_state->file_log.file_name = log_state->file_log.file_current;
}

static void set_file_name_from_log(FILE_LogState *log_state, void *ssn)
{
    if ((log_state) && (log_state->file_logged > log_state->file_current))
    {
        if (log_state->file_current > log_state->file_name)
            file_api->set_file_name(ssn, log_state->filenames + log_state->file_name,
                    log_state->file_current -log_state->file_name - 1, false);
        else
            file_api->set_file_name(ssn, log_state->filenames + log_state->file_current,
                    log_state->file_logged -log_state->file_current, false);
    }
    else
    {
        file_api->set_file_name(ssn, NULL, 0, false);
    }
}
/*
 * Return: 0: success
 *         -1: fail
 *
 */
int set_log_buffers(MAIL_LogState **log_state, MAIL_LogConfig *conf, void *mempool, void* scbPtr, uint32_t preproc_id)
{
    MemPool *log_mempool = (MemPool *)mempool;

    if((*log_state == NULL)
            && (conf->log_email_hdrs || conf->log_filename
                    || conf->log_mailfrom || conf->log_rcptto))
    {
        MemBucket *bkt = mempool_alloc(log_mempool);

        if(bkt == NULL)
            return -1;

        *log_state = (MAIL_LogState *)SnortPreprocAlloc(1, sizeof(MAIL_LogState), preproc_id, 0);
        if((*log_state) != NULL)
        {
            bkt->scbPtr = scbPtr;
            (*log_state)->log_hdrs_bkt = bkt;
            (*log_state)->log_depth = conf->email_hdrs_log_depth;
            (*log_state)->recipients = (uint8_t *)bkt->data;
            (*log_state)->rcpts_logged = 0;
            (*log_state)->senders = (uint8_t *)bkt->data + MAX_EMAIL;
            (*log_state)->snds_logged = 0;
            (*log_state)->file_log.filenames = (uint8_t *)bkt->data + (2*MAX_EMAIL);
            (*log_state)->file_log.file_logged = 0;
            (*log_state)->file_log.file_current = 0;
            (*log_state)->file_log.file_name = 0;
            (*log_state)->emailHdrs = (unsigned char *)bkt->data + (2*MAX_EMAIL) + MAX_FILE;
            (*log_state)->hdrs_logged = 0;
        }
        else
        {
            /*free bkt if calloc fails*/
            mempool_free(log_mempool, bkt);
            return -2;
        }

    }
    return 0;
}

static void set_mime_buffers(MimeState *ssn, void* scbPtr, uint32_t preproc_id)
{
    if ((ssn != NULL) && (ssn->decode_state == NULL))
    {
        MemBucket *bkt = mempool_alloc(ssn->mime_mempool);
        DecodeConfig *conf= ssn->decode_conf;

        if (bkt != NULL)
        {
            ssn->decode_state = SnortPreprocAlloc(1, sizeof(Email_DecodeState), preproc_id, 0);
            if((ssn->decode_state) != NULL )
            {
                bkt->scbPtr = scbPtr;
                ssn->decode_bkt = bkt;
                SetEmailDecodeState((Email_DecodeState *)(ssn->decode_state), bkt->data, conf->max_depth,
                        conf->b64_depth, conf->qp_depth,
                        conf->uu_depth, conf->bitenc_depth,
                        conf->file_depth);
            }
            else
            {
                /*free mempool if calloc fails*/
                mempool_free(ssn->mime_mempool, bkt);
            }
        }
        else
        {
            if (ssn->mime_stats)
            {
                if(((MimeStats *)ssn->mime_stats)->memcap_exceeded % 10000 == 0 && preprocessor )
                    LogMessage("WARNING: %s max_mime_mem exceeded",preprocessor);
                ((MimeStats *)ssn->mime_stats)->memcap_exceeded++;
            }
            DEBUG_WRAP(DebugMessage(DEBUG_FILE, "No memory available for decoding. Memcap exceeded: %s preprocessor \n", preprocessor););
        }
    }
}

void* init_mime_mempool(int max_mime_mem, int max_depth,
        void *mempool, const char *preproc_name)
{
    int encode_depth;
    int max_sessions;
    MemPool *mime_mempool = (MemPool *)mempool;

    if (mime_mempool != NULL)
        return mime_mempool ;

    if (max_depth <= 0)
        return NULL;

    encode_depth = max_depth;

    if (encode_depth & 7)
        encode_depth += (8 - (encode_depth & 7));

    max_sessions = max_mime_mem / ( 2 * encode_depth);

    mime_mempool = (MemPool *)SnortPreprocAlloc(1, sizeof(MemPool), PP_FILE, PP_MEM_CATEGORY_MEMPOOL);

    if ((!mime_mempool)||(mempool_init(mime_mempool, max_sessions,
            (2 * encode_depth)) != 0))
    {
        FatalError( "%s:  Could not allocate %s mime mempool.\n",
                preproc_name, preproc_name);
    }

    return mime_mempool;
}

void* init_log_mempool(uint32_t email_hdrs_log_depth, uint32_t memcap,
        void *mempool, const char *preproc_name)
{
    uint32_t max_bkt_size;
    uint32_t max_sessions_logged;
    MemPool *log_mempool = (MemPool *) mempool;

    if (log_mempool != NULL)
        return log_mempool;

    max_bkt_size = ((2* MAX_EMAIL) + MAX_FILE +
            email_hdrs_log_depth);

    max_sessions_logged = memcap/max_bkt_size;

    log_mempool = (MemPool *)SnortPreprocAlloc(1, sizeof(*log_mempool), PP_FILE, PP_MEM_CATEGORY_MEMPOOL);

    if ((!log_mempool)||(mempool_init(log_mempool, max_sessions_logged,
            max_bkt_size) != 0))
    {
        if(!max_sessions_logged)
        {
            FatalError(
                    "%s:  Could not allocate %s mempool.\n", preproc_name, preproc_name);
        }
        else
        {
            FatalError(
                    "%s: Error setting the \"memcap\" \n", preproc_name);
        }
    }

    return log_mempool;
}

void get_mime_eol(const uint8_t *ptr, const uint8_t *end,
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


/*
 * Callback function for string search
 *
 * @param   id      id in array of search strings from mime_config.cmds
 * @param   index   index in array of search strings from mime_config.cmds
 * @param   data    buffer passed in to search function
 *
 * @return response
 * @retval 1        commands caller to stop searching
 */
static int search_str_found(void *id, void *unused, int index, void *data, void *unused2)
{
    int search_id = (int)(uintptr_t)id;

    mime_search_info.id = search_id;
    mime_search_info.index = index;
    mime_search_info.length = mime_current_search[search_id].name_len;

    /* Returning non-zero stops search, which is okay since we only look for one at a time */
    return 1;
}

static inline void process_decode_type(const char *start, int length, bool cnt_xf, MimeState *mime_ssn)
{
    const char *tmp = NULL;
    Email_DecodeState *decode_state = (Email_DecodeState *)(mime_ssn->decode_state);

    if(cnt_xf)
    {
        if(decode_state->b64_state.encode_depth > -1)
        {
            tmp = SnortStrcasestr(start, length, "base64");
            if( tmp != NULL )
            {
                decode_state->decode_type = DECODE_B64;
                if (mime_ssn->mime_stats)
                    ((MimeStats *)mime_ssn->mime_stats)->attachments[DECODE_B64]++;
                return;
            }
        }

        if(decode_state->qp_state.encode_depth > -1)
        {
            tmp = SnortStrcasestr(start, length, "quoted-printable");
            if( tmp != NULL )
            {
                decode_state->decode_type = DECODE_QP;
                if (mime_ssn->mime_stats)
                    ((MimeStats *)mime_ssn->mime_stats)->attachments[DECODE_QP]++;
                return;
            }
        }

        if(decode_state->uu_state.encode_depth > -1)
        {
            tmp = SnortStrcasestr(start, length, "uuencode");
            if( tmp != NULL )
            {
                decode_state->decode_type = DECODE_UU;
                if (mime_ssn->mime_stats)
                    ((MimeStats *)mime_ssn->mime_stats)->attachments[DECODE_UU]++;
                return;
            }
        }
    }

    if(decode_state->bitenc_state.depth > -1)
    {
        decode_state->decode_type = DECODE_BITENC;
        if (mime_ssn->mime_stats)
            ((MimeStats *)mime_ssn->mime_stats)->attachments[DECODE_BITENC]++;
        return;
    }

    return;
}

static inline void setup_decode(const char *data, int size, bool cnt_xf, MimeState *mime_ssn, void* scbPtr, uint32_t preproc_id)
{
    /* Check for Encoding Type */
    if( file_api->is_decoding_enabled(mime_ssn->decode_conf) && !mime_ssn->decode_conf->ignore_data)
    {
        set_mime_buffers(mime_ssn, scbPtr, preproc_id);
        if(mime_ssn->decode_state != NULL)
        {
            ResetBytesRead((Email_DecodeState *)(mime_ssn->decode_state));
            process_decode_type(data, size, cnt_xf, mime_ssn );
            mime_ssn->state_flags |= MIME_FLAG_EMAIL_ATTACH;
            /* check to see if there are other attachments in this packet */
            if( ((Email_DecodeState *)(mime_ssn->decode_state))->decoded_bytes )
                mime_ssn->state_flags |= MIME_FLAG_MULTIPLE_EMAIL_ATTACH;
        }
    }
}
/*
 * Handle Headers - Data or Mime
 *
 * @param   packet  standard Packet structure
 *
 * @param   i       index into p->payload buffer to start looking at data
 *
 * @return  i       index into p->payload where we stopped looking at data
 */
static const uint8_t * process_mime_header(Packet *p, const uint8_t *ptr,
        const uint8_t *data_end_marker, MimeState *mime_ssn, uint32_t preproc_id)
{
    const uint8_t *eol;
    const uint8_t *eolm;
    const uint8_t *colon;
    const uint8_t *content_type_ptr = NULL;
    const uint8_t *cont_trans_enc = NULL;
    const uint8_t *cont_disp = NULL;
    int header_found;
    const uint8_t *start_hdr;

    start_hdr = ptr;

    /* if we got a content-type in a previous packet and are
     * folding, the boundary still needs to be checked for */
    if (mime_ssn->state_flags & MIME_FLAG_IN_CONTENT_TYPE)
        content_type_ptr = ptr;

    if (mime_ssn->state_flags & MIME_FLAG_IN_CONT_TRANS_ENC)
        cont_trans_enc = ptr;

    if (mime_ssn->state_flags & MIME_FLAG_IN_CONT_DISP)
        cont_disp = ptr;

    while (ptr < data_end_marker)
    {
        int header_name_len;
        int max_header_name_len = 0;
        get_mime_eol(ptr, data_end_marker, &eol, &eolm);

        /* got a line with only end of line marker should signify end of header */
        if (eolm == ptr)
        {
            /* reset global header state values */
            mime_ssn->state_flags &=
                    ~(MIME_FLAG_FOLDING | MIME_FLAG_IN_CONTENT_TYPE | MIME_FLAG_DATA_HEADER_CONT
                            | MIME_FLAG_IN_CONT_TRANS_ENC | MIME_FLAG_IN_CONT_DISP );

            mime_ssn->data_state = STATE_DATA_BODY;

            /* no header seen */
            if (ptr == start_hdr)
            {
                setup_decode((const char *)ptr, eolm  - (const uint8_t *)NULL, false, mime_ssn, p->ssnptr, preproc_id);
            }

            return eol;
        }

        /* if we're not folding, see if we should interpret line as a data line
         * instead of a header line */
        if (!(mime_ssn->state_flags & (MIME_FLAG_FOLDING | MIME_FLAG_DATA_HEADER_CONT)))
        {
            char got_non_printable_in_header_name = 0;

            /* if we're not folding and the first char is a space or
             * colon, it's not a header */
            if (isspace((int)*ptr) || *ptr == ':')
            {
                mime_ssn->data_state = STATE_DATA_BODY;
                return ptr;
            }

            /* look for header field colon - if we're not folding then we need
             * to find a header which will be all printables (except colon)
             * followed by a colon */
            colon = ptr;
            while ((colon < eolm) && (*colon != ':'))
            {
                if (((int)*colon < 33) || ((int)*colon > 126))
                    got_non_printable_in_header_name = 1;

                colon++;
            }

            /* Check for Exim 4.32 exploit where number of chars before colon is greater than 64 */
            header_name_len = colon - ptr;
            if ((mime_ssn->data_state != STATE_DATA_UNKNOWN) &&
                (colon < eolm) && (header_name_len > MAX_HEADER_NAME_LEN))
            {
                max_header_name_len = header_name_len;
            }

            /* If the end on line marker and end of line are the same, assume
             * header was truncated, so stay in data header state */
            if ((eolm != eol) &&
                    ((colon == eolm) || got_non_printable_in_header_name))
            {
                /* no colon or got spaces in header name (won't be interpreted as a header)
                 * assume we're in the body */
                mime_ssn->state_flags &=
                        ~(MIME_FLAG_FOLDING | MIME_FLAG_IN_CONTENT_TYPE | MIME_FLAG_DATA_HEADER_CONT
                                | MIME_FLAG_IN_CONT_TRANS_ENC | MIME_FLAG_IN_CONT_DISP);

                mime_ssn->data_state = STATE_DATA_BODY;

                return ptr;
            }

            if(tolower((int)*ptr) == 'c')
            {
                mime_current_search = &mime_hdr_search[0];
                header_found =search_api->search_instance_find
                        (mime_hdr_search_mpse, (const char *)ptr,
                                eolm - ptr, 1, search_str_found);

                /* Headers must start at beginning of line */
                if ((header_found > 0) && (mime_search_info.index == 0))
                {
                    switch (mime_search_info.id)
                    {
                    case HDR_CONTENT_TYPE:
                        content_type_ptr = ptr + mime_search_info.length;
                        mime_ssn->state_flags |= MIME_FLAG_IN_CONTENT_TYPE;
                        break;
                    case HDR_CONT_TRANS_ENC:
                        cont_trans_enc = ptr + mime_search_info.length;
                        mime_ssn->state_flags |= MIME_FLAG_IN_CONT_TRANS_ENC;
                        break;
                    case HDR_CONT_DISP:
                        cont_disp = ptr + mime_search_info.length;
                        mime_ssn->state_flags |= MIME_FLAG_IN_CONT_DISP;
                        break;
                    default:
                        break;
                    }
                }
            }
            else if(tolower((int)*ptr) == 'e')
            {
                if((eolm - ptr) >= 9)
                {
                    if(strncasecmp((const char *)ptr, "Encoding:", 9) == 0)
                    {
                        cont_trans_enc = ptr + 9;
                        mime_ssn->state_flags |= MIME_FLAG_IN_CONT_TRANS_ENC;
                    }
                }
            }
        }
        else
        {
            mime_ssn->state_flags &= ~MIME_FLAG_DATA_HEADER_CONT;
        }


        if (mime_ssn->methods && mime_ssn->methods->handle_header_line)
        {
            int ret = mime_ssn->methods->handle_header_line(p, ptr, eol, max_header_name_len, mime_ssn);
            if (ret < 0)
                return NULL;
            else if (ret > 0)
            {
                /* assume we guessed wrong and are in the body */
                mime_ssn->data_state = STATE_DATA_BODY;
                mime_ssn->state_flags &=
                        ~(MIME_FLAG_FOLDING | MIME_FLAG_IN_CONTENT_TYPE | MIME_FLAG_DATA_HEADER_CONT
                                | MIME_FLAG_IN_CONT_TRANS_ENC | MIME_FLAG_IN_CONT_DISP);
                return ptr;
            }
        }
        /* check for folding
         * if char on next line is a space and not \n or \r\n, we are folding */
        if ((eol < data_end_marker) && isspace((int)eol[0]) && (eol[0] != '\n'))
        {
            if ((eol < (data_end_marker - 1)) && (eol[0] != '\r') && (eol[1] != '\n'))
            {
                mime_ssn->state_flags |= MIME_FLAG_FOLDING;
            }
            else
            {
                mime_ssn->state_flags &= ~MIME_FLAG_FOLDING;
            }
        }
        else if (eol != eolm)
        {
            mime_ssn->state_flags &= ~MIME_FLAG_FOLDING;
        }

        /* check if we're in a content-type header and not folding. if so we have the whole
         * header line/lines for content-type - see if we got a multipart with boundary
         * we don't check each folded line, but wait until we have the complete header
         * because boundary=BOUNDARY can be split across mulitple folded lines before
         * or after the '=' */
        if ((mime_ssn->state_flags &
                (MIME_FLAG_IN_CONTENT_TYPE | MIME_FLAG_FOLDING)) == MIME_FLAG_IN_CONTENT_TYPE)
        {
            if ((mime_ssn->data_state == STATE_MIME_HEADER) && !(mime_ssn->state_flags & MIME_FLAG_EMAIL_ATTACH))
            {
                setup_decode((const char *)content_type_ptr, (eolm - content_type_ptr), false, mime_ssn, p->ssnptr, preproc_id);
            }

            mime_ssn->state_flags &= ~MIME_FLAG_IN_CONTENT_TYPE;
            content_type_ptr = NULL;
        }
        else if ((mime_ssn->state_flags &
                (MIME_FLAG_IN_CONT_TRANS_ENC | MIME_FLAG_FOLDING)) == MIME_FLAG_IN_CONT_TRANS_ENC)
        {
            setup_decode((const char *)cont_trans_enc, (eolm - cont_trans_enc), true, mime_ssn, p->ssnptr, preproc_id);

            mime_ssn->state_flags &= ~MIME_FLAG_IN_CONT_TRANS_ENC;

            cont_trans_enc = NULL;
        }
        else if (((mime_ssn->state_flags &
                (MIME_FLAG_IN_CONT_DISP | MIME_FLAG_FOLDING)) == MIME_FLAG_IN_CONT_DISP) && cont_disp)
        {
            bool disp_cont = (mime_ssn->state_flags & MIME_FLAG_IN_CONT_DISP_CONT)? true: false;
            if(mime_ssn->log_config->log_filename && mime_ssn->log_state )
            {
                if(!log_file_name(cont_disp, eolm - cont_disp,
                        &(mime_ssn->log_state->file_log), &disp_cont, true) )
                {
                    mime_ssn->log_flags |= FLAG_FILENAME_PRESENT;
                }
                mime_ssn->log_flags |= FLAG_FILENAME_IN_HEADER;
            }
            if (disp_cont)
            {
                mime_ssn->state_flags |= MIME_FLAG_IN_CONT_DISP_CONT;
            }
            else
            {
                if ((mime_ssn->data_state == STATE_MIME_HEADER) && !(mime_ssn->state_flags & MIME_FLAG_EMAIL_ATTACH))
                {
                    // setting up decode assuming possible file data after content-disposition header
                    setup_decode(NULL, eolm - (const uint8_t *)NULL, false, mime_ssn, p->ssnptr, preproc_id);
                }
                mime_ssn->state_flags &= ~MIME_FLAG_IN_CONT_DISP;
                mime_ssn->state_flags &= ~MIME_FLAG_IN_CONT_DISP_CONT;
            }

            cont_disp = NULL;
        }
        else
        {
            // unknown header
            if ((mime_ssn->data_state == STATE_MIME_HEADER) && !(mime_ssn->state_flags & MIME_FLAG_EMAIL_ATTACH))
            {
                // setting up decode assuming possible file data after unknown header
                setup_decode(NULL, eolm - (const uint8_t *)NULL, false, mime_ssn, p->ssnptr, preproc_id);
            }
        }

        /* if state was unknown, at this point assume we know */
        if (mime_ssn->data_state == STATE_DATA_UNKNOWN)
            mime_ssn->data_state = STATE_DATA_HEADER;

        ptr = eol;

        if (ptr == data_end_marker)
            mime_ssn->state_flags |= MIME_FLAG_DATA_HEADER_CONT;
    }

    return ptr;
}

/* Get the end of data body (excluding boundary)*/
static const uint8_t * GetDataEnd(const uint8_t *data_start,
        const uint8_t *data_end_marker)
{
    /* '\r\n' + '--' + MIME boundary string */
    const int Max_Search = 4 + MAX_BOUNDARY_LEN;
    uint8_t *start;
    /*Exclude 2 bytes because either \r\n or '--'  at the end */
    uint8_t *end = (uint8_t *) data_end_marker - 2;

    /*Search for the start of boundary, should be less than boundary length*/
    if (end > data_start + Max_Search)
        start = end - Max_Search;
    else
        start = (uint8_t *)data_start;

    while (end > start)
    {
        if (*(--end) != '\n')
            continue;

        if ((*(end+1) == '-') && (*(end+2) == '-'))
        {
           if ((end > start) && (*(end-1) == '\r'))
               return (end - 1);
           else
               return end;
        }
        break;
    }
    return data_end_marker;
}

/*
 * Handle DATA_BODY state
 *
 * @param   packet  standard Packet structure
 *
 * @param   i       index into p->payload buffer to start looking at data
 *
 * @return  i       index into p->payload where we stopped looking at data
 */
static const uint8_t * process_mime_body(Packet *p, const uint8_t *ptr,
        const uint8_t *data_end, MimeState *mime_ssn, bool is_data_end)
{
    Email_DecodeState *decode_state = (Email_DecodeState *)(mime_ssn->decode_state);

    if ( mime_ssn->state_flags & MIME_FLAG_EMAIL_ATTACH )
    {
        const uint8_t *attach_start = ptr;
        const uint8_t *attach_end;
        uint8_t filename[MAX_UNICODE_FILE_NAME] ;
        uint32_t file_name_size = 0;

        if (is_data_end )
        {
            attach_end = GetDataEnd(ptr, data_end);
        }
        else
        {
            attach_end = data_end;
        }

        if( attach_start < attach_end )
        {
            bool filename_in_mime_header = (mime_ssn->log_flags & FLAG_FILENAME_IN_HEADER) ? true: false;
            if(EmailDecode( attach_start, attach_end, decode_state, filename, &file_name_size, filename_in_mime_header ) < DECODE_SUCCESS )
            {
                if (mime_ssn->methods && mime_ssn->methods->decode_alert)
                    mime_ssn->methods->decode_alert(mime_ssn->decode_state);
            }
            else
            {
               if ( !filename_in_mime_header && (decode_state->decode_type == DECODE_UU) && file_name_size && mime_ssn->log_state)
              {
                    bool disp_cont = (mime_ssn->state_flags & MIME_FLAG_IN_CONT_DISP_CONT)? true: false;
                    if ( !log_file_name((const uint8_t *) filename, \
                                                 file_name_size , \
                                                 &(mime_ssn->log_state->file_log), &disp_cont, false) )
                    {
                         mime_ssn->log_flags |= FLAG_FILENAME_PRESENT;
                    }
                }
            }
        }
    }

    mime_ssn->log_flags &= ~FLAG_FILENAME_IN_HEADER;
    if (is_data_end)
    {
        mime_ssn->data_state = STATE_MIME_HEADER;
        mime_ssn->state_flags &= ~MIME_FLAG_EMAIL_ATTACH;
    }

    return data_end;
}

/*
 * Reset MIME session state
 */
static void reset_mime_state(MimeState *mime_ssn)
{
    Email_DecodeState *decode_state = (Email_DecodeState *)(mime_ssn->decode_state);

    mime_ssn->data_state = STATE_DATA_INIT;
    mime_ssn->state_flags = 0;
    ClearEmailDecodeState(decode_state);
}

/*
 * Assume PAF is enabled
 */
const uint8_t * process_mime_data_paf(void *packet, const uint8_t *start, const uint8_t *end,
        MimeState *mime_ssn, bool upload, FilePosition position, uint32_t preproc_id)
{
    Packet *p = (Packet *)packet;
    bool done_data = false;

    if  (mime_ssn->methods && mime_ssn->methods->is_end_of_data)
    {
        done_data = mime_ssn->methods->is_end_of_data(p->ssnptr);
    }

    /* if we've just entered the data state, check for a dot + end of line
     * if found, no data */
    if ((mime_ssn->data_state == STATE_DATA_INIT) ||
            (mime_ssn->data_state == STATE_DATA_UNKNOWN))
    {
        if ((start < end) && (*start == '.'))
        {
            const uint8_t *eol = NULL;
            const uint8_t *eolm = NULL;

            get_mime_eol(start, end, &eol, &eolm);

            /* this means we got a real end of line and not just end of payload
             * and that the dot is only char on line */
            if ((eolm != end) && (eolm == (start + 1)))
            {
                /* if we're normalizing and not ignoring data copy data end marker
                 * and dot to alt buffer */
                if (mime_ssn->methods && mime_ssn->methods->normalize_data)
                {
                    if (mime_ssn->methods->normalize_data(p, start, end) < 0)
                        return NULL;
                }

                reset_mime_state(mime_ssn);

                return eol;
            }
        }

        if (mime_ssn->data_state == STATE_DATA_INIT)
            mime_ssn->data_state = STATE_DATA_HEADER;

        /* XXX A line starting with a '.' that isn't followed by a '.' is
         * deleted (RFC 821 - 4.5.2.  TRANSPARENCY).  If data starts with
         * '. text', i.e a dot followed by white space then text, some
         * servers consider it data header and some data body.
         * Postfix and Qmail will consider the start of data:
         * . text\r\n
         * .  text\r\n
         * to be part of the header and the effect will be that of a
         * folded line with the '.' deleted.  Exchange will put the same
         * in the body which seems more reasonable. */
    }

    if ( mime_ssn->decode_conf && !mime_ssn->decode_conf->ignore_data)
        setFileDataPtr(start, (uint16_t)(end - start));

    if ((mime_ssn->data_state == STATE_DATA_HEADER) ||
            (mime_ssn->data_state == STATE_DATA_UNKNOWN))
    {
#ifdef DEBUG_MSGS
        if (mime_ssn->data_state == STATE_DATA_HEADER)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_FILE, "DATA HEADER STATE ~~~~~~~~~~~~~~~~~~~~~~\n"););
        }
        else
        {
            DEBUG_WRAP(DebugMessage(DEBUG_FILE, "DATA UNKNOWN STATE ~~~~~~~~~~~~~~~~~~~~~\n"););
        }
#endif

        start = process_mime_header(p, start, end, mime_ssn, preproc_id);
        if (start == NULL)
            return NULL;

    }

    if (mime_ssn->methods && mime_ssn->methods->normalize_data)
    {
        if (mime_ssn->methods->normalize_data(p, start, end) < 0)
            return NULL;
    }

    /* now we shouldn't have to worry about copying any data to the alt buffer
     * only mime headers if we find them and only if we're ignoring data */

    while ((start != NULL) && (start < end))
    {
        switch (mime_ssn->data_state)
        {
        case STATE_MIME_HEADER:
            DEBUG_WRAP(DebugMessage(DEBUG_FILE, "MIME HEADER STATE ~~~~~~~~~~~~~~~~~~~~~~\n"););
            start = process_mime_header(p, start, end, mime_ssn, preproc_id);
            update_file_name(mime_ssn->log_state);
            break;
        case STATE_DATA_BODY:
            DEBUG_WRAP(DebugMessage(DEBUG_FILE, "DATA BODY STATE ~~~~~~~~~~~~~~~~~~~~~~~~\n"););
            start = process_mime_body(p, start, end, mime_ssn, isFileEnd(position) );
            update_file_name(mime_ssn->log_state);
            break;
        }
    }

    /* We have either reached the end of MIME header or end of MIME encoded data*/

    if((mime_ssn->decode_state) != NULL)
    {
        DecodeConfig *conf= mime_ssn->decode_conf;
        Email_DecodeState *ds = (Email_DecodeState *)(mime_ssn->decode_state);

        if (conf)
        {
            int detection_size = getDetectionSize(conf->b64_depth, conf->qp_depth,
                    conf->uu_depth, conf->bitenc_depth, ds );
            setFileDataPtr((const uint8_t*)ds->decodePtr, (uint16_t)detection_size);
        }

        if (file_api->file_process(p,(uint8_t *)ds->decodePtr,
                (uint16_t)ds->decoded_bytes, position, upload, false, false)
                && (isFileStart(position))&& mime_ssn->log_state)
        {
            set_file_name_from_log(&(mime_ssn->log_state->file_log), p->ssnptr);
        }
        if (mime_ssn->mime_stats)
            ((MimeStats *)mime_ssn->mime_stats)->decoded_bytes[ds->decode_type] += ds->decoded_bytes;
        ResetDecodedBytes((Email_DecodeState *)(mime_ssn->decode_state));
    }

    /* if we got the data end reset state, otherwise we're probably still in the data
     * to expect more data in next packet */
    if (done_data)
    {
        reset_mime_state(mime_ssn);
        if (mime_ssn->methods && mime_ssn->methods->reset_state)
            mime_ssn->methods->reset_state();
    }

    return end;
}

/*
 * Main function for mime processing
 *
 * This should be called when mime data is available
 */
const uint8_t * process_mime_data(void *packet, const uint8_t *start,
        const uint8_t *data_end_marker, MimeState *mime_ssn, bool upload, bool paf_enabled, char *preproc_name, uint32_t preproc_id)
{
    const uint8_t *attach_start = start;
    const uint8_t *attach_end;
    Packet *p = (Packet *)packet;
    FilePosition position = SNORT_FILE_START;
    preprocessor = preproc_name;
    SAVE_DAQ_PKT_HDR(p);

    if (paf_enabled)
    {
       position = file_api->get_file_position(p);
       process_mime_data_paf(packet, attach_start, data_end_marker,
                            mime_ssn, upload, position, preproc_id);
       return data_end_marker;
    }

    initFilePosition(&position, file_api->get_file_processed_size(p->ssnptr));
    /* look for boundary */
    while (start < data_end_marker)
    {
        /*Found the boundary, start processing data*/
        if (process_mime_paf_data(&(mime_ssn->mime_boundary),  *start))
        {
            attach_end = start;
            finalFilePosition(&position);
            process_mime_data_paf(packet, attach_start, attach_end,
                    mime_ssn, upload, position, preproc_id);
            position = SNORT_FILE_START;
            attach_start = start + 1;
        }

        start++;
    }

    if ((start == data_end_marker) && (attach_start < data_end_marker))
    {
        updateFilePosition(&position, file_api->get_file_processed_size(p->ssnptr));
        process_mime_data_paf(packet, attach_start, data_end_marker,
                mime_ssn, upload, position, preproc_id);
    }
    preprocessor = 0;

    return data_end_marker;
}

/*
 * This is the initialization funcation for mime processing.
 * This should be called when snort initializes
 */
void init_mime(void)
{
    const MimeToken *tmp;

    /* Header search */
    mime_hdr_search_mpse = search_api->search_instance_new();
    if (mime_hdr_search_mpse == NULL)
    {
        FatalError("Could not allocate MIME "
                "header search.\n");
    }

    for (tmp = &mime_hdrs[0]; tmp->name != NULL; tmp++)
    {
        mime_hdr_search[tmp->search_id].name = tmp->name;
        mime_hdr_search[tmp->search_id].name_len = tmp->name_len;

        search_api->search_instance_add(mime_hdr_search_mpse, tmp->name,
                tmp->name_len, tmp->search_id);
    }

    search_api->search_instance_prep(mime_hdr_search_mpse);
}

/*
 * Free anything that needs it before shutting down preprocessor
 *
 * @param   none
 *
 * @return  none
 */
void free_mime(void)
{
    if (mime_hdr_search_mpse != NULL)
        search_api->search_instance_free(mime_hdr_search_mpse);
}

void free_mime_session(MimeState *mime_ssn)
{
    if (!mime_ssn)
        return;

    if(mime_ssn->decode_state != NULL)
    {
        mempool_free(mime_ssn->mime_mempool, mime_ssn->decode_bkt);
        SnortPreprocFree(mime_ssn->decode_state, sizeof(Email_DecodeState), PP_FILE, 
                PP_MEM_CATEGORY_SESSION);
    }
    if(mime_ssn->log_state != NULL)
    {
        mempool_free(mime_ssn->log_mempool, mime_ssn->log_state->log_hdrs_bkt);
        SnortPreprocFree(mime_ssn->log_state, sizeof(FILE_LogState), PP_FILE, PP_MEM_CATEGORY_SESSION);
    }

    SnortPreprocFree(mime_ssn, sizeof(MimeState),  PP_FILE, PP_MEM_CATEGORY_SESSION);
}

/*
 * When the file ends (a MIME boundary detected), position are updated
 */
void finalize_mime_position(void *ssnptr, void *decode_state, FilePosition *position)
{
    /* check to see if there are file data in the session or
     * new decoding data waiting for processing */
    if( file_api->get_file_processed_size(ssnptr) ||
            (decode_state && ((Email_DecodeState *)decode_state)->decoded_bytes) )
        finalFilePosition(position);
}

/* Save the bounday string into paf state*/
static inline bool store_boundary(MimeDataPafInfo *data_info,  uint8_t val)
{
    if (!data_info->boundary_search)
    {
        if ((val == '.') || isspace (val))
            data_info->boundary_search = (char *)&boundary_str[0];
        return 0;
    }

    if (*(data_info->boundary_search) == '=')
    {
        /*Skip spaces for the end of boundary*/
        if (val == '=')
            data_info->boundary_search++;
        else if (!isspace(val))
            data_info->boundary_search = NULL;
    }
    else if (*(data_info->boundary_search) == '\0')
    {
        /*get boundary string*/
        if (isspace(val) || (val == '"'))
        {
            if (!data_info->boundary_len)
                return 0;
            else
            {
                /*Found boundary string*/
                data_info->boundary[data_info->boundary_len] = '\0';
                return 1;
            }
        }

        if (data_info->boundary_len < sizeof (data_info->boundary))
        {
            data_info->boundary[data_info->boundary_len++] = val;
        }
        else
        {
            /*Found boundary string*/
            data_info->boundary[data_info->boundary_len -1] = '\0';
            return 1;
        }
    }
    else if ((val == *(data_info->boundary_search))
            || (val == *(data_info->boundary_search) - 'a' + 'A'))
    {
        data_info->boundary_search++;
    }
    else
    {
        if ((val == '.') || isspace (val))
            data_info->boundary_search = (char *)&boundary_str[0];
        else
            data_info->boundary_search = NULL;
    }

    return 0;
}

/* check the bounday string in the mail body*/
static inline bool check_boundary(MimeDataPafInfo *data_info,  uint8_t data)
{
    /* Search for boundary signature "--"*/
    switch (data_info->boundary_state)
    {
    case MIME_PAF_BOUNDARY_UNKNOWN:
        if (data == '\n')
            data_info->boundary_state = MIME_PAF_BOUNDARY_LF;
        break;

    case MIME_PAF_BOUNDARY_LF:
        if (data == '-')
            data_info->boundary_state = MIME_PAF_BOUNDARY_HYPEN_FIRST;
        else if (data != '\n')
            data_info->boundary_state = MIME_PAF_BOUNDARY_UNKNOWN;
        break;

    case MIME_PAF_BOUNDARY_HYPEN_FIRST:
        if (data == '-')
        {
            data_info->boundary_state = MIME_PAF_BOUNDARY_HYPEN_SECOND;
            data_info->boundary_search = data_info->boundary;
        }
        else if (data == '\n')
            data_info->boundary_state = MIME_PAF_BOUNDARY_LF;
        else
            data_info->boundary_state = MIME_PAF_BOUNDARY_UNKNOWN;
        break;

    case MIME_PAF_BOUNDARY_HYPEN_SECOND:
        /* Compare with boundary string stored */
        if (*(data_info->boundary_search) == '\0')
        {
            if (data == '\n')
            {
                /*reset boundary search etc.*/
                data_info->boundary_state = MIME_PAF_BOUNDARY_UNKNOWN;
                return 1;
            }
            else if ((data != '\r') && ((data != '-')))
                data_info->boundary_state = MIME_PAF_BOUNDARY_UNKNOWN;
        }
        else if (*(data_info->boundary_search) == data)
            data_info->boundary_search++;
        else
            data_info->boundary_state = MIME_PAF_BOUNDARY_UNKNOWN;

        break;
    }

    return 0;
}

void reset_mime_paf_state(MimeDataPafInfo *data_info)
{
    data_info->boundary_search = NULL;
    data_info->boundary_len = 0;
    data_info->boundary[0] = '\0';
    data_info->boundary_state = MIME_PAF_BOUNDARY_UNKNOWN;
    data_info->data_state = MIME_PAF_FINDING_BOUNDARY_STATE;
}

/*  Process data boundary and flush each file based on boundary*/
bool process_mime_paf_data(MimeDataPafInfo *data_info,  uint8_t data)
{
    switch (data_info->data_state)
    {
    case MIME_PAF_FINDING_BOUNDARY_STATE:
        /* Search for boundary*/
        /* Store bounday string in PAF state*/
        if (store_boundary(data_info, data))
        {
            /* End of boundary, move to MIME_PAF_FOUND_BOUNDARY_STATE*/
            DEBUG_WRAP(DebugMessage(DEBUG_FILE, "Create boudary string: %s\n",
                    data_info->boundary););
            data_info->data_state = MIME_PAF_FOUND_BOUNDARY_STATE;
        }

        break;
    case MIME_PAF_FOUND_BOUNDARY_STATE:
        if (check_boundary(data_info,  data))
        {
            /* End of boundary, move to MIME_PAF_FOUND_BOUNDARY_STATE*/
            DEBUG_WRAP(DebugMessage(DEBUG_FILE, "Found Boudary string: %s\n",
                    data_info->boundary););
            return 1;
        }
        break;
    default:
        break;
    }

    return 0;
}

bool check_data_end(void *data_end_state,  uint8_t val)
{
    DataEndState state =  *((DataEndState *)data_end_state);

    switch (state)
    {
    case PAF_DATA_END_UNKNOWN:
        if (val == '\n')
        {
            state = PAF_DATA_END_FIRST_LF;
        }
        break;

    case PAF_DATA_END_FIRST_LF:
        if (val == '.')
        {
            state = PAF_DATA_END_DOT;
        }
        else if ((val != '\r') && (val != '\n'))
        {
            state = PAF_DATA_END_UNKNOWN;
        }
        break;
    case PAF_DATA_END_DOT:
        if (val == '\n')
        {
            *((DataEndState *)data_end_state) = PAF_DATA_END_UNKNOWN;
            return 1;
        }
        else if (val != '\r')
        {
            state = PAF_DATA_END_UNKNOWN;
        }
        break;

    default:
        state = PAF_DATA_END_UNKNOWN;
        break;
    }

    *((DataEndState *)data_end_state) = state;
    return 0;
}

#ifdef SNORT_RELOAD
void update_mime_mempool(void *mempool, int new_max_memory, int encode_depth)
{
     size_t obj_size = 0;
     unsigned num_objects = 0;
     MemPool *memory_pool = (MemPool*)mempool;

     if (encode_depth & 7)
          encode_depth += (8 - (encode_depth & 7));

     if(encode_depth)
     {
         obj_size = (2*encode_depth);
         num_objects = new_max_memory / obj_size;
     }

#ifdef REG_TEST
     if (REG_TEST_EMAIL_FLAG_MIME_MEMPOOL_ADJUST & getRegTestFlagsForEmail())
     {
         printf("\n========== START# NEW MIME MEMPOOL VALUES ==============================\n");
         printf("Mime mempool object size: NEW VALUE # %zu \n", obj_size);
         printf("Mime mempool max memory : NEW VALUE # (%u * %zu = %zu) \n", num_objects,obj_size,(num_objects * obj_size));
         printf("Mime mempool total number of buckets: NEW VALUE # %u \n", num_objects);
         printf("========== END# NEW MIME MEMPOOL VALUES ==============================\n");
         fflush(stdout);
     }
#endif

     mempool_setObjectSize(memory_pool, num_objects, obj_size );
}

void update_log_mempool(void *mempool, int new_max_memory , int email_hdrs_log_depth)
{
    size_t obj_size = 0;
    unsigned num_objects = 0;
    MemPool *memory_pool = (MemPool*)mempool;

    if(email_hdrs_log_depth)
    {
         if (email_hdrs_log_depth & 7)
               email_hdrs_log_depth += (8 - (email_hdrs_log_depth & 7));
    }

    obj_size = ((2* MAX_EMAIL) + MAX_FILE + email_hdrs_log_depth);
    num_objects = new_max_memory / obj_size;

#ifdef REG_TEST
    if (REG_TEST_EMAIL_FLAG_LOG_MEMPOOL_ADJUST & getRegTestFlagsForEmail())
    {
         printf("\n========== START# NEW LOG MEMPOOL VALUES ==============================\n");
         printf("Log mempool object size: NEW VALUE # %zu \n", obj_size);
         printf("Log mempool max memory : NEW VALUE # (%u * %zu = %zu) \n",num_objects, obj_size, (num_objects * obj_size));
         printf("Log mempool total number of buckets: NEW VALUE # %u \n",num_objects);
         printf("========== END# NEW LOG MEMPOOL VALUES ==============================\n");
        fflush(stdout);
    }
#endif

    mempool_setObjectSize(memory_pool, num_objects, obj_size );
}

#ifdef REG_TEST
void displayMimeMempool(void *mempool, DecodeConfig *decode_conf_old, DecodeConfig *decode_conf_new)
{
    MemPool *memory_pool = (MemPool*)mempool;

    if (REG_TEST_EMAIL_FLAG_MIME_MEMPOOL_ADJUST & getRegTestFlagsForEmail())
    {
          printf("\nmax_mime_mem is : OLD VALUE # %u \n",decode_conf_old->max_mime_mem);
          printf("max_depth is# OLD VALUE %u\n",decode_conf_old->max_depth);
          printf("\n=========START# OLD MIME MEMPOOL VALUES ===============================\n");
          printf("Mime mempool object size: OLD VALUE # %zu \n",memory_pool->obj_size);
          printf("Mime mempool max memory : OLD VALUE # %zu \n",memory_pool->max_memory);
          printf("Mime mempool total number of buckets: OLD VALUE # %u \n",mempool_numTotalBuckets(memory_pool));
          printf("=========END# OLD MIME MEMPOOL VALUES ===================================== \n");
          printf("\nSetting max_mime_mem to # ( NEW VALUE ) %u \n",decode_conf_new->max_mime_mem);
          printf("Setting max_depth to # ( NEW VALUE )%u\n",decode_conf_new->max_depth);
          fflush(stdout);
    }
}

void displayLogMempool(void *mempool, unsigned memcap_old, unsigned memcap_new)
{
    MemPool *memory_pool = (MemPool*)mempool;

    if (REG_TEST_EMAIL_FLAG_LOG_MEMPOOL_ADJUST & getRegTestFlagsForEmail())
    {
          printf("\nmemcap is : OLD VALUE # %u \n", memcap_old);
          printf("\n=========START# OLD LOG MEMPOOL VALUES ==================================\n ");
          printf("Log mempool object size: OLD VALUE # %zu \n",memory_pool->obj_size);
          printf("Log mempool max memory : OLD VALUE # %zu \n",memory_pool->max_memory);
          printf("Log mempool total number of buckets: OLD VALUE # %u \n",mempool_numTotalBuckets(memory_pool));
          printf("=========END# OLD LOG MEMPOOL VALUES ================================== \n");
          printf("\nSetting memcap to# (NEW VALUE ) %u \n", memcap_new);
          fflush(stdout);
    }
}

void displayDecodeDepth(DecodeConfig *decode_conf_old, DecodeConfig *decode_conf_new)
{
     if(REG_TEST_EMAIL_FLAG_DECODE_DEPTH_ADJUST & getRegTestFlagsForEmail())
     {
          if(decode_conf_old->b64_depth != decode_conf_new->b64_depth )
          {
               printf("\nBase64 decode depth: OLD VALUE # %d",decode_conf_old->b64_depth);
               printf("\nSetting Base64 decoding depth to # (new value)%d \n\n", decode_conf_new->b64_depth);
          }
          if(decode_conf_old->qp_depth != decode_conf_new->qp_depth )
          {
               printf("\nQuoted-Printable decoding depth: OLD VALUE # %d",decode_conf_old->qp_depth);
               printf("\nSetting Quoted-Printable decoding depth to # (new value)%d \n\n",decode_conf_new->qp_depth);
          }
          if(decode_conf_old->bitenc_depth != decode_conf_new->bitenc_depth )
          {
               printf("\nNon-encoded MIME extraction depth (bitec_depth): OLD VALUE # %d",decode_conf_old->bitenc_depth);
               printf("\nSetting bitenc decoding depth to # (new value)%d \n\n", decode_conf_new->bitenc_depth);
          }
          if(decode_conf_old->uu_depth != decode_conf_new->uu_depth )
          {
               printf("\nUnix-to-Unix decoding depth: OLD VALUE # %d",decode_conf_old->uu_depth);
               printf("\nSetting Unix-to-Unix decoding depth depth to # (new value)%d \n\n", decode_conf_new->uu_depth);
          }
     }
}
#endif

#endif
