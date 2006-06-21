/*
 * WPA Supplicant / dbus-based control interface
 * Copyright (c) 2006, Dan Williams <dcbw@redhat.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, this software may be distributed under the terms of BSD
 * license.
 *
 * See README and COPYING for more details.
 */

#include "includes.h"

#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>

#include "common.h"
#include "eloop.h"
#include "wpa.h"
#include "wpa_supplicant.h"
#include "config.h"
#include "eapol_sm.h"
#include "wpa_supplicant_i.h"
#include "ctrl_iface_dbus.h"
#include "l2_packet.h"
#include "preauth.h"
#include "wpa_ctrl.h"
#include "eap.h"

struct ctrl_iface_dbus_priv {
	DBusConnection *con;
	int should_dispatch;
	struct wpa_global *global;
};


static void process_watch(struct ctrl_iface_dbus_priv *iface,
			  DBusWatch *watch, eloop_event_type type)
{
	dbus_connection_ref(iface->con);

	iface->should_dispatch = 0;

	if (type == EVENT_TYPE_READ)
		dbus_watch_handle(watch, DBUS_WATCH_READABLE);
	else if (type == EVENT_TYPE_WRITE)
		dbus_watch_handle(watch, DBUS_WATCH_WRITABLE);
	else if (type == EVENT_TYPE_EXCEPTION)
		dbus_watch_handle(watch, DBUS_WATCH_ERROR);

	if (iface->should_dispatch) {
		while (dbus_connection_get_dispatch_status(iface->con) ==
		       DBUS_DISPATCH_DATA_REMAINS)
			dbus_connection_dispatch(iface->con);
		iface->should_dispatch = 0;
	}

	dbus_connection_unref(iface->con);
}


static void process_watch_exception(int sock, void *eloop_ctx, void *sock_ctx)
{
	process_watch(eloop_ctx, sock_ctx, EVENT_TYPE_EXCEPTION);
}


static void process_watch_read(int sock, void *eloop_ctx, void *sock_ctx)
{
	process_watch(eloop_ctx, sock_ctx, EVENT_TYPE_READ);
}


static void process_watch_write(int sock, void *eloop_ctx, void *sock_ctx)
{
	process_watch(eloop_ctx, sock_ctx, EVENT_TYPE_WRITE);
}


static void connection_setup_add_watch(struct ctrl_iface_dbus_priv *iface,
				       DBusWatch *watch)
{
	unsigned int flags;
	int fd;

	if (!dbus_watch_get_enabled(watch))
		return;

	flags = dbus_watch_get_flags(watch);
	fd = dbus_watch_get_fd(watch);

	eloop_register_sock(fd, EVENT_TYPE_EXCEPTION, process_watch_exception,
			    iface, watch);

	if (flags & DBUS_WATCH_READABLE) {
		eloop_register_sock(fd, EVENT_TYPE_READ, process_watch_read,
				    iface, watch);
	}
	if (flags & DBUS_WATCH_WRITABLE) {
		eloop_register_sock(fd, EVENT_TYPE_WRITE, process_watch_write,
				    iface, watch);
	}

	dbus_watch_set_data(watch, iface, NULL);
}


static void connection_setup_remove_watch(struct ctrl_iface_dbus_priv *iface,
					  DBusWatch *watch)
{
	unsigned int flags;
	int fd;

	flags = dbus_watch_get_flags(watch);
	fd = dbus_watch_get_fd(watch);

	eloop_unregister_sock(fd, EVENT_TYPE_EXCEPTION);

	if (flags & DBUS_WATCH_READABLE)
		eloop_unregister_sock(fd, EVENT_TYPE_READ);
	if (flags & DBUS_WATCH_WRITABLE)
		eloop_unregister_sock(fd, EVENT_TYPE_WRITE);

	dbus_watch_set_data(watch, NULL, NULL);
}


static dbus_bool_t add_watch(DBusWatch *watch, void *data)
{
	connection_setup_add_watch(data, watch);
	return TRUE;
}


