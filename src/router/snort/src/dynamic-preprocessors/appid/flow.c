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

#include <stdlib.h>
#include <stdint.h>

#include "common_util.h"
#include "flow.h"
#include "service_api.h"
#include "fw_appid.h"

static AppIdFlowData *fd_free_list;

void AppIdFlowdataFree(tAppIdData *flowp)
{
    AppIdFlowData *tmp_fd;

    while ((tmp_fd = flowp->flowData))
    {
        flowp->flowData = tmp_fd->next;
        if (tmp_fd->fd_data && tmp_fd->fd_free)
            tmp_fd->fd_free(tmp_fd->fd_data);
        tmp_fd->next = fd_free_list;
        fd_free_list = tmp_fd;
        app_id_flow_data_free_list_count++;
    }
}
void AppIdFlowdataFini()
{
    AppIdFlowData *tmp_fd;

    while ((tmp_fd = fd_free_list))
    {
        fd_free_list = fd_free_list->next;
        app_id_flow_data_free_list_count--;
        _dpd.snortFree(tmp_fd, sizeof(*tmp_fd), PP_APP_ID, PP_MEM_CATEGORY_SESSION);
    }
}

void *AppIdFlowdataGet(tAppIdData *flowp, unsigned id)
{
    AppIdFlowData *tmp_fd;

    for (tmp_fd = flowp->flowData; tmp_fd && tmp_fd->fd_id != id; tmp_fd = tmp_fd->next);
    return tmp_fd ? tmp_fd->fd_data : NULL;
}

void *AppIdFlowdataRemove(tAppIdData *flowp, unsigned id)
{
    AppIdFlowData **pfd;
    AppIdFlowData *fd;

    for (pfd = &flowp->flowData; *pfd && (*pfd)->fd_id != id; pfd = &(*pfd)->next);
    if ((fd = *pfd))
    {
        *pfd = fd->next;
        fd->next = fd_free_list;
        fd_free_list = fd;
        app_id_flow_data_free_list_count++;
        return fd->fd_data;
    }
    return NULL;
}

void AppIdFlowdataDelete(tAppIdData *flowp, unsigned id)
{
    AppIdFlowData **pfd;
    AppIdFlowData *fd;

    for (pfd = &flowp->flowData; *pfd && (*pfd)->fd_id != id; pfd = &(*pfd)->next);
    if ((fd = *pfd))
    {
        *pfd = fd->next;
        if (fd->fd_data && fd->fd_free)
            fd->fd_free(fd->fd_data);
        fd->next = fd_free_list;
        fd_free_list = fd;
        app_id_flow_data_free_list_count++;
    }
}

void AppIdFlowdataDeleteAllByMask(tAppIdData *flowp, unsigned mask)
{
    AppIdFlowData **pfd;
    AppIdFlowData *fd;

    pfd = &flowp->flowData;
    while (*pfd)
    {
        if ((*pfd)->fd_id & mask)
        {
            fd = *pfd;
            *pfd = fd->next;
            if (fd->fd_data && fd->fd_free)
                fd->fd_free(fd->fd_data);
            fd->next = fd_free_list;
            fd_free_list = fd;
            app_id_flow_data_free_list_count++;
        }
        else
        {
            pfd = &(*pfd)->next;
        }
    }
}

int AppIdFlowdataAdd(tAppIdData *flowp, void *data, unsigned id, AppIdFreeFCN fcn)
{
    AppIdFlowData *tmp_fd;

    if (fd_free_list)
    {
        tmp_fd = fd_free_list;
        fd_free_list = tmp_fd->next;
        app_id_flow_data_free_list_count--;
    }
    else if (!(tmp_fd = _dpd.snortAlloc(1, sizeof(*tmp_fd), PP_APP_ID, PP_MEM_CATEGORY_SESSION)))
        return -1;
    tmp_fd->fd_id = id;
    tmp_fd->fd_data = data;
    tmp_fd->fd_free = fcn;
    tmp_fd->next = flowp->flowData;
    flowp->flowData = tmp_fd;
    return 0;
}

int AppIdFlowdataAddId(tAppIdData *flowp, uint16_t port, const tRNAServiceElement *svc_element)
{
    if (flowp->serviceData)
        return -1;
    flowp->serviceData = svc_element;
    flowp->service_port = port;
    return 0;
}

#ifdef RNA_DEBUG_EXPECTED_FLOWS
static void flowAppSharedDataDelete(tAppIdData *sharedData)
{
    _dpd.errMsg("Deleting %p\n",sharedData);
    appSharedDataDelete(sharedData);
}
#endif

tAppIdData *AppIdEarlySessionCreate(tAppIdData *flowp, SFSnortPacket *ctrlPkt, sfaddr_t *cliIp, uint16_t cliPort,
                          sfaddr_t *srvIp, uint16_t srvPort, uint8_t proto, int16_t app_id, int flags)
{
    char src_ip[INET6_ADDRSTRLEN];
    char dst_ip[INET6_ADDRSTRLEN];
    struct _ExpectNode** node;
    tAppIdData *data;

    if (app_id_debug_session_flag)
    {
        inet_ntop(sfaddr_family(cliIp), (void *)sfaddr_get_ptr(cliIp), src_ip, sizeof(src_ip));
        inet_ntop(sfaddr_family(srvIp), (void *)sfaddr_get_ptr(srvIp), dst_ip, sizeof(dst_ip));
    }

    data = appSharedDataAlloc(proto, (struct in6_addr*)sfaddr_get_ip6_ptr(cliIp), 0);
    if (data)
        data->common.policyId = appIdPolicyId;

    node = (flags & APPID_EARLY_SESSION_FLAG_FW_RULE) ? &ctrlPkt->expectedSession : NULL;

    if (_dpd.sessionAPI->set_application_protocol_id_expected(ctrlPkt, cliIp, cliPort, srvIp, srvPort,
                                                              proto, app_id, PP_APP_ID, data,
#ifdef RNA_DEBUG_EXPECTED_FLOWS
                                                              (void (*)(void *))flowAppSharedDataDelete
#else
                                                              (void (*)(void *))appSharedDataDelete
#endif
                                                              , node) )
    {
        if (app_id_debug_session_flag)
            _dpd.logMsg("AppIdDbg %s failed to create a related flow for %s-%u -> %s-%u %u\n", app_id_debug_session,
                        src_ip, (unsigned)cliPort, dst_ip, (unsigned)srvPort, (unsigned)proto);
        appSharedDataDelete(data);
        return NULL;
    }
    else if (app_id_debug_session_flag)
        _dpd.logMsg("AppIdDbg %s created a related flow for %s-%u -> %s-%u %u\n", app_id_debug_session,
                    src_ip, (unsigned)cliPort, dst_ip, (unsigned)srvPort, (unsigned)proto);

    return data;
}

