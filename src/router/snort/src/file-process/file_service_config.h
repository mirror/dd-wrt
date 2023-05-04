/*
**  Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
**  Copyright (C) 2012-2013 Sourcefire, Inc.
**
**  This program is free software; you can redistribute it and/or modify
**  it under the terms of the GNU General Public License Version 2 as
**  published by the Free Software Foundation.  You may not use, modify or
**  distribute this program under any other version of the GNU General
**  Public License.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program; if not, write to the Free Software
**  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
**
**  Author(s):  Hui Cao <hcao@sourcefire.com>
**
**  NOTES
**  5.25.2012 - Initial Source Code. Hcao
*/

#ifndef __FILE_SERVICE_CONFIG_H__
#define __FILE_SERVICE_CONFIG_H__

/* Default file type/signature/capture values.
 */

/* configure file services
 *
 * Args:
 *   struct _SnortConfig* sc: the snort configuration
 *   char *args: configuration string
 *   void *file_config: pointer to file config
 */
struct _SnortConfig;
void file_service_config(struct _SnortConfig* sc, char *args, void *file_config);

/* Create file service configuration and set default values
 *
 * Return:
 *   void *: pointer to file configuration
 */
struct _fileConfig;
struct _fileConfig* file_service_config_create(void);

# ifdef SNORT_RELOAD
/* Verify whether file configuration is valid
 * changing memory settings and depth settings
 * requires snort restart
 * 
 * Return
 *    0: valid
 *   -1: invalid
 */
int  file_sevice_config_verify(SnortConfig *old, SnortConfig *new);

# endif /* ifdef SNORT_RELOAD */
#endif /* #ifndef __FILE_SERVICE_CONFIG_H__ */

