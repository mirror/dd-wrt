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

/* spo_alert_full
 *
 * Purpose:  output plugin for full alerting
 *
 * Arguments:  alert file (eventually)
 *
 * Effect:
 *
 * Alerts are written to a file in the snort full alert format
 *
 * Comments:   Allows use of full alerts with other output plugin types
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include "sf_types.h"
#include "spo_alert_full.h"
#include "event.h"
#include "decode.h"
#include "plugbase.h"
#include "spo_plugbase.h"
#include "snort_debug.h"
#include "parser.h"
#include "util.h"
#include "log.h"
#include "mstring.h"
#include "snort.h"
#include "sfutil/sf_textlog.h"
#include "log_text.h"
#include "sfdaq.h"

typedef struct _SpoAlertFullData
{
    TextLog* log;
} SpoAlertFullData;

static void AlertFullInit(struct _SnortConfig *sc, char *);
static SpoAlertFullData *ParseAlertFullArgs(struct _SnortConfig *, char *);
static void AlertFull(Packet *, const char *, void *, Event *);
static void AlertFullCleanExit(int, void *);

/*
 * not defined for backwards compatibility
 * (default is produced by OpenAlertFile()
#define DEFAULT_FILE  "alert.full"
 */
#define DEFAULT_LIMIT (128*M_BYTES)
#define LOG_BUFFER    (4*K_BYTES)

/*
 * Function: SetupAlertFull()
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
void AlertFullSetup(void)
{
    /* link the preprocessor keyword to the init function in
       the preproc list */
    RegisterOutputPlugin("alert_full", OUTPUT_TYPE_FLAG__ALERT, AlertFullInit);

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Output plugin: AlertFull is setup...\n"););
}


/*
 * Function: AlertFullInit(char *)
 *
 * Purpose: Calls the argument parsing function, performs final setup on data
 *          structs, links the preproc function into the function list.
 *
 * Arguments: args => ptr to argument string
 *
 * Returns: void function
 *
 */
static void AlertFullInit(struct _SnortConfig *sc, char *args)
{
    SpoAlertFullData *data;
    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Output: AlertFull Initialized\n"););

    /* parse the argument list from the rules file */
    data = ParseAlertFullArgs(sc, args);
    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Linking AlertFull functions to call lists...\n"););

    /* Set the preprocessor function into the function list */
    AddFuncToOutputList(sc, AlertFull, OUTPUT_TYPE__ALERT, data);
    AddFuncToCleanExitList(AlertFullCleanExit, data);
}

static void AlertFull(Packet *p, const char *msg, void *arg, Event *event)
{
    SpoAlertFullData *data = (SpoAlertFullData *)arg;

    {
        TextLog_Puts(data->log, "[**] ");

        if(event != NULL)
        {
                TextLog_Print(data->log, "[%lu:%lu:%lu] ",
                        (unsigned long) event->sig_generator,
                        (unsigned long) event->sig_id,
                        (unsigned long) event->sig_rev);
        }

        if (ScAlertInterface())
        {
            const char* iface = PRINT_INTERFACE(DAQ_GetInterfaceSpec());
            TextLog_Print(data->log, " <%s> ", iface);
        }

        if(msg != NULL)
        {
            TextLog_Puts(data->log, msg);
            TextLog_Puts(data->log, " [**]\n");
        }
        else
        {
            TextLog_Puts(data->log, "[**]\n");
        }
    }

    if(p && IPH_IS_VALID(p))
    {
        LogPriorityData(data->log, TRUE);
#if defined(FEAT_OPEN_APPID)
        LogAppID(data->log, event->app_name, TRUE);
#endif
    }

    DEBUG_WRAP(DebugMessage(DEBUG_LOG, "Logging Alert data!\n"););

    LogTimeStamp(data->log, p);

    if(p && IPH_IS_VALID(p))
    {
        /* print the packet header to the alert file */

        if (ScOutputDataLink())
        {
            Log2ndHeader(data->log, p);
        }

        LogIPHeader(data->log, p);

        /* if this isn't a fragment, print the other header info */
        if(!p->frag_flag)
        {
            switch(GET_IPH_PROTO(p))
            {
                case IPPROTO_TCP:
                   LogTCPHeader(data->log, p);
                    break;

                case IPPROTO_UDP:
                   LogUDPHeader(data->log, p);
                    break;

                case IPPROTO_ICMP:
                   LogICMPHeader(data->log, p);
                    break;

                default:
                    break;
            }
        }
        LogXrefs(data->log, 1);

        TextLog_Putc(data->log, '\n');
    } /* End of if(p) */
    else
    {
        TextLog_Puts(data->log, "\n\n");
    }
    TextLog_Flush(data->log);
}

/*
 * Function: ParseAlertFullArgs(char *)
 *
 * Purpose: Process positional args, if any.  Syntax is:
 * output alert_full: [<logpath> [<limit>]]
 * limit ::= <number>('G'|'M'|K')
 *
 * Arguments: args => argument list
 *
 * Returns: void function
 */
static SpoAlertFullData *ParseAlertFullArgs(struct _SnortConfig *sc, char *args)
{
    char **toks;
    int num_toks;
    SpoAlertFullData *data;
    char* filename = NULL;
    unsigned long limit = DEFAULT_LIMIT;
    int i;

    DEBUG_WRAP(DebugMessage(DEBUG_LOG, "ParseAlertFullArgs: %s\n", args););
    data = (SpoAlertFullData *)SnortAlloc(sizeof(SpoAlertFullData));

    if ( !data )
    {
        FatalError("alert_full: unable to allocate memory!\n");
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
                limit = strtol(tok, &end, 10);

                if ( tok == end )
                    FatalError("alert_full error in %s(%i): %s\n",
                        file_name, file_line, tok);

                if ( end && toupper(*end) == 'G' )
                    limit <<= 30; /* GB */

                else if ( end && toupper(*end) == 'M' )
                    limit <<= 20; /* MB */

                else if ( end && toupper(*end) == 'K' )
                    limit <<= 10; /* KB */
                break;

            case 2:
                FatalError("alert_full: error in %s(%i): %s\n",
                    file_name, file_line, tok);
                break;
        }
    }
    mSplitFree(&toks, num_toks);

#ifdef DEFAULT_FILE
    if ( !filename ) filename = ProcessFileOption(sc, DEFAULT_FILE);
#endif

    DEBUG_WRAP(DebugMessage(
        DEBUG_INIT, "alert_full: '%s' %ld\n",
        filename ? filename : "alert", limit
    ););

    if ((filename == NULL) && (sc->alert_file != NULL))
        filename = SnortStrdup(sc->alert_file);

    data->log = TextLog_Init(filename, LOG_BUFFER, limit);

    if (filename != NULL)
        free(filename);

    return data;
}

static void AlertFullCleanup(int signal, void *arg, const char* msg)
{
    SpoAlertFullData *data = (SpoAlertFullData *)arg;
    DEBUG_WRAP(DebugMessage(DEBUG_LOG, "%s\n", msg););

    /* free memory from SpoAlertFullData */
    if ( data->log ) TextLog_Term(data->log);
    free(data);
}

static void AlertFullCleanExit(int signal, void *arg)
{
    AlertFullCleanup(signal, arg, "AlertFullCleanExit");
}

