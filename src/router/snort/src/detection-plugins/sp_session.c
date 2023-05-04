/* $Id$ */

/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
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
*/

/* Snort Session Logging Plugin */

/* sp_session
 *
 * Purpose:
 *
 * Drops data (printable or otherwise) into a SESSION file.  Useful for
 * logging user sessions (telnet, http, ftp, etc).
 *
 * Arguments:
 *
 * This plugin can take two arguments:
 *    printable => only log the "printable" ASCII characters.
 *    all       => log all traffic in the session, logging non-printable
 *                 chars in "\xNN" hexidecimal format
 *
 * Effect:
 *
 * Warning, this plugin may slow Snort *way* down!
 *
 */

/* put the name of your pluging header file here */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif


#include <sys/types.h>
#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif /* !WIN32 */
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include <errno.h>
#include <sys/stat.h>

#include "rules.h"
#include "treenodes.h"
#include "decode.h"
#include "plugbase.h"
#include "parser.h"
#include "snort_debug.h"
#include "util.h"
#include "plugin_enum.h"
#include "snort.h"

#include "snort.h"
#include "profiler.h"
#ifdef PERF_PROFILING
static PreprocStats sessionPerfStats;
extern PreprocStats ruleOTNEvalPerfStats;
#endif

#include "sfhashfcn.h"
#include "detection_options.h"

#define SESSION_PRINTABLE  1
#define SESSION_ALL        2
#define SESSION_BINARY     3

typedef struct _SessionData
{
    int session_flag;
} SessionData;

void SessionInit(struct _SnortConfig *, char *, OptTreeNode *, int);
void ParseSession(char *, OptTreeNode *);
int LogSessionData(void *option_data, Packet *p);
void DumpSessionData(FILE *, Packet *, SessionData *);
FILE *OpenSessionFile(Packet *);

uint32_t SessionHash(void *d)
{
    uint32_t a,b,c;
    SessionData *data = (SessionData *)d;

    a = data->session_flag;
    b = RULE_OPTION_TYPE_SESSION;
    c = 0;

    final(a,b,c);

    return c;
}

int SessionCompare(void *l, void *r)
{
    SessionData *left = (SessionData *)l;
    SessionData *right = (SessionData *)r;

    if (!left || !right)
        return DETECTION_OPTION_NOT_EQUAL;

    if (left->session_flag == right->session_flag)
    {
        return DETECTION_OPTION_EQUAL;
    }

    return DETECTION_OPTION_NOT_EQUAL;
}


/****************************************************************************
 *
 * Function: SetupSession()
 *
 * Purpose: Init the session plugin module.
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 ****************************************************************************/
void SetupSession(void)
{
    /* map the keyword to an initialization/processing function */
    RegisterRuleOption("session", SessionInit, NULL, OPT_TYPE_LOGGING, NULL);
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile("session", &sessionPerfStats, 3, &ruleOTNEvalPerfStats, NULL);
#endif
    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Plugin: Session Setup\n"););
}


/**************************************************************************
 *
 * Function: SessionInit(struct _SnortConfig *, char *, OptTreeNode *)
 *
 * Purpose: Initialize the sesion plugin, parsing the rule parameters and
 *          setting up any necessary data structures.
 *
 * Arguments: data => rule arguments/data
 *            otn => pointer to the current rule option list node
 *
 * Returns: void function
 *
 *************************************************************************/
