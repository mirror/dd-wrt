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
from Container import Container

"""
The Message class takes care of request/response message handling
"""
class Message:

    """
    Constructor
    """
    def __init__(self, dictionary, common_objects_dictionary):
        # The message service, e.g. "Ctl"
        self.service = dictionary['service']
        # The name of the specific message, e.g. "Something"
        self.name = dictionary['name']
        # The specific message ID
        self.id = dictionary['id']
        # The type, which must always be 'Message' or 'Indication'
        self.type = dictionary['type']
        # The version info, optional
        self.version_info = dictionary['version'].split('.') if 'version' in dictionary else []
        self.static = True if 'scope' in dictionary and dictionary['scope'] == 'library-only' else False
        self.abort = True if 'abort' in dictionary and dictionary['abort'] == 'yes' else False

        # The message prefix
        self.prefix = 'Qmi ' + self.type

        # Create the composed full name (prefix + service + name),
        #  e.g. "Qmi Message Ctl Something"
        self.fullname = self.prefix + ' ' + self.service + ' ' + self.name

        # Create the ID enumeration name
        self.id_enum_name = string.upper(utils.build_underscore_name(self.fullname))

        # Build output container.
        # Every defined message will have its own output container, which
        # will generate a new Output type and public getters for each output
        # field. This applies to both Request/Response and Indications.
        # Output containers are actually optional in Indications
        self.output = Container(self.fullname,
                                'Output',
                                dictionary['output'] if 'output' in dictionary else None,
                                common_objects_dictionary,
                                self.static)

        self.input = None
        if self.type == 'Message':
            # Build input container (Request/Response only).
            # Every defined message will have its own input container, which
            # will generate a new Input type and public getters for each input
            # field
            self.input = Container(self.fullname,
                                   'Input',
                                   dictionary['input'] if 'input' in dictionary else None,
                                   common_objects_dictionary,
                                   self.static)


    """
    Emit method responsible for creating a new request of the given type
    """
    def __emit_request_creator(self, hfile, cfile):
        translations = { 'name'       : self.name,
                         'service'    : self.service,
                         'container'  : utils.build_camelcase_name (self.input.fullname),
                         'underscore' : utils.build_underscore_name (self.fullname),
                         'message_id' : self.id_enum_name }

        input_arg_template = 'gpointer unused' if self.input.fields is None else '${container} *input'
        template = (
            '\n'
            'static QmiMessage *\n'
            '__${underscore}_request_create (\n'
            '    guint8 transaction_id,\n'
            '    guint8 cid,\n'
            '    %s,\n'
            '    GError **error)\n'
            '{\n'
            '    QmiMessage *self;\n'
            '\n'
            '    self = qmi_message_new (QMI_SERVICE_${service},\n'
            '                            cid,\n'
            '                            transaction_id,\n'
            '                            ${message_id});\n' % input_arg_template)
        cfile.write(string.Template(template).substitute(translations))

        if self.input.fields is not None:
            # Count how many mandatory fields we have
            n_mandatory = 0
            for field in self.input.fields:
                if field.mandatory == 'yes':
                    n_mandatory += 1

            if n_mandatory == 0:
                # If we don't have mandatory fields, we do allow to have
                # a NULL input
                cfile.write(
                    '\n'
                    '    /* All TLVs are optional, we allow NULL input */\n'
                    '    if (!input)\n'
                    '        return self;\n')
            else:
                # If we do have mandatory fields, issue error if no input
                # given.
                template = (
                    '\n'
                    '    /* There is at least one mandatory TLV, don\'t allow NULL input */\n'
                    '    if (!input) {\n'
                    '        g_set_error (error,\n'
                    '                     QMI_CORE_ERROR,\n'
                    '                     QMI_CORE_ERROR_INVALID_ARGS,\n'
                    '                     "Message \'${name}\' has mandatory TLVs");\n'
                    '        qmi_message_unref (self);\n'
                    '        return NULL;\n'
                    '    }\n')
                cfile.write(string.Template(template).substitute(translations))

            # Now iterate fields
            for field in self.input.fields:
                translations['tlv_name'] = field.name
                translations['variable_name'] = field.variable_name
                template = (
                    '\n'
                    '    /* Try to add the \'${tlv_name}\' TLV */\n'
                    '    if (input->${variable_name}_set) {\n')
                cfile.write(string.Template(template).substitute(translations))

                # Emit the TLV getter
                field.emit_input_tlv_add(cfile, '        ')

                if field.mandatory == 'yes':
                    template = (
                        '    } else {\n'
                        '        g_set_error (error,\n'
                        '                     QMI_CORE_ERROR,\n'
                        '                     QMI_CORE_ERROR_INVALID_ARGS,\n'
                        '                     "Missing mandatory TLV \'${tlv_name}\' in message \'${name}\'");\n'
                        '        qmi_message_unref (self);\n'
                        '        return NULL;\n')
                    cfile.write(string.Template(template).substitute(translations))

                cfile.write(
                    '    }\n')
        cfile.write(
            '\n'
            '    return self;\n'
            '}\n')


    """
    Emit method responsible for parsing a response/indication of the given type
    """
    def __emit_response_or_indication_parser(self, hfile, cfile):
        # If no output fields to parse, don't emit anything
        if self.output is None or self.output.fields is None:
            return

        translations = { 'name'                 : self.name,
                         'type'                 : 'response' if self.type == 'Message' else 'indication',
                         'container'            : utils.build_camelcase_name (self.output.fullname),
                         'container_underscore' : utils.build_underscore_name (self.output.fullname),
                         'underscore'           : utils.build_underscore_name (self.fullname),
                         'message_id'           : self.id_enum_name }

        template = (
            '\n'
            'static ${container} *\n'
            '__${underscore}_${type}_parse (\n'
            '    QmiMessage *message,\n'
            '    GError **error)\n'
            '{\n'
            '    ${container} *self;\n'
            '\n'
            '    g_return_val_if_fail (qmi_message_get_message_id (message) == ${message_id}, NULL);\n'
            '\n'
            '    self = g_slice_new0 (${container});\n'
            '    self->ref_count = 1;\n')
        cfile.write(string.Template(template).substitute(translations))

        for field in self.output.fields:
            cfile.write(
                '\n'
                '    do {\n')
            field.emit_output_prerequisite_check(cfile, '        ')
            cfile.write(
                '\n'
                '        {\n')
            field.emit_output_tlv_get(cfile, '            ')
            cfile.write(
                '\n'
                '        }\n')
            cfile.write(
                '    } while (0);\n')
        cfile.write(
            '\n'
            '    return self;\n'
            '}\n')


    """
    Emit method responsible for getting a printable representation of the whole
    request/response
    """
    def __emit_helpers(self, hfile, cfile):
        need_tlv_printable = False
        if self.input is not None and self.input.fields is not None:
            need_tlv_printable = True
            for field in self.input.fields:
                field.emit_tlv_helpers(cfile)

        if self.output is not None and self.output.fields is not None:
            need_tlv_printable = True
            for field in self.output.fields:
                field.emit_tlv_helpers(cfile)

        translations = { 'name'       : self.name,
                         'service'    : self.service,
                         'id'         : self.id,
                         'type'       : utils.build_underscore_name(self.type),
                         'underscore' : utils.build_underscore_name(self.name) }

        template = ''
        if need_tlv_printable:
            template += (
                '\n'
                'struct ${type}_${underscore}_context {\n'
                '    QmiMessage *self;\n'
                '    const gchar *line_prefix;\n'
                '    GString *printable;\n'
                '};\n'
                '\n'
                'static void\n'
                '${type}_${underscore}_get_tlv_printable (\n'
                '    guint8 type,\n'
                '    const guint8 *value,\n'
                '    gsize length,\n'
                '    struct ${type}_${underscore}_context *ctx)\n'
                '{\n'
                '    const gchar *tlv_type_str = NULL;\n'
                '    gchar *translated_value;\n'
                '\n')

            if self.type == 'Message':
                template += (
                    '    if (!qmi_message_is_response (ctx->self)) {\n'
                    '        switch (type) {\n')

                if self.input is not None and self.input.fields is not None:
                    for field in self.input.fields:
                        translations['underscore_field'] = utils.build_underscore_name(field.fullname)
                        translations['field_enum'] = field.id_enum_name
                        translations['field_name'] = field.name
                        field_template = (
                            '        case ${field_enum}:\n'
                            '            tlv_type_str = "${field_name}";\n'
                            '            translated_value = ${underscore_field}_get_printable (\n'
                            '                                   ctx->self,\n'
                            '                                   ctx->line_prefix);\n'
                            '            break;\n')
                        template += string.Template(field_template).substitute(translations)

                template += (
                    '        default:\n'
                    '            break;\n'
                    '        }\n'
                    '    } else {\n')
            else:
                template += ('    {\n')

            template += ('        switch (type) {\n')
            if self.output is not None and self.output.fields is not None:
                for field in self.output.fields:
                    translations['underscore_field'] = utils.build_underscore_name(field.fullname)
                    translations['field_enum'] = field.id_enum_name
                    translations['field_name'] = field.name
                    field_template = (
                        '        case ${field_enum}:\n'
                        '            tlv_type_str = "${field_name}";\n'
                        '            translated_value = ${underscore_field}_get_printable (\n'
                        '                                   ctx->self,\n'
                        '                                   ctx->line_prefix);\n'
                        '            break;\n')
                    template += string.Template(field_template).substitute(translations)

            template += (
                '        default:\n'
                '            break;\n'
                '        }\n'
                '    }\n'
                '\n'
                '    if (!tlv_type_str) {\n'
                '        gchar *value_str = NULL;\n'
                '\n'
                '        value_str = qmi_message_get_tlv_printable (ctx->self,\n'
                '                                                   ctx->line_prefix,\n'
                '                                                   type,\n'
                '                                                   value,\n'
                '                                                   length);\n'
                '        g_string_append (ctx->printable, value_str);\n'
                '        g_free (value_str);\n'
                '    } else {\n'
                '        gchar *value_hex;\n'
                '\n'
                '        value_hex = __qmi_utils_str_hex (value, length, \':\');\n'
                '        g_string_append_printf (ctx->printable,\n'
                '                                "%sTLV:\\n"\n'
                '                                "%s  type       = \\"%s\\" (0x%02x)\\n"\n'
                '                                "%s  length     = %" G_GSIZE_FORMAT "\\n"\n'
                '                                "%s  value      = %s\\n"\n'
                '                                "%s  translated = %s\\n",\n'
                '                                ctx->line_prefix,\n'
                '                                ctx->line_prefix, tlv_type_str, type,\n'
                '                                ctx->line_prefix, length,\n'
                '                                ctx->line_prefix, value_hex,\n'
                '                                ctx->line_prefix, translated_value ? translated_value : "");\n'
                '        g_free (value_hex);\n'
                '        g_free (translated_value);\n'
                '    }\n'
                '}\n')

        template += (
            '\n'
            'static gchar *\n'
            '${type}_${underscore}_get_printable (\n'
            '    QmiMessage *self,\n'
            '    const gchar *line_prefix)\n'
            '{\n'
            '    GString *printable;\n'
            '\n'
            '    printable = g_string_new ("");\n'
            '    g_string_append_printf (printable,\n'
            '                            "%s  message     = \\\"${name}\\\" (${id})\\n",\n'
            '                            line_prefix);\n')

        if need_tlv_printable:
            template += (
                '\n'
                '    {\n'
                '        struct ${type}_${underscore}_context ctx;\n'
                '        ctx.self = self;\n'
                '        ctx.line_prefix = line_prefix;\n'
                '        ctx.printable = printable;\n'
                '        qmi_message_foreach_raw_tlv (self,\n'
                '                                     (QmiMessageForeachRawTlvFn)${type}_${underscore}_get_tlv_printable,\n'
                '                                     &ctx);\n'
                '    }\n')
        template += (
            '\n'
            '    return g_string_free (printable, FALSE);\n'
            '}\n')
        cfile.write(string.Template(template).substitute(translations))


    """
    Emit request/response/indication handling implementation
    """
    def emit(self, hfile, cfile):
        if self.type == 'Message':
            utils.add_separator(hfile, 'REQUEST/RESPONSE', self.fullname);
            utils.add_separator(cfile, 'REQUEST/RESPONSE', self.fullname);
        else:
            utils.add_separator(hfile, 'INDICATION', self.fullname);
            utils.add_separator(cfile, 'INDICATION', self.fullname);

        if self.type == 'Message':
            hfile.write('\n/* --- Input -- */\n');
            cfile.write('\n/* --- Input -- */\n');
            self.input.emit(hfile, cfile)
            self.__emit_request_creator(hfile, cfile)

        hfile.write('\n/* --- Output -- */\n');
        cfile.write('\n/* --- Output -- */\n');
        self.output.emit(hfile, cfile)
        self.__emit_helpers(hfile, cfile)
        self.__emit_response_or_indication_parser(hfile, cfile)

    """
    Emit the sections
    """
    def emit_sections(self, sfile):
        if self.static:
            return

        translations = { 'hyphened'            : utils.build_dashed_name (self.fullname),
                         'fullname_underscore' : utils.build_underscore_name(self.fullname),
                         'camelcase'           : utils.build_camelcase_name (self.fullname),
                         'service'             : utils.build_underscore_name (self.service),
                         'name_underscore'     : utils.build_underscore_name (self.name),
                         'fullname'            : self.service + ' ' + self.name,
                         'type'                : 'response' if self.type == 'Message' else 'indication' }

        sections = { 'public-types'   : '',
                     'public-methods' : '',
                     'standard'       : '',
                     'private'        : '' }

        if self.input:
            self.input.add_sections (sections)
        self.output.add_sections (sections)

        if self.type == 'Message':
            template = (
                '<SUBSECTION ${camelcase}ClientMethods>\n'
                'qmi_client_${service}_${name_underscore}\n'
                'qmi_client_${service}_${name_underscore}_finish\n')
            sections['public-methods'] += string.Template(template).substitute(translations)

        translations['public_types']   = sections['public-types']
        translations['public_methods'] = sections['public-methods']
        translations['standard']       = sections['standard']
        translations['private']        = sections['private']

        template = (
            '<SECTION>\n'
            '<FILE>${hyphened}</FILE>\n'
            '<TITLE>${fullname}</TITLE>\n'
            '${public_types}'
            '${public_methods}'
            '<SUBSECTION Private>\n'
            '${private}'
            '<SUBSECTION Standard>\n'
            '${standard}'
            '</SECTION>\n'
            '\n')
        sfile.write(string.Template(template).substitute(translations))
