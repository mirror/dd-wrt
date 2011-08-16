/****************************************************************************
 * Copyright (C) 2008-2011 Sourcefire, Inc.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************
 * 
 ****************************************************************************/

#ifndef _SNORT_DCE2_H_
#define _SNORT_DCE2_H_

#include "dce2_utils.h"
#include "dce2_session.h"
#include "sf_snort_packet.h"
#include "sf_types.h"
#include "debug.h"

/********************************************************************
 * Macros
 ********************************************************************/
#define DCE2_PROTO_REF_STR__NBSS    "netbios-ssn"   /* Essentially SMB */
#define DCE2_PROTO_REF_STR__DCERPC  "dcerpc"

/********************************************************************
 * Enumerations
 ********************************************************************/
typedef enum _DCE2_RpktType
{
    DCE2_RPKT_TYPE__NULL = 0,
    DCE2_RPKT_TYPE__SMB_SEG,
    DCE2_RPKT_TYPE__SMB_TRANS,
    DCE2_RPKT_TYPE__SMB_CO_SEG,
    DCE2_RPKT_TYPE__SMB_CO_FRAG,
    DCE2_RPKT_TYPE__TCP_CO_SEG,
    DCE2_RPKT_TYPE__TCP_CO_FRAG,
    DCE2_RPKT_TYPE__UDP_CL_FRAG,
    DCE2_RPKT_TYPE__MAX

} DCE2_RpktType;

/********************************************************************
 * Structures
 ********************************************************************/
typedef struct _DCE2_ProtoIds
{
    int16_t dcerpc;
    int16_t nbss;

} DCE2_ProtoIds;

/********************************************************************
 * Public function prototypes
 ********************************************************************/
DCE2_Ret DCE2_Process(SFSnortPacket *);
void DCE2_InitRpkts(void);
SFSnortPacket * DCE2_GetRpkt(const SFSnortPacket *, DCE2_RpktType, const uint8_t *, uint32_t);
DCE2_Ret DCE2_AddDataToRpkt(SFSnortPacket *, DCE2_RpktType, const uint8_t *, uint32_t);
DCE2_Ret DCE2_PushPkt(SFSnortPacket *);
void DCE2_PopPkt(void);
void DCE2_Detect(DCE2_SsnData *);
uint16_t DCE2_GetRpktMaxData(DCE2_SsnData *, DCE2_RpktType);
void DCE2_FreeGlobals(void);

/********************************************************************
 * Inline function prototypes
 ********************************************************************/
static INLINE void DCE2_ResetRopts(DCE2_Roptions *);
static INLINE void DCE2_DisableDetect(SFSnortPacket *);

/********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns: None
 *
 ********************************************************************/
static INLINE void DCE2_ResetRopts(DCE2_Roptions *ropts)
{
    ropts->first_frag = DCE2_SENTINEL;
    ropts->opnum = DCE2_SENTINEL;
    ropts->hdr_byte_order = DCE2_SENTINEL;
    ropts->data_byte_order = DCE2_SENTINEL;
    ropts->stub_data = NULL;
}

/*********************************************************************
 * Function:
 *
 * Purpose:
 *
 * Arguments:
 *
 * Returns:
 *
 *********************************************************************/
static INLINE void DCE2_DisableDetect(SFSnortPacket *p)
{
    _dpd.disableAllDetect(p);
    _dpd.setPreprocBit(p, PP_SFPORTSCAN);
    _dpd.setPreprocBit(p, PP_PERFMONITOR);
    _dpd.setPreprocBit(p, PP_SDF);
}

#endif  /* _SNORT_DCE2_H_ */

