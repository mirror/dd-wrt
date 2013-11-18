#!/usr/bin/env python
#
# This tool is intended to automate away the drudgery in bring up support
# for a new AIS message type.  It parses the tabular description of a message
# and generates various useful code snippets from that.  It can also be used to
# correct offsets in the tables themselves.
#
# Requires the AIVDM.txt file on standard input. Takes a single argument,
# the line number of a table start.  Things you can generate:
#
# * -t: A corrected version of the table.  It will redo all the offsets to be
#   in conformance with the bit widths.  
#
# * -s: A structure definition capturing the message info, with member
#   names extracted from the table and types computed from it.
#
# * -c: Bit-extraction code for the AIVDM driver.  Grind out the right sequence
#   of UBITS, SBITS, and UCHARS macros, and assignments to structure members,
#   guaranteed correct if the table offsets and widths are.
#
# * -d: Code to dump the contents of the unpacked message structure as JSON. If
#   the structure has float members, you'll get an if/then/else  guarded by
#   the scaled flag.
#
# * -r: A Python initializer stanza for jsongen.py, which is in turn used to
#   generate the specification structure for a JSON parse that reads JSON
#   into an instance of the message structure.
#
# * -a: Generate all of -s, -d, -c, and -r, , not to stdout but to
#   files named with the argument as a distinguishing part of the stem.
#
# This generates almost all the code required to support a new message type.
# It's not quite "Look, ma, no handhacking!" You'll need to add default
# values to the Python stanza. If the structure definition contains character
# arrays, you'll have to fill in the dimensions by hand.  You'll need to add
# a bit of glue to ais_json.c so that json_ais_read() actually calls the parser
# handing it the specification structure as a control argument.
#
# The -a, -c, -s, -d, and -r modes all take an argument, which should be a
# structure reference prefix to be prepended (before a dot) to each fieldname.
# Usually you'll need this to look something like "ais->typeN", but it could be
# "ais->typeN.FOO" if the generated code has to operate on a union member
# inside a type 6 or 8, or something similar.
#
# The -S and -E options allow you to generate code only for a specified span
# of fields in the table.  This may be useful for dealing with groups of
# messages that have a common head section.
#
# TO-DO: generate code for ais.py.

import sys, getopt

def correct_table(wfp):
    # Writes the corrected table.
    print >>sys.stderr, "Total bits:", base 
    for (i, t) in enumerate(table):
        if offsets[i].strip():
            print >>wfp, "|" + offsets[i] + t[owidth+1:].rstrip()
        else:
            print >>wfp, t.rstrip()

def make_driver_code(wfp):
    # Writes calls to bit-extraction macros.
    # Requires UBITS, SBITS, UCHARS to act as they do in the AIVDM driver.
    # Also relies on ais_context->bitlen to be the message bit length.
    record = after is None
    arrayname = None
    base = '\t'
    step = " " * 4
    indent = base
    for (i, t) in enumerate(table):
        if '|' in t:
            fields = map(lambda s: s.strip(), t.split('|'))
            width = fields[2]
            name = fields[4]
            ftype = fields[5]
            if after == name:
                record = True
                continue
            if before == name:
                record = False
                continue
            if not record:
                continue
            if ftype == 'x':
                print >>wfp,"\t/* skip %s bit%s */" % (width, ["", "s"][width>'1'])
                continue
            if ftype[0] == 'a':
                arrayname = name
                explicit = ftype[1] == '^'
                print >>wfp, '#define ARRAY_BASE %s' % offsets[i].strip()
                print >>wfp, '#define ELEMENT_SIZE %s' % trailing
                if explicit:
                    lengthfield = last
                    print >>wfp, indent + "for (ind = 0; ind < %s; ind++) {" % lengthfield 
                else:
                    lengthfield = "n" + arrayname
                    print >>wfp, indent + "for (ind = 0; ARRAY_BASE + (ELEMENT_SIZE*ind) <= ais_context->bitlen; ind++) {" 
                indent += step
                print >>wfp, indent + "int a = ARRAY_BASE + (ELEMENT_SIZE*ind);" 
                continue
            offset = offsets[i].split('-')[0]
            if arrayname:
                target = "%s.%s[ind].%s" % (structname, arrayname, name)
                offset = "a + " + offset 
            else:
                target = "%s.%s" % (structname, name)
            if ftype[0].lower() in ('u', 'i', 'e'):
                print >>wfp, indent + "%s\t= %sBITS(%s, %s);" % \
                      (target, {'u':'U', 'e':'U', 'i':'S'}[ftype[0].lower()], offset, width)
            elif ftype == 't':
                print >>wfp, indent + "UCHARS(%s, %s);" % (offset, target)
            else:
                print >>wfp, indent + "/* %s bits of type %s */" % (width,ftype)
            last = name
    if arrayname:
        indent = base
        print >>wfp, indent + "}"
        if not explicit:
            print >>wfp, indent + "%s.%s = ind;" % (structname, lengthfield)
        print >>wfp, "#undef ARRAY_BASE" 
        print >>wfp, "#undef ELEMENT_SIZE" 

