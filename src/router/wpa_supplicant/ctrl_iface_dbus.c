/*
 * WPA Supplicant / dbus-based control interface
 * Copyright (c) 2006, Dan Williams <dcbw@redhat.com> and Red Hat, Inc.
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

#include <dbus/dbus.h>

#include "common.h"
#include "eloop.h"
#include "wpa.h"
#include "wpa_supplicant.h"
#include "config.h"
#include "eapol_sm.h"
#include "wpa_supplicant_i.h"
#include "ctrl_iface_dbus.h"
#include "ctrl_iface_dbus_handlers.h"
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


/**
 * wakeup_main - Attempt to wake our mainloop up
 * @data: dbus control interface private data
 *
 * Try to wake up the main eloop so it will process
 * dbus events that may have happened.
 */
static void wakeup_main(void *data)
{
	struct ctrl_iface_dbus_priv *iface = data;

	/* Use SIGPOLL to break out of the eloop select() */
	raise(SIGPOLL);
	iface->should_dispatch = 1;
}


/**
 * connection_setup_wakeup_main - Tell dbus about our wakeup_main function
 * @iface: dbus control interface private data
 * Returns: 0 on success, -1 on failure
 *
 * Register our wakeup_main handler with dbus
 */
static int connection_setup_wakeup_main(struct ctrl_iface_dbus_priv *iface)
{
	if (eloop_register_signal(SIGPOLL, process_wakeup_main, iface))
		return -1;

	dbus_connection_set_wakeup_main_function(iface->con, wakeup_main,
						 iface, NULL);

	return 0;
}


/**
 * get_iface_from_object_path - Decompose an object path
 * @path: the dbus object path
 * @network: (out) the configured network this object path refers to, if any
 * @bssid: (out) the scanned bssid this object path refers to, if any
 * Returns: the network interface this object path refers to
 *
 * Convenience function to create and return an invalid bssid error
 */
static char * get_iface_from_object_path(const char *path, char **network,
					 char **bssid)
{
	const unsigned int dev_path_prefix_len =
		strlen(WPAS_INTERFACES_DBUS_PATH "/");
	const char *path_iface_name;
	char *iface_name;
	char *next_sep;

	/* Be a bit paranoid about path */
	if (!path || strncmp(path, WPAS_INTERFACES_DBUS_PATH "/",
			     dev_path_prefix_len))
		return NULL;

	/* Ensure there's something at the end of the path */
	path_iface_name = path + dev_path_prefix_len;
	if (path_iface_name[0] == '\0')
		return NULL;

	/* Extract the interface name and possibly a network name too. */
	iface_name = strdup(path_iface_name);
	if (iface_name == NULL)
		return NULL;
	next_sep = strchr(iface_name, '/');
	if (next_sep != NULL) {
		const char *net_part = strstr(next_sep,
					      INTERFACES_PATH_NETWORKS_PART);
		const char *bssid_part = strstr(next_sep,
						INTERFACES_PATH_BSSIDS_PART);

		if (network && net_part) {
			/* Deal with a request for a configured network */
			const char *net_name = net_part +
				strlen(INTERFACES_PATH_NETWORKS_PART);
			*network = NULL;
			if (strlen(net_name))
				*network = strdup(net_name);
		} else if (bssid && bssid_part) {
			/* Deal with a request for a scanned BSSID */
			const char *bssid_name = bssid_part +
				strlen(INTERFACES_PATH_BSSIDS_PART);
			if (strlen(bssid_name))
				*bssid = strdup(bssid_name);
			else
				*bssid = NULL;
		}

		/* Cut off interface name before "/" */
		*next_sep = '\0';
	}

	return iface_name;
}


/**
 * wpas_dbus_new_invalid_network_error - Return a new invalid network error message
 * @message: Pointer to incoming dbus message this error refers to
 * Returns: a dbus error message
 *
 * Convenience function to create and return an invalid network error
 */
