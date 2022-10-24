#include "check_confuse.h"
#include <string.h>

int main(void)
{
	static cfg_opt_t sub_opts[] = {
		CFG_INT("int", 1, CFGF_NONE),
		CFG_END()
	};

	cfg_opt_t opts[] = {
		CFG_SEC("single", sub_opts, CFGF_NONE),
		CFG_SEC("multi", sub_opts, CFGF_MULTI),
		CFG_SEC("multi-title", sub_opts, CFGF_TITLE | CFGF_MULTI),
		CFG_SEC("multi-title-no-dupes", sub_opts,
			CFGF_TITLE | CFGF_MULTI | CFGF_NO_TITLE_DUPES),
		CFG_END()
	};

	const char *config_data =
		"single { int = 11 }\n"
		"multi { int = 21 }\n"
		"multi { int = 22 }\n"
		"multi { int = 23 }\n"
		"multi-title name { int = 30 }\n"
		"multi-title name { int = 31 }\n"
		"multi-title odd  { int = 32 }\n"
		"multi-title-no-dupes name1 { int = 41 }\n"
		"multi-title-no-dupes name2 { int = 42 }\n"
		"multi-title-no-dupes name3 { int = 43 }\n";

	int rc;
	cfg_t *cfg = cfg_init(opts, CFGF_NONE);
	cfg_t *sec;

	fail_unless(cfg);

	rc = cfg_parse_buf(cfg, config_data);
	fail_unless(rc == CFG_SUCCESS);

	fail_unless(cfg_addtsec(cfg, "multi-title", "'a-very'silly|option\\title") != NULL);

	fail_unless(cfg_getint(cfg, "single|int") == 11);
	fail_unless(cfg_getint(cfg, "multi=0|int") == 21);
	fail_unless(cfg_getint(cfg, "multi=1|int") == 22);
	fail_unless(cfg_getint(cfg, "multi=2|int") == 23);
	fail_unless(cfg_getint(cfg, "multi-title=name|int") == 31);
	fail_unless(cfg_getint(cfg, "multi-title=odd|int") == 32);
	fail_unless(cfg_getint(cfg, "multi-title='\\'a-very\\'silly|option\\\\title'|int") == 1);
	fail_unless(cfg_getint(cfg, "multi-title-no-dupes=name1|int") == 41);
	fail_unless(cfg_getint(cfg, "multi-title-no-dupes=name2|int") == 42);
	fail_unless(cfg_getint(cfg, "multi-title-no-dupes=name3|int") == 43);
	sec = cfg_getsec(cfg, "single");
	fail_unless(cfg_getint(sec, "int") == 11);
	sec = cfg_getsec(cfg, "multi=0");
	fail_unless(cfg_getint(sec, "int") == 21);
	sec = cfg_getsec(cfg, "multi=1");
	fail_unless(cfg_getint(sec, "int") == 22);
	sec = cfg_getsec(cfg, "multi=2");
	fail_unless(cfg_getint(sec, "int") == 23);
	sec = cfg_getsec(cfg, "multi-title=name");
	fail_unless(cfg_getint(sec, "int") == 31);
	sec = cfg_getsec(cfg, "multi-title=odd");
	fail_unless(cfg_getint(sec, "int") == 32);
	sec = cfg_getsec(cfg, "multi-title='\\'a-very\\'silly|option\\\\title'");
	fail_unless(cfg_getint(sec, "int") == 1);
	sec = cfg_getsec(cfg, "multi-title-no-dupes=name1");
	fail_unless(cfg_getint(sec, "int") == 41);
	sec = cfg_getsec(cfg, "multi-title-no-dupes=name2");
	fail_unless(cfg_getint(sec, "int") == 42);
	sec = cfg_getsec(cfg, "multi-title-no-dupes=name3");
	fail_unless(cfg_getint(sec, "int") == 43);

	/* for backwards compat */
	fail_unless(cfg_getint(cfg, "multi|int") == 21);
	fail_unless(cfg_getint(cfg, "multi-title|int") == 31);
	fail_unless(cfg_getint(cfg, "multi-title-no-dupes|int") == 41);
	sec = cfg_getsec(cfg, "multi");
	fail_unless(cfg_getint(sec, "int") == 21);
	sec = cfg_getsec(cfg, "multi-title");
	fail_unless(cfg_getint(sec, "int") == 31);
	sec = cfg_getsec(cfg, "multi-title-no-dupes");
	fail_unless(cfg_getint(sec, "int") == 41);

	/* expected failures */
	fail_unless(cfg_getopt(cfg, "single=0|int") == NULL);
	fail_unless(cfg_getopt(cfg, "multi=4|int") == NULL);
	fail_unless(cfg_getopt(cfg, "multi-title=0|int") == NULL);
	fail_unless(cfg_getopt(cfg, "multi-title=bad|int") == NULL);
	fail_unless(cfg_getopt(cfg, "multi-title-no-dupes=0|int") == NULL);
	fail_unless(cfg_getopt(cfg, "multi-title-no-dupes=bad|int") == NULL);
	fail_unless(cfg_getsec(cfg, "single=0") == NULL);
	fail_unless(cfg_getsec(cfg, "multi=4") == NULL);
	fail_unless(cfg_getsec(cfg, "multi-title=0") == NULL);
	fail_unless(cfg_getsec(cfg, "multi-title=bad") == NULL);
	fail_unless(cfg_getsec(cfg, "multi-title-no-dupes=0") == NULL);
	fail_unless(cfg_getsec(cfg, "multi-title-no-dupes=bad") == NULL);

	cfg_free(cfg);

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
