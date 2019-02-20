# Copyright (C) 2015-2016 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

from .automatedproperties import AutomatedProperties
from .utils import job_obj_path_generate, mt_async_call
from . import cfg
from .cfg import JOB_INTERFACE
import dbus
import threading
# noinspection PyUnresolvedReferences
from gi.repository import GLib


# Class that handles a client waiting for something to be complete.  We either
# get a timeout or the operation is done.
class WaitingClient(object):

	# A timeout occurred
	@staticmethod
	def _timeout(wc):
		with wc.rlock:
			if wc.in_use:
				wc.in_use = False
				# Remove ourselves from waiting client
				wc.job_state.remove_waiting_client(wc)
				wc.timer_id = -1
				mt_async_call(wc.cb, wc.job_state.Complete)
				wc.job_state = None

	def __init__(self, job_state, tmo, cb, cbe):
		self.rlock = threading.RLock()
		self.job_state = job_state
		self.cb = cb
		self.cbe = cbe
		self.in_use = True		# Indicates if object is in play
		self.timer_id = -1
		if tmo > 0:
			self.timer_id = GLib.timeout_add_seconds(
				tmo, WaitingClient._timeout, self)

	# The job finished before the timer popped and we are being notified that
	# it's done
	def notify(self):
		with self.rlock:
			if self.in_use:
				self.in_use = False
				# Clear timer
				if self.timer_id != -1:
					GLib.source_remove(self.timer_id)
					self.timer_id = -1

				mt_async_call(self.cb, self.job_state.Complete)
				self.job_state = None


# noinspection PyPep8Naming
class JobState(object):
	def __init__(self, request=None):
		self.rlock = threading.RLock()

		self._percent = 0
		self._complete = False
		self._request = request
		self._ec = 0
		self._stderr = ''
		self._waiting_clients = []

		# This is an lvm command that is just taking too long and doesn't
		# support background operation
		if self._request:
			# Faking the percentage when we don't have one
			self._percent = 1

	@property
	def Percent(self):
		with self.rlock:
			return self._percent

	@Percent.setter
	def Percent(self, value):
		with self.rlock:
			self._percent = value

	@property
	def Complete(self):
		with self.rlock:
			if self._request:
				self._complete = self._request.is_done()

			return self._complete

	@Complete.setter
	def Complete(self, value):
		with self.rlock:
			self._complete = value
			self._percent = 100
			self.notify_waiting_clients()

	@property
	def GetError(self):
		with self.rlock:
			if self.Complete:
				if self._request:
					(rc, error) = self._request.get_errors()
					return (rc, str(error))
				else:
					return (self._ec, self._stderr)
			else:
				return (-1, 'Job is not complete!')

	def dtor(self):
		with self.rlock:
			self._request = None

	@property
	def Result(self):
		with self.rlock:
			if self._request:
				return self._request.result()
			return '/'

	def add_waiting_client(self, client):
		with self.rlock:
			# Avoid race condition where it goes complete before we get added
			# to the list of waiting clients
			if self.Complete:
				client.notify()
			else:
				self._waiting_clients.append(client)

	def remove_waiting_client(self, client):
		# If a waiting client timer pops before the job is done we will allow
		# the client to remove themselves from the list.  As we have a lock
		# here and a lock in the waiting client too, and they can be obtained
		# in different orders, a dead lock can occur.
		# As this remove is really optional, we will try to acquire the lock
		# and remove.  If we are unsuccessful it's not fatal, we just delay
		# the time when the objects can be garbage collected by python
		if self.rlock.acquire(False):
			try:
				self._waiting_clients.remove(client)
			finally:
				self.rlock.release()

	def notify_waiting_clients(self):
		with self.rlock:
			for c in self._waiting_clients:
				c.notify()

			self._waiting_clients = []


# noinspection PyPep8Naming
class Job(AutomatedProperties):
	_Percent_meta = ('d', JOB_INTERFACE)
	_Complete_meta = ('b', JOB_INTERFACE)
	_Result_meta = ('o', JOB_INTERFACE)
	_GetError_meta = ('(is)', JOB_INTERFACE)

	def __init__(self, request, job_state=None):
		super(Job, self).__init__(job_obj_path_generate())
		self.set_interface(JOB_INTERFACE)

		if job_state:
			self.state = job_state
		else:
			self.state = JobState(request)

	@property
	def Percent(self):
		return dbus.Double(float(self.state.Percent))

	@property
	def Complete(self):
		return dbus.Boolean(self.state.Complete)

	@staticmethod
	def _signal_complete(obj):
		obj.PropertiesChanged(
			JOB_INTERFACE, dict(Complete=dbus.Boolean(obj.state.Complete)), [])

	@Complete.setter
	def Complete(self, value):
		self.state.Complete = value
		mt_async_call(Job._signal_complete, self)

	@property
	def GetError(self):
		return dbus.Struct(self.state.GetError, signature="(is)")

	@dbus.service.method(dbus_interface=JOB_INTERFACE)
	def Remove(self):
		if self.state.Complete:
			cfg.om.remove_object(self, True)
			self.state.dtor()
		else:
			raise dbus.exceptions.DBusException(
				JOB_INTERFACE, 'Job is not complete!')

	@dbus.service.method(dbus_interface=JOB_INTERFACE,
							in_signature='i',
							out_signature='b',
							async_callbacks=('cb', 'cbe'))
	def Wait(self, timeout, cb, cbe):
		if timeout == 0 or self.state.Complete:
			cb(dbus.Boolean(self.state.Complete))
		else:
			self.state.add_waiting_client(
				WaitingClient(self.state, timeout, cb, cbe))

	@property
	def Result(self):
		return dbus.ObjectPath(self.state.Result)

	@property
	def lvm_id(self):
		return str(id(self))

	@property
	def Uuid(self):
		import uuid
		return uuid.uuid1()
