/******************************************************************************
 *
 * Name:	$Id: //Release/Yukon_1G/Shared/vpd/V4/skvpd.c#4 $
 * Project:	Gigabit Ethernet Adapters, VPD-Module
 * Version:	$Revision: #4 $, $Change: 4281 $
 * Date:	$DateTime: 2010/11/05 11:58:07 $
 * Purpose:	Shared software to read and write VPD
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

/* Please refer skvpd.txt for information how to include this module. */

#include "h/skdrv1st.h"
#include "h/sktypes.h"
#include "h/skdebug.h"
#include "h/skdrv2nd.h"

/*
 * Static functions
 */
#ifndef SK_KR_PROTO
static SK_VPD_PARA	*vpd_find_para(
	SK_AC	*pAC,
	const char	*key,
	SK_VPD_PARA *p);
#else	/* SK_KR_PROTO */
static SK_VPD_PARA	*vpd_find_para();
#endif	/* SK_KR_PROTO */

/******************************************************************************
 *
 *	VpdWait()	- waits for a VPD read/write completion
 *
 * Description:
 *	Waits for a completion of a VPD read/write transfer
 *	The VPD transfer must complete within SK_TICKS_PER_SEC/16
 *
 * returns	0:	success, transfer complete
 *			1:	error, timeout
 */
static int VpdWait(
SK_AC	*pAC,	/* Adapters Context */
SK_IOC	IoC,	/* IO Context */
int		event)	/* event to wait for VPD_READ / VPD_WRITE completion */
{
	SK_U64	start_time;
	SK_U32	delta_time;
	SK_U16	state;
#ifdef xSK_DIAG
	SK_U32	StartTime;
	SK_U32	EndTime;
#endif /* SK_DIAG */

#ifdef XXX		/* Creates too much debug output */
	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_CTRL,
		("VPD wait for %s\n", event ? "Write" : "Read"));
#endif /* XXX */

	start_time = SkOsGetTime(pAC);

#ifdef xSK_DIAG
	SK_IN32(IoC, GMAC_TI_ST_VAL, &StartTime);
#endif /* SK_DIAG */

	do {
		delta_time = (SK_U32)(SkOsGetTime(pAC) - start_time);

		VPD_IN16(pAC, IoC, PCI_VPD_ADR_REG, &state);

		/* Error on reading VPD Address Register */
		if (state == 0xffff) {
			SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR | SK_DBGCAT_FATAL,
				("ERROR: VPD %s status\n", event ? "Write" : "Read"));
			return(1);
		}

		if (delta_time > SK_TICKS_PER_SEC / 16 &&
			(int)(state & PCI_VPD_FLAG) == event) {

			/* Bug fix AF: Thu Mar 28 2002
			 * Do not call: VPD_STOP(pAC, IoC);
			 * A pending VPD read cycle can not be aborted by writing VPD_WRITE
			 * to the PCI_VPD_ADR_REG (VPD address register).
			 * Although the write threshold in the OUR-register protects VPD RO
			 * space from being overwritten, this does not protect a VPD read
			 * from being `converted` into a VPD write operation (on the fly).
			 * As a consequence the VPD_STOP would delete VPD read only data.
			 * In case of any problems with the I2C bus we exit the loop here.
			 * The I2C read operation can not be aborted except by a reset.
			 */
			SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR | SK_DBGCAT_FATAL,
				("ERROR: VPD %s timeout\n", event ? "Write" : "Read"));
			return(1);
		}

	} while ((int)(state & PCI_VPD_FLAG) == event);

#ifdef xSK_DIAG
	SK_IN32(IoC, GMAC_TI_ST_VAL, &EndTime);

	delta_time = HW_TICKS_TO_USEC(pAC, EndTime - StartTime);
#endif /* SK_DIAG */

#ifdef xSK_DIAG
	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_CTRL,
		("VPD state = 0x%04X, delta = %lu\n", state, delta_time));
#endif /* SK_DIAG */

	return(0);
}

#if (!defined(SK_SLIM) || defined(EFI_UNDI)) /* needs to be enabled for ASF w/o EEPROM */
/******************************************************************************
 *
 *	SpiWait()	- waits for a SPI read completion
 *
 * Description:
 *	Waits for a completion of a SPI read transfer
 *	The SPI transfer must complete within SK_TICKS_PER_SEC/16
 *
 * returns	0:	success, transfer completes
 *			1:	error, timeout
 */
static int SpiWait(
SK_AC	*pAC,	/* Adapters Context */
SK_IOC	IoC)	/* IO Context */
{
	SK_I64	start_time;
	SK_U32	delta_time;
	SK_U32	stat;

#ifdef XXX		/* Creates too much debug output */
	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_CTRL, ("SPI wait\n"));
#endif /* XXX */

	start_time = (SK_I64)SkOsGetTime(pAC);

	do {
		delta_time = (SK_U32)(SkOsGetTime(pAC) - start_time);

		if (delta_time > SK_TICKS_PER_SEC / 16) {
			SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR | SK_DBGCAT_FATAL,
				("ERROR: SPI wait timeout\n"));
			return(1);
		}

		SK_IN32(IoC, SPI_CTRL, &stat);

	} while ((stat & SPI_CTRL_BSY) != 0);

	return(0);
}
#endif /* !SK_SLIM || EFI_UNDI */

#ifdef SK_DIAG
/*
 * Read the dword at address 'addr' from the VPD EEPROM.
 *
 * Needed Time:	MIN 1,3 ms	MAX 2,6 ms
 *
 * Note: The dword is returned in the endianess of the machine
 *
 * Returns the data read.
 */
SK_U32 VpdReadDWord(
SK_AC	*pAC,	/* Adapters Context */
SK_IOC	IoC,	/* IO Context */
int		Addr)	/* VPD address */
{
	SK_U32	DWord;

	/* start VPD read */
	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_RX,
		("VPD read dword at 0x%x: ", Addr));

	Addr &= ~VPD_WRITE;	/* ensure the R/W bit is set to read */

	VPD_OUT16(pAC, IoC, PCI_VPD_ADR_REG, (SK_U16)Addr);

	/* ignore return code here */
	(void)VpdWait(pAC, IoC, VPD_READ);

	/* get the VPD from data register */
	VPD_IN32(pAC, IoC, PCI_VPD_DAT_REG, &DWord);

	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_RX,
		("data = 0x%x\n", DWord));

	return(DWord);
}
#endif /* SK_DIAG */


