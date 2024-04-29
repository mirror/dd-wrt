/*
 **************************************************************************
 * Copyright (c) 2016-2018, 2020, The Linux Foundation. All rights reserved.
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
#include <linux/of.h>
#include <linux/ipv6.h>
#include <linux/skbuff.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/rtnetlink.h>
#include <net/route.h>
#include <net/ip6_route.h>
#include <asm/atomic.h>
#include <linux/debugfs.h>
#include <linux/vmalloc.h>

#include <nss_api_if.h>
#include <nss_ipsec.h>
#include <nss_ipsecmgr.h>
#if defined(NSS_IPSECMGR_IPQ807X_SUPPORT)
#include <nss_crypto_api.h>
#else
#include <nss_crypto_if.h>
#endif
#include "nss_ipsecmgr_priv.h"

extern struct nss_ipsecmgr_drv *ipsecmgr_ctx;

/*
 * SA operation info
 */
struct nss_ipsecmgr_sa_info {
	struct nss_ipsec_msg nim;
	struct nss_ipsecmgr_key sa_key;
	struct nss_ipsecmgr_key child_key;
	struct nss_ipsecmgr_sa *sa;
	uint32_t fail_hash_thresh;
	uint32_t sa_overhead;
	uint32_t dst_mtu;

	struct nss_ipsecmgr_ref * (*child_alloc)(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_key *key);
	struct nss_ipsecmgr_ref * (*child_lookup)(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_key *key);
};

/*
 * nss_ipsecmgr_sa_dump()
 *	dump sa statistics
 */
static ssize_t nss_ipsecmgr_sa_dump(struct nss_ipsecmgr_sa_entry *sa, char *buf, ssize_t max_len)
{
	struct nss_ipsec_rule_data *data;
	struct nss_ipsec_rule_oip *oip;
	uint32_t addr[4];
	ssize_t len;
	char *type;

	oip = &sa->nim.msg.rule.oip;
	data = &sa->nim.msg.rule.data;

	switch (sa->nim.type) {
	case NSS_IPSEC_TYPE_ENCAP:
		type = "encap";
		break;

	case NSS_IPSEC_TYPE_DECAP:
		type = "decap";
		break;

	default:
		return 0;
	}

	len = snprintf(buf, max_len, "Type:%s\n", type);

	switch (oip->ip_ver) {
	case IPVERSION:
		len += snprintf(buf + len, max_len - len, "dst_ip: %pI4h\n", &oip->dst_addr[0]);
		len += snprintf(buf + len, max_len - len, "src_ip: %pI4h\n", &oip->src_addr[0]);
		break;

	case 6:
		len += snprintf(buf + len, max_len - len, "dst_ip: %pI6c\n", nss_ipsecmgr_v6addr_ntoh(oip->dst_addr, addr));
		len += snprintf(buf + len, max_len - len, "src_ip: %pI6c\n", nss_ipsecmgr_v6addr_ntoh(oip->src_addr, addr));
		break;
	}

	len += snprintf(buf + len, max_len - len, "spi_idx: 0x%x\n", oip->esp_spi);
	len += snprintf(buf + len, max_len - len, "ttl: %d\n", oip->ttl_hop_limit);
	len += snprintf(buf + len, max_len - len, "crypto session: %d\n", data->crypto_index);

	len += snprintf(buf + len, max_len - len, "ESN: %d\n", data->enable_esn);
	len += snprintf(buf + len, max_len - len, "seq_num: %llx\n\n", sa->pkts.seq_num);

	/*
	 * display window information only for decap SA
	 */
	len += snprintf(buf + len, max_len - len, "win_size: %d\n", sa->pkts.window_size);
	len += snprintf(buf + len, max_len - len, "wmax: 0x%llx\n\n", sa->pkts.window_max);

	/*
	 * packet stats
	 */
	len += snprintf(buf + len, max_len - len, "processed: %llu\n", sa->pkts.count);
	len += snprintf(buf + len, max_len - len, "no_headroom: %d\n", sa->pkts.no_headroom);
	len += snprintf(buf + len, max_len - len, "no_tailroom: %d\n", sa->pkts.no_tailroom);
	len += snprintf(buf + len, max_len - len, "no_buf: %d\n", sa->pkts.no_buf);
	len += snprintf(buf + len, max_len - len, "fail_queue: %d\n", sa->pkts.fail_queue);
	len += snprintf(buf + len, max_len - len, "fail_hash: %d\n", sa->pkts.fail_hash);
	len += snprintf(buf + len, max_len - len, "fail_replay: %d\n\n\n", sa->pkts.fail_replay);

	return len;
}

/*
 * nss_ipsecmgr_sa_stats_read()
 * 	read sa statistics
 */
