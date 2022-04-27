/*-
 * Copyright (c) 2004 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: test_tkip.c 2990 2007-11-28 07:37:53Z proski $
 */

/*
 * TKIP test module.
 */
#ifndef AUTOCONF_INCLUDED
#include <linux/config.h>
#endif
#include <linux/version.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/skbuff.h>

#include <linux/netdevice.h>
#include "if_media.h"
#include <net80211/ieee80211_var.h>

/*
Key	12 34 56 78 90 12 34 56 78 90 12 34 56 78 90 12
	34 56 78 90 12 34 56 78 90 12 34 56 78 90 12 34
PN	0x000000000001
IV	00 20 01 20 00 00 00 00
Phase1	bb 58 07 1f 9e 93 b4 38 25 4b
Phase2	00 20 01 4c fe 67 be d2 7c 86 7b 1b f8 02 8b 1c 
*/

static const u_int8_t test1_key[] = {
	0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x34, 0x56, 0x78, 0x90, 0x12,
	0x34, 0x56, 0x78, 0x90, 0x12,

	0x34, 0x56, 0x78, 0x90, 0x12, 0x34, 0x56, 0x78,		/* TX MIC */
	/*
	 * NB: 11i test vector specifies a RX MIC key different
	 *     from the TX key.  But this doesn't work to enmic,
	 *     encrypt, then decrypt, demic.  So instead we use
	 *     the same key for doing the MIC in each direction.
	 *
	 * XXX need additional vectors to test alternate MIC keys
	 */
#if 0
	0x90, 0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x34,		/* 11i RX MIC */
#else
	0x34, 0x56, 0x78, 0x90, 0x12, 0x34, 0x56, 0x78,		/* TX copy */
#endif
};
static const u_int8_t test1_phase1[] = {
	0xbb, 0x58, 0x07, 0x1f, 0x9e, 0x93, 0xb4, 0x38, 0x25, 0x4b
};
static const u_int8_t test1_phase2[] = {
	0x00, 0x20, 0x01, 0x4c, 0xfe, 0x67, 0xbe, 0xd2, 0x7c, 0x86,
	0x7b, 0x1b, 0xf8, 0x02, 0x8b, 0x1c,
};

/* Plaintext MPDU with MIC */
static const u_int8_t test1_plaintext[] = {
0x08,0x42,0x2c,0x00,0x02,0x03,0x04,0x05,0x06,0x08,0x02,0x03,0x04,0x05,0x06,0x07,
0x02,0x03,0x04,0x05,0x06,0x07,0xd0,0x02,
0xaa,0xaa,0x03,0x00,0x00,0x00,0x08,0x00,0x45,0x00,0x00,0x54,0x00,0x00,0x40,0x00,
0x40,0x01,0xa5,0x55,0xc0,0xa8,0x0a,0x02,0xc0,0xa8,0x0a,0x01,0x08,0x00,0x3a,0xb0,
0x00,0x00,0x00,0x00,0xcd,0x4c,0x05,0x00,0x00,0x00,0x00,0x00,0x08,0x09,0x0a,0x0b,
0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,
0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2a,0x2b,
0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
/* MIC */ 0x68,0x81,0xa3,0xf3,0xd6,0x48,0xd0,0x3c
};

/* Encrypted MPDU with MIC and ICV */
static const u_int8_t test1_encrypted[] = {
0x08,0x42,0x2c,0x00,0x02,0x03,0x04,0x05,0x06,0x08,0x02,0x03,0x04,0x05,0x06,0x07,
0x02,0x03,0x04,0x05,0x06,0x07,0xd0,0x02,0x00,0x20,0x01,0x20,0x00,0x00,0x00,0x00,
0xc0,0x0e,0x14,0xfc,0xe7,0xcf,0xab,0xc7,0x75,0x47,0xe6,0x66,0xe5,0x7c,0x0d,0xac,
0x70,0x4a,0x1e,0x35,0x8a,0x88,0xc1,0x1c,0x8e,0x2e,0x28,0x2e,0x38,0x01,0x02,0x7a,
0x46,0x56,0x05,0x5e,0xe9,0x3e,0x9c,0x25,0x47,0x02,0xe9,0x73,0x58,0x05,0xdd,0xb5,
0x76,0x9b,0xa7,0x3f,0x1e,0xbb,0x56,0xe8,0x44,0xef,0x91,0x22,0x85,0xd3,0xdd,0x6e,
0x54,0x1e,0x82,0x38,0x73,0x55,0x8a,0xdb,0xa0,0x79,0x06,0x8a,0xbd,0x7f,0x7f,0x50,
0x95,0x96,0x75,0xac,0xc4,0xb4,0xde,0x9a,0xa9,0x9c,0x05,0xf2,0x89,0xa7,0xc5,0x2f,
0xee,0x5b,0xfc,0x14,0xf6,0xf8,0xe5,0xf8
};

#define	TEST(n,name,cipher,keyix) { \
	name, IEEE80211_CIPHER_##cipher, keyix, \
	test##n##_key,   sizeof(test##n##_key), \
	test##n##_plaintext, sizeof(test##n##_plaintext), \
	test##n##_encrypted, sizeof(test##n##_encrypted), \
	test##n##_phase1, sizeof(test##n##_phase1), \
	test##n##_phase2, sizeof(test##n##_phase2) \
}

