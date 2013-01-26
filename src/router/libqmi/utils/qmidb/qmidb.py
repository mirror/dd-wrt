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

import sys
import Entities
import Enums
import Fields
import Structs

if len(sys.argv) > 2:
    print "Usage: qmidb.py <path to Entity.txt>"
    sys.exit(1)
path = ""
if len(sys.argv) == 2:
    path = sys.argv[1] + "/"

enums = Enums.Enums(path)
entities = Entities.Entities(path)
fields = Fields.Fields(path)
structs = Structs.Structs(path)

structs.validate(fields)
entities.validate(structs)

print '/* GENERATED CODE. DO NOT EDIT. */'
print '\ntypedef uint8 bool;\n'
enums.emit()

print '\n\n'

structs_used = entities.emit(fields, structs, enums)

# emit structs that weren't associated with an entity
structs.emit_unused(structs_used, fields, enums)

