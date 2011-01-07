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

#ifdef CONFIG_SOC_LANTIQ_XWAY

#include "deu.h"

/**
  \defgroup LQ_DEU LQ_DEU_DRIVERS
  \ingroup API
  \brief DEU driver module
*/

/**
  \file deu_danube.c
  \ingroup LQ_DEU
  \brief board specific DEU driver file for danube
*/

/**
  \defgroup BOARD_SPECIFIC_FUNCTIONS LQ_BOARD_SPECIFIC_FUNCTIONS
  \ingroup LQ_DEU
  \brief board specific DEU functions
*/

static int danube_pre_1_4;

#ifdef CONFIG_CRYPTO_DEV_LANTIQ_DMA
u32 *des_buff_in = NULL;
u32 *des_buff_out = NULL;
u32 *aes_buff_in = NULL;
u32 *aes_buff_out = NULL;

struct lq_deu_device lq_deu[1];

static u8 *g_dma_page_ptr = NULL;
static u8 *g_dma_block = NULL;
static u8 *g_dma_block2 = NULL;

/** \fn int dma_init(void)
 *  \ingroup BOARD_SPECIFIC_FUNCTIONS
 *  \brief Initialize DMA for DEU usage. DMA specific registers are
 *         intialized here, including a pointer to the device, memory
 *         space for the device and DEU-DMA descriptors
 *  \return -1 if fail, otherwise return 0
*/
static int dma_init(void)
{
	struct dma_device_info *dma_device = NULL;
	int i = 0;
	volatile struct deu_dma *dma = (struct deu_dma *) LQ_DEU_DMA_CON;
	struct dma_device_info *deu_dma_device_ptr;

	/* get one free page and share between g_dma_block and g_dma_block2 */
	printk("PAGE_SIZE = %ld\n", PAGE_SIZE);
	/* need 16-byte alignment memory block */
	g_dma_page_ptr = (u8 *)__get_free_page(GFP_KERNEL);
	/* need 16-byte alignment memory block */
	g_dma_block = g_dma_page_ptr;
	/* need 16-byte alignment memory block */
	g_dma_block2 = (u8 *)(g_dma_page_ptr + (PAGE_SIZE >> 1));

	deu_dma_device_ptr = dma_device_reserve("DEU");
	if (!deu_dma_device_ptr) {
		printk("DEU: reserve DMA fail!\n");
		return -1;
	}
	lq_deu[0].dma_device = deu_dma_device_ptr;
	dma_device = deu_dma_device_ptr;
	/* dma_device->priv = &deu_dma_priv; */
	dma_device->buffer_alloc = &deu_dma_buffer_alloc;
	dma_device->buffer_free = &deu_dma_buffer_free;
	dma_device->intr_handler = &deu_dma_intr_handler;
	dma_device->tx_endianness_mode = LQ_DMA_ENDIAN_TYPE3;
	dma_device->rx_endianness_mode = LQ_DMA_ENDIAN_TYPE3;
	dma_device->port_num = 1;
	dma_device->tx_burst_len = 4;
	dma_device->max_rx_chan_num = 1;
	dma_device->max_tx_chan_num = 1;
	dma_device->port_packet_drop_enable = 0;

	for (i = 0; i < dma_device->max_rx_chan_num; i++) {
		dma_device->rx_chan[i]->packet_size = DEU_MAX_PACKET_SIZE;
		dma_device->rx_chan[i]->desc_len = 1;
		dma_device->rx_chan[i]->control = LQ_DMA_CH_ON;
		dma_device->rx_chan[i]->byte_offset = 0;
		dma_device->rx_chan[i]->chan_poll_enable = 1;

	}

	for (i = 0; i < dma_device->max_tx_chan_num; i++) {
		dma_device->tx_chan[i]->control = LQ_DMA_CH_ON;
		dma_device->tx_chan[i]->desc_len = 1;
		dma_device->tx_chan[i]->chan_poll_enable = 1;
	}

	dma_device->current_tx_chan = 0;
	dma_device->current_rx_chan = 0;

	dma_device_register(dma_device);
	for (i = 0; i < dma_device->max_rx_chan_num; i++) {
		(dma_device->rx_chan[i])->open(dma_device->rx_chan[i]);
	}

	dma->ctrl.BS = 0;
	dma->ctrl.RXCLS = 0;
	dma->ctrl.EN = 1;


	*LQ_DMA_PS = 1;

	/* DANUBE PRE 1.4 SOFTWARE FIX */
	if (danube_pre_1_4)
		*LQ_DMA_PCTRL = 0x14;
	else
		*LQ_DMA_PCTRL = 0xF14;

	return 0;
}

