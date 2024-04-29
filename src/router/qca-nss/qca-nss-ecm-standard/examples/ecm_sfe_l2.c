/*
 **************************************************************************
 * Copyright (c) 2022, Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 **************************************************************************
 */
#include <linux/module.h>

#include <linux/version.h>
#include <linux/types.h>
#include <linux/debugfs.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/etherdevice.h>
#include <linux/inet.h>

#include "exports/ecm_sfe_common_public.h"

/*
 * Global WAN interface name parameter.
 */
char wan_name[IFNAMSIZ];
int wan_name_len;

/*
 * DebugFS entry object.
 */
static struct dentry *ecm_sfe_l2_dentry;

/*
 * Policy rule directions.
 */
enum ecm_sfe_l2_policy_rule_dir {
	ECM_SFE_L2_POLICY_RULE_EGRESS = 1,
	ECM_SFE_L2_POLICY_RULE_INGRESS,
	ECM_SFE_L2_POLICY_RULE_EGRESS_INGRESS,
};

/*
 * Policy rule commands.
 */
enum ecm_sfe_l2_policy_rule_cmd {
	ECM_SFE_L2_POLICY_RULE_ADD = 1,
	ECM_SFE_L2_POLICY_RULE_DEL,
	ECM_SFE_L2_POLICY_RULE_FLUSH_ALL
};

/*
 *  ECM tuple directions.
 */
enum ecm_sfe_l2_tuple_dir {
	ECM_SFE_L2_TUPLE_DIR_ORIGINAL,
	ECM_SFE_L2_TUPLE_DIR_REPLY,
};

/*
 * Defunct by 5-tuple command option types.
 */
enum ecm_sfe_l2_defunct_by_5tuple_options {
	ECM_SFE_L2_DEFUNCT_BY_5TUPLE_OPTION_IP_VERSION,
	ECM_SFE_L2_DEFUNCT_BY_5TUPLE_OPTION_SIP,
	ECM_SFE_L2_DEFUNCT_BY_5TUPLE_OPTION_SPORT,
	ECM_SFE_L2_DEFUNCT_BY_5TUPLE_OPTION_DIP,
	ECM_SFE_L2_DEFUNCT_BY_5TUPLE_OPTION_DPORT,
	ECM_SFE_L2_DEFUNCT_BY_5TUPLE_OPTION_PROTOCOL,
	ECM_SFE_L2_DEFUNCT_BY_5TUPLE_OPTION_MAX,
};

/*
 * Policy rule structure
 */
struct ecm_sfe_l2_policy_rule {
	struct list_head list;
	int protocol;
	int src_port;
	int dest_port;
	uint32_t src_addr[4];
	uint32_t dest_addr[4];
	int ip_ver;
	enum ecm_sfe_l2_policy_rule_dir direction;
};

LIST_HEAD(ecm_sfe_l2_policy_rules);
DEFINE_SPINLOCK(ecm_sfe_l2_policy_rules_lock);

/*
 * ecm_sfe_l2_policy_rule_find()
 *	Finds a policy rule with the given parameters.
 */
static struct ecm_sfe_l2_policy_rule *ecm_sfe_l2_policy_rule_find(int ip_ver, uint32_t *sip_addr, int sport,
								  uint32_t *dip_addr, int dport,
								  int protocol)
{
	struct ecm_sfe_l2_policy_rule *rule = NULL;

	list_for_each_entry(rule , &ecm_sfe_l2_policy_rules, list) {
		if (rule->ip_ver != ip_ver)
			continue;

		if (rule->protocol && (rule->protocol != protocol))
			continue;

		if (rule->dest_port && (rule->dest_port != dport))
			continue;

		if (rule->src_port && (rule->src_port != sport))
			continue;

		if (ip_ver == 4) {
			if (rule->dest_addr[0] && (rule->dest_addr[0] != dip_addr[0]))
				continue;
		} else {
			if (rule->dest_addr[0] && memcmp(rule->dest_addr, dip_addr, sizeof(uint32_t) * 4))
				continue;
		}

		if (ip_ver == 4) {
			if (rule->src_addr[0] && (rule->src_addr[0] != sip_addr[0]))
				continue;
		} else {
			if (rule->src_addr[0] && memcmp(rule->src_addr, sip_addr, sizeof(uint32_t) * 4))
				continue;
		}

		return rule;
	}
	return NULL;
}

/*
 * ecm_sfe_l2_connection_check_with_policy_rules()
 *	Checks the ECM tuple with the policy rules in our rules list and
 *	set the L2 acceleration accordingly, if there is a match.
 */
