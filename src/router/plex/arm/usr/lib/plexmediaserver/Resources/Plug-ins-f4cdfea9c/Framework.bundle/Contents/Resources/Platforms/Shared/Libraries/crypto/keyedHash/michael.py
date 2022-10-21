""" crypto.keyedHash.michael

    A reference implementation of the Michael Message Integrety Chek (MIC)
    that is defined in IEEE 802.11 Task Group 'i'

    Michael is a 64-bit MIC, with a design strength of 20 bits.

    Copyright (c) 2002 by Paul A. Lambert
    Read LICENSE.txt for license information.
"""
from struct import pack, unpack
from binascii_plus import *

class Michael:
    """ The Michael keyed hash as defined in IEEE 802.11i D2.0 """
    def __init__(self, key = None):
        self.name = 'Michael'
        self.blocksize      = 1  # single octets can be hashed by padding to raw block size
        self.raw_block_size = 4  # operates on 32 bits or 4 byte blocks
        self.digest_size    = 8  # MIC size of 64 bits or 8 bytes
        self.keySize        = 8  # key size is 8 octets
        self.strength       = 20
        if key != None:
            self.setKey(key)
            
    def __del__(self):
        self.setKey(8*chr(0))   # feable attempt to clear keys on exit

    def setKey(self,key):
        """ setKey(key) ... key is binary string """
        assert( len(key)== self.keySize), 'Key must be 8 octets'
        self._key = unpack('<II', key) # unpack into 2 32bit integers

    def __call__(self,data,more=None):
        return self.hash(data)

    def hash(self,data):
        """ Michael keyed hash """
        fullBlocks, extraOctets = divmod(len(data),4)
        paddedData = data + chr(0x5a) + chr(0)*(7-extraOctets)
        l, r = self._key
        for i in range(fullBlocks+2):
            mSub_i = unpack('<I', paddedData[i*4:i*4+4])[0]  # ith block as 32 bit integer
            l = l ^ mSub_i
            l, r = b(l,r)
        digest = pack('<II', l, r )
        return digest

    def update(self,data):
        raise 'No update method supported for Michael keyed hash'
    def digest(self):
        raise 'No digest method supported for Michael keyed hash'
    def final(self,data):
        raise 'No final method supported for Michael keyed hash'

def b(l,r):
    """ The 'b' block function for the IEEE 802.11i Michael Integrity Check """
    r ^= (((l<<17) & 0xffffffffL)|((l>>15) & 0x1ffffL))       # r = r ^ (l <<< 17)
    l  = (l+r) & 0xffffffffL                                  # l = (l+r) mod 2**32
    r ^= ((l & 0xff00ff00L)>>8)|((l & 0x00ff00ffL)<<8)        # r = r ^ XSWAP(l)
    l  = (l+r) & 0xffffffffL                                  # l = (l+r) mod 2**32
    r ^= (((l<<3) & 0xffffffffL) | ((l>>29)& 0x7))            # r  = r ^ (l <<< 3)
    l  = (l+r) & 0xffffffffL                                  # l = (l+r) mod 2**32
    r ^= (((l<<30L) & 0xffffffffL)|((l>>2) & 0x3fffffff))     # r  = r ^ (l >>> 2)
    l  = (l+r) & 0xffffffffL                                  # l = (l+r) mod 2**32
    return (l,r)



