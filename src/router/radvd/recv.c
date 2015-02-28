/*
 *
 *   Authors:
 *    Pedro Roque		<roque@di.fc.ul.pt>
 *    Lars Fenneberg		<lf@elemental.net>
 *
 *   This software is Copyright 1996,1997 by the above mentioned author(s),
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <reubenhwk@gmail.com>.
 *
 */

#include "config.h"
#include "includes.h"
#include "radvd.h"

int recv_rs_ra(int sock, unsigned char *msg, struct sockaddr_in6 *addr, struct in6_pktinfo **pkt_info, int *hoplimit, unsigned char * chdr)
{
	struct iovec iov;
	iov.iov_len = MSG_SIZE_RECV;
	iov.iov_base = (caddr_t) msg;

	struct msghdr mhdr;
	memset(&mhdr, 0, sizeof(mhdr));
	mhdr.msg_name = (caddr_t) addr;
	mhdr.msg_namelen = sizeof(*addr);
	mhdr.msg_iov = &iov;
	mhdr.msg_iovlen = 1;
	mhdr.msg_control = (void *)chdr;
	mhdr.msg_controllen = CMSG_SPACE(sizeof(struct in6_pktinfo)) + CMSG_SPACE(sizeof(int));

	int len = recvmsg(sock, &mhdr, 0);

	if (len < 0) {
		if (errno != EINTR)
			flog(LOG_ERR, "recvmsg: %s", strerror(errno));

		return len;
	}

	*hoplimit = 255;

	for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(&mhdr); cmsg != NULL; cmsg = CMSG_NXTHDR(&mhdr, cmsg)) {
		if (cmsg->cmsg_level != IPPROTO_IPV6)
			continue;

		switch (cmsg->cmsg_type) {
#ifdef IPV6_HOPLIMIT
		case IPV6_HOPLIMIT:
			if ((cmsg->cmsg_len == CMSG_LEN(sizeof(int))) && (*(int *)CMSG_DATA(cmsg) >= 0)
			    && (*(int *)CMSG_DATA(cmsg) < 256)) {
				*hoplimit = *(int *)CMSG_DATA(cmsg);
			} else {
				flog(LOG_ERR, "received a bogus IPV6_HOPLIMIT from the kernel! len=%d, data=%d",
				     (int)cmsg->cmsg_len, *(int *)CMSG_DATA(cmsg));
				return -1;
			}
			break;
#endif				/* IPV6_HOPLIMIT */
		case IPV6_PKTINFO:
			if ((cmsg->cmsg_len == CMSG_LEN(sizeof(struct in6_pktinfo)))
			    && ((struct in6_pktinfo *)CMSG_DATA(cmsg))->ipi6_ifindex) {
				*pkt_info = (struct in6_pktinfo *)CMSG_DATA(cmsg);
			} else {
				flog(LOG_ERR, "received a bogus IPV6_PKTINFO from the kernel! len=%d, index=%d",
				     (int)cmsg->cmsg_len, ((struct in6_pktinfo *)CMSG_DATA(cmsg))->ipi6_ifindex);
				return -1;
			}
			break;
		}
	}

	char if_namebuf[IF_NAMESIZE] = { "" };
	char *if_name = 0;
	if (pkt_info && *pkt_info) {
		if_name = if_indextoname((*pkt_info)->ipi6_ifindex, if_namebuf);
	}
	if (!if_name) {
		if_name = "unknown interface";
	}
	dlog(LOG_DEBUG, 5, "%s recvmsg len=%d", if_name, len);

	return len;
}