static uint32_t ecm_sfe_l2_connection_check_with_policy_rules(struct ecm_sfe_common_tuple *tuple, enum ecm_sfe_l2_tuple_dir tuple_dir)
{
	struct ecm_sfe_l2_policy_rule *rule = NULL;
	enum ecm_sfe_l2_policy_rule_dir direction;
	uint32_t l2_accel_bits = (ECM_SFE_COMMON_FLOW_L2_ACCEL_ALLOWED | ECM_SFE_COMMON_RETURN_L2_ACCEL_ALLOWED);

	if (tuple_dir == ECM_SFE_L2_TUPLE_DIR_ORIGINAL) {
		spin_lock_bh(&ecm_sfe_l2_policy_rules_lock);
		rule = ecm_sfe_l2_policy_rule_find(tuple->ip_ver, tuple->src_addr, tuple->src_port,
						   tuple->dest_addr, tuple->dest_port, tuple->protocol);
		if (!rule) {
			spin_unlock_bh(&ecm_sfe_l2_policy_rules_lock);
			pr_warn("No rule with this tuple\n");
			goto done;
		}
		direction = rule->direction;
		spin_unlock_bh(&ecm_sfe_l2_policy_rules_lock);

		if (direction == ECM_SFE_L2_POLICY_RULE_EGRESS) {
			pr_debug("flow side should be L3 interface\n");
			l2_accel_bits &= ~ECM_SFE_COMMON_FLOW_L2_ACCEL_ALLOWED;
		} else if (direction == ECM_SFE_L2_POLICY_RULE_INGRESS) {
			pr_debug("return side should be L3 interface\n");
			l2_accel_bits &= ~ECM_SFE_COMMON_RETURN_L2_ACCEL_ALLOWED;
		}
	} else if (tuple_dir == ECM_SFE_L2_TUPLE_DIR_REPLY) {
		spin_lock_bh(&ecm_sfe_l2_policy_rules_lock);
		rule = ecm_sfe_l2_policy_rule_find(tuple->ip_ver, tuple->dest_addr, tuple->dest_port,
						   tuple->src_addr, tuple->src_port, tuple->protocol);

		if (!rule) {
			spin_unlock_bh(&ecm_sfe_l2_policy_rules_lock);
			pr_warn("No rule with this tuple\n");
			goto done;
		}
		direction = rule->direction;
		spin_unlock_bh(&ecm_sfe_l2_policy_rules_lock);

		if (direction == ECM_SFE_L2_POLICY_RULE_EGRESS) {
			pr_debug("return side should be L3 interface\n");
			l2_accel_bits &= ~ECM_SFE_COMMON_RETURN_L2_ACCEL_ALLOWED;
		} else if (direction == ECM_SFE_L2_POLICY_RULE_INGRESS) {
			pr_debug("flow side should be L3 interface\n");
			l2_accel_bits &= ~ECM_SFE_COMMON_FLOW_L2_ACCEL_ALLOWED;
		}
	} else {
		pr_err("unknow tuple_dir: %d\n", tuple_dir);
		goto done;
	}

	if (direction == ECM_SFE_L2_POLICY_RULE_EGRESS_INGRESS) {
		pr_debug("both sides should be L3 interface\n");
		l2_accel_bits &= ~ECM_SFE_COMMON_FLOW_L2_ACCEL_ALLOWED;
		l2_accel_bits &= ~ECM_SFE_COMMON_RETURN_L2_ACCEL_ALLOWED;
	}
done:
	return l2_accel_bits;
}

/*
 * ecm_sfe_l2_accel_check_callback()
 *	L2 acceleration check function callback.
 */
uint32_t ecm_sfe_l2_accel_check_callback(struct ecm_sfe_common_tuple *tuple)
{
	struct net_device *flow_dev;
	struct net_device *return_dev;
	struct net_device *wan_dev;

	if (strlen(wan_name) == 0) {
		pr_debug("WAN interface is not set in the debugfs\n");
		goto done;
	}

	wan_dev =  dev_get_by_name(&init_net, wan_name);
	if (!wan_dev) {
		pr_debug("WAN interface: %s couldn't be found\n", wan_name);
		goto done;
	}

	flow_dev = dev_get_by_index(&init_net, tuple->src_ifindex);
	if (!flow_dev) {
		pr_debug("flow netdevice couldn't be found with index: %d\n", tuple->src_ifindex);
		dev_put(wan_dev);
		goto done;
	}

	return_dev = dev_get_by_index(&init_net, tuple->dest_ifindex);
	if (!return_dev) {
		pr_debug("return netdevice couldn't be found with index: %d\n", tuple->dest_ifindex);
		dev_put(wan_dev);
		dev_put(flow_dev);
		goto done;
	}

	if (wan_dev == return_dev) {
		/*
		 * Check the tuple with the policy rules in the ORIGINAL direction of the tuple.
		 */
		return ecm_sfe_l2_connection_check_with_policy_rules(tuple, ECM_SFE_L2_TUPLE_DIR_ORIGINAL);
	} else if (wan_dev == flow_dev) {
		/*
		 * Check the tuple with the policy rules in the REPLY direction of the tuple.
		 */
		return ecm_sfe_l2_connection_check_with_policy_rules(tuple, ECM_SFE_L2_TUPLE_DIR_REPLY);
	}
done:
	/*
	 * Else, we allow both L2 acceleration for both interfaces.
	 */
	return (ECM_SFE_COMMON_FLOW_L2_ACCEL_ALLOWED | ECM_SFE_COMMON_RETURN_L2_ACCEL_ALLOWED);
}

