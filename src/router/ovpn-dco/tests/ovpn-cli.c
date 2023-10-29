// SPDX-License-Identifier: GPL-2.0
/*  OpenVPN data channel accelerator
 *
 *  Copyright (C) 2020-2023 OpenVPN, Inc.
 *
 *  Author:	Antonio Quartulli <antonio@openvpn.net>
 */

#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>

#include <linux/ovpn_dco.h>
#include <linux/types.h>
#include <linux/netlink.h>

#include <netlink/socket.h>
#include <netlink/netlink.h>
#include <netlink/genl/genl.h>
#include <netlink/genl/family.h>
#include <netlink/genl/ctrl.h>

#include <mbedtls/base64.h>
#include <mbedtls/error.h>

/* libnl < 3.5.0 does not set the NLA_F_NESTED on its own, therefore we
 * have to explicitly do it to prevent the kernel from failing upon
 * parsing of the message
 */
#define nla_nest_start(_msg, _type) \
	nla_nest_start(_msg, (_type) | NLA_F_NESTED)

typedef int (*ovpn_nl_cb)(struct nl_msg *msg, void *arg);

enum ovpn_key_direction {
	KEY_DIR_IN = 0,
	KEY_DIR_OUT,
};

#define KEY_LEN (256 / 8)
#define NONCE_LEN 8

#define PEER_ID_UNDEF 0x00FFFFFF

struct nl_ctx {
	struct nl_sock *nl_sock;
	struct nl_msg *nl_msg;
	struct nl_cb *nl_cb;

	int ovpn_dco_id;
};

struct ovpn_ctx {
	__u8 key_enc[KEY_LEN];
	__u8 key_dec[KEY_LEN];
	__u8 nonce[NONCE_LEN];

	enum ovpn_cipher_alg cipher;

	sa_family_t sa_family;

	__u32 peer_id;
	__u16 lport;

	union {
		struct sockaddr_in in4;
		struct sockaddr_in6 in6;
	} remote;

	union {
		struct sockaddr_in in4;
		struct sockaddr_in6 in6;
	} peer_ip;

	unsigned int ifindex;

	int socket;

	__u32 keepalive_interval;
	__u32 keepalive_timeout;

	enum ovpn_key_direction key_dir;
};

static int ovpn_nl_recvmsgs(struct nl_ctx *ctx)
{
	int ret;

	ret = nl_recvmsgs(ctx->nl_sock, ctx->nl_cb);

	switch (ret) {
	case -NLE_INTR:
		fprintf(stderr,
			"netlink received interrupt due to signal - ignoring\n");
		break;
	case -NLE_NOMEM:
		fprintf(stderr, "netlink out of memory error\n");
		break;
	case -NLE_AGAIN:
		fprintf(stderr,
			"netlink reports blocking read - aborting wait\n");
		break;
	default:
		if (ret)
			fprintf(stderr, "netlink reports error (%d): %s\n",
				ret, nl_geterror(-ret));
		break;
	}

	return ret;
}

static struct nl_ctx *nl_ctx_alloc_flags(struct ovpn_ctx *ovpn,
					 enum ovpn_nl_commands cmd, int flags)
{
	struct nl_ctx *ctx;
	int ret;

	ctx = calloc(1, sizeof(*ctx));
	if (!ctx)
		return NULL;

	ctx->nl_sock = nl_socket_alloc();
	if (!ctx->nl_sock) {
		fprintf(stderr, "cannot allocate netlink socket\n");
		goto err_free;
	}

	nl_socket_set_buffer_size(ctx->nl_sock, 8192, 8192);

	ret = genl_connect(ctx->nl_sock);
	if (ret) {
		fprintf(stderr, "cannot connect to generic netlink: %s\n",
			nl_geterror(ret));
		goto err_sock;
	}

	ctx->ovpn_dco_id = genl_ctrl_resolve(ctx->nl_sock, OVPN_NL_NAME);
	if (ctx->ovpn_dco_id < 0) {
		fprintf(stderr, "cannot find ovpn_dco netlink component: %d\n",
			ctx->ovpn_dco_id);
		goto err_free;
	}

	ctx->nl_msg = nlmsg_alloc();
	if (!ctx->nl_msg) {
		fprintf(stderr, "cannot allocate netlink message\n");
		goto err_sock;
	}

	ctx->nl_cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!ctx->nl_cb) {
		fprintf(stderr, "failed to allocate netlink callback\n");
		goto err_msg;
	}

	nl_socket_set_cb(ctx->nl_sock, ctx->nl_cb);

	genlmsg_put(ctx->nl_msg, 0, 0, ctx->ovpn_dco_id, 0, flags, cmd, 0);
	NLA_PUT_U32(ctx->nl_msg, OVPN_ATTR_IFINDEX, ovpn->ifindex);

	return ctx;
nla_put_failure:
err_msg:
	nlmsg_free(ctx->nl_msg);
err_sock:
	nl_socket_free(ctx->nl_sock);
err_free:
	free(ctx);
	return NULL;
}

static struct nl_ctx *nl_ctx_alloc(struct ovpn_ctx *ovpn,
				   enum ovpn_nl_commands cmd)
{
	return nl_ctx_alloc_flags(ovpn, cmd, 0);
}

static void nl_ctx_free(struct nl_ctx *ctx)
{
	if (!ctx)
		return;

	nl_socket_free(ctx->nl_sock);
	nlmsg_free(ctx->nl_msg);
	nl_cb_put(ctx->nl_cb);
	free(ctx);
}

static int ovpn_nl_cb_error(struct sockaddr_nl (*nla)__attribute__((unused)),
			    struct nlmsgerr *err, void *arg)
{
	struct nlmsghdr *nlh = (struct nlmsghdr *)err - 1;
	struct nlattr *tb_msg[NLMSGERR_ATTR_MAX + 1];
	int len = nlh->nlmsg_len;
	struct nlattr *attrs;
	int *ret = arg;
	int ack_len = sizeof(*nlh) + sizeof(int) + sizeof(*nlh);

	*ret = err->error;

	if (!(nlh->nlmsg_flags & NLM_F_ACK_TLVS))
		return NL_STOP;

	if (!(nlh->nlmsg_flags & NLM_F_CAPPED))
		ack_len += err->msg.nlmsg_len - sizeof(*nlh);

	if (len <= ack_len)
		return NL_STOP;

	attrs = (void *)((unsigned char *)nlh + ack_len);
	len -= ack_len;

	nla_parse(tb_msg, NLMSGERR_ATTR_MAX, attrs, len, NULL);
	if (tb_msg[NLMSGERR_ATTR_MSG]) {
		len = strnlen((char *)nla_data(tb_msg[NLMSGERR_ATTR_MSG]),
			      nla_len(tb_msg[NLMSGERR_ATTR_MSG]));
		fprintf(stderr, "kernel error: %*s\n", len,
			(char *)nla_data(tb_msg[NLMSGERR_ATTR_MSG]));
	}

	return NL_STOP;
}

