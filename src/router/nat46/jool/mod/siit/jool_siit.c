#include "mod/common/init.h"

#include <linux/module.h>
#include "common/iptables.h"
#include "mod/common/kernel_hook.h"
#include "mod/common/log.h"
#include "mod/common/xlator.h"

MODULE_LICENSE(JOOL_LICENSE);
MODULE_AUTHOR("NIC-ITESM");
MODULE_DESCRIPTION("Stateless IP/ICMP Translation (RFC 7915)");
MODULE_VERSION(JOOL_VERSION_STR);

#ifndef XTABLES_DISABLED

static int iptables_error;

static struct xt_target targets[] = {
	{
		.name       = IPTABLES_SIIT_MODULE_NAME,
		.revision   = 0,
		.family     = NFPROTO_IPV6,
		.target     = target_ipv6,
		.checkentry = target_checkentry,
		.targetsize = XT_ALIGN(sizeof(struct target_info)),
		.me         = THIS_MODULE,
	}, {
		.name       = IPTABLES_SIIT_MODULE_NAME,
		.revision   = 0,
		.family     = NFPROTO_IPV4,
		.target     = target_ipv4,
		.checkentry = target_checkentry,
		.targetsize = XT_ALIGN(sizeof(struct target_info)),
		.me         = THIS_MODULE,
	},
};

#endif /* !XTABLES_DISABLED */

static void flush_net(struct net *ns)
{
	jool_xlator_flush_net(ns, XT_SIIT);
}

static void flush_batch(struct list_head *net_exit_list)
{
	jool_xlator_flush_batch(net_exit_list, XT_SIIT);
}

/** Namespace-aware network operation registration object */
static struct pernet_operations joolns_ops = {
	.exit = flush_net,
	.exit_batch = flush_batch,
};

static int __init siit_init(void)
{
	int error;

	pr_debug("Inserting SIIT Jool...\n");

	/* Careful with the order */

	error = register_pernet_subsys(&joolns_ops);
	if (error)
		return error;

#ifndef XTABLES_DISABLED
	iptables_error = xt_register_targets(targets, ARRAY_SIZE(targets));
	if (iptables_error) {
		log_warn("Error code %d while trying to register the iptables targets.\n"
				"iptables SIIT Jool will not be available.",
				iptables_error);
	}
#endif

	/* SIIT instances can now function properly; unlock them. */
	error = jool_siit_get();
	if (error) {
#ifndef XTABLES_DISABLED
		if (!iptables_error)
			xt_unregister_targets(targets, ARRAY_SIZE(targets));
#endif
		unregister_pernet_subsys(&joolns_ops);
		return error;
	}

	pr_info("SIIT Jool v" JOOL_VERSION_STR " module inserted.\n");
	return 0;
}

static void __exit siit_exit(void)
{
	jool_siit_put();
#ifndef XTABLES_DISABLED
	if (!iptables_error)
		xt_unregister_targets(targets, ARRAY_SIZE(targets));
#endif
	unregister_pernet_subsys(&joolns_ops);
	pr_info("SIIT Jool v" JOOL_VERSION_STR " module removed.\n");
}

module_init(siit_init);
module_exit(siit_exit);
