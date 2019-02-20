# Copyright (C) 2015-2016 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

from subprocess import Popen, PIPE
import time
import threading
from itertools import chain
import collections
import traceback
import os

from lvmdbusd import cfg
from lvmdbusd.utils import pv_dest_ranges, log_debug, log_error, add_no_notify
from lvmdbusd.lvm_shell_proxy import LVMShellProxy

try:
	import simplejson as json
except ImportError:
	import json

SEP = '{|}'

total_time = 0.0
total_count = 0

# We need to prevent different threads from using the same lvm shell
# at the same time.
cmd_lock = threading.RLock()


class LvmExecutionMeta(object):

	def __init__(self, start, ended, cmd, ec, stdout_txt, stderr_txt):
		self.lock = threading.RLock()
		self.start = start
		self.ended = ended
		self.cmd = cmd
		self.ec = ec
		self.stdout_txt = stdout_txt
		self.stderr_txt = stderr_txt

	def __str__(self):
		with self.lock:
			return "EC= %d for %s\n" \
				"STARTED: %f, ENDED: %f\n" \
				"STDOUT=%s\n" \
				"STDERR=%s\n" % \
				(self.ec, str(self.cmd), self.start, self.ended, self.stdout_txt,
				self.stderr_txt)


class LvmFlightRecorder(object):

	def __init__(self, size=16):
		self.queue = collections.deque(maxlen=size)

	def add(self, lvm_exec_meta):
		self.queue.append(lvm_exec_meta)

	def dump(self):
		with cmd_lock:
			if len(self.queue):
				log_error("LVM dbus flight recorder START")
				for c in reversed(self.queue):
					log_error(str(c))
				log_error("LVM dbus flight recorder END")


cfg.blackbox = LvmFlightRecorder()


def _debug_c(cmd, exit_code, out):
	log_error('CMD= %s' % ' '.join(cmd))
	log_error(("EC= %d" % exit_code))
	log_error(("STDOUT=\n %s\n" % out[0]))
	log_error(("STDERR=\n %s\n" % out[1]))


def call_lvm(command, debug=False):
	"""
	Call an executable and return a tuple of exitcode, stdout, stderr
	:param command:     Command to execute
	:param debug:       Dump debug to stdout
	"""
	# print 'STACK:'
	# for line in traceback.format_stack():
	#    print line.strip()

	# Prepend the full lvm executable so that we can run different versions
	# in different locations on the same box
	command.insert(0, cfg.LVM_CMD)
	command = add_no_notify(command)

	process = Popen(command, stdout=PIPE, stderr=PIPE, close_fds=True,
					env=os.environ)
	out = process.communicate()

	stdout_text = bytes(out[0]).decode("utf-8")
	stderr_text = bytes(out[1]).decode("utf-8")

	if debug or process.returncode != 0:
		_debug_c(command, process.returncode, (stdout_text, stderr_text))

	return process.returncode, stdout_text, stderr_text

# The actual method which gets called to invoke the lvm command, can vary
# from forking a new process to using lvm shell
_t_call = call_lvm


def _shell_cfg():
	global _t_call
	# noinspection PyBroadException
	try:
		lvm_shell = LVMShellProxy()
		_t_call = lvm_shell.call_lvm
		cfg.SHELL_IN_USE = lvm_shell
		return True
	except Exception:
		_t_call = call_lvm
		cfg.SHELL_IN_USE = None
		log_error(traceback.format_exc())
		log_error("Unable to utilize lvm shell, dropping back to fork & exec")
		return False


def set_execution(shell):
	global _t_call
	with cmd_lock:
		# If the user requested lvm shell and we are currently setup that
		# way, just return
		if cfg.SHELL_IN_USE and shell:
			return True
		else:
			if not shell and cfg.SHELL_IN_USE:
				cfg.SHELL_IN_USE.exit_shell()
				cfg.SHELL_IN_USE = None

		_t_call = call_lvm
		if shell:
			if cfg.args.use_json:
				return _shell_cfg()
			else:
				return False
		return True


def time_wrapper(command, debug=False):
	global total_time
	global total_count

	with cmd_lock:
		start = time.time()
		results = _t_call(command, debug)
		ended = time.time()
		total_time += (ended - start)
		total_count += 1
		cfg.blackbox.add(LvmExecutionMeta(start, ended, command, *results))
	return results


call = time_wrapper


# Default cmd
# Place default arguments for every command here.
def _dc(cmd, args):
	c = [cmd, '--noheading', '--separator', '%s' % SEP, '--nosuffix',
		'--unbuffered', '--units', 'b']
	c.extend(args)
	return c


