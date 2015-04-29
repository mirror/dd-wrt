/*
 *  AR71xx SoC routines
 *
 *  Copyright (C) 2007 Atheros 
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <asm/byteorder.h>
#include <linux/init.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/bitops.h>
#include <asm/irq.h>
#include <asm/io.h>

#include "ar7100.h"

void __iomem *ar71xx_ddr_base;
void __iomem *ar71xx_pll_base;
void __iomem *ar71xx_reset_base;
void __iomem *ar71xx_gpio_base;
void __iomem *ar71xx_usb_ctrl_base;

/* 
 * GPIO Misc IRQ Functions
 */
void ar7100_misc_enable_irq(unsigned int mask)
{
	ar7100_reg_rmw_set(AR7100_MISC_INT_MASK, mask);
}

void ar7100_misc_disable_irq(unsigned int mask)
{
	ar7100_reg_rmw_clear(AR7100_MISC_INT_MASK, mask);
}

unsigned int ar7100_misc_get_irq_mask(void)
{
	return ar7100_reg_rd(AR7100_MISC_INT_MASK);
}

unsigned int ar7100_misc_get_irq_status(void)
{
	return ar7100_reg_rd(AR7100_MISC_INT_STATUS);
}

EXPORT_SYMBOL(ar7100_misc_enable_irq);
EXPORT_SYMBOL(ar7100_misc_disable_irq);
EXPORT_SYMBOL(ar7100_misc_get_irq_mask);
EXPORT_SYMBOL(ar7100_misc_get_irq_status);

/*
 * Reset function
 */
void ar7100_reset(unsigned int mask)
{
	ar7100_reg_rmw_set(AR7100_RESET, mask);
	udelay(100);
	ar7100_reg_rmw_clear(AR7100_RESET, mask);
}

EXPORT_SYMBOL(ar7100_reset);

/* 
 * DMA Functions for SLIC/STEREO Blocks
 */
void ar7100_dma_addr_wr(int chan, unsigned int val)
{
	ar7100_reg_wr(AR7100_DMA_BASE + 0 + chan * 12, val);
}

void ar7100_dma_config_wr(int chan, unsigned int val)
{
	ar7100_reg_wr(AR7100_DMA_BASE + 4 + chan * 12, val);
}

void ar7100_dma_update_wr(int chan, unsigned int val)
{
	ar7100_reg_wr(AR7100_DMA_BASE + 8 + chan * 12, val);
}

unsigned int ar7100_dma_addr_rd(int chan)
{
	return ar7100_reg_rd(AR7100_DMA_BASE + 0 + chan * 12);
}

unsigned int ar7100_dma_config_rd(int chan)
{
	return ar7100_reg_rd(AR7100_DMA_BASE + 4 + chan * 12);
}

void ar7100_dma_config_buffer(int chan, void *buffer, int sizeCfg)
{
	unsigned int addr = KSEG1ADDR(buffer);
	ar7100_dma_addr_wr(chan, (unsigned int)addr);
	ar7100_dma_config_wr(chan, ((sizeCfg & 0x7) << 4) | 0x100);
}

EXPORT_SYMBOL(ar7100_dma_addr_wr);
EXPORT_SYMBOL(ar7100_dma_config_wr);
EXPORT_SYMBOL(ar7100_dma_update_wr);
EXPORT_SYMBOL(ar7100_dma_addr_rd);
EXPORT_SYMBOL(ar7100_dma_config_rd);
EXPORT_SYMBOL(ar7100_dma_config_buffer);

/*
 * SLIC
 */
unsigned int ar7100_slic_status_rd(void)
{
	return ar7100_reg_rd(AR7100_SLIC_STATUS);
}

unsigned int ar7100_slic_cntrl_rd(void)
{
	return ar7100_reg_rd(AR7100_SLIC_CNTRL);
}

void ar7100_slic_cntrl_wr(unsigned int val)
{
	ar7100_reg_wr(AR7100_SLIC_CNTRL, val);
}

void ar7100_slic_0_slot_pos_wr(unsigned int val)
{
	ar7100_reg_wr(AR7100_SLIC_SLOT0_NUM, val);
}

void ar7100_slic_1_slot_pos_wr(unsigned int val)
{
	ar7100_reg_wr(AR7100_SLIC_SLOT1_NUM, val);
}

void ar7100_slic_freq_div_wr(unsigned int val)
{
	ar7100_reg_wr(AR7100_SLIC_FREQ_DIV, val);
}

