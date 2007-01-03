/* Copyright (C) 2003-2004 Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

/* Kernel module implementing an ip hash set */

#include <linux/module.h>
#include <linux/ip.h>
#include <linux/skbuff.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ip_set.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <asm/bitops.h>
#include <linux/spinlock.h>
#include <linux/vmalloc.h>
#include <linux/random.h>

#include <net/ip.h>

#include <linux/netfilter_ipv4/ip_set_malloc.h>
#include <linux/netfilter_ipv4/ip_set_iphash.h>
#include <linux/netfilter_ipv4/ip_set_jhash.h>
#include <linux/netfilter_ipv4/ip_set_prime.h>

static inline __u32
jhash_ip(const struct ip_set_iphash *map, ip_set_ip_t ip)
{
	return jhash_1word(ip, map->initval);
}

static inline __u32
randhash_ip(const struct ip_set_iphash *map, ip_set_ip_t ip)
{
	return (1 + ip % map->prime);
}

static inline __u32
hash_id(struct ip_set *set, ip_set_ip_t ip, ip_set_ip_t *hash_ip)
{
	struct ip_set_iphash *map = (struct ip_set_iphash *) set->data;
	__u32 jhash, randhash, id;
	u_int16_t i;

	*hash_ip = ip & map->netmask;
	jhash = jhash_ip(map, *hash_ip);
	randhash = randhash_ip(map, *hash_ip);
	DP("set: %s, ip:%u.%u.%u.%u, %u.%u.%u.%u, %u.%u.%u.%u",
	   set->name, HIPQUAD(ip), HIPQUAD(*hash_ip), HIPQUAD(map->netmask));

	for (i = 0; i < map->probes; i++) {
		id = (jhash + i * randhash) % map->hashsize;
		DP("hash key: %u", id);
		if (map->members[id] == *hash_ip)
			return id;
		/* No shortcut at testing - there can be deleted
		 * entries. */
	}
	return UINT_MAX;
}

static inline int
__testip(struct ip_set *set, ip_set_ip_t ip, ip_set_ip_t *hash_ip)
{
	return (hash_id(set, ip, hash_ip) != UINT_MAX);
}

static int
testip(struct ip_set *set, const void *data, size_t size,
       ip_set_ip_t *hash_ip)
{
	struct ip_set_req_iphash *req =
	    (struct ip_set_req_iphash *) data;

	if (size != sizeof(struct ip_set_req_iphash)) {
		ip_set_printk("data length wrong (want %zu, have %zu)",
			      sizeof(struct ip_set_req_iphash),
			      size);
		return -EINVAL;
	}
	return __testip(set, req->ip, hash_ip);
}

static int
testip_kernel(struct ip_set *set, const struct sk_buff *skb,
		u_int32_t flags, ip_set_ip_t *hash_ip)
{
	return __testip(set,
			ntohl(flags & IPSET_SRC ? skb->nh.iph->saddr
						: skb->nh.iph->daddr),
			hash_ip);
}

static inline int
__addip(struct ip_set_iphash *map, ip_set_ip_t ip, ip_set_ip_t *hash_ip)
{
	__u32 jhash, randhash, probe;
	u_int16_t i;

	*hash_ip = ip & map->netmask;
	jhash = jhash_ip(map, *hash_ip);
	randhash = randhash_ip(map, *hash_ip);

	for (i = 0; i < map->probes; i++) {
		probe = (jhash + i * randhash) % map->hashsize;
		if (map->members[probe] == *hash_ip)
			return -EEXIST;
		if (!map->members[probe]) {
			map->members[probe] = *hash_ip;
			return 0;
		}
	}
	/* Trigger rehashing */
	return -EAGAIN;
}

static int
addip(struct ip_set *set, const void *data, size_t size,
        ip_set_ip_t *hash_ip)
{
	struct ip_set_req_iphash *req =
	    (struct ip_set_req_iphash *) data;

	if (size != sizeof(struct ip_set_req_iphash)) {
		ip_set_printk("data length wrong (want %zu, have %zu)",
			      sizeof(struct ip_set_req_iphash),
			      size);
		return -EINVAL;
	}
	return __addip((struct ip_set_iphash *) set->data, req->ip, hash_ip);
}

