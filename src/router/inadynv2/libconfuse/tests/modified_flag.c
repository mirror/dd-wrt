#include "check_confuse.h"
#include <stdlib.h>
#include <string.h>

static int parse_ptr(cfg_t *cfg, cfg_opt_t *opt,
		     const char *value, void *result)
{
	char *ptr = strdup(value);
	if (!ptr)
		return -1;

	*(void **)result = ptr;
	return 0;
}

static int is_set(cfg_t *cfg, const char *name, int flags)
{
	cfg_opt_t *opt = cfg_getopt(cfg, name);
	if (!opt)
		return -1;
	return !!(opt->flags & flags);
}

int main(void)
{
	cfg_opt_t sub[] = {
		CFG_INT_LIST("int", "{0,1,2}", CFGF_NONE),
		CFG_INT_LIST("nodef", NULL, CFGF_NONE),
		CFG_FLOAT("float", 42.17, CFGF_NONE),
		CFG_BOOL("bool", cfg_true, CFGF_NONE),
		CFG_STR("str", "hello", CFGF_NODEFAULT),
		CFG_PTR_CB("ptr", "dummy", CFGF_NONE, parse_ptr, free),
		CFG_END()
	};
	cfg_opt_t opts[] = {
		CFG_INT_LIST("int", "{0,1,2}", CFGF_NONE),
		CFG_INT_LIST("nodef", NULL, CFGF_NONE),
		CFG_FLOAT("float", 17.42, CFGF_NONE),
		CFG_BOOL("bool", cfg_true, CFGF_NONE),
		CFG_STR("str", "hello", CFGF_NONE),
		CFG_SEC("sub", sub, CFGF_NONE),
		CFG_PTR_CB("ptr", "dummy", CFGF_NONE, parse_ptr, free),
		CFG_END()
	};

	cfg_t *cfg = cfg_init(opts, 0);

	fail_unless(is_set(cfg, "int", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "nodef", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "bool", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "ptr", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|int", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|nodef", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|bool", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|ptr", CFGF_MODIFIED) == 0);

	fail_unless(cfg_parse_buf(cfg, "") == CFG_SUCCESS);

	fail_unless(is_set(cfg, "int", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "nodef", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "bool", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "ptr", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|int", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|nodef", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|bool", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|ptr", CFGF_MODIFIED) == 0);

	fail_unless(cfg_parse_buf(cfg, "int = {3,4}") == CFG_SUCCESS);

	fail_unless(is_set(cfg, "int", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "nodef", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "bool", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "ptr", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|int", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|nodef", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|bool", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|ptr", CFGF_MODIFIED) == 0);

	fail_unless(cfg_parse_buf(cfg, "nodef = {}") == CFG_SUCCESS);

	fail_unless(is_set(cfg, "int", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "nodef", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "bool", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "ptr", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|int", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|nodef", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|bool", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|ptr", CFGF_MODIFIED) == 0);

	fail_unless(cfg_parse_buf(cfg, "float = 3.14") == CFG_SUCCESS);

	fail_unless(is_set(cfg, "int", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "nodef", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "float", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "bool", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "ptr", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|int", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|nodef", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|bool", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|ptr", CFGF_MODIFIED) == 0);

	cfg_free(cfg);
	cfg = cfg_init(opts, 0);
	fail_unless(cfg_setnint(cfg, "nodef", 1, 1) == CFG_SUCCESS);

	fail_unless(is_set(cfg, "int", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "nodef", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "bool", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "ptr", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|int", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|nodef", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|bool", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|ptr", CFGF_MODIFIED) == 0);

	fail_unless(cfg_setbool(cfg, "bool", cfg_true) == CFG_SUCCESS);

	fail_unless(is_set(cfg, "int", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "nodef", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "bool", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "ptr", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|int", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|nodef", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|bool", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|ptr", CFGF_MODIFIED) == 0);

	fail_unless(cfg_setfloat(cfg, "sub|float", 2.718) == CFG_SUCCESS);

	fail_unless(is_set(cfg, "int", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "nodef", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "bool", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "ptr", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|int", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|nodef", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|float", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "sub|bool", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|ptr", CFGF_MODIFIED) == 0);

	fail_unless(cfg_setbool(cfg, "sub|bool", cfg_true) == CFG_SUCCESS);

	fail_unless(is_set(cfg, "int", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "nodef", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "bool", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "ptr", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|int", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|nodef", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|float", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "sub|bool", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "sub|str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|ptr", CFGF_MODIFIED) == 0);

	cfg_free(cfg);
	cfg = cfg_init(opts, 0);
	fail_unless(cfg_parse_buf(cfg, "sub { bool = true }") == CFG_SUCCESS);

	fail_unless(is_set(cfg, "int", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "nodef", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "bool", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "ptr", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|int", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|nodef", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|bool", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "sub|str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|ptr", CFGF_MODIFIED) == 0);

	fail_unless(cfg_parse_buf(cfg, "sub { str = hello }") == CFG_SUCCESS);

	fail_unless(is_set(cfg, "int", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "nodef", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "bool", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "ptr", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|int", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|nodef", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|bool", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "sub|str", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "sub|ptr", CFGF_MODIFIED) == 0);

	char *bad_ints[] = { "1", "a" };
	fail_unless(cfg_setmulti(cfg, "int", 2, bad_ints) == CFG_FAIL);

	fail_unless(is_set(cfg, "int", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "nodef", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "bool", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "ptr", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|int", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|nodef", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|bool", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "sub|str", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "sub|ptr", CFGF_MODIFIED) == 0);

	char *good_ints[] = { "1", "2" };
	fail_unless(cfg_setmulti(cfg, "int", 2, good_ints) == CFG_SUCCESS);

	fail_unless(is_set(cfg, "int", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "nodef", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "bool", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "ptr", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|int", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|nodef", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|bool", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "sub|str", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "sub|ptr", CFGF_MODIFIED) == 0);

	fail_unless(cfg_setmulti(cfg, "sub|ptr", 1, good_ints) == CFG_SUCCESS);

	fail_unless(is_set(cfg, "int", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "nodef", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "bool", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "str", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "ptr", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|int", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|nodef", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|float", CFGF_MODIFIED) == 0);
	fail_unless(is_set(cfg, "sub|bool", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "sub|str", CFGF_MODIFIED) == 1);
	fail_unless(is_set(cfg, "sub|ptr", CFGF_MODIFIED) == 1);

	cfg_free(cfg);

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
