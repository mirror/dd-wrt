/* $Id$ */
/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
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

// @file    sp_react.c
// @author  Russ Combs <rcombs@sourcefire.com>

/* The original Snort React Plugin was contributed by Maciej Szarpak, Warsaw
 * University of Technology.  The module has been entirely rewritten by
 * Sourcefire as part of the effort to overhaul active response.  Some of the
 * changes include:
 *
 * - elimination of unworkable warn mode
 * - elimination of proxy port (rule header has ports)
 * - integration with unified active response mechanism
 * - queuing of rule option responses so at most one is issued
 * - allow override by rule action when action is drop
 * - addition of http headers to default response
 * - added custom page option
 * - and other stuff
 *
 * This version will send a web page to the client and then reset both
 * ends of the session.  The web page may be configured or the default
 * may be used.  The web page can have the default warning message
 * inserted or the message from the rule.
 *
 * If you wish to just reset the session, use the resp keyword instead.
 */

#ifdef ENABLE_REACT

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "sf_types.h"
#include "snort_debug.h"
#include "decode.h"
#include "encode.h"
#include "detection_options.h"
#include "parser.h"
#include "plugbase.h"
#include "plugin_enum.h"
#include "profiler.h"
#include "active.h"
#include "rules.h"
#include "sfhashfcn.h"
#include "sp_react.h"
#include "snort.h"

#ifdef PERF_PROFILING
static PreprocStats reactPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

static const char* MSG_KEY = "<>";
static const char* MSG_PERCENT = "%";

static const char* DEFAULT_HTTP =
    "HTTP/1.1 403 Forbidden\r\n"
    "Connection: close\r\n"
    "Content-Type: text/html; charset=utf-8\r\n"
    "Content-Length: %d\r\n"
    "\r\n";

static const char* DEFAULT_HTML =
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.1//EN\"\r\n"
    "    \"http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd\">\r\n"
    "<html xmlns=\"http://www.w3.org/1999/xhtml\" xml:lang=\"en\">\r\n"
    "<head>\r\n"
    "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\" />\r\n"
    "<title>Access Denied</title>\r\n"
    "</head>\r\n"
    "<body>\r\n"
    "<h1>Access Denied</h1>\r\n"
    "<p>%s</p>\r\n"
    "</body>\r\n"
    "</html>\r\n";

static const char* DEFAULT_MSG =
    "You are attempting to access a forbidden site.<br />"
    "Consult your system administrator for details.";

typedef struct _ReactData
{
    uint32_t id;
    int rule_msg;        // 1=>use rule msg; 0=>use DEFAULT_MSG
    ssize_t buf_len;     // length of response
    char* resp_buf;      // response to send
    const OptTreeNode* otn;

} ReactData;

static int s_init = 1;
static int s_deprecated = 0;
static char* s_page = NULL;

// When React_Init() is called the rule msg keyword may not have
// been processed.  This necessitates two things:
//
// * A unique instance id is used in the hash in lieu of the
//   message text.  The id starts at 1 since 0 is reserved for
//   the default msg.  Assuming all rules have different msg
//   strings, the id is a valid proxy.
//
// * React_Config() is installed to instantiate the page after
//   rule parsing is complete (when for sure the msg is
//   available).
//
// Ideally a separate rule configuration callback could be installed
// that would be called after all options are parsed and before the
// options are finalized.
static uint32_t s_id = 1;

// callback functions
static void React_Init(struct _SnortConfig *, char *, OptTreeNode *, int);
static void React_Cleanup(int signal, void *data);
static void React_Config (struct _SnortConfig *, void *data);

// core functions
static void React_GetPage(struct _SnortConfig *);
static void React_Parse(char *, OptTreeNode *, ReactData *);
static int React_Queue(Packet*, void*);
static void React_Send(Packet*,  void*);

//--------------------------------------------------------------------
// public functions

void ReactFree(void *d)
{
    ReactData *data = (ReactData *)d;
    if (data->resp_buf)
        free(data->resp_buf);
    free(data);
}

