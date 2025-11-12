/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Copyright (c) 2013 Arturo Borrero Gonzalez <arturo@debian.org>
 *
 * based on previous code from:
 *
 * Copyright (c) 2013 Pablo Neira Ayuso <pablo@netfilter.org>
 */

#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <netinet/in.h>
#include <errno.h>

#include <linux/netfilter.h>
#include <linux/netfilter/nf_tables.h>

#include <libmnl/libmnl.h>
#include <libnftnl/common.h>
#include <libnftnl/ruleset.h>
#include <libnftnl/table.h>
#include <libnftnl/chain.h>
#include <libnftnl/set.h>
#include <libnftnl/rule.h>

static int seq;

static void memory_allocation_error(void)
{
	perror("OOM");
	exit(EXIT_FAILURE);
}

static int
mnl_talk(struct mnl_socket *nf_sock, const void *data, unsigned int len,
	 int (*cb)(const struct nlmsghdr *nlh, void *data), void *cb_data)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	uint32_t portid = mnl_socket_get_portid(nf_sock);
	int ret;

	if (mnl_socket_sendto(nf_sock, data, len) < 0)
		return -1;

	ret = mnl_socket_recvfrom(nf_sock, buf, sizeof(buf));
	while (ret > 0) {
		ret = mnl_cb_run(buf, ret, seq, portid, cb, cb_data);
		if (ret <= 0)
			goto out;

		ret = mnl_socket_recvfrom(nf_sock, buf, sizeof(buf));
	}
out:
	if (ret < 0 && errno == EAGAIN)
		return 0;

	return ret;
}

/*
 * Rule
 */
static int rule_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nftnl_rule_list *nlr_list = data;
	struct nftnl_rule *r;

	r = nftnl_rule_alloc();
	if (r == NULL)
		memory_allocation_error();

	if (nftnl_rule_nlmsg_parse(nlh, r) < 0)
		goto err_free;

	nftnl_rule_list_add_tail(r, nlr_list);
	return MNL_CB_OK;

err_free:
	nftnl_rule_free(r);
	return MNL_CB_OK;
}

static struct nftnl_rule_list *mnl_rule_dump(struct mnl_socket *nf_sock,
					   int family)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	struct nftnl_rule_list *nlr_list;
	int ret;

	nlr_list = nftnl_rule_list_alloc();
	if (nlr_list == NULL)
		memory_allocation_error();

	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_GETRULE, family,
				    NLM_F_DUMP, seq);

	ret = mnl_talk(nf_sock, nlh, nlh->nlmsg_len, rule_cb, nlr_list);
	if (ret < 0)
		goto err;

	return nlr_list;
err:
	nftnl_rule_list_free(nlr_list);
	return NULL;
}

/*
 * Chain
 */
static int chain_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nftnl_chain_list *nlc_list = data;
	struct nftnl_chain *c;

	c = nftnl_chain_alloc();
	if (c == NULL)
		memory_allocation_error();

	if (nftnl_chain_nlmsg_parse(nlh, c) < 0)
		goto err_free;

	nftnl_chain_list_add_tail(c, nlc_list);
	return MNL_CB_OK;

err_free:
	nftnl_chain_free(c);
	return MNL_CB_OK;
}

static struct nftnl_chain_list *mnl_chain_dump(struct mnl_socket *nf_sock,
					     int family)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	struct nftnl_chain_list *nlc_list;
	int ret;

	nlc_list = nftnl_chain_list_alloc();
	if (nlc_list == NULL)
		memory_allocation_error();

	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_GETCHAIN, family,
				    NLM_F_DUMP, seq);

	ret = mnl_talk(nf_sock, nlh, nlh->nlmsg_len, chain_cb, nlc_list);
	if (ret < 0)
		goto err;

	return nlc_list;
err:
	nftnl_chain_list_free(nlc_list);
	return NULL;
}

/*
 * Table
 */
static int table_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nftnl_table_list *nlt_list = data;
	struct nftnl_table *t;

	t = nftnl_table_alloc();
	if (t == NULL)
		memory_allocation_error();

	if (nftnl_table_nlmsg_parse(nlh, t) < 0)
		goto err_free;

	nftnl_table_list_add_tail(t, nlt_list);
	return MNL_CB_OK;

err_free:
	nftnl_table_free(t);
	return MNL_CB_OK;
}

static struct nftnl_table_list *mnl_table_dump(struct mnl_socket *nf_sock,
					     int family)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	struct nftnl_table_list *nlt_list;
	int ret;

	nlt_list = nftnl_table_list_alloc();
	if (nlt_list == NULL)
		memory_allocation_error();

	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_GETTABLE, family,
				    NLM_F_DUMP, seq);

	ret = mnl_talk(nf_sock, nlh, nlh->nlmsg_len, table_cb, nlt_list);
	if (ret < 0)
		goto err;

	return nlt_list;
err:
	nftnl_table_list_free(nlt_list);
	return NULL;
}

/*
 * Set elements
 */