static int ovpn_nl_cb_finish(struct nl_msg (*msg)__attribute__((unused)),
			     void *arg)
{
	int *status = arg;

	*status = 0;
	return NL_SKIP;
}

static int ovpn_nl_msg_send(struct nl_ctx *ctx, ovpn_nl_cb cb)
{
	int status = 1;

	nl_cb_err(ctx->nl_cb, NL_CB_CUSTOM, ovpn_nl_cb_error, &status);
	nl_cb_set(ctx->nl_cb, NL_CB_FINISH, NL_CB_CUSTOM, ovpn_nl_cb_finish,
		  &status);
	nl_cb_set(ctx->nl_cb, NL_CB_ACK, NL_CB_CUSTOM, ovpn_nl_cb_finish,
		  &status);

	if (cb)
		nl_cb_set(ctx->nl_cb, NL_CB_VALID, NL_CB_CUSTOM, cb, ctx);

	nl_send_auto_complete(ctx->nl_sock, ctx->nl_msg);

	while (status == 1)
		ovpn_nl_recvmsgs(ctx);

	if (status < 0)
		fprintf(stderr, "failed to send netlink message: %s (%d)\n",
			strerror(-status), status);

	return status;
}

static int ovpn_read_key(const char *file, struct ovpn_ctx *ctx)
{
	int idx_enc, idx_dec, ret = -1;
	unsigned char *ckey = NULL;
	__u8 *bkey = NULL;
	size_t olen = 0;
	long ckey_len;
	FILE *fp;

	fp = fopen(file, "r");
	if (!fp) {
		fprintf(stderr, "cannot open: %s\n", file);
		return -1;
	}

	/* get file size */
	fseek(fp, 0L, SEEK_END);
	ckey_len = ftell(fp);
	rewind(fp);

	/* if the file is longer, let's just read a portion */
	if (ckey_len > 256)
		ckey_len = 256;

	ckey = malloc(ckey_len);
	if (!ckey)
		goto err;

	ret = fread(ckey, 1, ckey_len, fp);
	if (ret != ckey_len) {
		fprintf(stderr,
			"couldn't read enough data from key file: %dbytes read\n",
			ret);
		goto err;
	}

	olen = 0;
	ret = mbedtls_base64_decode(NULL, 0, &olen, ckey, ckey_len);
	if (ret != MBEDTLS_ERR_BASE64_BUFFER_TOO_SMALL) {
		char buf[256];

		mbedtls_strerror(ret, buf, sizeof(buf));
		fprintf(stderr, "unexpected base64 error1: %s (%d)\n", buf,
			ret);

		goto err;
	}

	bkey = malloc(olen);
	if (!bkey) {
		fprintf(stderr, "cannot allocate binary key buffer\n");
		goto err;
	}

	ret = mbedtls_base64_decode(bkey, olen, &olen, ckey, ckey_len);
	if (ret) {
		char buf[256];

		mbedtls_strerror(ret, buf, sizeof(buf));
		fprintf(stderr, "unexpected base64 error2: %s (%d)\n", buf,
			ret);

		goto err;
	}

	if (olen < 2 * KEY_LEN + NONCE_LEN) {
		fprintf(stderr,
			"not enough data in key file, found %zdB but needs %dB\n",
			olen, 2 * KEY_LEN + NONCE_LEN);
		goto err;
	}

	switch (ctx->key_dir) {
	case KEY_DIR_IN:
		idx_enc = 0;
		idx_dec = 1;
		break;
	case KEY_DIR_OUT:
		idx_enc = 1;
		idx_dec = 0;
		break;
	}

	memcpy(ctx->key_enc, bkey + KEY_LEN * idx_enc, KEY_LEN);
	memcpy(ctx->key_dec, bkey + KEY_LEN * idx_dec, KEY_LEN);
	memcpy(ctx->nonce, bkey + 2 * KEY_LEN, NONCE_LEN);

	ret = 0;

err:
	fclose(fp);
	free(bkey);
	free(ckey);

	return ret;
}

static int ovpn_read_cipher(const char *cipher, struct ovpn_ctx *ctx)
{
	if (strcmp(cipher, "aes") == 0)
		ctx->cipher = OVPN_CIPHER_ALG_AES_GCM;
	else if (strcmp(cipher, "chachapoly") == 0)
		ctx->cipher = OVPN_CIPHER_ALG_CHACHA20_POLY1305;
	else if (strcmp(cipher, "none") == 0)
		ctx->cipher = OVPN_CIPHER_ALG_NONE;
	else
		return -ENOTSUP;

	return 0;
}

static int ovpn_read_key_direction(const char *dir, struct ovpn_ctx *ctx)
{
	int in_dir;

	in_dir = strtoll(dir, NULL, 10);
	switch (in_dir) {
	case KEY_DIR_IN:
	case KEY_DIR_OUT:
		ctx->key_dir = in_dir;
		break;
	default:
		fprintf(stderr,
			"invalid key direction provided. Can be 0 or 1 only\n");
		return -1;
	}

	return 0;
}

static int ovpn_socket(struct ovpn_ctx *ctx, sa_family_t family, int proto)
{
	struct sockaddr_storage local_sock;
	struct sockaddr_in6 *in6;
	struct sockaddr_in *in;
	int ret, s, sock_type;
	size_t sock_len;

	if (proto == IPPROTO_UDP)
		sock_type = SOCK_DGRAM;
	else if (proto == IPPROTO_TCP)
		sock_type = SOCK_STREAM;
	else
		return -EINVAL;

	s = socket(family, sock_type, 0);
	if (s < 0) {
		perror("cannot create socket");
		return -1;
	}

	memset((char *)&local_sock, 0, sizeof(local_sock));

	switch (family) {
	case AF_INET:
		in = (struct sockaddr_in *)&local_sock;
		in->sin_family = family;
		in->sin_port = htons(ctx->lport);
		in->sin_addr.s_addr = htonl(INADDR_ANY);
		sock_len = sizeof(*in);
		break;
	case AF_INET6:
		in6 = (struct sockaddr_in6 *)&local_sock;
		in6->sin6_family = family;
		in6->sin6_port = htons(ctx->lport);
		in6->sin6_addr = in6addr_any;
		sock_len = sizeof(*in6);
		break;
	default:
		return -1;
	}

	int opt = 1;
	ret = setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (ret < 0) {
		perror("setsockopt for SO_REUSEADDR");
		return ret;
	}

	ret = setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt));
	if (ret < 0) {
		perror("setsockopt for SO_REUSEPORT");
		return ret;
	}

	if (family == AF_INET6) {
		opt = 0;
		if (setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, &opt, sizeof(opt))) {
			perror("failed to set IPV6_V6ONLY");
			return -1;
		}
	}

	ret = bind(s, (struct sockaddr *)&local_sock, sock_len);
	if (ret < 0) {
		perror("cannot bind socket");
		goto err_socket;
	}

	ctx->socket = s;
	ctx->sa_family = family;
	return 0;

err_socket:
	close(s);
	return -1;
}

static int ovpn_udp_socket(struct ovpn_ctx *ctx, sa_family_t family)
{
	return ovpn_socket(ctx, family, IPPROTO_UDP);
}