/*
 * ecm_sfe_l2_flush_policy_rules()
 *	Flushes all the policy rules.
 */
static void ecm_sfe_l2_flush_policy_rules(void)
{
	struct ecm_sfe_l2_policy_rule *rule;
	struct ecm_sfe_l2_policy_rule *tmp;

	spin_lock_bh(&ecm_sfe_l2_policy_rules_lock);
	list_for_each_entry_safe(rule , tmp, &ecm_sfe_l2_policy_rules, list) {
		list_del(&rule->list);
		spin_unlock_bh(&ecm_sfe_l2_policy_rules_lock);
		kfree(rule);
		spin_lock_bh(&ecm_sfe_l2_policy_rules_lock);
	}
	spin_unlock_bh(&ecm_sfe_l2_policy_rules_lock);
}

/*
 * ecm_sfe_l2_delete_policy_rule()
 *	Deletes a policy rule with the given parameters.
 */
static bool ecm_sfe_l2_delete_policy_rule(int ip_ver, uint32_t *sip_addr, int sport, uint32_t *dip_addr, int dport, int protocol)
{
	struct ecm_sfe_l2_policy_rule *rule;

	spin_lock_bh(&ecm_sfe_l2_policy_rules_lock);
	rule = ecm_sfe_l2_policy_rule_find(ip_ver, sip_addr, sport, dip_addr, dport, protocol);
	if (!rule) {
		spin_unlock_bh(&ecm_sfe_l2_policy_rules_lock);
		pr_warn("rule cannot be found in the list\n");
		return false;
	}
	list_del(&rule->list);
	spin_unlock_bh(&ecm_sfe_l2_policy_rules_lock);
	kfree(rule);

	pr_info("rule deleted\n");
	return true;
}

/*
 * ecm_sfe_l2_add_policy_rule()
 *	Adds a policy rule with the given parameters.
 */
static bool ecm_sfe_l2_add_policy_rule(int ip_ver, uint32_t *sip_addr, int sport, uint32_t *dip_addr, int dport, int protocol, enum ecm_sfe_l2_policy_rule_dir direction)
{
	struct ecm_sfe_l2_policy_rule *rule;

	spin_lock_bh(&ecm_sfe_l2_policy_rules_lock);
	rule = ecm_sfe_l2_policy_rule_find(ip_ver, sip_addr, sport, dip_addr, dport, protocol);
	if (rule) {
		if (rule->direction != direction) {
			pr_info("Update direction of the rule from %d to %d\n", rule->direction, direction);
			rule->direction = direction;
		}
		spin_unlock_bh(&ecm_sfe_l2_policy_rules_lock);
		pr_warn("rule is already present\n");
		return true;
	}
	spin_unlock_bh(&ecm_sfe_l2_policy_rules_lock);

	rule = kzalloc(sizeof(struct ecm_sfe_l2_policy_rule), GFP_ATOMIC);
	if (!rule) {
		pr_warn("alloc failed for new rule\n");
		return false;
	}

	rule->ip_ver = ip_ver;
	rule->protocol = protocol;
	rule->src_port = sport;
	rule->dest_port = dport;
	memcpy(rule->src_addr, sip_addr, sizeof(uint32_t) * 4);
	memcpy(rule->dest_addr, dip_addr, sizeof(uint32_t) * 4);
	rule->direction = direction;

	INIT_LIST_HEAD(&rule->list);

	spin_lock_bh(&ecm_sfe_l2_policy_rules_lock);
	list_add(&rule->list, &ecm_sfe_l2_policy_rules);
	spin_unlock_bh(&ecm_sfe_l2_policy_rules_lock);

	pr_info("rule added\n");
	return true;
}

/*
 * ecm_sfe_l2_policy_rule_write()
 *	Adds a policy rule to the rule table.
 *
 * Policy rule must include cmd, ip_ver and direction. It can also include src/dest IP and ports, protocol.
 * cmd and ip_ver MUST be the first 2 options in the command.
 */
