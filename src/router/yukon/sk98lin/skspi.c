/******************************************************************************
 *
 * Name:	$Id: //Release/Yukon_1G/Shared/flashfun/V2/skspi.c#10 $
 * Project:	Flash Programmer, Manufacturing and Diagnostic Tools
 * Version:	$Revision: #10 $, $Change: 4280 $
 * Date:	$DateTime: 2010/11/05 11:55:33 $
 * Purpose:	Contains SPI-Flash EEPROM specific functions
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
#ifndef SK_SPI_NO_UPDATE
#ifdef MRVL_UEFI
#include <stdlib.h>
#endif /* !MRVL_UEFI */
#endif /* !SK_SPI_NO_UPDATE */
#include "h/skdrv2nd.h"


/* defines *******************************************************************/

/* SPI flash types of max. 1 MB (8Mbit) are currently supported */
#define MAX_SPI_FLASH_SIZE			0x100000L			

/* man_name, man_id */
#define SPI_MAN_ATMEL		"Atmel",	SPI_MAN_ID_ATMEL
#define SPI_MAN_INTEL		"Intel",	SPI_MAN_ID_INTEL
#define SPI_MAN_WBOND		"WinBnd",	SPI_MAN_ID_WINBOND
#define SPI_MAN_SST			"SST",		SPI_MAN_ID_SST
#define SPI_MAN_NUMONYX		"Numonyx",	SPI_MAN_ID_NUMONYX
#define SPI_MAN_NUM			"Numonyx",	SPI_MAN_ID_NUM
#define SPI_MAN_ST			"ST"
#define SPI_MAN_CHTEK		"ChTek",	SPI_MAN_ID_CHTEK
#define SPI_MAN_MXIC		"MXIC",		SPI_MAN_ID_MXIC

/* RARAtbd determine id_byte_count and dev_id2 where the latter is "-0" */

/* dev_name, dev_id, sector_size, sector_num, set_protocol, clk_rate_div, id_byte_count, dev_id2 */
#define SPI_AT25F2048		"AT25F2048",	 0x63, 0x10000,   4, 0, 0, 1, 0
#define SPI_AT25F1024		"AT25F1024",	 0x60, 0x8000,    4, 0, 0, 1, 0
#define SPI_AT25DF021		"AT25DF021",	 0x43, 0x1000,   64, 0, 0, 3, 0
#define SPI_AT25DF041A		"AT25DF041A",	 0x44, 0x1000,  128, 0, 0, 3, 1
#define SPI_AT26DF081A		"AT26DF081A",	 0x45, 0x1000,  256, 0, 0, 3, 1
#define SPI_AT26DF161A		"AT26DF161A",	 0x46, 0x1000,  512, 0, 0, 3, 1
#define SPI_AT26DF321		"AT26DF321",	 0x47, 0x1000, 1024, 0, 0, 3, 0
#define SPI_AT25FS010		"AT25FS010",	 0x66, 0x1000,   32, 0, 0, 2, 1
#define SPI_AT25FS040		"AT25FS040",	 0x66, 0x1000,   32, 0, 0, 2, 4
#define SPI_QB25F016S33B8	"QB25F016S33B8", 0x89, 0x10000,  32, 0, 0, 1, -0
#define SPI_W25X10			"W25X10",		 0x10, 0x1000,   32, 1, 1, 1, -0
#define SPI_SST25VF512		"SST25VF512",	 0x48, 0x1000,   16, 1, 0, 1, -0
#define SPI_SST25VF010		"SST25VF010/SST25VF010A",	 0x49, 0x1000,   32, 1, 0, 1, -0
#define SPI_SST25VF020		"SST25VF020",	 0x43, 0x1000,   64, 1, 0, 1, -0
#define SPI_SST25VF040		"SST25VF040",	 0x44, 0x1000,  128, 1, 0, 1, -0
#define SPI_SST25VF040B		"SST25VF040B",	 0x8d, 0x1000,  128, 1, 0, 1, -0
#define SPI_X_M25P80		"M25P80",		 0x20, 0x10000,  16, 0, 0, 2, 0x13
#define SPI_X_M25P20		"M25P20",		 0x20, 0x10000,	  4, 0, 0, 2, 0x11
#define SPI_X_M25P10A		"M25P10A",		 0x20, 0x8000,	  4, 0, 0, 2, 0x10
#define SPI_M25P80			"M25P80",		 0x20, 0x10000,  16, 1, 0, 1, -0
#define SPI_ST_M25P80		"ST M25P80",	 0x13, 0x10000,  16, 1, 0, 1, -0
#define SPI_M25P20			"M25P20",		 0x11, 0x10000,   4, 1, 0, 1, -0
#define SPI_M25P10			"M25P10",		 0x10, 0x8000,    4, 1, 0, 1, -0
#define SPI_Pm25LV512A		"Pm25LV512A",	 0x7b, 0x1000,   16, 1, 0, 1, -0
#define SPI_Pm25LV010A		"Pm25LV010A",	 0x7c, 0x1000,   32, 1, 0, 1, -0
#define SPI_Pm25LV020		"Pm25LV020",	 0x7d, 0x1000,   64, 1, 0, 1, -0
#define SPI_Pm25LV040		"Pm25LV040",	 0x7e, 0x1000,  128, 1, 0, 1, -0
#define SPI_Pm25LD010C		"Pm25LD010C",	 0x10, 0x1000,   32, 1, 0, 1, -0
#define SPI_Pm25LD020C		"Pm25LD020C",	 0x11, 0x1000,   64, 1, 0, 1, -0
#define SPI_MX25L200x		"MX25L2005/MX25L2006E",	 0x11,  0x1000, 64, 1, 0, 1, -0
#define SPI_MX25L400x		"MX25L4005A/MX25L4005C/MX25L4006E",	 0x12,  0x1000, 128, 1, 0, 1, -0

/* op_read, op_write, op_write_enable, op_write_disable, op_read_status */
#define SPI_CMDS_GENERIC	0x03, 0x02, 0x06, 0x04, 0x05

/* op_read, op_write_aai, op_write_enable, op_write_disable, op_read_status */
#define SPI_CMDS_SST_GENERIC	0x03, 0xad, 0x06, 0x04, 0x05


/* GENERICS, op_read_id, op_sector_erase, op_chip_erase, op_write_status, op_read_sect_protect */
#define SPI_CMDS_ATMEL_F	{SPI_CMDS_GENERIC, 0x15, 0x52, 0x62, 0, 0}
#define SPI_CMDS_ATMEL_DF	{SPI_CMDS_GENERIC, 0x9f, 0x20, 0x60, 1, 0x3c}
#define SPI_CMDS_ATMEL_FS	{SPI_CMDS_GENERIC, 0xab, 0x20, 0xc7, 0, 0}
#define SPI_CMDS_INTEL		{SPI_CMDS_GENERIC, 0x9f, 0xd8, 0xc7, 0, 0}
#define SPI_CMDS_WINBOND	{SPI_CMDS_GENERIC, 0x90, 0x20, 0xc7, 0, 0}
#define SPI_CMDS_SST		{SPI_CMDS_GENERIC, 0x90, 0x20, 0x60, 0, 0}
#define SPI_CMDS_SST_040B	{SPI_CMDS_SST_GENERIC, 0x90, 0x20, 0x60, 0, 0}
#define SPI_CMDS_ST			{SPI_CMDS_GENERIC, 0xab, 0xd8, 0xc7, 0, 0}
#define SPI_CMDS_CHTEK		{SPI_CMDS_GENERIC, 0xab, 0xd7, 0xc7, 0, 0}
#define SPI_CMDS_CHTEK_LD	{SPI_CMDS_GENERIC, 0x90, 0xd7, 0xc7, 0, 0}
#define SPI_CMDS_MXIC		{SPI_CMDS_GENERIC, 0x90, 0x20, 0xc7, 0, 0}


/* typedefs ******************************************************************/

/* function prototypes *******************************************************/

#ifdef Core
#define fl_print	pAC->Gui.WPrint
#elif defined(VCPU)
#define fl_print	c_print
#else
extern void fl_print(char *str, ...);
#endif /* Core */

/* low level SPI programming external interface */
extern int  spi_timer(SK_AC *pAC, unsigned int);
#ifndef SK_SPI_NO_UPDATE
extern void *spi_malloc(unsigned long);
extern void spi_free(void *);
#endif	/* !SK_SPI_NO_UPDATE */

/* global variables **********************************************************/

/* local variables ***********************************************************/