static int set_elem_cb(const struct nlmsghdr *nlh, void *data)
{
	nftnl_set_elems_nlmsg_parse(nlh, data);
	return MNL_CB_OK;
}

static int mnl_setelem_get(struct mnl_socket *nf_sock, struct nftnl_set *nls)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	uint32_t family = nftnl_set_get_u32(nls, NFTNL_SET_FAMILY);

	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_GETSETELEM, family,
				    NLM_F_DUMP | NLM_F_ACK, seq);
	nftnl_set_nlmsg_build_payload(nlh, nls);

	return mnl_talk(nf_sock, nlh, nlh->nlmsg_len, set_elem_cb, nls);
}

/*
 * Set
 */
static int set_cb(const struct nlmsghdr *nlh, void *data)
{
	struct nftnl_set_list *nls_list = data;
	struct nftnl_set *s;

	s = nftnl_set_alloc();
	if (s == NULL)
		memory_allocation_error();

	if (nftnl_set_nlmsg_parse(nlh, s) < 0)
		goto err_free;

	nftnl_set_list_add_tail(s, nls_list);
	return MNL_CB_OK;

err_free:
	nftnl_set_free(s);
	return MNL_CB_OK;
}

static struct nftnl_set_list *
mnl_set_dump(struct mnl_socket *nf_sock, int family)
{
	char buf[MNL_SOCKET_BUFFER_SIZE];
	struct nlmsghdr *nlh;
	struct nftnl_set *s;
	struct nftnl_set_list *nls_list;
	struct nftnl_set *si;
	struct nftnl_set_list_iter *i;
	int ret;

	s = nftnl_set_alloc();
	if (s == NULL)
		memory_allocation_error();

	nlh = nftnl_nlmsg_build_hdr(buf, NFT_MSG_GETSET, family,
				    NLM_F_DUMP | NLM_F_ACK, seq);
	nftnl_set_nlmsg_build_payload(nlh, s);
	nftnl_set_free(s);

	nls_list = nftnl_set_list_alloc();
	if (nls_list == NULL)
		memory_allocation_error();

	ret = mnl_talk(nf_sock, nlh, nlh->nlmsg_len, set_cb, nls_list);
	if (ret < 0)
		goto err;

	i = nftnl_set_list_iter_create(nls_list);
	if (i == NULL)
		memory_allocation_error();

	si = nftnl_set_list_iter_next(i);
	while (si != NULL) {
		if (mnl_setelem_get(nf_sock, si) != 0) {
			perror("E: Unable to get set elements");
			nftnl_set_list_iter_destroy(i);
			goto err;
		}
		si = nftnl_set_list_iter_next(i);
	}

	nftnl_set_list_iter_destroy(i);

	return nls_list;
err:
	nftnl_set_list_free(nls_list);
	return NULL;
}

/*
 * ruleset
 */

static struct nftnl_ruleset *mnl_ruleset_dump(struct mnl_socket *nf_sock)
{
	struct nftnl_ruleset *rs;
	struct nftnl_table_list *t;
	struct nftnl_chain_list *c;
	struct nftnl_set_list *s;
	struct nftnl_rule_list *r;

	rs = nftnl_ruleset_alloc();
	if (rs == NULL)
		memory_allocation_error();

	t = mnl_table_dump(nf_sock, NFPROTO_UNSPEC);
	if (t != NULL)
		nftnl_ruleset_set(rs, NFTNL_RULESET_TABLELIST, t);

	c = mnl_chain_dump(nf_sock, NFPROTO_UNSPEC);
	if (c != NULL)
		nftnl_ruleset_set(rs, NFTNL_RULESET_CHAINLIST, c);

	s = mnl_set_dump(nf_sock, NFPROTO_UNSPEC);
	if (s != NULL)
		nftnl_ruleset_set(rs, NFTNL_RULESET_SETLIST, s);

	r = mnl_rule_dump(nf_sock, NFPROTO_UNSPEC);
	if (r != NULL)
		nftnl_ruleset_set(rs, NFTNL_RULESET_RULELIST, r);

	return rs;
}

int main(int argc, char *argv[])
{
	struct mnl_socket *nl;
	uint32_t type = NFTNL_OUTPUT_DEFAULT;
	struct nftnl_ruleset *rs;
	int ret;

	if (argc > 2) {
		fprintf(stderr, "%s\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	nl = mnl_socket_open(NETLINK_NETFILTER);
	if (nl == NULL) {
		perror("mnl_socket_open");
		exit(EXIT_FAILURE);
	}

	if (mnl_socket_bind(nl, 0, MNL_SOCKET_AUTOPID) < 0) {
		perror("mnl_socket_bind");
		exit(EXIT_FAILURE);
	}

	seq = time(NULL);

	rs = mnl_ruleset_dump(nl);
	if (rs == NULL) {
		perror("ruleset_dump");
		exit(EXIT_FAILURE);
	}

	ret = nftnl_ruleset_fprintf(stdout, rs, type, 0);
	fprintf(stdout, "\n");

	if (ret == -1)
		perror("E: Error during fprintf operations");

	return 0;
}
