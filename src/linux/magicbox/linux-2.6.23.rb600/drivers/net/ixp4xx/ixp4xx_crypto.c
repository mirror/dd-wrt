/*
 * ixp4xx_crypto.c - interface to the HW crypto
 *
 * Copyright (C) 2006 Christian Hohnstaedt <chohnstaedt@innominate.com>
 *
 * This file is released under the GPLv2
 */

#include <linux/ixp_qmgr.h>
#include <linux/ixp_npe.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/ixp_crypto.h>

#define SEND_QID 29
#define RECV_QID 30

#define NPE_ID   2 /* NPE C */

#define QUEUE_SIZE 64
#define MY_VERSION "0.0.1"

/* local head for all sa_ctx */
static struct ix_sa_master sa_master;

static const struct ix_hash_algo _hash_algos[] = {
{
	.name		= "MD5",
	.cfgword	= 0xAA010004,
	.digest_len	= 16,
	.icv		= "\x01\x23\x45\x67\x89\xAB\xCD\xEF"
			"\xFE\xDC\xBA\x98\x76\x54\x32\x10",
	.type		= HASH_TYPE_MD5,
},{
	.name		= "SHA1",
	.cfgword	= 0x00000005,
	.digest_len	= 20,
	.icv		= "\x67\x45\x23\x01\xEF\xCD\xAB\x89\x98\xBA"
			"\xDC\xFE\x10\x32\x54\x76\xC3\xD2\xE1\xF0",
	.type		= HASH_TYPE_SHA1,
#if 0
},{
	.name		= "CBC MAC",
	.digest_len	= 64,
	.aad_len	= 48,
	.type		= HASH_TYPE_CBCMAC,
#endif
} };

static const struct ix_cipher_algo _cipher_algos[] = {
{
	.name		= "DES ECB",
	.cfgword_enc	= CIPH_ENCR | MOD_DES | MOD_ECB | KEYLEN_192,
	.cfgword_dec	= CIPH_DECR | MOD_DES | MOD_ECB | KEYLEN_192,
	.block_len	= 8,
	.type		= CIPHER_TYPE_DES,
	.mode		= CIPHER_MODE_ECB,
},{
	.name		= "DES CBC",
	.cfgword_enc	= CIPH_ENCR | MOD_DES | MOD_CBC_ENC | KEYLEN_192,
	.cfgword_dec	= CIPH_DECR | MOD_DES | MOD_CBC_DEC | KEYLEN_192,
	.iv_len		= 8,
	.block_len	= 8,
	.type		= CIPHER_TYPE_DES,
	.mode		= CIPHER_MODE_CBC,
},{
	.name		= "3DES ECB",
	.cfgword_enc	= CIPH_ENCR | MOD_TDEA3 | MOD_ECB | KEYLEN_192,
	.cfgword_dec	= CIPH_DECR | MOD_TDEA3 | MOD_ECB | KEYLEN_192,
	.block_len	= 8,
	.type		= CIPHER_TYPE_3DES,
	.mode		= CIPHER_MODE_ECB,
},{
	.name		= "3DES CBC",
	.cfgword_enc	= CIPH_ENCR | MOD_TDEA3 | MOD_CBC_ENC | KEYLEN_192,
	.cfgword_dec	= CIPH_DECR | MOD_TDEA3 | MOD_CBC_DEC | KEYLEN_192,
	.iv_len		= 8,
	.block_len	= 8,
	.type		= CIPHER_TYPE_3DES,
	.mode		= CIPHER_MODE_CBC,
},{
	.name		= "AES ECB",
	.cfgword_enc	= CIPH_ENCR | ALGO_AES | MOD_ECB,
	.cfgword_dec	= CIPH_DECR | ALGO_AES | MOD_ECB,
	.block_len	= 16,
	.type		= CIPHER_TYPE_AES,
	.mode		= CIPHER_MODE_ECB,
},{
	.name		= "AES CBC",
	.cfgword_enc	= CIPH_ENCR | ALGO_AES | MOD_CBC_ENC,
	.cfgword_dec	= CIPH_DECR | ALGO_AES | MOD_CBC_DEC,
	.block_len	= 16,
	.iv_len		= 16,
	.type		= CIPHER_TYPE_AES,
	.mode		= CIPHER_MODE_CBC,
},{
	.name		= "AES CTR",
	.cfgword_enc	= CIPH_ENCR | ALGO_AES | MOD_CTR,
	.cfgword_dec	= CIPH_ENCR | ALGO_AES | MOD_CTR,
	.block_len	= 16,
	.iv_len		= 16,
	.type		= CIPHER_TYPE_AES,
	.mode		= CIPHER_MODE_CTR,
#if 0
},{
	.name		= "AES CCM",
	.cfgword_enc	= CIPH_ENCR | ALGO_AES | MOD_CCM_ENC,
	.cfgword_dec	= CIPH_ENCR | ALGO_AES | MOD_CCM_DEC,
	.block_len	= 16,
	.iv_len		= 16,
	.type		= CIPHER_TYPE_AES,
	.mode		= CIPHER_MODE_CCM,
#endif
} };