static struct ciphertest {
	const char	*name;
	int		cipher;
	int		keyix;
	const u_int8_t	*key;
	size_t		key_len;
	const u_int8_t	*plaintext;
	size_t		plaintext_len;
	const u_int8_t	*encrypted;
	size_t		encrypted_len;
	const u_int8_t	*phase1;
	size_t		phase1_len;
	const u_int8_t	*phase2;
	size_t		phase2_len;
} tkiptests[] = {
	TEST(1, "TKIP test mpdu 1", TKIP, 0),
};

static void
dumpdata(const char *tag, const void *p, size_t len)
{
	int i;

	printk("%s: 0x%p len %zu", tag, p, len);
	for (i = 0; i < len; i++) {
		if ((i % 16) == 0)
			printk("\n%03d:", i);
		printk(" %02x", ((u_int8_t *)p)[i]);
	}
	printk("\n");
}

static void
cmpfail(const void *gen, size_t genlen, const void *ref, size_t reflen)
{
	int i;

	for (i = 0; i < genlen; i++)
		if (((u_int8_t *)gen)[i] != ((u_int8_t *)ref)[i]) {
			printk("first difference at byte %u\n", i);
			break;
		}
	dumpdata("Generated", gen, genlen);
	dumpdata("Reference", ref, reflen);
}

struct tkip_ctx {
	struct ieee80211vap *tc_vap;	/* for diagnostics + statistics */
	struct ieee80211com *tc_ic;

	u16	tx_ttak[5];
	int	tx_phase1_done;
	u8	tx_rc4key[16];		/* XXX for test module; make locals? */

	u16	rx_ttak[5];
	int	rx_phase1_done;
	u8	rx_rc4key[16];		/* XXX for test module; make locals? */
	uint64_t rx_rsc;		/* held until MIC verified */
};

