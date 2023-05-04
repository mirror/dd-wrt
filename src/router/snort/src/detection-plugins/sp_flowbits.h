/* $Id$ */
/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2004-2013 Sourcefire, Inc.
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
 ****************************************************************************/
 
/* flowbits detection plugin header */

#ifndef __SP_FLOWBITS_H__
#define __SP_FLOWBITS_H__

#include "sfghash.h"
#include "sf_types.h"
#include "decode.h"
#include "bitop_funcs.h"
#include "snort_debug.h"

/* Normally exported functions, for plugin registration. */
void SetupFlowBits(void);
void FlowBitsVerify(void);
void FlowBitsFree(void *d);
uint32_t FlowBitsHash(void *d);
int FlowBitsCompare(void *l, void *r);
int FlowBitsCheck(void *, Packet *);
void FlowBitsHashInit(void);
void FlowbitResetCounts(void);


/**
**  The FLOWBITS_OBJECT is used to track the different
**  flowbit names that set/unset/etc. bits.  We use these
**  so that we can verify that the rules that use flowbits
**  make sense.
**
**  The types element tracks all the different operations that
**  may occur for a given object.  This is different from how
**  the type element is used from the FLOWBITS_ITEM structure.
*/
typedef struct _FLOWBITS_OBJECT
{
    uint16_t id;
    uint8_t  types;
    int toggle;
    int set;
    int isset;

} FLOWBITS_OBJECT;

typedef enum
{
    FLOWBITS_AND,
    FLOWBITS_OR,
    FLOWBITS_ANY,
    FLOWBITS_ALL
}Flowbits_eval;

/**
**  This structure is the context ptr for each detection option
**  on a rule.  The id is associated with a FLOWBITS_OBJECT id.
**
**  The type element track only one operation.
*/
typedef struct _FLOWBITS_OP
{
    uint16_t *ids;
    uint8_t  num_ids;
    uint8_t  type;        /* Set, Unset, Invert, IsSet, IsNotSet, Reset  */
    Flowbits_eval  eval;  /* and , or, all, any*/
    char *name;
    char *group;
    uint32_t group_id;
} FLOWBITS_OP;

typedef struct _FLOWBITS_GRP
{
    uint16_t count;
    uint16_t max_id;
    char *name;
    uint32_t group_id;
    BITOP GrpBitOp;
} FLOWBITS_GRP;

#define FLOWBITS_SET       0x01
#define FLOWBITS_UNSET     0x02
#define FLOWBITS_TOGGLE    0x04
#define FLOWBITS_ISSET     0x08
#define FLOWBITS_ISNOTSET  0x10
#define FLOWBITS_RESET     0x20
#define FLOWBITS_NOALERT   0x40
#define FLOWBITS_SETX      0x80

void processFlowBitsWithGroup(char *flowbitsName, char *groupName, FLOWBITS_OP *flowbits);
int checkFlowBits( uint8_t type, uint8_t evalType, uint16_t *ids, uint16_t num_ids, char *group, Packet *p);

static inline int FlowBits_SetOperation(void *option_data)
{
    FLOWBITS_OP *flowbits = (FLOWBITS_OP*)option_data;
    if (flowbits->type & (FLOWBITS_SET | FLOWBITS_SETX |FLOWBITS_UNSET | FLOWBITS_TOGGLE | FLOWBITS_RESET))
    {
        return 1;
    }
    return 0;
}

void setFlowbitSize(char *);
unsigned int getFlowbitSize(void);
unsigned int getFlowbitSizeInBytes(void);

#endif  /* __SP_FLOWBITS_H__ */
