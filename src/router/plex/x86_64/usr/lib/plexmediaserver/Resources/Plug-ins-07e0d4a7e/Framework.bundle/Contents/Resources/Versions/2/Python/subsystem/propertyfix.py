#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

# Original code by Yu-Jie Lin
# http://makeyjl.blogspot.com/2009/02/propery-setter-and-deleter-in-python-25.html

import sys
import __builtin__

# For Python 2.5-, this will enable the simliar property mechanism as in
# Python 2.6+/3.0+. The code is based on
# http://bruynooghe.blogspot.com/2008/04/xsetter-syntax-in-python-25.html
if sys.version_info[:2] <= (2, 5):
  class property(property):

      def __init__(self, fget=None, fset=None, fdel=None, doc=None, *args, **kwargs):
          self.__doc__ = doc if doc else fget.__doc__
          super(property, self).__init__(fget=fget, fset=fset, fdel=fdel, doc=doc, *args, **kwargs)

      def setter(self, fset):
          cls_ns = sys._getframe(1).f_locals
          for k, v in cls_ns.iteritems():
              if v == self:
                  propname = k
                  break
          cls_ns[propname] = property(self.fget, fset,
                                      self.fdel, self.__doc__)
          return cls_ns[propname]

      def deleter(self, fdel):
          cls_ns = sys._getframe(1).f_locals
          for k, v in cls_ns.iteritems():
              if v == self:
                  propname = k
                  break
          cls_ns[propname] = property(self.fget, self.fset,
                                      fdel, self.__doc__)
          return cls_ns[propname]

  __builtin__.property = property