uint32_t ReactHash(void *d)
{
    uint32_t a,b,c,tmp;
    unsigned int i,j,k,l;
    ReactData *data = (ReactData *)d;

    const char* s = s_page ? s_page : DEFAULT_HTML;
    unsigned n = strlen(s);

    a = data->rule_msg;
    b = n;
    c = (data->rule_msg ? data->id : 0);

    mix(a,b,c);

    for ( i=0,j=0; i<n; i+=4 )
    {
        tmp = 0;
        k = n - i;
        if (k > 4)
            k=4;

        for (l=0;l<k;l++)
        {
            tmp |= s[i + l] << l*8;
        }

        switch (j)
        {
            case 0:
                a += tmp;
                break;
            case 1:
                b += tmp;
                break;
            case 2:
                c += tmp;
                break;
        }
        j++;

        if (j == 3)
        {
            mix(a,b,c);
            j = 0;
        }
    }

    if (j != 0)
    {
        mix(a,b,c);
    }

    a += RULE_OPTION_TYPE_REACT;

    final(a,b,c);

    return c;
}

int ReactCompare(void *l, void *r)
{
    ReactData *left = (ReactData *)l;
    ReactData *right = (ReactData *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if (left->buf_len != right->buf_len)
        return DETECTION_OPTION_NOT_EQUAL;

    if (memcmp(left->resp_buf, right->resp_buf, left->buf_len) != 0)
        return DETECTION_OPTION_NOT_EQUAL;

    if (left->rule_msg != right->rule_msg)
        return DETECTION_OPTION_NOT_EQUAL;

    return DETECTION_OPTION_EQUAL;
}

void SetupReact(void)
{
    RegisterRuleOption("react", React_Init, NULL, OPT_TYPE_ACTION, NULL);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("react", &reactPerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif
}

//--------------------------------------------------------------------
// callback functions

static void React_Init(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    ReactData* rd;
    void *idx_dup;

    if ( otn->ds_list[PLUGIN_RESPONSE] )
        FatalError("%s(%d): Multiple response options in rule\n",
            file_name, file_line);

    if ( protocol != IPPROTO_TCP )
        FatalError("%s(%d): React options on non-TCP rule\n",
            file_name, file_line);

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"In React_Init()\n"););

    if ( s_init )
    {
        AddFuncToCleanExitList(React_Cleanup, NULL);

        React_GetPage(sc);

        Active_SetEnabled(1);
        s_init = 0;
    }

    /* parse the react keywords */
    rd = SnortAlloc(sizeof(*rd));
    React_Parse(data, otn, rd);
    rd->otn = otn;

    if (add_detection_option(sc, RULE_OPTION_TYPE_REACT, (void*)rd, &idx_dup)
        == DETECTION_OPTION_EQUAL)
    {
        free(rd);
        rd = idx_dup;
    }
    /* finally, attach the option's detection function to the rule's
       detect function pointer list */
    AddRspFuncToList(React_Queue, otn, (void*)rd);
    AddFuncToPreprocPostConfigList(sc, React_Config, rd);

    // this prevents multiple response options in rule
    otn->ds_list[PLUGIN_RESPONSE] = rd;
}

static void React_Cleanup(int signal, void* data)
{
    if ( s_page )
    {
        free(s_page);
        s_page = NULL;
    }
    s_init = 1;
}

//--------------------------------------------------------------------
// core functions

static void React_GetPage (struct _SnortConfig *sc)
{
    char* msg;
    char* percent_s;
    struct stat fs;
    FILE* fd;
    size_t n;

    if ( !sc )
        FatalError("react: %s(%d) Snort config for parsing is NULL.\n",
            file_name, file_line);

    if ( s_page || !sc->react_page ) return;

    if ( stat(sc->react_page, &fs) )
        FatalError("react: %s(%d) can't stat react page file '%s'.\n",
            file_name, file_line, sc->react_page);

    if ( fs.st_size < 2 )
        FatalError("react: react page %s size is not adequate.\n",
            sc->react_page);

    s_page = SnortAlloc(fs.st_size+1);
    fd = fopen(sc->react_page, "r");

    if ( !fd )
        FatalError("react: %s(%d) can't open react page file '%s'.\n",
            file_name, file_line, sc->react_page);

    n = fread(s_page, 1, fs.st_size, fd);
    fclose(fd);

    if ( n != (size_t)fs.st_size )
        FatalError("react: %s(%d) can't load react page file '%s'.\n",
            file_name, file_line, sc->react_page);

    s_page[n] = '\0';

    msg = strstr(s_page, MSG_KEY);
    if ( msg ) strncpy(msg, "%s", 2);

    // search for %
    percent_s = strstr(s_page, MSG_PERCENT);
    if (percent_s)
    {
        percent_s += strlen(MSG_PERCENT); // move past current
        // search for % again
        percent_s = strstr(percent_s, MSG_PERCENT);
        if (percent_s)
        {
            FatalError("react: %s(%d) can't specify more than one %%s or other "
                "printf style formatting characters in react page '%s'.\n",
                file_name, file_line, sc->react_page);
        }
    }
}

