/*
 **
 **
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
 **  5.25.12 - Initial Source Code. Hcao
 */

#ifndef __FILE_SERVICE_H__
#define __FILE_SERVICE_H__

#include "file_service_config.h"
#include "file_lib.h"

typedef struct _FileServiceConfig
{
    File_policy_callback_func file_policy_cb;
    File_type_callback_func  file_type_cb;
    File_signature_callback_func file_signature_cb;
    Log_file_action_func log_file_action;
    bool file_type_id_enabled;
    bool file_signature_enabled;
    bool file_capture_enabled;
}FileServiceConfig;

/* Initialize file API, this must be called when snort restarts */
void init_fileAPI(void);

/* Free file configuration, this must be called when snort reloads/restarts*/
void free_file_config(void*);

/* Swap the new config with current config */
void FileServiceInstall(void);

/* Close file API, this must be called when snort exits */
void close_fileAPI(void);

/* Get current file context */
FileContext* get_current_file_context(void *ssnptr);

/* Check and set the sidechannel to file cache sync */
#if defined (SIDE_CHANNEL)
void check_sidechannel_enabled(void *file_config);
#ifdef REG_TEST
void FileSSConfigFree(void *file_config);
#endif
#endif
#endif
