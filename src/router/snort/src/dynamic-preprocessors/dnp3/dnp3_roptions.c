/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
 *
 * Author: Ryan Jordan
 *
 * Rule options for the DNP3 preprocessor
 *
 */

#include <string.h>

#include "sf_types.h"
#include "sf_snort_plugin_api.h"
#include "sf_snort_packet.h"
#include "sf_dynamic_preprocessor.h"
#include "mempool.h"

#include "spp_dnp3.h"
#include "dnp3_map.h"
#include "dnp3_roptions.h"

/* Object decoding constants */
#define DNP3_OBJ_HDR_MIN_LEN 3 /* group, var, qualifier */
#define DNP3_OBJ_QUAL_PREFIX(x) ((x & 0x70) >> 4)
#define DNP3_OBJ_QUAL_RANGE(x) (x & 0x0F)

/* Object header prefix codes */
#define DNP3_PREFIX_NO_PREFIX   0x00
#define DNP3_PREFIX_1OCT_INDEX  0x01
#define DNP3_PREFIX_2OCT_INDEX  0x02
#define DNP3_PREFIX_4OCT_INDEX  0x03
#define DNP3_PREFIX_1OCT_SIZE   0x04
#define DNP3_PREFIX_2OCT_SIZE   0x05
#define DNP3_PREFIX_4OCT_SIZE   0x06
#define DNP3_PREFIX_RESERVED    0x07

/* Object header range specifiers -- 0x0A & 0x0C-0x0F are reserved */
#define DNP3_RANGE_1OCT_INDICES     0x00
#define DNP3_RANGE_2OCT_INDICES     0x01
#define DNP3_RANGE_4OCT_INDICES     0x02
#define DNP3_RANGE_1OCT_ADDRESSES   0x03
#define DNP3_RANGE_2OCT_ADDRESSES   0x04
#define DNP3_RANGE_4OCT_ADDRESSES   0x05
#define DNP3_RANGE_NO_RANGE         0x06
#define DNP3_RANGE_1OCT_COUNT       0x07
#define DNP3_RANGE_2OCT_COUNT       0x08
#define DNP3_RANGE_4OCT_COUNT       0x09
#define DNP3_RANGE_VARIABLE         0x0B

typedef enum _dnp3_option_type_t
{
    DNP3_FUNC = 0,
    DNP3_OBJ,
    DNP3_IND,
    DNP3_DATA
} dnp3_option_type_t;

typedef struct _dnp3_option_data_t
{
    dnp3_option_type_t type;
    uint16_t arg;
} dnp3_option_data_t;

/* Parsing functions */
int DNP3FuncInit(struct _SnortConfig *sc, char *name, char *params, void **data)
{
    char *endptr;
    dnp3_option_data_t *dnp3_data;
    long func_code;

    if (name == NULL || data == NULL)
        return 0;

    if (params == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d): dnp3_func requires a "
            "number beween 0 and 255, or a valid function name.\n",
            *_dpd.config_file, *_dpd.config_line);
    }

    if (strcmp(name, DNP3_FUNC_NAME) != 0)
        return 0;

    dnp3_data = (dnp3_option_data_t *)calloc(1, sizeof(dnp3_option_data_t));
    if (dnp3_data == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) Failed to allocate memory for "
                      "dnp3_func data structure.\n", __FILE__, __LINE__);
    }

    /* Parsing time */
    if (isdigit(params[0]))
    {
        /* Function code given as integer */
        func_code = _dpd.SnortStrtol(params, &endptr, 10);
        if ((func_code > 255) || (func_code < 0) || (*endptr != '\0'))
        {
            DynamicPreprocessorFatalMessage("%s(%d): dnp3_func requires a "
                "number beween 0 and 255, or a valid function name.\n",
                *_dpd.config_file, *_dpd.config_line);
        }
    }
    else
    {
        func_code = DNP3FuncStrToCode(params);

        if (func_code == -1)
        {
            DynamicPreprocessorFatalMessage("%s(%d): dnp3_func requires a "
                "number beween 0 and 255, or a valid function name.\n",
                *_dpd.config_file, *_dpd.config_line);
        }
    }

    dnp3_data->type = DNP3_FUNC;
    dnp3_data->arg = (uint16_t) func_code;

    *data = (void *)dnp3_data;

    return 1;
}

