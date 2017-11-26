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
# Copyright (C) 2013 - 2014 Aleksander Morgado <aleksander@aleksander.es>
#

import string
import utils


"""
Flag the values which always need to be read
"""
def flag_always_read_field(fields, field_name):
    for field in fields:
        if field['name'] == field_name:
            if field['format'] != 'guint32':
                    raise ValueError('Fields to always read \'%s\' must be a guint32' % field_name)
            field['always-read'] = True
            return
    raise ValueError('Couldn\'t find field to always read \'%s\'' % field_name)


"""
Validate fields in the dictionary
"""
def validate_fields(fields):
    for field in fields:
        # Look for condition fields, which need to be always read
        if 'available-if' in field:
            condition = field['available-if']
            flag_always_read_field(fields, condition['field'])

        # Look for array size fields, which need to be always read
        if field['format'] == 'byte-array':
            pass
        elif field['format'] == 'ref-byte-array':
            pass
        elif field['format'] == 'ref-byte-array-no-offset':
            pass
        elif field['format'] == 'unsized-byte-array':
            pass
        elif field['format'] == 'uuid':
            pass
        elif field['format'] == 'guint32':
            pass
        elif field['format'] == 'guint32-array':
            flag_always_read_field(fields, field['array-size-field'])
        elif field['format'] == 'guint64':
            pass
        elif field['format'] == 'guint64-array':
            flag_always_read_field(fields, field['array-size-field'])
        elif field['format'] == 'string':
            pass
        elif field['format'] == 'string-array':
            flag_always_read_field(fields, field['array-size-field'])
        elif field['format'] == 'struct':
            if 'struct-type' not in field:
                raise ValueError('Field type \'struct\' requires \'struct-type\' field')
        elif field['format'] == 'struct-array':
            flag_always_read_field(fields, field['array-size-field'])
            if 'struct-type' not in field:
                raise ValueError('Field type \'struct\' requires \'struct-type\' field')
        elif field['format'] == 'ref-struct-array':
            flag_always_read_field(fields, field['array-size-field'])
            if 'struct-type' not in field:
                raise ValueError('Field type \'struct\' requires \'struct-type\' field')
        elif field['format'] == 'ipv4':
            pass
        elif field['format'] == 'ref-ipv4':
            pass
        elif field['format'] == 'ipv4-array':
            flag_always_read_field(fields, field['array-size-field'])
        elif field['format'] == 'ipv6':
            pass
        elif field['format'] == 'ref-ipv6':
            pass
        elif field['format'] == 'ipv6-array':
            flag_always_read_field(fields, field['array-size-field'])
        else:
            raise ValueError('Cannot handle field type \'%s\'' % field['format'])


