#!/usr/bin/python3

# Copyright (C) 2015-2016 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#
# Simply connects to the dbus service and calls Refresh and ensures that the
# value returned is zero

import testlib
import dbus
from dbus.mainloop.glib import DBusGMainLoop
import sys
import os


if __name__ == "__main__":

	use_session = os.getenv('LVMDBUSD_USE_SESSION', False)

	if use_session:
		bus = dbus.SessionBus(mainloop=DBusGMainLoop())
	else:
		bus = dbus.SystemBus(mainloop=DBusGMainLoop())

	mgr_proxy = testlib.ClientProxy(bus, testlib.MANAGER_OBJ)
	sys.exit(mgr_proxy.Manager.Refresh())
