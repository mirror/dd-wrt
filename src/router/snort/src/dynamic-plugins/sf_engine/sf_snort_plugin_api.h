/*
 * sf_snort_plugin.h
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
 * Sourcefire Black-box Plugin API for rules
 *
 */
#ifndef SF_SNORT_PLUGIN_API_H_
#define SF_SNORT_PLUGIN_API_H_

#include "pcre.h"
#include "stdio.h"

#ifndef WIN32
#include <netinet/in.h>
#include <sys/types.h>
#else
#include <winsock2.h>
#include <windows.h>
#endif

#include "sf_dynamic_define.h"
#include "sf_dynamic_engine.h"

#define ANY_NET         "any"
#define HOME_NET        "$HOME_NET"
#define EXTERNAL_NET    "$EXTERNAL_NET"
#define ANY_PORT        "any"
#define HTTP_SERVERS    "$HTTP_SERVERS"
#define HTTP_PORTS      "$HTTP_PORTS"
#define SMTP_SERVERS    "$SMTP_SERVERS"

#ifdef WIN32
# ifdef SF_SNORT_ENGINE_DLL
#  define ENGINE_LINKAGE SF_SO_PUBLIC
# else
#  define ENGINE_LINKAGE
# endif
#else /* WIN32 */
# define ENGINE_LINKAGE SF_SO_PUBLIC
#endif

#define RULE_NOMATCH 0
#define RULE_MATCH 1
#define RULE_NOALERT 2
#define RULE_FAILED_BIT 3

#define RULE_DIRECTIONAL 0
#define RULE_BIDIRECTIONAL 1

#define CONTENT_MATCH          1
#define CONTENT_NOMATCH        0
#define CONTENT_TYPE_MISMATCH -1
#define CONTENT_TYPE_MISSING  -2
#define CONTENT_CURSOR_ERROR  -3
#define CONTENT_HASH_ERROR    -4
#define CURSOR_IN_BOUNDS       1
#define CURSOR_OUT_OF_BOUNDS   0

//==========================================
// these are all part of the same mask:
//------------------------------------------
// low nibble must be same as HTTP_BUFFER_*
// see detection_util.h for enum
// TBD include BUF_* as well in a single enum?
#define CONTENT_BUF_NONE          0x00000000
#define CONTENT_BUF_URI           0x00000001
#define CONTENT_BUF_HEADER        0x00000002
#define CONTENT_BUF_POST          0x00000003

#define CONTENT_BUF_METHOD        0x00000004
#define CONTENT_BUF_COOKIE        0x00000005
#define CONTENT_BUF_STAT_CODE     0x00000006
#define CONTENT_BUF_STAT_MSG      0x00000007

#define CONTENT_BUF_RAW_URI       0x00000008
#define CONTENT_BUF_RAW_HEADER    0x00000009
#define CONTENT_BUF_RAW_COOKIE    0x0000000A
#define CONTENT_BUF_HTTP          0x0000000F
//------------------------------------------

#define BUF_FILE_DATA             0x00000010
#define BUF_FILE_DATA_MIME        0x00000020
#define BUF_BASE64_DECODE         0x00000040

#define CONTENT_BUF_NORMALIZED    0x00000100
#define CONTENT_BUF_RAW           0x00000200
#define CONTENT_END_BUFFER        0x00000400

#define CONTENT_NOCASE            0x00001000
#define CONTENT_RELATIVE          0x00002000
#define NOT_FLAG                  0x00004000

#define CONTENT_FAST_PATTERN      0x00010000
#define CONTENT_FAST_PATTERN_ONLY 0x00020000  // implies fast pattern
#define JUMP_FROM_BEGINNING       0x00040000
#define JUMP_ALIGN                0x00080000

#define CONTENT_UNICODE2BYTE      0x00100000
#define CONTENT_UNICODE4BYTE      0x00200000
#define BYTE_LITTLE_ENDIAN        0x00400000
#define BYTE_BIG_ENDIAN           0x00800000

