""" crypto.entropy.prn_rijndael

    A Psudeo Random Number Generator based on Rijndael_256k_256b
    The algorithm is based on Section 13.4 of:
    "AES Proposal: Rijndael", Joan Daemen, Vincent Rijmen

    Copyright (c) 2002 by Paul A. Lambert
    Read LICENSE.txt for license information.
"""

from crypto.cipher.rijndael import Rijndael
from crypto.cipher.base     import noPadding
from binascii import b2a_hex

defaultSeed = "An arbirary 32 byte string!!!!!!"  # can be changed by the truely paranoid

class PRN_Rijndael:
    """ A Psudeo Random Number Generator based on Rijndael_256k_256b
        The algorithm is based on Section 13.4 of:
        "AES Proposal: Rijndael", Joan Daemen, Vincent Rijmen
    """
    def __init__(self, seed=defaultSeed):
        self.__algorithm = Rijndael(padding=noPadding(),keySize=32, blockSize=32)
        self.reset()
        self.reseed(seed)

    def reset(self):
        self.__algorithm.setKey(self.__algorithm.keySize*chr(0))      # set key to all zeros
        self.__state = self.__algorithm.blockSize*chr(0)              # a single block of zeros

    def reseed(self,seed):
        while len(seed) > 0 :
            if len(seed) < self.__algorithm.blockSize:
                block = seed + (self.__algorithm.blockSize-len(seed))*chr(0)
                seed = ''
            else:
                block =  seed[:self.__algorithm.blockSize]
                seed = seed[self.__algorithm.blockSize:]
            self.__algorithm.setKey( self.__algorithm.encrypt(block) )

    def getBytes(self, numBytes):
        """ Return a psuedo random byte string of length numBytes """
        bytes = ''
        while len(bytes)< numBytes :
            bytes = bytes + self.getSomeBytes()
        return bytes[:numBytes]     # truncate to the requested length

    def getSomeBytes(self):
        """ Psuedo random bytes are generated 16 bytes at a time.
            The state is updated by applying Rijndael using the Cipher
            Key. The first 128 bits of the state are output as a �pseudorandom number�.
        """
        self.__state = self.__algorithm.encrypt(self.__state)
        return self.__state[:16]