static ssize_t ecm_sfe_l2_policy_rule_write(struct file *file,
		const char __user *user_buf, size_t count, loff_t *offset)
{
	char *cmd_buf;
	char *fields;
	char *token;
	char *option, *value;
	int cmd = 0;		/* must be present in the rule */
	int ip_ver = 0;	/* must be present in the rule */
	uint32_t sip_addr[4] = {0};
	uint32_t dip_addr[4] = {0};
	int sport = 0;
	int dport = 0;
	int protocol = 0;
	int direction = 0;	/* must be present in the rule */

	/*
	 * Command is formed as:
	 * echo "cmd=1 ip_ver=4 dport=443 protocol=6 direction=1" > /sys/kernel/debug/ecm_sfe_l2/policy_rules
	 *
	 * cmd: 1 is to add, 2 is to delete a rule.
	 * direction: 1 is egress, 2 is ingress, 3 is both
	 */
	cmd_buf = kzalloc(count + 1, GFP_ATOMIC);
	if (!cmd_buf) {
		pr_warn("unable to allocate memory for cmd buffer\n");
		return -ENOMEM;
	}

	count = simple_write_to_buffer(cmd_buf, count, offset, user_buf, count);

	/*
	 * Split the buffer into tokens
	 */
	fields = cmd_buf;
	while ((token = strsep(&fields, " "))) {
		pr_info("\ntoken: %s\n", token);

		option = strsep(&token, "=");
		value = token;

		pr_info("\t\toption: %s\n", option);
		pr_info("\t\tvalue: %s\n", value);

		if (!strcmp(option, "cmd")) {
			if (sscanf(value, "%d", &cmd)) {
				if (cmd != ECM_SFE_L2_POLICY_RULE_ADD && cmd != ECM_SFE_L2_POLICY_RULE_DEL &&
					cmd != ECM_SFE_L2_POLICY_RULE_FLUSH_ALL) {
					pr_err("invalid cmd value: %d\n", cmd);
					goto fail;
				}
				continue;
			}
			pr_warn("cannot read value\n");
			goto fail;
		}

		if (!strcmp(option, "ip_ver")) {
			if (sscanf(value, "%d", &ip_ver)) {
				if (ip_ver != 4 && ip_ver != 6) {
					pr_err("invalid ip_ver: %d\n", ip_ver);
					goto fail;
				}
				continue;
			}
			pr_warn("cannot read value\n");
			goto fail;
		}

		if (!strcmp(option, "protocol")) {
			if (sscanf(value, "%d", &protocol)) {
				continue;
			}
			pr_warn("cannot read value\n");
			goto fail;
		}

		if (!strcmp(option, "sport")) {
			if (sscanf(value, "%d", &sport)) {
				continue;
			}
			pr_warn("cannot read value\n");
			goto fail;
		}

		if (!strcmp(option, "dport")) {
			if (sscanf(value, "%d", &dport)) {
				continue;
			}
			pr_warn("cannot read value\n");
			goto fail;
		}

		if (!strcmp(option, "direction")) {
			if (cmd == ECM_SFE_L2_POLICY_RULE_DEL) {
				pr_err("direction is not allowed in delete command\n");
				goto fail;
			}

			if (sscanf(value, "%d", &direction)) {
				if (direction != ECM_SFE_L2_POLICY_RULE_EGRESS
					&& direction != ECM_SFE_L2_POLICY_RULE_INGRESS
					&& direction != ECM_SFE_L2_POLICY_RULE_EGRESS_INGRESS) {

					pr_err("invalid direction: %d\n", direction);
					goto fail;
				}
				continue;
			}
			pr_warn("cannot read value\n");
			goto fail;
		}

		if (!strcmp(option, "sip")) {
			if (ip_ver == 4) {
				if (!in4_pton(value, -1, (uint8_t *)&sip_addr[0], -1, NULL)) {
					pr_err("invalid source IP V4 value: %s\n", value);
					goto fail;
				}
			} else if (ip_ver ==6) {
				if (!in6_pton(value, -1, (uint8_t *)sip_addr, -1, NULL)) {
					pr_err("invalid source IP V6 value: %s\n", value);
					goto fail;
				}
			} else {
				pr_err("ip_ver hasn't been set yet\n");
				goto fail;
			}
			continue;
		}

		if (!strcmp(option, "dip")) {
			if (ip_ver == 4) {
				if (!in4_pton(value, -1, (uint8_t *)&dip_addr[0], -1, NULL)) {
					pr_err("invalid destination IP V4 value: %s\n", value);
					goto fail;
				}
			} else if (ip_ver == 6) {
				if (!in6_pton(value, -1, (uint8_t *)dip_addr, -1, NULL)) {
					pr_err("invalid destination IP V6 value: %s\n", value);
					goto fail;
				}
			} else {
				pr_err("ip_ver hasn't been set yet\n");
				goto fail;
			}
			continue;
		}

		pr_warn("unrecognized option: %s\n", option);
		goto fail;
	}

	kfree(cmd_buf);

	if (cmd == ECM_SFE_L2_POLICY_RULE_ADD) {
		if (!ecm_sfe_l2_add_policy_rule(ip_ver, sip_addr, sport, dip_addr, dport, protocol, direction)) {
			pr_err("Add policy rule failed\n");
			return -ENOMEM;
		}
	} else if (cmd == ECM_SFE_L2_POLICY_RULE_DEL) {
		if (!ecm_sfe_l2_delete_policy_rule(ip_ver, sip_addr, sport, dip_addr, dport, protocol)) {
			pr_err("Delete policy rule failed\n");
			return -ENOMEM;
		}
	} else if (cmd == ECM_SFE_L2_POLICY_RULE_FLUSH_ALL) {
		ecm_sfe_l2_flush_policy_rules();
	}

	return count;
fail:
	kfree(cmd_buf);
	return -EINVAL;
}

/*
 * ecm_sfe_l2_policy_rule_seq_show()
 */
