/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

#include "iptraf-ng-compat.h"

#include "packet.h"
#include "capt.h"

struct capt_data_mmap_v3 {
	void				*mmap;
	size_t				mmap_size;
	int				hdrlen;
	struct tpacket_block_desc	**pbds;
	struct tpacket_block_desc	*pbd;
	struct tpacket3_hdr		*ppd;
	unsigned int			curblock;
	unsigned int			lastblock;

	uint32_t			num_pkts;	/* only for debug */
	uint32_t			cur_pkt;	/* only for debug */
};

#define BLOCKS 256		/* 256 blocks / 512 frames in each block:
				 *   this gets room for 128k packets, which
				 *   "should be enough for everybody" ;-) */

#define FRAMES_PER_BLOCK 512	/* keep it as power-of-two (dramaticly lowers
				 * CPU utilization) */

#define FRAMES (BLOCKS * FRAMES_PER_BLOCK)	/* frames over all blocks */

#define FRAME_SIZE TPACKET_ALIGN(MAX_PACKET_SIZE + TPACKET3_HDRLEN)

static struct tpacket_block_desc *capt_mmap_find_filled_block(struct capt_data_mmap_v3 *data)
{
	for (unsigned int i = data->lastblock; i < data->lastblock + BLOCKS; i++) {
		unsigned int block = i >= BLOCKS ? i - BLOCKS : i;

		if (data->pbds[block]->hdr.bh1.block_status & TP_STATUS_USER) {
			data->curblock = block;
			return data->pbds[block];
		}
	}
	return NULL;
}

static bool capt_have_packet_mmap_v3(struct capt *capt)
{
	struct capt_data_mmap_v3 *data = capt->priv;

	if (data->pbd != NULL)
		return true;

	data->pbd = capt_mmap_find_filled_block(data);
	if (data->pbd != NULL)
		return true;
	else
		return false;
}

static int capt_get_packet_mmap_v3(struct capt *capt, struct pkt_hdr *pkt)
{
	struct capt_data_mmap_v3 *data = capt->priv;

	if (data->pbd == NULL)
		data->pbd = capt_mmap_find_filled_block(data);

	if (data->pbd == NULL)
		return 0;	/* no packet ready */

	if (data->ppd == NULL) {
		data->ppd = (struct tpacket3_hdr *) ((uint8_t *)data->pbd + data->pbd->hdr.bh1.offset_to_first_pkt);
		data->num_pkts = data->pbd->hdr.bh1.num_pkts;
	}

	/* here should be at least one packet ready */
	pkt->pkt_buf = (char *)data->ppd + data->ppd->tp_mac;
	pkt->pkt_payload = NULL;
	pkt->pkt_caplen = data->ppd->tp_snaplen;
	pkt->pkt_len = data->ppd->tp_len;
	pkt->from = (struct sockaddr_ll *)((uint8_t *)data->ppd + data->hdrlen);
	pkt->pkt_protocol = ntohs(pkt->from->sll_protocol);

	return pkt->pkt_len;
}

static int capt_put_packet_mmap_v3(struct capt *capt, struct pkt_hdr *pkt __unused)
{
	struct capt_data_mmap_v3 *data = capt->priv;

	if (data->ppd->tp_next_offset != 0) {
		data->ppd = (struct tpacket3_hdr *)((uint8_t *)data->ppd + data->ppd->tp_next_offset);
		data->cur_pkt++;
	} else {
		data->ppd = NULL;
		data->num_pkts = 0;
		data->cur_pkt = 0;
		data->pbd->hdr.bh1.block_status = TP_STATUS_KERNEL;
		data->pbd = NULL;
		data->lastblock = data->curblock;
	}

	return 0;
}

static unsigned long capt_get_dropped_mmap_v3(struct capt *capt)
{
	struct tpacket_stats_v3 stats;
	socklen_t len = sizeof(stats);

	memset(&stats, 0, len);
	int err = getsockopt(capt->fd, SOL_PACKET, PACKET_STATISTICS, &stats, &len);
	if (err < 0)
		die_errno("%s(): getsockopt(PACKET_STATISTICS)", __func__);

	capt->dropped += stats.tp_drops;

	return capt->dropped;
}

static void capt_cleanup_mmap_v3(struct capt *capt)
{
	struct capt_data_mmap_v3 *data = capt->priv;

	free(data->pbds);

	munlock(data->mmap, data->mmap_size);
	munmap(data->mmap, data->mmap_size);

	memset(data, 0, sizeof(*data));

	free(capt->priv);
	capt->priv = NULL;

	capt->cleanup = NULL;
	capt->get_dropped = NULL;
	capt->put_packet = NULL;
	capt->get_packet = NULL;
	capt->have_packet = NULL;
}

int capt_setup_mmap_v3(struct capt *capt)
{
	if (capt_get_socket(capt) == -1)
		return -1;

	int version = TPACKET_V3;
	if (setsockopt(capt->fd, SOL_PACKET, PACKET_VERSION, &version, sizeof(version)) != 0)
		goto err;

	int hdrlen = version;
	socklen_t socklen = sizeof(hdrlen);
	if (getsockopt(capt->fd, SOL_PACKET, PACKET_HDRLEN, &hdrlen, &socklen) != 0)
		goto err;

	struct tpacket_req3 req;

	req.tp_block_nr = BLOCKS;
	req.tp_frame_nr = FRAMES;
	req.tp_frame_size = FRAME_SIZE;
	req.tp_block_size = FRAMES_PER_BLOCK * req.tp_frame_size;

	req.tp_retire_blk_tov = 20;	/* block retire timeout in msec */
	req.tp_sizeof_priv = 0;
	// req.tp_feature_req_word = TP_FT_REQ_FILL_RXHASH;

	if(setsockopt(capt->fd, SOL_PACKET, PACKET_RX_RING, &req, sizeof(req)) != 0)
		goto err;

	size_t size = req.tp_block_size * req.tp_block_nr;
	void *map = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, capt->fd, 0);
	if (map == MAP_FAILED)
		goto err;

	/* try to lock this memory to RAM */
	(void)mlock(map, size);	/* no need to check return value because the mlock() is
				 * not mandatory; if it fails packet capture just works OK
				 * albeit suboptimally */

	struct capt_data_mmap_v3 *data = xmallocz(sizeof(struct capt_data_mmap_v3));

	data->hdrlen = hdrlen;
	data->mmap = map;
	data->mmap_size = size;

	data->pbds = xmallocz(BLOCKS * sizeof(*data->pbd));
	for (int i = 0; i < BLOCKS; i++) {
		data->pbds[i] = map + i * req.tp_block_size;
	}

	capt->priv		= data;
	capt->have_packet	= capt_have_packet_mmap_v3;
	capt->get_packet	= capt_get_packet_mmap_v3;
	capt->put_packet	= capt_put_packet_mmap_v3;
	capt->get_dropped	= capt_get_dropped_mmap_v3;
	capt->cleanup		= capt_cleanup_mmap_v3;

	return 0;	/* All O.K. */
err:
	capt_put_socket(capt);
	return -1;
}