static int
addip_kernel(struct ip_set *set, const struct sk_buff *skb,
	     u_int32_t flags, ip_set_ip_t *hash_ip)
{
	return __addip((struct ip_set_iphash *) set->data,
		       ntohl(flags & IPSET_SRC ? skb->nh.iph->saddr
					       : skb->nh.iph->daddr),
		       hash_ip);
}

static int retry(struct ip_set *set)
{
	struct ip_set_iphash *map = (struct ip_set_iphash *) set->data;
	ip_set_ip_t hash_ip, *members;
	u_int32_t i, hashsize;
	unsigned newbytes;
	int res;
	struct ip_set_iphash tmp = {
		.hashsize = map->hashsize,
		.probes = map->probes,
		.resize = map->resize,
		.netmask = map->netmask,
	};

	if (map->resize == 0)
		return -ERANGE;

    again:
    	res = 0;

	/* Calculate new parameters */
	get_random_bytes(&tmp.initval, 4);
	hashsize = tmp.hashsize + (tmp.hashsize * map->resize)/100;
	if (hashsize == tmp.hashsize)
		hashsize++;
	tmp.prime = make_prime(hashsize);

	ip_set_printk("rehashing of set %s triggered: "
		      "hashsize grows from %u to %u",
		      set->name, tmp.hashsize, hashsize);
	tmp.hashsize = hashsize;

	newbytes = hashsize * sizeof(ip_set_ip_t);
	tmp.members = ip_set_malloc_atomic(newbytes);
	if (!tmp.members) {
		DP("out of memory for %d bytes", newbytes);
		return -ENOMEM;
	}
	memset(tmp.members, 0, newbytes);

	write_lock_bh(&set->lock);
	map = (struct ip_set_iphash *) set->data; /* Play safe */
	for (i = 0; i < map->hashsize && res == 0; i++) {
		if (map->members[i])
			res = __addip(&tmp, map->members[i], &hash_ip);
	}
	if (res) {
		/* Failure, try again */
		write_unlock_bh(&set->lock);
		ip_set_free(tmp.members, newbytes);
		goto again;
	}

	/* Success at resizing! */
	members = map->members;
	hashsize = map->hashsize;

	map->initval = tmp.initval;
	map->prime = tmp.prime;
	map->hashsize = tmp.hashsize;
	map->members = tmp.members;
	write_unlock_bh(&set->lock);

	ip_set_free(members, hashsize * sizeof(ip_set_ip_t));

	return 0;
}

static inline int
__delip(struct ip_set *set, ip_set_ip_t ip, ip_set_ip_t *hash_ip)
{
	struct ip_set_iphash *map = (struct ip_set_iphash *) set->data;
	ip_set_ip_t id = hash_id(set, ip, hash_ip);

	if (id == UINT_MAX)
		return -EEXIST;

	map->members[id] = 0;
	return 0;
}

static int
delip(struct ip_set *set, const void *data, size_t size,
        ip_set_ip_t *hash_ip)
{
	struct ip_set_req_iphash *req =
	    (struct ip_set_req_iphash *) data;

	if (size != sizeof(struct ip_set_req_iphash)) {
		ip_set_printk("data length wrong (want %zu, have %zu)",
			      sizeof(struct ip_set_req_iphash),
			      size);
		return -EINVAL;
	}
	return __delip(set, req->ip, hash_ip);
}

static int
delip_kernel(struct ip_set *set, const struct sk_buff *skb,
	       u_int32_t flags, ip_set_ip_t *hash_ip)
{
	return __delip(set,
		       ntohl(flags & IPSET_SRC ? skb->nh.iph->saddr
					       : skb->nh.iph->daddr),
		       hash_ip);
}

