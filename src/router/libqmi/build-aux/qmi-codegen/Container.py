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

import string

import utils
from FieldResult import FieldResult
from Field import Field

"""
The Container class takes care of handling collections of Input or
Output fields
"""
class Container:

    """
    Constructor
    """
    def __init__(self, prefix, container_type, dictionary, common_objects_dictionary, static):
        # The field container prefix usually contains the name of the Message,
        # e.g. "Qmi Message Ctl Something"
        self.prefix = prefix

        # We may have 'Input' or 'Output' containers
        if container_type == 'Input':
            self.readonly = False
        elif container_type == 'Output':
            self.readonly = True
        else:
            raise ValueError('Cannot handle container type \'%s\'' % container_type)

        self.name = container_type

        self.static = static

        # Create the composed full name (prefix + name),
        #  e.g. "Qmi Message Ctl Something Output"
        self.fullname = self.prefix + ' ' + self.name

        self.fields = None
        if dictionary is not None:
            self.fields = []

            # First, look for references to common types
            for field_dictionary in dictionary:
                if 'common-ref' in field_dictionary:
                    for common in common_objects_dictionary:
                        if common['type'] == 'TLV' and \
                           common['common-ref'] == field_dictionary['common-ref']:
                           # Replace the reference with a copy of the common dictionary
                           # If the source reference has prerequisites, add them to the copy
                           copy = dict(common)
                           if 'prerequisites' in field_dictionary:
                               copy['prerequisites'] = field_dictionary['prerequisites']
                           dictionary.remove(field_dictionary)
                           dictionary.append(copy)
                           break
                    else:
                        raise RuntimeError('Common type \'%s\' not found' % field_dictionary['name'])

            # We need to sort the fields, so that the ones with prerequisites are
            # include after the prerequisites themselves. Note: we don't currently
            # support complex setups yet.
            sorted_dictionary = []
            for field_dictionary in dictionary:
                if 'prerequisites' in field_dictionary:
                    sorted_dictionary.append(field_dictionary)
                else:
                    sorted_dictionary.insert(0, field_dictionary)

            # Then, really parse each field
            for field_dictionary in sorted_dictionary:
                if field_dictionary['type'] == 'TLV':
                    if field_dictionary['format'] == 'struct' and \
                       field_dictionary['name'] == 'Result':
                        self.fields.append(FieldResult(self.fullname, field_dictionary, common_objects_dictionary, container_type, static))
                    else:
                        self.fields.append(Field(self.fullname, field_dictionary, common_objects_dictionary, container_type, static))


    """
    Emit enumeration of TLVs in the container
    """
    def __emit_tlv_ids_enum(self, f):
        if self.fields is None:
            return

        f.write('\n')
        for tlv in self.fields:
            translations = { 'enum_name'  : tlv.id_enum_name,
                             'enum_value' : tlv.id }
            template = (
                '#define ${enum_name} ${enum_value}\n')
            f.write(string.Template(template).substitute(translations))


    """
    Emit new container types
    """
    def __emit_types(self, hfile, cfile, translations):
        translations['type_macro'] = 'QMI_TYPE_' + utils.remove_prefix(utils.build_underscore_uppercase_name(self.fullname), 'QMI_')
        # Emit types header
        template = (
            '\n'
            '/**\n'
            ' * ${camelcase}:\n'
            ' *\n'
            ' * The #${camelcase} structure contains private data and should only be accessed\n'
            ' * using the provided API.\n'
            ' */\n'
            'typedef struct _${camelcase} ${camelcase};\n'
            '${static}GType ${underscore}_get_type (void) G_GNUC_CONST;\n'
            '#define ${type_macro} (${underscore}_get_type ())\n')
        hfile.write(string.Template(template).substitute(translations))

        # Emit types source
        template = (
            '\n'
            'struct _${camelcase} {\n'
            '    volatile gint ref_count;\n')
        cfile.write(string.Template(template).substitute(translations))

        if self.fields is not None:
            for field in self.fields:
                if field.variable is not None:
                    variable_declaration = field.variable.build_variable_declaration('    ', field.variable_name)
                    translations['field_variable_name'] = field.variable_name
                    translations['field_name'] = field.name
                    template = (
                        '\n'
                        '    /* ${field_name} */\n'
                        '    gboolean ${field_variable_name}_set;\n')
                    cfile.write(string.Template(template).substitute(translations))
                    cfile.write(variable_declaration)

        cfile.write(
            '};\n')


    """
    Emit container handling core implementation
    """
    def __emit_core(self, hfile, cfile, translations):
        # Emit container core header
        template = (
            '\n'
            '${static}${camelcase} *${underscore}_ref (${camelcase} *self);\n'
            '${static}void ${underscore}_unref (${camelcase} *self);\n')
        if self.readonly == False:
            template += (
                '${static}${camelcase} *${underscore}_new (void);\n')

        if self.static:
            cfile.write(string.Template(template).substitute(translations))
        else:
            hfile.write(string.Template(template).substitute(translations))

        # Emit container core source
        template = (
            '\n'
            '${static}GType\n'
            '${underscore}_get_type (void)\n'
            '{\n'
            '    static volatile gsize g_define_type_id__volatile = 0;\n'
            '\n'
            '    if (g_once_init_enter (&g_define_type_id__volatile)) {\n'
            '        GType g_define_type_id =\n'
            '            g_boxed_type_register_static (g_intern_static_string ("${camelcase}"),\n'
            '                                          (GBoxedCopyFunc) ${underscore}_ref,\n'
            '                                          (GBoxedFreeFunc) ${underscore}_unref);\n'
            '\n'
            '        g_once_init_leave (&g_define_type_id__volatile, g_define_type_id);\n'
            '    }\n'
            '\n'
            '    return g_define_type_id__volatile;\n'
            '}\n'
            '\n'
            '/**\n'
            ' * ${underscore}_ref:\n'
            ' * @self: a #${camelcase}.\n'
            ' *\n'
            ' * Atomically increments the reference count of @self by one.\n'
            ' *\n'
            ' * Returns: the new reference to @self.\n'
            ' */\n'
            '${static}${camelcase} *\n'
            '${underscore}_ref (${camelcase} *self)\n'
            '{\n'
            '    g_return_val_if_fail (self != NULL, NULL);\n'
            '\n'
            '    g_atomic_int_inc (&self->ref_count);\n'
            '    return self;\n'
            '}\n'
            '\n'
            '/**\n'
            ' * ${underscore}_unref:\n'
            ' * @self: a #${camelcase}.\n'
            ' *\n'
            ' * Atomically decrements the reference count of @self by one.\n'
            ' * If the reference count drops to 0, @self is completely disposed.\n'
            ' */\n'
            '${static}void\n'
            '${underscore}_unref (${camelcase} *self)\n'
            '{\n'
            '    g_return_if_fail (self != NULL);\n'
            '\n'
            '    if (g_atomic_int_dec_and_test (&self->ref_count)) {\n')

        if self.fields is not None:
            for field in self.fields:
                if field.variable is not None and field.variable.needs_dispose is True:
                    template += field.variable.build_dispose('        ', 'self->' + field.variable_name)

        template += (
            '        g_slice_free (${camelcase}, self);\n'
            '    }\n'
            '}\n')
        cfile.write(string.Template(template).substitute(translations))

        # _new() is only generated if the container is not readonly
        if self.readonly == True:
            return

        template = (
            '\n'
            '/**\n'
            ' * ${underscore}_new:\n'
            ' *\n'
            ' * Allocates a new #${camelcase}.\n'
            ' *\n'
            ' * Returns: the newly created #${camelcase}. The returned value should be freed with ${underscore}_unref().\n'
            ' */\n'
            '${static}${camelcase} *\n'
            '${underscore}_new (void)\n'
            '{\n'
            '    ${camelcase} *self;\n'
            '\n'
            '    self = g_slice_new0 (${camelcase});\n'
            '    self->ref_count = 1;\n'
            '    return self;\n'
            '}\n')
        cfile.write(string.Template(template).substitute(translations))


    """
    Emit container implementation
    """
    def emit(self, hfile, cfile):
        translations = { 'name'       : self.name,
                         'camelcase'  : utils.build_camelcase_name (self.fullname),
                         'underscore' : utils.build_underscore_name (self.fullname),
                         'static'     : 'static ' if self.static else '' }

        auxfile = cfile if self.static else hfile

        if self.fields is None:
            template = ('\n'
                        '/* Note: no fields in the ${name} container */\n')
            auxfile.write(string.Template(template).substitute(translations))
            cfile.write(string.Template(template).substitute(translations))
            return

        # Emit the container and field  types
        # Emit field getter/setter
        if self.fields is not None:
            for field in self.fields:
                field.emit_types(auxfile, cfile)
        self.__emit_types(auxfile, cfile, translations)

        # Emit TLV enums
        self.__emit_tlv_ids_enum(cfile)

        # Emit fields
        if self.fields is not None:
            for field in self.fields:
                field.emit_getter(auxfile, cfile)
                if self.readonly == False:
                    field.emit_setter(auxfile, cfile)

        # Emit the container core
        self.__emit_core(auxfile, cfile, translations)


    """
    Add sections
    """
    def add_sections(self, sections):
        if self.fields is None:
            return

        translations = { 'name'       : self.name,
                         'camelcase'  : utils.build_camelcase_name (self.fullname),
                         'underscore' : utils.build_underscore_name (self.fullname),
                         'type_macro' : 'QMI_TYPE_' + utils.remove_prefix(utils.build_underscore_uppercase_name(self.fullname), 'QMI_') }

        # Standard
        template = (
            '${underscore}_get_type\n'
            '${type_macro}\n')
        sections['standard'] += string.Template(template).substitute(translations)

        # Public types
        template = (
            '${camelcase}\n')
        sections['public-types'] += string.Template(template).substitute(translations)

        # Public methods
        template = '<SUBSECTION ${camelcase}Methods>\n'
        if self.readonly == False:
            template += (
                '${underscore}_new\n')
        template += (
            '${underscore}_ref\n'
            '${underscore}_unref\n')
        sections['public-methods'] += string.Template(template).substitute(translations)

        for field in self.fields:
            field.add_sections(sections)
