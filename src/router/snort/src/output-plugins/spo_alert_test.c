/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2007-2013 Sourcefire, Inc.
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

/* spo_alert_test_
 *
 * Purpose:  output plugin for test alerting
 *
 * Arguments: file <file>, stdout, rebuilt, session, msg
 * arguments should be comma delimited.
 * file - specifiy alert file
 * stdout - no alert file, just print to screen
 * rebuilt - include info of whether packet was rebuilt or not
 *           S - Stream rebuilt
 *           F - IP frag rebuilt
 *           outputs: <rebuilt type>:<rebuilt count>
 * session - include src/dst IPs and ports
 *           outputs: <sip>:<sport>-<dip>:<dport>
 * msg - include alert message
 *
 * Output is tab delimited in the following order:
 * packet count, gid, sid, rev, msg, session, rebuilt
 *
 * Examples:
 * output alert_test
 * output alert_test: session
 * output alert_test: session, msg
 * output alert_test: rebuilt, session, msg
 * output alert_test: stdout, rebuilt, session, msg
 * output alert_test: file test.alert, rebuilt, session, msg
 *
 * Effect:
 *
 * Alerts are written to a file in the snort test alert format
 *
 * Comments:   Allows use of test alerts with other output plugin types
 *
 */

/* output plugin header file */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "event.h"
#include "decode.h"
#include "snort_debug.h"
#include "plugbase.h"
#include "spo_plugbase.h"
#include "parser.h"
#include "util.h"
#include "log.h"
#include "mstring.h"
#include "snort.h"
#include "spo_alert_test.h"

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

#define TEST_FLAG_FILE     0x01
#define TEST_FLAG_STDOUT   0x02
#define TEST_FLAG_MSG      0x04
#define TEST_FLAG_SESSION  0x08
#define TEST_FLAG_REBUILT  0x10

typedef struct _SpoAlertTestData
{
    FILE *file;
    uint8_t flags;

} SpoAlertTestData;

void AlertTestInit(struct _SnortConfig *, char *);
SpoAlertTestData *ParseAlertTestArgs(struct _SnortConfig *, char *);
void AlertTestCleanExitFunc(int, void *);
void AlertTest(Packet *, const char *, void *, Event *);

extern PacketCount pc;


/*
 * Function: SetupAlertTest()
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
void AlertTestSetup(void)
{
    /* link the preprocessor keyword to the init function in
       the preproc list */
    RegisterOutputPlugin("alert_test", OUTPUT_TYPE_FLAG__ALERT, AlertTestInit);
    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Output plugin: AlertTest is setup...\n"););
}


/*
 * Function: AlertTestInit(char *)
 *
 * Purpose: Calls the argument parsing function, performs final setup on data
 *          structs, links the preproc function into the function list.
 *
 * Arguments: args => ptr to argument string
 *
 * Returns: void function
 *
 */
void AlertTestInit(struct _SnortConfig *sc, char *args)
{
    SpoAlertTestData *data;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Output: AlertTest Initialized\n"););

    /* parse the argument list from the rules file */
    data = ParseAlertTestArgs(sc, args);

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Linking AlertTest functions to call lists...\n"););

    /* Set the preprocessor function into the function list */
    AddFuncToOutputList(sc, AlertTest, OUTPUT_TYPE__ALERT, data);
    AddFuncToCleanExitList(AlertTestCleanExitFunc, data);
}