ssize_t nss_ipsecmgr_sa_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	struct nss_ipsecmgr_sa_db *sa_db;
	struct nss_ipsecmgr_sa_entry *sa;
	ssize_t len, max_len, ret;
	struct list_head *head;
	uint32_t num_entries;
	char *buf;
	int i;

	sa_db = &ipsecmgr_ctx->sa_db;

	num_entries = atomic_read(&sa_db->num_entries);
	if (!num_entries) {
		return 0;
	}

	len = 0;
	max_len = num_entries * NSS_IPSECMGR_SA_STATS_SIZE;

	buf = vzalloc(max_len);
	if (!buf) {
		nss_ipsecmgr_error("unable to allocate local buffer for SA stats\n");
		return 0;
	}

	/*
	 * walk the SA database for each entry and retrieve the stats
	 */
	read_lock_bh(&ipsecmgr_ctx->lock);

	head = sa_db->entries;
	for (i = NSS_IPSECMGR_MAX_SA; ((max_len - len) > 0) && i--; head++) {
		list_for_each_entry(sa, head, node) {
			if (unlikely((max_len - len) <= 0)) {
				break;
			}

			len += nss_ipsecmgr_sa_dump(sa, buf + len, max_len - len);
		}
	}

	read_unlock_bh(&ipsecmgr_ctx->lock);

	ret = simple_read_from_buffer(ubuf, sz, ppos, buf, len);
	vfree(buf);

	return ret;
}

/*
 * nss_ipsecmgr_sa_free()
 * 	deallocate the SA if there are no references
 */
static void nss_ipsecmgr_sa_free(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_ref *ref)
{
	struct nss_ipsecmgr_sa_entry *entry = container_of(ref, struct nss_ipsecmgr_sa_entry, ref);
	struct nss_ipsecmgr_sa_db *db = &ipsecmgr_ctx->sa_db;

	if (!nss_ipsecmgr_ref_is_empty(ref)) {
		return;
	}

	/*
	 * there should be no references remove it from
	 * the sa_db and free the entry
	 */
	list_del_init(&entry->node);
	atomic_dec(&db->num_entries);
	kfree(entry);
}

/*
 * nss_ipsecmgr_sa_get_overhead()
 *	get overhead from SA
 */
static uint32_t nss_ipsecmgr_sa_overhead(struct nss_ipsecmgr_ref *ref)
{
	struct nss_ipsecmgr_sa_entry *entry;
	entry = container_of(ref, struct nss_ipsecmgr_sa_entry, ref);
	return entry->sa_overhead;
}

/*
 * nss_ipsecmgr_sa_del()
 * 	delete sa/child from the reference chain
 */
static bool nss_ipsecmgr_sa_del(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_sa_info *info)
{
	struct nss_ipsecmgr_ref *sa_ref, *child_ref;

	/*
	 * lock database
	 */
	write_lock_bh(&ipsecmgr_ctx->lock);

	/*
	 * search the flow for deletion
	 */
	child_ref = info->child_lookup(priv, &info->child_key);
	if (!child_ref) {
		/*
		 * unlock device
		 */
		write_unlock_bh(&ipsecmgr_ctx->lock);

		nss_ipsecmgr_warn("%px:failed to lookup child_entry\n", priv->dev);
		nss_ipsecmgr_trace("%px:child_lookup(%px)\n", priv, info->child_lookup);
		return false;
	}

	/*
	 * search the SA in sa_db
	 */
	sa_ref = nss_ipsecmgr_sa_lookup(&info->sa_key);
	if (!sa_ref) {
		write_unlock_bh(&ipsecmgr_ctx->lock);

		nss_ipsecmgr_warn("%px:failed to lookup sa_entry\n", priv->dev);
		return false;
	}

	/*
	 * Remove the reference if it is associated the SA
	 */
	if (nss_ipsecmgr_ref_is_child(child_ref, sa_ref)) {
		nss_ipsecmgr_ref_free(priv, child_ref);
	}

	/*
	 * This deallocates the SA if there are no further references
	 */
	nss_ipsecmgr_sa_free(priv, sa_ref);

	write_unlock_bh(&ipsecmgr_ctx->lock);
	return true;
}

/*
 * nss_ipsecmgr_sa_add()
 * 	add sa/child from the reference chain
 */
