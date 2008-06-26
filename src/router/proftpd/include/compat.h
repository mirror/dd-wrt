/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2005-2006 The ProFTPD Project team
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
 * $Id: compat.h,v 1.6 2007/01/11 04:05:07 castaglia Exp $
 */

#ifndef PR_COMPAT_H
#define PR_COMPAT_H

/* Legacy redefines, for compatibility (for a while). */

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

#define log_openfile            pr_log_openfile
#define log_pri                 pr_log_pri
#define log_debug               pr_log_debug
#define log_auth                pr_log_auth

#define pr_parse_expression     pr_expr_create
#define pr_class_and_expression pr_expr_eval_class_and
#define pr_class_or_expression  pr_expr_eval_class_or
#define pr_group_and_expression pr_expr_eval_group_and
#define pr_group_or_expression  pr_expr_eval_group_or
#define pr_user_and_expression  pr_expr_eval_user_and
#define pr_user_or_expression   pr_expr_eval_user_or

#define add_timer               pr_timer_add
#define remove_timer            pr_timer_remove
#define reset_timer             pr_timer_reset
#define timer_sleep             pr_timer_sleep

#define DECLINED		PR_DECLINED
#define	ERROR			PR_ERROR
#define	ERROR_INT		PR_ERROR_INT
#define	ERROR_MSG		PR_ERROR_MSG
#define HANDLED			PR_HANDLED

#define make_named_sub_pool(p, s)	make_sub_pool((p))

#endif /* PR_COMPAT_H */
