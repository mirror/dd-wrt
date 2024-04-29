/*
 **************************************************************************
 * Copyright (c) 2016-2017, 2020, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTUOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/inet.h>
#include <linux/of.h>
#include <linux/ipv6.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <asm/atomic.h>
#include <linux/debugfs.h>
#include <linux/completion.h>
#include <linux/vmalloc.h>
#include <net/icmp.h>
#include <net/route.h>
#include <net/ip6_route.h>

#include <nss_api_if.h>
#include <nss_ipsec.h>
#include <nss_ipsecmgr.h>

#include "nss_ipsecmgr_priv.h"

extern struct nss_ipsecmgr_drv *ipsecmgr_ctx;

/*
 *
 * nss_ipsecmgr_flow_resp()
 * 	response for the flow message
 *
 * Note: we don't have anything to process for flow responses as of now
 */
static void nss_ipsecmgr_flow_resp(void *app_data, struct nss_ipsec_msg *nim)
{
	struct nss_ipsecmgr_flow_entry *flow __attribute__((unused)) = app_data;

	return;
}

/*
 * nss_ipsecmgr_flow_update()
 * 	update the flow with its associated data and notify NSS
 */
static void nss_ipsecmgr_flow_update(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_ref *ref, struct nss_ipsec_msg *nim)
{
	struct nss_ipsecmgr_flow_entry *flow;
	struct nss_ipsec_tuple local_tuple;
	struct nss_ipsec_tuple *flow_tuple;
	struct nss_ipsec_msg nss_nim;

	flow = container_of(ref, struct nss_ipsecmgr_flow_entry, ref);
	flow_tuple = &flow->nim.tuple;

	/*
	 * Create a local copy of flow tuple
	 */
	memcpy(&local_tuple, flow_tuple, sizeof(struct nss_ipsec_tuple));
	memcpy(&flow->nim, nim, sizeof(struct nss_ipsec_msg));
	flow->data.sa_overhead = nss_ipsecmgr_ref_overhead(ref);

	/*
	 * If, this flow is only getting updated with a new SA contents. We
	 * need to make sure that the existing selector remains the same
	 */
	if (nss_ipsecmgr_ref_is_updated(ref)) {
		memcpy(flow_tuple, &local_tuple, sizeof(struct nss_ipsec_tuple));
	}

	/*
	 * Convert the message to NSS format
	 */
	nss_ipsecmgr_copy_nim(&flow->nim, &nss_nim);

	if (nss_ipsec_tx_msg(ipsecmgr_ctx->nss_ctx, &nss_nim) != NSS_TX_SUCCESS) {
		/*
		 * XXX: Stop the TX queue and add this "entry"
		 * to pending queue
		 */
		nss_ipsecmgr_info("%px:unable to send the flow_update message\n", ref);
		return;
	}
}

/*
 * nss_ipsecmgr_flow_free()
 * 	free the associated flow entry and notify NSS
 */
static void nss_ipsecmgr_flow_free(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_ref *ref)
{
	struct nss_ipsecmgr_flow_entry *flow = container_of(ref, struct nss_ipsecmgr_flow_entry, ref);
	struct nss_ipsecmgr_flow_db *db = &ipsecmgr_ctx->flow_db;
	struct nss_ipsec_msg nss_nim;

	/*
	 * update the common message structure
	 */
	flow->nim.cm.type = NSS_IPSEC_MSG_TYPE_DEL_RULE;

	/*
	 * Convert the message to NSS format
	 */
	nss_ipsecmgr_copy_nim(&flow->nim, &nss_nim);

	if (nss_ipsec_tx_msg(ipsecmgr_ctx->nss_ctx, &nss_nim) != NSS_TX_SUCCESS) {
		/*
		 * XXX: add this "entry" to pending queue
		 */
		nss_ipsecmgr_info("%px:unable to send flow_free message\n", ref);
	}

	list_del_init(&flow->node);
	atomic_dec(&db->num_entries);
	kfree(flow);
}

/*
 * nss_ipsecmgr_flow_dump()
 *	Display the common info for one flow.
 */
static size_t nss_ipsecmgr_flow_dump(struct net_device *dev, struct nss_ipsec_msg *nim, char *buf, int max_len)
{
	struct nss_ipsec_tuple *tuple = &nim->tuple;
	struct nss_ipsec_rule_oip *oip = &nim->msg.rule.oip;
	uint32_t src_ip[4] = {0}, dst_ip[4] = {0};
	uint32_t esp_spi;
	size_t len;
	char *type;

	switch (nim->type) {
	case NSS_IPSEC_TYPE_ENCAP:
		type = "encap";
		esp_spi = oip->esp_spi;
		break;

	case NSS_IPSEC_TYPE_DECAP:
		type = "decap";
		esp_spi = tuple->esp_spi;
		break;

	default:
		nss_ipsecmgr_info("%px:Invalid interface type(%d)\n", nim, nim->type);
		return 0;

	}

	switch (tuple->ip_ver) {
	case IPVERSION:
		len = snprintf(buf, max_len, "3-tuple type=%s tunnelid=%s ip_ver=4 src_ip=%pI4h dst_ip=%pI4h proto=%d spi=%x\n",
				type, dev->name, &tuple->src_addr[0], &tuple->dst_addr[0], tuple->proto_next_hdr, esp_spi);
		break;

	case 6:
		nss_ipsecmgr_v6addr_hton(tuple->dst_addr, dst_ip);
		nss_ipsecmgr_v6addr_hton(tuple->src_addr, src_ip);

		len = snprintf(buf, max_len, "3-tuple type=%s tunnelid=%s ip_ver=6 src_ip=%pI6c dst_ip=%pI6c proto=%d spi=%x\n",
				type, dev->name, src_ip, dst_ip, tuple->proto_next_hdr, esp_spi);
		break;

	default:
		nss_ipsecmgr_info("%px:Invalid IP_VERSION (%d)\n", tuple, tuple->ip_ver);
		return 0;
	}

	return len;
}

