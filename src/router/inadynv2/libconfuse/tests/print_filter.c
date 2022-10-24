#include "check_confuse.h"
#include <stdio.h>
#include <string.h>

static int no_foo(cfg_t *cfg, cfg_opt_t *opt)
{
	return !strncmp(cfg_opt_name(opt), "foo-", 4);
}

static int no_bar(cfg_t *cfg, cfg_opt_t *opt)
{
	return !strncmp(cfg_opt_name(opt), "bar-", 4);
}

int main(void)
{
	cfg_opt_t sub[] = {
		CFG_INT_LIST("bar-int", "{0,1,2}", CFGF_NONE),
		CFG_INT_LIST("foo-int", NULL, CFGF_NONE),
		CFG_BOOL("bar-bool", cfg_false, CFGF_NONE),
		CFG_FLOAT("foo-float", 2.718, CFGF_NONE),
		CFG_STR("gazonk-string", NULL, CFGF_NONE),
		CFG_END()
	};
	cfg_opt_t opts[] = {
		CFG_INT_LIST("foo-int", "{0,1,2}", CFGF_NONE),
		CFG_INT_LIST("bar-int", NULL, CFGF_NONE),
		CFG_BOOL("foo-bool", cfg_true, CFGF_NONE),
		CFG_FLOAT("bar-float", 3.14, CFGF_NONE),
		CFG_STR("gazonk-string", "foobar", CFGF_NONE),
		CFG_SEC("sub", sub, CFGF_NONE),
		CFG_END()
	};
	char buf[200]; /* should be enough */
	FILE *f;

	cfg_t *cfg = cfg_init(opts, 0);

	cfg_set_print_filter_func(cfg, no_foo);
	f = fmemopen(buf, sizeof(buf), "w+");
	fail_unless(f != NULL);
	cfg_print(cfg, f);
	fclose(f);

	fprintf(stderr, "no_foo filter:\n%s", buf);
	fail_unless(strstr(buf, "foo-") == NULL);
	fail_unless(strstr(buf, "bar-") != NULL);

	cfg_set_print_filter_func(cfg, no_bar);
	f = fmemopen(buf, sizeof(buf), "w+");
	fail_unless(f != NULL);
	cfg_print(cfg, f);
	fclose(f);

	fprintf(stderr, "----\n");
	fprintf(stderr, "no_bar filter:\n%s", buf);
	fail_unless(strstr(buf, "foo-") != NULL);
	fail_unless(strstr(buf, "bar-") == NULL);

	cfg_free(cfg);

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