static DBusMessage * wpas_dbus_new_invalid_network_error(DBusMessage *message)
{
	return dbus_message_new_error(message, WPAS_ERROR_INVALID_NETWORK,
				      "The network requested was invalid.");
}


/**
 * wpas_dbus_new_invalid_bssid_error - Return a new invalid bssid error message
 * @message: Pointer to incoming dbus message this error refers to
 * Returns: a dbus error message
 *
 * Convenience function to create and return an invalid bssid error
 */
static DBusMessage * wpas_dbus_new_invalid_bssid_error(DBusMessage *message)
{
	return dbus_message_new_error(message, WPAS_ERROR_INVALID_BSSID,
				      "The BSSID requested was invalid.");
}


/**
 * wpas_dispatch_network_method - dispatch messages for configured networks
 * @message: the incoming dbus message
 * @wpa_s: a network interface's data
 * @network_id: id of the configured network we're interested in
 * Returns: a reply dbus message, or a dbus error message
 *
 * This function dispatches all incoming dbus messages for configured networks.
 */
static DBusMessage * wpas_dispatch_network_method(DBusMessage *message,
						  struct wpa_supplicant *wpa_s,
						  int network_id)
{
	struct wpa_ssid *ssid;

	ssid = wpa_config_get_network(wpa_s->conf, network_id);
	if (ssid == NULL)
		return wpas_dbus_new_invalid_network_error(message);

	/* FIXME: do something here */

	return NULL;
}


/**
 * wpas_dispatch_bssid_method - dispatch messages for scanned networks
 * @message: the incoming dbus message
 * @wpa_s: a network interface's data
 * @bssid: bssid of the scanned network we're interested in
 * Returns: a reply dbus message, or a dbus error message
 *
 * This function dispatches all incoming dbus messages for scanned networks.
 */
static DBusMessage * wpas_dispatch_bssid_method(DBusMessage *message,
						struct wpa_supplicant *wpa_s,
						const char *bssid)
{
	DBusMessage *reply = NULL;
	const char *method = dbus_message_get_member(message);
	struct wpa_scan_result * res = NULL;
	int i;

	/* Ensure we actually have scan data */
	if (wpa_s->scan_results == NULL &&
	    wpa_supplicant_get_scan_results(wpa_s) < 0) {
		reply = wpas_dbus_new_invalid_bssid_error(message);
		goto out;
	}

	/* Find the bssid's scan data */
	for (i = 0; i < wpa_s->num_scan_results; i++) {
		struct wpa_scan_result * search_res = &wpa_s->scan_results[i];
		char mac_str[18];

		memset(mac_str, 0, sizeof(mac_str));
		snprintf(mac_str, sizeof(mac_str) - 1, WPAS_DBUS_BSSID_FORMAT,
			 MAC2STR(search_res->bssid));
		if (!strcmp(bssid, mac_str)) {
			res = search_res;
		}
	}

	if (!res) {
		reply = wpas_dbus_new_invalid_bssid_error(message);
		goto out;
	}

	/* Dispatch the method call against the scanned bssid */
	if (!strcmp(method, "properties"))
		reply = wpas_dbus_bssid_properties(message, wpa_s, res);

out:
	return reply;
}


/**
 * wpas_dispatch_iface_method - dispatch messages for interfaces or networks
 * @message: the incoming dbus message
 * @path: the target object path of the message
 * @global: %wpa_supplicant global data
 * Returns: a reply dbus message, or a dbus error message
 *
 * This function dispatches all incoming dbus messages for network interfaces,
 * or objects owned by them, such as scanned BSSIDs and configured networks.
 */
static DBusMessage * wpas_dispatch_iface_method(DBusMessage *message,
						const char *path,
						struct wpa_global *global)
{
	const char *method = dbus_message_get_member(message);
	char *network = NULL;
	char *bssid = NULL;
	char *iface_name = NULL;
	struct wpa_supplicant *wpa_s;
	DBusMessage *reply = NULL;

