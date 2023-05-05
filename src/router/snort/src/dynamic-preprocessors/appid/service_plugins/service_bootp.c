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

#include "appIdApi.h"
#include "appInfoTable.h"
#include "flow.h"
#include "service_api.h"

#define DHCP_MAGIC_COOKIE 0x63825363

#pragma pack(1)

typedef struct _SERVICE_BOOTP_HEADER
{
    uint8_t op;
    uint8_t htype;
    uint8_t hlen;
    uint8_t hops;
    uint32_t xid;
    uint16_t secs;
    uint16_t flags;
    uint32_t ciaddr;
    uint32_t yiaddr;
    uint32_t siaddr;
    uint32_t giaddr;
    uint8_t chaddr[16];
    uint8_t sname[64];
    uint8_t file[128];
} ServiceBOOTPHeader;

typedef enum {
    DHCP_OPT_SUBNET_MASK = 1,
    DHCP_OPT_ROUTER = 3,
    DHCP_OPT_DOMAIN_NAME_SERVER = 6,
    DHCP_OPT_DOMAIN_NAME = 15,
    DHCP_OPT_IPADDR_LEASE_TIME = 51,
    DHCP_OPT_DHCP_MESSAGE_TYPE =53

} DHCP_OPTIONS;

typedef struct _SERVICE_DHCP_HEADER
{
    uint8_t option;
    uint8_t len;
} ServiceDHCPOption;

#pragma pack()

static int bootp_init(const InitServiceAPI * const init_api);
static int bootp_validate(ServiceValidationArgs* args);

static tRNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &bootp_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "bootp",
    .ref_count = 1,
    .current_ref_count = 1,
};

static RNAServiceValidationPort pp[] =
{
    {&bootp_validate, 67, IPPROTO_UDP},
    {&bootp_validate, 67, IPPROTO_UDP, 1},
    {NULL, 0, 0}
};

tRNAServiceValidationModule bootp_service_mod =
{
    "DHCP",
    &bootp_init,
    pp
};


static tAppRegistryEntry appIdRegistry[] =
{
    {APP_ID_DHCP, APPINFO_FLAG_SERVICE_ADDITIONAL | APPINFO_FLAG_SERVICE_UDP_REVERSED}
};

static int bootp_init(const InitServiceAPI * const init_api)
{
	unsigned i;
	for (i=0; i < sizeof(appIdRegistry)/sizeof(*appIdRegistry); i++)
	{
		_dpd.debugMsg(DEBUG_LOG,"registering appId: %d\n",appIdRegistry[i].appId);
		init_api->RegisterAppId(&bootp_validate, appIdRegistry[i].appId, appIdRegistry[i].additionalInfo, init_api->pAppidConfig);
	}

    return 0;
}

