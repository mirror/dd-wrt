/******************************************************************************
 *
 * Name:    fwmain.h
 * Project: fwcommon
 * Version: $Revision: #4 $
 * Date:    $Date: 2010/11/04 $
 * Purpose: common firmware interface for driver
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

#ifndef __SK_FWMAIN_H__
#define __SK_FWMAIN_H__

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define SKERR_ASF_E001      (SK_ERRBASE_ASF)
#define SKERR_ASF_E001MSG   "SkAsfInit() error: wrong HCI version"
#define SKERR_ASF_E002      (SKERR_ASF_E001+1)
#define SKERR_ASF_E002MSG   "SkAsfInit() error: flash read"
#define SKERR_ASF_E003      (SKERR_ASF_E001+2)
#define SKERR_ASF_E003MSG   "SkAsfInit() error: flash erase"
#define SKERR_ASF_E004      (SKERR_ASF_E001+3)
#define SKERR_ASF_E004MSG   "SkAsfInit() error: flash write"
#define SKERR_ASF_E005      (SKERR_ASF_E001+4)
#define SKERR_ASF_E005MSG   "SkAsfInit() error: map FW image"
#define SKERR_ASF_E006      (SKERR_ASF_E001+5)
#define SKERR_ASF_E006MSG   "SkAsfInit() error: flash reread"
#define SKERR_ASF_E007      (SKERR_ASF_E001+6)
#define SKERR_ASF_E007MSG   "SkAsfInit() error: flash compare"
#define SKERR_ASF_E008      (SKERR_ASF_E001+7)
#define SKERR_ASF_E008MSG   "SkAsfInit() flash successfully updated"
#define SKERR_ASF_E009      (SKERR_ASF_E001+8)
#define SKERR_ASF_E009MSG   "SkAsfInit() updating flash"
#define SKERR_ASF_E010      (SKERR_ASF_E001+9)
#define SKERR_ASF_E010MSG   "SkAsfInit() CLOCK DIV"

#define SK_FW_DBG(x)        0

#define FW_DBG_MSG_IE(x)    SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT | SK_DBGCAT_ERR, x)
#define FW_DBG_MSG_I(x)     SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT, x)

extern void FwInit0( SK_AC *pAC, SK_IOC IoC);
extern void FwInit1( SK_AC *pAC, SK_IOC IoC);
extern SK_BOOL FwRemoveImage( SK_AC *pAC, SK_IOC IoC);
extern SK_BOOL FwInit2( SK_AC *pAC, SK_IOC IoC);
extern int FwInit( SK_AC *pAC, SK_IOC IoC, int Level);
extern int FwDeInit( SK_AC *pAC, SK_IOC IoC);
extern int FwDeInitStandBy(SK_AC *pAC, SK_IOC IoC);
extern int FwInitStandBy( SK_AC *pAC, SK_IOC IoC, int Level);
extern int FwEvent( SK_AC *pAC, SK_IOC IoC, SK_U32 Event, SK_EVPARA Param);
extern void FwResetCpu(	SK_AC *pAC, SK_IOC IoC);
extern void FwSmartResetCpu( SK_AC *pAC, SK_IOC IoC, SK_U8 Cold);
extern void FwStartCpu( SK_AC *pAC, SK_IOC IoC);
extern void FwAsfEnable( SK_AC *pAC, SK_IOC IoC);
extern void FwAsfDisable( SK_AC *pAC, SK_IOC IoC);
extern SK_U8 FwCpuState( SK_AC *pAC, SK_IOC IoC);
extern void FwSetOsPresentBit( SK_AC *pAC, SK_IOC IoC);
extern void FwResetOsPresentBit( SK_AC *pAC, SK_IOC IoC);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* __SK_FWMAIN_H__ */

