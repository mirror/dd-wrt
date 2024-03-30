/*
 * Qualcomm/Atheros Serial SPI FLASH driver utilizing SHIFT registers
 *
 * Copyright (C) 2015 Piotr Dymacz <piotr@dymacz.pl>
 * Copyright (C) 2016 Mantas Pucka <mantas@8devices.com>
 *
 * SPDX-License-Identifier:GPL-2.0
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/types.h>
#include "qca-sf.h"
static u32 flash_id;

/* Use CS0 by default */
static u32 qca_sf_cs_mask = QCA_SPI_SHIFT_CNT_CHNL_CS0_MASK;

inline void qca_sf_spi_en(void)
{
	qca_soc_reg_write(QCA_SPI_FUNC_SEL_REG, 1);
}

inline void qca_sf_spi_di(void)
{
	qca_soc_reg_write(QCA_SPI_SHIFT_CNT_REG, 0);
	qca_soc_reg_write_flush(QCA_SPI_FUNC_SEL_REG, 0);
}

static inline u32 qca_sf_shift_in(void)
{
	return qca_soc_reg_read(QCA_SPI_SHIFT_DATAIN_REG);
}

/*
 * Shifts out 'bits_cnt' bits from 'data_out' value
 * If 'terminate' is zero, then CS is not driven high at end of transaction
 */
static void qca_sf_shift_out(u32 data_out, u32 bits_cnt, u32 terminate)
{
	u32 reg_val = 0;

	qca_soc_reg_write(QCA_SPI_SHIFT_CNT_REG, 0);

	/* Data to shift out */
	qca_soc_reg_write(QCA_SPI_SHIFT_DATAOUT_REG, data_out);

	reg_val = reg_val | bits_cnt | qca_sf_cs_mask | QCA_SPI_SHIFT_CNT_SHIFT_EN_MASK;

	if (terminate)
		reg_val = reg_val | QCA_SPI_SHIFT_CNT_TERMINATE_MASK;

	/* Enable shifting in/out */
	qca_soc_reg_write(QCA_SPI_SHIFT_CNT_REG, reg_val);
}

static u32 qca_sf_sfdp_bfpt_dword(u32 ptp_offset, u32 dword_num)
{
	u32 data_out;

	data_out = (SPI_FLASH_CMD_SFDP << 24);
	data_out = data_out | (ptp_offset + ((dword_num - 1) * 4));

	qca_sf_shift_out(data_out, 32, 0);
	qca_sf_shift_out(0x0, 40, 1);

	return cpu_to_le32(qca_sf_shift_in());
}

static inline void qca_sf_write_en(void)
{
	qca_sf_shift_out(SPI_FLASH_CMD_WREN, 8, 1);
}

static inline void qca_sf_write_di(void)
{
	qca_sf_shift_out(SPI_FLASH_CMD_WRDI, 8, 1);
}

/* Poll status register and wait till busy bit is cleared */
static void qca_sf_busy_wait(void)
{
	volatile u32 data_in;

	/* Poll status register continuously (keep CS low during whole loop) */
	qca_sf_shift_out(SPI_FLASH_CMD_RDSR, 8, 0);

	do {
		qca_sf_shift_out(0x0, 8, 0);
		data_in = qca_sf_shift_in() & 0x1;
	} while (data_in);

	/* Disable CS chip */
	qca_sf_shift_out(0x0, 0, 1);
}
#ifdef CONFIG_UBNTFIX
static inline void qca_mxc_unlock(void)
{
	qca_sf_spi_en();
	qca_sf_write_en();
	qca_sf_shift_out(MXIC_EXSO, 8, 1);
	qca_sf_busy_wait();
}

static inline void qca_flash_unlock(void)
{
	qca_sf_spi_en();
	qca_sf_write_en();
	qca_sf_shift_out(SPI_FLASH_CMD_WRSR << 8, 16, 1);
	qca_sf_busy_wait();
}
#endif

