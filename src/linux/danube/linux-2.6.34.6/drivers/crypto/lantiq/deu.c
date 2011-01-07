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

/**
  \defgroup LQ_DEU LQ_DEU_DRIVERS
  \ingroup API
  \brief Lantiq DEU driver module
*/

/**
  \file deu.c
  \ingroup LQ_DEU
  \brief main DEU driver file
*/

/**
 \defgroup LQ_DEU_FUNCTIONS LQ_DEU_FUNCTIONS
 \ingroup LQ_DEU
 \brief Lantiq DEU functions
*/

#include <linux/version.h>
#if defined(CONFIG_MODVERSIONS)
#define MODVERSIONS
#include <linux/modversions.h>
#endif
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/crypto.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>       /* Stuff about file systems that we need */
#include <asm/byteorder.h>

#if 0
#ifdef CONFIG_SOC_LANTIQ_XWAY
#	include <lq_pmu.h>
#endif
#endif

#include "deu.h"

struct lq_crypto_priv lq_crypto_ops;

#ifdef CONFIG_CRYPTO_DEV_LANTIQ_DMA
int disable_deudma = 0;
#else
int disable_deudma = 1;
#endif /* CONFIG_CRYPTO_DEV_LANTIQ_DMA */

#ifdef CRYPTO_DEBUG
char deu_debug_level = 3;
#endif

#ifdef CONFIG_CRYPTO_DEV_LANTIQ_MODULE
#	define STATIC static
#else
#	define STATIC
#endif

/** \fn static int lq_deu_init(void)
 *  \ingroup LQ_DEU_FUNCTIONS
 *  \brief link all modules that have been selected in kernel config for Lantiq HW crypto support
 *  \return ret
*/
int lq_deu_init(void)
{
	int ret = -ENOSYS;
	u32 config;

	printk(KERN_INFO "Lantiq crypto hardware driver version %s\n",
	       LQ_DEU_DRV_VERSION);

	config = deu_chip_init();

#ifdef CONFIG_CRYPTO_DEV_LANTIQ_DMA
	deu_dma_init();
#endif

#if defined(CONFIG_CRYPTO_DEV_LANTIQ_AES)
	if(config & LQ_DEU_ID_AES) {
		if ((ret = lq_deu_init_aes())) {
			printk(KERN_ERR "Lantiq AES initialization failed!\n");
		}
	} else {
		printk(KERN_ERR "Lantiq AES not supported!\n");
	}
#endif

#ifdef CONFIG_SOL_LANTIQ_XWAY
#if defined(CONFIG_CRYPTO_DEV_LANTIQ_DES)
	if(config & LQ_DEU_ID_DES) {
		if ((ret = lq_deu_init_des())) {
			printk(KERN_ERR "Lantiq DES initialization failed!\n");
		}
	} else {
		printk(KERN_ERR "Lantiq DES not supported!\n");
	}
#endif
#if defined(CONFIG_CRYPTO_DEV_LANTIQ_ARC4) && defined(CONFIG_CRYPTO_DEV_LANTIQ_DMA)
	if ((ret = lq_deu_init_arc4())) {
		printk(KERN_ERR "Lantiq ARC4 initialization failed!\n");
	}
#endif
#endif

#if defined(CONFIG_CRYPTO_DEV_LANTIQ_SHA1)
	if(config & LQ_DEU_ID_HASH) {
		if ((ret = lq_deu_init_sha1())) {
			printk(KERN_ERR "Lantiq SHA1 initialization failed!\n");
		}
	} else {
		printk(KERN_ERR "Lantiq SHA1 not supported!\n");
	}
#endif
#if defined(CONFIG_CRYPTO_DEV_LANTIQ_MD5)
	if(config & LQ_DEU_ID_HASH) {
		if ((ret = lq_deu_init_md5())) {
			printk(KERN_ERR "Lantiq MD5 initialization failed!\n");
		}
	} else {
		printk(KERN_ERR "Lantiq MD5 not supported!\n");
	}
#endif
#if defined(CONFIG_CRYPTO_DEV_LANTIQ_SHA1_HMAC)
	if ((ret = lq_deu_init_sha1_hmac())) {
		printk(KERN_ERR "Lantiq SHA1_HMAC initialization failed!\n");
	}
#endif
#if defined(CONFIG_CRYPTO_DEV_LANTIQ_MD5_HMAC)
	if ((ret = lq_deu_init_md5_hmac())) {
		printk(KERN_ERR "Lantiq MD5_HMAC initialization failed!\n");
	}
#endif
	return ret;
}

/** \fn static void lq_deu_fini(void)
 *  \ingroup LQ_DEU_FUNCTIONS
 *  \brief remove the loaded crypto algorithms
*/
void lq_deu_exit(void)
{
#if defined(CONFIG_CRYPTO_DEV_LANTIQ_AES)
	lq_deu_fini_aes();
#endif
#ifdef CONFIG_SOL_LANTIQ_XWAY
#if defined(CONFIG_CRYPTO_DEV_LANTIQ_DES)
	lq_deu_fini_des();
#endif
#if defined(CONFIG_CRYPTO_DEV_LANTIQ_ARC4) \
	&& defined(CONFIG_CRYPTO_DEV_LANTIQ_DMA)
	lq_deu_fini_arc4();
#endif
#endif
#if defined(CONFIG_CRYPTO_DEV_LANTIQ_SHA1)
	lq_deu_fini_sha1();
#endif
#if defined(CONFIG_CRYPTO_DEV_LANTIQ_MD5)
	lq_deu_fini_md5();
#endif
#if defined(CONFIG_CRYPTO_DEV_LANTIQ_SHA1_HMAC)
	lq_deu_fini_sha1_hmac();
#endif
#if defined(CONFIG_CRYPTO_DEV_LANTIQ_MD5_HMAC)
	lq_deu_fini_md5_hmac();
#endif

	printk("DEU has exited successfully\n");

#if defined(CONFIG_CRYPTO_DEV_LANTIQ_DMA)
	deu_dma_exit();
	printk("DMA has deregistered successfully\n");
#endif
}

EXPORT_SYMBOL(lq_deu_init);
EXPORT_SYMBOL(lq_deu_exit);
