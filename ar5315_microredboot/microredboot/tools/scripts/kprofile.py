#!/usr/bin/python
#
# A script that takes information in /proc/ksyms format
# and a file containing program counter samples to
# produce a sorted histogram of calls.
#
# At the time of this writing, a line in /proc/ksysms looked
# like:
#
#   f88fc290 pcmcia_modify_window_R616fce8a	[pcmcia_core]
#
# The PC sample file is a file containing one hex address per
# line. e.g.
#
#  0xf88fc290
#  ...
#
# $Id: //depot/sw/releases/linuxsrc/tools/scripts/kprofile.py#3 $
# $File: //depot/sw/releases/linuxsrc/tools/scripts/kprofile.py $
# $Author: zhifeng $
# $DateTime: 2005/08/09 13:42:05 $
# $Change: 146233 $
#

import sys
import string

class KernelSyms:
    def __init__(self, fileName):
        try:
            f = open(fileName)
        except:
            raise
        lines = f.readlines()
        f.close()

        lines.sort()

        #
        # Normalize the list to have address length values
        # 
        self.symtab = []
        last   = []
        for l in lines:
            symstr = l.split()
            addr   = long('0x'+symstr[0], 16)
            if last == []:
                length = addr - 4
                addr   = 0
                key    = "Unknown"
            else:
                length  = 0
                last[1] = addr - last[0] - 4
                key = symstr[1]
                
            if len(symstr) == 3:
                key = symstr[2] + ' ' + key

            sym = [addr, length, key]
            last = sym
            self.symtab.append(sym)
        sym[1] = 0xFFFF                # Fix the last entry
        
    def findSymbol(self, addrStr):
        addr = long(addrStr, 16)
        for s in self.symtab:
            if (addr >= s[0]) and (addr <= s[0]+s[1]):
                return s
        return None

    def printSymbol(self, s):
        print "%s\t%s\t\t%s" % (hex(s[0]), hex(s[1]), s[2]),
        print ""

    def printTable(self):
        for s in self.symtab:
            self.printSymbol(s)



#####################################
if __name__ == '__main__':

    if len(sys.argv) < 3:
        print "Usage: %s <kernel symbol file> <PC sample file>" % sys.argv[0]
        sys.exit(1)

    symtab = KernelSyms(sys.argv[1])

    #
    # Read the sample file
    #
    try:
        f = open(sys.argv[2])
    except:
        raise
    addresses = f.readlines()
    f.close()

    #
    # Create the histogram (This takes the time!)
    #
    hist = {}
    for addr in addresses:
        s = symtab.findSymbol(addr)
        if s:
            if not s[2] in hist.keys():
                hist[s[2]] = 1
            else:
                hist[s[2]] += 1

    #
    # Create and sort the results list
    #
    result = []
    for k, v in hist.iteritems():
        result.append([v, k])
    result.sort()
    result.reverse()

    #
    # Show the results
    #
    print "Calls\tSymbol"
    print "-----\t------------------------------"
    for r in result:
        print "%d\t%s" % (r[0], r[1])
