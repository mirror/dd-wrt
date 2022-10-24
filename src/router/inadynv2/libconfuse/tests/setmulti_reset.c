#include "check_confuse.h"

int main(void)
{
	cfg_opt_t opts[] = {
		CFG_INT_LIST("int", "{1,2}", CFGF_NONE),
		CFG_END()
	};

	/* Calling cfg_setopt after cfg_init should not append to the list. */
	cfg_t *cfg = cfg_init(opts, 0);
	fail_unless(cfg_size(cfg, "int") == 2);
	fail_unless(cfg_setopt(cfg, cfg_getopt(cfg, "int"), "3"));
	fail_unless(cfg_size(cfg, "int") == 1);
	cfg_free(cfg);

	/* Not even if you first attempt to use cfg_setmulti with bad input. */
	cfg = cfg_init(opts, 0);
	char *bad[] = { "bad" };
	fail_unless(cfg_size(cfg, "int") == 2);
	fail_unless(cfg_setmulti(cfg, "int", 1, bad) == CFG_FAIL);
	fail_unless(cfg_size(cfg, "int") == 2);
	fail_unless(cfg_setopt(cfg, cfg_getopt(cfg, "int"), "3"));
	fail_unless(cfg_size(cfg, "int") == 1);
	cfg_free(cfg);

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
