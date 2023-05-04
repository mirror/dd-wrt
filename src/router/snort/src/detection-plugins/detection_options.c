/* $Id$ */
/*
** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
** Copyright (C) 2007-2013 Sourcefire, Inc.
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
**
**/

/**
**  @file        detection_options.c
**
**  @author      Steven Sturges
**
**  @brief       Support functions for rule option tree
**
**  This implements tree processing for rule options, evaluating common
**  detection options only once per pattern match.
**
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sfutil/sfxhash.h"
#include "sfutil/sfhashfcn.h"
#include "detection_options.h"
#include "detection_util.h"
#include "rules.h"
#include "treenodes.h"
#include "util.h"
#include "fpcreate.h"
#include "parser.h"

#include "sp_asn1.h"
#include "sp_byte_check.h"
#include "sp_byte_jump.h"
#include "sp_byte_extract.h"
#include "sp_byte_math.h"
#include "sp_clientserver.h"
#include "sp_cvs.h"
#include "sp_dsize_check.h"
#include "sp_flowbits.h"
#include "sp_ftpbounce.h"
#include "sp_icmp_code_check.h"
#include "sp_icmp_id_check.h"
#include "sp_icmp_seq_check.h"
#include "sp_icmp_type_check.h"
#include "sp_ip_fragbits.h"
#include "sp_ip_id_check.h"
#include "sp_ipoption_check.h"
#include "sp_ip_proto.h"
#include "sp_ip_same_check.h"
#include "sp_ip_tos_check.h"
#include "sp_file_data.h"
#include "sp_base64_decode.h"
#include "sp_isdataat.h"
#include "sp_pattern_match.h"
#include "sp_pcre.h"
#ifdef ENABLE_REACT
#include "sp_react.h"
#endif
#include "sp_replace.h"
#ifdef ENABLE_RESPOND
#include "sp_respond.h"
#endif
#include "sp_rpc_check.h"
#include "sp_session.h"
#include "sp_tcp_ack_check.h"
#include "sp_tcp_flag_check.h"
#include "sp_tcp_seq_check.h"
#include "sp_tcp_win_check.h"
#include "sp_ttl_check.h"
#include "sp_urilen_check.h"
#include "sp_hdr_opt_wrap.h"
# include "sp_file_type.h"

#include "sp_preprocopt.h"
#include "sp_dynamic.h"

#include "fpdetect.h"
#include "ppm.h"
#include "profiler.h"
#include "sfPolicy.h"
#include "detection_filter.h"
#include "encode.h"
#if defined(FEAT_OPEN_APPID)
#include "stream_common.h"
#include "sp_appid.h"
#endif /* defined(FEAT_OPEN_APPID) */

typedef struct _detection_option_key
{
    option_type_t option_type;
    void *option_data;
} detection_option_key_t;

#define HASH_RULE_OPTIONS 16384
#define HASH_RULE_TREE 8192

uint32_t detection_option_hash_func(SFHASHFCN *p, unsigned char *k, int n)
{
    uint32_t hash = 0;
    detection_option_key_t *key = (detection_option_key_t*)k;

    switch (key->option_type)
    {
        /* Call hash function specific to the key type */
        case RULE_OPTION_TYPE_ASN1:
            hash = Asn1Hash(key->option_data);
            break;
        case RULE_OPTION_TYPE_BYTE_TEST:
            hash = ByteTestHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_BYTE_JUMP:
            hash = ByteJumpHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_BYTE_EXTRACT:
            hash = ByteExtractHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_BYTE_MATH:
            hash = ByteMathHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_FLOW:
            hash = FlowHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_CVS:
            hash = CvsHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_DSIZE:
            hash = DSizeCheckHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_FLOWBIT:
            hash = FlowBitsHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_FTPBOUNCE:
            break;
        case RULE_OPTION_TYPE_FILE_DATA:
            hash = FileDataHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_BASE64_DECODE:
            hash = Base64DecodeHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_BASE64_DATA:
            break;
        case RULE_OPTION_TYPE_PKT_DATA:
            break;
        case RULE_OPTION_TYPE_ICMP_CODE:
            hash = IcmpCodeCheckHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_ICMP_ID:
            hash = IcmpIdCheckHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_ICMP_SEQ:
            hash = IcmpSeqCheckHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_ICMP_TYPE:
            hash = IcmpTypeCheckHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_IP_FRAGBITS:
            hash = IpFragBitsCheckHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_IP_FRAG_OFFSET:
            hash = IpFragOffsetCheckHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_IP_ID:
            hash = IpIdCheckHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_IP_OPTION:
            hash = IpOptionCheckHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_IP_PROTO:
            hash = IpProtoCheckHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_IP_SAME:
            hash = IpSameCheckHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_IP_TOS:
            hash = IpTosCheckHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_IS_DATA_AT:
            hash = IsDataAtHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_CONTENT:
        case RULE_OPTION_TYPE_CONTENT_URI:
            hash = PatternMatchHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_PCRE:
            hash = PcreHash(key->option_data);
            break;
#ifdef ENABLE_REACT
        case RULE_OPTION_TYPE_REACT:
            hash = ReactHash(key->option_data);
            break;
#endif
#ifdef ENABLE_RESPOND
        case RULE_OPTION_TYPE_RESPOND:
            hash = RespondHash(key->option_data);
            break;
#endif
        case RULE_OPTION_TYPE_RPC_CHECK:
            hash = RpcCheckHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_SESSION:
            hash = SessionHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_TCP_ACK:
            hash = TcpAckCheckHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_TCP_FLAG:
            hash = TcpFlagCheckHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_TCP_SEQ:
            hash = TcpSeqCheckHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_TCP_WIN:
            hash = TcpWinCheckHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_TTL:
            hash = TtlCheckHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_URILEN:
            hash = UriLenCheckHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_HDR_OPT_CHECK:
            hash = HdrOptCheckHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_FILE_TYPE:
            hash = FileTypeHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_PREPROCESSOR:
            hash = PreprocessorRuleOptionHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_DYNAMIC:
            hash = DynamicRuleHash(key->option_data);
            break;
        case RULE_OPTION_TYPE_LEAF_NODE:
            hash = 0;
            break;
#if defined(FEAT_OPEN_APPID)
        case RULE_OPTION_TYPE_APPID:
            hash = optionAppIdHash(key->option_data);
            break;
#endif /* defined(FEAT_OPEN_APPID) */
    }

    return hash;
}