const struct ix_hash_algo *ix_hash_by_id(int type)
{
	int i;

	for(i=0; i<ARRAY_SIZE(_hash_algos); i++) {
		if (_hash_algos[i].type == type)
			return _hash_algos + i;
	}
	return NULL;
}

const struct ix_cipher_algo *ix_cipher_by_id(int type, int mode)
{
	int i;

	for(i=0; i<ARRAY_SIZE(_cipher_algos); i++) {
		if (_cipher_algos[i].type==type && _cipher_algos[i].mode==mode)
			return _cipher_algos + i;
	}
	return NULL;
}

static void irqcb_recv(struct qm_queue *queue);

static int init_sa_master(struct ix_sa_master *master)
{
	struct npe_info *npe;
	int ret = -ENODEV;

	if (! (ix_fuse() & (IX_FUSE_HASH | IX_FUSE_AES | IX_FUSE_DES))) {
		printk(KERN_ERR "ixp_crypto: No HW crypto available\n");
		return ret;
	}
	memset(master, 0, sizeof(struct ix_sa_master));
	master->npe_dev = get_npe_by_id(NPE_ID);
	if (! master->npe_dev)
		goto err;

	npe = dev_get_drvdata(master->npe_dev);

	if (npe_status(npe) & IX_NPEDL_EXCTL_STATUS_RUN) {
		switch (npe->img_info[1]) {
		case 4:
			printk(KERN_INFO "Crypto AES avaialable\n");
			break;
		case 5:
			printk(KERN_INFO "Crypto AES and CCM avaialable\n");
			break;
		default:
			printk(KERN_WARNING "Current microcode for %s has no"
				" crypto capabilities\n", npe->plat->name);
			break;
		}
	}
	rwlock_init(&master->lock);
	master->dmapool = dma_pool_create("ixp4xx_crypto", master->npe_dev,
			sizeof(struct npe_crypt_cont), 32, 0);
	if (!master->dmapool) {
		ret = -ENOMEM;
		goto err;
	}
	master->sendq = request_queue(SEND_QID, QUEUE_SIZE);
	if (IS_ERR(master->sendq)) {
		printk(KERN_ERR "ixp4xx_crypto: Error requesting Q: %d\n",
				SEND_QID);
		ret = PTR_ERR(master->sendq);
		goto err;
	}
	master->recvq = request_queue(RECV_QID, QUEUE_SIZE);
	if (IS_ERR(master->recvq)) {
		printk(KERN_ERR "ixp4xx_crypto: Error requesting Q: %d\n",
				RECV_QID);
		ret = PTR_ERR(master->recvq);
		release_queue(master->sendq);
		goto err;
	}

	master->recvq->irq_cb = irqcb_recv;
	queue_set_watermarks(master->recvq, 0, 0);
	queue_set_irq_src(master->recvq, Q_IRQ_ID_NOT_E);
	queue_enable_irq(master->recvq);
	printk(KERN_INFO "ixp4xx_crypto " MY_VERSION " registered successfully\n");

	return 0;
err:
	if (master->dmapool)
		dma_pool_destroy(master->dmapool);
	if (! master->npe_dev)
		put_device(master->npe_dev);
	return ret;

}

