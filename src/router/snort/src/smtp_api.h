/*
 ** Copyright (C) 2021-2022 Cisco and/or its affiliates. All rights reserved.
 * ** This program is free software; you can redistribute it and/or modify
 * ** it under the terms of the GNU General Public License Version 2 as
 * ** published by the Free Software Foundation.  You may not use, modify or
 * ** distribute this program under any other version of the GNU General
 * ** Public License.
 * **
 * ** This program is distributed in the hope that it will be useful,
 * ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 * ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * ** GNU General Public License for more details.
 * **
 * ** You should have received a copy of the GNU General Public License
 * ** along with this program; if not, write to the Free Software
 * ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * */

/**************************************************************************/

#ifndef __SMTP_API_H__
#define __SMTP_API_H__

typedef struct _smtp_api {
    int (*smtp_session_exists)(void *data);
    int (*smtp_get_file_name)(void *data, uint8_t **buf, uint32_t *len, 
            uint32_t *type);
    int (*smtp_get_mail_from)(void *data, uint8_t **buf, uint32_t *len, 
            uint32_t *type);
    int (*smtp_get_recv_to)(void *data, uint8_t **buf, uint32_t *len, 
            uint32_t *type);
    int (*smtp_get_email_hdr)(void *data, uint8_t **buf, uint32_t *len, 
            uint32_t *type);
} SmtpAPI;
#endif 