static bool nss_ipsecmgr_sa_add(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_sa_info *info)
{
	struct nss_ipsecmgr_ref *sa_ref, *child_ref;
	struct nss_ipsecmgr_sa_entry *sa;

	BUG_ON(!info->child_alloc);
	BUG_ON(!info->child_lookup);

	/*
	 * lock database
	 */
	write_lock_bh(&ipsecmgr_ctx->lock);

	/*
	 * allocate a flow, this returns either a new flow or an existing
	 * one incase it is found
	 */
	child_ref = info->child_alloc(priv, &info->child_key);
	if (!child_ref) {
		/*
		 * unlock device
		 */
		write_unlock_bh(&ipsecmgr_ctx->lock);

		nss_ipsecmgr_warn("%px:failed to alloc child_entry\n", priv->dev);
		nss_ipsecmgr_trace("%px:child_alloc(%px)\n", priv, info->child_alloc);
		return false;
	}

	/*
	 * allocate a SA, when flow alloc is successful. This returns either
	 * new SA or an existing one incase it is found
	 */
	sa_ref = nss_ipsecmgr_sa_alloc(priv, &info->sa_key);
	if (!sa_ref) {
		/*
		 * release the flow and unlock device
		 */
		nss_ipsecmgr_ref_free(priv, child_ref);
		write_unlock_bh(&ipsecmgr_ctx->lock);

		nss_ipsecmgr_warn("%px:failed to alloc sa_entry\n", priv->dev);
		return false;
	}

	/*
	 * we are only interested the storing the SA portion of the message
	 */
	sa = container_of(sa_ref, struct nss_ipsecmgr_sa_entry, ref);
	sa->ifnum = info->nim.cm.interface;
	sa->fail_hash_thresh = info->fail_hash_thresh;
	sa->sa_overhead = info->sa_overhead;

	/*
	 * Set outer dst entry when overhead is non zero
	 */
	if (info->sa_overhead)
		atomic_set(&priv->outer_dst_mtu, info->dst_mtu);

	memcpy(&sa->nim, &info->nim, sizeof(struct nss_ipsec_msg));
	memset(&sa->nim.tuple, 0, sizeof(struct nss_ipsec_tuple));

	/*
	 * Store the SA information to update user for stats reporting
	 */
	memcpy(&sa->sa_info, info->sa, sizeof(struct nss_ipsecmgr_sa));

	/*
	 * clear the stats information
	 */
	memset(&sa->pkts, 0, sizeof(struct nss_ipsecmgr_sa_pkt_stats));

	/*
	 * add child to parent
	 */
	nss_ipsecmgr_ref_add(child_ref, sa_ref);

	/*
	 * Trigger the notification chain for the child
	 * Note: if there is change in any data then the trigger
	 * will update the NSS for the change
	 */
	nss_ipsecmgr_ref_update(priv, child_ref, &info->nim);

	write_unlock_bh(&ipsecmgr_ctx->lock);
	return true;
}

/*
 * nss_ipsecmgr_sa_alloc()
 * 	allocate the SA if there is none in the DB
 */
struct nss_ipsecmgr_ref *nss_ipsecmgr_sa_alloc(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_key *key)
{
	struct nss_ipsecmgr_sa_entry *sa;
	struct nss_ipsecmgr_sa_db *db;
	struct nss_ipsecmgr_ref *ref;
	int idx;

	/*
	 * Search the object in the database first
	 */
	ref = nss_ipsecmgr_sa_lookup(key);
	if (ref) {
		return ref;
	}

	/*
	 * Object doesn't exist, allocate it
	 */
	sa = kzalloc(sizeof(struct nss_ipsecmgr_sa_entry), GFP_ATOMIC);
	if (!sa) {
		nss_ipsecmgr_info("failed to alloc sa_entry\n");
		return NULL;
	}

	/*
	 * store tunnel private reference
	 */
	sa->priv = priv;

	/*
	 * initialize sa list node
	 */
	ref = &sa->ref;
	db = &ipsecmgr_ctx->sa_db;
	INIT_LIST_HEAD(&sa->node);

	/*
	 * update key
	 */
	idx = nss_ipsecmgr_key_data2idx(key, NSS_CRYPTO_MAX_IDXS);

	memcpy(&sa->key, key, sizeof(struct nss_ipsecmgr_key));
	list_add(&sa->node, &db->entries[idx]);

	atomic_inc(&db->num_entries);

	/*
	 * initiallize the reference object
	 */
	nss_ipsecmgr_ref_init(&sa->ref, NULL, nss_ipsecmgr_sa_free);
	nss_ipsecmgr_ref_set_overhead(&sa->ref, nss_ipsecmgr_sa_overhead);

	return ref;
}

/*
 * nss_ipsecmgr_sa_copy()
 * 	update the SA entry with the SA data
 */
void nss_ipsecmgr_copy_v4_sa(struct nss_ipsec_msg *nim, struct nss_ipsecmgr_sa_v4 *sa)
{
	struct nss_ipsec_rule_oip *oip = &nim->msg.rule.oip;

	oip->dst_addr[0] = sa->dst_ip;
	oip->src_addr[0] = sa->src_ip;
	oip->ttl_hop_limit = sa->ttl;
	oip->esp_spi = sa->spi_index;
	oip->ip_ver = IPVERSION;
	oip->dst_port = 0;
	oip->src_port = 0;
	oip->proto_next_hdr = 0;
}

