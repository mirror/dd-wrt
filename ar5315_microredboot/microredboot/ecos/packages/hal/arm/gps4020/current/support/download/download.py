#! /usr/bin/env python

#
# Copyright (C) 2003, MLB Associates
#

# This program is used to read a Motorola S-record file and
# download it using the GDB protocol.

import os, string, sys, time

trace = open("/tmp/download.trace", "w")

#
# Use up some time
#
def spin():
    j = 0
    for i in range(0,200):
        j = j + 1

#
# Compute the checksum for a string
#
def cksum(str):
#    sys.stderr.write("cksum %s\n" % str)
    sum = 0
    cs = str[1:]
    while cs:
        sum = sum + ord(cs[:1])
        cs = cs[1:]
    return sum & 0xFF

#
# Send a string via the GDB protocol.  Note: this routine
# computes and adds the checksum before starting.
#
def send(str):
    str = str + "#%02x" % cksum(str)
#    trace.write("ready to send: %s\n" % str)
#    trace.flush()
    while 1:
        s = str
        while s:
            os.write(1, s[:1])
            spin()
            # time.sleep(0.001)
            s = s[1:]
        c = os.read(0, 1)
        if c <> '+':
            trace.write("~ACK: %c\n" % c)
            trace.write("sent: %s\n" % str)
            trace.flush()
            continue
        res = ''
        while 1:
            c = os.read(0, 1)
            if c == '#': break
            res = res + c
            # trace.write("ACK: %c, res: %s\n" % (c, res))
            # trace.flush()
        # trace.write("res = %s\n" % res)
        # trace.flush()
        csum = cksum(res)
        cs = os.read(0, 1)
        cs = cs + os.read(0, 1)
        sum = string.atoi(cs, 16)
        if csum <> sum:
            os.write(1, '-')
            trace.write("RES = %s, sum: %x/%x\n" % (res, csum, sum))
            trace.write("sent: %s\n" % str)
            trace.flush()
            continue
        os.write(1, '+')
        trace.flush()
        return

#
# Process a stream of S-records, supplied by 'readline()'
#
def download(readline):
    # send("$Hc-1")
    # send("$Hg0")
    last_addr = 0
    while 1:
        line = readline()
        if not line: break
        if line[0] <> 'S':
            raise ("Invalid input:" + line)
        if line[1] in "123":
            len = string.atoi(line[2:4],16)
            an = ord(line[1]) - ord('1') + 2
            ae = 4 + (an*2)
            addr = string.atoi(line[4:ae],16)
            #print "len = %d, addr = 0x%x " % (len, addr)
            len = len - (an+1)
            line = line[ae:]
            out = "$M%x,%x:" % (addr, len)
            for i in range(0,len):
                val = string.atoi(line[:2],16)
                #print "val = 0x%x" % val
                line = line[2:]
                out = out + "%02x" % val
            if (addr - last_addr) >= 0x400:
                last_addr = addr
                sys.stderr.write("0x%x\n" % addr)
            send(out)
        elif line[1] in "789":
            len = string.atoi(line[2:4],16)
            eos = 10
            if line[1] == '7':
                eos = 12
            addr = string.atoi(line[4:12],16)
            #print "len = %d, addr = 0x%x " % (len, addr)
            len = len - 4
            line = line[eos:]
            out = "$P40=%08x" % addr
            sys.stderr.write("Set PC = 0x%x\n" % addr)
            send(out)
    # This command starts the program
    send("$c#63")
        
if __name__ == '__main__':                     # testing
    import sys
    if len(sys.argv) > 1: download(open(sys.argv[1]).readline)
    else: download(sys.stdin.readline)