/*
 * Yukon-II family SPI flash device table
 *
 *											timeout for chip erase
 *											  |
 *											 \/	
 */
SK_SPI_DEVICE spi_yuk2_dev_table[] = {
	{SPI_MAN_ATMEL,		SPI_AT25F2048,		10, SPI_CMDS_ATMEL_F},
	{SPI_MAN_ATMEL,		SPI_AT25F1024,		 6, SPI_CMDS_ATMEL_F},
	{SPI_MAN_ATMEL,		SPI_AT25DF021,		 6, SPI_CMDS_ATMEL_DF},
	{SPI_MAN_ATMEL,		SPI_AT25DF041A,		10, SPI_CMDS_ATMEL_DF},
	{SPI_MAN_ATMEL,		SPI_AT26DF081A,		20, SPI_CMDS_ATMEL_DF},
	{SPI_MAN_ATMEL,		SPI_AT26DF161A,		40, SPI_CMDS_ATMEL_DF},
	{SPI_MAN_ATMEL,		SPI_AT26DF321,		65, SPI_CMDS_ATMEL_DF},
	{SPI_MAN_ATMEL,		SPI_AT25FS010,		30, SPI_CMDS_ATMEL_FS},
	{SPI_MAN_ATMEL,		SPI_AT25FS040,		10, SPI_CMDS_ATMEL_FS},
	{SPI_MAN_INTEL,		SPI_QB25F016S33B8, 140, SPI_CMDS_INTEL},
	{SPI_MAN_WBOND,		SPI_W25X10,			10, SPI_CMDS_WINBOND},
	{SPI_MAN_SST,		SPI_SST25VF512,		 2, SPI_CMDS_SST},
	{SPI_MAN_SST,		SPI_SST25VF010,		 2, SPI_CMDS_SST},
	{SPI_MAN_SST,		SPI_SST25VF020,		 2, SPI_CMDS_SST},
	{SPI_MAN_SST,		SPI_SST25VF040,		 2, SPI_CMDS_SST},
	{SPI_MAN_SST,		SPI_SST25VF040B,	 2, SPI_CMDS_SST_040B},
	{SPI_MAN_NUMONYX,	SPI_X_M25P80,		30, SPI_CMDS_INTEL},
	{SPI_MAN_NUMONYX,	SPI_X_M25P20,		10, SPI_CMDS_INTEL},
	{SPI_MAN_NUMONYX,	SPI_X_M25P10A,		15, SPI_CMDS_INTEL},
	{SPI_MAN_NUM,		SPI_M25P80,			99, SPI_CMDS_ST},
	{SPI_MAN_CHTEK,		SPI_Pm25LD010C,		 2, SPI_CMDS_CHTEK_LD},
	{SPI_MAN_CHTEK,		SPI_Pm25LD020C,		 2, SPI_CMDS_CHTEK_LD},
	{SPI_MAN_MXIC,		SPI_MX25L200x,		10, SPI_CMDS_MXIC},
	{SPI_MAN_MXIC,		SPI_MX25L400x,		10, SPI_CMDS_MXIC},
	{SPI_MAN_ST,  0x13,	SPI_ST_M25P80,		99, SPI_CMDS_ST},
	{SPI_MAN_ST,  0x11,	SPI_M25P20,			99, SPI_CMDS_ST},
	{SPI_MAN_ST,  0x10,	SPI_M25P10,			99, SPI_CMDS_ST},
	{SPI_MAN_CHTEK,		SPI_Pm25LV512A,		 2, SPI_CMDS_CHTEK},
	{SPI_MAN_CHTEK,		SPI_Pm25LV010A,		 2, SPI_CMDS_CHTEK},
	{SPI_MAN_CHTEK,		SPI_Pm25LV020,		 2, SPI_CMDS_CHTEK},
	{SPI_MAN_CHTEK,		SPI_Pm25LV040,		 2, SPI_CMDS_CHTEK},
};

/* local functions ***********************************************************/

/*****************************************************************************
 *
 * spi_yuk_flash_erase - Erases sectors of the SPI Eprom
 *
 * Description:
 *	This function erases sectors of the SPI Eprom.
 *
 * Returns:
 *	0	Success
 *	1	Timeout
 */
static int spi_yuk_flash_erase(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned long	off,	/* start offset in flash eprom for erase */
unsigned long	len)	/* length in flash eprom for erase */
{
	char			cr;
	unsigned long	creg, i;

	if ((off + len - 1) > 0x1ffffL) {
		return (1);	/* more than 4 segments for erase */
	}

	/* Save SPI control register */
	SK_IN32(IoC, SPI_CTRL_REG, &creg);

#ifdef XXX
	/* there is some problem with chip erase. dr */

	/*
	 * RA:	Additionally, the newer SPI EPROM AT25FS010 used with
	 *		some 88E8001 chips uses a different code for chip erase.
	 */

	/* use faster chip erase command if all sectors should be erased */
	if (len > SPI_SECT_SIZE * 3L) {

		SK_OUT32(IoC, SPI_CTRL_REG, SPI_CHIP_ERASE);

		spi_timer(pAC, SPI_TIMER_SET);

		do {
			if (spi_timer(pAC, SPI_TIMEOUT)) {
				fl_print("\nSPI chip erase timeout: %d sec\n", SPI_TIMER_SET);
				return (1);
			}

			/* Read device status */
			SK_IN8(IoC, SPI_CTRL_REG, &cr);
		} while (cr & 1);		/* is chip busy? */

		/* Restore SPI control register */
		SK_OUT32(IoC, SPI_CTRL_REG, creg);

		return (0);
	}
#endif

	for (i = (off >> 15); i <= ((off + len - 1) >> 15); i++) {
		/* Clear chip command */
		SK_OUT32(IoC, SPI_CTRL_REG, (i << 22) | SPI_SECT_ERASE);

		spi_timer(pAC, SPI_TIMER_SET);

		do {
			if (spi_timer(pAC, SPI_TIMEOUT)) {
				fl_print("\nSPI chip erase timeout: %d sec\n", SPI_TIMER_SET);
				return (1);
			}

			/* Read device status */
			SK_IN8(IoC, SPI_CTRL_REG, &cr);
		} while (cr & 1);		/* is chip busy ? */
	}

	/* Restore SPI control register */
	SK_OUT32(IoC, SPI_CTRL_REG, creg);

	return (0);
}

/*****************************************************************************
 *
 * rol2 - Rotates a 16-bit value left by 2
 *
 * Description:
 *	This function rotates a 16-bit value left by 2.
 *
 * Returns:
 *	Address
 */
static unsigned short rol2(
SK_AC			*pAC,
unsigned short	addr)
{
	/* YUKON-Lite Rev. A1 */
	if (pAC->spi.yk_chip_id == CHIP_ID_YUKON_LITE) {

		/* switch address bits [1:0] with [14:13] */
		addr = ((addr >> 13) & 0x0003) | ((addr << 2) & 0x7ffc);
	}

	return (addr);
}

/*****************************************************************************
 *
 * spi_yuk_flash_manage - Reads, verifies or writes SPI Eprom
 *
 * Description:
 *	This function reads, verifies or writes the SPI Eprom.
 *
 * Returns:
 *	0	Success
 *	1	Timeout
 */