NORETURN static inline void DNP3ObjError(void)
{
    DynamicPreprocessorFatalMessage("%s(%d) dnp3_obj requires two arguments,"
        "where each argument is a number between 0 and 255.\n",
        *_dpd.config_file, *_dpd.config_line);
}

int DNP3ObjInit(struct _SnortConfig *sc, char *name, char *params, void **data)
{
    char *endptr, *token, *saveptr;
    dnp3_option_data_t *dnp3_data;
    unsigned int obj_group, obj_var;

    if (name == NULL || data == NULL)
        return 0;

    if (strcmp(name, DNP3_OBJ_NAME) != 0)
        return 0;

    if (params == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d): No argument given for dnp3_obj. "
            "dnp3_obj requires two arguments, where each argument is a number "
            "between 0 and 255.\n", *_dpd.config_file, *_dpd.config_line);
    }

    dnp3_data = (dnp3_option_data_t *)calloc(1, sizeof(dnp3_option_data_t));
    if (dnp3_data == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) Failed to allocate memory for "
                      "dnp3_func data structure.\n", __FILE__, __LINE__);
    }

    token = strtok_r(params, ",", &saveptr);
    if (token == NULL)
        DNP3ObjError();

    /* First token: object group */
    obj_group = _dpd.SnortStrtoul(token, &endptr, 10);
    if ((obj_group > 255) || (*endptr != '\0'))
        DNP3ObjError();

    token = strtok_r(NULL, ",", &saveptr);
    if (token == NULL)
        DNP3ObjError();

    /* Second token: object var */
    obj_var = _dpd.SnortStrtoul(token, &endptr, 10);
    if ((obj_var > 255) || (*endptr != '\0'))
        DNP3ObjError();

    /* pack the two arguments into one uint16_t */
    dnp3_data->type = DNP3_OBJ;
    dnp3_data->arg = ((obj_group << 8) | (obj_var));

    *data = dnp3_data;

    return 1;
}

int DNP3IndInit(struct _SnortConfig *sc, char *name, char *params, void **data)
{
    dnp3_option_data_t *dnp3_data;
    char *token, *saveptr;
    uint16_t flags = 0;

    if (name == NULL || data == NULL)
        return 0;

    if (params == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d): dnp3_ind requires a "
            "number beween 0 and 255, or a valid function name.\n",
            *_dpd.config_file, *_dpd.config_line);
    }

    dnp3_data = (dnp3_option_data_t *)calloc(1, sizeof(dnp3_option_data_t));
    if (dnp3_data == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) Failed to allocate memory for "
                      "dnp3_func data structure.\n", __FILE__, __LINE__);
    }

    token = strtok_r(params, ",", &saveptr);

    while (token != NULL)
    {
        int flag = DNP3IndStrToCode(token);

        if (flag == -1)
        {
            DynamicPreprocessorFatalMessage("%s(%d): dnp3_ind requires a "
                "valid indication flag name. '%s' is invalid.\n",
                *_dpd.config_file, *_dpd.config_line, token);
        }

        flags |= (uint16_t) flag;
        token = strtok_r(NULL, ",", &saveptr);
    }

    if (flags == 0)
    {
        DynamicPreprocessorFatalMessage("%s(%d): dnp3_ind requires a "
            "valid indication flag name. No flags were given.\n",
            *_dpd.config_file, *_dpd.config_line);
    }

    dnp3_data->type = DNP3_IND;
    dnp3_data->arg = flags;

    *data = (void *)dnp3_data;

    return 1;
}