static void release_sa_master(struct ix_sa_master *master)
{
	struct npe_crypt_cont *cont;
	unsigned long flags;

	write_lock_irqsave(&master->lock, flags);
	while (master->pool) {
		cont = master->pool;
		master->pool = cont->next;
		dma_pool_free(master->dmapool, cont, cont->phys);
		master->pool_size--;
	}
	write_unlock_irqrestore(&master->lock, flags);
	if (master->pool_size) {
		printk(KERN_ERR "ixp4xx_crypto: %d items lost from DMA pool\n",
				master->pool_size);
	}

	dma_pool_destroy(master->dmapool);
	release_queue(master->sendq);
	release_queue(master->recvq);
	return_npe_dev(master->npe_dev);
}

static struct npe_crypt_cont *ix_sa_get_cont(struct ix_sa_master *master)
{
	unsigned long flags;
	struct npe_crypt_cont *cont;
	dma_addr_t handle;

	write_lock_irqsave(&master->lock, flags);
	if (!master->pool) {
		cont = dma_pool_alloc(master->dmapool, GFP_ATOMIC, &handle);
		if (cont) {
			master->pool_size++;
			cont->phys = handle;
			cont->virt = cont;
		}
	} else {
		cont = master->pool;
		master->pool = cont->next;
	}
	write_unlock_irqrestore(&master->lock, flags);
	return cont;
}

static void
ix_sa_return_cont(struct ix_sa_master *master,struct npe_crypt_cont *cont)
{
	unsigned long flags;

	write_lock_irqsave(&master->lock, flags);
	cont->next = master->pool;
	master->pool = cont;
	write_unlock_irqrestore(&master->lock, flags);
}

static void free_sa_dir(struct ix_sa_ctx *sa_ctx, struct ix_sa_dir *dir)
{
	memset(dir->npe_ctx, 0, NPE_CTX_LEN);
	dma_pool_free(sa_ctx->master->dmapool, dir->npe_ctx,
			dir->npe_ctx_phys);
}

static void ix_sa_ctx_destroy(struct ix_sa_ctx *sa_ctx)
{
	BUG_ON(sa_ctx->state != STATE_UNLOADING);
	free_sa_dir(sa_ctx, &sa_ctx->encrypt);
	free_sa_dir(sa_ctx, &sa_ctx->decrypt);
	kfree(sa_ctx);
	module_put(THIS_MODULE);
}

static void recv_pack(struct qm_queue *queue, u32 phys)
{
	struct ix_sa_ctx *sa_ctx;
	struct npe_crypt_cont *cr_cont;
	struct npe_cont *cont;
	int failed;

	failed = phys & 0x1;
	phys &= ~0x3;

	cr_cont = dma_to_virt(queue->dev, phys);
	cr_cont = cr_cont->virt;
	sa_ctx = cr_cont->ctl.crypt.sa_ctx;

	phys = npe_to_cpu32(cr_cont->ctl.crypt.src_buf);
	if (phys) {
		cont = dma_to_virt(queue->dev, phys);
		cont = cont->virt;
	} else {
		cont = NULL;
	}
	if (cr_cont->ctl.crypt.oper_type == OP_PERFORM) {
		dma_unmap_single(sa_ctx->master->npe_dev,
				cont->eth.phys_addr,
				cont->eth.buf_len,
				DMA_BIDIRECTIONAL);
		if (sa_ctx->perf_cb)
			sa_ctx->perf_cb(sa_ctx, cont->data, failed);
		qmgr_return_cont(dev_get_drvdata(queue->dev), cont);
		ix_sa_return_cont(sa_ctx->master, cr_cont);
		if (atomic_dec_and_test(&sa_ctx->use_cnt))
			ix_sa_ctx_destroy(sa_ctx);
		return;
	}

	/* We are registering */
	switch (cr_cont->ctl.crypt.mode) {
	case NPE_OP_HASH_GEN_ICV:
		/* 1 out of 2 HMAC preparation operations completed */
		dma_unmap_single(sa_ctx->master->npe_dev,
				cont->eth.phys_addr,
				cont->eth.buf_len,
				DMA_TO_DEVICE);
		kfree(cont->data);
		qmgr_return_cont(dev_get_drvdata(queue->dev), cont);
		break;
	case NPE_OP_ENC_GEN_KEY:
		memcpy(sa_ctx->decrypt.npe_ctx + sizeof(u32),
			sa_ctx->rev_aes->ctl.rev_aes_key + sizeof(u32),
			sa_ctx->c_key.len);
		/* REV AES data not needed anymore, free it */
		ix_sa_return_cont(sa_ctx->master, sa_ctx->rev_aes);
		sa_ctx->rev_aes = NULL;
		break;
	default:
		printk(KERN_ERR "Unknown crypt-register mode: %x\n",
				cr_cont->ctl.crypt.mode);

	}
	if (cr_cont->ctl.crypt.oper_type == OP_REG_DONE) {
		if (sa_ctx->state == STATE_UNREGISTERED)
			sa_ctx->state = STATE_REGISTERED;
		if (sa_ctx->reg_cb)
			sa_ctx->reg_cb(sa_ctx, failed);
	}
	ix_sa_return_cont(sa_ctx->master, cr_cont);
	if (atomic_dec_and_test(&sa_ctx->use_cnt))
		ix_sa_ctx_destroy(sa_ctx);
}

