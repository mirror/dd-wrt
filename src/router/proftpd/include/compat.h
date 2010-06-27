/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2005-2009 The ProFTPD Project team
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

/* Compatibility
 * $Id: compat.h,v 1.13 2009/03/10 16:59:23 castaglia Exp $
 */

#ifndef PR_COMPAT_H
#define PR_COMPAT_H

/* Legacy redefines, for compatibility (for a while). */

/* The following macros all first appeared in 1.3.0rc2. */
#define USE_AUTO_SHADOW		PR_USE_AUTO_SHADOW
#define USE_CTRLS		PR_USE_CTRLS
#define USE_CURSES		PR_USE_CURSES
#define USE_DEVEL		PR_USE_DEVEL
#define USE_GETADDRINFO		PR_USE_GETADDRINFO
#define USE_GETNAMEINFO		PR_USE_GETNAMEINFO
#define USE_IPV6		PR_USE_IPV6
#define USE_LARGEFILES		PR_USE_LARGEFILES

#define LOG_WRITEABLE_DIR	PR_LOG_WRITABLE_DIR
#define LOG_SYMLINK		PR_LOG_SYMLINK

#define pr_parse_expression     pr_expr_create
#define pr_class_and_expression pr_expr_eval_class_and
#define pr_class_or_expression  pr_expr_eval_class_or
#define pr_group_and_expression pr_expr_eval_group_and
#define pr_group_or_expression  pr_expr_eval_group_or
#define pr_user_and_expression  pr_expr_eval_user_and
#define pr_user_or_expression   pr_expr_eval_user_or

/* The following macros first appeared in 1.3.1rc1. */
#define DECLINED		PR_DECLINED
#define	ERROR			PR_ERROR
#define	ERROR_INT		PR_ERROR_INT
#define	ERROR_MSG		PR_ERROR_MSG
#define HANDLED			PR_HANDLED

/* The following macros first appeared in 1.3.1rc2. */
#define add_timer               pr_timer_add
#define remove_timer            pr_timer_remove
#define reset_timer             pr_timer_reset
#define timer_sleep             pr_timer_sleep

#define make_named_sub_pool(p, s)	make_sub_pool((p))

/* The following macros first appeared in 1.3.2rc1. */
#define pr_scoreboard_add_entry		pr_scoreboard_entry_add
#define pr_scoreboard_del_entry		pr_scoreboard_entry_del
#define pr_scoreboard_read_entry	pr_scoreboard_entry_read
#define pr_scoreboard_update_entry	pr_scoreboard_entry_update

/* The following macros first appeared in 1.3.2rc2. */
#define pr_inet_validate		pr_netaddr_validate_dns_str
#define call_module			pr_module_call

/* The following macros first appeared in 1.3.3rc1. */
#define pr_inet_copy_connection		pr_inet_copy_conn
#define pr_inet_create_connection	pr_inet_create_conn
#define pr_inet_create_conn_portrange	pr_inet_create_conn_portrange

#define ctrls_check_acl			pr_ctrls_check_acl
#define ctrls_check_group_acl		pr_ctrls_check_group_acl
#define ctrls_check_user_acl		pr_ctrls_check_user_acl
#define ctrls_init_acl			pr_ctrls_init_acl
#define ctrls_parse_acl			pr_ctrls_parse_acl
#define ctrls_set_group_acl		pr_ctrls_set_group_acl
#define ctrls_set_module_acls		pr_ctrls_set_module_acls
#define ctrls_set_user_acl		pr_ctrls_set_user_acl
#define ctrls_unregister_module_actions	pr_ctrls_unregister_module_actions
#define ctrls_log			pr_ctrls_log

#endif /* PR_COMPAT_H */
