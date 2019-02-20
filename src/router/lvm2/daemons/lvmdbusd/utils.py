# Copyright (C) 2015-2016 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

import xml.etree.ElementTree as Et
import sys
import inspect
import ctypes
import os
import string
import datetime

import dbus
from lvmdbusd import cfg
# noinspection PyUnresolvedReferences
from gi.repository import GLib
import threading
import traceback
import signal

STDOUT_TTY = os.isatty(sys.stdout.fileno())


def rtype(dbus_type):
	"""
	Decorator making sure that the decorated function returns a value of
	specified type.
	:param dbus_type: The specific dbus type to return value as
	"""

	def decorator(fn):
		def decorated(*args, **kwargs):
			return dbus_type(fn(*args, **kwargs))

		return decorated

	return decorator


# Field is expected to be a number, handle the corner cases when parsing
@rtype(dbus.UInt64)
def n(v):
	if not v:
		return 0
	return int(float(v))


@rtype(dbus.UInt32)
def n32(v):
	if not v:
		return 0
	return int(float(v))


# noinspection PyProtectedMember
def init_class_from_arguments(obj_instance):
	for k, v in list(sys._getframe(1).f_locals.items()):
		if k != 'self':
			nt = k

			# If the current attribute has a value, but the incoming does
			# not, don't overwrite it.  Otherwise the default values on the
			# property decorator don't work as expected.
			cur = getattr(obj_instance, nt, v)

			# print 'Init class %s = %s' % (nt, str(v))
			if not (cur and len(str(cur)) and (v is None or len(str(v))) == 0):
				setattr(obj_instance, nt, v)


def get_properties(f):
	"""
	Walks through an object instance or it's parent class(es) and determines
	which attributes are properties and if they were created to be used for
	dbus.
	:param f:   Object to inspect
	:return:    A dictionary of tuples with each tuple being:
				0 = An array of dicts with the keys being: p_t, p_name,
				p_access(type, name, access)
				1 = Hash of property names and current value
	"""
	interfaces = dict()

	for c in inspect.getmro(f.__class__):

		h = vars(c)
		for p, value in h.items():
			if isinstance(value, property):
				# We found a property, see if it has a metadata type
				key = attribute_type_name(p)
				if key in h:
					interface = h[key][1]

					if interface not in interfaces:
						interfaces[interface] = ([], {})

					access = ''
					if getattr(f.__class__, p).fget:
						access += 'read'
					if getattr(f.__class__, p).fset:
						access += 'write'

					interfaces[interface][0].append(
						dict(
							p_t=getattr(f, key)[0],
							p_name=p,
							p_access=access))

					interfaces[interface][1][p] = getattr(f, p)

	return interfaces


def get_object_property_diff(o_prop, n_prop):
	"""
	Walk through each object properties and report what has changed and with
	the new values
	:param o_prop:   Old keys/values
	:param n_prop:   New keys/values
	:return: hash of properties that have changed and their new value
	"""
	rc = {}

	for intf_k, intf_v in o_prop.items():
		for k, v in list(intf_v[1].items()):
			# print('Comparing %s:%s to %s:%s' %
			#      (k, o_prop[intf_k][1][k], k, str(n_prop[intf_k][1][k])))
			if o_prop[intf_k][1][k] != n_prop[intf_k][1][k]:
				new_value = n_prop[intf_k][1][k]

				if intf_k not in rc:
					rc[intf_k] = dict()

				rc[intf_k][k] = new_value
	return rc


def add_properties(xml, interface, props):
	"""
	Given xml that describes the interface, add property values to the XML
	for the specified interface.
	:param xml:         XML to edit
	:param interface:   Interface to add the properties too
	:param props:       Output from get_properties
	:return: updated XML string
	"""
	if props:
		root = Et.fromstring(xml)
		interface_element = None

		# Check to see if interface is present
		for c in root:
			if c.attrib['name'] == interface:
				interface_element = c
				break

		# Interface is not present, lets create it so we have something to
		# attach the properties too
		if interface_element is None:
			interface_element = Et.Element("interface", name=interface)
			root.append(interface_element)

		# Add the properties
		for p in props:
			temp = '<property type="%s" name="%s" access="%s"/>\n' % \
				(p['p_t'], p['p_name'], p['p_access'])
			interface_element.append(Et.fromstring(temp))

		return Et.tostring(root, encoding='utf8')
	return xml


def attribute_type_name(name):
	"""
	Given the property name, return string of the attribute type
	:param name:
	:return:
	"""
	return "_%s_meta" % name


_type_map = dict(
	s=dbus.String,
	o=dbus.ObjectPath,
	t=dbus.UInt64,
	x=dbus.Int64,
	u=dbus.UInt32,
	i=dbus.Int32,
	n=dbus.Int16,
	q=dbus.UInt16,
	d=dbus.Double,
	y=dbus.Byte,
	b=dbus.Boolean)


