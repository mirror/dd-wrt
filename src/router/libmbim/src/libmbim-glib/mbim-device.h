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

#ifndef _LIBMBIM_GLIB_MBIM_DEVICE_H_
#define _LIBMBIM_GLIB_MBIM_DEVICE_H_

#if !defined (__LIBMBIM_GLIB_H_INSIDE__) && !defined (LIBMBIM_GLIB_COMPILATION)
#error "Only <libmbim-glib.h> can be included directly."
#endif

#include <glib-object.h>
#include <gio/gio.h>

#include "mbim-message.h"

G_BEGIN_DECLS

#define MBIM_TYPE_DEVICE            (mbim_device_get_type ())
#define MBIM_DEVICE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MBIM_TYPE_DEVICE, MbimDevice))
#define MBIM_DEVICE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass),  MBIM_TYPE_DEVICE, MbimDeviceClass))
#define MBIM_IS_DEVICE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MBIM_TYPE_DEVICE))
#define MBIM_IS_DEVICE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass),  MBIM_TYPE_DEVICE))
#define MBIM_DEVICE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj),  MBIM_TYPE_DEVICE, MbimDeviceClass))

typedef struct _MbimDevice MbimDevice;
typedef struct _MbimDeviceClass MbimDeviceClass;
typedef struct _MbimDevicePrivate MbimDevicePrivate;

#define MBIM_DEVICE_FILE           "device-file"
#define MBIM_DEVICE_TRANSACTION_ID "device-transaction-id"
#define MBIM_DEVICE_IN_SESSION     "device-in-session"

#define MBIM_DEVICE_SIGNAL_INDICATE_STATUS "device-indicate-status"
#define MBIM_DEVICE_SIGNAL_ERROR           "device-error"
#define MBIM_DEVICE_SIGNAL_REMOVED         "device-removed"

/**
 * MbimDevice:
 *
 * The #MbimDevice structure contains private data and should only be accessed
 * using the provided API.
 */
struct _MbimDevice {
    /*< private >*/
    GObject parent;
    MbimDevicePrivate *priv;
};

struct _MbimDeviceClass {
    /*< private >*/
    GObjectClass parent;
};

GType mbim_device_get_type (void);

void        mbim_device_new        (GFile                *file,
                                    GCancellable         *cancellable,
                                    GAsyncReadyCallback   callback,
                                    gpointer              user_data);
MbimDevice *mbim_device_new_finish (GAsyncResult         *res,
                                    GError              **error);

GFile       *mbim_device_get_file         (MbimDevice *self);
GFile       *mbim_device_peek_file        (MbimDevice *self);
const gchar *mbim_device_get_path         (MbimDevice *self);
const gchar *mbim_device_get_path_display (MbimDevice *self);
gboolean     mbim_device_is_open          (MbimDevice *self);

/**
 * MbimDeviceOpenFlags:
 * @MBIM_DEVICE_OPEN_FLAGS_NONE: None.
 * @MBIM_DEVICE_OPEN_FLAGS_PROXY: Try to open the port through the 'mbim-proxy'.
 *
 * Flags to specify which actions to be performed when the device is open.
 */
typedef enum {
    MBIM_DEVICE_OPEN_FLAGS_NONE  = 0,
    MBIM_DEVICE_OPEN_FLAGS_PROXY = 1 << 0
} MbimDeviceOpenFlags;

void     mbim_device_open_full        (MbimDevice           *self,
                                       MbimDeviceOpenFlags   flags,
                                       guint                 timeout,
                                       GCancellable         *cancellable,
                                       GAsyncReadyCallback   callback,
                                       gpointer              user_data);
gboolean mbim_device_open_full_finish (MbimDevice           *self,
                                       GAsyncResult         *res,
                                       GError              **error);

void     mbim_device_open        (MbimDevice           *self,
                                  guint                 timeout,
                                  GCancellable         *cancellable,
                                  GAsyncReadyCallback   callback,
                                  gpointer              user_data);
gboolean mbim_device_open_finish (MbimDevice           *self,
                                  GAsyncResult         *res,
                                  GError              **error);

void     mbim_device_close        (MbimDevice           *self,
                                   guint                 timeout,
                                   GCancellable         *cancellable,
                                   GAsyncReadyCallback   callback,
                                   gpointer              user_data);
gboolean mbim_device_close_finish (MbimDevice           *self,
                                   GAsyncResult         *res,
                                   GError              **error);

gboolean mbim_device_close_force (MbimDevice *self,
                                  GError **error);

guint32 mbim_device_get_next_transaction_id (MbimDevice *self);

void         mbim_device_command        (MbimDevice           *self,
                                         MbimMessage          *message,
                                         guint                 timeout,
                                         GCancellable         *cancellable,
                                         GAsyncReadyCallback   callback,
                                         gpointer              user_data);
MbimMessage *mbim_device_command_finish (MbimDevice           *self,
                                         GAsyncResult         *res,
                                         GError              **error);

G_END_DECLS

#endif /* _LIBMBIM_GLIB_MBIM_DEVICE_H_ */