static int ecm_sfe_l2_policy_rule_seq_show(struct seq_file *m, void *v)
{
	struct ecm_sfe_l2_policy_rule *rule;

	rule = list_entry(v, struct ecm_sfe_l2_policy_rule, list);

	if (rule->ip_ver == 4) {
		seq_printf(m,	"ip_ver: %d"
				"\tprotocol: %d"
				"\tsip_addr: %pI4"
				"\tdip_addr: %pI4"
				"\tsport: %d"
				"\tdport: %d"
				"\tdirection: %d\n",
				rule->ip_ver,
				rule->protocol,
				&rule->src_addr[0],
				&rule->dest_addr[0],
				rule->src_port,
				rule->dest_port,
				rule->direction);
	} else {
		struct in6_addr saddr;
		struct in6_addr daddr;

		memcpy(&saddr.s6_addr32, rule->src_addr, sizeof(uint32_t) * 4);
		memcpy(&daddr.s6_addr32, rule->dest_addr, sizeof(uint32_t) * 4);

		seq_printf(m,	"ip_ver: %d"
				"\tprotocol: %d"
				"\tsip_addr: %pI6"
				"\tdip_addr: %pI6"
				"\tsport: %d"
				"\tdport: %d"
				"\tdirection: %d\n",
				rule->ip_ver,
				rule->protocol,
				&saddr,
				&daddr,
				rule->src_port,
				rule->dest_port,
				rule->direction);
	}

	return 0;
}

/*
 * ecm_sfe_l2_policy_rule_seq_stop()
 */
static void ecm_sfe_l2_policy_rule_seq_stop(struct seq_file *p, void *v)
{
	spin_unlock_bh(&ecm_sfe_l2_policy_rules_lock);
}

/*
 * ecm_sfe_l2_policy_rule_seq_next()
 */
static void *ecm_sfe_l2_policy_rule_seq_next(struct seq_file *p, void *v,
					loff_t *pos)
{
	return seq_list_next(v, &ecm_sfe_l2_policy_rules, pos);
}

/*
 * ecm_sfe_l2_policy_rule_seq_start()
 */
static void *ecm_sfe_l2_policy_rule_seq_start(struct seq_file *m, loff_t *_pos)
{
	spin_lock_bh(&ecm_sfe_l2_policy_rules_lock);
	return seq_list_start(&ecm_sfe_l2_policy_rules, *_pos);
}

static const struct seq_operations ecm_sfe_l2_policy_rule_seq_ops = {
	.start = ecm_sfe_l2_policy_rule_seq_start,
	.next  = ecm_sfe_l2_policy_rule_seq_next,
	.stop  = ecm_sfe_l2_policy_rule_seq_stop,
	.show  = ecm_sfe_l2_policy_rule_seq_show,
};

/*
 * ecm_sfe_l2_policy_rule_open()
 */
static int ecm_sfe_l2_policy_rule_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &ecm_sfe_l2_policy_rule_seq_ops);
}

/*
 * File operations for policy rules add/delete/list operations.
 */
static const struct file_operations ecm_sfe_l2_policy_rule_fops = {
	.owner		= THIS_MODULE,
	.open		= ecm_sfe_l2_policy_rule_open,
	.write		= ecm_sfe_l2_policy_rule_write,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= seq_release,
};

/*
 * ecm_sfe_l2_defunct_by_5tuple_write()
 *	Writes the defunct by 5-tuple command to the debugfs node.
 */