def _pass_through(v):
	"""
	If we have something which is not a simple type we return the original
	value un-wrapped.
	:param v:
	:return:
	"""
	return v


def _dbus_type(t, value):
	return _type_map.get(t, _pass_through)(value)


def dbus_property(interface_name, name, dbus_type, doc=None):
	"""
	Creates the get/set properties for the given name.  It assumes that the
	actual attribute is '_' + name and the attribute metadata is stuffed in
	_name_type.

	There is probably a better way todo this.
	:param interface_name:  Dbus interface this property is associated with
	:param name:            Name of property
	:param dbus_type:       dbus string type eg. s,t,i,x
	:param doc:             Python __doc__ for the property
	:return:
	"""
	attribute_name = '_' + name

	def getter(self):
		t = getattr(self, attribute_name + '_meta')[0]
		return _dbus_type(t, getattr(self.state, attribute_name[1:]))

	prop = property(getter, None, None, doc)

	def decorator(cls):
		setattr(cls, attribute_name + '_meta', (dbus_type, interface_name))
		setattr(cls, name, prop)
		return cls

	return decorator


def parse_tags(tags):
	if len(tags):
		if ',' in tags:
			return tags.split(',')
		return dbus.Array(sorted([tags]), signature='s')
	return dbus.Array([], signature='s')


def _common_log(msg, *attributes):
	cfg.stdout_lock.acquire()
	tid = ctypes.CDLL('libc.so.6').syscall(186)

	if STDOUT_TTY:
		msg = "%s: %d:%d - %s" % \
			(datetime.datetime.now().strftime("%b %d %H:%M:%S.%f"),
			os.getpid(), tid, msg)

	else:
		msg = "%d:%d - %s" % (os.getpid(), tid, msg)

	if STDOUT_TTY and attributes:
		print(color(msg, *attributes))
	else:
		print(msg)

	cfg.stdout_lock.release()
	sys.stdout.flush()


# Serializes access to stdout to prevent interleaved output
# @param msg    Message to output to stdout
# @return None
def log_debug(msg, *attributes):
	if cfg.args and cfg.args.debug:
		_common_log(msg, *attributes)


def log_error(msg, *attributes):
	_common_log(msg, *attributes)


def dump_threads_stackframe():
	ident_to_name = {}

	for thread_object in threading.enumerate():
		ident_to_name[thread_object.ident] = thread_object

	stacks = []
	for thread_ident, frame in sys._current_frames().items():
		stack = traceback.format_list(traceback.extract_stack(frame))

		# There is a possibility that a thread gets created after we have
		# enumerated all threads, so this lookup table may be incomplete, so
		# account for this
		if thread_ident in ident_to_name:
			thread_name = ident_to_name[thread_ident].name
		else:
			thread_name = "unknown"

		stacks.append("Thread: %s" % (thread_name))
		stacks.append("".join(stack))

	log_error("Dumping thread stack frames!\n" + "\n".join(stacks))


# noinspection PyUnusedLocal
def handler(signum):
	try:
		if signum == signal.SIGUSR1:
			dump_threads_stackframe()
		else:
			cfg.run.value = 0
			log_debug('Exiting daemon with signal %d' % signum)
			if cfg.loop is not None:
				cfg.loop.quit()
	except:
		st = traceback.format_exc()
		log_error("signal handler: exception (logged, not reported!) \n %s" % st)

	# It's important we report that we handled the exception for the exception
	# handler to continue to work, especially for signal 10 (SIGUSR1)
	return True


def pv_obj_path_generate():
	return cfg.PV_OBJ_PATH + "/%d" % next(cfg.pv_id)


def vg_obj_path_generate():
	return cfg.VG_OBJ_PATH + "/%d" % next(cfg.vg_id)


def lv_object_path_method(name, meta):
	if name[0] == '[':
		return _hidden_lv_obj_path_generate
	elif meta[0][0] == 't':
		return _thin_pool_obj_path_generate
	elif meta[0][0] == 'C' and 'pool' in meta[1]:
		return _cache_pool_obj_path_generate

	return _lv_obj_path_generate


# Note: None of the individual LV path generate functions should be called
# directly, they should only be dispatched through lv_object_path_method

def _lv_obj_path_generate():
	return cfg.LV_OBJ_PATH + "/%d" % next(cfg.lv_id)


def _thin_pool_obj_path_generate():
	return cfg.THIN_POOL_PATH + "/%d" % next(cfg.thin_id)


def _cache_pool_obj_path_generate():
	return cfg.CACHE_POOL_PATH + "/%d" % next(cfg.cache_pool_id)


