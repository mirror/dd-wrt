/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2003-2013 Sourcefire, Inc.
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

/* We use some Linux only socket capabilities */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef LINUX

#include "sf_types.h"
#include "spo_plugbase.h"
#include "plugbase.h"

#include "event.h"
#include "rules.h"
#include "treenodes.h"
#include "snort_debug.h"
#include "util.h"
#include "sfPolicy.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <stdlib.h>
#include "generators.h"
#include "snort.h"
#include "parser.h"

/* error result codes */
#define SNORT_SUCCESS 0
#define SNORT_EINVAL 1
#define SNORT_ENOENT 2
#define SNORT_ENOMEM 3

static int configured = 0;
static int connected = 0;
static int sock = -1;
static struct sockaddr_un sockAddr;

typedef struct _SnortActionRequest
{
    uint32_t event_id;
    uint32_t tv_sec;
    uint32_t generator;
    uint32_t sid;
    uint32_t src_ip;
    uint32_t dest_ip;
    uint16_t sport;
    uint16_t dport;
    uint8_t  protocol;
} SnortActionRequest;

/* For the list of GID/SIDs that are used by the
 * Finalize routine.
 */
typedef struct _AlertSFSocketGidSid
{
    uint32_t sidValue;
    uint32_t gidValue;
    struct _AlertSFSocketGidSid *next;
} AlertSFSocketGidSid;
static AlertSFSocketGidSid *sid_list = NULL;

static void AlertSFSocket_Init(struct _SnortConfig *, char *args);
static void AlertSFSocketSid_Init(struct _SnortConfig *, char *args);
void AlertSFSocketSid_InitFinalize(struct _SnortConfig *sc, int unused, void *also_unused);
void AlertSFSocket(Packet *packet, const char *msg, void *arg, Event *event);

static int AlertSFSocket_Connect(void);
static OptTreeNode *OptTreeNode_Search(uint32_t gid, uint32_t sid);
static int SignatureAddOutputFunc(uint32_t gid, uint32_t sid,
        void (*outputFunc)(Packet *, const char *, void *, Event *),
        void *args);
int String2ULong(char *string, unsigned long *result);

void AlertSFSocket_Setup(void)
{
    RegisterOutputPlugin("alert_sf_socket", OUTPUT_TYPE_FLAG__ALERT,
                         AlertSFSocket_Init);

    RegisterOutputPlugin("alert_sf_socket_sid", OUTPUT_TYPE_FLAG__ALERT,
                         AlertSFSocketSid_Init);

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Output plugin: AlertSFSocket "
                            "registered\n"););
}

/* this is defined in linux/un.h (see aldo sys/un.h) */
#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX 108
#endif

static void AlertSFSocket_Init(struct _SnortConfig *sc, char *args)
{
    /* process argument */
    char *sockname;

    if(!args)
        FatalError("AlertSFSocket: must specify a socket name\n");

    sockname = (char*)args;

    if(strlen(sockname) == 0)
        FatalError("AlertSFSocket: must specify a socket name\n");

    if(strlen(sockname) > UNIX_PATH_MAX - 1)
        FatalError("AlertSFSocket: socket name must be less than %i "
                "characters\n", UNIX_PATH_MAX - 1);

    /* create socket */
    if((sock = socket(AF_UNIX, SOCK_DGRAM, 0)) == -1)
    {
        FatalError("Unable to create socket: %s\n", strerror(errno));
    }

    memset(&sockAddr, 0, sizeof(sockAddr));
    sockAddr.sun_family = AF_UNIX;
    memcpy(sockAddr.sun_path + 1, sockname, strlen(sockname));

    if(AlertSFSocket_Connect() == 0)
        connected = 1;

    configured = 1;
}

/*
 * Parse 'sidValue' or 'gidValue:sidValue'
 */
