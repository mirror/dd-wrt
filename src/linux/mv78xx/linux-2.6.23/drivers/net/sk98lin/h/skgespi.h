/******************************************************************************
 *
 * Name:	skspi.h
 * Project:	Flash Programmer, Manufacturing and Diagnostic Tools
 * Version:	$Revision: 1.1.2.3 $
 * Date:	$Date: 2007/06/27 15:54:44 $
 * Purpose:	Contains SPI-Flash EEPROM specific definitions and constants
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

#define	__INC_SKSPI_H 

/* SPI registers */
// CHIP_IDs should be defined in skgehw.h
#ifndef B2_CHIP_ID
#define B2_CHIP_ID			0x011b	/* Chip Identification Number */
#endif

#ifndef CHIP_ID_GENESIS
#define CHIP_ID_GENESIS		0x0a	/* Chip ID for GENESIS */
#endif

#ifndef CHIP_ID_YUKON
#define CHIP_ID_YUKON		0xb0	/* Chip ID for YUKON */
#endif

#ifndef CHIP_ID_YUKON_LITE
#define CHIP_ID_YUKON_LITE	0xb1	/* Chip ID for YUKON-Lite (Rev. A1) */
#endif

#ifndef CHIP_ID_YUKON_LP
#define CHIP_ID_YUKON_LP	0xb2	/* Chip ID for YUKON-LP */
#endif

#ifndef CHIP_ID_YUKON_XL
#define CHIP_ID_YUKON_XL	0xb3	/* Chip ID for YUKON-2 XL */
#endif

#ifndef CHIP_ID_YUKON_EC_U
#define CHIP_ID_YUKON_EC_U      0xb4	/* Chip ID for YUKON-EC Ultra */
#endif

#ifndef CHIP_ID_YUKON_EX
#define CHIP_ID_YUKON_EX	0xb5	/* Chip ID for YUKON-2 Extreme */
#endif

#ifndef CHIP_ID_YUKON_EC
#define CHIP_ID_YUKON_EC	0xb6	/* Chip ID for YUKON-2 EC */
#endif

#define SPI_ADR_REG1	0x0120		/* VPD low  addr, SPI loader start addr */
#define SPI_ADR_REG2	0x0124		/* VPD high addr, PiG loader start addr */
#define SPI_CTRL_REG	0x0128		/* SPI control & status register */

#define B2_TST_REG1		0x0158		/* Test control register */

/* SPI commands and constants */

#define SPI_PATTERN		0xffffffffL	/* Write value for SPI identification */
#define SPI_COMP_MASK	0xfffe0000L	/* Compare Mask for SPI identification */

#define SPI_VPD_MIN		0x0001f800L	/* Min Eprom addr for access via VPD port */
#define SPI_VPD_MAX		0xfffff000L /* Max Eprom addr for access via VPD port */

#define SPI_LSECT_OFF	0x18000L	/* Offset of last sector in SPI eprom */
#define SPI_CONF_OFF	0x1c000L	/* Offset of config space in SPI eprom */

#define SPI_PIG_OFF		0x1f000L	/* Plug-In-Go (PiG) Config space */
#define SPI_NOC_OFF		0x1f800L	/* Normal Oper. Config (NOC) space */
#define SPI_VPD_OFF		0x1c000L	/* Vital Product Data (VPD) space */
#define SPI_PET_OFF		0x1d000L	/* Pet Frames space */

#define SPI_CHIP_SIZE	0x20000L	/* Size of whole SPI eprom */
#define SPI_SECT_SIZE	0x8000L		/* Size of a sector in SPI eprom */

#define SPI_CONF_SIZE	0x4000L		/* Size of config area in SPI eprom */
#define SPI_PIG_SIZE	0x0800L		/* Size of PiG area in SPI eprom */
#define SPI_NOC_SIZE	0x0800L		/* Size of NOC area in SPI eprom */
#define SPI_VPD_SIZE	0x0100L		/* Size of VPD area in SPI eprom */
#define SPI_PET_SIZE	0x2000L		/* Size of PET area in SPI eprom */

#define SPI_SECT_ERASE	0x00008000L	/* Sector erase command */
#define SPI_CHIP_ERASE	0x00001000L /* Chip erase command */
#define SPI_VPD_MAP		0x00080000L /* VPD to Eprom mapping flag */
#define SPI_TIMER_SET	5			/* SPI timeout value (sec.) */
#define SPI_TIMEOUT		0			/* Timeout check flag */
#define SPI_READ		1			/* Read flag for spi_flash_manage() */
#define SPI_VERIFY		2			/* Verify flag for spi_flash_manage() */
#define SPI_WRITE		3			/* Write flag for spi_flash_manage() */

/* VPD regs from PCI config reg. file mapped to control reg. file */

#define VPD_ADR_REG		0x03d2		/* VPD address register in config file */
#define VPD_DATA_PORT	0x03d4		/* VPD data port in configuration file */
#define VPD_FLAG_MASK	0x8000		/* VPD read-write flag */

#define	FT_SPI_UNKNOWN	(-1)
#define FT_SPI			3			/* Flash type */
#define FT_SPI_Y2		4			/* Yukon 2/EC SPI flash */

