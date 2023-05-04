/*
 *  sf_snort_plugin_hdropts.c
 *
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
 * Copyright (C) 2005-2013 Sourcefire, Inc.
 *
 * Author: Steve Sturges
 *         Andy Mullican
 *
 * Date: 5/2005
 *
 *
 * Header Option operations for dynamic rule engine
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_dynamic_define.h"
#include "sf_snort_packet.h"
#include "sf_snort_plugin_api.h"
#include "sf_dynamic_engine.h"
#include "ipv6_port.h"

static int checkHdrOptInternal(void *, HdrOptCheck *);

int ValidateHeaderCheck(Rule *rule, HdrOptCheck *optData)
{
    int retVal =0;

    switch (optData->hdrField)
    {
    case IP_HDR_OPTIONS:
        if ((optData->op != CHECK_EQ) &&
            (optData->op != CHECK_NEQ))
        {
            _ded.errMsg("Invalid operator for Check Header IP Options: %d "
                "for dynamic rule [%d:%d].\n"
                "Must be either CHECK_EQ (option present) or "
                "CHECK_NEQ (not present).\n",
                optData->op, rule->info.genID, rule->info.sigID);
            retVal = -1;
        }
        break;
    case TCP_HDR_OPTIONS:
        if ((optData->op != CHECK_EQ) &&
            (optData->op != CHECK_NEQ))
        {
            _ded.errMsg("Invalid operator for Check Header IP Options: %d "
                "for dynamic rule [%d:%d].\n"
                "Must be either CHECK_EQ (option present) or "
                "CHECK_NEQ (not present).\n",
                optData->op, rule->info.genID, rule->info.sigID);
            retVal = -1;
        }
        break;
    case IP_HDR_FRAGBITS:
        if ((optData->op != CHECK_EQ) &&
            (optData->op != CHECK_ALL) &&
            (optData->op != CHECK_ATLEASTONE) &&
            (optData->op != CHECK_NONE))
        {
            _ded.errMsg("Invalid operator for Check IP Fragbits: %d "
                "for dynamic rule [%d:%d].\n",
                optData->op, rule->info.genID, rule->info.sigID);
            retVal = -1;
        }
    }
    return retVal;
}

int checkBits(uint32_t value, uint32_t op, uint32_t bits)
{
    switch (op)
    {
    case CHECK_EQ:
        if (value == bits)
            return RULE_MATCH;
        break;
    case CHECK_ALL:
        if ((bits & value) == value)
            return RULE_MATCH;
        break;
    case CHECK_ATLEASTONE:
        if ((bits & value) != 0)
            return RULE_MATCH;
        break;
    case CHECK_NONE:
        if ((bits & value) == 0)
            return RULE_MATCH;
        break;
    }
    return RULE_NOMATCH;
}

int checkOptions(uint32_t value, int op, IPOptions options[], int numOptions)
{
    int found = 0;
    int i;

    for (i=0;i<numOptions;i++)
    {
        if (options[i].option_code == value)
        {
            found = 1;
            break;
        }
    }

    switch (op)
    {
    case CHECK_EQ:
        if (found)
            return RULE_MATCH;
        else
            return RULE_NOMATCH;
        break;
    case CHECK_NEQ:
        if (found)
            return RULE_NOMATCH;
        else
            return RULE_MATCH;
        break;
    default: /* Should never get here! */
        break;
    }

    return RULE_NOMATCH;
}

int checkField(int op, uint32_t value1, uint32_t value2)
{
    switch (op)
    {
        case CHECK_EQ:
            if (value1 == value2)
                return RULE_MATCH;
            break;
        case CHECK_NEQ:
            if (value1 != value2)
                return RULE_MATCH;
            break;
        case CHECK_LT:
            if (value1 < value2)
                return RULE_MATCH;
            break;
        case CHECK_GT:
            if (value1 > value2)
                return RULE_MATCH;
            break;
        case CHECK_LTE:
            if (value1 <= value2)
                return RULE_MATCH;
            break;
        case CHECK_GTE:
            if (value1 >= value2)
                return RULE_MATCH;
            break;
        case CHECK_AND:
            if (value1 & value2)
                return RULE_MATCH;
            break;
        case CHECK_XOR:
            if (value1 ^ value2)
                return RULE_MATCH;
            break;
    }

    return RULE_NOMATCH;
}

