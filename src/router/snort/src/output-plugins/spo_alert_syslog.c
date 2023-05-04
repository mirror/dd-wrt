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
/* $Id$ */

/* spo_alert_syslog
 *
 * Purpose:
 *
 * This module sends alerts to the syslog service.
 *
 * Arguments:
 *
 * Logging mechanism?
 *
 * Effect:
 *
 * Alerts are written to the syslog service with in the facility indicated by
 * the module arguments.
 *
 * Comments:
 *
 * First try
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <syslog.h>
#include <stdlib.h>
#ifdef HAVE_STRINGS_H
#include <strings.h>
#endif

#ifndef WIN32
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif /* !WIN32 */

#include "decode.h"
#include "event.h"
#include "rules.h"
#include "treenodes.h"
#include "plugbase.h"
#include "spo_plugbase.h"
#include "snort_debug.h"
#include "parser.h"
#include "mstring.h"
#include "util.h"
#include "strlcatu.h"
#include "strlcpyu.h"
#include "util_net.h"
#include "snort.h"
#include "sfdaq.h"

extern OptTreeNode *otn_tmp;

typedef struct _SyslogData
{
    int facility;
    int priority;
    int options;
} SyslogData;

static void AlertSyslogInit(struct _SnortConfig *, char *);
static SyslogData *ParseSyslogArgs(struct _SnortConfig *, char *);
static void AlertSyslog(Packet *, const char *, void *, Event *);
static void AlertSyslogCleanExit(int, void *);



/*
 * Function: SetupSyslog()
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
void AlertSyslogSetup(void)
{
    /* link the preprocessor keyword to the init function in
       the preproc list */
    RegisterOutputPlugin("alert_syslog", OUTPUT_TYPE_FLAG__ALERT, AlertSyslogInit);
    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Output plugin: Alert-Syslog is setup...\n"););
}


/*
 * Function: AlertSyslogInit(char *)
 *
 * Purpose: Calls the argument parsing function, performs final setup on data
 *          structs, links the preproc function into the function list.
 *
 * Arguments: args => ptr to argument string
 *
 * Returns: void function
 *
 */
static void AlertSyslogInit(struct _SnortConfig *sc, char *args)
{
    SyslogData *data;
    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Output: Alert-Syslog Initialized\n"););

    /* parse the argument list from the rules file */
    data = ParseSyslogArgs(sc, args);

    if (ScDaemonMode())
        data->options |= LOG_PID;

    openlog("snort", data->options, data->facility);

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Linking syslog alert function to call list...\n"););

    /* Set the preprocessor function into the function list */
    AddFuncToOutputList(sc, AlertSyslog, OUTPUT_TYPE__ALERT, data);
    AddFuncToCleanExitList(AlertSyslogCleanExit, data);
}



/*
 * Function: ParseSyslogArgs(struct _SnortConfig *, char *)
 *
 * Purpose: Process the preprocessor arguments from the rules file and
 *          initialize the preprocessor's data struct.  This function doesn't
 *          have to exist if it makes sense to parse the args in the init
 *          function.
 *
 * Arguments: args => argument list
 *
 * Returns: void function
 *
 */