/*
 * nss_ipsecmgr_flow_stats_read()
 *	read flow statistics
 */
ssize_t nss_ipsecmgr_flow_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	struct nss_ipsecmgr_flow_db *db = &ipsecmgr_ctx->flow_db;
	struct nss_ipsecmgr_flow_entry *entry;
	struct nss_ipsec_rule_data *data;
	struct net_device *dev;
	struct list_head *head;
	uint32_t num_entries;
	uint32_t len;
	ssize_t ret;
	int max_len;
	char *buf;
	int i;

	num_entries = atomic_read(&db->num_entries);
	if (!num_entries) {
		return 0;
	}

	len = 0;
	max_len = (num_entries * NSS_IPSECMGR_PER_FLOW_STATS_SIZE);

	buf = vzalloc(max_len);
	if (!buf) {
		nss_ipsecmgr_error("Memory allocation failed for buffer\n");
		return 0;
	}

	read_lock_bh(&ipsecmgr_ctx->lock);

	/*
	 * Get the flow information from flow db. for both encap and decap
	 */
	head = db->entries;

	for (i = NSS_IPSECMGR_MAX_FLOW; ((max_len - len) > 0) && i--; head++) {
		list_for_each_entry(entry, head, node) {
			dev = dev_get_by_index(&init_net, entry->nim.tunnel_id);
			if (!dev) {

				/*
				 * If, the associated tunnel is deleted and
				 * the flow remains in table then reassociate
				 * the flow to the default tunnel
				 */
				dev = ipsecmgr_ctx->ndev;
				dev_hold(dev);
				entry->nim.tunnel_id = dev->ifindex;
			}

			if (unlikely((max_len - len) <= 0)) {
				dev_put(dev);
				break;
			}

			data = &entry->nim.msg.rule.data;
			len += nss_ipsecmgr_flow_dump(dev, &entry->nim, buf + len, max_len - len);
			len += snprintf(buf + len, max_len - len, "cindex:%d\n\n", data->crypto_index);

			dev_put(dev);
		}
	}

	read_unlock_bh(&ipsecmgr_ctx->lock);

	ret = simple_read_from_buffer(ubuf, sz, ppos, buf, len);
	vfree(buf);

	return ret;
}

/*
 * nss_ipsecmgr_per_flow_stats_resp()
 *	response for the flow message
 *	Note: we don't have anything to process for flow responses as of now
 */
static void nss_ipsecmgr_per_flow_stats_resp(void *app_data, struct nss_ipsec_msg *nim)
{
	struct nss_ipsec_msg *resp_nim = &ipsecmgr_ctx->resp_nim;

	/*
	 * Match the sequence number of the nim from NSS to that of the
	 * global resp nim. This will ensure the response is valid against
	 * flow parameters stored in global reponse nim. This will help to
	 * discard any stale responses from NSS.
	 */
	if (nim->cm.app_data != atomic_read(&ipsecmgr_ctx->seq_num)) {
		resp_nim->cm.response = NSS_CMN_RESPONSE_EMSG;
		goto done;

	}

	if (nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		resp_nim->cm.response = nim->cm.response;
		goto done;
	}

	/*
	 * Cannot copy entire nim because IP addresses will be in diffrent format in side the nim
	 */
	resp_nim->msg.stats.flow.processed  = nim->msg.stats.flow.processed;
	resp_nim->cm.response = nim->cm.response;

done:
	complete(&ipsecmgr_ctx->complete);
}

/*
 * nss_ipsecmgr_per_flow_stats_read()
 *	read flow statistics
 */
