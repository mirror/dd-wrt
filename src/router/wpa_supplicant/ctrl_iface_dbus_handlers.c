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
#include "config.h"
#include "wpa_supplicant_i.h"
#include "ctrl_iface_dbus.h"
#include "ctrl_iface_dbus_handlers.h"
#include "l2_packet.h"
#include "dbus_dict_helpers.h"


/**
 * wpas_dbus_new_invalid_opts_error - Return a new invalid options error message
 * @message: Pointer to incoming dbus message this error refers to
 * Returns: a dbus error message
 *
 * Convenience function to create and return an invalid options error
 */
static DBusMessage * wpas_dbus_new_invalid_opts_error(DBusMessage *message)
{
	return dbus_message_new_error(message, WPAS_ERROR_INVALID_OPTS,
				      "Did not receive correct message "
				      "arguments.");
}


/**
 * wpas_dbus_new_invalid_iface_error - Return a new invalid interface error message
 * @message: Pointer to incoming dbus message this error refers to
 * Returns: a dbus error message
 *
 * Convenience function to create and return an invalid interface error
 */
DBusMessage * wpas_dbus_new_invalid_iface_error(DBusMessage *message)
{
	return dbus_message_new_error(message, WPAS_ERROR_INVALID_IFACE,
				      "wpa_supplicant knows nothing about "
				      "this interface.");
}


/**
 * wpas_dbus_new_success_reply - Return a new success reply message
 * @message: Pointer to incoming dbus message this reply refers to
 * Returns: a dbus message containing a single UINT32 that indicates
 *          success (ie, a value of 1)
 *
 * Convenience function to create and return a success reply message
 */
static DBusMessage * wpas_dbus_new_success_reply(DBusMessage *message)
{
	DBusMessage *reply;
	unsigned int success = 1;

	reply = dbus_message_new_method_return(message);
	dbus_message_append_args(message, DBUS_TYPE_UINT32, &success,
				 DBUS_TYPE_INVALID);
	return reply;
}


static void wpas_dbus_free_wpa_interface(struct wpa_interface *iface)
{
	free((char *) iface->driver);
	free((char *) iface->driver_param);
	free((char *) iface->confname);
	free((char *) iface->bridge_ifname);
}


/**
 * wpas_dbus_global_add_interface - Request registration of a network interface
 * @message: Pointer to incoming dbus message
 * @global: wpa_supplicant global data structure
 * Returns: a dbus message containing a UINT32 indicating success (1) or
 *          failure (0), or returns a dbus error message with more information
 *
 * Handler function for "addInterface" method call. Handles requests
 * by dbus clients to register a network interface that wpa_supplicant
 * will manage.
 */
DBusMessage * wpas_dbus_global_add_interface(DBusMessage *message,
					     struct wpa_global *global)
{
	struct wpa_interface iface;
	char *ifname = NULL;
	DBusMessage *reply = NULL;
	DBusMessageIter iter;

	memset(&iface, 0, sizeof(iface));

	dbus_message_iter_init(message, &iter);

	/* First argument: interface name (DBUS_TYPE_STRING)
	 *    Required; must be non-zero length
	 */
	if (dbus_message_iter_get_arg_type(&iter) != DBUS_TYPE_STRING)
		goto error;
	dbus_message_iter_get_basic(&iter, &ifname);
	if (!strlen(ifname))
		goto error;
	iface.ifname = ifname;

	/* Second argument: dict of options */
	if (dbus_message_iter_next(&iter)) {
		DBusMessageIter iter_dict;
		struct wpa_dbus_dict_entry entry;

		if (!wpa_dbus_dict_open_read(&iter, &iter_dict))
			goto error;
		while (wpa_dbus_dict_has_dict_entry(&iter_dict)) {
			if (!wpa_dbus_dict_get_entry(&iter_dict, &entry))
				goto error;
			if (!strcmp(entry.key, "driver") &&
			    (entry.type == DBUS_TYPE_STRING)) {
				iface.driver = strdup(entry.str_value);
				if (iface.driver == NULL)
					goto error;
			} else if (!strcmp(entry.key, "driver-params") &&
				   (entry.type == DBUS_TYPE_STRING)) {
				iface.driver_param = strdup(entry.str_value);
				if (iface.driver_param == NULL)
					goto error;
			} else if (!strcmp(entry.key, "config-file") &&
				   (entry.type == DBUS_TYPE_STRING)) {
				iface.confname = strdup(entry.str_value);
				if (iface.confname == NULL)
					goto error;
			} else if (!strcmp(entry.key, "bridge-ifname") &&
				   (entry.type == DBUS_TYPE_STRING)) {
				iface.bridge_ifname = strdup(entry.str_value);
				if (iface.bridge_ifname == NULL)
					goto error;
			} else {
				wpa_dbus_dict_entry_clear(&entry);
				goto error;
			}
			wpa_dbus_dict_entry_clear(&entry);
		}
	}

