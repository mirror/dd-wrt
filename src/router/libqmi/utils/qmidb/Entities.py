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

class Entity:
    def __init__(self, line):
        # Each entity defines a TLV item in a QMI request or response.  The
        # entity's 'struct' field maps to the ID of a struct in Struct.txt,
        # which describes the fields of that struct.  For example:
        #
        # 33^"32,16"^"WDS/Start Network Interface Request/Primary DNS"^50021^-1
        #
        # The first field (33) indicates that this entity is valid for the QMI
        # message eDB2_ET_QMI_WDS_REQ. The "32,16" is a tuple
        # (eQMI_WDS_START_NET, 16), the first item of which (32) indicates the
        # general QMI service the entity is associated with (WDS, CTL, NAS, etc)
        # and the second item indicates the specific entity number (ie, the 'T'
        # in TLV).
        #
        # The 'struct' field (50021) points to:
        #
        # Struct.txt:50021^0^0^50118^""^-1^1^"4"
        #
        # which indicates that the only member of this struct is field #50118:
        #
        # Field.txt:50118^"IP V4 Address"^8^0^2^0
        #
        # which should be pretty self-explanatory.  Note that different entities
        # can point to the same struct.


        parts = line.split('^')
        if len(parts) < 4:
            raise Exception("Invalid entity line '%s'" % line)
        self.uniqueid = parts[0] + '.' + parts[1].replace('"', '').replace(',', '.')
        self.type = int(parts[0])  # eDB2EntityType

        self.key = parts[1].replace('"','')  # tuple of (eQMIMessageXXX, entity number)
        self.cmdno = int(self.key.split(",")[0])
        self.tlvno = int(self.key.split(",")[1])
        self.name = parts[2].replace('"', '')
        self.struct = int(parts[3])
        self.format = None
        self.internal = True
        self.extformat = None
        if len(parts) > 4:
            self.format = int(parts[4])
        if len(parts) > 5:
            self.internal = int(parts[5]) != 0
        if len(parts) > 6:
            self.extformat = int(parts[6])

    def validate(self, structs):
        if not structs.has_child(self.struct):
            raise Exception("Entity missing struct: %d" % self.struct)

    def emit(self, fields, structs, enums):
        if self.tlvno == 2 and self.name.find("/Result Code") > 0 and self.struct == 50000:
            # ignore this entity if it's a standard QMI result code struct
            return self.struct

        # Tell the struct this value is for to emit itself
        s = structs.get_child(self.struct)
        s.emit_header(self.name, self.cmdno, self.tlvno)
        s.emit(self.name, 0, 0, fields, structs, enums)
        return self.struct


class Entities(utils.DbFile):
    def __init__(self, path):
        self.byid = {}
        f = file(path + "Entity.txt")
        for line in f:
            ent = Entity(line)
            self.byid[ent.uniqueid] = ent

    def validate(self, structs):
        for e in self.byid.values():
            e.validate(structs)

    def emit(self, fields, structs, enums):
        # emit the standard status TLV struct
        print "struct qmi_result_code { /* QMI Result Code TLV (0x0002) */"
        print "\tgobi_qmi_results qmi_result; /* QMI Result */"
        print "\tgobi_qmi_errors qmi_error; /* QMI Error */"
        print "};"
        print ""

        structs_used = []
        for e in self.byid.values():
            sused = e.emit(fields, structs, enums)
            structs_used.append(sused)
        return structs_used