void ar7100_slic_sample_pos_wr(unsigned int val)
{
	ar7100_reg_wr(AR7100_SLIC_SAM_POS, val);
}

void ar7100_slic_setup(int _sam, int _s0n, int _s1n)
{
	unsigned int cntrl = 0;
	ar7100_reset(AR7100_RESET_SLIC);
	ar7100_gpio_enable_slic();
	ar7100_slic_freq_div_wr(0x60);
	ar7100_slic_sample_pos_wr(_sam);
	if (_s0n) {
		cntrl |= AR7100_SLIC_CNTRL_ENABLE;
		cntrl |= AR7100_SLIC_CNTRL_SLOT0_ENABLE;
		ar7100_slic_0_slot_pos_wr(_s0n);
	}
	if (_s1n) {
		cntrl |= AR7100_SLIC_CNTRL_ENABLE;
		cntrl |= AR7100_SLIC_CNTRL_SLOT1_ENABLE;
		ar7100_slic_1_slot_pos_wr(_s1n);
	}
	if (cntrl)
		ar7100_slic_cntrl_wr(cntrl);
}

EXPORT_SYMBOL(ar7100_slic_status_rd);
EXPORT_SYMBOL(ar7100_slic_cntrl_rd);

EXPORT_SYMBOL(ar7100_slic_cntrl_wr);
EXPORT_SYMBOL(ar7100_slic_0_slot_pos_wr);
EXPORT_SYMBOL(ar7100_slic_1_slot_pos_wr);
EXPORT_SYMBOL(ar7100_slic_freq_div_wr);
EXPORT_SYMBOL(ar7100_slic_sample_pos_wr);

EXPORT_SYMBOL(ar7100_slic_setup);

/*
 * STEREO Block Helper Functions
 */

/* Low-level registers */
void ar7100_stereo_config_wr(unsigned int val)
{
	ar7100_reg_wr(AR7100_STEREO_CONFIG, val);
}

void ar7100_stereo_volume_wr(unsigned int val)
{
	ar7100_reg_wr(AR7100_STEREO_VOLUME, val);
}

unsigned int ar7100_stereo_config_rd(void)
{
	return ar7100_reg_rd(AR7100_STEREO_CONFIG);
}

unsigned int ar7100_stereo_volume_rd(void)
{
	return ar7100_reg_rd(AR7100_STEREO_VOLUME);
}

/* Routine sets up STEREO block for use. Use one of the predefined
 * configurations. Example:
 * 
 * ar7100_stereo_config_setup(
 *   AR7100_STEREO_CFG_MASTER_STEREO_FS32_48KHZ(AR7100_STEREO_WS_16B))
 *
 */
void ar7100_stereo_config_setup(unsigned int cfg)
{
	unsigned int reset;
	ar7100_gpio_enable_stereo();
	ar7100_stereo_config_wr(cfg & ~AR7100_STEREO_CONFIG_ENABLE);
	do {
		reset = ar7100_stereo_config_rd();
	} while (reset & AR7100_STEREO_CONFIG_RESET);

	do {
		reset = ar7100_reg_rd(AR7100_GPIO_IN);
	} while (0 == (reset & 1 << 7));

	do {
		reset = ar7100_reg_rd(AR7100_GPIO_IN);
	} while (reset & 1 << 7);

	ar7100_stereo_config_wr(cfg | AR7100_STEREO_CONFIG_ENABLE);
}

#define down mutex_lock
#define up mutex_unlock
#define init_MUTEX mutex_init
#define DECLARE_MUTEX(a) struct mutex a

/*
 * GPIO Access
 */
DECLARE_MUTEX(ar7100_gpio_sem);

void ar7100_gpio_init(void)
{
	init_MUTEX(&ar7100_gpio_sem);
}

void ar7100_gpio_down(void)
{
	down(&ar7100_gpio_sem);
}

void ar7100_gpio_up(void)
{
	up(&ar7100_gpio_sem);
}

EXPORT_SYMBOL(ar7100_gpio_init);
EXPORT_SYMBOL(ar7100_gpio_down);
EXPORT_SYMBOL(ar7100_gpio_up);

/*
 * GPIO Function Enables
 */

/* enable SLIC block, takes away GPIO 5, 4, 3, and 2 */
void ar7100_gpio_enable_slic(void)
{
	ar7100_reg_rmw_set(AR7100_GPIO_FUNCTIONS, AR7100_GPIO_FUNCTION_SLIC_EN);
}

