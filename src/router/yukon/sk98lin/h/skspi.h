/******************************************************************************
 *
 * Name:	$Id: //Release/Yukon_1G/Shared/flashfun/V2/h/skspi.h#6 $
 * Project:	Flash Programmer, Manufacturing and Diagnostic Tools
 * Version:	$Revision: #6 $, $Change: 4280 $
 * Date:	$DateTime: 2010/11/05 11:55:33 $
 * Purpose:	Contains SPI-Flash EEPROM specific definitions and constants
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

#ifndef	__INC_SKSPI_H
#define	__INC_SKSPI_H

#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */

/* defines ******************************************************************/

#define SPI_ADR_REG1	B2_FAR		/* VPD low  addr, SPI loader start addr */
#define SPI_ADR_REG2	B2_FDP		/* VPD high addr, PiG loader start addr */
#define SPI_CTRL_REG	B2_LD_CTRL	/* SPI control & status register */

#define B2_TST_REG1		B2_TST_CTRL1/* Test control register */

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
#define SPI_TIMER_SET	4			/* SPI timeout value (sec.) */
#define SPI_TIMEOUT		0			/* Timeout check flag */
#define SPI_READ		1			/* Read flag for spi_flash_manage() */
#define SPI_VERIFY		2			/* Verify flag for spi_flash_manage() */
#define SPI_WRITE		3			/* Write flag for spi_flash_manage() */

/* VPD regs from PCI config reg. file mapped to control reg. file */

#define VPD_ADR_REG		(B7_CFG_SPC | PCI_VPD_ADR_REG)	/* VPD address register in config file */
#define VPD_DATA_PORT	(B7_CFG_SPC | PCI_VPD_DAT_REG)	/* VPD data port in configuration file */

#define VPD_FLAG_MASK	0x8000		/* VPD read-write flag */

#define	FT_SPI_UNKNOWN	(-1)
#define FT_SPI			3			/* Flash type */
#define FT_SPI_Y2		4			/* Yukon 2/EC SPI flash */

/********************************************************************************
 * Yukon 2/EC definitions and macros
 ********************************************************************************/

/* SPI EPROM control register */
#define SPI_Y2_CONTROL_REG			SPI_CTRL
/* SPI EPROM address register */
#define SPI_Y2_ADDRESS_REG			SPI_ADDR
/* SPI EPROM data register */
#define SPI_Y2_DATA_REG				SPI_DATA
/* SPI EPROM vendor-/device-id register */
#define SPI_Y2_VENDOR_DEVICE_ID_REG	SPI_ID
/* SPI EPROM first opcode register */
#define SPI_Y2_OPCODE_REG0			SPI_CFG
/* SPI EPROM second opcode register */
#define SPI_Y2_OPCODE_REG1			SPI_OPC1
/* SPI EPROM third opcode register */
#define SPI_Y2_OPCODE_REG2			SPI_OPC2

/* SPI EPROM Fast_Read_CYC_EN bit */
#define SPI_Y2_FAST_READ_EN			BIT_8

/* SPI EPROM instruction extension bit */
#define SPI_Y2_INST_EXT				BIT_9

/* SPI EPROM command mask */
#define SPI_Y2_CMD_MASK				(7L << 16 | SPI_Y2_INST_EXT)

/* Reg0 */
/* SPI EPROM write status instruction */
#define SPI_Y2_WST					(BIT_19 | 2L << 16 | SPI_Y2_INST_EXT)
/* SPI EPROM read sector protection instruction */
#define SPI_Y2_RDSP					(BIT_19 | 3L << 16 | SPI_Y2_INST_EXT)

/* Reg1 */
/* SPI EPROM read instruction */
#define SPI_Y2_RD					(BIT_19 | 1L << 16)
/* SPI EPROM read ID instruction */
#define SPI_Y2_RDID					(BIT_19 | 2L << 16)
/* SPI EPROM read status register instruction */
#define SPI_Y2_RDST					(BIT_19 | 3L << 16)

/* Reg2 */
/* SPI EPROM write enable instruction */
#define SPI_Y2_WEN					(BIT_19 | 4L << 16)
/* SPI EPROM write instruction */
#define SPI_Y2_WR					(BIT_19 | 5L << 16)
/* SPI EPROM sector erase instruction */
#define SPI_Y2_SERS					(BIT_19 | 6L << 16)
/* SPI EPROM chip erase instruction */
#define SPI_Y2_CERS					(BIT_19 | 7L << 16)

