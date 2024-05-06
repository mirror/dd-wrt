#include "genetlink.h"

#include "common/graybox-types.h"

static struct genl_family *family;

void genl_setup(struct genl_family *new_family)
{
	family = new_family;
}

static int respond(struct genl_info *info, int error_code,
		int attr_id, void *attr, size_t attr_len)
{
	struct sk_buff *skb;
	size_t total_size;
	void *msg_head;
	int error;
	uint32_t pid;

	total_size = nla_total_size(sizeof(__u16)) + nla_total_size(attr_len);

	skb = genlmsg_new(total_size, GFP_KERNEL);
	if (!skb) {
		pr_err("genlmsg_new() failed.\n");
		return -ENOMEM;
	}
	pid = info->snd_portid;

	msg_head = genlmsg_put(skb, pid, info->nlhdr->nlmsg_seq, family, 0, 0);
	if (!msg_head) {
		pr_err("genlmsg_put() failed.\n");
		kfree_skb(skb);
		return -ENOMEM;
	}

	error = nla_put_u16(skb, ATTR_ERROR_CODE, abs(error_code));
	if (error) {
		pr_err("nla_put_u16() failed. (errcode %d)\n", error);
		kfree_skb(skb);
		return error;
	}

	if (attr) {
		error = nla_put(skb, attr_id, attr_len, attr);
		if (error) {
			pr_err("nla_put() failed. (errcode %d)\n", error);
			kfree_skb(skb);
			return error;
		}
	}

	genlmsg_end(skb, msg_head);

	error = genlmsg_reply(skb, info); /* Implicit kfree_skb(skb) here. */
	if (error) {
		pr_err("genlmsg_reply() failed. (errcode %d)\n", error);
		return error;
	}

	return 0;
}

int genl_respond(struct genl_info *info, int error_code)
{
	pr_debug("Responding error code %d.\n", error_code);
	return respond(info, error_code, 0, NULL, 0);
}

int genl_respond_attr(struct genl_info *info, int attr_id, void *attr,
		size_t attr_len)
{
	return respond(info, 0, attr_id, attr, attr_len);
}
