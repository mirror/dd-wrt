/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

#include "iptraf-ng-compat.h"

#include "packet.h"
#include "capt.h"

struct capt_data_recvmsg {
	char			*buf;

	struct iovec		iov;
	struct msghdr		*msg;
	struct sockaddr_ll	*from;
};

static bool capt_have_packet_recvmsg(struct capt *capt __unused)
{
	return false;
}

static int capt_get_packet_recvmsg(struct capt *capt, struct pkt_hdr *pkt)
{
	struct capt_data_recvmsg *data = capt->priv;

	/* these are set upon return from recvmsg() so clean */
	/* them beforehand */
	data->msg->msg_controllen = 0;
	data->msg->msg_flags = 0;

	ssize_t len = recvmsg(capt->fd, data->msg, MSG_TRUNC | MSG_DONTWAIT);
	if (len > 0) {
		pkt->pkt_len = len;
		pkt->pkt_caplen = len;
		if (pkt->pkt_caplen > MAX_PACKET_SIZE)
			pkt->pkt_caplen = MAX_PACKET_SIZE;
		pkt->pkt_buf = data->buf;
		pkt->from = data->from;
		pkt->pkt_payload = NULL;
		pkt->pkt_protocol = ntohs(pkt->from->sll_protocol);
	}
	return len;
}

static void capt_cleanup_recvmsg(struct capt *capt)
{
	struct capt_data_recvmsg *data = capt->priv;

	capt->cleanup = NULL;
	capt->put_packet = NULL;
	capt->get_packet = NULL;
	capt->have_packet = NULL;

	free(data->from);
	data->from = NULL;
	free(data->msg);
	data->msg = NULL;

	free(data->buf);
	data->buf = NULL;
	free(capt->priv);
	capt->priv = NULL;
}

int capt_setup_recvmsg(struct capt *capt)
{
	struct capt_data_recvmsg *data;

	if (capt_get_socket(capt) == -1)
		return -1;

	data			= xmallocz(sizeof(struct capt_data_recvmsg));
	data->buf		= xmallocz(MAX_PACKET_SIZE);
	data->iov.iov_len	= MAX_PACKET_SIZE;
	data->iov.iov_base	= data->buf;

	data->msg		= xmallocz(sizeof(*data->msg));
	data->from		= xmallocz(sizeof(*data->from));

	data->msg->msg_name	= data->from;
	data->msg->msg_namelen	= sizeof(*data->from);
	data->msg->msg_iov	= &data->iov;
	data->msg->msg_iovlen	= 1;
	data->msg->msg_control	= NULL;

	capt->priv		= data;
	capt->have_packet	= capt_have_packet_recvmsg;
	capt->get_packet	= capt_get_packet_recvmsg;
	capt->put_packet	= NULL;
	capt->cleanup		= capt_cleanup_recvmsg;

	return 0;
}