#ifdef XXX
/*
	Write the dword 'data' at address 'addr' into the VPD EEPROM, and
	verify that the data is written.

 Needed Time:

.						MIN		MAX
. -------------------------------------------------------------------
. write					1.8 ms	 3.6 ms
. internal write cyles	0.7 ms	 7.0 ms
. -------------------------------------------------------------------
. over all program time	2.5 ms	10.6 ms
. read					1.3 ms	 2.6 ms
. -------------------------------------------------------------------
. over all				3.8 ms	13.2 ms
.
 Returns	0:	success
			1:	error,	I2C transfer does not terminate
			2:	error,	data verify error

 */
static int VpdWriteDWord(
SK_AC	*pAC,	/* Adapters Context */
SK_IOC	IoC,	/* IO Context */
int		Addr,	/* VPD address */
SK_U32	Data)	/* VPD to write */
{
	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_CTRL,
		("VPD write dword at addr 0x%x, data = 0x%x\n", Addr, Data));

	/* write the VPD into data register */
	VPD_OUT32(pAC, IoC, PCI_VPD_DAT_REG, Data);

	/* start VPD write */
	VPD_OUT16(pAC, IoC, PCI_VPD_ADR_REG, (SK_U16)(VPD_WRITE | Addr));

	/* this may take up to 10.6 ms */
	if (VpdWait(pAC, IoC, VPD_WRITE)) {
		return(1);
	}

	/* verify data */
	if (VpdReadDWord(pAC, IoC, Addr) != Data) {
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR | SK_DBGCAT_FATAL,
			("Data Verify Error\n"));
		return(2);
	}
	return(0);
}	/* VpdWriteDWord */

#endif /* XXX */


#ifndef SK_SLIM
/*
 *	Read one Stream of 'len' bytes of VPD, starting at 'addr' from
 *	or to the I2C EEPROM.
 *
 * Returns number of bytes read / written.
 */
static int VpdWriteStream(
SK_AC	*pAC,	/* Adapters Context */
SK_IOC	IoC,	/* IO Context */
char	*buf,	/* data buffer */
int		Addr,	/* VPD start address */
int		Len)	/* number of bytes to read / to write */
{
	int		i;
	int		Rtv;
	SK_U8	Byte;
	SK_U32	*pComp;	/* compare pointer */
	SK_U32	DWord;	/* read data for compare */
#if defined(SK_DIAG) || defined(DEBUG)
	SK_U32	StartTime;
	SK_U32	EndTime;
	SK_U32	DeltaWr;
	SK_U32	DeltaRd;
#endif /* SK_DIAG || DEBUG */

	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_CTRL, ("WriteStream:\n"));

	Addr &= ~VPD_WRITE;	/* ensure the R/W bit is set to read */

	/* Init Compare Pointer */
	pComp = (SK_U32 *)buf;

	for (i = 0; i < Len; i++) {
		if ((i % SZ_LONG) == 0) {
			/*
			 * At the begin of each cycle read the Data Reg
			 * So it is initialized even if only a few bytes are written.
			 */
			VPD_OUT16(pAC, IoC, PCI_VPD_ADR_REG, (SK_U16)Addr);

			/* Wait for termination */
			Rtv = VpdWait(pAC, IoC, VPD_READ);
			if (Rtv != 0) {
				return(i);
			}
		}

		Byte = *(buf + i);

		/* Write current byte */
		VPD_OUT8(pAC, IoC, PCI_VPD_DAT_REG + (i % SZ_LONG), Byte);

		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_CTRL,
			("0x%02X ", Byte));

		if (((i % SZ_LONG) == 3) || (i == (Len - 1))) {
			/* New address needs to be written to VPD_ADDR reg */
			VPD_OUT16(pAC, IoC, PCI_VPD_ADR_REG, (SK_U16)(VPD_WRITE | Addr));

#if defined(SK_DIAG) || defined(DEBUG)
			SK_IN32(IoC, GMAC_TI_ST_VAL, &StartTime);
#endif /* SK_DIAG || DEBUG */

			/* Wait for termination */
			Rtv = VpdWait(pAC, IoC, VPD_WRITE);
			if (Rtv != 0) {
				return(i - (i % SZ_LONG));
			}

#if defined(SK_DIAG) || defined(DEBUG)
			SK_IN32(IoC, GMAC_TI_ST_VAL, &EndTime);

			DeltaWr = HW_TICKS_TO_USEC(pAC, EndTime - StartTime);
#endif /* SK_DIAG || DEBUG */

			/* Now re-read VPD to verify */
			VPD_OUT16(pAC, IoC, PCI_VPD_ADR_REG, (SK_U16)Addr);

#if defined(SK_DIAG) || defined(DEBUG)
			SK_IN32(IoC, GMAC_TI_ST_VAL, &StartTime);
#endif /* SK_DIAG || DEBUG */

			/* Wait for termination */
			Rtv = VpdWait(pAC, IoC, VPD_READ);
			if (Rtv != 0) {
				return(i - (i % SZ_LONG));
			}

#if defined(SK_DIAG) || defined(DEBUG)
			SK_IN32(IoC, GMAC_TI_ST_VAL, &EndTime);

			DeltaRd = HW_TICKS_TO_USEC(pAC, EndTime - StartTime);
#endif /* SK_DIAG || DEBUG */

			/* get the VPD from data register */
			VPD_IN32(pAC, IoC, PCI_VPD_DAT_REG, &DWord);

			SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_CTRL,
				("==> VPD read: 0x%08X, Wr=%u us, Rd=%u us\n",
				DWord, DeltaWr, DeltaRd));

			if (DWord != *pComp) {
				/* Verify Error */
				SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR,
					("WriteStream Verify Error at address 0x%02X "
					 "(is 0x%08lX, exp. 0x%08lX)\n", Addr, DWord, *pComp));

				/* Retry reading */
				VPD_OUT16(pAC, IoC, PCI_VPD_ADR_REG, (SK_U16)Addr);

