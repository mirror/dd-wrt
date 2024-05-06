#include "mod/common/init.h"

#include <linux/module.h>

#include "mod/common/atomic_config.h"
#include "mod/common/joold.h"
#include "mod/common/log.h"
#include "mod/common/timer.h"
#include "mod/common/wkmalloc.h"
#include "mod/common/xlator.h"
#include "mod/common/db/bib/db.h"
#include "mod/common/db/pool4/rfc6056.h"
#include "mod/common/nl/nl_handler.h"

MODULE_LICENSE(JOOL_LICENSE);
MODULE_AUTHOR("NIC-ITESM");
MODULE_DESCRIPTION("IP/ICMP Translation (Core)");
MODULE_VERSION(JOOL_VERSION_STR);

static unsigned int siit_refs = 0;
static unsigned int nat64_refs = 0;
static DEFINE_MUTEX(lock);

static int setup_common_modules(void)
{
	int error;

	LOG_DEBUG("Initializing common modules.");
	/* Careful with the order. */

	/* NAT64 */
	error = rfc6056_setup();
	if (error)
		goto rfc6056_fail;
	/* TODO (performance) SIIT-only shouldn't need to pay for this; move. */
	error = jtimer_setup();
	if (error)
		goto jtimer_fail;

	/* Common */
	error = xlation_setup();
	if (error)
		goto xlation_fail;
	/*
	 * In kernel < 4.13, this opens the Netfilter packet gate, so all
	 * submodules needed for translation need to be up by now.
	 */
	error = xlator_setup();
	if (error)
		goto xlator_fail;
	error = nlhandler_setup();
	if (error)
		goto nlhandler_fail;

	return 0;

nlhandler_fail:
	xlator_teardown();
xlator_fail:
	xlation_teardown();
xlation_fail:
	jtimer_teardown();
jtimer_fail:
	rfc6056_teardown();
rfc6056_fail:
	return error;
}

static void teardown_common_modules(void)
{
	LOG_DEBUG("Tearing down common modules.");

	/* Careful with the order. */
	/* (iptables packet handler already stopped by jool.ko/jool_siit.ko) */

	/* Common */
	nlhandler_teardown(); /* Userspace requests no longer handled now */
	xlator_teardown(); /* Packets no longer handled by Netfilter now */
	xlation_teardown();
	atomconfig_teardown();

	/* NAT64 */
	jtimer_teardown();
	rfc6056_teardown();
	joold_teardown();
	bib_teardown();
}

int jool_siit_get(void)
{
	mutex_lock(&lock);
	siit_refs++;
	mutex_unlock(&lock);
	return 0;
}
EXPORT_SYMBOL_GPL(jool_siit_get);

void jool_siit_put(void)
{
	mutex_lock(&lock);
	if (!WARN(siit_refs == 0, "Too many jool_siit_put()s!"))
		siit_refs--;
	mutex_unlock(&lock);
}
EXPORT_SYMBOL_GPL(jool_siit_put);

int jool_nat64_get(void (*defrag_enable)(struct net *ns))
{
	mutex_lock(&lock);
	nat64_refs++;
	if (nat64_refs == 1)
		xlator_set_defrag(defrag_enable);
	mutex_unlock(&lock);
	return 0;
}
EXPORT_SYMBOL_GPL(jool_nat64_get);

void jool_nat64_put(void)
{
	mutex_lock(&lock);
	if (!WARN(nat64_refs == 0, "Too many jool_nat64_put()s!"))
		nat64_refs--;
	mutex_unlock(&lock);
}
EXPORT_SYMBOL_GPL(jool_nat64_put);

bool is_siit_enabled(void)
{
	int refs;

	mutex_lock(&lock);
	refs = siit_refs;
	mutex_unlock(&lock);

	return !!refs;
}

bool is_nat64_enabled(void)
{
	int refs;

	mutex_lock(&lock);
	refs = nat64_refs;
	mutex_unlock(&lock);

	return !!refs;
}

static int __init jool_init(void)
{
	int error;

	LOG_DEBUG("Inserting Core Jool...");

	error = setup_common_modules();
	if (error)
		return error;

	log_info("Core Jool v" JOOL_VERSION_STR " module inserted.");
	return 0;
}

static void __exit jool_exit(void)
{
	teardown_common_modules();

#ifdef JKMEMLEAK
	wkmalloc_print_leaks();
	wkmalloc_teardown();
#endif

	log_info("Core Jool v" JOOL_VERSION_STR " module removed.");
}

module_init(jool_init);
module_exit(jool_exit);
