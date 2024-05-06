#include "mod/common/nl/attribute.h"

#include <linux/sort.h>
#include "common/constants.h"
#include "mod/common/log.h"

static int validate_null(struct nlattr *attr, char const *name)
{
	if (!attr) {
		log_err("Invalid request: '%s' attribute is missing.", name);
		return -EINVAL;
	}

	return 0;
}

static int validate_len(struct nlattr *attr, char const *name, size_t expected_len)
{
	if (nla_len(attr) < expected_len) {
		log_err("Invalid request: %s has %d bytes instead of %zu.",
				name, nla_len(attr), expected_len);
		return -EINVAL;
	}

	return 0;
}

int jnla_get_u8(struct nlattr *attr, char const *name, __u8 *out)
{
	int error;

	error = validate_null(attr, name);
	if (error)
		return error;

	*out = nla_get_u8(attr);
	return 0;
}

int jnla_get_u16(struct nlattr *attr, char const *name, __u16 *out)
{
	int error;

	error = validate_null(attr, name);
	if (error)
		return error;

	*out = nla_get_u16(attr);
	return 0;
}

int jnla_get_u32(struct nlattr *attr, char const *name, __u32 *out)
{
	int error;

	error = validate_null(attr, name);
	if (error)
		return error;

	*out = nla_get_u32(attr);
	return 0;
}

static int validate_str(char const *str, size_t max_size)
{
	size_t i;

	for (i = 0; i < max_size; i++)
		if (str[i] == '\0')
			return 0;

	return -EINVAL;
}

int jnla_get_str(struct nlattr *attr, char const *name, size_t size, char *out)
{
	int error;

	error = validate_null(attr, name);
	if (error)
		return error;
	error = validate_str(nla_data(attr), size);
	if (error)
		return error;

	strcpy(out, nla_data(attr));
	return 0;
}

int jnla_get_addr6(struct nlattr *attr, char const *name, struct in6_addr *out)
{
	int error;

	error = validate_null(attr, name);
	if (error)
		return error;
	error = validate_len(attr, name, sizeof(struct in6_addr));
	if (error)
		return error;

	memcpy(out, nla_data(attr), sizeof(*out));
	return 0;
}

int jnla_get_addr4(struct nlattr *attr, char const *name, struct in_addr *out)
{
	int error;

	error = validate_null(attr, name);
	if (error)
		return error;
	error = validate_len(attr, name, sizeof(struct in_addr));
	if (error)
		return error;

	memcpy(out, nla_data(attr), sizeof(*out));
	return 0;
}

int jnla_get_prefix6(struct nlattr *attr, char const *name,
		struct ipv6_prefix *out)
{
	struct config_prefix6 tmp;
	int error;

	error = jnla_get_prefix6_optional(attr, name, &tmp);
	if (error)
		return error;

	if (!tmp.set) {
		log_err("Malformed %s: null despite being mandatory", name);
		return -EINVAL;
	}

	*out = tmp.prefix;
	return 0;
}

int jnla_get_prefix6_optional(struct nlattr *attr, char const *name,
		struct config_prefix6 *out)
{
	struct nlattr *attrs[JNLAP_COUNT];
	int error;

	error = validate_null(attr, name);
	if (error)
		return error;

	error = jnla_parse_nested(attrs, JNLAP_MAX, attr, joolnl_prefix6_policy,
			name);
	if (error)
		return error;

	if (!attrs[JNLAP_LEN]) {
		log_err("Malformed %s: length attribute is missing", name);
		return -EINVAL;
	}
	if (!attrs[JNLAP_ADDR]) {
		out->set = false;
		return 0;
	}

	out->set = true;
	out->prefix.len = nla_get_u8(attrs[JNLAP_LEN]);
	return jnla_get_addr6(attrs[JNLAP_ADDR], "IPv6 prefix address",
			&out->prefix.addr);
}

int jnla_get_prefix4(struct nlattr *attr, char const *name,
		struct ipv4_prefix *out)
{
	struct config_prefix4 tmp;
	int error;

	error = jnla_get_prefix4_optional(attr, name, &tmp);
	if (error)
		return error;

	if (!tmp.set) {
		log_err("Malformed %s: null despite being mandatory", name);
		return -EINVAL;
	}

	*out = tmp.prefix;
	return 0;
}

int jnla_get_prefix4_optional(struct nlattr *attr, char const *name,
		struct config_prefix4 *out)
{
	struct nlattr *attrs[JNLAP_COUNT];
	int error;