#define EXTRACT_AS_DEC            0x01000000
#define EXTRACT_AS_OCT            0x02000000
#define EXTRACT_AS_HEX            0x04000000
#define EXTRACT_AS_BIN            0x08000000

#define EXTRACT_AS_BYTE           0x10000000
#define EXTRACT_AS_STRING         0x20000000

#define JUMP_FROM_END             0x40000000

#define PROTECTED_CONTENT_HASH_MD5    (1)
#define PROTECTED_CONTENT_HASH_SHA256 (2)
#define PROTECTED_CONTENT_HASH_SHA512 (3)
//==========================================

#define CHECK_EQ            0
#define CHECK_NEQ           1
#define CHECK_LT            2
#define CHECK_GT            3
#define CHECK_LTE           4
#define CHECK_GTE           5
#define CHECK_AND           6
#define CHECK_XOR           7
#define CHECK_ALL           8
#define CHECK_ATLEASTONE    9
#define CHECK_ADD           10
#define CHECK_SUB           11
#define CHECK_MUL           12
#define CHECK_DIV           13
#define CHECK_LS            14
#define CHECK_RS            15
#define CHECK_NONE          16

#define HTTP_CONTENT(cf) (cf & CONTENT_BUF_HTTP)

#define NORMAL_CONTENT_BUFS ( CONTENT_BUF_NORMALIZED | CONTENT_BUF_RAW )

static inline int IsHttpFastPattern (uint32_t cf)
{
    cf = HTTP_CONTENT(cf);

    return ( cf == CONTENT_BUF_URI || cf == CONTENT_BUF_HEADER ||
             cf == CONTENT_BUF_POST );
}

typedef struct _ContentInfo
{
    const uint8_t *pattern;
    uint32_t depth;
    int32_t   offset;
    uint32_t flags;        /* must include a CONTENT_BUF_X */
    void     *boyer_ptr;
    uint8_t *patternByteForm;
    uint32_t patternByteFormLength;
    uint32_t incrementLength;
    uint16_t fp_offset;
    uint16_t fp_length;
    uint8_t fp_only;
    char *offset_refId;     /* To match up with a DynamicElement refId */
    char *depth_refId;      /* To match up with a DynamicElement refId */
    int32_t *offset_location;
    uint32_t *depth_location;
} ContentInfo;
typedef struct _ProtectedContentInfo
{
    const uint8_t *pattern;
    uint32_t depth;
    int32_t offset;
    uint32_t flags;        /* must include a CONTENT_BUF_X */
    uint8_t hash_type;
    uint32_t protected_length;
    uint8_t *patternByteForm;
    uint32_t patternByteFormLength;
    char *offset_refId;     /* To match up with a DynamicElement refId */
    char *depth_refId;      /* To match up with a DynamicElement refId */
    int32_t *offset_location;
    uint32_t *depth_location;
} ProtectedContentInfo;

typedef struct _CursorInfo
{
    int32_t   offset;
    uint32_t flags;        /* specify one of CONTENT_BUF_X */
    char *offset_refId;     /* To match up with a DynamicElement refId */
    int32_t *offset_location;
} CursorInfo;

/*
pcre.h provides flags:

PCRE_CASELESS
PCRE_MULTILINE
PCRE_DOTALL
PCRE_EXTENDED
PCRE_ANCHORED
PCRE_DOLLAR_ENDONLY
PCRE_UNGREEDY
*/

typedef struct _PCREInfo
{
    char     *expr;
    void     *compiled_expr;
    void     *compiled_extra;
    uint32_t compile_flags;
    uint32_t flags; /* must include a CONTENT_BUF_X */
    int32_t   offset;
} PCREInfo;

#define FLOWBIT_SET       0x01
#define FLOWBIT_UNSET     0x02
#define FLOWBIT_TOGGLE    0x04
#define FLOWBIT_ISSET     0x08
#define FLOWBIT_ISNOTSET  0x10
#define FLOWBIT_RESET     0x20
#define FLOWBIT_NOALERT   0x40
#define FLOWBIT_SETX      0x80