static void remove_watch(DBusWatch *watch, void *data)
{
	connection_setup_remove_watch(data, watch);
}


static void watch_toggled(DBusWatch *watch, void *data)
{
	if (dbus_watch_get_enabled(watch))
		add_watch(watch, data);
	else
		remove_watch(watch, data);
}


static void process_timeout(void *eloop_ctx, void *sock_ctx)
{
	DBusTimeout *timeout = sock_ctx;

	dbus_timeout_handle(timeout);
}


static void connection_setup_add_timeout(struct ctrl_iface_dbus_priv *iface,
					 DBusTimeout *timeout)
{
	if (!dbus_timeout_get_enabled(timeout))
		return;

	eloop_register_timeout(0, dbus_timeout_get_interval(timeout) * 1000,
			       process_timeout, iface, timeout);

	dbus_timeout_set_data(timeout, iface, NULL);
}


static void connection_setup_remove_timeout(struct ctrl_iface_dbus_priv *iface,
					    DBusTimeout *timeout)
{
	eloop_cancel_timeout(process_timeout, iface, timeout);
	dbus_timeout_set_data(timeout, NULL, NULL);
}


static dbus_bool_t add_timeout(DBusTimeout *timeout, void *data)
{
	if (!dbus_timeout_get_enabled(timeout))
		return TRUE;

	connection_setup_add_timeout(data, timeout);

	return TRUE;
}


static void remove_timeout(DBusTimeout *timeout, void *data)
{
	connection_setup_remove_timeout(data, timeout);
}


static void timeout_toggled(DBusTimeout *timeout, void *data)
{
	if (dbus_timeout_get_enabled(timeout))
		add_timeout(timeout, data);
	else
		remove_timeout(timeout, data);
}


static void process_wakeup_main(int sig, void *eloop_ctx, void *signal_ctx)
{
	struct ctrl_iface_dbus_priv *iface = signal_ctx;

	if (sig != SIGPOLL || !iface->con)
		return;

	if (dbus_connection_get_dispatch_status(iface->con) !=
	    DBUS_DISPATCH_DATA_REMAINS)
		return;

	/* Only dispatch once - we do not want to starve other events */
	dbus_connection_ref(iface->con);
	dbus_connection_dispatch(iface->con);
	dbus_connection_unref(iface->con);
}


static void wakeup_main(void *data)
{
	struct ctrl_iface_dbus_priv *iface = data;

	/* Use SIGPOLL to break out of the eloop select() */
	raise(SIGPOLL);
	iface->should_dispatch = 1;
}


static int connection_setup_wakeup_main(struct ctrl_iface_dbus_priv *iface)
{
	if (eloop_register_signal(SIGPOLL, process_wakeup_main, iface))
		return -1;

	dbus_connection_set_wakeup_main_function(iface->con, wakeup_main,
						 iface, NULL);

	return 0;
}


static DBusHandlerResult dbus_message_handler(DBusConnection *connection,
					      DBusMessage *message,
					      void *user_data)
{
	const char *method;
	const char *path;
	dbus_bool_t handled = TRUE;

	method = dbus_message_get_member(message);
	path = dbus_message_get_path(message);

	wpa_printf(MSG_ERROR, "dbus_message_handler() got method %s for path "
		   "%s", method, path);

	return handled ? DBUS_HANDLER_RESULT_HANDLED :
		DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}


/**
 * wpa_supplicant_dbus_ctrl_iface_init - Initialize dbus control interface
 * @global: Pointer to global data from wpa_supplicant_init()
 * Returns: Pointer to dbus_ctrl_iface date or %NULL on failure
 *
 * Initialize the dbus control interface and start receiving commands from
 * external programs over the bus.
 */
