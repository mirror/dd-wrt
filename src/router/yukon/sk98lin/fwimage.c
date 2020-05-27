/******************************************************************************
 *
 * Name:    fwimage.c
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

#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"

static void FwLockSpi( SK_AC *pAC, SK_IOC  IoC );
static void FwUnlockSpi( SK_AC *pAC, SK_IOC  IoC );

/*****************************************************************************
*
* FwLockSpi
*
* Description:
*
* Returns:
*
*/
static void FwLockSpi(
	SK_AC  *pAC,
	SK_IOC IoC)
{
	SK_U8  TmpVal8;
	SK_U32 TmpVal32;
	SK_U32 i;

	SK_IN8( IoC, GPHY_CTRL + 2, &TmpVal8);
	TmpVal8 |= BIT_0;
	SK_OUT8( IoC, GPHY_CTRL + 2, TmpVal8);

	if( (pAC->FwApp.ChipMode == SK_GEASF_CHIP_EX) ||
		(pAC->FwApp.ChipMode == SK_GEASF_CHIP_SU)    )  {
		for( i=0; i<10; i++ )  {
			SK_IN32( IoC, SPI_Y2_CONTROL_REG, &TmpVal32);
			if( (TmpVal32 & ( BIT_30 )) == 0 )
				break;
			FwOsSleep( 1000 );  /* 1ms */
		}
	}
}

/*****************************************************************************
*
* FwUnlockSpi
*
* Description:
*
* Returns:
*
*/
static void FwUnlockSpi(
	SK_AC  *pAC,
	SK_IOC IoC)
{
	SK_U8 TmpVal8;

	SK_IN8( IoC, GPHY_CTRL + 2, &TmpVal8);
	TmpVal8 &= ~BIT_0;
	SK_OUT8( IoC, GPHY_CTRL + 2, TmpVal8);
}

void FwFreePatchMemory(
	SK_AC *pAC)  /* Pointer to adapter context */
{
	int i;

	for (i = 0; i < pAC->FwCommon.PatchNumber; i++) {
		FwOsFreeMemory(pAC, pAC->FwCommon.Patch[i].Ptr);
	}
}

void FwReadFwVersionFromFile(
	SK_AC        *pAC,  /* Pointer to adapter context */
	SK_FW_HANDLE fh)    /* file handler */
{
	pAC->FwCommon.VersionLength = 5;
	pAC->FwCommon.VersionOffset = ASF_FLASH_SU_OFFS_VER;
	FwOsFileRead(pAC, fh,
				 (SK_U8 *)pAC->FwCommon.FileVersion,
				 pAC->FwCommon.VersionOffset,
				 pAC->FwCommon.VersionLength);

	FW_DBG_MSG_I(("FwReadFwVersionFromFile: %s\n", pAC->FwCommon.FileVersion));
	SK_DBG_PRINTF("%s: FwReadFwVersionFromFile: %s\n", DRV_NAME,
		pAC->FwCommon.FileVersion);
}

void FwAddPatch(
	SK_AC  *pAC,     /* Pointer to adapter context */
	SK_U32 Offset,
	SK_U32 Size,
	SK_U8  *Ptr)
{
	pAC->FwCommon.Patch[pAC->FwCommon.PatchNumber].Offset = Offset;
	pAC->FwCommon.Patch[pAC->FwCommon.PatchNumber].Size = Size;
	pAC->FwCommon.Patch[pAC->FwCommon.PatchNumber].Ptr = Ptr;

	pAC->FwCommon.PatchNumber++;

	FW_DBG_MSG_I(("FwAddPatch Offset 0x%x, Size 0x%x, Ptr 0x%x\n", Offset, Size, Ptr));
}

SK_U32 FwGetImageWord(
	SK_AC        *pAC,  /* Pointer to adapter context */
	SK_FW_HANDLE fh,    /* image file handler */
	SK_U32       Off)   /* offset in image */
{
	SK_U32 Val32;
	int    i;

	for (i = 0; i < pAC->FwCommon.PatchNumber; i++) {

		if( Off >= pAC->FwCommon.Patch[i].Offset &&
			Off < (pAC->FwCommon.Patch[i].Offset +
				   pAC->FwCommon.Patch[i].Size)) {

			Val32 = (*((SK_U32*)(pAC->FwCommon.Patch[i].Ptr +
								(Off - pAC->FwCommon.Patch[i].Offset) )));

			if (Off + 4 > pAC->FwCommon.Patch[i].Offset +
				pAC->FwCommon.Patch[i].Size) {

				Val32 &= (0x00ffffff >> (3 - pAC->FwCommon.Patch[i].Size%4) * 8);
			}
			return LITTLEENDIAN_TO_HOST_32(Val32);
		}
	}

	FwOsFileRead(pAC, fh, (SK_U8 *) &Val32, Off, 4);

	return LITTLEENDIAN_TO_HOST_32(Val32);
}