/* Returns flash configuration register that is accessible with command 'cmd' */
u8 qca_sf_read_reg(u8 cmd)
{
	qca_sf_shift_out(cmd << 8, 16, 1);
	return qca_sf_shift_in();
}

/* Writes 'data' to flash configuration register that has wirite command 'cmd' */
void qca_sf_write_reg(u8 cmd, u8 data)
{
	qca_sf_write_en();
	qca_sf_shift_out(cmd, 8, 0);
	qca_sf_shift_out(data, 8, 1);
	qca_sf_busy_wait();
	qca_sf_write_di();
}

static void qca_sf_set_addressing(flash_info_t * info, u32 enable)
{
	if (info->need_4byte_enable_op) {
		if (enable)
			qca_sf_shift_out(SPI_FLASH_CMD_EN4B, 8, 1);
		else {
			qca_sf_shift_out(SPI_FLASH_CMD_EX4B, 8, 1);
			/* Set extended address reg to 0 to access
			 * lower 16MB when using HW mapping */
			qca_sf_write_reg(SPI_FLASH_CMD_WEAR, 0x0);
		}
	}
}

static void qca_sf_bank_to_cs_mask(u32 bank)
{
	switch (bank) {
	case 0:
		qca_sf_cs_mask = QCA_SPI_SHIFT_CNT_CHNL_CS0_MASK;
		break;
	case 1:
		qca_sf_cs_mask = QCA_SPI_SHIFT_CNT_CHNL_CS1_MASK;
		break;
	case 2:
		qca_sf_cs_mask = QCA_SPI_SHIFT_CNT_CHNL_CS2_MASK;
		break;
	default:
		qca_sf_cs_mask = QCA_SPI_SHIFT_CNT_CHNL_CS0_MASK;
		break;
	}
}

void qca_sf_prepare_cmd(u8 cmd, u32 addr, u32 is_4byte, u32 buf[2])
{
	if (is_4byte) {
		buf[0] = cmd << 24;
		buf[0] = buf[0] | (addr >> 8);
		buf[1] = (addr & 0x000000FF);
	} else {
		buf[0] = cmd << 24;
		buf[0] = buf[0] | (addr & 0x00FFFFFF);
	}
}

/* Bulk (whole) FLASH erase */
void qca_sf_bulk_erase(u32 bank)
{
	qca_sf_bank_to_cs_mask(bank);
	qca_sf_spi_en();
	qca_sf_write_en();
	qca_sf_shift_out(SPI_FLASH_CMD_ES_ALL, 8, 1);
	qca_sf_busy_wait();
	qca_sf_spi_di();
}

/* Erase one sector at provided address */
u32 qca_sf_sect_erase(flash_info_t * info, u32 address)
{
	u32 data_out[2];
	qca_sf_bank_to_cs_mask(info->bank);

	if (address >= 0x1000000 && info->use_4byte_addr == 0) {
		return -1;
	}

	qca_sf_prepare_cmd(info->erase_cmd, address, info->use_4byte_addr, data_out);

	qca_sf_spi_en();
	qca_sf_set_addressing(info, 1);
	qca_sf_write_en();
	if (info->use_4byte_addr) {
		qca_sf_shift_out(data_out[0], 32, 0);
		qca_sf_shift_out(data_out[1], 8, 1);
	} else {
		qca_sf_shift_out(data_out[0], 32, 1);
	}

	qca_sf_busy_wait();
	qca_sf_set_addressing(info, 0);
	qca_sf_spi_di();

	return 0;
}