ssize_t nss_ipsecmgr_per_flow_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	struct nss_ipsec_flow_stats *flow_stats;
	struct nss_ipsec_msg *flow_nim, nss_nim;
	ssize_t ret, len, max_len;
	struct net_device *dev;
	char *buf;

	/*
	 * only one caller will be allowed to send a message
	 */
	if (down_interruptible(&ipsecmgr_ctx->sem)) {
		return 0;
	}

	flow_nim  = &ipsecmgr_ctx->resp_nim;

	/*
	 * The check is to ensure prevent the corrupted or uninitialized
	 * nim to be send to NSS.
	 */
	if (flow_nim->cm.len != NSS_IPSEC_MSG_LEN) {
		nss_ipsecmgr_error("wrong message length\n");
		goto error;
	}

	/*
	 * Update the sequence number in the app_data field of common message.
	 */
	flow_nim->cm.app_data = atomic_read(&ipsecmgr_ctx->seq_num);

	/*
	 * Before send the nim to NSS convert the ip addresses to NSS order
	 */
	nss_ipsecmgr_copy_nim(flow_nim, &nss_nim);

	/*
	 * send stats message to nss
	 */
	if (nss_ipsec_tx_msg(ipsecmgr_ctx->nss_ctx, &nss_nim) != NSS_TX_SUCCESS) {
		nss_ipsecmgr_error("nss tx msg error\n");
		goto error;
	}

	/*
	 * Blocking call, wait till we get ACK for this msg.
	 */
	ret = wait_for_completion_timeout(&ipsecmgr_ctx->complete, NSS_IPSECMGR_MSG_SYNC_TIMEOUT_TICKS);
	if (!ret) {
		nss_ipsecmgr_error("nss stats message timed out \n");

		/*
		 * increment the seq_num so that if any stale response comes
		 * it will get ignored.
		 */
		atomic_inc(&ipsecmgr_ctx->seq_num);

		goto error;
	}

	/*
	 * need to ensure that the response data has correctly arrived in
	 * current CPU cache
	 */
	smp_rmb();

	if (flow_nim->cm.response != NSS_CMN_RESPONSE_ACK) {
		goto error;
	}

	buf = vzalloc(NSS_IPSECMGR_PER_FLOW_STATS_SIZE);
	if (!buf) {
		nss_ipsecmgr_error("Memory allocation failed for buffer\n");
		goto error;
	}

	dev = dev_get_by_index(&init_net, flow_nim->tunnel_id);
	if (!dev) {

		/*
		 * Clean the global response nim
		 */
		memset(&ipsecmgr_ctx->resp_nim, 0, sizeof(ipsecmgr_ctx->resp_nim));
		vfree(buf);
		goto error;
	}

	/*
	 * packet stats
	 */
	max_len = NSS_IPSECMGR_PER_FLOW_STATS_SIZE;
	len = nss_ipsecmgr_flow_dump(dev, flow_nim, buf, max_len);

	max_len = max_len - len;
	if (max_len <= 0) {
		goto done;
	}

	flow_stats = &flow_nim->msg.stats.flow;
	len += snprintf(buf + len, max_len, "processed: %d\n\n", flow_stats->processed);
done:
	ret = simple_read_from_buffer(ubuf, sz, ppos, buf, len);

	dev_put(dev);
	vfree(buf);
	up(&ipsecmgr_ctx->sem);

	return ret;

error:
	up(&ipsecmgr_ctx->sem);
	return 0;
}

/*
 * nss_ipsecmgr_per_flow_stats_write()
 *	write flow entry
 */
ssize_t nss_ipsecmgr_per_flow_stats_write(struct file *file, const char __user *ubuf, size_t count, loff_t *f_pos)
{
	struct nss_ipsec_msg *resp_nim = &ipsecmgr_ctx->resp_nim;
	uint8_t *buf, *src_ip, *dst_ip, *type, *tunnel_id;
	uint32_t interface, ip_ver, proto;
	struct nss_ipsec_tuple *tuple;
	struct nss_ipsec_rule_oip *oip;
	struct net_device *dev;
	uint32_t buf_size;
	ssize_t ret;
	int status;

	if (*f_pos > NSS_IPSECMGR_PER_FLOW_BUF_SIZE) {
		return -EINVAL;
	}

	/*
	 * add all the expected input buffers into a single
	 * one
	 */
	buf_size = NSS_IPSECMGR_PER_FLOW_BUF_SIZE;
	buf_size = buf_size + NSS_IPSECMGR_PER_FLOW_BUF_SRC_IP_SIZE;
	buf_size = buf_size + NSS_IPSECMGR_PER_FLOW_BUF_DST_IP_SIZE;
	buf_size = buf_size + NSS_IPSECMGR_PER_FLOW_BUF_TYPE_SIZE;
	buf_size = buf_size + IFNAMSIZ;

	buf = vzalloc(buf_size);
	if (!buf) {
		return -ENOMEM;
	}

	ret = simple_write_to_buffer(buf, NSS_IPSECMGR_PER_FLOW_BUF_SIZE, f_pos, ubuf, count);
	if (ret < 0) {
		vfree(buf);
		return -ENOMEM;
	}

	/*
	 * need to ensure that the buffer ends with NULL character
	 */
	buf[ret] = '\0';

	/*
	 * only one outstanding read or write is allowed
	 */
	if (down_interruptible(&ipsecmgr_ctx->sem)) {
		vfree(buf);
		return -EINTR;
	}

	/*
	 * increment the sequence no. here to ensure
	 * that the any pending responses after this
	 * are invalid
	 */
	atomic_inc(&ipsecmgr_ctx->seq_num);

	/*
	 * prepare for string to numeric conversion of the data
	 */
	tuple  = &resp_nim->tuple;
	oip  = &resp_nim->msg.rule.oip;
	src_ip = buf + NSS_IPSECMGR_PER_FLOW_BUF_SIZE;
	dst_ip = src_ip + NSS_IPSECMGR_PER_FLOW_BUF_SRC_IP_SIZE;
	type = dst_ip + NSS_IPSECMGR_PER_FLOW_BUF_DST_IP_SIZE;
	tunnel_id = type + NSS_IPSECMGR_PER_FLOW_BUF_TYPE_SIZE;

	status = sscanf(buf, "type=%s tunnelid=%s ip_ver=%d src_ip=%s dst_ip=%s proto=%d spi=%x",
			type, tunnel_id, &ip_ver, src_ip, dst_ip, &proto, &tuple->esp_spi);
	if (status <= 0) {
		goto error;
	}

	if (strcmp(type, "encap") && strcmp(type, "decap")) {
		goto error;
	}

	if ((ip_ver != IPVERSION) && (ip_ver != 6)) {
		goto error;
	}

	dev = dev_get_by_name(&init_net, tunnel_id);
	if (!dev) {
		nss_ipsecmgr_info("Invalid tunnel Id:%s\n", tunnel_id);
		goto error;
	}

	resp_nim->tunnel_id = dev->ifindex;
	dev_put(dev);

	tuple->proto_next_hdr = proto;

	switch (ip_ver) {
	case IPVERSION: /* ipv4 */
		in4_pton(src_ip, strlen(src_ip), (uint8_t *)&tuple->src_addr[0], '\0', NULL);
		tuple->src_addr[0] = ntohl(tuple->src_addr[0]);

		in4_pton(dst_ip, strlen(dst_ip), (uint8_t *)&tuple->dst_addr[0], '\0', NULL);
		tuple->dst_addr[0] = ntohl(tuple->dst_addr[0]);

		tuple->ip_ver = IPVERSION;
		break;

	case 6: /* ipv6 */
		in6_pton(src_ip, strlen(src_ip), (uint8_t *)&tuple->src_addr[0], '\0', NULL);
		nss_ipsecmgr_v6addr_ntoh(tuple->src_addr, tuple->src_addr);

		in6_pton(dst_ip, strlen(dst_ip), (uint8_t *)&tuple->dst_addr[0], '\0', NULL);
		nss_ipsecmgr_v6addr_ntoh(tuple->dst_addr, tuple->dst_addr);

		tuple->ip_ver = 6;
		break;

	default:
		BUG_ON(true);
	}

	/*
	 * prepare IPsec message
	 */
	if (!strcmp(type, "encap")) {
		interface = ipsecmgr_ctx->encap_ifnum;
		resp_nim->type = NSS_IPSEC_TYPE_ENCAP;
		oip->esp_spi = tuple->esp_spi;
	} else {
		interface = ipsecmgr_ctx->decap_ifnum;
		resp_nim->type = NSS_IPSEC_TYPE_DECAP;
	}

	nss_ipsec_msg_init(resp_nim, interface, NSS_IPSEC_MSG_TYPE_SYNC_FLOW_STATS, NSS_IPSEC_MSG_LEN, nss_ipsecmgr_per_flow_stats_resp, NULL);

	vfree(buf);
	up(&ipsecmgr_ctx->sem);
	return ret;

error:
	memset(resp_nim, 0, sizeof(struct nss_ipsec_msg));
	vfree(buf);
	up(&ipsecmgr_ctx->sem);
	return -EINVAL;
}

