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
 * Copyright (C) 2014 Aleksander Morgado <aleksander@lanedo.com>
 * Copyright (C) 2014 Smith Micro Software, Inc.
 */

#ifndef MBIM_PROXY_H
#define MBIM_PROXY_H

#include <glib-object.h>
#include <gio/gio.h>

#define MBIM_TYPE_PROXY            (mbim_proxy_get_type ())
#define MBIM_PROXY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), MBIM_TYPE_PROXY, MbimProxy))
#define MBIM_PROXY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), MBIM_TYPE_PROXY, MbimProxyClass))
#define MBIM_IS_PROXY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), MBIM_TYPE_PROXY))
#define MBIM_IS_PROXY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), MBIM_TYPE_PROXY))
#define MBIM_PROXY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), MBIM_TYPE_PROXY, MbimProxyClass))

typedef struct _MbimProxy MbimProxy;
typedef struct _MbimProxyClass MbimProxyClass;
typedef struct _MbimProxyPrivate MbimProxyPrivate;

#define MBIM_PROXY_SOCKET_PATH "mbim-proxy"

#define MBIM_PROXY_N_CLIENTS   "mbim-proxy-n-clients"
#define MBIM_PROXY_N_DEVICES   "mbim-proxy-n-devices"

struct _MbimProxy {
    GObject parent;
    MbimProxyPrivate *priv;
};

struct _MbimProxyClass {
    GObjectClass parent;
};

GType mbim_proxy_get_type (void);

MbimProxy *mbim_proxy_new           (GError **error);
guint      mbim_proxy_get_n_clients (MbimProxy *self);
guint      mbim_proxy_get_n_devices (MbimProxy *self);

#endif /* MBIM_PROXY_H */
