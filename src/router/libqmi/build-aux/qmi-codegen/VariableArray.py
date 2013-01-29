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
Variable type for Arrays ('array' format)
"""
class VariableArray(Variable):

    """
    Constructor
    """
    def __init__(self, dictionary, array_element_type, container_type):

        # Call the parent constructor
        Variable.__init__(self, dictionary)

        self.private_format  = 'GArray *'
        self.public_format = self.private_format
        self.fixed_size = 0
        self.name = dictionary['name']

        # The array and its contents need to get disposed
        self.needs_dispose = True

        # We need to know whether the variable comes in an Input container or in
        # an Output container, as we should not dump the element clear() helper method
        # if the variable is from an Input container.
        self.container_type = container_type

        # Load variable type of this array
        if 'name' in dictionary['array-element']:
            self.array_element = VariableFactory.create_variable(dictionary['array-element'], array_element_type + ' ' + dictionary['array-element']['name'], self.container_type)
        else:
            self.array_element = VariableFactory.create_variable(dictionary['array-element'], '', self.container_type)

        # Load variable type for the array size prefix
        if 'size-prefix-format' in dictionary:
            # We do NOT allow 64-bit types as array sizes (GArray won't support them)
            if dictionary['size-prefix-format'] not in [ 'guint8', 'guint16', 'guint32' ]:
                raise ValueError('Invalid size prefix format (%s): not guint8 or guint16 or guint32' % dictionary['size-prefix-format'])
            default_array_size = { 'format' : dictionary['size-prefix-format'] }
            self.array_size_element = VariableFactory.create_variable(default_array_size, '', self.container_type)
        elif 'fixed-size' in dictionary:
            # fixed-size arrays have no size element, obviously
            self.fixed_size = dictionary['fixed-size']
            if int(self.fixed_size) == 0 or int(self.fixed_size) > 512:
                raise ValueError('Fixed array size %s out of bounds (not between 0 and 512)' % self.fixed_size)
        else:
            # Default to 'guint8' if no explicit array size given
            default_array_size = { 'format' : 'guint8' }
            self.array_size_element = VariableFactory.create_variable(default_array_size, '', self.container_type)


    """
    Emit the type for the array element
    """
    def emit_types(self, f):
        self.array_element.emit_types(f)


    """
    Constructs the name of the array clear function
    """
    def clear_func_name(self):
        # element public format might be a base type like 'gchar *' rather
        # than a structure name like QmiFooBar
        elt_name = self.array_element.public_format.replace('*', 'pointer')
        return utils.build_underscore_name(self.name) + \
             '_' + \
             utils.build_underscore_name_from_camelcase(utils.build_camelcase_name(elt_name))


    """
    Emits the code to clear the element of the array
    """
    def emit_helper_methods(self, hfile, cfile):
        self.array_element.emit_helper_methods(hfile, cfile)

        # No need for the clear func if no need to dispose the contents
        if self.array_element.needs_dispose == False:
            return

        # No need for the clear func if we were not the ones who created
        # the array
        if self.container_type == "Input":
            return

        translations = { 'element_format'   : self.array_element.public_format,
                         'underscore'       : self.clear_func_name(),
                         'dispose_contents' : self.array_element.build_dispose('    ', '(*p)') }

        template = (
            '\n'
            'static void\n'
            '${underscore}_clear (${element_format} *p)\n'
            '{\n'
            '$dispose_contents'
            '}\n')
        cfile.write(string.Template(template).substitute(translations))


    """
    Reading an array from the raw byte buffer is just about providing a loop to
    read every array element one by one.
    """
    def emit_buffer_read(self, f, line_prefix, variable_name, buffer_name, buffer_len):
        common_var_prefix = utils.build_underscore_name(self.name)
        translations = { 'lp'             : line_prefix,
                         'private_format' : self.private_format,
                         'public_array_element_format' : self.array_element.public_format,
                         'underscore'     : self.clear_func_name(),
                         'variable_name'  : variable_name,
                         'buffer_name'    : buffer_name,
                         'buffer_len'     : buffer_len,
                         'common_var_prefix' : common_var_prefix }

        template = (
            '${lp}{\n'
            '${lp}    guint ${common_var_prefix}_i;\n')
        f.write(string.Template(template).substitute(translations))

        if self.fixed_size:
            translations['fixed_size'] = self.fixed_size

            template = (
                '${lp}    guint16 ${common_var_prefix}_n_items = ${fixed_size};\n'
                '\n')
            f.write(string.Template(template).substitute(translations))
        else:
            translations['array_size_element_format'] = self.array_size_element.public_format

            template = (
                '${lp}    ${array_size_element_format} ${common_var_prefix}_n_items;\n'
                '\n'
                '${lp}    /* Read number of items in the array */\n')
            f.write(string.Template(template).substitute(translations))

            self.array_size_element.emit_buffer_read(f, line_prefix + '    ', common_var_prefix + '_n_items', buffer_name, buffer_len)

        template = (
            '\n'
            '${lp}    ${variable_name} = g_array_sized_new (\n'
            '${lp}        FALSE,\n'
            '${lp}        FALSE,\n'
            '${lp}        sizeof (${public_array_element_format}),\n'
            '${lp}        (guint)${common_var_prefix}_n_items);\n'
            '\n')

        if self.array_element.needs_dispose == True:
            template += (
                '${lp}    g_array_set_clear_func (${variable_name},\n'
                '${lp}                            (GDestroyNotify)${underscore}_clear);\n'
                '\n')

        template += (
            '${lp}    for (${common_var_prefix}_i = 0; ${common_var_prefix}_i < ${common_var_prefix}_n_items; ${common_var_prefix}_i++) {\n'
            '${lp}        ${public_array_element_format} ${common_var_prefix}_aux;\n'
            '\n')
        f.write(string.Template(template).substitute(translations))

        self.array_element.emit_buffer_read(f, line_prefix + '        ', common_var_prefix + '_aux', buffer_name, buffer_len)

        template = (
            '${lp}        g_array_insert_val (${variable_name}, ${common_var_prefix}_i, ${common_var_prefix}_aux);\n'
            '${lp}    }\n'
            '${lp}}\n')
        f.write(string.Template(template).substitute(translations))


    """
    Emits the code involved in computing the size of the variable.
    """
    def emit_size_read(self, f, line_prefix, variable_name, buffer_name, buffer_len):
        common_var_prefix = utils.build_underscore_name(self.name)
        translations = { 'lp'             : line_prefix,
                         'variable_name'  : variable_name,
                         'buffer_name'    : buffer_name,
                         'buffer_len'     : buffer_len,
                         'common_var_prefix' : common_var_prefix }

        template = (
            '${lp}{\n'
            '${lp}    guint ${common_var_prefix}_i;\n')
        f.write(string.Template(template).substitute(translations))

        if self.fixed_size:
            translations['fixed_size'] = self.fixed_size

            template = (
                '${lp}    guint16 ${common_var_prefix}_n_items = ${fixed_size};\n'
                '\n')
            f.write(string.Template(template).substitute(translations))
        else:
            translations['array_size_element_format'] = self.array_size_element.public_format
            if self.array_size_element.public_format == 'guint8':
                translations['array_size_element_size'] = '1'
            elif self.array_size_element.public_format == 'guint16':
                translations['array_size_element_size'] = '2'
            elif self.array_size_element.public_format == 'guint32':
                translations['array_size_element_size'] = '4'
            else:
                translations['array_size_element_size'] = '0'

            template = (
                '${lp}    ${array_size_element_format} ${common_var_prefix}_n_items;\n'
                '${lp}    const guint8 *${common_var_prefix}_aux_buffer = &${buffer_name}[${variable_name}];\n'
                '${lp}    guint16 ${common_var_prefix}_aux_buffer_len = ${buffer_len} - ${variable_name};\n'
                '\n'
                '${lp}    ${variable_name} += ${array_size_element_size};\n'
                '\n')
            f.write(string.Template(template).substitute(translations))

            self.array_size_element.emit_buffer_read(f, line_prefix + '    ', common_var_prefix + '_n_items', common_var_prefix + '_aux_buffer', common_var_prefix + '_aux_buffer_len')

        template = (
            '${lp}    for (${common_var_prefix}_i = 0; ${common_var_prefix}_i < ${common_var_prefix}_n_items; ${common_var_prefix}_i++) {\n'
            '\n')
        f.write(string.Template(template).substitute(translations))

        self.array_element.emit_size_read(f, line_prefix + '        ', variable_name, buffer_name, buffer_len)

        template = (
            '${lp}    }\n'
            '${lp}}\n')
        f.write(string.Template(template).substitute(translations))

    """
    Writing an array to the raw byte buffer is just about providing a loop to
    write every array element one by one.
    """
    def emit_buffer_write(self, f, line_prefix, variable_name, buffer_name, buffer_len):
        common_var_prefix = utils.build_underscore_name(self.name)
        translations = { 'lp'             : line_prefix,
                         'variable_name'  : variable_name,
                         'buffer_name'    : buffer_name,
                         'buffer_len'     : buffer_len,
                         'common_var_prefix' : common_var_prefix }

        template = (
            '${lp}{\n'
            '${lp}    guint ${common_var_prefix}_i;\n')
        f.write(string.Template(template).substitute(translations))

        if self.fixed_size == 0:
            translations['array_size_element_format'] = self.array_size_element.private_format

            template = (
                '${lp}    ${array_size_element_format} ${common_var_prefix}_n_items;\n'
                '\n'
                '${lp}    /* Write the number of items in the array first */\n'
                '${lp}    ${common_var_prefix}_n_items = (${array_size_element_format}) ${variable_name}->len;\n')
            f.write(string.Template(template).substitute(translations))

            self.array_size_element.emit_buffer_write(f, line_prefix + '    ', common_var_prefix + '_n_items', buffer_name, buffer_len)

        template = (
            '\n'
            '${lp}    for (${common_var_prefix}_i = 0; ${common_var_prefix}_i < ${variable_name}->len; ${common_var_prefix}_i++) {\n')
        f.write(string.Template(template).substitute(translations))

        self.array_element.emit_buffer_write(f, line_prefix + '        ', 'g_array_index (' + variable_name + ', ' + self.array_element.public_format + ',' + common_var_prefix + '_i)', buffer_name, buffer_len)

        template = (
            '${lp}    }\n'
            '${lp}}\n')
        f.write(string.Template(template).substitute(translations))


    """
    The array will be printed as a list of fields enclosed between curly
    brackets
    """
    def emit_get_printable(self, f, line_prefix, printable, buffer_name, buffer_len):
        common_var_prefix = utils.build_underscore_name(self.name)
        translations = { 'lp'          : line_prefix,
                         'printable'   : printable,
                         'buffer_name' : buffer_name,
                         'buffer_len'  : buffer_len,
                         'common_var_prefix' : common_var_prefix }

        template = (
            '${lp}{\n'
            '${lp}    guint ${common_var_prefix}_i;\n')
        f.write(string.Template(template).substitute(translations))

        if self.fixed_size:
            translations['fixed_size'] = self.fixed_size

            template = (
                '${lp}    guint16 ${common_var_prefix}_n_items = ${fixed_size};\n'
                '\n')
            f.write(string.Template(template).substitute(translations))
        else:
            translations['array_size_element_format'] = self.array_size_element.public_format

            template = (
                '${lp}    ${array_size_element_format} ${common_var_prefix}_n_items;\n'
                '\n'
                '${lp}    /* Read number of items in the array */\n')
            f.write(string.Template(template).substitute(translations))

            self.array_size_element.emit_buffer_read(f, line_prefix + '    ', common_var_prefix + '_n_items', buffer_name, buffer_len)

        template = (
            '\n'
            '${lp}    g_string_append (${printable}, "{");\n'
            '\n'
            '${lp}    for (${common_var_prefix}_i = 0; ${common_var_prefix}_i < ${common_var_prefix}_n_items; ${common_var_prefix}_i++) {\n'
            '${lp}        g_string_append_printf (${printable}, " [%u] = \'", ${common_var_prefix}_i);\n')
        f.write(string.Template(template).substitute(translations))

        self.array_element.emit_get_printable(f, line_prefix + '        ', printable, buffer_name, buffer_len);

        template = (
            '${lp}        g_string_append (${printable}, " \'");\n'
            '${lp}    }\n'
            '\n'
            '${lp}    g_string_append (${printable}, "}");\n'
            '${lp}}')
        f.write(string.Template(template).substitute(translations))


    """
    Variable declaration
    """
    def build_variable_declaration(self, line_prefix, variable_name):
        translations = { 'lp'   : line_prefix,
                         'name' : variable_name }

        template = (
            '${lp}GArray *${name};\n')
        return string.Template(template).substitute(translations)


    """
    Getter for the array type
    """
    def build_getter_declaration(self, line_prefix, variable_name):
        translations = { 'lp'   : line_prefix,
                         'name' : variable_name }

        template = (
            '${lp}GArray **${name},\n')
        return string.Template(template).substitute(translations)


    """
    Documentation for the getter
    """
    def build_getter_documentation(self, line_prefix, variable_name):
        translations = { 'lp'                          : line_prefix,
                         'public_array_element_format' : self.array_element.public_format,
                         'name'                        : variable_name }

        template = (
            '${lp}@${name}: a placeholder for the output #GArray of #${public_array_element_format} elements, or %NULL if not required. Do not free it, it is owned by @self.\n')
        return string.Template(template).substitute(translations)


    """
    Builds the array getter implementation
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
    Setter for the array type
    """
    def build_setter_declaration(self, line_prefix, variable_name):
        translations = { 'lp'   : line_prefix,
                         'name' : variable_name }

        template = (
            '${lp}GArray *${name},\n')
        return string.Template(template).substitute(translations)


    """
    Documentation for the setter
    """
    def build_setter_documentation(self, line_prefix, variable_name):
        translations = { 'lp'                          : line_prefix,
                         'public_array_element_format' : self.array_element.public_format,
                         'name'                        : variable_name }

        template = (
            '${lp}@${name}: a #GArray of #${public_array_element_format} elements. A new reference to @${name} will be taken.\n')
        return string.Template(template).substitute(translations)


    """
    Builds the array setter implementation
    """
    def build_setter_implementation(self, line_prefix, variable_name_from, variable_name_to):
        translations = { 'lp'   : line_prefix,
                         'from' : variable_name_from,
                         'to'   : variable_name_to }

        template = (
            '${lp}if (${to})\n'
            '${lp}    g_array_unref (${to});\n'
            '${lp}${to} = g_array_ref (${from});\n')
        return string.Template(template).substitute(translations)


    """
    Documentation for the struct field
    """
    def build_struct_field_documentation(self, line_prefix, variable_name):
        translations = { 'lp'                          : line_prefix,
                         'public_array_element_format' : self.array_element.public_format,
                         'name'                        : variable_name }

        template = (
            '${lp}@${name}: a #GArray of #${public_array_element_format} elements.\n')
        return string.Template(template).substitute(translations)


    """
    Dispose the array just with an unref
    """
    def build_dispose(self, line_prefix, variable_name):
        translations = { 'lp'            : line_prefix,
                         'variable_name' : variable_name }

        template = (
            '${lp}if (${variable_name})\n'
            '${lp}    g_array_unref (${variable_name});\n')
        return string.Template(template).substitute(translations)


    """
    Add sections
    """
    def add_sections(self, sections):
        self.array_element.add_sections(sections)
