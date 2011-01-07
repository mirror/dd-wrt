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
  \ingroup  API
  \brief Lantiq DEU driver module
*/

/**
  \file deu.h
  \brief Main DEU driver header file
*/

/**
  \defgroup LQ_DEU_DEFINITIONS LQ_DEU_DEFINITIONS
  \ingroup  LQ_DEU
  \brief Lantiq DEU definitions
*/


#ifndef DEU_H
#define DEU_H

#undef CRYPTO_DEBUG

#define LQ_DEU_DRV_VERSION		"1.0.1"

#if defined(CONFIG_LANTIQ_DANUBE)
#	include "deu_danube.h"
#elif defined(CONFIG_LANTIQ_AR9)
#	include "deu_ar9.h"
#elif defined(CONFIG_SOC_LANTIQ_FALCON)
#	include "deu_falcon.h"
#else
//#	error "Unknown platform"
#	include "deu_danube.h"
#endif

struct lq_crypto_priv {
#ifdef CONFIG_CRYPTO_DEV_LANTIQ_DMA
	u32 *des_buff_in;
	u32 *des_buff_out;
	u32 *aes_buff_in;
	u32 *aes_buff_out;

	int (*dma_init)(void);
	void (*dma_exit)(void);
	u32 (*dma_align)(const u8 *, u32 *, int, int);
	void (*aes_dma_memcpy)(u32 *, u32 *, u8 *, int);
	void (*des_dma_memcpy)(u32 *, u32 *, u8 *, int);
	int (*aes_dma_malloc)(int);
	int (*des_dma_malloc)(int);
	void (*dma_free)(u32 *);
#endif

	u32 (*endian_swap)(u32);
	u32 (*input_swap)(u32);
	void (*aes_chip_init)(void);
	void (*des_chip_init)(void);
	u32 (*chip_init)(void);
};

extern struct lq_crypto_priv lq_crypto_ops;

#define LQ_DEU_ALIGNMENT		16

#define PFX				"lq_deu: "

#define LQ_DEU_CRA_PRIORITY		300
#define LQ_DEU_COMPOSITE_PRIORITY	400

#define CRYPTO_DIR_ENCRYPT		1
#define CRYPTO_DIR_DECRYPT		0

#define CRTCL_SECT_INIT		spin_lock_init(&cipher_lock)
#define CRTCL_SECT_START	spin_lock_irqsave(&cipher_lock, flag)
#define CRTCL_SECT_END		spin_unlock_irqrestore(&cipher_lock, flag)

#define LQ_DEU_ID_REV		0x00001F
#define LQ_DEU_ID_ID		0x00FF00
#define LQ_DEU_ID_DMA		0x010000
#define LQ_DEU_ID_HASH		0x020000
#define LQ_DEU_ID_AES		0x040000
#define LQ_DEU_ID_3DES		0x080000
#define LQ_DEU_ID_DES		0x100000

extern int disable_deudma;

int lq_deu_init(void);
void lq_deu_exit(void);

int lq_deu_init_des(void);
int lq_deu_init_aes(void);
int lq_deu_init_arc4(void);
int lq_deu_init_sha1(void);
int lq_deu_init_md5(void);
int lq_deu_init_sha1_hmac(void);
int lq_deu_init_md5_hmac(void);

void lq_deu_fini_des(void);
void lq_deu_fini_aes(void);
void lq_deu_fini_arc4(void);
void lq_deu_fini_sha1(void);
void lq_deu_fini_md5(void);
void lq_deu_fini_sha1_hmac(void);
void lq_deu_fini_md5_hmac(void);

/* board specific functions */
/* { */
static inline u32 deu_chip_init(void)
{
	return lq_crypto_ops.chip_init();
}

static inline void deu_des_chip_init(void)
{
	lq_crypto_ops.des_chip_init();
}

static inline void deu_aes_chip_init(void)
{
	lq_crypto_ops.aes_chip_init();
}

static inline u32 deu_input_swap(u32 input)
{
	return lq_crypto_ops.input_swap(input);
}

static inline u32 deu_endian_swap(u32 input)
{
	return lq_crypto_ops.endian_swap(input);
}

#ifdef CONFIG_CRYPTO_DEV_LANTIQ_DMA
static inline int deu_aes_dma_malloc(int value)
{
	return lq_crypto_ops.aes_dma_malloc(value);
}

static inline int deu_des_dma_malloc(int value)
{
	return lq_crypto_ops.des_dma_malloc(value);
}

static inline u32 *deu_dma_align(const u8 *arg,
				  u32 *buff_alloc,
				  int in_out,
				  int nbytes)
{
	return lq_crypto_ops.dma_align(arg, buff_alloc, in_out, nbytes);
}

static inline void deu_aes_dma_memcpy(u32 *outcopy,
				       u32 *out_dma,
				       u8 *out_arg,
				       int nbytes)
{
	lq_crypto_ops.aes_dma_memcpy(outcopy, out_dma, out_arg, nbytes);
}

static inline void deu_des_dma_memcpy(u32 *outcopy,
				       u32 *out_dma,
				       u8 *out_arg,
				       int nbytes)
{
	lq_crypto_ops.des_dma_memcpy(outcopy, out_dma, out_arg, nbytes);
}

static inline void deu_dma_free(u32 *addr)
{
	lq_crypto_ops.dma_free(addr);
}

static inline int deu_dma_init(void)
{
	lq_crypto_ops.dma_init();
}

static inline void deu_dma_exit(void)
{
	lq_crypto_ops.dma_exit();
}
#endif

/* } */

#define DEU_WAKELIST_INIT(queue) \
	init_waitqueue_head(&queue)

#define DEU_WAIT_EVENT_TIMEOUT(queue, event, flags, timeout) \
	do { \
		wait_event_interruptible_timeout((queue), \
						 test_bit((event), \
						 &(flags)), (timeout)); \
		clear_bit((event), &(flags)); \
	}while (0)


#define DEU_WAKEUP_EVENT(queue, event, flags) \
	do { \
		set_bit((event), &(flags)); \
		wake_up_interruptible(&(queue)); \
	}while (0)

#define DEU_WAIT_EVENT(queue, event, flags) \
	do { \
		wait_event_interruptible(queue, \
					 test_bit((event), &(flags))); \
		clear_bit((event), &(flags)); \
	}while (0)

struct deu_drv_priv {
	wait_queue_head_t  deu_thread_wait;
#define DEU_EVENT	1
	volatile  long     deu_event_flags;
	u8                 *deu_rx_buf;
	u32                deu_rx_len;
};

#ifdef CRYPTO_DEBUG
extern char deu_debug_level;
#	define DPRINTF(level, format, args...) \
	if (level < deu_debug_level) \
		printk(KERN_INFO "[%s %s %d]: " format, \
		       __FILE__, __func__, __LINE__, ##args)
#else
#	define DPRINTF(level, format, args...) do { } while(0)
#endif

#endif	/* DEU_H */
