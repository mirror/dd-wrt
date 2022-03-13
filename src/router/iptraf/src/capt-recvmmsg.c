/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

#include "iptraf-ng-compat.h"

#include "packet.h"
#include "capt.h"

#define FRAMES 128

struct capt_data_recvmmsg {
	char			*buf;

	struct mmsghdr		*msgvec;
	struct iovec		*iov;
	struct sockaddr_ll	*from;

	unsigned int		lastslot;
	unsigned int		slot;
};

static unsigned int capt_recvmmsg_find_filled_slot(struct capt_data_recvmmsg *data)
{
	for (unsigned int i = data->lastslot; i < data->lastslot + FRAMES; i++) {
		unsigned int slot = i >= FRAMES ? i - FRAMES : i;

		if (data->msgvec[slot].msg_len != 0)
			return slot;
	}
	return FRAMES;
}

static bool capt_have_packet_recvmmsg(struct capt *capt)
{
	struct capt_data_recvmmsg *data = capt->priv;

	if (capt_recvmmsg_find_filled_slot(data) != FRAMES)
		return true;
	else
		return false;
}

static int capt_get_packet_recvmmsg(struct capt *capt, struct pkt_hdr *pkt)
{
	struct capt_data_recvmmsg *data = capt->priv;
	int ret = 0;

	unsigned int slot = capt_recvmmsg_find_filled_slot(data);
	if (slot == FRAMES) {
		/* these are set upon return from recvmsg() so clean */
		/* them beforehand */
		for (unsigned int i = 0; i < FRAMES; i++) {
			data->msgvec[i].msg_hdr.msg_controllen = 0;
			data->msgvec[i].msg_hdr.msg_flags = 0;
			data->msgvec[i].msg_len = 0;
		}

		int received = recvmmsg(capt->fd, data->msgvec, FRAMES, MSG_TRUNC | MSG_DONTWAIT, NULL);
		if (received <= 0)
			return received;
		slot = 0;
	}
	pkt->pkt_len = data->msgvec[slot].msg_len;
	pkt->pkt_caplen = data->msgvec[slot].msg_len;
	if (pkt->pkt_caplen > MAX_PACKET_SIZE)
		pkt->pkt_caplen = MAX_PACKET_SIZE;
	pkt->pkt_buf = data->buf + slot * MAX_PACKET_SIZE;
	pkt->from = &data->from[slot];
	pkt->pkt_payload = NULL;
	pkt->pkt_protocol = ntohs(pkt->from->sll_protocol);

	data->slot = slot;

	return ret;
}

static int capt_put_packet_recvmmsg(struct capt *capt, struct pkt_hdr *pkt __unused)
{
	struct capt_data_recvmmsg *data = capt->priv;

	/* hand out processed slot to kernel */
	if (data->slot < FRAMES) {
		data->msgvec[data->slot].msg_len = 0;
		data->lastslot = data->slot;
	} else
		data->slot = FRAMES;

	return 0;
}

static void capt_cleanup_recvmmsg(struct capt *capt)
{
	struct capt_data_recvmmsg *data = capt->priv;

	capt->cleanup = NULL;
	capt->put_packet = NULL;
	capt->get_packet = NULL;
	capt->have_packet = NULL;

	free(data->from);
	data->from = NULL;
	free(data->iov);
	data->iov = NULL;
	free(data->msgvec);
	data->msgvec = NULL;
	free(data->buf);
	data->buf = NULL;

	free(capt->priv);
	capt->priv = NULL;
}

int capt_setup_recvmmsg(struct capt *capt)
{
	struct capt_data_recvmmsg *data;

	if (capt_get_socket(capt) == -1)
		return -1;

	data			= xmallocz(sizeof(struct capt_data_recvmmsg));
	data->buf		= xmallocz(FRAMES * MAX_PACKET_SIZE);
	data->msgvec		= xmallocz(FRAMES * sizeof(*data->msgvec));
	data->iov		= xmallocz(FRAMES * sizeof(*data->iov));
	data->from		= xmallocz(FRAMES * sizeof(*data->from));

	for (unsigned int i = 0; i < FRAMES; i++) {
		data->iov[i].iov_len	= MAX_PACKET_SIZE;
		data->iov[i].iov_base	= data->buf + i * MAX_PACKET_SIZE;

		data->msgvec[i].msg_hdr.msg_name	= &data->from[i];
		data->msgvec[i].msg_hdr.msg_namelen	= sizeof(data->from[i]);
		data->msgvec[i].msg_hdr.msg_iov		= &data->iov[i];
		data->msgvec[i].msg_hdr.msg_iovlen	= 1;
		data->msgvec[i].msg_hdr.msg_control	= NULL;
	}
	data->slot = FRAMES;
	data->lastslot = 0;

	capt->priv		= data;
	capt->have_packet	= capt_have_packet_recvmmsg;
	capt->get_packet	= capt_get_packet_recvmmsg;
	capt->put_packet	= capt_put_packet_recvmmsg;
	capt->cleanup		= capt_cleanup_recvmmsg;

	return 0;
}
