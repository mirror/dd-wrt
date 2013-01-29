#!/usr/bin/env python
# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
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

import string
import utils

"""
Base class for every variable type defined in the database
"""
class Variable:

    """
    Constructor with common variable handling
    """
    def __init__(self, dictionary):
        """
        Variables can define specific public and private formats to be used.
        The public format will be that used in the generated interface file,
        while the private one will only be used internally.
        """
        self.format = dictionary['format']
        self.public_format = None
        self.private_format = None

        """
        Variables that get allocated in heap need to get properly disposed.
        """
        self.needs_dispose = False

        self.endian = "QMI_ENDIAN_LITTLE"
        if 'endian' in dictionary:
            endian = dictionary['endian']
            if endian == 'network' or endian == 'big':
                self.endian = "QMI_ENDIAN_BIG"
            elif endian == 'little':
                pass
            else:
                raise ValueError("Invalid endian value %s" % endian)

    """
    Emits the code to declare specific new types required by the variable.
    """
    def emit_types(self, f):
        pass


    """
    Emits the code to custom helper methods needed by this variable.
    They are emitted as early as possible.
    """
    def emit_helper_methods(self, hfile, cfile):
        pass


    """
    Emits the code involved in reading the variable from the raw byte stream
    into the specific private format.
    """
    def emit_buffer_read(self, f, line_prefix, variable_name, buffer_name, buffer_len):
        pass


    """
    Emits the code involved in writing the variable to the raw byte stream
    from the specific private format.
    """
    def emit_buffer_write(self, f, line_prefix, variable_name, buffer_name, buffer_len):
        pass


    """
    Emits the code involved in computing the size of the variable.
    """
    def emit_size_read(self, f, line_prefix, variable_name, buffer_name, buffer_len):
        pass


    """
    Emits the code to get the contents of the given variable as a printable string.
    """
    def emit_get_printable(self, f, line_prefix, printable, buffer_name, buffer_len):
        pass

    """
    Builds the code to include the declaration of a variable of this kind.
    """
    def build_variable_declaration(self, line_prefix, variable_name):
        return ''

    """
    Builds the code to include in the getter method declaration for this kind of variable.
    """
    def build_getter_declaration(self, line_prefix, variable_name):
        return ''

    """
    Builds the documentation of the getter code
    """
    def build_getter_documentation(self, line_prefix, variable_name):
        return ''

    """
    Builds the code to implement getting this kind of variable.
    """
    def build_getter_implementation(self, line_prefix, variable_name_from, variable_name_to, to_is_reference):
        return ''

    """
    Builds the code to include in the setter method for this kind of variable.
    """
    def build_setter_declaration(self, line_prefix, variable_name):
        return ''

    """
    Builds the documentation of the setter code
    """
    def build_setter_documentation(self, line_prefix, variable_name):
        return ''

    """
    Builds the code to implement setting this kind of variable.
    """
    def build_setter_implementation(self, line_prefix, variable_name_from, variable_name_to):
        return ''

    """
    Documentation for the struct field
    """
    def build_struct_field_documentation(self, line_prefix, variable_name):
        return ''

    """
    Emits the code to dispose the variable.
    """
    def build_dispose(self, line_prefix, variable_name):
        return ''

    """
    Add sections
    """
    def add_sections(self, sections):
        pass