def parse(out):
	rc = []

	for line in out.split('\n'):
		# This line includes separators, so process them
		if SEP in line:
			elem = line.split(SEP)
			cleaned_elem = []
			for e in elem:
				e = e.strip()
				cleaned_elem.append(e)

			if len(cleaned_elem) > 1:
				rc.append(cleaned_elem)
		else:
			t = line.strip()
			if len(t) > 0:
				rc.append(t)
	return rc


def parse_column_names(out, column_names):
	lines = parse(out)
	rc = []

	for i in range(0, len(lines)):
		d = dict(list(zip(column_names, lines[i])))
		rc.append(d)

	return rc


def options_to_cli_args(options):
	rc = []
	for k, v in list(dict(options).items()):
		if k.startswith("-"):
			rc.append(k)
		else:
			rc.append("--%s" % k)
		if v != "":
			rc.append(str(v))
	return rc


def pv_remove(device, remove_options):
	cmd = ['pvremove']
	cmd.extend(options_to_cli_args(remove_options))
	cmd.append(device)
	return call(cmd)


def _qt(tag_name):
	return '@%s' % tag_name


def _tag(operation, what, add, rm, tag_options):
	cmd = [operation]
	cmd.extend(options_to_cli_args(tag_options))

	if isinstance(what, list):
		cmd.extend(what)
	else:
		cmd.append(what)

	if add:
		cmd.extend(list(chain.from_iterable(
			('--addtag', _qt(x)) for x in add)))
	if rm:
		cmd.extend(list(chain.from_iterable(
			('--deltag', _qt(x)) for x in rm)))

	return call(cmd, False)


def pv_tag(pv_devices, add, rm, tag_options):
	return _tag('pvchange', pv_devices, add, rm, tag_options)


def vg_tag(vg_name, add, rm, tag_options):
	return _tag('vgchange', vg_name, add, rm, tag_options)


def lv_tag(lv_name, add, rm, tag_options):
	return _tag('lvchange', lv_name, add, rm, tag_options)


def vg_rename(vg, new_name, rename_options):
	cmd = ['vgrename']
	cmd.extend(options_to_cli_args(rename_options))
	cmd.extend([vg, new_name])
	return call(cmd)


def vg_remove(vg_name, remove_options):
	cmd = ['vgremove']
	cmd.extend(options_to_cli_args(remove_options))
	cmd.extend(['-f', vg_name])
	return call(cmd)


def vg_lv_create(vg_name, create_options, name, size_bytes, pv_dests):
	cmd = ['lvcreate']
	cmd.extend(options_to_cli_args(create_options))
	cmd.extend(['--size', str(size_bytes) + 'B'])
	cmd.extend(['--name', name, vg_name, '--yes'])
	pv_dest_ranges(cmd, pv_dests)
	return call(cmd)


def vg_lv_snapshot(vg_name, snapshot_options, name, size_bytes):
	cmd = ['lvcreate']
	cmd.extend(options_to_cli_args(snapshot_options))
	cmd.extend(["-s"])

	if size_bytes != 0:
		cmd.extend(['--size', str(size_bytes) + 'B'])

	cmd.extend(['--name', name, vg_name])
	return call(cmd)


def _vg_lv_create_common_cmd(create_options, size_bytes, thin_pool):
	cmd = ['lvcreate']
	cmd.extend(options_to_cli_args(create_options))

	if not thin_pool:
		cmd.extend(['--size', str(size_bytes) + 'B'])
	else:
		cmd.extend(['--thin', '--size', str(size_bytes) + 'B'])

	cmd.extend(['--yes'])
	return cmd


def vg_lv_create_linear(vg_name, create_options, name, size_bytes, thin_pool):
	cmd = _vg_lv_create_common_cmd(create_options, size_bytes, thin_pool)
	cmd.extend(['--name', name, vg_name])
	return call(cmd)


def vg_lv_create_striped(vg_name, create_options, name, size_bytes,
							num_stripes, stripe_size_kb, thin_pool):
	cmd = _vg_lv_create_common_cmd(create_options, size_bytes, thin_pool)
	cmd.extend(['--stripes', str(num_stripes)])

	if stripe_size_kb != 0:
		cmd.extend(['--stripesize', str(stripe_size_kb)])

	cmd.extend(['--name', name, vg_name])
	return call(cmd)


def _vg_lv_create_raid(vg_name, create_options, name, raid_type, size_bytes,
						num_stripes, stripe_size_kb):
	cmd = ['lvcreate']

	cmd.extend(options_to_cli_args(create_options))

	cmd.extend(['--type', raid_type])
	cmd.extend(['--size', str(size_bytes) + 'B'])

	if num_stripes != 0:
		cmd.extend(['--stripes', str(num_stripes)])

	if stripe_size_kb != 0:
		cmd.extend(['--stripesize', str(stripe_size_kb)])

	cmd.extend(['--name', name, vg_name, '--yes'])
	return call(cmd)