/* Writes 'length' bytes at 'address' using page program command */
void qca_sf_write_page(flash_info_t * info, u32 bank, u32 address, u32 length, const u8 *data)
{
	int i;
	u32 data_out[2];

	qca_sf_bank_to_cs_mask(bank);

	if (address >= 0x1000000 && info->use_4byte_addr == 0) {
		return;
	}

	/* assemble write command */
	qca_sf_prepare_cmd(info->page_program_cmd, address, info->use_4byte_addr, data_out);

	qca_sf_spi_en();
	qca_sf_set_addressing(info, 1);
	qca_sf_write_en();

	qca_sf_shift_out(data_out[0], 32, 0);
	if (info->use_4byte_addr) {
		qca_sf_shift_out(data_out[1], 8, 0);
	}

	length--;
	for (i = 0; i < length; i++) {
		qca_sf_shift_out(*(data + i), 8, 0);
	}

	/* Last byte and terminate */
	qca_sf_shift_out(*(data + i), 8, 1);

	qca_sf_busy_wait();
	qca_sf_set_addressing(info, 0);
	qca_sf_spi_di();
}

/* Reads 'length' bytes at 'address' to 'data' using read command */
int qca_sf_read(flash_info_t * info, u32 bank, u32 address, u32 length, u8 *data)
{
	int i;
	u32 data_out[2];

	if (address >= 0x1000000 && info->use_4byte_addr == 0)
		return -1;

	/* assemble read command */
	qca_sf_prepare_cmd(info->read_cmd, address, info->use_4byte_addr, data_out);

	qca_sf_spi_en();
	qca_sf_set_addressing(info, 1);
	qca_sf_shift_out(data_out[0], 32, 0);
	if (info->use_4byte_addr) {
		qca_sf_shift_out(data_out[1], 8, 0);
	}

	/* read data */
	length--;
	for (i = 0; i < length; i++) {
		qca_sf_shift_out(0, 8, 0);
		*(data + i) = qca_sf_shift_in();
	}

	qca_sf_shift_out(0, 8, 1);
	*(data + i) = qca_sf_shift_in();

	qca_sf_set_addressing(info, 0);
	qca_sf_spi_di();

	return 0;
}

/*
 * Checks if FLASH supports SFDP and if yes, tries to get following data:
 * - chip size
 * - erase sector size
 * - erase command
 */
u32 qca_sf_sfdp_info(u32 bank, u32 *flash_size, u32 *sect_size, u8 *erase_cmd)
{
	u8 buffer[12];
	u8 ss = 0, ec = 0;
	u32 data_in, i;
	u32 ptp_length, ptp_offset;

	qca_sf_bank_to_cs_mask(bank);

	qca_sf_spi_en();

	/* Shift out SFDP command with 0x0 address */
	qca_sf_shift_out(SPI_FLASH_CMD_SFDP << 24, 32, 0);

	/* 1 dummy byte and 4 bytes for SFDP signature */
	qca_sf_shift_out(0x0, 40, 0);
	data_in = qca_sf_shift_in();

	if (cpu_to_le32(data_in) != SPI_FLASH_SFDP_SIGN) {
		qca_sf_shift_out(0x0, 0, 1);
		qca_sf_spi_di();
		return 1;
	}

	/*
	 * We need to check SFDP and first parameter header major versions,
	 * because we support now only v1, exit also if ptp_length is < 9
	 */
	for (i = 0; i < 3; i++) {
		qca_sf_shift_out(0x0, 32, 0);
		data_in = qca_sf_shift_in();

		memcpy(&buffer[i * 4], &data_in, 4);
	}

	ptp_length = buffer[7];
	ptp_offset = buffer[8] | (buffer[10] << 16) | (buffer[9] << 8);

	if (buffer[1] != 1 || buffer[6] != 1 || ptp_length < 9) {
		qca_sf_shift_out(0x0, 0, 1);
		qca_sf_spi_di();
		return 1;
	}

	qca_sf_shift_out(0x0, 0, 1);

	/* FLASH density (2nd DWORD in JEDEC basic FLASH parameter table) */
	data_in = qca_sf_sfdp_bfpt_dword(ptp_offset, 2);

	/* We do not support >= 4 Gbits chips */
	if ((data_in & (1 << 31)) || data_in == 0) {
		qca_sf_spi_di();
		return 1;
	}

	/* TODO: it seems that density is 0-based, like max. available address? */
	if (flash_size != NULL)
		*flash_size = ((data_in & 0x7FFFFFFF) + 1) / 8;

	/* Sector/block erase size and command: 8th and 9th DWORD */
	data_in = qca_sf_sfdp_bfpt_dword(ptp_offset, 8);
	memcpy(&buffer[0], &data_in, 4);

	data_in = qca_sf_sfdp_bfpt_dword(ptp_offset, 9);
	memcpy(&buffer[4], &data_in, 4);

	/* We prefer bigger erase sectors */
	for (i = 0; i < 7; i += 2) {
		if ((buffer[i + 1] != 0) && buffer[i + 1] > ss) {
			ss = buffer[i + 1];
			ec = buffer[i];
		}
	}

	if (ss == 0) {
		qca_sf_spi_di();
		return 1;
	}
	if (sect_size != NULL)
		*sect_size = 1 << ss;

	if (erase_cmd != NULL)
		*erase_cmd = ec;

	qca_sf_spi_di();

	return 0;
}

