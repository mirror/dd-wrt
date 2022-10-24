#include "check_confuse.h"
#include <stdio.h>
#include <stdlib.h>

static int ptr_count;

static int parse_ptr(cfg_t *cfg, cfg_opt_t *opt,
		     const char *value, void *result)
{
	int *ptr = malloc(sizeof(int));
	if (!ptr)
		return -1;

	*ptr = atoi(value);
	*(void **)result = ptr;

	fprintf(stderr, "make ptr %p (value '%s')\n", ptr, value);
	ptr_count++;

	return 0;
}

static void free_ptr(void *ptr)
{
	ptr_count--;
	fprintf(stderr, "free ptr %p\n", ptr);

	free(ptr);
}

int main(void)
{
	cfg_opt_t opts[] = {
		CFG_PTR_CB("ptr", "1", CFGF_NONE, parse_ptr, free_ptr),
		CFG_END()
	};

	cfg_t *cfg = cfg_init(opts, 0);
	fail_unless(cfg_setopt(cfg, cfg_getopt(cfg, "ptr"), "2"));
	fail_unless(cfg_setopt(cfg, cfg_getopt(cfg, "ptr"), "3"));
	char *ptr4[] = { "4" };
	fail_unless(cfg_setmulti(cfg, "ptr", 1, ptr4) == CFG_SUCCESS);
	char *ptr5[] = { "5" };
	fail_unless(cfg_setmulti(cfg, "ptr", 1, ptr5) == CFG_SUCCESS);
	fail_unless(cfg_parse_buf(cfg, "ptr = 6") == CFG_SUCCESS);
	fail_unless(cfg_parse_buf(cfg, "ptr = 7") == CFG_SUCCESS);
	cfg_free(cfg);

	/* Is malloc/free of ptrs balanced? */
	fail_unless(ptr_count == 0);

	return 0;
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
