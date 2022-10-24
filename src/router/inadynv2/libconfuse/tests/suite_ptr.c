#include "check_confuse.h"
#include <string.h>

static int parse_ptr(cfg_t *cfg, cfg_opt_t *opt,
		     const char *value, void *result)
{
	int *ptr;

	if (!strcmp(value, "nil")) {
		*(void **)result = NULL;
		return 0;
	}

	ptr = malloc(sizeof(int));
	if (!ptr)
		return -1;

	*ptr = atoi(value);
	*(void **)result = ptr;
	return 0;
}

int main(void)
{
	cfg_opt_t opts[] = {
		CFG_PTR_CB("ptr", 0, CFGF_NONE, parse_ptr, free),
		CFG_PTR_CB("ptr-nil", "nil", CFGF_NONE, parse_ptr, free),
		CFG_PTR_CB("ptr-one", "1", CFGF_NONE, parse_ptr, free),
		CFG_END()
	};

	cfg_t *cfg = cfg_init(opts, 0);
	fail_unless(cfg_parse_buf(cfg, "") == CFG_SUCCESS);
	fail_unless(cfg_size(cfg, "ptr") == 0);
	fail_unless(cfg_getptr(cfg, "ptr") == 0);
	fail_unless(cfg_size(cfg, "ptr-nil") == 1);
	fail_unless(cfg_getptr(cfg, "ptr-nil") == 0);
	fail_unless(cfg_size(cfg, "ptr-one") == 1);
	fail_unless(cfg_getptr(cfg, "ptr-one") != 0);
	cfg_free(cfg);

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
