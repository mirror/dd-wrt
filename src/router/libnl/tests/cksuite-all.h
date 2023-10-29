#ifndef __LIBNL3_TESTS_CHECK_ALL_H__
#define __LIBNL3_TESTS_CHECK_ALL_H__

#include <check.h>

#include "nl-test-util.h"

Suite *make_nl_attr_suite(void);
Suite *make_nl_addr_suite(void);
Suite *make_nl_ematch_tree_clone_suite(void);
Suite *make_nl_netns_suite(void);

#endif /* __LIBNL3_TESTS_CHECK_ALL_H__ */
