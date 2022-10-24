/*
 * Example of how an application can allow free-form key=value strings as settings,
 * e.g., custom environment variables the program should set for child processes.
 *
 * env.conf:
 *     env {
 *          foo = bar
 *          baz = foo
 *     }
 */
#include <err.h>
#include <confuse.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	const char *file = "env.conf";
	cfg_opt_t opts[] = {
		CFG_SEC("env", NULL, CFGF_KEYSTRVAL),
		CFG_END()
	};
	cfg_t *cfg, *sec;
	int rc;

	if (argc > 1)
		file = argv[1];

	cfg = cfg_init(opts, 0);
	if (!cfg)
		err(1, "Failed cfg_init()");

	rc = cfg_parse(cfg, file);
	if (rc != CFG_SUCCESS) {
		if (rc == CFG_FILE_ERROR)
			err(1, "Failed opening %s", file);

		errx(1, "Failed parsing %s", file);
	}

	sec = cfg_getsec(cfg, "env");
	if (sec) {
		unsigned int i;

		for (i = 0; i < cfg_num(sec); i++) {
			cfg_opt_t *opt = cfg_getnopt(sec, i);

			printf("%s = \"%s\"\n", cfg_opt_name(opt), cfg_opt_getstr(opt));
		}
	}

//	cfg_print(cfg, stdout);
	cfg_free(cfg);

	return 0;
}
