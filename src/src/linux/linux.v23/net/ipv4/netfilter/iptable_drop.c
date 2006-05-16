/*
 * Table for dropped packets.
 *
 * Copyright (C) 2000 Paul `Rusty' Russell
 */
#include <linux/module.h>
#include <linux/netfilter_ipv4/ip_tables.h>

#define DROPPED_VALID_HOOKS (1 << NF_IP_DROPPING)

/* Standard entry. */
struct ipt_standard
{
	struct ipt_entry entry;
	struct ipt_standard_target target;
};

struct ipt_error_target
{
	struct ipt_entry_target target;
	char errorname[IPT_FUNCTION_MAXNAMELEN];
};

struct ipt_error
{
	struct ipt_entry entry;
	struct ipt_error_target target;
};

static struct
{
	struct ipt_replace repl;
	struct ipt_standard entries[1];
	struct ipt_error term;
} initial_table __initdata
= { { "drop", DROPPED_VALID_HOOKS, 2,
      sizeof(struct ipt_standard) + sizeof(struct ipt_error),
      { [NF_IP_DROPPING] 0 },
      { [NF_IP_DROPPING] 0 },
      0, NULL, { } },
    {
	    /* DROPPING */
	    { { { { 0 }, { 0 }, { 0 }, { 0 }, "", "", { 0 }, { 0 }, 0, 0, 0 },
		0,
		sizeof(struct ipt_entry),
		sizeof(struct ipt_standard),
		0, { 0, 0 }, { } },
	      { { { { IPT_ALIGN(sizeof(struct ipt_standard_target)), "" } }, { } },
		-NF_ACCEPT - 1 } }
    },
    /* ERROR */
    { { { { 0 }, { 0 }, { 0 }, { 0 }, "", "", { 0 }, { 0 }, 0, 0, 0 },
	0,
	sizeof(struct ipt_entry),
	sizeof(struct ipt_error),
	0, { 0, 0 }, { } },
      { { { { IPT_ALIGN(sizeof(struct ipt_error_target)), IPT_ERROR_TARGET } },
	  { } },
	"ERROR"
      }
    }
};

static struct ipt_table packet_dropped
= { { NULL, NULL }, "drop", &initial_table.repl,
    DROPPED_VALID_HOOKS, RW_LOCK_UNLOCKED, NULL };

static const char *dropnames[NF_IP_DROP_MAX]
= { [NF_IP_DROP_IGNORES_REDIRECT] = "Invalid redirect",
    [NF_IP_DROP_MARTIAN_SOURCE] = "Unexpected source address",
    [NF_IP_DROP_MARTIAN_DESTINATION] = "Unexpected destination address",
    [NF_IP_DROP_NAT_UNTRACKED] = "NAT dropped untracked packet",
    [NF_IP_DROP_NAT_NO_UNIQUE_TUPLE] = "NAT couldn't map connection",
    [NF_IP_DROP_NAT_FTP_ERROR] = "NAT failed on malformed FTP packet",
};

/* The work comes in here from netfilter.c. */
static unsigned int
ipt_hook(unsigned int hook,
	 struct sk_buff **pskb,
	 const struct net_device *in,
	 const struct net_device *out,
	 int (*okfn)(struct sk_buff *))
{
	const char *reason = NULL;

	if ((*pskb)->nfmark < NF_IP_DROP_MAX)
		reason = dropnames[(*pskb)->nfmark];

	return ipt_do_table(pskb, hook, in, out, &packet_dropped, (void *)reason);
}

static struct nf_hook_ops ipt_ops
= { { NULL, NULL }, ipt_hook, PF_INET, NF_IP_DROPPING, NF_IP_PRI_FILTER };

static int __init init(void)
{
	int ret;

	/* Register table */
	ret = ipt_register_table(&packet_dropped);
	if (ret < 0) {
		printk("iptable_drop: ipt_register_table failed!\n");
		return ret;
	}

	/* Register hooks */
	ret = nf_register_hook(&ipt_ops);
	if (ret < 0) {
		printk("iptable_drop: nf_register_hook failed!\n");
		ipt_unregister_table(&packet_dropped);
	}
	return ret;
}

static void __exit fini(void)
{
	nf_unregister_hook(&ipt_ops);
	ipt_unregister_table(&packet_dropped);
}

module_init(init);
module_exit(fini);