static int ovpn_listen(struct ovpn_ctx *ctx, sa_family_t family)
{
	int ret;

	ret = ovpn_socket(ctx, family, IPPROTO_TCP);
	if (ret < 0)
		return ret;

	ret = listen(ctx->socket, 10);
	if (ret < 0) {
		perror("listen");
		close(ctx->socket);
		return -1;
	}

	return 0;
}

static int ovpn_accept(struct ovpn_ctx *ctx)
{
	socklen_t socklen;
	int ret;

	socklen = sizeof(ctx->remote);
	ret = accept(ctx->socket, (struct sockaddr *)&ctx->remote, &socklen);
	if (ret < 0) {
		perror("accept");
		goto err;
	}

	fprintf(stderr, "Connection received!\n");

	switch (socklen) {
	case sizeof(struct sockaddr_in):
	case sizeof(struct sockaddr_in6):
		break;
	default:
		fprintf(stderr, "error: expecting IPv4 or IPv6 connection\n");
		close(ret);
		ret = -EINVAL;
		goto err;
	}

	return ret;
err:
	close(ctx->socket);
	return ret;
}

static int ovpn_connect(struct ovpn_ctx *ovpn)
{
	socklen_t socklen;
	int s, ret;

	s = socket(ovpn->remote.in4.sin_family, SOCK_STREAM, 0);
	if (s < 0) {
		perror("cannot create socket");
		return -1;
	}

	switch (ovpn->remote.in4.sin_family) {
	case AF_INET:
		socklen = sizeof(struct sockaddr_in);
		break;
	case AF_INET6:
		socklen = sizeof(struct sockaddr_in6);
		break;
	default:
		return -EOPNOTSUPP;
	}

	ret = connect(s, (struct sockaddr *)&ovpn->remote, socklen);
	if (ret < 0) {
		perror("connect");
		goto err;
	}

	fprintf(stderr, "connected\n");

	ovpn->socket = s;

	return 0;
err:
	close(s);
	return ret;
}

static int ovpn_new_peer(struct ovpn_ctx *ovpn, bool is_tcp)
{
	struct nlattr *attr;
	struct nl_ctx *ctx;
	size_t alen;
	int ret = -1;

	ctx = nl_ctx_alloc(ovpn, OVPN_CMD_NEW_PEER);
	if (!ctx)
		return -ENOMEM;

	attr = nla_nest_start(ctx->nl_msg, OVPN_ATTR_NEW_PEER);
	NLA_PUT_U32(ctx->nl_msg, OVPN_NEW_PEER_ATTR_PEER_ID, ovpn->peer_id);
	NLA_PUT_U32(ctx->nl_msg, OVPN_NEW_PEER_ATTR_SOCKET, ovpn->socket);

	if (!is_tcp) {
		switch (ovpn->remote.in4.sin_family) {
		case AF_INET:
			alen = sizeof(struct sockaddr_in);
			break;
		case AF_INET6:
			alen = sizeof(struct sockaddr_in6);
			break;
		default:
			fprintf(stderr, "Invalid family for remote socket address\n");
			goto nla_put_failure;
		}
		NLA_PUT(ctx->nl_msg, OVPN_NEW_PEER_ATTR_SOCKADDR_REMOTE, alen, &ovpn->remote);
	}


	switch (ovpn->peer_ip.in4.sin_family) {
	case AF_INET:
		NLA_PUT_U32(ctx->nl_msg, OVPN_NEW_PEER_ATTR_IPV4,
			    ovpn->peer_ip.in4.sin_addr.s_addr);
		break;
	case AF_INET6:
		NLA_PUT(ctx->nl_msg, OVPN_NEW_PEER_ATTR_IPV6, sizeof(struct in6_addr),
			&ovpn->peer_ip.in6.sin6_addr);
		break;
	default:
		fprintf(stderr, "Invalid family for peer address\n");
		goto nla_put_failure;
	}

	nla_nest_end(ctx->nl_msg, attr);

	ret = ovpn_nl_msg_send(ctx, NULL);
nla_put_failure:
	nl_ctx_free(ctx);
	return ret;
}

static int ovpn_set_peer(struct ovpn_ctx *ovpn)
{
	struct nlattr *attr;
	struct nl_ctx *ctx;
	int ret = -1;

	ctx = nl_ctx_alloc(ovpn, OVPN_CMD_SET_PEER);
	if (!ctx)
		return -ENOMEM;

	attr = nla_nest_start(ctx->nl_msg, OVPN_ATTR_SET_PEER);
	NLA_PUT_U32(ctx->nl_msg, OVPN_SET_PEER_ATTR_PEER_ID, ovpn->peer_id);
	NLA_PUT_U32(ctx->nl_msg, OVPN_SET_PEER_ATTR_KEEPALIVE_INTERVAL,
		    ovpn->keepalive_interval);
	NLA_PUT_U32(ctx->nl_msg, OVPN_SET_PEER_ATTR_KEEPALIVE_TIMEOUT,
		    ovpn->keepalive_timeout);
	nla_nest_end(ctx->nl_msg, attr);

	ret = ovpn_nl_msg_send(ctx, NULL);
nla_put_failure:
	nl_ctx_free(ctx);
	return ret;
}

static int ovpn_del_peer(struct ovpn_ctx *ovpn)
{
	struct nlattr *attr;
	struct nl_ctx *ctx;
	int ret = -1;

	ctx = nl_ctx_alloc(ovpn, OVPN_CMD_DEL_PEER);
	if (!ctx)
		return -ENOMEM;

	attr = nla_nest_start(ctx->nl_msg, OVPN_ATTR_DEL_PEER);
	NLA_PUT_U32(ctx->nl_msg, OVPN_DEL_PEER_ATTR_PEER_ID, ovpn->peer_id);
	nla_nest_end(ctx->nl_msg, attr);

	ret = ovpn_nl_msg_send(ctx, NULL);
nla_put_failure:
	nl_ctx_free(ctx);
	return ret;
}