/*
 * nss_ipsecmgr_encap_flow_init()
 * 	initiallize the encap flow with a particular type
 */
void nss_ipsecmgr_encap_flow_init(struct nss_ipsec_msg *nim, enum nss_ipsec_msg_type type, struct nss_ipsecmgr_priv *priv)
{
	memset(nim, 0, sizeof(struct nss_ipsec_msg));
	nss_ipsec_msg_init(nim, ipsecmgr_ctx->encap_ifnum, type, NSS_IPSEC_MSG_LEN, nss_ipsecmgr_flow_resp, priv->dev);
	nim->tunnel_id = priv->dev->ifindex;
	nim->type = NSS_IPSEC_TYPE_ENCAP;
}

/*
 * nss_ipsecmgr_decap_flow_init()
 * 	initiallize the decap flow with a particular type
 */
void nss_ipsecmgr_decap_flow_init(struct nss_ipsec_msg *nim, enum nss_ipsec_msg_type type, struct nss_ipsecmgr_priv *priv)
{
	memset(nim, 0, sizeof(struct nss_ipsec_msg));
	nss_ipsec_msg_init(nim, ipsecmgr_ctx->decap_ifnum, type, NSS_IPSEC_MSG_LEN, nss_ipsecmgr_flow_resp, priv->dev);
	nim->tunnel_id = priv->dev->ifindex;
	nim->type = NSS_IPSEC_TYPE_DECAP;
}

/*
 * nss_ipsecmgr_copy_encap_v4_flow()
 * 	copy flow data into the selector
 */
void nss_ipsecmgr_copy_encap_v4_flow(struct nss_ipsec_msg *nim, struct nss_ipsecmgr_encap_v4_tuple *flow)
{
	struct nss_ipsec_tuple *tuple = &nim->tuple;

	tuple->dst_addr[0] = flow->dst_ip;
	tuple->src_addr[0] = flow->src_ip;
	tuple->proto_next_hdr = flow->protocol;
	tuple->ip_ver = IPVERSION;

	tuple->esp_spi = 0;
	tuple->dst_port = 0;
	tuple->src_port = 0;
}

/*
 * nss_ipsecmgr_copy_decap_v4_flow()
 * 	copy decap flow
 */
void nss_ipsecmgr_copy_decap_v4_flow(struct nss_ipsec_msg *nim, struct nss_ipsecmgr_sa_v4 *flow)
{
	struct nss_ipsec_tuple *tuple = &nim->tuple;

	tuple->dst_addr[0] = flow->dst_ip;
	tuple->src_addr[0] = flow->src_ip;
	tuple->proto_next_hdr = IPPROTO_ESP;
	tuple->esp_spi = flow->spi_index;
	tuple->ip_ver = IPVERSION;

	tuple->dst_port = 0;
	tuple->src_port = 0;
}