SK_U32 FwChkSumImage(
	SK_AC        *pAC,  /* Pointer to adapter context */
	SK_FW_HANDLE fh)    /* image file handler */
{
	SK_U32 ImCs = 0;
	int i;

	/*   calculate CSum of the image including patches */

	pAC->FwApp.FlashSize = ASF_FLASH_SU_SIZE;

	for( i = 0; i < (pAC->FwApp.FlashSize - 4) ; i += 4 )  {
		ImCs += FwGetImageWord(pAC, fh, i);
	}
	FW_DBG_MSG_I(("FwChkSumImage: 0x%x\n", ImCs));

	return (ImCs);
}

SK_U32 FwGetFlashWord(
	SK_AC  *pAC,  /* Pointer to adapter context */
	SK_IOC IoC,   /* IO context handle */
	SK_U32 Off)
{
	SK_U32 Val32;

	if (SkPflManage(pAC, IoC, (SK_U8 *) &Val32, Off, 4, SK_PFL_READ) ) {

		FW_DBG_MSG_IE(("FwGetFlashWord ERROR: PFlash read timeout\n"));
	}
	return (Val32);
}

SK_U32 FwChkSumFlash(
	SK_AC  *pAC,  /* Pointer to adapter context */
	SK_IOC IoC)   /* IO context handle */
{
	SK_U32 FlCs = 0;
	int i;

	/*   calculate CSum of the immage including patches */
	if (!SkPflCheck( pAC, IoC) ) {
		FW_DBG_MSG_IE(("FwChkSumFlash ERROR: PFlash timeout\n"));
		return (0);
	}
	for( i=0; i < (pAC->FwApp.FlashSize - 4); i += 4 )  {

		FlCs += FwGetFlashWord(pAC, IoC, i);
	}
	FW_DBG_MSG_I(("FwChkSumFlash: 0x%x\n", FlCs));
	return (FlCs);
}

void FwReadFwVersionFromFlash(
	SK_AC  *pAC,  /* Pointer to adapter context */
	SK_IOC IoC)   /* IO context handle */
{
	unsigned char buf[4];
#ifndef	SK_LITTLE_ENDIAN
	SK_U32	*val32 = (SK_U32*)buf;
#endif
	int i;

	for (i = 0; i < pAC->FwCommon.VersionLength; i++) {

		if (i%4 == 0) { /*  one read for 4 byte */
			if (SkPflManage(pAC, IoC, buf,
							pAC->FwCommon.VersionOffset + i, 4, SK_PFL_READ) ) {
				FW_DBG_MSG_IE(("FwReadFwVersionFromFlash ERROR: version read timeout\n"));
				return;
			}
#ifndef	SK_LITTLE_ENDIAN
			*val32 = LITTLEENDIAN_TO_HOST_32(*val32);
#endif
		}
		pAC->FwCommon.FlashVersion[i] = buf[i%4];
	}
	FW_DBG_MSG_I(("FwReadFwVersionFromFlash: %s\n", pAC->FwCommon.FlashVersion));
}

SK_BOOL FwIsImageInFlashOk(
	SK_AC  *pAC,  /* Pointer to adapter context */
	SK_IOC IoC,   /* IO context handle */
	SK_U32 CSum)  /* flash checksum */
{
	SK_U32 Val32;

	if (SkPflManage(pAC, IoC, (SK_U8 *) &Val32, pAC->FwApp.FlashSize - 4, 4, SK_PFL_READ) ) {

		FW_DBG_MSG_IE(("FwGetFlashWord ERROR: PFlash CSum read timeout\n"));
	}
	if (Val32 + CSum) {
		FW_DBG_MSG_I(("FwIsImageInFlashOk: CSum is not ok!\n"));
		return SK_FALSE;
	}
	else {
		FW_DBG_MSG_I(("FwIsImageInFlashOk: CSum is ok\n"));
		return SK_TRUE;
	}
}