static SyslogData *ParseSyslogArgs(struct _SnortConfig *sc, char *args)
{
#ifdef WIN32
    char *DEFAULT_SYSLOG_HOST = "127.0.0.1";
    int   DEFAULT_SYSLOG_PORT = 514;
    char **config_toks;
    char **host_toks;
    char  *host_string = args;
    int num_config_toks, num_host_toks;
#endif
    char **facility_toks;
    char  *facility_string = args;
    int num_facility_toks = 0;
    int i = 0;
    SyslogData *data;
    char *tmp;

    data = (SyslogData *)SnortAlloc(sizeof(SyslogData));

    /* default values for syslog output */
    data->options = 0;
    data->facility = LOG_AUTH;
    data->priority = LOG_INFO;

    if(args == NULL)
    {
        /* horrible kludge to catch default initialization */
        if(file_name != NULL)
        {
            LogMessage("%s(%d) => No arguments to alert_syslog preprocessor!\n",
                    file_name, file_line);
        }

        return data;
    }

    /*
     * NON-WIN32:  Config should be in the format:
     *   output alert_syslog: LOG_AUTH LOG_ALERT
     *
     * WIN32:  Config can be in any of these formats:
     *   output alert_syslog: LOG_AUTH LOG_ALERT
     *   output alert_syslog: host=hostname, LOG_AUTH LOG_ALERT
     *   output alert_syslog: host=hostname:port, LOG_AUTH LOG_ALERT
     */

#ifdef WIN32
    /* split the host/port part from the facilities/priorities part */
    facility_string = NULL;
    config_toks = mSplit(args, ",", 2, &num_config_toks, '\\');
    switch( num_config_toks )
    {
        case 1:  /* config consists of only facility/priority info */
            LogMessage("alert_syslog output processor is defaulting to syslog "
                    "server on %s port %d!\n",
                    DEFAULT_SYSLOG_HOST, DEFAULT_SYSLOG_PORT);
            SnortStrncpy(snort_conf->syslog_server, DEFAULT_SYSLOG_HOST, sizeof(snort_conf->syslog_server));
            snort_conf->syslog_server_port = DEFAULT_SYSLOG_PORT;
            facility_string = SnortStrdup(config_toks[0]);
            break;

        case 2:  /* config consists of host info, and facility/priority info */
            host_string     = config_toks[0];
            facility_string = SnortStrdup(config_toks[1]);
            /* split host_string into "host" vs. "server" vs. "port" */
            host_toks = mSplit(host_string, "=:", 3, &num_host_toks, 0);
            if(num_host_toks > 0 && strcmp(host_toks[0], "host") != 0 )
            {
                FatalError("%s(%d) => Badly formed alert_syslog 'host' "
                        "argument ('%s')\n",
                        file_name, file_line, host_string);
            }
            /* check for empty strings */
            if((num_host_toks >= 1 && strlen(host_toks[0]) == 0) ||
                    (num_host_toks >= 2 && strlen(host_toks[1]) == 0) ||
                    (num_host_toks >= 3 && strlen(host_toks[2]) == 0))
            {
                FatalError("%s(%d) => Badly formed alert_syslog 'host' "
                        "argument ('%s')\n",
                        file_name, file_line, host_string);
            }
            switch(num_host_toks)
            {
                case 2:  /* ie,  host=localhost (defaults to port 514) */
                    SnortStrncpy(snort_conf->syslog_server, host_toks[1], sizeof(snort_conf->syslog_server));
                    snort_conf->syslog_server_port = DEFAULT_SYSLOG_PORT;  /* default */
                    break;

                case 3:  /* ie.  host=localhost:514 */
                    SnortStrncpy(snort_conf->syslog_server, host_toks[1], sizeof(snort_conf->syslog_server));
                    snort_conf->syslog_server_port = atoi(host_toks[2]);
                    if (snort_conf->syslog_server_port == 0)
                    {
                        snort_conf->syslog_server_port = DEFAULT_SYSLOG_PORT; /*default*/
                        LogMessage("WARNING %s(%d) => alert_syslog port "
                                "appears to be non-numeric ('%s').  Defaulting "
                                "to port %d!\n", file_name, file_line,
                                host_toks[2], DEFAULT_SYSLOG_PORT);

                    }
                    break;

                default:  /* badly formed, should never occur */
                    FatalError("%s(%d) => Badly formed alert_syslog 'host' "
                            "argument ('%s')\n",
                            file_name, file_line, host_string);
            }
            mSplitFree(&host_toks, num_host_toks);
            break;

        default:
            FatalError("%s(%d) => Badly formed alert_syslog arguments ('%s')\n",
                    file_name, file_line, args);
    }

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Logging alerts to syslog "
                "server %s on port %d\n", snort_conf->syslog_server,
                snort_conf->syslog_server_port););
    mSplitFree(&config_toks, num_facility_toks);