/*
 * nss_ipsecmgr_encap_v4_flow2key()
 * 	convert an encap v4_flow into a key
 */
void nss_ipsecmgr_encap_v4_flow2key(struct nss_ipsecmgr_encap_v4_tuple *flow, struct nss_ipsecmgr_key *key)
{
	nss_ipsecmgr_key_reset(key);

	nss_ipsecmgr_key_write_8(key, 4 /* v4 */, NSS_IPSECMGR_KEY_POS_IP_VER);
	nss_ipsecmgr_key_write_8(key, flow->protocol, NSS_IPSECMGR_KEY_POS_IP_PROTO);
	nss_ipsecmgr_key_write_32(key, flow->dst_ip, NSS_IPSECMGR_KEY_POS_IPV4_DST);
	nss_ipsecmgr_key_write_32(key, flow->src_ip, NSS_IPSECMGR_KEY_POS_IPV4_SRC);

	key->len = NSS_IPSECMGR_KEY_LEN_IPV4_ENCAP_FLOW;
}

/*
 * nss_ipsecmgr_decap_v4_flow2key()
 * 	convert a decap flow into a key
 */
void nss_ipsecmgr_decap_v4_flow2key(struct nss_ipsecmgr_sa_v4 *flow, struct nss_ipsecmgr_key *key)
{
	nss_ipsecmgr_key_reset(key);

	nss_ipsecmgr_key_write_8(key, 4 /* v4 */, NSS_IPSECMGR_KEY_POS_IP_VER);
	nss_ipsecmgr_key_write_8(key, IPPROTO_ESP, NSS_IPSECMGR_KEY_POS_IP_PROTO);
	nss_ipsecmgr_key_write_32(key, flow->dst_ip, NSS_IPSECMGR_KEY_POS_IPV4_DST);
	nss_ipsecmgr_key_write_32(key, flow->src_ip, NSS_IPSECMGR_KEY_POS_IPV4_SRC);
	nss_ipsecmgr_key_write_32(key, flow->spi_index, NSS_IPSECMGR_KEY_POS_IPV4_ESP_SPI);

	key->len = NSS_IPSECMGR_KEY_LEN_IPV4_DECAP_FLOW;
}

/*
 * nss_ipsecmgr_encap_v4_tuple2key()
 * 	convert a selector to key
 */
void nss_ipsecmgr_encap_tuple2key(struct nss_ipsec_tuple *tuple, struct nss_ipsecmgr_key *key)
{
	uint32_t i;

	nss_ipsecmgr_key_reset(key);
	switch (tuple->ip_ver) {
	case IPVERSION:
		nss_ipsecmgr_key_write_8(key, 4 /* v4 */, NSS_IPSECMGR_KEY_POS_IP_VER);
		nss_ipsecmgr_key_write_8(key, tuple->proto_next_hdr, NSS_IPSECMGR_KEY_POS_IP_PROTO);
		nss_ipsecmgr_key_write_32(key, nss_ipsecmgr_get_v4addr(tuple->dst_addr), NSS_IPSECMGR_KEY_POS_IPV4_DST);
		nss_ipsecmgr_key_write_32(key, nss_ipsecmgr_get_v4addr(tuple->src_addr), NSS_IPSECMGR_KEY_POS_IPV4_SRC);

		key->len = NSS_IPSECMGR_KEY_LEN_IPV4_ENCAP_FLOW;
		break;

	case 6:
		nss_ipsecmgr_key_write_8(key, 6 /* v6 */, NSS_IPSECMGR_KEY_POS_IP_VER);
		nss_ipsecmgr_key_write_8(key, tuple->proto_next_hdr, NSS_IPSECMGR_KEY_POS_IP_PROTO);

		for (i  = 0; i < 4; i++) {
			nss_ipsecmgr_key_write_32(key, tuple->dst_addr[i], NSS_IPSECMGR_KEY_POS_IPV6_DST + (i * 32));
			nss_ipsecmgr_key_write_32(key, tuple->src_addr[i], NSS_IPSECMGR_KEY_POS_IPV6_SRC + (i * 32));
		}

		key->len = NSS_IPSECMGR_KEY_LEN_IPV6_ENCAP_FLOW;
		break;

	default:
		nss_ipsecmgr_warn("%px:Invalid selector\n", tuple);
		return;
	}
}

/*
 * nss_ipsecmgr_decap_tuple2key()
 * 	convert a selector to key
 */
