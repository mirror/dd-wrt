/*
 * ProFTPD - FTP server daemon
 * Copyright (c) 2020-2022 The ProFTPD Project team
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
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335, USA.
 *
 * As a special exemption, Public Flood Software/MacGyver aka Habeeb J. Dihu
 * and other respective copyright holders give permission to link this program
 * with OpenSSL, and distribute the resulting executable, without including
 * the source code for OpenSSL in the source distribution.
 */

#ifndef PR_VERSION_H
#define PR_VERSION_H

#include "buildstamp.h"

/* Application version (in various forms) */
#define PROFTPD_VERSION_NUMBER		0x0001030805
#define PROFTPD_VERSION_TEXT		"1.3.8"

/* Module API version */
#define PR_MODULE_API_VERSION		0x20

unsigned long pr_version_get_module_api_number(void);
unsigned long pr_version_get_number(void);
const char *pr_version_get_str(void);

/* PR_STATUS is reported by --version-status -- don't ask why */
#define PR_STATUS          		"(stable)"

#endif /* PR_VERSION_H */
