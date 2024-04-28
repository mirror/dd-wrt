/*
 **************************************************************************
 * Copyright (c) 2016-2017, The Linux Foundation. All rights reserved.
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */

/* nss_ipsecmgr_subnet.c
 *	NSS IPsec manager subnet rules
 */

#include <linux/list.h>
#include <linux/hashtable.h>
#include <linux/types.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <net/ipv6.h>
#include <linux/version.h>
#include <linux/debugfs.h>
#include <linux/vmalloc.h>

#include <nss_ipsecmgr.h>

#include "nss_ipsecmgr_priv.h"

extern struct nss_ipsecmgr_drv *ipsecmgr_ctx;

/*
 * nss_ipsecmgr_subnet_key_data2idx()
 * 	subnet specific api for converting word stream to index
 */
static uint32_t nss_ipsecmgr_subnet_key_data2idx(struct nss_ipsecmgr_key *key, const uint32_t table_sz)
{
	struct nss_ipsecmgr_key tmp_key;

	/*
	 * The subnet table is keyed without the protocol. This allows us to support
	 * "any" protocol configuration
	 */

	memcpy(&tmp_key, key, sizeof(struct nss_ipsecmgr_key));
	nss_ipsecmgr_key_clear_8(&tmp_key, NSS_IPSECMGR_KEY_POS_IP_PROTO);

	return nss_ipsecmgr_key_data2idx(&tmp_key, table_sz);
}

/*
 * nss_ipsecmgr_netmask_is_default()
 * 	confirm if key is for default netmask.
 */
static inline bool nss_ipsecmgr_netmask_is_default(struct nss_ipsecmgr_key *key)
{
	uint8_t ip_ver;
	uint64_t mask64_low;
	uint64_t mask64_high;

	ip_ver = nss_ipsecmgr_key_read_8(key, NSS_IPSECMGR_KEY_POS_IP_VER);
	if (likely(ip_ver == 4)) {
		return !nss_ipsecmgr_key_read_mask32(key, NSS_IPSECMGR_KEY_POS_IPV4_DST);
	}

	mask64_low = nss_ipsecmgr_key_read_mask64(key, NSS_IPSECMGR_KEY_POS_IPV6_DST);
	mask64_high = nss_ipsecmgr_key_read_mask64(key, NSS_IPSECMGR_KEY_POS_IPV6_DST + 64);

	return !(mask64_low || mask64_high);
}

/*
 * nss_ipsecmgr_netmask2idx()
 * 	Get the index of the netmask array from key.
 */
static inline uint32_t nss_ipsecmgr_netmask2idx(struct nss_ipsecmgr_key *key)
{
	uint32_t mask32;
	uint64_t mask64_low;
	uint64_t mask64_high;
	uint8_t ip_ver;

	ip_ver = nss_ipsecmgr_key_read_8(key, NSS_IPSECMGR_KEY_POS_IP_VER);
	if (likely(ip_ver == 4)) {
		mask32  = nss_ipsecmgr_key_read_mask32(key, NSS_IPSECMGR_KEY_POS_IPV4_DST);
		return ffs(mask32) - 1;
	}

	BUG_ON(ip_ver != 6);

	mask64_low = nss_ipsecmgr_key_read_mask64(key, NSS_IPSECMGR_KEY_POS_IPV6_DST);
	if (mask64_low) {
		return __ffs64(mask64_low);
	}

	mask64_high = nss_ipsecmgr_key_read_mask64(key, NSS_IPSECMGR_KEY_POS_IPV6_DST + 64);
	if (mask64_high) {
		return __ffs64(mask64_high) + 64;
	}

	return NSS_IPSECMGR_MAX_NETMASK;
}

/*
 * nss_ipsecmgr_netmask_free()
 * 	deallocate a netmask entry
 */