int DNP3DataInit(struct _SnortConfig *sc, char *name, char *params, void **data)
{
    dnp3_option_data_t *dnp3_data;

    if (name == NULL || data == NULL)
        return 0;

    /* nothing to parse. */
    if (params)
    {
        DynamicPreprocessorFatalMessage("%s(%d): dnp3_data does not take "
            "any arguments.\n", *_dpd.config_file, *_dpd.config_line);
    }

    dnp3_data = (dnp3_option_data_t *)calloc(1, sizeof(dnp3_option_data_t));
    if (dnp3_data == NULL)
    {
        DynamicPreprocessorFatalMessage("%s(%d) Failed to allocate memory for "
                      "dnp3_data data structure.\n", __FILE__, __LINE__);
    }

    dnp3_data->type = DNP3_DATA;
    dnp3_data->arg = 0;

    *data = (void *)dnp3_data;

    return 1;
}

/* Evaluation functions */
int DNP3FuncEval(void *raw_packet, const uint8_t **cursor, void *data)
{
    SFSnortPacket *packet = (SFSnortPacket *)raw_packet;
    MemBucket *tmp_bucket;
    dnp3_option_data_t *rule_data = (dnp3_option_data_t *)data;
    dnp3_session_data_t *session_data;
    dnp3_reassembly_data_t *rdata;

    /* The preprocessor only evaluates PAF-flushed PDUs. If the rule options
       don't check for this, they'll fire on stale session data when the
       original packet goes through before flushing. */
    if (packet->tcp_header && !PacketHasFullPDU(packet))
        return RULE_NOMATCH;

    /* For UDP packets, there is no PAF so we use the Alt Decode buffer. */
    if (packet->udp_header && !_dpd.Is_DetectFlag(SF_FLAG_ALT_DECODE))
        return RULE_NOMATCH;

    tmp_bucket = (MemBucket *)
        _dpd.sessionAPI->get_application_data(packet->stream_session, PP_DNP3);

    if ((packet->payload_size == 0) || (tmp_bucket == NULL))
    {
        return RULE_NOMATCH;
    }

    session_data = (dnp3_session_data_t *)tmp_bucket->data;

    if (session_data->direction == DNP3_CLIENT)
        rdata = &(session_data->client_rdata);
    else
        rdata = &(session_data->server_rdata);

    /* Only evaluate rules against complete Application-layer fragments */
    if (rdata->state != DNP3_REASSEMBLY_STATE__DONE)
        return RULE_NOMATCH;

    if (session_data->func == rule_data->arg)
        return RULE_MATCH;

    return RULE_NOMATCH;
}