int GidSid2UInt(char * args, uint32_t * sidValue, uint32_t * gidValue)
{
    char gbuff[80];
    char sbuff[80];
    int  i;
    unsigned long glong,slong;

    *gidValue=GENERATOR_SNORT_ENGINE;
    *sidValue=0;

    i=0;
    while( args && *args && (i < 20) )
    {
        sbuff[i]=*args;
        if( sbuff[i]==':' ) break;
        args++;
        i++;
    }
    sbuff[i]=0;

    if( i >= 20 )
    {
       return SNORT_EINVAL;
    }

    if( *args == ':' )
    {
        memcpy(gbuff,sbuff,i);
        gbuff[i]=0;

        if(String2ULong(gbuff,&glong))
        {
            return SNORT_EINVAL;
        }
        *gidValue = (uint32_t)glong;

        args++;
        i=0;
        while( args && *args && i < 20 )
        {
          sbuff[i]=*args;
          args++;
          i++;
        }
        sbuff[i]=0;

        if( i >= 20 )
        {
          return SNORT_EINVAL;
        }

        if(String2ULong(sbuff,&slong))
        {
            return SNORT_EINVAL;
        }
        *sidValue = (uint32_t)slong;
    }
    else
    {
        if(String2ULong(sbuff,&slong))
        {
            return SNORT_EINVAL;
        }
        *sidValue=(uint32_t)slong;
    }

    return SNORT_SUCCESS;
}

static void AlertSFSocketSid_Init(struct _SnortConfig *sc, char *args)
{
    uint32_t sidValue;
    uint32_t gidValue;
    AlertSFSocketGidSid *new_sid = NULL;

    /* check configured value */
    if(!configured)
        FatalError("AlertSFSocket must be configured before attaching it to a "
                "sid");

    if (GidSid2UInt((char*)args, &sidValue, &gidValue) )
        FatalError("Invalid argument '%s' to alert_sf_socket_sid\n", args);

    new_sid = (AlertSFSocketGidSid *)SnortAlloc(sizeof(AlertSFSocketGidSid));

    new_sid->sidValue = sidValue;
    new_sid->gidValue = gidValue;

    if (sid_list)
    {
        /* Add this one to the front. */
        new_sid->next = sid_list;
    }
    else
    {
        AddFuncToPostConfigList(sc, AlertSFSocketSid_InitFinalize, NULL);
    }
    sid_list = new_sid;
}

void AlertSFSocketSid_InitFinalize(struct _SnortConfig *sc, int unused, void *also_unused)
{
    AlertSFSocketGidSid *new_sid = sid_list;
    AlertSFSocketGidSid *next_sid;
    uint32_t sidValue;
    uint32_t gidValue;
    int rval = 0;

    while (new_sid)
    {
        sidValue = new_sid->sidValue;
        gidValue = new_sid->gidValue;

        rval = SignatureAddOutputFunc( (uint32_t)gidValue, (uint32_t)sidValue, AlertSFSocket, NULL );

        switch(rval)
        {
            case SNORT_SUCCESS:
                DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "SFSocket output enabled for "
                            "sid %u.\n", sidValue););
                break;
            case SNORT_EINVAL:
                DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN, "Invalid argument "
                            "attempting to attach output for sid %u.\n",
                            sidValue););
                break;
            case SNORT_ENOENT:
                LogMessage("No entry found.  SFSocket output not enabled for "
                        "sid %u.\n", sidValue);
                break;
            case SNORT_ENOMEM:
                FatalError("Out of memory");
                break;
        }

        /* Save ptr to next one in the list */
        next_sid = new_sid->next;
        /* Free the current one, not needed any more */
        free(new_sid);
        /* Reset the list */
        sid_list = new_sid = next_sid;
    }
}

static int AlertSFSocket_Connect(void)
{
    /* check sock value */
    if(sock == -1)
        FatalError("AlertSFSocket: Invalid socket\n");

    if(connect(sock, (struct sockaddr *)&sockAddr, sizeof(sockAddr)) == -1)
    {
        if(errno == ECONNREFUSED || errno == ENOENT)
        {
            LogMessage("WARNING: AlertSFSocket: Unable to connect to socket: "
                    "%s.\n", strerror(errno));
            return 1;
        }
        else
        {
            FatalError("AlertSFSocket: Unable to connect to socket "
                    "(%i): %s\n", errno, strerror(errno));
        }
    }
    return 0;
}


static SnortActionRequest sar;