static bool nss_ipsecmgr_netmask_free(struct nss_ipsecmgr_netmask_db *db, struct nss_ipsecmgr_key *key)
{
	struct nss_ipsecmgr_netmask_entry *entry;
	bool is_default;
	uint32_t idx;

	is_default = nss_ipsecmgr_netmask_is_default(key);
	entry = db->default_entry;

	if (is_default && !entry) { /* default with no entry */
		return false;
	} else if (is_default && --entry->count) { /* default but more than 1 entry */
		return false;
	} else if (is_default) { /* default but last entry */
		db->default_entry = NULL;
		goto free;
	}

	/*
	 * !default
	 */
	idx = nss_ipsecmgr_netmask2idx(key);
	if (idx >= NSS_IPSECMGR_MAX_NETMASK) {
		return false;
	}

	entry = db->entries[idx];
	BUG_ON(!entry);

	if (--entry->count) {
		return false;
	}

	clear_bit(idx, db->bitmap);
	db->entries[idx] = NULL;

free:
	kfree(entry);
	return true;
}

/*
 * nss_ipsecmgr_subnet_dump()
 *	Dump single subnet stats
 */
static ssize_t nss_ipsecmgr_subnet_dump(struct nss_ipsecmgr_key *key, struct nss_ipsec_msg *nim, char *buf, ssize_t max_len)
{
	uint32_t subnet[4] = {0}, mask[4] = {0};
	ssize_t len = 0;
	uint8_t ip_ver;
	uint8_t proto;

	proto = nss_ipsecmgr_key_read_8(key, NSS_IPSECMGR_KEY_POS_IP_PROTO);
	ip_ver = nss_ipsecmgr_key_read_8(key, NSS_IPSECMGR_KEY_POS_IP_VER);

	switch (ip_ver) {
	case 4:
		nss_ipsecmgr_key_read(key, subnet, mask, NSS_IPSECMGR_KEY_POS_IPV4_DST, 1);
		len += snprintf(buf + len, max_len - len, "dst_ip: %pI4h\n", subnet);
		len += snprintf(buf + len, max_len - len, "dst_mask: %pI4h\n", mask);
		break;

	case 6:
		/*
		 * The subnet and mask bits from the key are read to the upper words of the array
		 * later converted to network order for display.
		 */
		nss_ipsecmgr_key_read(key, &subnet[0], &mask[0], NSS_IPSECMGR_KEY_POS_IPV6_DST, 4);
		nss_ipsecmgr_v6addr_hton(subnet, subnet);
		nss_ipsecmgr_v6addr_hton(mask, mask);

		len += snprintf(buf + len, max_len - len, "dst_ip: %pI6c\n", subnet);
		len += snprintf(buf + len, max_len - len, "dst_mask: %pI6c\n", mask);
		break;

	}

	len += snprintf(buf + len, max_len - len, "proto: %d\n", proto);
	return len;
}

/*
 * nss_ipsecmgr_subnet_read_stats()
 * 	retreive subnet stats
 */
static ssize_t nss_ipsecmgr_subnet_read_stats(struct nss_ipsecmgr_netmask_entry *netmask, char *buf, uint32_t max_len)
{
	struct nss_ipsecmgr_subnet_entry *entry;
	struct list_head *head;
	ssize_t len = 0;
	int idx;

	/*
	 * Check if we have valid entries in the netmask db
	 */
	if (!netmask || (netmask->count == 0)) {
		return 0;
	}

	head = &netmask->subnets[0];
	for (idx = NSS_IPSECMGR_MAX_SUBNET; (max_len > 0) && idx--; head++) {
		list_for_each_entry(entry, head, node) {
			if (unlikely(max_len <= 0)) {
				break;
			}

			len += nss_ipsecmgr_subnet_dump(&entry->key, &entry->nim, buf + len, max_len);
			max_len = max_len - len;
		}
	}

	return len;
}

/*
 * nss_ipsecmgr_netmask_stats_read()
 * 	read subnet statistics
 */