	error = validate_null(attr, name);
	if (error)
		return error;

	error = jnla_parse_nested(attrs, JNLAP_MAX, attr, joolnl_prefix4_policy,
			name);
	if (error)
		return error;

	if (!attrs[JNLAP_LEN]) {
		log_err("Malformed %s: length attribute is missing", name);
		return -EINVAL;
	}
	if (!attrs[JNLAP_ADDR]) {
		out->set = false;
		return 0;
	}

	out->set = true;
	out->prefix.len = nla_get_u8(attrs[JNLAP_LEN]);
	return jnla_get_addr4(attrs[JNLAP_ADDR], "IPv4 prefix address",
			&out->prefix.addr);
}

static int jnla_get_port(struct nlattr *attr, __u16 *out)
{
	int error;

	error = validate_null(attr, "port");
	if (error)
		return error;

	*out = nla_get_u16(attr);
	return 0;
}

int jnla_get_taddr6(struct nlattr *attr, char const *name,
		struct ipv6_transport_addr *out)
{
	struct nlattr *attrs[JNLAT_COUNT];
	int error;

	error = validate_null(attr, name);
	if (error)
		return error;

	error = jnla_parse_nested(attrs, JNLAT_MAX, attr, joolnl_taddr6_policy,
			name);
	if (error)
		return error;

	error = jnla_get_addr6(attrs[JNLAT_ADDR], "IPv6 address", &out->l3);
	if (error)
		return error;
	return jnla_get_port(attrs[JNLAT_PORT], &out->l4);
}

int jnla_get_taddr4(struct nlattr *attr, char const *name,
		struct ipv4_transport_addr *out)
{
	struct nlattr *attrs[JNLAT_COUNT];
	int error;

	error = validate_null(attr, name);
	if (error)
		return error;

	error = jnla_parse_nested(attrs, JNLAT_MAX, attr, joolnl_taddr4_policy,
			name);
	if (error)
		return error;

	error = jnla_get_addr4(attrs[JNLAT_ADDR], "IPv4 address", &out->l3);
	if (error)
		return error;
	return jnla_get_port(attrs[JNLAT_PORT], &out->l4);
}

int jnla_get_eam(struct nlattr *attr, char const *name, struct eamt_entry *eam)
{
	struct nlattr *attrs[JNLAE_COUNT];
	int error;

	error = validate_null(attr, name);
	if (error)
		return error;

	error = jnla_parse_nested(attrs, JNLAE_MAX, attr, eam_policy, name);
	if (error)
		return error;

	error = jnla_get_prefix6(attrs[JNLAE_PREFIX6], "IPv6 prefix", &eam->prefix6);
	if (error)
		return error;

	return jnla_get_prefix4(attrs[JNLAE_PREFIX4], "IPv4 prefix", &eam->prefix4);
}

int jnla_get_pool4(struct nlattr *attr, char const *name,
		struct pool4_entry *entry)
{
	struct nlattr *attrs[JNLAP4_COUNT];
	int error;

	error = validate_null(attr, name);
	if (error)
		return error;

	error = jnla_parse_nested(attrs, JNLAP4_MAX, attr,
			joolnl_pool4_entry_policy, name);
	if (error)
		return error;

	memset(entry, 0, sizeof(*entry));

	if (attrs[JNLAP4_MARK])
		entry->mark = nla_get_u32(attrs[JNLAP4_MARK]);
	if (attrs[JNLAP4_ITERATIONS])
		entry->iterations = nla_get_u32(attrs[JNLAP4_ITERATIONS]);
	if (attrs[JNLAP4_FLAGS])
		entry->flags = nla_get_u8(attrs[JNLAP4_FLAGS]);

	error = jnla_get_u8(attrs[JNLAP4_PROTO], "Protocol", &entry->proto);
	if (error)
		return error;
	error = jnla_get_prefix4(attrs[JNLAP4_PREFIX], "IPv4 prefix",
			&entry->range.prefix);
	if (error)
		return error;
	error = jnla_get_u16(attrs[JNLAP4_PORT_MIN], "Minimum port",
			&entry->range.ports.min);
	if (error)
		return error;
	return jnla_get_u16(attrs[JNLAP4_PORT_MAX], "Maximum port",
			&entry->range.ports.max);
}

