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

# eDB2FragmentType
TYPE_FIELD              = 0 # Simple field fragment
TYPE_STRUCT             = 1 # Structure fragment
TYPE_CONSTANT_PAD       = 2 # Pad fragment, fixed length (bits)
TYPE_VARIABLE_PAD_BITS  = 3 # Pad fragment, variable (bits)
TYPE_VARIABLE_PAD_BYTES = 4 # Pad fragment, variable (bytes)
TYPE_FULL_BYTE_PAD      = 5 # Pad fragment, pad to a full byte
TYPE_MSB_2_LSB          = 6 # Switch to MSB -> LSB order
TYPE_LSB_2_MSB          = 7 # Switch to LSB -> MSB order

def is_pad_type(t):
    if t == TYPE_CONSTANT_PAD or t == TYPE_VARIABLE_PAD_BITS or \
        t == TYPE_VARIABLE_PAD_BYTES or t == TYPE_FULL_BYTE_PAD:
        return True
    return False

# eDB2ModifierType
MOD_NONE             = 0  # Modifier is not used
MOD_CONSTANT_ARRAY   = 1  # Constant (elements) array
MOD_VARIABLE_ARRAY   = 2  # Variable (elements) array (modval gives #elements field)
MOD_OBSOLETE_3       = 3  # Constant (bits) array [OBS]
MOD_OBSOLETE_4       = 4  # Variable (bits) array [OBS]
MOD_OPTIONAL         = 5  # Fragment is optional
MOD_VARIABLE_ARRAY2  = 6  # Variable (elements) array, start/stop given
MOD_VARIABLE_ARRAY3  = 7  # Variable (elements) array, simple expression
MOD_VARIABLE_STRING1 = 8  # Variable length string (bit length)
MOD_VARIABLE_STRING2 = 9  # Variable length string (byte length)
MOD_VARIABLE_STRING3 = 10 # Variable length string (character length)


class FragmentBase:
    # Struct fragments (ie, each line in Struct.txt describe each member of
    # a struct.  The format is as follows:
    #
    # id^order^type^fieldid^name^offset^modtype^modval
    #
    # 50409^1^0^54024^""^-1^2^"54023"
    #
    # This describes the second element (ie, element #1) of struct 50409,
    # which is of type FIELD (0) and the value is described by Field #54024.
    # Additionally, it's "modtype" is VARIABLE_ARRAY which is described by
    # Field #54023.  Basically, this fragment is a variable array of
    # "Medium Preference" values, each element of which is one of the
    # "QMI PDS Mediums" (Enum.txt, #50407 given by Field.txt #54024).  The
    # number of elements in the array is given by the "modtype" and "modval"
    # pointers, which say that Field #54023 (which is also the first member
    # of this struct) specifies the number of elements in this variable array.

    def __init__(self, line):
        parts = line.split('^')
        if len(parts) < 8:
            raise Exception("Invalid struct fragment '%s'" % line)
        self.id = int(parts[0])
        self.order = int(parts[1])
        self.type = int(parts[2])   # eDB2FragmentType
        self.value = int(parts[3])  # id of field in Fields.txt
        self.name = parts[4].replace('"', '')
        self.offset = int(parts[5])
        self.modtype = int(parts[6])  # eDB2ModifierType
        self.modval = parts[7].replace('"', '')

    #Struct.txt:50021^0^0^50118^""^-1^1^"4"

    def validate(self, fields, structs):
        raise Exception("Surprising struct %d:%d field: %d" % (self.id, self.order, self.type))

    # Should return size in *bits* of this struct fragment, including all
    # sub-fragments
    def emit(self, do_print, entity_name, indent, reserved_bits, fields, structs, enums, cur_structsize):
        return 0

class Msb2LsbFragment(FragmentBase):
    # Subclass for TYPE_MSB_2_LSB
    def validate(self, fields, structs):
        pass