/* Returns JEDEC ID for selected FLASH chip */
u32 qca_sf_jedec_id(u32 bank)
{
	u32 data_in;

	qca_sf_bank_to_cs_mask(bank);

	qca_sf_spi_en();
	qca_sf_shift_out(SPI_FLASH_CMD_JEDEC << 24, 32, 1);
	data_in = qca_sf_shift_in();
	qca_sf_spi_di();

	return (data_in & 0x00FFFFFF);
}

int qca_sf_flash_erase(flash_info_t * info, u32 address, u32 length, u8 *buf)
{
	int ret;
	int sector_size = info->size / info->sector_count;
#ifdef CONFIG_UBNTFIX
	switch ((flash_id >> 16) & 0xff) {
	case MXIC_JEDEC_ID:
		qca_mxc_unlock();
	case ATMEL_JEDEC_ID:
	case WINB_JEDEC_ID:
	case INTEL_JEDEC_ID:
	case SST_JEDEC_ID:
		qca_flash_unlock();
	}
#endif

	if (address % sector_size || length % sector_size) {
		printk(KERN_ERR "SF: Erase offset/length not multiple of erase size\n");
		return -1;
	}

	while (length) {
		ret = qca_sf_sect_erase(info, address);

		if (ret) {
			printk(KERN_ERR "SF: erase failed\n");
			break;
		}
		address += sector_size;
		length -= sector_size;
	}

	return ret;
}

int qca_sf_write_buf(flash_info_t * info, u32 bank, u32 address, u32 length, const u8 *buf)
{
	const u8 *src;
	u32 dst;
	int total = 0;
	int len_this_lp, bytes_this_page;
#ifdef CONFIG_UBNTFIX
	switch ((flash_id >> 16) & 0xff) {
	case MXIC_JEDEC_ID:
		qca_mxc_unlock();
	case ATMEL_JEDEC_ID:
	case WINB_JEDEC_ID:
	case INTEL_JEDEC_ID:
	case SST_JEDEC_ID:
		qca_flash_unlock();
	}
#endif

	while (total < length) {
		src = buf + total;
		dst = address + total;
		bytes_this_page = ATH_SPI_PAGE_SIZE - (address % ATH_SPI_PAGE_SIZE);
		len_this_lp = ((length - total) > bytes_this_page) ? bytes_this_page : (length - total);
		qca_sf_write_page(info, 0, dst, len_this_lp, src);
		total += len_this_lp;
	}

	return 0;
}

#define JEDEC_MFR(info)	(((info)->flash_id >> 16) & 0xff )
#define MFR_ID_SPANSION 0x01
#define MFR_ID_WINBOND 0xef