"""
The Message class takes care of all message handling
"""
class Message:

    """
    Constructor
    """
    def __init__(self, dictionary):
        # The message service, e.g. "Basic Connect"
        self.service = dictionary['service']

        # The name of the specific message, e.g. "Something"
        self.name = dictionary['name']

        # Query
        if 'query' in dictionary:
            self.has_query = True
            self.query = dictionary['query']
            validate_fields(self.query)
        else:
            self.has_query = False
            self.query = []

        # Set
        if 'set' in dictionary:
            self.has_set = True
            self.set = dictionary['set']
            validate_fields(self.set)
        else:
            self.has_set = False
            self.set = []


        # Response
        if 'response' in dictionary:
            self.has_response = True
            self.response = dictionary['response']
            validate_fields(self.response)
        else:
            self.has_response = False
            self.response = []

        # Notification
        if 'notification' in dictionary:
            self.has_notification = True
            self.notification = dictionary['notification']
            validate_fields(self.notification)
        else:
            self.has_notification = False
            self.notification = []

        # Build Fullname
        if self.service == 'Basic Connect':
            self.fullname = 'MBIM Message ' + self.name
        elif self.name == "":
            self.fullname = 'MBIM Message ' + self.service
        else:
            self.fullname = 'MBIM Message ' + self.service + ' ' + self.name

        # Build CID enum
        self.cid_enum_name = 'MBIM CID ' + self.service
        if self.name != "":
            self.cid_enum_name += (' ' + self.name)
        self.cid_enum_name = utils.build_underscore_name(self.cid_enum_name).upper()


    """
    Emit the message handling implementation
    """
    def emit(self, hfile, cfile):
        if self.has_query:
            utils.add_separator(hfile, 'Message (Query)', self.fullname);
            utils.add_separator(cfile, 'Message (Query)', self.fullname);
            self._emit_message_creator(hfile, cfile, 'query', self.query)

        if self.has_set:
            utils.add_separator(hfile, 'Message (Set)', self.fullname);
            utils.add_separator(cfile, 'Message (Set)', self.fullname);
            self._emit_message_creator(hfile, cfile, 'set', self.set)

        if self.has_response:
            utils.add_separator(hfile, 'Message (Response)', self.fullname);
            utils.add_separator(cfile, 'Message (Response)', self.fullname);
            self._emit_message_parser(hfile, cfile, 'response', self.response)

        if self.has_notification:
            utils.add_separator(hfile, 'Message (Notification)', self.fullname);
            utils.add_separator(cfile, 'Message (Notification)', self.fullname);
            self._emit_message_parser(hfile, cfile, 'notification', self.notification)


    """
    Emit message creator
    """
    def _emit_message_creator(self, hfile, cfile, message_type, fields):
        translations = { 'message'                  : self.name,
                         'service'                  : self.service,
                         'underscore'               : utils.build_underscore_name (self.fullname),
                         'message_type'             : message_type,
                         'message_type_upper'       : message_type.upper(),
                         'service_underscore_upper' : utils.build_underscore_name (self.service).upper(),
                         'cid_enum_name'            : self.cid_enum_name }
        template = (
            '\n'
            'MbimMessage *${underscore}_${message_type}_new (\n')

        for field in fields:
            translations['field'] = utils.build_underscore_name_from_camelcase (field['name'])
            translations['struct'] = field['struct-type'] if 'struct-type' in field else ''
            translations['public'] = field['public-format'] if 'public-format' in field else field['format']

            if field['format'] == 'byte-array':
                inner_template = ('    const guint8 *${field},\n')
            elif field['format'] == 'unsized-byte-array' or \
                 field['format'] == 'ref-byte-array' or \
                 field['format'] == 'ref-byte-array-no-offset':
                inner_template = ('    const guint32 ${field}_size,\n'
                                  '    const guint8 *${field},\n')
            elif field['format'] == 'uuid':
                inner_template = ('    const MbimUuid *${field},\n')
            elif field['format'] == 'guint32':
                inner_template = ('    ${public} ${field},\n')
            elif field['format'] == 'guint32-array':
                inner_template = ('    const ${public} *${field},\n')
            elif field['format'] == 'guint64':
                inner_template = ('    ${public} ${field},\n')
            elif field['format'] == 'guint64-array':
                inner_template = ('    const ${public} *${field},\n')
            elif field['format'] == 'string':
                inner_template = ('    const gchar *${field},\n')
            elif field['format'] == 'string-array':
                inner_template = ('    const gchar *const *${field},\n')
            elif field['format'] == 'struct':
                inner_template = ('    const ${struct} *${field},\n')
            elif field['format'] == 'struct-array':
                inner_template = ('    const ${struct} *const *${field},\n')
            elif field['format'] == 'ref-struct-array':
                inner_template = ('    const ${struct} *const *${field},\n')
            elif field['format'] == 'ipv4':
                inner_template = ('    const MbimIPv4 *${field},\n')
            elif field['format'] == 'ref-ipv4':
                inner_template = ('    const MbimIPv4 *${field},\n')
            elif field['format'] == 'ipv4-array':
                inner_template = ('    const MbimIPv4 *${field},\n')
            elif field['format'] == 'ipv6':
                inner_template = ('    const MbimIPv6 *${field},\n')
            elif field['format'] == 'ref-ipv6':
                inner_template = ('    const MbimIPv6 *${field},\n')
            elif field['format'] == 'ipv6-array':
                inner_template = ('    const MbimIPv6 *${field},\n')

            template += (string.Template(inner_template).substitute(translations))

        template += (
            '    GError **error);\n')
        hfile.write(string.Template(template).substitute(translations))

        template = (
            '\n'
            '/**\n'
            ' * ${underscore}_${message_type}_new:\n')

        for field in fields:
            translations['name'] = field['name']
            translations['field'] = utils.build_underscore_name_from_camelcase (field['name'])
            translations['struct'] = field['struct-type'] if 'struct-type' in field else ''
            translations['public'] = field['public-format'] if 'public-format' in field else field['format']
            translations['array_size'] = field['array-size'] if 'array-size' in field else ''

            if field['format'] == 'byte-array':
                inner_template = (' * @${field}: the \'${name}\' field, given as an array of ${array_size} #guint8 values.\n')
            elif field['format'] == 'unsized-byte-array' or \
                 field['format'] == 'ref-byte-array' or \
                 field['format'] == 'ref-byte-array-no-offset':
                inner_template = (' * @${field}_size: size of the ${field} array.\n'
                                  ' * @${field}: the \'${name}\' field, given as an array of #guint8 values.\n')
            elif field['format'] == 'uuid':
                inner_template = (' * @${field}: the \'${name}\' field, given as a #MbimUuid.\n')
            elif field['format'] == 'guint32':
                inner_template = (' * @${field}: the \'${name}\' field, given as a #${public}.\n')
            elif field['format'] == 'guint32-array':
                inner_template = (' * @${field}: the \'${name}\' field, given as an array of #${public}.\n')
            elif field['format'] == 'guint64':
                inner_template = (' * @${field}: the \'${name}\' field, given as a #${public}.\n')
            elif field['format'] == 'guint64-array':
                inner_template = (' * @${field}: the \'${name}\' field, given as an array of #${public}.\n')
            elif field['format'] == 'string':
                inner_template = (' * @${field}: the \'${name}\' field, given as a string.\n')
            elif field['format'] == 'string-array':
                inner_template = (' * @${field}: the \'${name}\' field, given as an array of strings.\n')
            elif field['format'] == 'struct':
                inner_template = (' * @${field}: the \'${name}\' field, given as a #${struct}.\n')
            elif field['format'] == 'struct-array':
                inner_template = (' * @${field}: the \'${name}\' field, given as an array of #${struct}s.\n')
            elif field['format'] == 'ref-struct-array':
                inner_template = (' * @${field}: the \'${name}\' field, given as an array of #${struct}s.\n')
            elif field['format'] == 'ipv4':
                inner_template = (' * @${field}: the \'${name}\' field, given as a #MbimIPv4.\n')
            elif field['format'] == 'ref-ipv4':
                inner_template = (' * @${field}: the \'${name}\' field, given as a #MbimIPv4.\n')
            elif field['format'] == 'ipv4-array':
                inner_template = (' * @${field}: the \'${name}\' field, given as an array of #MbimIPv4.\n')
            elif field['format'] == 'ipv6':
                inner_template = (' * @${field}: the \'${name}\' field, given as a #MbimIPv6.\n')
            elif field['format'] == 'ref-ipv6':
                inner_template = (' * @${field}: the \'${name}\' field, given as a #MbimIPv6.\n')
            elif field['format'] == 'ipv6-array':
                inner_template = (' * @${field}: the \'${name}\' field, given as an array of #MbimIPv6.\n')

            template += (string.Template(inner_template).substitute(translations))

        template += (
            ' * @error: return location for error or %NULL.\n'
            ' *\n'
            ' * Create a new request for the \'${message}\' ${message_type} command in the \'${service}\' service.\n'
            ' *\n'
            ' * Returns: a newly allocated #MbimMessage, which should be freed with mbim_message_unref().\n'
            ' */\n'
            'MbimMessage *\n'
            '${underscore}_${message_type}_new (\n')

        for field in fields:
            translations['field'] = utils.build_underscore_name_from_camelcase (field['name'])
            translations['struct'] = field['struct-type'] if 'struct-type' in field else ''
            translations['public'] = field['public-format'] if 'public-format' in field else field['format']
            translations['array_size'] = field['array-size'] if 'array-size' in field else ''

            if field['format'] == 'byte-array':
                inner_template = ('    const guint8 *${field},\n')
            elif field['format'] == 'unsized-byte-array' or \
                 field['format'] == 'ref-byte-array' or \
                 field['format'] == 'ref-byte-array-no-offset':
                inner_template = ('    const guint32 ${field}_size,\n'
                                  '    const guint8 *${field},\n')
            elif field['format'] == 'uuid':
                inner_template = ('    const MbimUuid *${field},\n')
            elif field['format'] == 'guint32':
                inner_template = ('    ${public} ${field},\n')
            elif field['format'] == 'guint32-array':
                inner_template = ('    const ${public} *${field},\n')
            elif field['format'] == 'guint64':
                inner_template = ('    ${public} ${field},\n')
            elif field['format'] == 'guint64-array':
                inner_template = ('    const ${public} *${field},\n')
            elif field['format'] == 'string':
                inner_template = ('    const gchar *${field},\n')
            elif field['format'] == 'string-array':
                inner_template = ('    const gchar *const *${field},\n')
            elif field['format'] == 'struct':
                inner_template = ('    const ${struct} *${field},\n')
            elif field['format'] == 'struct-array':
                inner_template = ('    const ${struct} *const *${field},\n')
            elif field['format'] == 'ref-struct-array':
                inner_template = ('    const ${struct} *const *${field},\n')
            elif field['format'] == 'ipv4':
                inner_template = ('    const MbimIPv4 *${field},\n')
            elif field['format'] == 'ref-ipv4':
                inner_template = ('    const MbimIPv4 *${field},\n')
            elif field['format'] == 'ipv4-array':
                inner_template = ('    const MbimIPv4 *${field},\n')
            elif field['format'] == 'ipv6':
                inner_template = ('    const MbimIPv6 *${field},\n')
            elif field['format'] == 'ref-ipv6':
                inner_template = ('    const MbimIPv6 *${field},\n')
            elif field['format'] == 'ipv6-array':
                inner_template = ('    const MbimIPv6 *${field},\n')

            template += (string.Template(inner_template).substitute(translations))

        template += (
            '    GError **error)\n'
            '{\n'
            '    MbimMessageCommandBuilder *builder;\n'
            '\n'
            '    builder = _mbim_message_command_builder_new (0,\n'
            '                                                 MBIM_SERVICE_${service_underscore_upper},\n'
            '                                                 ${cid_enum_name},\n'
            '                                                 MBIM_MESSAGE_COMMAND_TYPE_${message_type_upper});\n')

        for field in fields:
            translations['field'] = utils.build_underscore_name_from_camelcase(field['name'])
            translations['array_size_field'] = utils.build_underscore_name_from_camelcase(field['array-size-field']) if 'array-size-field' in field else ''
            translations['struct'] = field['struct-type'] if 'struct-type' in field else ''
            translations['struct_underscore'] = utils.build_underscore_name_from_camelcase (translations['struct'])
            translations['array_size'] = field['array-size'] if 'array-size' in field else ''
            translations['pad_array'] = field['pad-array'] if 'pad-array' in field else 'TRUE'

            inner_template = ''
            if 'available-if' in field:
                condition = field['available-if']
                translations['condition_field'] = utils.build_underscore_name_from_camelcase(condition['field'])
                translations['condition_operation'] = condition['operation']
                translations['condition_value'] = condition['value']
                inner_template += (
                    '    if (${condition_field} ${condition_operation} ${condition_value}) {\n')
            else:
                inner_template += ('    {\n')

            if field['format'] == 'byte-array':
                inner_template += ('        _mbim_message_command_builder_append_byte_array (builder, FALSE, FALSE, ${pad_array}, ${field}, ${array_size});\n')
            elif field['format'] == 'unsized-byte-array':
                inner_template += ('        _mbim_message_command_builder_append_byte_array (builder, FALSE, FALSE, ${pad_array}, ${field}, ${field}_size);\n')
            elif field['format'] == 'ref-byte-array':
                inner_template += ('        _mbim_message_command_builder_append_byte_array (builder, TRUE, TRUE, ${pad_array}, ${field}, ${field}_size);\n')
            elif field['format'] == 'ref-byte-array-no-offset':
                inner_template += ('        _mbim_message_command_builder_append_byte_array (builder, FALSE, TRUE, ${pad_array}, ${field}, ${field}_size);\n')
            elif field['format'] == 'uuid':
                inner_template += ('        _mbim_message_command_builder_append_uuid (builder, ${field});\n')
            elif field['format'] == 'guint32':
                inner_template += ('        _mbim_message_command_builder_append_guint32 (builder, ${field});\n')
            elif field['format'] == 'guint32-array':
                inner_template += ('        _mbim_message_command_builder_append_guint32_array (builder, ${field}, ${array_size_field});\n')
            elif field['format'] == 'guint64':
                inner_template += ('        _mbim_message_command_builder_append_guint64 (builder, ${field});\n')
            elif field['format'] == 'string':
                inner_template += ('        _mbim_message_command_builder_append_string (builder, ${field});\n')
            elif field['format'] == 'string-array':
                inner_template += ('        _mbim_message_command_builder_append_string_array (builder, ${field}, ${array_size_field});\n')
            elif field['format'] == 'struct':
                inner_template += ('        _mbim_message_command_builder_append_${struct_underscore}_struct (builder, ${field});\n')
            elif field['format'] == 'struct-array':
                inner_template += ('        _mbim_message_command_builder_append_${struct_underscore}_struct_array (builder, ${field}, ${array_size_field}, FALSE);\n')
            elif field['format'] == 'ref-struct-array':
                inner_template += ('        _mbim_message_command_builder_append_${struct_underscore}_struct_array (builder, ${field}, ${array_size_field}, TRUE);\n')
            elif field['format'] == 'ipv4':
                inner_template += ('        _mbim_message_command_builder_append_ipv4 (builder, ${field}, FALSE);\n')
            elif field['format'] == 'ref-ipv4':
                inner_template += ('        _mbim_message_command_builder_append_ipv4 (builder, ${field}, TRUE);\n')
            elif field['format'] == 'ipv4-array':
                inner_template += ('        _mbim_message_command_builder_append_ipv4_array (builder, ${field}, ${array_size_field});\n')
            elif field['format'] == 'ipv6':
                inner_template += ('        _mbim_message_command_builder_append_ipv6 (builder, ${field}, FALSE);\n')
            elif field['format'] == 'ref-ipv6':
                inner_template += ('        _mbim_message_command_builder_append_ipv6 (builder, ${field}, TRUE);\n')
            elif field['format'] == 'ipv6-array':
                inner_template += ('        _mbim_message_command_builder_append_ipv6_array (builder, ${field}, ${array_size_field});\n')
            else:
                raise ValueError('Cannot handle field type \'%s\'' % field['format'])

            inner_template += ('    }\n')

            template += (string.Template(inner_template).substitute(translations))

        template += (
            '\n'
            '    return _mbim_message_command_builder_complete (builder);\n'
            '}\n')
        cfile.write(string.Template(template).substitute(translations))


    """
    Emit message parser
    """
    def _emit_message_parser(self, hfile, cfile, message_type, fields):
        translations = { 'message'                  : self.name,
                         'service'                  : self.service,
                         'underscore'               : utils.build_underscore_name (self.fullname),
                         'message_type'             : message_type,
                         'message_type_upper'       : message_type.upper(),
                         'service_underscore_upper' : utils.build_underscore_name (self.service).upper() }
        template = (
            '\n'
            'gboolean ${underscore}_${message_type}_parse (\n'
            '    const MbimMessage *message,\n')

        for field in fields:
            translations['field'] = utils.build_underscore_name_from_camelcase(field['name'])
            translations['public'] = field['public-format'] if 'public-format' in field else field['format']
            translations['struct'] = field['struct-type'] if 'struct-type' in field else ''

            if field['format'] == 'byte-array':
                inner_template = ('    const guint8 **${field},\n')
            elif field['format'] == 'unsized-byte-array' or field['format'] == 'ref-byte-array':
                inner_template = ('    guint32 *${field}_size,\n'
                                  '    const guint8 **${field},\n')
            elif field['format'] == 'uuid':
                inner_template = ('    const MbimUuid **${field},\n')
            elif field['format'] == 'guint32':
                inner_template = ('    ${public} *${field},\n')
            elif field['format'] == 'guint32-array':
                inner_template = ('    ${public} **${field},\n')
            elif field['format'] == 'guint64':
                inner_template = ('    ${public} *${field},\n')
            elif field['format'] == 'guint64-array':
                inner_template = ('    ${public} **${field},\n')
            elif field['format'] == 'string':
                inner_template = ('    gchar **${field},\n')
            elif field['format'] == 'string-array':
                inner_template = ('    gchar ***${field},\n')
            elif field['format'] == 'struct':
                inner_template = ('    ${struct} **${field},\n')
            elif field['format'] == 'struct-array':
                inner_template = ('    ${struct} ***${field},\n')
            elif field['format'] == 'ref-struct-array':
                inner_template = ('    ${struct} ***${field},\n')
            elif field['format'] == 'ipv4':
                inner_template = ('    const MbimIPv4 **${field},\n')
            elif field['format'] == 'ref-ipv4':
                inner_template = ('    const MbimIPv4 **${field},\n')
            elif field['format'] == 'ipv4-array':
                inner_template = ('    MbimIPv4 **${field},\n')
            elif field['format'] == 'ipv6':
                inner_template = ('    const MbimIPv6 **${field},\n')
            elif field['format'] == 'ref-ipv6':
                inner_template = ('    const MbimIPv6 **${field},\n')
            elif field['format'] == 'ipv6-array':
                inner_template = ('    MbimIPv6 **${field},\n')
            else:
                raise ValueError('Cannot handle field type \'%s\'' % field['format'])

            template += (string.Template(inner_template).substitute(translations))

        template += (
            '    GError **error);\n')
        hfile.write(string.Template(template).substitute(translations))

        template = (
            '\n'
            '/**\n'
            ' * ${underscore}_${message_type}_parse:\n'
            ' * @message: the #MbimMessage.\n')

        for field in fields:
            translations['field'] = utils.build_underscore_name_from_camelcase(field['name'])
            translations['name'] = field['name']
            translations['public'] = field['public-format'] if 'public-format' in field else field['format']
            translations['struct'] = field['struct-type'] if 'struct-type' in field else ''
            translations['struct_underscore'] = utils.build_underscore_name_from_camelcase (translations['struct'])
            translations['array_size'] = field['array-size'] if 'array-size' in field else ''

            if field['format'] == 'byte-array':
                inner_template = (' * @${field}: return location for an array of ${array_size} #guint8 values. Do not free the returned value, it is owned by @message.\n')
            elif field['format'] == 'unsized-byte-array' or field['format'] == 'ref-byte-array':
                inner_template = (' * @${field}_size: return location for the size of the ${field} array.\n'
                                  ' * @${field}: return location for an array of #guint8 values. Do not free the returned value, it is owned by @message.\n')
            elif field['format'] == 'uuid':
                inner_template = (' * @${field}: return location for a #MbimUuid, or %NULL if the \'${name}\' field is not needed. Do not free the returned value, it is owned by @message.\n')
            elif field['format'] == 'guint32':
                inner_template = (' * @${field}: return location for a #${public}, or %NULL if the \'${name}\' field is not needed.\n')
            elif field['format'] == 'guint32-array':
                inner_template = (' * @${field}: return location for a newly allocated array of #${public}s, or %NULL if the \'${name}\' field is not needed. Free the returned value with g_free().\n')
            elif field['format'] == 'guint64':
                inner_template = (' * @${field}: return location for a #guint64, or %NULL if the \'${name}\' field is not needed.\n')
            elif field['format'] == 'guint64-array':
                inner_template = (' * @${field}: return location for a newly allocated array of #guint64s, or %NULL if the \'${name}\' field is not needed. Free the returned value with g_free().\n')
            elif field['format'] == 'string':
                inner_template = (' * @${field}: return location for a newly allocated string, or %NULL if the \'${name}\' field is not needed. Free the returned value with g_free().\n')
            elif field['format'] == 'string-array':
                inner_template = (' * @${field}: return location for a newly allocated array of strings, or %NULL if the \'${name}\' field is not needed. Free the returned value with g_strfreev().\n')
            elif field['format'] == 'struct':
                inner_template = (' * @${field}: return location for a newly allocated #${struct}, or %NULL if the \'${name}\' field is not needed. Free the returned value with ${struct_underscore}_free().\n')
            elif field['format'] == 'struct-array':
                inner_template = (' * @${field}: return location for a newly allocated array of #${struct}s, or %NULL if the \'${name}\' field is not needed. Free the returned value with ${struct_underscore}_array_free().\n')
            elif field['format'] == 'ref-struct-array':
                inner_template = (' * @${field}: return location for a newly allocated array of #${struct}s, or %NULL if the \'${name}\' field is not needed. Free the returned value with ${struct_underscore}_array_free().\n')
            elif field['format'] == 'ipv4':
                inner_template = (' * @${field}: return location for a #MbimIPv4, or %NULL if the \'${name}\' field is not needed. Do not free the returned value, it is owned by @message.\n')
            elif field['format'] == 'ref-ipv4':
                inner_template = (' * @${field}: return location for a #MbimIPv4, or %NULL if the \'${name}\' field is not needed. Do not free the returned value, it is owned by @message.\n')
            elif field['format'] == 'ipv4-array':
                inner_template = (' * @${field}: return location for a newly allocated array of #MbimIPv4s, or %NULL if the \'${name}\' field is not needed. Free the returned value with g_free().\n')
            elif field['format'] == 'ipv6':
                inner_template = (' * @${field}: return location for a #MbimIPv6, or %NULL if the \'${name}\' field is not needed. Do not free the returned value, it is owned by @message.\n')
            elif field['format'] == 'ref-ipv6':
                inner_template = (' * @${field}: return location for a #MbimIPv6, or %NULL if the \'${name}\' field is not needed. Do not free the returned value, it is owned by @message.\n')
            elif field['format'] == 'ipv6-array':
                inner_template = (' * @${field}: return location for a newly allocated array of #MbimIPv6s, or %NULL if the \'${name}\' field is not needed. Free the returned value with g_free().\n')

            template += (string.Template(inner_template).substitute(translations))

        template += (
            ' * @error: return location for error or %NULL.\n'
            ' *\n'
            ' * Parses and returns parameters of the \'${message}\' ${message_type} command in the \'${service}\' service.\n'
            ' *\n'
            ' * Returns: %TRUE if the message was correctly parsed, %FALSE if @error is set.\n'
            ' */\n'
            'gboolean\n'
            '${underscore}_${message_type}_parse (\n'
            '    const MbimMessage *message,\n')

        for field in fields:
            translations['field'] = utils.build_underscore_name_from_camelcase(field['name'])
            translations['public'] = field['public-format'] if 'public-format' in field else field['format']
            translations['struct'] = field['struct-type'] if 'struct-type' in field else ''

            if field['format'] == 'byte-array':
                inner_template = ('    const guint8 **${field},\n')
            elif field['format'] == 'unsized-byte-array' or field['format'] == 'ref-byte-array':
                inner_template = ('    guint32 *${field}_size,\n'
                                  '    const guint8 **${field},\n')
            elif field['format'] == 'uuid':
                inner_template = ('    const MbimUuid **${field},\n')
            elif field['format'] == 'guint32':
                inner_template = ('    ${public} *${field},\n')
            elif field['format'] == 'guint32-array':
                inner_template = ('    ${public} **${field},\n')
            elif field['format'] == 'guint64':
                inner_template = ('    ${public} *${field},\n')
            elif field['format'] == 'guint64-array':
                inner_template = ('    ${public} **${field},\n')
            elif field['format'] == 'string':
                inner_template = ('    gchar **${field},\n')
            elif field['format'] == 'string-array':
                inner_template = ('    gchar ***${field},\n')
            elif field['format'] == 'struct':
                inner_template = ('    ${struct} **${field},\n')
            elif field['format'] == 'struct-array':
                inner_template = ('    ${struct} ***${field},\n')
            elif field['format'] == 'ref-struct-array':
                inner_template = ('    ${struct} ***${field},\n')
            elif field['format'] == 'ipv4':
                inner_template = ('    const MbimIPv4 **${field},\n')
            elif field['format'] == 'ref-ipv4':
                inner_template = ('    const MbimIPv4 **${field},\n')
            elif field['format'] == 'ipv4-array':
                inner_template = ('    MbimIPv4 **${field},\n')
            elif field['format'] == 'ipv6':
                inner_template = ('    const MbimIPv6 **${field},\n')
            elif field['format'] == 'ref-ipv6':
                inner_template = ('    const MbimIPv6 **${field},\n')
            elif field['format'] == 'ipv6-array':
                inner_template = ('    MbimIPv6 **${field},\n')

            template += (string.Template(inner_template).substitute(translations))

        template += (
            '    GError **error)\n'
            '{\n')

        if fields != []:
            template += (
                '    guint32 offset = 0;\n')

        for field in fields:
            if 'always-read' in field:
                translations['field'] = utils.build_underscore_name_from_camelcase(field['name'])
                inner_template = ('    guint32 _${field};\n')
                template += (string.Template(inner_template).substitute(translations))

        if message_type == 'response':
            template += (
                '\n'
                '    if (mbim_message_get_message_type (message) != MBIM_MESSAGE_TYPE_COMMAND_DONE) {\n'
                '        g_set_error (error,\n'
                '                     MBIM_CORE_ERROR,\n'
                '                     MBIM_CORE_ERROR_INVALID_MESSAGE,\n'
                '                     \"Message is not a response\");\n'
                '        return FALSE;\n'
                '    }\n')
        elif message_type == 'notification':
            template += (
                '\n'
                '    if (mbim_message_get_message_type (message) != MBIM_MESSAGE_TYPE_INDICATE_STATUS) {\n'
                '        g_set_error (error,\n'
                '                     MBIM_CORE_ERROR,\n'
                '                     MBIM_CORE_ERROR_INVALID_MESSAGE,\n'
                '                     \"Message is not a notification\");\n'
                '        return FALSE;\n'
                '    }\n')
        else:
            raise ValueError('Unexpected message type \'%s\'' % message_type)

        for field in fields:
            translations['field']                   = utils.build_underscore_name_from_camelcase(field['name'])
            translations['field_format_underscore'] = utils.build_underscore_name_from_camelcase(field['format'])
            translations['field_name']              = field['name']
            translations['array_size_field'] = utils.build_underscore_name_from_camelcase(field['array-size-field']) if 'array-size-field' in field else ''
            translations['struct_name'] = utils.build_underscore_name_from_camelcase(field['struct-type']) if 'struct-type' in field else ''
            translations['struct_type'] = field['struct-type'] if 'struct-type' in field else ''
            translations['array_size'] = field['array-size'] if 'array-size' in field else ''

            inner_template = (
                '\n'
                '    /* Read the \'${field_name}\' variable */\n')
            if 'available-if' in field:
                condition = field['available-if']
                translations['condition_field'] = utils.build_underscore_name_from_camelcase(condition['field'])
                translations['condition_operation'] = condition['operation']
                translations['condition_value'] = condition['value']
                inner_template += (
                    '    if (!(_${condition_field} ${condition_operation} ${condition_value})) {\n')
                if field['format'] == 'byte-array':
                    inner_template += (
                        '        if (${field})\n'
                        '            *${field} = NULL;\n')
                elif field['format'] == 'unsized-byte-array' or \
                   field['format'] == 'ref-byte-array':
                    inner_template += (
                        '        if (${field}_size)\n'
                        '            *${field}_size = 0;\n'
                        '        if (${field})\n'
                        '            *${field} = NULL;\n')
                elif field['format'] == 'guint32-array' or \
                     field['format'] == 'string' or \
                     field['format'] == 'string-array' or \
                     field['format'] == 'struct' or \
                     field['format'] == 'struct-array' or \
                     field['format'] == 'ref-struct-array' or \
                     field['format'] == 'ipv4' or \
                     field['format'] == 'ref-ipv4' or \
                     field['format'] == 'ipv4-array' or \
                     field['format'] == 'ipv6' or \
                     field['format'] == 'ref-ipv6' or \
                     field['format'] == 'ipv6-array':
                    inner_template += (
                        '        if (${field} != NULL)\n'
                        '            *${field} = NULL;\n')
                else:
                    raise ValueError('Field format \'%s\' unsupported as optional field' % field['format'])

                inner_template += (
                    '    } else {\n')
            else:
                inner_template += (
                    '    {\n')

            if 'always-read' in field:
                inner_template += (
                    '        _${field} = _mbim_message_read_guint32 (message, offset);\n'
                    '        if (${field} != NULL)\n'
                    '            *${field} = _${field};\n'
                    '        offset += 4;\n')
            elif field['format'] == 'byte-array':
                inner_template += (
                    '        const guint8 *tmp;\n'
                    '\n'
                    '        tmp = _mbim_message_read_byte_array (message, 0, offset, FALSE, FALSE, NULL);\n'
                    '        if (${field} != NULL)\n'
                    '            *${field} = tmp;\n'
                    '        offset += ${array_size};\n')
            elif field['format'] == 'unsized-byte-array':
                inner_template += (
                    '        const guint8 *tmp;\n'
                    '        guint32 tmpsize;\n'
                    '\n'
                    '        tmp = _mbim_message_read_byte_array (message, 0, offset, FALSE, FALSE, &tmpsize);\n'
                    '        if (${field} != NULL)\n'
                    '            *${field} = tmp;\n'
                    '        if (${field}_size != NULL)\n'
                    '            *${field}_size = tmpsize;\n'
                    '        offset += tmpsize;\n')
            elif field['format'] == 'ref-byte-array':
                inner_template += (
                    '        const guint8 *tmp;\n'
                    '        guint32 tmpsize;\n'
                    '\n'
                    '        tmp = _mbim_message_read_byte_array (message, 0, offset, TRUE, TRUE, &tmpsize);\n'
                    '        if (${field} != NULL)\n'
                    '            *${field} = tmp;\n'
                    '        if (${field}_size != NULL)\n'
                    '            *${field}_size = tmpsize;\n'
                    '        offset += 8;\n')
            elif field['format'] == 'uuid':
                inner_template += (
                    '        if (${field} != NULL)\n'
                    '            *${field} =  _mbim_message_read_uuid (message, offset);\n'
                    '        offset += 16;\n')
            elif field['format'] == 'guint32':
                inner_template += (
                    '        if (${field} != NULL)\n'
                    '            *${field} =  _mbim_message_read_guint32 (message, offset);\n'
                    '        offset += 4;\n')
            elif field['format'] == 'guint32-array':
                inner_template += (
                    '        if (${field} != NULL)\n'
                    '            *${field} = _mbim_message_read_guint32_array (message, _{array_size_field}, offset);\n'
                    '        offset += (4 * _${array_size_field});\n')
            elif field['format'] == 'guint64':
                inner_template += (
                    '        if (${field} != NULL)\n'
                    '            *${field} =  _mbim_message_read_guint64 (message, offset);\n'
                    '        offset += 8;\n')
            elif field['format'] == 'string':
                inner_template += (
                    '        if (${field} != NULL)\n'
                    '            *${field} = _mbim_message_read_string (message, 0, offset);\n'
                    '        offset += 8;\n')
            elif field['format'] == 'string-array':
                inner_template += (
                    '        if (${field} != NULL)\n'
                    '            *${field} = _mbim_message_read_string_array (message, _${array_size_field}, 0, offset);\n'
                    '        offset += (8 * _${array_size_field});\n')
            elif field['format'] == 'struct':
                inner_template += (
                    '        ${struct_type} *tmp;\n'
                    '        guint32 bytes_read = 0;\n'
                    '\n'
                    '        tmp = _mbim_message_read_${struct_name}_struct (message, offset, &bytes_read);\n'
                    '        if (${field} != NULL)\n'
                    '            *${field} = tmp;\n'
                    '        else\n'
                    '             _${struct_name}_free (tmp);\n'
                    '        offset += bytes_read;\n')
            elif field['format'] == 'struct-array':
                inner_template += (
                    '        if (${field} != NULL)\n'
                    '            *${field} = _mbim_message_read_${struct_name}_struct_array (message, _${array_size_field}, offset, FALSE);\n'
                    '        offset += 4;\n')
            elif field['format'] == 'ref-struct-array':
                inner_template += (
                    '        if (${field} != NULL)\n'
                    '            *${field} = _mbim_message_read_${struct_name}_struct_array (message, _${array_size_field}, offset, TRUE);\n'
                    '        offset += (8 * _${array_size_field});\n')
            elif field['format'] == 'ipv4':
                inner_template += (
                    '        if (${field} != NULL)\n'
                    '            *${field} =  _mbim_message_read_ipv4 (message, offset, FALSE);\n'
                    '        offset += 4;\n')
            elif field['format'] == 'ref-ipv4':
                inner_template += (
                    '        if (${field} != NULL)\n'
                    '            *${field} =  _mbim_message_read_ipv4 (message, offset, TRUE);\n'
                    '        offset += 4;\n')
            elif field['format'] == 'ipv4-array':
                inner_template += (
                    '        if (${field} != NULL)\n'
                    '            *${field} =  _mbim_message_read_ipv4_array (message, _${array_size_field}, offset);\n'
                    '        offset += 4;\n')
            elif field['format'] == 'ipv6':
                inner_template += (
                    '        if (${field} != NULL)\n'
                    '            *${field} =  _mbim_message_read_ipv6 (message, offset, FALSE);\n'
                    '        offset += 4;\n')
            elif field['format'] == 'ref-ipv6':
                inner_template += (
                    '        if (${field} != NULL)\n'
                    '            *${field} =  _mbim_message_read_ipv6 (message, offset, TRUE);\n'
                    '        offset += 4;\n')
            elif field['format'] == 'ipv6-array':
                inner_template += (
                    '        if (${field} != NULL)\n'
                    '            *${field} =  _mbim_message_read_ipv6_array (message, _${array_size_field}, offset);\n'
                    '        offset += 4;\n')

            inner_template += (
                '    }\n')

            template += (string.Template(inner_template).substitute(translations))

        template += (
            '\n'
            '    return TRUE;\n'
            '}\n')
        cfile.write(string.Template(template).substitute(translations))


    """
    Emit the section content
    """
    def emit_section_content(self, sfile):
        translations = { 'name_dashed' : utils.build_dashed_name(self.name),
                         'underscore'  : utils.build_underscore_name(self.fullname) }

        template = (
            '\n'
            '<SUBSECTION ${name_dashed}>\n')
        sfile.write(string.Template(template).substitute(translations))

        if self.has_query:
            template = (
                '${underscore}_query_new\n')
            sfile.write(string.Template(template).substitute(translations))

        if self.has_set:
            template = (
                '${underscore}_set_new\n')
            sfile.write(string.Template(template).substitute(translations))

        if self.has_response:
            template = (
                '${underscore}_response_parse\n')
            sfile.write(string.Template(template).substitute(translations))

        if self.has_notification:
            template = (
                '${underscore}_notification_parse\n')
            sfile.write(string.Template(template).substitute(translations))