class FieldFragment(FragmentBase):
    # Subclass for TYPE_FIELD
    def validate(self, fields, structs):
        self.field = fields.get_child(self.value)

    def emit(self, do_print, entity_name, indent, reserved_bits, fields, structs, enums, cur_structsize):
        # modify the field like cProtocolEntityNav::ProcessFragment()
        num_elements = 0
        comment = ""
        isarray = False

        if self.modtype == MOD_CONSTANT_ARRAY:
            # modval is number of elements
            num_elements = int(self.modval)
        elif self.modtype == MOD_VARIABLE_ARRAY or \
             self.modtype == MOD_VARIABLE_STRING1 or \
             self.modtype == MOD_VARIABLE_STRING2 or \
             self.modtype == MOD_VARIABLE_STRING3:
            # The "modval" is the field id that gives the # of elements
            # of this variable array.  That field is usually the immediately
            # previous fragment of this struct.  ie it's something like:
            #
            # struct foobar {
            #     u8 arraylen;   <- specified by modval
            #     char array[];  <- current struct fragment
            # }
            #
            fdesc = fields.get_child(int(self.modval))
            comment = "size given by %s" % utils.nicename(fdesc.name)
            isarray = True

        return self.field.emit(do_print, indent, enums, num_elements, comment, isarray)


class StructFragment(FragmentBase):
    # Subclass for TYPE_STRUCT
    def validate(self, fields, structs):
        self.struct = structs.get_child(self.value)

    def emit(self, do_print, entity_name, indent, reserved_bits, fields, structs, enums, cur_structsize):
        # embedded structs often won't have a name of their own
        structname = "a_item"
        if self.name:
            structname = "%s_item" % utils.nicename(self.name)

        bits = self.struct.emit(structname, indent, reserved_bits, fields, structs, enums)

        # Ignore the condition on some structs' modvals
        fdesc_id = 0
        if len(self.modval) and self.modval.find("=") < 0:
            fdesc_id = int(self.modval)

        comment = ""
        arraybits = ""
        try:
            fdesc = fields.get_child(fdesc_id)
            comment = "\t /* size given by %s */" % utils.nicename(fdesc.name)
            arraybits = "[0]"
        except KeyError:
            pass

        if do_print:
            varname = utils.nicename(self.name)
            if not varname:
                varname = "item"
            print "%sstruct %s %s%s;%s" % ("\t" * indent, structname, utils.nicename(self.name), arraybits, comment)

        return bits

class ConstantPadFragment(FragmentBase):
    # Subclass for TYPE_CONSTANT_PAD
    def __init__(self, line):
        FragmentBase.__init__(self, line)
        # padsize is total struct size (in bits) including this fragment; ie
        # given the current struct size, pad the struct out to padsize
        self.padsize = self.value

    def validate(self, fields, structs):
        if self.value < 0 or self.value > 1000:
            raise Exception("Invalid constant pad size %d" % self.value)

    def emit(self, do_print, entity_name, indent, reserved_bits, fields, structs, enums, cur_structsize):
        # cur_structsize is in bits
        if cur_structsize > self.padsize:
            raise ValueError("Current structure size (%d) is larger than pad size (%d)!")
 
        padbits = self.padsize - cur_structsize
        padtype = "uint8"
        if padbits > 8 and padbits <= 16:
            padtype = "uint16"
        elif padbits > 16 and padbits <= 32:
            padtype = "uint32"
        elif padbits > 32 and padbits <= 64:
            padtype = "uint64"
        else:
            raise ValueError("FIXME: handle multi-long padding")

        if do_print:
            if padbits in [8, 16, 32]:
                print "%s%s padding;" % ("\t" * indent, padtype)
            else:
                print "%s%s padding:%d;" % ("\t" * indent, padtype, padbits)
        return padbits

