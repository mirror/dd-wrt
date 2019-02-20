# Copyright (C) 2015-2016 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

from .automatedproperties import AutomatedProperties

from . import utils
from .utils import vg_obj_path_generate, log_error
import dbus
from . import cmdhandler
from . import cfg
from .cfg import LV_INTERFACE, THIN_POOL_INTERFACE, SNAPSHOT_INTERFACE, \
	LV_COMMON_INTERFACE, CACHE_POOL_INTERFACE, LV_CACHED
from .request import RequestEntry
from .utils import n, n32
from .loader import common
from .state import State
from . import background
from .utils import round_size, mt_remove_dbus_objects
from .job import JobState

import traceback


# Try and build a key for a LV, so that we sort the LVs with least dependencies
# first.  This may be error prone because of the flexibility LVM
# provides and what you can stack.
def get_key(i):

	name = i['lv_name']
	parent = i['lv_parent']
	pool = i['pool_lv']
	a1 = ""
	a2 = ""

	if name[0] == '[':
		a1 = '#'

	# We have a parent
	if parent:
		# Check if parent is hidden
		if parent[0] == '[':
			a2 = '##'
		else:
			a2 = '#'

	# If a LV has a pool, then it should be sorted/loaded after the pool
	# lv, unless it's a hidden too, then after other hidden, but before visible
	if pool:
		if pool[0] != '[':
			a2 += '~'
		else:
			a1 = '$' + a1

	return "%s%s%s" % (a1, a2, name)


# noinspection PyUnusedLocal
def lvs_state_retrieve(selection, cache_refresh=True):
	rc = []

	if cache_refresh:
		cfg.db.refresh()

	# When building up the model, it's best to process LVs with the least
	# dependencies to those that are dependant upon other LVs.  Otherwise, when
	# we are trying to gather information we could be in a position where we
	# don't have information available yet.
	lvs = sorted(cfg.db.fetch_lvs(selection), key=get_key)

	for l in lvs:
		rc.append(LvState(
			l['lv_uuid'], l['lv_name'],
			l['lv_path'], n(l['lv_size']),
			l['vg_name'],
			l['vg_uuid'], l['pool_lv_uuid'],
			l['pool_lv'], l['origin_uuid'], l['origin'],
			n32(l['data_percent']), l['lv_attr'],
			l['lv_tags'], l['lv_active'], l['data_lv'],
			l['metadata_lv'], l['segtype'], l['lv_role'],
			l['lv_layout'],
			n32(l['snap_percent']),
			n32(l['metadata_percent']),
			n32(l['copy_percent']),
			n32(l['sync_percent']),
			n(l['lv_metadata_size']),
			l['move_pv'],
			l['move_pv_uuid']))
	return rc


def load_lvs(lv_name=None, object_path=None, refresh=False, emit_signal=False,
				cache_refresh=True):
	# noinspection PyUnresolvedReferences
	return common(
		lvs_state_retrieve,
		(LvCommon, Lv, LvThinPool, LvSnapShot),
		lv_name, object_path, refresh, emit_signal, cache_refresh)


