/*
 *  ebtable_nat
 *
 *	Authors:
 *	Bart De Schuymer <bart.de.schuymer@pandora.be>
 *
 *  April, 2002
 *
 */

#include <linux/netfilter_bridge/ebtables.h>
#include <linux/module.h>
#define NAT_VALID_HOOKS ((1 << NF_BR_PRE_ROUTING) | (1 << NF_BR_LOCAL_OUT) | \
   (1 << NF_BR_POST_ROUTING))

static struct ebt_entries initial_chains[] =
{
  {0, "PREROUTING", 0, EBT_ACCEPT, 0},
  {0, "OUTPUT", 0, EBT_ACCEPT, 0},
  {0, "POSTROUTING", 0, EBT_ACCEPT, 0}
};

static struct ebt_replace initial_table =
{
  "nat", NAT_VALID_HOOKS, 0, 3 * sizeof(struct ebt_entries),
  { [NF_BR_PRE_ROUTING]&initial_chains[0], [NF_BR_LOCAL_OUT]&initial_chains[1],
    [NF_BR_POST_ROUTING]&initial_chains[2] }, 0, NULL, (char *)initial_chains
};

static int check(const struct ebt_table_info *info, unsigned int valid_hooks)
{
	if (valid_hooks & ~NAT_VALID_HOOKS)
		return -EINVAL;
	return 0;
}

static struct ebt_table frame_nat =
{
  {NULL, NULL}, "nat", &initial_table, NAT_VALID_HOOKS,
  RW_LOCK_UNLOCKED, check, NULL
};

static unsigned int
ebt_nat_dst(unsigned int hook, struct sk_buff **pskb, const struct net_device *in
   , const struct net_device *out, int (*okfn)(struct sk_buff *))
{
	return ebt_do_table(hook, pskb, in, out, &frame_nat);
}

static unsigned int
ebt_nat_src(unsigned int hook, struct sk_buff **pskb, const struct net_device *in
   , const struct net_device *out, int (*okfn)(struct sk_buff *))
{
	return ebt_do_table(hook, pskb, in, out, &frame_nat);
}

static struct nf_hook_ops ebt_ops_nat[] = {
	{ { NULL, NULL }, ebt_nat_dst, PF_BRIDGE, NF_BR_LOCAL_OUT,
	   NF_BR_PRI_NAT_DST_OTHER},
	{ { NULL, NULL }, ebt_nat_src, PF_BRIDGE, NF_BR_POST_ROUTING,
	   NF_BR_PRI_NAT_SRC},
	{ { NULL, NULL }, ebt_nat_dst, PF_BRIDGE, NF_BR_PRE_ROUTING,
	   NF_BR_PRI_NAT_DST_BRIDGED},
};

static int __init init(void)
{
	int i, ret, j;

	ret = ebt_register_table(&frame_nat);
	if (ret < 0)
		return ret;
	for (i = 0; i < sizeof(ebt_ops_nat) / sizeof(ebt_ops_nat[0]); i++)
		if ((ret = nf_register_hook(&ebt_ops_nat[i])) < 0)
			goto cleanup;
	return ret;
cleanup:
	for (j = 0; j < i; j++)
		nf_unregister_hook(&ebt_ops_nat[j]);
	ebt_unregister_table(&frame_nat);
	return ret;
}

static void __exit fini(void)
{
	int i;

	for (i = 0; i < sizeof(ebt_ops_nat) / sizeof(ebt_ops_nat[0]); i++)
		nf_unregister_hook(&ebt_ops_nat[i]);
	ebt_unregister_table(&frame_nat);
}

module_init(init);
module_exit(fini);
EXPORT_NO_SYMBOLS;
MODULE_LICENSE("GPL");