ssize_t nss_ipsecmgr_netmask_stats_read(struct file *fp, char __user *ubuf, size_t sz, loff_t *ppos)
{
	struct nss_ipsecmgr_netmask_db *net_db;
	ssize_t len, max_len;
	uint32_t num_entries;
	ssize_t ret;
	char *buf;
	int i;

	net_db = &ipsecmgr_ctx->net_db;

	num_entries = atomic_read(&net_db->num_entries);
	if (!num_entries) {
		return 0;
	}

	max_len = num_entries * NSS_IPSECMGR_SUBNET_STATS_SIZE;

	buf = vzalloc(max_len);
	if (!buf) {
		nss_ipsecmgr_error("unable to allocate local buffer for subnet stats\n");
		return 0;
	}

	/*
	 * Take the read lock.
	 */
	read_lock_bh(&ipsecmgr_ctx->lock);

	/*
	 * retreive the default subnet entry stats
	 */
	len = nss_ipsecmgr_subnet_read_stats(net_db->default_entry, buf, max_len);

	/*
	 * retreive the netmask entry db
	 */
	for (i = 0; i < NSS_IPSECMGR_MAX_NETMASK; i++) {
		len += nss_ipsecmgr_subnet_read_stats(net_db->entries[i], buf + len, max_len - len);
	}

	read_unlock_bh(&ipsecmgr_ctx->lock);

	ret = simple_read_from_buffer(ubuf, sz, ppos, buf, len);
	vfree(buf);
	return ret;
}

/*
 * nss_ipsecmgr_subnet_update()
 * 	update the subnet with its associated data
 */
static void nss_ipsecmgr_subnet_update(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_ref *ref, struct nss_ipsec_msg *nim)
{
	struct nss_ipsecmgr_subnet_entry *subnet;

	subnet = container_of(ref, struct nss_ipsecmgr_subnet_entry, ref);

	memcpy(&subnet->nim, nim, sizeof(struct nss_ipsec_msg));
}

/*
 * nss_ipsecmgr_subnet_free()
 * 	free the associated subnet entry and notify NSS
 */
static void nss_ipsecmgr_subnet_free(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_ref *ref)
{
	struct nss_ipsecmgr_subnet_entry *subnet;
	struct nss_ipsecmgr_netmask_db *db = &ipsecmgr_ctx->net_db;

	subnet = container_of(ref, struct nss_ipsecmgr_subnet_entry, ref);

	BUG_ON(nss_ipsecmgr_ref_is_empty(ref) == false);

	/*
	 * detach it from the netmask entry database and
	 * check if the netmask entry is empty. The netmask
	 * entry will get freed if there are no further entries
	 * available
	 */
	list_del(&subnet->node);
	atomic_dec(&db->num_entries);

	nss_ipsecmgr_netmask_free(&ipsecmgr_ctx->net_db, &subnet->key);
	kfree(subnet);
}

/*
 * nss_ipsecmgr_netmask_lookup()
 * 	lookup a netmask entry
 */
static inline struct nss_ipsecmgr_netmask_entry *nss_ipsecmgr_netmask_lookup(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_key *key)
{
	struct nss_ipsecmgr_netmask_db *db = &ipsecmgr_ctx->net_db;
	uint32_t idx;

	if (nss_ipsecmgr_netmask_is_default(key)) {
		return db->default_entry;
	}

	idx = nss_ipsecmgr_netmask2idx(key);
	return (idx >= NSS_IPSECMGR_MAX_NETMASK) ? NULL : db->entries[idx];
}

/*
 * nss_ipsecmgr_netmask_alloc()
 * 	allocate a netmask entry
 */
static struct nss_ipsecmgr_netmask_entry *nss_ipsecmgr_netmask_alloc(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_key *key)
{
	struct nss_ipsecmgr_netmask_db *db = &ipsecmgr_ctx->net_db;
	struct nss_ipsecmgr_netmask_entry *entry;
	uint32_t idx;

	entry = nss_ipsecmgr_netmask_lookup(priv, key);
	if (entry) {
		return entry;
	}

	entry = kzalloc(sizeof(struct nss_ipsecmgr_netmask_entry), GFP_ATOMIC);
	if (!entry) {
		return NULL;
	}

	nss_ipsecmgr_init_subnet_db(entry);
	entry->count = 1;

	if (nss_ipsecmgr_netmask_is_default(key)) {
		db->default_entry = entry;
		return entry;
	}

