/****************************************************************************
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2008-2013 Sourcefire, Inc.
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
 ****************************************************************************
 *
 ****************************************************************************/

#include <errno.h>
#include <string.h>
#include <stdarg.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "sf_types.h"
#include "dce2_roptions.h"
#include "dce2_memory.h"
#include "dcerpc.h"
#include "dce2_utils.h"
#include "dce2_session.h"
#include "sf_types.h"
#include "sf_dynamic_preprocessor.h"
#include "stream_api.h"
#include "sf_dynamic_engine.h"
#include "sf_snort_plugin_api.h"
#include "sfhashfcn.h"
#include "profiler.h"

/********************************************************************
 * Macros
 ********************************************************************/
#define DCE2_RTOKEN__OPT_SEP      ","     /* Rule option option separator */
#define DCE2_RTOKEN__ARG_SEP      " \t"   /* Rule option argument separator */
#define DCE2_RTOKEN__IFACE_SEP    "-"     /* Rule option interface separator */

#define DCE2_ROPT__IFACE      "dce_iface"
#define DCE2_ROPT__OPNUM      "dce_opnum"
#define DCE2_ROPT__STUB_DATA  "dce_stub_data"
#define DCE2_ROPT__BYTE_TEST  "byte_test"   /* Override keyword */
#define DCE2_ROPT__BYTE_JUMP  "byte_jump"   /* Override keyword */
#define DCE2_ROPT__BYTE_EXTRACT "byte_extract"  /* Override keyword */

#define DCE2_RARG__LT   '<'
#define DCE2_RARG__EQ   '='
#define DCE2_RARG__GT   '>'
#define DCE2_RARG__NE   '!'
#define DCE2_RARG__AND  '&'
#define DCE2_RARG__XOR  '^'
#define DCE2_RARG__ANY_FRAG      "any_frag"
#define DCE2_RARG__RELATIVE      "relative"
#define DCE2_RARG__MULTIPLIER    "multiplier"
#define DCE2_RARG__ALIGN         "align"
#define DCE2_RARG__POST_OFFSET   "post_offset"
#define DCE2_RARG__DCE_OVERRIDE  "dce"
#define DCE2_RARG__DCE_BYTEORDER "dce"

#define DCE2_IFACE__MIN_ARGS  1
#define DCE2_IFACE__MAX_ARGS  3
#define DCE2_IFACE__LEN  36  /* counting the dashes */
#define DCE2_IFACE__TIME_LOW_LEN    8
#define DCE2_IFACE__TIME_MID_LEN    4
#define DCE2_IFACE__TIME_HIGH_LEN   4
#define DCE2_IFACE__CLOCK_SEQ_LEN   4
#define DCE2_IFACE__NODE_LEN       12

#define DCE2_BTEST__MIN_ARGS  4
#define DCE2_BTEST__MAX_ARGS  6

#define DCE2_BJUMP__MIN_ARGS  2
#define DCE2_BJUMP__MAX_ARGS  7

#define DCE2_OPNUM__MAX  (UINT16_MAX + 1)
#define DCE2_OPNUM__MAX_INDEX  (DCE2_OPNUM__MAX / 8)

/********************************************************************
 * Enumerations
 ********************************************************************/
typedef enum _DCE2_IfOp
{
    DCE2_IF_OP__NONE = 0,
    DCE2_IF_OP__LT,
    DCE2_IF_OP__EQ,
    DCE2_IF_OP__GT,
    DCE2_IF_OP__NE

} DCE2_IfOp;

typedef enum _DCE2_BtOp
{
    DCE2_BT_OP__NONE = 0,
    DCE2_BT_OP__LT,
    DCE2_BT_OP__EQ,
    DCE2_BT_OP__GT,
    DCE2_BT_OP__AND,
    DCE2_BT_OP__XOR

} DCE2_BtOp;

typedef enum _DCE2_OpnumType
{
    DCE2_OPNUM_TYPE__SINGLE,
    DCE2_OPNUM_TYPE__MULTIPLE

} DCE2_OpnumType;

typedef enum _DCE2_OpnumListState
{
    DCE2_OPNUM_LIST_STATE__START,
    DCE2_OPNUM_LIST_STATE__OPNUM_START,
    DCE2_OPNUM_LIST_STATE__OPNUM_LO,
    DCE2_OPNUM_LIST_STATE__OPNUM_RANGE,
    DCE2_OPNUM_LIST_STATE__OPNUM_HI,
    DCE2_OPNUM_LIST_STATE__OPNUM_END,
    DCE2_OPNUM_LIST_STATE__END

} DCE2_OpnumListState;

/********************************************************************
 * Structures
 ********************************************************************/
typedef struct _DCE2_IfaceData
{
    Uuid iface;
    uint32_t iface_vers;
    int iface_vers_maj;
    int iface_vers_min;
    DCE2_IfOp operator;
    int any_frag;

} DCE2_IfaceData;

typedef struct _DCE_OpnumData
{
    DCE2_OpnumType type;

} DCE2_OpnumData;

typedef struct _DCE_OpnumSingle
{
    DCE2_OpnumData odata;
    uint16_t opnum;

} DCE2_OpnumSingle;

typedef struct _DCE2_OpnumMultiple
{
    DCE2_OpnumData odata;
    uint8_t *mask;
    uint16_t mask_size;
    uint16_t opnum_lo;
    uint16_t opnum_hi;

} DCE2_OpnumMultiple;

typedef struct _DCE2_ByteTestData
{
    int num_bytes;
    uint32_t value;
    int invert;
    DCE2_BtOp operator;
    int32_t offset;   /* can be negative */
    int relative;

} DCE2_ByteTestData;

typedef struct _DCE2_ByteJumpData
{
    int num_bytes;
    int32_t offset;   /* can be negative */
    int relative;
    int multiplier;
    int align;
    int32_t post_offset;   /* can be negative */

} DCE2_ByteJumpData;

/********************************************************************
 * Private function prototypes
 ********************************************************************/
static int DCE2_IfaceInit(struct _SnortConfig *, char *, char *, void **);
static int DCE2_OpnumInit(struct _SnortConfig *, char *, char *, void **);
static void DCE2_ParseOpnumList(char **, char *, uint8_t *);
static inline void DCE2_OpnumSet(uint8_t *, const uint16_t);
static inline void DCE2_OpnumSetRange(uint8_t *, uint16_t, uint16_t);
static inline int DCE2_OpnumIsSet(const uint8_t *, const uint16_t, const uint16_t, const uint16_t);
static int DCE2_StubDataInit(struct _SnortConfig *, char *, char *, void **);
static int DCE2_ByteTestInit(struct _SnortConfig *, char *, char *, void **);
static int DCE2_ByteJumpInit(struct _SnortConfig *, char *, char *, void **);
static void DCE2_ParseIface(char *, DCE2_IfaceData *);
static int DCE2_IfaceEval(void *, const uint8_t **, void *);
static int DCE2_OpnumEval(void *, const uint8_t **, void *);
static int DCE2_StubDataEval(void *, const uint8_t **, void *);
static int DCE2_ByteTestEval(void *, const uint8_t **, void *);
static int DCE2_ByteJumpEval(void *, const uint8_t **, void *);
static void DCE2_IfaceCleanup(void *);
static void DCE2_OpnumCleanup(void *);
static void DCE2_ByteTestCleanup(void *);
static void DCE2_ByteJumpCleanup(void *);
static uint32_t DCE2_IfaceHash(void *);
static uint32_t DCE2_OpnumHash(void *);
static uint32_t DCE2_ByteTestHash(void *);
static uint32_t DCE2_ByteJumpHash(void *);
static int DCE2_IfaceKeyCompare(void *, void *);
static int DCE2_OpnumKeyCompare(void *, void *);
static int DCE2_ByteTestKeyCompare(void *, void *);
static int DCE2_ByteJumpKeyCompare(void *, void *);
static inline int DCE2_RoptDoEval(SFSnortPacket *);
static NORETURN void DCE2_RoptError(const char *, ...);
static inline void * DCE2_AllocFp(uint32_t);
static int DCE2_IfaceAddFastPatterns(void *, int, int, FPContentInfo **);

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
void DCE2_RegRuleOptions(struct _SnortConfig *sc)
{
    _dpd.preprocOptRegister(sc, DCE2_ROPT__IFACE, DCE2_IfaceInit, DCE2_IfaceEval,
            DCE2_IfaceCleanup, DCE2_IfaceHash, DCE2_IfaceKeyCompare,
            NULL, DCE2_IfaceAddFastPatterns);
    _dpd.preprocOptRegister(sc, DCE2_ROPT__OPNUM, DCE2_OpnumInit, DCE2_OpnumEval,
            DCE2_OpnumCleanup, DCE2_OpnumHash, DCE2_OpnumKeyCompare, NULL, NULL);
    _dpd.preprocOptRegister(sc, DCE2_ROPT__STUB_DATA, DCE2_StubDataInit,
            DCE2_StubDataEval, NULL, NULL, NULL, NULL, NULL);
    _dpd.preprocOptOverrideKeyword(sc, DCE2_ROPT__BYTE_TEST, DCE2_RARG__DCE_OVERRIDE,
            DCE2_ByteTestInit, DCE2_ByteTestEval, DCE2_ByteTestCleanup,
            DCE2_ByteTestHash, DCE2_ByteTestKeyCompare, NULL, NULL);
    _dpd.preprocOptOverrideKeyword(sc, DCE2_ROPT__BYTE_JUMP, DCE2_RARG__DCE_OVERRIDE,
            DCE2_ByteJumpInit, DCE2_ByteJumpEval, DCE2_ByteJumpCleanup,
            DCE2_ByteJumpHash, DCE2_ByteJumpKeyCompare, NULL, NULL);
    _dpd.preprocOptByteOrderKeyword(DCE2_RARG__DCE_BYTEORDER, DCE2_GetByteOrder);
}

/********************************************************************
 * Function: DCE2_IfaceInit()
 *
 * Parses dce_iface rule option.
 *
 * XXX Connectionless uses a 32bit version, connection-oriented
 * a 16bit major version and 16bit minor version.  Not likely to
 * need to support versions greater than 65535, but may need to
 * support minor version.
 *
 * Arguments:
 *  char *
 *      Name of rule option.
 *  char *
 *      Arguments for rule option.
 *  data **
 *      Variable for saving rule option structure.
 *
 * Returns:
 *  1 if successful
 *  0 if name is not dce_iface
 *  Fatal errors if invalid arguments.
 *
 ********************************************************************/
