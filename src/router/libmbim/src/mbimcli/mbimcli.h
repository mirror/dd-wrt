/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * mbimcli -- Command line interface to control MBIM devices
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Copyright (C) 2013 - 2014 Aleksander Morgado <aleksander@aleksander.es>
 */

#include <glib.h>

#ifndef __MBIMCLI_H__
#define __MBIMCLI_H__

#define VALIDATE_UNKNOWN(str) ((str) ? (str) : "unknown")

/* Common */
void mbimcli_async_operation_done (gboolean operation_status);

/* Basic Connect group */
GOptionGroup *mbimcli_basic_connect_get_option_group    (void);
GOptionGroup *mbimcli_phonebook_get_option_group        (void);
GOptionGroup *mbimcli_dss_get_option_group              (void);
GOptionGroup *mbimcli_ms_firmware_id_get_option_group   (void);
GOptionGroup *mbimcli_ms_host_shutdown_get_option_group (void);
GOptionGroup *mbimcli_atds_get_option_group             (void);

gboolean      mbimcli_basic_connect_options_enabled     (void);
gboolean      mbimcli_phonebook_options_enabled         (void);
gboolean      mbimcli_dss_options_enabled               (void);
gboolean      mbimcli_ms_firmware_id_options_enabled    (void);
gboolean      mbimcli_ms_host_shutdown_options_enabled  (void);
gboolean      mbimcli_atds_options_enabled              (void);

void          mbimcli_basic_connect_run                 (MbimDevice *device,
                                                         GCancellable *cancellable);
void          mbimcli_phonebook_run                     (MbimDevice *device,
                                                         GCancellable *cancellable);
void          mbimcli_dss_run                           (MbimDevice *device,
                                                         GCancellable *cancellable);
void          mbimcli_ms_firmware_id_run                (MbimDevice *device,
                                                         GCancellable *cancellable);
void          mbimcli_ms_host_shutdown_run              (MbimDevice *device,
                                                         GCancellable *cancellable);
void          mbimcli_atds_run                          (MbimDevice *device,
                                                         GCancellable *cancellable);

#endif /* __MBIMCLI_H__ */