	idx = nss_ipsecmgr_netmask2idx(key);
	if (idx >= NSS_IPSECMGR_MAX_NETMASK) {
		kfree(entry);
		return NULL;
	}

	entry->mask_bits = NSS_IPSECMGR_MAX_NETMASK - idx;
	set_bit(idx, db->bitmap);
	db->entries[idx] = entry;

	return entry;
}


/*
 * nss_ipsecmgr_copy_subnet()
 * 	copy subnet nim
 */
void nss_ipsecmgr_copy_subnet(struct nss_ipsec_msg *nim, struct nss_ipsecmgr_ref *ref)
{
	struct nss_ipsecmgr_subnet_entry *entry;
	struct nss_ipsec_rule_data *data;
	struct nss_ipsec_rule_oip *oip;

	entry = container_of(ref, struct nss_ipsecmgr_subnet_entry, ref);

	oip = &entry->nim.msg.rule.oip;
	data = &entry->nim.msg.rule.data;

	memcpy(&nim->msg.rule.oip, oip, sizeof(struct nss_ipsec_rule_oip));
	memcpy(&nim->msg.rule.data, data, sizeof(struct nss_ipsec_rule_data));
}

/*
 * nss_ipsecmgr_v4_subnet_sel2key()
 * 	convert subnet selector to key
 */
void nss_ipsecmgr_v4_subnet_tuple2key(struct nss_ipsec_tuple *tuple, struct nss_ipsecmgr_key *key)
{
	nss_ipsecmgr_key_reset(key);

	nss_ipsecmgr_key_write_8(key, 4 /* ipv4 */, NSS_IPSECMGR_KEY_POS_IP_VER);
	nss_ipsecmgr_key_write_8(key, tuple->proto_next_hdr, NSS_IPSECMGR_KEY_POS_IP_PROTO);
	nss_ipsecmgr_key_write_32(key, nss_ipsecmgr_get_v4addr(tuple->dst_addr), NSS_IPSECMGR_KEY_POS_IPV4_DST);

	key->len = NSS_IPSECMGR_KEY_LEN_IPV4_SUBNET;
}

/*
 * nss_ipsecmgr_v6_subnet_sel2key()
 * 	convert subnet selector to key
 */
void nss_ipsecmgr_v6_subnet_tuple2key(struct nss_ipsec_tuple *tuple, struct nss_ipsecmgr_key *key)
{
	uint32_t pos;
	uint32_t i;

	nss_ipsecmgr_key_reset(key);

	nss_ipsecmgr_key_write_8(key, 6 /* ipv6 */, NSS_IPSECMGR_KEY_POS_IP_VER);
	nss_ipsecmgr_key_write_8(key, tuple->proto_next_hdr, NSS_IPSECMGR_KEY_POS_IP_PROTO);

	for (i = 0; i < 4; i++) {
		pos = NSS_IPSECMGR_KEY_POS_IPV6_DST + (i * NSS_IPSECMGR_BITS_PER_WORD);
		nss_ipsecmgr_key_write_32(key, tuple->dst_addr[i], pos);
	}

	key->len = NSS_IPSECMGR_KEY_LEN_IPV6_SUBNET;
}


/*
 * nss_ipsecmgr_v4_subnet2key()
 *      convert an v4 subnet into a key
 */
void nss_ipsecmgr_v4_subnet2key(struct nss_ipsecmgr_encap_v4_subnet *net, struct nss_ipsecmgr_key *key)
{
	nss_ipsecmgr_key_reset(key);

	nss_ipsecmgr_key_write_8(key, 4 /* ipv4 */, NSS_IPSECMGR_KEY_POS_IP_VER);
	nss_ipsecmgr_key_write_8(key, (uint8_t)net->protocol, NSS_IPSECMGR_KEY_POS_IP_PROTO);
	nss_ipsecmgr_key_write(key, &net->dst_subnet, &net->dst_mask, NSS_IPSECMGR_KEY_POS_IPV4_DST, 1);

	/*
	 * clear mask if caller specify protocol as any (0xff).
	 * this will serve as default entry for any-protocol.
	 */
	if (net->protocol == NSS_IPSECMGR_PROTO_NEXT_HDR_ANY) {
		nss_ipsecmgr_key_clear_8(key, NSS_IPSECMGR_KEY_POS_IP_PROTO);
	}

	key->len = NSS_IPSECMGR_KEY_LEN_IPV4_SUBNET;
}