static int DCE2_IfaceInit(struct _SnortConfig *sc, char *name, char *args, void **data)
{
    char *token, *saveptr = NULL;
    int iface_vers = 0, any_frag = 0;
    int tok_num = 0;
    DCE2_IfaceData *iface_data;

    if (strcasecmp(name, DCE2_ROPT__IFACE) != 0)
        return 0;

    iface_data = (DCE2_IfaceData *)DCE2_Alloc(sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
    if (iface_data == NULL)
    {
        DCE2_Die("%s(%d) Failed to allocate memory for iface data structure.",
                __FILE__, __LINE__);
    }

    iface_data->operator = DCE2_IF_OP__NONE;

    /* Must have arguments */
    if (DCE2_IsEmptyStr(args))
    {
        DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
        DCE2_RoptError("\"%s\" rule option requires arguments.", DCE2_ROPT__IFACE);
    }

    /* Get argument */
    token = strtok_r(args, DCE2_RTOKEN__OPT_SEP, &saveptr);
    if (token == NULL)
    {
        DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
        DCE2_Die("%s(%d) strtok_r() returned NULL when string argument "
                "was not NULL.", __FILE__, __LINE__);
    }

    do
    {
        tok_num++;

        token = DCE2_PruneWhiteSpace(token);

        if (tok_num == 1)   /* Iface uuid */
        {
            DCE2_ParseIface(token, iface_data);
        }
        else if ((tok_num > DCE2_IFACE__MIN_ARGS) && (tok_num <= DCE2_IFACE__MAX_ARGS))
        {
            int try_any_frag = 0;

            /* Need at least two bytes */
            if (strlen(token) < 2)
            {
                DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
                DCE2_RoptError("\"%s\" rule option: Invalid argument: %s.",
                        DCE2_ROPT__IFACE, token);
            }

            switch (*token)
            {
                case DCE2_RARG__LT:
                    iface_data->operator = DCE2_IF_OP__LT;
                    break;
                case DCE2_RARG__EQ:
                    iface_data->operator = DCE2_IF_OP__EQ;
                    break;
                case DCE2_RARG__GT:
                    iface_data->operator = DCE2_IF_OP__GT;
                    break;
                case DCE2_RARG__NE:
                    iface_data->operator = DCE2_IF_OP__NE;
                    break;
                default:
                    try_any_frag = 1;
            }

            if (!try_any_frag)
            {
                char *endptr;

                if (iface_vers)
                {
                    DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
                    DCE2_RoptError("\"%s\" rule option: Cannot configure interface "
                            "version more than once.", DCE2_ROPT__IFACE);
                }

                token++;
                iface_data->iface_vers = _dpd.SnortStrtoul(token, &endptr, 10);
                if ((errno == ERANGE) || (*endptr != '\0'))
                {
                    DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
                    DCE2_RoptError("\"%s\" rule option: Invalid argument: %s.",
                            DCE2_ROPT__IFACE, token);
                }

                switch (iface_data->operator)
                {
                    case DCE2_IF_OP__LT:
                        if (iface_data->iface_vers == 0)
                        {
                            DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
                            DCE2_RoptError("\"%s\" rule option: Interface version "
                                    "cannot be less than zero.", DCE2_ROPT__IFACE);
                        }
                        else if (iface_data->iface_vers > (UINT16_MAX + 1))
                        {
                            DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
                            DCE2_RoptError("\"%s\" rule option: Interface version "
                                    "cannot be greater than %u.",
                                    DCE2_ROPT__IFACE, UINT16_MAX);
                        }

                        break;

                    case DCE2_IF_OP__EQ:
                    case DCE2_IF_OP__NE:
                        if (iface_data->iface_vers > UINT16_MAX)
                        {
                            DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
                            DCE2_RoptError("\"%s\" rule option: Interface version "
                                    "cannot be greater than %u.",
                                    DCE2_ROPT__IFACE, UINT16_MAX);
                        }

                        break;

                    case DCE2_IF_OP__GT:
                        if (iface_data->iface_vers >= UINT16_MAX)
                        {
                            DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
                            DCE2_RoptError("\"%s\" rule option: Interface version "
                                    "cannot be greater than %u.",
                                    DCE2_ROPT__IFACE, UINT16_MAX);
                        }

                        break;

                    default:
                        /* Shouldn't get here */
                        DCE2_Die("%s(%d) Invalid operator: %d",
                                __FILE__, __LINE__, iface_data->operator);
                        break;
                }

                if (iface_data->iface_vers <= UINT16_MAX)
                    iface_data->iface_vers_maj = (int)iface_data->iface_vers;
                else
                    iface_data->iface_vers_maj = DCE2_SENTINEL;

                iface_vers = 1;
            }
            else
            {
                if (any_frag)
                {
                    DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
                    DCE2_RoptError("\"%s\" rule option: Cannot configure "
                            "\"%s\" more than once.",
                            DCE2_ROPT__IFACE, DCE2_RARG__ANY_FRAG);
                }

                if (strcasecmp(token, DCE2_RARG__ANY_FRAG) == 0)
                {
                    iface_data->any_frag = 1;
                }
                else
                {
                    DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
                    DCE2_RoptError("\"%s\" rule option: Invalid argument: %s.",
                            DCE2_ROPT__IFACE, token);
                }

                any_frag = 1;
            }
        }
        else
        {
            DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
            DCE2_RoptError("\"%s\" rule option: Too many arguments.", DCE2_ROPT__IFACE);
        }

    } while ((token = strtok_r(NULL, DCE2_RTOKEN__OPT_SEP, &saveptr)) != NULL);

    *data = (void *)iface_data;

    return 1;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static void DCE2_ParseIface(char *token, DCE2_IfaceData *iface_data)
{
    char *iface, *ifaceptr = NULL;
    char *if_hex, *if_hexptr = NULL;
    int num_pieces = 0;

    /* Has to be a uuid in string format, e.g 4b324fc8-1670-01d3-1278-5a47bf6ee188
     * Check the length */
    if (strlen(token) != DCE2_IFACE__LEN)
    {
        DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
        DCE2_RoptError("\"%s\" rule option: Invalid uuid.", DCE2_ROPT__IFACE);
    }

    /* Detach token */
    iface = strtok_r(token, DCE2_RTOKEN__ARG_SEP, &ifaceptr);
    if (iface == NULL)
    {
        DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
        DCE2_Die("%s(%d) strtok_r() returned NULL when string argument "
                "was not NULL.", __FILE__, __LINE__);
    }

    /* Cut into pieces separated by '-' */
    if_hex = strtok_r(iface, DCE2_RTOKEN__IFACE_SEP, &if_hexptr);
    if (if_hex == NULL)
    {
        DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
        DCE2_Die("%s(%d) strtok_r() returned NULL when string argument "
                "was not NULL.", __FILE__, __LINE__);
    }

    do
    {
        char *endptr;

        switch (num_pieces)
        {
            case 0:
                {
                    unsigned long int time_low;

                    if (strlen(if_hex) != DCE2_IFACE__TIME_LOW_LEN)
                    {
                        DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
                        DCE2_RoptError("\"%s\" rule option: Invalid uuid.", DCE2_ROPT__IFACE);
                    }

                    time_low = _dpd.SnortStrtoul(if_hex, &endptr, 16);
                    if ((errno == ERANGE) || (*endptr != '\0'))
                    {
                        DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
                        DCE2_RoptError("\"%s\" rule option: Invalid uuid.", DCE2_ROPT__IFACE);
                    }

                    iface_data->iface.time_low = (uint32_t)time_low;
                }

                break;

            case 1:
                {
                    unsigned long int time_mid;

                    if (strlen(if_hex) != DCE2_IFACE__TIME_MID_LEN)
                    {
                        DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
                        DCE2_RoptError("\"%s\" rule option: Invalid uuid.", DCE2_ROPT__IFACE);
                    }

                    time_mid = _dpd.SnortStrtoul(if_hex, &endptr, 16);
                    if ((errno == ERANGE) || (*endptr != '\0'))
                    {
                        DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
                        DCE2_RoptError("\"%s\" rule option: Invalid uuid.", DCE2_ROPT__IFACE);
                    }

                    /* Length check ensures 16 bit value */
                    iface_data->iface.time_mid = (uint16_t)time_mid;
                }

                break;

            case 2:
                {
                    unsigned long int time_high;

                    if (strlen(if_hex) != DCE2_IFACE__TIME_HIGH_LEN)
                    {
                        DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
                        DCE2_RoptError("\"%s\" rule option: Invalid uuid.", DCE2_ROPT__IFACE);
                    }

                    time_high = _dpd.SnortStrtoul(if_hex, &endptr, 16);
                    if ((errno == ERANGE) || (*endptr != '\0'))
                    {
                        DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
                        DCE2_RoptError("\"%s\" rule option: Invalid uuid.", DCE2_ROPT__IFACE);
                    }

                    /* Length check ensures 16 bit value */
                    iface_data->iface.time_high_and_version = (uint16_t)time_high;
                }

                break;

            case 3:
                {
                    unsigned long int clock_seq_and_reserved, clock_seq_low;

                    if (strlen(if_hex) != DCE2_IFACE__CLOCK_SEQ_LEN)
                    {
                        DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
                        DCE2_RoptError("\"%s\" rule option: Invalid uuid.", DCE2_ROPT__IFACE);
                    }

                    /* Work backwards */
                    clock_seq_low = _dpd.SnortStrtoul(&if_hex[2], &endptr, 16);
                    if ((errno == ERANGE) || (*endptr != '\0'))
                    {
                        DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
                        DCE2_RoptError("\"%s\" rule option: Invalid uuid.", DCE2_ROPT__IFACE);
                    }

                    iface_data->iface.clock_seq_low = (uint8_t)clock_seq_low;

                    /* Set third byte to null so we can _dpd.SnortStrtoul the first part */
                    if_hex[2] = '\x00';

                    clock_seq_and_reserved = _dpd.SnortStrtoul(if_hex, &endptr, 16);
                    if ((errno == ERANGE) || (*endptr != '\0'))
                    {
                        DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
                        DCE2_RoptError("\"%s\" rule option: Invalid uuid.", DCE2_ROPT__IFACE);
                    }

                    iface_data->iface.clock_seq_and_reserved = (uint8_t)clock_seq_and_reserved;
                }

                break;

            case 4:
                {
                    int i, j;

                    if (strlen(if_hex) != DCE2_IFACE__NODE_LEN)
                    {
                        DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
                        DCE2_RoptError("\"%s\" rule option: Invalid uuid.", DCE2_ROPT__IFACE);
                    }

                    /* Walk back a byte at a time - 2 hex digits */
                    for (i = DCE2_IFACE__NODE_LEN - 2, j = sizeof(iface_data->iface.node) - 1;
                            (i >= 0) && (j >= 0);
                            i -= 2, j--)
                    {
                        /* Only giving _dpd.SnortStrtoul 1 byte */
                        iface_data->iface.node[j] = (uint8_t)_dpd.SnortStrtoul(&if_hex[i], &endptr, 16);
                        if ((errno == ERANGE) || (*endptr != '\0'))
                        {
                            DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
                            DCE2_RoptError("\"%s\" rule option: Invalid uuid.", DCE2_ROPT__IFACE);
                        }

                        if_hex[i] = '\0';
                    }
                }

                break;

            default:
                break;
        }

        num_pieces++;

    } while ((if_hex = strtok_r(NULL, DCE2_RTOKEN__IFACE_SEP, &if_hexptr)) != NULL);

    if (num_pieces != 5)
    {
        DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
        DCE2_RoptError("\"%s\" rule option: Invalid uuid.", DCE2_ROPT__IFACE);
    }

    /* Check for more arguments */
    iface = strtok_r(NULL, DCE2_RTOKEN__ARG_SEP, &ifaceptr);
    if (iface != NULL)
    {
        DCE2_Free((void *)iface_data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
        DCE2_RoptError("\"%s\" rule option: Invalid uuid.", DCE2_ROPT__IFACE);
    }
}

static inline void * DCE2_AllocFp(uint32_t size)
{
    void *mem = calloc(1, (size_t)size);
    if (mem == NULL)
    {
        DCE2_Die("%s(%d) Out of memory!", __FILE__, __LINE__);
    }

    return mem;
}

static int DCE2_IfaceAddFastPatterns(void *rule_opt_data, int protocol,
        int direction, FPContentInfo **info)
{
    DCE2_IfaceData *iface_data = (DCE2_IfaceData *)rule_opt_data;

    if ((rule_opt_data == NULL) || (info == NULL))
        return -1;

    if ((protocol != IPPROTO_TCP) && (protocol != IPPROTO_UDP))
        return -1;

    if (protocol == IPPROTO_TCP)
    {
        FPContentInfo *tcp_fp = (FPContentInfo *)DCE2_AllocFp(sizeof(FPContentInfo));
        char *client_fp = "\x05\x00\x00";
        char *server_fp = "\x05\x00\x02";
        char *no_dir_fp = "\x05\x00";

        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS, "Adding fast pattern "
                    "content for TCP rule option.\n"));

        switch (direction)
        {
            case FLAG_FROM_CLIENT:
                tcp_fp->content = (char *)DCE2_AllocFp(3);
                memcpy(tcp_fp->content, client_fp, 3);
                tcp_fp->length = 3;
                break;

            case FLAG_FROM_SERVER:
                tcp_fp->content = (char *)DCE2_AllocFp(3);
                memcpy(tcp_fp->content, server_fp, 3);
                tcp_fp->length = 3;
                break;

            default:
                tcp_fp->content = (char *)DCE2_AllocFp(2);
                memcpy(tcp_fp->content, no_dir_fp, 2);
                tcp_fp->length = 2;
                break;
        }

        *info = tcp_fp;
    }
    else
    {
        //DCE2_IfaceData *iface_data = (DCE2_IfaceData *)rule_opt_data;
        FPContentInfo *big_fp = (FPContentInfo *)DCE2_AllocFp(sizeof(FPContentInfo));
        FPContentInfo *little_fp = (FPContentInfo *)DCE2_AllocFp(sizeof(FPContentInfo));
        char *big_content = (char *)DCE2_AllocFp(sizeof(Uuid));
        char *little_content = (char *)DCE2_AllocFp(sizeof(Uuid));
        uint32_t time32;
        uint16_t time16;
        int index = 0;

        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS, "Adding fast pattern "
                    "content for UDP rule option.\n"));

        time32 = DceRpcNtohl(&iface_data->iface.time_low,
                DCERPC_BO_FLAG__BIG_ENDIAN);
        memcpy(&big_content[index], &time32, sizeof(uint32_t));
        time32 = DceRpcNtohl(&iface_data->iface.time_low,
                DCERPC_BO_FLAG__LITTLE_ENDIAN);
        memcpy(&little_content[index], &time32, sizeof(uint32_t));
        index += sizeof(uint32_t);

        time16 = DceRpcNtohs(&iface_data->iface.time_mid,
                DCERPC_BO_FLAG__BIG_ENDIAN);
        memcpy(&big_content[index], &time16, sizeof(uint16_t));
        time16 = DceRpcNtohs(&iface_data->iface.time_mid,
                DCERPC_BO_FLAG__LITTLE_ENDIAN);
        memcpy(&little_content[index], &time16, sizeof(uint16_t));
        index += sizeof(uint16_t);

        time16 = DceRpcNtohs(&iface_data->iface.time_high_and_version,
                DCERPC_BO_FLAG__BIG_ENDIAN);
        memcpy(&big_content[index], &time16, sizeof(uint16_t));
        time16 = DceRpcNtohs(&iface_data->iface.time_high_and_version,
                DCERPC_BO_FLAG__LITTLE_ENDIAN);
        memcpy(&little_content[index], &time16, sizeof(uint16_t));
        index += sizeof(uint16_t);

        big_content[index] = iface_data->iface.clock_seq_and_reserved;
        little_content[index] = iface_data->iface.clock_seq_and_reserved;
        index += sizeof(uint8_t);

        big_content[index] = iface_data->iface.clock_seq_low;
        little_content[index] = iface_data->iface.clock_seq_low;
        index += sizeof(uint8_t);

        memcpy(&big_content[index], iface_data->iface.node, 6);
        memcpy(&little_content[index], iface_data->iface.node, 6);

        big_fp->content = big_content;
        big_fp->length = sizeof(Uuid);
        little_fp->content = little_content;
        little_fp->length = sizeof(Uuid);

        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    " Iface: %s\n  Big endian: %s\n  Little endian: %s\n",
                    DCE2_UuidToStr(&iface_data->iface, DCERPC_BO_FLAG__NONE),
                    DCE2_UuidToStr((Uuid *)big_fp->content, DCERPC_BO_FLAG__NONE),
                    DCE2_UuidToStr((Uuid *)little_fp->content, DCERPC_BO_FLAG__NONE)););

        big_fp->next = little_fp;
        *info = big_fp;
    }

    return 0;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static int DCE2_OpnumInit(struct _SnortConfig *sc, char *name, char *args, void **data)
{
    uint8_t opnum_mask[DCE2_OPNUM__MAX_INDEX];  /* 65536 bits */
    char *args_end;
    uint16_t num_opnums = 0;
    unsigned int i;
    int opnum_lo = DCE2_SENTINEL;
    int opnum_hi = 0;

    if (strcasecmp(name, DCE2_ROPT__OPNUM) != 0)
        return 0;

    /* Must have arguments */
    if (DCE2_IsEmptyStr(args))
    {
        DCE2_RoptError("\"%s\" rule option: No arguments. Must supply "
                "the value of the opnum.", DCE2_ROPT__OPNUM);
    }

    /* Include NULL byte for parsing */
    args_end = args + (strlen(args) + 1);
    memset(opnum_mask, 0, sizeof(opnum_mask));

    DCE2_ParseOpnumList(&args, args_end, opnum_mask);

    /* Must have at least one bit set or the parsing would have errored */
    for (i = 0; i < DCE2_OPNUM__MAX; i++)
    {
        if (DCE2_OpnumIsSet(opnum_mask, 0, DCE2_OPNUM__MAX - 1, (uint16_t)i))
        {
            num_opnums++;

            if (opnum_lo == DCE2_SENTINEL)
                opnum_lo = (uint16_t)i;

            opnum_hi = (uint16_t)i;
        }
    }

    if (num_opnums == 1)
    {
        DCE2_OpnumSingle *odata =
            (DCE2_OpnumSingle *)DCE2_Alloc(sizeof(DCE2_OpnumSingle), DCE2_MEM_TYPE__ROPTION);

        if (odata == NULL)
        {
            DCE2_Die("%s(%d) Failed to allocate memory for opnum data.",
                    __FILE__, __LINE__);
        }

        odata->odata.type = DCE2_OPNUM_TYPE__SINGLE;
        odata->opnum = (uint16_t)opnum_lo;

        *data = (void *)odata;
    }
    else
    {
        int opnum_range = opnum_hi - opnum_lo;
        int mask_size = (opnum_range / 8) + 1;
        DCE2_OpnumMultiple *odata =
            (DCE2_OpnumMultiple *)DCE2_Alloc(sizeof(DCE2_OpnumMultiple), DCE2_MEM_TYPE__ROPTION);

        if (odata == NULL)
        {
            DCE2_Die("%s(%d) Failed to allocate memory for opnum data.",
                    __FILE__, __LINE__);
        }

        odata->mask = (uint8_t *)DCE2_Alloc(mask_size, DCE2_MEM_TYPE__ROPTION);
        if (odata->mask == NULL)
        {
            DCE2_Free((void *)odata, sizeof(DCE2_OpnumMultiple), DCE2_MEM_TYPE__ROPTION);
            DCE2_Die("%s(%d) Failed to allocate memory for opnum data.",
                    __FILE__, __LINE__);
        }

        odata->odata.type = DCE2_OPNUM_TYPE__MULTIPLE;
        odata->mask_size = (uint16_t)mask_size;
        odata->opnum_lo = (uint16_t)opnum_lo;
        odata->opnum_hi = (uint16_t)opnum_hi;

        /* Set the opnum bits in our reduced size opnum mask */
        for (i = (unsigned int)opnum_lo; i <= (unsigned int)opnum_hi; i++)
        {
            if (DCE2_OpnumIsSet(opnum_mask, 0, DCE2_OPNUM__MAX - 1, (uint16_t)i))
                DCE2_OpnumSet(odata->mask, (uint16_t)(i - opnum_lo));
        }

        *data = (void *)odata;
    }

    return 1;
}

