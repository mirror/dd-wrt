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

"""
Variable type for Strings ('string' format)
"""
class VariableString(Variable):

    """
    Constructor
    """
    def __init__(self, dictionary):

        # Call the parent constructor
        Variable.__init__(self, dictionary)

        self.private_format = 'gchar *'
        self.public_format = self.private_format

        if 'fixed-size' in dictionary:
            self.is_fixed_size = True
            # Fixed-size strings
            self.needs_dispose = False
            self.length_prefix_size = 0
            self.fixed_size = dictionary['fixed-size']
            self.max_size = ''
        else:
            self.is_fixed_size = False
            # Variable-length strings in heap
            self.needs_dispose = True
            # Strings which are given as the full value of a TLV will NOT have a
            # length prefix
            if 'type' in dictionary and dictionary['type'] == 'TLV':
                self.length_prefix_size = 0
            elif 'size-prefix-format' in dictionary:
                if dictionary['size-prefix-format'] == 'guint8':
                    self.length_prefix_size = 8
                elif dictionary['size-prefix-format'] == 'guint16':
                    self.length_prefix_size = 16
                else:
                    raise ValueError('Invalid size prefix format (%s): not guint8 or guint16' % dictionary['size-prefix-format'])
            else:
                # Default to UINT8
                self.length_prefix_size = 8
            self.fixed_size = ''
            self.max_size = dictionary['max-size'] if 'max-size' in dictionary else ''


    """
    Read a string from the raw byte buffer.
    """
    def emit_buffer_read(self, f, line_prefix, variable_name, buffer_name, buffer_len):
        translations = { 'lp'             : line_prefix,
                         'variable_name'  : variable_name,
                         'buffer_name'    : buffer_name,
                         'buffer_len'     : buffer_len }

        if self.is_fixed_size:
            translations['fixed_size'] = self.fixed_size
            template = (
                '${lp}/* Read the fixed-size string variable from the buffer */\n'
                '${lp}qmi_utils_read_fixed_size_string_from_buffer (\n'
                '${lp}    &${buffer_name},\n'
                '${lp}    &${buffer_len},\n'
                '${lp}    ${fixed_size},\n'
                '${lp}    &${variable_name}[0]);\n'
                '${lp}${variable_name}[${fixed_size}] = \'\\0\';\n')
        else:
            translations['length_prefix_size'] = self.length_prefix_size
            translations['max_size'] = self.max_size if self.max_size != '' else '0'
            template = (
                '${lp}/* Read the string variable from the buffer */\n'
                '${lp}qmi_utils_read_string_from_buffer (\n'
                '${lp}    &${buffer_name},\n'
                '${lp}    &${buffer_len},\n'
                '${lp}    ${length_prefix_size},\n'
                '${lp}    ${max_size},\n'
                '${lp}    &(${variable_name}));\n')

        f.write(string.Template(template).substitute(translations))


    """
    Emits the code involved in computing the size of the variable.
    """
    def emit_size_read(self, f, line_prefix, variable_name, buffer_name, buffer_len):
        translations = { 'lp'            : line_prefix,
                         'variable_name' : variable_name,
                         'buffer_name'   : buffer_name,
                         'buffer_len'    : buffer_len }

        if self.is_fixed_size:
            translations['fixed_size'] = self.fixed_size
            template = (
                '${lp}${variable_name} += ${fixed_size};\n')
        elif self.length_prefix_size == 0:
            template = (
                '${lp}${variable_name} += ${buffer_len};\n')
        elif self.length_prefix_size == 8:
            template = (
                '${lp}{\n'
                '${lp}    guint8 size8;\n'
                '${lp}    const guint8 *aux_buffer = &${buffer_name}[${variable_name}];\n'
                '${lp}    guint16 aux_buffer_len = ${buffer_len} - ${variable_name};\n'
                '\n'
                '${lp}    qmi_utils_read_guint8_from_buffer (&aux_buffer, &aux_buffer_len, &size8);\n'
                '${lp}    ${variable_name} += (1 + size8);\n'
                '${lp}}\n')
        elif self.length_prefix_size == 16:
            template = (
                '${lp}{\n'
                '${lp}    guint16 size16;\n'
                '${lp}    const guint8 *aux_buffer = &${buffer_name}[${variable_name}];\n'
                '${lp}    guint16 aux_buffer_len = ${buffer_len} - ${variable_name};\n'
                '\n'
                '${lp}    qmi_utils_read_guint16_from_buffer (&aux_buffer, &aux_buffer_len, QMI_ENDIAN_LITTLE, &size16);\n'
                '${lp}    ${variable_name} += (2 + size16);\n'
                '${lp}}\n')
        f.write(string.Template(template).substitute(translations))


    """
    Write a string to the raw byte buffer.
    """
    def emit_buffer_write(self, f, line_prefix, variable_name, buffer_name, buffer_len):
        translations = { 'lp'             : line_prefix,
                         'variable_name'  : variable_name,
                         'buffer_name'    : buffer_name,
                         'buffer_len'     : buffer_len }

        if self.is_fixed_size:
            translations['fixed_size'] = self.fixed_size
            template = (
                '${lp}/* Write the fixed-size string variable to the buffer */\n'
                '${lp}qmi_utils_write_fixed_size_string_to_buffer (\n'
                '${lp}    &${buffer_name},\n'
                '${lp}    &${buffer_len},\n'
                '${lp}    ${fixed_size},\n'
                '${lp}    ${variable_name});\n')
        else:
            translations['length_prefix_size'] = self.length_prefix_size
            template = (
                '${lp}/* Write the string variable to the buffer */\n'
                '${lp}qmi_utils_write_string_to_buffer (\n'
                '${lp}    &${buffer_name},\n'
                '${lp}    &${buffer_len},\n'
                '${lp}    ${length_prefix_size},\n'
                '${lp}    ${variable_name});\n')

        f.write(string.Template(template).substitute(translations))


    """
    Get the string as printable
    """
    def emit_get_printable(self, f, line_prefix, printable, buffer_name, buffer_len):
        translations = { 'lp'             : line_prefix,
                         'printable'      : printable,
                         'buffer_name'    : buffer_name,
                         'buffer_len'     : buffer_len }

        if self.is_fixed_size:
            translations['fixed_size'] = self.fixed_size
            translations['fixed_size_plus_one'] = int(self.fixed_size) + 1
            template = (
                '\n'
                '${lp}{\n'
                '${lp}    gchar tmp[${fixed_size_plus_one}];\n'
                '\n'
                '${lp}    /* Read the fixed-size string variable from the buffer */\n'
                '${lp}    qmi_utils_read_fixed_size_string_from_buffer (\n'
                '${lp}        &${buffer_name},\n'
                '${lp}        &${buffer_len},\n'
                '${lp}        ${fixed_size},\n'
                '${lp}        &tmp[0]);\n'
                '${lp}    tmp[${fixed_size}] = \'\\0\';\n'
                '\n'
                '${lp}    g_string_append_printf (${printable}, "%s", tmp);\n'
                '${lp}}\n')
        else:
            translations['length_prefix_size'] = self.length_prefix_size
            translations['max_size'] = self.max_size if self.max_size != '' else '0'
            template = (
                '\n'
                '${lp}{\n'
                '${lp}    gchar *tmp;\n'
                '\n'
                '${lp}    /* Read the string variable from the buffer */\n'
                '${lp}    qmi_utils_read_string_from_buffer (\n'
                '${lp}        &${buffer_name},\n'
                '${lp}        &${buffer_len},\n'
                '${lp}        ${length_prefix_size},\n'
                '${lp}        ${max_size},\n'
                '${lp}        &tmp);\n'
                '\n'
                '${lp}    g_string_append_printf (${printable}, "%s", tmp);\n'
                '${lp}    g_free (tmp);\n'
                '${lp}}\n')

        f.write(string.Template(template).substitute(translations))


    """
    Variable declaration
    """
    def build_variable_declaration(self, line_prefix, variable_name):
        translations = { 'lp'   : line_prefix,
                         'name' : variable_name }

        if self.is_fixed_size:
            translations['fixed_size_plus_one'] = int(self.fixed_size) + 1
            template = (
                '${lp}gchar ${name}[${fixed_size_plus_one}];\n')
        else:
            template = (
                '${lp}gchar *${name};\n')
        return string.Template(template).substitute(translations)


    """
    Getter for the string type
    """
    def build_getter_declaration(self, line_prefix, variable_name):
        translations = { 'lp'   : line_prefix,
                         'name' : variable_name }

        template = (
            '${lp}const gchar **${name},\n')
        return string.Template(template).substitute(translations)


    """
    Documentation for the getter
    """
    def build_getter_documentation(self, line_prefix, variable_name):
        translations = { 'lp'   : line_prefix,
                         'name' : variable_name }

        template = (
            '${lp}@${name}: a placeholder for the output constant string, or %NULL if not required.\n')
        return string.Template(template).substitute(translations)


    """
    Builds the String getter implementation
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
    Setter for the string type
    """
    def build_setter_declaration(self, line_prefix, variable_name):
        translations = { 'lp'   : line_prefix,
                         'name' : variable_name }

        template = (
            '${lp}const gchar *${name},\n')
        return string.Template(template).substitute(translations)


    """
    Documentation for the setter
    """
    def build_setter_documentation(self, line_prefix, variable_name):
        translations = { 'lp'   : line_prefix,
                         'name' : variable_name }

        if self.is_fixed_size:
            translations['fixed_size'] = self.fixed_size
            template = (
                '${lp}@${name}: a constant string of exactly ${fixed_size} characters.\n')
        elif self.max_size != '':
            translations['max_size'] = self.max_size
            template = (
                '${lp}@${name}: a constant string with a maximum length of ${max_size} characters.\n')
        else:
            template = (
                '${lp}@${name}: a constant string.\n')
        return string.Template(template).substitute(translations)


    """
    Builds the String setter implementation
    """
    def build_setter_implementation(self, line_prefix, variable_name_from, variable_name_to):
        translations = { 'lp'   : line_prefix,
                         'from' : variable_name_from,
                         'to'   : variable_name_to }

        if self.is_fixed_size:
            translations['fixed_size'] = self.fixed_size
            template = (
                '${lp}if (!${from} || strlen (${from}) != ${fixed_size}) {\n'
                '${lp}    g_set_error (error,\n'
                '${lp}                 QMI_CORE_ERROR,\n'
                '${lp}                 QMI_CORE_ERROR_INVALID_ARGS,\n'
                '${lp}                 "Input variable \'${from}\' must be ${fixed_size} characters long");\n'
                '${lp}    return FALSE;\n'
                '${lp}}\n'
                '${lp}memcpy (${to}, ${from}, ${fixed_size});\n'
                '${lp}${to}[${fixed_size}] = \'\\0\';\n')
        else:
            template = ''
            if self.max_size != '':
                translations['max_size'] = self.max_size
                template += (
                    '${lp}if (${from} && strlen (${from}) > ${max_size}) {\n'
                    '${lp}    g_set_error (error,\n'
                    '${lp}                 QMI_CORE_ERROR,\n'
                    '${lp}                 QMI_CORE_ERROR_INVALID_ARGS,\n'
                    '${lp}                 "Input variable \'${from}\' must be less than ${max_size} characters long");\n'
                    '${lp}    return FALSE;\n'
                    '${lp}}\n')
            template += (
                '${lp}g_free (${to});\n'
                '${lp}${to} = g_strdup (${from} ? ${from} : "");\n')

        return string.Template(template).substitute(translations)


    """
    Documentation for the struct field
    """
    def build_struct_field_documentation(self, line_prefix, variable_name):
        translations = { 'lp'   : line_prefix,
                         'name' : variable_name }

        if self.is_fixed_size:
            translations['fixed_size'] = self.fixed_size
            template = (
                '${lp}@${name}: a string of exactly ${fixed_size} characters.\n')
        elif self.max_size != '':
            translations['max_size'] = self.max_size
            template = (
                '${lp}@${name}: a string with a maximum length of ${max_size} characters.\n')
        else:
            template = (
                '${lp}@${name}: a string.\n')
        return string.Template(template).substitute(translations)


    """
    Dispose the string
    """
    def build_dispose(self, line_prefix, variable_name):
        # Fixed-size strings don't need dispose
        if self.is_fixed_size:
            return ''

        translations = { 'lp'            : line_prefix,
                         'variable_name' : variable_name }

        template = (
            '${lp}g_free (${variable_name});\n')
        return string.Template(template).substitute(translations)
