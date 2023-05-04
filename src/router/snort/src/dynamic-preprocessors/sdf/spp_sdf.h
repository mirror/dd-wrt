/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2009-2013 Sourcefire, Inc.
**
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

/*
 * spp_sdf.h: Definitions, prototypes, etc. for the SDF preprocessor.
 * Author: Ryan Jordan
 */

#ifndef SPP_SDF_H
#define SPP_SDF_H

/*#include "sdf_pattern_match.h"*/
#include <stdint.h>
#include "sfPolicyUserData.h"
#include "sdf_us_ssn.h"
#include "sdf_detection_option.h"

#define GENERATOR_SPP_SDF_RULES 138
#define GENERATOR_SPP_SDF_PREPROC 139

/* This is the maximum defined area number */
#define MAX_AREA 772

#define MAX_PORTS 65536
#define PORT_INDEX(port) port/8
#define CONV_PORT(port) 1 << (port % 8)
#define MAX_PROTOCOL_ORDINAL 8192

typedef struct _sdf_tree_node
{
    char *pattern;
    uint16_t num_children;
    uint16_t num_option_data;
    struct _sdf_tree_node **children;
    SDFOptionData **option_data_list;
} sdf_tree_node;

typedef struct _SDFSessionData
{
    sdf_tree_node *part_match_node;
    uint16_t part_match_index;
    uint32_t num_patterns, global_counter;
    uint8_t *counters;
    int8_t *rtns_matched;
    uint32_t config_num;
    uint32_t last_pkt_seq_num;
    int last_pkt_data_len;
} SDFSessionData;

typedef struct _SDFContext
{
    tSfPolicyUserContextId context_id;
    sdf_tree_node *head_node;
    uint32_t num_patterns;
} SDFContext;

typedef struct _SDFConfig
{
    SFSnortPacket *pseudo_packet;
    SDFSessionData *stateless_session;
    uint32_t threshold;
    uint8_t mask_output;
    int ssn_max_group[MAX_AREA+1];
    unsigned char src_ports[MAX_PORTS/8];
    unsigned char dst_ports[MAX_PORTS/8];
    unsigned char protocol_ordinals[MAX_PROTOCOL_ORDINAL];
    uint32_t config_num;
} SDFConfig;

/* Definitions of config options */
#define SDF_THRESHOLD_KEYWORD "alert_threshold"
#define SDF_MASK_KEYWORD "mask_output"
#define SDF_SSN_FILE_KEYWORD "ssn_file"
#define SDF_OPTION_NAME "sd_pattern"
#define SDF_OPTION_SEPARATORS ","

/* Order of SDF options */
#define SDF_OPTION_COUNT_NUM 1
#define SDF_OPTION_PATTERN_NUM 2

/* Keywords for SDF built-in option */

/* This pattern matches Visa/Mastercard/Amex, with & without spaces or dashes.
   The pattern alone would match other non-credit patterns, but the function
   SDFLuhnAlgorithm() does stricter checking. */
#define SDF_CREDIT_KEYWORD "credit_card"
#define SDF_CREDIT_PATTERN_ALL "\\D\\d{4} ?-?\\d{4} ?-?\\d{2} ?-?\\d{2} ?-?\\d{3}\\d?\\D"

#define SDF_SOCIAL_KEYWORD "us_social"
#define SDF_SOCIAL_PATTERN "\\D\\d{3}-\\d{2}-\\d{4}\\D"

#define SDF_SOCIAL_NODASHES_KEYWORD "us_social_nodashes"
#define SDF_SOCIAL_NODASHES_PATTERN "\\D\\d{9}\\D"

#define SDF_EMAIL_KEYWORD "email"
#define SDF_EMAIL_PATTERN "\\w@\\w"

/* Obfuscation constants */
#define SDF_OBFUSCATION_CHAR 'X'
#define SDF_OBFUSCATION_DIGITS_SHOWN 4

/* Length of ": 255\0" */
#define SDF_ALERT_LENGTH 6

/* Combo Alert constants */
#define SDF_COMBO_ALERT_SID 1
#define SDF_COMBO_ALERT_REV 1
#define SDF_COMBO_ALERT_CLASS 1
#define SDF_COMBO_ALERT_PRIORITY 1
#define SDF_COMBO_ALERT_STR "(spp_sdf) SDF Combination Alert"

extern SDFContext *sdf_context;

#endif