static ssize_t ecm_sfe_l2_defunct_by_5tuple_write(struct file *f, const char *user_buf,
					  size_t count, loff_t *offset)
{
	int ret = -EINVAL;
	char *cmd_buf;
	int field_count;
	char *fields_ptr;
	char *fields[ECM_SFE_L2_DEFUNCT_BY_5TUPLE_OPTION_MAX];
	char *option, *value;
	int ip_ver;
	uint32_t sip_addr_v4;
	uint32_t dip_addr_v4;
	struct in6_addr sip_addr_v6;
	struct in6_addr dip_addr_v6;
	int sport, dport;
	int protocol;
	bool defunct_result;

	/*
	 * Command is formed as for IPv4 and IPv6 5-tuples as below respectively.
	 *
	 * echo "ip_ver=4 sip=192.168.1.100 sport=443 dip=192.168.2.100 dport=1000 protocol=6" > /sys/kernel/debug/ecm_sfe_l2/defunct_by_5tuple
	 * echo “ip_ver=6 sip=2aaa::100 sport=443 dip=3bbb::200 dport=1000 protocol=6” > /sys/kernel/debug/ecm_sfe_l2/defunct_by_5tuple
	 *
	 * The order of the options MUST be as above and it MUST contain all the 5-tuple fields and the ip_ver.
	 */
	cmd_buf = kzalloc(count + 1, GFP_ATOMIC);
	if (!cmd_buf) {
		pr_warn("unable to allocate memory for cmd buffer\n");
		return -ENOMEM;
	}

	count = simple_write_to_buffer(cmd_buf, count, offset, user_buf, count);

	/*
	 * Split the buffer into its fields
	 */
	field_count = 0;
	fields_ptr = cmd_buf;
	fields[field_count] = strsep(&fields_ptr, " ");
	while (fields[field_count] != NULL) {
		pr_info("Field %d: %s\n", field_count, fields[field_count]);
		field_count++;
		if (field_count == ECM_SFE_L2_DEFUNCT_BY_5TUPLE_OPTION_MAX)
			break;

		fields[field_count] = strsep(&fields_ptr, " ");
	}

	if (field_count != ECM_SFE_L2_DEFUNCT_BY_5TUPLE_OPTION_MAX) {
		kfree(cmd_buf);
		pr_err("Invalid field count %d\n", field_count);
		return -EINVAL;
	}

	/*
	 * IP version (ip_ver) field validation.
	 */
	option = strsep(&fields[ECM_SFE_L2_DEFUNCT_BY_5TUPLE_OPTION_IP_VERSION], "=");
	if (!option || strcmp(option, "ip_ver")) {
		pr_err("invalid IP version option name: %s\n", option);
		goto fail;
	}
	value = fields[ECM_SFE_L2_DEFUNCT_BY_5TUPLE_OPTION_IP_VERSION];
	if (!sscanf(value, "%d", &ip_ver)) {
		pr_err("Unable to read IP version value %s\n", value);
		goto fail;
	}
	if (ip_ver != 4 && ip_ver != 6) {
		pr_err("invalid IP version: %d\n", ip_ver);
		goto fail;
	}

	/*
	 * Source IP (sip) field validation.
	 */
	option = strsep(&fields[ECM_SFE_L2_DEFUNCT_BY_5TUPLE_OPTION_SIP], "=");
	if (!option || strcmp(option, "sip")) {
		pr_err("invalid source IP option name: %s\n", option);
		goto fail;
	}
	value = fields[ECM_SFE_L2_DEFUNCT_BY_5TUPLE_OPTION_SIP];

	if (ip_ver == 4) {
		if (!in4_pton(value, -1, (uint8_t *)&sip_addr_v4, -1, NULL)) {
			pr_err("invalid source IP V4 value: %s\n", value);
			goto fail;
		}
	} else {
		if (!in6_pton(value, -1, (uint8_t *)sip_addr_v6.s6_addr, -1, NULL)) {
			pr_err("invalid source IP V6 value: %s\n", value);
			goto fail;
		}
	}

	/*
	 * Source port (sport) field validadtion.
	 */
	option = strsep(&fields[ECM_SFE_L2_DEFUNCT_BY_5TUPLE_OPTION_SPORT], "=");
	if (!option || strcmp(option, "sport")) {
		pr_err("invalid source port option name: %s\n", option);
		goto fail;
	}
	value = fields[ECM_SFE_L2_DEFUNCT_BY_5TUPLE_OPTION_SPORT];
	if (!sscanf(value, "%d", &sport)) {
		pr_err("Unable to read source port value %s\n", value);
		goto fail;
	}

	/*
	 * Destination IP (dip) field validation.
	 */
	option = strsep(&fields[ECM_SFE_L2_DEFUNCT_BY_5TUPLE_OPTION_DIP], "=");
	if (!option || strcmp(option, "dip")) {
		pr_err("invalid destination IP option name: %s\n", option);
		goto fail;
	}
	value = fields[ECM_SFE_L2_DEFUNCT_BY_5TUPLE_OPTION_DIP];

	if (ip_ver == 4) {
		if (!in4_pton(value, -1, (uint8_t *)&dip_addr_v4, -1, NULL)) {
			pr_err("invalid destination IP V4 value: %s\n", value);
			goto fail;
		}
	} else {
		if (!in6_pton(value, -1, (uint8_t *)dip_addr_v6.s6_addr, -1, NULL)) {
			pr_err("invalid destination IP V6 value: %s\n", value);
			goto fail;
		}
	}

	/*
	 * Destination port (dport) field validadtion.
	 */
	option = strsep(&fields[ECM_SFE_L2_DEFUNCT_BY_5TUPLE_OPTION_DPORT], "=");
	if (!option || strcmp(option, "dport")) {
		pr_err("invalid destination port option name: %s\n", option);
		goto fail;
	}
	value = fields[ECM_SFE_L2_DEFUNCT_BY_5TUPLE_OPTION_DPORT];
	if (!sscanf(value, "%d", &dport)) {
		pr_err("Unable to read destination port value %s\n", value);
		goto fail;
	}

	/*
	 * Protocol field validadtion.
	 */
	option = strsep(&fields[ECM_SFE_L2_DEFUNCT_BY_5TUPLE_OPTION_PROTOCOL], "=");
	if (!option || strcmp(option, "protocol")) {
		pr_err("invalid protocol option name: %s\n", option);
		goto fail;
	}
	value = fields[ECM_SFE_L2_DEFUNCT_BY_5TUPLE_OPTION_PROTOCOL];
	if (!sscanf(value, "%d", &protocol)) {
		pr_err("Unable to read protocol value %s\n", value);
		goto fail;
	}

	/*
	 * Call 5-tuple defunct functions.
	 */
	if (ip_ver == 4) {
		pr_debug("sip: %pI4 sport: %d dip: %pI4 dport: %d protocol: %d\n", &sip_addr_v4, sport, &dip_addr_v4, dport, protocol);
		defunct_result = ecm_sfe_common_defunct_ipv4_connection(sip_addr_v4, htons(sport), dip_addr_v4, htons(dport), protocol);
	} else {
		pr_debug("sip: %pI6 sport: %d dip: %pI6 dport: %d protocol: %d\n", &sip_addr_v6, sport, &dip_addr_v6, dport, protocol);
		defunct_result = ecm_sfe_common_defunct_ipv6_connection(&sip_addr_v6, htons(sport), &dip_addr_v6, htons(dport), protocol);
	}

	if (!defunct_result) {
		pr_warn("No connection found with this 5-tuple\n");
	}

	ret = count;
fail:
	kfree(cmd_buf);

	return ret;
}