typedef struct _FlowBitsInfo
{
    char     *flowBitsName;
    uint8_t   operation;
    uint16_t id;
    uint32_t flags;
    char     *groupName;
    uint8_t   eval;
    uint16_t *ids;
    uint8_t  num_ids;
} FlowBitsInfo;

typedef struct _ByteData
{
    uint32_t bytes;      /* Number of bytes to extract */
    uint32_t op;         /* Type of byte comparison, for checkValue */
    uint32_t value;      /* Value to compare value against, for checkValue, or extracted value */
    int32_t   offset;     /* Offset from cursor */
    uint32_t multiplier; /* Used for byte jump -- 32bits is MORE than enough */
    uint32_t flags;      /* must include a CONTENT_BUF_X */
    int32_t   post_offset;/* Use for byte jump -- adjust cusor by this much after the jump */
    char *offset_refId;     /* To match up with a DynamicElement refId */
    char *value_refId;      /* To match up with a DynamicElement refId */
    int32_t *offset_location;
    uint32_t *value_location;
    uint32_t bitmask_val;
    char *postoffset_refId;     /* To match up with a DynamicElement refId */
    char *refId;      /* To match up with a DynamicElement refId */
} ByteData;

typedef struct _ByteExtract
{
    uint32_t bytes;      /* Number of bytes to extract */
    int32_t   offset;     /* Offset from cursor */
    uint32_t multiplier; /* Multiply value by this (similar to byte jump) */
    uint32_t flags;      /* must include a CONTENT_BUF_X */
    char *refId;          /* To match up with a DynamicElement refId */
    void *memoryLocation; /* Location to store the data extracted */
    uint8_t  align;      /* Align to 2 or 4 bit boundary after extraction */
    uint32_t bitmask_val;
} ByteExtract;

typedef struct _FlowFlags
{
    uint32_t   flags;    /* FLOW_* values */
} FlowFlags;


#define ASN1_ABS_OFFSET 1
#define ASN1_REL_OFFSET 2

typedef struct _Asn1Context
{
    int bs_overflow;
    int double_overflow;
    int print;
    int length;
    unsigned int max_length;
    int offset;
    int offset_type;
    uint32_t  flags;
} Asn1Context;

#define IP_HDR_ID           0x0001  /* IP Header ID */
#define IP_HDR_PROTO        0x0002  /* IP Protocol */
#define IP_HDR_FRAGBITS     0x0003  /* Frag Flags set in IP Header */
#define IP_HDR_FRAGOFFSET   0x0004  /* Frag Offset set in IP Header */
#define IP_HDR_OPTIONS      0x0005  /* IP Options -- is option xx included */
#define IP_HDR_TTL          0x0006  /* IP Time to live */
#define IP_HDR_TOS          0x0007  /* IP Type of Service */
#define IP_HDR_OPTCHECK_MASK 0x000f

#define TCP_HDR_ACK         0x0010  /* TCP Ack Value */
#define TCP_HDR_SEQ         0x0020  /* TCP Seq Value */
#define TCP_HDR_FLAGS       0x0030  /* Flags set in TCP Header */
#define TCP_HDR_OPTIONS     0x0040  /* TCP Options -- is option xx included */
#define TCP_HDR_WIN         0x0050  /* TCP Window */
#define TCP_HDR_OPTCHECK_MASK 0x00f0

#define ICMP_HDR_CODE       0x1000  /* ICMP Header Code */
#define ICMP_HDR_TYPE       0x2000  /* ICMP Header Type */
#define ICMP_HDR_ID         0x3000  /* ICMP ID for ICMP_ECHO/ICMP_ECHO_REPLY */
#define ICMP_HDR_SEQ        0x4000  /* ICMP ID for ICMP_ECHO/ICMP_ECHO_REPLY */
#define ICMP_HDR_OPTCHECK_MASK 0xf000

typedef struct _HdrOptCheck
{
    uint16_t hdrField;   /* Field to check */
    uint32_t op;         /* Type of comparison */
    uint32_t value;      /* Value to compare value against */
    uint32_t mask_value; /* bits of value to ignore */
    uint32_t flags;
} HdrOptCheck;