def make_structure(wfp):
    # Write a structure definition correponding to the table.
    record = after is None
    baseindent = 8
    step = 4
    inwards = step
    arrayname = None
    def tabify(n):
        return ('\t' * (n / 8)) + (" " * (n % 8)) 
    print >>wfp, tabify(baseindent) + "struct {"
    for (i, t) in enumerate(table):
        if '|' in t:
            fields = map(lambda s: s.strip(), t.split('|'))
            width = fields[2]
            description = fields[3].strip()
            name = fields[4]
            ftype = fields[5]
            if after == name:
                record = True
                continue
            if before == name:
                record = False
                continue
            if ftype == 'x' or not record:
                continue
            if ftype[0] == 'a':
                arrayname = name
                if ftype[1] == '^':
                    lengthfield = last
                    ftype = ftype[1:]
                else:
                    lengthfield = "n%s" % arrayname
                    print >>wfp, tabify(baseindent + inwards) + "signed int %s;" % lengthfield
                if arrayname.endswith("s"):
                    typename = arrayname[:-1]
                else:
                    typename = arrayname
                print >>wfp, tabify(baseindent + inwards) + "struct %s_t {" % typename
                inwards += step
                arraydim = ftype[1:]
                continue
            elif ftype == 'u' or ftype == 'e' or ftype[0] == 'U':
                decl = "unsigned int %s;\t/* %s */" % (name, description)
            elif ftype == 'i' or ftype[0] == 'I':
                decl = "signed int %s;\t/* %s */" % (name, description)
            elif ftype == 'b':
                decl = "signed int %s;\t/* %s */" % (name, description)
            elif ftype == 't':
                stl = int(width)/6
                decl = "char %s[%d+1];\t/* %s */" % (name, stl, description)
            else:
                decl = "/* %s bits of type %s */" % (width, ftype)
            print >>wfp, tabify(baseindent + inwards) + decl
        last = name
    if arrayname:
        inwards -= step
        print >>wfp, tabify(baseindent + inwards) + "} %s[%s];" % (arrayname, arraydim)
    print >>wfp, tabify(baseindent) + "} %s;" % structname

