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
from .utils import pv_obj_path_generate, vg_obj_path_generate, n
import dbus
from . import cfg
from .cfg import VG_INTERFACE
from . import cmdhandler
from .request import RequestEntry
from .loader import common
from .state import State
from . import background
from .utils import round_size, mt_remove_dbus_objects
from .job import JobState


# noinspection PyUnusedLocal
def vgs_state_retrieve(selection, cache_refresh=True):
	rc = []

	if cache_refresh:
		cfg.db.refresh()

	for v in cfg.db.fetch_vgs(selection):
		rc.append(
			VgState(
				v['vg_uuid'], v['vg_name'], v['vg_fmt'], n(v['vg_size']),
				n(v['vg_free']), v['vg_sysid'], n(v['vg_extent_size']),
				n(v['vg_extent_count']), n(v['vg_free_count']),
				v['vg_profile'], n(v['max_lv']), n(v['max_pv']),
				n(v['pv_count']), n(v['lv_count']), n(v['snap_count']),
				n(v['vg_seqno']), n(v['vg_mda_count']),
				n(v['vg_mda_free']), n(v['vg_mda_size']),
				n(v['vg_mda_used_count']), v['vg_attr'], v['vg_tags']))
	return rc


def load_vgs(vg_specific=None, object_path=None, refresh=False,
		emit_signal=False, cache_refresh=True):
	return common(vgs_state_retrieve, (Vg,), vg_specific, object_path, refresh,
					emit_signal, cache_refresh)


# noinspection PyPep8Naming,PyUnresolvedReferences,PyUnusedLocal
class VgState(State):
	@property
	def lvm_id(self):
		return self.Name

	def identifiers(self):
		return (self.Uuid, self.Name)

	def _lv_paths_build(self):
		rc = []
		for lv in cfg.db.lvs_in_vg(self.Uuid):
			(lv_name, meta, lv_uuid) = lv
			full_name = "%s/%s" % (self.Name, lv_name)

			gen = utils.lv_object_path_method(lv_name, meta)

			lv_path = cfg.om.get_object_path_by_uuid_lvm_id(
				lv_uuid, full_name, gen)
			rc.append(lv_path)
		return dbus.Array(rc, signature='o')

	def _pv_paths_build(self):
		rc = []
		for p in cfg.db.pvs_in_vg(self.Uuid):
			(pv_name, pv_uuid) = p
			rc.append(cfg.om.get_object_path_by_uuid_lvm_id(
				pv_uuid, pv_name, pv_obj_path_generate))
		return rc

	def __init__(self, Uuid, Name, Fmt,
			SizeBytes, FreeBytes, SysId, ExtentSizeBytes,
			ExtentCount, FreeCount, Profile, MaxLv, MaxPv, PvCount,
			LvCount, SnapCount, Seqno, MdaCount, MdaFree,
			MdaSizeBytes, MdaUsedCount, attr, tags):
		utils.init_class_from_arguments(self)
		self.Pvs = self._pv_paths_build()
		self.Lvs = self._lv_paths_build()

	def create_dbus_object(self, path):
		if not path:
			path = cfg.om.get_object_path_by_uuid_lvm_id(
				self.Uuid, self.Name, vg_obj_path_generate)
		return Vg(path, self)

	# noinspection PyMethodMayBeStatic
	def creation_signature(self):
		return (Vg, vg_obj_path_generate)


