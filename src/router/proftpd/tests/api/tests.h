/*
 * ProFTPD - FTP server testsuite
 * Copyright (c) 2008 The ProFTPD Project team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 *
 * As a special exemption, The ProFTPD Project team and other respective
 * copyright holders give permission to link this program with OpenSSL, and
 * distribute the resulting executable, without including the source code for
 * OpenSSL in the source distribution.
 */

/*
 * Testsuite management
 * $Id: tests.h,v 1.1 2008/10/06 18:16:50 castaglia Exp $
 */

#ifndef PR_TESTS_H
#define PR_TESTS_H

#include "conf.h"
#include "privs.h"

#ifdef HAVE_CHECK_H
# include <check.h>
#else
# error "Missing Check installation; necessary for ProFTPD testsuite"
#endif

Suite *tests_get_pool_suite(void);
Suite *tests_get_array_suite(void);
Suite *tests_get_str_suite(void);
Suite *tests_get_sets_suite(void);
Suite *tests_get_timers_suite(void);
Suite *tests_get_table_suite(void);
Suite *tests_get_var_suite(void);
Suite *tests_get_event_suite(void);
Suite *tests_get_env_suite(void);
Suite *tests_get_version_suite(void);
Suite *tests_get_feat_suite(void);
Suite *tests_get_netaddr_suite(void);
Suite *tests_get_netacl_suite(void);
Suite *tests_get_class_suite(void);
Suite *tests_get_regexp_suite(void);
Suite *tests_get_expr_suite(void);
Suite *tests_get_scoreboard_suite(void);
Suite *tests_get_modules_suite(void);

/* Temporary hack/placement for this variable, until we get to testing
 * the Signals API.
 */
unsigned int recvd_signal_flags;

extern char ServerType;
extern int ServerUseReverseDNS;
extern server_rec *main_server;
extern module *loaded_modules;
extern module *static_modules[];

#endif /* PR_TESTS_H */