/*
 * nss_ipsecmgr_copy_v6_sa()
 * 	update the SA entry with the SA data
 */
void nss_ipsecmgr_copy_v6_sa(struct nss_ipsec_msg *nim, struct nss_ipsecmgr_sa_v6 *sa)
{
	struct nss_ipsec_rule_oip *oip = &nim->msg.rule.oip;

	/*
	 * copy outer header
	 */
	memcpy(oip->dst_addr, sa->dst_ip, sizeof(uint32_t) * 4);
	memcpy(oip->src_addr, sa->src_ip, sizeof(uint32_t) * 4);

	oip->esp_spi = sa->spi_index;
	oip->ttl_hop_limit = sa->hop_limit;
	oip->ip_ver = 6;
	oip->dst_port = 0;
	oip->src_port = 0;
	oip->proto_next_hdr = 0;
}

/*
 * nss_ipsecmgr_sa_copy()
 * 	update the SA entry with the SA data
 */
void nss_ipsecmgr_copy_sa_data(struct nss_ipsec_msg *nim, struct nss_ipsecmgr_sa_data *sa_data)
{
	struct nss_ipsec_rule_data *data = &nim->msg.rule.data;

	data->crypto_index = (uint16_t)sa_data->crypto_index;

	data->window_size = sa_data->esp.replay_win;
	data->enable_esn = sa_data->enable_esn;

	data->nat_t_req = sa_data->esp.nat_t_req;

	data->cipher_blk_len = nss_crypto_get_cipher_block_len(data->crypto_index);
	data->iv_len = nss_crypto_get_iv_len(data->crypto_index);

	data->esp_icv_len = sa_data->esp.icv_len;
	data->esp_seq_skip = sa_data->esp.seq_skip;
	data->esp_tail_skip = sa_data->esp.trailer_skip;
	data->use_pattern = sa_data->use_pattern;
	data->dscp = sa_data->esp.dscp;
	data->df = !!sa_data->esp.df;
	data->copy_dscp = !!sa_data->esp.dscp_copy;
	data->copy_df = !!sa_data->esp.df_copy;
}

/*
 * nss_ipsecmgr_v4_sa2key()
 * 	convert a SA into a key
 */
void nss_ipsecmgr_v4_sa2key(struct nss_ipsecmgr_sa_v4 *sa, struct nss_ipsecmgr_key *key)
{
	nss_ipsecmgr_key_reset(key);

	nss_ipsecmgr_key_write_8(key, 4 /* v4 */, NSS_IPSECMGR_KEY_POS_IP_VER);
	nss_ipsecmgr_key_write_8(key, IPPROTO_ESP, NSS_IPSECMGR_KEY_POS_IP_PROTO);
	nss_ipsecmgr_key_write_32(key, sa->dst_ip, NSS_IPSECMGR_KEY_POS_IPV4_DST);
	nss_ipsecmgr_key_write_32(key, sa->src_ip, NSS_IPSECMGR_KEY_POS_IPV4_SRC);
	nss_ipsecmgr_key_write_32(key, sa->spi_index, NSS_IPSECMGR_KEY_POS_IPV4_ESP_SPI);

	key->len = NSS_IPSECMGR_KEY_LEN_IPV4_SA;
}

/*
 * nss_ipsecmgr_sa_tuple2key()
 * 	convert a SA into a key
 */
void nss_ipsecmgr_sa_tuple2key(struct nss_ipsec_tuple *tuple, struct nss_ipsecmgr_key *key)
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

		key->len = NSS_IPSECMGR_KEY_LEN_IPV4_SA;
		break;

	case 6:
		nss_ipsecmgr_key_write_8(key, 6 /* v6 */, NSS_IPSECMGR_KEY_POS_IP_VER);
		nss_ipsecmgr_key_write_8(key, IPPROTO_ESP, NSS_IPSECMGR_KEY_POS_IP_PROTO);

		nss_ipsecmgr_v6addr_swap(tuple->dst_addr, tuple->dst_addr);
		nss_ipsecmgr_v6addr_swap(tuple->src_addr, tuple->src_addr);

		for (i  = 0; i < 4; i++) {
			nss_ipsecmgr_key_write_32(key, tuple->dst_addr[i], NSS_IPSECMGR_KEY_POS_IPV6_DST + (i * 32));
			nss_ipsecmgr_key_write_32(key, tuple->src_addr[i], NSS_IPSECMGR_KEY_POS_IPV6_SRC + (i * 32));
		}

		nss_ipsecmgr_key_write_32(key, tuple->esp_spi, NSS_IPSECMGR_KEY_POS_IPV6_ESP_SPI);
		key->len = NSS_IPSECMGR_KEY_LEN_IPV6_SA;
		break;
	}
}

