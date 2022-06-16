""" crypto.hash.sha1Hash

    Wrapper for python sha module to support crypo module standard interface

    Copyright (c) 2002 by Paul A. Lambert
    Read LICENSE.txt for license information.
"""
import sha
from crypto.hash.hash import Hash

class SHA1(Hash):

    def __init__( self ):
        self.name = 'SHA1'
        self.blocksize      = 1   # single octets can be hashed by padding to raw block size
        self.raw_block_size = 64  # SHA1 operates on 512 bit / 64 byte blocks
        self.digest_size    = 20  # or 160 bits
        self.reset()

    def reset(self):
        self.pysha1 = sha.new()

    def update(self,data):
        """ Update the sha object with the string arg. Repeated calls are
            equivalent to a single call with the concatenation of all the
            arguments: m.update(a); m.update(b) is equivalent to m.update(a+b).
        """
        self.pysha1.update(data)

    def digest(self):
        """ Return the digest of the strings passed to the update()
            method so far. This is a 20-byte string which may contain
            non-ASCII characters, including null bytes.
        """
        return self.pysha1.digest()