void flash_enable_reset(void)
{
	u8 reg;
	qca_sf_spi_en();
	reg = qca_sf_read_reg(SPI_FLASH_CMD_RSR3);
	// Enable RESET signal on HOLD/RESET pin
	if ((reg & (1 << FLASH_SR3_HOLD_RESET_OFS)) == 0) {
		reg = reg | (1 << FLASH_SR3_HOLD_RESET_OFS);
		qca_sf_write_reg(SPI_FLASH_CMD_WSR3, reg);
	}

	reg = qca_sf_read_reg(SPI_FLASH_CMD_RSR2);
	// Disable Quad mode to allow RESET signal
	if ((reg & (1 << FLASH_SR2_QE_OFS)) != 0) {
		reg = reg & ~(1 << FLASH_SR2_QE_OFS);
		qca_sf_write_reg(SPI_FLASH_CMD_WSR2, reg);
	}
	qca_sf_spi_di();
}

static unsigned int guessflashsize(void *base)
{
	unsigned int size;
	unsigned int *guess = (unsigned int *)base;
	unsigned int max = 16 << 20;
//check 3 patterns since we can't write. 
	unsigned int p1 = guess[0];
	unsigned int p2 = guess[4096];
	unsigned int p3 = guess[8192];
	unsigned int c1;
	unsigned int c2;
	unsigned int c3;
	for (size = 2 << 20; size <= (max >> 1); size <<= 1) {
		unsigned int ofs = size / 4;
		c1 = guess[ofs];
		c2 = guess[ofs + 4096];
		c3 = guess[ofs + 8192];
		if (p1 == c1 && p2 == c2 && p3 == c3)	// mirror found
		{
			break;
		}
	}
	printk(KERN_INFO "guessed flashsize = %dM\n", size >> 20);
	return size;

}

unsigned long flash_get_geom(flash_info_t * flash_info)
{
	int i;
	int ret;
	u32 flash_size, sect_size;
	u8 erase_cmd;
	ret = qca_sf_sfdp_info(0, &flash_size, &sect_size, &erase_cmd);
	flash_id = qca_sf_jedec_id(0);
	printk(KERN_INFO "jedec ID %X\n",flash_id);
	flash_info->use_4byte_addr = 0;
	flash_info->need_4byte_enable_op = 0;
	flash_info->read_cmd = SPI_FLASH_CMD_READ;
	flash_info->page_program_cmd = SPI_FLASH_CMD_PP;

	if (ret == 0) {
		flash_info->flash_id = flash_id;
		flash_info->size = flash_size;	/* bytes */
		flash_info->sector_size = sect_size;
		flash_info->sector_count = flash_size / sect_size;
		flash_info->erase_cmd = erase_cmd;
		if (flash_info->size > 16 * 1024 * 1024) {
			printk(KERN_INFO "flash: >16MB detected\n");
			flash_info->use_4byte_addr = 1;
			if (JEDEC_MFR(flash_info) == MFR_ID_SPANSION) {
				printk(KERN_INFO "flash_mfr:Spansion\n");
				flash_info->need_4byte_enable_op = 0;
				flash_info->erase_cmd = SPI_FLASH_CMD_4SE;
				flash_info->read_cmd = SPI_FLASH_CMD_4READ;
				flash_info->page_program_cmd = SPI_FLASH_CMD_4PP;
			} else if (JEDEC_MFR(flash_info) == MFR_ID_WINBOND) {
				printk(KERN_INFO "flash_mfr:Winbond\n");
				flash_info->need_4byte_enable_op = 1;
				flash_enable_reset();
			} else {
				printk(KERN_INFO "flash_mfr:unknonwn\n");
				flash_info->need_4byte_enable_op = 1;
			}
		}
	} else {
		flash_info->flash_id = FLASH_M25P64;
		flash_info->size = guessflashsize((void *)0xbf000000);	/* bytes */
		flash_info->sector_size = CFG_DEFAULT_FLASH_SECTOR_SIZE;
		flash_info->sector_count = flash_info->size / CFG_DEFAULT_FLASH_SECTOR_SIZE;
		flash_info->erase_cmd = ATH_SPI_CMD_SECTOR_ERASE;
	}
	printk(KERN_INFO "flash size %dB, sector count = %d\n", flash_info->size, flash_info->sector_count);

	return (flash_info->size);
}