/********************************************************************
 * Function:
 *
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static void DCE2_ParseOpnumList(char **ptr, char *end, uint8_t *opnum_mask)
{
    char *lo_start = NULL;
    char *hi_start = NULL;
    DCE2_OpnumListState state = DCE2_OPNUM_LIST_STATE__START;
    uint16_t lo_opnum = 0, hi_opnum = 0;

    while (*ptr < end)
    {
        char c = **ptr;

        if (state == DCE2_OPNUM_LIST_STATE__END)
            break;

        switch (state)
        {
            case DCE2_OPNUM_LIST_STATE__START:
                if (DCE2_IsOpnumChar(c))
                {
                    lo_start = *ptr;
                    state = DCE2_OPNUM_LIST_STATE__OPNUM_LO;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_RoptError("\"%s\" rule option: Invalid opnum list: %s.",
                            DCE2_ROPT__OPNUM, *ptr);
                }

                break;

            case DCE2_OPNUM_LIST_STATE__OPNUM_LO:
                if (!DCE2_IsOpnumChar(c))
                {
                    DCE2_Ret status = DCE2_GetValue(lo_start, *ptr, &lo_opnum,
                            0, DCE2_INT_TYPE__UINT16, 10);

                    if (status != DCE2_RET__SUCCESS)
                    {
                        DCE2_RoptError("\"%s\" rule option: Invalid opnum: %.*s",
                                DCE2_ROPT__OPNUM, *ptr - lo_start, lo_start);
                    }

                    if (DCE2_IsOpnumRangeChar(c))
                    {
                        state = DCE2_OPNUM_LIST_STATE__OPNUM_RANGE;
                    }
                    else
                    {
                        DCE2_OpnumSet(opnum_mask, lo_opnum);
                        state = DCE2_OPNUM_LIST_STATE__OPNUM_END;
                        continue;
                    }
                }

                break;

            case DCE2_OPNUM_LIST_STATE__OPNUM_RANGE:
                if (DCE2_IsOpnumChar(c))
                {
                    hi_start = *ptr;
                    state = DCE2_OPNUM_LIST_STATE__OPNUM_HI;
                }
                else
                {
                    DCE2_OpnumSetRange(opnum_mask, lo_opnum, UINT16_MAX);
                    state = DCE2_OPNUM_LIST_STATE__OPNUM_END;
                    continue;
                }

                break;

            case DCE2_OPNUM_LIST_STATE__OPNUM_HI:
                if (!DCE2_IsOpnumChar(c))
                {
                    DCE2_Ret status = DCE2_GetValue(hi_start, *ptr, &hi_opnum,
                            0, DCE2_INT_TYPE__UINT16, 10);

                    if (status != DCE2_RET__SUCCESS)
                    {
                        DCE2_RoptError("\"%s\" rule option: Invalid opnum: %.*s",
                                DCE2_ROPT__OPNUM, *ptr - hi_start, hi_start);
                    }

                    DCE2_OpnumSetRange(opnum_mask, lo_opnum, hi_opnum);
                    state = DCE2_OPNUM_LIST_STATE__OPNUM_END;
                    continue;
                }

                break;

            case DCE2_OPNUM_LIST_STATE__OPNUM_END:
                if (DCE2_IsListSepChar(c))
                {
                    state = DCE2_OPNUM_LIST_STATE__START;
                }
                else if (DCE2_IsConfigEndChar(c))
                {
                    state = DCE2_OPNUM_LIST_STATE__END;
                    continue;
                }
                else if (!DCE2_IsSpaceChar(c))
                {
                    DCE2_RoptError("\"%s\" rule option: Invalid opnum list: %s.",
                            DCE2_ROPT__OPNUM, *ptr);
                }

                break;

            default:
                DCE2_Die("%s(%d) Invalid opnum list state: %d",
                        __FILE__, __LINE__, state);
                break;
        }

        (*ptr)++;
    }

    if (state != DCE2_OPNUM_LIST_STATE__END)
    {
        DCE2_RoptError("\"%s\" rule option: Invalid opnum list: %s",
                DCE2_ROPT__OPNUM, *ptr);
    }
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static inline int DCE2_OpnumIsSet(const uint8_t *opnum_mask, const uint16_t opnum_lo,
        const uint16_t opnum_hi, const uint16_t opnum)
{
    uint16_t otmp = opnum - opnum_lo;

    if ((opnum < opnum_lo) || (opnum > opnum_hi))
        return 0;

    return opnum_mask[(otmp / 8)] & (1 << (otmp % 8));
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static inline void DCE2_OpnumSet(uint8_t *opnum_mask, const uint16_t opnum)
{
    opnum_mask[(opnum / 8)] |= (1 << (opnum % 8));
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static inline void DCE2_OpnumSetRange(uint8_t *opnum_mask, uint16_t lo_opnum, uint16_t hi_opnum)
{
    uint16_t i;

    if (lo_opnum > hi_opnum)
    {
        uint16_t tmp = lo_opnum;
        lo_opnum = hi_opnum;
        hi_opnum = tmp;
    }

    for (i = lo_opnum; i <= hi_opnum; i++)
        DCE2_OpnumSet(opnum_mask, i);
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static int DCE2_StubDataInit(struct _SnortConfig *sc, char *name, char *args, void **data)
{
    if (strcasecmp(name, DCE2_ROPT__STUB_DATA) != 0)
        return 0;

    /* Must not have arguments */
    if (!DCE2_IsEmptyStr(args))
    {
        DCE2_RoptError("\"%s\" rule option: This option has no arguments.",
                DCE2_ROPT__STUB_DATA);
    }

    /* Set it to something even though we don't need it */
    *data = (void *)1;

    return 1;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static int DCE2_ByteTestInit(struct _SnortConfig *sc, char *name, char *args, void **data)
{
    char *token, *saveptr = NULL;
    int tok_num = 0;
    DCE2_ByteTestData *bt_data;

    if (strcasecmp(name, DCE2_ROPT__BYTE_TEST) != 0)
        return 0;

    bt_data = (DCE2_ByteTestData *)DCE2_Alloc(sizeof(DCE2_ByteTestData), DCE2_MEM_TYPE__ROPTION);
    if (bt_data == NULL)
    {
        DCE2_Die("%s(%d) Failed to allocate memory for byte test data structure.",
                __FILE__, __LINE__);
    }

    bt_data->operator = DCE2_BT_OP__NONE;

    /* Must have arguments */
    if (DCE2_IsEmptyStr(args))
    {
        DCE2_Free((void *)bt_data, sizeof(DCE2_ByteTestData), DCE2_MEM_TYPE__ROPTION);
        DCE2_RoptError("\"%s\" rule option: No arguments.", DCE2_ROPT__BYTE_TEST);
    }

    /* Get argument */
    token = strtok_r(args, DCE2_RTOKEN__OPT_SEP, &saveptr);
    if (token == NULL)
    {
        DCE2_Free((void *)bt_data, sizeof(DCE2_ByteTestData), DCE2_MEM_TYPE__ROPTION);
        DCE2_Die("%s(%d) strtok_r() returned NULL when string argument "
                "was not NULL.", __FILE__, __LINE__);
    }

    do
    {
        tok_num++;

        token = DCE2_PruneWhiteSpace(token);

        if (tok_num == 1)   /* Number of bytes to convert */
        {
            char *endptr;
            unsigned long int num_bytes = _dpd.SnortStrtoul(token, &endptr, 10);

            if ((errno == ERANGE) || (*endptr != '\0'))
            {
                DCE2_Free((void *)bt_data, sizeof(DCE2_ByteTestData), DCE2_MEM_TYPE__ROPTION);
                DCE2_RoptError("\"%s\" rule option: Invalid number of bytes to "
                        "convert: %s.  Should be one of 1, 2 or 4.",
                        DCE2_ROPT__BYTE_TEST, token);
            }

            if ((num_bytes != 1) && (num_bytes != 2) && (num_bytes != 4))
            {
                DCE2_Free((void *)bt_data, sizeof(DCE2_ByteTestData), DCE2_MEM_TYPE__ROPTION);
                DCE2_RoptError("\"%s\" rule option: Invalid number of bytes to "
                        "convert: %s.  Should be one of 1, 2 or 4.",
                        DCE2_ROPT__BYTE_TEST, token);
            }

            bt_data->num_bytes = num_bytes;
        }
        else if (tok_num == 2)  /* Operator */
        {
            /* Should only be one byte */
            if (strlen(token) > 2)
            {
                DCE2_Free((void *)bt_data, sizeof(DCE2_ByteTestData), DCE2_MEM_TYPE__ROPTION);
                DCE2_RoptError("\"%s\" rule option: Invalid argument: %s",
                        DCE2_ROPT__BYTE_TEST, token);
            }

            /* If two bytes first must be '!' */
            if (strlen(token) == 2)
            {
                if (*token != DCE2_RARG__NE)
                {
                    DCE2_Free((void *)bt_data, sizeof(DCE2_ByteTestData), DCE2_MEM_TYPE__ROPTION);
                    DCE2_RoptError("\"%s\" rule option: Invalid argument: %s",
                            DCE2_ROPT__BYTE_TEST, token);
                }
                else
                {
                    bt_data->invert = 1;
                }

                token++;
            }

            switch (*token)
            {
                case DCE2_RARG__LT:
                    bt_data->operator = DCE2_BT_OP__LT;
                    break;
                case DCE2_RARG__EQ:
                    bt_data->operator = DCE2_BT_OP__EQ;
                    break;
                case DCE2_RARG__GT:
                    bt_data->operator = DCE2_BT_OP__GT;
                    break;
                case DCE2_RARG__AND:
                    bt_data->operator = DCE2_BT_OP__AND;
                    break;
                case DCE2_RARG__XOR:
                    bt_data->operator = DCE2_BT_OP__XOR;
                    break;
                default:
                    DCE2_Free((void *)bt_data, sizeof(DCE2_ByteTestData), DCE2_MEM_TYPE__ROPTION);
                    DCE2_RoptError("\"%s\" rule option: Invalid argument: %s",
                            DCE2_ROPT__BYTE_TEST, token);
                    break;
            }
        }
        else if (tok_num == 3)  /* Value to compare to */
        {
            char *endptr;
            unsigned long int value = _dpd.SnortStrtoul(token, &endptr, 10);

            if ((errno == ERANGE) || (*endptr != '\0') || (value > UINT32_MAX))
            {
                DCE2_Free((void *)bt_data, sizeof(DCE2_ByteTestData), DCE2_MEM_TYPE__ROPTION);
                DCE2_RoptError("\"%s\" rule option: Invalid compare value: %s. Must be "
                        "between 0 and %u inclusive.",
                        DCE2_ROPT__BYTE_TEST, token, UINT32_MAX);
            }

            bt_data->value = value;
        }
        else if (tok_num == 4)  /* Offset in packet data */
        {
            char *endptr;
            long int offset = _dpd.SnortStrtol(token, &endptr, 10);

            if ((errno == ERANGE) || (*endptr != '\0') ||
                    (offset > (long int)UINT16_MAX) || (offset < (-1 * (long int)UINT16_MAX)))
            {
                DCE2_Free((void *)bt_data, sizeof(DCE2_ByteTestData), DCE2_MEM_TYPE__ROPTION);
                DCE2_RoptError("\"%s\" rule option: Invalid offset: %s. Must be "
                        "between -%u and %u inclusive.",
                        DCE2_ROPT__BYTE_TEST, token, UINT16_MAX, UINT16_MAX);
            }

            bt_data->offset = offset;
        }
        else if ((tok_num == 5) || (tok_num == 6))
        {
            if (strcasecmp(token, DCE2_RARG__RELATIVE) == 0)
            {
                /* Can't configure it twice */
                if (bt_data->relative)
                {
                    DCE2_Free((void *)bt_data, sizeof(DCE2_ByteTestData), DCE2_MEM_TYPE__ROPTION);
                    DCE2_RoptError("\"%s\" rule option: Can't configure \"%s\" "
                            "more than once.",
                            DCE2_ROPT__BYTE_TEST, DCE2_RARG__RELATIVE);
                }

                bt_data->relative = 1;
            }
            else if (strcasecmp(token, DCE2_RARG__DCE_OVERRIDE) != 0)
            {
                DCE2_Free((void *)bt_data, sizeof(DCE2_ByteTestData), DCE2_MEM_TYPE__ROPTION);
                DCE2_RoptError("\"%s\" rule option: Invalid argument: %s.",
                        DCE2_ROPT__BYTE_TEST, token);
            }
        }
        else
        {
            DCE2_Free((void *)bt_data, sizeof(DCE2_ByteTestData), DCE2_MEM_TYPE__ROPTION);
            DCE2_RoptError("\"%s\" rule option: Too many arguments.", DCE2_ROPT__BYTE_TEST);
        }

    } while ((token = strtok_r(NULL, DCE2_RTOKEN__OPT_SEP, &saveptr)) != NULL);

    if (tok_num < DCE2_BTEST__MIN_ARGS)
    {
        DCE2_Free((void *)bt_data, sizeof(DCE2_ByteTestData), DCE2_MEM_TYPE__ROPTION);
        DCE2_RoptError("\"%s\" rule option: Not enough arguments.", DCE2_ROPT__BYTE_TEST);
    }

    *data = (void *)bt_data;

    return 1;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static int DCE2_ByteJumpInit(struct _SnortConfig *sc, char *name, char *args, void **data)
{
    char *token, *saveptr = NULL;
    int tok_num = 0;
    DCE2_ByteJumpData *bj_data;
    int post_offset_configured = 0;

    if (strcasecmp(name, DCE2_ROPT__BYTE_JUMP) != 0)
        return 0;

    bj_data = (DCE2_ByteJumpData *)DCE2_Alloc(sizeof(DCE2_ByteJumpData), DCE2_MEM_TYPE__ROPTION);
    if (bj_data == NULL)
    {
        DCE2_Die("%s(%d) Failed to allocate memory for byte jump data structure.",
                __FILE__, __LINE__);
    }

    bj_data->multiplier = DCE2_SENTINEL;

    /* Must have arguments */
    if (DCE2_IsEmptyStr(args))
    {
        DCE2_Free((void *)bj_data, sizeof(DCE2_ByteJumpData), DCE2_MEM_TYPE__ROPTION);
        DCE2_RoptError("\"%s\" rule option: No arguments.", DCE2_ROPT__BYTE_JUMP);
    }

    /* Get argument */
    token = strtok_r(args, DCE2_RTOKEN__OPT_SEP, &saveptr);
    if (token == NULL)
    {
        DCE2_Free((void *)bj_data, sizeof(DCE2_ByteJumpData), DCE2_MEM_TYPE__ROPTION);
        DCE2_Die("%s(%d) strtok_r() returned NULL when string argument "
                "was not NULL.", __FILE__, __LINE__);
    }

    do
    {
        tok_num++;

        token = DCE2_PruneWhiteSpace(token);

        if (tok_num == 1)   /* Number of bytes to convert */
        {
            char *endptr;
            unsigned long int num_bytes = _dpd.SnortStrtoul(token, &endptr, 10);

            if ((errno == ERANGE) || (*endptr != '\0'))
            {
                DCE2_Free((void *)bj_data, sizeof(DCE2_ByteJumpData), DCE2_MEM_TYPE__ROPTION);
                DCE2_RoptError("\"%s\" rule option: Invalid number of bytes to "
                        "convert: %s.  Should be one of 1, 2 or 4.",
                        DCE2_ROPT__BYTE_JUMP, token);
            }

            if ((num_bytes != 4) && (num_bytes != 2) && (num_bytes != 1))
            {
                DCE2_Free((void *)bj_data, sizeof(DCE2_ByteJumpData), DCE2_MEM_TYPE__ROPTION);
                DCE2_RoptError("\"%s\" rule option: Invalid number of bytes to "
                        "convert: %s.  Should be one of 1, 2 or 4.",
                        DCE2_ROPT__BYTE_JUMP, token);
            }

            bj_data->num_bytes = num_bytes;
        }
        else if (tok_num == 2)  /* Offset in packet data */
        {
            char *endptr;
            long int offset = _dpd.SnortStrtol(token, &endptr, 10);

            if ((errno == ERANGE) || (*endptr != '\0') ||
                    (offset > (long int)UINT16_MAX) || (offset < (-1 * (long int)UINT16_MAX)))
            {
                DCE2_Free((void *)bj_data, sizeof(DCE2_ByteJumpData), DCE2_MEM_TYPE__ROPTION);
                DCE2_RoptError("\"%s\" rule option: Invalid offset: %s. Must be "
                        "between -%u and %u inclusive.",
                        DCE2_ROPT__BYTE_JUMP, token, UINT16_MAX, UINT16_MAX);
            }

            bj_data->offset = offset;
        }
        else if ((tok_num > DCE2_BJUMP__MIN_ARGS) && (tok_num <= DCE2_BJUMP__MAX_ARGS))
        {
            char *arg, *argptr;

            /* Detach arg to get potenial sub-arg */
            arg = strtok_r(token, DCE2_RTOKEN__ARG_SEP, &argptr);
            if (arg == NULL)
            {
                DCE2_Free((void *)bj_data, sizeof(DCE2_ByteJumpData), DCE2_MEM_TYPE__ROPTION);
                DCE2_Die("%s(%d) strtok_r() returned NULL when string argument "
                        "was not NULL.", __FILE__, __LINE__);
            }

            if (strcasecmp(arg, DCE2_RARG__RELATIVE) == 0)
            {
                /* Can't configure it twice */
                if (bj_data->relative)
                {
                    DCE2_Free((void *)bj_data, sizeof(DCE2_ByteJumpData), DCE2_MEM_TYPE__ROPTION);
                    DCE2_RoptError("\"%s\" rule option: Can't configure \"%s\" "
                            "more than once.",
                            DCE2_ROPT__BYTE_TEST, DCE2_RARG__RELATIVE);
                }

                bj_data->relative = 1;
            }
            else if (strcasecmp(arg, DCE2_RARG__ALIGN) == 0)
            {
                if (bj_data->align)
                {
                    DCE2_Free((void *)bj_data, sizeof(DCE2_ByteJumpData), DCE2_MEM_TYPE__ROPTION);
                    DCE2_RoptError("\"%s\" rule option: Can't configure \"%s\" "
                            "more than once.",
                            DCE2_ROPT__BYTE_TEST, DCE2_RARG__ALIGN);
                }

                bj_data->align = 1;
            }
            else if (strcasecmp(arg, DCE2_RARG__MULTIPLIER) == 0)
            {
                char *endptr;
                unsigned long int multiplier;

                if (bj_data->multiplier != DCE2_SENTINEL)
                {
                    DCE2_Free((void *)bj_data, sizeof(DCE2_ByteJumpData), DCE2_MEM_TYPE__ROPTION);
                    DCE2_RoptError("\"%s\" rule option: Can't configure \"%s\" "
                            "more than once.",
                            DCE2_ROPT__BYTE_TEST, DCE2_RARG__MULTIPLIER);
                }

                arg = strtok_r(NULL, DCE2_RTOKEN__ARG_SEP, &argptr);
                if (arg == NULL)
                {
                    DCE2_Free((void *)bj_data, sizeof(DCE2_ByteJumpData), DCE2_MEM_TYPE__ROPTION);
                    DCE2_RoptError("\"%s\" rule option: \"%s\" requires an argument.",
                            DCE2_ROPT__BYTE_JUMP, DCE2_RARG__MULTIPLIER);
                }

                multiplier = _dpd.SnortStrtoul(arg, &endptr, 10);
                if ((errno == ERANGE) || (*endptr != '\0') ||
                        (multiplier <= 1) || (multiplier > UINT16_MAX))
                {
                    DCE2_Free((void *)bj_data, sizeof(DCE2_ByteJumpData), DCE2_MEM_TYPE__ROPTION);
                    DCE2_RoptError("\"%s\" rule option: Invalid multiplier: %s. "
                            "Must be between 2 and %u inclusive.",
                            DCE2_ROPT__BYTE_JUMP, arg, UINT16_MAX);
                }

                bj_data->multiplier = multiplier;
            }
            else if (strcasecmp(arg, DCE2_RARG__POST_OFFSET) == 0)
            {
                char *endptr;
                long int post_offset;

                if (post_offset_configured)
                {
                    DCE2_Free((void *)bj_data, sizeof(DCE2_ByteJumpData), DCE2_MEM_TYPE__ROPTION);
                    DCE2_RoptError("\"%s\" rule option: Can't configure \"%s\" "
                            "more than once.",
                            DCE2_ROPT__BYTE_TEST, DCE2_RARG__POST_OFFSET);
                }

                arg = strtok_r(NULL, DCE2_RTOKEN__ARG_SEP, &argptr);
                if (arg == NULL)
                {
                    DCE2_Free((void *)bj_data, sizeof(DCE2_ByteJumpData), DCE2_MEM_TYPE__ROPTION);
                    DCE2_RoptError("\"%s\" rule option: \"%s\" requires an argument.",
                            DCE2_ROPT__BYTE_JUMP, DCE2_RARG__POST_OFFSET);
                }

                post_offset = _dpd.SnortStrtol(arg, &endptr, 10);
                if ((errno == ERANGE) || (*endptr != '\0') ||
                        (post_offset > (long int)UINT16_MAX) || (post_offset < (-1 * (long int)UINT16_MAX)))
                {
                    DCE2_Free((void *)bj_data, sizeof(DCE2_ByteJumpData), DCE2_MEM_TYPE__ROPTION);
                    DCE2_RoptError("\"%s\" rule option: Invalid post offset "
                            "value: %s. Must be between -%u to %u inclusive",
                            DCE2_ROPT__BYTE_JUMP, arg, UINT16_MAX, UINT16_MAX);
                }

                bj_data->post_offset = post_offset;
                post_offset_configured = 1;
            }
            else if (strcasecmp(arg, DCE2_RARG__DCE_OVERRIDE) != 0)
            {
                DCE2_Free((void *)bj_data, sizeof(DCE2_ByteJumpData), DCE2_MEM_TYPE__ROPTION);
                DCE2_RoptError("\"%s\" rule option: Invalid argument: %s.",
                        DCE2_ROPT__BYTE_JUMP, arg);
            }
        }
        else
        {
            DCE2_Free((void *)bj_data, sizeof(DCE2_ByteJumpData), DCE2_MEM_TYPE__ROPTION);
            DCE2_RoptError("\"%s\" rule option: Too many arguments.", DCE2_ROPT__BYTE_JUMP);
        }

    } while ((token = strtok_r(NULL, DCE2_RTOKEN__OPT_SEP, &saveptr)) != NULL);

    if (tok_num < DCE2_BJUMP__MIN_ARGS)
    {
        DCE2_Free((void *)bj_data, sizeof(DCE2_ByteJumpData), DCE2_MEM_TYPE__ROPTION);
        DCE2_RoptError("\"%s\" rule option: Not enough arguments.", DCE2_ROPT__BYTE_JUMP);
    }

    *data = (void *)bj_data;

    return 1;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static int DCE2_IfaceEval(void *pkt, const uint8_t **cursor, void *data)
{
    SFSnortPacket *p = (SFSnortPacket *)pkt;
    DCE2_SsnData *sd;
    DCE2_Roptions *ropts;
    DCE2_IfaceData *iface_data;
    int ret = RULE_NOMATCH;

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                "Evaluating \"%s\" rule option.\n", DCE2_ROPT__IFACE));

    if (!DCE2_RoptDoEval(p))
        return RULE_NOMATCH;

    sd = (DCE2_SsnData *)_dpd.sessionAPI->get_application_data(p->stream_session, PP_DCE2);
    if ((sd == NULL) || DCE2_SsnNoInspect(sd))
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "No session data - not evaluating.\n"));
        return RULE_NOMATCH;
    }

    ropts = &sd->ropts;

    if (ropts->first_frag == DCE2_SENTINEL)
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "First frag not set - not evaluating.\n"));
        return RULE_NOMATCH;
    }

    iface_data = (DCE2_IfaceData *)data;
    if (iface_data == NULL)
        return RULE_NOMATCH;

    if (!iface_data->any_frag && !ropts->first_frag)
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "Not a first fragment and rule set to only look at "
                    "first fragment.\n"));

        return RULE_NOMATCH;
    }

    /* Compare the uuid */
    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS, "Comparing \"%s\" to \"%s\"\n",
                DCE2_UuidToStr(&ropts->iface, DCERPC_BO_FLAG__NONE),
                DCE2_UuidToStr(&iface_data->iface, DCERPC_BO_FLAG__NONE)));

    if (DCE2_UuidCompare((void *)&ropts->iface, (void *)&iface_data->iface) != 0)
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS, "Uuids don't match\n"));
        return RULE_NOMATCH;
    }

    if (iface_data->operator == DCE2_IF_OP__NONE)
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "\"%s\" Match\n", DCE2_ROPT__IFACE));
        return RULE_MATCH;
    }

    switch (iface_data->operator)
    {
        case DCE2_IF_OP__LT:
            if (IsTCP(p) && (iface_data->iface_vers_maj != DCE2_SENTINEL))
            {
                if ((int)ropts->iface_vers_maj < iface_data->iface_vers_maj)
                    ret = RULE_MATCH;
            }
            else
            {
                if (ropts->iface_vers < iface_data->iface_vers)
                    ret = RULE_MATCH;
            }

            break;

        case DCE2_IF_OP__EQ:
            if (IsTCP(p) && (iface_data->iface_vers_maj != DCE2_SENTINEL))
            {
                if ((int)ropts->iface_vers_maj == iface_data->iface_vers_maj)
                    ret = RULE_MATCH;
            }
            else
            {
                if (ropts->iface_vers == iface_data->iface_vers)
                    ret = RULE_MATCH;
            }

            break;

        case DCE2_IF_OP__GT:
            if (IsTCP(p) && (iface_data->iface_vers_maj != DCE2_SENTINEL))
            {
                if ((int)ropts->iface_vers_maj > iface_data->iface_vers_maj)
                    ret = RULE_MATCH;
            }
            else
            {
                if (ropts->iface_vers > iface_data->iface_vers)
                    ret = RULE_MATCH;
            }

            break;

        case DCE2_IF_OP__NE:
            if (IsTCP(p) && (iface_data->iface_vers_maj != DCE2_SENTINEL))
            {
                if ((int)ropts->iface_vers_maj != iface_data->iface_vers_maj)
                    ret = RULE_MATCH;
            }
            else
            {
                if (ropts->iface_vers != iface_data->iface_vers)
                    ret = RULE_MATCH;
            }

            break;

        default:
            break;
    }

    return ret;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static int DCE2_OpnumEval(void *pkt, const uint8_t **cursor, void *data)
{
    SFSnortPacket *p = (SFSnortPacket *)pkt;
    DCE2_OpnumData *opnum_data = (DCE2_OpnumData *)data;
    DCE2_SsnData *sd;
    DCE2_Roptions *ropts;

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                "Evaluating \"%s\" rule option.\n", DCE2_ROPT__OPNUM));

    if (!DCE2_RoptDoEval(p))
        return RULE_NOMATCH;

    sd = (DCE2_SsnData *)_dpd.sessionAPI->get_application_data(p->stream_session, PP_DCE2);
    if ((sd == NULL) || DCE2_SsnNoInspect(sd))
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "No session data - not evaluating.\n"));
        return RULE_NOMATCH;
    }

    ropts = &sd->ropts;

    if (ropts->opnum == DCE2_SENTINEL)
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "Opnum not set - not evaluating.\n"));
        return RULE_NOMATCH;
    }

    switch (opnum_data->type)
    {
        case DCE2_OPNUM_TYPE__SINGLE:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                        "Rule opnum: %u, ropts opnum: %u\n",
                        ((DCE2_OpnumSingle *)opnum_data)->opnum, ropts->opnum));

            if (ropts->opnum == ((DCE2_OpnumSingle *)opnum_data)->opnum)
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                            "\"%s\" Match\n", DCE2_ROPT__OPNUM));
                return RULE_MATCH;
            }

            break;

        case DCE2_OPNUM_TYPE__MULTIPLE:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                        "Multiple opnums: ropts opnum: %u\n", ropts->opnum));

            {
                DCE2_OpnumMultiple *omult = (DCE2_OpnumMultiple *)opnum_data;

                if (DCE2_OpnumIsSet(omult->mask, omult->opnum_lo,
                            omult->opnum_hi, (uint16_t)ropts->opnum))
                {
                    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                                "\"%s\" Match\n", DCE2_ROPT__OPNUM));
                    return RULE_MATCH;
                }
            }

            break;

        default:
            DCE2_Log(DCE2_LOG_TYPE__ERROR,
                    "%s(%d) Invalid opnum type: %d",
                    __FILE__, __LINE__, opnum_data->type);
            break;
    }

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                "\"%s\" Fail\n", DCE2_ROPT__OPNUM));

    return RULE_NOMATCH;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static int DCE2_StubDataEval(void *pkt, const uint8_t **cursor, void *data)
{
    SFSnortPacket *p = (SFSnortPacket *)pkt;
    DCE2_SsnData *sd;
    DCE2_Roptions *ropts;

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                "Evaluating \"%s\" rule option.\n", DCE2_ROPT__STUB_DATA));

    if (!DCE2_RoptDoEval(p))
        return RULE_NOMATCH;

    sd = (DCE2_SsnData *)_dpd.sessionAPI->get_application_data(p->stream_session, PP_DCE2);
    if ((sd == NULL) || DCE2_SsnNoInspect(sd))
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "No session data - not evaluating.\n"));
        return RULE_NOMATCH;
    }

    ropts = &sd->ropts;

    if (ropts->stub_data != NULL)
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "Setting cursor to stub data: %p.\n", ropts->stub_data));
        *cursor = ropts->stub_data;
        _dpd.SetAltDetect((uint8_t *)ropts->stub_data, (uint16_t)(p->payload_size - (ropts->stub_data - p->payload)));
        return RULE_MATCH;
    }

    return RULE_NOMATCH;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static int DCE2_ByteTestEval(void *pkt, const uint8_t **cursor, void *data)
{
    SFSnortPacket *p = (SFSnortPacket *)pkt;
    DCE2_SsnData *sd;
    DCE2_Roptions *ropts;
    DCE2_ByteTestData *bt_data;
    const uint8_t *start_ptr;
    uint16_t dsize;
    const uint8_t *bt_ptr;
    uint32_t pkt_value;
    DceRpcBoFlag byte_order;
    int ret = RULE_NOMATCH;

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                "Evaluating \"%s\" rule option.\n", DCE2_ROPT__BYTE_TEST));

    if (*cursor == NULL)
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "Cursor is NULL - not evaluating.\n"));
        return RULE_NOMATCH;
    }

    if (!DCE2_RoptDoEval(p))
        return RULE_NOMATCH;

    sd = (DCE2_SsnData *)_dpd.sessionAPI->get_application_data(p->stream_session, PP_DCE2);
    if ((sd == NULL) || DCE2_SsnNoInspect(sd))
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "No session data - not evaluating.\n"));
        return RULE_NOMATCH;
    }

    ropts = &sd->ropts;

    if ((ropts->data_byte_order == DCE2_SENTINEL) ||
            (ropts->hdr_byte_order == DCE2_SENTINEL))
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "Data byte order or header byte order not set "
                    "in rule options - not evaluating.\n"));
        return RULE_NOMATCH;
    }

    bt_data = (DCE2_ByteTestData *)data;
    if (bt_data == NULL)
        return RULE_NOMATCH;

    if (_dpd.Is_DetectFlag(SF_FLAG_ALT_DETECT))
    {
        _dpd.GetAltDetect((uint8_t **)&start_ptr, &dsize);
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                    "Using Alternative Detect buffer!\n"););
    }
    else
    {
        start_ptr = p->payload;
        dsize = p->payload_size;
    }

    /* Make sure we don't read past the end of the payload or before
     * beginning of payload */
    if (bt_data->relative)
    {
        if ((bt_data->offset < 0) && (*cursor + bt_data->offset) < start_ptr)
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                        "Offset is negative and puts cursor before beginning "
                        "of payload - not evaluating.\n"));
            return RULE_NOMATCH;
        }

        if ((*cursor + bt_data->offset + bt_data->num_bytes) > (start_ptr + dsize))
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                        "Offset plus number of bytes to read puts cursor past "
                        "end of payload - not evaluating.\n"));
            return RULE_NOMATCH;
        }

        bt_ptr = *cursor + bt_data->offset;
    }
    else
    {
        if (bt_data->offset < 0)
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                        "Offset is negative but is not relative - "
                        "not evaluating.\n"));
            return RULE_NOMATCH;
        }
        else if ((start_ptr + bt_data->offset + bt_data->num_bytes) > (start_ptr + dsize))
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                        "Offset plus number of bytes to read puts cursor past "
                        "end of payload - not evaluating.\n"));
            return RULE_NOMATCH;
        }

        bt_ptr = start_ptr + bt_data->offset;
    }

    /* Determine which byte order to use */
    if (ropts->stub_data == NULL)
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "Stub data is NULL.  Setting byte order to that "
                    "of the header.\n"));
        byte_order = (DceRpcBoFlag)ropts->hdr_byte_order;
    }
    else if (bt_ptr < ropts->stub_data)
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "Reading data in the header.  Setting byte order "
                    "to that of the header.\n"));
        byte_order = (DceRpcBoFlag)ropts->hdr_byte_order;
    }
    else
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "Reading data in the stub.  Setting byte order "
                    "to that of the stub data.\n"));
        byte_order = (DceRpcBoFlag)ropts->data_byte_order;
    }

    /* Get the value */
    switch (bt_data->num_bytes)
    {
        case 1:
            pkt_value = *((uint8_t *)bt_ptr);
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                        "Got 1 byte: %u.\n", pkt_value));
            break;
        case 2:
            pkt_value = DceRpcNtohs((uint16_t *)bt_ptr, byte_order);
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                        "Got 2 bytes: %u.\n", pkt_value));
            break;
        case 4:
            pkt_value = DceRpcNtohl((uint32_t *)bt_ptr, byte_order);
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                        "Got 4 bytes: %u.\n", pkt_value));
            break;
        default:
            return RULE_NOMATCH;
    }

    /* Invert the return value. */
    if (bt_data->invert)
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS, "Applying not flag.\n"));
        ret = RULE_MATCH;
    }

    switch (bt_data->operator)
    {
        case DCE2_BT_OP__LT:
            if (pkt_value < bt_data->value)
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                            "Packet value (%u) < Option value (%u).\n",
                            pkt_value, bt_data->value));
                if (ret == RULE_MATCH)
                    ret = RULE_NOMATCH;
                else
                    ret = RULE_MATCH;
            }

            break;

        case DCE2_BT_OP__EQ:
            if (pkt_value == bt_data->value)
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                            "Packet value (%u) == Option value (%u).\n",
                            pkt_value, bt_data->value));
                if (ret == RULE_MATCH)
                    ret = RULE_NOMATCH;
                else
                    ret = RULE_MATCH;
            }

            break;

        case DCE2_BT_OP__GT:
            if (pkt_value > bt_data->value)
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                            "Packet value (%u) > Option value (%u).\n",
                            pkt_value, bt_data->value));
                if (ret == RULE_MATCH)
                    ret = RULE_NOMATCH;
                else
                    ret = RULE_MATCH;
            }

            break;

        case DCE2_BT_OP__AND:
            if (pkt_value & bt_data->value)
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                            "Packet value (%08x) & Option value (%08x).\n",
                            pkt_value, bt_data->value));
                if (ret == RULE_MATCH)
                    ret = RULE_NOMATCH;
                else
                    ret = RULE_MATCH;
            }

            break;

        case DCE2_BT_OP__XOR:
            if (pkt_value ^ bt_data->value)
            {
                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                            "Packet value (%08x) ^ Option value (%08x).\n",
                            pkt_value, bt_data->value));
                if (ret == RULE_MATCH)
                    ret = RULE_NOMATCH;
                else
                    ret = RULE_MATCH;
            }

            break;

        default:
            return RULE_NOMATCH;
    }