/*
 * nss_ipsecmgr_v6_sa2key()
 * 	convert a SA into a key
 */
void nss_ipsecmgr_v6_sa2key(struct nss_ipsecmgr_sa_v6 *sa, struct nss_ipsecmgr_key *key)
{
	uint32_t i;

	nss_ipsecmgr_key_reset(key);

	nss_ipsecmgr_key_write_8(key, 6 /* v6 */, NSS_IPSECMGR_KEY_POS_IP_VER);
	nss_ipsecmgr_key_write_8(key, IPPROTO_ESP, NSS_IPSECMGR_KEY_POS_IP_PROTO);

	for (i  = 0; i < 4; i++) {
		nss_ipsecmgr_key_write_32(key, sa->dst_ip[i], NSS_IPSECMGR_KEY_POS_IPV6_DST + (i * 32));
		nss_ipsecmgr_key_write_32(key, sa->src_ip[i], NSS_IPSECMGR_KEY_POS_IPV6_SRC + (i * 32));
	}

	nss_ipsecmgr_key_write_32(key, sa->spi_index, NSS_IPSECMGR_KEY_POS_IPV6_ESP_SPI);

	key->len = NSS_IPSECMGR_KEY_LEN_IPV6_SA;
}

/*
 * nss_ipsecmgr_sa_stats_update()
 * 	Update sa stats locally
 */
void nss_ipsecmgr_sa_stats_update(struct nss_ipsec_msg *nim, struct nss_ipsecmgr_sa_entry *sa)
{
	struct nss_ipsecmgr_sa_pkt_stats *stats;
	struct nss_ipsec_sa_stats *sa_stats;
	struct nss_ipsec_sa_stats *pkts;

	pkts = &nim->msg.stats.sa;

	sa_stats = &nim->msg.stats.sa;
	stats = &sa->pkts;

	/*
	 * update SA specific stats
	 */
	stats->count += pkts->count;
	stats->bytes += pkts->bytes;

	stats->no_headroom += pkts->no_headroom;
	stats->no_tailroom += pkts->no_tailroom;
	stats->no_buf += pkts->no_resource;

	stats->fail_queue += pkts->fail_queue;
	stats->fail_hash += pkts->fail_hash;
	stats->fail_replay += pkts->fail_replay;

	sa->esn_enabled = sa_stats->esn_enabled;

	sa->pkts.seq_num = sa_stats->seq_num;
	sa->pkts.window_max = sa_stats->window_max;
	sa->pkts.window_size = sa_stats->window_size;
}

/*
 * nss_ipsecmgr_sa_lookup()
 * 	lookup the SA in the sa_db
 */
struct nss_ipsecmgr_ref *nss_ipsecmgr_sa_lookup(struct nss_ipsecmgr_key *key)
{
	struct nss_ipsecmgr_sa_db *db = &ipsecmgr_ctx->sa_db;
	struct nss_ipsecmgr_sa_entry *entry;
	struct list_head *head;
	int idx;

	idx = nss_ipsecmgr_key_data2idx(key, NSS_CRYPTO_MAX_IDXS);
	head = &db->entries[idx];

	list_for_each_entry(entry, head, node) {
		if (nss_ipsecmgr_key_cmp(&entry->key, key)) {
			return &entry->ref;
		}
	}

	return NULL;
}

/*
 * nss_ipsecmgr_sa_flush_all()
 * 	remove all SA and its corresponding references
 */
void nss_ipsecmgr_sa_flush_all(struct nss_ipsecmgr_priv *priv)
{
	struct nss_ipsecmgr_sa_db *sa_db = &ipsecmgr_ctx->sa_db;
	struct nss_ipsecmgr_sa_entry *entry;
	struct nss_ipsecmgr_sa_entry *tmp;
	int ifindex  = priv->dev->ifindex;
	struct list_head *head;
	int i;

	/*
	 * lock database
	 */
	write_lock_bh(&ipsecmgr_ctx->lock);

	/*
	 * walk the SA database for each entry and delete the attached SA.
	 * Assumption is that single SA cannot be associated to multiple ipsectunX interfaces.
	 */
	for (i = 0, head = sa_db->entries; i < NSS_IPSECMGR_MAX_SA; i++, head++) {
		list_for_each_entry_safe(entry, tmp, head, node) {
			if (entry->nim.tunnel_id == ifindex) {
				nss_ipsecmgr_ref_free(priv, &entry->ref);
			}
		}
	}

	/*
	 * unlock database
	 */
	write_unlock_bh(&ipsecmgr_ctx->lock);
}