void SessionInit(struct _SnortConfig *sc, char *data, OptTreeNode *otn, int protocol)
{
    OptFpList *fpl;

    /*
     * Theoretically we should only all this plugin to be used when there's a
     * possibility of a session happening (i.e. TCP), but I get enough
     * requests that I'm going to pull the verifier so that things should work
     * for everyone
     */
/*    if(protocol != IPPROTO_TCP)
    {
        FatalError("%(%d): Session keyword can not be used in non-TCP rule\n",
                file_name, file_line);
    }*/

    /* multiple declaration check */
    if(otn->ds_list[PLUGIN_SESSION])
    {
        FatalError("%s(%d): Multiple session options in rule\n", file_name,
                file_line);
    }

    /* allocate the data structure and attach it to the
       rule's data struct list */
    otn->ds_list[PLUGIN_SESSION] = (SessionData *)
        SnortAlloc(sizeof(SessionData));

    /* be sure to check that the protocol that is passed in matches the
       transport layer protocol that you're using for this rule! */

    /* this is where the keyword arguments are processed and placed into
       the rule option's data structure */
    ParseSession(data, otn);

    /* finally, attach the option's detection function to the rule's
       detect function pointer list */
    fpl = AddOptFuncToList(LogSessionData, otn);
    fpl->context = otn->ds_list[PLUGIN_SESSION];
    fpl->type = RULE_OPTION_TYPE_SESSION;
}



/****************************************************************************
 *
 * Function: ParseSession(char *, OptTreeNode *)
 *
 * Purpose: Figure out how much of the session data we're collecting
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: void function
 *
 ****************************************************************************/
void ParseSession(char *data, OptTreeNode *otn)
{
    SessionData *ds_ptr;  /* data struct pointer */
    //void *ds_ptr_dup;

    /* set the ds pointer to make it easier to reference the option's
       particular data struct */
    ds_ptr = otn->ds_list[PLUGIN_SESSION];

    /* manipulate the option arguments here */
    while(isspace((int)*data))
        data++;

    if(!strncasecmp(data, "printable", 9))
    {
        ds_ptr->session_flag = SESSION_PRINTABLE;
        return;
    }

    if(!strncasecmp(data, "binary", 6))
    {
        ds_ptr->session_flag = SESSION_BINARY;
        return;
    }

    if(!strncasecmp(data, "all", 3))
    {
        ds_ptr->session_flag = SESSION_ALL;
        return;
    }

    FatalError("%s(%d): invalid session modifier: %s\n", file_name, file_line, data);

#if 0
    if (add_detection_option(sc, RULE_OPTION_TYPE_SESSION, (void *)ds_ptr, &ds_ptr_dup) == DETECTION_OPTION_EQUAL)
    {
        free(ds_ptr);
        ds_ptr = otn->ds_list[PLUGIN_SESSION] = ds_ptr_dup;
    }
#endif
}



/****************************************************************************
 *
 * Function: LogSessionData(char *, OptTreeNode *)
 *
 * Purpose: Dumps the session data to the log file.
 *
 * Arguments: data => argument data
 *            otn => pointer to the current rule's OTN
 *
 * Returns: Always calls the next function (this one doesn't test the data,
 *          it just logs it....)
 *
 ****************************************************************************/
int LogSessionData(void *option_data, Packet *p)
{
    SessionData *session_data = (SessionData *)option_data;
    FILE *session;         /* session file ptr */
    PROFILE_VARS;

    PREPROC_PROFILE_START(sessionPerfStats);

    /* if there's data in this packet */
    if(p != NULL)
    {
        if((p->dsize != 0 && p->data != NULL) || p->frag_flag != 1)
        {
             session = OpenSessionFile(p);

             if(session == NULL)
             {
                 PREPROC_PROFILE_END(sessionPerfStats);
                 return DETECTION_OPTION_MATCH;
             }

             DumpSessionData(session, p, session_data);

             fclose(session);
        }
    }

    PREPROC_PROFILE_END(sessionPerfStats);
    return DETECTION_OPTION_MATCH;
}