static int DNP3DecodeObject(uint8_t *buf, uint16_t buflen, uint8_t rule_group, uint8_t rule_var)
{
    uint8_t group, var;

    /* XXX: uncomment these when fixing the below TODO regarding multiple objects
    uint8_t qualifier, prefix_size, prefix_code, range_specifier;
    uint32_t begin, end, num_objects;
    */

    if (buf == NULL || buflen < DNP3_OBJ_HDR_MIN_LEN)
        return RULE_NOMATCH;

    /* Decode group */
    group = *buf;
    buf++;
    buflen--;

    /* Decode var */
    var = *buf;
    buf++;
    buflen--;

    /* Match the rule option here, quit decoding if we found the right header. */
    if ((group == rule_group) && (var == rule_var))
        return RULE_MATCH;

/* TODO: Implement matching with multiple objects in a Request/Response. */
#if 0
    /* Decode qualifier */
    qualifier = *buf;
    prefix_code = DNP3_OBJ_QUAL_PREFIX(qualifier);
    range_specifier = DNP3_OBJ_QUAL_RANGE(qualifier);
    buf++;
    buflen--;

    /* The size of object prefixes depends on the prefix code */
    switch (prefix_code)
    {
        case DNP3_PREFIX_NO_PREFIX:
            prefix_size = 0;
            break;

        case DNP3_PREFIX_1OCT_INDEX:
        case DNP3_PREFIX_1OCT_SIZE:
            prefix_size = 1;
            break;

        case DNP3_PREFIX_2OCT_INDEX:
        case DNP3_PREFIX_2OCT_SIZE:
            prefix_size = 2;
            break;

        case DNP3_PREFIX_4OCT_INDEX:
        case DNP3_PREFIX_4OCT_SIZE:
            prefix_size = 4;
            break;

        default:
            /* TODO: Preprocessor alert on reserved value */
            return DNP3_FAIL;
    }

    /* Decoding of the range field depends on the Range Specifier */
    switch (range_specifier)
    {
        case DNP3_RANGE_1OCT_INDICES:
            if (buflen < 2)
                return DNP3_FAIL;

            /* Decode 8-bit indices for object prefixes */
            begin = *(uint8_t *)buf++;
            end = *(uint8_t *)buf++;
            buflen -= 2;

            /* Check that indices make sense */
            if (begin > end)
                return DNP3_FAIL; /* TODO: Preprocessor alert */

            num_objects = end - begin + 1;
            break;

        case DNP3_RANGE_2OCT_INDICES:
            if (buflen < 2)
                return DNP3_FAIL;

            /* Decode 8-bit indices for object prefixes */
            begin = *(uint16_t *)buf++;
            end = *(uint16_t *)buf++;
            buflen -= 2;

            /* Check that indices make sense */
            if (begin > end)
                return DNP3_FAIL; /* TODO: Preprocessor alert */

            num_objects = end - begin + 1;
            break;

        case DNP3_RANGE_4OCT_INDICES:
        case DNP3_RANGE_1OCT_ADDRESSES:
        case DNP3_RANGE_2OCT_ADDRESSES:
        case DNP3_RANGE_4OCT_ADDRESSES:
        case DNP3_RANGE_NO_RANGE:
        case DNP3_RANGE_1OCT_COUNT:
        case DNP3_RANGE_2OCT_COUNT:
        case DNP3_RANGE_4OCT_COUNT:
        case DNP3_RANGE_VARIABLE:
        default:
    }
#endif /* 0 */

    return RULE_NOMATCH;
}

int DNP3ObjEval(void *raw_packet, const uint8_t **cursor, void *data)
{
    SFSnortPacket *packet = (SFSnortPacket *)raw_packet;
    MemBucket *tmp_bucket;
    dnp3_option_data_t *rule_data = (dnp3_option_data_t *)data;
    dnp3_session_data_t *session_data;
    dnp3_reassembly_data_t *rdata;
    uint8_t group, var;
    uint8_t *obj_buffer;
    uint16_t obj_buflen;
    size_t header_size;
    int rval = RULE_NOMATCH;

    /* The preprocessor only evaluates PAF-flushed PDUs. If the rule options
       don't check for this, they'll fire on stale session data when the
       original packet goes through before flushing. */
    if (packet->tcp_header && !PacketHasFullPDU(packet))
        return RULE_NOMATCH;

    /* For UDP packets, there is no PAF so we use the Alt Decode buffer. */
    if (packet->udp_header && !_dpd.Is_DetectFlag(SF_FLAG_ALT_DECODE))
        return RULE_NOMATCH;

    tmp_bucket = (MemBucket *)
        _dpd.sessionAPI->get_application_data(packet->stream_session, PP_DNP3);

    if ((packet->payload_size == 0) || (tmp_bucket == NULL))
    {
        return RULE_NOMATCH;
    }

    session_data = (dnp3_session_data_t *)tmp_bucket->data;

    if (session_data->direction == DNP3_CLIENT)
    {
        rdata = &(session_data->client_rdata);
        header_size = sizeof(dnp3_app_request_header_t);
    }
    else
    {
        rdata = &(session_data->server_rdata);
        header_size = sizeof(dnp3_app_response_header_t);
    }

    /* Only evaluate rules against complete Application-layer fragments */
    if (rdata->state != DNP3_REASSEMBLY_STATE__DONE)
        return RULE_NOMATCH;

    /* Skip over the App request/response header.
       They are different sizes, depending on whether it is a request or response! */
    if (rdata->buflen < header_size)
        return RULE_NOMATCH;

    obj_buffer = (uint8_t *)rdata->buffer + header_size;
    obj_buflen = rdata->buflen - header_size;

    /* Rule parsing code combined our two arguments into a single uint16_t */
    group = (rule_data->arg >> 8);
    var = (rule_data->arg & 0x00FF);

    rval = DNP3DecodeObject(obj_buffer, obj_buflen, group, var);

    return rval;
}