static int spi_yuk_flash_manage(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned char	*data,	/* data buffer */
unsigned long	off,	/* start offset in flash eprom for operation */
unsigned long	len,	/* length in flash eprom */
int				flag)	/* action SPI_READ / SPI_VERIFY / SPI_WRITE */
{
	unsigned long	a1, a2, cr, i;
	unsigned short	addr, as, ret=0;
	unsigned char	ch, tr;
	unsigned long	progress;
	unsigned long	last_progress;

	/* Save VPD lower address */
	SK_IN32(IoC, SPI_ADR_REG1, &a1);

	/* Save VPD higher address */
	SK_IN32(IoC, SPI_ADR_REG2, &a2);

	/* Set VPD lower address to 0 (15 higher bit) */
	SK_OUT32(IoC, SPI_ADR_REG1, SPI_VPD_MIN);

	/* Set VPD higher address to 0x7fff (15 higher bit) */
	SK_OUT32(IoC, SPI_ADR_REG2, SPI_VPD_MAX);

	/* Save SPI control register */
	SK_IN32(IoC, SPI_CTRL_REG, &cr);

	/* Enable VPD to SPI mapping (set bit 19) */
	SK_OUT32(IoC, SPI_CTRL_REG, SPI_VPD_MAP);

	/* Save Test Control Register 1 */
	SK_IN8(IoC, B2_TST_REG1, &tr);

	/* Enable write to mapped PCI config register file */
	SK_OUT8(IoC, B2_TST_REG1, 2);

	progress = last_progress = 0;

	for (i = off, addr = (unsigned short)(off >> 2); i < off + len; i++) {
		progress = (i * 100) / len;

#ifdef MRVL_UEFI
		if ((progress - last_progress) >= 20) {
			w_print(".");
			last_progress += 20;
		}
#else	/*  MRVL_UEFI */
		if ((progress - last_progress) >= 10) {
			fl_print(".");
			last_progress += 10;
		}
#endif	/*  MRVL_UEFI */

		/* Set new address to VPD Address reg, initiate reading */
		if ((i % 4) == 0) {

			SK_OUT16(IoC, VPD_ADR_REG, rol2(pAC, addr));
			addr++;

			/* Wait for termination */
			spi_timer(pAC, SPI_TIMER_SET);

			do {
				/* Read VPD_ADDR reg for flag check */
				SK_IN16(IoC, VPD_ADR_REG, &as);

				if (spi_timer(pAC, SPI_TIMEOUT)) {
					fl_print("\nSPI read timeout: %d sec. Offset:0x%05lx\n",
						SPI_TIMER_SET, i);
					ret = 1;
					break;
				}
			} while (!(as & VPD_FLAG_MASK)); /* check  flag */

			if (ret != 0) {
				break;
			}
		}

		switch (flag) {
		case SPI_READ:
			/* Read byte from VPD port */
			SK_IN8(IoC, (unsigned short)(VPD_DATA_PORT + i % 4), &ch);
			*(data++) = ch;
			break;
		case SPI_VERIFY:
			/* Read and verify byte from VPD port */
			SK_IN8(IoC, (unsigned short)(VPD_DATA_PORT + i % 4), &ch);
			if (ch != *(data++)) {
				fl_print("\n*** SPI data verify error at address 0x%05lx, "
					"is %x, should be %x\n", i, ch, *(data - 1));
				ret = 1;
			}
			break;
		case SPI_WRITE:
			/* Write byte to VPD port */
			SK_OUT8(IoC, (unsigned short)(VPD_DATA_PORT +
				(unsigned short)(i % 4)), *data);
			data++;

			if ((i % 4) == 3) {
				/* Set old Address to VPD_ADDR reg, initiate writing */
				SK_OUT16(IoC, (unsigned short)VPD_ADR_REG, (unsigned short)
					(rol2(pAC, (unsigned short)(addr - 1)) | VPD_FLAG_MASK));

				/* Wait for termination */
				spi_timer(pAC, SPI_TIMER_SET);

				do {
					/* Read VPD_ADDR reg for flag check*/
					SK_IN16(IoC, VPD_ADR_REG, &as);

					if (spi_timer(pAC, SPI_TIMEOUT)) {
						fl_print("\nSPI write timeout: %d sec. Offset: 0x%05lx\n",
							SPI_TIMER_SET, i);
						ret = 1;
						break;
					}
				} while (as & VPD_FLAG_MASK); /* check  flag */
			}
			break;
		}

		if (ret != 0) {
			break;
		}
	}
	/* Restore Test Control Register 1 */
	SK_OUT8(IoC, B2_TST_REG1, tr);

	/* Restore VPD lower address*/
	SK_OUT32(IoC, SPI_ADR_REG1, a1);

	/* Restore VPD higher address*/
	SK_OUT32(IoC, SPI_ADR_REG2, a2);

	/* Restore SPI control register */
	SK_OUT32(IoC, SPI_CTRL_REG, cr);

	fl_print(".");
	return (ret);
}

#ifndef SK_SPI_NO_UPDATE

/*****************************************************************************
 *
 * spi_yuk_update_config - Updates part of config area
 *
 * Description:
 *	This function updates part of the config area.
 *
 * Returns:
 *	0	Success
 *	1	Timeout
 */
static int spi_yuk_update_config(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned char	*data,	/* data buffer */
unsigned long	off,	/* start offset in flash eprom (config area) for operation */
unsigned long	len)	/* length of changing data */
{
	unsigned char *spibuf;
	unsigned long i;

	spibuf = spi_malloc((unsigned long)SPI_SECT_SIZE);

	if (spibuf == NULL) {
		return (51);
	}

	if (spi_flash_manage(pAC, IoC, spibuf, SPI_LSECT_OFF,
		SPI_SECT_SIZE, SPI_READ)) {

		spi_free(spibuf);
		return (1);
	}

	for (i = 0; i < len; i++) {
		spibuf[off + i - SPI_LSECT_OFF] = data[i];
	}

	if (spi_flash_erase(pAC, IoC, SPI_LSECT_OFF, SPI_SECT_SIZE)) {

		spi_free(spibuf);
		return (7);
	}

	if (spi_flash_manage(pAC, IoC, spibuf, SPI_LSECT_OFF,
		SPI_SECT_SIZE, SPI_WRITE)) {

		spi_free(spibuf);
		return (8);
	}
	spi_free(spibuf);
	return (0);
}

#endif	/* !SK_SPI_NO_UPDATE */

/*****************************************************************************
 *
 * spi_yuk2_write_enable - Enables writing on the SPI EPROM
 *
 * Description:
 *	This function enables writing on the SPI EPROM.
 *
 * Returns:
 *	Nothing
 */