SK_BOOL FwIsImageNewer(SK_AC *pAC)  /* Pointer to adapter context */
{
	int i;

	for (i = 0; i < pAC->FwCommon.VersionLength; i++) {
		if (pAC->FwCommon.FileVersion[i] > pAC->FwCommon.FlashVersion[i] ) {
			FW_DBG_MSG_I(("FwIsImageNewer: yes\n"));
			return SK_TRUE;
		}
	}
	FW_DBG_MSG_I(("FwIsImageNewer: no\n"));
	return SK_FALSE;
}

SK_BOOL FwProgrammSPI(
	SK_AC        *pAC,      /* Pointer to adapter context */
	SK_IOC       IoC,       /* IO context handle */
	SK_FW_HANDLE fh,        /* image file handler */
	SK_U32       ImageOff,
	SK_U32       SPIOff,
	SK_U32       Size)
{
	SK_U32 i;
	SK_U32 Val32, Ver32;

	if( spi_flash_erase( pAC, IoC, SPIOff, Size))  {

			FW_DBG_MSG_IE(("FwProgrammSPI ERROR: SPI erase timeout\n"));
			return SK_FALSE;

	}

	for( i = 0; i < Size; i += 4 )  {

		FwOsFileRead(pAC, fh, (SK_U8 *) &Val32, ImageOff + i, 4);

		/*  write buffer to flash  */
		if( spi_flash_manage( pAC, IoC, (SK_U8 *) &Val32, SPIOff + i, 4, SPI_WRITE )) {

			FW_DBG_MSG_IE(("FwProgrammSPI ERROR: SPI write at 0x%x\n",
						   SPIOff + i ));
			return SK_FALSE;
		}
	}

	for( i = 0; i < Size; i += 4 )  {

		FwOsFileRead(pAC, fh, (SK_U8 *) &Ver32, ImageOff + i, 4);

		if( spi_flash_manage( pAC, IoC, (SK_U8 *) &Val32, SPIOff + i, 4, SPI_READ )) {

			FW_DBG_MSG_IE(("FwProgrammSPI ERROR: SPI read at 0x%x\n",
							   SPIOff + i ));
			return SK_FALSE;
		}

		if (Val32 != Ver32) {
			FW_DBG_MSG_IE(("FwProgrammSPI ERROR: SPI verify at 0x%x\n",
						   SPIOff + i ));
			return SK_FALSE;
		}
	}

	FW_DBG_MSG_I(("FwProgrammSPI: ok\n"));

	return (SK_TRUE);
}

SK_BOOL FwProgrammPFlash(
	SK_AC        *pAC,      /* Pointer to adapter context */
	SK_IOC       IoC,       /* IO context handle */
	SK_FW_HANDLE fh,        /* image file handler */
	SK_U32       ImageOff,
	SK_U32       PFlashOff,
	SK_U32       Size)
{
	SK_U32 i, maxcnt;
	SK_U32 cnt = 0;
	SK_U32 Val32;
	SK_U32 buf [16];

	if( SkPflManage( pAC, IoC, NULL, PFlashOff, Size, SK_PFL_ERASE )) {

		FW_DBG_MSG_IE(("FwProgrammPFlash ERROR: erase timeout\n"));
		return (SK_FALSE);
	}

	SK_DBG_PRINTF("%s: Flash device ", DRV_NAME);
	maxcnt = (Size/4)/200;

	for( i = 0; i < Size; i += 4 )  {
		Val32 = FwGetImageWord(pAC, fh, ImageOff + i);
		buf[(i%64)/4] = Val32;
		if (i == 0 || (i+4)%64 ) {
			continue;
		}

		if (cnt++ >= maxcnt) {
			cnt = 0;
			SK_DBG_PRINTF(".");
		}

		if (i == (Size - 4)) {
			buf[(i%64)/4] = ~pAC->FwCommon.ImageChkSum + 1;
		}

		if (SkPflManage(pAC, IoC, (SK_U8*)buf, PFlashOff + i - 60, 64,
						 SK_PFL_WRITE)) {
			FW_DBG_MSG_IE(("FwProgrammPFlash ERROR: write timeout at 0x%x\n",
						    PFlashOff +i ));
			return (SK_FALSE);
		}
	}

	for( i = 0; i < Size; i += 4 )  {
			if( SkPflManage(pAC, IoC, (SK_U8 *) &Val32, PFlashOff + i, 4, SK_PFL_READ)) {

			FW_DBG_MSG_IE(("FwProgrammPFlash ERROR: read timeout at 0x%x\n",
					   PFlashOff + i ));
			return (SK_FALSE);
		}

		if (i == (Size - 4)) {
			if (Val32 + pAC->FwCommon.ImageChkSum) { /* should be 0 */
				FW_DBG_MSG_IE(("FwProgrammPFlash ERROR: CSum verify at 0x%x\n",
							   PFlashOff + i ));
				return (SK_FALSE);
			}
			continue;
		}

		if (Val32 != FwGetImageWord(pAC, fh, ImageOff + i)) {
			FW_DBG_MSG_IE(("FwProgrammPFlash ERROR: verify at 0x%x\n",
					   PFlashOff + i ));
			return (SK_FALSE);
		}
	}

	SK_DBG_PRINTF(" finished!\n");

	FW_DBG_MSG_I(("FwProgrammPFlash: ok\n"));
	return (SK_TRUE);
}