static int
runtest(struct ieee80211vap *vap, struct ciphertest *t)
{
	struct ieee80211_key *key;
	struct sk_buff *skb = NULL;
	const struct ieee80211_cipher *cip;
	u_int8_t mac[IEEE80211_ADDR_LEN];
	struct tkip_ctx *ctx;
	int hdrlen;

	printk("%s: ", t->name);

	if (!ieee80211_crypto_available(vap, t->cipher)) {
		printk("FAIL: ieee80211_crypto_available failed\n");
		return 0;
	}

	/*
	 * Setup key.
	 */
	key = &vap->iv_nw_keys[t->keyix];
	key->wk_keyix = t->keyix;
	if (!ieee80211_crypto_newkey(vap, t->cipher,
				     IEEE80211_KEY_XMIT | IEEE80211_KEY_RECV,
				     key)) {
		printk("FAIL: ieee80211_crypto_newkey failed\n");
		goto bad;
	}

	memcpy(key->wk_key, t->key, t->key_len);
	key->wk_keylen = 128 / NBBY;
	memset(key->wk_keyrsc, 0, sizeof(key->wk_keyrsc));
	key->wk_keytsc = 0;
	if (!ieee80211_crypto_setkey(vap, key, mac, NULL)) {
		printk("FAIL: ieee80211_crypto_setkey failed\n");
		goto bad;
	}

	/*
	 * Craft frame from plaintext data.  Note that
	 * we leave the MIC off as we'll add it ourself
	 * and then check it against the reference data.
	 */
	cip = key->wk_cipher;
	skb = ieee80211_dev_alloc_skb(t->plaintext_len +
		cip->ic_miclen + cip->ic_header + cip->ic_trailer);
	if (skb == NULL) {
		printk("FAIL: unable to allocate skbuff\n");
		goto bad;
	}
	skb_reserve(skb, cip->ic_header);
	memcpy(skb_put(skb, t->plaintext_len - cip->ic_miclen),
		t->plaintext, t->plaintext_len - cip->ic_miclen);

	/*
	 * Add MIC.
	 */
	if (!ieee80211_crypto_enmic(vap, key, skb, 0)) {
		printk("FAIL: tkip enmic failed\n");
		goto bad;
	}
	/*
	 * Verify: frame length, frame contents.
	 */
	if (skb->len != t->plaintext_len) {
		printk("FAIL: enmic botch; length mismatch\n");
		cmpfail(skb->data, skb->len,
			t->plaintext, t->plaintext_len);
		goto bad;
	}
	if (memcmp(skb->data, t->plaintext, t->plaintext_len)) {
		printk("FAIL: enmic botch\n");
		cmpfail(skb->data, skb->len,
			t->plaintext, t->plaintext_len);
		goto bad;
	}
	/*
	 * Encrypt frame w/ MIC.
	 */
	if (!(*cip->ic_encap)(key, skb, t->keyix << 6)) {
		printk("FAIL: tkip encap failed\n");
		goto bad;
	}
	/*
	 * Verify: phase1, phase2, frame length, frame contents.
	 */
	ctx = key->wk_private;
	if (memcmp(ctx->tx_ttak, t->phase1, t->phase1_len)) {
		printk("FAIL: encrypt phase1 botch\n");
		cmpfail(ctx->tx_ttak, sizeof(ctx->tx_ttak),
			t->phase1, t->phase1_len);
		goto bad;
	} else if (memcmp(ctx->tx_rc4key, t->phase2, t->phase2_len)) {
		printf("FAIL: encrypt phase2 botch\n");
		cmpfail(ctx->tx_rc4key, sizeof(ctx->tx_rc4key),
			t->phase2, t->phase2_len);
		goto bad;
	} else if (skb->len != t->encrypted_len) {
		printk("FAIL: encrypt data length mismatch\n");
		cmpfail(skb->data, skb->len,
			t->encrypted, t->encrypted_len);
		goto bad;
	} else if (memcmp(skb->data, t->encrypted, skb->len)) {
		printk("FAIL: encrypt data does not compare\n");
		cmpfail(skb->data, skb->len,
			t->encrypted, t->encrypted_len);
		dumpdata("Plaintext", t->plaintext, t->plaintext_len);
		goto bad;
	}

	/*
	 * Decrypt frame.
	 */
	hdrlen = ieee80211_hdrspace(vap->iv_ic, skb->data);
	if (!(*cip->ic_decap)(key, skb, hdrlen)) {
		printk("FAIL: tkip decap failed\n");
		/*
		 * Check reason for failure: phase1, phase2, frame data (ICV).
		 */
		if (memcmp(ctx->rx_ttak, t->phase1, t->phase1_len)) {
			printk("decrypt phase1 botch\n");
			cmpfail(ctx->rx_ttak, sizeof(ctx->rx_ttak),
				t->phase1, t->phase1_len);
		} else if (memcmp(ctx->rx_rc4key, t->phase2, t->phase2_len)) {
			printf("decrypt phase2 botch\n");
			cmpfail(ctx->rx_rc4key, sizeof(ctx->rx_rc4key),
				t->phase2, t->phase2_len);
		} else {
			printk("decrypt data does not compare\n");
			cmpfail(skb->data, skb->len,
				t->plaintext, t->plaintext_len);
		}
		goto bad;
	}
	/*
	 * Verify: frame length, frame contents.
	 */
	if (skb->len != t->plaintext_len) {
		printk("FAIL: decap botch; length mismatch\n");
		cmpfail(skb->data, skb->len,
			t->plaintext, t->plaintext_len);
		goto bad;
	}
	if (memcmp(skb->data, t->plaintext, t->plaintext_len)) {
		printk("FAIL: decap botch; data does not compare\n");
		cmpfail(skb->data, skb->len,
			t->plaintext, t->plaintext_len);
		goto bad;
	}
	/*
	 * De-MIC decrypted frame.
	 */
	if (!ieee80211_crypto_demic(vap, key, skb, hdrlen)) {
		printk("FAIL: tkip demic failed\n");
		goto bad;
	}
	/* XXX check frame length and contents... */
	ieee80211_dev_kfree_skb(&skb);
	ieee80211_crypto_delkey(vap, key, NULL);
	printk("PASS\n");
	return 1;
bad:
	if (skb != NULL)
		ieee80211_dev_kfree_skb(&skb);
	ieee80211_crypto_delkey(vap, key, NULL);
	return 0;
}

/*
 * Module glue.
 */
MODULE_AUTHOR("Errno Consulting, Sam Leffler");
MODULE_DESCRIPTION("802.11 wireless support: TKIP cipher tester");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif

static int tests = -1;
static int debug = 0;

#if (LINUX_VERSION_CODE < KERNEL_VERSION(2,5,52))
MODULE_PARM(tests, "i");
MODULE_PARM(debug, "i");
#else
#include <linux/moduleparam.h>
module_param(tests, int, 0600);
module_param(debug, int, 0600);
#endif

MODULE_PARM_DESC(tests, "Specify which tests to run");
MODULE_PARM_DESC(debug, "Enable IEEE80211_MSG_CRYPTO");

static int __init
init_crypto_tkip_test(void)
{
	struct ieee80211com ic;
	struct ieee80211vap vap;
	int i, pass, total;

	memset(&ic, 0, sizeof(ic));
	memset(&vap, 0, sizeof(vap));
	vap.iv_ic = &ic;
	if (debug)
		vap.iv_debug = IEEE80211_MSG_CRYPTO;
	ieee80211_crypto_attach(&ic);
	ieee80211_crypto_vattach(&vap);

	pass = 0;
	total = 0;
	for (i = 0; i < ARRAY_SIZE(tkiptests); i++)
		if (tests & (1 << i)) {
			total++;
			pass += runtest(&vap, &tkiptests[i]);
		}
	printk("%u of %u 802.11i TKIP test vectors passed\n", pass, total);
	ieee80211_crypto_vdetach(&vap);
	ieee80211_crypto_detach(&ic);
	return (pass == total ? 0 : -ENXIO);
}
module_init(init_crypto_tkip_test);

static void __exit
exit_crypto_tkip_test(void)
{
}
module_exit(exit_crypto_tkip_test);
