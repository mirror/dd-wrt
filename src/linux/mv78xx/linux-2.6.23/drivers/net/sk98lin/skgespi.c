/******************************************************************************
 *
 * Name:	skspi.c
 * Project:	Flash Programmer, Manufacturing and Diagnostic Tools
 * Version:	$Revision: 1.1.2.4 $
 * Date:	$Date: 2007/06/28 09:28:04 $
 * Purpose:	Contains SPI-Flash EEPROM specific functions
 *
 ******************************************************************************/

/******************************************************************************
 *
 *	(C)Copyright 1998-2002 SysKonnect
 *	(C)Copyright 2002-2003 Marvell
 *
 *	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF SYSKONNECT
 *	The copyright notice above does not evidence any
 *	actual or intended publication of such source code.
 *
 *	This Module contains Proprietary Information of SysKonnect
 *	and should be treated as Confidential.
 *
 *	The information in this file is provided for the exclusive use of
 *	the licensees of SysKonnect.
 *	Such users have the right to use, modify, and incorporate this code
 *	into products for purposes authorized by the license agreement
 *	provided they include this notice and the associated copyright notice
 *	with any such product.
 *	The information in this file is provided "AS IS" without warranty.
 *
 ******************************************************************************/

static const char SysKonnectFileId[] =
	"@(#) $Id: skgespi.c,v 1.1.2.4 2007/06/28 09:28:04 marcusr Exp $ (C) Marvell.";

/*
#include <stdio.h>
#include <malloc.h>
#include <assert.h>
*/

#include "h/skdrv1st.h"
#include "h/skdrv2nd.h"
#include "h/skgespi.h"

#ifdef Core
	#include "h/skdrv1st.h"
	#include "h/skdrv2nd.h"
	#define fl_print pAC->Gui.WPrint
#else
	extern void fl_print(char *msg, ...);	
#endif /* Core */

extern int fl_type;
extern long max_pages;
extern long max_faddr;

/* low level SPI programming external interface */

extern void spi_in8(unsigned short, unsigned char *);
extern void spi_in16(unsigned short, unsigned short *);
extern void spi_in32(unsigned short, unsigned long *);
extern void spi_out8(unsigned short, unsigned char);
extern void spi_out16(unsigned short, unsigned short);
extern void spi_out32(unsigned short, unsigned long);
extern int  spi_timer(unsigned int);
extern void *spi_malloc(unsigned long);
extern void spi_free(void *);

/* global variables */
unsigned char chip_id;

/* local variables */
static int spi_yuk2_dev = -1;

/*
 * Yukon 2/EC SPI flash device structure/table
 */
static struct {
	char			*man_name;
	unsigned char	man_id;
	char			*dev_name;
	unsigned char	dev_id;
	unsigned long	sector_size;
	unsigned long	sector_num;
	char			set_protocol;

	struct {
		unsigned char	op_read;
		unsigned char	op_write;
		unsigned char	op_write_enable;
		unsigned char	op_write_disable;
		unsigned char	op_read_status;
		unsigned char	op_read_id;
		unsigned char	op_sector_erase;
		unsigned char	op_chip_erase;
	} opcodes;
} spi_yuk2_dev_table[] = {
	{ "Atmel",	SPI_MAN_ID_ATMEL,		"AT25F2048",	0x63,	0x10000,	4,		0,	{0x03,0x02,0x06,0x04,0x05,0x15,0x52,0x62}	} ,
	{ "Atmel",	SPI_MAN_ID_ATMEL,		"AT25F1024",	0x60,	0x8000,		4,		0,	{0x03,0x02,0x06,0x04,0x05,0x15,0x52,0x62}	} ,
	{ "SST",	SPI_MAN_ID_SST,			"SST25VF512",	0x48,	0x1000,		16,		1,	{0x03,0x02,0x06,0x04,0x05,0x90,0x20,0x60}	} ,
	{ "SST",	SPI_MAN_ID_SST,			"SST25VF010",	0x49,	0x1000,		32,		1,	{0x03,0x02,0x06,0x04,0x05,0x90,0x20,0x60}	} ,
	{ "SST",	SPI_MAN_ID_SST,			"SST25VF020",	0x43,	0x1000,		64,		1,	{0x03,0x02,0x06,0x04,0x05,0x90,0x20,0x60}	} ,
	{ "SST",	SPI_MAN_ID_SST,			"SST25VF040",	0x44,	0x1000,		128,	1,	{0x03,0x02,0x06,0x04,0x05,0x90,0x20,0x60}	} ,
	{ "ST",		SPI_MAN_ID_ST_M25P20,	"M25P20",		0x11,	0x10000,	4,		1,	{0x03,0x02,0x06,0x04,0x05,0xab,0xd8,0xc7}	} ,
	{ "ST",		SPI_MAN_ID_ST_M25P10,	"M25P10",		0x10,	0x8000,		4,		1,	{0x03,0x02,0x06,0x04,0x05,0xab,0xd8,0xc7}	}
};

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	int     spi_yuk_flash_erase(unsigned long off,unsigned long len)
 *
 *	Send chip erase command to SPI Eprom
 *
 * Uses:	spi_in32, spi_out32, spi_in8, spi_timer
 *
 * IN:
 *	off	start offset in flash eprom for erase
 *      len	length in flash eprom for erase
 *		(all nessesory sectors will be erased)
 *
 * return:
 *	0	Success
 *	1	Timeout
 *
 * END_MANUAL_ENTRY()
 */
