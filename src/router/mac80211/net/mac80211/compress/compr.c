#include <linux/types.h>
#include <linux/lzo.h>
#include <linux/module.h>
#include <linux/random.h>
#include <linux/export.h>
#include "lzma/lzma.h"
#include <linux/lz4.h>

#include "compress.h"

#define COMPR_TYPE_LZO	1
#define COMPR_TYPE_LZMA	2
#define COMPR_TYPE_LZ4	3
#define COMPR_TYPE_ZSTD	4

#if IS_ENABLED(CONFIG_ZSTD_COMPRESS) && IS_ENABLED(CONFIG_ZSTD_DECOMPRESS)
#include <linux/zstd.h>
#define ZSTD 1
size_t ZSTD_freeCCtx(ZSTD_CCtx * cctx);
size_t ZSTD_freeDCtx(ZSTD_DCtx * dctx);
#if LINUX_VERSION_IS_GEQ(5,10,0)
#define ZSTD_CStreamWorkspaceBound zstd_cstream_workspace_bound
#define ZSTD_DStreamWorkspaceBound zstd_dstream_workspace_bound
//#define ZSTD_getParams zstd_get_params
#define ZSTD_compressionParameters zstd_compression_parameters
#define ZSTD_parameters zstd_parameters
#define ZSTD_initCCtx zstd_init_cctx
#define ZSTD_CCtx zstd_cctx
#define ZSTD_initDCtx zstd_init_dctx
#define ZSTD_DCtx zstd_dctx
#define ZSTD_compressCCtx zstd_compress_cctx
#define ZSTD_decompressDCtx zstd_decompress_dctx
#endif
#endif


#ifdef ZSTD