/*
 * File operations for defunct by 5-tuple operations.
 */
static struct file_operations ecm_sfe_l2_defunct_by_5tuple_fops = {
	.owner = THIS_MODULE,
	.write = ecm_sfe_l2_defunct_by_5tuple_write,
};

/*
 * ecm_sfe_l2_defunct_by_port_write()
 *	Writes the defunct by port command to the debugfs node.
 */
static ssize_t ecm_sfe_l2_defunct_by_port_write(struct file *f, const char *user_buf,
					  size_t count, loff_t *offset)
{
	char *cmd_buf;
	char *fields;
	char *option, *value;
	int port;
	int direction;

	/*
	 * Command is formed as:
	 *
	 * echo “sport=443” > /sys/kernel/debug/ecm_sfe_l2/defunct_by_port
	 * echo “dport=443” > /sys/kernel/debug/ecm_sfe_l2/defunct_by_port
	 */
	cmd_buf = kzalloc(count + 1, GFP_ATOMIC);
	if (!cmd_buf) {
		pr_warn("unable to allocate memory for cmd buffer\n");
		return -ENOMEM;
	}

	count = simple_write_to_buffer(cmd_buf, count, offset, user_buf, count);

	/*
	 * Split the buffer into its fields
	 */
	fields = cmd_buf;
	option = strsep(&fields, "=");
	if (!strcmp(option, "sport")) {
		direction = 0;
	} else if (!strcmp(option, "dport")) {
		direction = 1;
	} else {
		pr_err("invalid option name: %s\n", option);
		kfree(cmd_buf);
		return -EINVAL;
	}

	value = fields;
	if (!sscanf(value, "%d", &port)) {
		pr_err("Unable to read port value %s\n", value);
		kfree(cmd_buf);
		return -EINVAL;
	}
	pr_debug("option: %s value: %d\n", option, port);

	kfree(cmd_buf);

	/*
	 * Call port based defunct function.
	 */
	ecm_sfe_common_defunct_by_port(port, direction, wan_name);

	return count;
}

/*
 * File operations for defunct by port operations.
 */
static struct file_operations ecm_sfe_l2_defunct_by_port_fops = {
	.owner = THIS_MODULE,
	.write = ecm_sfe_l2_defunct_by_port_write,
};

/*
 * ecm_sfe_l2_defunct_by_protocol_write()
 *	Writes the defunct by protocol command to the debugfs node.
 */
static ssize_t ecm_sfe_l2_defunct_by_protocol_write(struct file *f, const char *user_buf,
					  size_t count, loff_t *offset)
{
	char *cmd_buf;
	char *fields;
	char *option, *value;
	int protocol;

	/*
	 * Command is formed as:
	 *
	 * echo “protocol=6” > /sys/kernel/debug/ecm_sfe_l2/defunct_by_protocol
	 */
	cmd_buf = kzalloc(count + 1, GFP_ATOMIC);
	if (!cmd_buf) {
		pr_warn("unable to allocate memory for cmd buffer\n");
		return -ENOMEM;
	}

	count = simple_write_to_buffer(cmd_buf, count, offset, user_buf, count);

	/*
	 * Split the buffer into its fields
	 */
	fields = cmd_buf;
	option = strsep(&fields, "=");
	if (strcmp(option, "protocol")) {
		pr_err("invalid option name: %s\n", option);
		kfree(cmd_buf);
		return -EINVAL;
	}

	value = fields;
	if (!sscanf(value, "%d", &protocol)) {
		pr_err("Unable to read protocol value %s\n", value);
		kfree(cmd_buf);
		return -EINVAL;
	}
	pr_debug("option: %s value: %d\n", option, protocol);

	kfree(cmd_buf);

	/*
	 * Defunct the connections which has this protocol number.
	 */
	ecm_sfe_common_defunct_by_protocol(protocol);

	return count;
}