static void spi_yuk2_write_enable(
SK_AC	*pAC,
SK_IOC	IoC)
{
	unsigned long spi_ctrl_reg;

	/* execute write enable command */
	SK_IN32(IoC, SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
	spi_ctrl_reg &= ~SPI_Y2_CMD_MASK;
	spi_ctrl_reg |= SPI_Y2_WEN;
	SK_OUT32(IoC, SPI_Y2_CONTROL_REG, spi_ctrl_reg);

	/* wait for the SPI to finish command */
	SPI_Y2_WAIT_SE_FINISH_WR(pAC, IoC, SPI_TIMER_SET);
}

/*****************************************************************************
 *
 * spi_yuk2_write_disable - Disables writing on the SPI EPROM
 *
 * Description:
 *	This function disables writing on the SPI EPROM.
 *
 * Returns:
 *	Nothing
 */
static void spi_yuk2_write_disable(
SK_AC	*pAC,
SK_IOC	IoC)
{
	unsigned long	op2;

	/* Replace Write Enable opcode by Write Disable opcode */
	SK_IN32(IoC, SPI_Y2_OPCODE_REG2, &op2);
	SK_OUT32(IoC, SPI_Y2_OPCODE_REG2, (op2 & 0xffffff00) | 
				(unsigned long)(pAC->spi.pSpiDev->opcodes.op_write_disable));

	spi_yuk2_write_enable(pAC, IoC);

	/* Restore Write Enable opcode */
	SK_OUT32(IoC, SPI_Y2_OPCODE_REG2, op2);
}

/*****************************************************************************
 *
 * spi_yuk2_write_status - execute the write status register command
 *
 * Description:
 *	This function execute the write status register command
 *	to protect or unprotect all sectors of the chip.
 *
 * Returns:
 *	0	Success
 *	1	Timeout
 */
static int spi_yuk2_write_status(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned char	Val)	/* value to write (0 = unprotect, 0x3c = protect) */
{
	unsigned long	spi_ctrl_reg;
	unsigned long	op;
#ifdef SK_DIAG
	unsigned long	RdVal;
#endif /* SK_DIAG */

	if (!pAC->spi.needs_unprotect) {
		/* Chip does not need unprotect before write/erase operation. */
		return (0);
	}

	/* enable writing */
	spi_yuk2_write_enable(pAC, IoC);

	if (pAC->spi.yk_chip_id >= CHIP_ID_YUKON_OPT &&
		pAC->spi.pSpiDev->opcodes.op_read_sect_protect != 0) {

		/* write SPI address */
		SK_OUT32(IoC, SPI_Y2_ADDRESS_REG, 0);

		/* execute the read sector protection registers command */
		SK_IN32(IoC, SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
		spi_ctrl_reg &= ~SPI_Y2_CMD_MASK;
		spi_ctrl_reg |= SPI_Y2_RDSP;
		SK_OUT32(IoC, SPI_Y2_CONTROL_REG, spi_ctrl_reg);

		/* wait for the SPI to finish RD operation */
		SPI_Y2_WAIT_SE_FINISH_CMD(pAC, IoC, SPI_TIMER_SET);

#ifdef SK_DIAG
		/* read SPI data */
		SK_IN32(IoC, SPI_Y2_DATA_REG, &RdVal);
		c_print("Sector 0 protection before write status is %02x.\n",
			RdVal >> 24);
#endif /* SK_DIAG */
	}

	/* write SPI address */
	SK_OUT32(IoC, SPI_Y2_ADDRESS_REG, Val | Val << 8 | Val << 16 | Val << 24);
	/* write SPI data */
	SK_OUT32(IoC, SPI_Y2_DATA_REG, Val | Val << 8 | Val << 16 | Val << 24);

	SK_IN32(IoC, SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
	spi_ctrl_reg &= ~SPI_Y2_CMD_MASK;

	if (pAC->spi.yk_chip_id >= CHIP_ID_YUKON_OPT &&
		pAC->spi.pSpiDev->opcodes.op_write_status != 0) {

		/* execute the write status register command */
		spi_ctrl_reg |= SPI_Y2_WST;
		SK_OUT32(IoC, SPI_Y2_CONTROL_REG, spi_ctrl_reg);

		/* wait for the SPI to finish RD operation */
		SPI_Y2_WAIT_SE_FINISH_CMD(pAC, IoC, SPI_TIMER_SET);
	}
	else {
		/* change command register 2 */
		SK_IN32(IoC, SPI_Y2_OPCODE_REG2, &op);
		SK_OUT32(IoC, SPI_Y2_OPCODE_REG2, (op & 0xff00ffff) | 0x00010000);

		/*
		 * execute the write status register command
		 * using the sector erase protocol
		 */
		spi_ctrl_reg |= SPI_Y2_SERS;
		SK_OUT32(IoC, SPI_Y2_CONTROL_REG, spi_ctrl_reg);

		/* wait for the SPI to finish RD operation */
		SPI_Y2_WAIT_SE_FINISH_CMD(pAC, IoC, SPI_TIMER_SET);

		/* restore command register 2 */
		SK_OUT32(IoC, SPI_Y2_OPCODE_REG2, op);
	}

	SK_IN32(IoC, SPI_Y2_CONTROL_REG, &spi_ctrl_reg);

	if (pAC->spi.yk_chip_id >= CHIP_ID_YUKON_OPT &&
		pAC->spi.pSpiDev->opcodes.op_read_sect_protect != 0) {

		/* write SPI address */
		SK_OUT32(IoC, SPI_Y2_ADDRESS_REG, 0);

		/* execute the read sector protection registers command */
		SK_IN32(IoC, SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
		spi_ctrl_reg &= ~SPI_Y2_CMD_MASK;
		spi_ctrl_reg |= SPI_Y2_RDSP;
		SK_OUT32(IoC, SPI_Y2_CONTROL_REG, spi_ctrl_reg);

		/* wait for the SPI to finish RD operation */
		SPI_Y2_WAIT_SE_FINISH_CMD(pAC, IoC, SPI_TIMER_SET);

#ifdef SK_DIAG
		/* read SPI data */
		SK_IN32(IoC, SPI_Y2_DATA_REG, &RdVal);
		c_print("Sector 0 protection after write status is %02x.\n",
			RdVal >> 24);
#endif /* SK_DIAG */
	}

	return (0);
}	/* spi_yuk2_write_status */

/*****************************************************************************
 *
 * spi_yuk2_read_dword - Reads a DWord from the SPI EPROM
 *
 * Description:
 *	This function reads a DWord from the specified address in the SPI EPROM.
 *
 * Returns:
 *	DWord read from given address
 */
static unsigned long spi_yuk2_read_dword(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned long	address)		/* address in the SPI EPROM to read from */
{
	unsigned long reg_val;
	unsigned long spi_ctrl_reg;

	/* write SPI address */
	SK_OUT32(IoC, SPI_Y2_ADDRESS_REG, address);

	/* execute SPI read command */
	SK_IN32(IoC, SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
	spi_ctrl_reg &= ~SPI_Y2_CMD_MASK;
	spi_ctrl_reg |= SPI_Y2_RD;
	SK_OUT32(IoC, SPI_Y2_CONTROL_REG, spi_ctrl_reg);

	/* wait for the SPI to finish RD operation */
	SPI_Y2_WAIT_SE_FINISH_CMD(pAC, IoC, SPI_TIMER_SET);

	/* read the returned data */
	SK_IN32(IoC, SPI_Y2_DATA_REG, &reg_val);

	return (reg_val);
}

/*****************************************************************************
 *
 * spi_yuk2_write_dword - Writes a DWord to the SPI EPROM
 *
 * Description:
 *	This function writes a DWord to the specified address in the SPI EPROM.
 *  The target sector should have been erased previously for the access
 *	to succeed.
 *
 * Returns:
 *	0	Success
 *	1	Failure
 */
static int spi_yuk2_write_dword(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned long	address,			/* address to write to */
unsigned long	value)				/* new value to be written */
{
	unsigned long spi_ctrl_reg;
	unsigned long verify_value;

	spi_yuk2_write_enable(pAC, IoC);

	/* write SPI address */
	SK_OUT32(IoC, SPI_Y2_ADDRESS_REG, address);
	/* write the new value */
	SK_OUT32(IoC, SPI_Y2_DATA_REG, value);

	/* execute the SPI write command */
	SK_IN32(IoC, SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
	spi_ctrl_reg &= ~SPI_Y2_CMD_MASK;
	spi_ctrl_reg |= SPI_Y2_WR;
	SK_OUT32(IoC, SPI_Y2_CONTROL_REG, spi_ctrl_reg);

	/* wait for write to finish */
	SPI_Y2_WAIT_SE_FINISH_WR(pAC, IoC, SPI_TIMER_SET);

	verify_value = spi_yuk2_read_dword(pAC, IoC, address);

	/* verify if write was successful*/
	if (verify_value != value) {
		fl_print("\n*** SPI data write error at address 0x%08lx, "
			"is 0x%08lx, should be 0x%08lx\n", address, verify_value, value);
		return (1);
	}

	return (0);
}

/*****************************************************************************
 *
 * spi_yuk2_sst_write_dword - Writes a DWord to an SST SPI EPROM
 *
 * Description:
 *	This function writes a DWord to the specified address in an SST SPI EPROM.
 *  The target sector should have been erased previously for the access
 *	to succeed.
 *
 * Returns:
 *	0	Success
 *	1	Failure
 */
static int spi_yuk2_sst_write_dword(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned long	address,		/* address to write to */
unsigned long	value)			/* new value to be written */
{
	unsigned long spi_ctrl_reg;
	unsigned long verify_value;
	unsigned long i;

	if (pAC->spi.pSpiDev->dev_id == 0x8d) {
		/* 
		 * For SST25VF040B we use auto address increment (AAI) mode 
		 * to program the SPI flash due to a byte program issue with 
		 * 80MHz type at the Yukon SPI flash interface.
		 */
		for (i = 0; i < 4; i+=2) {
			spi_yuk2_write_enable(pAC, IoC);

			/* write SPI address */
			SK_OUT32(IoC, SPI_Y2_ADDRESS_REG, address + i);
			/* write the new value */
			SK_OUT32(IoC, SPI_Y2_DATA_REG, (value >> i * 8) & 0x0000ffff);

			/* execute the SPI write command */
			SK_IN32(IoC, SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
			spi_ctrl_reg &= ~SPI_Y2_CMD_MASK;
			spi_ctrl_reg |= SPI_Y2_WR;
			SK_OUT32(IoC, SPI_Y2_CONTROL_REG, spi_ctrl_reg);

			/* wait for write to finish */
			SPI_Y2_WAIT_SE_FINISH_WR(pAC, IoC, SPI_TIMER_SET);

			spi_yuk2_write_disable(pAC, IoC);

			verify_value = spi_yuk2_read_dword(pAC, IoC, address);

			/* verify if write was successful*/
			if (((verify_value >> i * 8) & 0x0000ffff) !=
				((value >> i * 8) & 0x0000ffff)) {

				fl_print("\n*** SPI data write error at address 0x%08lx, "
					"is 0x%08lx, should be 0x%08lx\n", address + i,
					((verify_value >> i * 8) & 0x0000ffff),
					((value >> i * 8) & 0x0000ffff));
				return (1);
			}
		}
	}
	else {
		for (i = 0; i < 4; i++) {
			spi_yuk2_write_enable(pAC, IoC);

			/* write SPI address */
			SK_OUT32(IoC, SPI_Y2_ADDRESS_REG, address + i);
			/* write the new value */
			SK_OUT32(IoC, SPI_Y2_DATA_REG, (value >> i * 8) & 0x000000ff);

			/* execute the SPI write command */
			SK_IN32(IoC, SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
			spi_ctrl_reg &= ~SPI_Y2_CMD_MASK;
			spi_ctrl_reg |= SPI_Y2_WR;
			SK_OUT32(IoC, SPI_Y2_CONTROL_REG, spi_ctrl_reg);

			/* wait for write to finish */
			SPI_Y2_WAIT_SE_FINISH_WR(pAC, IoC, SPI_TIMER_SET);

			verify_value = spi_yuk2_read_dword(pAC, IoC, address);

			/* verify if write was successful*/
			if (((verify_value >> i * 8) & 0x000000ff) !=
				((value >> i * 8) & 0x000000ff)) {

				fl_print("\n*** SPI data write error at address 0x%08lx, "
					"is 0x%08lx, should be 0x%08lx\n", address + i,
					((verify_value >> i * 8) & 0x000000ff),
					((value >> i * 8) & 0x000000ff));
				return (1);
			}
		}
	}

	return (0);
}

/*****************************************************************************
 *
 * spi_yuk2_sst_clear_write_protection - Enables writing on an SST SPI EPROM
 *
 * Description:
 *	This function clears the write protection bits for SST flash types because
 *  they are set by default.
 *
 * Returns:
 *	Nothing
 */
static void spi_yuk2_sst_clear_write_protection(
SK_AC	*pAC,
SK_IOC	IoC)
{
	unsigned long	spi_ctrl_reg;
	unsigned long	op1;
	unsigned long	op2;
	unsigned long	addr;
	unsigned char	status;

	/* change command registers */
	SK_IN32(IoC, SPI_Y2_OPCODE_REG1, &op1);
	SK_OUT32(IoC, SPI_Y2_OPCODE_REG1, (op1 & 0xffff00ff) | 0x00000100);

	SK_IN32(IoC, SPI_Y2_OPCODE_REG2, &op2);
	SK_OUT32(IoC, SPI_Y2_OPCODE_REG2, (op2 & 0xffffff00) | 0x00000050);

	/* execute write status register enable command */
	SK_IN32(IoC, SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
	spi_ctrl_reg &= ~SPI_Y2_CMD_MASK;
	spi_ctrl_reg |= SPI_Y2_WEN;
	SK_OUT32(IoC, SPI_Y2_CONTROL_REG, spi_ctrl_reg);

	/* wait for the SPI to finish RD operation */
	SPI_Y2_WAIT_SE_FINISH_CMD(pAC, IoC, SPI_TIMER_SET);

	SK_IN32(IoC, SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
	status = (unsigned char)(spi_ctrl_reg & 0xfffffff3);
	addr = ((unsigned long)status | (unsigned long)(status << 8) |
		(unsigned long)(status << 16) | (unsigned long)(status << 24));

	/* write SPI address */
	SK_OUT32(IoC, SPI_Y2_ADDRESS_REG, addr);
	/* write the new value */
	SK_OUT32(IoC, SPI_Y2_DATA_REG, addr);

	/* execute the write status register command */
	SK_IN32(IoC, SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
	spi_ctrl_reg &= ~SPI_Y2_CMD_MASK;
	spi_ctrl_reg |= SPI_Y2_RD;
	SK_OUT32(IoC, SPI_Y2_CONTROL_REG, spi_ctrl_reg);

	/* wait for the SPI to finish RD operation */
	SPI_Y2_WAIT_SE_FINISH_CMD(pAC, IoC, SPI_TIMER_SET);

	SK_IN32(IoC, SPI_Y2_CONTROL_REG, &spi_ctrl_reg);

	/* restore command registers */
	SK_OUT32(IoC, SPI_Y2_OPCODE_REG1, op1);
	SK_OUT32(IoC, SPI_Y2_OPCODE_REG2, op2);
}	/* spi_yuk2_sst_clear_write_protection */

/*****************************************************************************
 *
 * spi_yuk2_erase_sector - Erases one sector of the SPI EPROM
 *
 * Description:
 *	This function erases one sector of the SPI EPROM.
 *
 * Returns:
 *	Nothing
 */
static void spi_yuk2_erase_sector(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned long	sector_num)		/* sector to be erased */
{
	unsigned long spi_ctrl_reg;

	spi_yuk2_write_enable(pAC, IoC);

	/* write sector start address */
	SK_OUT32(IoC, SPI_Y2_ADDRESS_REG, pAC->spi.pSpiDev->sector_size * sector_num);

	/* execute erase sector command */
	SK_IN32(IoC, SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
	spi_ctrl_reg &= ~SPI_Y2_CMD_MASK;
	spi_ctrl_reg |= SPI_Y2_SERS;
	SK_OUT32(IoC, SPI_Y2_CONTROL_REG, spi_ctrl_reg);

	SPI_Y2_WAIT_SE_FINISH_WR(pAC, IoC, SPI_TIMER_SET);
}

/*****************************************************************************
 *
 * spi_yuk2_erase_chip - Erases the SPI EPROM
 *
 * Description:
 *	This function erases the complete SPI EPROM.
 *	Timeout for chip erase / bulk erase varies widely
 *	between chips and manufacturers.
 *
 * Returns:
 *	Nothing
 */
static void spi_yuk2_erase_chip(
SK_AC		*pAC,
SK_IOC		IoC,
unsigned	Timeout)
{
	unsigned long spi_ctrl_reg;

	spi_yuk2_write_enable(pAC, IoC);

	/* execute erase chip command */
	SK_IN32(IoC, SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
	spi_ctrl_reg &= ~SPI_Y2_CMD_MASK;
	spi_ctrl_reg |= SPI_Y2_CERS;
	SK_OUT32(IoC, SPI_Y2_CONTROL_REG, spi_ctrl_reg);

	SPI_Y2_WAIT_SE_FINISH_WR(pAC, IoC, Timeout);
}

/*****************************************************************************
 *
 * spi_yuk2_read_chip_id - Reads the SPI vendor and device ID
 *
 * Description:
 *	This function reads the SPI vendor and device ID.
 *
 * Returns:
 *	The lower byte contains the device ID and the upper byte the vendor ID
 */
static unsigned long spi_yuk2_read_chip_id(
SK_AC	*pAC,
SK_IOC	IoC)
{
	unsigned long	chip_id;
	unsigned long	spi_ctrl_reg;
	unsigned long	opcode_reg1;

	/* Workaround for Yukon-Extreme SPI issue */
	SK_IN32(IoC, SPI_Y2_CONTROL_REG, &spi_ctrl_reg);

	spi_ctrl_reg &= SPI_Y2_CLK_DIV_MASK;
	SK_OUT32(IoC, SPI_Y2_CONTROL_REG, spi_ctrl_reg);

	/* Set read id opcode for currently selected flash */
	SK_IN32(IoC, SPI_Y2_OPCODE_REG1, &opcode_reg1);
	opcode_reg1 &= 0xff00ffffL;
	opcode_reg1 |= (unsigned long)pAC->spi.pSpiDev->opcodes.op_read_id << 16;
	SK_OUT32(IoC, SPI_Y2_OPCODE_REG1, opcode_reg1);

	/* Read SPI control register */
	SK_IN32(IoC, SPI_Y2_CONTROL_REG, &spi_ctrl_reg);

	spi_ctrl_reg &=
		~(SPI_Y2_CMD_MASK | SPI_Y2_ID_BYTE_COUNT_MASK | SPI_Y2_RDID_PROT);

	if (pAC->spi.yk_chip_id >= CHIP_ID_YUKON_OPT) {
		/* Select byte count for chip id readout */
		spi_ctrl_reg |= pAC->spi.pSpiDev->id_byte_count << 20;
	}

	/* Select protocol for chip id readout */
	if (pAC->spi.pSpiDev->set_protocol) {
		spi_ctrl_reg |= SPI_Y2_RDID_PROT;
	}

	/* Execute read chip id command */
	spi_ctrl_reg |= SPI_Y2_RDID;
	SK_OUT32(IoC, SPI_Y2_CONTROL_REG, spi_ctrl_reg);

	SPI_Y2_WAIT_SE_FINISH_CMD(pAC, IoC, SPI_TIMER_SET);

	SK_IN32(IoC, SPI_Y2_VENDOR_DEVICE_ID_REG, &chip_id);

	return (chip_id);
}

/*****************************************************************************
 *
 * spi_yuk2_set_dev_ptr - Identifies the SPI device and sets a pointer
 *
 * Description:
 *	This function identifies the SPI device and
 *	sets the pointer to its device table entry.
 *
 * Returns:
 *	pAC->spi.pSpiDev == NULL: unknown or no flash device
 *	pAC->spi.pSpiDev != NULL: pointer to flash device table entry
 */
static void spi_yuk2_set_dev_ptr(
SK_AC	*pAC,
SK_IOC	IoC)
{
	unsigned long	spi_chip_id;
	unsigned char	man_id;
	unsigned char	dev_id;
	unsigned char	dev_id2 = 0;
	unsigned long	spi_ctrl_reg;
	unsigned int	i;

	/* Search for flash in device table. */
	for (i = 0; i < (sizeof(spi_yuk2_dev_table) / sizeof(SK_SPI_DEVICE)); i++) {
		if ((pAC->spi.yk_chip_id != CHIP_ID_YUKON_EC_U &&
			 pAC->spi.yk_chip_id != CHIP_ID_YUKON_EX &&
			 pAC->spi.yk_chip_id < CHIP_ID_YUKON_SUPR &&
			 spi_yuk2_dev_table[i].man_id == SPI_MAN_ID_ATMEL) ||
			(pAC->spi.yk_chip_id < CHIP_ID_YUKON_OPT &&
			 spi_yuk2_dev_table[i].id_byte_count == 2)) {

			continue;
		}

		/* Used by e.g. spi_yuk2_read_chip_id(). */
		pAC->spi.pSpiDev = &spi_yuk2_dev_table[i];

		spi_chip_id = spi_yuk2_read_chip_id(pAC, IoC);

		if (pAC->spi.yk_chip_id < CHIP_ID_YUKON_OPT) {
			man_id = (unsigned char)((spi_chip_id & SPI_Y2_MAN_ID_MASK) >> 8);
			dev_id = (unsigned char)(spi_chip_id & SPI_Y2_DEV_ID_MASK);
		}
		else {
			man_id  = (unsigned char)(spi_chip_id & 0xff);
			dev_id  = (unsigned char)((spi_chip_id >> 8) & 0xff);
			dev_id2 = 0;

			if (pAC->spi.pSpiDev->id_byte_count != 1) {
				dev_id2 = (unsigned char)((spi_chip_id >> 16) & 0xff);
			}
		}

		if (spi_yuk2_dev_table[i].man_id == man_id &&
			spi_yuk2_dev_table[i].dev_id == dev_id &&
			(pAC->spi.yk_chip_id < CHIP_ID_YUKON_OPT ||
			 spi_yuk2_dev_table[i].dev_id2 == dev_id2)) {

			if (pAC->spi.yk_chip_id < CHIP_ID_YUKON_OPT ||
				pAC->spi.pSpiDev->id_byte_count == 1) {

				fl_print("\nFlash Device: %s (VID 0x%2.2x, DID 0x%2.2x)\n",
					spi_yuk2_dev_table[i].dev_name, man_id, dev_id);
			}
			else {
				fl_print("\nFlash Device: %s (VID 0x%2.2x, DID 0x%2.2x, "
					"DID2 0x%2.2x)\n", spi_yuk2_dev_table[i].dev_name,
					man_id, dev_id, dev_id2);
			}

			if (pAC->spi.yk_chip_id >= CHIP_ID_YUKON_OPT) {
				SK_IN32(IoC, SPI_Y2_CONTROL_REG, &spi_ctrl_reg);

				spi_ctrl_reg &= ~SPI_Y2_CLK_DIV_MASK;
				/* Set SPI clock rate divider. */
				spi_ctrl_reg |= SPI_Y2_CLK_DIV(pAC->spi.pSpiDev->clk_rate_div);
				SK_OUT32(IoC, SPI_Y2_CONTROL_REG, spi_ctrl_reg);
			}
			return;
		}
	}

	pAC->spi.pSpiDev = NULL;
	return;
}

/*****************************************************************************
 *
 * spi_yuk2_flash_erase - Erases sectors touched by address range
 *
 * Description:
 *	This function erases all sectors of the flash prom affected by
 *	the address range denoted by parameters "off" (address offset)
 *	and "len" (length of address range).
 *
 * Returns:
 *	0	Success
 *	1	Timeout
 */
static int spi_yuk2_flash_erase(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned long	off,	/* start offset in flash eprom for erase */
unsigned long	len)	/* length in flash eprom for erase */
{
	SK_SPI_DEVICE	*pSpiDev;
	unsigned long	flash_size;
	unsigned long	i;
	unsigned long	StartSector;
	unsigned long	EndSector;

	if (len == 0) {
		return (0);
	}

	pSpiDev = pAC->spi.pSpiDev;
	flash_size = pSpiDev->sector_size * pSpiDev->sector_num;

	/*
	 * If flash size is smaller than address range
	 * which should be erased, don't erase flash.
	 */
	if ((off + len - 1) > flash_size) {
		return (1);
	}

	StartSector = off / pSpiDev->sector_size;
	EndSector   = (off + len - 1) / pSpiDev->sector_size;

	/* Unprotect if necessary */
	if (spi_yuk2_write_status(pAC, IoC, 0) != 0) {
		return (1);
	}

	if (StartSector == 0 && EndSector == pSpiDev->sector_num - 1) {
		fl_print("\nAllowing for up to %d seconds\nfor chip erase to complete.\n",
			pSpiDev->chip_erase_timeout);

		/* Erase complete flash. */
		spi_yuk2_erase_chip(pAC, IoC, pSpiDev->chip_erase_timeout);
	}
	else {
		/* Erase all affected sectors. */
		for (i = StartSector; i <= EndSector; i++) {
			spi_yuk2_erase_sector(pAC, IoC, i);
		}
	}

	/* Re-protect if necessary */
	if (spi_yuk2_write_status(pAC, IoC, 0x3c) != 0) {
		return (1);
	}

	return (0);
}

/*****************************************************************************
 *
 * spi_yuk2_flash_manage - Reads, verifies, or writes SPI EPROM
 *
 * Description:
 *	This function reads, verifies, or writes the SPI EPROM.
 *
 * Returns:
 *	0	Success
 *	1	Timeout
 */
static int spi_yuk2_flash_manage(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned char	*data,	/* data buffer */
unsigned long	off,	/* start offset in flash eprom for operation */
unsigned long	len,	/* length in flash eprom */
int				flag)	/* action SPI_READ / SPI_VERIFY / SPI_WRITE */
{
	unsigned long	addr;
	unsigned long	spi_data;
	unsigned long	*spi_data_ptr;
	unsigned long	progress;
	unsigned long	last_progress;
	int				ret = 0;
	unsigned char	ByteVal;

	len = (len + 3) & ~3;

	if ((off & 3) != 0 || (len & 3) != 0) {
		return (1);
	}

	if (pAC->spi.yk_chip_id == CHIP_ID_YUKON_EX &&
		pAC->spi.yk_chip_rev != CHIP_REV_YU_EX_A0 &&
		pAC->spi.pSpiDev->sector_size * pAC->spi.pSpiDev->sector_num >=
			0x40000) {

		SK_IN8(IoC, SPI_CFG, &ByteVal);
		ByteVal |= SPI_CFG_A17_GATE;
		SK_OUT8(IoC, SPI_CFG, ByteVal);
	}

	progress = last_progress = 0;

	for (addr = off, spi_data_ptr = (unsigned long *)data;
		addr < off + len; addr += 4, spi_data_ptr++) {

		progress = ((addr - off) * 100) / len;

#ifdef MRVL_UEFI
		if ((progress - last_progress) >= 20) {
			w_print(".");
			last_progress += 20;
		}
#else	/*  MRVL_UEFI */
		if ((progress - last_progress) >= 10) {
			fl_print(".");
			last_progress += 10;
		}
#endif	/*  MRVL_UEFI */

		switch (flag) {
		case SPI_READ:
			/* Read a dword from SPI flash */
			*spi_data_ptr = spi_yuk2_read_dword(pAC, IoC, addr);
			break;

		case SPI_VERIFY:
			if (pAC->spi.yk_chip_id == CHIP_ID_YUKON_EX &&
				pAC->spi.max_faddr == 0x1000 &&
				(addr >> 12) < 0x80 &&
				pAC->spi.ModifiedPages[addr >> 12] != 1) {
				/* Do not compare sector that was skipped when writing. */
#ifdef XXX
				if ((addr % 4096) == 0) {
					fl_print("not comparing sector %d\n", addr >> 12);
				}
#endif
				continue;
			}

			/* Read and verify dword from SPI flash */
			spi_data = spi_yuk2_read_dword(pAC, IoC, addr);

			if (spi_data != *spi_data_ptr) {
				fl_print("\n*** SPI data verify error at address 0x%08lx, "
					"is 0x%08lx, should be 0x%08lx\n", addr, spi_data, *spi_data_ptr);
				ret = 1;
			}
			break;

		case SPI_WRITE:
			/*
			 * Flash-only adapters only support flash sector sizes of 4096 bytes.
			 */
			if (pAC->spi.yk_chip_id == CHIP_ID_YUKON_EX &&
				pAC->spi.max_faddr == 0x1000 &&
				(addr >> 12) < 0x80 &&
				pAC->spi.ModifiedPages[addr >> 12] != 1) {
				/*
				 * Skip sector when writing. Used for special handling
				 * of e.g. the Config Sector of flash-only adapters.
				 */
#ifdef XXX
				if ((addr % 4096) == 0) {
					fl_print("not writing to sector %d\n", addr >> 12);
				}
#endif
				continue;
			}

			/* Unprotect if necessary */
			if (spi_yuk2_write_status(pAC, IoC, 0) != 0) {
				return (1);
			}

			/* Write a dword to SPI flash */
			if (pAC->spi.pSpiDev->man_id == SPI_MAN_ID_SST) {
				ret = spi_yuk2_sst_write_dword(pAC, IoC, addr, *spi_data_ptr);
			}
			else {
				ret = spi_yuk2_write_dword(pAC, IoC, addr, *spi_data_ptr);
			}

			/* Re-protect if necessary */
			if (spi_yuk2_write_status(pAC, IoC, 0x3c) != 0) {
				return (1);
			}
			break;
		}

		if (ret != 0) {
			break;
		}
	}

	fl_print(".");
	return (ret);
}

#ifndef SK_SPI_NO_UPDATE

/*****************************************************************************
 *
 * spi_yuk2_update_config - Updates part of the config area
 *
 * Description:
 *	This function updates part of the config area.
 *
 * Returns:
 *	0	Success
 *	1	Timeout
 */
static int spi_yuk2_update_config(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned char	*data,	/* data buffer */
unsigned long	off,	/* start offset in flash eprom (config area) for operation */
unsigned long	len)	/* length of changing data */
{
	SK_SPI_DEVICE	*pSpiDev;
	unsigned char	*spibuf;
	unsigned long	start_sector;
	unsigned long	end_sector;
	unsigned long	i;

	pSpiDev = pAC->spi.pSpiDev;

	/* determine the affected sectors */
	start_sector = off / pSpiDev->sector_size;
	end_sector   = (off + len - 1) / pSpiDev->sector_size;

	/*
	 * allocate the necessary memory for temporary
	 * save of the affected sectors
	 */
	spibuf = spi_malloc((unsigned long)((end_sector - start_sector + 1) *
		pSpiDev->sector_size));

	if (spibuf == NULL) {
		return (51);
	}

	/* read out the affected sectors */
	if (spi_flash_manage(pAC, IoC, spibuf, start_sector * pSpiDev->sector_size,
		(end_sector - start_sector + 1) * pSpiDev->sector_size,
		SPI_READ)) {

		spi_free(spibuf);
		return (1);
	}

	/* update the just read out data */
	for (i = 0; i < len; i++) {
		spibuf[off + i - SPI_LSECT_OFF] = data[i];
	}

	/* erase the affected sectors */
	if (spi_flash_erase(pAC, IoC, start_sector * pSpiDev->sector_size,
		(end_sector - start_sector + 1) * pSpiDev->sector_size)) {

		spi_free(spibuf);
		return (7);
	}

	/* write the updated data back to the flash */
	if (spi_flash_manage(pAC, IoC, spibuf, start_sector * pSpiDev->sector_size,
		(end_sector - start_sector + 1) * pSpiDev->sector_size,
		SPI_WRITE)) {

		spi_free(spibuf);
		return (8);
	}

	spi_free(spibuf);
	return (0);
}

/*****************************************************************************
 *
 * spi_update_config - Updates part of the config area (Y1 and Y2)
 *
 * Description:
 *	This function updates part of the config area.
 *
 * Returns:
 *	0	Success
 *	1	Timeout
 */
static int spi_update_config(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned char	*data,	/* data buffer */
unsigned long	off,	/* start offset in flash eprom (config area) for operation */
unsigned long	len)	/* length of changing data */
{
	switch (pAC->spi.fl_type) {
	case FT_SPI:
		return (spi_yuk_update_config(pAC, IoC, data, off, len));
	case FT_SPI_Y2:
		return (spi_yuk2_update_config(pAC, IoC, data, off, len));
	}

	return (1);
}

#endif	/* !SK_SPI_NO_UPDATE */

/*****************************************************************************
 *
 * flash_check_spi - Determines whether an SPI EPROM is present
 *
 * Description:
 *	This function determines whether an SPI EPROM is present.
 *
 * Returns:
 *	0	No SPI
 *	1	SPI detected
 */
int flash_check_spi(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned long	*FlashSize)
{
	unsigned long	a1,a2;
	unsigned long	opcodes;
	int				i;

	*FlashSize = 0;

	SK_IN8(IoC, B2_CHIP_ID, &pAC->spi.yk_chip_id);
	SK_IN8(IoC, B2_MAC_CFG, &pAC->spi.yk_chip_rev);

	pAC->spi.yk_chip_rev = (pAC->spi.yk_chip_rev & CFG_CHIP_R_MSK) >> 4;

	if (pAC->spi.yk_chip_id == CHIP_ID_YUKON_LITE) {

		/* YUKON-Lite Rev. A1 */
		pAC->spi.fl_type = FT_SPI;
		*FlashSize = pAC->spi.max_pages * pAC->spi.max_faddr;
		fl_print("\nFlash Device\t: SPI\n");
		return (1);
	}

	if (pAC->spi.yk_chip_id >= CHIP_ID_YUKON_XL &&
		pAC->spi.yk_chip_id <= CHIP_ID_YUKON_OP_2) {

		/* WA for A17 problem */
		if (pAC->spi.yk_chip_id == CHIP_ID_YUKON_SUPR &&
			pAC->spi.yk_chip_rev == CHIP_REV_YU_SU_A0) {

			SK_OUT16(IoC, SPI_CFG, 0x1c1);
		}

		spi_yuk2_set_dev_ptr(pAC, IoC);
		if (pAC->spi.pSpiDev == NULL) {
			/* unknown or no flash */
			fl_print("\nFlash device\t: none\n");
			pAC->spi.fl_type = FT_SPI_UNKNOWN;
			return (0);
		}

		pAC->spi.fl_type = FT_SPI_Y2;
		pAC->spi.max_pages = (long)pAC->spi.pSpiDev->sector_num;
		pAC->spi.max_faddr = (long)pAC->spi.pSpiDev->sector_size;
		*FlashSize = pAC->spi.max_pages * pAC->spi.max_faddr;

		/* 
		 * NOTE:
		 * SPI flash types of max. 1 MB (8 MBit) are currently supported. 
		 * Thus restrict flash size and sector/page number for bigger
		 * SPI flash devices.
		 */
		if (*FlashSize > MAX_SPI_FLASH_SIZE) {
			pAC->spi.max_pages = MAX_SPI_FLASH_SIZE / pAC->spi.max_faddr;
			*FlashSize = pAC->spi.max_pages * pAC->spi.max_faddr;
		}

		/*
		 * For flash-only adapters: Do not overwrite
		 * Config Sector (array is modified by MT-Tool).
		 */
		for (i = 0; i < 0x80; i++) {
			pAC->spi.ModifiedPages[i] = 1;
		}

		if (pAC->spi.yk_chip_id == CHIP_ID_YUKON_EX &&
			pAC->spi.max_faddr == 0x1000) {
			pAC->spi.ModifiedPages[SPI_PIG_OFF >> 12] = 0;
		}

		/* set the opcodes for the SPI flash found */
		SK_IN32(IoC, SPI_Y2_OPCODE_REG1, &opcodes);
		opcodes &= 0x000000ffL;
		opcodes |=
			(unsigned long)pAC->spi.pSpiDev->opcodes.op_read << 8 |
			(unsigned long)pAC->spi.pSpiDev->opcodes.op_read_id << 16 |
			(unsigned long)pAC->spi.pSpiDev->opcodes.op_read_status << 24;
		SK_OUT32(IoC, SPI_Y2_OPCODE_REG1, opcodes);

		opcodes =
			(unsigned long)pAC->spi.pSpiDev->opcodes.op_write_enable |
			(unsigned long)pAC->spi.pSpiDev->opcodes.op_write << 8 |
			(unsigned long)pAC->spi.pSpiDev->opcodes.op_sector_erase << 16 |
			(unsigned long)pAC->spi.pSpiDev->opcodes.op_chip_erase << 24;
		SK_OUT32(IoC, SPI_Y2_OPCODE_REG2, opcodes);

		if (pAC->spi.yk_chip_id >= CHIP_ID_YUKON_OPT) {
			/* set the opcodes for the SPI flash found */
			SK_IN32(IoC, SPI_Y2_OPCODE_REG0, &opcodes);
			opcodes &= 0x0000ffffL;
			opcodes |=
				(unsigned long)pAC->spi.pSpiDev->opcodes.op_write_status << 16 |
				(unsigned long)pAC->spi.pSpiDev->opcodes.op_read_sect_protect << 24;
			SK_OUT32(IoC, SPI_Y2_OPCODE_REG0, opcodes);
		}

		pAC->spi.needs_unprotect = SK_FALSE;

		if (pAC->spi.pSpiDev->man_id == SPI_MAN_ID_SST) {
			/*
			 * workaround for SST flash types to clear write
			 * protection bits which are set by default
			 */
			spi_yuk2_sst_clear_write_protection(pAC, IoC);
		}
		else if (pAC->spi.pSpiDev->man_id == SPI_MAN_ID_ATMEL &&
				 pAC->spi.pSpiDev->dev_id >= 0x43 &&
				 pAC->spi.pSpiDev->dev_id <= 0x47) {

#ifndef XXX
			/* unprotect before / re-protect after erase and write */
			pAC->spi.needs_unprotect = SK_TRUE;
#else	/* XXX */
			/* unprotect here for complete runtime */
			if (spi_yuk2_write_status(pAC, IoC, 0) != 0) {
				return (0);
			}
#endif	/* XXX */
		}

		return (1);
	}

	/* Save Eprom Address Register 1 */
	SK_IN32(IoC, SPI_ADR_REG1, &a1);

	/* Write SPI pattern */
	SK_OUT32(IoC, SPI_ADR_REG1, SPI_PATTERN);

	/* Read SPI pattern */
	SK_IN32(IoC, SPI_ADR_REG1, &a2);

	/* Restore Eprom Address Register 1 */
	SK_OUT32(IoC, SPI_ADR_REG1, a1);

	/* This is an SPI Eprom if one of bits 31..16 are set */
	if ((a2 & SPI_COMP_MASK) != 0) {
		/* YUKON-Lite Rev. A0 */
		pAC->spi.fl_type = FT_SPI;
		*FlashSize = pAC->spi.max_pages * pAC->spi.max_faddr;
		return (1);
	}
	return (0);
}

/*****************************************************************************
 *
 * spi_flash_erase - Erases sectors touched by address range (Y1 and Y2)
 *
 * Description:
 *	This function erases all sectors of the flash prom affected by
 *	the address range denoted by parameters "off" (address offset)
 *	and "len" (length of address range).
 *
 * Returns:
 *	0	Success
 *	1	Timeout
 */
int spi_flash_erase(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned long	off,	/* start offset in flash eprom for erase */
unsigned long	len)	/* length in flash eprom for erase */
{
	switch (pAC->spi.fl_type) {
	case FT_SPI:
		return (spi_yuk_flash_erase(pAC, IoC, off, len));
	case FT_SPI_Y2:
		return (spi_yuk2_flash_erase(pAC, IoC, off, len));
	}

	return (1);
}

/*****************************************************************************
 *
 * spi_flash_manage - Reads, verifies, or writes SPI EPROM (Y1 and Y2)
 *
 * Description:
 *	This function reads, verifies, or writes the SPI EPROM.
 *
 * Returns:
 *	0	Success
 *	1	Timeout
 */
int spi_flash_manage(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned char	*data,	/* data buffer */
unsigned long	off,	/* start offset in flash eprom for operation */
unsigned long	len,	/* length in flash eprom */
int				flag)	/* action SPI_READ / SPI_VERIFY / SPI_WRITE */
{
	switch (pAC->spi.fl_type) {
	case FT_SPI:
		return (spi_yuk_flash_manage(pAC, IoC, data, off, len, flag));
	case FT_SPI_Y2:
		return (spi_yuk2_flash_manage(pAC, IoC, data, off, len, flag));
	}

	return (1);
}

/*****************************************************************************
 *
 * spi_vpd_transfer - Reads or updates data in VPD area of SPI EPROM
 *
 * Description:
 *	This function reads or updates data in the VPD area of the SPI EPROM.
 *
 * Returns:
 *	0	Success
 *	1	Timeout
 */
int spi_vpd_transfer(
SK_AC	*pAC,
SK_IOC	IoC,
char	*buf,	/* data buffer */
int		addr,	/* VPD start address */
int		len,	/* number of bytes to read / to write */
int		dir)	/* transfer direction may be VPD_READ or VPD_WRITE */
{
	if (dir == 0) {
		return (spi_flash_manage(pAC, IoC, (SK_U8*)buf, SPI_VPD_OFF + addr,
			len, SPI_READ));
	}
#ifndef SK_SPI_NO_UPDATE
	return (spi_update_config(pAC, IoC, (SK_U8*)buf, SPI_VPD_OFF + addr, len));
#else	/* SK_SPI_NO_UPDATE */
	return (1);
#endif	/* SK_SPI_NO_UPDATE */
}

/*****************************************************************************
 *
 * spi_get_pig - Gets data from PiG area in SPI EPROM
 *
 * Description:
 *	This function gets data from the PiG area in the SPI EPROM.
 *
 * Returns:
 *	0	Success
 *	1	Timeout
 */
int spi_get_pig(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned char	*data,	/* data buffer */
unsigned long	len)	/* length of data wanted */
{
	return (spi_flash_manage(pAC, IoC, data, SPI_PIG_OFF, len, SPI_READ));
}

/*****************************************************************************
 *
 * spi_get_noc - Gets data from VPD area in SPI EPROM
 *
 * Description:
 *	This function gets data from the VPD area in the SPI EPROM.
 *
 * Returns:
 *	0	Success
 *	1	Timeout
 */
int spi_get_noc(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned char	*data,	/* data buffer */
unsigned long	len)	/* length of data wanted */
{
	return (spi_flash_manage(pAC, IoC, data, SPI_NOC_OFF, len, SPI_READ));
}

/*****************************************************************************
 *
 * spi_get_pet - Gets data from PET area in SPI EPROM
 *
 * Description:
 *	This function gets data from the PET area in the SPI EPROM.
 *
 * Returns:
 *	0	Success
 *	1	Timeout
 */
int spi_get_pet(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned char	*data,	/* data buffer */
unsigned long	len)	/* length of data wanted */
{
	return (spi_flash_manage(pAC, IoC, data, SPI_PET_OFF, len, SPI_READ));
}

#ifndef SK_SPI_NO_UPDATE

/*****************************************************************************
 *
 * spi_update_pig - Updates data in PiG area in SPI EPROM
 *
 * Description:
 *	This function updates data in the PiG area in the SPI EPROM.
 *
 * Returns:
 *	0	Success
 *	1	Timeout
 */
int spi_update_pig(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned char	*data,	/* data buffer */
unsigned long	len)	/* length of data to update */
{
	return (spi_update_config(pAC, IoC, data, SPI_PIG_OFF, len));
}

/*****************************************************************************
 *
 * spi_update_noc - Updates data in NOC area in SPI EPROM
 *
 * Description:
 *	This function updates data in the NOC area in the SPI EPROM.
 *
 * Returns:
 *	0	Success
 *	1	Timeout
 */
int spi_update_noc(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned char	*data,	/* data buffer */
unsigned long	len)	/* length of data to update */
{
	return (spi_update_config(pAC, IoC, data, SPI_NOC_OFF, len));
}

/*****************************************************************************
 *
 * spi_update_pet - Updates data in PET area in SPI EPROM
 *
 * Description:
 *	This function updates data in the PET area in the SPI EPROM.
 *
 * Returns:
 *	0	Success
 *	1	Timeout
 */
int spi_update_pet(
SK_AC			*pAC,
SK_IOC			IoC,
unsigned char	*data,	/* data buffer */
unsigned long	len)	/* length of data to update */
{
	return (spi_update_config(pAC, IoC, data, SPI_PET_OFF, len));
}

#endif	/* !SK_SPI_NO_UPDATE */

/* End of File */
