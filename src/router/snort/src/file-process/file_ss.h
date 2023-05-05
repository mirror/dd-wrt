/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************/

/**************************************************************************
 *
 * file_ss.h
 *
 * Authors: Bhargava Jandhyala <bjandhyha@cisco.com>
 *
 * Description:
 *
 * File state sharing exported functionality.
 *
 **************************************************************************/

#ifndef __FILE_SS_H__
#define __FILE_SS_H__

#ifdef SIDE_CHANNEL

#include "file_config.h"

void FilePrintSSStats(void);
void FileSSConfigInit(void);
#ifdef REG_TEST
void FilePrintSSConfig(FileSSConfig *file_ss_config);
void FileCleanSS(void);
#endif

int CreateFileSSUpdate(void **msg_handle, void **hdr_ptr, void **data_ptr, uint32_t type, uint32_t data_len);
int SendFileSSUpdate(void *msg_handle, void *hdr_ptr, void *data_ptr, uint32_t type, uint32_t data_len);

#endif /* SIDE_CHANNEL */

#endif /* __FILE_SS_H__ */
