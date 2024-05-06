#include "usr/argp/wargp/file.h"

#include <errno.h>

#include "usr/argp/log.h"
#include "usr/argp/requirements.h"
#include "usr/argp/wargp.h"
#include "usr/argp/xlator_type.h"
#include "usr/nl/core.h"
#include "usr/nl/file.h"

struct update_args {
	struct wargp_string file_name;
	struct wargp_bool force;
};

static struct wargp_option update_opts[] = {
	WARGP_FORCE(struct update_args, force),
	{
		.name = "File name",
		.key = ARGP_KEY_ARG,
		.doc = "Path to a JSON file containing Jool's configuration.",
		.offset = offsetof(struct update_args, file_name),
		.type = &wt_string,
	},
	{ 0 },
};

int handle_file_update(char *iname, int argc, char **argv, void const *arg)
{
	struct update_args uargs = { 0 };
	struct joolnl_socket sk;
	struct jool_result result;

	result.error = wargp_parse(update_opts, argc, argv, &uargs);
	if (result.error)
		return result.error;

	if (!uargs.file_name.value) {
		struct requirement reqs[] = {
				{ false, "a file name" },
				{ 0 }
		};
		return requirement_print(reqs);
	}

	result = joolnl_setup(&sk, xt_get());
	if (result.error)
		return pr_result(&result);

	result = joolnl_file_parse(&sk, xt_get(), iname, uargs.file_name.value,
			uargs.force.value);

	joolnl_teardown(&sk);
	return pr_result(&result);
}

void autocomplete_file_update(void const *args)
{
	/* Do nothing; default to autocomplete directory path */
}