void nss_ipsecmgr_decap_tuple2key(struct nss_ipsec_tuple *tuple, struct nss_ipsecmgr_key *key)
{
	uint32_t i;

	nss_ipsecmgr_key_reset(key);

	switch (tuple->ip_ver) {
	case IPVERSION:
		nss_ipsecmgr_key_write_8(key, 4 /* v4 */, NSS_IPSECMGR_KEY_POS_IP_VER);
		nss_ipsecmgr_key_write_8(key, IPPROTO_ESP, NSS_IPSECMGR_KEY_POS_IP_PROTO);
		nss_ipsecmgr_key_write_32(key, nss_ipsecmgr_get_v4addr(tuple->dst_addr), NSS_IPSECMGR_KEY_POS_IPV4_DST);
		nss_ipsecmgr_key_write_32(key, nss_ipsecmgr_get_v4addr(tuple->src_addr), NSS_IPSECMGR_KEY_POS_IPV4_SRC);
		nss_ipsecmgr_key_write_32(key, tuple->esp_spi, NSS_IPSECMGR_KEY_POS_IPV4_ESP_SPI);

		key->len = NSS_IPSECMGR_KEY_LEN_IPV4_DECAP_FLOW;
		break;

	case 6:
		nss_ipsecmgr_key_write_8(key, 6 /* v6 */, NSS_IPSECMGR_KEY_POS_IP_VER);
		nss_ipsecmgr_key_write_8(key, IPPROTO_ESP, NSS_IPSECMGR_KEY_POS_IP_PROTO);

		for (i  = 0; i < 4; i++) {
			nss_ipsecmgr_key_write_32(key, tuple->dst_addr[i], NSS_IPSECMGR_KEY_POS_IPV6_DST + (i * 32));
			nss_ipsecmgr_key_write_32(key, tuple->src_addr[i], NSS_IPSECMGR_KEY_POS_IPV6_SRC + (i * 32));
		}

		nss_ipsecmgr_key_write_32(key, tuple->esp_spi, NSS_IPSECMGR_KEY_POS_IPV6_ESP_SPI);

		key->len = NSS_IPSECMGR_KEY_LEN_IPV6_DECAP_FLOW;
		break;

	default:
		nss_ipsecmgr_warn("%px:Invalid selector\n", tuple);
		return;
	}
}

/*
 * nss_ipsecmgr_copy_encap_v6_flow()
 * 	copy flow data into the selector
 */
void nss_ipsecmgr_copy_encap_v6_flow(struct nss_ipsec_msg *nim, struct nss_ipsecmgr_encap_v6_tuple *flow)
{
	struct nss_ipsec_tuple *tuple = &nim->tuple;

	memcpy(tuple->src_addr, flow->src_ip, sizeof(uint32_t) * 4);
	memcpy(tuple->dst_addr, flow->dst_ip, sizeof(uint32_t) * 4);

	tuple->proto_next_hdr = flow->next_hdr;
	tuple->ip_ver = 6;

	tuple->esp_spi = 0;
	tuple->dst_port = 0;
	tuple->src_port = 0;
}

/*
 * nss_ipsecmgr_copy_decap_v6_flow()
 * 	copy decap flow
 */
void nss_ipsecmgr_copy_decap_v6_flow(struct nss_ipsec_msg *nim, struct nss_ipsecmgr_sa_v6 *flow)
{
	struct nss_ipsec_tuple *tuple = &nim->tuple;

	memcpy(tuple->src_addr, flow->src_ip, sizeof(uint32_t) * 4);
	memcpy(tuple->dst_addr, flow->dst_ip, sizeof(uint32_t) * 4);

	tuple->esp_spi = flow->spi_index;
	tuple->ip_ver = 6;

	tuple->proto_next_hdr = IPPROTO_ESP;

	tuple->dst_port = 0;
	tuple->src_port = 0;
}

/*
 * nss_ipsecmgr_encap_v6_flow2key()
 * 	convert an encap v6_flow into a key
 */
void nss_ipsecmgr_encap_v6_flow2key(struct nss_ipsecmgr_encap_v6_tuple *flow, struct nss_ipsecmgr_key *key)
{
	uint32_t i;

	nss_ipsecmgr_key_reset(key);

	nss_ipsecmgr_key_write_8(key, 6 /* v6 */, NSS_IPSECMGR_KEY_POS_IP_VER);
	nss_ipsecmgr_key_write_8(key, flow->next_hdr, NSS_IPSECMGR_KEY_POS_IP_PROTO);

	for (i  = 0; i < 4; i++) {
		nss_ipsecmgr_key_write_32(key, flow->dst_ip[i], NSS_IPSECMGR_KEY_POS_IPV6_DST + (i * 32));
		nss_ipsecmgr_key_write_32(key, flow->src_ip[i], NSS_IPSECMGR_KEY_POS_IPV6_SRC + (i * 32));
	}

	key->len = NSS_IPSECMGR_KEY_LEN_IPV6_ENCAP_FLOW;
}

/*
 * nss_ipsecmgr_decap_v6_flow2key()
 * 	convert a decap flow into a key
 */
void nss_ipsecmgr_decap_v6_flow2key(struct nss_ipsecmgr_sa_v6 *flow, struct nss_ipsecmgr_key *key)
{
	uint32_t i;

	nss_ipsecmgr_key_reset(key);

	nss_ipsecmgr_key_write_8(key, 6 /* v6 */, NSS_IPSECMGR_KEY_POS_IP_VER);
	nss_ipsecmgr_key_write_8(key, IPPROTO_ESP, NSS_IPSECMGR_KEY_POS_IP_PROTO);

	for (i  = 0; i < 4; i++) {
		nss_ipsecmgr_key_write_32(key, flow->dst_ip[i], NSS_IPSECMGR_KEY_POS_IPV6_DST + (i * 32));
		nss_ipsecmgr_key_write_32(key, flow->src_ip[i], NSS_IPSECMGR_KEY_POS_IPV6_SRC + (i * 32));
	}

	nss_ipsecmgr_key_write_32(key, flow->spi_index, NSS_IPSECMGR_KEY_POS_IPV6_ESP_SPI);

	key->len = NSS_IPSECMGR_KEY_LEN_IPV6_DECAP_FLOW;
}

