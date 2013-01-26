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

class EnumEntry:
    def __init__(self, line):
        parts = line.split('^')
        if len(parts) < 3:
            raise Exception("Invalid enum entry line '%s'" % line)
        self.id = int(parts[0])
        self.value = int(parts[1], 0)
        self.name = parts[2].replace('"', '')
        self.descid = None
        if len(parts) > 3:
            self.descid = int(parts[3])

    def emit(self, enum_name):
        print "\tGOBI_%s_%s\t\t= 0x%08x,     /* %s */" % (
                    utils.constname(enum_name),
                    utils.constname(self.name),
                    self.value, self.name)

class Enum:
    def __init__(self, line):
        parts = line.split('^')
        if len(parts) < 4:
            raise Exception("Invalid enum line '%s'" % line)
        self.id = int(parts[0])
        self.name = parts[1].replace('"', '')
        self.descid = int(parts[2])
        self.internal = int(parts[3]) != 0
        self.values = []   # list of EnumEntry objects

    def add_entry(self, entry):
        for v in self.values:
            if entry.value == v.value:
                raise Exception("Enum %d already has value %d" % (self.id, v.value))
        self.values.append(entry)
        self.values.sort(lambda x, y: cmp(x.value, y.value))

    def emit(self):
        print 'typedef enum { /* %s */ ' % self.name
        for en in self.values:
            en.emit(self.name)
        print "} gobi_%s;\n" % utils.nicename(self.name)
            

class Enums(utils.DbFile):
    def __init__(self, path):
        self.enums = {}

        # parse the enums
        f = file(path + "Enum.txt")
        for line in f:
            try:
                enum = Enum(line.strip())
                self.enums[enum.id] = enum
            except Exception(e):
                pass
        f.close()

        # and now the enum entries
        f = file(path + "EnumEntry.txt")
        for line in f:
            try:
                entry = EnumEntry(line.strip())
                self.enums[entry.id].add_entry(entry)
            except Exception(e):
                pass
        f.close()

    def emit(self):
        for e in self.enums:
            self.enums[e].emit()

    def get_child(self, eid):
        return self.enums[eid]

