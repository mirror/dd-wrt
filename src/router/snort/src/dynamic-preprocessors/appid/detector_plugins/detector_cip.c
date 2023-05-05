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

/**
 * @author RA/Cisco
 */

/* Description: AppID detector for CIP protocol. This is used to match against Lua-C API registrations. */


#include <stdlib.h>                   // For calloc

#include "appInfoTable.h"             // For appGetSnortIdFromAppId
#include "cip_common.h"               // For event data.
#include "flow.h"                     // For tAppIdData
#include "preprocids.h"               // For PP_APP_ID
#include "session_api.h"              // For SessionAPI
#include "stream_api.h"               // For StreamAPI
#include "sf_dynamic_preprocessor.h"  // For _dpd
#include "sf_snort_packet.h"          // For SFSnortPacket
#include "detector_cip.h"

#define CIP_DUMMY_PORT 25965
#define ENIP_DUMMY_PORT 49548

static const char svc_name[] = "cip";

CipPatternLists cipPatternLists;

static CLIENT_APP_RETCODE cip_client_init(const InitClientAppAPI * const init_api, SF_LIST *config);
static int cip_service_init(const InitServiceAPI * const init_api);
static CLIENT_APP_RETCODE cip_client_validate(const uint8_t *data, 
        uint16_t size, const int dir, tAppIdData *flowp, 
        SFSnortPacket *pkt, struct _Detector *userData, 
        const tAppIdConfig *pConfig);
MakeRNAServiceValidationPrototype(cip_service_validate);

static tAppRegistryEntry appIdClientRegistry[] =
{
    {APP_ID_ENIP, APPINFO_FLAG_CLIENT_ADDITIONAL},
    {APP_ID_CIP, APPINFO_FLAG_CLIENT_ADDITIONAL},
};

static tAppRegistryEntry appIdServiceRegistry[] =
{
    {APP_ID_ENIP, APPINFO_FLAG_SERVICE_ADDITIONAL},
    {APP_ID_CIP, APPINFO_FLAG_SERVICE_ADDITIONAL},
};

tRNAClientAppModule cip_client_mod =
{
    .name = "CIP",
    .proto = IPPROTO_TCP,
    .init = &cip_client_init,
    .validate = &cip_client_validate,
    .minimum_matches = 1,
    .provides_user = 0,
};

tRNAClientAppModule enip_client_mod =
{
    .name = "ENIP",
    .proto = IPPROTO_TCP,
    .init = &cip_client_init,
    .validate = &cip_client_validate,
    .minimum_matches = 1,
    .provides_user = 0,
};

static RNAServiceValidationPort cip_pp[] =
{
    {&cip_service_validate, CIP_DUMMY_PORT, IPPROTO_TCP},
    {NULL, 0, 0}
};

static RNAServiceValidationPort enip_pp[] =
{
    {&cip_service_validate, ENIP_DUMMY_PORT, IPPROTO_TCP},
    {NULL, 0, 0}
};

SF_SO_PUBLIC tRNAServiceValidationModule cip_service_mod =
{
    svc_name,
    &cip_service_init,
    cip_pp
};

SF_SO_PUBLIC tRNAServiceValidationModule enip_service_mod =
{
    svc_name,
    &cip_service_init,
    enip_pp
};

static tRNAServiceElement svc_element =
{
    .next = NULL,
    .validate = &cip_service_validate,
    .detectorType = DETECTOR_TYPE_DECODER,
    .name = "cip",
    .ref_count = 1,
    .current_ref_count = 1,
};

static CLIENT_APP_RETCODE cip_client_init(const InitClientAppAPI * const init_api, SF_LIST *config)
{
    unsigned j;
    for (j=0; j < sizeof(appIdClientRegistry)/sizeof(*appIdClientRegistry); j++)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_APPID, "registering appId: %d\n",appIdClientRegistry[j].appId););
        init_api->RegisterAppId(&cip_client_validate, appIdClientRegistry[j].appId, appIdClientRegistry[j].additionalInfo, init_api->pAppidConfig);
    }
    return CLIENT_APP_SUCCESS;
}