void AlertTest(Packet *p, const char *msg, void *arg, Event *event)
{
    SpoAlertTestData *data;

    if (p == NULL || arg == NULL)
        return;

    data = (SpoAlertTestData *)arg;

    fprintf(data->file, "" STDu64 "\t", pc.total_from_daq);

    if (event != NULL)
    {
        fprintf(data->file, "%lu\t%lu\t%lu\t",
                (unsigned long) event->sig_generator,
                (unsigned long) event->sig_id,
                (unsigned long) event->sig_rev);
    }

    if (data->flags & TEST_FLAG_MSG)
    {
        if (msg != NULL)
            fprintf(data->file, "%s\t", msg);
    }

    if (data->flags & TEST_FLAG_SESSION)
        PrintIpAddrs(data->file, p);

    if (data->flags & TEST_FLAG_REBUILT)
    {
        if (p->packet_flags & PKT_REBUILT_FRAG)
            fprintf(data->file, "F:" STDu64 "\t", pc.rebuilt_frags);
        else if (p->packet_flags & PKT_REBUILT_STREAM)
            fprintf(data->file, "S:" STDu64 "\t", pc.rebuilt_tcp);
    }

    fprintf(data->file, "\n");
    fflush(data->file);
}

/*
 * Function: ParseAlertTestArgs(char *)
 *
 * Purpose: Process the preprocessor arguements from the rules file and
 *          initialize the preprocessor's data struct.  This function doesn't
 *          have to exist if it makes sense to parse the args in the init
 *          function.
 *
 * Arguments: args => argument list
 *
 * Returns: void function
 *
 */
SpoAlertTestData * ParseAlertTestArgs(struct _SnortConfig *sc, char *args)
{
    char **toks;
    int num_toks;
    SpoAlertTestData *data;
    int i;

    data = (SpoAlertTestData *)SnortAlloc(sizeof(SpoAlertTestData));

    if (args == NULL)
    {
        data->file = OpenAlertFile(NULL);
        data->flags |= TEST_FLAG_FILE;
        return data;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_LOG, "ParseAlertTestArgs: %s\n", args););

    toks = mSplit(args, ",", 0, &num_toks, 0);

    for (i = 0; i < num_toks; i++)
    {
        char *option;
        char **atoks;
        int num_atoks;

        atoks = mSplit(toks[i], " ", 0, &num_atoks, 0);
        option = atoks[0];

        if (!strcasecmp("stdout", option))
        {
            if (data->flags & TEST_FLAG_FILE)
                ParseError("alert_test: cannot specify both stdout and file\n");

            data->file = stdout;
            data->flags |= TEST_FLAG_STDOUT;
        }
        else if (!strcasecmp("session", option))
        {
            data->flags |= TEST_FLAG_SESSION;
        }
        else if (!strcasecmp("rebuilt", option))
        {
            data->flags |= TEST_FLAG_REBUILT;
        }
        else if (!strcasecmp("msg", option))
        {
            data->flags |= TEST_FLAG_MSG;
        }
        else if (!strcasecmp("file", option))
        {
            if (data->flags & TEST_FLAG_STDOUT)
                ParseError("alert_test: cannot specify both stdout and file\n");

            data->flags |= TEST_FLAG_FILE;

            if (num_atoks == 1)
            {
                data->file = OpenAlertFile(NULL);
            }
            else if (num_atoks == 2)
            {
                char *outfile = ProcessFileOption(sc, atoks[1]);
                data->file = OpenAlertFile(outfile);
                free(outfile);
            }
            else
            {
                ParseError("Invalid \"file\" argument to alert_test: %s", option);
            }
        }
        else
        {
            ParseError("Unrecognized alert_test option: %s\n", option);
        }

        mSplitFree(&atoks, num_atoks);
    }

    /* free toks */
    mSplitFree(&toks, num_toks);

    /* didn't get stdout or a file to log to */
    if (!(data->flags & (TEST_FLAG_STDOUT | TEST_FLAG_FILE)))
    {
        data->file = OpenAlertFile(NULL);
        data->flags |= TEST_FLAG_FILE;
    }

    return data;
}

void AlertTestCleanExitFunc(int signal, void *arg)
{
    SpoAlertTestData *data = (SpoAlertTestData *)arg;

    /* close alert file */
    DEBUG_WRAP(DebugMessage(DEBUG_LOG,"AlertTestCleanExitFunc\n"););
    fclose(data->file);

    /*free memory from SpoAlertTestData */
    free(data);
}