#define DYNAMIC_TYPE_INT_STATIC 1
#define DYNAMIC_TYPE_INT_REF    2

typedef struct _DynamicElement
{
    char dynamicType;           /* type of this field - static or reference */
    char *refId;                /* reference ID (NULL if static) */
    union
    {
        void *voidPtr;          /* Holder */
        int32_t staticInt;        /* Value of static */
        int32_t *dynamicInt;  /* Pointer to value of dynamic */
    } data;
} DynamicElement;

typedef struct _LoopInfo
{
    DynamicElement *start;      /* Starting value of FOR loop (i=start) */
    DynamicElement *end;        /* Ending value of FOR loop (i OP end) */
    DynamicElement *increment;  /* Increment value of FOR loop (i+= increment) */
    uint32_t op;               /* Type of comparison for loop termination */
    CursorInfo *cursorAdjust;   /* How to move cursor each iteration of loop */
    struct _Rule *subRule;      /* Pointer to SubRule & options to evaluate within
                                 * the loop */
    uint8_t initialized;       /* Loop initialized properly (safeguard) */
    uint32_t flags;            /* can be used to negate loop results, specifies
                                 * relative. */
} LoopInfo;

typedef struct _base64DecodeData
{
    uint32_t bytes;
    uint32_t offset;
    uint8_t relative;
}base64DecodeData;

typedef struct _PreprocessorOption
{
    const char *optionName;
    const char *optionParameters;
    uint32_t flags;
    PreprocOptionInit optionInit;
    PreprocOptionEval optionEval;
    void *dataPtr;
    PreprocOptionFastPatternFunc optionFpFunc;
    PreprocOptionCleanup optionCleanup;
} PreprocessorOption;

typedef struct _RuleOption
{
    DynamicOptionType optionType;
    union
    {
        void *ptr;
        ContentInfo *content;
        ProtectedContentInfo *protectedContent;
        CursorInfo *cursor;
        PCREInfo *pcre;
        FlowBitsInfo *flowBit;
        ByteData *byte;
        ByteExtract *byteExtract;
        FlowFlags *flowFlags;
        Asn1Context *asn1;
        HdrOptCheck *hdrData;
        LoopInfo    *loop;
        base64DecodeData *bData;
        PreprocessorOption *preprocOpt;
    } option_u;
} RuleOption;

typedef struct _IPInfo
{
    uint8_t protocol;
    char *   src_addr;
    char *   src_port; /* 0 for non TCP/UDP */
    char     direction;     /* non-zero is bi-directional */
    char *   dst_addr;
    char *   dst_port; /* 0 for non TCP/UDP */
} IPInfo;

typedef struct _RuleReference
{
    char *systemName;
    char *refIdentifier;
} RuleReference;

#define REGISTER_RULE 1
#define DONT_REGISTER_RULE 0

typedef struct _RuleMetaData {
    char *data;
} RuleMetaData;

typedef struct _RuleInformation
{
    uint32_t genID;
    uint32_t sigID;
    uint32_t revision;
    char     *classification; /* String format of classification name */
    uint32_t priority;
    char     *message;
    RuleReference **references; /* NULL terminated array of references */
    RuleMetaData **meta; /* NULL terminated array of references */
} RuleInformation;

typedef int (*ruleEvalFunc)(void *);

typedef struct _Rule
{
    IPInfo ip;
    RuleInformation info;

    RuleOption **options; /* NULL terminated array of RuleOption union */

    ruleEvalFunc evalFunc;

    char initialized;     /* Rule Initialized, used internally */
    uint32_t numOptions; /* Rule option count, used internally */
    char noAlert;         /* Flag with no alert, used internally */
    void *ruleData;    /* Hash table for dynamic data pointers */

} Rule;


struct _SnortConfig;
ENGINE_LINKAGE int RegisterRules(struct _SnortConfig *sc, Rule **rules);
ENGINE_LINKAGE int DumpRules(char *rulesFileName, Rule **rules);

