##############################################################################
#
# Copyright (c) 2002 Zope Corporation and Contributors. All Rights Reserved.
#
# This software is subject to the provisions of the Zope Public License,
# Version 2.1 (ZPL).  A copy of the ZPL should accompany this distribution.
# THIS SOFTWARE IS PROVIDED "AS IS" AND ANY AND ALL EXPRESS OR IMPLIED
# WARRANTIES ARE DISCLAIMED, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF TITLE, MERCHANTABILITY, AGAINST INFRINGEMENT, AND FITNESS
# FOR A PARTICULAR PURPOSE
#
##############################################################################

__version__='$Revision: 1.7 $'[11:-2]

import math
import random
import string
import warnings

_old_filters = warnings.filters[:]
warnings.filterwarnings('ignore', category=DeprecationWarning)
try:
    try:
        import sets
    except ImportError:
        sets = None
finally:
    warnings.filters[:] = _old_filters

utility_builtins = {}

utility_builtins['string'] = string
utility_builtins['math'] = math
utility_builtins['random'] = random
utility_builtins['whrandom'] = random
utility_builtins['sets'] = sets

try:
    import DateTime
    utility_builtins['DateTime']= DateTime.DateTime
except ImportError:
    pass

try:
    import DocumentTemplate.sequence
    utility_builtins['sequence']= DocumentTemplate.sequence
except ImportError:
    pass


def same_type(arg1, *args):
    '''Compares the class or type of two or more objects.'''
    t = getattr(arg1, '__class__', type(arg1))
    for arg in args:
        if getattr(arg, '__class__', type(arg)) is not t:
            return 0
    return 1
utility_builtins['same_type'] = same_type

def test(*args):
    length = len(args)
    for i in range(1, length, 2):
        if args[i-1]:
            return args[i]

    if length % 2:
        return args[-1]
utility_builtins['test'] = test

def reorder(s, with_=None, without=()):
    # s, with_, and without are sequences treated as sets.
    # The result is subtract(intersect(s, with_), without),
    # unless with_ is None, in which case it is subtract(s, without).
    if with_ is None:
        with_ = s
    orig = {}
    for item in s:
        if isinstance(item, tuple) and len(item) == 2:
            key, value = item
        else:
            key = value = item
        orig[key] = value

    result = []

    for item in without:
        if isinstance(item, tuple) and len(item) == 2:
            key, ignored = item
        else:
            key = item
        if key in orig:
            del orig[key]

    for item in with_:
        if isinstance(item, tuple) and len(item) == 2:
            key, ignored = item
        else:
            key = item
        if key in orig:
            result.append((key, orig[key]))
            del orig[key]

    return result
utility_builtins['reorder'] = reorder
