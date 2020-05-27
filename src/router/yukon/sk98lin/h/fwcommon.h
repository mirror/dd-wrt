/******************************************************************************
 *
 * Name:    fwcommon.h
 * Project: firmware common modules
 * Version: $Revision: #4 $
 * Date:    $Date: 2010/11/04 $
 * Purpose: firmware common function
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	LICENSE:
 *	(C)Copyright Marvell.
 *	
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; either version 2 of the License, or
 *	(at your option) any later version.
 *	
 *	The information in this file is provided "AS IS" without warranty.
 *	/LICENSE
 *
 ******************************************************************************/

#ifndef __SK_FWCOMMON_H__
#define __SK_FWCOMMON_H__

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#include "fwhci.h"
#include "fwos.h"
#include "fwapp.h"
#include "fwimage.h"
#include "fwptrn.h"
#include "fwoids.h"
#include "fwmain.h"



/*  Return codes to the PNMI module  */
#define SK_ASF_PNMI_ERR_OK              0
#define SK_ASF_PNMI_ERR_GENERAL         1
#define SK_ASF_PNMI_ERR_TOO_SHORT       2
#define SK_ASF_PNMI_ERR_BAD_VALUE       3
#define SK_ASF_PNMI_ERR_READ_ONLY       4
#define SK_ASF_PNMI_ERR_UNKNOWN_OID     5
#define SK_ASF_PNMI_ERR_UNKNOWN_INST    6
#define SK_ASF_PNMI_ERR_UNKNOWN_NET     7
#define SK_ASF_PNMI_ERR_NOT_SUPPORTED   10


#define ASF_CPU_STATE_UNKNOWN       0
#define ASF_CPU_STATE_RESET         1
#define ASF_CPU_STATE_RUNNING       2



typedef struct s_FwCommon {

    SK_U8       ChipID;
    char*       ImageName;
    SK_U32      ImageSize;
    char        FileVersion[5+1];
    char        FlashVersion[5+1];
    SK_U32      PatchNumber;
    SK_PATCH    Patch[FW_MAX_PATCH_NUMBER];
    SK_U32      VersionOffset; 
    SK_U32      VersionLength;
    SK_U32      ImageChkSum;
    STR_HCI     Hci;

} SK_FWCOMMON;


/* Functions provided by SkGeAsf */

/* ANSI/C++ compliant function prototypes */

/*                                    
 * Public Function prototypes
 */
extern int FwDeInit(SK_AC *pAC, SK_IOC IoC );
extern int FwInit (SK_AC *pAC , SK_IOC IoC , int level);
extern int FwDeInitStandBy( SK_AC *pAC, SK_IOC IoC );
extern int FwInitStandBy( SK_AC *pAC, SK_IOC IoC, int Level );
extern int FwGet (SK_AC *pAC , SK_IOC IoC , SK_U8 *pBuf, SK_U32 *pLen);
extern int FwPreSet (SK_AC *pAC , SK_IOC IoC , SK_U8 *pBuf, SK_U32 *pLen);
extern int FwSet (SK_AC *pAC , SK_IOC IoC , SK_U8 *pBuf, SK_U32 *pLen);
extern int FwEvent (SK_AC *pAC , SK_IOC IoC , SK_U32 Event , SK_EVPARA Param);
extern int FwSetOid(SK_AC *pAC, SK_IOC IoC, SK_U32 Id, SK_U32 Inst, SK_U8 *pBuf, unsigned int *pLen);
extern int FwPreSetOid(SK_AC *pAC, SK_IOC IoC, SK_U32 Id, SK_U32 Inst, SK_U8 *pBuf, unsigned int *pLen);
extern int FwGetOid(SK_AC *pAC, SK_IOC IoC, SK_U32 Id, SK_U32 Inst, SK_U8 *pBuf, unsigned int *pLen);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* __SK_FWCOMMON_H__ */