static void irqcb_recv(struct qm_queue *queue)
{
	u32 phys;

	queue_ack_irq(queue);
	while ((phys = queue_get_entry(queue)))
		recv_pack(queue, phys);
}

static int init_sa_dir(struct ix_sa_ctx *sa_ctx, struct ix_sa_dir *dir)
{
	dir->npe_ctx = dma_pool_alloc(sa_ctx->master->dmapool,
			sa_ctx->gfp_flags, &dir->npe_ctx_phys);
	if (!dir->npe_ctx) {
		return 1;
	}
	memset(dir->npe_ctx, 0, NPE_CTX_LEN);
	return 0;
}

struct ix_sa_ctx *ix_sa_ctx_new(int priv_len, gfp_t flags)
{
	struct ix_sa_ctx *sa_ctx;
	struct ix_sa_master *master = &sa_master;
	struct npe_info *npe = dev_get_drvdata(master->npe_dev);

	/* first check if Microcode was downloaded into this NPE */
	if (!( npe_status(npe) & IX_NPEDL_EXCTL_STATUS_RUN)) {
		printk(KERN_ERR "%s not running\n", npe->plat->name);
		return NULL;
	}
	switch (npe->img_info[1]) {
		case 4:
		case 5:
			break;
		default:
			/* No crypto Microcode */
			return NULL;
	}
	if (!try_module_get(THIS_MODULE)) {
		return NULL;
	}

	sa_ctx = kzalloc(sizeof(struct ix_sa_ctx) + priv_len, flags);
	if (!sa_ctx) {
		goto err_put;
	}

	sa_ctx->master = master;
	sa_ctx->gfp_flags = flags;

	if (init_sa_dir(sa_ctx, &sa_ctx->encrypt))
		goto err_free;
	if (init_sa_dir(sa_ctx, &sa_ctx->decrypt)) {
		free_sa_dir(sa_ctx, &sa_ctx->encrypt);
		goto err_free;
	}
	if (priv_len)
		sa_ctx->priv = sa_ctx + 1;

	atomic_set(&sa_ctx->use_cnt, 1);
	return sa_ctx;

err_free:
	 kfree(sa_ctx);
err_put:
	 module_put(THIS_MODULE);
	 return NULL;
}

void ix_sa_ctx_free(struct ix_sa_ctx *sa_ctx)
{
	sa_ctx->state = STATE_UNLOADING;
	if (atomic_dec_and_test(&sa_ctx->use_cnt))
		ix_sa_ctx_destroy(sa_ctx);
	else
		printk("ix_sa_ctx_free -> delayed: %p %d\n",
				sa_ctx, atomic_read(&sa_ctx->use_cnt));
}

/* http://www.ietf.org/rfc/rfc2104.txt */
#define HMAC_IPAD_VALUE 0x36
#define HMAC_OPAD_VALUE 0x5C
#define PAD_BLOCKLEN 64