# noinspection PyPep8Naming,PyUnresolvedReferences,PyUnusedLocal
class LvState(State):
	@staticmethod
	def _pv_devices(uuid):
		rc = []
		for pv in sorted(cfg.db.lv_contained_pv(uuid)):
			(pv_uuid, pv_name, pv_segs) = pv
			pv_obj = cfg.om.get_object_path_by_uuid_lvm_id(pv_uuid, pv_name)

			segs_decorate = []
			for i in pv_segs:
				segs_decorate.append((dbus.UInt64(i[0]),
									dbus.UInt64(i[1]),
									dbus.String(i[2])))

			rc.append((dbus.ObjectPath(pv_obj), segs_decorate))

		return dbus.Array(rc, signature="(oa(tts))")

	def vg_name_lookup(self):
		return cfg.om.get_object_by_path(self.Vg).Name

	@property
	def lvm_id(self):
		return "%s/%s" % (self.vg_name_lookup(), self.Name)

	def identifiers(self):
		return (self.Uuid, self.lvm_id)

	def _get_hidden_lv(self):
		rc = dbus.Array([], "o")

		vg_name = self.vg_name_lookup()

		for l in cfg.db.hidden_lvs(self.Uuid):
			full_name = "%s/%s" % (vg_name, l[1])
			op = cfg.om.get_object_path_by_uuid_lvm_id(l[0], full_name)
			assert op
			rc.append(dbus.ObjectPath(op))
		return rc

	def __init__(self, Uuid, Name, Path, SizeBytes,
			vg_name, vg_uuid, pool_lv_uuid, PoolLv,
			origin_uuid, OriginLv, DataPercent, Attr, Tags, active,
			data_lv, metadata_lv, segtypes, role, layout, SnapPercent,
			MetaDataPercent, CopyPercent, SyncPercent, MetaDataSizeBytes,
			move_pv, move_pv_uuid):
		utils.init_class_from_arguments(self)

		# The segtypes is possibly an array with potentially dupes or a single
		# value
		self._segs = dbus.Array([], signature='s')
		if not isinstance(segtypes, list):
			self._segs.append(dbus.String(segtypes))
		else:
			self._segs.extend([dbus.String(x) for x in set(segtypes)])

		self.Vg = cfg.om.get_object_path_by_uuid_lvm_id(
			vg_uuid, vg_name, vg_obj_path_generate)

		self.Devices = LvState._pv_devices(self.Uuid)

		if PoolLv:
			gen = utils.lv_object_path_method(Name, (Attr, layout, role))

			self.PoolLv = cfg.om.get_object_path_by_uuid_lvm_id(
				pool_lv_uuid, '%s/%s' % (vg_name, PoolLv), gen)
		else:
			self.PoolLv = '/'

		if OriginLv:
			self.OriginLv = \
				cfg.om.get_object_path_by_uuid_lvm_id(
					origin_uuid, '%s/%s' % (vg_name, OriginLv),
					vg_obj_path_generate)
		else:
			self.OriginLv = '/'

		self.HiddenLvs = self._get_hidden_lv()

	@property
	def SegType(self):
		return self._segs

	def _object_path_create(self):
		return utils.lv_object_path_method(
			self.Name, (self.Attr, self.layout, self.role))

	def _object_type_create(self):
		if self.Attr[0] == 't':
			return LvThinPool
		elif self.Attr[0] == 'C':
			if 'pool' in self.layout:
				return LvCachePool
			else:
				return LvCacheLv
		elif self.Name[0] == '[':
			return LvCommon
		elif self.OriginLv != '/':
			return LvSnapShot
		else:
			return Lv

	def create_dbus_object(self, path):
		if not path:
			path = cfg.om.get_object_path_by_uuid_lvm_id(
				self.Uuid, self.lvm_id, self._object_path_create())

		obj_ctor = self._object_type_create()
		return obj_ctor(path, self)

	def creation_signature(self):
		klass = self._object_type_create()
		path_method = self._object_path_create()
		return (klass, path_method)