int detection_option_key_compare_func(const void *k1, const void *k2, size_t n)
{
    int ret = DETECTION_OPTION_NOT_EQUAL;
    const detection_option_key_t *key1 = (detection_option_key_t*)k1;
    const detection_option_key_t *key2 = (detection_option_key_t*)k2;

#ifdef KEEP_THEM_ALLOCATED
    return DETECTION_OPTION_NOT_EQUAL;
#endif

    if (!key1 || !key2)
        return DETECTION_OPTION_NOT_EQUAL;

    if (key1->option_type != key2->option_type)
        return DETECTION_OPTION_NOT_EQUAL;

    switch (key1->option_type)
    {
        /* Call compare function specific to the key type */
        case RULE_OPTION_TYPE_LEAF_NODE:
            /* Leaf node always not equal. */
            break;
        case RULE_OPTION_TYPE_ASN1:
            ret = Asn1Compare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_BYTE_TEST:
            ret = ByteTestCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_BYTE_JUMP:
            ret = ByteJumpCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_BYTE_EXTRACT:
            ret = ByteExtractCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_BYTE_MATH:
            ret = ByteMathCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_FLOW:
            ret = FlowCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_CVS:
            ret = CvsCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_DSIZE:
            ret = DSizeCheckCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_FLOWBIT:
            ret = FlowBitsCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_FTPBOUNCE:
            break;
        case RULE_OPTION_TYPE_FILE_DATA:
            ret = FileDataCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_BASE64_DECODE:
            ret = Base64DecodeCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_BASE64_DATA:
            break;
        case RULE_OPTION_TYPE_PKT_DATA:
            break;
        case RULE_OPTION_TYPE_ICMP_CODE:
            ret = IcmpCodeCheckCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_ICMP_ID:
            ret = IcmpIdCheckCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_ICMP_SEQ:
            ret = IcmpSeqCheckCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_ICMP_TYPE:
            ret = IcmpTypeCheckCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_IP_FRAGBITS:
            ret = IpFragBitsCheckCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_IP_FRAG_OFFSET:
            ret = IpFragOffsetCheckCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_IP_ID:
            ret = IpIdCheckCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_IP_OPTION:
            ret = IpOptionCheckCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_IP_PROTO:
            ret = IpProtoCheckCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_IP_SAME:
            ret = IpSameCheckCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_IP_TOS:
            ret = IpTosCheckCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_IS_DATA_AT:
            ret = IsDataAtCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_CONTENT:
        case RULE_OPTION_TYPE_CONTENT_URI:
            ret = PatternMatchCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_PCRE:
            ret = PcreCompare(key1->option_data, key2->option_data);
            break;
#ifdef ENABLE_REACT
        case RULE_OPTION_TYPE_REACT:
            ret = ReactCompare(key1->option_data, key2->option_data);
            break;
#endif
#ifdef ENABLE_RESPOND
        case RULE_OPTION_TYPE_RESPOND:
            ret = RespondCompare(key1->option_data, key2->option_data);
            break;
#endif
        case RULE_OPTION_TYPE_RPC_CHECK:
            ret = RpcCheckCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_SESSION:
            ret = SessionCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_TCP_ACK:
            ret = TcpAckCheckCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_TCP_FLAG:
            ret = TcpFlagCheckCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_TCP_SEQ:
            ret = TcpSeqCheckCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_TCP_WIN:
            ret = TcpWinCheckCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_TTL:
            ret = TtlCheckCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_URILEN:
            ret = UriLenCheckCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_HDR_OPT_CHECK:
            ret = HdrOptCheckCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_FILE_TYPE:
            ret = FileTypeCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_PREPROCESSOR:
            ret = PreprocessorRuleOptionCompare(key1->option_data, key2->option_data);
            break;
        case RULE_OPTION_TYPE_DYNAMIC:
            ret = DynamicRuleCompare(key1->option_data, key2->option_data);
            break;
#if defined(FEAT_OPEN_APPID)
        case RULE_OPTION_TYPE_APPID:
            ret = optionAppIdCompare(key1->option_data, key2->option_data);
            break;
#endif /* defined(FEAT_OPEN_APPID) */
    }

    return ret;
}

