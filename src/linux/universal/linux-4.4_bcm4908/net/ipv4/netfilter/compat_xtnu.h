#ifndef _COMPAT_XTNU_H
#define _COMPAT_XTNU_H 1

#include <linux/list.h>
#include <linux/netfilter/x_tables.h>
#include <linux/spinlock.h>

struct flowi;
struct module;
struct net_device;
struct rtable;
struct sk_buff;

struct xtnu_match {
	/*
	 * Making it smaller by sizeof(void *) on purpose to catch
	 * lossy translation, if any.
	 */
	char name[sizeof(((struct xt_match *)NULL)->name) - 1 - sizeof(void *)];
	uint8_t revision;
	bool (*match)(const struct sk_buff *, struct xt_action_param *);
	int (*checkentry)(const struct xt_mtchk_param *);
	void (*destroy)(const struct xt_mtdtor_param *);
	struct module *me;
	const char *table;
	unsigned int matchsize, hooks;
	unsigned short proto, family;

	void *__compat_match;
};

struct xtnu_target {
	char name[sizeof(((struct xt_target *)NULL)->name) - 1 - sizeof(void *)];
	uint8_t revision;
	unsigned int (*target)(struct sk_buff **,
		const struct xt_action_param *);
	int (*checkentry)(const struct xt_tgchk_param *);
	void (*destroy)(const struct xt_tgdtor_param *);
	struct module *me;
	const char *table;
	unsigned int targetsize, hooks;
	unsigned short proto, family;

	void *__compat_target;
};

static inline struct xtnu_match *xtcompat_numatch(const struct xt_match *m)
{
	void *q;
	memcpy(&q, m->name + sizeof(m->name) - sizeof(void *), sizeof(void *));
	return q;
}

static inline struct xtnu_target *xtcompat_nutarget(const struct xt_target *t)
{
	void *q;
	memcpy(&q, t->name + sizeof(t->name) - sizeof(void *), sizeof(void *));
	return q;
}

extern int xtnu_ip_local_out(struct sk_buff *);
extern int xtnu_ip_route_me_harder(struct sk_buff **, unsigned int);
extern int xtnu_skb_make_writable(struct sk_buff **, unsigned int);
extern int xtnu_register_match(struct xtnu_match *);
extern int xtnu_ip_route_output_key(void *, struct rtable **, struct flowi *);
extern void xtnu_unregister_match(struct xtnu_match *);
extern int xtnu_register_matches(struct xtnu_match *, unsigned int);
extern void xtnu_unregister_matches(struct xtnu_match *, unsigned int);
extern int xtnu_register_target(struct xtnu_target *);
extern void xtnu_unregister_target(struct xtnu_target *);
extern int xtnu_register_targets(struct xtnu_target *, unsigned int);
extern void xtnu_unregister_targets(struct xtnu_target *, unsigned int);
extern struct xt_match *xtnu_request_find_match(unsigned int,
	const char *, uint8_t);
extern void xtnu_proto_csum_replace4(__u16 __bitwise *, struct sk_buff *,
	__be32, __be32, bool);
extern int xtnu_ipv6_skip_exthdr(const struct sk_buff *, int,
	uint8_t *, __be16 *);
extern int xtnu_ipv6_find_hdr(const struct sk_buff *, unsigned int *,
	int, unsigned short *, int *);

extern void *HX_memmem(const void *, size_t, const void *, size_t);

#endif /* _COMPAT_XTNU_H */