static int ovpn_handle_peer(struct nl_msg *msg, void *arg)
{
	struct nlattr *attrs_peer[OVPN_GET_PEER_RESP_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *attrs[OVPN_ATTR_MAX + 1];
	__u16 port = 0;

	nla_parse(attrs, OVPN_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!attrs[OVPN_ATTR_GET_PEER]) {
		fprintf(stderr, "no packet content in netlink message\n");
		return NL_SKIP;
	}

	nla_parse(attrs_peer, OVPN_GET_PEER_RESP_ATTR_MAX, nla_data(attrs[OVPN_ATTR_GET_PEER]),
		  nla_len(attrs[OVPN_ATTR_GET_PEER]), NULL);

	if (attrs_peer[OVPN_GET_PEER_RESP_ATTR_PEER_ID])
		fprintf(stderr, "* Peer %u\n",
			nla_get_u32(attrs_peer[OVPN_GET_PEER_RESP_ATTR_PEER_ID]));

	if (attrs_peer[OVPN_GET_PEER_RESP_ATTR_IPV4]) {
		char buf[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, nla_data(attrs_peer[OVPN_GET_PEER_RESP_ATTR_IPV4]), buf,
			  sizeof(buf));
		fprintf(stderr, "\tVPN IPv4: %s\n", buf);
	}

	if (attrs_peer[OVPN_GET_PEER_RESP_ATTR_IPV6]) {
		char buf[INET6_ADDRSTRLEN];
		inet_ntop(AF_INET6, nla_data(attrs_peer[OVPN_GET_PEER_RESP_ATTR_IPV6]), buf,
			  sizeof(buf));
		fprintf(stderr, "\tVPN IPv6: %s\n", buf);
	}

	if (attrs_peer[OVPN_GET_PEER_RESP_ATTR_LOCAL_PORT])
		port = ntohs(nla_get_u16(attrs_peer[OVPN_GET_PEER_RESP_ATTR_LOCAL_PORT]));

	if (attrs_peer[OVPN_GET_PEER_RESP_ATTR_SOCKADDR_REMOTE]) {
		struct sockaddr_storage ss;
		struct sockaddr_in6 *in6 = (struct sockaddr_in6 *)&ss;
		struct sockaddr_in *in = (struct sockaddr_in *)&ss;

		memcpy(&ss, nla_data(attrs_peer[OVPN_GET_PEER_RESP_ATTR_SOCKADDR_REMOTE]),
		       nla_len(attrs_peer[OVPN_GET_PEER_RESP_ATTR_SOCKADDR_REMOTE]));

		if (in->sin_family == AF_INET) {
			char buf[INET_ADDRSTRLEN];

			if (attrs_peer[OVPN_GET_PEER_RESP_ATTR_LOCAL_IP]) {
				inet_ntop(AF_INET,
					  nla_data(attrs_peer[OVPN_GET_PEER_RESP_ATTR_LOCAL_IP]),
					  buf, sizeof(buf));
				fprintf(stderr, "\tLocal: %s:%hu\n", buf, port);
			}

			inet_ntop(AF_INET, &in->sin_addr, buf, sizeof(buf));
			fprintf(stderr, "\tRemote: %s:%u\n", buf, ntohs(in->sin_port));
		} else if (in->sin_family == AF_INET6) {
			char buf[INET6_ADDRSTRLEN];

			if (attrs_peer[OVPN_GET_PEER_RESP_ATTR_LOCAL_IP]) {
				inet_ntop(AF_INET6,
					  nla_data(attrs_peer[OVPN_GET_PEER_RESP_ATTR_LOCAL_IP]),
					  buf, sizeof(buf));
				fprintf(stderr, "\tLocal: %s\n", buf);
			}

			inet_ntop(AF_INET6, &in6->sin6_addr, buf, sizeof(buf));
			fprintf(stderr, "\tRemote: %s:%u (scope-id: %u)\n", buf,
				ntohs(in6->sin6_port), ntohl(in6->sin6_scope_id));
		}
	}

	if (attrs_peer[OVPN_GET_PEER_RESP_ATTR_KEEPALIVE_INTERVAL])
		fprintf(stderr, "\tKeepalive interval: %u sec\n",
			nla_get_u32(attrs_peer[OVPN_GET_PEER_RESP_ATTR_KEEPALIVE_INTERVAL]));

	if (attrs_peer[OVPN_GET_PEER_RESP_ATTR_KEEPALIVE_TIMEOUT])
		fprintf(stderr, "\tKeepalive timeout: %u sec\n",
			nla_get_u32(attrs_peer[OVPN_GET_PEER_RESP_ATTR_KEEPALIVE_TIMEOUT]));

	if (attrs_peer[OVPN_GET_PEER_RESP_ATTR_RX_BYTES])
		fprintf(stderr, "\tRX bytes: %" PRIu64 "\n",
			nla_get_u64(attrs_peer[OVPN_GET_PEER_RESP_ATTR_RX_BYTES]));

	if (attrs_peer[OVPN_GET_PEER_RESP_ATTR_TX_BYTES])
		fprintf(stderr, "\tTX bytes: %" PRIu64 "\n",
			nla_get_u64(attrs_peer[OVPN_GET_PEER_RESP_ATTR_TX_BYTES]));

	if (attrs_peer[OVPN_GET_PEER_RESP_ATTR_RX_PACKETS])
		fprintf(stderr, "\tRX packets: %u\n",
			nla_get_u32(attrs_peer[OVPN_GET_PEER_RESP_ATTR_RX_PACKETS]));

	if (attrs_peer[OVPN_GET_PEER_RESP_ATTR_TX_PACKETS])
		fprintf(stderr, "\tTX packets: %u\n",
			nla_get_u32(attrs_peer[OVPN_GET_PEER_RESP_ATTR_TX_PACKETS]));

	return NL_SKIP;
}

static int ovpn_get_peer(struct ovpn_ctx *ovpn)
{
	int flags = 0, ret = -1;
	struct nlattr *attr;
	struct nl_ctx *ctx;

	if (ovpn->peer_id == PEER_ID_UNDEF)
		flags = NLM_F_DUMP;

	ctx = nl_ctx_alloc_flags(ovpn, OVPN_CMD_GET_PEER, flags);
	if (!ctx)
		return -ENOMEM;

	if (ovpn->peer_id >= 0) {
		attr = nla_nest_start(ctx->nl_msg, OVPN_ATTR_GET_PEER);
		NLA_PUT_U32(ctx->nl_msg, OVPN_GET_PEER_ATTR_PEER_ID, ovpn->peer_id);
		nla_nest_end(ctx->nl_msg, attr);
	}

	ret = ovpn_nl_msg_send(ctx, ovpn_handle_peer);
nla_put_failure:
	nl_ctx_free(ctx);
	return ret;
}

static int ovpn_new_key(struct ovpn_ctx *ovpn)
{
	struct nlattr *attr, *key_dir;
	struct nl_ctx *ctx;
	int ret = -1;

	ctx = nl_ctx_alloc(ovpn, OVPN_CMD_NEW_KEY);
	if (!ctx)
		return -ENOMEM;

	attr = nla_nest_start(ctx->nl_msg, OVPN_ATTR_NEW_KEY);
	NLA_PUT_U32(ctx->nl_msg, OVPN_NEW_KEY_ATTR_PEER_ID, ovpn->peer_id);
	NLA_PUT_U8(ctx->nl_msg, OVPN_NEW_KEY_ATTR_KEY_SLOT, OVPN_KEY_SLOT_PRIMARY);
	NLA_PUT_U8(ctx->nl_msg, OVPN_NEW_KEY_ATTR_KEY_ID, 0);

	NLA_PUT_U16(ctx->nl_msg, OVPN_NEW_KEY_ATTR_CIPHER_ALG, ovpn->cipher);

	key_dir = nla_nest_start(ctx->nl_msg, OVPN_NEW_KEY_ATTR_ENCRYPT_KEY);
	NLA_PUT(ctx->nl_msg, OVPN_KEY_DIR_ATTR_CIPHER_KEY, KEY_LEN,
		ovpn->key_enc);
	NLA_PUT(ctx->nl_msg, OVPN_KEY_DIR_ATTR_NONCE_TAIL, NONCE_LEN,
		ovpn->nonce);
	nla_nest_end(ctx->nl_msg, key_dir);

	key_dir = nla_nest_start(ctx->nl_msg, OVPN_NEW_KEY_ATTR_DECRYPT_KEY);
	NLA_PUT(ctx->nl_msg, OVPN_KEY_DIR_ATTR_CIPHER_KEY, KEY_LEN,
		ovpn->key_dec);
	NLA_PUT(ctx->nl_msg, OVPN_KEY_DIR_ATTR_NONCE_TAIL, NONCE_LEN,
		ovpn->nonce);
	nla_nest_end(ctx->nl_msg, key_dir);
	nla_nest_end(ctx->nl_msg, attr);

	ret = ovpn_nl_msg_send(ctx, NULL);
nla_put_failure:
	nl_ctx_free(ctx);
	return ret;
}

static int ovpn_del_key(struct ovpn_ctx *ovpn)
{
	struct nlattr *attr;
	struct nl_ctx *ctx;
	int ret = -1;

	ctx = nl_ctx_alloc(ovpn, OVPN_CMD_DEL_KEY);
	if (!ctx)
		return -ENOMEM;

	attr = nla_nest_start(ctx->nl_msg, OVPN_ATTR_DEL_KEY);
	NLA_PUT_U32(ctx->nl_msg, OVPN_DEL_KEY_ATTR_PEER_ID, ovpn->peer_id);
	NLA_PUT_U8(ctx->nl_msg, OVPN_DEL_KEY_ATTR_KEY_SLOT, OVPN_KEY_SLOT_PRIMARY);
	nla_nest_end(ctx->nl_msg, attr);

	ret = ovpn_nl_msg_send(ctx, NULL);
nla_put_failure:
	nl_ctx_free(ctx);
	return ret;
}

static int ovpn_swap_keys(struct ovpn_ctx *ovpn)
{
	struct nlattr *attr;
	struct nl_ctx *ctx;
	int ret = -1;

	ctx = nl_ctx_alloc(ovpn, OVPN_CMD_SWAP_KEYS);
	if (!ctx)
		return -ENOMEM;

	attr = nla_nest_start(ctx->nl_msg, OVPN_ATTR_SWAP_KEYS);
	NLA_PUT_U32(ctx->nl_msg, OVPN_SWAP_KEYS_ATTR_PEER_ID, ovpn->peer_id);
	nla_nest_end(ctx->nl_msg, attr);

	ret = ovpn_nl_msg_send(ctx, NULL);
nla_put_failure:
	nl_ctx_free(ctx);
	return ret;
}

static int ovpn_send_data(struct ovpn_ctx *ovpn, const void *data, size_t len)
{
	struct nlattr *attr;
	struct nl_ctx *ctx;
	int ret = -1;

	ctx = nl_ctx_alloc(ovpn, OVPN_CMD_PACKET);
	if (!ctx)
		return -ENOMEM;

	attr = nla_nest_start(ctx->nl_msg, OVPN_ATTR_PACKET);
	NLA_PUT(ctx->nl_msg, OVPN_PACKET_ATTR_PACKET, len, data);
	nla_nest_end(ctx->nl_msg, attr);

	ret = ovpn_nl_msg_send(ctx, NULL);
nla_put_failure:
	nl_ctx_free(ctx);
	return ret;
}

/*static int ovpn_handle_packet(struct nl_msg *msg, void *arg)
{
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *attrs[OVPN_PACKET_ATTR_MAX + 1];
	const __u8 *data;
	size_t i, len;

	fprintf(stderr, "received message\n");

	nla_parse(attrs, OVPN_PACKET_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!attrs[OVPN_PACKET_ATTR_PACKET]) {
		fprintf(stderr, "no packet content in netlink message\n");
		return NL_SKIP;
	}

	len = nla_len(attrs[OVPN_PACKET_ATTR_PACKET]);
	data = nla_data(attrs[OVPN_PACKET_ATTR_PACKET]);

	fprintf(stderr, "received message, len=%zd:\n", len);
	for (i = 0; i < len; i++) {
		if (i && !(i % 16))
			fprintf(stderr, "\n");
		fprintf(stderr, "%.2x ", data[i]);
	}
	fprintf(stderr, "\n");

	return NL_SKIP;
}*/

static int nl_seq_check(struct nl_msg *msg, void *arg)
{
	return NL_OK;
}

/*static struct nl_ctx *ovpn_register(struct ovpn_ctx *ovpn)
{
	struct nl_ctx *ctx;
	int ret;

	ctx = nl_ctx_alloc(ovpn, OVPN_CMD_REGISTER_PACKET);
	if (!ctx)
		return NULL;

	nl_cb_set(ctx->nl_cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, nl_seq_check,
		  NULL);

	ret = ovpn_nl_msg_send(ctx, ovpn_handle_packet);
	if (ret < 0) {
		nl_ctx_free(ctx);
		return NULL;
	}

	return ctx;
}*/

struct mcast_handler_args {
	const char *group;
	int id;
};

static int mcast_family_handler(struct nl_msg *msg, void *arg)
{
	struct mcast_handler_args *grp = arg;
	struct nlattr *tb[CTRL_ATTR_MAX + 1];
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *mcgrp;
	int rem_mcgrp;

	nla_parse(tb, CTRL_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		  genlmsg_attrlen(gnlh, 0), NULL);

	if (!tb[CTRL_ATTR_MCAST_GROUPS])
		return NL_SKIP;

	nla_for_each_nested(mcgrp, tb[CTRL_ATTR_MCAST_GROUPS], rem_mcgrp) {
		struct nlattr *tb_mcgrp[CTRL_ATTR_MCAST_GRP_MAX + 1];

		nla_parse(tb_mcgrp, CTRL_ATTR_MCAST_GRP_MAX,
			  nla_data(mcgrp), nla_len(mcgrp), NULL);

		if (!tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME] ||
		    !tb_mcgrp[CTRL_ATTR_MCAST_GRP_ID])
			continue;
		if (strncmp(nla_data(tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME]),
			    grp->group, nla_len(tb_mcgrp[CTRL_ATTR_MCAST_GRP_NAME])))
			continue;
		grp->id = nla_get_u32(tb_mcgrp[CTRL_ATTR_MCAST_GRP_ID]);
		break;
	}

	return NL_SKIP;
}

