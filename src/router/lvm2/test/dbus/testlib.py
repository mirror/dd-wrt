#!/usr/bin/python3

# Copyright (C) 2015-2016 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
import string
import random
import functools
import xml.etree.ElementTree as Et
from collections import OrderedDict
import dbus
import os
import sys
import time

BUS_NAME = os.getenv('LVM_DBUS_NAME', 'com.redhat.lvmdbus1')
BASE_INTERFACE = 'com.redhat.lvmdbus1'
MANAGER_INT = BASE_INTERFACE + '.Manager'
MANAGER_OBJ = '/' + BASE_INTERFACE.replace('.', '/') + '/Manager'
PV_INT = BASE_INTERFACE + ".Pv"
VG_INT = BASE_INTERFACE + ".Vg"
LV_INT = BASE_INTERFACE + ".Lv"
THINPOOL_INT = BASE_INTERFACE + ".ThinPool"
SNAPSHOT_INT = BASE_INTERFACE + ".Snapshot"
LV_COMMON_INT = BASE_INTERFACE + ".LvCommon"
JOB_INT = BASE_INTERFACE + ".Job"
CACHE_POOL_INT = BASE_INTERFACE + ".CachePool"
CACHE_LV_INT = BASE_INTERFACE + ".CachedLv"
THINPOOL_LV_PATH = '/' + THINPOOL_INT.replace('.', '/')


validate_introspection = True


def rs(length, suffix, character_set=string.ascii_lowercase):
	return ''.join(random.choice(character_set) for _ in range(length)) + suffix


def mib(s):
	return 1024 * 1024 * s


def std_err_print(*args):
	sys.stderr.write(' '.join(map(str, args)) + '\n')
	sys.stderr.flush()


class DbusIntrospection(object):
	@staticmethod
	def introspect(xml_representation):
		interfaces = {}

		root = Et.fromstring(xml_representation)

		for c in root:
			if c.tag == "interface":
				in_f = c.attrib['name']
				interfaces[in_f] = dict(methods=OrderedDict(), properties={})
				for nested in c:
					if nested.tag == "method":
						mn = nested.attrib['name']
						interfaces[in_f]['methods'][mn] = OrderedDict()

						for arg in nested:
							if arg.tag == 'arg':
								arg_dir = arg.attrib['direction']
								if arg_dir == 'in':
									n = arg.attrib['name']
								else:
									n = 'RETURN_VALUE'

								arg_type = arg.attrib['type']

								if n:
									v = dict(
											name=mn,
											a_dir=arg_dir,
											a_type=arg_type
									)
									interfaces[in_f]['methods'][mn][n] = v

					elif nested.tag == 'property':
						pn = nested.attrib['name']
						p_access = nested.attrib['access']
						p_type = nested.attrib['type']

						interfaces[in_f]['properties'][pn] = \
							dict(p_access=p_access, p_type=p_type)
					else:
						pass

		# print('Interfaces...')
		# for k, v in list(interfaces.items()):
		# 	print('Interface %s' % k)
		# 	if v['methods']:
		# 		for m, args in list(v['methods'].items()):
		# 			print('    method: %s' % m)
		# 			for a, aa in args.items():
		# 				print('         method arg: %s type %s' %
		# 					  (a, aa['a_type']))
		# 	if v['properties']:
		# 		for p, d in list(v['properties'].items()):
		# 			print('    Property: %s type= %s' % (p, d['p_type']))
		# print('End interfaces')

		return interfaces


def btsr(value):
	t = type(value)
	if t == dbus.Boolean:
		return 'b'
	elif t == dbus.ObjectPath:
		return 'o'
	elif t == dbus.String:
		return 's'
	elif t == dbus.Byte:
		return 'y'
	elif t == dbus.Int16:
		return 'n'
	elif t == dbus.Int32:
		return 'i'
	elif t == dbus.Int64:
		return 'x'
	elif t == dbus.UInt16:
		return 'q'
	elif t == dbus.UInt32:
		return 'u'
	elif t == dbus.UInt64:
		return 't'
	elif t == dbus.Double:
		return 'd'
	elif t == dbus.Struct:
		rc = '('
		for vt in value:
			rc += btsr(vt)
		rc += ')'
		return rc
	elif t == dbus.Array:
		rc = "a"
		for i in value:
			rc += btsr(i)
			break
		return rc
	else:
		raise RuntimeError("Unhandled type %s" % str(t))