static int register_chain_var(struct ix_sa_ctx *sa_ctx,
	unsigned char *pad, u32 target, int init_len, u32 ctx_addr, int oper)
{
	struct npe_crypt_cont *cr_cont;
	struct npe_cont *cont;

	cr_cont = ix_sa_get_cont(sa_ctx->master);
	if (!cr_cont)
		return -ENOMEM;

	cr_cont->ctl.crypt.sa_ctx = sa_ctx;
	cr_cont->ctl.crypt.auth_offs = 0;
	cr_cont->ctl.crypt.auth_len =cpu_to_npe16(PAD_BLOCKLEN);
	cr_cont->ctl.crypt.crypto_ctx = cpu_to_npe32(ctx_addr);

	cont = qmgr_get_cont(dev_get_drvdata(sa_ctx->master->sendq->dev));
	if (!cont) {
		ix_sa_return_cont(sa_ctx->master, cr_cont);
		return -ENOMEM;
	}

	cont->data = pad;
	cont->eth.next = 0;
	cont->eth.buf_len = cpu_to_npe16(PAD_BLOCKLEN);
	cont->eth.pkt_len = 0;

	cont->eth.phys_addr = cpu_to_npe32(dma_map_single(
		sa_ctx->master->npe_dev, pad, PAD_BLOCKLEN, DMA_TO_DEVICE));

	cr_cont->ctl.crypt.src_buf = cpu_to_npe32(cont->phys);
	cr_cont->ctl.crypt.oper_type = oper;

	cr_cont->ctl.crypt.addr.icv = cpu_to_npe32(target);
	cr_cont->ctl.crypt.mode = NPE_OP_HASH_GEN_ICV;
	cr_cont->ctl.crypt.init_len = init_len;

	atomic_inc(&sa_ctx->use_cnt);
	queue_put_entry(sa_ctx->master->sendq, cr_cont->phys);
	if (queue_stat(sa_ctx->master->sendq) == 2) { /* overflow */
		atomic_dec(&sa_ctx->use_cnt);
		qmgr_return_cont(dev_get_drvdata(sa_ctx->master->sendq->dev),
				cont);
		ix_sa_return_cont(sa_ctx->master, cr_cont);
		return -ENOMEM;
	}
	return 0;
}

/* Return value
 * 0 if nothing registered,
 * 1 if something registered and
 * < 0 on error
 */
static int ix_sa_ctx_setup_auth(struct ix_sa_ctx *sa_ctx,
		const struct ix_hash_algo *algo, int len, int oper, int encrypt)
{
	unsigned char *ipad, *opad;
	u32 itarget, otarget, ctx_addr;
	unsigned char *cinfo;
	int init_len, i, ret = 0;
	struct qm_qmgr *qmgr;
	struct ix_sa_dir *dir;
	u32 cfgword;

	dir = encrypt ? &sa_ctx->encrypt : &sa_ctx->decrypt;
	cinfo = dir->npe_ctx + dir->npe_ctx_idx;

	qmgr = dev_get_drvdata(sa_ctx->master->sendq->dev);

	cinfo = dir->npe_ctx + dir->npe_ctx_idx;
	sa_ctx->h_algo = algo;

	if (!algo) {
		dir->npe_mode |= NPE_OP_HMAC_DISABLE;
		return 0;
	}
	if (algo->type == HASH_TYPE_CBCMAC) {
		dir->npe_mode |= NPE_OP_CCM_ENABLE | NPE_OP_HMAC_DISABLE;
		return 0;
	}
	if (sa_ctx->h_key.len > 64 || sa_ctx->h_key.len < algo->digest_len)
		return -EINVAL;
	if (len > algo->digest_len || (len % 4))
		return -EINVAL;
	if (!len)
		len = algo->digest_len;

	sa_ctx->digest_len = len;

	/* write cfg word to cryptinfo */
	cfgword = algo->cfgword | ((len/4) << 8);
	*(u32*)cinfo = cpu_to_be32(cfgword);
	cinfo += sizeof(cfgword);

	/* write ICV to cryptinfo */
	memcpy(cinfo, algo->icv, algo->digest_len);
	cinfo += algo->digest_len;

	itarget = dir->npe_ctx_phys + dir->npe_ctx_idx
				+ sizeof(algo->cfgword);
	otarget = itarget + algo->digest_len;

	opad = kzalloc(PAD_BLOCKLEN, sa_ctx->gfp_flags | GFP_DMA);
	if (!opad) {
		return -ENOMEM;
	}
	ipad = kzalloc(PAD_BLOCKLEN, sa_ctx->gfp_flags | GFP_DMA);
	if (!ipad) {
		kfree(opad);
		return -ENOMEM;
	}
	memcpy(ipad, sa_ctx->h_key.key, sa_ctx->h_key.len);
	memcpy(opad, sa_ctx->h_key.key, sa_ctx->h_key.len);
	for (i = 0; i < PAD_BLOCKLEN; i++) {
		ipad[i] ^= HMAC_IPAD_VALUE;
		opad[i] ^= HMAC_OPAD_VALUE;
	}
	init_len = cinfo - (dir->npe_ctx + dir->npe_ctx_idx);
	ctx_addr = dir->npe_ctx_phys + dir->npe_ctx_idx;

	dir->npe_ctx_idx += init_len;
	dir->npe_mode |= NPE_OP_HASH_ENABLE;

	if (!encrypt)
		dir->npe_mode |= NPE_OP_HASH_VERIFY;

	/* register first chainvar */
	ret = register_chain_var(sa_ctx, opad, otarget,
			init_len, ctx_addr, OP_REGISTER);
	if (ret) {
		kfree(ipad);
		kfree(opad);
		return ret;
	}

	/* register second chainvar */
	ret = register_chain_var(sa_ctx, ipad, itarget,
			init_len, ctx_addr, oper);
	if (ret) {
		kfree(ipad);
		return ret;
	}

	return 1;
}

