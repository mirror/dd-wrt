#include "nl_handler.h"

#include "expecter.h"
#include "genetlink.h"
#include "log.h"
#include "sender.h"
#include "common/types.h"
#include "mod/common/linux_version.h"

static DEFINE_MUTEX(config_mutex);

static int verify_superpriv(void)
{
	if (!capable(CAP_NET_ADMIN)) {
		log_err("Administrative privileges required.");
		return -EPERM;
	}

	return 0;
}

static int handle_expect_add(struct genl_info *info)
{
	struct expected_packet pkt;
	struct nlattr *attr;
	int rem;

	log_debug("========= Expect Add =========");

	if (verify_superpriv())
		return genl_respond(info, -EPERM);

	attr = info->attrs[ATTR_FILENAME];
	if (!attr) {
		log_err("Request lacks a file name.");
		return genl_respond(info, -EINVAL);
	}
	pkt.filename = nla_data(attr);

	attr = info->attrs[ATTR_PKT];
	if (!attr) {
		log_err("Request lacks a packet.");
		return genl_respond(info, -EINVAL);
	}
	pkt.bytes = nla_data(attr);
	pkt.bytes_len = nla_len(attr);

	if (info->attrs[ATTR_EXCEPTIONS]) {
		pkt.exceptions.count = 0;
		nla_for_each_nested(attr, info->attrs[ATTR_EXCEPTIONS], rem) {
			if (pkt.exceptions.count >= PLATEAUS_MAX) {
				log_err("Too many exceptions.");
				return genl_respond(info, -EINVAL);
			}
			pkt.exceptions.values[pkt.exceptions.count] = nla_get_u16(attr);
			pkt.exceptions.count++;
		}
	} else {
		pkt.exceptions.count = 0;
	}

	return genl_respond(info, expecter_add(&pkt));
}

static int handle_send(struct genl_info *info)
{
	char *filename;
	struct nlattr *attr;
	int error;

	log_debug("========= Send =========");

	attr = info->attrs[ATTR_FILENAME];
	if (!attr) {
		log_err("Request lacks a file name.");
		return genl_respond(info, -EINVAL);
	}
	filename = nla_data(attr);

	attr = info->attrs[ATTR_PKT];
	if (!attr) {
		log_err("Request lacks a packet.");
		return genl_respond(info, -EINVAL);
	}

	error = sender_send(filename, nla_data(attr), nla_len(attr));
	log_debug("Ending graybox send");
	return genl_respond(info, error);
}

static int handle_expect_flush(struct genl_info *info)
{
	log_debug("========= Expect Flush =========");

	if (verify_superpriv())
		return genl_respond(info, -EPERM);

	expecter_flush();
	return genl_respond(info, 0);
}

static int handle_stats_display(struct genl_info *info)
{
	struct graybox_stats stats;

	log_debug("========= Stats Display =========");

	expecter_stat(&stats);
	return genl_respond_attr(info, ATTR_STATS, &stats, sizeof(stats));
}

static int handle_stats_flush(struct genl_info *info)
{
	log_debug("========= Stats Flush =========");

	expecter_stat_flush();
	return genl_respond(info, 0);
}

static int handle_userspace_msg(struct sk_buff *skb, struct genl_info *info)
{
	int error;

	mutex_lock(&config_mutex);

	switch (info->genlhdr->cmd) {
	case COMMAND_EXPECT_ADD:
		error = handle_expect_add(info);
		break;
	case COMMAND_EXPECT_FLUSH:
		error = handle_expect_flush(info);
		break;
	case COMMAND_SEND:
		error = handle_send(info);
		break;
	case COMMAND_STATS_DISPLAY:
		error = handle_stats_display(info);
		break;
	case COMMAND_STATS_FLUSH:
		error = handle_stats_flush(info);
		break;
	default:
		log_err("Unknown command code: %d", info->genlhdr->cmd);
		return genl_respond(info, -EINVAL);
	}

	mutex_unlock(&config_mutex);

	return error;
}

static struct nla_policy const graybox_policy[__ATTR_MAX] = {
	[ATTR_FILENAME] = { .type = NLA_STRING },
	[ATTR_PKT] = { .type = NLA_BINARY },
	[ATTR_EXCEPTIONS] = { .type = NLA_NESTED },
	[ATTR_ERROR_CODE] = { .type = NLA_U16 },
	[ATTR_STATS] = { .type = NLA_BINARY },
};

#if LINUX_VERSION_AT_LEAST(5, 2, 0, 8, 0)
#define GRAYBOX_POLICY
#else
#define GRAYBOX_POLICY .policy = graybox_policy,
#endif

static struct genl_ops ops[] = {
	{
		.cmd = COMMAND_EXPECT_ADD,
		.doit = handle_userspace_msg,
		GRAYBOX_POLICY
	},
	{
		.cmd = COMMAND_EXPECT_FLUSH,
		.doit = handle_userspace_msg,
		GRAYBOX_POLICY
	},
	{
		.cmd = COMMAND_SEND,
		.doit = handle_userspace_msg,
		GRAYBOX_POLICY
	},
	{
		.cmd = COMMAND_STATS_DISPLAY,
		.doit = handle_userspace_msg,
		GRAYBOX_POLICY
	},
	{
		.cmd = COMMAND_STATS_FLUSH,
		.doit = handle_userspace_msg,
		GRAYBOX_POLICY
	},
};

static struct genl_family family = {
	.hdrsize = 0,
	.name = "graybox",
	.version = 1,
	.maxattr = __ATTR_MAX,
	.netnsok = true,
	.module = THIS_MODULE,
#if LINUX_VERSION_AT_LEAST(5, 2, 0, 8, 0)
	.policy = graybox_policy,
#endif
	.ops = ops,
	.n_ops = ARRAY_SIZE(ops),
};

int nlhandler_setup(void)
{
	int error;

	error = genl_register_family(&family);
	if (error) {
		log_err("Errcode %d registering the Genetlink family.", error);
		return error;
	}

	genl_setup(&family);
	return 0;
}

void nlhandler_teardown(void)
{
	genl_unregister_family(&family);
}