	/*
	 * Try to get the wpa_supplicant record for this iface, return
	 * an error if we already control it.
	 */
	if (wpa_supplicant_get_iface(global, iface.ifname) != 0) {
		reply = dbus_message_new_error(message,
					       WPAS_ERROR_EXISTS_ERROR,
					       "wpa_supplicant already "
					       "controls this interface.");
	} else {
		/* Otherwise, have wpa_supplicant attach to it. */
		if (wpa_supplicant_add_iface(global, &iface)) {
			reply = wpas_dbus_new_success_reply(message);
		} else {
			reply = dbus_message_new_error(message,
						       WPAS_ERROR_ADD_ERROR,
						       "wpa_supplicant "
						       "couldn't grab this "
						       "interface.");
		}
	}
	wpas_dbus_free_wpa_interface(&iface);
	return reply;

error:
	wpas_dbus_free_wpa_interface(&iface);
	return wpas_dbus_new_invalid_opts_error(message);
}


/**
 * wpas_dbus_global_remove_interface - Request deregistration of an interface
 * @message: Pointer to incoming dbus message
 * @global: wpa_supplicant global data structure
 * Returns: a dbus message containing a UINT32 indicating success (1) or
 *          failure (0), or returns a dbus error message with more information
 *
 * Handler function for "removeInterface" method call.  Handles requests
 * by dbus clients to deregister a network interface that wpa_supplicant
 * currently manages.
 */
DBusMessage * wpas_dbus_global_remove_interface(DBusMessage *message,
						struct wpa_global *global)
{
	struct wpa_supplicant *wpa_s;
	char *iface_name;
	DBusMessage *reply = NULL;

	if (!dbus_message_get_args(message, NULL,
				   DBUS_TYPE_STRING, &iface_name,
				   DBUS_TYPE_INVALID)) {
		reply = wpas_dbus_new_invalid_opts_error(message);
		goto out;
	}

	wpa_s = wpa_supplicant_get_iface(global, iface_name);
	if (wpa_s == NULL) {
		reply = wpas_dbus_new_invalid_iface_error(message);
		goto out;
	}

	if (wpa_supplicant_remove_iface(global, wpa_s)) {
		reply = wpas_dbus_new_success_reply(message);
	} else {
		reply = dbus_message_new_error(message,
					       WPAS_ERROR_REMOVE_ERROR,
					       "wpa_supplicant couldn't "
					       "remove this interface.");
	}

out:
	return reply;
}


/**
 * wpas_dbus_iface_scan - Request a wireless scan on an interface
 * @message: Pointer to incoming dbus message
 * @wpa_s: wpa_supplicant structure for a network interface
 * Returns: a dbus message containing a UINT32 indicating success (1) or
 *          failure (0)
 *
 * Handler function for "scan" method call of a network device. Requests
 * that wpa_supplicant perform a wireless scan as soon as possible
 * on a particular wireless interface.
 */
DBusMessage * wpas_dbus_iface_scan(DBusMessage *message,
				   struct wpa_supplicant *wpa_s)
{
	wpa_s->scan_req = 2;
	wpa_supplicant_req_scan(wpa_s, 0, 0);
	return wpas_dbus_new_success_reply(message);
}


/**
 * wpas_dbus_iface_scan_results - Get the results of a recent scan request
 * @message: Pointer to incoming dbus message
 * @wpa_s: wpa_supplicant structure for a network interface
 * Returns: a dbus message containing a dbus array of objects paths, or returns
 *          a dbus error message if not scan results could be found
 *
 * Handler function for "scanResults" method call of a network device. Returns
 * a dbus message containing the object paths of wireless networks found.
 */
DBusMessage * wpas_dbus_iface_scan_results(DBusMessage *message,
					   struct wpa_supplicant *wpa_s)
{
	DBusMessage *reply = NULL;
	DBusMessageIter iter;
	DBusMessageIter sub_iter;
	int i;

	/* Ensure we've actually got scan results to return */
	if (wpa_s->scan_results == NULL &&
	    wpa_supplicant_get_scan_results(wpa_s) < 0) {
		reply = dbus_message_new_error(message, WPAS_ERROR_SCAN_ERROR,
					       "An error ocurred getting scan "
					       "results.");
		goto out;
	}

	/* Create and initialize the return message */
	reply = dbus_message_new_method_return(message);
	dbus_message_iter_init_append(reply, &iter);
	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
					 DBUS_TYPE_OBJECT_PATH_AS_STRING,
					 &sub_iter);

	/* Loop through scan results and append each result's object path */
	for (i = 0; i < wpa_s->num_scan_results; i++) {
		struct wpa_scan_result *res = &wpa_s->scan_results[i];
		char *path;

		path = wpa_zalloc(WPAS_DBUS_MAX_OBJECT_PATH_LEN);
		if (path == NULL) {
			perror("wpas_dbus_iface_scan_results[dbus]: out of "
			       "memory.");
			wpa_printf(MSG_ERROR, "dbus control interface: not "
				   "enough memory to send scan results "
				   "signal.");
			break;
		}
		/* Construct the object path for this network.  Note that ':'
		 * is not a valid character in dbus object paths.
		 */
		snprintf(path, WPAS_DBUS_MAX_OBJECT_PATH_LEN,
			 WPAS_INTERFACES_DBUS_PATH "/%s/BSSIDs/"
			 WPAS_DBUS_BSSID_FORMAT,
			 wpa_s->ifname, MAC2STR(res->bssid));
		dbus_message_iter_append_basic(&sub_iter,
					       DBUS_TYPE_OBJECT_PATH, &path);
		free(path);
	}

	dbus_message_iter_close_container(&iter, &sub_iter);