static int gen_rev_aes_key(struct ix_sa_ctx *sa_ctx,
		u32 keylen_cfg, int cipher_op)
{
	unsigned char *cinfo;
	struct npe_crypt_cont *cr_cont;

	keylen_cfg |= CIPH_ENCR | ALGO_AES | MOD_ECB;
	sa_ctx->rev_aes = ix_sa_get_cont(sa_ctx->master);
	if (!sa_ctx->rev_aes)
		return -ENOMEM;

	cinfo = sa_ctx->rev_aes->ctl.rev_aes_key;
	*(u32*)cinfo = cpu_to_be32(keylen_cfg);
	cinfo += sizeof(keylen_cfg);

	memcpy(cinfo, sa_ctx->c_key.key, sa_ctx->c_key.len);

	cr_cont = ix_sa_get_cont(sa_ctx->master);
	if (!cr_cont) {
		ix_sa_return_cont(sa_ctx->master, sa_ctx->rev_aes);
		sa_ctx->rev_aes = NULL;
		return -ENOMEM;
	}
	cr_cont->ctl.crypt.sa_ctx = sa_ctx;
	cr_cont->ctl.crypt.oper_type = cipher_op;

	cr_cont->ctl.crypt.crypt_offs = 0;
	cr_cont->ctl.crypt.crypt_len = cpu_to_npe16(AES_BLOCK128);
	cr_cont->ctl.crypt.addr.rev_aes = cpu_to_npe32(
			sa_ctx->rev_aes->phys + sizeof(keylen_cfg));

	cr_cont->ctl.crypt.src_buf = 0;
	cr_cont->ctl.crypt.crypto_ctx = cpu_to_npe32(sa_ctx->rev_aes->phys);
	cr_cont->ctl.crypt.mode = NPE_OP_ENC_GEN_KEY;
	cr_cont->ctl.crypt.init_len = sa_ctx->decrypt.npe_ctx_idx;

	atomic_inc(&sa_ctx->use_cnt);
	queue_put_entry(sa_ctx->master->sendq, cr_cont->phys);
	if (queue_stat(sa_ctx->master->sendq) == 2) { /* overflow */
		atomic_dec(&sa_ctx->use_cnt);
		ix_sa_return_cont(sa_ctx->master, cr_cont);
		ix_sa_return_cont(sa_ctx->master, sa_ctx->rev_aes);
		sa_ctx->rev_aes = NULL;
		return -ENOMEM;
	}

	return 1;
}

