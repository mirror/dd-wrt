#include "usr/argp/wargp/joold.h"

#include "usr/nl/joold.h"
#include "usr/argp/log.h"
#include "usr/argp/xlator_type.h"

int handle_joold_advertise(char *iname, int argc, char **argv, void const *arg)
{
	struct joolnl_socket sk;
	struct jool_result result;

	result = joolnl_setup(&sk, xt_get());
	if (result.error)
		return pr_result(&result);

	result = joolnl_joold_advertise(&sk, iname);

	joolnl_teardown(&sk);
	return pr_result(&result);
}

void autocomplete_joold_advertise(void const *args)
{
	/* joold advertise has no arguments. */
}