def _hidden_lv_obj_path_generate():
	return cfg.HIDDEN_LV_PATH + "/%d" % next(cfg.hidden_lv)


def job_obj_path_generate():
	return cfg.JOB_OBJ_PATH + "/%d" % next(cfg.job_id)


def color(text, *user_styles):
	styles = {
		# styles
		'reset': '\033[0m',
		'bold': '\033[01m',
		'disabled': '\033[02m',
		'underline': '\033[04m',
		'reverse': '\033[07m',
		'strike_through': '\033[09m',
		'invisible': '\033[08m',
		# text colors
		'fg_black': '\033[30m',
		'fg_red': '\033[31m',
		'fg_green': '\033[32m',
		'fg_orange': '\033[33m',
		'fg_blue': '\033[34m',
		'fg_purple': '\033[35m',
		'fg_cyan': '\033[36m',
		'fg_light_grey': '\033[37m',
		'fg_dark_grey': '\033[90m',
		'fg_light_red': '\033[91m',
		'fg_light_green': '\033[92m',
		'fg_yellow': '\033[93m',
		'fg_light_blue': '\033[94m',
		'fg_pink': '\033[95m',
		'fg_light_cyan': '\033[96m',
		# background colors
		'bg_black': '\033[40m',
		'bg_red': '\033[41m',
		'bg_green': '\033[42m',
		'bg_orange': '\033[43m',
		'bg_blue': '\033[44m',
		'bg_purple': '\033[45m',
		'bg_cyan': '\033[46m',
		'bg_light_grey': '\033[47m'
	}

	color_text = ''
	for style in user_styles:
		try:
			color_text += styles[style]
		except KeyError:
			return 'def color: parameter {} does not exist'.format(style)
	color_text += text
	return '\033[0m{0}\033[0m'.format(color_text)


def pv_range_append(cmd, device, start, end):
	if (start, end) == (0, 0):
		cmd.append(device)
	else:
		if start != 0 and end == 0:
			cmd.append("%s:%d-" % (device, start))
		else:
			cmd.append(
				"%s:%d-%d" %
				(device, start, end))


def pv_dest_ranges(cmd, pv_dest_range_list):
	if len(pv_dest_range_list):
		for i in pv_dest_range_list:
			pv_range_append(cmd, *i)


def round_size(size_bytes):
	bs = 512
	remainder = size_bytes % bs
	if not remainder:
		return size_bytes
	return size_bytes + bs - remainder


_ALLOWABLE_CH = string.ascii_letters + string.digits + '#+-.:=@_\/%'
_ALLOWABLE_CH_SET = set(_ALLOWABLE_CH)

_ALLOWABLE_VG_LV_CH = string.ascii_letters + string.digits + '.-_+'
_ALLOWABLE_VG_LV_CH_SET = set(_ALLOWABLE_VG_LV_CH)
_LV_NAME_RESERVED = ("_cdata", "_cmeta", "_corig", "_mimage", "_mlog",
	"_pmspare", "_rimage", "_rmeta", "_tdata", "_tmeta", "_vorigin")

# Tags can have the characters, based on the code
# a-zA-Z0-9._-+/=!:&#
_ALLOWABLE_TAG_CH = string.ascii_letters + string.digits + "._-+/=!:&#"
_ALLOWABLE_TAG_CH_SET = set(_ALLOWABLE_TAG_CH)


def _allowable_tag(tag_name):
	# LVM should impose a length restriction
	return set(tag_name) <= _ALLOWABLE_TAG_CH_SET


def _allowable_vg_name(vg_name):
	if vg_name is None:
		raise ValueError("VG name is None or empty")

	vg_len = len(vg_name)
	if vg_len == 0 or vg_len > 127:
		raise ValueError("VG name (%s) length (%d) not in the domain 1..127" %
			(vg_name, vg_len))

	if not set(vg_name) <= _ALLOWABLE_VG_LV_CH_SET:
		raise ValueError("VG name (%s) contains invalid character, "
			"allowable set(%s)" % (vg_name, _ALLOWABLE_VG_LV_CH))

	if vg_name == "." or vg_name == "..":
		raise ValueError('VG name (%s) cannot be "." or ".."' % (vg_name))