static int spi_yuk_flash_erase(
unsigned long off,
unsigned long len)
{
	char cr;
	unsigned long creg, i;
	/* Save SPI control register */
	spi_in32(SPI_CTRL_REG, &creg);

	if ((off+len-1) > 0x1ffffL) {
		return(1);	/* more than 4 segments for erase */
	}

#ifdef XXX
    /* there is some problem with chip erase. dr */

    /* use faster chip erase command if all sectors should be erased */
	if(len > SPI_SECT_SIZE*3L) {

		spi_out32(SPI_CTRL_REG, SPI_CHIP_ERASE);
		
		spi_timer(SPI_TIMER_SET);

		do {
			/* Read device status */				
			spi_in8(SPI_CTRL_REG, &cr);
			
			if (spi_timer(SPI_TIMEOUT)) {
				fl_print("\nSPI chip erase timeout: %d sec\n", SPI_TIMER_SET);
				return(1);
			}
		}
		while (cr & 1);          /* is chip busy ? */
		/* Restore SPI control register */
		spi_out32(SPI_CTRL_REG, creg);
	
		return(0);
	}
#endif

	for (i = (off >> 15); i <= ((off + len - 1) >> 15); i++) {
		/* Clear chip command */
		spi_out32(SPI_CTRL_REG, (i << 22) | SPI_SECT_ERASE);
		
		spi_timer(SPI_TIMER_SET);

		do {
			/* Read device status */				
			spi_in8(SPI_CTRL_REG, &cr);
			
			if (spi_timer(SPI_TIMEOUT)) {
				fl_print("\nSPI chip erase timeout: %d sec\n", SPI_TIMER_SET);
				return(1);
			}
		}
		while (cr & 1);          /* is chip busy ? */
	}
	/* Restore SPI control register */
	spi_out32(SPI_CTRL_REG, creg);
	
	return(0);
}

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	int     spi_yuk_flash_manage(
 *	unsigned char  *data,
 *	unsigned long off,
 *	unsigned long len,
 *	int flag)
 *
 *	Read, Verify or Write SPI Eprom
 *
 * Uses:	spi_in32, spi_out32, spi_in8, spi_out8, spi_in16, spi_out16, spi_timer
 *
 * IN:
 *	data	data buffer
 *	off	start offset in flash eprom for operation
 *      len	length in flash eprom
 *	flag	SPI_READ
 *		SPI_VERIFY
 *		SPI_WRITE
 *
 * return:
 *	0	Success
 *	1	Timeout
 *
 * END_MANUAL_ENTRY()
 */
static unsigned short rol2(
unsigned short adr)
{
	unsigned short	adr2;
	
	/* YUKON-Lite Rev. A1 */
	if (chip_id == CHIP_ID_YUKON_LITE || chip_id == CHIP_ID_YUKON_LP) {
		
		/* switch address bits [1:0] with [14:13] */
		adr2 = adr & 0x6000;
		adr2 = adr2 >> 13;
		adr2 = adr2 & 0x0003;
		adr  = adr << 2;
		adr  = adr & 0x7ffc;
		adr  = adr | adr2;
	}
	
	return(adr);
}

static int spi_yuk_flash_manage(
unsigned char  *data,
unsigned long off,
unsigned long len,
int flag)
{
	unsigned long a1, a2, cr, i;
	unsigned short adr, as, ret=0;
	unsigned char ch, tr;
	unsigned long progress;
	unsigned long last_progress;

	/* Save VPD lower address */
	spi_in32(SPI_ADR_REG1, &a1);

	/* Save VPD higher address */
	spi_in32(SPI_ADR_REG2, &a2);

	/* Set VPD lower address to 0 (15 higher bit)*/
	spi_out32(SPI_ADR_REG1, SPI_VPD_MIN);

	/* Set VPD higher address to 0x7fff (15 higher bit)*/
	spi_out32(SPI_ADR_REG2, SPI_VPD_MAX);

	/* Save SPI control register */
	spi_in32(SPI_CTRL_REG, &cr);

	/* Enable VPD to SPI mapping (set bit 19) */
	spi_out32(SPI_CTRL_REG, SPI_VPD_MAP);

	/* Save Test Control Register 1 */
	spi_in8(B2_TST_REG1, &tr);

	/* Enable write to mapped PCI config register file */
	spi_out8(B2_TST_REG1, 2);

	progress = last_progress = 0;

	for (i = off, adr = (unsigned short)(off >> 2); i < off + len; i++) {		
		progress = (i * 100) / len;

		if((progress - last_progress) >= 10) {
			fl_print(".");
			last_progress += 10;
		}

		/* Set new address to VPD Address reg, initiate reading */
		if ((i % 4) == 0) {
		
			spi_out16(VPD_ADR_REG, rol2(adr++));
			
			/* Wait for termination */
			spi_timer(SPI_TIMER_SET);
			do {
				/* Read VPD_ADDR reg for flag check */	
				spi_in16(VPD_ADR_REG, &as);
				
				if (spi_timer(SPI_TIMEOUT)) {
					fl_print("\nSPI read timeout: %d sec. Offset:0x%05lx\n",
						SPI_TIMER_SET, i);
					ret = 1;
					break;
				}
			}
			while (!(as & VPD_FLAG_MASK)); /* check  flag */
			
			if (ret) {
				break;
			}
		}
		
		switch (flag) {
		case SPI_READ:
			/* Read byte from VPD port */
			spi_in8((unsigned short)(VPD_DATA_PORT +
				(unsigned short)(i % 4)), &ch);
			*(data++) = ch;
			break;
		case SPI_VERIFY:
			/* Read and verify byte from VPD port */
			spi_in8((unsigned short)(VPD_DATA_PORT +
				(unsigned short)(i % 4)), &ch);
			if (ch != *(data++)) {
				fl_print("\n*** SPI data verify error at address 0x%05lx, ",
					"is %x, should %x\n",
					i, ch, *(data - 1));
				ret = 1;
			}
			break;
		case SPI_WRITE:
			/* Write byte to VPD port */
			spi_out8((unsigned short)(VPD_DATA_PORT +
				(unsigned short)(i % 4)), *(data++));
	
			if ((i % 4) == 3) {
				/* Set old Address to VPD_ADDR reg, initiate writing */
				spi_out16((unsigned short)VPD_ADR_REG,
					(unsigned short)(rol2((unsigned short)(adr - 1)) | VPD_FLAG_MASK));
		
				/* Wait for termination */
				spi_timer(SPI_TIMER_SET);
				do {
					/* Read VPD_ADDR reg for flag check*/	
					spi_in16(VPD_ADR_REG, &as);
					
					if (spi_timer(SPI_TIMEOUT)) {
						fl_print("\nSPI write timeout: %d sec. Offset:0x%05lx\n",
							SPI_TIMER_SET, i);
						ret = 1;
						break;
					}
				}
				while (as & VPD_FLAG_MASK); /* check  flag */
			}
			break;
		}

		if (ret) {
			break;	
		}
	}
	/* Restore Test Control Register 1 */
	spi_out8(B2_TST_REG1, tr);

	/* Restore VPD lower address*/
	spi_out32(SPI_ADR_REG1, a1);

	/* Restore VPD higher address*/
	spi_out32(SPI_ADR_REG2, a2);

	/* Restore SPI control register */
	spi_out32(SPI_CTRL_REG, cr);

	fl_print(".");
	return(ret);	
}

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	int     spi_yuk_update_config(
 *	unsigned char  *data,
 *	unsigned long off,
 *	unsigned long len)
 *
 *	Update part of config area
 *
 * Uses:	spi_flash_manage, spi_flash_erase
 *
 * IN:
 *	data	data buffer
 *	off	start offset in flash eprom (config area) for operation
 *      len	length of changing data
 *
 * return:
 *	0	Success
 *	1	Timeout
 *
 * END_MANUAL_ENTRY()
 */