class Struct:
    def __init__(self, sid):
        self.id = sid
        self.fragments = []
        self.name = "struct_%d" % sid

    def add_fragment(self, fragment):
        for f in self.fragments:
            if fragment.order == f.order:
                raise Exception("Struct %d already has fragment order %d" % (self.id, fragment.order))
        self.fragments.append(fragment)
        self.fragments.sort(lambda x, y: cmp(x.order, y.order))

    def validate(self, fields, structs):
        for f in self.fragments:
            f.validate(fields, structs)

    def emit_header(self, entity_name, cmdno, tlvno):
        if not entity_name:
            entity_name = self.name
            svcname = "Unknown"
            cmdname = "Unknown"
            tlvname = "Unknown"
        else:
            parts = entity_name.split("/")
            svcname = parts[0].upper()
            cmdname = parts[1].upper().replace(" ", "_")
            tlvname = parts[2]

        print '/**'
        print ' * SVC: %s' % svcname
        print ' * CMD: 0x%04x (%s)' % (cmdno, cmdname)
        print ' * TLV: 0x%02x   (%s)' % (tlvno, tlvname)
        print ' * ID:  %d' % self.id
        print ' */'

    def need_union(self, frag_idx):
        # scan to see if there is any pad fragment in this struct; if there is,
        # and if there's a struct fragment before the pad fragment, we'll need
        # to make a union
        found_struct = False
        for i in range(frag_idx, len(self.fragments)):
            if self.fragments[i].type == TYPE_STRUCT:
                found_struct = True
            if is_pad_type(self.fragments[i].type):
                if found_struct:
                    return True
                break
        return False

    def emit(self, entity_name, indent, pad_bits, fields, structs, enums):
        print '%sstruct %s {' % ("\t" * indent, utils.nicename(entity_name))

        # current structure size in *bits*
        size_in_bits = 0

        # if there was a previous member we're supposed to add some padding
        # for (ie, this struct is in a union and something is before it) do
        # that now
        if pad_bits:
            print '%suint8 padding:%d;' % ("\t" * (indent + 1), pad_bits)
            size_in_bits += pad_bits

        union_frag = None
        reserved_bits = 0

        for i in range(0, len(self.fragments)):
            frag = self.fragments[i]

            if not union_frag:
                if self.need_union(i):
                    union_frag = frag
                    indent += 1
                    print '%sunion u_%s {' % ("\t" * indent, utils.nicename(frag.field.name))

            bits = frag.emit(True, entity_name, indent + 1, reserved_bits, fields, structs, enums, size_in_bits)
            if frag == union_frag:
                # Track the first union fragment's reserved bits; we'll ignore
                # them for total struct size since the following struct(s) will
                # include it's size
                reserved_bits = bits
            elif is_pad_type(frag.type):
                # When we reach the pad fragment, the union terminates so
                # clear out union-specific state
                reserved_bits = 0
                if union_frag:
                    print '%s};' % ("\t" * indent)
                    indent -= 1
                    union_frag = None
                size_in_bits += bits
            else:
                size_in_bits += bits

        print "%s};\n" % ("\t" * indent)
        return size_in_bits

class Structs(utils.DbFile):
    def __init__(self, path):
        self.structs = {}
        f = file(path + "Struct.txt")
        for line in f:
            if len(line.strip()) == 0:
                continue

            # parse enough of the line to get struct ID and type
            parts = line.split('^')
            if len(parts) < 3:
                raise Exception("Invalid struct fragment '%s'" % line)

            # make a new struct if we don't know about this one already
            struct_id = int(parts[0])
            try:
                struct = self.structs[struct_id]
            except KeyError:
                struct = Struct(struct_id)
                self.structs[struct.id] = struct

            frag_type = int(parts[2])
            try:
                frag_class = { TYPE_FIELD: FieldFragment,
                               TYPE_STRUCT: StructFragment,
                               TYPE_CONSTANT_PAD: ConstantPadFragment,
                               TYPE_MSB_2_LSB: Msb2LsbFragment
                             }[frag_type]
            except KeyError:
                # fall back to base class
                frag_class = FragmentBase

            frag = frag_class(line.strip())
            struct.add_fragment(frag)
        f.close()

    def validate(self, fields):
        for s in self.structs.values():
            s.validate(fields, self)

    def has_child(self, sid):
        return sid in self.structs

    def get_child(self, sid):
        return self.structs[sid]

    def emit_unused(self, used, fields, enums):
        print '/**** UNKNOWN TLVs ****/\n'
        for s in self.structs.values():
            if not s.id in used:
                s.emit_header(None, 0, 0)
                s.emit(None, 0, fields, self, enums)

