/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2002-2013 Sourcefire, Inc.
** Copyright (C) 1998-2002 Martin Roesch <roesch@sourcefire.com>
**
** Author(s):   Martin Roesch <roesch@sourcefire.com>
**              Andrew R. Baker <andrewb@sourcefire.com>
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

/* spo_log_ascii
 *
 * Purpose:
 *
 * This output module provides the default packet logging funtionality
 *
 * Arguments:
 *
 * None.
 *
 * Effect:
 *
 * None.
 *
 * Comments:
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif /* ! WIN32 */

#include "spo_log_ascii.h"
#include "plugbase.h"
#include "spo_plugbase.h"
#include "parser.h"
#include "snort_debug.h"
#include "decode.h"
#include "event.h"
#include "log.h"
#include "util.h"

#include "snort.h"

/* external globals from rules.c */
extern OptTreeNode *otn_tmp;

/* internal functions */
static void LogAsciiInit(struct _SnortConfig *, char *args);
static void LogAscii(Packet *p, const char *msg, void *arg, Event *event);
static void LogAsciiCleanExit(int signal, void *arg);
static char *IcmpFileName(Packet * p);
static FILE *OpenLogFile(int mode, Packet * p);


#define DUMP              1
#define BOGUS             2
#define NON_IP            3
#define ARP               4
#define GENERIC_LOG   5

void LogAsciiSetup(void)
{
    /* link the preprocessor keyword to the init function in
       the preproc list */
    RegisterOutputPlugin("log_ascii", OUTPUT_TYPE_FLAG__LOG, LogAsciiInit);

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Output: LogAscii is setup\n"););
}

static void LogAsciiInit(struct _SnortConfig *sc, char *args)
{
    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Output: Ascii logging initialized\n"););

    /* Set the preprocessor function into the function list */
    AddFuncToOutputList(sc, LogAscii, OUTPUT_TYPE__LOG, NULL);
    AddFuncToCleanExitList(LogAsciiCleanExit, NULL);
}

static void LogAscii(Packet *p, const char *msg, void *arg, Event *event)
{
    FILE *log_ptr = NULL;
    DEBUG_WRAP(DebugMessage(DEBUG_LOG, "LogPkt started\n"););
    if(p)
    {
        if(IPH_IS_VALID(p))
            log_ptr = OpenLogFile(0, p);
#ifndef NO_NON_ETHER_DECODER
        else if(p->ah)
            log_ptr = OpenLogFile(ARP, p);
#endif
        else
            log_ptr = OpenLogFile(NON_IP, p);
    }
    else
        log_ptr = OpenLogFile(GENERIC_LOG, p);

    if(!log_ptr)
        FatalError("Unable to open packet log file\n");

    if(msg)
    {
        fwrite("[**] ", 5, 1, log_ptr);

        /*
         * Protect against potential log injection,
         * check for delimiters and newlines in msg
         */
        if(  !strstr(msg,"[**]") && !strchr(msg,'\n') )
        {
          fwrite(msg, strlen(msg), 1, log_ptr);
        }
        fwrite(" [**]\n", 6, 1, log_ptr);
    }
    if(p)
    {
        if(IPH_IS_VALID(p))
            PrintIPPkt(log_ptr, GET_IPH_PROTO(p), p);
#ifndef NO_NON_ETHER_DECODER
        else if(p->ah)
            PrintArpHeader(log_ptr, p);
#endif
    }
    if(log_ptr)
        fclose(log_ptr);
}


static void LogAsciiCleanExit(int signal, void *arg)
{
    return;
}

static char *logfile[] =
        { "", "PACKET_FRAG", "PACKET_BOGUS", "PACKET_NONIP", "ARP", "log" };

/*
 * Function: OpenLogFile()
 *
 * Purpose: Create the log directory and file to put the packet log into.
 *          This function sucks, I've got to find a better way to do this
 *          this stuff.
 *
 * Arguments: None.
 *
 * Returns: FILE pointer on success, else NULL
 */