/* enable UART block, takes away GPIO 10 and 9 */
void ar7100_gpio_enable_uart(void)
{
	ar7100_reg_rmw_set(AR7100_GPIO_FUNCTIONS, AR7100_GPIO_FUNCTION_UART_EN);
	ar7100_reg_rmw_clear(AR7100_GPIO_OE, 1 << 9);
	ar7100_reg_rmw_set(AR7100_GPIO_OE, 1 << 10);
}

/* enable STEREO block, takes away GPIO 11,8,7, and 6 */
void ar7100_gpio_enable_stereo(void)
{
	ar7100_reg_rmw_clear(AR7100_GPIO_INT_ENABLE, 1 << 11);
	ar7100_reg_rmw_clear(AR7100_GPIO_OE, 1 << 11);
	ar7100_reg_rmw_set(AR7100_GPIO_FUNCTIONS, AR7100_GPIO_FUNCTION_STEREO_EN);
}

/* allow CS0/CS1 to be controlled via SPI register, takes away GPIO0/GPIO1 */
void ar7100_gpio_enable_spi_cs1_cs0(void)
{
	ar7100_reg_rmw_set(AR7100_GPIO_FUNCTIONS, AR7100_GPIO_FUNCTION_SPI_CS_0_EN | AR7100_GPIO_FUNCTION_SPI_CS_1_EN);
	ar7100_reg_rmw_clear(AR7100_GPIO_INT_ENABLE, 3);
	ar7100_reg_rmw_set(AR7100_GPIO_OE, 3);
}

/* allow GPIO0/GPIO1 to be used as SCL/SDA for software based i2c */
void ar7100_gpio_enable_i2c_on_gpio_0_1(void)
{
	ar7100_reg_rmw_clear(AR7100_GPIO_FUNCTIONS, AR7100_GPIO_FUNCTION_SPI_CS_0_EN | AR7100_GPIO_FUNCTION_SPI_CS_1_EN);
	ar7100_reg_rmw_clear(AR7100_GPIO_INT_ENABLE, 3);
	ar7100_reg_rmw_clear(AR7100_GPIO_OE, 3);
}

EXPORT_SYMBOL(ar7100_gpio_enable_slic);
EXPORT_SYMBOL(ar7100_gpio_enable_uart);
EXPORT_SYMBOL(ar7100_gpio_enable_stereo);
EXPORT_SYMBOL(ar7100_gpio_enable_spi_cs1_cs0);
EXPORT_SYMBOL(ar7100_gpio_enable_i2c_on_gpio_0_1);

/*
 * GPIO General Functions
 */

/* drive bits in mask low */
void ar7100_gpio_drive_low(unsigned int mask)
{
	ar7100_reg_wr(AR7100_GPIO_CLEAR, mask);
	ar7100_reg_rmw_set(AR7100_GPIO_OE, mask);
}

/* drive bits in mask high */
void ar7100_gpio_drive_high(unsigned int mask)
{
	ar7100_reg_wr(AR7100_GPIO_SET, mask);
	ar7100_reg_rmw_set(AR7100_GPIO_OE, mask);
}

/* Allow bits in mask to float to their quiescent state and test results */
unsigned int ar7100_gpio_float_high_test(unsigned int mask)
{
	volatile unsigned int d;
	ar7100_reg_rmw_clear(AR7100_GPIO_OE, mask);
	d = ar7100_reg_rd(AR7100_GPIO_IN);
	d = ar7100_reg_rd(AR7100_GPIO_IN) & mask;
	return d != mask;
}

EXPORT_SYMBOL(ar7100_gpio_drive_low);
EXPORT_SYMBOL(ar7100_gpio_drive_high);
EXPORT_SYMBOL(ar7100_gpio_float_high_test);

#ifdef USE_TEST_CODE

void ar7100_gpio_test_toggle(unsigned int mask)
{
	do {
		ar7100_gpio_drive_low(mask);
		udelay(10);
		ar7100_gpio_drive_high(mask);
		udelay(10);
	} while (0 == test_ui_char_present());
}

void ar7100_gpio_test_toggle_pull_high(unsigned int mask)
{
	do {
		ar7100_gpio_drive_low(mask);
		udelay(10);
		ar7100_gpio_float_high_test(mask);
		udelay(10);
	} while (0 == test_ui_char_present());
}