#if defined(SK_DIAG) || defined(DEBUG)
				SK_IN32(IoC, GMAC_TI_ST_VAL, &StartTime);
#endif /* SK_DIAG || DEBUG */

				/* Wait for termination */
				Rtv = VpdWait(pAC, IoC, VPD_READ);
				if (Rtv != 0) {
					return(i - (i % SZ_LONG));
				}

#if defined(SK_DIAG) || defined(DEBUG)
				SK_IN32(IoC, GMAC_TI_ST_VAL, &EndTime);

				DeltaRd = HW_TICKS_TO_USEC(pAC, EndTime - StartTime);
#endif /* SK_DIAG || DEBUG */

				/* get the VPD from data register */
				VPD_IN32(pAC, IoC, PCI_VPD_DAT_REG, &DWord);

				SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_CTRL,
					("Re-read VPD: 0x%08X, Rd=%u us\n", DWord, DeltaRd));

				if (DWord != *pComp) {
					/* Verify Error */
					SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR,
						("Verify Error, is 0x%08lX, exp. 0x%08lX)\n",
						 DWord, *pComp));

					return(i - (i % SZ_LONG));
				}
			}

			Addr += SZ_LONG;

			pComp++;
		}
	}

	return(Len);
}
#endif /* !SK_SLIM */

/*
 *	Read one Stream of 'len' bytes of VPD, starting at 'addr' from
 *	or to the I2C EEPROM.
 *
 * Returns number of bytes read / written.
 */
static int VpdReadStream(
SK_AC	*pAC,	/* Adapters Context */
SK_IOC	IoC,	/* IO Context */
char	*buf,	/* data buffer */
int		Addr,	/* VPD start address */
int		Len)	/* number of bytes to read / to write */
{
	int		i;
	int		Rtv;
	SK_U32	DWord;

	Addr &= ~VPD_WRITE;	/* ensure the R/W bit is set to read */

	for (i = 0; i < Len; i++) {
		if ((i % SZ_LONG) == 0) {
			/* New Address needs to be written to VPD_ADDR reg */
			VPD_OUT16(pAC, IoC, PCI_VPD_ADR_REG, (SK_U16)Addr);

			/* Wait for termination */
			Rtv = VpdWait(pAC, IoC, VPD_READ);
			if (Rtv != 0) {
				return(i);
			}

			Addr += SZ_LONG;

			/* get the VPD from data register */
			VPD_IN32(pAC, IoC, PCI_VPD_DAT_REG, &DWord);
		}

#ifdef SK_LITTLE_ENDIAN
		*(buf + i) = *((char *)&DWord + (i % SZ_LONG));
#else
		*(buf + i) = *((char *)&DWord + (3 - (i % SZ_LONG)));
#endif
	}

	return(Len);
}


/*
 *	Read ore writes 'len' bytes of VPD, starting at 'addr' from
 *	or to the I2C EEPROM.
 *
 * Returns number of bytes read / written.
 */
static int VpdTransferBlock(
SK_AC	*pAC,	/* Adapters Context */
SK_IOC	IoC,	/* IO Context */
char	*buf,	/* data buffer */
int		addr,	/* VPD start address */
int		len,	/* number of bytes to read / to write */
int		dir)	/* transfer direction (VPD_READ or VPD_WRITE) */
{
	int		Rtv = 0;
	int		vpd_rom_size;

	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_CTRL,
		("VPD %s block, addr = 0x%x, len = %d\n",
		dir ? "write" : "read", addr, len));

	if (len == 0) {
		return(0);
	}

	vpd_rom_size = pAC->vpd.rom_size;

	if (addr > vpd_rom_size - 4) {
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR | SK_DBGCAT_FATAL,
			("Address error: 0x%x, exp. < 0x%x\n",
			addr, vpd_rom_size - 4));
		return(0);
	}

	if (addr + len > vpd_rom_size) {
		len = vpd_rom_size - addr;
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR,
			("Warning: len was cut to %d\n", len));
	}

	/* WA for dev. #5.18 */
	if (pAC->GIni.GIChipId >= CHIP_ID_YUKON_FE_P) {
		SK_TST_MODE_ON(IoC);
	}

	if (dir == VPD_READ) {
		Rtv = VpdReadStream(pAC, IoC, buf, addr, len);
	}
#ifndef SK_SLIM
	else {
		Rtv = VpdWriteStream(pAC, IoC, buf, addr, len);
	}
#endif /* !SK_SLIM */

	SK_TST_MODE_OFF(IoC);

	return(Rtv);
}

#if (!defined(SK_SLIM) || defined(EFI_UNDI)) /* needs to be enabled for ASF w/o EEPROM */
/******************************************************************************
 *
 *	SpiReadBlock()	-	Read VPD from SPI flash
 *
 * Description:
 *	Read 'len' bytes of VPD, starting at 'addr' from the SPI flash.
 *	If the device doesn't have an EEPROM, the VPD is stored in the
 *	SPI flash and is read only.
 *
 * Returns:
 *	number of read bytes
 */
static int SpiReadBlock(
SK_AC	*pAC,	/* Adapters Context */
SK_IOC	IoC,	/* IO Context */
char	*buf,	/* data buffer */
int		addr,	/* VPD start address */
int		len)	/* number of bytes to read / to write */
{
	int		vpd_rom_size;
	int		i;
	union {
		SK_U32	RegVal;
		SK_U8	Byte[4];
	} uBuf;
	SK_U32	SpiCtrlReg;

	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_CTRL,
		("VPD SPI read block, addr = 0x%x, len = %d\n", addr, len));

	if (len == 0) {
		return(0);
	}

	vpd_rom_size = pAC->vpd.rom_size;

	if (addr > vpd_rom_size - 4) {
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR | SK_DBGCAT_FATAL,
			("Address error: 0x%x, exp. < 0x%x\n",
			addr, vpd_rom_size - 4));
		return(0);
	}

	if (addr + len > vpd_rom_size) {
		len = vpd_rom_size - addr;
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR,
			("Warning: len was cut to %d\n", len));
	}

	addr += VPD_SPI_ADDR;

	for (i = 0; i < len; i++, buf++) {
		if ((i % SZ_LONG) == 0) {
			/* write SPI address */
			SK_OUT32(IoC, SPI_ADDR, addr);

			/* execute SPI read command */
			SK_IN32(IoC, SPI_CTRL, &SpiCtrlReg);

			SpiCtrlReg &= ~SPI_CTRL_INST_MSK;
			SpiCtrlReg |= (1L << SPI_CTRL_INST_BASE);
			SpiCtrlReg |= SPI_CTRL_STRT_SPI;

			SK_OUT32(IoC, SPI_CTRL, SpiCtrlReg);

			/* wait for the SPI to finish RD operation */
			if (SpiWait(pAC, IoC)) {
				break;
			}

			/* read the returned data */
			SK_IN32(IoC, SPI_DATA, &uBuf.RegVal);

			addr += 4;
		}

		*buf = uBuf.Byte[i % SZ_LONG];
	}

	return(i);
}
#endif /* !SK_SLIM || EFI_UNDI */

