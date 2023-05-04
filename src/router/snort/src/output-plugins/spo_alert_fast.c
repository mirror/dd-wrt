/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
** Copyright (C) 2000,2001 Andrew R. Baker <andrewb@uab.edu>
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

/* $Id$ */

/* spo_alert_fast
 *
 * Purpose:  output plugin for fast alerting
 *
 * Arguments:  alert file
 *
 * Effect:
 *
 * Alerts are written to a file in the snort fast alert format
 *
 * Comments:   Allows use of fast alerts with other output plugin types
 *
 */

/* output plugin header file */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif /* !WIN32 */

#include <sys/types.h>

#include "spo_alert_fast.h"
#include "event.h"
#include "decode.h"
#include "snort_debug.h"
#include "plugbase.h"
#include "spo_plugbase.h"
#include "parser.h"
#include "util.h"
#include "log.h"
#include "mstring.h"
#include "active.h"
#include "sfutil/sf_textlog.h"
#include "log_text.h"
#include "sf_textlog.h"
#include "snort.h"
#include "sfdaq.h"

/* full buf was chosen to allow printing max size packets
 * in hex/ascii mode:
 * each byte => 2 nibbles + space + ascii + overhead
 */
#define FULL_BUF  (4*IP_MAXPACKET)
#define FAST_BUF  (4*K_BYTES)

/*
 * not defined for backwards compatibility
 * (default is produced by OpenAlertFile()
#define DEFAULT_FILE  "alert.fast"
 */
#define DEFAULT_LIMIT (128*M_BYTES)

typedef struct _SpoAlertFastData
{
    TextLog* log;
    uint8_t packet_flag;
} SpoAlertFastData;

static void AlertFastInit(struct _SnortConfig *, char *);
static SpoAlertFastData *ParseAlertFastArgs(struct _SnortConfig *, char *);
static void AlertFastCleanExitFunc(int, void *);
static void AlertFast(Packet *, const char *, void *, Event *);

/*
 * Function: SetupAlertFast()
 *
 * Purpose: Registers the output plugin keyword and initialization
 *          function into the output plugin list.  This is the function that
 *          gets called from InitOutputPlugins() in plugbase.c.
 *
 * Arguments: None.
 *
 * Returns: void function
 *
 */
void AlertFastSetup(void)
{
    /* link the preprocessor keyword to the init function in
       the preproc list */
    RegisterOutputPlugin("alert_fast", OUTPUT_TYPE_FLAG__ALERT, AlertFastInit);
    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Output plugin: AlertFast is setup...\n"););
}


/*
 * Function: AlertFastInit(char *)
 *
 * Purpose: Calls the argument parsing function, performs final setup on data
 *          structs, links the preproc function into the function list.
 *
 * Arguments: args => ptr to argument string
 *
 * Returns: void function
 *
 */
static void AlertFastInit(struct _SnortConfig *sc, char *args)
{
    SpoAlertFastData *data;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Output: AlertFast Initialized\n"););

    /* parse the argument list from the rules file */
    data = ParseAlertFastArgs(sc, args);

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Linking AlertFast functions to call lists...\n"););

    /* Set the preprocessor function into the function list */
    AddFuncToOutputList(sc, AlertFast, OUTPUT_TYPE__ALERT, data);
    AddFuncToCleanExitList(AlertFastCleanExitFunc, data);
}

static const char* s_dispos[] = { " [Allow]", " [CDrop]", " [WDrop]", " [Drop]", " [FDrop]" };