static int cip_service_init(const InitServiceAPI * const init_api)
{
    unsigned i;
    for (i=0; i < sizeof(appIdServiceRegistry)/sizeof(*appIdServiceRegistry); i++)
    {
        DEBUG_WRAP(DebugMessage(DEBUG_APPID, "registering appId: %d\n",appIdServiceRegistry[i].appId););
        init_api->RegisterAppId(&cip_service_validate, appIdServiceRegistry[i].appId, appIdServiceRegistry[i].additionalInfo, init_api->pAppidConfig);
    }

    return 0;
}

static CLIENT_APP_RETCODE cip_client_validate(const uint8_t *data, uint16_t size, const int dir,
                                              tAppIdData *flowp, SFSnortPacket *pkt,
                                              struct _Detector *userData,
                                              const tAppIdConfig *pConfig)
{
    return CLIENT_APP_INPROCESS;
}

MakeRNAServiceValidationPrototype(cip_service_validate)
{
    return SERVICE_INPROCESS;
}

int CipAddEnipCommand(tAppId app_id, uint16_t command_id)
{
    EnipCommandList *pattern = calloc(1, sizeof(EnipCommandList));
    if (!pattern)
    {
        return -1;
    }

    pattern->data.app_id = app_id;
    pattern->data.command_id = command_id;

    pattern->next = cipPatternLists.enip_command_list;
    cipPatternLists.enip_command_list = pattern;

    return 0;
}

int CipAddPath(tAppId app_id,
    uint32_t class_id,
    uint8_t service_id)
{
    CipPathList *pattern = calloc(1, sizeof(CipPathList));
    if (!pattern)
    {
        return -1;
    }

    pattern->data.app_id = app_id;
    pattern->data.class_id = class_id;
    pattern->data.service_id = service_id;

    pattern->next = cipPatternLists.path_list;
    cipPatternLists.path_list = pattern;

    return 0;
}

int CipAddSetAttribute(tAppId app_id,
    uint32_t class_id,
    bool is_class_instance,
    uint32_t attribute_id)
{
    CipSetAttributeList *pattern = calloc(1, sizeof(CipSetAttributeList));
    if (!pattern)
    {
        return -1;
    }

    pattern->data.app_id = app_id;
    pattern->data.class_id = class_id;
    pattern->data.is_class_instance = is_class_instance;
    pattern->data.attribute_id = attribute_id;

    pattern->next = cipPatternLists.set_attribute_list;
    cipPatternLists.set_attribute_list = pattern;

    return 0;
}

int CipAddConnectionClass(tAppId app_id, uint32_t class_id)
{
    CipConnectionClassList *pattern = calloc(1, sizeof(CipConnectionClassList));
    if (!pattern)
    {
        return -1;
    }

    pattern->data.app_id = app_id;
    pattern->data.class_id = class_id;

    pattern->next = cipPatternLists.connection_list;
    cipPatternLists.connection_list = pattern;

    return 0;
}

int CipAddExtendedSymbolService(tAppId app_id, uint8_t service_id)
{
    CipServiceList *pattern = calloc(1, sizeof(CipServiceList));
    if (!pattern)
    {
        return -1;
    }

    pattern->data.app_id = app_id;
    pattern->data.service_id = service_id;

    pattern->next = cipPatternLists.symbol_list;
    cipPatternLists.symbol_list = pattern;

    return 0;
}

int CipAddService(tAppId app_id, uint8_t service_id)
{
    CipServiceList *pattern = calloc(1, sizeof(CipServiceList));
    if (!pattern)
    {
        return -1;
    }

    pattern->data.app_id = app_id;
    pattern->data.service_id = service_id;

    pattern->next = cipPatternLists.service_list;
    cipPatternLists.service_list = pattern;

    return 0;
}

static tAppId MatchEnipCommand(const EnipCommandList *enip_command_list,
    const CipEventData *event_data)
{
    tAppId found_app_id = APP_ID_ENIP;

    while (enip_command_list)
    {
        if (event_data->enip_command_id == enip_command_list->data.command_id)
        {
            found_app_id = enip_command_list->data.app_id;
            break;
        }

        enip_command_list = enip_command_list->next;
    }

    return found_app_id;
}

