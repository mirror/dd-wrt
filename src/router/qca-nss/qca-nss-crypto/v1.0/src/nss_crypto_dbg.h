/* Copyright (c) 2013, 2016, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 *
 *
 */
#ifndef __NSS_CRYPTO_DBG_H
#define __NSS_CRYPTO_DBG_H

#if defined (CONFIG_NSS_CRYPTO_DBG)
#define nss_crypto_assert(expr)		BUG_ON(!(expr))
#define nss_crypto_dbg(fmt, arg...)	printk(KERN_DEBUG fmt, ## arg)

/*
 * nss_crypto_dump_desc()
 * 	dump the crypto descriptor, use the string to identify the dump
 */
static inline void nss_crypto_dump_desc(struct nss_crypto_bam_desc *head, uint32_t num, uint8_t *str)
{
	struct nss_crypto_bam_desc *desc;
	int i;


	nss_crypto_dbg("========== %s  ===========\n", str);

	for (i = 0, desc = head; i < num; i++, desc++) {
		nss_crypto_dbg("desc%d = %p, data_start = 0x%x, data_len = %d, flags = 0x%x\n",
				i, desc, desc->data_start, desc->data_len, desc->flags);
	}
	nss_crypto_dbg("================================\n");
}

/*
 * nss_crypto_dump_cblk()
 * 	dump command block
 */
static inline void nss_crypto_dump_cblk(struct nss_crypto_bam_cmd *cmd, uint32_t len, uint8_t *str)
{
	int i;

	nss_crypto_dbg("========== %s:CMD Block[%d] ===========\n", str, len);
	for (i = 0; i < len; i++, cmd++) {
		nss_crypto_dbg("cmd = %p, reg_addr = 0x%x, reg_val = 0x%x, reg_mask = 0x%x\n",
				cmd, cmd->addr, cmd->value, cmd->mask);
	}
	nss_crypto_dbg("================================\n");
}

/*
 * nss_crypto_dump_buf()
 * 	dump a data buffer till buf_len
 */
static inline void nss_crypto_dump_buf(uint8_t *buf, uint32_t buf_len, uint8_t *str)
{
	int i = 0;

	nss_crypto_dbg("===== :%s: ===== \n", str);

	for (i = 0; i < buf_len; i++) {
		nss_crypto_dbg("0x%02x ", buf[i]);
		if ((i + 1) % 16) {
			nss_crypto_dbg("\n");
		}
	}

	nss_crypto_dbg("\n==============\n");
}

/*
 * nss_crypto_dump_bitmap()
 * 	dump bitmap till the size of session bitmap
 */
static inline void nss_crypto_dump_bitmap(unsigned long *addr, uint16_t nbits)
{
	uint32_t i = 0;
	uint32_t loops = BITS_TO_LONGS(nbits);

	nss_crypto_dbg("\n==============\n");

	for(i = 0; i < loops; i++) {
		nss_crypto_dbg("0x%x ", addr[i]);
	}

	nss_crypto_dbg("\n==============\n");
}
#else

#define nss_crypto_dbg(fmt, arg...)
#define nss_crypto_assert(expr)
#define nss_crypto_dump_desc(head, num, str)
#define nss_crypto_dump_cblk(cmd, len, str)
#define nss_crypto_dump_buf(buf, len, str)
#define nss_crypto_dump_bitmap(addr, nbits)

#endif /* !CONFIG_NSS_CRYPTO_DBG */

#endif /* __NSS_CRYPTO_DBG_H */
