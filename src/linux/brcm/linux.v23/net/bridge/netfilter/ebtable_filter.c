/*
 *  ebtable_filter
 *
 *	Authors:
 *	Bart De Schuymer <bart.de.schuymer@pandora.be>
 *
 *  April, 2002
 *
 */

#include <linux/netfilter_bridge/ebtables.h>
#include <linux/module.h>

#define FILTER_VALID_HOOKS ((1 << NF_BR_LOCAL_IN) | (1 << NF_BR_FORWARD) | \
   (1 << NF_BR_LOCAL_OUT))

static struct ebt_entries initial_chains[] =
{
  {0, "INPUT", 0, EBT_ACCEPT, 0},
  {0, "FORWARD", 0, EBT_ACCEPT, 0},
  {0, "OUTPUT", 0, EBT_ACCEPT, 0}
};

static struct ebt_replace initial_table =
{
  "filter", FILTER_VALID_HOOKS, 0, 3 * sizeof(struct ebt_entries),
  { [NF_BR_LOCAL_IN]&initial_chains[0], [NF_BR_FORWARD]&initial_chains[1],
    [NF_BR_LOCAL_OUT]&initial_chains[2] }, 0, NULL, (char *)initial_chains
};

static int check(const struct ebt_table_info *info, unsigned int valid_hooks)
{
	if (valid_hooks & ~FILTER_VALID_HOOKS)
		return -EINVAL;
	return 0;
}

static struct ebt_table frame_filter =
{ 
  {NULL, NULL}, "filter", &initial_table, FILTER_VALID_HOOKS, 
  RW_LOCK_UNLOCKED, check, NULL
};

static unsigned int
ebt_hook (unsigned int hook, struct sk_buff **pskb, const struct net_device *in,
   const struct net_device *out, int (*okfn)(struct sk_buff *))
{
	return ebt_do_table(hook, pskb, in, out, &frame_filter);
}

static struct nf_hook_ops ebt_ops_filter[] = {
	{ { NULL, NULL }, ebt_hook, PF_BRIDGE, NF_BR_LOCAL_IN,
	   NF_BR_PRI_FILTER_BRIDGED},
	{ { NULL, NULL }, ebt_hook, PF_BRIDGE, NF_BR_FORWARD,
	   NF_BR_PRI_FILTER_BRIDGED},
	{ { NULL, NULL }, ebt_hook, PF_BRIDGE, NF_BR_LOCAL_OUT,
	   NF_BR_PRI_FILTER_OTHER}
};

static int __init init(void)
{
	int i, j, ret;

	ret = ebt_register_table(&frame_filter);
	if (ret < 0)
		return ret;
	for (i = 0; i < sizeof(ebt_ops_filter) / sizeof(ebt_ops_filter[0]); i++)
		if ((ret = nf_register_hook(&ebt_ops_filter[i])) < 0)
			goto cleanup;
	return ret;
cleanup:
	for (j = 0; j < i; j++)
		nf_unregister_hook(&ebt_ops_filter[j]);
	ebt_unregister_table(&frame_filter);
	return ret;
}

static void __exit fini(void)
{
	int i;

	for (i = 0; i < sizeof(ebt_ops_filter) / sizeof(ebt_ops_filter[0]); i++)
		nf_unregister_hook(&ebt_ops_filter[i]);
	ebt_unregister_table(&frame_filter);
}

module_init(init);
module_exit(fini);
EXPORT_NO_SYMBOLS;
MODULE_LICENSE("GPL");
