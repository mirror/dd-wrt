""" crypto.entropy.pagingEntropy

    Uses variations in disk access time to generator entropy.  A long string is
    created that is bigger than available memory. Virtual memory access' create
    random variations in retrieval time.

    Just an experiment, not recommended for use at this time.

    Copyright (c) 2002 by Paul A. Lambert
    Read LICENSE.txt for license information.
"""
import struct

class PagingEntropyCollector:
    """ collect entropy from memory paging """
    def __init__(self, memSize=500000000L):            #? how should this be picked?
        """ Initialize paging entropy collector,
            memSize must be larger than allocated memory """
        self.size     = memSize
        self.memBlock = self.size*chr(0) # long string of length self.size
        self.index    = 0
        import random
        self.rand     = random.Random(1555551)

    def randomBytes(self, numberOfBytes, secondsPerBit=.05):
        byteString = ''
        for b in range(numberOfBytes):
            aByte = 0
            for bit in range(8):
                aByte = aByte << 1
                aByte = aByte ^ self.collectBit(secondsPerBit)
            byteString += chr(aByte)
        return byteString

    def collectBit(self, secondsPerBit=1.0):
        """ Collect an entropy bit by jumping around a long string and
            collecting the variation in time and number of samples per
            time interval """
        t1 = time()
        count = 0
        while (time()-t1) < secondsPerBit:           # seconds per sample
            # use random to sample various virtual memory locations
            sample = self.memBlock[int(self.rand.random()*self.size)]
            count += 1
        randomBit = intToParity(count)^floatToParity(time()-t1)
        return randomBit

def intToParity(integer):
    s = struct.pack('i',integer)
    parity = 0
    for character in s:
        byte = ord(character)
        parity = parity^(0x01&(byte^(byte>>1)^(byte>>2)^(byte>>3)^(byte>>4)^(byte>>5)^(byte>>6)^(byte>>7)))
    return parity

def floatToParity(float):
    s = struct.pack('d',float)
    parity = 0
    for character in s:
        byte = ord(character)
        parity = parity^(0x01&(byte^(byte>>1)^(byte>>2)^(byte>>3)^(byte>>4)^(byte>>5)^(byte>>6)^(byte>>7)))
    return parity

if __name__ == "__main__":
    from binascii import b2a_hex
    e = PagingEntropyCollector()
    for i in range(20):
        e.rand.seed(1)        # make each sample set the same to allow examination of statistics
        print b2a_hex( e.randomBytes(16) )






