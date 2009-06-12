#! /usr/bin/env python

#
# Copyright (C) 2003, MLB Associates
#

# This program is used to download a binary file, using the
# raw [minimal] protocol used by the hardware bootstrap on
# the GPS-4020

import os, string, sys, time

#
# Use up some time
#
def spin():
    j = 0
    for i in range(0,2000):
        j = j + 1

#
# Send a string via the RAW protocol.
#
def send(str):
    while 1:
        s = str
        while s:
            c = s[:1]
            os.write(1, c)
            spin()
            # time.sleep(0.001)
            s = s[1:]
        return

#
# Process a stream of S-records, supplied by 'read()'
#
def download(read):
    line = read(16384)
    if not line: return
    length =  len(line)
    # Build up a length descriptor, in big-endian layout
    data_length = chr(length >> 16) + chr((length >> 8) & 0xFF) + chr(length & 0xFF)
    send(data_length)
    send(line)
        
if __name__ == '__main__':                     # testing
    import sys
    if len(sys.argv) > 1: download(open(sys.argv[1],'rb').read)
    else: download(sys.stdin.read)
