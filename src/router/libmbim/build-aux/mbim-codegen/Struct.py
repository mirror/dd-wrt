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
# Copyright (C) 2013 - 2014 Aleksander Morgado <aleksander@aleksander.es>
#

import string

import utils

"""
The Struct class takes care of emitting the struct type
"""
class Struct:

    """
    Constructor
    """
    def __init__(self, dictionary):
        self.name = dictionary['name']
        self.contents = dictionary['contents']

        # Whether the struct is used as a single field, or as an array of
        # fields. Will be updated after having created the object.
        self.single_member = False
        self.array_member = False

        # Check whether the struct is composed of fixed-sized fields
        self.size = 0
        for field in self.contents:
            if field['format'] == 'guint32':
                self.size += 4
            elif field['format'] == 'guint64':
                self.size += 8
            elif field['format'] == 'uuid':
                self.size += 16
            elif field['format'] == 'ipv4':
                self.size += 4
            elif field['format'] == 'ipv6':
                self.size += 16
            else:
                self.size = 0
                break


    """
    Emit the new struct type
    """
    def _emit_type(self, hfile):
        translations = { 'name' : self.name }
        template = (
            '\n'
            '/**\n'
            ' * ${name}:\n')
        for field in self.contents:
            translations['field_name_underscore'] = utils.build_underscore_name_from_camelcase(field['name'])
            if field['format'] == 'uuid':
                inner_template = (
                    ' * @${field_name_underscore}: a #MbimUuid.\n')
            elif field['format'] =='byte-array':
                inner_template = (' * @${field_name_underscore}: an array of #guint8 values.\n')
            elif field['format'] =='unsized-byte-array' or field['format'] == 'ref-byte-array':
                inner_template = ''
                if 'array-size-field' not in field:
                    inner_template += (' * @${field_name_underscore}_size: size of the ${field_name_underscore} array.\n')
                inner_template += (' * @${field_name_underscore}: an array of #guint8 values.\n')
            elif field['format'] == 'guint32':
                inner_template = (
                    ' * @${field_name_underscore}: a #guint32.\n')
            elif field['format'] == 'guint32-array':
                inner_template = (
                    ' * @${field_name_underscore}: an array of #guint32 values.\n')
            elif field['format'] == 'guint64':
                inner_template = (
                    ' * @${field_name_underscore}: a #guint64.\n')
            elif field['format'] == 'string':
                inner_template = (
                    ' * @${field_name_underscore}: a string.\n')
            elif field['format'] == 'string-array':
                inner_template = (
                    ' * @${field_name_underscore}: an array of strings.\n')
            elif field['format'] == 'ipv4':
                inner_template = (
                    ' * @${field_name_underscore}: a #MbimIPv4.\n')
            elif field['format'] == 'ref-ipv4':
                inner_template = (
                    ' * @${field_name_underscore}: a #MbimIPv4.\n')
            elif field['format'] == 'ipv4-array':
                inner_template = (
                    ' * @${field_name_underscore}: an array of #MbimIPv4 values.\n')
            elif field['format'] == 'ipv6':
                inner_template = (
                    ' * @${field_name_underscore}: a #MbimIPv6\n')
            elif field['format'] == 'ref-ipv6':
                inner_template = (
                    ' * @${field_name_underscore}: a #MbimIPv6\n')
            elif field['format'] == 'ipv6-array':
                inner_template = (
                    ' * @${field_name_underscore}: an array of #MbimIPv6 values.\n')
            else:
                raise ValueError('Cannot handle format \'%s\' in struct' % field['format'])
            template += string.Template(inner_template).substitute(translations)

        template += (
            ' */\n'
            'typedef struct {\n')
        for field in self.contents:
            translations['field_name_underscore'] = utils.build_underscore_name_from_camelcase(field['name'])
            if field['format'] == 'uuid':
                inner_template = (
                    '    MbimUuid ${field_name_underscore};\n')
            elif field['format'] == 'byte-array':
                translations['array_size'] = field['array-size']
                inner_template = (
                    '    guint8 ${field_name_underscore}[${array_size}];\n')
            elif field['format'] == 'unsized-byte-array' or field['format'] == 'ref-byte-array':
                inner_template = ''
                if 'array-size-field' not in field:
                    inner_template += (
                        '    guint32 ${field_name_underscore}_size;\n')
                inner_template += (
                    '    guint8 *${field_name_underscore};\n')
            elif field['format'] == 'guint32':
                inner_template = (
                    '    guint32 ${field_name_underscore};\n')
            elif field['format'] == 'guint32-array':
                inner_template = (
                    '    guint32 *${field_name_underscore};\n')
            elif field['format'] == 'guint64':
                inner_template = (
                    '    guint64 ${field_name_underscore};\n')
            elif field['format'] == 'string':
                inner_template = (
                    '    gchar *${field_name_underscore};\n')
            elif field['format'] == 'string-array':
                inner_template = (
                    '    gchar **${field_name_underscore};\n')
            elif field['format'] == 'ipv4':
                inner_template = (
                    '    MbimIPv4 ${field_name_underscore};\n')
            elif field['format'] == 'ref-ipv4':
                inner_template = (
                    '    MbimIPv4 ${field_name_underscore};\n')
            elif field['format'] == 'ipv4-array':
                inner_template = (
                    '    MbimIPv4 *${field_name_underscore};\n')
            elif field['format'] == 'ipv6':
                inner_template = (
                    '    MbimIPv6 ${field_name_underscore};\n')
            elif field['format'] == 'ref-ipv6':
                inner_template = (
                    '    MbimIPv6 ${field_name_underscore};\n')
            elif field['format'] == 'ipv6-array':
                inner_template = (
                    '    MbimIPv6 *${field_name_underscore};\n')
            else:
                raise ValueError('Cannot handle format \'%s\' in struct' % field['format'])
            template += string.Template(inner_template).substitute(translations)
        template += (
            '} ${name};\n')
        hfile.write(string.Template(template).substitute(translations))


    """
    Emit the type's free methods
    """
    def _emit_free(self, hfile, cfile):
        translations = { 'name' : self.name,
                         'name_underscore' : utils.build_underscore_name_from_camelcase(self.name) }
        template = ''

        if self.single_member == True:
            template = (
                '\n'
                'void ${name_underscore}_free (${name} *var);\n')
            hfile.write(string.Template(template).substitute(translations))


        template = (
            '\n'
            'static void\n'
            '_${name_underscore}_free (${name} *var)\n'
            '{\n'
            '    if (!var)\n'
            '        return;\n'
            '\n')

        for field in self.contents:
            translations['field_name_underscore'] = utils.build_underscore_name_from_camelcase(field['name'])
            inner_template = ''
            if field['format'] == 'uuid':
                pass
            elif field['format'] == 'unsized-byte-array' or field['format'] == 'ref-byte-array':
                inner_template += (
                    '    g_free (var->${field_name_underscore});\n')
            elif field['format'] == 'guint32':
                pass
            elif field['format'] == 'guint32-array':
                inner_template += (
                    '    g_free (var->${field_name_underscore});\n')
            elif field['format'] == 'guint64':
                pass
            elif field['format'] == 'string':
                inner_template += (
                    '    g_free (var->${field_name_underscore});\n')
            elif field['format'] == 'string-array':
                inner_template += (
                    '    g_strfreev (var->${field_name_underscore});\n')
            elif field['format'] == 'ipv4':
                pass
            elif field['format'] == 'ref-ipv4':
                pass
            elif field['format'] == 'ipv4-array':
                inner_template += (
                    '    g_free (var->${field_name_underscore});\n')
            elif field['format'] == 'ipv6':
                pass
            elif field['format'] == 'ref-ipv6':
                pass
            elif field['format'] == 'ipv6-array':
                inner_template += (
                    '    g_free (var->${field_name_underscore});\n')
            else:
                raise ValueError('Cannot handle format \'%s\' in struct clear' % field['format'])
            template += string.Template(inner_template).substitute(translations)

        template += (
            '    g_free (var);\n'
            '}\n')
        cfile.write(string.Template(template).substitute(translations))

        if self.single_member == True:
            template = (
                '\n'
                '/**\n'
                ' * ${name_underscore}_free:\n'
                ' * @var: a #${name}.\n'
                ' *\n'
                ' * Frees the memory allocated for the #${name}.\n'
                ' */\n'
                'void\n'
                '${name_underscore}_free (${name} *var)\n'
                '{\n'
                '    _${name_underscore}_free (var);\n'
                '}\n')
            cfile.write(string.Template(template).substitute(translations))

        if self.array_member:
            template = (
                '\n'
                'void ${name_underscore}_array_free (${name} **array);\n')
            hfile.write(string.Template(template).substitute(translations))

            template = (
                '\n'
                '/**\n'
                ' * ${name_underscore}_array_free:\n'
                ' * @array: a #NULL terminated array of #${name} structs.\n'
                ' *\n'
                ' * Frees the memory allocated for the array of #${name}s.\n'
                ' */\n'
                'void\n'
                '${name_underscore}_array_free (${name} **array)\n'
                '{\n'
                '    guint32 i;\n'
                '\n'
                '    if (!array)\n'
                '        return;\n'
                '\n'
                '    for (i = 0; array[i]; i++)\n'
                '        _${name_underscore}_free (array[i]);\n'
                '    g_free (array);\n'
                '}\n')
            cfile.write(string.Template(template).substitute(translations))


    """
    Emit the type's read methods
    """
    def _emit_read(self, cfile):
        translations = { 'name'            : self.name,
                         'name_underscore' : utils.build_underscore_name_from_camelcase(self.name),
                         'struct_size'     : self.size }

        template = (
            '\n'
            'static ${name} *\n'
            '_mbim_message_read_${name_underscore}_struct (\n'
            '    const MbimMessage *self,\n'
            '    guint32 relative_offset,\n'
            '    guint32 *bytes_read)\n'
            '{\n'
            '    ${name} *out;\n'
            '    guint32 offset = relative_offset;\n'
            '\n'
            '    g_assert (self != NULL);\n'
            '\n'
            '    out = g_new (${name}, 1);\n')

        for field in self.contents:
            translations['field_name_underscore'] = utils.build_underscore_name_from_camelcase(field['name'])

            inner_template = ''
            if field['format'] == 'uuid':
                inner_template += (
                    '\n'
                    '    memcpy (&(out->${field_name_underscore}), _mbim_message_read_uuid (self, offset), 16);\n'
                    '    offset += 16;\n')
            elif field['format'] == 'ref-byte-array':
                if 'array-size-field' in field:
                    translations['array_size_field_name_underscore'] = utils.build_underscore_name_from_camelcase(field['array-size-field'])
                    inner_template += (
                        '\n'
                        '    {\n'
                        '        const guint8 *tmp;\n'
                        '\n'
                        '        tmp = _mbim_message_read_byte_array (self, relative_offset, offset, TRUE, FALSE, NULL);\n'
                        '        out->${field_name_underscore} = g_malloc (out->${array_size_field_name_underscore});\n'
                        '        memcpy (out->${field_name_underscore}, tmp, out->${array_size_field_name_underscore});\n'
                        '        offset += 4;\n'
                        '    }\n')
                else:
                    inner_template += (
                        '\n'
                        '    {\n'
                        '        const guint8 *tmp;\n'
                        '\n'
                        '        tmp = _mbim_message_read_byte_array (self, relative_offset, offset, TRUE, TRUE, &(out->${field_name_underscore}_size));\n'
                        '        out->${field_name_underscore} = g_malloc (out->${field_name_underscore}_size);\n'
                        '        memcpy (out->${field_name_underscore}, tmp, out->${field_name_underscore}_size);\n'
                        '        offset += 8;\n'
                        '    }\n')
            elif field['format'] == 'unsized-byte-array':
                inner_template += (
                    '\n'
                    '    {\n'
                    '        const guint8 *tmp;\n'
                    '\n'
                    '        tmp = _mbim_message_read_byte_array (self, relative_offset, offset, FALSE, FALSE, &(out->${field_name_underscore}_size));\n'
                    '        out->${field_name_underscore} = g_malloc (out->${field_name_underscore}_size);\n'
                    '        memcpy (out->${field_name_underscore}, tmp, out->${field_name_underscore}_size);\n'
                    '        /* no offset update expected, this should be the last field */\n'
                    '    }\n')
            elif field['format'] == 'byte-array':
                translations['array_size'] = field['array-size']
                inner_template += (
                    '\n'
                    '    {\n'
                    '        const guint8 *tmp;\n'
                    '\n'
                    '        tmp = _mbim_message_read_byte_array (self, relative_offset, offset, FALSE, FALSE, NULL));\n'
                    '        memcpy (out->${field_name_underscore}, tmp, ${array_size});\n'
                    '        offset += ${array_size};\n'
                    '    }\n')
            elif field['format'] == 'guint32':
                inner_template += (
                    '\n'
                    '    out->${field_name_underscore} = _mbim_message_read_guint32 (self, offset);\n'
                    '    offset += 4;\n')
            elif field['format'] == 'guint32-array':
                translations['array_size_field_name_underscore'] = utils.build_underscore_name_from_camelcase(field['array-size-field'])
                inner_template += (
                    '\n'
                    '    out->${field_name_underscore} = _mbim_message_read_guint32_array (self, out->${array_size_field_name_underscore}, offset);\n'
                    '    offset += (4 * out->${array_size_field_name_underscore});\n')
            elif field['format'] == 'guint64':
                inner_template += (
                    '\n'
                    '    out->${field_name_underscore} = _mbim_message_read_guint64 (self, offset);\n'
                    '    offset += 8;\n')
            elif field['format'] == 'string':
                inner_template += (
                    '\n'
                    '    out->${field_name_underscore} = _mbim_message_read_string (self, relative_offset, offset);\n'
                    '    offset += 8;\n')
            elif field['format'] == 'string-array':
                translations['array_size_field_name_underscore'] = utils.build_underscore_name_from_camelcase(field['array-size-field'])
                inner_template += (
                    '\n'
                    '    out->${field_name_underscore} = _mbim_message_read_string_array (self, out->${array_size_field_name_underscore}, relative_offset, offset);\n'
                    '    offset += (8 * out->${array_size_field_name_underscore});\n')
            elif field['format'] == 'ipv4':
                inner_template += (
                    '\n'
                    '    memcpy (&(out->${field_name_underscore}), _mbim_message_read_ipv4 (self, offset, FALSE), 4);\n'
                    '    offset += 4;\n')
            elif field['format'] == 'ref-ipv4':
                inner_template += (
                    '\n'
                    '    memcpy (&(out->${field_name_underscore}), _mbim_message_read_ipv4 (self, offset, TRUE), 4);\n'
                    '    offset += 4;\n')
            elif field['format'] == 'ipv4-array':
                inner_template += (
                    '\n'
                    '    out->${field_name_underscore} =_mbim_message_read_ipv4_array (self, out->${array_size_field_name_underscore}, offset);\n'
                    '    offset += 4;\n')
            elif field['format'] == 'ipv6':
                inner_template += (
                    '\n'
                    '    memcpy (&(out->${field_name_underscore}), _mbim_message_read_ipv6 (self, offset, FALSE), 16);\n'
                    '    offset += 4;\n')
            elif field['format'] == 'ref-ipv6':
                inner_template += (
                    '\n'
                    '    memcpy (&(out->${field_name_underscore}), _mbim_message_read_ipv6 (self, offset, TRUE), 16);\n'
                    '    offset += 4;\n')
            elif field['format'] == 'ipv6-array':
                inner_template += (
                    '\n'
                    '    out->${field_name_underscore} =_mbim_message_read_ipv6_array (self, out->${array_size_field_name_underscore}, offset);\n'
                    '    offset += 4;\n')
            else:
                raise ValueError('Cannot handle format \'%s\' in struct' % field['format'])

            template += string.Template(inner_template).substitute(translations)

        template += (
            '\n'
            '    if (bytes_read)\n'
            '        *bytes_read = (offset - relative_offset);\n'
            '\n'
            '    return out;\n'
            '}\n')
        cfile.write(string.Template(template).substitute(translations))

        template = (
            '\n'
            'static ${name} **\n'
            '_mbim_message_read_${name_underscore}_struct_array (\n'
            '    const MbimMessage *self,\n'
            '    guint32 array_size,\n'
            '    guint32 relative_offset_array_start,\n'
            '    gboolean refs)\n'
            '{\n'
            '    ${name} **out;\n'
            '    guint32 i;\n'
            '    guint32 offset;\n'
            '\n'
            '    if (!array_size)\n'
            '        return NULL;\n'
            '\n'
            '    out = g_new (${name} *, array_size + 1);\n'
            '\n'
            '    if (!refs) {\n'
            '        offset = _mbim_message_read_guint32 (self, relative_offset_array_start);\n'
            '        for (i = 0; i < array_size; i++, offset += ${struct_size})\n'
            '            out[i] = _mbim_message_read_${name_underscore}_struct (self, offset, NULL);\n'
            '    } else {\n'
            '        offset = relative_offset_array_start;\n'
            '        for (i = 0; i < array_size; i++, offset += 8)\n'
            '            out[i] = _mbim_message_read_${name_underscore}_struct (self, _mbim_message_read_guint32 (self, offset), NULL);\n'
            '    }\n'
            '    out[array_size] = NULL;\n'
            '\n'
            '    return out;\n'
            '}\n')
        cfile.write(string.Template(template).substitute(translations))


    """
    Emit the type's append methods
    """
    def _emit_append(self, cfile):
        translations = { 'name'            : self.name,
                         'name_underscore' : utils.build_underscore_name_from_camelcase(self.name),
                         'struct_size'     : self.size }

        template = (
            '\n'
            'static GByteArray *\n'
            '_${name_underscore}_struct_new (const ${name} *value)\n'
            '{\n'
            '    MbimStructBuilder *builder;\n'
            '\n'
            '    g_assert (value != NULL);\n'
            '\n'
            '    builder = _mbim_struct_builder_new ();\n')

        for field in self.contents:
            translations['field'] = utils.build_underscore_name_from_camelcase(field['name'])
            translations['array_size'] = field['array-size'] if 'array-size' in field else ''
            translations['array_size_field'] = utils.build_underscore_name_from_camelcase(field['array-size-field']) if 'array-size-field' in field else ''
            translations['pad_array'] = field['pad-array'] if 'pad-array' in field else 'TRUE'

            if field['format'] == 'uuid':
                inner_template = ('    _mbim_struct_builder_append_uuid (builder, &(value->${field}));\n')
            elif field['format'] == 'byte-array':
                inner_template = ('    _mbim_struct_builder_append_byte_array (builder, FALSE, FALSE, ${pad_array}, value->${field}, ${array_size});\n')
            elif field['format'] == 'unsized-byte-array':
                inner_template = ('    _mbim_struct_builder_append_byte_array (builder, FALSE, FALSE, ${pad_array}, value->${field}, value->${field}_size);\n')
            elif field['format'] == 'ref-byte-array':
                if 'array-size-field' in field:
                    inner_template = ('    _mbim_struct_builder_append_byte_array (builder, TRUE, FALSE, ${pad_array}, value->${field}, value->${array_size_field});\n')
                else:
                    inner_template = ('    _mbim_struct_builder_append_byte_array (builder, TRUE, TRUE, ${pad_array}, value->${field}, value->${field}_size);\n')
            elif field['format'] == 'guint32':
                inner_template = ('    _mbim_struct_builder_append_guint32 (builder, value->${field});\n')
            elif field['format'] == 'guint32-array':
                inner_template = ('    _mbim_struct_builder_append_guint32_array (builder, value->${field}, value->${array_size_field});\n')
            elif field['format'] == 'guint64':
                inner_template = ('    _mbim_struct_builder_append_guint64 (builder, value->${field});\n')
            elif field['format'] == 'guint64-array':
                inner_template = ('    _mbim_struct_builder_append_guint64_array (builder, value->${field}, value->${array_size_field});\n')
            elif field['format'] == 'string':
                inner_template = ('    _mbim_struct_builder_append_string (builder, value->${field});\n')
            elif field['format'] == 'string-array':
                inner_template = ('    _mbim_struct_builder_append_string_array (builder, value->${field}, value->${array_size_field});\n')
            elif field['format'] == 'ipv4':
                inner_template = ('    _mbim_struct_builder_append_ipv4 (builder, &value->${field}, FALSE);\n')
            elif field['format'] == 'ref-ipv4':
                inner_template = ('    _mbim_struct_builder_append_ipv4 (builder, &value->${field}, TRUE);\n')
            elif field['format'] == 'ipv4-array':
                inner_template = ('    _mbim_struct_builder_append_ipv4_array (builder, value->${field}, value->${array_size_field});\n')
            elif field['format'] == 'ipv6':
                inner_template = ('    _mbim_struct_builder_append_ipv6 (builder, &value->${field}, FALSE);\n')
            elif field['format'] == 'ref-ipv6':
                inner_template = ('    _mbim_struct_builder_append_ipv6 (builder, &value->${field}, TRUE);\n')
            elif field['format'] == 'ipv6-array':
                inner_template = ('    _mbim_struct_builder_append_ipv6_array (builder, value->${field}, value->${array_size_field});\n')
            else:
                raise ValueError('Cannot handle format \'%s\' in struct' % field['format'])

            template += string.Template(inner_template).substitute(translations)

        template += (
            '\n'
            '    return _mbim_struct_builder_complete (builder);\n'
            '}\n')
        cfile.write(string.Template(template).substitute(translations))

        template = (
            '\n'
            'static void\n'
            '_mbim_struct_builder_append_${name_underscore}_struct (\n'
            '    MbimStructBuilder *builder,\n'
            '    const ${name} *value)\n'
            '{\n'
            '    GByteArray *raw;\n'
            '\n'
            '    raw = _${name_underscore}_struct_new (value);\n'
            '    g_byte_array_append (builder->fixed_buffer, raw->data, raw->len);\n'
            '    g_byte_array_unref (raw);\n'
            '}\n'
            '\n'
            'static void\n'
            '_mbim_message_command_builder_append_${name_underscore}_struct (\n'
            '    MbimMessageCommandBuilder *builder,\n'
            '    const ${name} *value)\n'
            '{\n'
            '    _mbim_struct_builder_append_${name_underscore}_struct (builder->contents_builder, value);\n'
            '}\n')
        cfile.write(string.Template(template).substitute(translations))

        template = (
            '\n'
            'static void\n'
            '_mbim_struct_builder_append_${name_underscore}_struct_array (\n'
            '    MbimStructBuilder *builder,\n'
            '    const ${name} *const *values,\n'
            '    guint32 n_values,\n'
            '    gboolean refs)\n'
            '{\n'
            '    guint32 offset;\n'
            '    guint32 i;\n'
            '    GByteArray *raw_all = NULL;\n'
            '\n'
            '    if (!refs) {\n'
            '        for (i = 0; i < n_values; i++) {\n'
            '            GByteArray *raw;\n'
            '\n'
            '            raw = _${name_underscore}_struct_new (values[i]);\n'
            '            if (!raw_all)\n'
            '                raw_all = raw;\n'
            '            else {\n'
            '                g_byte_array_append (raw_all, raw->data, raw->len);\n'
            '                g_byte_array_unref (raw);\n'
            '            }\n'
            '        }\n'
            '\n'
            '        if (!raw_all) {\n'
            '            offset = 0;\n'
            '            g_byte_array_append (builder->fixed_buffer, (guint8 *)&offset, sizeof (offset));\n'
            '        } else {\n'
            '            guint32 offset_offset;\n'
            '\n'
            '            /* Offset of the offset */\n'
            '            offset_offset = builder->fixed_buffer->len;\n'
            '            /* Length *not* in LE yet */\n'
            '            offset = builder->variable_buffer->len;\n'
            '            /* Add the offset value */\n'
            '            g_byte_array_append (builder->fixed_buffer, (guint8 *)&offset, sizeof (offset));\n'
            '            /* Configure the value to get updated */\n'
            '            g_array_append_val (builder->offsets, offset_offset);\n'
            '            /* Add the final array itself */\n'
            '            g_byte_array_append (builder->variable_buffer, raw_all->data, raw_all->len);\n'
            '            g_byte_array_unref (raw_all);\n'
            '        }\n'
            '    } else {\n'
            '        for (i = 0; i < n_values; i++) {\n'
            '            guint32 length;\n'
            '            guint32 offset_offset;\n'
            '            GByteArray *raw;\n'
            '\n'
            '            raw = _${name_underscore}_struct_new (values[i]);\n'
            '            g_assert (raw->len > 0);\n'
            '\n'
            '            /* Offset of the offset */\n'
            '            offset_offset = builder->fixed_buffer->len;\n'
            '\n'
            '            /* Length *not* in LE yet */\n'
            '            offset = builder->variable_buffer->len;\n'
            '            /* Add the offset value */\n'
            '            g_byte_array_append (builder->fixed_buffer, (guint8 *)&offset, sizeof (offset));\n'
            '            /* Configure the value to get updated */\n'
            '            g_array_append_val (builder->offsets, offset_offset);\n'
            '\n'
            '            /* Add the length value */\n'
            '            length = GUINT32_TO_LE (raw->len);\n'
            '            g_byte_array_append (builder->fixed_buffer, (guint8 *)&length, sizeof (length));\n'
            '\n'
            '            /* And finally, the bytearray itself to the variable buffer */\n'
            '            g_byte_array_append (builder->variable_buffer, (const guint8 *)raw->data, (guint)raw->len);\n'
            '            g_byte_array_unref (raw);\n'
            '        }\n'
            '    }\n'
            '}\n')
        cfile.write(string.Template(template).substitute(translations))

        template = (
            '\n'
            'static void\n'
            '_mbim_message_command_builder_append_${name_underscore}_struct_array (\n'
            '    MbimMessageCommandBuilder *builder,\n'
            '    const ${name} *const *values,\n'
            '    guint32 n_values,\n'
            '    gboolean refs)\n'
            '{\n'
            '    _mbim_struct_builder_append_${name_underscore}_struct_array (builder->contents_builder, values, n_values, refs);\n'
            '}\n')
        cfile.write(string.Template(template).substitute(translations))


    """
    Emit the struct handling implementation
    """
    def emit(self, hfile, cfile):
        utils.add_separator(hfile, 'Struct', self.name);
        utils.add_separator(cfile, 'Struct', self.name);

        # Emit type
        self._emit_type(hfile)
        # Emit type's free
        self._emit_free(hfile, cfile)
        # Emit type's read
        self._emit_read(cfile)
        # Emit type's append
        self._emit_append(cfile)


    """
    Emit the section contents
    """
    def emit_section_content(self, sfile):
        translations = { 'struct_name'     : self.name,
                         'name_underscore' : utils.build_underscore_name_from_camelcase(self.name) }
        template = (
            '<SUBSECTION ${struct_name}>\n'
            '${struct_name}\n')
        if self.single_member == True:
            template += (
                '${name_underscore}_free\n')
        if self.array_member == True:
            template += (
                '${name_underscore}_array_free\n')
        sfile.write(string.Template(template).substitute(translations))