SK_BOOL FwProgrammImage(
	SK_AC        *pAC,  /* Pointer to adapter context */
	SK_IOC       IoC,   /* IO context handle */
	SK_FW_HANDLE fh)    /* image file handler */
{
	FwSmartResetCpu( pAC, IoC, ASF_RESET_COLD );

	if (pAC->FwCommon.ChipID == CHIP_ID_YUKON_SUPR) {
		if ( pAC->FwCommon.ImageSize > pAC->FwApp.FlashSize) {

			if ( !FwProgrammSPI(pAC, IoC, fh,
								pAC->FwApp.FlashSize,
								0,
								pAC->FwCommon.ImageSize -
								pAC->FwApp.FlashSize )) {
			return SK_FALSE;
			}

		}

		return ( FwProgrammPFlash(pAC, IoC, fh, 0, 0,
								  pAC->FwApp.FlashSize ));
	}
	else {
		return ( FwProgrammSPI(pAC, IoC, fh,
							   0,
							   0,
							   pAC->FwApp.FlashSize));
	}
}

SK_BOOL FwIsImageOk(
	SK_AC        *pAC,  /* Pointer to adapter context */
	SK_FW_HANDLE fh)    /* file handler */
{
	SK_U32 ImCs = 0;
	SK_U32 Val32;
	int i;

	for( i = 0; i < pAC->FwApp.FlashSize; i += 4 )  {
		FwOsFileRead(pAC, fh, (SK_U8 *)&Val32, i, 4);
		ImCs += LITTLEENDIAN_TO_HOST_32(Val32);
	}

	if (ImCs) { /* should be 0 */
		FW_DBG_MSG_IE(("FwIsImageOk ERROR: bad CSum\n"));
		return SK_FALSE;
	}
	FwReadFwVersionFromFile(pAC, fh);

	if (!FwAppIsVersionOk(pAC)) {
		FW_DBG_MSG_IE(("FwIsImageOk ERROR: wrong image version\n"));
		return SK_FALSE;
	}

	FW_DBG_MSG_I(("FwIsImageOk: ok\n"));
	return SK_TRUE;
}

SK_U32	FwGetSpiCheckSum(
	SK_AC *pAC,  /* Pointer to adapter context */
	SK_IOC IoC)  /* IO context handle */
{
	SK_U32 Val32;

	/* read SPI checksum from PFlash */
	if (SkPflManage(pAC, IoC, (SK_U8 *) &Val32,
					pAC->FwApp.FlashSize - 8, 4, SK_PFL_READ) ) {

		FW_DBG_MSG_IE(("FwGetSpiCheckSum ERROR: PFlash read timeout\n"));
	}
	FW_DBG_MSG_I(("FwGetSpiCheckSum: 0x%x\n", Val32));
	return (Val32);
}

SK_U32	FwGetSpiCodeSize(
	SK_AC *pAC,  /* Pointer to adapter context */
	SK_IOC IoC)  /* IO context handle */
{
	SK_U32 Val32;

	/* read SPI code size from PFlash */
	if (SkPflManage(pAC, IoC, (SK_U8 *) &Val32,
					pAC->FwApp.FlashSize - 12, 4, SK_PFL_READ) ) {

		FW_DBG_MSG_IE(("FwGetSpiCodeSize ERROR: PFlash read timeout\n"));
	}
	FW_DBG_MSG_I(("FwGetSpiCodeSize: 0x%x\n", Val32));
	return (Val32);
}

