# Copyright (C) 2015-2016 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

import subprocess
from . import cfg
from .cmdhandler import options_to_cli_args, LvmExecutionMeta
import dbus
from .utils import pv_range_append, pv_dest_ranges, log_error, log_debug,\
	add_no_notify
import os
import threading
import time


def pv_move_lv_cmd(move_options, lv_full_name,
					pv_source, pv_source_range, pv_dest_range_list):
	cmd = ['pvmove', '-i', '1']
	cmd.extend(options_to_cli_args(move_options))

	if lv_full_name:
		cmd.extend(['-n', lv_full_name])

	pv_range_append(cmd, pv_source, *pv_source_range)
	pv_dest_ranges(cmd, pv_dest_range_list)

	return cmd


def lv_merge_cmd(merge_options, lv_full_name):
	cmd = ['lvconvert', '--merge', '-i', '1']
	cmd.extend(options_to_cli_args(merge_options))
	cmd.append(lv_full_name)
	return cmd


def _move_merge(interface_name, command, job_state):
	# We need to execute these command stand alone by forking & exec'ing
	# the command always as we will be getting periodic output from them on
	# the status of the long running operation.
	command.insert(0, cfg.LVM_CMD)

	# Instruct lvm to not register an event with us
	command = add_no_notify(command)

	#(self, start, ended, cmd, ec, stdout_txt, stderr_txt)
	meta = LvmExecutionMeta(time.time(), 0, command, -1000, None, None)

	cfg.blackbox.add(meta)

	process = subprocess.Popen(command, stdout=subprocess.PIPE,
								env=os.environ,
								stderr=subprocess.PIPE, close_fds=True)

	log_debug("Background process for %s is %d" %
				(str(command), process.pid))

	lines_iterator = iter(process.stdout.readline, b"")
	for line in lines_iterator:
		line_str = line.decode("utf-8")

		# Check to see if the line has the correct number of separators
		try:
			if line_str.count(':') == 2:
				(device, ignore, percentage) = line_str.split(':')
				job_state.Percent = round(
					float(percentage.strip()[:-1]), 1)

				# While the move is in progress we need to periodically update
				# the state to reflect where everything is at.
				cfg.load()
		except ValueError:
			log_error("Trying to parse percentage which failed for %s" %
				line_str)

	out = process.communicate()

	with meta.lock:
		meta.ended = time.time()
		meta.ec = process.returncode
		meta.stderr_txt = out[1]

	if process.returncode == 0:
		job_state.Percent = 100
	else:
		raise dbus.exceptions.DBusException(
			interface_name,
			'Exit code %s, stderr = %s' % (str(process.returncode), out[1]))

	cfg.load()
	return '/'


def move(interface_name, lv_name, pv_src_obj, pv_source_range,
			pv_dests_and_ranges, move_options, job_state):
	"""
	Common code for the pvmove handling.
	:param interface_name:  What dbus interface we are providing for
	:param lv_name:     Optional (None or name of LV to move)
	:param pv_src_obj:  dbus object patch for source PV
	:param pv_source_range: (0,0 to ignore, else start, end segments)
	:param pv_dests_and_ranges: Array of PV object paths and start/end segs
	:param move_options: Hash with optional arguments
	:param job_state: Used to convey information about jobs between processes
	:return: '/' When complete, the empty object path
	"""
	pv_dests = []
	pv_src = cfg.om.get_object_by_path(pv_src_obj)
	if pv_src:

		# Check to see if we are handling a move to a specific
		# destination(s)
		if len(pv_dests_and_ranges):
			for pr in pv_dests_and_ranges:
				pv_dbus_obj = cfg.om.get_object_by_path(pr[0])
				if not pv_dbus_obj:
					raise dbus.exceptions.DBusException(
						interface_name,
						'PV Destination (%s) not found' % pr[0])

				pv_dests.append((pv_dbus_obj.lvm_id, pr[1], pr[2]))

		cmd = pv_move_lv_cmd(move_options,
								lv_name,
								pv_src.lvm_id,
								pv_source_range,
								pv_dests)

		return _move_merge(interface_name, cmd, job_state)
	else:
		raise dbus.exceptions.DBusException(
			interface_name, 'pv_src_obj (%s) not found' % pv_src_obj)


def merge(interface_name, lv_uuid, lv_name, merge_options, job_state):
	# Make sure we have a dbus object representing it
	dbo = cfg.om.get_object_by_uuid_lvm_id(lv_uuid, lv_name)
	if dbo:
		cmd = lv_merge_cmd(merge_options, dbo.lvm_id)
		return _move_merge(interface_name, cmd, job_state)
	else:
		raise dbus.exceptions.DBusException(
			interface_name,
			'LV with uuid %s and name %s not present!' % (lv_uuid, lv_name))


def _run_cmd(req):
	log_debug(
		"_run_cmd: Running method: %s with args %s" %
		(str(req.method), str(req.arguments)))
	req.run_cmd()
	log_debug("_run_cmd: complete!")


def cmd_runner(request):
	t = threading.Thread(target=_run_cmd, args=(request,),
							name="cmd_runner %s" % str(request.method))
	t.start()
