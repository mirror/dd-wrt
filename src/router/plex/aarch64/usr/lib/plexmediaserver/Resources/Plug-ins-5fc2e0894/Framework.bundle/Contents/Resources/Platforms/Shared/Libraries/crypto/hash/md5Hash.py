"""   md5Hash.py
      Wrapper for python md5 module to support crypo module standard interface
"""
import md5
from crypto.hash.hash import Hash

class MD5(Hash):

    def __init__( self ):
        self.name = 'MD5'
        self.blocksize      = 1   # single octets can be hashed by padding to raw block size
        self.raw_block_size = 64  # MD5 operates on 512 bits or 64 byte blocks
        self.digest_size    = 16  # or 128 bits
        self.reset()

    def reset(self):
        self.pymd5 = md5.new()
    def update(self,data):
        """ Update the md5 object with the string arg. Repeated calls are
            equivalent to a single call with the concatenation of all the
            arguments: m.update(a); m.update(b) is equivalent to m.update(a+b).
        """
        self.pymd5.update(data)
    def digest(self):
        """ Return the digest of the strings passed to the update()
            method so far. This is a 20-byte string which may contain
            non-ASCII characters, including null bytes.
        """
        return self.pymd5.digest()