/* SPI EPROM ID byte count mask */
#define SPI_Y2_ID_BYTE_COUNT_MASK	(7L << 20)

/* SPI EPROM clock divisor mask */
#define SPI_Y2_CLK_DIV_MASK			(3L << 26)

#define SPI_Y2_CLK_DIV(x)			(SHIFT26(x) & SPI_Y2_CLK_DIV_MASK)

/* SPI flash read ID protocol */
#define SPI_Y2_RDID_PROT			(1L << 28)

/* SPI flash VPD mapping enable */
#define SPI_Y2_VPD_ENABLE			(1L << 29)

/* SPI EPROM BUSY CHECK */
#define SPI_Y2_IS_BUSY(w)			(((w) >> 30) & 1)
#define SPI_Y2_IS_BUSY_WR(w)		((w) & 1)

#define SPI_Y2_MAN_ID_MASK			0xff00
#define SPI_Y2_DEV_ID_MASK			0x00ff

/* SPI flash manufacturer ID's */
#define SPI_MAN_ID_ATMEL			0x1f
#define SPI_MAN_ID_SST				0xbf
#define SPI_MAN_ID_NUM				0x13
#define SPI_MAN_ID_NUMONYX			0x20
#define SPI_MAN_ID_CHTEK			0x9d
#define SPI_MAN_ID_WINBOND			0xef
#define SPI_MAN_ID_INTEL			0x89
#define SPI_MAN_ID_MXIC				0xc2

/* wait loop for SPI EPROM to finish command (start spi_timer() before) */
#define SPI_Y2_WAIT_LP_SE_FINISH_CMD(pAC, IoC, stat, timeout) {		\
	do {															\
		SK_IN32(IoC, SPI_Y2_CONTROL_REG, &(stat));					\
		if (0 != spi_timer(pAC, SPI_TIMEOUT)) {						\
			fl_print("\nSPI timeout: %d sec.\n", timeout);			\
			break;													\
		}															\
	} while (SPI_Y2_IS_BUSY(stat));									\
}

/* wait for SPI EPROM to finish command */
#define SPI_Y2_WAIT_SE_FINISH_CMD(pAC, IoC, timeout) {				\
	unsigned long stat;												\
	/* wait for command to finish */								\
	spi_timer(pAC, timeout);										\
	SPI_Y2_WAIT_LP_SE_FINISH_CMD(pAC, IoC, stat, timeout);			\
}

/* wait for SPI EPROM to finish write/erase operation */
#define SPI_Y2_WAIT_SE_FINISH_WR(pAC, IoC, timeout) {				\
	unsigned long stat2;											\
	/* wait for command to finish */								\
	spi_timer(pAC, SPI_TIMER_SET);									\
	SPI_Y2_WAIT_LP_SE_FINISH_CMD(pAC, IoC, stat2, SPI_TIMER_SET);	\
	if (!SPI_Y2_IS_BUSY(stat2)) {									\
		/* wait for write to finish or timeout */					\
		spi_timer(pAC, timeout);									\
		do {														\
			SK_IN32(IoC, SPI_Y2_CONTROL_REG, &stat2);				\
			stat2 &= ~SPI_Y2_CMD_MASK;								\
			stat2 |= SPI_Y2_RDST;									\
			SK_OUT32(IoC, SPI_Y2_CONTROL_REG, stat2);				\
																	\
			SPI_Y2_WAIT_LP_SE_FINISH_CMD(pAC, IoC, stat2, timeout);	\
			if (SPI_Y2_IS_BUSY(stat2) ||							\
				0 != spi_timer(pAC, SPI_TIMEOUT)) {					\
				break;												\
			}														\
			SK_IN32(IoC, SPI_Y2_CONTROL_REG, &stat2);				\
		} while (SPI_Y2_IS_BUSY_WR(stat2));							\
	}																\
}

#if defined(Core) || defined(DJGPP) || !defined(MSDOS)
#define huge
#endif /* Core || DJGPP || !MSDOS */

/* typedefs ******************************************************************/

/*
 * Yukon-II family SPI flash device structure
 */