#endif /* WIN32 */

    /* tokenize the facility/priority argument list */
    facility_toks = mSplit(facility_string, " |", 31, &num_facility_toks, '\\');

    for(i = 0; i < num_facility_toks; i++)
    {
        if(*facility_toks[i] == '$')
        {
            if((tmp = VarGet(sc, facility_toks[i]+1)) == NULL)
            {
                FatalError("%s(%d) => Undefined variable %s\n",
                        file_name, file_line, facility_toks[i]);
            }
        }
        else
        {
            tmp = facility_toks[i];
        }

        /* possible openlog options */

#ifdef LOG_CONS
        if(!strcasecmp("LOG_CONS", tmp))
        {
            data->options |= LOG_CONS;
        }
        else
#endif
#ifdef LOG_NDELAY
        if(!strcasecmp("LOG_NDELAY", tmp))
        {
            data->options |= LOG_NDELAY;
        }
        else
#endif
#ifdef LOG_PERROR
        if(!strcasecmp("LOG_PERROR", tmp))
        {
            data->options |= LOG_PERROR;
        }
        else
#endif
#ifdef LOG_PID
        if(!strcasecmp("LOG_PID", tmp))
        {
            data->options |= LOG_PID;
        }
        else
#endif
#ifdef LOG_NOWAIT
        if(!strcasecmp("LOG_NOWAIT", tmp))
        {
            data->options |= LOG_NOWAIT;
        }
        else
#endif

        /* possible openlog facilities */

#ifdef LOG_AUTHPRIV
        if(!strcasecmp("LOG_AUTHPRIV", tmp))
        {
            data->facility = LOG_AUTHPRIV;
        }
        else
#endif
#ifdef LOG_AUTH
        if(!strcasecmp("LOG_AUTH", tmp))
        {
            data->facility = LOG_AUTH;
        }
        else
#endif
#ifdef LOG_DAEMON
        if(!strcasecmp("LOG_DAEMON", tmp))
        {
            data->facility = LOG_DAEMON;
        }
        else
#endif
#ifdef LOG_LOCAL0
        if(!strcasecmp("LOG_LOCAL0", tmp))
        {
            data->facility = LOG_LOCAL0;
        }
        else
#endif
#ifdef LOG_LOCAL1
        if(!strcasecmp("LOG_LOCAL1", tmp))
        {
            data->facility = LOG_LOCAL1;
        }
        else
#endif
#ifdef LOG_LOCAL2
        if(!strcasecmp("LOG_LOCAL2", tmp))
        {
            data->facility = LOG_LOCAL2;
        }
        else
#endif
#ifdef LOG_LOCAL3
        if(!strcasecmp("LOG_LOCAL3", tmp))
        {
            data->facility = LOG_LOCAL3;
        }
        else
#endif
#ifdef LOG_LOCAL4
        if(!strcasecmp("LOG_LOCAL4", tmp))
        {
            data->facility = LOG_LOCAL4;
        }
        else
#endif
#ifdef LOG_LOCAL5
        if(!strcasecmp("LOG_LOCAL5", tmp))
        {
            data->facility = LOG_LOCAL5;
        }
        else
#endif
#ifdef LOG_LOCAL6
        if(!strcasecmp("LOG_LOCAL6", tmp))
        {
            data->facility = LOG_LOCAL6;
        }
        else
#endif
#ifdef LOG_LOCAL7
        if(!strcasecmp("LOG_LOCAL7", tmp))
        {
            data->facility = LOG_LOCAL7;
        }
        else
#endif
#ifdef LOG_USER
        if(!strcasecmp("LOG_USER", tmp))
        {
            data->facility = LOG_USER;
        }
        else
#endif

        /* possible syslog priorities */

#ifdef LOG_EMERG
        if(!strcasecmp("LOG_EMERG", tmp))
        {
            data->priority = LOG_EMERG;
        }
        else
#endif
#ifdef LOG_ALERT
        if(!strcasecmp("LOG_ALERT", tmp))
        {
            data->priority = LOG_ALERT;
        }
        else
#endif
#ifdef LOG_CRIT
        if(!strcasecmp("LOG_CRIT", tmp))
        {
            data->priority = LOG_CRIT;
        }
        else
#endif
#ifdef LOG_ERR
        if(!strcasecmp("LOG_ERR", tmp))
        {
            data->priority = LOG_ERR;
        }
        else
#endif
#ifdef LOG_WARNING
        if(!strcasecmp("LOG_WARNING", tmp))
        {
            data->priority = LOG_WARNING;
        }
        else
#endif
#ifdef LOG_NOTICE
        if(!strcasecmp("LOG_NOTICE", tmp))
        {
            data->priority = LOG_NOTICE;
        }
        else
#endif
#ifdef LOG_INFO
        if(!strcasecmp("LOG_INFO", tmp))
        {
            data->priority = LOG_INFO;
        }
        else
#endif
#ifdef LOG_DEBUG
        if(!strcasecmp("LOG_DEBUG", tmp))
        {
            data->priority = LOG_DEBUG;
        }
        else
#endif
        {
            LogMessage("WARNING: %s (%d) => Unrecognized syslog "
                    "facility/priority: %s\n",
                    file_name, file_line, tmp);
        }
    }

    mSplitFree(&facility_toks, num_facility_toks);

    /* Add facility flags to priority flags for logging to syslog */
    data->priority |= data->facility;

