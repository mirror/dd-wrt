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
from . import cfg
import dbus
from .cfg import PV_INTERFACE
from . import cmdhandler
from .utils import vg_obj_path_generate, n, pv_obj_path_generate, \
	lv_object_path_method
from .loader import common
from .request import RequestEntry
from .state import State
from .utils import round_size


# noinspection PyUnusedLocal
def pvs_state_retrieve(selection, cache_refresh=True):
	rc = []

	if cache_refresh:
		cfg.db.refresh()

	for p in cfg.db.fetch_pvs(selection):
		rc.append(
			PvState(
				p["pv_name"], p["pv_uuid"], p["pv_name"],
				p["pv_fmt"], n(p["pv_size"]), n(p["pv_free"]),
				n(p["pv_used"]), n(p["dev_size"]), n(p["pv_mda_size"]),
				n(p["pv_mda_free"]), int(p["pv_ba_start"]),
				n(p["pv_ba_size"]), n(p["pe_start"]),
				int(p["pv_pe_count"]), int(p["pv_pe_alloc_count"]),
				p["pv_attr"], p["pv_tags"], p["vg_name"], p["vg_uuid"]))
	return rc


def load_pvs(device=None, object_path=None, refresh=False, emit_signal=False,
		cache_refresh=True):
	return common(
		pvs_state_retrieve, (Pv,), device, object_path, refresh,
		emit_signal, cache_refresh)


# noinspection PyUnresolvedReferences
class PvState(State):
	@property
	def lvm_id(self):
		return self.lvm_path

	def _lv_object_list(self, vg_name):
		rc = []
		if vg_name:
			for lv in sorted(cfg.db.pv_contained_lv(self.lvm_id)):
				lv_uuid, lv_name, meta, segs = lv
				full_name = "%s/%s" % (vg_name, lv_name)

				path_create = lv_object_path_method(lv_name, meta)
				lv_path = cfg.om.get_object_path_by_uuid_lvm_id(
					lv_uuid, full_name, path_create)

				rc.append((lv_path, segs))
		return rc

	# noinspection PyUnusedLocal,PyPep8Naming
	def __init__(self, lvm_path, Uuid, Name,
			Fmt, SizeBytes, FreeBytes, UsedBytes, DevSizeBytes,
			MdaSizeBytes, MdaFreeBytes, BaStart, BaSizeBytes,
			PeStart, PeCount, PeAllocCount, attr, Tags, vg_name,
			vg_uuid):
		utils.init_class_from_arguments(self)
		self.pe_segments = cfg.db.pv_pe_segments(Uuid)

		self.lv = self._lv_object_list(vg_name)

		# It's possible to have a vg_name and no uuid with the main example
		# being when the vg_name == '[unknown]'
		if vg_uuid and vg_name:
			self.vg_path = cfg.om.get_object_path_by_uuid_lvm_id(
				vg_uuid, vg_name, vg_obj_path_generate)
		else:
			self.vg_path = '/'

	def identifiers(self):
		return (self.Uuid, self.lvm_path)

	def create_dbus_object(self, path):
		if not path:
			path = cfg.om.get_object_path_by_uuid_lvm_id(self.Uuid, self.Name,
														pv_obj_path_generate)
		return Pv(path, self)

	# noinspection PyMethodMayBeStatic
	def creation_signature(self):
		return (Pv, pv_obj_path_generate)