int DNP3IndEval(void *raw_packet, const uint8_t **cursor, void *data)
{
    SFSnortPacket *packet = (SFSnortPacket *)raw_packet;
    MemBucket *tmp_bucket;
    dnp3_option_data_t *rule_data = (dnp3_option_data_t *)data;
    dnp3_session_data_t *session_data;
    dnp3_reassembly_data_t *rdata;

    /* The preprocessor only evaluates PAF-flushed PDUs. If the rule options
       don't check for this, they'll fire on stale session data when the
       original packet goes through before flushing. */
    if (packet->tcp_header && !PacketHasFullPDU(packet))
        return RULE_NOMATCH;

    /* For UDP packets, there is no PAF so we use the Alt Decode buffer. */
    if (packet->udp_header && !_dpd.Is_DetectFlag(SF_FLAG_ALT_DECODE))
        return RULE_NOMATCH;

    tmp_bucket = (MemBucket *)
        _dpd.sessionAPI->get_application_data(packet->stream_session, PP_DNP3);

    if ((packet->payload_size == 0) || (tmp_bucket == NULL))
    {
        return RULE_NOMATCH;
    }

    session_data = (dnp3_session_data_t *)tmp_bucket->data;

    /* Internal Indications only apply to DNP3 responses, not requests. */
    if (session_data->direction == DNP3_CLIENT)
        return RULE_NOMATCH;

    rdata = &(session_data->server_rdata);

    /* Only evaluate rules against complete Application-layer fragments */
    if (rdata->state != DNP3_REASSEMBLY_STATE__DONE)
        return RULE_NOMATCH;

    if (session_data->indications & rule_data->arg)
        return RULE_MATCH;

    return RULE_NOMATCH;
}

int DNP3DataEval(void *raw_packet, const uint8_t **cursor, void *data)
{
    SFSnortPacket *packet = (SFSnortPacket *)raw_packet;
    MemBucket *tmp_bucket;
    dnp3_session_data_t *session_data;
    dnp3_reassembly_data_t *rdata;

    /* The preprocessor only evaluates PAF-flushed PDUs. If the rule options
       don't check for this, they'll fire on stale session data when the
       original packet goes through before flushing. */
    if (packet->tcp_header && !PacketHasFullPDU(packet))
        return RULE_NOMATCH;

    /* For UDP packets, there is no PAF so we use the Alt Decode buffer. */
    if (packet->udp_header && !_dpd.Is_DetectFlag(SF_FLAG_ALT_DECODE))
        return RULE_NOMATCH;

    tmp_bucket = (MemBucket *)
        _dpd.sessionAPI->get_application_data(packet->stream_session, PP_DNP3);

    if ((packet->payload_size == 0) || (tmp_bucket == NULL))
    {
        return RULE_NOMATCH;
    }

    session_data = (dnp3_session_data_t *)tmp_bucket->data;

    if (session_data->direction == DNP3_CLIENT)
        rdata = &(session_data->client_rdata);
    else
        rdata = &(session_data->server_rdata);

    /* Only evaluate rules against complete Application-layer fragments */
    if (rdata->state != DNP3_REASSEMBLY_STATE__DONE)
        return RULE_NOMATCH;

    /* Set the cursor to the reassembled Application-layer buffer */
    *cursor = (uint8_t *)rdata->buffer;
    _dpd.SetAltDetect((uint8_t *)rdata->buffer, rdata->buflen);

    return RULE_MATCH;
}
