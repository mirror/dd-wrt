#include <check.h>

#define nl_fail_if(condition, error, message) \
	fail_if((condition), "nlerr=%d (%s): %s", \
		(error), nl_geterror(error), (message))

Suite *make_nl_attr_suite(void);
Suite *make_nl_addr_suite(void);
Suite *make_nl_ematch_tree_clone_suite(void);