static tAppId MatchCipService(const CipServiceList *service_list, const CipEventData *event_data)
{
    tAppId found_app_id = APP_ID_CIP_UNKNOWN;

    while (service_list)
    {
        if (event_data->service_id == service_list->data.service_id)
        {
            found_app_id = service_list->data.app_id;
            break;
        }

        service_list = service_list->next;
    }

    return found_app_id;
}

static tAppId MatchCipPath(const CipPathList *path_list, const CipEventData *event_data)
{
    tAppId found_app_id = APP_ID_CIP_UNKNOWN;

    while (path_list)
    {
        if ((event_data->class_id == path_list->data.class_id)
            && (event_data->service_id == path_list->data.service_id))
        {
            found_app_id = path_list->data.app_id;
            break;
        }

        path_list = path_list->next;
    }

    return found_app_id;
}

static tAppId MatchCipSetAttribute(const CipSetAttributeList *set_attribute_list,
    const CipEventData *event_data)
{
    tAppId found_app_id = APP_ID_CIP_UNKNOWN;

    bool is_class_instance = (event_data->instance_id == 0);

    while (set_attribute_list)
    {
        if ((event_data->class_id == set_attribute_list->data.class_id)
            && (is_class_instance == set_attribute_list->data.is_class_instance)
            && (event_data->attribute_id == set_attribute_list->data.attribute_id))
        {
            found_app_id = set_attribute_list->data.app_id;
            break;
        }

        set_attribute_list = set_attribute_list->next;
    }

    return found_app_id;
}

static tAppId MatchCipConnection(const CipConnectionClassList *connection_list,
    const CipEventData *event_data)
{
    tAppId found_app_id = APP_ID_CIP_UNKNOWN;

    while (connection_list)
    {
        if (event_data->class_id == connection_list->data.class_id)
        {
            found_app_id = connection_list->data.app_id;
            break;
        }

        connection_list = connection_list->next;
    }

    return found_app_id;
}

static tAppId GetCipPayloadId(const CipPatternLists *cip_pattern_lists, const CipEventData *event_data)
{
    tAppId found_app_id = APP_ID_CIP_UNKNOWN;

    // Look up the appid.
    switch (event_data->type)
    {
    case CIP_DATA_TYPE_PATH_CLASS:
    {
        found_app_id = MatchCipPath(cip_pattern_lists->path_list, event_data);

        if (found_app_id == APP_ID_CIP_UNKNOWN)
        {
            found_app_id = MatchCipService(cip_pattern_lists->service_list, event_data);
        }
    }
    break;

    case CIP_DATA_TYPE_PATH_EXT_SYMBOL:
    {
        found_app_id = MatchCipService(cip_pattern_lists->symbol_list, event_data);

        if (found_app_id == APP_ID_CIP_UNKNOWN)
        {
            found_app_id = MatchCipService(cip_pattern_lists->service_list, event_data);
        }
    }
    break;

    case CIP_DATA_TYPE_SET_ATTRIBUTE:
    {
        found_app_id = MatchCipSetAttribute(cip_pattern_lists->set_attribute_list, event_data);

        if (found_app_id == APP_ID_CIP_UNKNOWN)
        {
            found_app_id = MatchCipService(cip_pattern_lists->symbol_list, event_data);

            if (found_app_id == APP_ID_CIP_UNKNOWN)
            {
                found_app_id = MatchCipService(cip_pattern_lists->service_list, event_data);
            }
        }
    }
    break;

    case CIP_DATA_TYPE_CONNECTION:
    {
        found_app_id = MatchCipConnection(cip_pattern_lists->connection_list, event_data);
    }
    break;

    case CIP_DATA_TYPE_IMPLICIT:
    {
        found_app_id = MatchCipConnection(cip_pattern_lists->connection_list, event_data);
    }
    break;
 
    case CIP_DATA_TYPE_OTHER:
    {
        found_app_id = APP_ID_CIP_UNKNOWN;
    }
    break;

    case CIP_DATA_TYPE_MALFORMED:
    {
        found_app_id = APP_ID_CIP_MALFORMED;
    }
    break;

    case CIP_DATA_TYPE_ENIP_COMMAND:
    {
        found_app_id = MatchEnipCommand(cip_pattern_lists->enip_command_list, event_data);
    }
    break;

    default:
        // Will not happen.
        break;
    }

    return found_app_id;
}