int jnla_get_bib(struct nlattr *attr, char const *name, struct bib_entry *entry)
{
	struct nlattr *attrs[JNLAB_COUNT];
	int error;

	error = validate_null(attr, name);
	if (error)
		return error;

	error = jnla_parse_nested(attrs, JNLAB_MAX, attr,
			joolnl_bib_entry_policy, name);
	if (error)
		return error;

	memset(entry, 0, sizeof(*entry));

	error = jnla_get_taddr6(attrs[JNLAB_SRC6], "IPv6 transport address",
			&entry->addr6);
	if (error)
		return error;
	error = jnla_get_taddr4(attrs[JNLAB_SRC4], "IPv4 transport address",
			&entry->addr4);
	if (error)
		return error;
	error = jnla_get_u8(attrs[JNLAB_PROTO], "Protocol", &entry->l4_proto);
	if (error)
		return error;
	if (attrs[JNLAB_STATIC])
		entry->is_static = nla_get_u8(attrs[JNLAB_STATIC]);

	return 0;
}

static int get_timeout(struct bib_config *config, struct session_entry *entry)
{
	unsigned long timeout;

	switch (entry->proto) {
	case L4PROTO_TCP:
		switch (entry->timer_type) {
		case SESSION_TIMER_EST:
			timeout = config->ttl.tcp_est;
			break;
		case SESSION_TIMER_TRANS:
			timeout = config->ttl.tcp_trans;
			break;
		case SESSION_TIMER_SYN4:
			timeout = TCP_INCOMING_SYN;
			break;
		default:
			log_err("Unknown session timer: %u", entry->timer_type);
			return -EINVAL;
		}
		break;
	case L4PROTO_UDP:
		timeout = config->ttl.udp;
		break;
	case L4PROTO_ICMP:
		timeout = config->ttl.icmp;
		break;
	default:
		log_err("Unknown protocol: %u", entry->proto);
		return -EINVAL;
	}

	entry->timeout = msecs_to_jiffies(timeout);
	return 0;
}

int jnla_get_session(struct nlattr *attr, char const *name,
		struct bib_config *config, struct session_entry *entry)
{
	struct nlattr *attrs[JNLASE_COUNT];
	__u8 u8;
	__u32 expiration;
	int error;

	error = validate_null(attr, name);
	if (error)
		return error;

	error = jnla_parse_nested(attrs, JNLASE_MAX, attr,
			joolnl_session_entry_policy, name);
	if (error)
		return error;

	memset(entry, 0, sizeof(*entry));

	error = jnla_get_taddr6(attrs[JNLASE_SRC6], "IPv6 source address",
			&entry->src6);
	if (error)
		return error;
	error = jnla_get_taddr6(attrs[JNLASE_DST6], "IPv6 destination address",
			&entry->dst6);
	if (error)
		return error;
	error = jnla_get_taddr4(attrs[JNLASE_SRC4], "IPv4 source address",
			&entry->src4);
	if (error)
		return error;
	error = jnla_get_taddr4(attrs[JNLASE_DST4], "IPv4 destination address",
			&entry->dst4);
	if (error)
		return error;

	error = jnla_get_u8(attrs[JNLASE_PROTO], "Protocol", &u8);
	if (error)
		return error;
	entry->proto = u8;
	error = jnla_get_u8(attrs[JNLASE_STATE], "State", &u8);
	if (error)
		return error;
	entry->state = u8;
	error = jnla_get_u8(attrs[JNLASE_TIMER], "Timer", &u8);
	if (error)
		return error;
	entry->timer_type = u8;

	error = get_timeout(config, entry);
	if (error)
		return error;
	error = jnla_get_u32(attrs[JNLASE_EXPIRATION], "Expiration", &expiration);
	if (error)
		return error;
	entry->update_time = jiffies + msecs_to_jiffies(expiration) - entry->timeout;
	entry->has_stored = false;

	return 0;
}

static int u16_compare(const void *a, const void *b)
{
	return *(__u16 *)b - *(__u16 *)a;
}

static void u16_swap(void *a, void *b, int size)
{
	__u16 t = *(__u16 *)a;
	*(__u16 *)a = *(__u16 *)b;
	*(__u16 *)b = t;
}

static int validate_plateaus(struct mtu_plateaus *plateaus)
{
	__u16 *values = plateaus->values;
	unsigned int i, j;

	/* Sort descending. */
	sort(values, plateaus->count, sizeof(*values), u16_compare, u16_swap);

	/* Remove zeroes and duplicates. */
	for (i = 0, j = 1; j < plateaus->count; j++) {
		if (values[j] == 0)
			break;
		if (values[i] != values[j]) {
			i++;
			values[i] = values[j];
		}
	}

	if (values[0] == 0) {
		log_err("The plateaus list contains nothing but zeroes.");
		return -EINVAL;
	}

	/* Update. */
	plateaus->count = i + 1;
	return 0;
}


