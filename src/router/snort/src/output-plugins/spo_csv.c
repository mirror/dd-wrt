/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
** Copyright (C) 2001 Brian Caswell <bmc@mitre.org>
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

/* spo_csv
 *
 * Purpose:  output plugin for csv alerting
 *
 * Arguments:  alert file (eventually)
 *
 * Effect:
 *
 * Alerts are written to a file in the snort csv alert format
 *
 * Comments:   Allows use of csv alerts with other output plugin types
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif /* !WIN32 */

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#include "spo_csv.h"
#include "event.h"
#include "decode.h"
#include "plugbase.h"
#include "spo_plugbase.h"
#include "parser.h"
#include "snort_debug.h"
#include "mstring.h"
#include "util.h"
#include "log.h"

#include "snort.h"

#include "sfutil/sf_textlog.h"
#include "log_text.h"

#define DEFAULT_CSV "timestamp,sig_generator,sig_id,sig_rev,msg,proto,src,srcport,dst,dstport,ethsrc,ethdst,ethlen,tcpflags,tcpseq,tcpack,tcpln,tcpwindow,ttl,tos,id,dgmlen,iplen,icmptype,icmpcode,icmpid,icmpseq"

#define DEFAULT_FILE  "alert.csv"
#define DEFAULT_LIMIT (128*M_BYTES)
#define LOG_BUFFER    (4*K_BYTES)

typedef struct _AlertCSVConfig
{
    char *type;
    struct _AlertCSVConfig *next;
} AlertCSVConfig;

typedef struct _AlertCSVData
{
    TextLog* log;
    char * csvargs;
    char ** args;
    int numargs;
    AlertCSVConfig *config;
} AlertCSVData;


/* list of function prototypes for this preprocessor */
static void AlertCSVInit(struct _SnortConfig *, char *);
static AlertCSVData *AlertCSVParseArgs(struct _SnortConfig *, char *);
static void AlertCSV(Packet *, const char *, void *, Event *);
static void AlertCSVCleanExit(int, void *);
static void RealAlertCSV(
    Packet*, const char* msg, char **args, int numargs, Event*, TextLog*
);

/*
 * Function: SetupCSV()
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
void AlertCSVSetup(void)
{
    /* link the preprocessor keyword to the init function in
       the preproc list */
    RegisterOutputPlugin("alert_CSV", OUTPUT_TYPE_FLAG__ALERT, AlertCSVInit);

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Output plugin: alert_CSV is setup...\n"););
}


/*
 * Function: CSVInit(char *)
 *
 * Purpose: Calls the argument parsing function, performs final setup on data
 *          structs, links the preproc function into the function list.
 *
 * Arguments: args => ptr to argument string
 *
 * Returns: void function
 *
 */
static void AlertCSVInit(struct _SnortConfig *sc, char *args)
{
    AlertCSVData *data;
    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Output: CSV Initialized\n"););

    /* parse the argument list from the rules file */
    data = AlertCSVParseArgs(sc, args);

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Linking CSV functions to call lists...\n"););

    /* Set the preprocessor function into the function list */
    AddFuncToOutputList(sc, AlertCSV, OUTPUT_TYPE__ALERT, data);
    AddFuncToCleanExitList(AlertCSVCleanExit, data);
}

/*
 * Function: ParseCSVArgs(char *)
 *
 * Purpose: Process positional args, if any.  Syntax is:
 * output alert_csv: [<logpath> ["default"|<list> [<limit>]]]
 * list ::= <field>(,<field>)*
 * field ::= "dst"|"src"|"ttl" ...
 * limit ::= <number>('G'|'M'|K')
 *
 * Arguments: args => argument list
 *
 * Returns: void function
 */
static AlertCSVData *AlertCSVParseArgs(struct _SnortConfig *sc, char *args)
{
    char **toks;
    int num_toks;
    AlertCSVData *data;
    char* filename = NULL;
    unsigned long limit = DEFAULT_LIMIT;
    int i;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "ParseCSVArgs: %s\n", args););
    data = (AlertCSVData *)SnortAlloc(sizeof(AlertCSVData));

    if ( !args ) args = "";
    toks = mSplit((char *)args, " \t", 4, &num_toks, '\\');

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
                if ( !strcasecmp("default", tok) )
                    data->csvargs = SnortStrdup(DEFAULT_CSV);
                else
                    data->csvargs = SnortStrdup(toks[i]);
                break;

            case 2:
                limit = strtol(tok, &end, 10);

                if ( tok == end )
                    FatalError("alert_csv error in %s(%i): %s\n",
                        file_name, file_line, tok);

                if ( end && toupper(*end) == 'G' )
                    limit <<= 30; /* GB */

                else if ( end && toupper(*end) == 'M' )
                    limit <<= 20; /* MB */

                else if ( end && toupper(*end) == 'K' )
                    limit <<= 10; /* KB */
                break;

            case 3:
                FatalError("alert_csv: error in %s(%i): %s\n",
                    file_name, file_line, tok);
                break;
        }
    }
    if ( !data->csvargs ) data->csvargs = SnortStrdup(DEFAULT_CSV);
    if ( !filename ) filename = ProcessFileOption(sc, DEFAULT_FILE);

    mSplitFree(&toks, num_toks);
    toks = mSplit(data->csvargs, ",", 0, &num_toks, 0);

    data->args = toks;
    data->numargs = num_toks;

    DEBUG_WRAP(DebugMessage(
        DEBUG_INIT, "alert_csv: '%s' '%s' %ld\n", filename, data->csvargs, limit
    ););
    data->log = TextLog_Init(filename, LOG_BUFFER, limit);
    if ( filename ) free(filename);

    return data;
}

