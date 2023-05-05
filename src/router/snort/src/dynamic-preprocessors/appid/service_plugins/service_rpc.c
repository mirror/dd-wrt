/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2005-2013 Sourcefire, Inc.
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


#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#include "appIdApi.h"
#include "appInfoTable.h"
#include "flow.h"
#include "service_api.h"

#include <tirpc/rpc/rpcent.h>
#if defined(FREEBSD) || defined(OPENBSD)
#include "rpc/rpc.h"
#endif

/*#define RNA_DEBUG_RPC   1 */

typedef enum
{
    RPC_STATE_CALL,
    RPC_STATE_REPLY,
    RPC_STATE_DONE
} RPCState;

typedef enum
{
    RPC_TCP_STATE_FRAG,
    RPC_TCP_STATE_HEADER,
    RPC_TCP_STATE_CRED,
    RPC_TCP_STATE_CRED_DATA,
    RPC_TCP_STATE_VERIFY,
    RPC_TCP_STATE_VERIFY_DATA,
    RPC_TCP_STATE_REPLY_HEADER,
    RPC_TCP_STATE_PARTIAL,
    RPC_TCP_STATE_DONE
} RPCTCPState;

typedef enum
{
    RPC_REPLY_BEGIN,
    RPC_REPLY_MULTI,
    RPC_REPLY_MID
} RPCReplyState;

#define min(x,y) ((x)<(y) ? (x):(y))

#define RPC_TYPE_CALL  0
#define RPC_TYPE_REPLY 1

#define RPC_PROGRAM_PORTMAP     100000
#define RPC_PORTMAP_GETPORT     3

#define RPC_REPLY_ACCEPTED 0
#define RPC_REPLY_DENIED   1

#define RPC_MAX_ACCEPTED 4
#define RPC_MAX_DENIED   5

#define RPC_TCP_FRAG_MASK   0x80000000

/* sizeof(ServiceRPCCall)+sizeof(_SERVICE_RPC_PORTMAP)==56 */
#define RPC_MAX_TCP_PACKET_SIZE  56

#pragma pack(1)

typedef struct _SERVICE_RPC_FRAG
{
    uint32_t length;
} ServiceRPCFragment;

typedef struct _SERVICE_RPC_AUTH
{
    uint32_t flavor;
    uint32_t length;
} ServiceRPCAuth;

typedef struct _SERVICE_RPC_PORTMAP
{
    uint32_t program;
    uint32_t version;
    uint32_t proto;
    uint32_t port;
} ServiceRPCPortmap;

typedef struct _SERVICE_RPC_PORTMAP_REPLY
{
    uint32_t port;
} ServiceRPCPortmapReply;

typedef struct _SERVICE_RPC_HEADER
{
    uint32_t xid;
    uint32_t type;
} ServiceRPC;

typedef struct _SERVICE_RPC_CALL_HEADER
{
    ServiceRPC header;
    uint32_t version;
    uint32_t program;
    uint32_t program_version;
    uint32_t procedure;
    ServiceRPCAuth cred;
    ServiceRPCAuth verify;
} ServiceRPCCall;

typedef struct _SERVICE_RPC_REPLY_HEADER
{
    ServiceRPC header;
    uint32_t reply_state;
    ServiceRPCAuth verify;
    uint32_t state;
} ServiceRPCReply;

#pragma pack()

typedef struct _SERVICE_RPC_DATA
{
    RPCState state;
    RPCTCPState tcpstate[APP_ID_APPID_SESSION_DIRECTION_MAX];
    RPCTCPState tcpfragstate[APP_ID_APPID_SESSION_DIRECTION_MAX];
    uint32_t program;
    uint32_t procedure;
    uint32_t xid;
    uint32_t proto;
    uint32_t tcpsize[APP_ID_APPID_SESSION_DIRECTION_MAX];
    uint32_t tcpfragpos[APP_ID_APPID_SESSION_DIRECTION_MAX];
    uint32_t tcpauthsize[APP_ID_APPID_SESSION_DIRECTION_MAX];
    uint32_t tcppos[APP_ID_APPID_SESSION_DIRECTION_MAX];
    uint8_t tcpdata[APP_ID_APPID_SESSION_DIRECTION_MAX][RPC_MAX_TCP_PACKET_SIZE];
    int once;
} ServiceRPCData;

static int rpc_init(const InitServiceAPI * const init_api);
static int rpc_validate(ServiceValidationArgs* args);
static int rpc_tcp_validate(ServiceValidationArgs* args);
static void rpc_clean(const CleanServiceAPI * const clean_api);

static tRNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &rpc_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "rpc",
    .ref_count = 1,
    .current_ref_count = 1,
};
static tRNAServiceElement tcp_svc_element =
{
    .next = NULL,
    .validate = &rpc_tcp_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "tcp rpc",
    .ref_count = 1,
    .current_ref_count = 1,
};

#define RPC_PORT_PORTMAPPER 111
#define RPC_PORT_NFS        2049
#define RPC_PORT_MOUNTD     4046
#define RPC_PORT_NLOCKMGR   4045

static RNAServiceValidationPort pp[] =
{
    {&rpc_validate, RPC_PORT_PORTMAPPER, IPPROTO_UDP},
    {&rpc_validate, RPC_PORT_PORTMAPPER, IPPROTO_UDP, 1},
    {&rpc_tcp_validate, RPC_PORT_PORTMAPPER, IPPROTO_TCP},
    {&rpc_validate, RPC_PORT_NFS, IPPROTO_UDP},
    {&rpc_validate, RPC_PORT_NFS, IPPROTO_UDP, 1},
    {&rpc_tcp_validate, RPC_PORT_NFS, IPPROTO_TCP},
    {&rpc_validate, RPC_PORT_MOUNTD, IPPROTO_UDP},
    {&rpc_validate, RPC_PORT_MOUNTD, IPPROTO_UDP, 1},
    {&rpc_tcp_validate, RPC_PORT_MOUNTD, IPPROTO_TCP},
    {&rpc_validate, RPC_PORT_NLOCKMGR, IPPROTO_UDP},
    {&rpc_validate, RPC_PORT_NLOCKMGR, IPPROTO_UDP, 1},
    {&rpc_tcp_validate, RPC_PORT_NLOCKMGR, IPPROTO_TCP},
    {NULL, 0, 0}
};

tRNAServiceValidationModule rpc_service_mod =
{
    "RPC",
    &rpc_init,
    pp,
    .clean = rpc_clean
};

typedef struct _RPC_PROGRAM
{
    struct _RPC_PROGRAM *next;
    uint32_t program;
    char *name;
} RPCProgram;

static RPCProgram *rpc_programs = NULL;

static uint8_t rpc_reply_accepted_pattern[8] = {0,0,0,1,0,0,0,0};
static uint8_t rpc_reply_denied_pattern[8] = {0,0,0,1,0,0,0,1};

static tAppRegistryEntry appIdRegistry[] =
{
    {APP_ID_SUN_RPC, APPINFO_FLAG_SERVICE_ADDITIONAL | APPINFO_FLAG_SERVICE_UDP_REVERSED}
};

static int16_t app_id = 0;

static int rpc_init(const InitServiceAPI * const init_api)
{
    struct rpcent *rpc;
    RPCProgram *prog;

#ifdef TARGET_BASED
    app_id = init_api->dpd->addProtocolReference("sunrpc");
#endif

    if (!rpc_programs)
    {
        while ((rpc = getrpcent()))
        {
            if (rpc->r_name)
            {
                prog = calloc(1, sizeof(RPCProgram));
                if (prog)
                {
                    prog->program = rpc->r_number;
                    prog->next = rpc_programs;
                    rpc_programs = prog;
                    prog->name = strdup(rpc->r_name);
                    if (!prog->name)
                        _dpd.errMsg("failed to allocate rpc program name");
                }
            }
        }
        endrpcent();
    }

    init_api->RegisterPattern(&rpc_tcp_validate, IPPROTO_TCP, rpc_reply_accepted_pattern,
                              sizeof(rpc_reply_accepted_pattern), 8, "rpc", init_api->pAppidConfig);
    init_api->RegisterPattern(&rpc_tcp_validate, IPPROTO_TCP, rpc_reply_denied_pattern,
                              sizeof(rpc_reply_denied_pattern), 8, "rpc", init_api->pAppidConfig);
    init_api->RegisterPattern(&rpc_validate, IPPROTO_UDP, rpc_reply_accepted_pattern,
                              sizeof(rpc_reply_accepted_pattern), 4, "rpc", init_api->pAppidConfig);
    init_api->RegisterPattern(&rpc_validate, IPPROTO_UDP, rpc_reply_denied_pattern,
                              sizeof(rpc_reply_denied_pattern), 4, "rpc", init_api->pAppidConfig);

	unsigned i;
	for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
		init_api->RegisterAppId(&rpc_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, init_api->pAppidConfig);
	}

    return 0;
}