void AlertSFSocket(Packet *packet, const char *msg, void *arg, Event *event)
{
    int tries = 0;

    if(!event || !packet || !IPH_IS_VALID(packet))
        return;

    // for now, only support ip4
    if ( !IS_IP4(packet) )
        return;

    /* construct the action request */
    sar.event_id = event->event_id;
    sar.tv_sec = packet->pkth->ts.tv_sec;
    sar.generator = event->sig_generator;
    sar.sid = event->sig_id;

    // when ip6 is supported:
    // * suggest TLV format where T == family, L is implied by
    //   T (and not sent), and V is just the address octets in
    //   network order
    // * if T is made the 1st octet of struct, bytes to read
    //   can be determined by reading 1 byte
    // * addresses could be moved to end of struct in uint8_t[32]
    //   and only 1st 8 used for ip4
    sar.src_ip =  ntohl(sfaddr_get_ip4_value(GET_SRC_IP(packet)));
    sar.dest_ip = ntohl(sfaddr_get_ip4_value(GET_DST_IP(packet)));
    sar.protocol = GET_IPH_PROTO(packet);

    if(sar.protocol == IPPROTO_UDP || sar.protocol == IPPROTO_TCP)
    {
        sar.sport = packet->sp;
        sar.dport = packet->dp;
    }
    else
    {
        sar.sport = 0;
        sar.dport = 0;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_PLUGIN,"AlertSFSocket fired for sid %u\n",
                            event->sig_id););

    do
    {
        tries++;
        /* connect as needed */
        if(!connected)
        {
            if(AlertSFSocket_Connect() != 0)
                break;
            connected = 1;
        }

        /* send request */
        if(send(sock, &sar, sizeof(sar), 0) == sizeof(sar))
        {
            /* success */
            return;
        }
        /* send failed */
        if(errno == ENOBUFS)
        {
            LogMessage("ERROR: AlertSFSocket: out of buffer space\n");
            break;
        }
        else if(errno == ECONNRESET)
        {
            connected = 0;
            LogMessage("WARNING: AlertSFSocket: connection reset, will attempt "
                    "to reconnect.\n");
        }
        else if(errno == ECONNREFUSED)
        {
            LogMessage("WARNING: AlertSFSocket: connection refused, "
                    "will attempt to reconnect.\n");
            connected = 0;
        }
        else if(errno == ENOTCONN)
        {
            LogMessage("WARNING: AlertSFSocket: not connected, "
                    "will attempt to reconnect.\n");
            connected = 0;
        }
        else
        {
            LogMessage("ERROR: AlertSFSocket: unhandled error '%i' in send(): "
                    "%s\n", errno, strerror(errno));
            connected = 0;
        }
    } while(tries <= 1);
    LogMessage("ERROR: AlertSFSocket: Alert not sent\n");
    return;
}

static int SignatureAddOutputFunc( uint32_t gid, uint32_t sid,
        void (*outputFunc)(Packet *, const char *, void *, Event *),
        void *args)
{
    OptTreeNode *optTreeNode = NULL;
    OutputFuncNode *outputFuncs = NULL;

    if(!outputFunc)
        return SNORT_EINVAL;  /* Invalid argument */

    if(!(optTreeNode = OptTreeNode_Search(gid,sid)))
    {
        LogMessage("Unable to find OptTreeNode for SID %u\n", sid);
        return SNORT_ENOENT;
    }

    if(!(outputFuncs = (OutputFuncNode *)calloc(1, sizeof(OutputFuncNode))))
    {
        LogMessage("Out of memory adding output function to SID %u\n", sid);
        return SNORT_ENOMEM;
    }

    outputFuncs->func = outputFunc;
    outputFuncs->arg = args;

    outputFuncs->next = optTreeNode->outputFuncs;

    optTreeNode->outputFuncs = outputFuncs;

    return SNORT_SUCCESS;
}


/* search for an OptTreeNode by sid in specific policy*/
static OptTreeNode *OptTreeNode_Search(uint32_t gid, uint32_t sid)
{
    SFGHASH_NODE *hashNode;
    OptTreeNode *otn = NULL;
    RuleTreeNode *rtn = NULL;

    if(sid == 0)
        return NULL;

    for (hashNode = sfghash_findfirst(snort_conf->otn_map);
            hashNode;
            hashNode = sfghash_findnext(snort_conf->otn_map))
    {
        otn = (OptTreeNode *)hashNode->data;
        rtn = getRuntimeRtnFromOtn(otn);
        if (rtn)
        {
            if ((rtn->proto == IPPROTO_TCP) || (rtn->proto == IPPROTO_UDP)
                    || (rtn->proto == IPPROTO_ICMP) || (rtn->proto == ETHERNET_TYPE_IP))
            {
                if (otn->sigInfo.id == sid)
                {
                    return otn;
                }
            }
        }
    }

    return NULL;
}

int String2ULong(char *string, unsigned long *result)
{
    unsigned long value;
    char *endptr;
    if(!string)
        return -1;

    value = strtoul(string, &endptr, 10);
    if(*endptr != '\0')
        return -1;

    *result = value;

    return 0;
}


#endif   /* LINUX */

