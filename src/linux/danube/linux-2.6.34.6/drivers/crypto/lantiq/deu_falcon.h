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
 *   Copyright (C) 2009 Mohammad Firdaus / Infineon Technologies
 */

/**
  \defgroup LQ_DEU LQ_DEU_DRIVERS
  \ingroup API
  \brief DEU driver module
*/

/**
  \defgroup LQ_DEU_DEFINITIONS LQ_DEU_DEFINITIONS
  \ingroup LQ_DEU
  \brief Lantiq DEU definitions
*/

/**
  \file deu_falcon.h
  \brief DEU driver header file
*/


#ifndef DEU_FALCON_H
#define DEU_FALCON_H

#define HASH_START			0xbd008100
#define AES_START			0xbd008000

#ifdef CONFIG_CRYPTO_DEV_DMA
#	include "deu_dma.h"
#	define DEU_DWORD_REORDERING(ptr, buffer, in_out, bytes) \
		deu_dma_align(ptr, buffer, in_out, bytes)
#	define AES_MEMORY_COPY(outcopy, out_dma, out_arg, nbytes) \
		deu_aes_dma_memcpy(outcopy, out_dma, out_arg, nbytes)
#	define DES_MEMORY_COPY(outcopy, out_dma, out_arg, nbytes) \
		deu_des_dma_memcpy(outcopy, out_dma, out_arg, nbytes)
#	define BUFFER_IN			1
#	define BUFFER_OUT			0
#	define AES_ALGO				1
#	define DES_ALGO				0
#	define ALLOCATE_MEMORY(val, type)	1
#	define FREE_MEMORY(buff)
extern struct lq_deu_device lq_deu[1];
#endif /* CONFIG_CRYPTO_DEV_DMA */

/* SHA CONSTANTS */
#define HASH_CON_VALUE				0x0700002C

#define INPUT_ENDIAN_SWAP(input)		deu_input_swap(input)
#define DEU_ENDIAN_SWAP(input)			deu_endian_swap(input)
#define DELAY_PERIOD				10
#define FIND_DEU_CHIP_VERSION			chip_version()

#define WAIT_AES_DMA_READY() \
	do { \
		int i; \
		volatile struct deu_dma *dma = \
			(struct deu_dma *) LQ_DEU_DMA_CON; \
		volatile struct deu_aes *aes = \
			(volatile struct deu_aes *) AES_START; \
		for (i = 0; i < 10; i++) \
			udelay(DELAY_PERIOD); \
		while (dma->ctrl.BSY) {}; \
		while (aes->ctrl.BUS) {}; \
	} while (0)

#define WAIT_DES_DMA_READY() \
	do { \
		int i; \
		volatile struct deu_dma *dma = \
			(struct deu_dma *) LQ_DEU_DMA_CON; \
		volatile struct deu_des *des = \
			(struct deu_des *) DES_3DES_START; \
		for (i = 0; i < 10; i++) \
			udelay(DELAY_PERIOD); \
		while (dma->ctrl.BSY) {}; \
		while (des->ctrl.BUS) {}; \
	} while (0)

#define AES_DMA_MISC_CONFIG() \
	do { \
		volatile struct deu_aes *aes = \
			(volatile struct deu_aes *) AES_START; \
		aes->ctrl.KRE = 1; \
		aes->ctrl.GO = 1; \
	} while(0)

#define SHA_HASH_INIT \
	do { \
		volatile struct deu_hash *hash = \
			(struct deu_hash *) HASH_START; \
		hash->ctrl.SM = 1; \
		hash->ctrl.ALGO = 0; \
		hash->ctrl.INIT = 1; \
	} while(0)

/* DEU Common Structures for Falcon*/

struct deu_clk_ctrl {
	u32 Res:26;
	u32 FSOE:1;
	u32 SBWE:1;
	u32 EDIS:1;
	u32 SPEN:1;
	u32 DISS:1;
	u32 DISR:1;
};

struct deu_des {
	struct deu_des_ctrl {	/* 10h */
		u32 KRE:1;
		u32 reserved1:5;
		u32 GO:1;
		u32 STP:1;
		u32 Res2:6;
		u32 NDC:1;
		u32 ENDI:1;
		u32 Res3:2;
		u32 F:3;
		u32 O:3;
		u32 BUS:1;
		u32 DAU:1;
		u32 ARS:1;
		u32 SM:1;
		u32 E_D:1;
		u32 M:3;
	} ctrl;