int jnla_get_plateaus(struct nlattr *root, struct mtu_plateaus *out)
{
	struct nlattr *attr;
	int rem;
	int error;

	error = validate_null(root, "MTU plateaus");
	if (error)
		return error;
	error = nla_validate(nla_data(root), nla_len(root), JNLAL_MAX,
			joolnl_plateau_list_policy, NULL);
	if (error)
		return error;

	out->count = 0;
	nla_for_each_nested(attr, root, rem) {
		if (out->count >= PLATEAUS_MAX) {
			log_err("Too many plateaus.");
			return -EINVAL;
		}

		out->values[out->count] = nla_get_u16(attr);
		out->count++;
	}

	return validate_plateaus(out);
}

int jnla_put_addr6(struct sk_buff *skb, int attrtype,
		struct in6_addr const *addr)
{
	return nla_put(skb, attrtype, sizeof(*addr), addr);
}

int jnla_put_addr4(struct sk_buff *skb, int attrtype,
		struct in_addr const *addr)
{
	return nla_put(skb, attrtype, sizeof(*addr), addr);
}

int jnla_put_prefix6(struct sk_buff *skb, int attrtype,
		struct ipv6_prefix const *prefix)
{
	struct nlattr *root;
	int error;

	root = nla_nest_start(skb, attrtype);
	if (!root)
		return -EMSGSIZE;

	if (prefix) {
		error = jnla_put_addr6(skb, JNLAP_ADDR, &prefix->addr);
		if (error)
			goto cancel;
		error = nla_put_u8(skb, JNLAP_LEN, prefix->len);
		if (error)
			goto cancel;
	} else {
		error = nla_put_u8(skb, JNLAP_LEN, 0);
		if (error)
			goto cancel;
	}

	nla_nest_end(skb, root);
	return 0;

cancel:
	nla_nest_cancel(skb, root);
	return error;
}

int jnla_put_prefix4(struct sk_buff *skb, int attrtype,
		struct ipv4_prefix const *prefix)
{
	struct nlattr *root;
	int error;

	root = nla_nest_start(skb, attrtype);
	if (!root)
		return -EMSGSIZE;

	if (prefix) {
		error = jnla_put_addr4(skb, JNLAP_ADDR, &prefix->addr);
		if (error)
			goto cancel;
		error = nla_put_u8(skb, JNLAP_LEN, prefix->len);
		if (error)
			goto cancel;
	} else {
		error = nla_put_u8(skb, JNLAP_LEN, 0);
		if (error)
			goto cancel;
	}

	nla_nest_end(skb, root);
	return 0;

cancel:
	nla_nest_cancel(skb, root);
	return error;
}

int jnla_put_taddr6(struct sk_buff *skb, int attrtype,
		struct ipv6_transport_addr const *taddr)
{
	struct nlattr *root;
	int error;

	root = nla_nest_start(skb, attrtype);
	if (!root)
		return -EMSGSIZE;

	error = jnla_put_addr6(skb, JNLAT_ADDR, &taddr->l3);
	if (error)
		goto cancel;
	error = nla_put_u16(skb, JNLAT_PORT, taddr->l4);
	if (error)
		goto cancel;

	nla_nest_end(skb, root);
	return 0;

cancel:
	nla_nest_cancel(skb, root);
	return error;
}

int jnla_put_taddr4(struct sk_buff *skb, int attrtype,
		struct ipv4_transport_addr const *taddr)
{
	struct nlattr *root;
	int error;

	root = nla_nest_start(skb, attrtype);
	if (!root)
		return -EMSGSIZE;

	error = jnla_put_addr4(skb, JNLAT_ADDR, &taddr->l3);
	if (error)
		goto cancel;
	error = nla_put_u16(skb, JNLAT_PORT, taddr->l4);
	if (error)
		goto cancel;

	nla_nest_end(skb, root);
	return 0;

cancel:
	nla_nest_cancel(skb, root);
	return error;
}

int jnla_put_eam(struct sk_buff *skb, int attrtype,
		struct eamt_entry const *eam)
{
	struct nlattr *root;
	int error;

	root = nla_nest_start(skb, attrtype);
	if (!root)
		return -EMSGSIZE;

	error = jnla_put_prefix6(skb, JNLAE_PREFIX6, &eam->prefix6);
	if (error)
		goto cancel;
	error = jnla_put_prefix4(skb, JNLAE_PREFIX4, &eam->prefix4);
	if (error)
		goto cancel;

