#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <libnfnetlink/libnfnetlink.h>
#include <libnetfilter_log/libnetfilter_log.h>
#include <libnetfilter_log/libipulog.h>

/* private */
#define PAYLOAD_SIZE	0xffff


struct ipulog_handle
{
	struct nflog_handle *nfulh;
	struct nflog_g_handle *nful_gh;
	struct nlmsghdr *last_nlh;
#if 0
	int fd;
	u_int8_t blocking;
	struct sockaddr_nl local;
	struct sockaddr_nl peer;
#endif
	struct ulog_packet_msg upmsg;	/* has to be last in structure */
};

static const struct ipulog_errmap_t
{
	int errcode;
	const char *message;
} ipulog_errmap[] = 
{
	{ IPULOG_ERR_NONE, "No error" },
	{ IPULOG_ERR_IMPL, "Not implemented yet" },
	{ IPULOG_ERR_HANDLE, "Unable to create netlink handle" },
	{ IPULOG_ERR_SOCKET, "Unable to create netlink socket" },
	{ IPULOG_ERR_BIND, "Unable to bind netlink socket" },
	{ IPULOG_ERR_RECVBUF, "Receive buffer size invalid" },
	{ IPULOG_ERR_RECV, "Error during netlink receive" },
	{ IPULOG_ERR_NLEOF, "Received EOF on netlink socket" },
	{ IPULOG_ERR_TRUNC, "Receive message truncated" },
	{ IPULOG_ERR_INVGR, "Invalid group specified" },
	{ IPULOG_ERR_INVNL, "Invalid netlink message" },
};

/* obviously this only finds the highest group in the mask */
static unsigned int gmask2group(unsigned int gmask)
{
	int bit;

	for (bit = sizeof(gmask)*4 -1; bit >= 0; bit--) {
		if (gmask & (1 << bit))
			return bit+1;
	}
	return 0;
}



/* public */

int ipulog_errno = IPULOG_ERR_NONE;

const char *ipulog_strerror(int errcode)
{
	if (errcode < 0 || errcode > IPULOG_MAXERR)
		errcode = IPULOG_ERR_IMPL;
	return ipulog_errmap[errcode].message;
}

/* convert a netlink group (1-32) to a group_mask suitable for create_handle */
u_int32_t ipulog_group2gmask(u_int32_t group)
{
	if (group < 1 || group > 32)
	{
		ipulog_errno = IPULOG_ERR_INVGR;
		return 0;
	}
	return (1 << (group - 1));
}

/* create a ipulog handle for the reception of packets sent to gmask */
struct ipulog_handle *ipulog_create_handle(u_int32_t gmask, 
					   u_int32_t rcvbufsize)
{
	int rv;
	struct ipulog_handle *h;
	unsigned int group = gmask2group(gmask);

	h = malloc(sizeof(*h)+PAYLOAD_SIZE);
	if (! h) {
		ipulog_errno = IPULOG_ERR_HANDLE;
		return NULL;
	}
	memset(h, 0, sizeof(*h));
	h->nfulh = nflog_open();
	if (!h->nfulh)
		goto out_free;
	
	/* bind_pf returns EEXIST if we are already registered */
	rv = nflog_bind_pf(h->nfulh, AF_INET);
	if (rv < 0 && rv != -EEXIST)
		goto out_free;

	h->nful_gh = nflog_bind_group(h->nfulh, group);
	if (!h->nful_gh)
		goto out_free;

	return h;

out_free:
	ipulog_errno = IPULOG_ERR_HANDLE;
	free(h);
	return NULL;
}

void ipulog_destroy_handle(struct ipulog_handle *h)
{
	nflog_unbind_group(h->nful_gh);
	nflog_close(h->nfulh);
	free(h);
}

