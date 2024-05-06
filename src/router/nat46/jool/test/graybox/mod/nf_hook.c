#include <linux/kernel.h>
#include <linux/module.h>

#include "common/types.h"
#include "common/xlat.h"

#include "expecter.h"
#include "log.h"
#include "nl_handler.h"

/* Implementation function required by xlat.h */
char const *xlat_get_name(void)
{
	return "Graybox";
}

MODULE_LICENSE(JOOL_LICENSE);
MODULE_AUTHOR("NIC-ITESM");
MODULE_DESCRIPTION("Graybox test gimmic for Jool.");

static int graybox_init(void)
{
	int error;

	log_debug("Inserting the module...");

	error = nlhandler_setup();
	if (error)
		return error;
	error = expecter_setup();
	if (error) {
		nlhandler_teardown();
		return error;
	}

	log_info("%s module inserted.", xlat_get_name());
	return 0;
}

static void graybox_exit(void)
{
	expecter_teardown();
	nlhandler_teardown();

	log_info("%s module removed.", xlat_get_name());
}

module_init(graybox_init);
module_exit(graybox_exit);