/*
 * File operations for defunct by protocol operations.
 */
static struct file_operations ecm_sfe_l2_defunct_by_protocol_fops = {
	.owner = THIS_MODULE,
	.write = ecm_sfe_l2_defunct_by_protocol_write,
};

/*
 * ecm_sfe_l2_wan_name_read()
 *	Reads the WAN interface name from the debugfs node wan_name
 */
static ssize_t ecm_sfe_l2_wan_name_read(struct file *f, char *buffer,
					 size_t len, loff_t *offset)
{
	return simple_read_from_buffer(buffer, len, offset, wan_name, wan_name_len);
}

/*
 * ecm_sfe_l2_wan_name_write()
 *	Writes the WAN interface name to the debugfs node wan_name
 */
static ssize_t ecm_sfe_l2_wan_name_write(struct file *f, const char *buffer,
					  size_t len, loff_t *offset)
{
	ssize_t ret;

	if (len > IFNAMSIZ) {
		pr_err("WAN interface name is too long\n");
		return -EINVAL;
	}

	ret = simple_write_to_buffer(wan_name, IFNAMSIZ, offset, buffer, len);
	if (ret < 0) {
		pr_err("WAN interface name cannot be written\n");
		return ret;
	}

	wan_name[ret - 1] = '\0';
	wan_name_len = ret;

	return ret;
}

/*
 * File operations for wan interface name.
 */
static struct file_operations ecm_sfe_l2_wan_name_fops = {
	.owner = THIS_MODULE,
	.write = ecm_sfe_l2_wan_name_write,
	.read = ecm_sfe_l2_wan_name_read,
};

struct ecm_sfe_common_callbacks sfe_cbs = {
	.l2_accel_check = ecm_sfe_l2_accel_check_callback,	/**< Callback to decide if L2 acceleration is wanted for the flow. */
};

/*
 * ecm_sfe_l2_init()
 */
static int __init ecm_sfe_l2_init(void)
{
	pr_debug("ECM SFE L2 module INIT\n");

	/*
	 * Create entries in DebugFS for control functions
	 */
	ecm_sfe_l2_dentry = debugfs_create_dir("ecm_sfe_l2", NULL);
	if (!ecm_sfe_l2_dentry) {
		pr_info("Failed to create SFE L2 directory entry\n");
		return -1;
	}

	if (!debugfs_create_file("wan_name", S_IWUSR, ecm_sfe_l2_dentry,
					NULL, &ecm_sfe_l2_wan_name_fops)) {
		pr_debug("Failed to create ecm wan interface file in debugfs\n");
		debugfs_remove_recursive(ecm_sfe_l2_dentry);
		return -1;
	}

	if (!debugfs_create_file("policy_rules", S_IWUSR, ecm_sfe_l2_dentry,
					NULL, &ecm_sfe_l2_policy_rule_fops)) {
		pr_debug("Failed to create ecm SFE L2 policy rules file in debugfs\n");
		debugfs_remove_recursive(ecm_sfe_l2_dentry);
		return -1;
	}

	if (!debugfs_create_file("defunct_by_protocol", S_IWUSR, ecm_sfe_l2_dentry,
					NULL, &ecm_sfe_l2_defunct_by_protocol_fops)) {
		pr_debug("Failed to create ecm defunct by protocol file in debugfs\n");
		debugfs_remove_recursive(ecm_sfe_l2_dentry);
		return -1;
	}

	if (!debugfs_create_file("defunct_by_5tuple", S_IWUSR, ecm_sfe_l2_dentry,
					NULL, &ecm_sfe_l2_defunct_by_5tuple_fops)) {
		pr_debug("Failed to create ecm defunct by 5tuple file in debugfs\n");
		debugfs_remove_recursive(ecm_sfe_l2_dentry);
		return -1;
	}

	if (!debugfs_create_file("defunct_by_port", S_IWUSR, ecm_sfe_l2_dentry,
					NULL, &ecm_sfe_l2_defunct_by_port_fops)) {
		pr_debug("Failed to create ecm defunct by port file in debugfs\n");
		debugfs_remove_recursive(ecm_sfe_l2_dentry);
		return -1;
	}

	if (ecm_sfe_common_callbacks_register(&sfe_cbs)) {
		pr_debug("Failed to register callbacks\n");
		debugfs_remove_recursive(ecm_sfe_l2_dentry);
		return -1;
	}
	return 0;
}

/*
 * ecm_sfe_l2_exit()
 */
static void __exit ecm_sfe_l2_exit(void)
{
	pr_debug("ECM SFE L2 check module EXIT\n");

	ecm_sfe_common_callbacks_unregister();

	/*
	 * Remove the debugfs files recursively.
	 */
	debugfs_remove_recursive(ecm_sfe_l2_dentry);
}

module_init(ecm_sfe_l2_init)
module_exit(ecm_sfe_l2_exit)

MODULE_DESCRIPTION("ECM SFE L2 Module");
#ifdef MODULE_LICENSE
MODULE_LICENSE("Dual BSD/GPL");
#endif
