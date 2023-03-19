"""  hmacHash.py

    Implemention of Request for Comments: 2104
    HMAC: Keyed-Hashing for Message Authentication

    HMAC is a mechanism for message authentication
    using cryptographic hash functions. HMAC can be used with any
    iterative cryptographic hash function, e.g., MD5, SHA-1, in
    combination with a secret shared key.  The cryptographic strength of
    HMAC depends on the properties of the underlying hash function.

    This implementation of HMAC uses a generic cryptographic 'hashFunction'
    (self.H). Hash functions must conform to the crypto.hash method
    conventions and are not directly compatible with the Python sha1 or md5 algorithms.

    [IETF]  RFC 2104 "HMAC: Keyed-Hashing for Message Authentication"
    
    >>>key = '\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00'
    >>>keyedHashAlg = HMAC(SHA1, key)
    >>>result = keyedHashAlg(data)
"""
from crypto.hash.hash     import Hash

class HMAC(Hash):
    """    To compute HMAC over the data `text' we perform
                    H(K XOR opad, H(K XOR ipad, text))
    """
    def __init__(self, hashFunction, key = None):
        """ initialize HMAC with hashfunction and optionally the key """
        # should check for right type of function
        self.H       = hashFunction()      # a new instance for inner hash
        self.H_outer = hashFunction()      # separate outer context to allow intermediate digests
        self.B = self.H.raw_block_size     # in bytes, note - hash block size typically 1
                                           # and raw_block_size much larger
                                           # e.g. raw_block_size is 64 bytes for SHA1 and MD5
        self.name = 'HMAC_'+self.H.name
        self.blocksize      = 1   # single octets can be hashed by padding to raw block size
        self.raw_block_size = self.H.raw_block_size
        self.digest_size    = self.H.digest_size
        if key != None:
            self.setKey(key)
        else:
            self.keyed = None

    def setKey(self,key):
        """ setKey(key) ... key is binary string """
        if len(key) > self.B:   # if key is too long then hash it
            key = self.H(key)   # humm... this is odd, hash can be smaller than B
        else:                   # should raise error on short key, but breaks tests :-(
            key =key + (self.B-len(key)) * chr(0)
        self.k_xor_ipad = ''.join([chr(ord(bchar)^0x36) for bchar in key])
        self.k_xor_opad = ''.join([chr(ord(bchar)^0x5C) for bchar in key])
        self.keyed = 1
        self.reset()

    def reset(self):
        self.H.reset()
        if self.keyed == None :
            raise 'no key defined'
        self.H.update(self.k_xor_ipad) # start inner hash with key xored with ipad
                                       # outer hash always called as one full pass (no updates)
    def update(self,data):
        if self.keyed == None :
            raise 'no key defined'
        self.H.update(data)
    def digest(self):
        if self.keyed == None :
            raise 'no key defined'
        return self.H_outer(self.k_xor_opad+self.H.digest())

from crypto.hash.sha1Hash import SHA1
class HMAC_SHA1(HMAC):
    """ Predefined HMAC built on SHA1 """
    def __init__(self, key = None):
        """ optionally initialize with key """
        HMAC.__init__(self,SHA1,key)

from crypto.hash.md5Hash  import MD5
class HMAC_MD5(HMAC):
    """ Predefined HMAC built on SHA1 """
    def __init__(self, key = None):
        """ optionally initialize with key """
        HMAC.__init__(self,MD5,key)