 	iface_name = get_iface_from_object_path(path, &network, &bssid);
 	if (!iface_name || !strlen(iface_name)) {
		reply = wpas_dbus_new_invalid_iface_error(message);
 		goto out;
	}

	/* Find the interface this message is for */
	wpa_s = wpa_supplicant_get_iface(global, iface_name);
	if (wpa_s == NULL) {
		reply = wpas_dbus_new_invalid_iface_error(message);
		goto out;
	}

	if (network) {
		/* A method for one of this interface's configured networks. */
		int nid = strtoul(network, NULL, 10);
		if (errno != EINVAL)
			reply = wpas_dispatch_network_method(message, wpa_s,
							     nid);
		else
			reply = wpas_dbus_new_invalid_network_error(message);
	} else if (bssid) {
		/* A method for one of this interface's scanned BSSIDs */
		reply = wpas_dispatch_bssid_method(message, wpa_s, bssid);
	} else {
		/* A method for an interface only. */
		if (!strcmp(method, "scan"))
			reply = wpas_dbus_iface_scan(message, wpa_s);
		else if (!strcmp(method, "scanResults"))
			reply = wpas_dbus_iface_scan_results(message, wpa_s);
		else if (!strcmp(method, "addNetwork"))
			reply = wpas_dbus_iface_add_network(message, wpa_s);
	}

out:
	free(iface_name);
	free(network);
	free(bssid);
	return reply;
}


/**
 * wpas_message_handler - dispatch incoming dbus messages
 * @connection: connection to the system message bus
 * @message: an incoming dbus message
 * @user_data: a pointer to a dbus control interface data structure
 * Returns: whether or not the message was handled
 *
 * This function dispatches all incoming dbus messages to the correct
 * handlers, depending on what the message's target object path is,
 * and what the method call is.
 */
static DBusHandlerResult wpas_message_handler(DBusConnection *connection,
	DBusMessage *message, void *user_data)
{
	struct ctrl_iface_dbus_priv *ctrl_iface = user_data;
	const char *method;
	const char *path;
	DBusMessage *reply = NULL;

	method = dbus_message_get_member(message);
	path = dbus_message_get_path(message);
	if (!method || !path || !ctrl_iface)
		return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;

	if (!strcmp(path, WPAS_DBUS_PATH)) {
		/* dispatch methods against our global dbus interface here */
		if (!strcmp(method, "addInterface")) {
			reply = wpas_dbus_global_add_interface(
				message, ctrl_iface->global);
		} else if (!strcmp(method, "removeInterface")) {
			reply = wpas_dbus_global_remove_interface(
				message, ctrl_iface->global);
		}
	} else if (!strncmp(path, WPAS_INTERFACES_DBUS_PATH "/",
			    strlen(WPAS_INTERFACES_DBUS_PATH "/"))) {
		/* Call interface message handler for all other messages */
		reply = wpas_dispatch_iface_method(message, path,
						   ctrl_iface->global);
	}

	/* If the message was handled, send back the reply */
	if (reply) {
		dbus_connection_send(connection, reply, NULL);
		dbus_message_unref(reply);
	}

	return reply ? DBUS_HANDLER_RESULT_HANDLED :
		DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}


/**
 * wpa_supplicant_dbus_notify_scan_results - Send a scan results signal
 * @iface: a dbus control interface data structure
 * @wpa_s: %wpa_supplicant network interface data
 * Returns: 0 on success, -1 on failure
 *
 * Notify listeners that this interface has updated scan results.
 */