typedef struct s_SpiDevTable {
	char			*man_name;
	unsigned char	man_id;
	char			*dev_name;
	unsigned char	dev_id;
	unsigned long	sector_size;
	unsigned long	sector_num;
	unsigned char	set_protocol;
	unsigned char	clk_rate_div;
	unsigned char	id_byte_count;	/* For Yukon-Optima and later */
	unsigned char	dev_id2;		/* For Yukon-Optima and later */
	unsigned		chip_erase_timeout;

	struct {
		unsigned char	op_read;
		unsigned char	op_write;
		unsigned char	op_write_enable;
		unsigned char	op_write_disable;	/* Unused? */
		unsigned char	op_read_status;
		unsigned char	op_read_id;
		unsigned char	op_sector_erase;
		unsigned char	op_chip_erase;

		/* For SPI_OPC0 on Yukon-Optima and later */
		unsigned char	op_write_status;
		unsigned char	op_read_sect_protect;
	} opcodes;
} SK_SPI_DEVICE;

typedef	struct s_GeSpi {
#ifndef SK_SPI_NO_FLASHFUN
/* Used in flashfun.c */
#if defined(MSDOS) || defined(INTERN) || defined(MRVL_UEFI)
	unsigned char	*fprom;				/* Address of ROM */
#endif	/* MSDOS || INTERN || MRVL_UEFI */

	/* Statistics */
	long			prog_retries;
	int				max_retries;
	int				erase_cycles;
	int				vpp;				/* Flag: Vpp is on/off */
#endif	/* !SK_SPI_NO_FLASHFUN */

/* Used in flashfun.c and skspi.c */
	long			max_pages;			/* Number of pages in SPI eprom */
	long			max_faddr;			/* Pagesize of SPI eprom */
	int				fl_type;			/* Flash type */

/* Used in skspi.c */
	SK_SPI_DEVICE	*pSpiDev;				/* Pointer to type of SPI flash */
	unsigned char	yk_chip_id;				/* Chip ID */
	unsigned char	yk_chip_rev;			/* Chip Revision */
	unsigned char	needs_unprotect;		/* Needs unprotect before write/erase. */
	unsigned char	ModifiedPages[0x80];	/* Modified pages: 0x80000 / 0x1000 */
} SK_SPI;

/* function prototypes *******************************************************/

/*
 * After every call to flash_check_spi() (which is also called by
 * init_flash()), the MT-Tools should correct pAC->spi.ModifiedPages[].
 * For all other users, the values set in flash_check_spi() should be fine.
 */
int flash_check_spi(
	SK_AC			*pAC,
	SK_IOC			IoC,
	unsigned long	*FlashSize);

int spi_flash_erase(
	SK_AC			*pAC,
	SK_IOC			IoC,
	unsigned long	off,
	unsigned long	len);

int spi_flash_manage(
	SK_AC			*pAC,
	SK_IOC			IoC,
	unsigned char	*data,
	unsigned long	off,
	unsigned long	len,
	int				flag);

int spi_vpd_transfer(
	SK_AC	*pAC,
	SK_IOC	IoC,
	char	*buf,
	int		addr,
	int		len,
	int		dir);

int spi_get_pig(
	SK_AC			*pAC,
	SK_IOC			IoC,
	unsigned char	*data,
	unsigned long	len);

int spi_get_noc(
	SK_AC			*pAC,
	SK_IOC			IoC,
	unsigned char	*data,
	unsigned long	len);

int spi_get_pet(
	SK_AC			*pAC,
	SK_IOC			IoC,
	unsigned char	*data,
	unsigned long	len);

#ifndef SK_SPI_NO_UPDATE

int spi_update_pig(
	SK_AC			*pAC,
	SK_IOC			IoC,
	unsigned char	*data,
	unsigned long	len);

int spi_update_noc(
	SK_AC			*pAC,
	SK_IOC			IoC,
	unsigned char	*data,
	unsigned long	len);

int spi_update_pet(
	SK_AC			*pAC,
	SK_IOC			IoC,
	unsigned char	*data,
	unsigned long	len);

#endif	/* !SK_SPI_NO_UPDATE */

#ifdef __cplusplus
}
#endif	/* __cplusplus */

#endif /* __INC_SKSPI_H */

/* End of File */