static const RPCProgram *FindRPCProgram(uint32_t program)
{
    RPCProgram *rpc;

    for (rpc=rpc_programs; rpc; rpc=rpc->next)
    {
        if (program == rpc->program) break;
    }
    return rpc;
}

static int validate_packet(const uint8_t *data, uint16_t size, int dir,
                           tAppIdData *flowp, SFSnortPacket *pkt, ServiceRPCData *rd,
                           const char * *pname, uint32_t *program)
{
    const ServiceRPCCall *call;
    const ServiceRPCReply *reply;
    const ServiceRPC *rpc;
    const ServiceRPCPortmap *pm;
    const ServiceRPCAuth *a;
    const ServiceRPCPortmapReply *pmr;
    uint32_t tmp;
    uint32_t val;
    const uint8_t *end;
    tAppIdData *pf;
    const RPCProgram *rprog;

    if (!size) return SERVICE_INPROCESS;

    end = data + size;

    if (flowp->proto == IPPROTO_UDP)
    {
        if (!rd->once)
        {
            rd->once = 1;
            if (size < sizeof(ServiceRPC)) return SERVICE_NOMATCH;
            rpc = (ServiceRPC *)data;
            if (ntohl(rpc->type) == RPC_TYPE_REPLY)
            {
                setAppIdFlag(flowp, APPID_SESSION_UDP_REVERSED);
                rd->state = RPC_STATE_REPLY;
                dir = APP_ID_FROM_RESPONDER;
            }
        }
        else if (getAppIdFlag(flowp, APPID_SESSION_UDP_REVERSED))
        {
            dir = (dir == APP_ID_FROM_RESPONDER) ? APP_ID_FROM_INITIATOR:APP_ID_FROM_RESPONDER;
        }
    }

    switch (rd->state)
    {
    case RPC_STATE_CALL:
        if (dir != APP_ID_FROM_INITIATOR) return SERVICE_INPROCESS;
        rd->state = RPC_STATE_DONE;
        if (size < sizeof(ServiceRPCCall)) return SERVICE_NOT_COMPATIBLE;
        call = (ServiceRPCCall *)data;
        if (ntohl(call->header.type) != RPC_TYPE_CALL) return SERVICE_NOT_COMPATIBLE;
        if (ntohl(call->version) != 2) return SERVICE_NOT_COMPATIBLE;
        rd->program = ntohl(call->program);
        rd->procedure = ntohl(call->procedure);
        tmp = ntohl(call->cred.length);
        if (sizeof(ServiceRPCCall)+tmp > size) return SERVICE_NOT_COMPATIBLE;
        data += (sizeof(ServiceRPCCall) - sizeof(ServiceRPCAuth)) + tmp;
        a = (ServiceRPCAuth *)data;
        tmp = ntohl(a->length);
        if (tmp+sizeof(ServiceRPCAuth) > (unsigned)(end-data)) return SERVICE_NOT_COMPATIBLE;
        data += sizeof(ServiceRPCAuth) + tmp;
        if (rd->program >= 0x60000000) return SERVICE_NOT_COMPATIBLE;
        switch (rd->program)
        {
        case RPC_PROGRAM_PORTMAP:
            switch (rd->procedure)
            {
            case RPC_PORTMAP_GETPORT:
                if (end-data < (int)sizeof(ServiceRPCPortmap)) return SERVICE_NOT_COMPATIBLE;
                pm = (ServiceRPCPortmap *)data;
                rd->proto = pm->proto;
                break;
            default:
                break;
            }
            break;
        default:
            break;
        }
        rd->xid = call->header.xid;
        rd->state = RPC_STATE_REPLY;
        break;
    case RPC_STATE_REPLY:
        if (dir != APP_ID_FROM_RESPONDER) return SERVICE_INPROCESS;
        rd->state = RPC_STATE_DONE;
        if (size < sizeof(ServiceRPCReply)) return SERVICE_NOMATCH;
        reply = (ServiceRPCReply *)data;
        if (ntohl(reply->header.type) != RPC_TYPE_REPLY) return SERVICE_NOMATCH;
        if (rd->xid != reply->header.xid && rd->xid != 0xFFFFFFFF) return SERVICE_NOMATCH;
        tmp = ntohl(reply->verify.length);
        if (sizeof(ServiceRPCReply)+tmp > size)
            return SERVICE_NOMATCH;
        data += sizeof(ServiceRPCReply) + tmp;
        tmp = ntohl(reply->reply_state);
        val = ntohl(reply->state);
        if (tmp == RPC_REPLY_ACCEPTED)
        {
            if (val > RPC_MAX_ACCEPTED) return SERVICE_NOMATCH;
            if (rd->xid == 0xFFFFFFFF && reply->header.xid != 0xFFFFFFFF)
            {
                rd->state = RPC_STATE_CALL;
                return SERVICE_INPROCESS;
            }
            *program = rd->program;
            switch (rd->program)
            {
            case RPC_PROGRAM_PORTMAP:
                switch (rd->procedure)
                {
                case RPC_PORTMAP_GETPORT:
                    if (end-data < (int)sizeof(ServiceRPCPortmapReply))
                        return SERVICE_NOMATCH;
                    pmr = (ServiceRPCPortmapReply *)data;
                    if (pmr->port)
                    {
                        sfaddr_t *sip;
                        sfaddr_t *dip;

                        dip = GET_DST_IP(pkt);
                        sip = GET_SRC_IP(pkt);
                        tmp = ntohl(pmr->port);
#ifdef TARGET_BASED
                        pf = rpc_service_mod.api->flow_new(flowp, pkt, dip, 0, sip, (uint16_t)tmp, (uint8_t)ntohl(rd->proto), app_id, 0);
                        if (pf)
                        {
                            rpc_service_mod.api->data_add_id(pf, (uint16_t)tmp,
                                                             flowp->proto==IPPROTO_TCP ? &tcp_svc_element:&svc_element);
                            pf->rnaServiceState = RNA_STATE_STATEFUL;
                            setAppIdFlag(pf,
                                         getAppIdFlag(flowp,
                                                      APPID_SESSION_RESPONDER_MONITORED |
                                                      APPID_SESSION_INITIATOR_MONITORED |
                                                      APPID_SESSION_SPECIAL_MONITORED |
                                                      APPID_SESSION_RESPONDER_CHECKED |
                                                      APPID_SESSION_INITIATOR_CHECKED |
                                                      APPID_SESSION_DISCOVER_APP |
                                                      APPID_SESSION_DISCOVER_USER));
                        }
#endif
                    }
                    break;
                default:
                    break;
                }
                *pname = "portmap";
                break;
            default:
                rprog = FindRPCProgram(rd->program);
                if (rprog && rprog->name) *pname = rprog->name;
                break;
            }
        }
        else if (tmp == RPC_REPLY_DENIED)
        {
            if (val > RPC_MAX_DENIED) return SERVICE_NOMATCH;
        }
        else return SERVICE_NOMATCH;
        rd->state = RPC_STATE_CALL;
        return SERVICE_SUCCESS;
    default:
        return SERVICE_NOMATCH;
    }
    return SERVICE_INPROCESS;
}

