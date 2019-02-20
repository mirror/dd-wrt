# Copyright (C) 2015-2016 Red Hat, Inc. All rights reserved.
#
# This copyrighted material is made available to anyone wishing to use,
# modify, copy, or redistribute it subject to the terms and conditions
# of the GNU General Public License v.2.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.

from abc import ABCMeta, abstractmethod


class State(object, metaclass=ABCMeta):
	@abstractmethod
	def lvm_id(self):
		pass

	@abstractmethod
	def identifiers(self):
		pass

	@abstractmethod
	def create_dbus_object(self, path):
		pass

	def __str__(self):
		return '*****\n' + str(self.__dict__) + '\n******\n'