int detection_hash_free_func(void *option_key, void *data)
{
    detection_option_key_t *key = (detection_option_key_t*)option_key;

    switch (key->option_type)
    {
        /* Call free function specific to the key type */
        case RULE_OPTION_TYPE_ASN1:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_BYTE_TEST:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_BYTE_JUMP:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_BYTE_EXTRACT:
            ByteExtractFree(key->option_data);
            break;
        case RULE_OPTION_TYPE_BYTE_MATH:
            ByteMathFree(key->option_data);
            break;
        case RULE_OPTION_TYPE_FLOW:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_CVS:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_DSIZE:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_FLOWBIT:
            FlowBitsFree(key->option_data);
            break;
        case RULE_OPTION_TYPE_FTPBOUNCE:
            /* Data is NULL, nothing to free */
            break;
        case RULE_OPTION_TYPE_FILE_DATA:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_BASE64_DECODE:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_BASE64_DATA:
            break;
        case RULE_OPTION_TYPE_PKT_DATA:
            break;
        case RULE_OPTION_TYPE_ICMP_CODE:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_ICMP_ID:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_ICMP_SEQ:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_ICMP_TYPE:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_IP_FRAGBITS:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_IP_FRAG_OFFSET:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_IP_ID:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_IP_OPTION:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_IP_PROTO:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_IP_SAME:
            /* Data is NULL, nothing to free */
            break;
        case RULE_OPTION_TYPE_IP_TOS:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_IS_DATA_AT:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_CONTENT:
        case RULE_OPTION_TYPE_CONTENT_URI:
            PatternMatchFree(key->option_data);
            break;
        case RULE_OPTION_TYPE_PCRE:
            PcreFree(key->option_data);
            break;
#ifdef ENABLE_REACT
        case RULE_OPTION_TYPE_REACT:
            ReactFree(key->option_data);
            break;
#endif
#ifdef ENABLE_RESPOND
        case RULE_OPTION_TYPE_RESPOND:
            free(key->option_data);
            break;
#endif
        case RULE_OPTION_TYPE_RPC_CHECK:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_SESSION:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_TCP_ACK:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_TCP_FLAG:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_TCP_SEQ:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_TCP_WIN:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_TTL:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_URILEN:
            free(key->option_data);
            break;
        case RULE_OPTION_TYPE_HDR_OPT_CHECK:
            break;
        case RULE_OPTION_TYPE_FILE_TYPE:
            FileTypeFree(key->option_data);
            break;
        case RULE_OPTION_TYPE_PREPROCESSOR:
            PreprocessorRuleOptionsFreeFunc(key->option_data);
            break;
        case RULE_OPTION_TYPE_DYNAMIC:
            fpDynamicDataFree(key->option_data);
            break;
        case RULE_OPTION_TYPE_LEAF_NODE:
            break;
#if defined(FEAT_OPEN_APPID)
        case RULE_OPTION_TYPE_APPID:
            optionAppIdFree(key->option_data);
            break;
#endif /* defined(FEAT_OPEN_APPID) */
    }
    return 0;
}

SFXHASH * DetectionHashTableNew(void)
{
    SFXHASH *doht = sfxhash_new(HASH_RULE_OPTIONS,
                                sizeof(detection_option_key_t),
                                0,      /* Data size == 0, just store the ptr */
                                0,      /* Memcap */
                                0,      /* Auto node recovery */
                                NULL,   /* Auto free function */
                                detection_hash_free_func,   /* User free function */
                                1);     /* Recycle nodes */


    if (doht == NULL)
        FatalError("Failed to create rule detection option hash table");

    sfxhash_set_keyops(doht, detection_option_hash_func,
                       detection_option_key_compare_func);

    return doht;
}

void DetectionHashTableFree(SFXHASH *doht)
{
    if (doht != NULL)
        sfxhash_delete(doht);
}

int add_detection_option(struct _SnortConfig *sc, option_type_t type, void *option_data, void **existing_data)
{
    detection_option_key_t key;

    if (sc == NULL)
    {
        FatalError("%s(%d) Snort config is NULL.\n",
                   __FILE__, __LINE__);
    }

    if (sc->detection_option_hash_table == NULL)
        sc->detection_option_hash_table = DetectionHashTableNew();

    if (!option_data)
    {
        /* No option data, no conflict to resolve. */
        return DETECTION_OPTION_EQUAL;
    }

    key.option_type = type;
    key.option_data = option_data;

    *existing_data = sfxhash_find(sc->detection_option_hash_table, &key);
    if (*existing_data)
    {
        return DETECTION_OPTION_EQUAL;
    }

    sfxhash_add(sc->detection_option_hash_table, &key, option_data);
    return DETECTION_OPTION_NOT_EQUAL;
}

uint32_t detection_option_tree_hash(detection_option_tree_node_t *node)
{
    uint32_t a,b,c;
    int i;

    if (!node)
        return 0;

    a = b = c = 0;

    for (i=0;i<node->num_children;i++)
    {
#if (defined(__ia64) || defined(__amd64) || defined(_LP64))
        {
            /* Cleanup warning because of cast from 64bit ptr to 32bit int
             * warning on 64bit OSs */
            uint64_t ptr; /* Addresses are 64bits */
            ptr = (uint64_t)node->children[i]->option_data;
            a += (ptr >> 32);
            b += (ptr & 0xFFFFFFFF);
        }
#else
        a += (uint32_t)node->children[i]->option_data;
        b += 0;
#endif
        c += detection_option_tree_hash(node->children[i]);
        mix(a,b,c);
        a += node->children[i]->num_children;
        mix(a,b,c);
#if 0
        a += (uint32_t)node->children[i]->option_data;
        /* Recurse & hash up this guy's children */
        b += detection_option_tree_hash(node->children[i]);
        c += node->children[i]->num_children;
        mix(a,b,c);
#endif
    }

    final(a,b,c);

    return c;
}

uint32_t detection_option_tree_hash_func(SFHASHFCN *p, unsigned char *k, int n)
{
    detection_option_key_t *key = (detection_option_key_t *)k;
    detection_option_tree_node_t *node;

    if (!key || !key->option_data)
        return 0;

    node = (detection_option_tree_node_t*)key->option_data;

    return detection_option_tree_hash(node);
}

int detection_option_tree_compare(detection_option_tree_node_t *r, detection_option_tree_node_t *l)
{
    int ret = DETECTION_OPTION_NOT_EQUAL;
    int i;

    if ((r == NULL) && (l == NULL))
        return DETECTION_OPTION_EQUAL;

    if ((!r && l) || (r && !l))
        return DETECTION_OPTION_NOT_EQUAL;

    if (r->option_data != l->option_data)
        return DETECTION_OPTION_NOT_EQUAL;

    if (r->num_children != l->num_children)
        return DETECTION_OPTION_NOT_EQUAL;

    for (i=0;i<r->num_children;i++)
    {
        /* Recurse & check the children for equality */
        ret = detection_option_tree_compare(r->children[i], l->children[i]);
        if (ret != DETECTION_OPTION_EQUAL)
            return ret;
    }

    return DETECTION_OPTION_EQUAL;
}