static int create(struct ip_set *set, const void *data, size_t size)
{
	unsigned newbytes;
	struct ip_set_req_iphash_create *req =
	    (struct ip_set_req_iphash_create *) data;
	struct ip_set_iphash *map;

	if (size != sizeof(struct ip_set_req_iphash_create)) {
		ip_set_printk("data length wrong (want %zu, have %zu)",
			       sizeof(struct ip_set_req_iphash_create),
			       size);
		return -EINVAL;
	}

	if (req->hashsize < 1) {
		ip_set_printk("hashsize too small");
		return -ENOEXEC;
	}

	map = kmalloc(sizeof(struct ip_set_iphash), GFP_KERNEL);
	if (!map) {
		DP("out of memory for %d bytes",
		   sizeof(struct ip_set_iphash));
		return -ENOMEM;
	}
	get_random_bytes(&map->initval, 4);
	map->prime = make_prime(req->hashsize);
	map->hashsize = req->hashsize;
	map->probes = req->probes;
	map->resize = req->resize;
	map->netmask = req->netmask;
	newbytes = map->hashsize * sizeof(ip_set_ip_t);
	map->members = ip_set_malloc(newbytes);
	if (!map->members) {
		DP("out of memory for %d bytes", newbytes);
		kfree(map);
		return -ENOMEM;
	}
	memset(map->members, 0, newbytes);

	set->data = map;
	return 0;
}

static void destroy(struct ip_set *set)
{
	struct ip_set_iphash *map = (struct ip_set_iphash *) set->data;

	ip_set_free(map->members, map->hashsize * sizeof(ip_set_ip_t));
	kfree(map);

	set->data = NULL;
}

static void flush(struct ip_set *set)
{
	struct ip_set_iphash *map = (struct ip_set_iphash *) set->data;
	memset(map->members, 0, map->hashsize * sizeof(ip_set_ip_t));
}

static void list_header(const struct ip_set *set, void *data)
{
	struct ip_set_iphash *map = (struct ip_set_iphash *) set->data;
	struct ip_set_req_iphash_create *header =
	    (struct ip_set_req_iphash_create *) data;

	header->hashsize = map->hashsize;
	header->probes = map->probes;
	header->resize = map->resize;
	header->netmask = map->netmask;
}

static int list_members_size(const struct ip_set *set)
{
	struct ip_set_iphash *map = (struct ip_set_iphash *) set->data;

	return (map->hashsize * sizeof(ip_set_ip_t));
}

static void list_members(const struct ip_set *set, void *data)
{
	struct ip_set_iphash *map = (struct ip_set_iphash *) set->data;
	int bytes = map->hashsize * sizeof(ip_set_ip_t);

	memcpy(data, map->members, bytes);
}

static struct ip_set_type ip_set_iphash = {
	.typename		= SETTYPE_NAME,
	.typecode		= IPSET_TYPE_IP,
	.protocol_version	= IP_SET_PROTOCOL_VERSION,
	.create			= &create,
	.destroy		= &destroy,
	.flush			= &flush,
	.reqsize		= sizeof(struct ip_set_req_iphash),
	.addip			= &addip,
	.addip_kernel		= &addip_kernel,
	.retry			= &retry,
	.delip			= &delip,
	.delip_kernel		= &delip_kernel,
	.testip			= &testip,
	.testip_kernel		= &testip_kernel,
	.header_size		= sizeof(struct ip_set_req_iphash_create),
	.list_header		= &list_header,
	.list_members_size	= &list_members_size,
	.list_members		= &list_members,
	.me			= THIS_MODULE,
};

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Jozsef Kadlecsik <kadlec@blackhole.kfki.hu>");
MODULE_DESCRIPTION("iphash type of IP sets");

static int __init init(void)
{
	init_max_malloc_size();
	return ip_set_register_set_type(&ip_set_iphash);
}

static void __exit fini(void)
{
	/* FIXME: possible race with ip_set_create() */
	ip_set_unregister_set_type(&ip_set_iphash);
}

module_init(init);
module_exit(fini);
