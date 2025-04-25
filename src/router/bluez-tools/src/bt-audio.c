/*
 *
 *  bluez-tools - a set of tools to manage bluetooth devices for linux
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib.h>
#include <locale.h>
#include <stdlib.h>

#include "lib/dbus-common.h"
#include "lib/helpers.h"
#include "lib/bluez-api.h"

int main(int argc, char** argv)
{
	GError *error = NULL;

	setlocale(LC_CTYPE, "");

	dbus_init();

	gchar *adapter_arg = NULL;
	gchar *connect_arg = NULL;
	gchar *disconnect_arg = NULL;
	GOptionEntry options[] = {
		{"adapter", 'a', 0, G_OPTION_ARG_STRING, &adapter_arg, "Adapter name or MAC", "<name|mac>"},
		{"connect", 'c', 0, G_OPTION_ARG_STRING, &connect_arg, "The device to connect to", "<name|mac>"},
		{"disconnect", 'd', 0, G_OPTION_ARG_STRING, &disconnect_arg, "The device to disconnect from", "<name|mac>"},
		{NULL}
	};    

	GOptionContext *context = g_option_context_new(" - a Bluetooth generic audio manager");
	g_option_context_add_main_entries(context, options, NULL);
	g_option_context_set_summary(context, "Version "PACKAGE_VERSION);
	g_option_context_set_description(context,
					 "Report bugs to <"PACKAGE_BUGREPORT">.\n"
					 "Project home page <"PACKAGE_URL">.");

	if (!g_option_context_parse(context, &argc, &argv, &error)) {
		g_print("%s: %s\n", g_get_prgname(), error->message);
		g_print("Try `%s --help` for more information.\n", g_get_prgname());

		g_option_context_free(context);
		return EXIT_FAILURE;
	}

	g_option_context_free(context);

	if (!dbus_system_connect(&error)) {
		g_printerr("Couldn't connect to D-Bus system bus: %s\n", error->message);

		g_free(adapter_arg);
		g_free(connect_arg);
		g_free(disconnect_arg);
		return EXIT_FAILURE;
	}

	if (!intf_supported(BLUEZ_DBUS_SERVICE_NAME, MANAGER_DBUS_PATH, MANAGER_DBUS_INTERFACE)) {
		g_printerr("%s: BlueZ service not found."
			   "Did you forget to run bluetoothd?\n",
			   g_get_prgname());

		g_free(adapter_arg);
		g_free(connect_arg);
		g_free(disconnect_arg);
		return EXIT_FAILURE;
	}

	Adapter *adapter = find_adapter(adapter_arg, &error);
	g_free(adapter_arg);
	exit_if_error(error);

	Device *device;
	if (connect_arg) {
		device = find_device(adapter, connect_arg, &error);
	} else if (disconnect_arg) {
		device = find_device(adapter, disconnect_arg, &error);
	} else {
		g_printerr("%s: You need to provide either --connect or --disconnect.\n", g_get_prgname());

		g_free(connect_arg);
		g_free(disconnect_arg);
		return EXIT_FAILURE;
	}

	g_object_unref(adapter);
	exit_if_error(error);

	if (device == NULL) {
		g_printerr("%s: Invalid device '%s'.\n", g_get_prgname(),
			   connect_arg ? connect_arg : disconnect_arg);

		g_free(connect_arg);
		g_free(disconnect_arg);

		return EXIT_FAILURE;
	}

	g_free(connect_arg);
	g_free(disconnect_arg);

	if (connect_arg)
		device_connect(device, &error);
	else
		device_disconnect(device, &error);

	g_object_unref(device);
	exit_if_error(error);

	return EXIT_SUCCESS;
}