#ifdef DEBUG_MSGS
    if (ret == RULE_MATCH)
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "\"%s\" Match.\n", DCE2_ROPT__BYTE_TEST));
    }
    else
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "\"%s\" Fail.\n", DCE2_ROPT__BYTE_TEST));
    }
#endif

    return ret;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static int DCE2_ByteJumpEval(void *pkt, const uint8_t **cursor, void *data)
{
    SFSnortPacket *p = (SFSnortPacket *)pkt;
    DCE2_SsnData *sd;
    DCE2_Roptions *ropts;
    DCE2_ByteJumpData *bj_data;
    const uint8_t *start_ptr;
    uint16_t dsize;
    const uint8_t *bj_ptr;
    uint32_t jmp_value;
    DceRpcBoFlag byte_order;

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                "Evaluating \"%s\" rule option.\n", DCE2_ROPT__BYTE_JUMP));

    if (*cursor == NULL)
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "Cursor is NULL - not evaluating.\n"));
        return RULE_NOMATCH;
    }

    if (!DCE2_RoptDoEval(p))
        return RULE_NOMATCH;

    sd = (DCE2_SsnData *)_dpd.sessionAPI->get_application_data(p->stream_session, PP_DCE2);
    if ((sd == NULL) || DCE2_SsnNoInspect(sd))
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "No session data - not evaluating.\n"));
        return RULE_NOMATCH;
    }

    ropts = &sd->ropts;

    if ((ropts->data_byte_order == DCE2_SENTINEL) ||
            (ropts->hdr_byte_order == DCE2_SENTINEL))
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "Data byte order or header byte order not set "
                    "in rule options - not evaluating.\n"));
        return RULE_NOMATCH;
    }

    bj_data = (DCE2_ByteJumpData *)data;
    if (bj_data == NULL)
        return RULE_NOMATCH;

    if (_dpd.Is_DetectFlag(SF_FLAG_ALT_DETECT))
    {
        _dpd.GetAltDetect((uint8_t **)&start_ptr, &dsize);
        DEBUG_WRAP(DebugMessage(DEBUG_PATTERN_MATCH,
                    "Using Alternative Detect buffer!\n"););
    }
    else
    {
        start_ptr = p->payload;
        dsize = p->payload_size;
    }

    /* Make sure we don't read past the end of the payload or before
     * beginning of payload */
    if (bj_data->relative)
    {
        if ((bj_data->offset < 0) && (*cursor + bj_data->offset) < start_ptr)
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                        "Offset is negative and puts cursor before beginning "
                        "of payload - not evaluating.\n"));
            return RULE_NOMATCH;
        }

        if ((*cursor + bj_data->offset + bj_data->num_bytes) > (start_ptr + dsize))
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                        "Offset plus number of bytes to read puts cursor past "
                        "end of payload - not evaluating.\n"));
            return RULE_NOMATCH;
        }

        bj_ptr = *cursor + bj_data->offset;
    }
    else
    {
        if (bj_data->offset < 0)
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                        "Offset is negative but is not relative - "
                        "not evaluating.\n"));
            return RULE_NOMATCH;
        }
        else if ((start_ptr + bj_data->offset + bj_data->num_bytes) > (start_ptr + dsize))
        {
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                        "Offset plus number of bytes to read puts cursor past "
                        "end of payload - not evaluating.\n"));
            return RULE_NOMATCH;
        }

        bj_ptr = start_ptr + bj_data->offset;
    }

    /* Determine which byte order to use */
    if (ropts->stub_data == NULL)
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS, "Stub data is NULL.  "
                    "Setting byte order to that of the header.\n"));
        byte_order = (DceRpcBoFlag)ropts->hdr_byte_order;
    }
    else if (bj_ptr < ropts->stub_data)
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "Reading data in the header.  Setting byte order "
                    "to that of the header.\n"));
        byte_order = (DceRpcBoFlag)ropts->hdr_byte_order;
    }
    else
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "Reading data in the stub.  Setting byte order "
                    "to that of the stub data.\n"));
        byte_order = (DceRpcBoFlag)ropts->data_byte_order;
    }

    /* Get the value */
    switch (bj_data->num_bytes)
    {
        case 1:
            jmp_value = *((uint8_t *)bj_ptr);
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                        "Got 1 byte: %u.\n", jmp_value));
            break;
        case 2:
            jmp_value = DceRpcNtohs((uint16_t *)bj_ptr, byte_order);
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                        "Got 2 bytes: %u.\n", jmp_value));
            break;
        case 4:
            jmp_value = DceRpcNtohl((uint32_t *)bj_ptr, byte_order);
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                        "Got 4 bytes: %u.\n", jmp_value));
            break;
        default:
            return 0;
    }

    if (bj_data->multiplier != DCE2_SENTINEL)
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "Applying multiplier: %u * %u = %u.\n",
                    jmp_value, bj_data->multiplier,
                    jmp_value * bj_data->multiplier));

        jmp_value *= bj_data->multiplier;
    }

    if (bj_data->align && (jmp_value & 3))
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "Aligning to 4 byte boundary: %u => %u.\n",
                    jmp_value, jmp_value + (4 - (jmp_value & 3))));

        jmp_value += (4 - (jmp_value & 3));
    }

    bj_ptr += bj_data->num_bytes + jmp_value + bj_data->post_offset;
    if ((bj_ptr < start_ptr) || (bj_ptr >= (start_ptr + dsize)))
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "\"%s\" Fail.  Jump puts us past end of payload.\n",
                    DCE2_ROPT__BYTE_JUMP));
        return RULE_NOMATCH;
    }

    *cursor = bj_ptr;

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                "\"%s\" Match.\n", DCE2_ROPT__BYTE_JUMP));

    return RULE_MATCH;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static inline int DCE2_RoptDoEval(SFSnortPacket *p)
{
    if ((p->payload_size == 0) ||
            (p->stream_session == NULL) ||
            (!IsTCP(p) && !IsUDP(p)))
    {

        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS, "No payload or no "
                    "session pointer or not TCP or UDP - not evaluating.\n"));
        return 0;
    }

    return 1;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static void DCE2_IfaceCleanup(void *data)
{
    if (data == NULL)
        return;

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MEMORY,
                "Cleaning Iface data: %u bytes.\n", sizeof(DCE2_IfaceData)));

    DCE2_Free(data, sizeof(DCE2_IfaceData), DCE2_MEM_TYPE__ROPTION);
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static void DCE2_OpnumCleanup(void *data)
{
    DCE2_OpnumData *odata = (DCE2_OpnumData *)data;

    if (data == NULL)
        return;

    switch (odata->type)
    {
        case DCE2_OPNUM_TYPE__SINGLE:
            DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MEMORY,
                        "Cleaning Single opnum data: %u bytes.\n",
                        sizeof(DCE2_OpnumSingle)));

            DCE2_Free((void *)odata, sizeof(DCE2_OpnumSingle), DCE2_MEM_TYPE__ROPTION);

            break;

        case DCE2_OPNUM_TYPE__MULTIPLE:
            {
                DCE2_OpnumMultiple *omult = (DCE2_OpnumMultiple *)odata;

                DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MEMORY,
                            "Cleaning Multiple opnum data: %u bytes.\n",
                            sizeof(DCE2_OpnumMultiple) + omult->mask_size));

                if (omult->mask != NULL)
                    DCE2_Free((void *)omult->mask, omult->mask_size, DCE2_MEM_TYPE__ROPTION);

                DCE2_Free((void *)omult, sizeof(DCE2_OpnumMultiple), DCE2_MEM_TYPE__ROPTION);
            }

            break;

        default:
            break;
    }
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static void DCE2_ByteTestCleanup(void *data)
{
    if (data == NULL)
        return;

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MEMORY,
                "Cleaning ByteTest data: %u bytes.\n",
                sizeof(DCE2_ByteTestData)));

    DCE2_Free(data, sizeof(DCE2_ByteTestData), DCE2_MEM_TYPE__ROPTION);
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static void DCE2_ByteJumpCleanup(void *data)
{
    if (data == NULL)
        return;

    DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__MEMORY,
                "Cleaning ByteJump data: %u bytes.\n",
                sizeof(DCE2_ByteJumpData)));

    DCE2_Free(data, sizeof(DCE2_ByteJumpData), DCE2_MEM_TYPE__ROPTION);
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static uint32_t DCE2_IfaceHash(void *data)
{
    uint32_t a, b, c;
    DCE2_IfaceData *iface_data = (DCE2_IfaceData *)data;

    if (iface_data == NULL)
        return 0;

    a = iface_data->iface.time_low;
    b = (iface_data->iface.time_mid << 16) | (iface_data->iface.time_high_and_version);
    c = (iface_data->iface.clock_seq_and_reserved << 24) |
        (iface_data->iface.clock_seq_low << 16) |
        (iface_data->iface.node[0] << 8) |
        (iface_data->iface.node[1]);

    mix(a, b, c);

    a += (iface_data->iface.node[2] << 24) |
        (iface_data->iface.node[3] << 16) |
        (iface_data->iface.node[4] << 8) |
        (iface_data->iface.node[5]);
    b += iface_data->iface_vers;
    c += iface_data->iface_vers_maj;

    mix(a, b, c);

    a += iface_data->iface_vers_min;
    b += iface_data->operator;
    c += iface_data->any_frag;

    final(a, b, c);

    return c;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static uint32_t DCE2_OpnumHash(void *data)
{
    uint32_t a = 0, b = 0, c = 0;
    DCE2_OpnumData *odata = (DCE2_OpnumData *)data;

    if (odata == NULL)
        return 0;

    switch (odata->type)
    {
        case DCE2_OPNUM_TYPE__SINGLE:
            {
                DCE2_OpnumSingle *osingle = (DCE2_OpnumSingle *)odata;

                a = odata->type;
                b = osingle->opnum;
                c = 10;

                final(a, b, c);
            }

            break;

        case DCE2_OPNUM_TYPE__MULTIPLE:
            {
                DCE2_OpnumMultiple *omult = (DCE2_OpnumMultiple *)odata;
                unsigned int i;

                a = odata->type;
                b = omult->mask_size;
                c = 0;

                /* Don't care about potential wrapping if it exists */
                for (i = 0; i < omult->mask_size; i++)
                    c += omult->mask[i];

                mix(a, b, c);

                a = omult->opnum_lo;
                b = omult->opnum_hi;
                c = 10;

                final(a, b, c);
            }

            break;

        default:
            DCE2_Die("%s(%d) Invalid opnum type: %d",
                    __FILE__, __LINE__, odata->type);
            break;
    }

    return c;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static uint32_t DCE2_ByteTestHash(void *data)
{
    uint32_t a, b, c;
    DCE2_ByteTestData *bt_data = (DCE2_ByteTestData *)data;

    if (bt_data == NULL)
        return 0;

    a = bt_data->num_bytes;
    b = bt_data->value;
    c = bt_data->invert;

    mix(a, b, c);

    a += bt_data->operator;
    b += bt_data->offset;
    c += bt_data->relative;

    final(a, b, c);

    return c;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static uint32_t DCE2_ByteJumpHash(void *data)
{
    uint32_t a, b, c;
    DCE2_ByteJumpData *bj_data = (DCE2_ByteJumpData *)data;

    if (bj_data == NULL)
        return 0;

    a = bj_data->num_bytes;
    b = bj_data->offset;
    c = bj_data->relative;

    mix(a, b, c);

    a += bj_data->multiplier;
    b += bj_data->align;

    final(a, b, c);

    return c;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static int DCE2_IfaceKeyCompare(void *l, void *r)
{
    DCE2_IfaceData *left = (DCE2_IfaceData *)l;
    DCE2_IfaceData *right = (DCE2_IfaceData *)r;

    if ((left == NULL) || (right == NULL))
        return PREPROC_OPT_NOT_EQUAL;

    if ((DCE2_UuidCompare(&left->iface, &right->iface) == 0) &&
            (left->iface_vers == right->iface_vers) &&
            (left->iface_vers_maj == right->iface_vers_maj) &&
            (left->iface_vers_min == right->iface_vers_min) &&
            (left->operator == right->operator) &&
            (left->any_frag == right->any_frag))
    {
        return PREPROC_OPT_EQUAL;
    }

    return PREPROC_OPT_NOT_EQUAL;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static int DCE2_OpnumKeyCompare(void *l, void *r)
{
    DCE2_OpnumData *left = (DCE2_OpnumData *)l;
    DCE2_OpnumData *right = (DCE2_OpnumData *)r;

    if ((left == NULL) || (right == NULL))
        return PREPROC_OPT_NOT_EQUAL;

    if (left->type != right->type)
        return PREPROC_OPT_NOT_EQUAL;

    switch (left->type)
    {
        case DCE2_OPNUM_TYPE__SINGLE:
            {
                DCE2_OpnumSingle *lsingle = (DCE2_OpnumSingle *)left;
                DCE2_OpnumSingle *rsingle = (DCE2_OpnumSingle *)right;

                if (lsingle->opnum != rsingle->opnum)
                    return PREPROC_OPT_NOT_EQUAL;

            }

            break;

        case DCE2_OPNUM_TYPE__MULTIPLE:
            {
                unsigned int i;
                DCE2_OpnumMultiple *lmult = (DCE2_OpnumMultiple *)left;
                DCE2_OpnumMultiple *rmult = (DCE2_OpnumMultiple *)right;

                if ((lmult->mask_size != rmult->mask_size) ||
                        (lmult->opnum_lo != rmult->opnum_lo) ||
                        (lmult->opnum_hi != rmult->opnum_hi))
                {
                    return PREPROC_OPT_NOT_EQUAL;
                }

                for (i = 0; i < lmult->mask_size; i++)
                {
                    if (lmult->mask[i] != rmult->mask[i])
                        return PREPROC_OPT_NOT_EQUAL;
                }
            }

            break;

        default:
            DCE2_Die("%s(%d) Invalid opnum type: %d",
                    __FILE__, __LINE__, left->type);
            break;
    }

    return PREPROC_OPT_EQUAL;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static int DCE2_ByteTestKeyCompare(void *l, void *r)
{
    DCE2_ByteTestData *left = (DCE2_ByteTestData *)l;
    DCE2_ByteTestData *right = (DCE2_ByteTestData *)r;

    if ((left == NULL) || (right == NULL))
        return PREPROC_OPT_NOT_EQUAL;

    if ((left->num_bytes == right->num_bytes) &&
            (left->value == right->value) &&
            (left->invert == right->invert) &&
            (left->operator == right->operator) &&
            (left->offset == right->offset) &&
            (left->relative == right->relative))
    {
        return PREPROC_OPT_EQUAL;
    }

    return PREPROC_OPT_NOT_EQUAL;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
static int DCE2_ByteJumpKeyCompare(void *l, void *r)
{
    DCE2_ByteJumpData *left = (DCE2_ByteJumpData *)l;
    DCE2_ByteJumpData *right = (DCE2_ByteJumpData *)r;

    if ((left == NULL) || (right == NULL))
        return PREPROC_OPT_NOT_EQUAL;

    if ((left->num_bytes == right->num_bytes) &&
            (left->offset == right->offset) &&
            (left->relative == right->relative) &&
            (left->multiplier == right->multiplier) &&
            (left->align == right->align))
    {
        return PREPROC_OPT_EQUAL;
    }

    return PREPROC_OPT_NOT_EQUAL;
}

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 ********************************************************************/
void DCE2_PrintRoptions(DCE2_Roptions *ropts)
{
    printf("  First frag: %s\n", ropts->first_frag == 1 ? "yes" : (ropts->first_frag == 0 ? "no" : "unset"));
    if (ropts->first_frag == DCE2_SENTINEL)
    {
        printf("  Iface: unset\n");
        printf("  Iface version: unset\n");
    }
    else
    {
        printf("  Iface: %s\n", DCE2_UuidToStr(&ropts->iface, DCERPC_BO_FLAG__NONE));
        printf("  Iface version: %u\n", ropts->iface_vers_maj);
    }
    if (ropts->opnum == DCE2_SENTINEL) printf("  Opnum: unset\n");
    else printf("  Opnum: %u\n", ropts->opnum);
    printf("  Header byte order: %s\n",
            ropts->hdr_byte_order == DCERPC_BO_FLAG__LITTLE_ENDIAN ? "little endian" :
            (ropts->hdr_byte_order == DCERPC_BO_FLAG__BIG_ENDIAN ? "big endian" : "unset"));
    printf("  Data byte order: %s\n",
            ropts->data_byte_order == DCERPC_BO_FLAG__LITTLE_ENDIAN ? "little endian" :
            (ropts->data_byte_order == DCERPC_BO_FLAG__BIG_ENDIAN ? "big endian" : "unset"));
    if (ropts->stub_data != NULL) printf("  Stub data: %p\n", ropts->stub_data);
    else printf("  Stub data: NULL\n");
}

/********************************************************************
 * Function: DCE2_RoptError()
 *
 * Prints rule option error and dies.
 *
 * Arguments:
 *  const char *
 *      Format string
 *  ...
 *      Arguments to format string
 *
 * Returns: None
 *
 ********************************************************************/
static NORETURN void DCE2_RoptError(const char *format, ...)
{
    char buf[1024];
    va_list ap;

    va_start(ap, format);
    vsnprintf(buf, sizeof(buf), format, ap);
    va_end(ap);

    buf[sizeof(buf) - 1] = '\0';

    DCE2_Die("%s(%d): %s  Please consult documentation.",
            *_dpd.config_file, *_dpd.config_line, buf);
}


/**********************************
 * Function: DCE2_GetByteOrder()
 *
 * Gets the byte order needed for a byte_test, byte_jump, or byte_extract.
 *
 * Arguments:
 *  Packet *
 *    packet being evaluated
 *  int32_t
 *    offset into the packet payload where the rule will be evaluated.
 *    calling function is responsible for checking that the offset is in-bounds.
 *
 * Returns:
 *  DCE2_SENTINEL (-1) if byte order not set, or otherwise not evaluating
 *  BIG (0) if byte order is big-endian
 *  LITTLE (1) if byte order is little-endian
 *
 **********************************/
#define BIG 0
#define LITTLE 1
int DCE2_GetByteOrder(void *data, int32_t offset)
{
    DCE2_SsnData *sd;
    DCE2_Roptions *ropts;
    DceRpcBoFlag byte_order;
    const uint8_t *data_ptr;
    SFSnortPacket *p = (SFSnortPacket *)data;

    if (p == NULL)
        return -1;

    sd = (DCE2_SsnData *)_dpd.sessionAPI->get_application_data(p->stream_session, PP_DCE2);
    if ((sd == NULL) || DCE2_SsnNoInspect(sd))
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "No session data - not evaluating.\n"));
        return -1;
    }

    ropts = &sd->ropts;

    if ((ropts->data_byte_order == DCE2_SENTINEL) ||
            (ropts->hdr_byte_order == DCE2_SENTINEL))
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "Data byte order or header byte order not set "
                    "in rule options - not evaluating.\n"));
        return -1;
    }

    /* Determine which byte order to use */
    data_ptr = p->payload + offset;

    if (ropts->stub_data == NULL)
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS, "Stub data is NULL.  "
                    "Setting byte order to that of the header.\n"));
        byte_order = (DceRpcBoFlag)ropts->hdr_byte_order;
    }
    else if (data_ptr < ropts->stub_data)
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "Reading data in the header.  Setting byte order "
                    "to that of the header.\n"));
        byte_order = (DceRpcBoFlag)ropts->hdr_byte_order;
    }
    else
    {
        DEBUG_WRAP(DCE2_DebugMsg(DCE2_DEBUG__ROPTIONS,
                    "Reading data in the stub.  Setting byte order "
                    "to that of the stub data.\n"));
        byte_order = (DceRpcBoFlag)ropts->data_byte_order;
    }

    /* Return ints, since this enum doesn't exist back in Snort-land. */
    if (byte_order == DCERPC_BO_FLAG__BIG_ENDIAN)
        return BIG;
    if (byte_order == DCERPC_BO_FLAG__LITTLE_ENDIAN)
        return LITTLE;

    return -1;
}