SK_BOOL FwIsSpiOk(
	SK_AC *pAC,  /* Pointer to adapter context */
	SK_IOC IoC)  /* IO context handle */
{
	SK_U32	i, Val32;
	SK_U32	SpiCs = FwGetSpiCheckSum(pAC, IoC);
	SK_U32  SPICodeSize = FwGetSpiCodeSize(pAC, IoC);

	if (SPICodeSize ) { /*  if there code in spi flash */

		FwLockSpi( pAC, IoC );

		for (i = 0; i < SPICodeSize/4; i++ ) {

			if (spi_flash_manage( pAC, IoC, (SK_U8 *) &Val32, i*4, 4, SPI_READ) ) {

				FW_DBG_MSG_IE(("FwIsSpiOk ERROR: SPI read timeout at 0x%0x\n", i*4));
				FwUnlockSpi( pAC, IoC );
				return (SK_FALSE);
			}
			SpiCs = SpiCs + Val32;
		}

		if (SPICodeSize%4) {
			/* read last incomplete word */
			if (spi_flash_manage( pAC, IoC, (SK_U8 *) &Val32, i*4, 4, SPI_READ) ) {

				FW_DBG_MSG_IE(("FwIsSpiOk ERROR: SPI read timeout at 0x%0x\n", i*4));
				FwUnlockSpi( pAC, IoC );
				return (SK_FALSE);
			}
			SpiCs = SpiCs + (Val32 & (0x00ffffff >> ((3 - SPICodeSize%4) * 8)));
		}

		FwUnlockSpi( pAC, IoC );

		if (SpiCs) { /* Should be 0 if ok */
			FW_DBG_MSG_IE(("FwIsSpiOk ERROR: SPI CSum 0x%x is not ok\n", SpiCs));
			return (SK_FALSE);
		}

		FW_DBG_MSG_I(("FwIsSpiOk: SPI CSum ok\n"));
		return (SK_TRUE);
	}
	else {
		return (SK_FALSE);
	}
}

SK_BOOL FwIsFwInFlashOk(
	SK_AC *pAC,  /* Pointer to adapter context */
	SK_IOC IoC)  /* IO context handle */
{
	SK_U32 Val32;

	/* Calculate Checksum in Flash */
	SK_U32 FlashChkSum = FwChkSumFlash(pAC, IoC);

	/* read CSum from Flash */
	if (SkPflManage(pAC, IoC, (SK_U8 *) &Val32,
					pAC->FwApp.FlashSize - 4, 4, SK_PFL_READ) ) {

		FW_DBG_MSG_IE(("FwIsFlashImageOk ERROR: PFlash CSum read timeout\n"));
	}

	FwReadFwVersionFromFlash(pAC, IoC);

	if ( (Val32 + FlashChkSum) /* CSum is not ok, Fw in Flash damaged or absent*/
		|| FlashChkSum != pAC->FwCommon.ImageChkSum /* Fw needs new patches*/
		|| FwIsImageNewer(pAC) ) {	/* image newer than fw in flash */

		FW_DBG_MSG_I(("FwIsFlashImageOk: Flash needs update\n"));

		return (SK_FALSE);
	}

	if (pAC->FwCommon.ImageSize > pAC->FwApp.FlashSize &&
		!FwIsSpiOk(pAC, IoC)) {

		FW_DBG_MSG_I(("FwIsFlashImageOk: Image in SPI needs update\n"));
		return (SK_FALSE);
	}

	FW_DBG_MSG_I(("FwIsFlashImageOk: Image in Flash is up to date\n"));

	return (SK_TRUE);
}

#define SPI_ASF_FW_ADDR			0x20000
#define SPI_ASF_FW_SIZE			0x20000
#define PXE_MAGIC_NUM_LEN		2
#define PXE_MAGIC_NUM			0xaa55
#define SPI_ASF_CONFIG_COPY		0x1FF80
#define SPI_ASF_CONFIG2			0x1CF80

