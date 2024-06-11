#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

class LazyModule(object):
	def __init__(self, name):
		self._name = name

	def __getattribute__(self, name):
		if name == '_name':
			return object.__getattribute__(self, name)
		else:
			mod = __import__(self._name)
			globals()[name] = mod
			return mod.__getattribute__(name)

import utils
import exceptions

from base import Object, CoreObject, ConstantGroup, Serializable

import modelling
import code
import objects
import components
import handlers
import policies
import api
import interfaces
import core
import constants

# Lock all ConstantGroup subclasses.
ConstantGroup.lock()