	nla_nest_end(skb, root);
	return 0;

cancel:
	nla_nest_cancel(skb, root);
	return error;
}

int jnla_put_pool4(struct sk_buff *skb, int attrtype,
		struct pool4_entry const *entry)
{
	struct nlattr *root;
	int error;

	root = nla_nest_start(skb, attrtype);
	if (!root)
		return -EMSGSIZE;

	error = nla_put_u32(skb, JNLAP4_MARK, entry->mark)
		|| nla_put_u32(skb, JNLAP4_ITERATIONS, entry->iterations)
		|| nla_put_u8(skb, JNLAP4_FLAGS, entry->flags)
		|| nla_put_u8(skb, JNLAP4_PROTO, entry->proto)
		|| jnla_put_prefix4(skb, JNLAP4_PREFIX, &entry->range.prefix)
		|| nla_put_u16(skb, JNLAP4_PORT_MIN, entry->range.ports.min)
		|| nla_put_u16(skb, JNLAP4_PORT_MAX, entry->range.ports.max);
	if (error) {
		nla_nest_cancel(skb, root);
		return error;
	}

	nla_nest_end(skb, root);
	return 0;
}

int jnla_put_bib(struct sk_buff *skb, int attrtype, struct bib_entry const *bib)
{
	struct nlattr *root;
	int error;

	root = nla_nest_start(skb, attrtype);
	if (!root)
		return -EMSGSIZE;

	error = jnla_put_taddr6(skb, JNLAB_SRC6, &bib->addr6)
		|| jnla_put_taddr4(skb, JNLAB_SRC4, &bib->addr4)
		|| nla_put_u8(skb, JNLAB_PROTO, bib->l4_proto)
		|| nla_put_u8(skb, JNLAB_STATIC, bib->is_static);
	if (error) {
		nla_nest_cancel(skb, root);
		return -EMSGSIZE;
	}

	nla_nest_end(skb, root);
	return 0;
}

int jnla_put_session(struct sk_buff *skb, int attrtype,
		struct session_entry const *entry)
{
	struct nlattr *root;
	unsigned long dying_time;
	int error;

	root = nla_nest_start(skb, attrtype);
	if (!root)
		return -EMSGSIZE;

	dying_time = entry->update_time + entry->timeout;
	dying_time = (dying_time > jiffies)
			? jiffies_to_msecs(dying_time - jiffies)
			: 0;
	if (dying_time > MAX_U32)
		dying_time = MAX_U32;

	error = jnla_put_taddr6(skb, JNLASE_SRC6, &entry->src6)
		|| jnla_put_taddr6(skb, JNLASE_DST6, &entry->dst6)
		|| jnla_put_taddr4(skb, JNLASE_SRC4, &entry->src4)
		|| jnla_put_taddr4(skb, JNLASE_DST4, &entry->dst4)
		|| nla_put_u8(skb, JNLASE_PROTO, entry->proto)
		|| nla_put_u8(skb, JNLASE_STATE, entry->state)
		|| nla_put_u8(skb, JNLASE_TIMER, entry->timer_type)
		|| nla_put_u32(skb, JNLASE_EXPIRATION, dying_time);
	if (error) {
		nla_nest_cancel(skb, root);
		return error;
	}

	nla_nest_end(skb, root);
	return 0;
}

int jnla_put_plateaus(struct sk_buff *skb, int attrtype,
		struct mtu_plateaus const *plateaus)
{
	struct nlattr *root;
	unsigned int i;
	int error;

	root = nla_nest_start(skb, attrtype);
	if (!root)
		return -EMSGSIZE;

	for (i = 0; i < plateaus->count; i++) {
		error = nla_put_u16(skb, JNLAL_ENTRY, plateaus->values[i]);
		if (error) {
			nla_nest_cancel(skb, root);
			return error;
		}
	}

	nla_nest_end(skb, root);
	return 0;
}

int jnla_parse_nested(struct nlattr *tb[], int maxtype,
		const struct nlattr *nla, const struct nla_policy *policy,
		char const *name)
{
	int error;
	struct netlink_ext_ack extack;

	error = nla_parse_nested(tb, maxtype, nla, policy, &extack);
	if (error)
		log_err("The '%s' attribute is malformed: %s", name, extack._msg);

	return error;
}


void report_put_failure(void)
{
	log_err("The allocated Netlink packet is too small to contain the response. This might be a bug; please report it. PAGE_SIZE is %lu.",
			PAGE_SIZE);
}