/******************************************************************************
 *
 *	VpdReadBlock() - Read a block of config data from the EEPROM
 *
 * Description:
 *	Read 'len' bytes of configuration data, starting at 'addr'.
 *	VpdInit() needs to be run prior calling this function.
 *
 * Returns number of bytes read.
 */
int VpdReadBlock(
SK_AC	*pAC,	/* Adapters Context */
SK_IOC	IoC,	/* IO Context */
char	*buf,	/* buffer were the data should be stored */
int		addr,	/* start reading at the VPD address */
int		len)	/* number of bytes to read */
{
	int n;	/* number of read bytes */

#ifndef SK_SLIM		/* needs to be enabled for ASF w/o EEPROM */
	if (pAC->vpd.CfgInSpi) {
		n = SpiReadBlock(pAC, IoC, buf, addr, len);
	}
	else
#endif /* !SK_SLIM */
	{
		n = VpdTransferBlock(pAC, IoC, buf, addr, len, VPD_READ);
	}

	return(n);
}

/******************************************************************************
 *
 *	VpdWriteBlock()	- Write a block of config data to the EEPROM
 *
 * Description:
 *	Writes 'len' bytes of *buf to the VPD EEPROM, starting at 'addr'.
 *	VpdInit() needs to be run prior calling this function.
 *
 * Returns number of bytes written.
 */
int VpdWriteBlock(
SK_AC	*pAC,	/* Adapters Context */
SK_IOC	IoC,	/* IO Context */
char	*buf,	/* buffer, holds the data to write */
int		addr,	/* start writing at the VPD address */
int		len)	/* number of bytes to write */
{
	int n;	/* number of written bytes */

#ifndef SK_SLIM		/* needs to be enabled for ASF w/o EEPROM */
	if (pAC->vpd.CfgInSpi) {
		n = 0;
	}
	else
#endif /* !SK_SLIM */
	{
		n = VpdTransferBlock(pAC, IoC, buf, addr, len, VPD_WRITE);
	}

	return(n);
}

/******************************************************************************
 *
 *	VpdInit() - (re)initialize the VPD buffer
 *
 * Description:
 *	Reads the VPD from the EEPROM into the VPD buffer.
 *	Get the remaining read only and read / write space.
 *
 * Returns:
 *	0:	success
 *	1:	fatal VPD error
 */
int VpdInit(
SK_AC	*pAC,	/* Adapters Context */
SK_IOC	IoC)	/* IO Context */
{
	SK_VPD_PARA *r, rp;	/* RW or RV */
	int		i;
	int		vpd_size;
	int		vpd_ro_size;
	SK_U8	byte;
	SK_U32	our_reg2;
	SK_U32	vpd_ctrl_add_reg;

	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_INIT, ("VpdInit ... "));

	/* Reset VPD status */
	pAC->vpd.v.vpd_status = 0;

	if (pAC->GIni.GILevel < SK_INIT_IO) {
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR, ("Wrong Init Level"));
		return(1);
	}

	if (!HW_SUP_VPD(pAC)) {
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR, ("VPD not supported"));
		return(1);
	}

	VPD_IN8(pAC, IoC, PCI_PM_NITEM, &byte);

#ifndef SK_SLIM		/* needs to be enabled for ASF w/o EEPROM */
	pAC->vpd.CfgInSpi = (byte != PCI_VPD_CAP_ID) && CHIP_ID_YUKON_2(pAC);
#endif /* !SK_SLIM */

	VPD_IN32(pAC, IoC, PCI_OUR_REG_2, &our_reg2);

	pAC->vpd.rom_size = 256 << ((our_reg2 & PCI_VPD_ROM_SZ) >> 14);

	/*
	 * this function might get used before the hardware is initialized
	 * therefore we cannot always trust in GIChipId
	 */

	/* for Yukon the VPD size is always 256 */
	vpd_size = VPD_SIZE_YUKON;

	vpd_ro_size = vpd_size / 2;

	if (pAC->GIni.GIChipId >= CHIP_ID_YUKON_SUPR) {
		/*
		 * Supreme: VPD may be in parallel flash or EEPROM
		 * Ultra 2: VPD only in EEPROM
		 *
		 * Read VPD_CTRL_ADD to get length for VPD RO and RW section.
		 */
		SK_IN32(IoC, PCI_C2(pAC, IoC, VPD_CTRL_ADD), &vpd_ctrl_add_reg);

		if (pAC->GIni.GIChipId == CHIP_ID_YUKON_SUPR ||
			(pAC->GIni.GIChipId >= CHIP_ID_YUKON_UL_2 &&
			 (vpd_ctrl_add_reg & VPD_CTRL_ADD_VPD_SEL) != 0)) {

			vpd_size = (int)((vpd_ctrl_add_reg & VPD_CTRL_ADD_VPD_END_MSK) >> 24) + 1;
			vpd_size -= (int)((vpd_ctrl_add_reg & VPD_CTRL_ADD_VPD_START_MSK) >> 8);
			vpd_size *= 4;

			vpd_ro_size = (int)((vpd_ctrl_add_reg & VPD_CTRL_ADD_VPD_W_START_MSK) >> 16);
			vpd_ro_size -= (int)((vpd_ctrl_add_reg & VPD_CTRL_ADD_VPD_START_MSK) >> 8);
			vpd_ro_size *= 4;
		}
		else {
			SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR, ("VPD not supported"));
			return(1);
		}
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_INIT,
		("VPD Size = %d, VPD RO Size = %d\n", vpd_size, vpd_ro_size));

	if (vpd_ro_size == 0) {
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR,
			("Error: VPD R/O size is 0\n"));
		return(1);
	}

	if (vpd_size > VPD_SIZE) {
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR,
			("Error: Too much VPD (%u)\n", vpd_size));
		return(1);
	}

	/* read the VPD into the VPD buffer */
	if (VpdReadBlock(pAC, IoC, pAC->vpd.vpd_buf, 0, vpd_size) != vpd_size) {

		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR, ("Block Read Error\n"));
		return(1);
	}

	pAC->vpd.vpd_size = vpd_size;