EXPORT_SYMBOL(ar7100_gpio_test_toggle)
    EXPORT_SYMBOL(ar7100_gpio_test_toggle_pull_high)
#endif
/*
 * Software support of i2c on gpio 0/1
 */
#define AR7100_I2C_SCL  (1<<0)
#define AR7100_I2C_SDA  (1<<1)
#define AR7100_I2C_PAUSE 2
static int ar7100_i2c_errcnt = 0;

static void ar7100_i2c_errclr(void)
{
	ar7100_i2c_errcnt = 0;
}

static void ar7100_i2c_check_rc(unsigned int rc)
{
	if (rc)
		ar7100_i2c_errcnt++;
}

static int ar7100_i2c_errget(void)
{
	return ar7100_i2c_errcnt;
}

static void ar7100_i2c_chigh_dhigh(void)
{
	ar7100_i2c_check_rc(ar7100_gpio_float_high_test(AR7100_I2C_SCL | AR7100_I2C_SDA));
	udelay(AR7100_I2C_PAUSE);
}

static void ar7100_i2c_chigh_dlow(void)
{
	ar7100_i2c_check_rc(ar7100_gpio_float_high_test(AR7100_I2C_SCL));
	ar7100_gpio_drive_low(AR7100_I2C_SDA);
	udelay(AR7100_I2C_PAUSE);
}

static void ar7100_i2c_clow_dlow(void)
{
	ar7100_gpio_drive_low(AR7100_I2C_SCL | AR7100_I2C_SDA);
	udelay(AR7100_I2C_PAUSE);
}

static void ar7100_i2c_clow_dhigh(void)
{
	ar7100_gpio_drive_low(AR7100_I2C_SCL);
	ar7100_i2c_check_rc(ar7100_gpio_float_high_test(AR7100_I2C_SDA));
	udelay(AR7100_I2C_PAUSE);
}

static void ar7100_i2c_clow_dfloat(void)
{
	ar7100_gpio_drive_low(AR7100_I2C_SCL);
	ar7100_reg_rmw_clear(AR7100_GPIO_OE, AR7100_I2C_SDA);
	udelay(AR7100_I2C_PAUSE);
}

static void ar7100_i2c_chigh_dfloat(void)
{
	ar7100_gpio_drive_high(AR7100_I2C_SCL);
	ar7100_reg_rmw_clear(AR7100_GPIO_OE, AR7100_I2C_SDA);
	udelay(AR7100_I2C_PAUSE);
}

static int ar7100_i2c_chigh_dread(void)
{
	int d;

	ar7100_gpio_float_high_test(AR7100_I2C_SCL);
	ar7100_reg_rmw_clear(AR7100_GPIO_OE, AR7100_I2C_SDA);
	udelay(AR7100_I2C_PAUSE / 2);

	d = (ar7100_reg_rd(AR7100_GPIO_IN) & AR7100_I2C_SDA) ? 1 : 0;
	udelay(AR7100_I2C_PAUSE / 2);

	return d;
}

static void ar7100_i2c_start(void)
{
	ar7100_i2c_chigh_dhigh();
	ar7100_i2c_chigh_dlow();
	ar7100_i2c_clow_dlow();
}

static void ar7100_i2c_stop(void)
{
	ar7100_i2c_clow_dlow();
	ar7100_i2c_chigh_dlow();
	ar7100_i2c_chigh_dhigh();
}

static int ar7100_i2c_raw_write_8(unsigned char v)
{
	int ack;
	int ii = 7;
	do {
		if ((1 << ii) & v) {
			ar7100_i2c_clow_dhigh();
			ar7100_i2c_chigh_dhigh();
		} else {
			ar7100_i2c_clow_dlow();
			ar7100_i2c_chigh_dlow();
		}
	} while (ii--);

	ar7100_i2c_clow_dfloat();
	ack = ar7100_i2c_chigh_dread();
	ar7100_i2c_clow_dfloat();

	return ack;
}

static void ar7100_i2c_raw_read_8(char lastByte, unsigned char *v)
{
	int d;
	int ii = 7;
	int jj = 0;
	do {
		ar7100_i2c_clow_dfloat();
		d = ar7100_i2c_chigh_dread();
		if (d)
			jj |= 1 << ii;
	} while (ii--);

	if (lastByte) {
		ar7100_i2c_clow_dfloat();
		ar7100_i2c_chigh_dfloat();
	} else {
		ar7100_i2c_clow_dlow();
		ar7100_i2c_chigh_dlow();
	}
	*v = jj & 0xff;
}