/** \fn u32 *dma_align(const u8 *arg, u32 *buffer_alloc, int in_buff, int nbytes)
 *  \ingroup BOARD_SPECIFIC_FUNCTIONS
 *  \brief  A fix to align mis-aligned address for Danube version 1.3 chips
 *          which has memory alignment issues.
 *  \param arg Pointer to the input / output memory address
 *  \param buffer_alloc A pointer to the buffer
 *  \param in_buff Input (if == 1) or Output (if == 0) buffer
 *  \param nbytes Number of bytes of data
 *  \return returns arg: if address is aligned, buffer_alloc: if memory address is not aligned
*/
static u32 *dma_align(const u8 *arg, u32 *buffer_alloc, int in_buff, int nbytes)
{
	if (danube_pre_1_4) {
		/* for input buffer */
		if (in_buff) {
			if (((u32) arg) & 0xF) {
				memcpy(buffer_alloc, arg, nbytes);
				return (u32 *) buffer_alloc;
			} else {
				return (u32 *) arg;
			}
		}
		else {
			/* for output buffer */
			if (((u32) arg) & 0x3)
				return buffer_alloc;
			else
				return (u32 *) arg;
		}
	}

	return (u32 *) arg;
}

/** \fn  void aes_dma_memcpy(u32 *outcopy, u32 *out_dma, u8 *out_arg, int nbytes)
 *  \ingroup BOARD_SPECIFIC_FUNCTIONS
 *  \brief copy the DMA data to the memory address space for AES. The swaping
 *         of the 4 bytes is done only for Danube version 1.3 (FIX). Otherwise,
 *         it is a direct memory copy to out_arg pointer
 *  \param outcopy Pointer to the address to store swapped copy
 *  \param out_dma A pointer to the memory address that stores the DMA data
 *  \param out_arg The pointer to the memory address that needs to be copied to
 *  \param nbytes Number of bytes of data
*/
static void aes_dma_memcpy(u32 *outcopy, u32 *out_dma, u8 *out_arg, int nbytes)
{
	int i = 0;
	int x = 0;

	/* DANUBE PRE 1.4 SOFTWARE FIX */
	if (danube_pre_1_4) {
		for (i = 0; i < (nbytes / 4); i++) {
			x = i ^ 0x3;
			outcopy[i] = out_dma[x];

		}
		if (((u32) out_arg) & 0x3) {
			memcpy((u8 *)out_arg, outcopy, nbytes);
		}
	} else {
		memcpy(out_arg, out_dma, nbytes);
	}
}

