# Copyright (C) 2015-2016 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

import sys
import threading
import traceback
import dbus
import os
import copy
from . import cfg
from .utils import log_debug, pv_obj_path_generate, log_error
from .automatedproperties import AutomatedProperties


# noinspection PyPep8Naming
class ObjectManager(AutomatedProperties):
	"""
	Implements the org.freedesktop.DBus.ObjectManager interface
	"""

	def __init__(self, object_path, interface):
		super(ObjectManager, self).__init__(object_path, interface)
		self.set_interface(interface)
		self._ap_o_path = object_path
		self._objects = {}
		self._id_to_object_path = {}
		self.rlock = threading.RLock()

	@staticmethod
	def _get_managed_objects(obj):
		with obj.rlock:
			rc = {}
			try:
				for k, v in list(obj._objects.items()):
					path, props = v[0].emit_data()
					rc[path] = props
			except Exception:
				traceback.print_exc(file=sys.stdout)
				sys.exit(1)
			return rc

	@dbus.service.method(
		dbus_interface="org.freedesktop.DBus.ObjectManager",
		out_signature='a{oa{sa{sv}}}', async_callbacks=('cb', 'cbe'))
	def GetManagedObjects(self, cb, cbe):
		r = cfg.create_request_entry(-1, ObjectManager._get_managed_objects,
									(self, ), cb, cbe, False)
		cfg.worker_q.put(r)

	def locked(self):
		"""
		If some external code need to run across a number of different
		calls into ObjectManager while blocking others they can use this method
		to lock others out.
		:return:
		"""
		return ObjectManagerLock(self.rlock)

	@dbus.service.signal(
		dbus_interface="org.freedesktop.DBus.ObjectManager",
		signature='oa{sa{sv}}')
	def InterfacesAdded(self, object_path, int_name_prop_dict):
		log_debug(
			('SIGNAL: InterfacesAdded(%s, %s)' %
			(str(object_path), str(int_name_prop_dict))))

	@dbus.service.signal(
		dbus_interface="org.freedesktop.DBus.ObjectManager",
		signature='oas')
	def InterfacesRemoved(self, object_path, interface_list):
		log_debug(('SIGNAL: InterfacesRemoved(%s, %s)' %
			(str(object_path), str(interface_list))))

	def validate_lookups(self):
		with self.rlock:
			tmp_lookups = copy.deepcopy(self._id_to_object_path)

			# iterate over all we know, removing from the copy.  If all is well
			# we will have zero items left over
			for path, md in self._objects.items():
				obj, lvm_id, uuid = md

				if lvm_id:
					assert path == tmp_lookups[lvm_id]
					del tmp_lookups[lvm_id]

				if uuid:
					assert path == tmp_lookups[uuid]
					del tmp_lookups[uuid]

			rc = len(tmp_lookups)
			if rc:
				# Error condition
				log_error("_id_to_object_path has extraneous lookups!")
				for key, path in tmp_lookups.items():
					log_error("Key= %s, path= %s" % (key, path))
		return rc

	def _lookup_add(self, obj, path, lvm_id, uuid):
		"""
		Store information about what we added to the caches so that we
		can remove it cleanly
		:param obj:     The dbus object we are storing
		:param lvm_id:  The lvm id for the asset
		:param uuid:    The uuid for the asset
		:return:
		"""
		# Note: Only called internally, lock implied

		# We could have a temp entry from the forward creation of a path
		self._lookup_remove(path)

		self._objects[path] = (obj, lvm_id, uuid)

		# Make sure we have one or the other
		assert lvm_id or uuid

		if lvm_id:
			self._id_to_object_path[lvm_id] = path

		if uuid:
			self._id_to_object_path[uuid] = path

	def _lookup_remove(self, obj_path):
		# Note: Only called internally, lock implied
		if obj_path in self._objects:
			(obj, lvm_id, uuid) = self._objects[obj_path]

			if lvm_id in self._id_to_object_path:
				del self._id_to_object_path[lvm_id]

			if uuid in self._id_to_object_path:
				del self._id_to_object_path[uuid]

			del self._objects[obj_path]

	def lookup_update(self, dbus_obj, new_uuid, new_lvm_id):
		with self.rlock:
			obj_path = dbus_obj.dbus_object_path()
			self._lookup_remove(obj_path)
			self._lookup_add(
				dbus_obj, obj_path,
				new_lvm_id, new_uuid)

	def object_paths_by_type(self, o_type):
		with self.rlock:
			rc = {}

			for k, v in list(self._objects.items()):
				if isinstance(v[0], o_type):
					rc[k] = True
			return rc

	def register_object(self, dbus_object, emit_signal=False):
		"""
		Given a dbus object add it to the collection
		:param dbus_object: Dbus object to register
		:param emit_signal: If true emit a signal for interfaces added
		"""
		with self.rlock:
			path, props = dbus_object.emit_data()

			# print('Registering object path %s for %s' %
			# (path, dbus_object.lvm_id))

			# We want fast access to the object by a number of different ways
			# so we use multiple hashs with different keys
			self._lookup_add(dbus_object, path, dbus_object.lvm_id,
				dbus_object.Uuid)

			if emit_signal:
				self.InterfacesAdded(path, props)

	def remove_object(self, dbus_object, emit_signal=False):
		"""
		Given a dbus object, remove it from the collection and remove it
		from the dbus framework as well
		:param dbus_object:  Dbus object to remove
		:param emit_signal:  If true emit the interfaces removed signal
		"""
		with self.rlock:
			# Store off the object path and the interface first
			path = dbus_object.dbus_object_path()
			interfaces = dbus_object.interface()

			# print 'UN-Registering object path %s for %s' % \
			#      (path, dbus_object.lvm_id)

			self._lookup_remove(path)

			# Remove from dbus library
			dbus_object.remove_from_connection(cfg.bus, path)

			# Optionally emit a signal
			if emit_signal:
				self.InterfacesRemoved(path, interfaces)

	def get_object_by_path(self, path):
		"""
		Given a dbus path return the object registered for it
		:param path: The dbus path
		:return: The object
		"""
		with self.rlock:
			if path in self._objects:
				return self._objects[path][0]
			return None

	def get_object_by_uuid_lvm_id(self, uuid, lvm_id):
		with self.rlock:
			return self.get_object_by_path(
				self.get_object_path_by_uuid_lvm_id(uuid, lvm_id))

	def get_object_by_lvm_id(self, lvm_id):
		"""
		Given an lvm identifier, return the object registered for it
		:param lvm_id: The lvm identifier
		"""
		with self.rlock:
			lookup_rc = self._id_lookup(lvm_id)
			if lookup_rc:
				return self.get_object_by_path(lookup_rc)
			return None

	def get_object_path_by_lvm_id(self, lvm_id):
		"""
		Given an lvm identifier, return the object path for it
		:param lvm_id: The lvm identifier
		:return: Object path or '/' if not found
		"""
		with self.rlock:
			lookup_rc = self._id_lookup(lvm_id)
			if lookup_rc:
				return lookup_rc
			return '/'

	def _uuid_verify(self, path, uuid, lvm_id):
		"""
		Ensure uuid is present for a successful lvm_id lookup
		NOTE: Internal call, assumes under object manager lock
		:param path: 		Path to object we looked up
		:param uuid: 		lvm uuid to verify
		:param lvm_id:		lvm_id used to find object
		:return: None
		"""
		# This gets called when we found an object based on lvm_id, ensure
		# uuid is correct too, as they can change. There is no durable
		# non-changeable name in lvm
		if lvm_id != uuid:
			if uuid and uuid not in self._id_to_object_path:
				obj = self.get_object_by_path(path)
				self._lookup_add(obj, path, lvm_id, uuid)

	def _lvm_id_verify(self, path, uuid, lvm_id):
		"""
		Ensure lvm_id is present for a successful uuid lookup
		NOTE: Internal call, assumes under object manager lock
		:param path: 		Path to object we looked up
		:param uuid: 		uuid used to find object
		:param lvm_id:		lvm_id to verify
		:return: None
		"""
		# This gets called when we found an object based on uuid, ensure
		# lvm_id is correct too, as they can change.  There is no durable
		# non-changeable name in lvm
		if lvm_id != uuid:
			if lvm_id and lvm_id not in self._id_to_object_path:
				obj = self.get_object_by_path(path)
				self._lookup_add(obj, path, lvm_id, uuid)

	def _id_lookup(self, the_id):
		path = None

		if the_id:
			# The _id_to_object_path contains hash keys for everything, so
			# uuid and lvm_id
			if the_id in self._id_to_object_path:
				path = self._id_to_object_path[the_id]
			else:
				if "/" in the_id:
					if the_id.startswith('/'):
						# We could have a pv device path lookup that failed,
						# lets try canonical form and try again.
						canonical = os.path.realpath(the_id)
						if canonical in self._id_to_object_path:
							path = self._id_to_object_path[canonical]
					else:
						vg, lv = the_id.split("/", 1)
						int_lvm_id = vg + "/" + ("[%s]" % lv)
						if int_lvm_id in self._id_to_object_path:
							path = self._id_to_object_path[int_lvm_id]
		return path

	def get_object_path_by_uuid_lvm_id(self, uuid, lvm_id, path_create=None):
		"""
		For a given lvm asset return the dbus object path registered for it.
		This method first looks up by uuid and then by lvm_id.  You
		can search by just one by setting uuid == lvm_id (uuid or lvm_id).
		If the object is not found and path_create is a not None, the
		path_create function will be called to create a new object path and
		register it with the object manager for the specified uuid & lvm_id.
		Note: If path create is not None, uuid and lvm_id cannot be equal
		:param uuid: The uuid for the lvm object we are searching for
		:param lvm_id: The lvm name (eg. pv device path, vg name, lv full name)
		:param path_create: If not None, create the path using this function if
				we fail to find the object by uuid or lvm_id.
		:returns None if lvm asset not found and path_create == None otherwise
				a valid dbus object path
		"""
		with self.rlock:
			assert lvm_id
			assert uuid

			if path_create:
				assert uuid != lvm_id

			# Check for Manager.LookUpByLvmId query, we cannot
			# check/verify/update the uuid and lvm_id lookups so don't!
			if uuid == lvm_id:
				path = self._id_lookup(lvm_id)
			else:
				# We have a uuid and a lvm_id we can do sanity checks to ensure
				# that they are consistent

				# If a PV is missing it's device path is '[unknown]' or some
				# other text derivation of unknown.  When we find that a PV is
				# missing we will clear out the lvm_id as it's likely not unique
				# and thus not useful and potentially harmful for lookups.
				if path_create == pv_obj_path_generate and \
						cfg.db.pv_missing(uuid):
					lvm_id = None

				# Lets check for the uuid first
				path = self._id_lookup(uuid)
				if path:
					# Verify the lvm_id is sane
					self._lvm_id_verify(path, uuid, lvm_id)
				else:
					# Unable to find by UUID, lets lookup by lvm_id
					path = self._id_lookup(lvm_id)
					if path:
						# Verify the uuid is sane
						self._uuid_verify(path, uuid, lvm_id)
					else:
						# We have exhausted all lookups, let's create if we can
						if path_create:
							path = path_create()
							self._lookup_add(None, path, lvm_id, uuid)

			# print('get_object_path_by_lvm_id(%s, %s, %s, %s: return %s' %
			# 	   (uuid, lvm_id, str(path_create), str(gen_new), path))

			return path


class ObjectManagerLock(object):
	"""
	The sole purpose of this class is to allow other code the ability to
	lock the object manager using a `with` statement, eg.

	with cfg.om.locked():
		# Do stuff with object manager

	This will ensure that the lock is always released (assuming this is done
	correctly)
	"""

	def __init__(self, recursive_lock):
		self._lock = recursive_lock

	def __enter__(self):
		# Acquire lock
		self._lock.acquire()

	# noinspection PyUnusedLocal
	def __exit__(self, e_type, e_value, e_traceback):
		# Release lock
		self._lock.release()
		self._lock = None