# noinspection PyPep8Naming
@utils.dbus_property(LV_COMMON_INTERFACE, 'Uuid', 's')
@utils.dbus_property(LV_COMMON_INTERFACE, 'Name', 's')
@utils.dbus_property(LV_COMMON_INTERFACE, 'Path', 's')
@utils.dbus_property(LV_COMMON_INTERFACE, 'SizeBytes', 't')
@utils.dbus_property(LV_COMMON_INTERFACE, 'SegType', 'as')
@utils.dbus_property(LV_COMMON_INTERFACE, 'Vg', 'o')
@utils.dbus_property(LV_COMMON_INTERFACE, 'OriginLv', 'o')
@utils.dbus_property(LV_COMMON_INTERFACE, 'PoolLv', 'o')
@utils.dbus_property(LV_COMMON_INTERFACE, 'Devices', "a(oa(tts))")
@utils.dbus_property(LV_COMMON_INTERFACE, 'HiddenLvs', "ao")
@utils.dbus_property(LV_COMMON_INTERFACE, 'Attr', 's')
@utils.dbus_property(LV_COMMON_INTERFACE, 'DataPercent', 'u')
@utils.dbus_property(LV_COMMON_INTERFACE, 'SnapPercent', 'u')
@utils.dbus_property(LV_COMMON_INTERFACE, 'MetaDataPercent', 'u')
@utils.dbus_property(LV_COMMON_INTERFACE, 'CopyPercent', 'u')
@utils.dbus_property(LV_COMMON_INTERFACE, 'SyncPercent', 'u')
@utils.dbus_property(LV_COMMON_INTERFACE, 'MetaDataSizeBytes', 't')
class LvCommon(AutomatedProperties):
	_Tags_meta = ("as", LV_COMMON_INTERFACE)
	_Roles_meta = ("as", LV_COMMON_INTERFACE)
	_IsThinVolume_meta = ("b", LV_COMMON_INTERFACE)
	_IsThinPool_meta = ("b", LV_COMMON_INTERFACE)
	_Active_meta = ("b", LV_COMMON_INTERFACE)
	_VolumeType_meta = ("(ss)", LV_COMMON_INTERFACE)
	_Permissions_meta = ("(ss)", LV_COMMON_INTERFACE)
	_AllocationPolicy_meta = ("(ss)", LV_COMMON_INTERFACE)
	_State_meta = ("(ss)", LV_COMMON_INTERFACE)
	_TargetType_meta = ("(ss)", LV_COMMON_INTERFACE)
	_Health_meta = ("(ss)", LV_COMMON_INTERFACE)
	_FixedMinor_meta = ('b', LV_COMMON_INTERFACE)
	_ZeroBlocks_meta = ('b', LV_COMMON_INTERFACE)
	_SkipActivation_meta = ('b', LV_COMMON_INTERFACE)
	_MovePv_meta = ('o', LV_COMMON_INTERFACE)

	def _get_move_pv(self):
		path = None

		# It's likely that the move_pv is empty
		if self.state.move_pv_uuid and self.state.move_pv:
			path = cfg.om.get_object_path_by_uuid_lvm_id(
				self.state.move_pv_uuid, self.state.move_pv)
		if not path:
			path = '/'
		return path

	# noinspection PyUnusedLocal,PyPep8Naming
	def __init__(self, object_path, object_state):
		super(LvCommon, self).__init__(object_path, lvs_state_retrieve)
		self.set_interface(LV_COMMON_INTERFACE)
		self.state = object_state
		self._move_pv = self._get_move_pv()

	@staticmethod
	def handle_execute(rc, out, err):
		if rc == 0:
			cfg.load()
		else:
			# Need to work on error handling, need consistent
			raise dbus.exceptions.DBusException(
				LV_INTERFACE,
				'Exit code %s, stderr = %s' % (str(rc), err))

	@staticmethod
	def validate_dbus_object(lv_uuid, lv_name):
		dbo = cfg.om.get_object_by_uuid_lvm_id(lv_uuid, lv_name)
		if not dbo:
			raise dbus.exceptions.DBusException(
				LV_INTERFACE,
				'LV with uuid %s and name %s not present!' %
				(lv_uuid, lv_name))
		return dbo

	def attr_struct(self, index, type_map, default='undisclosed'):
		try:
			if self.state.Attr[index] not in type_map:
				log_error("LV %s %s with lv_attr %s, lv_attr[%d] = "
					"'%s' is not known" %
					(self.Uuid, self.Name, self.Attr, index,
					self.state.Attr[index]))

			return dbus.Struct((self.state.Attr[index],
				type_map.get(self.state.Attr[index], default)),
								signature="(ss)")
		except BaseException:
			st = traceback.format_exc()
			log_error("attr_struct: \n%s" % st)
			return dbus.Struct(('?', 'Unavailable'), signature="(ss)")

	@property
	def VolumeType(self):
		type_map = {'C': 'Cache', 'm': 'mirrored',
					'M': 'Mirrored without initial sync', 'o': 'origin',
					'O': 'Origin with merging snapshot', 'r': 'raid',
					'R': 'Raid without initial sync', 's': 'snapshot',
					'S': 'merging Snapshot', 'p': 'pvmove',
					'v': 'virtual', 'i': 'mirror  or  raid  image',
					'I': 'mirror or raid Image out-of-sync',
					'l': 'mirror log device', 'c': 'under conversion',
					'V': 'thin Volume', 't': 'thin pool', 'T': 'Thin pool data',
					'e': 'raid or pool metadata or pool metadata spare',
					'-': 'Unspecified'}
		return self.attr_struct(0, type_map)

	@property
	def Permissions(self):
		type_map = {'w': 'writable', 'r': 'read-only',
					'R': 'Read-only activation of non-read-only volume',
					'-': 'Unspecified'}
		return self.attr_struct(1, type_map)

	@property
	def AllocationPolicy(self):
		type_map = {'a': 'anywhere', 'A': 'anywhere locked',
					'c': 'contiguous', 'C': 'contiguous locked',
					'i': 'inherited', 'I': 'inherited locked',
					'l': 'cling', 'L': 'cling locked',
					'n': 'normal', 'N': 'normal locked', '-': 'Unspecified'}
		return self.attr_struct(2, type_map)

	@property
	def FixedMinor(self):
		return dbus.Boolean(self.state.Attr[3] == 'm')

	@property
	def State(self):
		type_map = {'a': 'active',
					's': 'suspended',
					'I': 'Invalid snapshot',
					'S': 'invalid Suspended snapshot',
					'm': 'snapshot merge failed',
					'M': 'suspended snapshot (M)erge failed',
					'd': 'mapped device present without  tables',
					'i': 'mapped device present with inactive table',
					'h': 'historical',
					'c': 'check needed suspended thin-pool',
					'C': 'check needed',
					'X': 'unknown',
					'-': 'Unspecified'}
		return self.attr_struct(4, type_map)

	@property
	def TargetType(self):
		type_map = {'C': 'Cache', 'm': 'mirror', 'r': 'raid',
					's': 'snapshot', 't': 'thin', 'u': 'unknown',
					'v': 'virtual', '-': 'Unspecified'}
		return dbus.Struct((self.state.Attr[6], type_map[self.state.Attr[6]]),
						signature="(ss)")

	@property
	def ZeroBlocks(self):
		return dbus.Boolean(self.state.Attr[7] == 'z')

	@property
	def Health(self):
		type_map = {'p': 'partial',
					'r': 'refresh needed',
					'm': 'mismatches',
					'w': 'writemostly',
					'X': 'unknown',
					'-': 'unspecified',
					's': 'reshaping',
					'F': 'failed',
					'D': 'Data space',
					'R': 'Remove',
					'M': 'Metadata'}
		return self.attr_struct(8, type_map)

	@property
	def SkipActivation(self):
		return dbus.Boolean(self.state.Attr[9] == 'k')

	def vg_name_lookup(self):
		return self.state.vg_name_lookup()

	def lv_full_name(self):
		return "%s/%s" % (self.state.vg_name_lookup(), self.state.Name)

	@property
	def identifiers(self):
		return self.state.identifiers

	@property
	def Tags(self):
		return utils.parse_tags(self.state.Tags)

	@property
	def Roles(self):
		return utils.parse_tags(self.state.role)

	@property
	def lvm_id(self):
		return self.state.lvm_id

	@property
	def IsThinVolume(self):
		return dbus.Boolean(self.state.Attr[0] == 'V')

	@property
	def IsThinPool(self):
		return dbus.Boolean(self.state.Attr[0] == 't')

	@property
	def Active(self):
		return dbus.Boolean(self.state.active == "active")

	@property
	def MovePv(self):
		return dbus.ObjectPath(self._move_pv)