static int mcast_error_handler(struct sockaddr_nl *nla, struct nlmsgerr *err,
			       void *arg)
{
	int *ret = arg;

	*ret = err->error;
	return NL_STOP;
}

static int mcast_ack_handler(struct nl_msg *msg, void *arg)
{
	int *ret = arg;

	*ret = 0;
	return NL_STOP;
}

static int ovpn_handle_msg(struct nl_msg *msg, void *arg)
{
	struct genlmsghdr *gnlh = nlmsg_data(nlmsg_hdr(msg));
	struct nlattr *attrs[OVPN_ATTR_MAX + 1];
	struct nlmsghdr *nlh = nlmsg_hdr(msg);
	//enum ovpn_del_peer_reason reason;
	char ifname[IF_NAMESIZE];
	__u32 ifindex;

	fprintf(stderr, "received message from ovpn-dco\n");

	if (!genlmsg_valid_hdr(nlh, 0)) {
		fprintf(stderr, "invalid header\n");
		return NL_STOP;
	}

	if (nla_parse(attrs, OVPN_ATTR_MAX, genlmsg_attrdata(gnlh, 0),
		      genlmsg_attrlen(gnlh, 0), NULL)) {
		fprintf(stderr, "received bogus data from ovpn-dco\n");
		return NL_STOP;
	}

	if (!attrs[OVPN_ATTR_IFINDEX]) {
		fprintf(stderr, "no ifindex in this message\n");
		return NL_STOP;
	}

	ifindex = nla_get_u32(attrs[OVPN_ATTR_IFINDEX]);
	if (!if_indextoname(ifindex, ifname)) {
		fprintf(stderr, "cannot resolve ifname for ifinxed: %u\n",
			ifindex);
		return NL_STOP;
	}

	switch (gnlh->cmd) {
	case OVPN_CMD_DEL_PEER:
		/*if (!attrs[OVPN_ATTR_DEL_PEER_REASON]) {
			fprintf(stderr, "no reason in DEL_PEER message\n");
			return NL_STOP;
		}
		reason = nla_get_u8(attrs[OVPN_ATTR_DEL_PEER_REASON]);
		fprintf(stderr,
			"received CMD_DEL_PEER, ifname: %s reason: %d\n",
			ifname, reason);
		*/
		fprintf(stdout, "received CMD_DEL_PEER\n");
		break;
	default:
		fprintf(stderr, "received unknown command: %d\n", gnlh->cmd);
		return NL_STOP;
	}

	return NL_OK;
}

