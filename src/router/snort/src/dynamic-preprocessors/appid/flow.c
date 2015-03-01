/*
** Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
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
    }
}
void AppIdFlowdataFini()
{
    AppIdFlowData *tmp_fd;

    while ((tmp_fd = fd_free_list))
    {
        fd_free_list = fd_free_list->next;
        free(tmp_fd);
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
    }
}

int AppIdFlowdataAdd(tAppIdData *flowp, void *data, unsigned id, AppIdFreeFCN fcn)
{
    AppIdFlowData *tmp_fd;

    if (fd_free_list)
    {
        tmp_fd = fd_free_list;
        fd_free_list = tmp_fd->next;
    }
    else if (!(tmp_fd = malloc(sizeof(*tmp_fd))))
        return -1;
    tmp_fd->fd_id = id;
    tmp_fd->fd_data = data;
    tmp_fd->fd_free = fcn;
    tmp_fd->next = flowp->flowData;
    flowp->flowData = tmp_fd;
    return 0;
}

int AppIdFlowdataAddId(tAppIdData *flowp, uint16_t port, const RNAServiceElement *svc_element)
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

tAppIdData *AppIdEarlySessionCreate(const SFSnortPacket *ctrlPkt, snort_ip *cliIp, uint16_t cliPort,
                          snort_ip *srvIp, uint16_t srvPort, uint8_t proto, int16_t app_id)
{
    tAppIdData *data;

    data = appSharedDataAlloc(proto, cliIp);
#ifdef RNA_DEBUG_EXPECTED_FLOWS
    _dpd.errMsg("Allocated %p\n",data);
#endif

    if (_dpd.sessionAPI->set_application_protocol_id_expected(ctrlPkt, cliIp, cliPort, srvIp, srvPort,
                                                             proto, app_id, PP_APP_ID, data,
#ifdef RNA_DEBUG_EXPECTED_FLOWS
                                                             (void (*)(void *))flowAppSharedDataDelete)
#else
                                                             (void (*)(void *))appSharedDataDelete)
#endif
        )
    {
#ifdef RNA_DEBUG_EXPECTED_FLOWS
        char src_ip[INET6_ADDRSTRLEN];
        char dst_ip[INET6_ADDRSTRLEN];

        inet_ntop(cliIp->family, (void *)cliIp->ip32, src_ip, sizeof(src_ip));
        inet_ntop(srvIp->family, (void *)srvIp->ip32, dst_ip, sizeof(dst_ip));
        _dpd.errMsg( "Could not create an expected flow for %s-%u -> %s-%u %u", src_ip,
               (unsigned)cliPort, dst_ip, (unsigned)srvPort, (unsigned)proto);
        flowAppSharedDataDelete(data);
#else
        appSharedDataDelete(data);
#endif
        return NULL;
    }
#ifdef RNA_DEBUG_EXPECTED_FLOWS
    else
    {
        char src_ip[INET6_ADDRSTRLEN];
        char dst_ip[INET6_ADDRSTRLEN];

        inet_ntop(cliIp->family, (void *)cliIp->ip32, src_ip, sizeof(src_ip));
        inet_ntop(srvIp->family, (void *)srvIp->ip32, dst_ip, sizeof(dst_ip));
        _dpd.errMsg( "created an expected flow for %s-%u -> %s-%u %u", src_ip,
               (unsigned)cliPort, dst_ip, (unsigned)srvPort, (unsigned)proto);
    }
#endif
    return data;
}