/********************************************************************************
 * Yukon 2/EC definitions and macros
 ********************************************************************************/

/* SPI EPROM CONTROL REGISTER */
#define SPI_Y2_CONTROL_REG				0x60
/* SPI EPROM ADDRESS REGISTER */
#define SPI_Y2_ADDRESS_REG				0x64
/* SPI EPROM DATA REGISTER */
#define SPI_Y2_DATA_REG					0x68
/* SPI EPROM VENDOR-/DEVICE-ID REGISTER */
#define SPI_Y2_VENDOR_DEVICE_ID_REG		0x6c
/* SPI EPROM FIRST OPCODE REGISTER */
#define SPI_Y2_OPCODE_REG1				0x78
/* SPI EPROM SECOND OPCODE REGISTER */
#define SPI_Y2_OPCODE_REG2				0x7c

/* SPI EPROM READ INSTRUCTION */
#define SPI_Y2_RD						(0x09L<<16)
/* SPI EPROM READ ID INSTRUCTION */
#define SPI_Y2_RDID						(0x0AL<<16)
/* SPI EPROM READ STATUS REGISTER INSTRUCTION */
#define SPI_Y2_RDST						(0x0BL<<16)
/* SPI EPROM WRITE ENABLE INSTRUCTION */
#define SPI_Y2_WEN						(0x0CL<<16)
/* SPI EPROM WRITE INSTRUCTION */
#define SPI_Y2_WR						(0x0DL<<16)
/* SPI EPROM SECTOR ERASE INSTRUCTION */
#define SPI_Y2_SERS						(0x0EL<<16)
/* SPI EPROM CHIP ERASE INSTRUCTION */
#define SPI_Y2_CERS						(0x0FL<<16)
/* SPI EPROM command mask  */
#define SPI_Y2_CMD_MASK					(0x07L<<16)

/* SPI flash read ID protocol */
#define SPI_Y2_RDID_PROT				(0x01L<<28)

/* SPI flash VPD mapping enable */
#define SPI_Y2_VPD_ENABLE				(0x01L<<29)

/* SPI EPROM BUSY CHECK */
#define SPI_Y2_IS_BUSY(w)		((w)&(1L<<30))
#define SPI_Y2_IS_BUSY_WR(w)	((w)&(1))

#define SPI_Y2_MAN_ID_MASK				0xff00
#define SPI_Y2_DEV_ID_MASK				0x00ff

/* SPI flash manufacturer ID's */
#define SPI_MAN_ID_ATMEL				0x1f
#define SPI_MAN_ID_SST					0xbf
#define SPI_MAN_ID_ST_M25P20			0x11
#define SPI_MAN_ID_ST_M25P10			0x10

/* wait for SPI EPROM to finish write/erase operation */
#define SPI_Y2_WAIT_SE_FINISH_WR() {														\
	unsigned long stat=1;																	\
	SPI_Y2_WAIT_SE_FINISH_CMD();															\
	/* wait for write to finish or timeout */												\
	spi_timer(SPI_TIMER_SET);																\
	while( SPI_Y2_IS_BUSY_WR(stat) ){														\
		if (spi_timer(SPI_TIMEOUT)) {														\
			break;																			\
		}																					\
		spi_out32(SPI_Y2_CONTROL_REG, SPI_Y2_RDST);											\
		SPI_Y2_WAIT_SE_FINISH_CMD() 														\
		spi_in32(SPI_Y2_CONTROL_REG, &stat);												\
	}																						\
}

/* wait for SPI EPROM to finish command */
#define SPI_Y2_WAIT_SE_FINISH_CMD() {														\
	unsigned long stat=(1L<<30);																\
	/* wait for command to finish */														\
	spi_timer(SPI_TIMER_SET);																\
	while( SPI_Y2_IS_BUSY(stat) ){															\
		if (spi_timer(SPI_TIMEOUT)) {														\
			break;																			\
		}																					\
		spi_in32(SPI_Y2_CONTROL_REG, &stat);												\
	}																						\
}

#if (defined Core || defined DJGPP || !defined MSDOS)
#define huge
#endif /* Core || DJGPP || !MSDOS */

/* function prototypes */

int flash_check_spi( unsigned long *FlashSize );

int spi_flash_erase(
	unsigned long off,
	unsigned long len);

int spi_flash_manage(
	unsigned char  *data,
	unsigned long off,
	unsigned long len,
	int flag);

int spi_vpd_transfer(
	char	*buf,	
	int	addr,	
	int	len,	
	int	dir);	

int spi_get_pig(
	unsigned char  *data,
	unsigned long len);

int spi_get_noc(
	unsigned char  *data,
	unsigned long len);

int spi_update_pig(
	unsigned char  *data,
	unsigned long len);

int spi_update_noc(
	unsigned char  *data,
	unsigned long len);

int spi_update_pet(
	unsigned char  *data,
	unsigned long len);

void spi_yuk2_write_enable(void);
void spi_yuk2_sst_clear_write_protection(void);
void spi_yuk2_erase_chip(void);
unsigned short spi_yuk2_read_chip_id(void);
int spi_yuk2_get_dev_index(void);