# noinspection PyPep8Naming
class Lv(LvCommon):
	def _fetch_hidden(self, name):

		# The name is vg/name
		full_name = "%s/%s" % (self.vg_name_lookup(), name)
		return cfg.om.get_object_path_by_lvm_id(full_name)

	def _get_data_meta(self):

		# Get the data
		return (self._fetch_hidden(self.state.data_lv),
				self._fetch_hidden(self.state.metadata_lv))

	# noinspection PyUnusedLocal,PyPep8Naming
	def __init__(self, object_path, object_state):
		super(Lv, self).__init__(object_path, object_state)
		self.set_interface(LV_INTERFACE)
		self.state = object_state

	@staticmethod
	def _remove(lv_uuid, lv_name, remove_options):
		# Make sure we have a dbus object representing it
		LvCommon.validate_dbus_object(lv_uuid, lv_name)
		# Remove the LV, if successful then remove from the model
		rc, out, err = cmdhandler.lv_remove(lv_name, remove_options)
		LvCommon.handle_execute(rc, out, err)
		return '/'

	@dbus.service.method(
		dbus_interface=LV_INTERFACE,
		in_signature='ia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def Remove(self, tmo, remove_options, cb, cbe):
		r = RequestEntry(
			tmo, Lv._remove,
			(self.Uuid, self.lvm_id, remove_options),
			cb, cbe, False)
		cfg.worker_q.put(r)

	@staticmethod
	def _rename(lv_uuid, lv_name, new_name, rename_options):
		# Make sure we have a dbus object representing it
		LvCommon.validate_dbus_object(lv_uuid, lv_name)
		# Rename the logical volume
		rc, out, err = cmdhandler.lv_rename(lv_name, new_name,
											rename_options)
		LvCommon.handle_execute(rc, out, err)
		return '/'

	@dbus.service.method(
		dbus_interface=LV_INTERFACE,
		in_signature='sia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def Rename(self, name, tmo, rename_options, cb, cbe):
		utils.validate_lv_name(LV_INTERFACE, self.vg_name_lookup(), name)

		r = RequestEntry(
			tmo, Lv._rename,
			(self.Uuid, self.lvm_id, name, rename_options),
			cb, cbe, False)
		cfg.worker_q.put(r)

	@dbus.service.method(
		dbus_interface=LV_INTERFACE,
		in_signature='o(tt)a(ott)ia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def Move(self, pv_src_obj, pv_source_range,
				pv_dests_and_ranges,
				tmo, move_options, cb, cbe):

		job_state = JobState()

		r = RequestEntry(
				tmo, background.move,
				(LV_INTERFACE, self.lvm_id, pv_src_obj, pv_source_range,
				pv_dests_and_ranges, move_options, job_state), cb, cbe, False,
				job_state)

		background.cmd_runner(r)

	@staticmethod
	def _snap_shot(lv_uuid, lv_name, name, optional_size,
			snapshot_options):
		# Make sure we have a dbus object representing it
		dbo = LvCommon.validate_dbus_object(lv_uuid, lv_name)
		# If you specify a size you get a 'thick' snapshot even if
		# it is a thin lv
		if not dbo.IsThinVolume:
			if optional_size == 0:
				space = dbo.SizeBytes // 80
				remainder = space % 512
				optional_size = space + 512 - remainder

		rc, out, err = cmdhandler.vg_lv_snapshot(
			lv_name, snapshot_options, name, optional_size)
		LvCommon.handle_execute(rc, out, err)
		full_name = "%s/%s" % (dbo.vg_name_lookup(), name)
		return cfg.om.get_object_path_by_lvm_id(full_name)


	@dbus.service.method(
		dbus_interface=LV_INTERFACE,
		in_signature='stia{sv}',
		out_signature='(oo)',
		async_callbacks=('cb', 'cbe'))
	def Snapshot(self, name, optional_size, tmo,
			snapshot_options, cb, cbe):

		utils.validate_lv_name(LV_INTERFACE, self.vg_name_lookup(), name)

		r = RequestEntry(
			tmo, Lv._snap_shot,
			(self.Uuid, self.lvm_id, name,
			optional_size, snapshot_options), cb, cbe)
		cfg.worker_q.put(r)

	@staticmethod
	def _resize(lv_uuid, lv_name, new_size_bytes, pv_dests_and_ranges,
				resize_options):
		# Make sure we have a dbus object representing it
		pv_dests = []
		dbo = LvCommon.validate_dbus_object(lv_uuid, lv_name)

		# If we have PVs, verify them
		if len(pv_dests_and_ranges):
			for pr in pv_dests_and_ranges:
				pv_dbus_obj = cfg.om.get_object_by_path(pr[0])
				if not pv_dbus_obj:
					raise dbus.exceptions.DBusException(
						LV_INTERFACE,
						'PV Destination (%s) not found' % pr[0])

				pv_dests.append((pv_dbus_obj.lvm_id, pr[1], pr[2]))

		size_change = new_size_bytes - dbo.SizeBytes
		rc, out, err = cmdhandler.lv_resize(dbo.lvm_id, size_change,
											pv_dests, resize_options)
		LvCommon.handle_execute(rc, out, err)
		return "/"

	@dbus.service.method(
		dbus_interface=LV_INTERFACE,
		in_signature='ta(ott)ia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def Resize(self, new_size_bytes, pv_dests_and_ranges, tmo,
			resize_options, cb, cbe):
		"""
		Resize a LV
		:param new_size_bytes: The requested final size in bytes
		:param pv_dests_and_ranges: An array of pv object paths and src &
									dst. segment ranges
		:param tmo: -1 to wait forever, 0 to return job immediately, else
					number of seconds to wait for operation to complete
					before getting a job
		:param resize_options: key/value hash of options
		:param cb:  Used by framework not client facing API
		:param cbe: Used by framework not client facing API
		:return: '/' if complete, else job object path
		"""
		r = RequestEntry(
			tmo, Lv._resize,
			(self.Uuid, self.lvm_id, round_size(new_size_bytes),
			pv_dests_and_ranges,
			resize_options), cb, cbe, return_tuple=False)
		cfg.worker_q.put(r)

	@staticmethod
	def _lv_activate_deactivate(uuid, lv_name, activate, control_flags,
								options):
		# Make sure we have a dbus object representing it
		LvCommon.validate_dbus_object(uuid, lv_name)
		rc, out, err = cmdhandler.activate_deactivate(
			'lvchange', lv_name, activate, control_flags, options)
		LvCommon.handle_execute(rc, out, err)
		return '/'

	@dbus.service.method(
		dbus_interface=LV_INTERFACE,
		in_signature='tia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def Activate(self, control_flags, tmo, activate_options, cb, cbe):
		r = RequestEntry(
			tmo, Lv._lv_activate_deactivate,
			(self.state.Uuid, self.state.lvm_id, True,
			control_flags, activate_options),
			cb, cbe, return_tuple=False)
		cfg.worker_q.put(r)

	# noinspection PyProtectedMember
	@dbus.service.method(
		dbus_interface=LV_INTERFACE,
		in_signature='tia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def Deactivate(self, control_flags, tmo, activate_options, cb, cbe):
		r = RequestEntry(
			tmo, Lv._lv_activate_deactivate,
			(self.state.Uuid, self.state.lvm_id, False,
			control_flags, activate_options),
			cb, cbe, return_tuple=False)
		cfg.worker_q.put(r)

	@staticmethod
	def _add_rm_tags(uuid, lv_name, tags_add, tags_del, tag_options):
		# Make sure we have a dbus object representing it
		LvCommon.validate_dbus_object(uuid, lv_name)
		rc, out, err = cmdhandler.lv_tag(
			lv_name, tags_add, tags_del, tag_options)
		LvCommon.handle_execute(rc, out, err)
		return '/'

	@dbus.service.method(
		dbus_interface=LV_INTERFACE,
		in_signature='asia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def TagsAdd(self, tags, tmo, tag_options, cb, cbe):

		for t in tags:
			utils.validate_tag(LV_INTERFACE, t)

		r = RequestEntry(
			tmo, Lv._add_rm_tags,
			(self.state.Uuid, self.state.lvm_id,
			tags, None, tag_options),
			cb, cbe, return_tuple=False)
		cfg.worker_q.put(r)

	@dbus.service.method(
		dbus_interface=LV_INTERFACE,
		in_signature='asia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def TagsDel(self, tags, tmo, tag_options, cb, cbe):

		for t in tags:
			utils.validate_tag(LV_INTERFACE, t)

		r = RequestEntry(
			tmo, Lv._add_rm_tags,
			(self.state.Uuid, self.state.lvm_id,
			None, tags, tag_options),
			cb, cbe, return_tuple=False)
		cfg.worker_q.put(r)


# noinspection PyPep8Naming
class LvThinPool(Lv):
	_DataLv_meta = ("o", THIN_POOL_INTERFACE)
	_MetaDataLv_meta = ("o", THIN_POOL_INTERFACE)

	def __init__(self, object_path, object_state):
		super(LvThinPool, self).__init__(object_path, object_state)
		self.set_interface(THIN_POOL_INTERFACE)
		self._data_lv, self._metadata_lv = self._get_data_meta()

	@property
	def DataLv(self):
		return dbus.ObjectPath(self._data_lv)

	@property
	def MetaDataLv(self):
		return dbus.ObjectPath(self._metadata_lv)

	@staticmethod
	def _lv_create(lv_uuid, lv_name, name, size_bytes, create_options):
		# Make sure we have a dbus object representing it
		dbo = LvCommon.validate_dbus_object(lv_uuid, lv_name)

		rc, out, err = cmdhandler.lv_lv_create(
			lv_name, create_options, name, size_bytes)
		LvCommon.handle_execute(rc, out, err)
		full_name = "%s/%s" % (dbo.vg_name_lookup(), name)
		return cfg.om.get_object_path_by_lvm_id(full_name)

	@dbus.service.method(
		dbus_interface=THIN_POOL_INTERFACE,
		in_signature='stia{sv}',
		out_signature='(oo)',
		async_callbacks=('cb', 'cbe'))
	def LvCreate(self, name, size_bytes, tmo, create_options, cb, cbe):
		utils.validate_lv_name(THIN_POOL_INTERFACE, self.vg_name_lookup(), name)

		r = RequestEntry(
			tmo, LvThinPool._lv_create,
			(self.Uuid, self.lvm_id, name,
			round_size(size_bytes), create_options), cb, cbe)
		cfg.worker_q.put(r)


# noinspection PyPep8Naming
class LvCachePool(Lv):
	_DataLv_meta = ("o", CACHE_POOL_INTERFACE)
	_MetaDataLv_meta = ("o", CACHE_POOL_INTERFACE)

	def __init__(self, object_path, object_state):
		super(LvCachePool, self).__init__(object_path, object_state)
		self.set_interface(CACHE_POOL_INTERFACE)
		self._data_lv, self._metadata_lv = self._get_data_meta()

	@property
	def DataLv(self):
		return dbus.ObjectPath(self._data_lv)

	@property
	def MetaDataLv(self):
		return dbus.ObjectPath(self._metadata_lv)

	@staticmethod
	def _cache_lv(lv_uuid, lv_name, lv_object_path, cache_options):
		# Make sure we have a dbus object representing cache pool
		dbo = LvCommon.validate_dbus_object(lv_uuid, lv_name)

		# Make sure we have dbus object representing lv to cache
		lv_to_cache = cfg.om.get_object_by_path(lv_object_path)

		if lv_to_cache:
			fcn = lv_to_cache.lv_full_name()
			rc, out, err = cmdhandler.lv_cache_lv(
				dbo.lv_full_name(), fcn, cache_options)
			if rc == 0:
				# When we cache an LV, the cache pool and the lv that is getting
				# cached need to be removed from the object manager and
				# re-created as their interfaces have changed!
				mt_remove_dbus_objects((dbo, lv_to_cache))
				cfg.load()

				lv_converted = cfg.om.get_object_path_by_lvm_id(fcn)
			else:
				raise dbus.exceptions.DBusException(
					LV_INTERFACE,
					'Exit code %s, stderr = %s' % (str(rc), err))
		else:
			raise dbus.exceptions.DBusException(
				LV_INTERFACE, 'LV to cache with object path %s not present!' %
				lv_object_path)
		return lv_converted

	@dbus.service.method(
		dbus_interface=CACHE_POOL_INTERFACE,
		in_signature='oia{sv}',
		out_signature='(oo)',
		async_callbacks=('cb', 'cbe'))
	def CacheLv(self, lv_object, tmo, cache_options, cb, cbe):
		r = RequestEntry(
			tmo, LvCachePool._cache_lv,
			(self.Uuid, self.lvm_id, lv_object,
			cache_options), cb, cbe)
		cfg.worker_q.put(r)


# noinspection PyPep8Naming
class LvCacheLv(Lv):
	_CachePool_meta = ("o", LV_CACHED)

	def __init__(self, object_path, object_state):
		super(LvCacheLv, self).__init__(object_path, object_state)
		self.set_interface(LV_CACHED)

	@property
	def CachePool(self):
		return dbus.ObjectPath(self.state.PoolLv)

	@staticmethod
	def _detach_lv(lv_uuid, lv_name, detach_options, destroy_cache):
		# Make sure we have a dbus object representing cache pool
		dbo = LvCommon.validate_dbus_object(lv_uuid, lv_name)

		# Get current cache name
		cache_pool = cfg.om.get_object_by_path(dbo.CachePool)

		rc, out, err = cmdhandler.lv_detach_cache(
			dbo.lv_full_name(), detach_options, destroy_cache)
		if rc == 0:
			# The cache pool gets removed as hidden and put back to
			# visible, so lets delete
			mt_remove_dbus_objects((cache_pool, dbo))
			cfg.load()

			uncached_lv_path = cfg.om.get_object_path_by_lvm_id(lv_name)
		else:
			raise dbus.exceptions.DBusException(
				LV_INTERFACE,
				'Exit code %s, stderr = %s' % (str(rc), err))

		return uncached_lv_path

	@dbus.service.method(
		dbus_interface=LV_CACHED,
		in_signature='bia{sv}',
		out_signature='(oo)',
		async_callbacks=('cb', 'cbe'))
	def DetachCachePool(self, destroy_cache, tmo, detach_options, cb, cbe):
		r = RequestEntry(
			tmo, LvCacheLv._detach_lv,
			(self.Uuid, self.lvm_id, detach_options,
			destroy_cache), cb, cbe)
		cfg.worker_q.put(r)


# noinspection PyPep8Naming
class LvSnapShot(Lv):
	def __init__(self, object_path, object_state):
		super(LvSnapShot, self).__init__(object_path, object_state)
		self.set_interface(SNAPSHOT_INTERFACE)

	@dbus.service.method(
		dbus_interface=SNAPSHOT_INTERFACE,
		in_signature='ia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def Merge(self, tmo, merge_options, cb, cbe):
		job_state = JobState()

		r = RequestEntry(tmo, background.merge,
							(SNAPSHOT_INTERFACE, self.Uuid, self.lvm_id,
							merge_options, job_state), cb, cbe, False,
							job_state)
		background.cmd_runner(r)