def vg_lv_create_raid(vg_name, create_options, name, raid_type, size_bytes,
						num_stripes, stripe_size_kb):
	cmd = ['lvcreate']
	cmd.extend(options_to_cli_args(create_options))

	return _vg_lv_create_raid(vg_name, create_options, name, raid_type,
								size_bytes, num_stripes, stripe_size_kb)


def vg_lv_create_mirror(
		vg_name, create_options, name, size_bytes, num_copies):
	cmd = ['lvcreate']
	cmd.extend(options_to_cli_args(create_options))

	cmd.extend(['--type', 'mirror'])
	cmd.extend(['--mirrors', str(num_copies)])
	cmd.extend(['--size', str(size_bytes) + 'B'])
	cmd.extend(['--name', name, vg_name, '--yes'])
	return call(cmd)


def vg_create_cache_pool(md_full_name, data_full_name, create_options):
	cmd = ['lvconvert']
	cmd.extend(options_to_cli_args(create_options))
	cmd.extend(['--type', 'cache-pool', '--force', '-y',
				'--poolmetadata', md_full_name, data_full_name])
	return call(cmd)


def vg_create_thin_pool(md_full_name, data_full_name, create_options):
	cmd = ['lvconvert']
	cmd.extend(options_to_cli_args(create_options))
	cmd.extend(['--type', 'thin-pool', '--force', '-y',
				'--poolmetadata', md_full_name, data_full_name])
	return call(cmd)


def lv_remove(lv_path, remove_options):
	cmd = ['lvremove']
	cmd.extend(options_to_cli_args(remove_options))
	cmd.extend(['-f', lv_path])
	return call(cmd)


def lv_rename(lv_path, new_name, rename_options):
	cmd = ['lvrename']
	cmd.extend(options_to_cli_args(rename_options))
	cmd.extend([lv_path, new_name])
	return call(cmd)


def lv_resize(lv_full_name, size_change, pv_dests,
				resize_options):
	cmd = ['lvresize', '--force']

	cmd.extend(options_to_cli_args(resize_options))

	if size_change < 0:
		cmd.append("-L-%dB" % (-size_change))
	else:
		cmd.append("-L+%dB" % (size_change))

	cmd.append(lv_full_name)
	pv_dest_ranges(cmd, pv_dests)
	return call(cmd)


def lv_lv_create(lv_full_name, create_options, name, size_bytes):
	cmd = ['lvcreate']
	cmd.extend(options_to_cli_args(create_options))
	cmd.extend(['--virtualsize', str(size_bytes) + 'B', '-T'])
	cmd.extend(['--name', name, lv_full_name, '--yes'])
	return call(cmd)


def lv_cache_lv(cache_pool_full_name, lv_full_name, cache_options):
	# lvconvert --type cache --cachepool VG/CachePoolLV VG/OriginLV
	cmd = ['lvconvert']
	cmd.extend(options_to_cli_args(cache_options))
	cmd.extend(['-y', '--type', 'cache', '--cachepool',
				cache_pool_full_name, lv_full_name])
	return call(cmd)


def lv_detach_cache(lv_full_name, detach_options, destroy_cache):
	cmd = ['lvconvert']
	if destroy_cache:
		option = '--uncache'
	else:
		# Currently fairly dangerous
		# see: https://bugzilla.redhat.com/show_bug.cgi?id=1248972
		option = '--splitcache'
	cmd.extend(options_to_cli_args(detach_options))
	# needed to prevent interactive questions
	cmd.extend(["--yes", "--force"])
	cmd.extend([option, lv_full_name])
	return call(cmd)


def supports_json():
	cmd = ['help']
	rc, out, err = call(cmd)
	if rc == 0:
		if cfg.SHELL_IN_USE:
			return True
		else:
			if 'fullreport' in err:
				return True
	return False