int FwRemoveFirmware(
	SK_AC  *pAC,
	SK_IOC IoC)
{
	int i, ret;
	SK_U32 flash_size = 0;
	SK_U32 our_reg2;
	SK_U8	TmpVal8;
	SK_U32  TmpVal32, TmpVpd;
	SK_U32 offset = SPI_ASF_FW_ADDR;
	unsigned char cv;
	unsigned char cdb[0x90];
	unsigned char cd12[] = {
		255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,
		85,1,104,14,0,0,0,0,
		85,1,104,14,2,0,0,0,
		85,12,88,28,0,0,0,2,
		255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,};

	unsigned char cd13[] = {
		85,12,88,28,0,0,0,1,
		255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,
		85,1,104,14,0,0,0,0,
		85,1,104,14,2,0,0,0,
		85,1,16,15,1,0,0,0,
		85,15,128,28,20,52,7,0,
		85,12,88,28,0,0,0,2,
		255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,
		255,255,255,255,255,255,255,255,};

	if( pAC->FwApp.ChipMode == SK_GEASF_CHIP_SU )  {
		/*  reset CPU Watchdog */
		SK_OUT32( IoC, CPU_WDOG, 0);
		/* Stop Asf timer to avoid restart CPU (dr) */
		SkTimerStop(pAC, IoC, &pAC->FwApp.AsfTimer);
		FwDisablePattern( pAC, IoC, 0, ASF_DASH_PATTERN_NUM_ARP );
		FwDisablePattern( pAC, IoC, 0, ASF_DASH_PATTERN_NUM_ICMP );
		FwDisablePattern( pAC, IoC, 0, ASF_DASH_PATTERN_NUM_TCP );
		FwDisablePattern( pAC, IoC, 0, ASF_DASH_PATTERN_NUM_UDP );
	}
	else  {
		/* Stop Asf timer to avoid restart CPU (dr) */
		SkTimerStop(pAC, IoC, &pAC->FwApp.AsfTimer);
		/* disable all pattern for asf/ipmi */
		FwDisablePattern(pAC, IoC, 0, ASF_PATTERN_ID_RSP);
		FwDisablePattern(pAC, IoC, 0, ASF_PATTERN_ID_ARP);
		FwDisablePattern(pAC, IoC, 0, ASF_PATTERN_ID_RMCP);
	}

	pAC->FwApp.InitState  = ASF_INIT_UNDEFINED;
	pAC->FwApp.OpMode     = SK_GEASF_MODE_UNKNOWN;

	/* Stop CPU */
	FwLockSpi( pAC, IoC );

	FwResetCpu( pAC, IoC );

	SK_OUT32( IoC, SMB_CSR, SMB_CSR_TX_CLR | SMB_CSR_TX_STOP); /*  clear SmBus  */

	SK_OUT8(IoC, SMB_CFG, SMB_CFG_RST_SET); /*  reset SmBus  */

	/* disable ASF logic (dr) */
	SK_IN32(IoC, B0_CTST, &TmpVal32);
	TmpVal32 &= ~0x00003000;    /* clear bit13, bit12 */
	TmpVal32 |= 0x00001000;             /* set bit13 */
	SK_OUT32(IoC, B0_CTST, TmpVal32);

	pAC->GIni.GIAsfEnabled = SK_FALSE;   /*  update asf flag for common modules  */
	pAC->GIni.GIAsfRunning = SK_FALSE;   /*  update asf flag for common modules  */

	if( pAC->FwApp.ChipMode == SK_GEASF_CHIP_SU )  {

		/*  set ASF FIFO in Reset */
		SK_IN32( IoC, ASF_CTRL, &TmpVal32 );
		TmpVal32 &= ~(BIT_0 | BIT_1);
		TmpVal32 |= BIT_0;
		SK_OUT32( IoC, ASF_CTRL, TmpVal32 );

		/*   erase pflash */
		if( SkPflCheck( pAC, IoC) )  {
			if( SkPflManage( 	pAC, IoC,
								pAC->FwApp.FlashBuffer,
								pAC->FwApp.FlashOffs,
								pAC->FwApp.FlashSize, SK_PFL_ERASE) == 0 )  {
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT, ("pflash successfully erased !\n"));
			}
			else  {
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT, ("Error: pflash erase failed\n"));
			}
		}

		/*  erase SPI - flash */
		if (spi_flash_erase( pAC, IoC, 0, pAC->FwApp.SPIFlashSize) != 0) {
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT, ("Error: flash erase failed (0x%x)\n",pAC->FwApp.SPIFlashSize));
		}
		else  {
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT, ("spi flash successfully erased (0x%x)\n",pAC->FwApp.SPIFlashSize));
		}

		/*  erase vpd */
		for( i=0; i<ASF_VPD_MAX_CONFIG_SIZE; i++ )
			pAC->FwApp.VpdConfigBuf[i] = 0;
		SK_TST_MODE_ON(IoC);
		/*  select EEPROM */
		SK_IN32(IoC, PCI_C(pAC, VPD_CTRL_ADD), &TmpVal32);
		TmpVpd = TmpVal32;
		TmpVal32 = 0xff000007;
		SK_OUT32(IoC, PCI_C(pAC, VPD_CTRL_ADD), TmpVal32);

		/*  read vpd size
		 * VPD_IN32(pAC, IoC, PCI_OUR_REG_2, &TmpVal32);
		 * pAC->vpd.rom_size = 256 << ((TmpVal32 & PCI_VPD_ROM_SZ) >> 14);
		 */

		if (pAC->FwApp.VpdInitOk == 0) {
			VpdInit( pAC, IoC );
		}
		ret = VpdWriteBlock( pAC, IoC, &pAC->FwApp.VpdConfigBuf[0], 0x0, ASF_VPD_MAX_CONFIG_SIZE);
		if (ret == ASF_VPD_MAX_CONFIG_SIZE) {
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT, ("VPD 0 - 0x%x successfully erased\n",ASF_VPD_MAX_CONFIG_SIZE));
		}
		else  {
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT, ("Error: VPD 0 - 0x%x erase failed\n",ASF_VPD_MAX_CONFIG_SIZE));
		}

		/*  restore VPD ctrl */
		SK_TST_MODE_ON(IoC);  /*  will be switched off in VpdWriteBlock */
		SK_OUT32(IoC, PCI_C(pAC, VPD_CTRL_ADD), TmpVpd );
		SK_TST_MODE_OFF(IoC);
	}
	else  {  /*  != SK_GEASF_CHIP_SU */

#ifdef CHIP_ID_YUKON_EX
		if( pAC->FwApp.ChipMode == SK_GEASF_CHIP_EX ) {
			SK_IN32( IoC, ASF_CTRL, &TmpVal32 );
			TmpVal32 &= ~(BIT_0 | BIT_1);
			TmpVal32 |= BIT_0;  /*  set ASF FIFO in Reset */
			SK_OUT32( IoC, ASF_CTRL, TmpVal32 );
		}
#endif
		flash_check_spi( pAC, IoC, (unsigned long *) &flash_size);

		if (flash_size <= SPI_ASF_FW_ADDR) {
			offset = 0;
		}

		if (pAC->FwApp.FlashSize == ASF_FLASH_NOVPD_SIZE) {
			flash_size =  ASF_FLASH_NOVPD_SIZE + 8192;
		}
		else {
			flash_size =  ASF_FLASH_SIZE;
		}

		if (spi_flash_erase( pAC, IoC, offset, flash_size) != 0) {
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT, ("Error: flash erase failed\n"));
		}
		else  {
			SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT, ("spi flash successfully erased\n"));
		}

		SK_IN8( IoC, B2_CHIP_ID, &cv); /* read Chip ID */

		SK_IN8( IoC, Y2_CFG_SPC + PCI_PM_NITEM, &TmpVal8);

		if (TmpVal8 != PCI_VPD_CAP_ID ) { /* Parameters in SPI */
			if (spi_flash_manage(pAC, IoC, cdb, SPI_ASF_CONFIG_COPY, 0x80, SPI_READ) == 0) {
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,
						   ("Error: SPI flash not read\n"));

				spi_flash_manage(pAC, IoC, cdb, offset + SPI_ASF_CONFIG2, 0x80, SPI_WRITE);
			}
		}
		else  { /* if Yukon Extreme */

			for (i=0; i < 0x88; i++) {
				cdb[i] = 0xff;
			}

			SK_OUT8(IoC, B0_CTST, CS_RST_CLR); /* clear software reset for sure eeprom write */

			VPD_IN32(pAC, IoC, PCI_OUR_REG_2, &our_reg2);
			pAC->vpd.rom_size = 256 << ((our_reg2 & PCI_VPD_ROM_SZ) >> 14);

			/* enable Config write Reg 0x158 */
			SK_IN8( IoC, B2_TST_REG1, &TmpVal8 );
			TmpVal8 &= ~0x03;
			TmpVal8 |= 0x02;
			SK_OUT8( IoC, B2_TST_REG1, TmpVal8 );

			/* Restore ASF area  */
			ret = VpdWriteBlock(pAC,IoC, (char *)cdb, cv == CHIP_ID_YUKON_EX ? 0x160 : 0x340, 0x88);

			if (ret != 0x88) {
				SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,
						("Error: Restory ASF area failed\n"));

			}

			if (cv == CHIP_ID_YUKON_EX) {
				ret = VpdReadBlock(pAC, IoC, (char *)cdb, 0x300, 0x80);

				if (ret != 0x80) {
					SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,
							("Error: Read Config data failed\n"));

				}
				if (cdb[0] == 0x55) { /* is this stored data? */
					ret = VpdWriteBlock(pAC,IoC, (char *)cdb, 0x380, 0x80);
					if (ret != 0x80) {
						SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,
								("Error: Restory Config data failed\n"));

					}
				}
				else {
					SK_IN8( IoC, 0x1c08, &cv); /* read Config Data Version */

					switch (cv) {
					case 0x12:
						ret = VpdWriteBlock(pAC, IoC, (char *)cd12, 0x380, 0x80);
						if (ret != 0x80) {
							SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,
									("Error: Restory Config data failed\n"));
						}
						break;
					case 0x13:
						ret = VpdWriteBlock(pAC, IoC, (char *)cd13, 0x380, 0x80);
						if (ret != 0x80) {
							SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,
									("Error: Restory Config data failed\n"));
						}
						break;
					default:
						SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,
							("Error: Uknown Config Data Version, not restored\n"));
						break;
					}
				}
				/* disable Config write Reg 0x158 */
				SK_IN8( IoC, B2_TST_REG1, &TmpVal8 );
				TmpVal8 &= ~0x03;
				TmpVal8 |= 0x01;
				SK_OUT8( IoC, B2_TST_REG1, TmpVal8 );
			}
		}
	}

	FwUnlockSpi( pAC, IoC );

	return(0);
}