static int zstd_init(struct zstd_workspace *wksp)
{
	ZSTD_parameters params = ZSTD_getParams(3, 4096, 0);
	if (wksp == NULL)
		goto failed;

#if LINUX_VERSION_IS_GEQ(5,10,0)
	wksp->mem_size = max_t(size_t, ZSTD_CStreamWorkspaceBound(&params.cParams), ZSTD_DStreamWorkspaceBound(4096));
#else
	wksp->mem_size = max_t(size_t, ZSTD_CStreamWorkspaceBound(params.cParams), ZSTD_DStreamWorkspaceBound(4096));
#endif

	wksp->param = params;
	wksp->mem = kvmalloc(wksp->mem_size, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
	wksp->dmem = kvmalloc(wksp->mem_size, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
	if (wksp->mem == NULL)
		goto failed;
	if (wksp->dmem == NULL)
		goto failed;
	printk(KERN_INFO "mac80211: allocated %zu bytes for zstd algorithm\n", wksp->mem_size * 2);
	return 0;

failed:
	if (wksp->mem)
		kvfree(wksp->mem);
	if (wksp->dmem)
		kvfree(wksp->dmem);
	printk(KERN_ERR "mac80211: Failed to allocate zstd workspace (%zu bytes)\n", wksp->mem_size * 2);
	return -ENOMEM;
}

static size_t zstd_compress(struct ieee80211_sub_if_data *sdata, const char *source, char *dest, int isize, int osize, int level)
{
	size_t result;
	ZSTD_CCtx *cctx;

	cctx = ZSTD_initCCtx(sdata->zstd.mem, sdata->zstd.mem_size);
	/*
	 * out of kernel memory, gently fall through - this will disable
	 * compression
	 */
	if (cctx == NULL)
		return (0);

#if LINUX_VERSION_IS_GEQ(5,10,0)
	result = ZSTD_compressCCtx(cctx, dest, osize, source, isize, &sdata->zstd.param);
#else
	result = ZSTD_compressCCtx(cctx, dest, osize, source, isize, sdata->zstd.param);
#endif
	ZSTD_freeCCtx(cctx);
	return (result);
}

static size_t zstd_decompress(struct ieee80211_sub_if_data *sdata, const char *source, char *dest, int isize, int maxosize)
{
	size_t result;
	ZSTD_DCtx *dctx;

	dctx = ZSTD_initDCtx(sdata->zstd.dmem, sdata->zstd.mem_size);
	if (dctx == NULL)
		return 0;

	result = ZSTD_decompressDCtx(dctx, dest, maxosize, source, isize);

	ZSTD_freeDCtx(dctx);
	return (result);
}

#endif

static void *mac80211_compress_alloc(size_t size)
{
	if (size == 0)
		return NULL;

	return kmalloc(size, in_interrupt()? GFP_ATOMIC : GFP_KERNEL);
}

static void mac80211_compress_free(void *address)
{
	if (address != NULL)
		kfree(address);
}

void mac80211_compress_uninit(struct ieee80211_sub_if_data *sdata)
{
	if (sdata->tx_outbuf) {
		mac80211_compress_free(sdata->tx_outbuf);
		sdata->tx_wrkbuf = NULL;
		sdata->tx_outbuf = NULL;
	}
	if (sdata->rx_outbuf) {
		mac80211_compress_free(sdata->rx_outbuf);
		sdata->rx_outbuf = NULL;
	}
	if (sdata->dev->ieee80211_ptr->use_compr == COMPR_TYPE_LZMA && sdata->lzma_p) {
		LzmaEnc_Destroy(sdata->lzma_p, &lzma_alloc, &lzma_alloc);
		sdata->lzma_p = NULL;
	}
#ifdef ZSTD
	if (sdata->zstd.mem)
		kvfree(sdata->zstd.mem);
	if (sdata->zstd.dmem)
		kvfree(sdata->zstd.dmem);
#endif
}


void mac80211_compress_init(struct ieee80211_sub_if_data *sdata)
{
	CLzmaEncProps props;
	sdata->tx_wrkbuf = NULL;
	sdata->tx_outbuf = NULL;
	sdata->rx_outbuf = NULL;
	if ((sdata->tx_outbuf = mac80211_compress_alloc(4096 * sizeof(u8) + (LZO1X_1_MEM_COMPRESS << 2))) != NULL)
		sdata->tx_wrkbuf = sdata->tx_outbuf + 4096 * sizeof(u8);
	sdata->rx_outbuf = mac80211_compress_alloc(4096 * sizeof(u8));
	if (!sdata->rx_outbuf || !sdata->tx_wrkbuf || !sdata->tx_outbuf) {
		printk(KERN_ERR "mac80211: Failed to allocate lzo workspace\n");
		mac80211_compress_uninit(sdata);
	}

	sdata->lzma_propsSize = sizeof(sdata->lzma_propsEncoded);
	if ((sdata->lzma_p = (CLzmaEncHandle *) LzmaEnc_Create(&lzma_alloc)) == NULL) {
		printk(KERN_ERR "mac80211: Failed to allocate lzma workspace\n");
		mac80211_compress_uninit(sdata);
		return;
	}
	LzmaEncProps_Init(&props);

	props.dictSize = LZMA_BEST_DICT(0x2000);
	props.level = 1;
	props.lc = LZMA_BEST_LC;
	props.lp = LZMA_BEST_LP;
	props.pb = LZMA_BEST_PB;
	props.fb = LZMA_BEST_FB;

	if (LzmaEnc_SetProps(sdata->lzma_p, &props) != SZ_OK) {
		mac80211_compress_uninit(sdata);
		return;
	}

	if (LzmaEnc_WriteProperties(sdata->lzma_p, sdata->lzma_propsEncoded, &sdata->lzma_propsSize) != SZ_OK) {
		mac80211_compress_uninit(sdata);
		return;
	}
#ifdef ZSTD
	if (zstd_init(&sdata->zstd)) {
		mac80211_compress_uninit(sdata);
		return;
	}
#endif
}


bool mac80211_tx_compress(struct ieee80211_sub_if_data * sdata, struct sk_buff * skb)
{
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)skb->data;
	u8 *in;
	size_t inlen = 0, outlen = 0, hdrlen;
	if (!sdata->dev->ieee80211_ptr->use_compr) {
		return false;
	}

	if (!ieee80211_is_data_present(hdr->frame_control)) {
		return false;
	}
	if ((hdrlen = ieee80211_hdrlen(hdr->frame_control)) < 24) {
		return false;
	}
	if (skb_is_nonlinear(skb) && skb_linearize(skb)) {
		return false;
	}
#ifdef CPTCFG_MAC80211_TDMA
	if (sdata->vif.type == NL80211_IFTYPE_TDMA)
		hdrlen += tdma_hdr_len();
#endif
	inlen = skb->len - hdrlen;
	if (inlen >= sdata->dev->ieee80211_ptr->compr_threshold) {
		in = (u8 *)((u8 *)skb->data + hdrlen);
		if (sdata->tx_outbuf == NULL) {
#if IS_ENABLED(CPTCFG_MAC80211_COMPRESS)
			mac80211_compress_init(sdata);
#endif
			if (sdata->tx_outbuf == NULL) {
				return false;
			}
		}
		switch (sdata->dev->ieee80211_ptr->use_compr) {
		case COMPR_TYPE_LZO:
			lzo1x_1_compress(in, inlen, sdata->tx_outbuf, &outlen, sdata->tx_wrkbuf);
			break;
		case COMPR_TYPE_LZMA:
			LzmaEnc_MemEncode(sdata->lzma_p, sdata->tx_outbuf, &outlen, in, inlen, 0, NULL, &lzma_alloc, &lzma_alloc);
			break;
		case COMPR_TYPE_LZ4:
			lz4_compress(in, inlen, sdata->tx_outbuf, &outlen, sdata->tx_wrkbuf);
			break;
#ifdef ZSTD
		case COMPR_TYPE_ZSTD:
			outlen = zstd_compress(sdata, in, sdata->tx_outbuf, inlen, 4096, 3);
			break;
#endif
		default:
			return true;
			break;
		}
//          printk("COMPRESS: Initial len - %lu. Result len - %lu. Ratio %u percents\n", inlen, outlen, outlen * 100 / inlen);
		if ((outlen > 0) && ((outlen + sizeof(u16) + sizeof(__u8)) < inlen)) {
			put_unaligned_le16((u16)outlen, (__le16 *)in);
			in += sizeof(__le16);
			put_unaligned((u8)sdata->dev->ieee80211_ptr->use_compr, (__u8 *)in);	// 1 = lzo, 2 = lzma ....
			in += sizeof(__u8);
			memcpy(in, sdata->tx_outbuf, outlen);
			inlen = hdrlen + sizeof(__le16) + sizeof(__u8) + outlen;
			pskb_trim(skb, inlen);
			skb->len = inlen;
			hdr->frame_control |= cpu_to_le16((ieee80211_is_data_qos(hdr->frame_control) ? IEEE80211_STYPE_QOS_DATA_CFACK : IEEE80211_STYPE_DATA_CFACK));
			skb->ip_summed = CHECKSUM_UNNECESSARY;
#ifdef CPTCFG_MAC80211_DEBUGFS
			sdata->compressed++;
			sdata->tx_compressed_bytes += outlen;
#endif
		}
	}
	return true;
}


static ieee80211_tx_result ieee80211_tx_h_compress(struct ieee80211_tx_data * tx)
{
	mac80211_tx_compress(tx->sdata, tx->skb);
	return TX_CONTINUE;
}

static ieee80211_rx_result ieee80211_rx_h_decompress(struct ieee80211_rx_data * rx)
{
	struct ieee80211_hdr *hdr = (struct ieee80211_hdr *)rx->skb->data;
	u8 *in;
	int ret;
	SizeT dl;
	SizeT sl;
	int compression = 0;
	ELzmaStatus status;
	size_t inlen = 0, outlen = LZO1X_1_MEM_COMPRESS, hdrlen, extralen, reqlen;
	ieee80211_rx_result res = RX_DROP_MONITOR;
	bool qos;
	__le16 flag;

	if (!ieee80211_is_data_present(hdr->frame_control)) {
		return RX_CONTINUE;
	}
	if ((hdrlen = ieee80211_hdrlen(hdr->frame_control)) == 0) {
		return RX_CONTINUE;
	}
	qos = ieee80211_is_data_qos(hdr->frame_control);
	flag = cpu_to_le16((qos ? IEEE80211_STYPE_QOS_DATA_CFACK : IEEE80211_STYPE_DATA_CFACK));
	if ((hdr->frame_control & flag) != flag) {
		return RX_CONTINUE;
	}
	if (skb_is_nonlinear(rx->skb) && skb_linearize(rx->skb))
		return res;
#ifdef CPTCFG_MAC80211_TDMA
	if (rx->sdata->vif.type == NL80211_IFTYPE_TDMA)
		hdrlen += tdma_hdr_len();
#endif
	if (rx->skb->len < hdrlen + sizeof(u16) + sizeof(u8))
		return res;
	in = (u8 *)((u8 *)rx->skb->data + hdrlen);
	inlen = (size_t)get_unaligned_le16((void *)in);
	in += sizeof(u16);
	compression = (u8)get_unaligned((u8 *)in);
	in += sizeof(u8);
	if (rx->skb->len < inlen + hdrlen + sizeof(__le16) + sizeof(u8)) {
		return res;
	}
	extralen = rx->skb->len - inlen - hdrlen - sizeof(__le16) - sizeof(u8);
	hdr->frame_control &= ~flag;
	if (qos)
		hdr->frame_control |= cpu_to_le16(IEEE80211_STYPE_QOS_DATA);
	if (rx->sdata->rx_outbuf == NULL) {
		mac80211_compress_init(rx->sdata);
		if (rx->sdata->rx_outbuf == NULL) {
			return RX_CONTINUE;
		}
	}
	switch (compression) {
	case COMPR_TYPE_LZO:
		lzo1x_decompress_safe(in, inlen, rx->sdata->rx_outbuf, &outlen);
		break;
	case COMPR_TYPE_LZMA:
		dl = (SizeT) outlen;
		sl = (SizeT) inlen;
		ret = LzmaDecode(rx->sdata->rx_outbuf, &dl, in, &sl, rx->sdata->lzma_propsEncoded, rx->sdata->lzma_propsSize, LZMA_FINISH_ANY, &status, &lzma_alloc);
		if (ret != SZ_OK || status == LZMA_STATUS_NOT_FINISHED)
			return res;
		outlen = dl;
		break;
	case COMPR_TYPE_LZ4:
		lz4_decompress_unknownoutputsize(in, inlen, rx->sdata->rx_outbuf, &outlen);
		break;
#ifdef ZSTD
	case COMPR_TYPE_ZSTD:
		outlen = zstd_decompress(rx->sdata, in, rx->sdata->rx_outbuf, inlen, 4096);
		break;
#endif
	default:
		return RX_CONTINUE;
		break;
	}
//      printk("DECOMPRESS: Incoming len - %lu, Result len - %lu. Ratio %u percents\n", inlen, outlen, outlen * 100 / inlen);
	if (outlen > inlen + sizeof(u16) + sizeof(u8)) {
		reqlen = outlen - inlen - sizeof(__le16) - sizeof(u8);
		if (skb_tailroom(rx->skb) < reqlen) {
			if (pskb_expand_head(rx->skb, 0, reqlen - skb_tailroom(rx->skb), GFP_ATOMIC)) {
				//printk("DECOMPRESS: Could not allocate memory for new skb\n");
				goto rx_decompr_out;
			}
		}
		if (extralen)
			memcpy((u8 *)((u8 *)rx->skb->data + hdrlen + outlen), (u8 *)((u8 *)rx->skb->data + inlen + hdrlen + sizeof(__le16) + sizeof(u8)), extralen);
		in = (u8 *)((u8 *)rx->skb->data + hdrlen);
		memcpy(in, rx->sdata->rx_outbuf, outlen);
		skb_set_tail_pointer(rx->skb, outlen + hdrlen + extralen);
		rx->skb->len = outlen + hdrlen + extralen;
		rx->skb->ip_summed = CHECKSUM_UNNECESSARY;
		res = RX_CONTINUE;
#ifdef CPTCFG_MAC80211_DEBUGFS
		rx->sdata->decompressed++;
		rx->sdata->rx_compressed_bytes += inlen;
#endif
	} else {
//              printk("DECOMPRESS: Decompressed len not bigger than initial\n");
	}
rx_decompr_out:
	return res;
}

size_t decompress_wrapper(struct ieee80211_sub_if_data *sdata, char *in, size_t inlen, u8 compression)
{
	size_t outlen = LZO1X_1_MEM_COMPRESS;
	int ret;
	SizeT dl;
	SizeT sl;
	ELzmaStatus status;
	switch (compression) {
	case COMPR_TYPE_LZO:
		lzo1x_decompress_safe(in, inlen, sdata->rx_outbuf, &outlen);
		break;
	case COMPR_TYPE_LZMA:
		dl = (SizeT) outlen;
		sl = (SizeT) inlen;
		ret = LzmaDecode(sdata->rx_outbuf, &dl, in, &sl, sdata->lzma_propsEncoded, sdata->lzma_propsSize, LZMA_FINISH_ANY, &status, &lzma_alloc);
		if (ret != SZ_OK || status == LZMA_STATUS_NOT_FINISHED)
			return 0;
		outlen = dl;
		break;
	case COMPR_TYPE_LZ4:
		lz4_decompress_unknownoutputsize(in, inlen, sdata->rx_outbuf, &outlen);
		break;
#ifdef ZSTD
	case COMPR_TYPE_ZSTD:
		outlen = zstd_decompress(sdata, in, sdata->rx_outbuf, inlen, 4096);
		break;
#endif
	default:
		return 0;
	}
	return outlen;
}

MODULE_LICENSE("GPL");
