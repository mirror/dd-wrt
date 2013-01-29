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
import re

"""
Add the common copyright header to the given file
"""
def add_copyright(f):
    f.write(
        "\n"
        "/* GENERATED CODE... DO NOT EDIT */\n"
        "\n"
        "/*\n"
        " * This library is free software; you can redistribute it and/or\n"
        " * modify it under the terms of the GNU Lesser General Public\n"
        " * License as published by the Free Software Foundation; either\n"
        " * version 2 of the License, or (at your option) any later version.\n"
        " *\n"
        " * This library is distributed in the hope that it will be useful,\n"
        " * but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        " * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
        " * Lesser General Public License for more details.\n"
        " *\n"
        " * You should have received a copy of the GNU Lesser General Public\n"
        " * License along with this library; if not, write to the\n"
        " * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,\n"
        " * Boston, MA 02110-1301 USA.\n"
        " *\n"
        " * Copyright (C) 2012 Lanedo GmbH\n"
        " */\n"
        "\n");


"""
Build a header guard string based on the given filename
"""
def build_header_guard(output_name):
    return "__LIBQMI_GLIB_" + output_name.replace('-', '_').upper() + "__"

"""
Write the common header start chunk
"""
def add_header_start(f, output_name, service):
    translations = { 'guard'   : build_header_guard(output_name),
                     'service' : build_underscore_name(service) }
    template = (
        "\n"
        "#include <glib.h>\n"
        "#include <glib-object.h>\n"
        "#include <gio/gio.h>\n"
        "\n"
        "#include \"qmi-enums.h\"\n")
    # CTL doesn't have enums
    if service != 'CTL':
        template += (
            "#include \"qmi-enums-${service}.h\"\n")
    else:
        template += (
            "#include \"qmi-enums-private.h\"\n")
    # CTL, WDS, WMS and PDS don't have flags64
    if service != 'CTL' and service != 'WDS' and service != 'WMS' and service != 'PDS':
        template += (
            "#include \"qmi-flags64-${service}.h\"\n")
    template += (
        "#include \"qmi-message.h\"\n"
        "#include \"qmi-client.h\"\n"
        "\n"
        "#ifndef ${guard}\n"
        "#define ${guard}\n"
        "\n"
        "G_BEGIN_DECLS\n"
        "\n")
    f.write(string.Template(template).substitute(translations))


"""
Write the common header stop chunk
"""
def add_header_stop(f, output_name):
    template = string.Template (
        "\n"
        "G_END_DECLS\n"
        "\n"
        "#endif /* ${guard} */\n")
    f.write(template.substitute(guard = build_header_guard(output_name)))


"""
Write the common source file start chunk
"""
def add_source_start(f, output_name):
    template = string.Template (
        "\n"
        "#include <string.h>\n"
        "\n"
        "#include \"${name}.h\"\n"
        "#include \"qmi-enum-types.h\"\n"
        "#include \"qmi-enum-types-private.h\"\n"
        "#include \"qmi-flags64-types.h\"\n"
        "#include \"qmi-error-types.h\"\n"
        "#include \"qmi-device.h\"\n"
        "#include \"qmi-utils.h\"\n"
        '\n'
        '#define QMI_STATUS_SUCCESS 0x0000\n'
        '#define QMI_STATUS_FAILURE 0x0001\n'
        "\n")
    f.write(template.substitute(name = output_name))


"""
Write a separator comment in the file
"""
def add_separator(f, separator_type, separator_name):
    template = string.Template (
        "\n"
        "/*****************************************************************************/\n"
        "/* ${type}: ${name} */\n"
        "\n")
    f.write(template.substitute(type = separator_type,
                                name = separator_name))


"""
Build an underscore name from the given full name
e.g.: "This is a message" --> "this_is_a_message"
"""
def build_underscore_name(name):
    return name.replace(' ', '_').lower()


"""
Build an underscore uppercase name from the given full name
e.g.: "This is a message" --> "THIS_IS_A_MESSAGE"
"""
def build_underscore_uppercase_name(name):
    return name.replace(' ', '_').upper()


"""
Build an underscore name from the given camelcase name
e.g.: "ThisIsAMessage" --> "this_is_a_message"
"""
def build_underscore_name_from_camelcase(camelcase):
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', camelcase)
    return re.sub('([a-z0-9])([A-Z])', r'\1_\2', s1).lower()


"""
Build a camelcase name from the given full name
e.g.: "This is a message" --> "ThisIsAMessage"
"""
def build_camelcase_name(name):
    return string.capwords(name).replace(' ', '')


"""
Build a dashed lowercase name from the given full name
e.g.: "This is a message" --> "this-is-a-message"
"""
def build_dashed_name(name):
    return name.lower().replace(' ', '-')


"""
Remove the given prefix from the string
"""
def remove_prefix(line, prefix):
    return line[len(prefix):] if line.startswith(prefix) else line


"""
Read the contents of the JSON file, skipping lines prefixed with '//', which are
considered comments.
"""
def read_json_file(path):
    f = open(path)
    out = ''
    for line in f.readlines():
        stripped = line.strip()
        if stripped.startswith('//'):
            # Skip this line
            # We add an empty line instead so that errors when parsing the JSON
            # report the proper line number
            out += "\n"
        else:
            out += line
    return out


"""
Returns True if the given format corresponds to a basic unsigned integer type
"""
def format_is_unsigned_integer(fmt):
    if fmt == 'guint8'  or \
       fmt == 'guint16' or \
       fmt == 'guint32' or \
       fmt == 'guint64' or \
       fmt == 'guint-sized' :
        return True
    else:
        return False


"""
Returns True if the given format corresponds to a basic signed integer type
"""
def format_is_signed_integer(fmt):
    if fmt == 'gint8'   or \
       fmt == 'gint16'  or \
       fmt == 'gint32'  or \
       fmt == 'gint64':
        return True
    else:
        return False


"""
Returns True if the given format corresponds to a basic signed or unsigned
integer type
"""
def format_is_integer(fmt):
    if format_is_unsigned_integer(fmt) or \
       format_is_signed_integer(fmt):
        return True
    else:
        return False
