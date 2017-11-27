
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>

#include <libnetfilter_log/libnetfilter_log.h>

static int print_pkt(struct nflog_data *ldata)
{
	struct nfulnl_msg_packet_hdr *ph = nflog_get_msg_packet_hdr(ldata);
	u_int32_t mark = nflog_get_nfmark(ldata);
	u_int32_t indev = nflog_get_indev(ldata);
	u_int32_t outdev = nflog_get_outdev(ldata);
	char *prefix = nflog_get_prefix(ldata);
	char *payload;
	int payload_len = nflog_get_payload(ldata, &payload);
	
	if (ph) {
		printf("hw_protocol=0x%04x hook=%u ", 
			ntohs(ph->hw_protocol), ph->hook);
	}

	printf("mark=%u ", mark);

	if (indev > 0)
		printf("indev=%u ", indev);

	if (outdev > 0)
		printf("outdev=%u ", outdev);


	if (prefix) {
		printf("prefix=\"%s\" ", prefix);
	}
	if (payload_len >= 0)
		printf("payload_len=%d ", payload_len);

	fputc('\n', stdout);
	return 0;
}

static int cb(struct nflog_g_handle *gh, struct nfgenmsg *nfmsg,
		struct nflog_data *nfa, void *data)
{
	print_pkt(nfa);
	return 0;
}


int main(int argc, char **argv)
{
	struct nflog_handle *h;
	struct nflog_g_handle *qh;
	struct nflog_g_handle *qh100;
	int rv, fd;
	char buf[4096];

	h = nflog_open();
	if (!h) {
		fprintf(stderr, "error during nflog_open()\n");
		exit(1);
	}

	printf("unbinding existing nf_log handler for AF_INET (if any)\n");
	if (nflog_unbind_pf(h, AF_INET) < 0) {
		fprintf(stderr, "error nflog_unbind_pf()\n");
		exit(1);
	}

	printf("binding nfnetlink_log to AF_INET\n");
	if (nflog_bind_pf(h, AF_INET) < 0) {
		fprintf(stderr, "error during nflog_bind_pf()\n");
		exit(1);
	}
	printf("binding this socket to group 0\n");
	qh = nflog_bind_group(h, 0);
	if (!qh) {
		fprintf(stderr, "no handle for grup 0\n");
		exit(1);
	}

	printf("binding this socket to group 100\n");
	qh100 = nflog_bind_group(h, 100);
	if (!qh100) {
		fprintf(stderr, "no handle for group 100\n");
		exit(1);
	}

	printf("setting copy_packet mode\n");
	if (nflog_set_mode(qh, NFULNL_COPY_PACKET, 0xffff) < 0) {
		fprintf(stderr, "can't set packet copy mode\n");
		exit(1);
	}

	fd = nflog_fd(h);

	printf("registering callback for group 0\n");
	nflog_callback_register(qh, &cb, NULL);

	printf("going into main loop\n");
	while ((rv = recv(fd, buf, sizeof(buf), 0)) && rv >= 0) {
		printf("pkt received (len=%u)\n", rv);

		/* handle messages in just-received packet */
		nflog_handle_packet(h, buf, rv);
	}

	printf("unbinding from group 100\n");
	nflog_unbind_group(qh100);
	printf("unbinding from group 0\n");
	nflog_unbind_group(qh);

#ifdef INSANE
	/* norally, applications SHOULD NOT issue this command,
	 * since it detaches other programs/sockets from AF_INET, too ! */
	printf("unbinding from AF_INET\n");
	nflog_unbind_pf(h, AF_INET);
#endif

	printf("closing handle\n");
	nflog_close(h);

	return EXIT_SUCCESS;
}