def lvm_full_report_json():
	pv_columns = ['pv_name', 'pv_uuid', 'pv_fmt', 'pv_size', 'pv_free',
					'pv_used', 'dev_size', 'pv_mda_size', 'pv_mda_free',
					'pv_ba_start', 'pv_ba_size', 'pe_start', 'pv_pe_count',
					'pv_pe_alloc_count', 'pv_attr', 'pv_tags', 'vg_name',
					'vg_uuid', 'pv_missing']

	pv_seg_columns = ['pvseg_start', 'pvseg_size', 'segtype',
						'pv_uuid', 'lv_uuid', 'pv_name']

	vg_columns = ['vg_name', 'vg_uuid', 'vg_fmt', 'vg_size', 'vg_free',
					'vg_sysid', 'vg_extent_size', 'vg_extent_count',
					'vg_free_count', 'vg_profile', 'max_lv', 'max_pv',
					'pv_count', 'lv_count', 'snap_count', 'vg_seqno',
					'vg_mda_count', 'vg_mda_free', 'vg_mda_size',
					'vg_mda_used_count', 'vg_attr', 'vg_tags']

	lv_columns = ['lv_uuid', 'lv_name', 'lv_path', 'lv_size',
				'vg_name', 'pool_lv_uuid', 'pool_lv', 'origin_uuid',
				'origin', 'data_percent',
				'lv_attr', 'lv_tags', 'vg_uuid', 'lv_active', 'data_lv',
				'metadata_lv', 'lv_parent', 'lv_role', 'lv_layout',
				'snap_percent', 'metadata_percent', 'copy_percent',
				'sync_percent', 'lv_metadata_size', 'move_pv', 'move_pv_uuid']

	lv_seg_columns = ['seg_pe_ranges', 'segtype', 'lv_uuid']

	cmd = _dc('fullreport', [
		'-a',		# Need hidden too
		'--configreport', 'pv', '-o', ','.join(pv_columns),
		'--configreport', 'vg', '-o', ','.join(vg_columns),
		'--configreport', 'lv', '-o', ','.join(lv_columns),
		'--configreport', 'seg', '-o', ','.join(lv_seg_columns),
		'--configreport', 'pvseg', '-o', ','.join(pv_seg_columns),
		'--reportformat', 'json'
	])

	rc, out, err = call(cmd)
	if rc == 0:
		# With the current implementation, if we are using the shell then we
		# are using JSON and JSON is returned back to us as it was parsed to
		# figure out if we completed OK or not
		if cfg.SHELL_IN_USE:
			assert(type(out) == dict)
			return out
		else:
			return json.loads(out)
	return None


def pv_retrieve_with_segs(device=None):
	d = []
	err = ""
	out = ""
	rc = 0

	columns = ['pv_name', 'pv_uuid', 'pv_fmt', 'pv_size', 'pv_free',
				'pv_used', 'dev_size', 'pv_mda_size', 'pv_mda_free',
				'pv_ba_start', 'pv_ba_size', 'pe_start', 'pv_pe_count',
				'pv_pe_alloc_count', 'pv_attr', 'pv_tags', 'vg_name',
				'vg_uuid', 'pvseg_start', 'pvseg_size', 'segtype', 'pv_missing']

	# Lvm has some issues where it returns failure when querying pvs when other
	# operations are in process, see:
	# https://bugzilla.redhat.com/show_bug.cgi?id=1274085
	for i in range(0, 10):
		cmd = _dc('pvs', ['-o', ','.join(columns)])

		if device:
			cmd.extend(device)

		rc, out, err = call(cmd)

		if rc == 0:
			d = parse_column_names(out, columns)
			break
		else:
			time.sleep(0.2)
			log_debug("LVM Bug workaround, retrying pvs command...")

	if rc != 0:
		msg = "We were unable to get pvs to return without error after " \
			"trying 10 times, RC=%d, STDERR=(%s), STDOUT=(%s)" % \
			(rc, err, out)
		log_error(msg)
		raise RuntimeError(msg)

	return d


def pv_resize(device, size_bytes, create_options):
	cmd = ['pvresize']

	cmd.extend(options_to_cli_args(create_options))

	if size_bytes != 0:
		cmd.extend(['--yes', '--setphysicalvolumesize', str(size_bytes) + 'B'])

	cmd.extend([device])
	return call(cmd)


def pv_create(create_options, devices):
	cmd = ['pvcreate', '-ff']
	cmd.extend(options_to_cli_args(create_options))
	cmd.extend(devices)
	return call(cmd)


def pv_allocatable(device, yes, allocation_options):
	yn = 'n'

	if yes:
		yn = 'y'

	cmd = ['pvchange']
	cmd.extend(options_to_cli_args(allocation_options))
	cmd.extend(['-x', yn, device])
	return call(cmd)


def pv_scan(activate, cache, device_paths, major_minors, scan_options):
	cmd = ['pvscan']
	cmd.extend(options_to_cli_args(scan_options))

	if activate:
		cmd.extend(['--activate', "ay"])

	if cache:
		cmd.append('--cache')

		if len(device_paths) > 0:
			for d in device_paths:
				cmd.append(d)

		if len(major_minors) > 0:
			for mm in major_minors:
				cmd.append("%s:%s" % (mm))

	return call(cmd)