/* Return value
 * 0 if nothing registered,
 * 1 if something registered and
 * < 0 on error
 */
static int ix_sa_ctx_setup_cipher(struct ix_sa_ctx *sa_ctx,
		const struct ix_cipher_algo *algo, int cipher_op, int encrypt)
{
	unsigned char *cinfo;
	int keylen, init_len;
	u32 cipher_cfg;
	u32 keylen_cfg = 0;
	struct ix_sa_dir *dir;

	dir = encrypt ? &sa_ctx->encrypt : &sa_ctx->decrypt;
	cinfo = dir->npe_ctx + dir->npe_ctx_idx;

	sa_ctx->c_algo = algo;

	if (!algo)
		return 0;

	if (algo->type == CIPHER_TYPE_DES && sa_ctx->c_key.len != 8)
		return -EINVAL;

	if (algo->type == CIPHER_TYPE_3DES && sa_ctx->c_key.len != 24)
		return -EINVAL;

	keylen = 24;

	if (encrypt) {
		cipher_cfg = algo->cfgword_enc;
		dir->npe_mode |= NPE_OP_CRYPT_ENCRYPT;
	} else {
		cipher_cfg = algo->cfgword_dec;
	}
	if (algo->type == CIPHER_TYPE_AES) {
		switch (sa_ctx->c_key.len) {
			case 16: keylen_cfg = MOD_AES128 | KEYLEN_128; break;
			case 24: keylen_cfg = MOD_AES192 | KEYLEN_192; break;
			case 32: keylen_cfg = MOD_AES256 | KEYLEN_256; break;
			default: return -EINVAL;
		}
		keylen = sa_ctx->c_key.len;
		cipher_cfg |= keylen_cfg;
	}

	/* write cfg word to cryptinfo */
	*(u32*)cinfo = cpu_to_be32(cipher_cfg);
	cinfo += sizeof(cipher_cfg);

	/* write cipher key to cryptinfo */
	memcpy(cinfo, sa_ctx->c_key.key, sa_ctx->c_key.len);
	cinfo += keylen;

	init_len = cinfo - (dir->npe_ctx + dir->npe_ctx_idx);
	dir->npe_ctx_idx += init_len;

	dir->npe_mode |= NPE_OP_CRYPT_ENABLE;

	if (algo->type == CIPHER_TYPE_AES && !encrypt) {
		return gen_rev_aes_key(sa_ctx, keylen_cfg, cipher_op);
	}

	return 0;
}