/*
 * nss_ipsecmgr_encap_add()
 * 	add encap flow/subnet to an existing or new SA
 */
bool nss_ipsecmgr_encap_add(struct net_device *tun, struct nss_ipsecmgr_encap_flow *flow,
				struct nss_ipsecmgr_sa *sa, struct nss_ipsecmgr_sa_data *data)
{
	struct nss_ipsecmgr_priv *priv = netdev_priv(tun);
	struct nss_ipsecmgr_sa_info info = { {{0} } };
	struct dst_entry *dst;
	struct flowi6 fl6;

	nss_ipsecmgr_info("%px:encap_add initiated\n", tun);

	info.sa_overhead = sizeof(struct ip_esp_hdr);
	info.sa_overhead += NSS_IPSECMGR_ESP_PAD_SZ;
	info.sa_overhead += NSS_IPSECMGR_ESP_TRAIL_SZ;
	info.sa_overhead += ETH_HLEN;

	nss_ipsecmgr_encap_flow_init(&info.nim, NSS_IPSEC_MSG_TYPE_ADD_RULE, priv);

	switch (flow->type) {
	case NSS_IPSECMGR_FLOW_TYPE_V4_TUPLE:
		nss_ipsecmgr_copy_encap_v4_flow(&info.nim, &flow->data.v4_tuple);
		nss_ipsecmgr_encap_v4_flow2key(&flow->data.v4_tuple, &info.child_key);

		info.child_alloc = nss_ipsecmgr_flow_alloc;
		info.child_lookup = nss_ipsecmgr_flow_lookup;
		break;

	case NSS_IPSECMGR_FLOW_TYPE_V4_SUBNET:
		if (nss_ipsecmgr_verify_v4_subnet(&flow->data.v4_subnet)) {
			nss_ipsecmgr_warn("%px:invalid subnet and mask\n", tun);
			return false;
		}

		nss_ipsecmgr_v4_subnet2key(&flow->data.v4_subnet, &info.child_key);

		info.child_alloc = nss_ipsecmgr_subnet_alloc;
		info.child_lookup = nss_ipsecmgr_subnet_lookup;
		break;

	case NSS_IPSECMGR_FLOW_TYPE_V6_TUPLE:
		nss_ipsecmgr_copy_encap_v6_flow(&info.nim, &flow->data.v6_tuple);
		nss_ipsecmgr_encap_v6_flow2key(&flow->data.v6_tuple, &info.child_key);

		info.child_alloc = nss_ipsecmgr_flow_alloc;
		info.child_lookup = nss_ipsecmgr_flow_lookup;
		break;

	case NSS_IPSECMGR_FLOW_TYPE_V6_SUBNET:
		if (nss_ipsecmgr_verify_v6_subnet(&flow->data.v6_subnet)) {
			nss_ipsecmgr_warn("%px:invalid subnet and mask\n", tun);
			return false;
		}

		nss_ipsecmgr_v6_subnet2key(&flow->data.v6_subnet, &info.child_key);

		info.child_alloc = nss_ipsecmgr_subnet_alloc;
		info.child_lookup = nss_ipsecmgr_subnet_lookup;
		break;

	default:
		nss_ipsecmgr_warn("%px:unknown flow type(%d)\n", tun, flow->type);
		return false;
	}

	switch (sa->type) {
	case NSS_IPSECMGR_SA_TYPE_V4:

		nss_ipsecmgr_copy_v4_sa(&info.nim, &sa->data.v4);
		nss_ipsecmgr_copy_sa_data(&info.nim, data);
		nss_ipsecmgr_v4_sa2key(&sa->data.v4, &info.sa_key);

		info.sa_overhead += sizeof(struct iphdr);
		info.sa_overhead += data->esp.nat_t_req ?
			sizeof(struct udphdr) : 0;

		/*
		 * Update initial value for outer_dst_mtu
		 * Further this value will be updated as per the PMTU error
		 * generated for the Tunnel.
		 */
		dst = (struct dst_entry *)ip_route_output(&init_net,
				htonl(sa->data.v4.dst_ip), 0, 0, 0);
		if (IS_ERR(dst)) {
			return false;
		}
		info.dst_mtu = dst_mtu(dst);
		dst_release(dst);
		break;

	case NSS_IPSECMGR_SA_TYPE_V6:

		nss_ipsecmgr_copy_v6_sa(&info.nim, &sa->data.v6);
		nss_ipsecmgr_copy_sa_data(&info.nim, data);
		nss_ipsecmgr_v6_sa2key(&sa->data.v6, &info.sa_key);

		info.sa_overhead += sizeof(struct ipv6hdr);

		nss_ipsecmgr_v6addr_ntoh((uint32_t *)&sa->data.v6.dst_ip, (uint32_t *)&fl6.daddr);

		/*
		 * Update initial value for outer_dst_mtu
		 * Further this value will be updated as per the PMTU error
		 * generated for the Tunnel.
		 */
		dst = ip6_route_output(&init_net, NULL, &fl6);
		if (IS_ERR(dst)) {
			return false;
		}
		info.dst_mtu = dst_mtu(dst);
		dst_release(dst);
		break;

	default:
		nss_ipsecmgr_warn("%px:unknown sa type(%d)\n", tun, sa->type);
		return false;
	}