static int ovpn_get_mcast_id(struct nl_sock *sock, const char *family,
			     const char *group)
{
	struct nl_msg *msg;
	struct nl_cb *cb;
	int ret, ctrlid;
	struct mcast_handler_args grp = {
		.group = group,
		.id = -ENOENT,
	};

	msg = nlmsg_alloc();
	if (!msg)
		return -ENOMEM;

	cb = nl_cb_alloc(NL_CB_DEFAULT);
	if (!cb) {
		ret = -ENOMEM;
		goto out_fail_cb;
	}

	ctrlid = genl_ctrl_resolve(sock, "nlctrl");

	genlmsg_put(msg, 0, 0, ctrlid, 0, 0, CTRL_CMD_GETFAMILY, 0);

	ret = -ENOBUFS;
	NLA_PUT_STRING(msg, CTRL_ATTR_FAMILY_NAME, family);

	ret = nl_send_auto_complete(sock, msg);
	if (ret < 0)
		goto nla_put_failure;

	ret = 1;

	nl_cb_err(cb, NL_CB_CUSTOM, mcast_error_handler, &ret);
	nl_cb_set(cb, NL_CB_ACK, NL_CB_CUSTOM, mcast_ack_handler, &ret);
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, mcast_family_handler, &grp);

	while (ret > 0)
		nl_recvmsgs(sock, cb);

	if (ret == 0)
		ret = grp.id;
 nla_put_failure:
	nl_cb_put(cb);
 out_fail_cb:
	nlmsg_free(msg);
	return ret;
}

static void ovpn_listen_mcast(void)
{
	struct nl_sock *sock;
	struct nl_cb *cb;
	int mcid, ret;

	sock = nl_socket_alloc();
	if (!sock) {
		fprintf(stderr, "cannot allocate netlink socket\n");
		goto err_free;
	}

	nl_socket_set_buffer_size(sock, 8192, 8192);

	ret = genl_connect(sock);
	if (ret < 0) {
		fprintf(stderr, "cannot connect to generic netlink: %s\n",
			nl_geterror(ret));
		goto err_free;
	}

	mcid = ovpn_get_mcast_id(sock, OVPN_NL_NAME,
				 OVPN_NL_MULTICAST_GROUP_PEERS);
	if (mcid < 0) {
		fprintf(stderr, "cannot get mcast group: %s\n",
			nl_geterror(mcid));
		goto err_free;
	}

	ret = nl_socket_add_membership(sock, mcid);
	if (ret) {
		fprintf(stderr, "failed to join mcast group: %d\n", ret);
		goto err_free;
	}

	ret = 0;
	cb = nl_cb_alloc(NL_CB_DEFAULT);
	nl_cb_set(cb, NL_CB_SEQ_CHECK, NL_CB_CUSTOM, nl_seq_check, NULL);
	nl_cb_set(cb, NL_CB_VALID, NL_CB_CUSTOM, ovpn_handle_msg, &ret);
	nl_cb_err(cb, NL_CB_CUSTOM, ovpn_nl_cb_error, &ret);

	while (ret != -EINTR)
		ret = nl_recvmsgs(sock, cb);

	nl_cb_put(cb);
err_free:
	nl_socket_free(sock);
}

static void usage(const char *cmd)
{
	fprintf(stderr, "Error: invalid arguments.\n\n");
	fprintf(stderr,
		"Usage %s <iface> <connect|listen|new_peer|new_multi_peer|set_peer|del_peer|new_key|del_key|recv|send|listen_mcast> [arguments..]\n",
		cmd);
	fprintf(stderr, "\tiface: tun interface name\n\n");

	fprintf(stderr, "* connect <peer_id> <raddr> <rport> <vpnaddr>: start connecting peer of TCP-based VPN session\n");
	fprintf(stderr, "\tpeer-id: peer ID of the connecting peer\n");
	fprintf(stderr, "\tremote-addr: peer IP address\n");
	fprintf(stderr, "\tremote-port: peer TCP port\n");
	fprintf(stderr, "\tvpn-ip: peer VPN IP\n\n");

	fprintf(stderr, "* listen <lport> <peers_file>: listen for incoming peer TCP connections\n");
	fprintf(stderr, "\tlport: src TCP port\n");
	fprintf(stderr, "\tpeers_file: file containing one peer per line: Line format:\n");
	fprintf(stderr, "\t\t<peer_id> <vpnaddr>\n\n");

	fprintf(stderr, "* new_peer <lport> <peer-id> <raddr> <rport> <vpnaddr>: add new peer\n");
	fprintf(stderr, "\tpeer-id: peer ID to be used in data packets to/from this peer\n");
	fprintf(stderr, "\tlocal-port: local UDP port\n");
	fprintf(stderr, "\tremote-addr: peer IP address\n");
	fprintf(stderr, "\tremote-port: peer UDP port\n");
	fprintf(stderr, "\tvpnaddr: peer VPN IP\n\n");

	fprintf(stderr, "* new_multi_peer <lport> <file>: add multiple peers as listed in the file\n");
	fprintf(stderr, "\tlport: local UDP port to bind to\n");
	fprintf(stderr, "\tfile: text file containing one peer per line. Line format:\n");
	fprintf(stderr, "\t\t<peer-id> <raddr> <rport> <vpnaddr>\n\n");

	fprintf(stderr,
		"* set_peer <peer-id> <keepalive_interval> <keepalive_timeout>: set peer attributes\n");
	fprintf(stderr, "\tpeer-id: peer ID of the peer to modify\n");
	fprintf(stderr,
		"\tkeepalive_interval: interval for sending ping messages\n");
	fprintf(stderr,
		"\tkeepalive_timeout: time after which a peer is timed out\n\n");

	fprintf(stderr, "* del_peer <peer-id>: delete peer\n");
	fprintf(stderr, "\tpeer-id: peer ID of the peer to delete\n\n");

	fprintf(stderr,
		"* new_key <peer-id> <cipher> <key_dir> <key_file>: set data channel key\n");
	fprintf(stderr, "\tpeer-id: peer ID of the peer to configure the key for\n");
	fprintf(stderr,
		"\tcipher: cipher to use, supported: aes (AES-GCM), chachapoly (CHACHA20POLY1305), none\n");
	fprintf(stderr,
		"\tkey_dir: key direction, must 0 on one host and 1 on the other\n");
	fprintf(stderr, "\tkey_file: file containing the pre-shared key\n\n");

	fprintf(stderr, "* del_key <peer-id>: erase existing data channel key\n");
	fprintf(stderr, "\tpeer-id: peer ID of the peer to modify\n\n");

	fprintf(stderr, "* swap_keys <peer-id>: swap primary and seconday key slots\n");
	fprintf(stderr, "\tpeer-id: peer ID of the peer to modify\n\n");

	fprintf(stderr, "* recv: receive packet and exit\n\n");

	fprintf(stderr, "* send <string>: send packet with string\n");
	fprintf(stderr, "\tstring: message to send to the peer\n\n");

	fprintf(stderr, "* listen_mcast: listen to ovpn-dco netlink multicast messages\n");
}

