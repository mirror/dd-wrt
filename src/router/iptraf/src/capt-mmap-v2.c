/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

#include "iptraf-ng-compat.h"

#include "packet.h"
#include "capt.h"

struct capt_data_mmap_v2 {
	void			*mmap;
	size_t			mmap_size;
	struct tpacket2_hdr	**hdr;
	struct sockaddr_ll	**sll;
	unsigned int		lastslot;
	unsigned int		slot;
};

#define FRAMES 512
#define FRAME_SIZE TPACKET_ALIGN(MAX_PACKET_SIZE + TPACKET2_HDRLEN)

static unsigned int capt_mmap_find_filled_slot(struct capt_data_mmap_v2 *data)
{
	for (unsigned int i = data->lastslot; i < data->lastslot + FRAMES; i++) {
		unsigned int slot = i >= FRAMES ? i - FRAMES : i;

		if (data->hdr[slot]->tp_status & TP_STATUS_USER)
			return slot;
	}
	return FRAMES;
}

static bool capt_have_packet_mmap_v2(struct capt *capt)
{
	struct capt_data_mmap_v2 *data = capt->priv;

	if (capt_mmap_find_filled_slot(data) != FRAMES)
		return true;
	else
		return false;
}

static int capt_get_packet_mmap_v2(struct capt *capt, struct pkt_hdr *pkt)
{
	struct capt_data_mmap_v2 *data = capt->priv;
	int ss = 0;

	unsigned int slot = capt_mmap_find_filled_slot(data);
	if (slot < FRAMES) {
		struct tpacket2_hdr *hdr = data->hdr[slot];
		struct sockaddr_ll *sll = data->sll[slot];

		pkt->pkt_buf = (char *)hdr + hdr->tp_mac;
		pkt->pkt_payload = NULL;
		pkt->pkt_caplen = hdr->tp_snaplen;
		pkt->pkt_len = hdr->tp_len;
		pkt->from = sll;
		pkt->pkt_protocol = ntohs(sll->sll_protocol);

		data->slot = slot;
		ss = hdr->tp_len;
	}
	return ss;
}

static int capt_put_packet_mmap_v2(struct capt *capt, struct pkt_hdr *pkt __unused)
{
	struct capt_data_mmap_v2 *data = capt->priv;

	/* hand out processed slot to kernel */
	if (data->slot < FRAMES) {
		data->hdr[data->slot]->tp_status = TP_STATUS_KERNEL;
		data->lastslot = data->slot;
	} else
		data->slot = FRAMES;

	return 0;
}

static void capt_cleanup_mmap_v2(struct capt *capt)
{
	struct capt_data_mmap_v2 *data = capt->priv;

	free(data->sll);
	data->sll = NULL;

	free(data->hdr);
	data->hdr = NULL;

	munlock(data->mmap, data->mmap_size);
	munmap(data->mmap, data->mmap_size);
	data->mmap = NULL;
	data->mmap_size = 0;

	free(capt->priv);
	capt->priv = NULL;

	capt->cleanup = NULL;
	capt->put_packet = NULL;
	capt->get_packet = NULL;
	capt->have_packet = NULL;
}

int capt_setup_mmap_v2(struct capt *capt)
{
	if (capt_get_socket(capt) == -1)
		return -1;

	int version = TPACKET_V2;
	if (setsockopt(capt->fd, SOL_PACKET, PACKET_VERSION, &version, sizeof(version)) != 0)
		goto err;

	int hdrlen = version;
	socklen_t socklen = sizeof(hdrlen);
	if (getsockopt(capt->fd, SOL_PACKET, PACKET_HDRLEN, &hdrlen, &socklen) != 0)
		goto err;

	struct tpacket_req req;

	req.tp_block_nr = 1;	/* TODO: number of CPUs */
	req.tp_frame_nr = FRAMES;
	req.tp_frame_size = FRAME_SIZE;
	req.tp_block_size = req.tp_frame_nr * req.tp_frame_size;

	if (setsockopt(capt->fd, SOL_PACKET, PACKET_RX_RING, &req, sizeof(req)) != 0)
		goto err;

	size_t size = req.tp_block_size * req.tp_block_nr;
	void *map = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, capt->fd, 0);
	if (map == MAP_FAILED)
		goto err;

	/* try to lock this memory to RAM */
	(void)mlock(map, size);	/* no need to check return value because the mlock() is
				 * not mandatory; if it fails packet capture just works OK
				 * albeit suboptimally */

	struct capt_data_mmap_v2 *data = xmallocz(sizeof(struct capt_data_mmap_v2));

	data->mmap = map;
	data->mmap_size = size;
	data->hdr = xmallocz(FRAMES * sizeof(*data->hdr));
	data->sll = xmallocz(FRAMES * sizeof(*data->sll));
	for (int i = 0; i < FRAMES; i++) {
		data->hdr[i] = (struct tpacket2_hdr *)((char *)map + i * FRAME_SIZE);
		data->sll[i] = (struct sockaddr_ll *)((char *)data->hdr[i] + TPACKET_ALIGN(hdrlen));
	}
	data->lastslot = 0;
	data->slot = FRAMES;

	capt->priv		= data;
	capt->have_packet	= capt_have_packet_mmap_v2;
	capt->get_packet	= capt_get_packet_mmap_v2;
	capt->put_packet	= capt_put_packet_mmap_v2;
	capt->cleanup		= capt_cleanup_mmap_v2;

	return 0;	/* All O.K. */
err:
	capt_put_socket(capt);
	return -1;
}
