# Copyright (C) 2015-2016 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

from . import cfg


def _compare_construction(o_state, new_state):
	# We need to check to see if the objects would get constructed
	# the same
	existing_ctor, existing_path = o_state.creation_signature()
	new_ctor, new_path = new_state.creation_signature()

	# print("%s == %s and %s == %s" % (str(existing_ctor), str(new_ctor),
	#      str(existing_path), str(new_path)))

	return ((existing_ctor == new_ctor) and (existing_path == new_path))


def common(retrieve, o_type, search_keys,
			object_path, refresh, emit_signal, cache_refresh):
	num_changes = 0
	existing_paths = []
	rc = []

	if search_keys:
		assert isinstance(search_keys, list)

	if cache_refresh:
		cfg.db.refresh()

	objects = retrieve(search_keys, cache_refresh=False)

	# If we are doing a refresh we need to know what we have in memory, what's
	# in lvm and add those that are new and remove those that are gone!
	if refresh:
		existing_paths = cfg.om.object_paths_by_type(o_type)

	for o in objects:
		# Assume we need to add this one to dbus, unless we are refreshing
		# and it's already present
		return_object = True

		if refresh:
			# We are refreshing all the PVs from LVM, if this one exists
			# we need to refresh our state.
			dbus_object = cfg.om.get_object_by_uuid_lvm_id(*o.identifiers())

			if dbus_object:
				del existing_paths[dbus_object.dbus_object_path()]

				# If the old object state and new object state wouldn't be
				# created with the same path and same object constructor we
				# need to remove the old object and construct the new one
				# instead!
				if not _compare_construction(dbus_object.state, o):
					# Remove existing and construct new one
					cfg.om.remove_object(dbus_object, emit_signal)
					dbus_object = o.create_dbus_object(None)
					cfg.om.register_object(dbus_object, emit_signal)
					num_changes += 1
				else:
					num_changes += dbus_object.refresh(object_state=o)
				return_object = False

		if return_object:
			dbus_object = o.create_dbus_object(object_path)
			cfg.om.register_object(dbus_object, emit_signal)
			rc.append(dbus_object)

		object_path = None

	if refresh:
		for k in list(existing_paths.keys()):
			cfg.om.remove_object(cfg.om.get_object_by_path(k), True)
			num_changes += 1

	num_changes += len(rc)

	return rc, num_changes