/*
 * nss_ipsecmgr_flow_lookup()
 * 	lookup flow in flow_db
 */
struct nss_ipsecmgr_ref *nss_ipsecmgr_flow_lookup(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_key *key)
{
	struct nss_ipsecmgr_flow_db *db = &ipsecmgr_ctx->flow_db;
	struct nss_ipsecmgr_flow_entry *entry;
	struct list_head *head;
	int idx;

	idx = nss_ipsecmgr_key_data2idx(key, NSS_IPSECMGR_MAX_FLOW);
	head = &db->entries[idx];

	list_for_each_entry(entry, head, node) {
		if (nss_ipsecmgr_key_cmp(&entry->key, key)) {
			return &entry->ref;
		}
	}

	return NULL;
}

/*
 * nss_ipsecmgr_flow_alloc()
 * 	allocate a flow entry
 */
struct nss_ipsecmgr_ref *nss_ipsecmgr_flow_alloc(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_key *key)
{
	struct nss_ipsecmgr_flow_entry *flow;
	struct nss_ipsecmgr_flow_db *db;
	struct nss_ipsecmgr_ref *ref;
	int idx;

	/*
	 * flow lookup before allocating a new one
	 */
	ref = nss_ipsecmgr_flow_lookup(priv, key);
	if (ref) {
		return ref;
	}

	flow = kzalloc(sizeof(struct nss_ipsecmgr_flow_entry), GFP_ATOMIC);
	if (!flow) {
		nss_ipsecmgr_info("failed to alloc flow_entry\n");
		return NULL;
	}

	flow->priv = priv;
	ref = &flow->ref;

	/*
	 * add flow to the database
	 */
	db = &ipsecmgr_ctx->flow_db;
	INIT_LIST_HEAD(&flow->node);

	/*
	 * update key
	 */
	idx = nss_ipsecmgr_key_data2idx(key, NSS_IPSECMGR_MAX_FLOW);

	memcpy(&flow->key, key, sizeof(struct nss_ipsecmgr_key));
	list_add(&flow->node, &db->entries[idx]);

	atomic_inc(&db->num_entries);

	/*
	 * initiallize the reference object
	 */
	nss_ipsecmgr_ref_init(ref, nss_ipsecmgr_flow_update, nss_ipsecmgr_flow_free);

	return ref;
}

/*
 * nss_ipsecmgr_flow_process_pmtu()
 *	process the mtu and send an ICMP error
 *
 * note: in case we do not send an ICMP error back to sender
 * then we rely on post fragmentation to handle larger than
 * MTU size packets
 */
bool nss_ipsecmgr_flow_process_pmtu(struct nss_ipsecmgr_priv *priv,
			struct sk_buff *skb,
			struct nss_ipsecmgr_flow_data *data)
{
	struct dst_entry *dst;
	struct flowi6 fl6;
	struct rtable *rt;
	uint32_t mtu;

	/*
	 * If length of the packet is more than computed MTU,
	 * then send ICMP PMTU error back to Linux
	 */
	mtu = atomic_read(&priv->outer_dst_mtu) - data->sa_overhead;
	if (likely(mtu >= skb->len))
		return false;

	switch (skb->protocol) {
	case htons(ETH_P_IP):
		if (unlikely(skb_dst(skb)))
			goto send_icmp;

		rt = ip_route_output(&init_net, ip_hdr(skb)->daddr, 0, 0, 0);
		if (IS_ERR(rt)) {
			return false;
		}

		if (rt->rt_type != RTN_UNICAST && rt->rt_type != RTN_LOCAL) {
			ip_rt_put(rt);
			return false;
		}

		skb_dst_set(skb, &rt->dst);
send_icmp:
		icmp_send(skb, ICMP_DEST_UNREACH, ICMP_FRAG_NEEDED, htonl(mtu));
		return true;

	case htons(ETH_P_IPV6):

		/*
		 * If the computed MTU is less than IPV6_MIN_MTU[1280]
		 * proceed with post fragmentation
		 */
		if (unlikely(mtu < IPV6_MIN_MTU))
			return false;

		if (likely(skb_dst(skb)))
			goto send_icmp6;

		memset(&fl6, 0, sizeof(fl6));
		memcpy(&fl6.daddr, &ipv6_hdr(skb)->saddr, sizeof(fl6.daddr));

		dst = ip6_route_output(&init_net, NULL, &fl6);
		if (IS_ERR(dst)) {
			return false;
		}

		skb_dst_set(skb, dst);
send_icmp6:
		icmpv6_send(skb, ICMPV6_PKT_TOOBIG, 0, mtu);
		return true;

	default:
		BUG_ON(true);
	}

	return true;
}

/*
 * nss_ipsecmgr_flow_offload()
 * 	check if the flow can be offloaded to NSS for encapsulation
 */
