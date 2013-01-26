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
from Variable import Variable
import VariableFactory

"""
Variable type for Structs ('struct' format)
"""
class VariableStruct(Variable):

    """
    Constructor
    """
    def __init__(self, dictionary, struct_type_name, container_type):

        # Call the parent constructor
        Variable.__init__(self, dictionary)

        # The public format of the struct is built directly from the suggested
        # struct type name
        self.public_format = utils.build_camelcase_name(struct_type_name)
        self.private_format = self.public_format
        self.container_type = container_type

        # Load members of this struct
        self.members = []
        for member_dictionary in dictionary['contents']:
            member = {}
            member['name'] = utils.build_underscore_name(member_dictionary['name'])
            member['object'] = VariableFactory.create_variable(member_dictionary, struct_type_name + ' ' + member['name'], self.container_type)
            self.members.append(member)

        # We'll need to dispose if at least one of the members needs it
        for member in self.members:
            if member['object'].needs_dispose == True:
                self.needs_dispose = True


    """
    Emit all types for the members of the struct plus the new struct type itself
    """
    def emit_types(self, f):
        # Emit types for each member
        for member in self.members:
            member['object'].emit_types(f)

        translations = { 'format' : self.public_format }
        template = (
            '\n'
            '/**\n'
            ' * ${format}:\n')
        f.write(string.Template(template).substitute(translations))
        for member in self.members:
            f.write(member['object'].build_struct_field_documentation(' * ', member['name']))

        template = (
            ' *\n'
            ' * A ${format} struct.\n'
            ' */\n'
            'typedef struct _${format} {\n')
        f.write(string.Template(template).substitute(translations))

        for member in self.members:
            translations['variable_format'] = member['object'].public_format
            translations['variable_name'] = member['name']
            template = (
                '    ${variable_format} ${variable_name};\n')
            f.write(string.Template(template).substitute(translations))

        template = ('} ${format};\n')
        f.write(string.Template(template).substitute(translations))


    """
    Emit helper methods for all types in the struct
    """
    def emit_helper_methods(self, hfile, cfile):
        # Emit for each member
        for member in self.members:
            member['object'].emit_helper_methods(hfile, cfile)


    """
    Reading the contents of a struct is just about reading each of the struct
    fields one by one.
    """
    def emit_buffer_read(self, f, line_prefix, variable_name, buffer_name, buffer_len):
        for member in self.members:
            member['object'].emit_buffer_read(f, line_prefix, variable_name + '.' +  member['name'], buffer_name, buffer_len)


    """
    Emits the code involved in computing the size of the variable.
    """
    def emit_size_read(self, f, line_prefix, variable_name, buffer_name, buffer_len):
        for member in self.members:
            member['object'].emit_size_read(f, line_prefix, variable_name, buffer_name, buffer_len)


    """
    Writing the contents of a struct is just about writing each of the struct
    fields one by one.
    """
    def emit_buffer_write(self, f, line_prefix, variable_name, buffer_name, buffer_len):
        for member in self.members:
            member['object'].emit_buffer_write(f, line_prefix, variable_name + '.' +  member['name'], buffer_name, buffer_len)


    """
    The struct will be printed as a list of fields enclosed between square
    brackets
    """
    def emit_get_printable(self, f, line_prefix, printable, buffer_name, buffer_len):
        translations = { 'lp'        : line_prefix,
                         'printable' : printable }

        template = (
            '${lp}g_string_append (${printable}, "[");\n')
        f.write(string.Template(template).substitute(translations))

        for member in self.members:
            translations['variable_name'] = member['name']
            template = (
                '${lp}g_string_append (${printable}, " ${variable_name} = \'");\n')
            f.write(string.Template(template).substitute(translations))

            member['object'].emit_get_printable(f, line_prefix, printable, buffer_name, buffer_len)

            template = (
                '${lp}g_string_append (${printable}, "\'");\n')
            f.write(string.Template(template).substitute(translations))

        template = (
            '${lp}g_string_append (${printable}, " ]");\n')
        f.write(string.Template(template).substitute(translations))


    """
    Variable declaration
    """
    def build_variable_declaration(self, line_prefix, variable_name):
        translations = { 'lp'     : line_prefix,
                         'format' : self.public_format,
                         'name'   : variable_name }

        template = (
            '${lp}${format} ${name};\n')
        return string.Template(template).substitute(translations)


    """
    The getter for a struct variable will include independent getters for each
    of the variables in the struct.
    """
    def build_getter_declaration(self, line_prefix, variable_name):
        translations = { 'lp'     : line_prefix,
                         'format' : self.public_format,
                         'name'   : variable_name }

        template = (
            '${lp}${format} *${name},\n')
        return string.Template(template).substitute(translations)


    """
    Documentation for the getter
    """
    def build_getter_documentation(self, line_prefix, variable_name):
        translations = { 'lp'     : line_prefix,
                         'format' : self.public_format,
                         'name'   : variable_name }

        template = (
            '${lp}@${name}: a placeholder for the output constant #${format}, or %NULL if not required.\n')
        return string.Template(template).substitute(translations)


    """
    Builds the Struct getter implementation
    """
    def build_getter_implementation(self, line_prefix, variable_name_from, variable_name_to, to_is_reference):
        translations = { 'lp'   : line_prefix,
                         'from' : variable_name_from,
                         'to'   : variable_name_to }

        if to_is_reference:
            template = (
                '${lp}if (${to})\n'
                '${lp}    *${to} = ${from};\n')
            return string.Template(template).substitute(translations)
        else:
            template = (
                '${lp}${to} = ${from};\n')
            return string.Template(template).substitute(translations)


    """
    The setter for a struct variable will include independent setters for each
    of the variables in the struct.
    """
    def build_setter_declaration(self, line_prefix, variable_name):
        translations = { 'lp'     : line_prefix,
                         'format' : self.public_format,
                         'name'   : variable_name }

        template = (
            '${lp}const ${format} *${name},\n')
        return string.Template(template).substitute(translations)


    """
    Documentation for the setter
    """
    def build_setter_documentation(self, line_prefix, variable_name):
        translations = { 'lp'     : line_prefix,
                         'format' : self.public_format,
                         'name'   : variable_name }

        template = (
            '${lp}@${name}: the address of the #${format} to set.\n')
        return string.Template(template).substitute(translations)

    """
    Builds the Struct setter implementation
    """
    def build_setter_implementation(self, line_prefix, variable_name_from, variable_name_to):
        built = ''
        for member in self.members:
            built += member['object'].build_setter_implementation(line_prefix,
                                                                  variable_name_from + '->' + member['name'],
                                                                  variable_name_to + '.' + member['name'])
        return built


    """
    Documentation for the struct field
    """
    def build_struct_field_documentation(self, line_prefix, variable_name):
        translations = { 'lp'     : line_prefix,
                         'format' : self.public_format,
                         'name'   : variable_name }

        template = (
            '${lp}@${name}: a #${format} struct.\n')
        return string.Template(template).substitute(translations)


    """
    Disposing a struct is just about disposing each of the struct fields one by
    one.
    """
    def build_dispose(self, line_prefix, variable_name):
        built = ''
        for member in self.members:
            built += member['object'].build_dispose(line_prefix, variable_name + '.' + member['name'])
        return built


    """
    Add sections
    """
    def add_sections(self, sections):
        # Add sections for each member
        for member in self.members:
            member['object'].add_sections(sections)

        sections['public-types'] += self.public_format + '\n'