def verify_type(value, dbus_str_rep):
	actual_str_rep = btsr(value)

	if dbus_str_rep != actual_str_rep:
		# print("%s ~= %s" % (dbus_str_rep, actual_str_rep))
		# Unless we have a full filled out type we won't match exactly
		if not dbus_str_rep.startswith(actual_str_rep):
			raise RuntimeError(
				"Incorrect type, expected= %s actual = %s object= %s" %
				(dbus_str_rep, actual_str_rep, str(type(value))))


class RemoteInterface(object):
	def _set_props(self, props=None):
		if not props:
			for _ in range(0, 3):
				try:
					prop_interface = dbus.Interface(self.dbus_object,
						'org.freedesktop.DBus.Properties')
					props = prop_interface.GetAll(self.interface)
					break
				except dbus.exceptions.DBusException as dbe:
					if "GetAll" not in str(dbe):
						raise dbe
		if props:
			for kl, vl in list(props.items()):
				# Verify type is correct!
				if self.introspect:
					verify_type(vl, self.introspect[self.interface]
					['properties'][kl]['p_type'])
				setattr(self, kl, vl)

	@property
	def object_path(self):
		return self.dbus_object.object_path

	def __init__(
			self, dbus_object, interface, introspect,
			properties=None, timelimit=-1):
		self.dbus_object = dbus_object
		self.interface = interface
		self.introspect = introspect
		self.tmo = 0

		if timelimit >= 0:
			self.tmo = float(timelimit)
			self.tmo *= 1.10

		self.dbus_interface = dbus.Interface(self.dbus_object, self.interface)
		self._set_props(properties)

	def __getattr__(self, item):
		if hasattr(self.dbus_interface, item):
			return functools.partial(self._wrapper, item)
		else:
			return functools.partial(self, item)

	def _wrapper(self, _method_name, *args, **kwargs):

		# Lets see how long a method takes to execute, in call cases we should
		# return something when the time limit has been reached.
		start = time.time()
		result = getattr(self.dbus_interface, _method_name)(*args, **kwargs)
		end = time.time()

		diff = end - start

		if self.tmo > 0.0:
			if diff > self.tmo:
				std_err_print("\n Time exceeded: %f > %f %s" %
								(diff, self.tmo, _method_name))

		if self.introspect:
			if 'RETURN_VALUE' in self.introspect[
					self.interface]['methods'][_method_name]:
				r_type = self.introspect[
					self.interface]['methods'][
					_method_name]['RETURN_VALUE']['a_type']

				verify_type(result, r_type)

		return result

	def update(self):
		self._set_props()


class ClientProxy(object):
	@staticmethod
	def _intf_short_name(nm):
		return nm.split('.')[-1:][0]

	def get_introspect(self):
		i = dbus.Interface(
				self.dbus_object,
				'org.freedesktop.DBus.Introspectable')

		return DbusIntrospection.introspect(i.Introspect())

	def _common(self, interface, introspect, properties):
		short_name = ClientProxy._intf_short_name(interface)
		self.short_interface_names.append(short_name)
		ro = RemoteInterface(self.dbus_object, interface, introspect,
								properties, timelimit=self.tmo)
		setattr(self, short_name, ro)

	def __init__(self, bus, object_path, interface_prop_hash=None,
					interfaces=None, timelimit=-1):
		self.object_path = object_path
		self.short_interface_names = []
		self.tmo = timelimit
		self.dbus_object = bus.get_object(
			BUS_NAME, self.object_path, introspect=False)

		if interface_prop_hash:
			assert interfaces is None
		if interfaces:
			assert interface_prop_hash is None

		if interface_prop_hash and not validate_introspection:
			# We have everything including the values of the properties
			for i, props in interface_prop_hash.items():
				self._common(i, None, props)
		elif interfaces and not validate_introspection:
			# We are retrieving the values of the properties
			for i in interfaces:
				self._common(i, None, None)
		else:
			# We need to query the interfaces and gather all the properties
			# for each interface, as we have the introspection data we
			# will also utilize it to verify what we get back verifies
			introspect = self.get_introspect()

			if interface_prop_hash:
				introspect_interfaces = list(introspect.keys())
				for object_manager_key in interface_prop_hash.keys():
					assert object_manager_key in introspect_interfaces

			for i in list(introspect.keys()):
				self._common(i, introspect, None)

	def update(self):
		# Go through all interfaces and update them
		for sn in self.short_interface_names:
			getattr(self, sn).update()