static int rpc_validate(ServiceValidationArgs* args)
{
    static char subname[64];
    ServiceRPCData *rd;
    RNAServiceSubtype sub;
    RNAServiceSubtype *subtype;
    uint32_t program = 0;
    const char *pname = NULL;
    int rval;
    tAppIdData *flowp = args->flowp;
    const uint8_t *data = args->data;
    SFSnortPacket *pkt = args->pkt; 
    const int dir = args->dir;
    uint16_t size = args->size;

    if (!size)
    {
        rval = SERVICE_INPROCESS;
        goto done;
    }

    rd = rpc_service_mod.api->data_get(flowp, rpc_service_mod.flow_data_index);
    if (!rd)
    {
        rd = calloc(1, sizeof(*rd));
        if (!rd)
            return SERVICE_ENOMEM;
        if (rpc_service_mod.api->data_add(flowp, rd, rpc_service_mod.flow_data_index, &free))
        {
            free(rd);
            return SERVICE_ENOMEM;
        }
        rd->state = (dir == APP_ID_FROM_INITIATOR) ? RPC_STATE_CALL:RPC_STATE_REPLY;
        rd->xid = 0xFFFFFFFF;
    }

#ifdef RNA_DEBUG_RPC
    fprintf(SF_DEBUG_FILE, "Begin %u -> %u %u %d state %d\n", pkt->src_port, pkt->dst_port, flowp->proto, dir, rd->state);
#endif

    rval = validate_packet(data, size, dir, flowp, pkt, rd, &pname, &program);

#ifdef RNA_DEBUG_RPC
    fprintf(SF_DEBUG_FILE, "End %u -> %u %u %d state %d rval %d\n", pkt->src_port, pkt->dst_port, flowp->proto, dir, rd->state, rval);
#endif

done:
    switch (rval)
    {
    case SERVICE_INPROCESS:
        if (!getAppIdFlag(flowp, APPID_SESSION_SERVICE_DETECTED))
        {
            rpc_service_mod.api->service_inprocess(flowp, pkt, dir, &svc_element, NULL);
        }
        return SERVICE_INPROCESS;

    case SERVICE_SUCCESS:
        if (!getAppIdFlag(flowp, APPID_SESSION_SERVICE_DETECTED))
        {
            if (pname && *pname)
            {
                memset(&sub, 0, sizeof(sub));
                sub.service = pname;
                subtype = &sub;
            }
            else if (program)
            {
                snprintf(subname, sizeof(subname), "(%u)", program);
                memset(&sub, 0, sizeof(sub));
                sub.service = subname;
                subtype = &sub;
            }
            else subtype = NULL;
            rpc_service_mod.api->add_service(flowp, pkt, dir, &svc_element,
                                             APP_ID_SUN_RPC, NULL, NULL, subtype, NULL);
        }
        setAppIdFlag(flowp, APPID_SESSION_CONTINUE);
        return SERVICE_SUCCESS;

    case SERVICE_NOT_COMPATIBLE:
        if (!getAppIdFlag(flowp, APPID_SESSION_SERVICE_DETECTED))
        {
            rpc_service_mod.api->incompatible_data(flowp, pkt, dir, &svc_element,
                                                   rpc_service_mod.flow_data_index,
                                                   args->pConfig, NULL);
        }
        clearAppIdFlag(flowp, APPID_SESSION_CONTINUE);
        return SERVICE_NOT_COMPATIBLE;

    case SERVICE_NOMATCH:
        if (!getAppIdFlag(flowp, APPID_SESSION_SERVICE_DETECTED))
        {
            rpc_service_mod.api->fail_service(flowp, pkt, dir, &svc_element,
                                              rpc_service_mod.flow_data_index,
                                              args->pConfig, NULL);
        }
        clearAppIdFlag(flowp, APPID_SESSION_CONTINUE);
        return SERVICE_NOMATCH;
    default:
        return rval;
    }
}