struct ctrl_iface_dbus_priv *
wpa_supplicant_dbus_ctrl_iface_init(struct wpa_global *global)
{
	struct ctrl_iface_dbus_priv *iface;
	DBusError error;
	int ret = -1;
	DBusObjectPathVTable vtable = {
		NULL, &dbus_message_handler, NULL, NULL, NULL, NULL
	};

	iface = wpa_zalloc(sizeof(struct ctrl_iface_dbus_priv));
	if (iface == NULL)
		return NULL;

	iface->global = global;

	dbus_error_init(&error);
	iface->con = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
	dbus_error_free(&error);
	if (!iface->con) {
		perror("dbus_bus_get[ctrl_iface_dbus]");
		wpa_printf(MSG_ERROR, "Could not acquire the system bus.");
		goto fail;
	}

	if (!dbus_connection_set_watch_functions(iface->con, add_watch,
						 remove_watch, watch_toggled,
						 iface, NULL)) {
		perror("dbus_connection_set_watch_functions[dbus]");
		wpa_printf(MSG_ERROR, "Not enough memory to set up dbus.");
		goto fail;
	}

	if (!dbus_connection_set_timeout_functions(iface->con, add_timeout,
						   remove_timeout,
						   timeout_toggled, iface,
						   NULL)) {
		perror("dbus_connection_set_timeout_functions[dbus]");
		wpa_printf(MSG_ERROR, "Not enough memory to set up dbus.");
		goto fail;
	}

	if (connection_setup_wakeup_main(iface) < 0) {
		perror("connection_setup_wakeup_main[dbus]");
		wpa_printf(MSG_ERROR, "Could not setup main wakeup function.");
		goto fail;
	}

	dbus_error_init(&error);
	switch (dbus_bus_request_name(iface->con, WPA_SUPPLICANT_DBUS_SERVICE,
				      0, &error)) {
	case DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER:
		ret = 0;
		break;
	case DBUS_REQUEST_NAME_REPLY_EXISTS:
	case DBUS_REQUEST_NAME_REPLY_IN_QUEUE:
	case DBUS_REQUEST_NAME_REPLY_ALREADY_OWNER:
		perror("dbus_bus_request_name[dbus]");
		wpa_printf(MSG_ERROR, "Could not request DBus service name: "
			   "already registered.");
		break;
	default:
		perror("dbus_bus_request_name[dbus]");
		wpa_printf(MSG_ERROR, "Could not request DBus service name: "
			   "%s %s.", error.name, error.message);
		break;
	}
	dbus_error_free(&error);

	if (!dbus_connection_register_object_path(iface->con,
			WPA_SUPPLICANT_DBUS_PATH, &vtable, iface)) {
		perror("dbus_connection_register_object_path[dbus]");
		wpa_printf(MSG_ERROR, "Could not set up DBus message "
			   "handler.");
		ret = -1;
	}

	if (ret != 0)
		goto fail;

	wpa_printf(MSG_DEBUG, "Providing DBus service '"
		   WPA_SUPPLICANT_DBUS_SERVICE "'.");

	return iface;

fail:
	wpa_supplicant_dbus_ctrl_iface_deinit(iface);
	return NULL;
}


/**
 * wpa_supplicant_dbus_ctrl_iface_deinit - Deinitialize dbus ctrl interface
 * @iface: Pointer to dbus private data from
 * wpa_supplicant_dbus_ctrl_iface_init()
 *
 * Deinitialize the dbus control interface that was initialized with
 * wpa_supplicant_dbus_ctrl_iface_init().
 */
void wpa_supplicant_dbus_ctrl_iface_deinit(struct ctrl_iface_dbus_priv *iface)
{
	if (iface == NULL)
		return;

	if (iface->con) {
		dbus_connection_set_watch_functions(iface->con, NULL, NULL,
						    NULL, NULL, NULL);
		dbus_connection_set_timeout_functions(iface->con, NULL, NULL,
						      NULL, NULL, NULL);
		dbus_connection_close(iface->con);
	}

	memset(iface, 0, sizeof(struct ctrl_iface_dbus_priv));
	free(iface);
}