int detection_option_tree_compare_func(const void *k1, const void *k2, size_t n)
{
    detection_option_key_t *key_r = (detection_option_key_t *)k1;
    detection_option_key_t *key_l = (detection_option_key_t *)k2;
    detection_option_tree_node_t *r;
    detection_option_tree_node_t *l;

    if (!key_r || !key_l)
        return DETECTION_OPTION_NOT_EQUAL;

    r = (detection_option_tree_node_t *)key_r->option_data;
    l = (detection_option_tree_node_t *)key_l->option_data;

    return detection_option_tree_compare(r, l);
}

int detection_option_tree_free_func(void *option_key, void *data)
{
    detection_option_tree_node_t *node = (detection_option_tree_node_t *)data;
    /* In fpcreate.c */
    free_detection_option_tree(node);
    return 0;
}

void DetectionTreeHashTableFree(SFXHASH *dtht)
{
    if (dtht != NULL)
        sfxhash_delete(dtht);
}

SFXHASH * DetectionTreeHashTableNew(void)
{
    SFXHASH *dtht = sfxhash_new(HASH_RULE_TREE,
                                sizeof(detection_option_key_t),
                                0,      /* Data size == 0, just store the ptr */
                                0,      /* Memcap */
                                0,      /* Auto node recovery */
                                NULL,   /* Auto free function */
                                detection_option_tree_free_func,   /* User free function */
                                1);     /* Recycle nodes */


    if (dtht == NULL)
        FatalError("Failed to create rule detection option hash table");

    sfxhash_set_keyops(dtht, detection_option_tree_hash_func,
                       detection_option_tree_compare_func);

    return dtht;
}

char *option_type_str[] =
{
    "RULE_OPTION_TYPE_LEAF_NODE",
    "RULE_OPTION_TYPE_ASN1",
    "RULE_OPTION_TYPE_BYTE_TEST",
    "RULE_OPTION_TYPE_BYTE_JUMP",
    "RULE_OPTION_TYPE_BYTE_EXTRACT",
    "RULE_OPTION_TYPE_FLOW",
    "RULE_OPTION_TYPE_CVS",
    "RULE_OPTION_TYPE_DSIZE",
    "RULE_OPTION_TYPE_FLOWBIT",
    "RULE_OPTION_TYPE_FTPBOUNCE",
    "RULE_OPTION_TYPE_ICMP_CODE",
    "RULE_OPTION_TYPE_ICMP_ID",
    "RULE_OPTION_TYPE_ICMP_SEQ",
    "RULE_OPTION_TYPE_ICMP_TYPE",
    "RULE_OPTION_TYPE_IP_FRAGBITS",
    "RULE_OPTION_TYPE_IP_FRAG_OFFSET",
    "RULE_OPTION_TYPE_IP_ID",
    "RULE_OPTION_TYPE_IP_OPTION",
    "RULE_OPTION_TYPE_IP_PROTO",
    "RULE_OPTION_TYPE_IP_SAME",
    "RULE_OPTION_TYPE_IP_TOS",
    "RULE_OPTION_TYPE_IS_DATA_AT",
    "RULE_OPTION_TYPE_FILE_DATA",
    "RULE_OPTION_TYPE_FILE_TYPE",
    "RULE_OPTION_TYPE_BASE64_DECODE",
    "RULE_OPTION_TYPE_BASE64_DATA",
    "RULE_OPTION_TYPE_PKT_DATA",
    "RULE_OPTION_TYPE_CONTENT",
    "RULE_OPTION_TYPE_CONTENT_URI",
    "RULE_OPTION_TYPE_PCRE",
#ifdef ENABLE_REACT
    "RULE_OPTION_TYPE_REACT",
#endif
#ifdef ENABLE_RESPOND
    "RULE_OPTION_TYPE_RESPOND",
#endif
    "RULE_OPTION_TYPE_RPC_CHECK",
    "RULE_OPTION_TYPE_SESSION",
    "RULE_OPTION_TYPE_TCP_ACK",
    "RULE_OPTION_TYPE_TCP_FLAG",
    "RULE_OPTION_TYPE_TCP_SEQ",
    "RULE_OPTION_TYPE_TCP_WIN",
    "RULE_OPTION_TYPE_TTL",
    "RULE_OPTION_TYPE_URILEN",
    "RULE_OPTION_TYPE_HDR_OPT_CHECK",
    "RULE_OPTION_TYPE_PREPROCESSOR",
    "RULE_OPTION_TYPE_DYNAMIC"
#if defined(FEAT_OPEN_APPID)
    ,"RULE_OPTION_TYPE_APPID"
#endif /* defined(FEAT_OPEN_APPID) */
    ,"RULE_OPTION_TYPE_BYTE_MATH"
};

#ifdef DEBUG_OPTION_TREE
void print_option_tree(detection_option_tree_node_t *node, int level)
{
    int i;
    unsigned int indent = 12 - (11 - level) + strlen(option_type_str[node->option_type]);
    unsigned int offset = 0;
    if (level >= 10)
        offset++;

    DEBUG_WRAP(
        DebugMessage(DEBUG_DETECT, "%d%*s%*d 0x%x\n",
           level, indent - offset, option_type_str[node->option_type],
           54 - indent, node->num_children,
           node->option_data);
        for (i=0;i<node->num_children;i++)
            print_option_tree(node->children[i], level+1);
    );
}
#endif

int add_detection_option_tree(SnortConfig *sc, detection_option_tree_node_t *option_tree, void **existing_data)
{
    detection_option_key_t key;

    if (sc == NULL)
    {
        FatalError("%s(%d) Snort config for parsing is NULL.\n",
                   __FILE__, __LINE__);
    }

    if (sc->detection_option_tree_hash_table == NULL)
        sc->detection_option_tree_hash_table = DetectionTreeHashTableNew();

    if (!option_tree)
    {
        /* No option data, no conflict to resolve. */
        return DETECTION_OPTION_EQUAL;
    }

    key.option_data = (void *)option_tree;
    key.option_type = RULE_OPTION_TYPE_LEAF_NODE;

    *existing_data = sfxhash_find(sc->detection_option_tree_hash_table, &key);
    if (*existing_data)
    {
        return DETECTION_OPTION_EQUAL;
    }

    sfxhash_add(sc->detection_option_tree_hash_table, &key, option_tree);
    return DETECTION_OPTION_NOT_EQUAL;
}