static void AlertCSVCleanup(int signal, void *arg, const char* msg)
{
    AlertCSVData *data = (AlertCSVData *)arg;
    /* close alert file */
    DEBUG_WRAP(DebugMessage(DEBUG_LOG,"%s\n", msg););

    if(data)
    {
        mSplitFree(&data->args, data->numargs);
        if (data->log) TextLog_Term(data->log);
        free(data->csvargs);
        /* free memory from SpoCSVData */
        free(data);
    }
}

static void AlertCSVCleanExit(int signal, void *arg)
{
    AlertCSVCleanup(signal, arg, "AlertCSVCleanExit");
}

static void AlertCSV(Packet *p, const char *msg, void *arg, Event *event)
{
    AlertCSVData *data = (AlertCSVData *)arg;
    RealAlertCSV(p, msg, data->args, data->numargs, event, data->log);
}

/*
 *
 * Function: RealAlertCSV(Packet *, char *, FILE *, char *, numargs const int)
 *
 * Purpose: Write a user defined CSV message
 *
 * Arguments:     p => packet. (could be NULL)
 *              msg => the message to send
 *             args => CSV output arguments
 *          numargs => number of arguments
 *             log => Log
 * Returns: void function
 *
 */
static void RealAlertCSV(Packet * p, const char *msg, char **args,
        int numargs, Event *event, TextLog* log)
{
    int num;
    char *type;
    char tcpFlags[9];

    if(p == NULL)
        return;

    DEBUG_WRAP(DebugMessage(DEBUG_LOG,"Logging CSV Alert data\n"););

    for (num = 0; num < numargs; num++)
    {
        type = args[num];

        DEBUG_WRAP(DebugMessage(DEBUG_LOG, "CSV Got type %s %d\n", type, num););

        if (!strcasecmp("timestamp", type))
        {
            LogTimeStamp(log, p);
        }
        else if (!strcasecmp("sig_generator", type))
        {
            if (event != NULL)
                TextLog_Print(log, "%lu",  (unsigned long) event->sig_generator);
        }
        else if (!strcasecmp("sig_id", type))
        {
            if (event != NULL)
                TextLog_Print(log, "%lu",  (unsigned long) event->sig_id);
        }
        else if (!strcasecmp("sig_rev", type))
        {
            if (event != NULL)
                TextLog_Print(log, "%lu",  (unsigned long) event->sig_rev);
        }
        else if (!strcasecmp("msg", type))
        {
            TextLog_Quote(log, msg);  /* Don't fatal */
        }
        else if (!strcasecmp("proto", type))
        {
            if (IPH_IS_VALID(p))
            {
                switch (GET_IPH_PROTO(p))
                {
                    case IPPROTO_UDP:
                        TextLog_Puts(log, "UDP");
                        break;
                    case IPPROTO_TCP:
                        TextLog_Puts(log, "TCP");
                        break;
                    case IPPROTO_ICMP:
                        TextLog_Puts(log, "ICMP");
                        break;
                    default:
                        break;
                }
            }
        }
        else if (!strcasecmp("ethsrc", type))
        {
            if (p->eh != NULL)
            {
                TextLog_Print(log, "%02X:%02X:%02X:%02X:%02X:%02X", p->eh->ether_src[0],
                        p->eh->ether_src[1], p->eh->ether_src[2], p->eh->ether_src[3],
                        p->eh->ether_src[4], p->eh->ether_src[5]);
            }
        }
        else if (!strcasecmp("ethdst", type))
        {
            if (p->eh != NULL)
            {
                TextLog_Print(log, "%02X:%02X:%02X:%02X:%02X:%02X", p->eh->ether_dst[0],
                        p->eh->ether_dst[1], p->eh->ether_dst[2], p->eh->ether_dst[3],
                        p->eh->ether_dst[4], p->eh->ether_dst[5]);
            }
        }
        else if (!strcasecmp("ethtype", type))
        {
            if (p->eh != NULL)
                TextLog_Print(log, "0x%X", ntohs(p->eh->ether_type));
        }
        else if (!strcasecmp("udplength", type))
        {
            if (p->udph != NULL)
                TextLog_Print(log, "%d", ntohs(p->udph->uh_len));
        }
        else if (!strcasecmp("ethlen", type))
        {
            if (p->eh != NULL)
                TextLog_Print(log, "0x%X", p->pkth->pktlen);
        }
#ifndef NO_NON_ETHER_DECODER
        else if (!strcasecmp("trheader", type))
        {
            if (p->trh != NULL)
                LogTrHeader(log, p);
        }
#endif
        else if (!strcasecmp("srcport", type))
        {
            if (IPH_IS_VALID(p))
            {
                switch (GET_IPH_PROTO(p))
                {
                    case IPPROTO_UDP:
                    case IPPROTO_TCP:
                        TextLog_Print(log, "%d", p->sp);
                        break;
                    default:
                        break;
                }
            }
        }
        else if (!strcasecmp("dstport", type))
        {
            if (IPH_IS_VALID(p))
            {
                switch (GET_IPH_PROTO(p))
                {
                    case IPPROTO_UDP:
                    case IPPROTO_TCP:
                        TextLog_Print(log, "%d", p->dp);
                        break;
                    default:
                        break;
                }
            }
        }
        else if (!strcasecmp("src", type))
        {
            if (IPH_IS_VALID(p))
                TextLog_Puts(log, inet_ntoa(GET_SRC_ADDR(p)));
        }
        else if (!strcasecmp("dst", type))
        {
            if (IPH_IS_VALID(p))
                TextLog_Puts(log, inet_ntoa(GET_DST_ADDR(p)));
        }
        else if (!strcasecmp("icmptype", type))
        {
            if (p->icmph != NULL)
                TextLog_Print(log, "%d", p->icmph->type);
        }
        else if (!strcasecmp("icmpcode", type))
        {
            if (p->icmph != NULL)
                TextLog_Print(log, "%d", p->icmph->code);
        }
        else if (!strcasecmp("icmpid", type))
        {
            if (p->icmph != NULL)
                TextLog_Print(log, "%d", ntohs(p->icmph->s_icmp_id));
        }
        else if (!strcasecmp("icmpseq", type))
        {
            if (p->icmph != NULL)
                TextLog_Print(log, "%d", ntohs(p->icmph->s_icmp_seq));
        }
        else if (!strcasecmp("ttl", type))
        {
            if (IPH_IS_VALID(p))
                TextLog_Print(log, "%d", GET_IPH_TTL(p));
        }
        else if (!strcasecmp("tos", type))
        {
            if (IPH_IS_VALID(p))
                TextLog_Print(log, "%d", GET_IPH_TOS(p));
        }
        else if (!strcasecmp("id", type))
        {
            if (IPH_IS_VALID(p))
            {
                TextLog_Print(log, "%u", IS_IP6(p) ? ntohl(GET_IPH_ID(p))
                        : ntohs((uint16_t)GET_IPH_ID(p)));
            }
        }
        else if (!strcasecmp("iplen", type))
        {
            if (IPH_IS_VALID(p))
                TextLog_Print(log, "%d", GET_IPH_LEN(p) << 2);
        }
        else if (!strcasecmp("dgmlen", type))
        {
            if (IPH_IS_VALID(p))
            {
                // XXX might cause a bug when IPv6 is printed?
                TextLog_Print(log, "%d", ntohs(GET_IPH_LEN(p)));
            }
        }
        else if (!strcasecmp("tcpseq", type))
        {
            if (p->tcph != NULL)
                TextLog_Print(log, "0x%lX", (u_long)ntohl(p->tcph->th_seq));
        }
        else if (!strcasecmp("tcpack", type))
        {
            if (p->tcph != NULL)
                TextLog_Print(log, "0x%lX", (u_long)ntohl(p->tcph->th_ack));
        }
        else if (!strcasecmp("tcplen", type))
        {
            if (p->tcph != NULL)
                TextLog_Print(log, "%d", TCP_OFFSET(p->tcph) << 2);
        }
        else if (!strcasecmp("tcpwindow", type))
        {
            if (p->tcph != NULL)
                TextLog_Print(log, "0x%X", ntohs(p->tcph->th_win));
        }
        else if (!strcasecmp("tcpflags",type))
        {
            if (p->tcph != NULL)
            {
                CreateTCPFlagString(p, tcpFlags);
                TextLog_Print(log, "%s", tcpFlags);
            }
        }

        if (num < numargs - 1)
            TextLog_Putc(log, ',');
    }

    TextLog_NewLine(log);
    TextLog_Flush(log);
}

