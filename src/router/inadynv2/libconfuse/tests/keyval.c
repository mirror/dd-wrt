/* Test key=value pairs, e.g. env vars. a program should set to children */

#include "config.h"
#include <string.h>
#include <stdlib.h>
#include "check_confuse.h"

static void check_byname(cfg_t *cfg, const char *key, const char *val)
{
	char *value;

	printf("Checking for key:%s, expected value %s\n", key, val);

	value = cfg_getstr(cfg, key);
	printf("Found value %s\n", value);

	fail_unless(value != NULL);
	fail_unless(!strcmp(value, val));
}

static void check_keyval(cfg_t *cfg, unsigned int index, const char *key, const char *val)
{
	cfg_opt_t *opt;
	int rc;

	printf("Checking index %u for key:%s val:%s\n", index, key, val);

	opt = cfg_getnopt(cfg, index);
	fail_unless(opt != NULL);

	printf("Found key:%s val:%s\n", cfg_opt_name(opt), cfg_opt_getstr(opt));

	rc =strcmp(cfg_opt_name(opt), key);
	fail_unless(rc == 0);

	rc =strcmp(cfg_opt_getstr(opt), val);
	fail_unless(rc == 0);
}

int main(void)
{
	cfg_opt_t opts[] = {
		CFG_SEC("env", NULL, CFGF_KEYSTRVAL),
		CFG_END()
	};
	unsigned int num;
	cfg_t *cfg, *sec;
	cfg_opt_t *opt;
	char *key;
	int rc;

	cfg = cfg_init(opts, CFGF_NONE);
	fail_unless(cfg != NULL);

	rc = cfg_parse_buf(cfg, "env {\n"
			   "	foo=bar\n"
			   "	bar=for"
			   "	baz=qux\n"
			   "	bar=xyzzy\n" /* Should replace previous 'bar' */
			   "}");
	fail_unless(rc == CFG_SUCCESS);

	sec = cfg_getsec(cfg, "env");
	fail_unless(sec != NULL);

	/* Fuzz internals a bit, check for non-existing key */
	cfg_getstr(sec, "some-key-not-in-the-config-file");

	num = cfg_num(sec);
	fail_unless(num == 3);	/* { foo, bar, baz } */

	check_keyval(sec, 0, "foo", "bar");
	check_keyval(sec, 1, "bar", "xyzzy");
	check_keyval(sec, 2, "baz", "qux");

	check_byname(sec, "foo", "bar");
	check_byname(sec, "baz", "qux");
	check_byname(sec, "bar", "xyzzy");

	printf("PASS\n");
	cfg_free(cfg);

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
