"""   crypto.cipher.hash
      Base class for cryptographic hash algorithms
      An alternate interface (no 'new').
      Copyright (c) 2002 by Paul A. Lambert.
"""
from binascii import b2a_hex

class Hash:
    def __init__( self ):
        raise 'must overload'
    def reset(self):
        raise 'must overload'
    def __call__(self, data, more=None):
        return self.hash(data,more)
    def hash(self,data,more=None):
        self.update(data)
        digest = self.digest()
        if more==None:
            self.reset()  # no more data, reset
        return digest
    def update(self,data):
        """ Update the hash object with the data. Repeated calls are
            equivalent to a single call with the concatenation of all the
            arguments: m.update(a); m.update(b) is equivalent to m.update(a+b).
        """
        raise 'must overload'
    def digest(self):
        raise 'must overload'
    def final(self,data):
        return self.hash(data)
    def hexdigest(self):
        """ Return the digest of the data in ascii-hex format """
        return b2a_hex(self.digest())
#   def hexdigest(self):       not supported yet
#   def copy(self):            not supported yet ... may change

