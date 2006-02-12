/* 
 * libipulog.c, 1.6
 *
 * netfilter ULOG userspace library.
 *
 * (C) 2000 by Harald Welte <laforge@gnumonks.org>
 * released under the terms of GNU GPL
 *
 * This library is still under development, so be aware of sudden interface
 * changes
 *
 * libipulog.c,v 1.6 2001/01/30 09:28:04 laforge Exp
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <libipulog/libipulog.h>

struct ipulog_handle
{
	int fd;
	u_int8_t blocking;
	struct sockaddr_nl local;
	struct sockaddr_nl peer;
	struct nlmsghdr* last_nlhdr;
};

/* internal */

enum 
{
	IPULOG_ERR_NONE = 0,
	IPULOG_ERR_IMPL,
	IPULOG_ERR_HANDLE,
	IPULOG_ERR_SOCKET,
	IPULOG_ERR_BIND,
	IPULOG_ERR_RECVBUF,
	IPULOG_ERR_RECV,
	IPULOG_ERR_NLEOF,
	IPULOG_ERR_TRUNC,
	IPULOG_ERR_INVGR,
	IPULOG_ERR_INVNL,
};

#define IPULOG_MAXERR IPULOG_ERR_INVNL

static int ipulog_errno = IPULOG_ERR_NONE;

struct ipulog_errmap_t 
{
	int errcode;
	char *message;
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

static ssize_t ipulog_netlink_recvfrom(const struct ipulog_handle *h,
			unsigned char *buf, size_t len)
{
	int addrlen, status;
	struct nlmsghdr *nlh;
	
	if (len < sizeof(struct nlmsgerr)) {
		ipulog_errno = IPULOG_ERR_RECVBUF;
		return -1; 
	}
	addrlen = sizeof(h->peer);
	status = recvfrom(h->fd, buf, len, 0, (struct sockaddr *)&h->peer,	
			&addrlen);
	if (status < 0) 
	{
		ipulog_errno = IPULOG_ERR_RECV;
		return status;
	}
	if (addrlen != sizeof (h->peer))
	{
		ipulog_errno = IPULOG_ERR_RECV;
		return -1;
	}	
	if (status == 0)
	{
		ipulog_errno = IPULOG_ERR_NLEOF;
		return -1;
	}
	nlh = (struct nlmsghdr *)buf;
	if (nlh->nlmsg_flags & MSG_TRUNC || status > len)
	{
		ipulog_errno = IPULOG_ERR_TRUNC;
		return -1;
	}
	return status;
}

static char *ipulog_strerror(int errcode)
{
	if (errcode < 0 || errcode > IPULOG_MAXERR)
		errcode = IPULOG_ERR_IMPL;
	return ipulog_errmap[errcode].message;
}


/* public */

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
struct ipulog_handle *ipulog_create_handle(unsigned int gmask)
{
	struct ipulog_handle *h;
	int status;

	h = (struct ipulog_handle *) malloc(sizeof(struct ipulog_handle));
	if (h == NULL)
	{
		ipulog_errno = IPULOG_ERR_HANDLE;
		return NULL;
	}
	memset(h, 0, sizeof(struct ipulog_handle));
	h->fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_NFLOG);
	if (h->fd == -1)
	{
		ipulog_errno = IPULOG_ERR_SOCKET;
		close(h->fd);
		free(h);
		return NULL;
	}
	memset(&h->local, 0, sizeof(struct sockaddr_nl));
	h->local.nl_family = AF_NETLINK;
	h->local.nl_pid = getpid();
	h->local.nl_groups = gmask;
	status = bind(h->fd, (struct sockaddr *)&h->local, sizeof(h->local));
	if (status == -1)
	{
		ipulog_errno = IPULOG_ERR_BIND;
		close(h->fd);
		free(h);
		return NULL;
	}
	memset(&h->peer, 0, sizeof(struct sockaddr_nl));
	h->peer.nl_family = AF_NETLINK;
	h->peer.nl_pid = 0;
	h->peer.nl_groups = gmask;

	return h;
} 

/* destroy a ipulog handle */
void ipulog_destroy_handle(struct ipulog_handle *h)
{
	close(h->fd);
	free(h);
}


/* do a BLOCKING read on an ipulog handle */
ssize_t ipulog_read(struct ipulog_handle *h, unsigned char *buf,
		    size_t len, int timeout)
{
	return ipulog_netlink_recvfrom(h, buf, len);
}

/* get a pointer to the actual start of the ipulog packet,
   use this to strip netlink header */
ulog_packet_msg_t *ipulog_get_packet(struct ipulog_handle *h,
				     const unsigned char *buf, 
				     size_t len)
{
	struct nlmsghdr *nlh;
	size_t remain_len;

	/* if last header in handle not inside this buffer,
	 * drop reference to last header */
	if ((unsigned char *)h->last_nlhdr > (buf + len) || 
	    (unsigned char *)h->last_nlhdr < buf) {
		h->last_nlhdr = NULL;
	}
	
	if (!h->last_nlhdr) {
		/* fist message in buffer */
		nlh = (struct nlmsghdr *) buf;
		if (!NLMSG_OK(nlh, len)) {
			/* ERROR */
			ipulog_errno = IPULOG_ERR_INVNL;
			return NULL;
		}
	} else {
		/* we are in n-th part of multilink message */
		if (h->last_nlhdr->nlmsg_type == NLMSG_DONE) {
			/* if last part in multilink message, return */
			h->last_nlhdr = NULL;
			return NULL;
		}

		/* calculate remaining lenght from lasthdr to end of buffer */
		remain_len = (len - 
				((unsigned char *)h->last_nlhdr - buf));
		nlh = NLMSG_NEXT(h->last_nlhdr, remain_len);
	}

	/* update last_nlhdr field */
	if (nlh->nlmsg_flags & NLM_F_MULTI) {
			h->last_nlhdr = nlh;
	}

	return NLMSG_DATA(nlh);
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