static int ovpn_parse_remote(struct ovpn_ctx *ovpn, const char *host, const char *service,
			     const char *vpn_addr)
{
	int ret;
	struct addrinfo *result;
	struct addrinfo hints = {
		.ai_family = ovpn->sa_family,
		.ai_socktype = SOCK_DGRAM,
		.ai_protocol = IPPROTO_UDP
	};

	if (host) {
		ret = getaddrinfo(host, service, &hints, &result);
		if (ret == EAI_NONAME || ret == EAI_FAIL)
			return -1;

		if (!(result->ai_family == AF_INET && result->ai_addrlen == sizeof(struct sockaddr_in)) &&
		    !(result->ai_family == AF_INET6 && result->ai_addrlen == sizeof(struct sockaddr_in6))) {
			ret = -EINVAL;
			goto out;
		}

		memcpy(&ovpn->remote, result->ai_addr, result->ai_addrlen);
	}

	ret = getaddrinfo(vpn_addr, NULL, &hints, &result);
	if (ret == EAI_NONAME || ret == EAI_FAIL)
		return -1;

	if (!(result->ai_family == AF_INET && result->ai_addrlen == sizeof(struct sockaddr_in)) &&
	    !(result->ai_family == AF_INET6 && result->ai_addrlen == sizeof(struct sockaddr_in6))) {
		ret = -EINVAL;
		goto out;
	}

	memcpy(&ovpn->peer_ip, result->ai_addr, result->ai_addrlen);
	ovpn->sa_family = result->ai_family;

	ret = 0;
out:
	freeaddrinfo(result);
	return ret;
}

static int ovpn_parse_new_peer(struct ovpn_ctx *ovpn, const char *peer_id, const char *raddr,
			       const char *rport, const char *vpnip)
{
	ovpn->peer_id = strtoul(peer_id, NULL, 10);
	if (errno == ERANGE) {
		fprintf(stderr, "peer ID value out of range\n");
		return -1;
	}

	return ovpn_parse_remote(ovpn, raddr, rport, vpnip);
}

static int ovpn_parse_set_peer(struct ovpn_ctx *ovpn, int argc, char *argv[])
{
	if (argc < 5) {
		usage(argv[0]);
		return -1;
	}

	ovpn->keepalive_interval = strtoul(argv[3], NULL, 10);
	if (errno == ERANGE) {
		fprintf(stderr, "keepalive interval value out of range\n");
		return -1;
	}

	ovpn->keepalive_timeout = strtoul(argv[4], NULL, 10);
	if (errno == ERANGE) {
		fprintf(stderr, "keepalive interval value out of range\n");
		return -1;
	}

	return 0;
}