def vg_create(create_options, pv_devices, name):
	cmd = ['vgcreate']
	cmd.extend(options_to_cli_args(create_options))
	cmd.append(name)
	cmd.extend(pv_devices)
	return call(cmd)


def vg_change(change_options, name):
	cmd = ['vgchange']
	cmd.extend(options_to_cli_args(change_options))
	cmd.append(name)
	return call(cmd)


def vg_reduce(vg_name, missing, pv_devices, reduce_options):
	cmd = ['vgreduce']
	cmd.extend(options_to_cli_args(reduce_options))

	if missing:
		cmd.append('--removemissing')
	elif len(pv_devices) == 0:
		cmd.append('--all')

	cmd.append(vg_name)
	cmd.extend(pv_devices)
	return call(cmd)


def vg_extend(vg_name, extend_devices, extend_options):
	cmd = ['vgextend']
	cmd.extend(options_to_cli_args(extend_options))
	cmd.append(vg_name)
	cmd.extend(extend_devices)
	return call(cmd)


def _vg_value_set(name, arguments, options):
	cmd = ['vgchange']
	cmd.extend(options_to_cli_args(options))
	cmd.append(name)
	cmd.extend(arguments)
	return call(cmd)


def vg_allocation_policy(vg_name, policy, policy_options):
	return _vg_value_set(vg_name, ['--alloc', policy], policy_options)


def vg_max_pv(vg_name, number, max_options):
	return _vg_value_set(vg_name, ['--maxphysicalvolumes', str(number)],
							max_options)


def vg_max_lv(vg_name, number, max_options):
	return _vg_value_set(vg_name, ['-l', str(number)], max_options)


def vg_uuid_gen(vg_name, ignore, options):
	assert ignore is None
	return _vg_value_set(vg_name, ['--uuid'], options)


def activate_deactivate(op, name, activate, control_flags, options):
	cmd = [op]
	cmd.extend(options_to_cli_args(options))

	op = '-a'

	if control_flags:
		# Autoactivation
		if (1 << 0) & control_flags:
			op += 'a'
		# Exclusive locking (Cluster)
		if (1 << 1) & control_flags:
			op += 'e'

		# Local node activation
		if (1 << 2) & control_flags:
			op += 'l'

		# Activation modes
		if (1 << 3) & control_flags:
			cmd.extend(['--activationmode', 'complete'])
		elif (1 << 4) & control_flags:
			cmd.extend(['--activationmode', 'partial'])

		# Ignore activation skip
		if (1 << 5) & control_flags:
			cmd.append('--ignoreactivationskip')

	if activate:
		op += 'y'
	else:
		op += 'n'

	cmd.append(op)
	cmd.append(name)
	return call(cmd)


def vg_retrieve(vg_specific):
	if vg_specific:
		assert isinstance(vg_specific, list)

	columns = ['vg_name', 'vg_uuid', 'vg_fmt', 'vg_size', 'vg_free',
				'vg_sysid', 'vg_extent_size', 'vg_extent_count',
				'vg_free_count', 'vg_profile', 'max_lv', 'max_pv',
				'pv_count', 'lv_count', 'snap_count', 'vg_seqno',
				'vg_mda_count', 'vg_mda_free', 'vg_mda_size',
				'vg_mda_used_count', 'vg_attr', 'vg_tags']

	cmd = _dc('vgs', ['-o', ','.join(columns)])

	if vg_specific:
		cmd.extend(vg_specific)

	d = []
	rc, out, err = call(cmd)
	if rc == 0:
		d = parse_column_names(out, columns)

	return d


def lv_retrieve_with_segments():
	columns = ['lv_uuid', 'lv_name', 'lv_path', 'lv_size',
				'vg_name', 'pool_lv_uuid', 'pool_lv', 'origin_uuid',
				'origin', 'data_percent',
				'lv_attr', 'lv_tags', 'vg_uuid', 'lv_active', 'data_lv',
				'metadata_lv', 'seg_pe_ranges', 'segtype', 'lv_parent',
				'lv_role', 'lv_layout',
				'snap_percent', 'metadata_percent', 'copy_percent',
				'sync_percent', 'lv_metadata_size', 'move_pv', 'move_pv_uuid']

	cmd = _dc('lvs', ['-a', '-o', ','.join(columns)])
	rc, out, err = call(cmd)

	d = []

	if rc == 0:
		d = parse_column_names(out, columns)

	return d


if __name__ == '__main__':
	pv_data = pv_retrieve_with_segs()

	for p in pv_data:
		print(str(p))