# noinspection PyPep8Naming
@utils.dbus_property(PV_INTERFACE, 'Uuid', 's')  # PV UUID/pv_uuid
@utils.dbus_property(PV_INTERFACE, 'Name', 's')  # PV/pv_name
@utils.dbus_property(PV_INTERFACE, 'Fmt', 's')  # Fmt/pv_fmt
@utils.dbus_property(PV_INTERFACE, 'SizeBytes', 't')  # PSize/pv_size
@utils.dbus_property(PV_INTERFACE, 'FreeBytes', 't')  # PFree/pv_free
@utils.dbus_property(PV_INTERFACE, 'UsedBytes', 't')  # Used/pv_used
@utils.dbus_property(PV_INTERFACE, 'DevSizeBytes', 't')  # DevSize/dev_size
@utils.dbus_property(PV_INTERFACE, 'MdaSizeBytes', 't')  # PMdaSize/pv_mda_size
@utils.dbus_property(PV_INTERFACE, 'MdaFreeBytes', 't')  # PMdaFree/pv_mda_free
@utils.dbus_property(PV_INTERFACE, 'BaStart', 't')  # BA start/pv_ba_start
@utils.dbus_property(PV_INTERFACE, 'BaSizeBytes', 't')  # BA size/pv_ba_size
@utils.dbus_property(PV_INTERFACE, 'PeStart', 't')  # 1st PE/pe_start
@utils.dbus_property(PV_INTERFACE, 'PeCount', 't')  # PE/pv_pe_count
@utils.dbus_property(PV_INTERFACE, 'PeAllocCount', 't')  # PE Allocation count
class Pv(AutomatedProperties):
	# For properties that we need custom handlers we need these, otherwise
	# we won't get our introspection data
	_Tags_meta = ("as", PV_INTERFACE)
	_PeSegments_meta = ("a(tt)", PV_INTERFACE)
	_Exportable_meta = ("b", PV_INTERFACE)
	_Allocatable_meta = ("b", PV_INTERFACE)
	_Missing_meta = ("b", PV_INTERFACE)
	_Lv_meta = ("a(oa(tts))", PV_INTERFACE)
	_Vg_meta = ("o", PV_INTERFACE)

	# noinspection PyUnusedLocal,PyPep8Naming
	def __init__(self, object_path, state_obj):
		super(Pv, self).__init__(object_path, pvs_state_retrieve)
		self.set_interface(PV_INTERFACE)
		self.state = state_obj

	@staticmethod
	def _remove(pv_uuid, pv_name, remove_options):
		# Remove the PV, if successful then remove from the model
		# Make sure we have a dbus object representing it
		Pv.validate_dbus_object(pv_uuid, pv_name)
		rc, out, err = cmdhandler.pv_remove(pv_name, remove_options)
		Pv.handle_execute(rc, out, err)
		return '/'

	@staticmethod
	def handle_execute(rc, out, err):
		if rc == 0:
			cfg.load()
		else:
			# Need to work on error handling, need consistent
			raise dbus.exceptions.DBusException(
				PV_INTERFACE,
				'Exit code %s, stderr = %s' % (str(rc), err))

	@staticmethod
	def validate_dbus_object(pv_uuid, pv_name):
		dbo = cfg.om.get_object_by_uuid_lvm_id(pv_uuid, pv_name)
		if not dbo:
			raise dbus.exceptions.DBusException(
				PV_INTERFACE,
				'PV with uuid %s and name %s not present!' %
				(pv_uuid, pv_name))
		return dbo

	@dbus.service.method(
		dbus_interface=PV_INTERFACE,
		in_signature='ia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def Remove(self, tmo, remove_options, cb, cbe):
		r = RequestEntry(
			tmo, Pv._remove,
			(self.Uuid, self.lvm_id, remove_options),
			cb, cbe, return_tuple=False)
		cfg.worker_q.put(r)

	@staticmethod
	def _resize(pv_uuid, pv_name, new_size_bytes, resize_options):
		# Make sure we have a dbus object representing it
		Pv.validate_dbus_object(pv_uuid, pv_name)

		rc, out, err = cmdhandler.pv_resize(pv_name, new_size_bytes,
												resize_options)
		Pv.handle_execute(rc, out, err)
		return '/'

	@dbus.service.method(
		dbus_interface=PV_INTERFACE,
		in_signature='tia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def ReSize(self, new_size_bytes, tmo, resize_options, cb, cbe):
		r = RequestEntry(
			tmo, Pv._resize,
			(self.Uuid, self.lvm_id, round_size(new_size_bytes),
			resize_options), cb, cbe, False)
		cfg.worker_q.put(r)

	@staticmethod
	def _allocation_enabled(pv_uuid, pv_name, yes_no, allocation_options):
		# Make sure we have a dbus object representing it
		Pv.validate_dbus_object(pv_uuid, pv_name)
		rc, out, err = cmdhandler.pv_allocatable(
			pv_name, yes_no, allocation_options)
		Pv.handle_execute(rc, out, err)
		return '/'

	@dbus.service.method(
		dbus_interface=PV_INTERFACE,
		in_signature='bia{sv}',
		out_signature='o',
		async_callbacks=('cb', 'cbe'))
	def AllocationEnabled(self, yes, tmo, allocation_options, cb, cbe):
		r = RequestEntry(
			tmo, Pv._allocation_enabled,
			(self.Uuid, self.lvm_id,
			yes, allocation_options),
			cb, cbe, False)
		cfg.worker_q.put(r)

	@property
	def Tags(self):
		return utils.parse_tags(self.state.Tags)

	@property
	def PeSegments(self):
		if len(self.state.pe_segments):
			return dbus.Array(self.state.pe_segments, signature='(tt)')
		return dbus.Array([], '(tt)')

	@property
	def Exportable(self):
		return dbus.Boolean(self.state.attr[1] == 'x')

	@property
	def Allocatable(self):
		return dbus.Boolean(self.state.attr[0] == 'a')

	@property
	def Missing(self):
		return dbus.Boolean(self.state.attr[2] == 'm')

	def object_path(self):
		return self._object_path

	@property
	def lvm_id(self):
		return self.state.lvm_id

	@property
	def identifiers(self):
		return self.state.identifiers()

	@property
	def Lv(self):
		return dbus.Array(self.state.lv, signature="(oa(tts))")

	@property
	def Vg(self):
		return dbus.ObjectPath(self.state.vg_path)