int main(int argc, char *argv[])
{
	struct ovpn_ctx ovpn;
//	struct nl_ctx *ctx;
	int ret;

	if (argc < 3) {
		usage(argv[0]);
		return -1;
	}

	memset(&ovpn, 0, sizeof(ovpn));
	ovpn.sa_family = AF_INET;

	ovpn.ifindex = if_nametoindex(argv[1]);
	if (!ovpn.ifindex) {
		fprintf(stderr, "cannot find interface: %s\n",
			strerror(errno));
		return -1;
	}

	if (!strcmp(argv[2], "listen")) {
		char peer_id[10], vpnip[100];
		int n;
		FILE *fp;

		if (argc < 4) {
			usage(argv[0]);
			return -1;
		}

		ovpn.lport = strtoul(argv[3], NULL, 10);
		if (errno == ERANGE || ovpn.lport > 65535) {
			fprintf(stderr, "lport value out of range\n");
			return -1;
		}

		if (argc > 4 && !strcmp(argv[4], "ipv6"))
			ovpn.sa_family = AF_INET6;

		ret = ovpn_listen(&ovpn, ovpn.sa_family);
		if (ret < 0) {
			fprintf(stderr, "cannot listen on TCP socket\n");
			return ret;
		}

		fp = fopen(argv[4], "r");
		if (!fp) {
			fprintf(stderr, "cannot open file: %s\n", argv[4]);
			return -1;
		}

		while ((n = fscanf(fp, "%s %s\n", peer_id, vpnip)) == 2) {
			struct ovpn_ctx peer_ctx = { 0 };

			peer_ctx.ifindex = if_nametoindex(argv[1]);
			peer_ctx.sa_family = ovpn.sa_family;

			peer_ctx.socket = ovpn_accept(&ovpn);
			if (peer_ctx.socket < 0) {
				fprintf(stderr, "cannot accept connection!\n");
				return -1;
			}

			ret = ovpn_parse_new_peer(&peer_ctx, peer_id, NULL, NULL, vpnip);
			if (ret < 0) {
				fprintf(stderr, "error while parsing line\n");
				return -1;
			}

			ret = ovpn_new_peer(&peer_ctx, true);
			if (ret < 0) {
				fprintf(stderr, "cannot add peer to VPN: %s %s\n", peer_id, vpnip);
				return ret;
			}
		}
	} else if (!strcmp(argv[2], "connect")) {
		if (argc < 6) {
			usage(argv[0]);
			return -1;
		}

		ovpn.sa_family = AF_INET;

		ret = ovpn_parse_new_peer(&ovpn, argv[3], argv[4], argv[5], argv[6]);
		if (ret < 0) {
			fprintf(stderr, "Cannot parse remote peer data\n");
			return ret;
		}

		ret = ovpn_connect(&ovpn);
		if (ret < 0) {
			fprintf(stderr, "cannot connect TCP socket\n");
			return ret;
		}

		ret = ovpn_new_peer(&ovpn, true);
		if (ret < 0) {
			fprintf(stderr, "cannot add peer to VPN\n");
			close(ovpn.socket);
			return ret;
		}
	} else if (!strcmp(argv[2], "new_peer")) {
		if (argc < 8) {
			usage(argv[0]);
			return -1;
		}

		ovpn.lport = strtoul(argv[3], NULL, 10);
		if (errno == ERANGE || ovpn.lport > 65535) {
			fprintf(stderr, "lport value out of range\n");
			return -1;
		}

		ret = ovpn_parse_new_peer(&ovpn, argv[4], argv[5], argv[6], argv[7]);
		if (ret < 0)
			return ret;

		ret = ovpn_udp_socket(&ovpn, AF_INET6);//ovpn.sa_family);
		if (ret < 0)
			return ret;

		ret = ovpn_new_peer(&ovpn, false);
		if (ret < 0) {
			fprintf(stderr, "cannot add peer to VPN\n");
			return ret;
		}
	} else if (!strcmp(argv[2], "new_multi_peer")) {
		char peer_id[10], raddr[128], rport[10], vpnip[100];
		FILE *fp;
		int n;

		if (argc < 5) {
			usage(argv[0]);
			return -1;
		}

		ovpn.lport = strtoul(argv[3], NULL, 10);
		if (errno == ERANGE || ovpn.lport > 65535) {
			fprintf(stderr, "lport value out of range\n");
			return -1;
		}

		fp = fopen(argv[4], "r");
		if (!fp) {
			fprintf(stderr, "cannot open file: %s\n", argv[4]);
			return -1;
		}

		ret = ovpn_udp_socket(&ovpn, AF_INET6);
		if (ret < 0)
			return ret;

		while ((n = fscanf(fp, "%s %s %s %s\n", peer_id, raddr, rport, vpnip)) == 4) {
			struct ovpn_ctx peer_ctx = { 0 };

			peer_ctx.ifindex = if_nametoindex(argv[1]);
			peer_ctx.socket = ovpn.socket;
			peer_ctx.sa_family = AF_UNSPEC;

			ret = ovpn_parse_new_peer(&peer_ctx, peer_id, raddr, rport, vpnip);
			if (ret < 0) {
				fprintf(stderr, "error while parsing line\n");
				return -1;
			}

			ret = ovpn_new_peer(&peer_ctx, false);
			if (ret < 0) {
				fprintf(stderr, "cannot add peer to VPN: %s %s %s %s\n", peer_id,
					raddr, rport, vpnip);
				return ret;
			}
		}
	} else if (!strcmp(argv[2], "set_peer")) {
		ovpn.peer_id = strtoul(argv[3], NULL, 10);
		if (errno == ERANGE) {
			fprintf(stderr, "peer ID value out of range\n");
			return -1;
		}

		argv++;
		argc--;

		ret = ovpn_parse_set_peer(&ovpn, argc, argv);
		if (ret < 0)
			return ret;

		ret = ovpn_set_peer(&ovpn);
		if (ret < 0) {
			fprintf(stderr, "cannot set peer to VPN\n");
			return ret;
		}
	} else if (!strcmp(argv[2], "del_peer")) {
		if (argc < 4) {
			usage(argv[0]);
			return -1;
		}

		ovpn.peer_id = strtoul(argv[3], NULL, 10);
		if (errno == ERANGE) {
			fprintf(stderr, "peer ID value out of range\n");
			return -1;
		}

		ret = ovpn_del_peer(&ovpn);
		if (ret < 0) {
			fprintf(stderr, "cannot delete peer to VPN\n");
			return ret;
		}
	} else if (!strcmp(argv[2], "get_peer")) {
		ovpn.peer_id = PEER_ID_UNDEF;
		if (argc > 3)
			ovpn.peer_id = strtoul(argv[3], NULL, 10);

		fprintf(stderr, "List of peers connected to: %s\n", argv[1]);

		ret = ovpn_get_peer(&ovpn);
		if (ret < 0) {
			fprintf(stderr, "cannot get peer(s): %d\n", ret);
			return ret;
		}
	} else if (!strcmp(argv[2], "new_key")) {
		if (argc < 6) {
			usage(argv[0]);
			return -1;
		}

		ovpn.peer_id = strtoul(argv[3], NULL, 10);
		if (errno == ERANGE) {
			fprintf(stderr, "peer ID value out of range\n");
			return -1;
		}

		ret = ovpn_read_cipher(argv[4], &ovpn);
		if (ret < 0)
			return ret;

		ret = ovpn_read_key_direction(argv[5], &ovpn);
		if (ret < 0)
			return ret;

		ret = ovpn_read_key(argv[6], &ovpn);
		if (ret)
			return ret;

		ret = ovpn_new_key(&ovpn);
		if (ret < 0) {
			fprintf(stderr, "cannot set key\n");
			return ret;
		}
	} else if (!strcmp(argv[2], "del_key")) {
		if (argc < 3) {
			usage(argv[0]);
			return -1;
		}

		ovpn.peer_id = strtoul(argv[3], NULL, 10);
		if (errno == ERANGE) {
			fprintf(stderr, "peer ID value out of range\n");
			return -1;
		}

		argv++;
		argc--;

		ret = ovpn_del_key(&ovpn);
		if (ret < 0) {
			fprintf(stderr, "cannot delete key\n");
			return ret;
		}
	} else if (!strcmp(argv[2], "swap_keys")) {
		if (argc < 3) {
			usage(argv[0]);
			return -1;
		}

		ovpn.peer_id = strtoul(argv[3], NULL, 10);
		if (errno == ERANGE) {
			fprintf(stderr, "peer ID value out of range\n");
			return -1;
		}

		argv++;
		argc--;

		ret = ovpn_swap_keys(&ovpn);
		if (ret < 0) {
			fprintf(stderr, "cannot swap keys\n");
			return ret;
		}
	}/* else if (!strcmp(argv[2], "recv")) {
		ctx = ovpn_register(&ovpn);
		if (!ctx) {
			fprintf(stderr, "cannot register for packets\n");
			return -1;
		}

		ret = ovpn_nl_recvmsgs(ctx);
		nl_ctx_free(ctx);
	}*/ else if (!strcmp(argv[2], "send")) {
		if (argc < 4) {
			usage(argv[0]);
			return -1;
		}

		ret = ovpn_send_data(&ovpn, argv[3], strlen(argv[3]) + 1);
		if (ret < 0)
			fprintf(stderr, "cannot send data\n");
	} else if (!strcmp(argv[2], "listen_mcast")) {
		ovpn_listen_mcast();
	} else {
		usage(argv[0]);
		return -1;
	}

	return ret;
}