uint64_t rule_eval_pkt_count = 0;

/* Include "detection_leaf_node.c"
 *
 * Service matches, toggles 'check_ports' and then evaluation
 * of the leaf nodes (ie. the rule header stuffs).
 *
 * This defines the routine "detection_leaf_node_eval($,$)" which
 * is called from the switch case RULE_OPTION_TYPE_LEAF_NODE below.
 */
#include "detection_leaf_node.c"

int detection_option_node_evaluate(detection_option_tree_node_t *node, detection_option_eval_data_t *eval_data)
{
    int i, result = 0, prior_result = 0;
    int rval = DETECTION_OPTION_NO_MATCH;
    const uint8_t *orig_doe_ptr;
    char tmp_noalert_flag = 0;
    PatternMatchData dup_content_option_data;
    PcreData dup_pcre_option_data;
    const uint8_t *dp = NULL;
    char continue_loop = 1;
    char flowbits_setoperation = 0;
    int loop_count = 0;
    uint32_t tmp_byte_extract_vars[NUM_BYTE_EXTRACT_VARS];
    uint16_t save_dflags = 0;
    uint64_t cur_eval_pkt_count = (rule_eval_pkt_count + (GetRebuiltPktCount()));
    NODE_PROFILE_VARS;

    if (!node || !eval_data || !eval_data->p || !eval_data->pomd)
        return 0;

    save_dflags = Get_DetectFlags();

    /* see if evaluated it before ... */
    if (node->last_check.is_relative == 0)
    {
        /* Only matters if not relative... */
        if ((node->last_check.ts.tv_usec == eval_data->p->pkth->ts.tv_usec) &&
            (node->last_check.ts.tv_sec == eval_data->p->pkth->ts.tv_sec) &&
            (node->last_check.packet_number == cur_eval_pkt_count) &&
            (node->last_check.rebuild_flag == (eval_data->p->packet_flags & PKT_REBUILT_STREAM)) &&
            (!(eval_data->p->packet_flags & PKT_ALLOW_MULTIPLE_DETECT)))
        {
            /* eval'd this rule option before on this packet,
             * use the cached result. */
            if ((node->last_check.flowbit_failed == 0) &&
                !(eval_data->p->packet_flags & PKT_IP_RULE_2ND) &&
                !(eval_data->p->proto_bits & (PROTO_BIT__TEREDO | PROTO_BIT__GTP )))
            {
                return node->last_check.result;
            }
        }
    }

    NODE_PROFILE_START(node);

    node->last_check.ts.tv_sec = eval_data->p->pkth->ts.tv_sec;
    node->last_check.ts.tv_usec = eval_data->p->pkth->ts.tv_usec;
    node->last_check.packet_number = cur_eval_pkt_count;
    node->last_check.rebuild_flag = (eval_data->p->packet_flags & PKT_REBUILT_STREAM);
    node->last_check.flowbit_failed = 0;

    /* Save some stuff off for repeated pattern tests */
    orig_doe_ptr = doe_ptr;

    if ((node->option_type == RULE_OPTION_TYPE_CONTENT) ||
            (node->option_type == RULE_OPTION_TYPE_CONTENT_URI))
    {
        PatternMatchDuplicatePmd(node->option_data, &dup_content_option_data);

        if (dup_content_option_data.buffer_func == CHECK_URI_PATTERN_MATCH)
        {
            const HttpBuffer* hb = GetHttpBuffer(dup_content_option_data.http_buffer);
            dp = hb ? hb->buf : NULL;  // FIXTHIS set length too
        }
        else if (dup_content_option_data.rawbytes == 0)
        {
            /* If AltDetect is set by calling the rule options which set it,
             * we should use the Alt Detect before checking for any other buffers.
             * Alt Detect will take precedence over the Alt Decode and/or packet data.
             */
            if(Is_DetectFlag(FLAG_ALT_DETECT))
                dp = DetectBuffer.data;
            else if(Is_DetectFlag(FLAG_ALT_DECODE))
                dp = (uint8_t *)DecodeBuffer.data;
            else
                dp = eval_data->p->data;
        }
        else
        {
            dp = eval_data->p->data;
        }
    }
    else if (node->option_type == RULE_OPTION_TYPE_PCRE)
    {
        unsigned hb_type;
        PcreDuplicatePcreData(node->option_data, &dup_pcre_option_data);
        hb_type = dup_pcre_option_data.options & SNORT_PCRE_HTTP_BUFS;

        if ( hb_type )
        {
            const HttpBuffer* hb = GetHttpBuffer(hb_type);
            dp = hb ? hb->buf : NULL;  // FIXTHIS set length too
        }
        else if (!(dup_pcre_option_data.options & SNORT_PCRE_RAWBYTES))
        {
            /* If AltDetect is set by calling the rule options which set it,
             * we should use the Alt Detect before checking for any other buffers.
             * Alt Detect will take precedence over the Alt Decode and/or packet data.
             */
            if(Is_DetectFlag(FLAG_ALT_DETECT))
                dp = DetectBuffer.data;
            else if(Is_DetectFlag(FLAG_ALT_DECODE))
                dp = (uint8_t *)DecodeBuffer.data;
            else
                dp = eval_data->p->data;
        }
        else
        {
            dp = eval_data->p->data;
        }
    }

    /* No, haven't evaluated this one before... Check it. */
    do
    {
        switch (node->option_type)
        {
            case RULE_OPTION_TYPE_LEAF_NODE:
                /* Add the match for this otn to the queue. */
                {
                    int pattern_size    = 0;
                    int eval_rtn_result = 1;
                    int check_ports     = 1;
                    OptTreeNode *otn = (OptTreeNode*) node->option_data;
                    PatternMatchData *pmd = (PatternMatchData*) eval_data->pmd;

                    if (pmd) 
                        pattern_size = pmd->pattern_size;

                    // See "detection_leaf_node.c" (detection_leaf_node_eval).
#ifdef TARGET_BASED
                    switch (detection_leaf_node_eval (node, eval_data))
                    {
                        case Leaf_Abort:
                            eval_rtn_result = 0;
                            break;

                        case Leaf_SkipPorts:
                            check_ports = 0;
                            // fall through

                        case Leaf_CheckPorts:
                            NODE_PROFILE_TMPEND(node);
                            eval_rtn_result = fpEvalRTN (getRuntimeRtnFromOtn (otn), eval_data->p, check_ports);
                            NODE_PROFILE_TMPSTART(node);
                            break;
                    }
#endif

                    if (eval_rtn_result)
                    {
			    if ((!otn->detection_filter) ||
                                 !detection_filter_test(
                                 otn->detection_filter,
                                 GET_SRC_IP(eval_data->p), GET_DST_IP(eval_data->p),
                                 eval_data->p->pkth->ts.tv_sec, eval_data))
                        {
#ifdef PERF_PROFILING
                            if (PROFILING_RULES)
                                otn->matches++;
#endif
                            if (!eval_data->flowbit_noalert)
                            {
                                fpAddMatch(eval_data->pomd, pattern_size, otn);
                            }
                            result = rval = DETECTION_OPTION_MATCH;
                        }
                    }
                }
                break;

            case RULE_OPTION_TYPE_CONTENT:
                if (node->evaluate)
                {
                    /* This will be set in the fast pattern matcher if we found
                     * a content and the rule option specifies not that
                     * content. Essentially we've already evaluated this rule
                     * option via the content option processing since only not
                     * contents that are not relative in any way will have this
                     * flag set */
                    if (dup_content_option_data.exception_flag)
                    {
                        if ((dup_content_option_data.last_check.ts.tv_sec == eval_data->p->pkth->ts.tv_sec) &&
                            (dup_content_option_data.last_check.ts.tv_usec == eval_data->p->pkth->ts.tv_usec) &&
                            (dup_content_option_data.last_check.packet_number == cur_eval_pkt_count) &&
                            (dup_content_option_data.last_check.rebuild_flag == (eval_data->p->packet_flags & PKT_REBUILT_STREAM)))
                        {
                            rval = DETECTION_OPTION_NO_MATCH;
                            break;
                        }
                    }

                    rval = node->evaluate(&dup_content_option_data, eval_data->p);
                }
                break;
            case RULE_OPTION_TYPE_CONTENT_URI:
                if (node->evaluate)
                {
                    rval = node->evaluate(&dup_content_option_data, eval_data->p);
                }
                break;
            case RULE_OPTION_TYPE_PCRE:
                if (node->evaluate)
                {
                    rval = node->evaluate(&dup_pcre_option_data, eval_data->p);
                }
                break;
            case RULE_OPTION_TYPE_PKT_DATA:
            case RULE_OPTION_TYPE_FILE_DATA:
            case RULE_OPTION_TYPE_BASE64_DATA:
                if (node->evaluate)
                {
                    save_dflags = Get_DetectFlags();
                    rval = node->evaluate(node->option_data, eval_data->p);
                }
                break;
            case RULE_OPTION_TYPE_FLOWBIT:
                if (node->evaluate)
                {
                    flowbits_setoperation = FlowBits_SetOperation(node->option_data);
                    if (!flowbits_setoperation)
                    {
                        rval = node->evaluate(node->option_data, eval_data->p);
                    }
                    else
                    {
                        /* set to match so we don't bail early.  */
                        rval = DETECTION_OPTION_MATCH;
                    }
                }
                break;
            case RULE_OPTION_TYPE_ASN1:
            case RULE_OPTION_TYPE_BYTE_TEST:
            case RULE_OPTION_TYPE_BYTE_JUMP:
            case RULE_OPTION_TYPE_BYTE_EXTRACT:
            case RULE_OPTION_TYPE_BYTE_MATH:
            case RULE_OPTION_TYPE_FLOW:
            case RULE_OPTION_TYPE_CVS:
            case RULE_OPTION_TYPE_DSIZE:
            case RULE_OPTION_TYPE_FTPBOUNCE:
            case RULE_OPTION_TYPE_BASE64_DECODE:
            case RULE_OPTION_TYPE_ICMP_CODE:
            case RULE_OPTION_TYPE_ICMP_ID:
            case RULE_OPTION_TYPE_ICMP_SEQ:
            case RULE_OPTION_TYPE_ICMP_TYPE:
            case RULE_OPTION_TYPE_IP_FRAGBITS:
            case RULE_OPTION_TYPE_IP_FRAG_OFFSET:
            case RULE_OPTION_TYPE_IP_ID:
            case RULE_OPTION_TYPE_IP_OPTION:
            case RULE_OPTION_TYPE_IP_PROTO:
            case RULE_OPTION_TYPE_IP_SAME:
            case RULE_OPTION_TYPE_IP_TOS:
            case RULE_OPTION_TYPE_IS_DATA_AT:
#ifdef ENABLE_REACT
            case RULE_OPTION_TYPE_REACT:
#endif
#ifdef ENABLE_RESPOND
            case RULE_OPTION_TYPE_RESPOND:
#endif
            case RULE_OPTION_TYPE_RPC_CHECK:
            case RULE_OPTION_TYPE_SESSION:
            case RULE_OPTION_TYPE_TCP_ACK:
            case RULE_OPTION_TYPE_TCP_FLAG:
            case RULE_OPTION_TYPE_TCP_SEQ:
            case RULE_OPTION_TYPE_TCP_WIN:
            case RULE_OPTION_TYPE_TTL:
            case RULE_OPTION_TYPE_URILEN:
            case RULE_OPTION_TYPE_HDR_OPT_CHECK:
            case RULE_OPTION_TYPE_FILE_TYPE:
            case RULE_OPTION_TYPE_PREPROCESSOR:
                if (node->evaluate)
                    rval = node->evaluate(node->option_data, eval_data->p);
                break;
            case RULE_OPTION_TYPE_DYNAMIC:
                if (node->evaluate)
                    rval = node->evaluate(node->option_data, eval_data->p);
                break;
#if defined(FEAT_OPEN_APPID)
            case RULE_OPTION_TYPE_APPID:
                if (node->evaluate)
                    rval = node->evaluate(node->option_data, eval_data->p);
                break;
#endif /* defined(FEAT_OPEN_APPID) */
        }

        if (rval == DETECTION_OPTION_NO_MATCH)
        {
            node->last_check.result = result;
            NODE_PROFILE_END_NOMATCH(node);
            return result;
        }
        else if (rval == DETECTION_OPTION_FAILED_BIT)
        {
            eval_data->flowbit_failed = 1;
            /* clear the timestamp so failed flowbit gets eval'd again */
            node->last_check.flowbit_failed = 1;
            node->last_check.result = result;
            NODE_PROFILE_END_NOMATCH(node);
            return 0;
        }
        else if (rval == DETECTION_OPTION_NO_ALERT)
        {
            /* Cache the current flowbit_noalert flag, and set it
             * so nodes below this don't alert. */
            tmp_noalert_flag = eval_data->flowbit_noalert;
            eval_data->flowbit_noalert = 1;
        }

        /* Back up byte_extract vars so they don't get overwritten between rules */
        for (i = 0; i < NUM_BYTE_EXTRACT_VARS; i++)
        {
            GetByteExtractValue(&(tmp_byte_extract_vars[i]), (int8_t)i);
        }

#ifdef PPM_MGR
        if( PPM_PKTS_ENABLED() )
        {
            PPM_GET_TIME();
            PPM_PACKET_TEST();
            if( PPM_PACKET_ABORT_FLAG() )
            {
                /* bail if we exceeded time */
                if (result == DETECTION_OPTION_NO_MATCH)
                {
                    NODE_PROFILE_END_NOMATCH(node);
                }
                else
                {
                    NODE_PROFILE_END_MATCH(node);
                }
                node->last_check.result = result;
                Reset_DetectFlags(save_dflags);
                return result;
            }
        }
#endif
        /* Don't include children's time in this node */
        NODE_PROFILE_TMPEND(node);

        /* Passed, check the children. */
        if (node->num_children)
        {
            const uint8_t *tmp_doe_ptr = doe_ptr;
            const uint8_t tmp_doe_flags = doe_buf_flags;

            for (i=0;i<node->num_children; i++)
            {
                int j = 0;
                detection_option_tree_node_t *child_node = node->children[i];

                /* reset the DOE ptr for each child from here */
                SetDoePtr(tmp_doe_ptr, tmp_doe_flags);

                for (j = 0; j < NUM_BYTE_EXTRACT_VARS; j++)
                {
                    SetByteExtractValue(tmp_byte_extract_vars[j], (int8_t)j);
                }

                if (loop_count > 0)
                {
                    if (child_node->result == DETECTION_OPTION_NO_MATCH)
                    {
                        if (((child_node->option_type == RULE_OPTION_TYPE_CONTENT)
                                    || (child_node->option_type == RULE_OPTION_TYPE_PCRE))
                                && !child_node->last_check.is_relative)
                        {
                            /* If it's a non-relative content or pcre, no reason
                             * to check again.  Only increment result once.
                             * Should hit this condition on first loop iteration. */
                            if (loop_count == 1)
                                result++;
                            continue;
                        }
                        else if ((child_node->option_type == RULE_OPTION_TYPE_CONTENT)
                                && child_node->last_check.is_relative)
                        {
                            PatternMatchData *pmd = (PatternMatchData *)child_node->option_data;

                            /* Check for an unbounded relative search.  If this
                             * failed before, it's going to fail again so don't
                             * go down this path again 
                             * Check for protected pattern because in this case 
                             * we had checked for 'n'bytes only where 'n' is the 
                             * length of protected pattern.
                             * */
                            if (pmd->within == PMD_WITHIN_UNDEFINED && !pmd->protected_pattern)
                            {
                                /* Only increment result once. Should hit this
                                 * condition on first loop iteration. */
                                if (loop_count == 1)
                                    result++;
                                continue;
                            }
                        }
                    }
                    else if (child_node->option_type == RULE_OPTION_TYPE_LEAF_NODE)
                    {
                        /* Leaf node matched, don't eval again */
                        continue;
                    }
                    else if (child_node->result == child_node->num_children)
                    {
                        /* This branch of the tree matched or has options that
                         * don't need to be evaluated again, so don't need to
                         * evaluate this option again */
                        continue;
                    }
                }

                child_node->result = detection_option_node_evaluate(node->children[i], eval_data);
                if (child_node->option_type == RULE_OPTION_TYPE_LEAF_NODE)
                {
                    /* Leaf node won't have any children but will return success
                     * or failure */
                    result += child_node->result;
                }
                else if (child_node->result == child_node->num_children)
                {
                    /* Indicate that the child's tree branches are done */
                    result++;
                }
#ifdef PPM_MGR
                if( PPM_PKTS_ENABLED() )
                {
                    PPM_GET_TIME();
                    PPM_PACKET_TEST();
                    if( PPM_PACKET_ABORT_FLAG() )
                    {
                        /* bail if we exceeded time */
                        node->last_check.result = result;
                        Reset_DetectFlags(save_dflags);
                        return result;
                    }
                }
#endif
            }

            /* If all children branches matched, we don't need to reeval any of
             * the children so don't need to reeval this content/pcre rule
             * option at a new offset.
             * Else, reset the DOE ptr to last eval for offset/depth,
             * distance/within adjustments for this same content/pcre
             * rule option */
            if (result == node->num_children)
                continue_loop = 0;
            else
                SetDoePtr(tmp_doe_ptr, tmp_doe_flags);

            /* Don't need to reset since it's only checked after we've gone
             * through the loop at least once and the result will have
             * been set again already */
            //for (i = 0; i < node->num_children; i++)
            //    node->children[i]->result;
        }

        if (result - prior_result > 0
            && node->option_type == RULE_OPTION_TYPE_CONTENT
            && Replace_OffsetStored(&dup_content_option_data) && ScIpsInlineMode())
        {
          if(!ScDisableReplaceOpt())
          {
            Replace_QueueChange(&dup_content_option_data);
            prior_result = result;
          }
        }

        NODE_PROFILE_TMPSTART(node);

        if (rval == DETECTION_OPTION_NO_ALERT)
        {
            /* Reset the flowbit_noalert flag in eval data */
            eval_data->flowbit_noalert = tmp_noalert_flag;
        }

        if (continue_loop && (rval == DETECTION_OPTION_MATCH) && (node->relative_children))
        {
            if ((node->option_type == RULE_OPTION_TYPE_CONTENT) ||
                    (node->option_type == RULE_OPTION_TYPE_CONTENT_URI))
            {
                if (dup_content_option_data.exception_flag)
                {
                    continue_loop = 0;
                }
                else
                {
                    const uint8_t *orig_ptr;

                    if (dup_content_option_data.use_doe)
                        orig_ptr = (orig_doe_ptr == NULL) ? dp : orig_doe_ptr;
                    else
                        orig_ptr = dp;

                    continue_loop = PatternMatchAdjustRelativeOffsets((PatternMatchData *)node->option_data,
                            &dup_content_option_data, doe_ptr, orig_ptr);
                }
            }
            else if (node->option_type == RULE_OPTION_TYPE_PCRE)
            {
                if (dup_pcre_option_data.options & SNORT_PCRE_INVERT)
                {
                    continue_loop = 0;
                }
                else
                {
                    const uint8_t *orig_ptr;

                    if (dup_pcre_option_data.options & SNORT_PCRE_RELATIVE)
                        orig_ptr = (orig_doe_ptr == NULL) ? dp : orig_doe_ptr;
                    else
                        orig_ptr = dp;

                    continue_loop = PcreAdjustRelativeOffsets(&dup_pcre_option_data, doe_ptr - orig_ptr);
                }
            }
            else
            {
                continue_loop = 0;
            }
        }
        else
        {
            continue_loop = 0;
        }

#ifdef PERF_PROFILING
        /* We're essentially checking this node again and it potentially
         * might match again */
        if (continue_loop && PROFILING_RULES)
            node->checks++;
#endif

        loop_count++;

        if (continue_loop)
            UpdateDoePtr(orig_doe_ptr, 0);

    } while (continue_loop);

    if (flowbits_setoperation && (result == DETECTION_OPTION_MATCH))
    {
        /* Do any setting/clearing/resetting/toggling of flowbits here
         * given that other rule options matched. */
        rval = node->evaluate(node->option_data, eval_data->p);
        if (rval != DETECTION_OPTION_MATCH)
        {
            result = rval;
        }
    }

    if (eval_data->flowbit_failed)
    {
        /* something deeper in the tree failed a flowbit test, we may need to
         * reeval this node. */
        node->last_check.flowbit_failed = 1;
    }
    node->last_check.result = result;

    if (result == DETECTION_OPTION_NO_MATCH)
    {
        NODE_PROFILE_END_NOMATCH(node);
    }
    else
    {
        NODE_PROFILE_END_MATCH(node);
    }

    Reset_DetectFlags(save_dflags);
    return result;
}