# noinspection PyPep8Naming
@utils.dbus_property(VG_INTERFACE, 'Uuid', 's')
@utils.dbus_property(VG_INTERFACE, 'Name', 's')
@utils.dbus_property(VG_INTERFACE, 'Fmt', 's')
@utils.dbus_property(VG_INTERFACE, 'SizeBytes', 't', 0)
@utils.dbus_property(VG_INTERFACE, 'FreeBytes', 't', 0)
@utils.dbus_property(VG_INTERFACE, 'SysId', 's')
@utils.dbus_property(VG_INTERFACE, 'ExtentSizeBytes', 't')
@utils.dbus_property(VG_INTERFACE, 'ExtentCount', 't')
@utils.dbus_property(VG_INTERFACE, 'FreeCount', 't')
@utils.dbus_property(VG_INTERFACE, 'Profile', 's')
@utils.dbus_property(VG_INTERFACE, 'MaxLv', 't')
@utils.dbus_property(VG_INTERFACE, 'MaxPv', 't')
@utils.dbus_property(VG_INTERFACE, 'PvCount', 't')
@utils.dbus_property(VG_INTERFACE, 'LvCount', 't')
@utils.dbus_property(VG_INTERFACE, 'SnapCount', 't')
@utils.dbus_property(VG_INTERFACE, 'Seqno', 't')
@utils.dbus_property(VG_INTERFACE, 'MdaCount', 't')
@utils.dbus_property(VG_INTERFACE, 'MdaFree', 't')
@utils.dbus_property(VG_INTERFACE, 'MdaSizeBytes', 't')
@utils.dbus_property(VG_INTERFACE, 'MdaUsedCount', 't')
class Vg(AutomatedProperties):
	_Tags_meta = ("as", VG_INTERFACE)
	_Pvs_meta = ("ao", VG_INTERFACE)
	_Lvs_meta = ("ao", VG_INTERFACE)
	_Writeable_meta = ("b", VG_INTERFACE)
	_Readable_meta = ("b", VG_INTERFACE)
	_Resizeable_meta = ("b", VG_INTERFACE)
	_Exportable_meta = ('b', VG_INTERFACE)
	_Partial_meta = ('b', VG_INTERFACE)
	_AllocContiguous_meta = ('b', VG_INTERFACE)
	_AllocCling_meta = ('b', VG_INTERFACE)
	_AllocNormal_meta = ('b', VG_INTERFACE)
	_AllocAnywhere_meta = ('b', VG_INTERFACE)
	_Clustered_meta = ('b', VG_INTERFACE)

	# noinspection PyUnusedLocal,PyPep8Naming
	def __init__(self, object_path, object_state):
		super(Vg, self).__init__(object_path, vgs_state_retrieve)
		self.set_interface(VG_INTERFACE)
		self._object_path = object_path
		self.state = object_state

	@staticmethod
	def fetch_new_lv(vg_name, lv_name):
		return cfg.om.get_object_path_by_lvm_id("%s/%s" % (vg_name, lv_name))

	@staticmethod
	def handle_execute(rc, out, err):
		if rc == 0:
			cfg.load()
		else:
			# Need to work on error handling, need consistent
			raise dbus.exceptions.DBusException(
				VG_INTERFACE,
				'Exit code %s, stderr = %s' % (str(rc), err))

	@staticmethod
	def validate_dbus_object(vg_uuid, vg_name):
		dbo = cfg.om.get_object_by_uuid_lvm_id(vg_uuid, vg_name)
		if not dbo:
			raise dbus.exceptions.DBusException(
				VG_INTERFACE,
				'VG with uuid %s and name %s not present!' %
				(vg_uuid, vg_name))
		return dbo

	@staticmethod
	def _rename(uuid, vg_name, new_name, rename_options):
		# Make sure we have a dbus object representing it
		Vg.validate_dbus_object(uuid, vg_name)
		rc, out, err = cmdhandler.vg_rename(
			vg_name, new_name, rename_options)
		Vg.handle_execute(rc, out, err)
		return '/'

	@dbus.service.method(
		dbus_interface=VG_INTERFACE,
		in_signature='sia{sv}', out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def Rename(self, name, tmo, rename_options, cb, cbe):
		utils.validate_vg_name(VG_INTERFACE, name)
		r = RequestEntry(tmo, Vg._rename,
				(self.state.Uuid, self.state.lvm_id, name,
				rename_options), cb, cbe, False)
		cfg.worker_q.put(r)

	@staticmethod
	def _remove(uuid, vg_name, remove_options):
		# Make sure we have a dbus object representing it
		Vg.validate_dbus_object(uuid, vg_name)
		# Remove the VG, if successful then remove from the model
		rc, out, err = cmdhandler.vg_remove(vg_name, remove_options)
		Vg.handle_execute(rc, out, err)
		return '/'

	@dbus.service.method(
		dbus_interface=VG_INTERFACE,
		in_signature='ia{sv}', out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def Remove(self, tmo, remove_options, cb, cbe):
		r = RequestEntry(tmo, Vg._remove,
				(self.state.Uuid, self.state.lvm_id, remove_options),
				cb, cbe, False)
		cfg.worker_q.put(r)

	@staticmethod
	def _change(uuid, vg_name, change_options):
		Vg.validate_dbus_object(uuid, vg_name)
		rc, out, err = cmdhandler.vg_change(change_options, vg_name)
		Vg.handle_execute(rc, out, err)
		return '/'

	# TODO: This should be broken into a number of different methods
	# instead of having one method that takes a hash for parameters.  Some of
	# the changes that vgchange does works on entire system, not just a
	# specfic vg, thus that should be in the Manager interface.
	@dbus.service.method(
		dbus_interface=VG_INTERFACE,
		in_signature='ia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def Change(self, tmo, change_options, cb, cbe):
		r = RequestEntry(tmo, Vg._change,
				(self.state.Uuid, self.state.lvm_id, change_options),
				cb, cbe, False)
		cfg.worker_q.put(r)

	@staticmethod
	def _reduce(uuid, vg_name, missing, pv_object_paths, reduce_options):
		# Make sure we have a dbus object representing it
		Vg.validate_dbus_object(uuid, vg_name)

		pv_devices = []

		# If pv_object_paths is not empty, then get the device paths
		if pv_object_paths and len(pv_object_paths) > 0:
			for pv_op in pv_object_paths:
				pv = cfg.om.get_object_by_path(pv_op)
				if pv:
					pv_devices.append(pv.lvm_id)
				else:
					raise dbus.exceptions.DBusException(
						VG_INTERFACE,
						'PV Object path not found = %s!' % pv_op)

		rc, out, err = cmdhandler.vg_reduce(vg_name, missing, pv_devices,
											reduce_options)
		Vg.handle_execute(rc, out, err)
		return '/'

	@dbus.service.method(
		dbus_interface=VG_INTERFACE,
		in_signature='baoia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def Reduce(self, missing, pv_object_paths, tmo, reduce_options, cb, cbe):
		r = RequestEntry(tmo, Vg._reduce,
				(self.state.Uuid, self.state.lvm_id, missing,
				pv_object_paths, reduce_options), cb, cbe, False)
		cfg.worker_q.put(r)

	@staticmethod
	def _extend(uuid, vg_name, pv_object_paths, extend_options):
		# Make sure we have a dbus object representing it
		Vg.validate_dbus_object(uuid, vg_name)

		extend_devices = []

		for i in pv_object_paths:
			pv = cfg.om.get_object_by_path(i)
			if pv:
				extend_devices.append(pv.lvm_id)
			else:
				raise dbus.exceptions.DBusException(
					VG_INTERFACE, 'PV Object path not found = %s!' % i)

		if len(extend_devices):
			rc, out, err = cmdhandler.vg_extend(vg_name, extend_devices,
												extend_options)
			Vg.handle_execute(rc, out, err)
		else:
			raise dbus.exceptions.DBusException(
				VG_INTERFACE, 'No pv_object_paths provided!')

		return '/'

	@dbus.service.method(
		dbus_interface=VG_INTERFACE,
		in_signature='aoia{sv}', out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def Extend(self, pv_object_paths, tmo, extend_options, cb, cbe):
		r = RequestEntry(tmo, Vg._extend,
				(self.state.Uuid, self.state.lvm_id, pv_object_paths,
				extend_options),
				cb, cbe, False)
		cfg.worker_q.put(r)

	@dbus.service.method(
		dbus_interface=VG_INTERFACE,
		in_signature='o(tt)a(ott)ia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def Move(self, pv_src_obj, pv_source_range, pv_dests_and_ranges,
			tmo, move_options, cb, cbe):

		job_state = JobState()

		r = RequestEntry(
				tmo, background.move,
				(VG_INTERFACE, None, pv_src_obj, pv_source_range,
				pv_dests_and_ranges, move_options, job_state), cb, cbe, False,
				job_state)

		cfg.worker_q.put(r)

	@staticmethod
	def _lv_create(uuid, vg_name, name, size_bytes, pv_dests_and_ranges,
			create_options):
		# Make sure we have a dbus object representing it
		pv_dests = []

		Vg.validate_dbus_object(uuid, vg_name)

		if len(pv_dests_and_ranges):
			for pr in pv_dests_and_ranges:
				pv_dbus_obj = cfg.om.get_object_by_path(pr[0])
				if not pv_dbus_obj:
					raise dbus.exceptions.DBusException(
						VG_INTERFACE,
						'PV Destination (%s) not found' % pr[0])

				pv_dests.append((pv_dbus_obj.lvm_id, pr[1], pr[2]))

		rc, out, err = cmdhandler.vg_lv_create(
			vg_name, create_options, name, size_bytes, pv_dests)

		Vg.handle_execute(rc, out, err)
		return Vg.fetch_new_lv(vg_name, name)

	@dbus.service.method(
		dbus_interface=VG_INTERFACE,
		in_signature='sta(ott)ia{sv}',
		out_signature='(oo)',
		async_callbacks=('cb', 'cbe'))
	def LvCreate(self, name, size_bytes, pv_dests_and_ranges,
			tmo, create_options, cb, cbe):
		"""
		This one it for the advanced users that want to roll their own
		:param name:            Name of the LV
		:param size_bytes:      Size of LV in bytes
		:param pv_dests_and_ranges:   Optional array of PV object paths and
									ranges
		:param tmo: -1 == Wait forever, 0 == return job immediately, > 0 ==
							willing to wait that number of seconds before
							getting a job
		:param create_options:  hash of key/value pairs
		:param cb: Internal, not accessible by dbus API user
		:param cbe: Internal, not accessible by dbus API user
		:return: (oo) First object path is newly created object, second is
					job object path if created.  Each == '/' when it doesn't
					apply.
		"""
		utils.validate_lv_name(VG_INTERFACE, self.Name, name)
		r = RequestEntry(tmo, Vg._lv_create,
				(self.state.Uuid, self.state.lvm_id,
				name, round_size(size_bytes), pv_dests_and_ranges,
				create_options), cb, cbe)
		cfg.worker_q.put(r)

	@staticmethod
	def _lv_create_linear(uuid, vg_name, name, size_bytes,
			thin_pool, create_options):
		# Make sure we have a dbus object representing it
		Vg.validate_dbus_object(uuid, vg_name)

		rc, out, err = cmdhandler.vg_lv_create_linear(
			vg_name, create_options, name, size_bytes, thin_pool)

		Vg.handle_execute(rc, out, err)
		return Vg.fetch_new_lv(vg_name, name)

	@dbus.service.method(
		dbus_interface=VG_INTERFACE,
		in_signature='stbia{sv}',
		out_signature='(oo)',
		async_callbacks=('cb', 'cbe'))
	def LvCreateLinear(self, name, size_bytes,
			thin_pool, tmo, create_options, cb, cbe):
		utils.validate_lv_name(VG_INTERFACE, self.Name, name)
		r = RequestEntry(tmo, Vg._lv_create_linear,
						(self.state.Uuid, self.state.lvm_id,
						name, round_size(size_bytes), thin_pool,
						create_options), cb, cbe)
		cfg.worker_q.put(r)

	@staticmethod
	def _lv_create_striped(uuid, vg_name, name, size_bytes, num_stripes,
			stripe_size_kb, thin_pool, create_options):
		# Make sure we have a dbus object representing it
		Vg.validate_dbus_object(uuid, vg_name)
		rc, out, err = cmdhandler.vg_lv_create_striped(
			vg_name, create_options, name, size_bytes,
			num_stripes, stripe_size_kb, thin_pool)
		Vg.handle_execute(rc, out, err)
		return Vg.fetch_new_lv(vg_name, name)

	@dbus.service.method(
		dbus_interface=VG_INTERFACE,
		in_signature='stuubia{sv}',
		out_signature='(oo)',
		async_callbacks=('cb', 'cbe'))
	def LvCreateStriped(self, name, size_bytes, num_stripes,
						stripe_size_kb, thin_pool, tmo, create_options,
						cb, cbe):
		utils.validate_lv_name(VG_INTERFACE, self.Name, name)
		r = RequestEntry(
				tmo, Vg._lv_create_striped,
				(self.state.Uuid, self.state.lvm_id, name,
				round_size(size_bytes), num_stripes, stripe_size_kb,
				thin_pool, create_options),
				cb, cbe)
		cfg.worker_q.put(r)

	@staticmethod
	def _lv_create_mirror(uuid, vg_name, name, size_bytes,
			num_copies, create_options):
		# Make sure we have a dbus object representing it
		Vg.validate_dbus_object(uuid, vg_name)
		rc, out, err = cmdhandler.vg_lv_create_mirror(
			vg_name, create_options, name, size_bytes, num_copies)
		Vg.handle_execute(rc, out, err)
		return Vg.fetch_new_lv(vg_name, name)

	@dbus.service.method(
		dbus_interface=VG_INTERFACE,
		in_signature='stuia{sv}',
		out_signature='(oo)',
		async_callbacks=('cb', 'cbe'))
	def LvCreateMirror(self, name, size_bytes, num_copies,
			tmo, create_options, cb, cbe):
		utils.validate_lv_name(VG_INTERFACE, self.Name, name)
		r = RequestEntry(
			tmo, Vg._lv_create_mirror,
			(self.state.Uuid, self.state.lvm_id, name,
			round_size(size_bytes), num_copies,
			create_options), cb, cbe)
		cfg.worker_q.put(r)

	@staticmethod
	def _lv_create_raid(uuid, vg_name, name, raid_type, size_bytes,
						num_stripes, stripe_size_kb, create_options):
		# Make sure we have a dbus object representing it
		Vg.validate_dbus_object(uuid, vg_name)
		rc, out, err = cmdhandler.vg_lv_create_raid(
			vg_name, create_options, name, raid_type, size_bytes,
			num_stripes, stripe_size_kb)
		Vg.handle_execute(rc, out, err)
		return Vg.fetch_new_lv(vg_name, name)

	@dbus.service.method(
		dbus_interface=VG_INTERFACE,
		in_signature='sstuuia{sv}',
		out_signature='(oo)',
		async_callbacks=('cb', 'cbe'))
	def LvCreateRaid(self, name, raid_type, size_bytes,
			num_stripes, stripe_size_kb, tmo,
			create_options, cb, cbe):
		utils.validate_lv_name(VG_INTERFACE, self.Name, name)
		r = RequestEntry(tmo, Vg._lv_create_raid,
				(self.state.Uuid, self.state.lvm_id, name,
				raid_type, round_size(size_bytes), num_stripes,
				stripe_size_kb, create_options), cb, cbe)
		cfg.worker_q.put(r)

	@staticmethod
	def _create_pool(uuid, vg_name, meta_data_lv, data_lv,
						create_options, create_method):
		# Make sure we have a dbus object representing it
		Vg.validate_dbus_object(uuid, vg_name)

		# Retrieve the full names for the metadata and data lv
		md = cfg.om.get_object_by_path(meta_data_lv)
		data = cfg.om.get_object_by_path(data_lv)

		if md and data:

			new_name = data.Name

			rc, out, err = create_method(
				md.lv_full_name(), data.lv_full_name(), create_options)

			if rc == 0:
				mt_remove_dbus_objects((md, data))

			Vg.handle_execute(rc, out, err)

		else:
			msg = ""

			if not md:
				msg += 'Meta data LV with object path %s not present!' % \
					(meta_data_lv)

			if not data_lv:
				msg += 'Data LV with object path %s not present!' % \
					(meta_data_lv)

			raise dbus.exceptions.DBusException(VG_INTERFACE, msg)

		return Vg.fetch_new_lv(vg_name, new_name)

	@dbus.service.method(
		dbus_interface=VG_INTERFACE,
		in_signature='ooia{sv}',
		out_signature='(oo)',
		async_callbacks=('cb', 'cbe'))
	def CreateCachePool(self, meta_data_lv, data_lv, tmo, create_options,
						cb, cbe):
		r = RequestEntry(
			tmo, Vg._create_pool,
			(self.state.Uuid, self.state.lvm_id, meta_data_lv,
			data_lv, create_options, cmdhandler.vg_create_cache_pool), cb, cbe)
		cfg.worker_q.put(r)

	@dbus.service.method(
		dbus_interface=VG_INTERFACE,
		in_signature='ooia{sv}',
		out_signature='(oo)',
		async_callbacks=('cb', 'cbe'))
	def CreateThinPool(self, meta_data_lv, data_lv, tmo, create_options,
						cb, cbe):
		r = RequestEntry(
			tmo, Vg._create_pool,
			(self.state.Uuid, self.state.lvm_id, meta_data_lv,
			data_lv, create_options, cmdhandler.vg_create_thin_pool), cb, cbe)
		cfg.worker_q.put(r)

	@staticmethod
	def _pv_add_rm_tags(uuid, vg_name, pv_object_paths, tags_add,
						tags_del, tag_options):
		pv_devices = []

		# Make sure we have a dbus object representing it
		Vg.validate_dbus_object(uuid, vg_name)

		# Check for existence of pv object paths
		for p in pv_object_paths:
			pv = cfg.om.get_object_by_path(p)
			if pv:
				pv_devices.append(pv.Name)
			else:
				raise dbus.exceptions.DBusException(
					VG_INTERFACE, 'PV object path = %s not found' % p)

		rc, out, err = cmdhandler.pv_tag(
			pv_devices, tags_add, tags_del, tag_options)
		Vg.handle_execute(rc, out, err)
		return '/'

	@dbus.service.method(
		dbus_interface=VG_INTERFACE,
		in_signature='aoasia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def PvTagsAdd(self, pvs, tags, tmo, tag_options, cb, cbe):

		for t in tags:
			utils.validate_tag(VG_INTERFACE, t)

		r = RequestEntry(tmo, Vg._pv_add_rm_tags,
				(self.state.Uuid, self.state.lvm_id,
				pvs, tags, None, tag_options),
				cb, cbe, return_tuple=False)
		cfg.worker_q.put(r)

	@dbus.service.method(
		dbus_interface=VG_INTERFACE,
		in_signature='aoasia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def PvTagsDel(self, pvs, tags, tmo, tag_options, cb, cbe):

		for t in tags:
			utils.validate_tag(VG_INTERFACE, t)

		r = RequestEntry(
			tmo, Vg._pv_add_rm_tags,
			(self.state.Uuid, self.state.lvm_id,
			pvs, None, tags, tag_options),
			cb, cbe, return_tuple=False)
		cfg.worker_q.put(r)

	@staticmethod
	def _vg_add_rm_tags(uuid, vg_name, tags_add, tags_del, tag_options):
		# Make sure we have a dbus object representing it
		Vg.validate_dbus_object(uuid, vg_name)

		rc, out, err = cmdhandler.vg_tag(
			vg_name, tags_add, tags_del, tag_options)
		Vg.handle_execute(rc, out, err)
		return '/'

	@dbus.service.method(
		dbus_interface=VG_INTERFACE,
		in_signature='asia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def TagsAdd(self, tags, tmo, tag_options, cb, cbe):

		for t in tags:
			utils.validate_tag(VG_INTERFACE, t)

		r = RequestEntry(tmo, Vg._vg_add_rm_tags,
				(self.state.Uuid, self.state.lvm_id,
				tags, None, tag_options),
				cb, cbe, return_tuple=False)
		cfg.worker_q.put(r)

	@dbus.service.method(
		dbus_interface=VG_INTERFACE,
		in_signature='asia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def TagsDel(self, tags, tmo, tag_options, cb, cbe):

		for t in tags:
			utils.validate_tag(VG_INTERFACE, t)

		r = RequestEntry(tmo, Vg._vg_add_rm_tags,
				(self.state.Uuid, self.state.lvm_id,
				None, tags, tag_options),
				cb, cbe, return_tuple=False)
		cfg.worker_q.put(r)

	@staticmethod
	def _vg_change_set(uuid, vg_name, method, value, options):
		# Make sure we have a dbus object representing it
		Vg.validate_dbus_object(uuid, vg_name)
		rc, out, err = method(vg_name, value, options)
		Vg.handle_execute(rc, out, err)
		return '/'

	@dbus.service.method(
		dbus_interface=VG_INTERFACE,
		in_signature='sia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def AllocationPolicySet(self, policy, tmo, policy_options, cb, cbe):
		r = RequestEntry(tmo, Vg._vg_change_set,
				(self.state.Uuid, self.state.lvm_id,
				cmdhandler.vg_allocation_policy,
				policy, policy_options),
				cb, cbe, return_tuple=False)
		cfg.worker_q.put(r)

	@dbus.service.method(
		dbus_interface=VG_INTERFACE,
		in_signature='tia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def MaxPvSet(self, number, tmo, max_options, cb, cbe):
		r = RequestEntry(tmo, Vg._vg_change_set,
				(self.state.Uuid, self.state.lvm_id,
				cmdhandler.vg_max_pv, number, max_options),
				cb, cbe, return_tuple=False)
		cfg.worker_q.put(r)

	@dbus.service.method(
		dbus_interface=VG_INTERFACE,
		in_signature='ia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def UuidGenerate(self, tmo, options, cb, cbe):
		r = RequestEntry(tmo, Vg._vg_change_set,
				(self.state.Uuid, self.state.lvm_id,
				cmdhandler.vg_uuid_gen, None, options),
				cb, cbe, return_tuple=False)
		cfg.worker_q.put(r)

	def _attribute(self, pos, ch):
		return dbus.Boolean(self.state.attr[pos] == ch)

	@dbus.service.method(
		dbus_interface=VG_INTERFACE,
		in_signature='tia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def MaxLvSet(self, number, tmo, max_options, cb, cbe):
		r = RequestEntry(tmo, Vg._vg_change_set,
				(self.state.Uuid, self.state.lvm_id,
				cmdhandler.vg_max_lv, number, max_options),
				cb, cbe, return_tuple=False)
		cfg.worker_q.put(r)

	@staticmethod
	def _vg_activate_deactivate(uuid, vg_name, activate, control_flags,
								options):
		# Make sure we have a dbus object representing it
		Vg.validate_dbus_object(uuid, vg_name)
		rc, out, err = cmdhandler.activate_deactivate(
			'vgchange', vg_name, activate, control_flags, options)
		Vg.handle_execute(rc, out, err)
		return '/'

	@dbus.service.method(
		dbus_interface=VG_INTERFACE,
		in_signature='tia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def Activate(self, control_flags, tmo, activate_options, cb, cbe):
		r = RequestEntry(tmo, Vg._vg_activate_deactivate,
				(self.state.Uuid, self.state.lvm_id, True,
				control_flags, activate_options),
				cb, cbe, return_tuple=False)
		cfg.worker_q.put(r)

	@dbus.service.method(
		dbus_interface=VG_INTERFACE,
		in_signature='tia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def Deactivate(self, control_flags, tmo, activate_options, cb, cbe):
		r = RequestEntry(tmo, Vg._vg_activate_deactivate,
				(self.state.Uuid, self.state.lvm_id, False,
				control_flags, activate_options),
				cb, cbe, return_tuple=False)
		cfg.worker_q.put(r)

	@property
	def Tags(self):
		return utils.parse_tags(self.state.tags)

	@property
	def Pvs(self):
		return dbus.Array(self.state.Pvs, signature='o')

	@property
	def Lvs(self):
		return dbus.Array(self.state.Lvs, signature='o')

	@property
	def lvm_id(self):
		return self.state.lvm_id

	@property
	def Writeable(self):
		return self._attribute(0, 'w')

	@property
	def Readable(self):
		return self._attribute(0, 'r')

	@property
	def Resizeable(self):
		return self._attribute(1, 'z')

	@property
	def Exportable(self):
		return self._attribute(2, 'x')

	@property
	def Partial(self):
		return self._attribute(3, 'p')

	@property
	def AllocContiguous(self):
		return self._attribute(4, 'c')

	@property
	def AllocCling(self):
		return self._attribute(4, 'l')

	@property
	def AllocNormal(self):
		return self._attribute(4, 'n')

	@property
	def AllocAnywhere(self):
		return self._attribute(4, 'a')

	@property
	def Clustered(self):
		return self._attribute(5, 'c')