#ifndef SK_SLIM
	/* Asus K8V Se Deluxe bugfix. Correct VPD content */
	i = 62;
	if (!SK_STRNCMP(pAC->vpd.vpd_buf + i, " 8<E", 4)) {

		pAC->vpd.vpd_buf[i + 2] = '8';
	}
#endif /* !SK_SLIM */

	/* find the end tag of the RO area */
	if ((r = vpd_find_para(pAC, VPD_RV, &rp)) == NULL) {
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR | SK_DBGCAT_FATAL,
			("Encoding Error: RV Tag not found\n"));
		return(1);
	}

	if (r->p_val + r->p_len > pAC->vpd.vpd_buf + vpd_ro_size) {
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR | SK_DBGCAT_FATAL,
			("Encoding Error: Invalid VPD struct size\n"));
		return(1);
	}

	pAC->vpd.v.vpd_free_ro = r->p_len - 1;

	/* test the checksum */
	for (i = 0, byte = 0; (unsigned)i <= (unsigned)vpd_ro_size - r->p_len; i++) {
		byte += pAC->vpd.vpd_buf[i];
	}

	if (byte != 0) {
		/* checksum error */
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR | SK_DBGCAT_FATAL,
			("VPD Checksum Error\n"));
		return(1);
	}

#ifndef SK_SLIM		/* needs to be enabled for ASF w/o EEPROM */
	if (pAC->vpd.CfgInSpi) {
		pAC->vpd.v.vpd_free_rw = 0;
		pAC->vpd.v.vpd_status |= VPD_NO_RW;
	}
	else {
#endif /* !SK_SLIM */
		/* find and check the end tag of the RW area */
		if (!(r = vpd_find_para(pAC, VPD_RW, &rp))) {
			SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR | SK_DBGCAT_FATAL,
				("Encoding Error: RW Tag not found\n"));
			return(1);
		}

		if (r->p_val < pAC->vpd.vpd_buf + vpd_ro_size) {
			SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR | SK_DBGCAT_FATAL,
				("Encoding Error: Invalid VPD struct size\n"));
			return(1);
		}

		pAC->vpd.v.vpd_free_rw = r->p_len;

#ifndef SK_SLIM		/* needs to be enabled for ASF w/o EEPROM */
		if ((pAC->vpd.v.vpd_free_rw < 3) &&
			(r->p_val == pAC->vpd.vpd_buf + vpd_ro_size + 6)) {
			pAC->vpd.v.vpd_status |= VPD_NO_RW;
		}
	}
#endif /* !SK_SLIM */

	/* everything seems to be ok */
	if (pAC->GIni.GIChipId != 0) {
		pAC->vpd.v.vpd_status |= VPD_VALID;
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_INIT,
		("done. Free RO = %d, Free RW = %d\n",
		pAC->vpd.v.vpd_free_ro, pAC->vpd.v.vpd_free_rw));

	return(0);
}


/*
 *	find the Keyword 'key' in the VPD buffer and fills the
 *	parameter struct 'p' with it's values
 *
 * returns	*p	success
 *		0:	parameter was not found or VPD encoding error
 */
static SK_VPD_PARA *vpd_find_para(
SK_AC		*pAC,	/* Adapters Context */
const char	*key,	/* keyword to find (e.g. "MN") */
SK_VPD_PARA	*p)		/* parameter description struct */
{
	char *v	;	/* points to VPD buffer */
	int max;	/* Maximum Number of Iterations */
	int len;

	v = pAC->vpd.vpd_buf;
	max = 128;

	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_CTRL,
		("VPD find para %s .. ", key));

	/* check mandatory resource type ID string (Product Name) */
	if (*v != (char)RES_ID) {
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR | SK_DBGCAT_FATAL,
			("Error: Resource type ID (0x%x) missing\n", RES_ID));
		return(NULL);
	}

	len = VPD_GET_RES_LEN(v);

	if (SK_STRCMP(key, VPD_NAME) == 0) {
		p->p_len = len;
		p->p_val = VPD_GET_VAL(v);

		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_CTRL,
			("found, len = %d\n", len));
		return(p);
	}

	v += 3 + len + 3;

	for ( ; ; ) {
		if (key[0] == v[0] && key[1] == v[1]) {

			p->p_len = VPD_GET_VPD_LEN(v);
			p->p_val = VPD_GET_VAL(v);

			SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_CTRL,
				("\nfound %s, len = %d\n", key, p->p_len));
			return(p);
		}

		/* exit when reaching the "RW" Tag or the maximum of iterations */
		max--;

		if (SK_MEMCMP(VPD_RW, v, 2) == 0 || max == 0) {
			break;
		}

		len = 3 + VPD_GET_VPD_LEN(v);

		if (SK_MEMCMP(VPD_RV, v, 2) == 0) {
			len += 3;					/* skip VPD-W */
		}
		v += len;

		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_CTRL,
			("\nscanning '%c%c' len = %d ", v[0], v[1], v[2]));
	}

#ifdef DEBUG
	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_CTRL,
		("\nnot found para %s\n", key));

	if (max == 0) {
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR | SK_DBGCAT_FATAL,
			("key/len encoding error\n"));
	}
#endif /* DEBUG */
	return(NULL);
}


/*
 *	Move 'n' bytes. Begin with the last byte if 'n' is > 0,
 *	Start with the last byte if n is < 0.
 *
 * returns nothing
 */