bool nss_ipsecmgr_flow_offload(struct nss_ipsecmgr_priv *priv, struct sk_buff *skb, struct nss_ipsecmgr_flow_data *data)
{
	struct nss_ipsecmgr_ref *subnet_ref, *flow_ref;
	struct nss_ipsecmgr_key subnet_key, flow_key;
	struct nss_ipsecmgr_flow_entry *flow;
	struct nss_ipsec_tuple *tuple;
	struct nss_ipsec_msg nim;

	nss_ipsecmgr_encap_flow_init(&nim, NSS_IPSEC_MSG_TYPE_ADD_RULE, priv);

	switch (skb->protocol) {
	case htons(ETH_P_IP):
		tuple = &nim.tuple;

		nss_ipsecmgr_v4_hdr2tuple(ip_hdr(skb), tuple);
		nss_ipsecmgr_encap_tuple2key(tuple, &flow_key);

		/*
		 * flow lookup is done with read lock
		 */
		read_lock_bh(&ipsecmgr_ctx->lock);

		/*
		 * if flow is found then proceed with the TX
		 */
		flow_ref = nss_ipsecmgr_flow_lookup(priv, &flow_key);
		if (flow_ref) {
			flow = container_of(flow_ref, struct nss_ipsecmgr_flow_entry, ref);
			memcpy(data, &flow->data, sizeof(*data));
			read_unlock_bh(&ipsecmgr_ctx->lock);
			return true;
		}

		read_unlock_bh(&ipsecmgr_ctx->lock);

		/*
		 * flow table miss results in lookup in the subnet table. If,
		 * a match is found then a rule is inserted in NSS for encapsulating
		 * this flow.
		 */
		nss_ipsecmgr_v4_subnet_tuple2key(tuple, &subnet_key);

		/*
		 * write lock as it can update the flow database
		 */
		write_lock_bh(&ipsecmgr_ctx->lock);

		subnet_ref = nss_ipsecmgr_v4_subnet_match(priv, &subnet_key);
		if (!subnet_ref) {
			write_unlock_bh(&ipsecmgr_ctx->lock);
			return false;
		}

		/*
		 * copy nim data from subnet entry
		 */
		nss_ipsecmgr_copy_subnet(&nim, subnet_ref);

		/*
		 * if, the same flow was added in between then flow alloc will return the
		 * same flow. The only side affect of this will be NSS getting duplicate
		 * add requests and thus rejecting one of them
		 */
		flow_ref = nss_ipsecmgr_flow_alloc(priv, &flow_key);
		if (!flow_ref) {
			write_unlock_bh(&ipsecmgr_ctx->lock);
			return false;
		}

		/*
		 * add reference to subnet and trigger an update
		 */
		nss_ipsecmgr_ref_add(flow_ref, subnet_ref);
		nss_ipsecmgr_ref_update(priv, flow_ref, &nim);

		flow = container_of(flow_ref, struct nss_ipsecmgr_flow_entry, ref);
		memcpy(data, &flow->data, sizeof(*data));
		write_unlock_bh(&ipsecmgr_ctx->lock);

		break;

	case htons(ETH_P_IPV6):
		tuple = &nim.tuple;

		nss_ipsecmgr_v6_hdr2tuple((struct ipv6hdr *)skb_network_header(skb), tuple);
		nss_ipsecmgr_encap_tuple2key(tuple, &flow_key);

		/*
		 * flow lookup is done with read lock
		 */
		read_lock_bh(&ipsecmgr_ctx->lock);

		/*
		 * if flow is found then proceed with the TX
		 */
		flow_ref = nss_ipsecmgr_flow_lookup(priv, &flow_key);
		if (flow_ref) {
			flow = container_of(flow_ref, struct nss_ipsecmgr_flow_entry, ref);
			memcpy(data, &flow->data, sizeof(*data));
			read_unlock_bh(&ipsecmgr_ctx->lock);
			return true;
		}

		read_unlock_bh(&ipsecmgr_ctx->lock);

		/*
		 * flow table miss results in lookup in the subnet table. If,
		 * a match is found then a rule is inserted in NSS for encapsulating
		 * this flow.
		 */
		nss_ipsecmgr_v6_subnet_tuple2key(tuple, &subnet_key);

		/*
		 * write lock as it can update the flow database
		 */
		write_lock(&ipsecmgr_ctx->lock);

		subnet_ref = nss_ipsecmgr_v6_subnet_match(priv, &subnet_key);
		if (!subnet_ref) {
			write_unlock(&ipsecmgr_ctx->lock);
			return false;
		}

		/*
		 * copy nim data from subnet entry
		 */
		nss_ipsecmgr_copy_subnet(&nim, subnet_ref);

		/*
		 * if, the same flow was added in between then flow alloc will return the
		 * same flow. The only side affect of this will be NSS getting duplicate
		 * add requests and thus rejecting one of them
		 */
		flow_ref = nss_ipsecmgr_flow_alloc(priv, &flow_key);
		if (!flow_ref) {
			write_unlock(&ipsecmgr_ctx->lock);
			return false;
		}

		/*
		 * add reference to subnet and trigger an update
		 */
		nss_ipsecmgr_ref_add(flow_ref, subnet_ref);
		nss_ipsecmgr_ref_update(priv, flow_ref, &nim);

		flow = container_of(flow_ref, struct nss_ipsecmgr_flow_entry, ref);
		memcpy(data, &flow->data, sizeof(*data));
		write_unlock(&ipsecmgr_ctx->lock);
		break;

	default:
		nss_ipsecmgr_warn("%px:protocol(%d) offload not supported\n", priv->dev, ntohs(skb->protocol));
		return false;
	}

	return true;
}