out:
	return reply;
}


/**
 * wpas_dbus_bssid_properties - Return the properties of a scanned network
 * @message: Pointer to incoming dbus message
 * @wpa_s: wpa_supplicant structure for a network interface
 * @res: wpa_supplicant scan result for which to get properties
 * Returns: a dbus message containing the properties for the requested network
 *
 * Handler function for "properties" method call of a scanned network.
 * Returns a dbus message containing the the properties.
 */
DBusMessage * wpas_dbus_bssid_properties(DBusMessage *message,
					 struct wpa_supplicant *wpa_s,
					 struct wpa_scan_result *res)
{
	DBusMessage *reply = NULL;
	char *bssid_data, *ssid_data, *wpa_ie_data, *rsn_ie_data;
	DBusMessageIter iter, iter_dict;

	/* dbus needs the address of a pointer to the actual value
	 * for array types, not the address of the value itself.
	 */
	bssid_data = (char *) &res->bssid;
	ssid_data = (char *) &res->ssid;
	wpa_ie_data = (char *) &res->wpa_ie;
	rsn_ie_data = (char *) &res->rsn_ie;

	/* Dump the properties into a dbus message */
	reply = dbus_message_new_method_return(message);

	dbus_message_iter_init_append(reply, &iter);
	if (!wpa_dbus_dict_open_write(&iter, &iter_dict))
		goto error;

	if (!wpa_dbus_dict_append_byte_array(&iter_dict, "bssid",
					     bssid_data, ETH_ALEN))
		goto error;
	if (!wpa_dbus_dict_append_byte_array(&iter_dict, "ssid",
					     ssid_data, res->ssid_len))
		goto error;
	if (res->wpa_ie_len) {
		if (!wpa_dbus_dict_append_byte_array(&iter_dict, "wpaie",
						     wpa_ie_data,
						     res->wpa_ie_len)) {
			goto error;
		}
	}
	if (res->rsn_ie_len) {
		if (!wpa_dbus_dict_append_byte_array(&iter_dict, "rsnie",
						     rsn_ie_data,
						     res->rsn_ie_len)) {
			goto error;
		}
	}
	if (res->freq) {
		if (!wpa_dbus_dict_append_int32(&iter_dict, "frequency",
						res->freq))
			goto error;
	}
	if (!wpa_dbus_dict_append_uint16(&iter_dict, "capabilities",
					 res->caps))
		goto error;
	if (!wpa_dbus_dict_append_int32(&iter_dict, "quality", res->qual))
		goto error;
	if (!wpa_dbus_dict_append_int32(&iter_dict, "noise", res->noise))
		goto error;
	if (!wpa_dbus_dict_append_int32(&iter_dict, "level", res->level))
		goto error;
	if (!wpa_dbus_dict_append_int32(&iter_dict, "maxrate", res->maxrate))
		goto error;

	if (!wpa_dbus_dict_close_write(&iter, &iter_dict))
		goto error;

	return reply;

error:
	if (reply)
		dbus_message_unref(reply);
	return dbus_message_new_error(message, WPAS_ERROR_INTERNAL_ERROR,
				      "an internal error occurred returning "
				      "BSSID properties.");
}


/**
 * wpas_dbus_iface_add_network - Add a new configured network
 * @message: Pointer to incoming dbus message
 * @wpa_s: wpa_supplicant structure for a network interface
 * Returns: a dbus message containing a UINT32 indicating success (1) or
 *          failure (0)
 *
 * Handler function for "addNetwork" method call of a network interface.
 * Allows properties to be set in the message as well.
 */
DBusMessage * wpas_dbus_iface_add_network(DBusMessage *message,
					  struct wpa_supplicant *wpa_s)
{
#if 0 /* TODO: implement */
	DBusMessage *reply = NULL;
	struct wpa_ssid *ssid;

	/* Parse and validate message options before we actually add the
	 * network */

	ssid = wpa_config_add_network(wpa_s->conf);
	if (ssid == NULL) {
		reply = dbus_message_new_error(message,
					       WPAS_ERROR_ADD_NETWORK_ERROR,
					       "wpa_supplicant could not add "
					       "a network on this interface.");
		goto out;
	}
	ssid->disabled = 1;
	wpa_config_set_network_defaults(ssid);

out:
	return reply;
#endif

	return NULL;
}