/******************************************************************************
 *
 * FwCheckSPI() - Check if SPI is present
 *
 * Description:
 *
 * Returns:
 *  0:         SPI not found
 *  2:         PXE is present
 *  1:         PXE is not present
 *  otherwise: error code (take a look at asfinstdll.h)
 */
SK_U8  FwCheckSPI(
	SK_AC  *pAC,
	SK_IOC IoC)
{
	SK_U32 flash_size = 0;
	SK_U8 buffer[PXE_MAGIC_NUM_LEN];
	SK_U16 *p_pxe_magic_number;
	int ret;

	FwLockSpi( pAC, IoC );

	ret = flash_check_spi( pAC, IoC, (unsigned long *) &flash_size);
	pAC->FwApp.SPIFlashSize = flash_size;

	if (ret == 0) {
		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,
				("Error: SPI flash not found\n"));
		FwUnlockSpi( pAC, IoC );
		return(0);/* ASFINST_ERROR_NO_SPI_FLASH); */
	}

	/*
	 * Read the necessary number of bytes from the SPI Flash
	 * to check if PXE is present.
	 */
	if (spi_flash_manage(pAC, IoC, buffer, SPI_ASF_FW_ADDR, PXE_MAGIC_NUM_LEN, SPI_READ) != 0) {
		SK_DBG_MSG(pAC, SK_DBGMOD_ASF, SK_DBGCAT_INIT,
				("Error: SPI flash not read\n"));
		FwUnlockSpi( pAC, IoC );
		return(0);/* ASFINST_ERROR_READ_FLASH); */
	}

	/*
	 * Check if the read bytes are equal to
	 * the PXE magic number
	 */
	p_pxe_magic_number = (SK_U16*)buffer;

	if ((*p_pxe_magic_number) == PXE_MAGIC_NUM) {
		FwUnlockSpi( pAC, IoC );
		return(2);/* ASFINST_PXE_PRESENT); */
	}
	FwUnlockSpi( pAC, IoC );
	return(1);/* ASFINST_PXE_NOT_PRESENT); */
}