#ifdef WIN32
    if (facility_string != NULL)
        free(facility_string);
#endif

    return data;
}


/*
 * Function: PreprocFunction(Packet *)
 *
 * Purpose: Perform the preprocessor's intended function.  This can be
 *          simple (statistics collection) or complex (IP defragmentation)
 *          as you like.  Try not to destroy the performance of the whole
 *          system by trying to do too much....
 *
 * Arguments: p => pointer to the current packet data struct
 *
 * Returns: void function
 *
 */
static void AlertSyslog(Packet *p, const char *msg, void *arg, Event *event)
{
    SyslogData *data = (SyslogData *)arg;
    char event_string[STD_BUF];

    if (data == NULL)
        return;

    event_string[0] = '\0';

    if ((p != NULL) && IPH_IS_VALID(p))
    {
        if (event != NULL)
        {
            SnortSnprintfAppend(event_string, sizeof(event_string),
                    "[%lu:%lu:%lu] ",
                    (unsigned long)event->sig_generator,
                    (unsigned long)event->sig_id,
                    (unsigned long)event->sig_rev);
        }

        if (msg != NULL)
            SnortSnprintfAppend(event_string, sizeof(event_string), "%s ", msg);
        else
            SnortSnprintfAppend(event_string, sizeof(event_string), "ALERT ");

        if (otn_tmp != NULL)
        {
            if ((otn_tmp->sigInfo.classType != NULL)
                    && (otn_tmp->sigInfo.classType->name != NULL))
            {
                SnortSnprintfAppend(event_string, sizeof(event_string),
                        "[Classification: %s] ",
                        otn_tmp->sigInfo.classType->name);
            }

            if (otn_tmp->sigInfo.priority != 0)
            {
                SnortSnprintfAppend(event_string, sizeof(event_string),
                        "[Priority: %d] ", otn_tmp->sigInfo.priority);
            }
        }

        if (ScAlertInterface())
        {
            SnortSnprintfAppend(event_string, sizeof(event_string),
                    "<%s> ", PRINT_INTERFACE(DAQ_GetInterfaceSpec()));
        }

        if (protocol_names[GET_IPH_PROTO(p)] != NULL)
        {
            SnortSnprintfAppend(event_string, sizeof(event_string),
                    "{%s} ", protocol_names[GET_IPH_PROTO(p)]);
        }
        else
        {
            SnortSnprintfAppend(event_string, sizeof(event_string),
                    "{%d} ", GET_IPH_PROTO(p));
        }

        if (p->frag_flag
                || ((GET_IPH_PROTO(p) != IPPROTO_TCP)
                    && (GET_IPH_PROTO(p) != IPPROTO_UDP)))
        {
            char *ip_fmt = "%s -> %s";

            if (ScObfuscate())
            {
                SnortSnprintfAppend(event_string, sizeof(event_string), ip_fmt,
                        ObfuscateIpToText(GET_SRC_ADDR(p)),
                        ObfuscateIpToText(GET_DST_ADDR(p)));
            }
            else
            {
                SnortSnprintfAppend(event_string, sizeof(event_string), ip_fmt,
                        inet_ntoax(GET_SRC_ADDR(p)), inet_ntoax(GET_DST_ADDR(p)));
            }
        }
        else
        {
            char *ip_fmt = "%s:%d -> %s:%d";

            if (ScObfuscate())
            {
                SnortSnprintfAppend(event_string, sizeof(event_string), ip_fmt,
                        ObfuscateIpToText(GET_SRC_ADDR(p)), p->sp,
                        ObfuscateIpToText(GET_DST_ADDR(p)), p->dp);
            }
            else
            {
                SnortSnprintfAppend(event_string, sizeof(event_string), ip_fmt,
                        inet_ntoax(GET_SRC_ADDR(p)), p->sp,
                        inet_ntoax(GET_DST_ADDR(p)), p->dp);
            }
        }

        syslog(data->priority, "%s", event_string);
    }
    else
    {
        syslog(data->priority, "%s", msg == NULL ? "ALERT!" : msg);
    }
}

static void AlertSyslogCleanExit(int signal, void *arg)
{
    SyslogData *data = (SyslogData *)arg;
    DEBUG_WRAP(DebugMessage(DEBUG_LOG, "AlertSyslogCleanExit\n"););
    /* free memory from SyslogData */
    if(data)
        free(data);
}

