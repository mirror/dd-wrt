/*
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 *   Copyright (C) 2010 Ralph Hempel <ralph.hempel@lantiq.com>
 *   Copyright (C) 2009 Mohammad Firdaus
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <asm/io.h> /* dma_cache_inv */
#include <linux/platform_device.h>

#ifdef CONFIG_SOC_LANTIQ_FALCON

#include "deu.h"

/**
  \defgroup LQ_DEU LQ_DEU_DRIVERS
  \ingroup API
  \brief Lantiq DEU driver module
*/

/**
  \file deu_falcon.c
  \brief Lantiq DEU board specific driver file for ar9
*/

/**
 \defgroup BOARD_SPECIFIC_FUNCTIONS LQ_BOARD_SPECIFIC_FUNCTIONS
 \ingroup LQ_DEU
 \brief board specific functions
*/

#include <falcon/gpon_reg_base.h>
#include <falcon/sys1_reg.h>
#include <falcon/status_reg.h>
#include <falcon/sysctrl.h>

#define reg_r32(reg)			__raw_readl(reg)
#define reg_w32(val, reg)		__raw_writel(val, reg)
#define reg_w32_mask(clear, set, reg)	reg_w32((reg_r32(reg) & ~(clear)) | (set), reg)

static gpon_sys1_t * const sys1 = (gpon_sys1_t *)GPON_SYS1_BASE;
static gpon_status_t * const status = (gpon_status_t *)GPON_STATUS_BASE;

/** \fn u32 endian_swap(u32 input)
 *  \ingroup BOARD_SPECIFIC_FUNCTIONS
 *  \brief Swap data given to the function
 *  \param input   Data input to be swapped
 *  \return either the swapped data or the input data depending on whether it is in DMA mode or FPI mode
*/
static u32 endian_swap(u32 input)
{
	return input;
}

/** \fn u32 input_swap(u32 input)
 *  \ingroup BOARD_SPECIFIC_FUNCTIONS
 *  \brief Not used
 *  \return input
*/
static u32 input_swap(u32 input)
{
	return input;
}

/** \fn void aes_chip_init(void)
 *  \ingroup BOARD_SPECIFIC_FUNCTIONS
 *  \brief initialize AES hardware
*/
static void aes_chip_init(void)
{
	volatile struct deu_aes *aes = (struct deu_aes *) AES_START;

	aes->ctrl.SM = 1;
	aes->ctrl.ARS = 1;
}

/** \fn void des_chip_init(void)
 *  \ingroup BOARD_SPECIFIC_FUNCTIONS
 *  \brief initialize DES hardware
*/
static void des_chip_init(void)
{
}

static u32 chip_init(void)
{
	sys1_hw_clk_enable(CLKEN_SHA1_SET | CLKEN_AES_SET);
	sys1_hw_activate(ACT_SHA1_SET | ACT_AES_SET);

	return LQ_DEU_ID_AES | LQ_DEU_ID_HASH;
}

static int lq_crypto_probe(struct platform_device *pdev)
{
#ifdef CONFIG_CRYPTO_DEV_LANTIQ_DMA
	lq_crypto_ops.dma_init = NULL;
	lq_crypto_ops.dma_exit = NULL;
	lq_crypto_ops.aes_dma_memcpy = NULL;
	lq_crypto_ops.des_dma_memcpy = NULL;
	lq_crypto_ops.aes_dma_malloc = NULL;
	lq_crypto_ops.des_dma_malloc = NULL;
	lq_crypto_ops.dma_align = NULL;
	lq_crypto_ops.dma_free = NULL;
#endif

	lq_crypto_ops.endian_swap = endian_swap;
	lq_crypto_ops.input_swap = input_swap;
	lq_crypto_ops.aes_chip_init = aes_chip_init;
	lq_crypto_ops.des_chip_init = des_chip_init;
	lq_crypto_ops.chip_init = chip_init;

	printk("lq_falcon_deu: driver loaded!\n");

	lq_deu_init();

	return 0;
}

static int lq_crypto_remove(struct platform_device *pdev)
{
	lq_deu_exit();

	return 0;
}

static struct platform_driver lq_crypto = {
	.probe	= lq_crypto_probe,
	.remove	= lq_crypto_remove,
	.driver	= {
		.owner	= THIS_MODULE,
		.name	= "lq_falcon_deu"
	}
};

static int __init lq_crypto_init(void)
{
	return platform_driver_register(&lq_crypto);
}
module_init(lq_crypto_init);

static void __exit lq_crypto_exit(void)
{
	platform_driver_unregister(&lq_crypto);
}
module_exit(lq_crypto_exit);

#endif
