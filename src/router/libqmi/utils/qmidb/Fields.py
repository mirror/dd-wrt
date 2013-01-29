#!/usr/bin/env python
# -*- Mode: python; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation, version 2.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 51
# Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
#
# Copyright (C) 2011 - 2012 Red Hat, Inc.
#

import utils


# eDB2FieldType
FIELD_TYPE_STD           = 0 # Field is a standard type (see below)
FIELD_TYPE_ENUM_UNSIGNED = 1 # Field is an unsigned enumerated type
FIELD_TYPE_ENUM_SIGNED   = 2 # Field is a signed enumerated type

# eDB2StdFieldType
FIELD_STD_BOOL = 0         # boolean (0/1, false/true)
FIELD_STD_INT8 = 1         # 8-bit signed integer
FIELD_STD_UINT8 = 2        # 8-bit unsigned integer
FIELD_STD_INT16 = 3        # 16-bit signed integer
FIELD_STD_UINT16 = 4       # 16-bit unsigned integer
FIELD_STD_INT32 = 5        # 32-bit signed integer
FIELD_STD_UINT32 = 6       # 32-bit unsigned integer
FIELD_STD_INT64 = 7        # 64-bit signed integer
FIELD_STD_UINT64 = 8       # 64-bit unsigned integer
FIELD_STD_STRING_A = 9     # ANSI (ASCII?) fixed length string; size in bits
FIELD_STD_STRING_U = 10    # UCS-2 fixed length string
FIELD_STD_STRING_ANT = 11  # ANSI (ASCII?) NULL terminated string
FIELD_STD_STRING_UNT = 12  # UCS-2 NULL terminated string
FIELD_STD_FLOAT32 = 13     # 32-bit floating point value
FIELD_STD_FLOAT64 = 14     # 64-bit floating point value
FIELD_STD_STRING_U8 = 15   # UTF-8 encoded fixed length string
FIELD_STD_STRING_U8NT = 16 # UTF-8 encoded NULL terminated string

stdtypes = {
    # Maps field type to [ <C type>, <is array>, <element size in bits> ]
    FIELD_STD_BOOL: [ 'bool', False, 8 ],
    FIELD_STD_INT8: [ 'int8', False, 8 ],
    FIELD_STD_UINT8: [ 'uint8', False, 8 ],
    FIELD_STD_INT16: [ 'int16', False, 16 ],
    FIELD_STD_UINT16: [ 'uint16', False, 16 ],
    FIELD_STD_INT32: [ 'int32', False, 32 ],
    FIELD_STD_UINT32: [ 'uint32', False, 32 ],
    FIELD_STD_INT64: [ 'int64', False, 64 ],
    FIELD_STD_UINT64: [ 'uint64', False, 64 ],
    FIELD_STD_STRING_A: [ 'char', True, 8 ],
    FIELD_STD_STRING_U: [ 'uint16', True, 16 ],
    FIELD_STD_STRING_ANT: [ 'char *', False, 8 ],
    FIELD_STD_STRING_UNT: [ 'char *', False, 8 ],
    FIELD_STD_FLOAT32: [ 'float32', False, 32 ],
    FIELD_STD_FLOAT64: [ 'float64', False, 64 ],
}

def is_array_type(t):
    return stdtypes[t][1]


class Field:
    def __init__(self, line):
        parts = line.split('^')
        if len(parts) < 6:
            raise Exception("Invalid field line '%s'" % line)
        self.id = int(parts[0])
        self.name = parts[1].replace('"', '')
        self.size = int(parts[2]) # in *bits*
        self.type = int(parts[3]) # eDB2FieldType
        self.typeval = int(parts[4])  # eDB2StdFieldType if 'type' == 0
        self.hex = int(parts[5]) != 0
        self.descid = None
        self.internal = False
        if len(parts) > 6:
            self.descid = int(parts[6])
        if len(parts) > 7:
            self.internal = int(parts[7])

        # Field.txt:50118^"IP V4 Address"^8^0^2^0

    #Field.txt:50118^"IP V4 Address"^8^0^2^0

    def get_charsize(self):
        if self.type == FIELD_TYPE_STD:
            if self.typeval == FIELD_STD_STRING_A or self.typeval == FIELD_STD_STRING_ANT or \
                self.typeval == FIELD_STD_STRING_U or self.typeval == FIELD_STD_STRING_UNT:
                return stdtypes[self.typeval][2]
        raise Exception("Called for non-string type")

    def emit(self, do_print, indent, enums, num_elements, comment, isarray):
        ctype = ''
        arraypart = ''

        sizebits = 0
        if self.type == FIELD_TYPE_STD:  # eDB2_FIELD_STD
            tinfo = stdtypes[self.typeval]
            ctype = tinfo[0]
            
            if is_array_type(self.typeval) or num_elements > 0:
                if num_elements > 0:
                    arraypart = "[%d]" % num_elements
                    sizebits = num_elements * tinfo[2]
                elif isarray:
                    # array with size given by previous fragment
                    arraypart = "[0]"
                    sizebits = 0
                else:
                    arraypart = "[%d]" % (self.size / tinfo[2])
                    sizebits = self.size * tinfo[2]
        elif self.type == FIELD_TYPE_ENUM_UNSIGNED or self.type == FIELD_TYPE_ENUM_SIGNED:
            # It's a enum; find the enum
            e = enums.get_child(self.typeval)
            ctype = "gobi_%s" % utils.nicename(e.name)
            if isarray:
                if num_elements != 0:
                    raise Exception("Unhandled ENUM field type with size %d" % num_elements)
                arraypart = "[0]";
                sizebits = 0
            else:
                if self.size > 0:
                    # enum size is # of bits
                    arraypart = ":%d" % self.size
                    sizebits = self.size
                else:
                    sizebits = 32
        else:
            raise ValueError("Unknown Field type")

        if comment:
            comment = " (%s)" % comment
        if do_print:
            print "%s%s %s%s; /* %s%s */" % ("\t" * indent, ctype, utils.nicename(self.name), arraypart, self.name, comment)
        return sizebits

class Fields:
    def __init__(self, path):
        self.byid = {}

        f = file(path + "Field.txt")
        for line in f:
            field = Field(line.strip())
            self.byid[field.id] = field

    def has_child(self, fid):
        return fid in self.byid

    def get_child(self, fid):
        return self.byid[fid]