int ar7100_i2c_raw_write_bytes_to_addr(int addr, unsigned char *buffer, int count)
{
	volatile int ack;
	int ii;
	ar7100_i2c_errclr();
	ar7100_i2c_start();
	ack = ar7100_i2c_raw_write_8(addr & 0xfe);
	if (ack)
		return 1;

	for (ii = 0; ii < count; ii++) {
		ack = ar7100_i2c_raw_write_8(buffer[ii]);
	}
	ar7100_i2c_stop();
	return ar7100_i2c_errget();
}

int ar7100_i2c_raw_read_bytes_from_addr(int addr, unsigned char *buffer, int count)
{
	int ack;
	int ii;
	ar7100_i2c_errclr();
	ar7100_i2c_start();
	ack = ar7100_i2c_raw_write_8((addr & 0xff) | 0x01);
	for (ii = 0; ii < count; ii++)
		ar7100_i2c_raw_read_8(ii == (count - 1), &buffer[ii]);
	ar7100_i2c_stop();
	return ar7100_i2c_errget();
}

EXPORT_SYMBOL(ar7100_i2c_raw_write_bytes_to_addr);
EXPORT_SYMBOL(ar7100_i2c_raw_read_bytes_from_addr);

#ifdef USE_TEST_CODE

void ar7100_i2c_test_write_bits(void)
{
	printk("Writing bit stream of AA00\n");
	ar7100_i2c_errclr();
	do {
		ar7100_i2c_start();
		ar7100_i2c_raw_write_8(0xAA);
		ar7100_i2c_raw_write_8(0x00);
		ar7100_i2c_stop();
		udelay(1000);
	} while (0 == test_ui_char_present());
}

void ar7100_i2c_test_addr_strapping(void)
{
	int jj;

	int end = 0x7e;
	int addr = 0x20;

	jj = 0;
	printk("Looping through addresses %02x .. %02x\n", addr, end);
	while (addr < end) {
		volatile int ack;
		ar7100_i2c_start();
		ack = ar7100_i2c_raw_write_8(addr & 0xfe);
		ar7100_i2c_stop();
		if (0 == ack) {
			jj++;
			printk(" Found addr:  %02x\n", addr);
		}
		addr += 2;
	};

	if (0 == jj)
		printk(" Failed test, no i2c found\n");
}

EXPORT_SYMBOL(ar7100_i2c_test_write_bits);
EXPORT_SYMBOL(ar7100_i2c_test_addr_strapping);

#endif

/* 
 * SPI Access Functions 
 */

DECLARE_MUTEX(ar7100_spi_sem);

void ar7100_spi_init(void)
{
	init_MUTEX(&ar7100_spi_sem);
	ar7100_reg_wr_nf(AR7100_SPI_CLOCK, 0x41);
}

void ar7100_spi_down(void)
{
	down(&ar7100_spi_sem);
}

void ar7100_spi_up(void)
{
	up(&ar7100_spi_sem);
}

EXPORT_SYMBOL(ar7100_spi_init);
EXPORT_SYMBOL(ar7100_spi_down);
EXPORT_SYMBOL(ar7100_spi_up);

void ar7100_spi_raw_output_u8(unsigned char val)
{
	int ii;
	unsigned int cs;

	cs = ar7100_reg_rd(AR7100_SPI_WRITE) & ~(AR7100_SPI_D0_HIGH | AR7100_SPI_CLK_HIGH);
	for (ii = 7; ii >= 0; ii--) {
		unsigned char jj = (val >> ii) & 1;
		ar7100_reg_wr_nf(AR7100_SPI_WRITE, cs | jj);
		ar7100_reg_wr_nf(AR7100_SPI_WRITE, cs | jj | AR7100_SPI_CLK_HIGH);
	}
}

void ar7100_spi_raw_output_u32(unsigned int val)
{
	int ii;
	unsigned int cs;
	cs = ar7100_reg_rd(AR7100_SPI_WRITE) & ~(AR7100_SPI_D0_HIGH | AR7100_SPI_CLK_HIGH);
	for (ii = 31; ii >= 0; ii--) {
		unsigned char jj = (val >> ii) & 1;
		ar7100_reg_wr_nf(AR7100_SPI_WRITE, cs | jj);
		ar7100_reg_wr_nf(AR7100_SPI_WRITE, cs | jj | AR7100_SPI_CLK_HIGH);
	}
}