ulog_packet_msg_t *ipulog_get_packet(struct ipulog_handle *h,
				     const unsigned char *buf,
				     size_t len)
{
	struct nlmsghdr *nlh;
	struct nfattr *tb[NFULA_MAX];
	struct nfulnl_msg_packet_hdr *hdr;

	if (!h->last_nlh) {
		printf("first\n");
		nlh = nfnl_get_msg_first(nflog_nfnlh(h->nfulh), buf, len);
	}else {
next_msg:	printf("next\n");
		nlh = nfnl_get_msg_next(nflog_nfnlh(h->nfulh), buf, len);
	}
	h->last_nlh = nlh;

	if (!nlh)
		return NULL;

	nfnl_parse_attr(tb, NFULA_MAX, NFM_NFA(NLMSG_DATA(nlh)),
			NFM_PAYLOAD(nlh));
	
	if (!tb[NFULA_PACKET_HDR-1])
		goto next_msg;

	/* now build the fake ulog_packet_msg */
	hdr = NFA_DATA(tb[NFULA_PACKET_HDR-1]);
	h->upmsg.hook = hdr->hook;

	if (tb[NFULA_MARK-1])
		h->upmsg.mark = ntohl(*(u_int32_t *)NFA_DATA(tb[NFULA_MARK-1]));
	else
		h->upmsg.mark = 0;

	if (tb[NFULA_TIMESTAMP]) {
		/* FIXME: 64bit network-to-host */
		h->upmsg.timestamp_sec = h->upmsg.timestamp_usec = 0;
	} else
		h->upmsg.timestamp_sec = h->upmsg.timestamp_usec = 0;

	if (tb[NFULA_IFINDEX_INDEV-1]) {
		/* FIXME: ifindex lookup */	
		h->upmsg.indev_name[0] = '\0';
	} else
		h->upmsg.indev_name[0] = '\0';

	if (tb[NFULA_IFINDEX_OUTDEV-1]) {
		/* FIXME: ifindex lookup */	
		h->upmsg.outdev_name[0] = '\0';
	} else
		h->upmsg.outdev_name[0] = '\0';

	if (tb[NFULA_HWADDR-1]) {
		struct nfulnl_msg_packet_hw *phw = NFA_DATA(tb[NFULA_HWADDR-1]);
		h->upmsg.mac_len = ntohs(phw->hw_addrlen);
		memcpy(h->upmsg.mac, phw->hw_addr, 8);
	} else
		h->upmsg.mac_len = 0;

	if (tb[NFULA_PREFIX-1]) {
		int plen = NFA_PAYLOAD(tb[NFULA_PREFIX-1]);
		if (ULOG_PREFIX_LEN < plen)
			plen = ULOG_PREFIX_LEN;
		memcpy(h->upmsg.prefix, NFA_DATA(tb[NFULA_PREFIX-1]), plen);
		h->upmsg.prefix[ULOG_PREFIX_LEN-1] = '\0';
	}

	if (tb[NFULA_PAYLOAD-1]) {
		memcpy(h->upmsg.payload, NFA_DATA(tb[NFULA_PAYLOAD-1]),
			NFA_PAYLOAD(tb[NFULA_PAYLOAD-1]));
		h->upmsg.data_len = NFA_PAYLOAD(tb[NFULA_PAYLOAD-1]);
	} else
		h->upmsg.data_len = 0;
	
	return &h->upmsg;
}

ssize_t ipulog_read(struct ipulog_handle *h, unsigned char *buf,
		    size_t len, int timeout)
{
	/* 'timeout' was never implemented in the original libipulog,
	 * so we don't bother emulating it */
	return nfnl_recv(nflog_nfnlh(h->nfulh), buf, len);
}

/* print a human readable description of the last error to stderr */
void ipulog_perror(const char *s)
{
	if (s)
		fputs(s, stderr);
	else
		fputs("ERROR", stderr);
	if (ipulog_errno)
		fprintf(stderr, ": %s", ipulog_strerror(ipulog_errno));
	if (errno)
		fprintf(stderr, ": %s", strerror(errno));
	fputc('\n', stderr);
}