	/*
	 * Compute additional overhead for this SA
	 */
	info.sa = sa;
	info.sa_overhead += nss_crypto_get_iv_len(data->crypto_index);
	info.sa_overhead += data->esp.icv_len;
	return nss_ipsecmgr_sa_add(priv, &info);
}
EXPORT_SYMBOL(nss_ipsecmgr_encap_add);

/*
 * nss_ipsecmgr_encap_del()
 * 	del encap flow/subnet to an existing SA
 *
 * Note: if this is the only/last flow or subnet in the SA then
 * the SA will be also be deallocated
 */
bool nss_ipsecmgr_encap_del(struct net_device *tun, struct nss_ipsecmgr_encap_flow *flow, struct nss_ipsecmgr_sa *sa)
{
	struct nss_ipsecmgr_priv *priv = netdev_priv(tun);
	struct nss_ipsecmgr_sa_info info;

	nss_ipsecmgr_info("%px:encap_del initiated\n", tun);

	memset(&info, 0, sizeof(struct nss_ipsecmgr_sa_info));
	nss_ipsecmgr_encap_flow_init(&info.nim, NSS_IPSEC_MSG_TYPE_DEL_RULE, priv);

	switch (flow->type) {
	case NSS_IPSECMGR_FLOW_TYPE_V4_TUPLE:

		nss_ipsecmgr_copy_encap_v4_flow(&info.nim, &flow->data.v4_tuple);
		nss_ipsecmgr_copy_v4_sa(&info.nim, &sa->data.v4);

		nss_ipsecmgr_encap_v4_flow2key(&flow->data.v4_tuple, &info.child_key);
		nss_ipsecmgr_v4_sa2key(&sa->data.v4, &info.sa_key);

		info.child_alloc = nss_ipsecmgr_flow_alloc;
		info.child_lookup = nss_ipsecmgr_flow_lookup;
		break;

	case NSS_IPSECMGR_FLOW_TYPE_V4_SUBNET:

		if (nss_ipsecmgr_verify_v4_subnet(&flow->data.v4_subnet)) {
			nss_ipsecmgr_warn("%px:invalid subnet and mask\n", tun);
			return false;
		}

		nss_ipsecmgr_copy_v4_sa(&info.nim, &sa->data.v4);

		nss_ipsecmgr_v4_subnet2key(&flow->data.v4_subnet, &info.child_key);
		nss_ipsecmgr_v4_sa2key(&sa->data.v4, &info.sa_key);

		info.child_alloc = nss_ipsecmgr_subnet_alloc;
		info.child_lookup = nss_ipsecmgr_subnet_lookup;
		break;

	case NSS_IPSECMGR_FLOW_TYPE_V6_TUPLE:

		nss_ipsecmgr_copy_encap_v6_flow(&info.nim, &flow->data.v6_tuple);
		nss_ipsecmgr_copy_v6_sa(&info.nim, &sa->data.v6);

		nss_ipsecmgr_encap_v6_flow2key(&flow->data.v6_tuple, &info.child_key);
		nss_ipsecmgr_v6_sa2key(&sa->data.v6, &info.sa_key);

		info.child_alloc = nss_ipsecmgr_flow_alloc;
		info.child_lookup = nss_ipsecmgr_flow_lookup;
		break;

	case NSS_IPSECMGR_FLOW_TYPE_V6_SUBNET:

		if (nss_ipsecmgr_verify_v6_subnet(&flow->data.v6_subnet)) {
			nss_ipsecmgr_warn("%px:invalid subnet and mask\n", tun);
			return false;
		}

		nss_ipsecmgr_copy_v6_sa(&info.nim, &sa->data.v6);

		nss_ipsecmgr_v6_subnet2key(&flow->data.v6_subnet, &info.child_key);
		nss_ipsecmgr_v6_sa2key(&sa->data.v6, &info.sa_key);

		info.child_alloc = nss_ipsecmgr_subnet_alloc;
		info.child_lookup = nss_ipsecmgr_subnet_lookup;
		break;

	default:
		nss_ipsecmgr_warn("%px:unknown flow type(%d)\n", tun, flow->type);
		return false;
	}

	return nss_ipsecmgr_sa_del(priv, &info);
}
EXPORT_SYMBOL(nss_ipsecmgr_encap_del);