static int spi_yuk_update_config(
unsigned char  *data,
unsigned long off,
unsigned long len)
{
	unsigned char *spibuf;
	unsigned long i;

	spibuf = spi_malloc((unsigned long)SPI_SECT_SIZE);

	if(spibuf == NULL) {
		return(51);
	}
	
	if (spi_flash_manage(spibuf, SPI_LSECT_OFF, SPI_SECT_SIZE, SPI_READ)) {
	
		spi_free(spibuf);
		return(1);
	}
	
	for (i = 0; i < len; i++) {
		spibuf[off + i - SPI_LSECT_OFF] = data[i];
	}
	
	if (spi_flash_erase(SPI_LSECT_OFF, SPI_SECT_SIZE)) {

		spi_free(spibuf);
		return(7);
	}
	
	if (spi_flash_manage(spibuf, SPI_LSECT_OFF, SPI_SECT_SIZE, SPI_WRITE)) {
	
		spi_free(spibuf);
		return(8);
	}
	spi_free(spibuf);	
	return(0);
}	

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	void spi_yuk2_write_enable()
 *
 *	This function will set the Write enable setting of the SPI EPROM
 *
 * Uses:
 *
 * IN:
 *
 * return:
 *	nothing
 *
 * END_MANUAL_ENTRY()
 */