static void vpd_move_para(
char	*start,		/* start of memory block */
char	*end,		/* end of memory block to move */
int		n)			/* number of bytes the memory block has to be moved */
{
	char *p;
	int i;		/* number of byte copied */

	if (n == 0) {
		return;
	}

	i = (int) (end - start + 1);
	if (n < 0) {
		p = start + n;
		while (i != 0) {
			*p++ = *start++;
			i--;
		}
	}
	else {
		p = end + n;
		while (i != 0) {
			*p-- = *end--;
			i--;
		}
	}
}


/*
 *	setup the VPD keyword 'key' at 'ip'.
 *
 * returns nothing
 */
static void vpd_insert_key(
const char	*key,	/* keyword to insert */
const char	*buf,	/* buffer with the keyword value */
int		len,		/* length of the value string */
char	*ip)		/* inseration point */
{
	SK_VPD_KEY *p;

	p = (SK_VPD_KEY *)ip;
	p->p_key[0] = key[0];
	p->p_key[1] = key[1];
	p->p_len = (unsigned char)len;

	SK_MEMCPY(&p->p_val, buf, len);
}


/*
 *	Setup the VPD end tag "RV" / "RW".
 *	Also correct the remaining space variables vpd_free_ro / vpd_free_rw.
 *
 * returns	0:	success
 *		1:	encoding error
 */
static int vpd_mod_endtag(
SK_AC	*pAC,		/* Adapters Context */
char	*etp)		/* end pointer input position */
{
	SK_VPD_KEY	*p;
	unsigned char	x;
	int	i;
	int	vpd_size;

	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_CTRL,
		("VPD modify endtag at 0x%x = '%c%c'\n", (SK_U32)etp, etp[0], etp[1]));

	vpd_size = pAC->vpd.vpd_size;

	p = (SK_VPD_KEY *)etp;

	if (p->p_key[0] != 'R' || (p->p_key[1] != 'V' && p->p_key[1] != 'W')) {
		/* something wrong here, encoding error */
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR | SK_DBGCAT_FATAL,
			("Encoding Error: invalid end tag\n"));
		return(1);
	}

	if (etp > pAC->vpd.vpd_buf + vpd_size/2) {
		/* create "RW" tag */
		p->p_len = (unsigned char)(pAC->vpd.vpd_buf + vpd_size - etp - 4);
		pAC->vpd.v.vpd_free_rw = (int)p->p_len;
		i = pAC->vpd.v.vpd_free_rw;
		etp += 3;
	}
	else {
		/* create "RV" tag */
		p->p_len = (unsigned char)(pAC->vpd.vpd_buf + vpd_size/2 - etp - 3);
		pAC->vpd.v.vpd_free_ro = (int) p->p_len - 1;

		/* setup checksum */
		for (i = 0, x = 0; i < vpd_size/2 - p->p_len; i++) {
			x += pAC->vpd.vpd_buf[i];
		}
		p->p_val = (char)0 - x;
		i = pAC->vpd.v.vpd_free_ro;
		etp += 4;
	}

	while (i) {
		*etp++ = 0x00;
		i--;
	}

	return(0);
}


/*
 *	Insert a VPD keyword into the VPD buffer.
 *
 *	The keyword 'key' is inserted at the position 'ip' in the VPD buffer.
 *	The keywords behind the input position will be moved.
 *	The VPD end tag "RV" or "RW" is generated again.
 *
 * returns	0:	success
 *		2:	value string was cut
 *		4:	VPD full, keyword was not written
 *		6:	fatal VPD error
 *
 */
int	VpdSetupPara(
SK_AC	*pAC,		/* Adapters Context */
const char	*key,	/* keyword to insert */
const char	*buf,	/* buffer with the keyword value */
int		len,		/* length of the keyword value */
int		type,		/* VPD_RO_KEY or VPD_RW_KEY */
int		op)			/* operation to do: ADD_KEY or OWR_KEY */
{
	SK_VPD_PARA vp = { 0, 0 }; /* to make prefast happy */
	char	*etp;		/* end tag position */
	char	*ip;		/* input position inside the VPD buffer */
	int		free_sp;	/* remaining space in selected area */
	int		rtv;		/* return code */
	int		head;		/* additional header bytes to move */
	int		found;		/* additional bytes if the keyword was found */
	int		vpd_size;

	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_CTRL,
		("VPD setup para key = %s, val = %s\n", key, buf));

	vpd_size = pAC->vpd.vpd_size;

	rtv = 0;
	ip = 0;
	if (type == VPD_RW_KEY) {
		/* end tag is "RW" */
		free_sp = pAC->vpd.v.vpd_free_rw;
		etp = pAC->vpd.vpd_buf + (vpd_size - free_sp - 1 - 3);
	}
	else {
		/* end tag is "RV" */
		free_sp = pAC->vpd.v.vpd_free_ro;
		etp = pAC->vpd.vpd_buf + (vpd_size/2 - free_sp - 4);
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_CTRL,
		("Free RO = %d, Free RW = %d\n",
		pAC->vpd.v.vpd_free_ro, pAC->vpd.v.vpd_free_rw));

	head = 0;
	found = 0;
	if (op == OWR_KEY) {
		if (vpd_find_para(pAC, key, &vp)) {
			found = 3;
			ip = vp.p_val - 3;
			free_sp += vp.p_len + 3;

			SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_CTRL,
				("Overwrite Key\n"));
		}
		else {
			op = ADD_KEY;

			SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_CTRL,
				("Add Key\n"));
		}
	}

	if (op == ADD_KEY) {
		ip = etp;
		vp.p_len = 0;
		head = 3;
	}

	if (len + 3 > free_sp) {
		if (free_sp < 7) {
			SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR,
				("VPD Buffer Overflow, keyword not written\n"));
			return(4);
		}
		/* cut it again */
		len = free_sp - 3;
		rtv = 2;
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR,
			("VPD Buffer Full, Keyword was cut\n"));
	}

	vpd_move_para(ip + vp.p_len + found, etp + 2, len-vp.p_len + head);

	vpd_insert_key(key, buf, len, ip);

	if (vpd_mod_endtag(pAC, etp + len - vp.p_len + head)) {
		pAC->vpd.v.vpd_status &= ~VPD_VALID;
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR,
			("VPD Encoding Error\n"));
		return(6);
	}

	return(rtv);
}