static int bootp_validate(ServiceValidationArgs* args)
{
    const ServiceBOOTPHeader *bh;
    const ServiceDHCPOption *op;
    unsigned i;
    unsigned op55_len=0;
    unsigned op60_len=0;
    const uint8_t *op55=NULL;
    const uint8_t *op60=NULL;
    tAppIdData *flowp = args->flowp;
    const uint8_t *data = args->data;
    SFSnortPacket *pkt = args->pkt; 
    const int dir = args->dir;
    uint16_t size = args->size;

    if (!size)
        goto inprocess;

    if (size < sizeof(ServiceBOOTPHeader))
        goto fail;

    bh = (const ServiceBOOTPHeader *)data;

    if (bh->htype != 0x01)
        goto fail;
    if (bh->hlen != 0x06)
        goto fail;

    for (i=0; i<sizeof(bh->sname); i++)
    {
        if (!bh->sname[i]) break;
    }
    if (i >= sizeof(bh->sname)) goto fail;

    for (i=0; i<sizeof(bh->file); i++)
    {
        if (!bh->file[i]) break;
    }
    if (i >= sizeof(bh->file)) goto fail;

    if (bh->op == 0x01)
    {
        if (size > sizeof(ServiceBOOTPHeader) + 4)
        {
            if (ntohl(*((uint32_t *)(data + sizeof(ServiceBOOTPHeader)))) ==
                    DHCP_MAGIC_COOKIE)
            {
                int option53 = 0;
                for (i=sizeof(ServiceBOOTPHeader)+sizeof(uint32_t); i<size;)
                {
                    op = (ServiceDHCPOption *)&data[i];
                    if (op->option == 0xff)
                    {
                        if (!pkt->ether_header) goto fail;

                        if (option53 && op55_len && (memcmp(pkt->ether_header->ether_source, bh->chaddr, 6) == 0))
                        {
                            if(bootp_service_mod.api->data_add_dhcp(flowp, op55_len, op55, op60_len, op60,
                                                                    bh->chaddr))
                            {
                                return SERVICE_ENOMEM;
                            }
                        }
                        goto inprocess;
                    }
                    i += sizeof(ServiceDHCPOption);
                    if (i >= size) goto not_compatible;
                    if (op->option == 53 && op->len == 1 && i + 1 < size && data[i] == 3)
                    {
                        option53 = 1;
                    }
                    else if (op->option == 55 && op->len >= 1)
                    {
                        if(option53)
                        {
                            op55_len = op->len;
                            op55 = &data[i];
                        }
                    }
                    else if (op->option == 60 && op->len >= 1)
                    {
                        if(option53)
                        {
                            op60_len = op->len;
                            op60 = &data[i];
                        }
                    }
                    i += op->len;
                    if (i >= size) goto not_compatible;
                }
                goto not_compatible;
            }
        }
        goto not_compatible;
    }

    if (bh->op != 0x02) goto fail;

    if (dir == APP_ID_FROM_INITIATOR)
    {
        setAppIdFlag(flowp, APPID_SESSION_UDP_REVERSED);
    }
    else
    {
        clearAppIdFlag(flowp, APPID_SESSION_UDP_REVERSED);
    }

    if (size > sizeof(ServiceBOOTPHeader) + 4)
    {
        if (ntohl(*((uint32_t *)(data + sizeof(ServiceBOOTPHeader)))) ==
                DHCP_MAGIC_COOKIE)
        {
            int option53 = 0;
            uint32_t subnet = 0;
            uint32_t router = 0;
            uint32_t leaseTime = 0;

            for (i=sizeof(ServiceBOOTPHeader)+sizeof(uint32_t);
                 i<size;
                 )
            {
                op = (ServiceDHCPOption *)&data[i];
                if (op->option == 0xff)
                {
                    if (!pkt->ether_header) goto fail;

                    if (option53 && (memcmp(pkt->ether_header->ether_destination, bh->chaddr, 6) == 0))
                        bootp_service_mod.api->dhcpNewLease(flowp, bh->chaddr, bh->yiaddr, pkt->pkt_header->ingress_group, ntohl(subnet), ntohl(leaseTime), router);
                    goto success;
                }
                i += sizeof(ServiceDHCPOption);
                if (i + op->len > size) goto fail;

                switch (op->option)
                {
                    case DHCP_OPT_DHCP_MESSAGE_TYPE:
                        if (op->len == 1 && data[i] == 5)
                        {
                            option53 = 1;
                        }
                        break;
                    case DHCP_OPT_SUBNET_MASK:
                        if (op->len == 4)
                        {
                            memcpy(&subnet, &data[i], sizeof(subnet));
                        }
                        break;
                    case DHCP_OPT_ROUTER:
                        if (op->len == 4)
                        {
                            memcpy(&router, &data[i], sizeof(router));
                        }
                        break;
                    case DHCP_OPT_IPADDR_LEASE_TIME:
                        if (op->len == 4 )
                        {
                            memcpy(&leaseTime, &data[i], sizeof(leaseTime));
                        }
                        break;
                    default:
                        ;
                }
                i += op->len;
                if (i >= size) goto fail;
            }
            goto fail;
        }
    }

success:
    if (!getAppIdFlag(flowp, APPID_SESSION_SERVICE_DETECTED))
    {
        setAppIdFlag(flowp, APPID_SESSION_CONTINUE);
        bootp_service_mod.api->add_service(flowp, args->pkt, args->dir, &svc_element,
                                           APP_ID_DHCP, NULL, NULL, NULL, NULL);
    }
    return SERVICE_SUCCESS;

inprocess:
    if (!getAppIdFlag(flowp, APPID_SESSION_SERVICE_DETECTED))
    {
        bootp_service_mod.api->service_inprocess(flowp, args->pkt, args->dir, &svc_element, NULL);
    }
    return SERVICE_INPROCESS;

fail:
    if (!getAppIdFlag(flowp, APPID_SESSION_SERVICE_DETECTED))
    {
        bootp_service_mod.api->fail_service(flowp, args->pkt, args->dir, &svc_element,
                                            bootp_service_mod.flow_data_index, args->pConfig, NULL);
    }
    clearAppIdFlag(flowp, APPID_SESSION_CONTINUE);
    return SERVICE_NOMATCH;

not_compatible:
    if (!getAppIdFlag(flowp, APPID_SESSION_SERVICE_DETECTED))
    {
        bootp_service_mod.api->incompatible_data(flowp, args->pkt, args->dir, &svc_element,
                                                 bootp_service_mod.flow_data_index, args->pConfig, NULL);
    }
    return SERVICE_NOT_COMPATIBLE;
}