void spi_yuk2_write_enable()
{
	unsigned long spi_ctrl_reg;
	
	/* execute write enable command */
	spi_in32(SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
	spi_ctrl_reg &= ~SPI_Y2_CMD_MASK;
	spi_ctrl_reg |= SPI_Y2_WEN;	
	spi_out32(SPI_Y2_CONTROL_REG, spi_ctrl_reg);
	
	/* wait for the SPI to finish command */
	SPI_Y2_WAIT_SE_FINISH_WR();
}

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	static unsigned long spi_yuk2_read_dword(
 *  unsigned long	address)
 *
 *	The function reads a dword from the specified address in the SPI EPROM.
 *
 * Uses:
 *
 * IN:
 *
 * return:
 *	dword read from given address
 *
 * END_MANUAL_ENTRY()
 */
static unsigned long spi_yuk2_read_dword(
unsigned long	address)		/* addres in the SPI EPROM to read from */
{
	unsigned long reg_val = 0;
	unsigned long spi_ctrl_reg;

	/* write spi address */
	spi_out32(SPI_Y2_ADDRESS_REG, address);

	/* execute spi read command */
	spi_in32(SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
	spi_ctrl_reg &= ~SPI_Y2_CMD_MASK;
	spi_ctrl_reg |= SPI_Y2_RD;	
	spi_out32(SPI_Y2_CONTROL_REG, spi_ctrl_reg);

	/* wait for the SPI to finish RD operation */
	SPI_Y2_WAIT_SE_FINISH_CMD();
   
	/* read the returned data */
	spi_in32(SPI_Y2_DATA_REG, &reg_val);

	return reg_val;
}

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	static int spi_yuk2_write_dword(
 *	unsigned long	address,
 *	unsigned long	value)
 *
 *	This function writes the specified dword in the specified SPI EPROM location.
 *  The SPI EPROM should have been put previously in write enable mode and
 *  the target sector erased for the access to succeed.
 *
 * Uses:
 *
 * IN:
 *
 * return:
 *	0 - if successful
 *	1 - if fails
 *
 * END_MANUAL_ENTRY()
 */
static int spi_yuk2_write_dword(
unsigned long	address,			/* address to write to */
unsigned long	value)				/* new value to be written */
{
	unsigned long spi_ctrl_reg;
	unsigned long verify_value;

	spi_yuk2_write_enable();
	
	/* write spi address */
	spi_out32(SPI_Y2_ADDRESS_REG, address);   
	/* write the new value */
	spi_out32(SPI_Y2_DATA_REG, value);

	/* execute the spi write command */
	spi_in32(SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
	spi_ctrl_reg &= ~SPI_Y2_CMD_MASK;
	spi_ctrl_reg |= SPI_Y2_WR;	
	spi_out32(SPI_Y2_CONTROL_REG, spi_ctrl_reg);
	
	/* wait for write to finish */
	SPI_Y2_WAIT_SE_FINISH_WR();
	
	verify_value = spi_yuk2_read_dword(address);

	/* verify if write was successful*/
	if ( verify_value == value ) {
		return 0;
	} 
	else {
		fl_print("\n*** SPI data write error at address 0x%08x, ","is 0x%08x, should 0x%08x\n", address, 
																	verify_value, value);
		return 1;
	}
}

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	static int spi_yuk2_sst_write_dword(
 *	unsigned long	address,
 *	unsigned long	value)
 *
 *	This function writes the specified dword in the specified SPI EPROM location.
 *  The SPI EPROM should have been put previously in write enable mode and
 *  the target sector erased for the access to succeed.
 *
 * Uses:
 *
 * IN:
 *
 * return:
 *	0 - if successful
 *	1 - if fails
 *
 * END_MANUAL_ENTRY()
 */
static int spi_yuk2_sst_write_dword(
unsigned long	address,			/* address to write to */
unsigned long	value)				/* new value to be written */
{
	unsigned long spi_ctrl_reg;
	unsigned long verify_value;
	unsigned long i;

	for (i = 0; i < 4; i++) {

		spi_yuk2_write_enable();

		/* write spi address */
		spi_out32(SPI_Y2_ADDRESS_REG, address + i);   
		/* write the new value */
		spi_out32(SPI_Y2_DATA_REG, (value >> i*8) & 0x000000ff);

		/* execute the spi write command */
		spi_in32(SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
		spi_ctrl_reg &= ~SPI_Y2_CMD_MASK;
		spi_ctrl_reg |= SPI_Y2_WR;	
		spi_out32(SPI_Y2_CONTROL_REG, spi_ctrl_reg);
		
		/* wait for write to finish */
		SPI_Y2_WAIT_SE_FINISH_WR();
		
		verify_value = spi_yuk2_read_dword(address);

		/* verify if write was successful*/
		if ( ((verify_value >> i*8) & 0x000000ff) != ((value >> i*8) & 0x000000ff) ) {
			fl_print("\n*** SPI data write error at address 0x%08x, ","is 0x%08x, should 0x%08x\n", address + i, 
																		((verify_value >> i*8) & 0x000000ff), ((value >> i*8) & 0x000000ff));
			return 1;
		}
	}

	return 0;
}

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	int spi_yuk2_sst_clear_write_protection()
 *
 *	Clear the write protection bits for SST flash types because
 *	they are set by default.
 *
 * Uses:
 *
 * IN:
 *
 * return:
 *	nothing
 *
 * END_MANUAL_ENTRY()
 */
void spi_yuk2_sst_clear_write_protection()
{
	unsigned long spi_ctrl_reg;
	unsigned long op;
	unsigned long addr;
	unsigned char status;

	spi_in32(SPI_Y2_OPCODE_REG1, &op);
	op &= 0xffff00ff;
	op |= 0x00000100;
	spi_out32(SPI_Y2_OPCODE_REG1, op);

	spi_in32(SPI_Y2_OPCODE_REG2, &op);
	op &= 0xffffff00;
	op |= 0x00000050;
	spi_out32(SPI_Y2_OPCODE_REG2, op);

	/* execute write status register enable command */
	spi_in32(SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
	spi_ctrl_reg &= ~SPI_Y2_CMD_MASK;
	spi_ctrl_reg |= SPI_Y2_WEN;	
	spi_out32(SPI_Y2_CONTROL_REG, spi_ctrl_reg);
	
	/* wait for the SPI to finish RD operation */
	SPI_Y2_WAIT_SE_FINISH_CMD();

	spi_in32(SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
	status = (unsigned char)(spi_ctrl_reg & 0xfffffff3);
	addr = ((unsigned long)status | (unsigned long)(status << 8) | (unsigned long)(status << 16) | (unsigned long)(status << 24));
        
	/* write spi address */
	spi_out32(SPI_Y2_ADDRESS_REG, addr);   
	/* write the new value */
	spi_out32(SPI_Y2_DATA_REG, addr);

	/* execute the write status register command */
	spi_in32(SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
	spi_ctrl_reg &= ~SPI_Y2_CMD_MASK;
	spi_ctrl_reg |= SPI_Y2_RD;
	spi_out32(SPI_Y2_CONTROL_REG, spi_ctrl_reg);

        /* wait for the SPI to finish RD operation */
	SPI_Y2_WAIT_SE_FINISH_CMD();
			
	spi_in32(SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
}

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	static void spi_yuk2_erase_sector(
 *	unsigned long	sector_num)
 *
 *	This function will erase the sector specified allowing thus data to be written
 *
 * Uses:
 *
 * IN:
 *
 * return:
 *	nothing
 *
 * END_MANUAL_ENTRY()
 */
static void spi_yuk2_erase_sector(
unsigned long	sector_num)		/* sector to be erased */
{
	unsigned long spi_ctrl_reg;

	spi_yuk2_write_enable();

	/* write sector start address */
	spi_out32(SPI_Y2_ADDRESS_REG, spi_yuk2_dev_table[spi_yuk2_dev].sector_size * sector_num);

	/* execute erase sector command */
	spi_in32(SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
	spi_ctrl_reg &= ~SPI_Y2_CMD_MASK;
	spi_ctrl_reg |= SPI_Y2_SERS;		
	spi_out32(SPI_Y2_CONTROL_REG, spi_ctrl_reg);
	SPI_Y2_WAIT_SE_FINISH_WR();
}

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	void spi_yuk2_erase_chip()
 *
 *	This function will erase the complete SPI EPROM chip
 *
 * Uses:
 *
 * IN:
 *
 * return:
 *	nothing
 *
 * END_MANUAL_ENTRY()
 */
void spi_yuk2_erase_chip()
{
	unsigned long spi_ctrl_reg;

	spi_yuk2_write_enable();

	/* execute erase chip command */
	spi_in32(SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
	spi_ctrl_reg &= ~SPI_Y2_CMD_MASK;
	spi_ctrl_reg |= SPI_Y2_CERS;		
	spi_out32(SPI_Y2_CONTROL_REG, spi_ctrl_reg);
	SPI_Y2_WAIT_SE_FINISH_WR();
}

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	unsigned short spi_yuk2_read_chip_id()
 *
 *	Read SPI vendor and device id
 *
 * Uses:
 *
 * IN:
 *
 * return:
 *	The lower byte containes the device id and the upper the vendor id
 *
 * END_MANUAL_ENTRY()
 */
unsigned short spi_yuk2_read_chip_id()
{
	unsigned short	chip_id_local;
	unsigned long	spi_ctrl_reg = 0;
	unsigned long	opcode_reg1;

	/* 
	 * set read id opcode for flash currently selected
	 */
	spi_in32(SPI_Y2_OPCODE_REG1, &opcode_reg1);
	opcode_reg1 &= 0xff00ffffL;
	opcode_reg1 |= (((unsigned long)spi_yuk2_dev_table[spi_yuk2_dev].opcodes.op_read_id) << 16);
	spi_out32(SPI_Y2_OPCODE_REG1, opcode_reg1);

	/*
	 * read spi control register
	 */
	spi_in32(SPI_Y2_CONTROL_REG, &spi_ctrl_reg);
	spi_ctrl_reg &= ~(SPI_Y2_CMD_MASK | SPI_Y2_RDID_PROT);

	/* select protocol for chip id read out */
	if( spi_yuk2_dev_table[spi_yuk2_dev].set_protocol ) {
		spi_ctrl_reg |= SPI_Y2_RDID_PROT;
	}
	
	/* execute read chip id command */
	spi_ctrl_reg |= SPI_Y2_RDID;
	spi_out32(SPI_Y2_CONTROL_REG, spi_ctrl_reg);
	SPI_Y2_WAIT_SE_FINISH_CMD();

	spi_in16(SPI_Y2_VENDOR_DEVICE_ID_REG, &chip_id_local);
	return (chip_id_local);
}

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	int spi_yuk2_get_dev_index(void)
 *
 *	Identify the SPI device and return its device
 *	table index
 *
 * Uses:
 *
 * IN:
 *
 * return:
 *	-1	unknown or no flash device
 *	>=0	flash device table index
 *
 * END_MANUAL_ENTRY()
 */
#ifdef USE_ASF_DASH_FW
int spi_yuk2_get_dev_index()
{
	unsigned short	spi_chip_id;
	unsigned char	man_id;
	unsigned char	dev_id;
	unsigned int	i;

	/* search for flash in device table */
	for(i = 0; i < (sizeof(spi_yuk2_dev_table) / sizeof(spi_yuk2_dev_table[0])); i++) {
		if (chip_id != CHIP_ID_YUKON_EC_U && chip_id != CHIP_ID_YUKON_EX &&
			(i == 0 || i == 1)) {
			continue;
		}
		spi_yuk2_dev = i;

		spi_chip_id = spi_yuk2_read_chip_id();

		man_id = (unsigned char)((spi_chip_id & SPI_Y2_MAN_ID_MASK) >> 8);
		dev_id = (unsigned char)(spi_chip_id & SPI_Y2_DEV_ID_MASK);

		if(spi_yuk2_dev_table[i].man_id == man_id &&
		   spi_yuk2_dev_table[i].dev_id == dev_id) {
			fl_print("\nFlash Device\t: %s ", spi_yuk2_dev_table[i].dev_name);
			fl_print("(VID 0x%2.2x, ", man_id);
			fl_print("DID 0x%2.2x)\n", dev_id);
			return(i);
		}
	}

	spi_yuk2_dev = -1;
	return(-1);
}
#else
int spi_yuk2_get_dev_index()
{
	unsigned short	chip_id_local;
	unsigned char	man_id;
	unsigned char	dev_id;
	int		i;

	/* search for flash in device table */
	for(i = 0; i < (sizeof(spi_yuk2_dev_table) / sizeof(spi_yuk2_dev_table[0])); i++) {
      spi_yuk2_dev = i;
		chip_id_local = spi_yuk2_read_chip_id();

		man_id = (unsigned char)((chip_id_local & SPI_Y2_MAN_ID_MASK) >> 8);
		dev_id = (unsigned char)(chip_id_local & SPI_Y2_DEV_ID_MASK);

		if(spi_yuk2_dev_table[i].man_id == man_id &&
		   spi_yuk2_dev_table[i].dev_id == dev_id) {
			fl_print("\nFlash Device %s found\n", spi_yuk2_dev_table[i].dev_name);
			fl_print("  - Vendor ID : 0x%2.2x \n", man_id);
			fl_print("  - Device ID : 0x%2.2x \n", dev_id);
			return(i);
		}
	}

	spi_yuk2_dev = -1;
	return(-1);
}
#endif

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	static int spi_yuk2_flash_erase(unsigned long off,unsigned long len)
 *
 *	Erase all sectors of the flash prom affected by the address
 *	range denoted by parameters "off" (address offset) and "len"
 *	(length of address range).
 *
 * Uses:	spi_in32, spi_out32, spi_in8, spi_timer
 *
 * IN:
 *	off:	start offset in flash eprom for erase
 *  len:	length in flash eprom for erase
 *			(all necessary sectors will be erased)
 *
 * return:
 *	0	Success
 *	1	Timeout
 *
 * END_MANUAL_ENTRY()
 */

static int spi_yuk2_flash_erase(
unsigned long off,
unsigned long len)
{
	unsigned long	flash_size;
	unsigned long	i;
	unsigned long	start_sector;
	unsigned long	end_sector;

	if(len == 0) {
		return(0);
	}

	flash_size = spi_yuk2_dev_table[spi_yuk2_dev].sector_size * 
							spi_yuk2_dev_table[spi_yuk2_dev].sector_num;

	/*
	 * flash size is smaller than address range which
	 * should be erased --> don't erase flash.
	 */
	if ((off+len-1) > flash_size) {
		return(1);
	}

	/*
	 * Erase complete flash if all sectors of flash are affected
	 */
	if(len > (spi_yuk2_dev_table[spi_yuk2_dev].sector_size * 
		(spi_yuk2_dev_table[spi_yuk2_dev].sector_num - 1))) {
		
		spi_yuk2_erase_chip();
	}
	/*
	 * Erase all affected sectors
	 */
	else {
		start_sector = off / spi_yuk2_dev_table[spi_yuk2_dev].sector_size;
		end_sector   = (off + len - 1) / spi_yuk2_dev_table[spi_yuk2_dev].sector_size;
		
		for (i = start_sector; i <= end_sector; i++) {
			spi_yuk2_erase_sector(i);
		}
	}

	return(0);
}

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	static int spi_yuk2_flash_manage(
 *	unsigned char  *data,
 *	unsigned long off,
 *	unsigned long len,
 *	int flag)
 *
 *	Read, Verify or Write SPI Eprom
 *
 * Uses:	spi_in32, spi_out32, spi_in8, spi_out8, spi_in16, spi_out16, spi_timer
 *
 * IN:
 *	data	data buffer
 *	off		start offset in flash eprom for operation
 *  len		length in flash eprom
 *	flag	SPI_READ
 *			SPI_VERIFY
 *			SPI_WRITE
 *
 * return:
 *	0	Success
 *	1	Timeout
 *
 * END_MANUAL_ENTRY()
 */
#ifdef USE_ASF_DASH_FW
static int spi_yuk2_flash_manage(
unsigned char huge *data,
unsigned long off,
unsigned long len,
int flag)
{
	unsigned long		addr; 
	unsigned long		spi_data;
	unsigned long huge	*spi_data_ptr;
	unsigned long		progress;
	unsigned long		last_progress;
	int					ret = 0;
	unsigned char		ChipId,ChipRev,ByteVal;

	// assert(!(off&3));

	len = (len + 3) & ~3;

	spi_in8(B2_CHIP_ID, &ChipId);
	spi_in8(B2_MAC_CFG, &ChipRev);
	ChipRev = (unsigned char)((ChipRev & CFG_CHIP_R_MSK) >> 4);
	
	if ((ChipId == CHIP_ID_YUKON_EX) &&
		(ChipRev != CHIP_REV_YU_EX_A0) &&
		(spi_yuk2_dev_table[spi_yuk2_dev].sector_size *
		spi_yuk2_dev_table[spi_yuk2_dev].sector_num >= 0x40000)) {

		spi_in8(SPI_CFG, &ByteVal);
		ByteVal |= SPI_CFG_A17_GATE;	
		spi_out8(SPI_CFG, ByteVal);
	}

	progress = last_progress = 0;

	for (addr = off, spi_data_ptr = (unsigned long huge *)data;
							addr < off + len; addr+=4, spi_data_ptr++) {
		progress = ((addr - off) * 100) / len;

		if((progress - last_progress) >= 10) {
			fl_print(".");
			last_progress += 10;
		}

		switch (flag) {
			case SPI_READ:
				/* Read a dword from SPI flash */
				*(spi_data_ptr) = spi_yuk2_read_dword(addr);				
			break;

			case SPI_VERIFY:
				/* Read and verify dword from SPI flash */
				spi_data = spi_yuk2_read_dword(addr);


				if (spi_data != *(spi_data_ptr)) {
					fl_print("\n*** SPI data verify error at address 0x%08lx, ",
						"is %x, should %x\n", addr, spi_data, *(spi_data_ptr));
					ret = 1;
				}
			break;

			case SPI_WRITE:
#if 0
#ifndef SK_DIAG
				/*
				 * Flash-only adapters only support
				 * Flash sector sizes of 4096 bytes
				 */
				if (max_faddr == 0x1000) {
					if (ModifiedPages[addr >> 12] != (unsigned char)1) {
#if 0

						if ((addr % 4096) == 0) {
							fl_print("not writing to sector %d\n",addr >> 12);
						}
#endif
						continue;
					}
				}
#endif	/* !SK_DIAG */
#endif

					/* Write a dword to SPI flash */
				if (spi_yuk2_dev_table[spi_yuk2_dev].man_id == SPI_MAN_ID_SST) {
					ret = spi_yuk2_sst_write_dword(addr, *(spi_data_ptr));
				}
				else {
					ret = spi_yuk2_write_dword(addr, *(spi_data_ptr));
				}

			break;
		}

		if (ret) {
			break;	
		}
	}

	fl_print(".");
	return(ret);
}
#else
static int spi_yuk2_flash_manage(
unsigned char  *data,
unsigned long off,
unsigned long len,
int flag)
{
	unsigned long		addr; 
	unsigned long		spi_data;
	unsigned int		*spi_data_ptr;
	int			ret = 0;
	len = (len + 3) & ~3;

	for (addr = off, spi_data_ptr = (unsigned int *)data; addr < off + len; addr+=4, spi_data_ptr++) {

		switch (flag) {
			case SPI_READ:
				/* Read a dword from SPI flash */
				*(spi_data_ptr) = spi_yuk2_read_dword(addr);
			break;

			case SPI_VERIFY:
				/* Read and verify dword from SPI flash */
				spi_data = spi_yuk2_read_dword(addr);
			break;

				if (spi_data != *(spi_data_ptr)) {
					fl_print("\n*** SPI data verify error at address 0x%08lx, ","is %x, should %x\n", addr, 
																				spi_data, *(spi_data_ptr));
					ret = 1;
				}
			break;

			case SPI_WRITE:
				/* Write a dword to SPI flash */
				if (spi_yuk2_dev_table[spi_yuk2_dev].man_id == SPI_MAN_ID_SST) {
					ret = spi_yuk2_sst_write_dword(addr, *(spi_data_ptr));
				}
				else {
					ret = spi_yuk2_write_dword(addr, *(spi_data_ptr));
				}
			break;
		}

		if (ret) {
			break;	
		}
	}

	return(ret);
}
#endif

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	static int spi_yuk2_update_config(
 *	unsigned char  *data,
 *	unsigned long off,
 *	unsigned long len)
 *
 *	Update part of config area
 *
 * Uses:	spi_flash_manage, spi_flash_erase
 *
 * IN:
 *	data	data buffer
 *	off	start offset in flash eprom (config area) for operation
 *      len	length of changing data
 *
 * return:
 *	0	Success
 *	1	Timeout
 *
 * END_MANUAL_ENTRY()
 */
static int spi_yuk2_update_config(
unsigned char  *data,
unsigned long off,
unsigned long len)
{
	unsigned char  *spibuf;
	unsigned long start_sector;
	unsigned long end_sector;
	unsigned long i;

	/* determine the affected sectors */
	start_sector = off / spi_yuk2_dev_table[spi_yuk2_dev].sector_size;
	end_sector   = (off + len - 1) / spi_yuk2_dev_table[spi_yuk2_dev].sector_size;
	
	/* 
	 * allocate the necessary memory for temporary 
	 * save of the affected sectors
	 */
	spibuf = spi_malloc((unsigned long)(((end_sector - start_sector) + 1) * spi_yuk2_dev_table[spi_yuk2_dev].sector_size));

	if(spibuf == NULL) {
		return(51);
	}
	
	/* read out the affected sectors */
	if (spi_flash_manage(spibuf, 
							start_sector * spi_yuk2_dev_table[spi_yuk2_dev].sector_size, 
							((end_sector - start_sector) + 1) * spi_yuk2_dev_table[spi_yuk2_dev].sector_size, 
							SPI_READ)) {	

		spi_free(spibuf);
		return(1);
	}
	
	/* update the just read out data */
	for (i = 0; i < len; i++) {
		spibuf[off + i - SPI_LSECT_OFF] = data[i];
	}
	
	/* erase the affected sectors */
	if (spi_flash_erase(start_sector * spi_yuk2_dev_table[spi_yuk2_dev].sector_size, 
						((end_sector - start_sector) + 1) * spi_yuk2_dev_table[spi_yuk2_dev].sector_size)) {

		spi_free(spibuf);
		return(7);
	}
	
	/* write the updated data back to the flash */
	if (spi_flash_manage(spibuf, 
						 start_sector * spi_yuk2_dev_table[spi_yuk2_dev].sector_size,  
						 ((end_sector - start_sector) + 1) * spi_yuk2_dev_table[spi_yuk2_dev].sector_size, 
						 SPI_WRITE)) {
	
		spi_free(spibuf);
		return(8);
	}

	spi_free(spibuf);	
	return(0);
}

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	static int spi_update_config(
 *	unsigned char  *data,
 *	unsigned long off,
 *	unsigned long len)
 *
 *	Update part of config area
 *
 * Uses:	spi_flash_manage, spi_flash_erase
 *
 * IN:
 *	data	data buffer
 *	off	start offset in flash eprom (config area) for operation
 *      len	length of changing data
 *
 * return:
 *	0	Success
 *	1	Timeout
 *
 * END_MANUAL_ENTRY()
 */
static int spi_update_config(
unsigned char  *data,
unsigned long off,
unsigned long len)
{
	switch (fl_type) {
		case FT_SPI:
			return(spi_yuk_update_config(data, off, len));
		break;

		case FT_SPI_Y2:
			return(spi_yuk2_update_config(data, off, len));
		break;
	}

	return(1);
}

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	int     flash_check_spi( unsigned long *FlashSize )
 *
 *	Determines whether a SPI Eprom present
 *
 * Uses:	spi_in32, spi_out32
 *
 * IN:
 *
 * return:
 *	0	No SPI
 *	1	SPI detected
 *
 * END_MANUAL_ENTRY()
 */

#ifdef USE_ASF_DASH_FW
int flash_check_spi( 
unsigned long *FlashSize )
{
	unsigned long a1,a2;
	unsigned long opcodes;

	
	*FlashSize = 0;

	spi_in8(B2_CHIP_ID, &chip_id);

	if (chip_id == CHIP_ID_YUKON_LITE || chip_id == CHIP_ID_YUKON_LP) {
		/* YUKON-Lite Rev. A1 */
		fl_type = FT_SPI;
		*FlashSize = max_pages * max_faddr;
		fl_print("\nFlash Device\t: SPI\n");
		return(1);
	}

	if (chip_id >= CHIP_ID_YUKON_XL && chip_id <= CHIP_ID_YUKON_FE) {		
		fl_type = FT_SPI_Y2;

		spi_yuk2_dev = spi_yuk2_get_dev_index();

		/* unknown or no flash */
		if(spi_yuk2_dev < 0) {
			fl_print("\nFlash device\t: none\n");
			fl_type = FT_SPI_UNKNOWN;
			return(0);
		}
		
		max_pages = (long)(spi_yuk2_dev_table[spi_yuk2_dev].sector_num);
		max_faddr = (long)(spi_yuk2_dev_table[spi_yuk2_dev].sector_size);
		*FlashSize = spi_yuk2_dev_table[spi_yuk2_dev].sector_num * spi_yuk2_dev_table[spi_yuk2_dev].sector_size;

		/*
		 * work around for SST flash types to clear write 
		 * protection bits which are set by default.
		 */
		if (spi_yuk2_dev_table[spi_yuk2_dev].man_id == SPI_MAN_ID_SST) {
			spi_yuk2_sst_clear_write_protection();
		}

		/*
		 * set the opcodes for the SPI flash found 
		 */
		spi_in32(SPI_Y2_OPCODE_REG1, &opcodes);
		opcodes &= 0x000000ffL;
		opcodes |= ((((unsigned long)(spi_yuk2_dev_table[spi_yuk2_dev].opcodes.op_read)) << 8) | 
					(((unsigned long)(spi_yuk2_dev_table[spi_yuk2_dev].opcodes.op_read_id)) << 16) |
					(((unsigned long)(spi_yuk2_dev_table[spi_yuk2_dev].opcodes.op_read_status)) << 24));

		spi_out32(SPI_Y2_OPCODE_REG1, opcodes);

		opcodes = (((unsigned long)(spi_yuk2_dev_table[spi_yuk2_dev].opcodes.op_write_enable)) | 
				   (((unsigned long)(spi_yuk2_dev_table[spi_yuk2_dev].opcodes.op_write)) << 8) | 
				   (((unsigned long)(spi_yuk2_dev_table[spi_yuk2_dev].opcodes.op_sector_erase)) << 16) |
				   (((unsigned long)(spi_yuk2_dev_table[spi_yuk2_dev].opcodes.op_chip_erase)) << 24));

		spi_out32(SPI_Y2_OPCODE_REG2, opcodes);

		return(1);
	}	

	/* Save Eprom Address Register 1 */
	spi_in32(SPI_ADR_REG1, &a1);
	
	/* Write SPI pattern */
	spi_out32(SPI_ADR_REG1, SPI_PATTERN);
	
	/* Read SPI pattern */
	spi_in32(SPI_ADR_REG1, &a2);
	
	/* Restore Eprom Address Register 1 */
	spi_out32(SPI_ADR_REG1, a1);
	
	/* This is a SPI Eprom if one of bits 31..16 are set */
	if ((a2 & SPI_COMP_MASK) != 0) {
		/* YUKON-Lite Rev. A0 */
		fl_type = FT_SPI;
		*FlashSize = max_pages * max_faddr;
		return(1);
	}
	return(0);
}
#else
int flash_check_spi( 
unsigned long *FlashSize )
{
	unsigned long a1,a2;
	unsigned long opcodes;
	
	*FlashSize = 0;

	spi_in8(B2_CHIP_ID, &chip_id);

	if (chip_id == CHIP_ID_YUKON_LITE || chip_id == CHIP_ID_YUKON_LP) {
		/* YUKON-Lite Rev. A1 */
		fl_type = FT_SPI;
		*FlashSize = max_pages * max_faddr;
		return(1);
	}

	if (chip_id == CHIP_ID_YUKON_XL || chip_id == CHIP_ID_YUKON_EC) {		
		fl_type = FT_SPI_Y2;
		spi_yuk2_dev = spi_yuk2_get_dev_index();

		/* unknown or no flash */
		if(spi_yuk2_dev < 0) {
			fl_print("\nNo SPI flash found !\n");
			fl_type = FT_SPI_UNKNOWN;
			return(0);
		}
		
		max_pages = (long)(spi_yuk2_dev_table[spi_yuk2_dev].sector_num);
		max_faddr = (long)(spi_yuk2_dev_table[spi_yuk2_dev].sector_size);
		*FlashSize = spi_yuk2_dev_table[spi_yuk2_dev].sector_num * spi_yuk2_dev_table[spi_yuk2_dev].sector_size;

		/*
		 * work around for SST flash types to clear write 
		 * protection bits which are set by default.
		 */
		if (spi_yuk2_dev_table[spi_yuk2_dev].man_id == SPI_MAN_ID_SST) {
			spi_yuk2_sst_clear_write_protection();
		}

		/*
		 * set the opcodes for the SPI flash found 
		 */
		spi_in32(SPI_Y2_OPCODE_REG1, &opcodes);
		opcodes &= 0x000000ffL;
		opcodes |= ((((unsigned long)(spi_yuk2_dev_table[spi_yuk2_dev].opcodes.op_read)) << 8) | 
					(((unsigned long)(spi_yuk2_dev_table[spi_yuk2_dev].opcodes.op_read_id)) << 16) |
					(((unsigned long)(spi_yuk2_dev_table[spi_yuk2_dev].opcodes.op_read_status)) << 24));

		spi_out32(SPI_Y2_OPCODE_REG1, opcodes);

		opcodes = (((unsigned long)(spi_yuk2_dev_table[spi_yuk2_dev].opcodes.op_write_enable)) | 
				   (((unsigned long)(spi_yuk2_dev_table[spi_yuk2_dev].opcodes.op_write)) << 8) | 
				   (((unsigned long)(spi_yuk2_dev_table[spi_yuk2_dev].opcodes.op_sector_erase)) << 16) |
				   (((unsigned long)(spi_yuk2_dev_table[spi_yuk2_dev].opcodes.op_chip_erase)) << 24));

		spi_out32(SPI_Y2_OPCODE_REG2, opcodes);

		return(1);
	}	

	/* Save Eprom Address Register 1 */
	spi_in32(SPI_ADR_REG1, &a1);
	
	/* Write SPI pattern */
	spi_out32(SPI_ADR_REG1, SPI_PATTERN);
	
	/* Read SPI pattern */
	spi_in32(SPI_ADR_REG1, &a2);
	
	/* Restore Eprom Address Register 1 */
	spi_out32(SPI_ADR_REG1, a1);
	
	/* This is a SPI Eprom if one of bits 31..16 are set */
	if ((a2 & SPI_COMP_MASK) != 0) {
		/* YUKON-Lite Rev. A0 */
		fl_type = FT_SPI;
		*FlashSize = max_pages * max_faddr;
		return(1);
	}
	return(0);
}
#endif

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	int     spi_flash_erase(unsigned long off,unsigned long len)
 *
 *	Send chip erase command to SPI Eprom
 *
 * Uses:	spi_in32, spi_out32, spi_in8, spi_timer
 *
 * IN:
 *	off	start offset in flash eprom for erase
 *      len	length in flash eprom for erase
 *		(all nessesory sectors will be erased)
 *
 * return:
 *	0	Success
 *	1	Timeout
 *
 * END_MANUAL_ENTRY()
 */
int spi_flash_erase(
unsigned long off,
unsigned long len)
{
	switch (fl_type) {
		case FT_SPI:
			return(spi_yuk_flash_erase(off, len));
		break;

		case FT_SPI_Y2:
			return(spi_yuk2_flash_erase(off, len));
		break;
	}

	return(1);
}

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	int     spi_flash_manage(
 *	unsigned char  *data,
 *	unsigned long off,
 *	unsigned long len,
 *	int flag)
 *
 *	Read, Verify or Write SPI Eprom
 *
 * Uses:	spi_in32, spi_out32, spi_in8, spi_out8, spi_in16, spi_out16, spi_timer
 *
 * IN:
 *	data	data buffer
 *	off		start offset in flash eprom for operation
 *  len		length in flash eprom
 *	flag	SPI_READ
 *			SPI_VERIFY
 *			SPI_WRITE
 *
 * return:
 *	0	Success
 *	1	Timeout
 *
 * END_MANUAL_ENTRY()
 */
int spi_flash_manage(
unsigned char  *data,
unsigned long off,
unsigned long len,
int flag)
{
	switch (fl_type) {
		case FT_SPI:
			return(spi_yuk_flash_manage(data, off, len, flag));
		break;

		case FT_SPI_Y2:
			return(spi_yuk2_flash_manage(data, off, len, flag));
		break;
	}

	return(1);
}

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	int     spi_get_pig(
 *	unsigned char  *data,
 *	unsigned long len)
 *
 *	Get data from PiG area in SPI eprom
 *
 * Uses: spi_update_config
 *
 * IN:
 *	data	data buffer
 *      len	length of wanted data
 *
 * return:
 *	0	Success
 *	1	Timeout
 *
 * END_MANUAL_ENTRY()
 */
int spi_get_pig(
unsigned char  *data,
unsigned long len)
{
	return(spi_flash_manage(data, SPI_PIG_OFF, len, SPI_READ));
}	

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	int     spi_get_noc(
 *	unsigned char  *data,
 *	unsigned long len)
 *
 *	Get data from VPD area in SPI eprom
 *
 * Uses: spi_update_config
 *
 * IN:
 *	data	data buffer
 *      len	length of wanted data
 *
 * return:
 *	0	Success
 *	1	Timeout
 *
 * END_MANUAL_ENTRY()
 */
int spi_get_noc(
unsigned char  *data,
unsigned long len)
{
	return(spi_flash_manage(data, SPI_NOC_OFF, len, SPI_READ));
}	

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	int     spi_update_pig(
 *	unsigned char  *data,
 *	unsigned long len)
 *
 *	Update data in PiG area in SPI eprom
 *
 * Uses: spi_update_config
 *
 * IN:
 *	data	data buffer
 *      len	length of data to update
 *
 * return:
 *	0	Success
 *	1	Timeout
 *
 * END_MANUAL_ENTRY()
 */
int spi_update_pig(
unsigned char  *data,
unsigned long len)
{
	return(spi_update_config(data, SPI_PIG_OFF, len));
}	

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	int     spi_update_noc(
 *	unsigned char  *data,
 *	unsigned long len)
 *
 *	Update data in NOC area in SPI eprom
 *
 * Uses: spi_update_config
 *
 * IN:
 *	data	data buffer
 *      len	length of data to update
 *
 * return:
 *	0	Success
 *	1	Timeout
 *
 * END_MANUAL_ENTRY()
 */
int spi_update_noc(
unsigned char  *data,
unsigned long len)
{
	return(spi_update_config(data, SPI_NOC_OFF, len));
}	

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	int     spi_vpd_tranfer(
 *		char	*buf,	
 *		int	addr,	
 *		int	len,	
 *		int	dir)	
 *
 *	Read or update data in VPD area in SPI eprom
 *
 * Uses: spi_update_config
 *
 * IN:
 *	buf		data buffer
 *	addr		VPD start address
 *	len		number of bytes to read / to write
 *	dir		transfer direction may be VPD_READ or VPD_WRITE
 *
 * return:
 *	0	Success
 *	1	Timeout
 *
 * END_MANUAL_ENTRY()
 */
int spi_vpd_transfer(
char	*buf,	/* data buffer */
int		addr,	/* VPD start address */
int		len,	/* number of bytes to read / to write */
int		dir)	/* transfer direction may be VPD_READ or VPD_WRITE */
{
	if (dir == 0) {
		return(spi_flash_manage(buf, SPI_VPD_OFF + addr, len, SPI_READ));
	}
	else {
		return(spi_update_config(buf, SPI_VPD_OFF + addr, len));
	}	
}

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	int     spi_update_pet(
 *	unsigned char  *data,
 *	unsigned long len)
 *
 *	Update data in PET area in SPI eprom
 *
 * Uses: spi_update_config
 *
 * IN:
 *	data	data buffer
 *      len	length of data to update
 *
 * return:
 *	0	Success
 *	1	Timeout
 *
 * END_MANUAL_ENTRY()
 */
int spi_update_pet(
unsigned char  *data,
unsigned long len)
{
	return(spi_update_config(data, SPI_PET_OFF, len));
}	

/*
 * BEGIN_MANUAL_ENTRY()
 *
 *	int     spi_get_pet(
 *	unsigned char  *data,
 *	unsigned long len)
 *
 *	Get data from Pet frames area in SPI eprom
 *
 * Uses: spi_update_config
 *
 * IN:
 *	data	data buffer
 *      len	length of wanted data
 *
 * return:
 *	0	Success
 *	1	Timeout
 *
 * END_MANUAL_ENTRY()
 */
int spi_get_pet(
unsigned char  *data,
unsigned long len)
{
	return(spi_flash_manage(data, SPI_PET_OFF, len, SPI_READ));
}	