void DumpSessionData(FILE *fp, Packet *p, SessionData *sessionData)
{
    const u_char *idx;
    const u_char *end;
    char conv[] = "0123456789ABCDEF"; /* xlation lookup table */

    if(p->dsize == 0 || p->data == NULL || p->frag_flag)
        return;

    idx = p->data;
    end = idx + p->dsize;

    if(sessionData->session_flag == SESSION_PRINTABLE)
    {
        while(idx != end)
        {
            if((*idx > 0x1f && *idx < 0x7f) || *idx == 0x0a || *idx == 0x0d)
            {
                fputc(*idx, fp);
            }
            idx++;
        }
    }
    else if(sessionData->session_flag == SESSION_BINARY)
    {
        fwrite(p->data, p->dsize, sizeof(char), fp);
    }
    else
    {
        while(idx != end)
        {
            if((*idx > 0x1f && *idx < 0x7f) || *idx == 0x0a || *idx == 0x0d)
            {
                /* Escape all occurences of '\' */
                if(*idx == '\\')
                    fputc('\\', fp);
                fputc(*idx, fp);
            }
            else
            {
                fputc('\\', fp);
                fputc(conv[((*idx&0xFF) >> 4)], fp);
                fputc(conv[((*idx&0xFF)&0x0F)], fp);
            }

            idx++;
        }
    }
}



FILE *OpenSessionFile(Packet *p)
{
    char filename[STD_BUF];
    char log_path[STD_BUF];
    char session_file[STD_BUF]; /* name of session file */
    sfaddr_t *dst, *src;

    FILE *ret;

    if(p->frag_flag)
    {
        return NULL;
    }

    memset((char *)session_file, 0, STD_BUF);
    memset((char *)log_path, 0, STD_BUF);

    /* figure out which way this packet is headed in relation to the homenet */
    dst = GET_DST_IP(p);
    src = GET_SRC_IP(p);
    if(sfip_contains(&snort_conf->homenet, dst) == SFIP_CONTAINS) {
        if(sfip_contains(&snort_conf->homenet, src) == SFIP_NOT_CONTAINS)
        {
            SnortSnprintf(log_path, STD_BUF, "%s/%s", snort_conf->log_dir, inet_ntoa(GET_SRC_ADDR(p)));
        }
        else
        {
            if(p->sp >= p->dp)
            {
                SnortSnprintf(log_path, STD_BUF, "%s/%s", snort_conf->log_dir, inet_ntoa(GET_SRC_ADDR(p)));
            }
            else
            {
                SnortSnprintf(log_path, STD_BUF, "%s/%s", snort_conf->log_dir, inet_ntoa(GET_DST_ADDR(p)));
            }
        }
    }
    else
    {
        if(sfip_contains(&snort_conf->homenet, src) == SFIP_CONTAINS)
        {
            SnortSnprintf(log_path, STD_BUF, "%s/%s", snort_conf->log_dir, inet_ntoa(GET_DST_ADDR(p)));
        }
        else
        {
            if(p->sp >= p->dp)
            {
                SnortSnprintf(log_path, STD_BUF, "%s/%s", snort_conf->log_dir, inet_ntoa(GET_SRC_ADDR(p)));
            }
            else
            {
                SnortSnprintf(log_path, STD_BUF, "%s/%s", snort_conf->log_dir, inet_ntoa(GET_DST_ADDR(p)));
            }
        }
    }

    /* build the log directory */
    if(mkdir(log_path,S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
    {
        if(errno != EEXIST)
        {
            FatalError("Problem creating directory %s: %s\n",
                       log_path,strerror(errno));
        }
    }

    if(p->sp >= p->dp)
    {
#ifdef WIN32
        SnortSnprintf(session_file, STD_BUF, "%s/SESSION_%d-%d", log_path, p->sp, p->dp);
#else
        SnortSnprintf(session_file, STD_BUF, "%s/SESSION:%d-%d", log_path, p->sp, p->dp);
#endif
    }
    else
    {
#ifdef WIN32
        SnortSnprintf(session_file, STD_BUF, "%s/SESSION_%d-%d", log_path, p->dp, p->sp);
#else
        SnortSnprintf(session_file, STD_BUF, "%s/SESSION:%d-%d", log_path, p->dp, p->sp);
#endif
    }


    strncpy(filename, session_file, STD_BUF - 1);
    filename[STD_BUF - 1] = '\0';

    ret = fopen(session_file, "a");

    if(ret == NULL)
    {
        FatalError("OpenSessionFile() => fopen(%s) session file: %s\n",
                   session_file, strerror(errno));
    }

    return ret;

}




