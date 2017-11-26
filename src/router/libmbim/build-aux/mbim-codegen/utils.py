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
# Copyright (C) 2013 - 2014 Aleksander Morgado <aleksander@aleksander.es>
#
# Implementation originally developed in 'libqmi'.
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
        " * Copyright (C) 2013 - 2014 Aleksander Morgado <aleksander@aleksander.es>\n"
        " */\n"
        "\n");


"""
Build a header guard string based on the given filename
"""
def build_header_guard(output_name):
    return "__LIBMBIM_GLIB_" + output_name.replace('-', '_').upper() + "__"

"""
Write the common header start chunk
"""
def add_header_start(f, output_name):
    translations = { 'guard'   : build_header_guard(output_name) }
    template = (
        "\n"
        "#include <glib.h>\n"
        "#include <glib-object.h>\n"
        "#include <gio/gio.h>\n"
        "\n"
        "#include \"mbim-message.h\"\n"
        "#include \"mbim-device.h\"\n"
        "#include \"mbim-enums.h\"\n"
        "\n"
        "#ifndef ${guard}\n"
        "#define ${guard}\n"
        "\n"
        "G_BEGIN_DECLS\n")
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
        "#include \"mbim-message-private.h\"\n"
        "#include \"mbim-enum-types.h\"\n"
        "#include \"mbim-error-types.h\"\n"
        "#include \"mbim-device.h\"\n"
        "#include \"mbim-utils.h\"\n")
    f.write(template.substitute(name = output_name))


"""
Write a separator comment in the file
"""
def add_separator(f, separator_type, separator_name):
    template = string.Template (
        "\n"
        "/*****************************************************************************/\n"
        "/* ${type}: ${name} */\n")
    f.write(template.substitute(type = separator_type,
                                name = separator_name))


"""
Build an underscore name from the given full name
e.g.: "This is a message" --> "this_is_a_message"
"""
def build_underscore_name(name):
    return name.replace(' ', '_').replace('-', '_').lower()


"""
Build an underscore uppercase name from the given full name
e.g.: "This is a message" --> "THIS_IS_A_MESSAGE"
"""
def build_underscore_uppercase_name(name):
    return name.replace(' ', '_').replace('-', '_').upper()



"""
Build an underscore name from the given camelcase name
e.g.: "ThisIsAMessage" --> "this_is_a_message"
"""
def build_underscore_name_from_camelcase(camelcase):
    s0 = camelcase.replace('IP','Ip')
    s1 = re.sub('(.)([A-Z][a-z]+)', r'\1_\2', s0)
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
