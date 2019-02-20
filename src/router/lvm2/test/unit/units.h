/*
 * Copyright (C) 2018 Red Hat, Inc. All rights reserved.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU General Public License v.2.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef TEST_UNIT_UNITS_H
#define TEST_UNIT_UNITS_H

#include "framework.h"

//-----------------------------------------------------------------

// Declare the function that adds tests suites here ...
void activation_generator_tests(struct dm_list *suites);
void bcache_tests(struct dm_list *suites);
void bcache_utils_tests(struct dm_list *suites);
void bitset_tests(struct dm_list *suites);
void config_tests(struct dm_list *suites);
void dm_list_tests(struct dm_list *suites);
void dm_status_tests(struct dm_list *suites);
void io_engine_tests(struct dm_list *suites);
void percent_tests(struct dm_list *suites);
void radix_tree_tests(struct dm_list *suites);
void regex_tests(struct dm_list *suites);
void string_tests(struct dm_list *suites);
void vdo_tests(struct dm_list *suites);

// ... and call it in here.
static inline void register_all_tests(struct dm_list *suites)
{
        activation_generator_tests(suites);
	bcache_tests(suites);
	bcache_utils_tests(suites);
	bitset_tests(suites);
	config_tests(suites);
	dm_list_tests(suites);
	dm_status_tests(suites);
	io_engine_tests(suites);
	percent_tests(suites);
	radix_tree_tests(suites);
	regex_tests(suites);
	string_tests(suites);
	vdo_tests(suites);
}

//-----------------------------------------------------------------

#endif