/*
 * nss_ipsecmgr_decap_add()
 * 	add decap flow/subnet to an existing or new SA
 *
 * Note: In case of decap rule, sa become flow for lookup into flow table
 */
bool nss_ipsecmgr_decap_add(struct net_device *tun, struct nss_ipsecmgr_sa *sa, struct nss_ipsecmgr_sa_data *data)
{
	struct nss_ipsecmgr_priv *priv = netdev_priv(tun);
	struct nss_ipsec_rule *ipsec_rule;
	struct nss_ipsecmgr_sa_info info;

	nss_ipsecmgr_info("%px:decap_add initiated\n", tun);

	memset(&info, 0, sizeof(struct nss_ipsecmgr_sa_info));
	nss_ipsecmgr_decap_flow_init(&info.nim, NSS_IPSEC_MSG_TYPE_ADD_RULE, priv);

	switch (sa->type) {
	case NSS_IPSECMGR_SA_TYPE_V4:
		nss_ipsecmgr_copy_decap_v4_flow(&info.nim, &sa->data.v4);
		nss_ipsecmgr_copy_v4_sa(&info.nim, &sa->data.v4);
		nss_ipsecmgr_copy_sa_data(&info.nim, data);

		/*
		 * if NATT is set override the protocol and port numbers
		 */
		ipsec_rule = &info.nim.msg.rule;
		if (ipsec_rule->data.nat_t_req) {
			info.nim.tuple.proto_next_hdr = IPPROTO_UDP;
			info.nim.tuple.dst_port = NSS_IPSECMGR_NATT_PORT_DATA;
			info.nim.tuple.src_port = NSS_IPSECMGR_NATT_PORT_DATA;
		}

		nss_ipsecmgr_decap_v4_flow2key(&sa->data.v4, &info.child_key);
		nss_ipsecmgr_v4_sa2key(&sa->data.v4, &info.sa_key);
		break;

	case NSS_IPSECMGR_SA_TYPE_V6:
		nss_ipsecmgr_copy_decap_v6_flow(&info.nim, &sa->data.v6);
		nss_ipsecmgr_copy_v6_sa(&info.nim, &sa->data.v6);
		nss_ipsecmgr_copy_sa_data(&info.nim, data);

		nss_ipsecmgr_decap_v6_flow2key(&sa->data.v6, &info.child_key);
		nss_ipsecmgr_v6_sa2key(&sa->data.v6, &info.sa_key);
		break;

	default:
		nss_ipsecmgr_warn("%px:unknown flow type(%d)\n", tun, sa->type);
		return false;
	}

	/*
	 * Store the fail_hash_threshold in the info
	 */
	info.sa = sa;
	info.sa_overhead = 0;
	info.fail_hash_thresh  = data->fail_hash_thresh;
	info.child_alloc = nss_ipsecmgr_flow_alloc;
	info.child_lookup = nss_ipsecmgr_flow_lookup;

	return nss_ipsecmgr_sa_add(priv, &info);
}
EXPORT_SYMBOL(nss_ipsecmgr_decap_add);

/*
 * nss_ipsecmgr_sa_flush()
 * 	flush sa and all associated references.
 */
bool nss_ipsecmgr_sa_flush(struct net_device *tun, struct nss_ipsecmgr_sa *sa)
{
	struct nss_ipsecmgr_priv *priv = netdev_priv(tun);
	struct nss_ipsecmgr_key sa_key;
	struct nss_ipsecmgr_ref *sa_ref;

	switch (sa->type) {
	case NSS_IPSECMGR_SA_TYPE_V4:
		nss_ipsecmgr_v4_sa2key(&sa->data.v4, &sa_key);
		break;

	case NSS_IPSECMGR_SA_TYPE_V6:
		nss_ipsecmgr_v6_sa2key(&sa->data.v6, &sa_key);
		break;

	default:
		nss_ipsecmgr_warn("%px:Unsupported sa type (type - %d)\n", tun, sa->type);
		return false;
	}

	/*
	 * lock database
	 */
	write_lock_bh(&ipsecmgr_ctx->lock);

	/*
	 * search the SA in sa_db
	 */
	sa_ref = nss_ipsecmgr_sa_lookup(&sa_key);
	if (!sa_ref) {
		write_unlock_bh(&ipsecmgr_ctx->lock);
		nss_ipsecmgr_warn("%px:failed to lookup SA\n", priv);
		return false;
	}

	/*
	 * remove the reference from its associated SA
	 */
	nss_ipsecmgr_ref_free(priv, sa_ref);

	/*
	 * unlock database
	 */
	write_unlock_bh(&ipsecmgr_ctx->lock);

	return true;
}
EXPORT_SYMBOL(nss_ipsecmgr_sa_flush);