//--------------------------------------------------------------------

static void React_Parse(char* data, OptTreeNode* otn, ReactData* rd)
{
    char* tok = NULL;

    if ( data )
    {
        while(isspace((int)*data)) data++;

        tok = strtok(data, ",");
    }
    while(tok)
    {
        /* parse the react option keywords */
        if (
            !strncasecmp(tok, "proxy", 5) ||
            !strcasecmp(tok, "block") ||
            !strcasecmp(tok, "warn") )
        {
            if ( !s_deprecated )
            {
                ParseWarning("proxy, block, and warn options are deprecated.\n");
                s_deprecated = 1;
            }
        }
        else if ( !strcasecmp(tok, "msg") )
        {
            rd->rule_msg = 1;
        }
        else
            FatalError("%s(%d): invalid react option: %s\n",
                file_name, file_line, tok);

        tok = strtok(NULL, ",");

        /* get rid of spaces */
        while ( tok && isspace((int)*tok) ) tok++;
    }
    rd->resp_buf = NULL;
    rd->buf_len = 0;
    rd->id = s_id++;
}

//--------------------------------------------------------------------
// format response buffer

static void React_Config (struct _SnortConfig *sc, void *data)
{
    ReactData *rd = (ReactData *)data;
    size_t body_len, head_len, total_len;
    char dummy;

    const char* head = DEFAULT_HTTP;
    const char* body = s_page ? s_page : DEFAULT_HTML;

    const char* msg = rd->otn->sigInfo.message;
    if ( !msg || !rd->rule_msg ) msg = DEFAULT_MSG;

    body_len = snprintf(&dummy, 1, body, msg);
    head_len = snprintf(&dummy, 1, head, body_len);
    total_len = head_len + body_len + 1;

    rd->resp_buf = (char*)SnortAlloc(total_len);

    SnortSnprintf((char*)rd->resp_buf, head_len+1, head, body_len);
    SnortSnprintf((char*)rd->resp_buf+head_len, body_len+1, body, msg);

    // set actual length
    rd->resp_buf[total_len-1] = '\0';
    rd->buf_len = strlen(rd->resp_buf);
}

//--------------------------------------------------------------------

static int React_Queue (Packet* p, void* pv)
{
    ReactData* rd = (ReactData*)pv;
    PROFILE_VARS;

    PREPROC_PROFILE_START(reactPerfStats);

    if ( Active_IsRSTCandidate(p) )
        Active_QueueResponse(React_Send, rd);

    Active_DropSession(p);
    if (pkt_trace_enabled)
    {
        if (rd && rd->otn)
            addPktTraceData(VERDICT_REASON_REACT, snprintf(trace_line, MAX_TRACE_LINE,
                "Snort React: web page %s, gid %u, sid %u, %s\n", Active_IsRSTCandidate(p)? "is sent" : "isn't sent",
                rd->otn->sigInfo.generator, rd->otn->sigInfo.id, getPktTraceActMsg()));
        else addPktTraceData(VERDICT_REASON_REACT, snprintf(trace_line, MAX_TRACE_LINE,
                "Snort React: web page %s, %s\n", Active_IsRSTCandidate(p)? "is sent" : "isn't sent", getPktTraceActMsg()));
    }
    else addPktTraceData(VERDICT_REASON_REACT, 0);

    PREPROC_PROFILE_END(reactPerfStats);
    return 0;
}

//--------------------------------------------------------------------

static void React_Send (Packet* p,  void* pv)
{
    ReactData* rd = (ReactData*)pv;
    EncodeFlags df = (p->packet_flags & PKT_FROM_SERVER) ? ENC_FLAG_FWD : 0;
    EncodeFlags sent = rd->buf_len;
    EncodeFlags rf;
    PROFILE_VARS;

    PREPROC_PROFILE_START(reactPerfStats);
    Active_IgnoreSession(p);

    if (p->packet_flags & PKT_STREAM_EST)
    {
        Active_SendData(p, df, (uint8_t*)rd->resp_buf, rd->buf_len);
        // Active_SendData sends a FIN, so need to bump seq by 1.
        sent++;
    }
    rf = df ^ ENC_FLAG_FWD;
    df |= ENC_FLAG_SEQ | (ENC_FLAG_VAL & sent);
    Active_SendReset(p, df);
    Active_SendReset(p, rf);

    PREPROC_PROFILE_END(reactPerfStats);
}

#endif /* ENABLE_REACT */