#if (!defined(SK_SLIM) || defined(EFI_UNDI))
/*
 *	Copy the content of the VPD EEPROM to the VPD buffer if not already done.
 *
 * return:	A pointer to the s_vpd_status structure.
 */
SK_VPD_STATUS *VpdStat(
SK_AC	*pAC,	/* Adapters Context */
SK_IOC	IoC)	/* IO Context */
{
	if ((pAC->vpd.v.vpd_status & VPD_VALID) == 0) {
		(void)VpdInit(pAC, IoC);
	}
	return(&pAC->vpd.v);
}


/*
 *	Copy the content of the VPD EEPROM to the VPD buffer if not already done.
 *	Scan the VPD buffer for VPD keywords and create the VPD keyword list
 *	by copying the keywords to 'buf', terminated with a '\0'.
 *
 * Exceptions:
 *		o The Resource Type ID String (product name) is called "Name"
 *		o The VPD end tags 'RV' and 'RW' are not listed
 *
 *	The number of copied keywords is counted in 'elements'.
 *
 * returns	0:	success
 *		2:	buffer overflow, one or more keywords are missing
 *		6:	fatal VPD error
 *
 *	example values after returning:
 *
 *		buf =	"Name\0PN\0EC\0MN\0SN\0CP\0VF\0VL\0YA\0"
 *		*len =		30
 *		*elements =	 9
 */
int VpdKeys(
SK_AC	*pAC,		/* Adapters Context */
SK_IOC	IoC,		/* IO Context */
char	*buf,		/* buffer where to copy the keywords */
int		*len,		/* buffer length */
int		*elements)	/* number of keywords returned */
{
	char *v;
	int n;

	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_RX, ("list VPD keys .. "));

	*elements = 0;

	if ((pAC->vpd.v.vpd_status & VPD_VALID) == 0) {
		if (VpdInit(pAC, IoC) != 0) {
			*len = 0;
			SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR,
				("VPD Init Error, terminated\n"));
			return(6);
		}
	}

	n = SK_STRLEN(VPD_NAME) + 1;

	if (*len < n) {
		*len = 0;
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR, ("buffer overflow\n"));
		return(2);
	}

	SK_STRNCPY(buf, VPD_NAME, n);

	v = pAC->vpd.vpd_buf;

	buf += n;
	*elements = 1;
	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_CTRL,
		("Res.ID: 0x%02X (len: %u), ", (unsigned char)v[0], v[1]));

	v += 3 + VPD_GET_RES_LEN(v) + 3;

	for ( ; ; ) {
		/* exit when reaching the "RW" Tag */
		if (SK_MEMCMP(VPD_RW, v, 2) == 0) {
			break;
		}

		if (SK_MEMCMP(VPD_RV, v, 2) == 0) {
			v += 3 + VPD_GET_VPD_LEN(v) + 3;	/* skip VPD-W */
			continue;
		}

		if (*len < n + 3) {
			*len = n;
			SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR,
				("buffer overflow\n"));
			return(2);
		}

		SK_MEMCPY(buf, v, 2);

		buf += 2;
		*buf++ = '\0';
		n += 3;
		v += 3 + VPD_GET_VPD_LEN(v);
		*elements += 1;
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_CTRL,
			("'%c%c' ", v[0], v[1]));
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_RX, ("\n"));
	*len = n;
	return(0);
}
#endif /* !SK_SLIM || EFI_UNDI */

/*
 *	Copy the content of the VPD EEPROM to the VPD buffer if not already done.
 *	Search for the VPD keyword 'key' and copy its value to 'buf'.
 *	Add a terminating '\0'.
 *	If the value does not fit into the buffer cut it after 'len' - 1 bytes.
 *
 * returns	0:	success
 *		1:	keyword not found
 *		2:	value string was cut
 *		3:	VPD transfer timeout
 *		6:	fatal VPD error
 */
int VpdRead(
SK_AC		*pAC,	/* Adapters Context */
SK_IOC		IoC,	/* IO Context */
const char	*key,	/* keyword to read (e.g. "MN") */
char		*buf,	/* buffer where to copy the keyword value */
int			*len)	/* buffer length */
{
	SK_VPD_PARA *p, vp;

	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_RX, ("VPD read %s .. ", key));

	if ((pAC->vpd.v.vpd_status & VPD_VALID) == 0) {
		if (VpdInit(pAC, IoC) != 0) {
			*len = 0;
			SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR,
				("VPD init error\n"));
			return(6);
		}
	}

	if ((p = vpd_find_para(pAC, key, &vp)) == NULL) {
		*len = 0;
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR,
			("not found para %s\n", key));
		return(1);
	}

	if (p->p_len > (unsigned)(*len - 1)) {
		p->p_len = *len - 1;
	}

	SK_MEMCPY(buf, p->p_val, p->p_len);

	buf[p->p_len] = '\0';
	*len = p->p_len;

	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_RX,
		("0x%02X 0x%02X 0x%02X 0x%02X ... '%c%c%c%c...', len = %d\n",
		 (unsigned char)buf[0], (unsigned char)buf[1],
		 (unsigned char)buf[2], (unsigned char)buf[3],
		 buf[0], buf[1], buf[2], buf[3], *len));

	return(0);
}

#if (!defined(SK_SLIM) || defined(EFI_UNDI))
/*
 *	Check whether a given key may be written
 *
 * returns
 *	SK_TRUE		Yes, it may be written
 *	SK_FALSE	No
 */
SK_BOOL VpdMayWrite(
char	*key)	/* keyword to write (allowed values "Yx", "Vx") */
{
	if ((*key != 'Y' && *key != 'V') ||
		key[1] < '0' || key[1] > 'Z' ||
		(key[1] > '9' && key[1] < 'A') || SK_STRLEN(key) != 2) {

		return(SK_FALSE);
	}
	return(SK_TRUE);
}

/*
 *	Copy the content of the VPD EEPROM to the VPD buffer if not already done.
 *	Insert/overwrite the keyword 'key' in the VPD buffer.
 *	Cut the keyword value if it does not fit into the VPD read / write area.
 *
 * returns	0:	success
 *		2:	value string was cut
 *		3:	VPD transfer timeout
 *		4:	VPD full, keyword was not written
 *		5:	keyword cannot be written
 *		6:	fatal VPD error
 */
