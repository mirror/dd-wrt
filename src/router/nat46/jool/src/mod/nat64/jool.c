#include "mod/common/init.h"

#include <linux/module.h>
#include <net/netfilter/ipv4/nf_defrag_ipv4.h>
#include <net/netfilter/ipv6/nf_defrag_ipv6.h>

#include "common/iptables.h"
#include "mod/common/log.h"
#include "mod/common/kernel_hook.h"
#include "mod/common/xlator.h"

MODULE_LICENSE(JOOL_LICENSE);
MODULE_AUTHOR("NIC-ITESM");
MODULE_DESCRIPTION("Stateful NAT64 (RFC 6146)");
MODULE_VERSION(JOOL_VERSION_STR);

static char const *banner = "\n"
	"                                   ,----,                       \n"
	"         ,--.                    ,/   .`|                 ,--,  \n"
	"       ,--.'|   ,---,          ,`   .'**:               ,--.'|  \n"
	"   ,--,:  :*|  '  .'*\\       ;    ;*****/  ,---.     ,--,  |#:  \n"
	",`--.'`|  '*: /  ;****'.   .'___,/****,'  /     \\ ,---.'|  :#'  \n"
	"|   :**:  |*|:  :*******\\  |    :*****|  /    /#' ;   :#|  |#;  \n"
	":   |***\\ |*::  |***/\\***\\ ;    |.';**; .    '#/  |   |#: _'#|  \n"
	"|   :*'**'; ||  :**' ;.***:`----'  |**|'    /#;   :   :#|.'##|  \n"
	"'   '*;.****;|  |**;/  \\***\\   '   :**;|   :##\\   |   '#'##;#:  \n"
	"|   |*| \\***|'  :**| \\  \\*,'   |   |**';   |###``.\\   \\##.'.#|  \n"
	"'   :*|  ;*.'|  |**'  '--'     '   :**|'   ;######\\`---`:  |#'  \n"
	"|   |*'`--'  |  :**:           ;   |.' '   |##.\\##|     '  ;#|  \n"
	"'   :*|      |  |*,'           '---'   |   :##';##:     |  :#;  \n"
	";   |.'      `--''                      \\   \\####/      '  ,/   \n"
	"'---'                                    `---`--`       '--'    \n";

#ifndef XTABLES_DISABLED

static int iptables_error;

/** iptables module registration object */
static struct xt_target targets[] = {
	{
		.name       = IPTABLES_NAT64_MODULE_NAME,
		.revision   = 0,
		.family     = NFPROTO_IPV6,
		.target     = target_ipv6,
		.checkentry = target_checkentry,
		.targetsize = XT_ALIGN(sizeof(struct target_info)),
		.me         = THIS_MODULE,
	}, {
		.name       = IPTABLES_NAT64_MODULE_NAME,
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
	jool_xlator_flush_net(ns, XT_NAT64);
}

static void flush_batch(struct list_head *net_exit_list)
{
	jool_xlator_flush_batch(net_exit_list, XT_NAT64);
}

/** Namespace-aware network operation registration object */
static struct pernet_operations joolns_ops = {
	.exit = flush_net,
	.exit_batch = flush_batch,
};

static void defrag_enable(struct net *ns)
{
	nf_defrag_ipv4_enable(ns);
	nf_defrag_ipv6_enable(ns);
}

static int __init nat64_init(void)
{
	int error;

	pr_debug("%s", banner);
	pr_debug("Inserting NAT64 Jool...\n");

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

	/* NAT64 instances can now function properly; unlock them. */
	error = jool_nat64_get(defrag_enable);
	if (error) {
#ifndef XTABLES_DISABLED
		if (!iptables_error)
			xt_unregister_targets(targets, ARRAY_SIZE(targets));
#endif
		unregister_pernet_subsys(&joolns_ops);
		return error;
	}

	pr_info("NAT64 Jool v" JOOL_VERSION_STR " module inserted.\n");
	return error;
}

static void __exit nat64_exit(void)
{
	jool_nat64_put();
#ifndef XTABLES_DISABLED
	if (!iptables_error)
		xt_unregister_targets(targets, ARRAY_SIZE(targets));
#endif
	unregister_pernet_subsys(&joolns_ops);
	pr_info("NAT64 Jool v" JOOL_VERSION_STR " module removed.\n");
}

module_init(nat64_init);
module_exit(nat64_exit);
