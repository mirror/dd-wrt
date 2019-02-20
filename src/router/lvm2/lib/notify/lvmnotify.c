/*
 * Copyright (C) 2015 Red Hat, Inc.
 *
 * This file is part of LVM2.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 */

#include "lib/misc/lib.h"
#include "lib/commands/toolcontext.h"
#include "lib/notify/lvmnotify.h"

#define LVM_DBUS_DESTINATION "com.redhat.lvmdbus1"
#define LVM_DBUS_PATH        "/com/redhat/lvmdbus1/Manager"
#define LVM_DBUS_INTERFACE   "com.redhat.lvmdbus1.Manager"
#define SD_BUS_SYSTEMD_NO_SUCH_UNIT_ERROR "org.freedesktop.systemd1.NoSuchUnit"
#define SD_BUS_DBUS_SERVICE_UNKNOWN_ERROR "org.freedesktop.DBus.Error.ServiceUnknown"

#ifdef NOTIFYDBUS_SUPPORT
#include <systemd/sd-bus.h>

int lvmnotify_is_supported(void)
{
	return 1;
}

void lvmnotify_send(struct cmd_context *cmd)
{
	static const char _dbus_notification_failed_msg[] = "D-Bus notification failed";
	sd_bus *bus = NULL;
	sd_bus_message *m = NULL;
	sd_bus_error error = SD_BUS_ERROR_NULL;
	const char *cmd_name;
	int ret;
	int result = 0;

	if (!cmd->vg_notify && !cmd->lv_notify && !cmd->pv_notify)
		return;

	cmd->vg_notify = 0;
	cmd->lv_notify = 0;
	cmd->pv_notify = 0;

	cmd_name = get_cmd_name();

	ret = sd_bus_open_system(&bus);
	if (ret < 0) {
		log_debug_dbus("Failed to connect to dbus: %d", ret);
		return;
	}

	log_debug_dbus("Nofify dbus at %s.", LVM_DBUS_DESTINATION);

	ret = sd_bus_call_method(bus,
				 LVM_DBUS_DESTINATION,
				 LVM_DBUS_PATH,
				 LVM_DBUS_INTERFACE,
				 "ExternalEvent",
				 &error,
				 &m,
				 "s",
				 cmd_name);

	if (ret < 0) {
		if (sd_bus_error_has_name(&error, SD_BUS_SYSTEMD_NO_SUCH_UNIT_ERROR) ||
		    sd_bus_error_has_name(&error, SD_BUS_DBUS_SERVICE_UNKNOWN_ERROR))
			log_debug_dbus("%s: %s", _dbus_notification_failed_msg, error.message);
		else
			log_warn("WARNING: %s: %s", _dbus_notification_failed_msg, error.message);
		goto out;
	}

	ret = sd_bus_message_read(m, "i", &result);
	if (ret < 0)
		log_debug_dbus("Failed to parse dbus response message: %d", ret);
	if (result)
		log_debug_dbus("Bad return value from dbus service: %d", result);
out:
	sd_bus_error_free(&error);
	sd_bus_message_unref(m);
	sd_bus_flush_close_unref(bus);
}

void set_vg_notify(struct cmd_context *cmd)
{
	cmd->vg_notify = 1;
}

void set_lv_notify(struct cmd_context *cmd)
{
	cmd->lv_notify = 1;
}

void set_pv_notify(struct cmd_context *cmd)
{
	cmd->pv_notify = 1;
}

#else

int lvmnotify_is_supported(void)
{
	return 0;
}

void lvmnotify_send(struct cmd_context *cmd)
{
}

void set_vg_notify(struct cmd_context *cmd)
{
}

void set_lv_notify(struct cmd_context *cmd)
{
}

void set_pv_notify(struct cmd_context *cmd)
{
}

#endif