int VpdWrite(
SK_AC		*pAC,	/* Adapters Context */
SK_IOC		IoC,	/* IO Context */
const char	*key,	/* keyword to write (allowed values "Yx", "Vx") */
const char	*buf)	/* buffer where the keyword value can be read from */
{
	int len;		/* length of the keyword to write */
	int rtv;		/* return code */
	int rtv2;

	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_TX,
		("VPD write %s = %s\n", key, buf));

	if ((*key != 'Y' && *key != 'V') ||
		key[1] < '0' || key[1] > 'Z' ||
		(key[1] > '9' && key[1] < 'A') || SK_STRLEN(key) != 2) {

		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR,
			("illegal key tag, keyword not written\n"));
		return(5);
	}

	if ((pAC->vpd.v.vpd_status & VPD_VALID) == 0) {
		if (VpdInit(pAC, IoC) != 0) {
			SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR,
				("VPD init error\n"));
			return(6);
		}
	}

	rtv = 0;
	len = SK_STRLEN(buf);
	if (len > VPD_MAX_LEN) {
		/* cut it */
		len = VPD_MAX_LEN;
		rtv = 2;
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR,
			("keyword too long, cut after %d bytes\n", VPD_MAX_LEN));
	}

	if ((rtv2 = VpdSetupPara(pAC, key, buf, len, VPD_RW_KEY, OWR_KEY)) != 0) {
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR,
			("VPD write error\n"));
		return(rtv2);
	}

	return(rtv);
}

/*
 *	Copy the content of the VPD EEPROM to the VPD buffer if not already done.
 *	Remove the VPD keyword 'key' from the VPD buffer.
 *	Only the keywords in the read/write area can be deleted.
 *	Keywords in the read only area cannot be deleted.
 *
 * returns	0:	success, keyword was removed
 *		1:	keyword not found
 *		5:	keyword cannot be deleted
 *		6:	fatal VPD error
 */
int VpdDelete(
SK_AC	*pAC,	/* Adapters Context */
SK_IOC	IoC,	/* IO Context */
char	*key)	/* keyword to read (e.g. "MN") */
{
	SK_VPD_PARA *p, vp;
	char *etp;
	int	vpd_size;

	vpd_size = pAC->vpd.vpd_size;

	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_TX, ("VPD delete key %s\n", key));

	if ((pAC->vpd.v.vpd_status & VPD_VALID) == 0) {
		if (VpdInit(pAC, IoC) != 0) {
			SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR,
				("VPD init error\n"));
			return(6);
		}
	}

	if ((p = vpd_find_para(pAC, key, &vp)) == NULL) {
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR,
			("keyword not found\n"));
		return(1);
	}

	if (p->p_val < pAC->vpd.vpd_buf + vpd_size/2) {
		/* try to delete read only keyword */
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR,
			("cannot delete RO keyword\n"));
		return(5);
	}

	etp = pAC->vpd.vpd_buf + (vpd_size-pAC->vpd.v.vpd_free_rw-1-3);

	vpd_move_para(vp.p_val + vp.p_len, etp + 2,
		- ((int)(vp.p_len + 3)));

	if (vpd_mod_endtag(pAC, etp - vp.p_len - 3)) {
		pAC->vpd.v.vpd_status &= ~VPD_VALID;
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR,
			("VPD encoding error\n"));
		return(6);
	}

	return(0);
}

/*
 *	If the VPD buffer contains valid data write the VPD read/write area
 *	back to the VPD EEPROM.
 *
 * returns	0:	success
 *		2:	not supported
 *		3:	VPD transfer timeout
 */
int VpdUpdate(
SK_AC	*pAC,	/* Adapters Context */
SK_IOC	IoC)	/* IO Context */
{
	int vpd_size;

	vpd_size = pAC->vpd.vpd_size;

	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_TX, ("VPD update .. "));

#ifndef SK_SLIM		/* needs to be enabled for ASF w/o EEPROM */
	if (pAC->vpd.CfgInSpi) {
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_TX, ("not supported"));
		return(2);
	}
#endif /* !SK_SLIM */

	if ((pAC->vpd.v.vpd_status & VPD_VALID) != 0) {
		if (VpdTransferBlock(pAC, IoC, pAC->vpd.vpd_buf + vpd_size/2,
			vpd_size/2, vpd_size/2, VPD_WRITE) != vpd_size/2) {

			SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR,
				("transfer timed out\n"));
			return(3);
		}
	}

	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_TX, ("done\n"));
	return(0);
}

/*
 *	Copy the content of the VPD EEPROM to the VPD buffer if not already done.
 *	If the keyword "VF" is not present it will be created and
 *	the error log message will be stored to this keyword.
 *	If "VF" is not present the error log message will be stored to the
 *	keyword "VL". "VL" will created or overwritten if "VF" is present.
 *	The VPD read/write area is saved to the VPD EEPROM.
 *
 * returns nothing, errors will be ignored.
 */
void VpdErrLog(
SK_AC	*pAC,	/* Adapters Context */
SK_IOC	IoC,	/* IO Context */
char	*msg)	/* error log message */
{
	SK_VPD_PARA vf;
	int len;

	SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_TX,
		("VPD error log message %s\n", msg));

	if ((pAC->vpd.v.vpd_status & VPD_VALID) == 0) {
		if (VpdInit(pAC, IoC) != 0) {
			SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_ERR,
				("VPD init error\n"));
			return;
		}
	}

	len = SK_STRLEN(msg);
	if (len > VPD_MAX_LEN) {
		/* cut it */
		len = VPD_MAX_LEN;
	}

	if (vpd_find_para(pAC, VPD_VF, &vf) != NULL) {
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_TX, ("overwrite VL\n"));
		(void)VpdSetupPara(pAC, VPD_VL, msg, len, VPD_RW_KEY, OWR_KEY);
	}
	else {
		SK_DBG_MSG(pAC, SK_DBGMOD_VPD, SK_DBGCAT_TX, ("write VF\n"));
		(void)VpdSetupPara(pAC, VPD_VF, msg, len, VPD_RW_KEY, ADD_KEY);
	}

	(void)VpdUpdate(pAC, IoC);
}
#endif /* !SK_SLIM || EFI_UNDI*/

