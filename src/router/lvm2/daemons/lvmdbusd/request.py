# Copyright (C) 2015-2016 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

import threading
# noinspection PyUnresolvedReferences
from gi.repository import GLib
from .job import Job
from . import cfg
import traceback
from .utils import log_error, mt_async_call


class RequestEntry(object):
	def __init__(self, tmo, method, arguments, cb, cb_error,
			return_tuple=True, job_state=None):
		self.method = method
		self.arguments = arguments
		self.cb = cb
		self.cb_error = cb_error

		self.timer_id = -1
		self.lock = threading.RLock()
		self.done = False
		self._result = None
		self._job = None
		self._rc = 0
		self._rc_error = None
		self._return_tuple = return_tuple
		self._job_state = job_state

		if tmo < 0:
			# Client is willing to block forever
			pass
		elif tmo == 0:
			self._return_job()
		else:
			# Note: using 990 instead of 1000 for second to ms conversion to
			# account for overhead.  Goal is to return just before the
			# timeout amount has expired.  Better to be a little early than
			# late.
			self.timer_id = GLib.timeout_add(
				tmo * 990, RequestEntry._request_timeout, self)

	@staticmethod
	def _request_timeout(r):
		"""
		Method which gets called when the timer runs out!
		:param r:  RequestEntry which timed out
		:return: Result of timer_expired
		"""
		return r.timer_expired()

	def _return_job(self):
		# Return job is only called when we create a request object or when
		# we pop a timer.  In both cases we are running in the correct context
		# and do not need to schedule the call back in main context.
		self._job = Job(self, self._job_state)
		cfg.om.register_object(self._job, True)
		if self._return_tuple:
			self.cb(('/', self._job.dbus_object_path()))
		else:
			self.cb(self._job.dbus_object_path())

	def run_cmd(self):
		try:
			result = self.method(*self.arguments)
			self.register_result(result)
		except Exception as e:
			# Use the request entry to return the result as the client may
			# have gotten a job by the time we hit an error
			# Lets get the stacktrace and set that to the error message
			st = traceback.format_exc()
			cfg.blackbox.dump()
			log_error("Exception returned to client: \n%s" % st)
			self.register_error(-1, str(e), e)

	def is_done(self):
		with self.lock:
			rc = self.done
		return rc

	def get_errors(self):
		with self.lock:
			return (self._rc, self._rc_error)

	def result(self):
		with self.lock:
			if self.done:
				return self._result
			return '/'

	def _reg_ending(self, result, error_rc=0, error_msg=None,
					error_exception=None):
		with self.lock:
			self.done = True
			if self.timer_id != -1:
				# Try to prevent the timer from firing
				GLib.source_remove(self.timer_id)

			self._result = result
			self._rc = error_rc
			self._rc_error = error_msg

			if not self._job:
				# We finished and there is no job, so return result or error
				# now!
				# Note: If we don't have a valid cb or cbe, this indicates a
				# request that doesn't need a response as we already returned
				# one before the request was processed.
				if error_rc == 0:
					if self.cb:
						if self._return_tuple:
							mt_async_call(self.cb, (result, '/'))
						else:
							mt_async_call(self.cb, result)
				else:
					if self.cb_error:
						if not error_exception:
							if not error_msg:
								error_exception = Exception(
									"An error occurred, but no reason was "
									"given, see service logs!")
							else:
								error_exception = Exception(error_msg)

						mt_async_call(self.cb_error, error_exception)
			else:
				# We have a job and it's complete, indicate that it's done.
				self._job.Complete = True
				self._job = None

	def register_error(self, error_rc, error_message, error_exception):
		self._reg_ending('/', error_rc, error_message, error_exception)

	def register_result(self, result):
		self._reg_ending(result)

	def timer_expired(self):
		with self.lock:
			# Set the timer back to -1 as we will get a warning if we try
			# to remove a timer that doesn't exist
			self.timer_id = -1
			if not self.done:
				# Create dbus job object and return path to caller
				self._return_job()
			else:
				# The job is done, we have nothing to do
				pass

		return False