/** \fn void des_dma_memcpy(u32 *outcopy, u32 *out_dma, u8 *out_arg, int nbytes)
 *  \ingroup BOARD_SPECIFIC_FUNCTIONS
 *  \brief copy the DMA data to the memory address space for DES. The swaping
 *         of the 4 bytes is done only for Danube version 1.3 (FIX). Otherwise,
 *         it is a direct memory copy to out_arg pointer
 *  \param outcopy Pointer to the address to store swapped copy
 *  \param out_dma A pointer to the memory address that stores the DMA data
 *  \param out_arg The pointer to the memory address that needs to be copied to
 *  \param nbytes Number of bytes of data
*/
static void des_dma_memcpy(u32 *outcopy, u32 *out_dma, u8 *out_arg, int nbytes)
{
	int i = 0;
	int x = 0;

	/* DANUBE PRE 1.4 SOFTWARE FIX */
	if (danube_pre_1_4) {
		for (i = 0; i < (nbytes / 4); i++) {
			x = i ^ 1;
			outcopy[i] = out_dma[x];

		}
		if (((u32) out_arg) & 0x3) {
			memcpy((u8 *)out_arg, outcopy, nbytes);
		}
	} else {
		memcpy(out_arg, out_dma, nbytes);
	}
}

/** \fn int des_dma_malloc(int value)
 *  \ingroup BOARD_SPECIFIC_FUNCTIONS
 *  \brief allocates memory to the necessary memory input/output buffer
 *         location, used during the DES algorithm DMA transfer (memory
 *         alignment issues)
 *  \param  value value determinds whether the calling of the function is for a
 *          input buffer or for an output buffer memory allocation
*/
static int des_dma_malloc(int value)
{
	if (danube_pre_1_4) {
		if (value == BUFFER_IN) {
			des_buff_in = kmalloc(DEU_MAX_PACKET_SIZE, GFP_ATOMIC);
			if (!des_buff_in)
				return -1;
			else
				return 0;
		}
		else {
			des_buff_out = kmalloc(DEU_MAX_PACKET_SIZE, GFP_ATOMIC);
			if (!des_buff_out)
				return -1;
			else
				return 0;
		}
	} else {
		return 0;
	}
}

/** \fn int aes_dma_malloc(int value)
 *  \ingroup BOARD_SPECIFIC_FUNCTIONS
 *  \brief allocates memory to the necessary memory input/output buffer
 *         location, used during the AES algorithm DMA transfer (memory
 *         alignment issues)
 *  \param value value determinds whether the calling of the function is for a
 *         input buffer or for an output buffer memory allocation
*/
static int aes_dma_malloc(int value)
{
	if (danube_pre_1_4) {
		if (value == BUFFER_IN) {
			aes_buff_in = kmalloc(DEU_MAX_PACKET_SIZE, GFP_ATOMIC);
			if (!aes_buff_in)
				return -1;
			else
				return 0;
		}
		else {
			aes_buff_out = kmalloc(DEU_MAX_PACKET_SIZE, GFP_ATOMIC);
			if (!aes_buff_out)
				return -1;
			else
				return 0;
		}
	} else {
		return 0;
	}
}

/** \fn void dma_free(u32 *addr)
 *  \ingroup BOARD_SPECIFIC_FUNCTIONS
 *  \brief frees previously allocated memory
 *  \param addr memory address of the buffer that needs to be freed
*/
static void dma_free(u32 *addr)
{
	if (addr)
		kfree(addr);
	return;
}

/** \fn dma_exit(void)
 *  \ingroup BOARD_SPECIFIC_FUNCTIONS
 *  \brief unregister dma devices after exit
*/
static void dma_exit(void)
{
	if (g_dma_page_ptr)
		free_page((u32) g_dma_page_ptr);
	dma_device_release(lq_deu[0].dma_device);
	dma_device_unregister(lq_deu[0].dma_device);
}
#endif /* CONFIG_CRYPTO_DEV_LANTIQ_DMA */

/** \fn u32 endian_swap(u32 input)
 *  \ingroup BOARD_SPECIFIC_FUNCTIONS
 *  \brief function is not used
 *  \param input Data input to be swapped
 *  \return input
*/
static u32 endian_swap(u32 input)
{
	return input;
}

