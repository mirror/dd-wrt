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
Variable type for signed/unsigned Integers
('guint8', 'gint8', 'guint16', 'gint16', 'guint32', 'gint32', 'guint64', 'gint64' 'guint-sized' formats)
"""
class VariableInteger(Variable):

    """
    Constructor
    """
    def __init__(self, dictionary):

        # Call the parent constructor
        Variable.__init__(self, dictionary)

        self.guint_sized_size = ''
        if self.format == "guint-sized":
            if 'guint-size' not in dictionary:
                raise RuntimeError('Format \'guint-sized\' requires \'guint-size\' parameter')
            else:
                self.guint_sized_size = dictionary['guint-size']
            self.private_format = 'guint64'
            self.public_format  = 'guint64'
        else:
            self.private_format = self.format
            self.public_format = dictionary['public-format'] if 'public-format' in dictionary else self.private_format

    """
    Read a single integer from the raw byte buffer
    """
    def emit_buffer_read(self, f, line_prefix, variable_name, buffer_name, buffer_len):
        translations = { 'lp'             : line_prefix,
                         'public_format'  : self.public_format,
                         'private_format' : self.private_format,
                         'len'            : self.guint_sized_size,
                         'variable_name'  : variable_name,
                         'buffer_name'    : buffer_name,
                         'buffer_len'     : buffer_len,
                         'endian'         : self.endian }

        if self.format == 'guint-sized':
            template = (
                '${lp}/* Read the ${len}-byte long variable from the buffer */\n'
                '${lp}qmi_utils_read_sized_guint_from_buffer (\n'
                '${lp}    &${buffer_name},\n'
                '${lp}    &${buffer_len},\n'
                '${lp}    ${len},\n'
                '${lp}    ${endian},\n'
                '${lp}    &(${variable_name}));\n')
        elif self.private_format == self.public_format:
            template = (
                '${lp}/* Read the ${private_format} variable from the buffer */\n'
                '${lp}qmi_utils_read_${private_format}_from_buffer (\n'
                '${lp}    &${buffer_name},\n'
                '${lp}    &${buffer_len},\n')
            if self.private_format != 'guint8' and self.private_format != 'gint8':
                template += (
                    '${lp}    ${endian},\n')
            template += (
                '${lp}    &(${variable_name}));\n')
        else:
            template = (
                '${lp}{\n'
                '${lp}    ${private_format} tmp;\n'
                '\n'
                '${lp}    /* Read the ${private_format} variable from the buffer */\n'
                '${lp}    qmi_utils_read_${private_format}_from_buffer (\n'
                '${lp}        &${buffer_name},\n'
                '${lp}        &${buffer_len},\n')
            if self.private_format != 'guint8' and self.private_format != 'gint8':
                template += (
                    '${lp}        ${endian},\n')
            template += (
                '${lp}        &tmp);\n'
                '${lp}    ${variable_name} = (${public_format})tmp;\n'
                '${lp}}\n')
        f.write(string.Template(template).substitute(translations))


    """
    Emits the code involved in computing the size of the variable.
    """
    def emit_size_read(self, f, line_prefix, variable_name, buffer_name, buffer_len):
        translations = { 'lp'            : line_prefix,
                         'len'           : self.guint_sized_size,
                         'variable_name' : variable_name }
        template = ''
        if self.format == 'guint-sized':
            template += (
                '${lp}${variable_name} += ${len};\n')
        elif self.private_format == 'guint8' or self.private_format == 'gint8':
            template += (
                '${lp}${variable_name} += 1;\n')
        elif self.private_format == 'guint16' or self.private_format == 'gint16':
            template += (
                '${lp}${variable_name} += 2;\n')
        elif self.private_format == 'guint32' or self.private_format == 'gint32':
            template += (
                '${lp}${variable_name} += 4;\n')
        elif self.private_format == 'guint64' or self.private_format == 'gint64':
            template += (
                '${lp}${variable_name} += 8;\n')
        f.write(string.Template(template).substitute(translations))


    """
    Write a single integer to the raw byte buffer
    """
    def emit_buffer_write(self, f, line_prefix, variable_name, buffer_name, buffer_len):
        translations = { 'lp'             : line_prefix,
                         'private_format' : self.private_format,
                         'len'            : self.guint_sized_size,
                         'variable_name'  : variable_name,
                         'buffer_name'    : buffer_name,
                         'buffer_len'     : buffer_len,
                         'endian'         : self.endian }

        if self.format == 'guint-sized':
            template = (
                '${lp}/* Write the ${len}-byte long variable to the buffer */\n'
                '${lp}qmi_utils_write_sized_guint_to_buffer (\n'
                '${lp}    &${buffer_name},\n'
                '${lp}    &${buffer_len},\n'
                '${lp}    ${len},\n'
                '${lp}    ${endian},\n'
                '${lp}    &(${variable_name}));\n')
        elif self.private_format == self.public_format:
            template = (
                '${lp}/* Write the ${private_format} variable to the buffer */\n'
                '${lp}qmi_utils_write_${private_format}_to_buffer (\n'
                '${lp}    &${buffer_name},\n'
                '${lp}    &${buffer_len},\n')
            if self.private_format != 'guint8' and self.private_format != 'gint8':
                template += (
                    '${lp}    ${endian},\n')
            template += (
                '${lp}    &(${variable_name}));\n')
        else:
            template = (
                '${lp}{\n'
                '${lp}    ${private_format} tmp;\n'
                '\n'
                '${lp}    tmp = (${private_format})${variable_name};\n'
                '${lp}    /* Write the ${private_format} variable to the buffer */\n'
                '${lp}    qmi_utils_write_${private_format}_to_buffer (\n'
                '${lp}        &${buffer_name},\n'
                '${lp}        &${buffer_len},\n')
            if self.private_format != 'guint8' and self.private_format != 'gint8':
                template += (
                    '${lp}        ${endian},\n')
            template += (
                '${lp}        &tmp);\n'
                '${lp}}\n')
        f.write(string.Template(template).substitute(translations))


    """
    Get the integer as a printable string.
    """
    def emit_get_printable(self, f, line_prefix, printable, buffer_name, buffer_len):
        common_format = ''
        common_cast = ''

        if self.private_format == 'guint8':
            common_format = '%u'
            common_cast = '(guint)'
        elif self.private_format == 'guint16':
            common_format = '%" G_GUINT16_FORMAT "'
        elif self.private_format == 'guint32':
            common_format = '%" G_GUINT32_FORMAT "'
        elif self.private_format == 'guint64':
            common_format = '%" G_GUINT64_FORMAT "'
        elif self.private_format == 'gint8':
            common_format = '%d'
            common_cast = '(gint)'
        elif self.private_format == 'gint16':
            common_format = '%" G_GINT16_FORMAT "'
        elif self.private_format == 'gint32':
            common_format = '%" G_GINT32_FORMAT "'
        elif self.private_format == 'gint64':
            common_format = '%" G_GINT64_FORMAT "'

        translations = { 'lp'             : line_prefix,
                         'private_format' : self.private_format,
                         'public_format'  : self.public_format,
                         'len'            : self.guint_sized_size,
                         'printable'      : printable,
                         'buffer_name'    : buffer_name,
                         'buffer_len'     : buffer_len,
                         'common_format'  : common_format,
                         'common_cast'    : common_cast,
                         'endian'         : self.endian }
        template = (
            '\n'
            '${lp}{\n'
            '${lp}    ${private_format} tmp;\n'
            '\n')

        if self.format == 'guint-sized':
            template += (
                '${lp}    /* Read the ${len}-byte long variable from the buffer */\n'
                '${lp}    qmi_utils_read_sized_guint_from_buffer (\n'
                '${lp}        &${buffer_name},\n'
                '${lp}        &${buffer_len},\n'
                '${lp}        ${len},\n'
                '${lp}        ${endian},\n'
                '${lp}        &tmp);\n'
                '\n')
        else:
            template += (
                '${lp}    /* Read the ${private_format} variable from the buffer */\n'
                '${lp}    qmi_utils_read_${private_format}_from_buffer (\n'
                '${lp}        &${buffer_name},\n'
                '${lp}        &${buffer_len},\n')
            if self.private_format != 'guint8' and self.private_format != 'gint8':
                template += (
                    '${lp}        ${endian},\n')
            template += (
                '${lp}        &tmp);\n'
                '\n')

        if self.public_format == 'gboolean':
            template += (
                '${lp}    g_string_append_printf (${printable}, "%s", tmp ? "yes" : "no");\n')
        elif self.public_format != self.private_format:
            translations['public_type_underscore'] = utils.build_underscore_name_from_camelcase(self.public_format)
            translations['public_type_underscore_upper'] = string.upper(utils.build_underscore_name_from_camelcase(self.public_format))
            template += (
                '#if defined  __${public_type_underscore_upper}_IS_ENUM__\n'
                '${lp}    g_string_append_printf (${printable}, "%s", ${public_type_underscore}_get_string ((${public_format})tmp));\n'
                '#elif defined  __${public_type_underscore_upper}_IS_FLAGS__\n'
                '${lp}    {\n'
                '${lp}        gchar *flags_str;\n'
                '\n'
                '${lp}        flags_str = ${public_type_underscore}_build_string_from_mask ((${public_format})tmp);\n'
                '${lp}        g_string_append_printf (${printable}, "%s", flags_str);\n'
                '${lp}        g_free (flags_str);\n'
                '${lp}    }\n'
                '#else\n'
                '# error unexpected public format: ${public_format}\n'
                '#endif\n')
        else:
            template += (
                '${lp}    g_string_append_printf (${printable}, "${common_format}", ${common_cast}tmp);\n')

        template += (
            '${lp}}\n')

        f.write(string.Template(template).substitute(translations))


    """
    Variable declaration
    """
    def build_variable_declaration(self, line_prefix, variable_name):
        translations = { 'lp'             : line_prefix,
                         'private_format' : self.private_format,
                         'name'           : variable_name }

        template = (
            '${lp}${private_format} ${name};\n')
        return string.Template(template).substitute(translations)


    """
    Getter for the integer type
    """
    def build_getter_declaration(self, line_prefix, variable_name):
        translations = { 'lp'            : line_prefix,
                         'public_format' : self.public_format,
                         'name'          : variable_name }

        template = (
            '${lp}${public_format} *${name},\n')
        return string.Template(template).substitute(translations)


    """
    Documentation for the getter
    """
    def build_getter_documentation(self, line_prefix, variable_name):
        translations = { 'lp'            : line_prefix,
                         'public_format' : self.public_format,
                         'name'          : variable_name }

        template = (
            '${lp}@${name}: a placeholder for the output #${public_format}, or %NULL if not required.\n')
        return string.Template(template).substitute(translations)

    """
    Builds the Integer getter implementation
    """
    def build_getter_implementation(self, line_prefix, variable_name_from, variable_name_to, to_is_reference):
        needs_cast = True if self.public_format != self.private_format else False
        translations = { 'lp'       : line_prefix,
                         'from'     : variable_name_from,
                         'to'       : variable_name_to,
                         'cast_ini' : '(' + self.public_format + ')(' if needs_cast else '',
                         'cast_end' : ')' if needs_cast else '' }

        if to_is_reference:
            template = (
                '${lp}if (${to})\n'
                '${lp}    *${to} = ${cast_ini}${from}${cast_end};\n')
            return string.Template(template).substitute(translations)
        else:
            template = (
                '${lp}${to} = ${cast_ini}${from}${cast_end};\n')
            return string.Template(template).substitute(translations)


    """
    Setter for the integer type
    """
    def build_setter_declaration(self, line_prefix, variable_name):
        translations = { 'lp'            : line_prefix,
                         'public_format' : self.public_format,
                         'name'          : variable_name }

        template = (
            '${lp}${public_format} ${name},\n')
        return string.Template(template).substitute(translations)


    """
    Documentation for the setter
    """
    def build_setter_documentation(self, line_prefix, variable_name):
        translations = { 'lp'            : line_prefix,
                         'public_format' : self.public_format,
                         'name'          : variable_name }

        template = (
            '${lp}@${name}: a #${public_format}.\n')
        return string.Template(template).substitute(translations)


    """
    Implementation of the setter
    """
    def build_setter_implementation(self, line_prefix, variable_name_from, variable_name_to):
        needs_cast = True if self.public_format != self.private_format else False
        translations = { 'lp'       : line_prefix,
                         'from'     : variable_name_from,
                         'to'       : variable_name_to,
                         'cast_ini' : '(' + self.private_format + ')(' if needs_cast else '',
                         'cast_end' : ')' if needs_cast else '' }

        template = (
            '${lp}${to} = ${cast_ini}${from}${cast_end};\n')
        return string.Template(template).substitute(translations)


    """
    Documentation for the struct field
    """
    def build_struct_field_documentation(self, line_prefix, variable_name):
        translations = { 'lp'            : line_prefix,
                         'public_format' : self.public_format,
                         'name'          : variable_name }

        template = (
            '${lp}@${name}: a #${public_format}.\n')
        return string.Template(template).substitute(translations)
