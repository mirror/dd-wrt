/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * libmbim-glib -- GLib/GIO based library to control MBIM devices
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 * Copyright (C) 2013 - 2014 Aleksander Morgado <aleksander@aleksander.es>
 */

#ifndef _LIBMBIM_GLIB_H_
#define _LIBMBIM_GLIB_H_

#define __LIBMBIM_GLIB_H_INSIDE__

/* libmbim-glib headers */

#include "mbim-version.h"
#include "mbim-utils.h"
#include "mbim-uuid.h"
#include "mbim-cid.h"
#include "mbim-message.h"
#include "mbim-device.h"
#include "mbim-enums.h"
#include "mbim-proxy.h"

/* generated */
#include "mbim-enum-types.h"
#include "mbim-error-types.h"
#include "mbim-basic-connect.h"
#include "mbim-sms.h"
#include "mbim-ussd.h"
#include "mbim-auth.h"
#include "mbim-phonebook.h"
#include "mbim-stk.h"
#include "mbim-dss.h"
#include "mbim-ms-firmware-id.h"
#include "mbim-ms-host-shutdown.h"
#include "mbim-qmi.h"
#include "mbim-atds.h"

/* backwards compatibility */
#include "mbim-compat.h"

#endif /* _LIBMBIM_GLIB_H_ */