ENGINE_LINKAGE int checkHdrOpt(void *p, HdrOptCheck *optData)
{
    if (optData->flags & NOT_FLAG)
        return invertMatchResult(checkHdrOptInternal(p, optData));
    return checkHdrOptInternal(p, optData);
}

/* Exported C source routines */
/*
 * Check header option specified against packet
 *
 * Return 1 if check is true (e.g. data matches)
 * Return 0 if check is not true.
 */
static int checkHdrOptInternal(void *p, HdrOptCheck *optData)
{
    SFSnortPacket *pkt = (SFSnortPacket *)p;
    /* Header field will be extracted from its native
     * 1 or 2 bytes, converted to host byte order,
     * and placed in a 4 byte value for easy comparison
     */
    uint32_t value = 0;

    if ((optData->hdrField & IP_HDR_OPTCHECK_MASK) && (!pkt->ip4_header))
        return RULE_NOMATCH;

    if ((optData->hdrField & TCP_HDR_OPTCHECK_MASK) &&
        (!pkt->ip4_header || !pkt->tcp_header))
        return RULE_NOMATCH;

    if ((optData->hdrField & ICMP_HDR_OPTCHECK_MASK) &&
        (!IPH_IS_VALID(pkt) || !pkt->icmp_header))
        return RULE_NOMATCH;

    switch (optData->hdrField)
    {
    /* IP Header Checks */
    case IP_HDR_ID:
        value = IS_IP6(pkt) ? ntohl(GET_IPH_ID(pkt)) : ntohs((uint16_t)GET_IPH_ID(pkt));
        break;
    case IP_HDR_PROTO:
        //value = pkt->ip4_header->proto;
        value = GET_IPH_PROTO(pkt);
        break;
    case IP_HDR_FRAGBITS:
        return checkBits(optData->value, optData->op, ((ntohs(GET_IPH_OFF(pkt)) & 0xe000) & ~optData->mask_value));
        break;
    case IP_HDR_FRAGOFFSET:
        value = ntohs(GET_IPH_OFF((pkt))) & 0x1FFF;
        break;
    case IP_HDR_TOS:
        //value = pkt->ip4_header->type_service;
        value = GET_IPH_TOS(pkt);
        break;
    case IP_HDR_TTL:
        //value = pkt->ip4_header->time_to_live;
        value = GET_IPH_TTL(pkt);
        break;
    case IP_HDR_OPTIONS:
        return checkOptions(optData->value, optData->op, pkt->ip_options, pkt->num_ip_options);
        break;

    /* TCP Header checks */
    case TCP_HDR_ACK:
        value = ntohl(pkt->tcp_header->acknowledgement);
        break;
    case TCP_HDR_SEQ:
        value = ntohl(pkt->tcp_header->sequence);
        break;
    case TCP_HDR_FLAGS:
        return checkBits(optData->value, optData->op, (pkt->tcp_header->flags & ~optData->mask_value));
        break;
    case TCP_HDR_WIN:
        value = ntohs(pkt->tcp_header->window);
        break;
    case TCP_HDR_OPTIONS:
        return checkOptions(optData->value, optData->op, pkt->tcp_options, pkt->num_tcp_options);
        break;

    /* ICMP Header checks */
    case ICMP_HDR_CODE:
        value = pkt->icmp_header->code;
        break;
    case ICMP_HDR_TYPE:
        value = pkt->icmp_header->type;
        break;
    case ICMP_HDR_ID:
        if ((pkt->icmp_header->code == ICMP_ECHO_REQUEST) ||
            (pkt->icmp_header->code == ICMP_ECHO_REPLY))
        {
            value = ntohs(pkt->icmp_header->icmp_header_union.echo.id);
        }
        else
        {
            return RULE_NOMATCH;
        }
        break;
    case ICMP_HDR_SEQ:
        if ((pkt->icmp_header->code == ICMP_ECHO_REQUEST) ||
            (pkt->icmp_header->code == ICMP_ECHO_REPLY))
        {
            value = ntohs(pkt->icmp_header->icmp_header_union.echo.seq);
        }
        else
        {
            return RULE_NOMATCH;
        }
        break;

    default:
        return RULE_NOMATCH;
        break;
    }

    return checkField(optData->op, value, optData->value);
}