/* returns 0 on OK, <0 on error and 1 on overflow */
int ix_sa_crypto_perform(struct ix_sa_ctx *sa_ctx, u8 *data, void *ptr,
	int datalen, int c_offs, int c_len, int a_offs, int a_len,
	int hmac, char *iv, int encrypt)
{
	struct npe_crypt_cont *cr_cont;
	struct npe_cont *cont;
	u32 data_phys;
	int ret = -ENOMEM;
	struct ix_sa_dir *dir;

	dir = encrypt ? &sa_ctx->encrypt : &sa_ctx->decrypt;

	if (sa_ctx->state != STATE_REGISTERED)
		return -ENOENT;

	cr_cont = ix_sa_get_cont(sa_ctx->master);
	if (!cr_cont)
		return ret;

	cr_cont->ctl.crypt.sa_ctx = sa_ctx;
	cr_cont->ctl.crypt.crypto_ctx = cpu_to_npe32(dir->npe_ctx_phys);
	cr_cont->ctl.crypt.oper_type = OP_PERFORM;
	cr_cont->ctl.crypt.mode = dir->npe_mode;
	cr_cont->ctl.crypt.init_len = dir->npe_ctx_idx;

	if (sa_ctx->c_algo) {
		cr_cont->ctl.crypt.crypt_offs = cpu_to_npe16(c_offs);
		cr_cont->ctl.crypt.crypt_len = cpu_to_npe16(c_len);
		if (sa_ctx->c_algo->iv_len) {
			if (!iv) {
				ret = -EINVAL;
				goto err_cr;
			}
			memcpy(cr_cont->ctl.crypt.iv, iv,
					sa_ctx->c_algo->iv_len);
		}
	}

	if (sa_ctx->h_algo) {
		/* prepare hashing */
		cr_cont->ctl.crypt.auth_offs = cpu_to_npe16(a_offs);
		cr_cont->ctl.crypt.auth_len = cpu_to_npe16(a_len);
	}

	data_phys = dma_map_single(sa_ctx->master->npe_dev,
			data, datalen, DMA_BIDIRECTIONAL);
	if (hmac)
		cr_cont->ctl.crypt.addr.icv = cpu_to_npe32(data_phys + hmac);

	/* Prepare the data ptr */
	cont = qmgr_get_cont(dev_get_drvdata(sa_ctx->master->sendq->dev));
	if (!cont) {
		goto err_unmap;
	}

	cont->data = ptr;
	cont->eth.next = 0;
	cont->eth.buf_len = cpu_to_npe16(datalen);
	cont->eth.pkt_len = 0;

	cont->eth.phys_addr = cpu_to_npe32(data_phys);
	cr_cont->ctl.crypt.src_buf = cpu_to_npe32(cont->phys);

	atomic_inc(&sa_ctx->use_cnt);
	queue_put_entry(sa_ctx->master->sendq, cr_cont->phys);
	if (queue_stat(sa_ctx->master->sendq) != 2) {
		return 0;
	}

	/* overflow */
	printk("%s: Overflow\n", __FUNCTION__);
	ret = -EAGAIN;
	atomic_dec(&sa_ctx->use_cnt);
	qmgr_return_cont(dev_get_drvdata(sa_ctx->master->sendq->dev), cont);

err_unmap:
	dma_unmap_single(sa_ctx->master->npe_dev, data_phys, datalen,
			DMA_BIDIRECTIONAL);
err_cr:
	ix_sa_return_cont(sa_ctx->master, cr_cont);

	return ret;
}

int ix_sa_ctx_setup_cipher_auth(struct ix_sa_ctx *sa_ctx,
		const struct ix_cipher_algo *cipher,
		const struct ix_hash_algo *auth, int len)
{
	int ret = 0, sum = 0;
	int cipher_op;

	if (sa_ctx->state != STATE_UNREGISTERED)
		return -ENOENT;

	atomic_inc(&sa_ctx->use_cnt);

	cipher_op = auth ? OP_REGISTER : OP_REG_DONE;
	if ((ret = ix_sa_ctx_setup_cipher(sa_ctx, cipher, OP_REGISTER, 1)) < 0)
		goto out;
	sum += ret;
	if ((ret = ix_sa_ctx_setup_cipher(sa_ctx, cipher, cipher_op, 0)) < 0)
		goto out;
	sum += ret;
	if ((ret = ix_sa_ctx_setup_auth(sa_ctx, auth, len, OP_REGISTER, 1)) < 0)
		goto out;
	sum += ret;
	if ((ret = ix_sa_ctx_setup_auth(sa_ctx, auth, len, OP_REG_DONE, 0)) < 0)
		goto out;
	sum += ret;

	/* Nothing registered ?
	 * Ok, then we are done and call the callback here.
	 */
	if (!sum) {
		if (sa_ctx->state == STATE_UNREGISTERED)
			sa_ctx->state = STATE_REGISTERED;
		if (sa_ctx->reg_cb)
			sa_ctx->reg_cb(sa_ctx, 0);
	}
out:
	atomic_dec(&sa_ctx->use_cnt);
	return ret;
}

static int __init init_crypto(void)
{
	return init_sa_master(&sa_master);
}

static void __exit finish_crypto(void)
{
	release_sa_master(&sa_master);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Christian Hohnstaedt <chohnstaedt@innominate.com>");

EXPORT_SYMBOL(ix_hash_by_id);
EXPORT_SYMBOL(ix_cipher_by_id);

EXPORT_SYMBOL(ix_sa_ctx_new);
EXPORT_SYMBOL(ix_sa_ctx_free);
EXPORT_SYMBOL(ix_sa_ctx_setup_cipher_auth);
EXPORT_SYMBOL(ix_sa_crypto_perform);

module_init(init_crypto);
module_exit(finish_crypto);