/*
 * nss_ipsecmgr_v6_subnet2key()
 *      convert an v6 subnet into a key
 */
void nss_ipsecmgr_v6_subnet2key(struct nss_ipsecmgr_encap_v6_subnet *net, struct nss_ipsecmgr_key *key)
{
	nss_ipsecmgr_key_reset(key);

	nss_ipsecmgr_key_write_8(key, 6 /* ipv6 */, NSS_IPSECMGR_KEY_POS_IP_VER);
	nss_ipsecmgr_key_write_8(key, (uint8_t)net->next_hdr, NSS_IPSECMGR_KEY_POS_IP_PROTO);
	nss_ipsecmgr_key_write(key, net->dst_subnet, net->dst_mask, NSS_IPSECMGR_KEY_POS_IPV6_DST, 4);

	/*
	 * clear mask if caller specify protocol as any (0xff).
	 * this will serve as default entry for any-protocol.
	 */
	if (net->next_hdr == NSS_IPSECMGR_PROTO_NEXT_HDR_ANY) {
		nss_ipsecmgr_key_clear_8(key, NSS_IPSECMGR_KEY_POS_IP_PROTO);
	}

	key->len = NSS_IPSECMGR_KEY_LEN_IPV6_SUBNET;
}


/*
 * nss_ipsecmgr_v4_subnet_match()
 * 	peform a v4 subnet based match in netmask database
 */
struct nss_ipsecmgr_ref *nss_ipsecmgr_v4_subnet_match(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_key *key)
{
	struct nss_ipsecmgr_netmask_db *db = &ipsecmgr_ctx->net_db;
	struct nss_ipsecmgr_key tmp_key;
	struct nss_ipsecmgr_ref *ref;
	int i;

	/*
	 * cycle through the bitmap for each subnet
	 */
	for_each_set_bit(i, db->bitmap, 32) {

		BUG_ON(db->entries[i] == NULL);
		BUG_ON(db->entries[i]->count == 0);

		memcpy(&tmp_key, key, sizeof(struct nss_ipsecmgr_key));

		/*
		 * set the key with the right mask for hash index computation;
		 * each subnet index has its associated mask value
		 */
		nss_ipsecmgr_key_lshift_mask(&tmp_key, i, NSS_IPSECMGR_KEY_POS_IPV4_DST);

		ref = nss_ipsecmgr_subnet_lookup(priv, &tmp_key);
		if (ref) {
			return ref;
		}
	}

	memcpy(&tmp_key, key, sizeof(struct nss_ipsecmgr_key));
	/*
	 * normal lookup failed; check default subnet entry
	 * - clear the destination netmask before lookup
	 */
	nss_ipsecmgr_key_clear_32(&tmp_key, NSS_IPSECMGR_KEY_POS_IPV4_DST);

	ref = nss_ipsecmgr_subnet_lookup(priv, &tmp_key);
	if (ref) {
		return ref;
	}

	return NULL;
}

/*
 * nss_ipsecmgr_v6_subnet_match()
 * 	peform a v6 subnet based match in netmask database
 */
struct nss_ipsecmgr_ref *nss_ipsecmgr_v6_subnet_match(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_key *key)
{
	struct nss_ipsecmgr_netmask_db *db = &ipsecmgr_ctx->net_db;
	struct nss_ipsecmgr_key tmp_key;
	struct nss_ipsecmgr_ref *ref;
	int i;