/** \fn u32 input_swap(u32 input)
 *  \ingroup BOARD_SPECIFIC_FUNCTIONS
 *  \brief Swap the input data if the current chip is Danube version
 *         1.4 and do nothing to the data if the current chip is
 *         Danube version 1.3
 *  \param input data that needs to be swapped
 *  \return input or swapped input
*/
static u32 input_swap(u32 input)
{
	if (!danube_pre_1_4) {
		u8 *ptr = (u8 *)&input;
		return ((ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0]);
	} else {
		return input;
	}
}

/** \fn void aes_chip_init(void)
 *  \ingroup BOARD_SPECIFIC_FUNCTIONS
 * \brief initialize AES hardware
*/
static void aes_chip_init(void)
{
	volatile struct deu_aes *aes = (struct deu_aes *) AES_START;

#ifndef CONFIG_CRYPTO_DEV_LANTIQ_DMA
	/* start crypto engine with write to ILR */
	aes->ctrl.SM = 1;
	aes->ctrl.ARS = 1;
#else
	aes->ctrl.SM = 1;
	aes->ctrl.ARS = 1; /* 0 for dma */
#endif
}

/** \fn void des_chip_init(void)
 *  \ingroup BOARD_SPECIFIC_FUNCTIONS
 *  \brief initialize DES hardware
*/
static void des_chip_init(void)
{
	volatile struct deu_des *des = (struct deu_des *) DES_3DES_START;

#ifndef CONFIG_CRYPTO_DEV_LANTIQ_DMA
	/* start crypto engine with write to ILR */
	des->ctrl.SM = 1;
	des->ctrl.ARS = 1;
#else
	des->ctrl.SM = 1;
	des->ctrl.ARS = 1; /* 0 for dma */
#endif
}

/** \fn void deu_chip_version(void)
 *  \ingroup LQ_DES_FUNCTIONS
 *  \brief To find the version of the chip by looking at the chip ID
 *  \param danube_pre_1_4 (sets to 1 if Chip is Danube less than v1.4)
*/
static void deu_chip_version(void)
{
	/* DANUBE PRE 1.4 SOFTWARE FIX */
	int chip_id = 0;
	chip_id = *LQ_MPS_CHIPID;
	chip_id >>= 28;

	if (chip_id >= 4) {
		danube_pre_1_4 = 0;
		printk("Danube Chip ver. 1.4 detected. \n");
	}
	else {
		danube_pre_1_4 = 1;
		printk("Danube Chip ver. 1.3 or below detected. \n");
	}
}

static u32 chip_init(void)
{
	volatile struct deu_clk_ctrl *clc = (struct deu_clk_ctrl *) LQ_DEU_CLK;

#if 0
	lq_pmu_enable(1<<20);
#endif

	deu_chip_version();

	clc->FSOE = 0;
	clc->SBWE = 0;
	clc->SPEN = 0;
	clc->SBWE = 0;
	clc->DISS = 0;
	clc->DISR = 0;

	return *LQ_DEU_ID;
}

static int lq_crypto_probe(struct platform_device *pdev)
{
#ifdef CONFIG_CRYPTO_DEV_LANTIQ_DMA
	lq_crypto_ops.dma_init = dma_init;
	lq_crypto_ops.dma_exit = dma_exit;
	lq_crypto_ops.aes_dma_memcpy = aes_dma_memcpy;
	lq_crypto_ops.des_dma_memcpy = des_dma_memcpy;
	lq_crypto_ops.aes_dma_malloc = aes_dma_malloc;
	lq_crypto_ops.des_dma_malloc = des_dma_malloc;
	lq_crypto_ops.dma_align = dma_align;
	lq_crypto_ops.dma_free = dma_free;
#endif

	lq_crypto_ops.endian_swap = endian_swap;
	lq_crypto_ops.input_swap = input_swap;
	lq_crypto_ops.aes_chip_init = aes_chip_init;
	lq_crypto_ops.des_chip_init = des_chip_init;
	lq_crypto_ops.chip_init = chip_init;

	printk("lq_danube_deu: driver loaded!\n");

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
		.name	= "lq_danube_deu"
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