static void AlertFast(Packet *p, const char *msg, void *arg, Event *event)
{
    SpoAlertFastData *data = (SpoAlertFastData *)arg;
    tActiveDrop dispos = Active_GetDisposition();

    LogTimeStamp(data->log, p);

    if( p != NULL && dispos > ACTIVE_ALLOW )
    {
        if ( dispos > ACTIVE_DROP )
            dispos = ACTIVE_DROP;

        TextLog_Puts(data->log, s_dispos[dispos]);
    }

    {
#ifdef MARK_TAGGED
        char c=' ';
        if ((p != NULL) && (p->packet_flags & PKT_REBUILT_STREAM))
            c = 'R';
        else if ((p != NULL) && (p->packet_flags & PKT_REBUILT_FRAG))
            c = 'F';
        TextLog_Print(data->log, " [**] %c ", c);
#else
        TextLog_Puts(data->log, " [**] ");
#endif

        if(event != NULL)
        {
            TextLog_Print(data->log, "[%lu:%lu:%lu] ",
                    (unsigned long) event->sig_generator,
                    (unsigned long) event->sig_id,
                    (unsigned long) event->sig_rev);
        }

        if (ScAlertInterface())
        {
            TextLog_Print(data->log, " <%s> ", PRINT_INTERFACE(DAQ_GetInterfaceSpec()));
        }

        if (msg != NULL)
        {
            TextLog_Puts(data->log, msg);
            TextLog_Puts(data->log, " [**] ");
        }
        else
        {
            TextLog_Puts(data->log, "[**] ");
        }
    }

    /* print the packet header to the alert file */
    if ((p != NULL) && IPH_IS_VALID(p))
    {
        LogPriorityData(data->log, 0);
#if defined(FEAT_OPEN_APPID)
        LogAppID(data->log, event->app_name, 0);
#endif
        TextLog_Print(data->log, "{%s} ", protocol_names[GET_IPH_PROTO(p)]);
        LogIpAddrs(data->log, p);
    }

    if(p && data->packet_flag)
    {
        /* Log whether or not this is reassembled data - only indicate
         * if we're actually going to show any of the payload */
        if (ScOutputAppData() && (p->dsize > 0) && PacketWasCooked(p))
        {
            switch ( p->pseudo_type ) {
            case PSEUDO_PKT_SMB_SEG:
                TextLog_Print(data->log, "\n%s", "SMB desegmented packet");
                break;
            case PSEUDO_PKT_DCE_SEG:
                TextLog_Print(data->log, "\n%s", "DCE/RPC desegmented packet");
                break;
            case PSEUDO_PKT_DCE_FRAG:
                TextLog_Print(data->log, "\n%s", "DCE/RPC defragmented packet");
                break;
            case PSEUDO_PKT_SMB_TRANS:
                TextLog_Print(data->log, "\n%s", "SMB Transact reassembled packet");
                break;
            case PSEUDO_PKT_DCE_RPKT:
                TextLog_Print(data->log, "\n%s", "DCE/RPC reassembled packet");
                break;
            case PSEUDO_PKT_TCP:
                TextLog_Print(data->log, "\n%s", "Stream reassembled packet");
                break;
            case PSEUDO_PKT_IP:
                TextLog_Print(data->log, "\n%s", "Frag reassembled packet");
                break;
            default:
                // FIXTHIS do we get here for portscan or sdf?
                break;
            }
        }

        TextLog_NewLine(data->log);

        if(IPH_IS_VALID(p))
            LogIPPkt(data->log, GET_IPH_PROTO(p), p);
#ifndef NO_NON_ETHER_DECODER
        else if(p->ah)
            LogArpHeader(data->log, p);
#endif
    }
    TextLog_NewLine(data->log);
    TextLog_Flush(data->log);
}

/*
 * Function: ParseAlertFastArgs(char *)
 *
 * Purpose: Process positional args, if any.  Syntax is:
 * output alert_fast: [<logpath> ["packet"] [<limit>]]
 * limit ::= <number>('G'|'M'|K')
 *
 * Arguments: args => argument list
 *
 * Returns: void function
 *
 */
static SpoAlertFastData *ParseAlertFastArgs(struct _SnortConfig *sc, char *args)
{
    char **toks;
    int num_toks;
    SpoAlertFastData *data;
    char* filename = NULL;
    unsigned long limit = DEFAULT_LIMIT;
    unsigned int bufSize = FAST_BUF;
    int i;

    DEBUG_WRAP(DebugMessage(DEBUG_LOG, "ParseAlertFastArgs: %s\n", args););
    data = (SpoAlertFastData *)SnortAlloc(sizeof(SpoAlertFastData));

    if ( !data )
    {
        FatalError("alert_fast: unable to allocate memory!\n");
    }
    if ( !args ) args = "";
    toks = mSplit((char *)args, " \t", 0, &num_toks, '\\');

    for (i = 0; i < num_toks; i++)
    {
        const char* tok = toks[i];
        char *end;

        switch (i)
        {
            case 0:
                if ( !strcasecmp(tok, "stdout") )
                    filename = SnortStrdup(tok);

                else
                    filename = ProcessFileOption(sc, tok);
                break;

            case 1:
                if ( !strcasecmp("packet", tok) )
                {
                    data->packet_flag = 1;
                    bufSize = FULL_BUF;
                    break;
                }
                /* in this case, only 2 options allowed */
                else i++;
                /* fall thru so "packet" is optional ... */

            case 2:
                limit = strtol(tok, &end, 10);

                if ( tok == end )
                    FatalError("alert_fast error in %s(%i): %s\n",
                        file_name, file_line, tok);

                if ( end && toupper(*end) == 'G' )
                    limit <<= 30; /* GB */

                else if ( end && toupper(*end) == 'M' )
                    limit <<= 20; /* MB */

                else if ( end && toupper(*end) == 'K' )
                    limit <<= 10; /* KB */
                break;

            case 3:
                FatalError("alert_fast: error in %s(%i): %s\n",
                    file_name, file_line, tok);
                break;
        }
    }
    mSplitFree(&toks, num_toks);

#ifdef DEFAULT_FILE
    if ( !filename ) filename = ProcessFileOption(sc, DEFAULT_FILE);
#endif

    DEBUG_WRAP(DebugMessage(
        DEBUG_INIT, "alert_fast: '%s' %d %ld\n",
        filename?filename:"alert", data->packet_flag, limit
    ););

    if ((filename == NULL) && (sc->alert_file != NULL))
        filename = SnortStrdup(sc->alert_file);

    data->log = TextLog_Init(filename, bufSize, limit);

    if (filename != NULL)
        free(filename);

    return data;
}

static void AlertFastCleanup(int signal, void *arg, const char* msg)
{
    SpoAlertFastData *data = (SpoAlertFastData *)arg;
    DEBUG_WRAP(DebugMessage(DEBUG_LOG, "%s\n", msg););

    /*free memory from SpoAlertFastData */
    if ( data->log ) TextLog_Term(data->log);
    free(data);
}

static void AlertFastCleanExitFunc(int signal, void *arg)
{
    AlertFastCleanup(signal, arg, "AlertFastCleanExitFunc");
}