	/*
	 * cycle through the bitmap for each subnet
	 */
	for_each_set_bit(i, db->bitmap, NSS_IPSECMGR_MAX_NETMASK) {

		BUG_ON(db->entries[i] == NULL);
		BUG_ON(db->entries[i]->count == 0);
		memcpy(&tmp_key, key, sizeof(struct nss_ipsecmgr_key));

		nss_ipsecmgr_key_lshift_mask128(&tmp_key, i, NSS_IPSECMGR_KEY_POS_IPV6_DST);

		ref = nss_ipsecmgr_subnet_lookup(priv, &tmp_key);
		if (ref) {
			return ref;
		}
	}

	memcpy(&tmp_key, key, sizeof(struct nss_ipsecmgr_key));

	/*
	 * normal lookup failed; check default subnet entry
	 * - clear the destination netmask before lookup
	 */
	nss_ipsecmgr_key_clear_128(&tmp_key, NSS_IPSECMGR_KEY_POS_IPV6_DST);

	ref = nss_ipsecmgr_subnet_lookup(priv, &tmp_key);
	if (ref) {
		return ref;
	}
	return NULL;
}

/*
 * nss_ipsecmgr_subnet_lookup()
 * 	lookup a subnet entry
 */
struct nss_ipsecmgr_ref *nss_ipsecmgr_subnet_lookup(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_key *key)
{
	struct nss_ipsecmgr_netmask_entry *netmask;
	struct nss_ipsecmgr_subnet_entry *entry;
	struct list_head *head;
	int idx;

	netmask = nss_ipsecmgr_netmask_lookup(priv, key);
	if (!netmask) {
		return NULL;
	}

	BUG_ON(netmask->count == 0);

	idx = nss_ipsecmgr_subnet_key_data2idx(key, NSS_IPSECMGR_MAX_SUBNET);
	head = &netmask->subnets[idx];

	list_for_each_entry(entry, head, node) {
		if (nss_ipsecmgr_key_cmp(&entry->key, key)) {
			return &entry->ref;
		}
	}
	return NULL;
}

/*
 * nss_ipsecmgr_subnet_alloc()
 *      allocate a subnet entry
 */
struct nss_ipsecmgr_ref *nss_ipsecmgr_subnet_alloc(struct nss_ipsecmgr_priv *priv, struct nss_ipsecmgr_key *key)
{
	struct nss_ipsecmgr_netmask_entry *netmask;
	struct nss_ipsecmgr_subnet_entry *subnet;
	struct nss_ipsecmgr_netmask_db *db;
	struct nss_ipsecmgr_ref *ref;
	uint32_t idx;

	/*
	 * subnet lookup before allocating a new one
	 */
	ref = nss_ipsecmgr_subnet_lookup(priv, key);
	if (ref) {
		return ref;
	}

	/*
	 * allocate the netmask
	 */
	netmask = nss_ipsecmgr_netmask_alloc(priv, key);
	if (!netmask) {
		return NULL;
	}

	/*
	 * allocate the subnet entry
	 */
	subnet = kzalloc(sizeof(struct nss_ipsecmgr_subnet_entry), GFP_ATOMIC);
	if (!subnet) {
		nss_ipsecmgr_netmask_free(&ipsecmgr_ctx->net_db, key);
		return NULL;
	}

	subnet->priv = priv;
	ref = &subnet->ref;

	/*
	 * add flow to the database
	 */
	INIT_LIST_HEAD(&subnet->node);

	/*
	 * update key
	 */
	idx = nss_ipsecmgr_subnet_key_data2idx(key, NSS_IPSECMGR_MAX_SUBNET);

	/*
	 * TODO
	 * For subnet entry with any protocol, just add it to the tail.
	 * This will force subnet with any-protcol to have least priority
	 * over other specific protocol entries during subnet lookup.
	 */
	memcpy(&subnet->key, key, sizeof(struct nss_ipsecmgr_key));
	list_add(&subnet->node, &netmask->subnets[idx]);
	netmask->count++;

	db = &ipsecmgr_ctx->net_db;
	atomic_inc(&db->num_entries);

	/*
	 * initiallize the reference object
	 */
	nss_ipsecmgr_ref_init(&subnet->ref, nss_ipsecmgr_subnet_update, nss_ipsecmgr_subnet_free);

	return ref;
}