void wpa_supplicant_dbus_notify_scan_results(struct wpa_supplicant *wpa_s)
{
	struct ctrl_iface_dbus_priv *iface = wpa_s->global->dbus_ctrl_iface;
	DBusMessage *signal;
	char *path;

	path = wpa_zalloc(WPAS_DBUS_MAX_OBJECT_PATH_LEN);
	if (path == NULL) {
		perror("wpa_supplicant_dbus_notify_scan_results[dbus]: "
		       "out of memory");
		wpa_printf(MSG_ERROR, "dbus control interface: not enough "
			   "memory to send scan results signal.");
		return;
	}
	snprintf(path, WPAS_DBUS_MAX_OBJECT_PATH_LEN,
		 WPAS_INTERFACES_DBUS_PATH "/%s", wpa_s->ifname);
	signal = dbus_message_new_signal(path, WPAS_INTERFACES_DBUS_INTERFACE,
					 "ScanResultsAvailable");
	if (signal == NULL) {
		perror("wpa_supplicant_dbus_notify_scan_results[dbus]: "
		       "couldn't create dbus signal; likely out of memory");
		wpa_printf(MSG_ERROR, "dbus control interface: not enough"
			   " memory to send scan results signal.");
		goto out;
	}
	dbus_connection_send(iface->con, signal, NULL);

out:
	free(path);
}


/**
 * integrate_with_eloop - Register our mainloop integration with dbus
 * @connection: connection to the system message bus
 * @iface: a dbus control interface data structure
 * Returns: 0 on success, -1 on failure
 *
 * We register our mainloop integration functions with dbus here.
 */
static int integrate_with_eloop(DBusConnection *connection,
	struct ctrl_iface_dbus_priv *iface)
{
	if (!dbus_connection_set_watch_functions(connection, add_watch,
						 remove_watch, watch_toggled,
						 iface, NULL)) {
		perror("dbus_connection_set_watch_functions[dbus]");
		wpa_printf(MSG_ERROR, "Not enough memory to set up dbus.");
		return -1;
	}

	if (!dbus_connection_set_timeout_functions(connection, add_timeout,
						   remove_timeout,
						   timeout_toggled, iface,
						   NULL)) {
		perror("dbus_connection_set_timeout_functions[dbus]");
		wpa_printf(MSG_ERROR, "Not enough memory to set up dbus.");
		return -1;
	}

	if (connection_setup_wakeup_main(iface) < 0) {
		perror("connection_setup_wakeup_main[dbus]");
		wpa_printf(MSG_ERROR, "Could not setup main wakeup function.");
		return -1;
	}

	return 0;
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
	DBusObjectPathVTable wpas_vtable = {
		NULL, &wpas_message_handler, NULL, NULL, NULL, NULL
	};

	iface = wpa_zalloc(sizeof(struct ctrl_iface_dbus_priv));
	if (iface == NULL)
		return NULL;

	iface->global = global;

	/* Get a reference to the system bus */
	dbus_error_init(&error);
	iface->con = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
	dbus_error_free(&error);
	if (!iface->con) {
		perror("dbus_bus_get[ctrl_iface_dbus]");
		wpa_printf(MSG_ERROR, "Could not acquire the system bus.");
		goto fail;
	}

	/* Tell dbus about our mainloop integration functions */
	if (integrate_with_eloop(iface->con, iface))
		goto fail;

	/* Register our service with the message bus */
	dbus_error_init(&error);
	switch (dbus_bus_request_name(iface->con, WPAS_DBUS_SERVICE,
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

	/* Register the message handler for the global dbus interface */
	if (!dbus_connection_register_object_path(iface->con,
						  WPAS_DBUS_PATH, &wpas_vtable,
						  iface)) {
		perror("dbus_connection_register_object_path[dbus]");
		wpa_printf(MSG_ERROR, "Could not set up DBus message "
			   "handler.");
		ret = -1;
	}

	/* Register the message handler for the network-interface-specific
	 * dbus interface.
	 */
	if (!dbus_connection_register_fallback(iface->con,
					       WPAS_INTERFACES_DBUS_PATH,
					       &wpas_vtable, iface)) {
		perror("dbus_connection_register_object_path[dbus]");
		wpa_printf(MSG_ERROR, "Could not set up DBus message "
			   "handler for interfaces.");
		ret = -1;
	}

	if (ret != 0)
		goto fail;

	wpa_printf(MSG_DEBUG, "Providing DBus service '" WPAS_DBUS_SERVICE
		   "'.");

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