	u32 IHR;		/* 14h */
	u32 ILR;		/* 18h */
	u32 K1HR;		/* 1c */
	u32 K1LR;
	u32 K2HR;
	u32 K2LR;
	u32 K3HR;
	u32 K3LR;		/* 30h */
	u32 IVHR;		/* 34h */
	u32 IVLR;		/* 38 */
	u32 OHR;		/* 3c */
	u32 OLR;		/* 40 */
};

struct deu_aes {
	struct deu_aes_ctrl {
		u32 KRE:1;
		u32 reserved1:4;
		u32 PNK:1;
		u32 GO:1;
		u32 STP:1;
		u32 reserved2:6;
		u32 NDC:1;
		u32 ENDI:1;
		u32 reserved3:2;
		u32 F:3;	/* fbs */
		u32 O:3;	/* om */
		u32 BUS:1;	/* bsy */
		u32 DAU:1;
		u32 ARS:1;
		u32 SM:1;
		u32 E_D:1;
		u32 KV:1;
		u32 K:2;	/* KL */
	} ctrl;

	u32 ID3R;		/* 80h */
	u32 ID2R;		/* 84h */
	u32 ID1R;		/* 88h */
	u32 ID0R;		/* 8Ch */
	u32 K7R;		/* 90h */
	u32 K6R;		/* 94h */
	u32 K5R;		/* 98h */
	u32 K4R;		/* 9Ch */
	u32 K3R;		/* A0h */
	u32 K2R;		/* A4h */
	u32 K1R;		/* A8h */
	u32 K0R;		/* ACh */
	u32 IV3R;		/* B0h */
	u32 IV2R;		/* B4h */
	u32 IV1R;		/* B8h */
	u32 IV0R;		/* BCh */
	u32 OD3R;		/* D4h */
	u32 OD2R;		/* D8h */
	u32 OD1R;		/* DCh */
	u32 OD0R;		/* E0h */
};

struct deu_arc4 {
	struct arc4_controlr {
		u32 KRE:1;
		u32 KLEN:4;
		u32 KSAE:1;
		u32 GO:1;
		u32 STP:1;
		u32 reserved1:6;
		u32 NDC:1;
		u32 ENDI:1;
		u32 reserved2:8;
		u32 BUS:1;	/* bsy */
		u32 reserved3:1;
		u32 ARS:1;
		u32 SM:1;
		u32 reserved4:4;
	} ctrl;

	u32 K3R;		/* 104h */
	u32 K2R;		/* 108h */
	u32 K1R;		/* 10Ch */
	u32 K0R;		/* 110h */
	u32 IDLEN;		/* 114h */
	u32 ID3R;		/* 118h */
	u32 ID2R;		/* 11Ch */
	u32 ID1R;		/* 120h */
	u32 ID0R;		/* 124h */
	u32 OD3R;		/* 128h */
	u32 OD2R;		/* 12Ch */
	u32 OD1R;		/* 130h */
	u32 OD0R;		/* 134h */
};

struct deu_hash {
	struct deu_hash_ctrl {
		u32 reserved1:5;
		u32 KHS:1;
		u32 GO:1;
		u32 INIT:1;
		u32 reserved2:6;
		u32 NDC:1;
		u32 ENDI:1;
		u32 reserved3:7;
		u32 DGRY:1;
		u32 BSY:1;
		u32 reserved4:1;
		u32 IRCL:1;
		u32 SM:1;
		u32 KYUE:1;
		u32 HMEN:1;
		u32 SSEN:1;
		u32 ALGO:1;
	} ctrl;

	u32 MR;			/* B4h */
	u32 D1R;		/* B8h */
	u32 D2R;		/* BCh */
	u32 D3R;		/* C0h */
	u32 D4R;		/* C4h */
	u32 D5R;		/* C8h */
	u32 dummy;		/* CCh */
	u32 KIDX;		/* D0h */
	u32 KEY;		/* D4h */
	u32 DBN;		/* D8h */
};

struct deu_dma {
	struct deu_dma_ctrl {
		u32 reserved1:22;
		u32 BS:2;
		u32 BSY:1;
		u32 reserved2:1;
		u32 ALGO:2;
		u32 RXCLS:2;
		u32 reserved3:1;
		u32 EN:1;
	} ctrl;
};

#endif /* DEU_FALCON_H */