#ifdef PERF_PROFILING
typedef struct node_profile_stats
{
    uint64_t ticks;
    uint64_t ticks_match;
    uint64_t ticks_no_match;
    uint64_t checks;
    uint64_t disables;
} node_profile_stats_t;

static void detection_option_node_update_otn_stats(detection_option_tree_node_t *node,
                                                   node_profile_stats_t *stats, uint64_t checks
#ifdef PPM_MGR
                                                   , uint64_t disables
#endif
                                                   )
{
    int i;
    node_profile_stats_t local_stats; /* cumulative stats for this node */

    if (stats)
    {
        local_stats.ticks = stats->ticks + node->ticks;
        local_stats.ticks_match = stats->ticks_match + node->ticks_match;
        local_stats.ticks_no_match = stats->ticks_no_match + node->ticks_no_match;
        if (node->checks > stats->checks)
            local_stats.checks = node->checks;
        else
            local_stats.checks = stats->checks;
#ifdef PPM_MGR
        local_stats.disables = disables;
#endif
    }
    else
    {
        local_stats.ticks = node->ticks;
        local_stats.ticks_match = node->ticks_match;
        local_stats.ticks_no_match = node->ticks_no_match;
        local_stats.checks = node->checks;
#ifdef PPM_MGR
        local_stats.disables = disables;
#endif
    }

    if (node->option_type == RULE_OPTION_TYPE_LEAF_NODE)
    {
        OptTreeNode *otn = (OptTreeNode *)node->option_data;
        /* Update stats for this otn */
        otn->ticks += local_stats.ticks;
        otn->ticks_match += local_stats.ticks_match;
        otn->ticks_no_match += local_stats.ticks_no_match;
        if (local_stats.checks > otn->checks)
            otn->checks = local_stats.checks;
#ifdef PPM_MGR
        otn->ppm_disable_cnt += local_stats.disables;
#endif
    }

    if (node->num_children)
    {
        for (i=0;i<node->num_children; i++)
        {
            detection_option_node_update_otn_stats(node->children[i], &local_stats, checks
#ifdef PPM_MGR
                , disables
#endif
                );
        }
    }
}

void detection_option_tree_update_otn_stats(SFXHASH *doth)
{
    SFXHASH_NODE *hashnode;

    if (doth == NULL)
        return;

    /* Find the first tree root in the table */
    hashnode = sfxhash_findfirst(doth);
    while (hashnode)
    {
        detection_option_tree_node_t *node = hashnode->data;
        if (node->checks)
        {
            detection_option_node_update_otn_stats(node, NULL, node->checks
#ifdef PPM_MGR
                                                   , node->ppm_disable_cnt
#endif
                                                  );
        }
        hashnode = sfxhash_findnext(doth);
    }
}
#endif