static FILE *OpenLogFile(int mode, Packet * p)
{
    char log_path[STD_BUF]; /* path to log file */
    char log_file[STD_BUF]; /* name of log file */
    char proto[5];      /* logged packet protocol */
    char suffix[5];     /* filename suffix */
    FILE *log_ptr = NULL;
    sfaddr_t* ip;

#ifdef WIN32
    SnortStrncpy(suffix, ".ids", sizeof(suffix));
#else
    suffix[0] = '\0';
#endif

    /* zero out our buffers */
    memset((char *) log_path, 0, STD_BUF);
    memset((char *) log_file, 0, STD_BUF);
    memset((char *) proto, 0, 5);

    if (mode == GENERIC_LOG || mode == DUMP || mode == BOGUS ||
        mode == NON_IP || mode == ARP)
    {
        SnortSnprintf(log_file, STD_BUF, "%s/%s", snort_conf->log_dir, logfile[mode]);

        log_ptr = fopen(log_file, "a");
        if (!log_ptr)
        {
            FatalError("OpenLogFile() => fopen(%s) log file: %s\n",
                       log_file, strerror(errno));
        }

        return log_ptr;
    }

    if(otn_tmp != NULL)
    {
        if(otn_tmp->logto != NULL)
        {
            SnortSnprintf(log_file, STD_BUF, "%s/%s", snort_conf->log_dir, otn_tmp->logto);

            log_ptr = fopen(log_file, "a");
            if (!log_ptr)
            {
                FatalError("OpenLogFile() => fopen(%s) log file: %s\n",
                           log_file, strerror(errno));
            }
            return log_ptr;
        }
    }
    ip = GET_DST_IP(p);
    if(sfip_contains(&snort_conf->homenet, ip) == SFIP_CONTAINS)
    {
        if(sfip_contains(&snort_conf->homenet, ip) == SFIP_CONTAINS)
        {
            SnortSnprintf(log_path, STD_BUF, "%s/%s", snort_conf->log_dir,
                    inet_ntoa(GET_SRC_ADDR(p)));
        }
        else
        {
            if(p->sp >= p->dp)
            {
                SnortSnprintf(log_path, STD_BUF, "%s/%s", snort_conf->log_dir,
                        inet_ntoa(GET_SRC_ADDR(p)));
            }
            else
            {
                SnortSnprintf(log_path, STD_BUF, "%s/%s", snort_conf->log_dir,
                        inet_ntoa(GET_DST_ADDR(p)));
            }
        }
    }
    else
    {
        ip = GET_SRC_IP(p);
        if(sfip_contains(&snort_conf->homenet, ip) == SFIP_CONTAINS)
        {
            SnortSnprintf(log_path, STD_BUF, "%s/%s", snort_conf->log_dir,
                    inet_ntoa(GET_DST_ADDR(p)));
        }
        else
        {
            if(p->sp >= p->dp)
            {
                SnortSnprintf(log_path, STD_BUF, "%s/%s", snort_conf->log_dir,
                        inet_ntoa(GET_SRC_ADDR(p)));
            }
            else
            {
                SnortSnprintf(log_path, STD_BUF, "%s/%s", snort_conf->log_dir,
                        inet_ntoa(GET_DST_ADDR(p)));
            }
        }
    }

    DEBUG_WRAP(DebugMessage(DEBUG_FLOW, "Creating directory: %s\n", log_path););

    /* build the log directory */
    if(mkdir(log_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
    {

        if(errno != EEXIST)
        {
            FatalError("OpenLogFile() => mkdir(%s) log directory: %s\n",
                       log_path, strerror(errno));
        }
    }

    DEBUG_WRAP(DebugMessage(DEBUG_FLOW, "Directory Created!\n"););

    /* build the log filename */
    if(GET_IPH_PROTO(p) == IPPROTO_TCP ||
            GET_IPH_PROTO(p) == IPPROTO_UDP)
    {
        if(p->frag_flag)
        {
            SnortSnprintf(log_file, STD_BUF, "%s/IP_FRAG%s", log_path, suffix);
        }
        else
        {
            if(p->sp >= p->dp)
            {
#ifdef WIN32
                SnortSnprintf(log_file, STD_BUF, "%s/%s_%d-%d%s", log_path,
                        protocol_names[GET_IPH_PROTO(p)], p->sp, p->dp, suffix);
#else
                SnortSnprintf(log_file, STD_BUF, "%s/%s:%d-%d%s", log_path,
                        protocol_names[GET_IPH_PROTO(p)], p->sp, p->dp, suffix);
#endif
            }
            else
            {
#ifdef WIN32
                SnortSnprintf(log_file, STD_BUF, "%s/%s_%d-%d%s", log_path,
                        protocol_names[GET_IPH_PROTO(p)], p->dp, p->sp, suffix);
#else
                SnortSnprintf(log_file, STD_BUF, "%s/%s:%d-%d%s", log_path,
                        protocol_names[GET_IPH_PROTO(p)], p->dp, p->sp, suffix);
#endif
            }
        }
    }
    else
    {
        if(p->frag_flag)
        {
            SnortSnprintf(log_file, STD_BUF, "%s/IP_FRAG%s", log_path, suffix);
        }
        else
        {
            if (GET_IPH_PROTO(p) == IPPROTO_ICMP)
            {
                SnortSnprintf(log_file, STD_BUF, "%s/%s_%s%s", log_path, "ICMP",
                              IcmpFileName(p), suffix);
            }
            else
            {
                SnortSnprintf(log_file, STD_BUF, "%s/PROTO%d%s", log_path,
                         GET_IPH_PROTO(p), suffix);
            }
        }
    }

    DEBUG_WRAP(DebugMessage(DEBUG_FLOW, "Opening file: %s\n", log_file););

    /* finally open the log file */
    log_ptr = fopen(log_file, "a");
    if (!log_ptr)
    {
        FatalError("OpenLogFile() => fopen(%s) log file: %s\n",
                   log_file, strerror(errno));
    }

    DEBUG_WRAP(DebugMessage(DEBUG_FLOW, "File opened...\n"););
    return log_ptr;
}



/****************************************************************************
 *
 * Function: IcmpFileName(Packet *p)
 *
 * Purpose: Set the filename of an ICMP output log according to its type
 *
 * Arguments: p => Packet data struct
 *
 * Returns: the name of the file to set
 *
 ***************************************************************************/
static char *IcmpFileName(Packet * p)
{
    if(p->icmph == NULL)
    {
        return "ICMP_TRUNC";
    }

    switch(p->icmph->type)
    {
        case ICMP_ECHOREPLY:
            return "ECHO_REPLY";

        case ICMP_DEST_UNREACH:
            switch(p->icmph->code)
            {
                case ICMP_NET_UNREACH:
                    return "NET_UNRCH";

                case ICMP_HOST_UNREACH:
                    return "HST_UNRCH";

                case ICMP_PROT_UNREACH:
                    return "PROTO_UNRCH";

                case ICMP_PORT_UNREACH:
                    return "PORT_UNRCH";

                case ICMP_FRAG_NEEDED:
                    return "UNRCH_FRAG_NEEDED";

                case ICMP_SR_FAILED:
                    return "UNRCH_SOURCE_ROUTE_FAILED";

                case ICMP_NET_UNKNOWN:
                    return "UNRCH_NETWORK_UNKNOWN";

                case ICMP_HOST_UNKNOWN:
                    return "UNRCH_HOST_UNKNOWN";

                case ICMP_HOST_ISOLATED:
                    return "UNRCH_HOST_ISOLATED";

                case ICMP_PKT_FILTERED_NET:
                    return "UNRCH_PKT_FILTERED_NET";

                case ICMP_PKT_FILTERED_HOST:
                    return "UNRCH_PKT_FILTERED_HOST";

                case ICMP_NET_UNR_TOS:
                    return "UNRCH_NET_UNR_TOS";

                case ICMP_HOST_UNR_TOS:
                    return "UNRCH_HOST_UNR_TOS";

                case ICMP_PKT_FILTERED:
                    return "UNRCH_PACKET_FILT";

                case ICMP_PREC_VIOLATION:
                    return "UNRCH_PREC_VIOL";

                case ICMP_PREC_CUTOFF:
                    return "UNRCH_PREC_CUTOFF";

                default:
                    return "UNKNOWN";

            }

        case ICMP_SOURCE_QUENCH:
            return "SRC_QUENCH";

        case ICMP_REDIRECT:
            return "REDIRECT";

        case ICMP_ECHO:
            return "ECHO";

        case ICMP_TIME_EXCEEDED:
            return "TTL_EXCEED";

        case ICMP_PARAMETERPROB:
            return "PARAM_PROB";

        case ICMP_TIMESTAMP:
            return "TIMESTAMP";

        case ICMP_TIMESTAMPREPLY:
            return "TIMESTAMP_RPL";

        case ICMP_INFO_REQUEST:
            return "INFO_REQ";

        case ICMP_INFO_REPLY:
            return "INFO_RPL";

        case ICMP_ADDRESS:
            return "ADDR";

        case ICMP_ADDRESSREPLY:
            return "ADDR_RPL";

        default:
            return "UNKNOWN";
    }
}