void CipSessionSnortCallback(void *ssnptr, ServiceEventType eventType, void *data)
{
    tAppIdData *flowp = NULL;
    CipEventData *event_data = (CipEventData *)data;
    const SFSnortPacket *p = event_data->snort_packet;

    int direction;

    if (!p)
    {
        _dpd.errMsg("Missing packet: CipSessionSnortCallback\n");
        return;
    }

    if (p->stream_session)
    {
        flowp = getAppIdData(p->stream_session);
    }

    if (!flowp)
    {
        _dpd.errMsg("Missing session: CipSessionSnortCallback\n");
        return;
    }

    if (appidStaticConfig->multipayload_max_packets && 
        flowp->session_packet_count > appidStaticConfig->multipayload_max_packets)
        return;

    tAppId found_app_id = GetCipPayloadId(&cipPatternLists, event_data);

    // Set CIP app and payload id.
    direction = (_dpd.sessionAPI->get_packet_direction((SFSnortPacket *)p) & FLAG_FROM_CLIENT) ? APP_ID_FROM_INITIATOR : APP_ID_FROM_RESPONDER;
    cip_service_mod.api->add_service(flowp, p, direction, &svc_element,
                                     APP_ID_CIP, NULL, NULL, NULL, NULL);
    cip_service_mod.api->add_multipayload(flowp, found_app_id);
}

static int FreeEnipCommandList(EnipCommandList* enip_command_list)
{
    int number_enip_rules = 0;
    EnipCommandList* command_node;
    for (command_node = enip_command_list;
        command_node != NULL;
        command_node = enip_command_list)
    {
        enip_command_list = command_node->next;
        free(command_node);

        number_enip_rules++;
    }

    return number_enip_rules;
}

static int FreeCipPathList(CipPathList* path_list)
{
    int number_path_rules = 0;
    CipPathList* path_node;
    for (path_node = path_list;
        path_node != NULL;
        path_node = path_list)
    {
        path_list = path_node->next;
        free(path_node);

        number_path_rules++;
    }

    return number_path_rules;
}

static int FreeCipSetAttributeList(CipSetAttributeList* set_attribute_list)
{
    int number_set_attribute_rules = 0;
    CipSetAttributeList* set_attribute_node;
    for (set_attribute_node = set_attribute_list;
        set_attribute_node != NULL;
        set_attribute_node = set_attribute_list)
    {
        set_attribute_list = set_attribute_node->next;
        free(set_attribute_node);

        number_set_attribute_rules++;
    }

    return number_set_attribute_rules;
}

static int FreeCipConnectionClassList(CipConnectionClassList* connection_list)
{
    int number_connection_rules = 0;
    CipConnectionClassList* connection_node;
    for (connection_node = connection_list;
        connection_node != NULL;
        connection_node = connection_list)
    {
        connection_list = connection_node->next;
        free(connection_node);

        number_connection_rules++;
    }

    return number_connection_rules;
}

static int FreeCipServiceList(CipServiceList* service_list)
{
    int number_service_rules = 0;
    CipServiceList* symbol_node;
    for (symbol_node = service_list;
        symbol_node != NULL;
        symbol_node = service_list)
    {
        service_list = symbol_node->next;
        free(symbol_node);

        number_service_rules++;
    }

    return number_service_rules;
}

void CipClean(void)
{
    FreeEnipCommandList(cipPatternLists.enip_command_list);
    cipPatternLists.enip_command_list = NULL;

    FreeCipPathList(cipPatternLists.path_list);
    cipPatternLists.path_list = NULL;

    FreeCipSetAttributeList(cipPatternLists.set_attribute_list);
    cipPatternLists.set_attribute_list = NULL;

    FreeCipConnectionClassList(cipPatternLists.connection_list);
    cipPatternLists.connection_list = NULL;

    FreeCipServiceList(cipPatternLists.symbol_list);
    cipPatternLists.symbol_list = NULL;

    FreeCipServiceList(cipPatternLists.service_list);
    cipPatternLists.service_list = NULL;
}
