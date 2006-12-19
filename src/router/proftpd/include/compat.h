/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2005 The ProFTPD Project team
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
 * $Id: compat.h,v 1.1 2006/04/24 11:39:28 honor Exp $
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

#define auth_setpwent           pr_auth_setpwent
#define auth_endpwent           pr_auth_endpwent
#define auth_setgrent           pr_auth_setgrent
#define auth_endgrent           pr_auth_endgrent
#define auth_getpwent           pr_auth_getpwent
#define auth_getgrent           pr_auth_getgrent
#define auth_getpwnam           pr_auth_getpwnam
#define auth_getpwuid           pr_auth_getpwuid
#define auth_getgrnam           pr_auth_getgrnam
#define auth_getgrgid           pr_auth_getgrgid
#define auth_authenticate       pr_auth_authenticate
#define auth_check              pr_auth_check
#define auth_uid2name           pr_auth_uid2name
#define auth_gid2name           pr_auth_gid2name
#define auth_name2uid           pr_auth_name2uid
#define auth_name2gid           pr_auth_name2gid
#define auth_getgroups          pr_auth_getgroups

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

#endif /* PR_COMPAT_H */
