/******************************************************************************
 *
 * Name:    fwimage.h
 * Project: fwcommon
 * Version: $Revision: #4 $
 * Date:    $Date: 2010/11/04 $
 * Purpose: firmware image function
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

#ifndef __SK_FWIMAGE_H__
#define __SK_FWIMAGE_H__

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

#define FW_MAX_PATCH_NUMBER 10

/*
 * ASF specific adapter context structure
 */
typedef struct s_Patch{

    SK_U8*  Ptr;
    SK_U32  Offset; 
    SK_U32  Size;

}SK_PATCH;

extern void  FwFreePatchMemory( SK_AC *pAC);
extern void FwReadFwVersionFromFile( SK_AC *pAC, SK_FW_HANDLE fh);
extern void FwAddPatch( SK_AC *pAC, SK_U32 Offset, SK_U32 Size, SK_U8 * Ptr);
extern SK_U32 FwGetImageWord( SK_AC * pAC, SK_FW_HANDLE fh, SK_U32  Off);
extern SK_U32 FwChkSumImage( SK_AC * pAC, SK_FW_HANDLE fh);
extern SK_U32 FwGetFlashWord( SK_AC *pAC, SK_IOC IoC, SK_U32 Off);
extern SK_U32 FwChkSumFlash( SK_AC *pAC, SK_IOC IoC);
extern void	FwReadFwVersionFromFlash(  SK_AC *pAC, SK_IOC IoC);
extern SK_BOOL FwIsImageInFlashOk( SK_AC *pAC, SK_IOC IoC, SK_U32 CSum);
extern SK_BOOL FwIsImageNewer(SK_AC *pAC);
extern SK_BOOL FwProgrammSPI(
	SK_AC			*pAC,	/* Pointer to adapter context */
	SK_IOC			IoC,	/* IO context handle */
	SK_FW_HANDLE	fh,	/* image file handler */
	SK_U32			ImageOff,
	SK_U32			SPIOff,
	SK_U32			Size);
extern SK_BOOL FwProgrammPFlash(
	SK_AC			*pAC,	/* Pointer to adapter context */
	SK_IOC			IoC,	/* IO context handle */
	SK_FW_HANDLE	fh,	/* image file handler */
	SK_U32			ImageOff,
	SK_U32			PFlashOff,
	SK_U32			Size);
extern SK_BOOL FwProgrammImage( SK_AC *pAC, SK_IOC IoC, SK_FW_HANDLE fh);
extern SK_BOOL FwIsImageOk( SK_AC *pAC, SK_FW_HANDLE fh);
extern SK_U32	FwGetSpiCheckSum( SK_AC *pAC, SK_IOC IoC);
extern SK_U32	FwGetSpiCodeSize( SK_AC *pAC, SK_IOC IoC);
extern SK_BOOL FwIsSpiOk( SK_AC *pAC, SK_IOC IoC);
extern SK_BOOL FwIsFwInFlashOk( SK_AC *pAC, SK_IOC IoC);
extern int FwRemoveFirmware( SK_AC   *pAC, SK_IOC  IoC);
extern SK_U8 FwCheckSPI( SK_AC *pAC, SK_IOC IoC);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* __SK_FWIMAGE_H__ */