ENGINE_LINKAGE int contentMatch(void *p, ContentInfo* content, const uint8_t **cursor);
ENGINE_LINKAGE int protectedContentMatch(void *p, ProtectedContentInfo* content, const uint8_t **cursor);
ENGINE_LINKAGE int checkFlow(void *p, FlowFlags *flowFlags);
ENGINE_LINKAGE int extractValue(void *p, ByteExtract *byteExtract, const uint8_t *cursor);
ENGINE_LINKAGE int processFlowbits(void *p, FlowBitsInfo *flowBits);
ENGINE_LINKAGE int getBuffer(void *p, int flags, const uint8_t **start, const uint8_t **end);
ENGINE_LINKAGE int setCursor(void *p, CursorInfo *cursorInfo, const uint8_t **cursor);
ENGINE_LINKAGE int fileData(void *p, CursorInfo* cursorInfo, const uint8_t **cursor);
ENGINE_LINKAGE int pktData(void *p, CursorInfo* cursorInfo, const uint8_t **cursor);
ENGINE_LINKAGE int base64Data(void *p, CursorInfo* cursorInfo, const uint8_t **cursor);
ENGINE_LINKAGE int base64Decode(void *p, base64DecodeData *data, const uint8_t *cursor);
ENGINE_LINKAGE int checkCursor(void *p, CursorInfo *cursorInfo, const uint8_t *cursor);
ENGINE_LINKAGE int checkValue(void *p, ByteData *byteData, uint32_t value, const uint8_t *cursor);
/* Same as extractValue plus checkValue */
ENGINE_LINKAGE int byteTest(void *p, ByteData *byteData, const uint8_t *cursor);
ENGINE_LINKAGE int byteMath(void *p, ByteData *byteData, const uint8_t *cursor);
/* Same as extractValue plus setCursor */
ENGINE_LINKAGE int byteJump(void *p, ByteData *byteData, const uint8_t **cursor);
ENGINE_LINKAGE int pcreMatch(void *p, PCREInfo* pcre, const uint8_t **cursor);
ENGINE_LINKAGE int detectAsn1(void *p, Asn1Context* asn1, const uint8_t *cursor);
ENGINE_LINKAGE int checkHdrOpt(void *p, HdrOptCheck *optData);
ENGINE_LINKAGE int loopEval(void *p, LoopInfo *loop, const uint8_t **cursor);
ENGINE_LINKAGE int preprocOptionEval(void *p, PreprocessorOption *preprocOpt, const uint8_t **cursor);
ENGINE_LINKAGE void setTempCursor(const uint8_t **temp_cursor, const uint8_t **cursor);
ENGINE_LINKAGE void revertTempCursor(const uint8_t **temp_cursor, const uint8_t **cursor);
ENGINE_LINKAGE int ruleMatch(void *p, Rule *rule);
ENGINE_LINKAGE int MatchDecryptedRC4(
    const uint8_t *key, uint16_t keylen, const uint8_t *encrypted_data,
    uint8_t *plain_data, uint16_t datalen
);
ENGINE_LINKAGE int storeRuleData(void *packet, const RuleInformation *info,
    void *rule_data, void *compression_data);
ENGINE_LINKAGE void getRuleData(void *packet, const RuleInformation *info,
    void **p_rule_data, void **p_compression_data);
ENGINE_LINKAGE void *allocRuleData(size_t);
ENGINE_LINKAGE void freeRuleData(void *);

ENGINE_LINKAGE int isDetectFlag(SFDetectFlagType df);
ENGINE_LINKAGE void detectFlagDisable(SFDetectFlagType df);
ENGINE_LINKAGE int getAltDetect(uint8_t **bufPtr, uint16_t *altLenPtr);
ENGINE_LINKAGE void setAltDetect(uint8_t *buf, uint16_t altLen);

ENGINE_LINKAGE int pcreExecWrapper(const PCREInfo *pcre_info, const char *buf, int len, int start_offset,
                                    int options, int *ovector, int ovecsize);

static inline int invertMatchResult(int retVal)
{
    return (retVal <= RULE_NOMATCH) ? RULE_MATCH : RULE_NOMATCH;
}

#endif /* SF_SNORT_PLUGIN_API_H_ */