static int rpc_tcp_validate(ServiceValidationArgs* args)
{
    ServiceRPCData *rd;
    const ServiceRPCFragment *frag;
    uint32_t length;
    uint32_t fragsize;
    int ret;
    int retval = -1;
    ServiceRPCCall *call;
    ServiceRPCReply *reply;

    static char subname[64];
    RNAServiceSubtype sub;
    RNAServiceSubtype *subtype;
    uint32_t program = 0;
    const char *pname = NULL;

    tAppIdData *flowp = args->flowp;
    const uint8_t *data = args->data;
    SFSnortPacket *pkt = args->pkt; 
    const int dir = args->dir;
    uint16_t size = args->size;

    if (!size)
        goto inprocess;

    rd = rpc_service_mod.api->data_get(flowp, rpc_service_mod.flow_data_index);
    if (!rd)
    {
        rd = calloc(1, sizeof(*rd));
        if (!rd)
            return SERVICE_ENOMEM;
        if (rpc_service_mod.api->data_add(flowp, rd, rpc_service_mod.flow_data_index, &free))
        {
            free(rd);
            return SERVICE_ENOMEM;
        }
        rd->state = RPC_STATE_CALL;
        for (ret=0; ret<APP_ID_APPID_SESSION_DIRECTION_MAX; ret++)
        {
            rd->tcpstate[ret] = RPC_TCP_STATE_FRAG;
            rd->tcpfragstate[ret] = RPC_TCP_STATE_HEADER;
        }
    }

    while (size)
    {
        fragsize = min(size, (rd->tcpsize[dir] & ~RPC_TCP_FRAG_MASK) -
                                rd->tcpfragpos[dir]);

        switch (rd->tcpstate[dir])
        {
        case RPC_TCP_STATE_FRAG:
            if (size < sizeof(ServiceRPCFragment)) goto bail;
            frag = (ServiceRPCFragment *)data;
            data += sizeof(ServiceRPCFragment);
            size -= sizeof(ServiceRPCFragment);

            rd->tcpsize[dir] = ntohl(frag->length);
            rd->tcpfragpos[dir] = 0;
            rd->tcpstate[dir] = rd->tcpfragstate[dir];
            break;
        case RPC_TCP_STATE_HEADER:
            if (dir == APP_ID_FROM_INITIATOR)
            {
                length = min(fragsize, offsetof(ServiceRPCCall, cred) -
                                        rd->tcppos[dir]);
                memcpy(&rd->tcpdata[dir][rd->tcppos[dir]], data, length);
                rd->tcppos[dir] += length;
                rd->tcpfragpos[dir] += length;
                data += length;
                size -= length;
                if (rd->tcppos[dir] >= offsetof(ServiceRPCCall, cred))
                {
                    call = (ServiceRPCCall *)rd->tcpdata[dir];
                    if (ntohl(call->header.type) != RPC_TYPE_CALL) goto bail;
                    if (ntohl(call->version) != 2) goto bail;
                    rd->tcpstate[dir] = RPC_TCP_STATE_CRED;
                    rd->tcppos[dir] = 0;
                }
            }
            else
            {
                length = min(fragsize, offsetof(ServiceRPCReply, verify) -
                                        rd->tcppos[dir]);
                memcpy(&rd->tcpdata[dir][rd->tcppos[dir]], data, length);
                rd->tcppos[dir] += length;
                rd->tcpfragpos[dir] += length;
                data += length;
                size -= length;
                if (rd->tcppos[dir] >= offsetof(ServiceRPCReply, verify))
                {
                    reply = (ServiceRPCReply *)rd->tcpdata[dir];
                    if (ntohl(reply->header.type) != RPC_TYPE_REPLY) goto fail;
                    rd->tcpstate[dir] = RPC_TCP_STATE_VERIFY;
                    rd->tcppos[dir] = 0;
                }
            }
            break;
        case RPC_TCP_STATE_CRED:
            if (dir != APP_ID_FROM_INITIATOR)
                goto bail;
            length = min(fragsize, sizeof(ServiceRPCAuth) - rd->tcppos[dir]);
            memcpy(&rd->tcpdata[dir][offsetof(ServiceRPCCall, cred)+rd->tcppos[dir]],
                   data, length);
            rd->tcppos[dir] += length;
            rd->tcpfragpos[dir] += length;
            data += length;
            size -= length;
            if (rd->tcppos[dir] >= sizeof(ServiceRPCAuth))
            {
                call = (ServiceRPCCall *)rd->tcpdata[dir];
                length = ntohl(call->cred.length);
                if (length > (rd->tcpsize[dir] & ~RPC_TCP_FRAG_MASK) ||
                    rd->tcpfragpos[dir]+length > (rd->tcpsize[dir] & ~RPC_TCP_FRAG_MASK))
                    goto bail;
                rd->tcpauthsize[dir] = length;
                rd->tcpstate[dir] = RPC_TCP_STATE_CRED_DATA;
                rd->tcppos[dir] = 0;
            }
            break;
        case RPC_TCP_STATE_CRED_DATA:
            if (dir != APP_ID_FROM_INITIATOR)
                goto bail;
            length = min(fragsize, rd->tcpauthsize[dir] - rd->tcppos[dir]);
            rd->tcppos[dir] += length;
            rd->tcpfragpos[dir] += length;
            data += length;
            size -= length;
            if (rd->tcppos[dir] >= rd->tcpauthsize[dir])
            {
                call = (ServiceRPCCall *)rd->tcpdata[dir];
                call->cred.flavor = 0;
                call->cred.length = 0;
                rd->tcpstate[dir] = RPC_TCP_STATE_VERIFY;
                rd->tcppos[dir] = 0;
            }
            break;
        case RPC_TCP_STATE_VERIFY:
            length = min(fragsize, sizeof(ServiceRPCAuth) - rd->tcppos[dir]);
            if (dir == APP_ID_FROM_INITIATOR)
                memcpy(&rd->tcpdata[dir][offsetof(ServiceRPCCall, verify)+rd->tcppos[dir]],
                       data, length);
            else
                memcpy(&rd->tcpdata[dir][offsetof(ServiceRPCReply, verify)+rd->tcppos[dir]],
                       data, length);
            rd->tcppos[dir] += length;
            rd->tcpfragpos[dir] += length;
            data += length;
            size -= length;
            fragsize -= length;
            if (rd->tcppos[dir] >= sizeof(ServiceRPCAuth))
            {
                if (dir == APP_ID_FROM_INITIATOR)
                {
                    call = (ServiceRPCCall *)rd->tcpdata[dir];
                    length = ntohl(call->verify.length);
                }
                else
                {
                    reply = (ServiceRPCReply *)rd->tcpdata[dir];
                    length = ntohl(reply->verify.length);
                }
                if (length > (rd->tcpsize[dir] & ~RPC_TCP_FRAG_MASK) ||
                    rd->tcpfragpos[dir]+length > (rd->tcpsize[dir] & ~RPC_TCP_FRAG_MASK))
                    goto bail;
                rd->tcpauthsize[dir] = length;
                rd->tcpstate[dir] = RPC_TCP_STATE_VERIFY_DATA;
                rd->tcppos[dir] = 0;
            }
            else
            {
                break;
            }
        case RPC_TCP_STATE_VERIFY_DATA:
            length = min(fragsize, rd->tcpauthsize[dir] - rd->tcppos[dir]);
            rd->tcppos[dir] += length;
            rd->tcpfragpos[dir] += length;
            data += length;
            size -= length;
            if (rd->tcppos[dir] >= rd->tcpauthsize[dir])
            {
                if (dir == APP_ID_FROM_INITIATOR)
                {
                    call = (ServiceRPCCall *)rd->tcpdata[dir];
                    call->verify.flavor = 0;
                    call->verify.length = 0;
                    rd->tcpstate[dir] = RPC_TCP_STATE_PARTIAL;
                    rd->tcppos[dir] = sizeof(ServiceRPCCall);
                    if (rd->tcpfragpos[dir] >= (rd->tcpsize[dir] & ~RPC_TCP_FRAG_MASK))
                    {
                        if (rd->tcpsize[dir] & RPC_TCP_FRAG_MASK)
                        {

#ifdef RNA_DEBUG_RPC
                            fprintf(SF_DEBUG_FILE, "V Begin %u -> %u %u %d state %d\n", pkt->src_port, pkt->dst_port, flowp->proto, dir, rd->state);
#endif

                            ret = validate_packet(rd->tcpdata[dir], rd->tcppos[dir], dir, flowp, pkt,
                                                  rd, &pname, &program);

#ifdef RNA_DEBUG_RPC
                            fprintf(SF_DEBUG_FILE, "V End %u -> %u %u %d state %d rval %d\n", pkt->src_port, pkt->dst_port, flowp->proto, dir, rd->state, ret);
#endif

                            if (retval == -1) retval = ret;
                            rd->tcpfragstate[dir] = RPC_TCP_STATE_HEADER;
                            rd->tcppos[dir] = 0;
                        }
                        else rd->tcpfragstate[dir] = rd->tcpstate[dir];
                        rd->tcpstate[dir] = RPC_TCP_STATE_FRAG;
                    }
                }
                else
                {
                    reply = (ServiceRPCReply *)rd->tcpdata[dir];
                    reply->verify.flavor = 0;
                    reply->verify.length = 0;
                    rd->tcpstate[dir] = RPC_TCP_STATE_REPLY_HEADER;
                    if (rd->tcpfragpos[dir]+sizeof(uint32_t) > (rd->tcpsize[dir] & ~RPC_TCP_FRAG_MASK))
                        goto bail;
                    rd->tcppos[dir] = 0;
                }
            }
            break;
        case RPC_TCP_STATE_REPLY_HEADER:
            if (dir != APP_ID_FROM_RESPONDER) goto bail;
            length = min(fragsize, sizeof(uint32_t) - rd->tcppos[dir]);
            memcpy(&rd->tcpdata[dir][offsetof(ServiceRPCReply, state)+rd->tcppos[dir]],
                   data, length);
            rd->tcppos[dir] += length;
            rd->tcpfragpos[dir] += length;
            data += length;
            size -= length;

            if (rd->tcppos[dir] >= sizeof(uint32_t))
            {
                rd->tcpstate[dir] = RPC_TCP_STATE_PARTIAL;
                rd->tcppos[dir] = sizeof(ServiceRPCReply);
            }
            if (rd->tcpfragpos[dir] >= (rd->tcpsize[dir] & ~RPC_TCP_FRAG_MASK))
            {
                fragsize = 0;
            }
            else
            {
                break;
            }
        case RPC_TCP_STATE_PARTIAL:
            if (rd->tcppos[dir] < RPC_MAX_TCP_PACKET_SIZE && fragsize)
            {
                length = min(fragsize, RPC_MAX_TCP_PACKET_SIZE - rd->tcppos[dir]);
                memcpy(&rd->tcpdata[dir][rd->tcppos[dir]], data, length);
                rd->tcppos[dir] += length;
            }
            else
            {
                length = fragsize;
            }
            rd->tcpfragpos[dir] += length;
            data += length;
            size -= length;
            if (rd->tcpfragpos[dir] >= (rd->tcpsize[dir] & ~RPC_TCP_FRAG_MASK))
            {
                if (rd->tcpsize[dir] & RPC_TCP_FRAG_MASK)
                {

#ifdef RNA_DEBUG_RPC
                    fprintf(SF_DEBUG_FILE, "P Begin %u -> %u %u %d state %d\n", pkt->src_port, pkt->dst_port, flowp->proto, dir, rd->state);
#endif

                    ret = validate_packet(rd->tcpdata[dir], rd->tcppos[dir], dir, flowp, pkt,
                                          rd, &pname, &program);

#ifdef RNA_DEBUG_RPC
                    fprintf(SF_DEBUG_FILE, "P End %u -> %u %u %d state %d rval %d\n", pkt->src_port, pkt->dst_port, flowp->proto, dir, rd->state, ret);
#endif

                    if (retval == -1) retval = ret;
                    rd->tcpfragstate[dir] = RPC_TCP_STATE_HEADER;
                    rd->tcppos[dir] = 0;
                }
                else rd->tcpfragstate[dir] = rd->tcpstate[dir];
                rd->tcpstate[dir] = RPC_TCP_STATE_FRAG;
            }
            break;
        default:
            if (retval == -1) goto fail;
            else
            {
                clearAppIdFlag(flowp, APPID_SESSION_CONTINUE);
                goto done;
            }
        }
        if (rd->tcpstate[dir] != RPC_TCP_STATE_FRAG &&
            rd->tcpstate[dir] != RPC_TCP_STATE_PARTIAL &&
            rd->tcpfragpos[dir] >= (rd->tcpsize[dir] & ~RPC_TCP_FRAG_MASK))
        {
            if (rd->tcpsize[dir] & RPC_TCP_FRAG_MASK) goto bail;
            rd->tcpfragstate[dir] = rd->tcpstate[dir];
            rd->tcpstate[dir] = RPC_TCP_STATE_FRAG;
        }
    }
    if (retval == -1) retval = SERVICE_INPROCESS;

done:
    switch (retval)
    {
    case SERVICE_INPROCESS:
inprocess:
        if (!getAppIdFlag(flowp, APPID_SESSION_SERVICE_DETECTED))
        {
            rpc_service_mod.api->service_inprocess(flowp, pkt, dir, &tcp_svc_element, NULL);
        }
        return SERVICE_INPROCESS;

    case SERVICE_SUCCESS:
        if (!getAppIdFlag(flowp, APPID_SESSION_SERVICE_DETECTED))
        {
            if (pname && *pname)
            {
                memset(&sub, 0, sizeof(sub));
                sub.service = pname;
                subtype = &sub;
            }
            else if (program)
            {
                sprintf(subname, "(%u)", program);
                memset(&sub, 0, sizeof(sub));
                sub.service = subname;
                subtype = &sub;
            }
            else subtype = NULL;
            rpc_service_mod.api->add_service(flowp, pkt, dir, &tcp_svc_element,
                                             APP_ID_SUN_RPC, NULL, NULL, subtype, NULL);
        }
        setAppIdFlag(flowp, APPID_SESSION_CONTINUE);
        return SERVICE_SUCCESS;

    case SERVICE_NOT_COMPATIBLE:
        if (!getAppIdFlag(flowp, APPID_SESSION_SERVICE_DETECTED))
        {
            rpc_service_mod.api->incompatible_data(flowp, pkt, dir, &tcp_svc_element,
                                                   rpc_service_mod.flow_data_index,
                                                   args->pConfig, NULL);
        }
        clearAppIdFlag(flowp, APPID_SESSION_CONTINUE);
        return SERVICE_NOT_COMPATIBLE;

    case SERVICE_NOMATCH:
fail:
        if (!getAppIdFlag(flowp, APPID_SESSION_SERVICE_DETECTED))
        {
            rpc_service_mod.api->fail_service(flowp, pkt, dir, &tcp_svc_element,
                                              rpc_service_mod.flow_data_index,
                                              args->pConfig, NULL);
        }
        clearAppIdFlag(flowp, APPID_SESSION_CONTINUE);
        return SERVICE_NOMATCH;
    default:
        return retval;
    }

bail:
    clearAppIdFlag(flowp, APPID_SESSION_CONTINUE);
    rd->tcpstate[APP_ID_FROM_INITIATOR] = RPC_TCP_STATE_DONE;
    rd->tcpstate[APP_ID_FROM_RESPONDER] = RPC_TCP_STATE_DONE;
    if (dir == APP_ID_FROM_INITIATOR)
    {
        if (retval == -1) retval = SERVICE_NOT_COMPATIBLE;
    }
    else
    {
        if (retval == -1) retval = SERVICE_NOMATCH;
    }
    goto done;
}

static void rpc_clean(const CleanServiceAPI * const clean_api)
{
    RPCProgram *prog = NULL;

    while(rpc_programs)
    {
        prog = rpc_programs;

        rpc_programs = rpc_programs->next;

        if(prog->name)
            free(prog->name);

        free(prog);
    }
}
