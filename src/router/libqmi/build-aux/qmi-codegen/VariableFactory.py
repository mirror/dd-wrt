#!/usr/bin/env python
# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil -*-
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option) any
# later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License along
# with this program; if not, write to the Free Software Foundation, Inc., 51
# Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Copyright (C) 2012 Lanedo GmbH
#

import utils
from VariableInteger import VariableInteger
from VariableString import VariableString
from VariableStruct import VariableStruct
from VariableSequence import VariableSequence
from VariableArray import VariableArray


"""
Helps in the creation of Variable objects based on the specific 'format' found
in the given dictionary
"""
def create_variable(dictionary, new_type_name, container_type):
    if utils.format_is_integer(dictionary['format']):
        return VariableInteger(dictionary)
    elif dictionary['format'] == 'string':
        return VariableString(dictionary)
    elif dictionary['format'] == 'struct':
        return VariableStruct(dictionary, new_type_name, container_type)
    elif dictionary['format'] == 'sequence':
        return VariableSequence(dictionary, new_type_name, container_type)
    elif dictionary['format'] == 'array':
        return VariableArray(dictionary, new_type_name, container_type)
    else:
        raise RuntimeError('Unexpected field format \'%s\'' % dictionary['format'])
