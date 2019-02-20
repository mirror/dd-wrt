# Copyright (C) 2015-2016 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

from .pv import load_pvs
from .vg import load_vgs
from .lv import load_lvs
from . import cfg
from .utils import MThreadRunner, log_debug, log_error
import threading
import queue
import traceback


def _main_thread_load(refresh=True, emit_signal=True):
	num_total_changes = 0

	num_total_changes += load_pvs(
		refresh=refresh,
		emit_signal=emit_signal,
		cache_refresh=False)[1]
	num_total_changes += load_vgs(
		refresh=refresh,
		emit_signal=emit_signal,
		cache_refresh=False)[1]
	num_total_changes += load_lvs(
		refresh=refresh,
		emit_signal=emit_signal,
		cache_refresh=False)[1]

	return num_total_changes


def load(refresh=True, emit_signal=True, cache_refresh=True, log=True,
			need_main_thread=True):
	# Go through and load all the PVs, VGs and LVs
	if cache_refresh:
		cfg.db.refresh(log)

	if need_main_thread:
		rc = MThreadRunner(_main_thread_load, refresh, emit_signal).done()
	else:
		rc = _main_thread_load(refresh, emit_signal)

	return rc


# Even though lvm can handle multiple changes concurrently it really doesn't
# make sense to make a 1-1 fetch of data for each change of lvm because when
# we fetch the data once all previous changes are reflected.
class StateUpdate(object):

	class UpdateRequest(object):

		def __init__(self, refresh, emit_signal, cache_refresh, log,
						need_main_thread):
			self.is_done = False
			self.refresh = refresh
			self.emit_signal = emit_signal
			self.cache_refresh = cache_refresh
			self.log = log
			self.need_main_thread = need_main_thread
			self.result = None
			self.cond = threading.Condition(threading.Lock())

		def done(self):
			with self.cond:
				if not self.is_done:
					self.cond.wait()
			return self.result

		def set_result(self, result):
			with self.cond:
				self.result = result
				self.is_done = True
				self.cond.notify_all()

	@staticmethod
	def update_thread(obj):
		queued_requests = []
		while cfg.run.value != 0:
			# noinspection PyBroadException
			try:
				refresh = True
				emit_signal = True
				cache_refresh = True
				log = True
				need_main_thread = True

				with obj.lock:
					wait = not obj.deferred
					obj.deferred = False

				if len(queued_requests) == 0 and wait:
					queued_requests.append(obj.queue.get(True, 2))

				# Ok we have one or the deferred queue has some,
				# check if any others
				try:
					while True:
						queued_requests.append(obj.queue.get(False))

				except queue.Empty:
					pass

				if len(queued_requests) > 1:
					log_debug("Processing %d updates!" % len(queued_requests),
							'bg_black', 'fg_light_green')

				# We have what we can, run the update with the needed options
				for i in queued_requests:
					if not i.refresh:
						refresh = False
					if not i.emit_signal:
						emit_signal = False
					if not i.cache_refresh:
						cache_refresh = False
					if not i.log:
						log = False
					if not i.need_main_thread:
						need_main_thread = False

				num_changes = load(refresh, emit_signal, cache_refresh, log,
									need_main_thread)
				# Update is done, let everyone know!
				for i in queued_requests:
					i.set_result(num_changes)

				# Only clear out the requests after we have given them a result
				# otherwise we can orphan the waiting threads and they never
				# wake up if we get an exception
				queued_requests = []

			except queue.Empty:
				pass
			except Exception:
				st = traceback.format_exc()
				log_error("update_thread exception: \n%s" % st)
				cfg.blackbox.dump()

	def __init__(self):
		self.lock = threading.RLock()
		self.queue = queue.Queue()
		self.deferred = False

		# Do initial load
		load(refresh=False, emit_signal=False, need_main_thread=False)

		self.thread = threading.Thread(target=StateUpdate.update_thread,
										args=(self,),
										name="StateUpdate.update_thread")

	def load(self, refresh=True, emit_signal=True, cache_refresh=True,
					log=True, need_main_thread=True):
		# Place this request on the queue and wait for it to be completed
		req = StateUpdate.UpdateRequest(refresh, emit_signal, cache_refresh,
										log, need_main_thread)
		self.queue.put(req)
		return req.done()

	def event(self):
		with self.lock:
			self.deferred = True