def make_json_dumper(wfp):
    # Write the skeleton of a JSON dump corresponding to the table
    record = after is None
    # Elements of each tuple type except 'a':
    #   1. variable name,
    #   2. unscaled printf format
    #   3. wrapper for unscaled variable reference 
    #   4. scaled printf format
    #   5. wrapper for scaled variable reference
    # Elements of 'a' tuple:
    #   1. Name of array field
    #   2. None
    #   3. None
    #   4. None
    #   5. Name of length field
    tuples = []
    for (i, t) in enumerate(table):
        if '|' in t:
            fields = map(lambda s: s.strip(), t.split('|'))
            name = fields[4]
            ftype = fields[5]
            if after == name:
                record = True
                continue
            if before == name:
                record = False
                continue
            if ftype == 'x' or not record:
                continue
            fmt = r'\"%s\":' % name
            if ftype == 'u':
                tuples.append((name,
                               fmt+"%u", "%s",
                               None, None))
            elif ftype == 'e':
                tuples.append((name,
                               fmt+"%u", "%s",
                               fmt+r"\"%s\"", 'FOO[%s]'))
            elif ftype == 'i':
                tuples.append((name,
                               fmt+"%d", "%s",
                               None, None))
            elif ftype == 't':
                tuples.append((name,
                               fmt+r'\"%s\"', "%s",
                               None, None))
            elif ftype == 'b':
                tuples.append((name,
                               fmt+r'\"%s\"', "JSON_BOOL(%s)",
                               None, None))
            elif ftype[0] == 'd':
                print >>sys.stderr, "Cannot generate code for data members"
                sys.exit(1)
            elif ftype[0] == 'U':
                tuples.append((name,
                               fmt+"%u", "%s",
                               fmt+"%%.%sf" % ftype[1], '%s / SCALE'))
            elif ftype[0] == 'I':
                tuples.append((name,
                               fmt+"%d", "%s",
                               fmt+"%%.%sf" % ftype[1], '%s / SCALE'))
            elif ftype[0] == 'a':
                ftype = ftype[1:]
                if ftype[0] == '^':
                    lengthfield = last
                else:
                    lengthfield = "n" + name
                tuples.append((name, None, None, None, lengthfield))
            else:
                print >>sys.stderr, "Unknown type code", ftype
                sys.exit(1)
        last = name
    startspan = 0
    def scaled(i):
        return tuples[i][3] is not None
    def tslice(e, i):
        return map(lambda x: x[i], tuples[startspan:e+1])
    base = " " * 8
    step = " " * 4
    inarray = None
    header = "(void)snprintf(buf + strlen(buf), buflen - strlen(buf),"
    for (i, (var, uf, uv, sf, sv)) in enumerate(tuples):
        if uf == None:
            print >>wfp, base + "for (i = 0; i < %s.%s; i++) {" % (structname, sv)
            inarray = var
            base = " " * 12
            startspan = i+1
            continue        
        # At end of tuples, or if scaled flag changes, or if next op is array,
        # flush out dump code for a span of fields.
        if tuples[i+1][1] == None:
            endit = r',\"%s\":['
        elif i+1 == len(tuples):
            if not inarray:
                endit = '}\r\n'
        elif scaled(i) != scaled(i+1):
            endit =  '.",'
        else:
            endit = None
        if endit:
            if not scaled(i):
                print >>wfp, base + header
                print >>wfp, base + step + '"'+','.join(tslice(i,1)) + endit
                for (j, t) in enumerate(tuples[startspan:i+1]):
                    if inarray:
                        ref = structname + "." + inarray + "[i]." + t[0]
                    else:
                        ref = structname + "." + t[0]
                    wfp.write(base + step + t[2] % ref)
                    if j == i - startspan:
                        wfp.write(");\n")
                    else:
                        wfp.write(",\n")
            else:
                print >>wfp, base + "if (scaled)"
                print >>wfp, base + step + header
                print >>wfp, base + step*2 + '"'+','.join(tslice(i,3)) + endit
                for (j, t) in enumerate(tuples[startspan:i+1]):
                    if inarray:
                        ref = structname + "." + inarray + "[i]." + t[0]
                    else:
                        ref = structname + "." + t[0]
                    wfp.write(base + step*2 + t[4] % ref)
                    if j == i - startspan:
                        wfp.write(");\n")
                    else:
                        wfp.write(",\n")
                print >>wfp, base + "else"
                print >>wfp, base + step + header
                print >>wfp, base + step*2 + '"'+','.join(tslice(i,1)) + endit
                for (j, t) in enumerate(tuples[startspan:i+1]):
                    if inarray:
                        ref = structname + "." + inarray + "[i]." + t[0]
                    else:
                        ref = structname + "." + t[0]
                    wfp.write(base + step*2 + t[2] % ref)
                    if j == i - startspan:
                        wfp.write(");\n")
                    else:
                        wfp.write(",\n")
            startspan = i+1
    # If we were looking at a trailing array, close scope 
    if inarray:
        base = " " * 8
        print >>wfp, base + "}"
        print >>wfp, base + "if (buf[strlen(buf) - 1] == ',')"
        print >>wfp, base + step + "buf[strlen(buf)-1] = '\0';"
        print >>wfp, base + "(void)strlcat(buf, ']}\r\n', buflen - strlen(buf));"