unsigned int ar7100_spi_raw_input_u8(void)
{
	int ii;
	unsigned int cs;

	cs = ar7100_reg_rd(AR7100_SPI_WRITE) & ~(AR7100_SPI_D0_HIGH | AR7100_SPI_CLK_HIGH);

	for (ii = 7; ii >= 0; ii--) {
		ar7100_reg_wr_nf(AR7100_SPI_WRITE, cs);
		ar7100_reg_wr_nf(AR7100_SPI_WRITE, cs | AR7100_SPI_CLK_HIGH);
	}

	return ar7100_reg_rd(AR7100_SPI_RD_STATUS) & 0xff;
}

unsigned int ar7100_spi_raw_input_u32(void)
{
	int ii;
	unsigned int cs;

	cs = ar7100_reg_rd(AR7100_SPI_WRITE) & ~(AR7100_SPI_D0_HIGH | AR7100_SPI_CLK_HIGH);

	for (ii = 31; ii >= 0; ii--) {
		ar7100_reg_wr_nf(AR7100_SPI_WRITE, cs);
		ar7100_reg_wr_nf(AR7100_SPI_WRITE, cs | AR7100_SPI_CLK_HIGH);
	}

	return ar7100_reg_rd(AR7100_SPI_RD_STATUS);
}

EXPORT_SYMBOL(ar7100_spi_raw_output_u8);
EXPORT_SYMBOL(ar7100_spi_raw_output_u32);
EXPORT_SYMBOL(ar7100_spi_raw_input_u8);
EXPORT_SYMBOL(ar7100_spi_raw_input_u32);

#define AR7100_SPI_CMD_WREN         0x06
#define AR7100_SPI_CMD_RD_STATUS    0x05
#define AR7100_SPI_CMD_FAST_READ    0x0b
#define AR7100_SPI_CMD_PAGE_PROG    0x02
#define AR7100_SPI_CMD_SECTOR_ERASE 0xd8

static void ar7100_spi_wait_done(void)
{
	int rd;

	do {
		ar7100_reg_wr_nf(AR7100_SPI_WRITE, AR7100_SPI_CS_DIS);
		ar7100_spi_raw_output_u8(AR7100_SPI_CMD_RD_STATUS);
		ar7100_spi_raw_output_u8(0);
		rd = (ar7100_reg_rd(AR7100_SPI_RD_STATUS) & 1);
	} while (rd);
}

static void ar7100_spi_send_addr(unsigned int addr)
{
	ar7100_spi_raw_output_u8(((addr & 0xff0000) >> 16));
	ar7100_spi_raw_output_u8(((addr & 0x00ff00) >> 8));
	ar7100_spi_raw_output_u8(addr & 0x0000ff);
}

void ar7100_spi_flash_read_page(unsigned int addr, unsigned char *data, int len)
{
	printk("### %s not implemented \n", __FUNCTION__);
}

void ar7100_spi_flash_write_page(unsigned int addr, unsigned char *data, int len)
{
	int i;
	uint8_t ch;

	ar7100_spi_raw_output_u8(AR7100_SPI_CMD_WREN);
	ar7100_spi_raw_output_u8(AR7100_SPI_CMD_PAGE_PROG);
	ar7100_spi_send_addr(addr);

	for (i = 0; i < len; i++) {
		ch = *(data + i);
		ar7100_spi_raw_output_u8(ch);
	}
	ar7100_reg_wr_nf(AR7100_SPI_WRITE, AR7100_SPI_CS_DIS);
	ar7100_spi_wait_done();
}

void ar7100_spi_flash_sector_erase(unsigned int addr)
{
	ar7100_spi_raw_output_u8(AR7100_SPI_CMD_WREN);
	ar7100_spi_raw_output_u8(AR7100_SPI_CMD_SECTOR_ERASE);
	ar7100_spi_send_addr(addr);
	ar7100_reg_wr_nf(AR7100_SPI_WRITE, AR7100_SPI_CS_DIS);
	ar7100_spi_wait_done();
}

EXPORT_SYMBOL(ar7100_spi_flash_read_page);
EXPORT_SYMBOL(ar7100_spi_flash_write_page);
EXPORT_SYMBOL(ar7100_spi_flash_sector_erase);