def _allowable_lv_name(vg_name, lv_name):

	if lv_name is None:
		raise ValueError("LV name is None or empty")

	lv_len = len(lv_name)

	# This length is derived from empirical testing
	if lv_len == 0 or (len(vg_name) + lv_len) > 125:
		raise ValueError("LV name (%s) length (%d) + VG name length "
			"not in the domain 1..125" % (lv_name, lv_len))

	if not set(lv_name) <= _ALLOWABLE_VG_LV_CH_SET:
		raise ValueError("LV name (%s) contains invalid character, "
			"allowable (%s)" % (lv_name, _ALLOWABLE_VG_LV_CH))

	if any(x in lv_name for x in _LV_NAME_RESERVED):
		raise ValueError("LV name (%s) contains a reserved word, "
			"reserved set(%s)" % (lv_name, str(_LV_NAME_RESERVED)))

	if lv_name.startswith("snapshot") or lv_name.startswith("pvmove"):
		raise ValueError("LV name (%s) starts with a reserved word, "
			"reserved set(%s)" % (lv_name, str(["snapshot", "pvmove"])))

	if lv_name[0] == '-':
		raise ValueError("LV name (%s) cannot start with a '-' "
				"character" % lv_name)


def validate_device_path(interface, device):
	if not set(device) <= _ALLOWABLE_CH_SET:
		raise dbus.exceptions.DBusException(
			interface, 'Device path (%s) has invalid characters, '
			'allowable (%s)' % (device, _ALLOWABLE_CH))


def validate_vg_name(interface, vg_name):
	try:
		_allowable_vg_name(vg_name)
	except ValueError as ve:
		raise dbus.exceptions.DBusException(
			interface, str(ve))


def validate_lv_name(interface, vg_name, lv_name):
	try:
		_allowable_lv_name(vg_name, lv_name)
	except ValueError as ve:
		raise dbus.exceptions.DBusException(
			interface, str(ve))


def validate_tag(interface, tag):
	if not _allowable_tag(tag):
		raise dbus.exceptions.DBusException(
			interface, 'tag (%s) contains invalid character, allowable set(%s)'
			% (tag, _ALLOWABLE_TAG_CH))


def add_no_notify(cmdline):
	"""
	Given a command line to execute we will see if `--config` is present, if it
	is we will add the global/notify_dbus=0 to it, otherwise we will append it
	to the end of the list.
	:param: cmdline: The command line to inspect
	:type: cmdline: list
	:return: cmdline with notify_dbus config option present
	:rtype: list
	"""

	# Only after we have seen an external event will be disable lvm from sending
	# us one when we call lvm
	if cfg.got_external_event:
		if 'help' in cmdline:
			return cmdline

		if '--config' in cmdline:
			for i, arg in enumerate(cmdline):
				if arg == '--config':
					if len(cmdline) <= i+1:
						raise dbus.exceptions.DBusException("Missing value for --config option.")
					cmdline[i+1] += " global/notify_dbus=0"
					break
		else:
			cmdline.extend(['--config', 'global/notify_dbus=0'])
	return cmdline


# The methods below which start with mt_* are used to execute the desired code
# on the the main thread of execution to alleviate any issues the dbus-python
# library with regards to multi-threaded access.  Essentially, we are trying to
# ensure all dbus library interaction is done from the same thread!


def _async_handler(call_back, parameters):
	params_str = ", ".join(str(x) for x in parameters)
	log_debug('Main thread execution, callback = %s, parameters = (%s)' %
				(str(call_back), params_str))

	try:
		if parameters:
			call_back(*parameters)
		else:
			call_back()
	except:
		st = traceback.format_exc()
		log_error("mt_async_call: exception (logged, not reported!) \n %s" % st)


# Execute the function on the main thread with the provided parameters, do
# not return *any* value or wait for the execution to complete!
def mt_async_call(function_call_back, *parameters):
	GLib.idle_add(_async_handler, function_call_back, parameters)


# Run the supplied function and arguments on the main thread and wait for them
# to complete while allowing the ability to get the return value too.
#
# Example:
# result = MThreadRunner(foo, arg1, arg2).done()
#
class MThreadRunner(object):

	@staticmethod
	def runner(obj):
		# noinspection PyProtectedMember
		obj._run()
		with obj.cond:
			obj.function_complete = True
			obj.cond.notify_all()

	def __init__(self, function, *args):
		self.f = function
		self.rc = None
		self.exception = None
		self.args = args
		self.function_complete = False
		self.cond = threading.Condition(threading.Lock())

	def done(self):
		GLib.idle_add(MThreadRunner.runner, self)
		with self.cond:
			if not self.function_complete:
				self.cond.wait()
		if self.exception:
			raise self.exception
		return self.rc

	def _run(self):
		try:
			if self.args:
				self.rc = self.f(*self.args)
			else:
				self.rc = self.f()
		except BaseException as be:
			self.exception = be
			st = traceback.format_exc()
			log_error("MThreadRunner: exception \n %s" % st)
			log_error("Exception will be raised in calling thread!")


def _remove_objects(dbus_objects_rm):
	for o in dbus_objects_rm:
		cfg.om.remove_object(o, emit_signal=True)


# Remove dbus objects from main thread
def mt_remove_dbus_objects(objs):
	MThreadRunner(_remove_objects, objs).done()