def make_json_generator(wfp):
    # Write a stanza for jsongen.py.in describing how to generate a
    # JSON parser initializer from this table. You need to fill in
    # __INITIALIZER__ and default values after this is generated.
    extra = ""
    arrayname = None
    record = after is None
    print >>wfp, '''\
    {
        "initname" : "__INITIALIZER__",
        "headers": ("AIS_HEADER",),
        "structname": "%s",
        "fieldmap":(
            # fieldname    type        default''' % (structname,)
    for (i, t) in enumerate(table):
        if '|' in t:
            fields = map(lambda s: s.strip(), t.split('|'))
            name = fields[4]
            ftype = fields[5]
            if after == name:
                record = True
                continue
            if before == name:
                record = False
                continue
            if ftype == 'x' or not record:
                continue
            if ftype[0] == 'a':
                arrayname = name
                if arrayname.endswith("s"):
                    typename = arrayname[:-1]
                else:
                    typename = arrayname
                readtype = 'array'
                dimension = ftype[1:]
                if dimension[0] == '^':
                    lengthfield = last
                    dimension = dimension[1:]
                else:
                    lengthfield = "n" + arrayname
                extra = " " * 8
                print >>wfp, "            ('%s',%s 'array', (" % \
                      (arrayname, " "*(10-len(arrayname)))
                print >>wfp, "                ('%s_t', '%s', (" % (typename, lengthfield)
            else:
                # Depends on the assumption that the read code
                # always sees unscaled JSON.
                readtype = {
                    'u': "uinteger",
                    'U': "uinteger",
                    'e': "uinteger",
                    'i': "integer",
                    'I': "integer",
                    'b': "boolean",
                    't': "string",
                    'd': "string",
                    }[ftype[0]]
                default = {
                    'u': "'PUT_DEFAULT_HERE'",
                    'U': "'PUT_DEFAULT_HERE'",
                    'e': "'PUT DEFAULT HERE'",
                    'i': "'PUT_DEFAULT_HERE'",
                    'I': "'PUT_DEFAULT_HERE'",
                    'b': "\'false\'",
                    't': "None",
                    }[ftype[0]]
                print >>wfp, extra + "            ('%s',%s '%s',%s %s)," % (name,
                                                     " "*(10-len(name)),
                                                     readtype,
                                                     " "*(8-len(readtype)),
                                                     default)
            last = name
    if arrayname:
        print >>wfp, "                    )))),"
    print >>wfp, "        ),"
    print >>wfp, "    },"

if __name__ == '__main__':
    try:
        (options, arguments) = getopt.getopt(sys.argv[1:], "a:tc:s:d:S:E:r:")
    except getopt.GetoptError, msg:
        print "tablecheck.py: " + str(msg)
        raise SystemExit, 1
    generate = maketable = makestruct = makedump = readgen = all = False
    after = before = None
    for (switch, val) in options:
        if switch == '-a':
            all = True
            structname = val
        elif switch == '-c':
            generate = True
            structname = val
        elif switch == '-s':
            makestruct = True
            structname = val
        elif switch == '-t':
            maketable = True
        elif switch == '-d':
            makedump = True
            structname = val
        elif switch == '-r':
            readgen = True
            structname = val
        elif switch == '-S':
            after = val
        elif switch == '-E':
            before = val

    if not generate and not maketable and not makestruct and not makedump and not readgen and not all:
        print >>sys.stderr, "tablecheck.py: no mode selected"
        sys.exit(1)

    # First, read in the table.
    # Sets the following:
    #    table - the table lines
    #    widths - array of table widths
    #    trailing - bit length of the table or trailing array elemend 
    startline = int(arguments[0])
    table = []
    keep = False
    i = 0
    for line in sys.stdin:
        i += 1
        if i == startline:
            if line.startswith("|="):
                keep = True
            else:
                print >>sys.stderr, "Bad table start"
                sys.exit(1)
        elif line.startswith("|="):
            keep = False
        if keep:
            if line[0] == '|':
                fields = line.split("|")
                trailing = fields[1]
                fields[1] = " " * len(fields[1])
                line = "|".join(fields)
            table.append(line)
    table = table[2:]
    widths = []
    for line in table:
        fields = line.split('|')
        if '|' not in line:        # Continuation line
            widths.append('')
        elif fields[5][0] == 'a':     # Array boundary indicator
            widths.append(None)
        else:
            widths.append(fields[2].strip())
    if '-' in trailing:
        trailing = trailing.split('-')[1]
    trailing = str(int(trailing)+1)

    # Compute offsets for an AIVDM message breakdown, given the bit widths.
    offsets = []
    base = 0
    for w in widths:
        if w is None:
            offsets.append(`base`)
            base = 0
        elif w == '':
            offsets.append('')
        else:
            w = int(w)
            offsets.append("%d-%d" % (base, base + w - 1))
            base += w
    owidth = max(*map(len, offsets)) 
    for (i, off) in enumerate(offsets):
        offsets[i] += " " * (owidth - len(offsets[i]))

    # Here's where we generate useful output.
    if all:
        make_driver_code(open(structname + ".c", "w"))
        make_structure(open(structname + ".h", "w"))
        make_json_dumper(open(structname + "_json.c", "w"))
        make_json_generator(open(structname + ".py", "w"))
    elif maketable:
        correct_table(sys.stdout)
    elif generate:
        make_driver_code(sys.stdout)
    elif makestruct:
        make_structure(sys.stdout)
    elif makedump:
        make_json_dumper(sys.stdout)
    elif readgen:
        make_json_generator(sys.stdout)
# end
  
