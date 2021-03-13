""" crypto.cipher.icedoll

    Modification of Rijndael to provide infinite error extension.
    The ith round of Rijndael is tapped and used to process the
    subsequent block.

    Changes to base Rijndael are marked with: '# --------------------------'

        For Rijndael with N rounds, normally ECB mode is C[i] = Ek(N,P[i])

        Modification is:
        Fi = Ek(t,P[i-1]) ; Fi, with i=0 is nonce or a fixed value
        C[i] = Fi^Ek(N,P[i]^Fi)

    Copyright (c) 2002 by Paul A. Lambert
    Read LICENSE.txt for license information.

    June 2002
    February 2003 -> discovered Ron Rivest's "Tweakable Block Ciphers"
                     http://theory.lcs.mit.edu/~rivest/publications.html
                     These are about the same concept ....
"""

from crypto.cipher.base import BlockCipherWithIntegrity, padWithPadLen, noPadding
from crypto.cipher.rijndael import  *
from binascii_plus import b2a_hex
from copy import deepcopy

class Icedoll(Rijndael):
    """ IceDoll encryption algorithm
        based on Rijndael, with added feedback for better integrity processing.
        Note - no integrity check is built into Icedoll directly
    """
    def __init__(self,key=None,padding=padWithPadLen(),keySize=16,blockSize=16,tapRound=6,extraRounds=6):
        """ key, keysize, blockSize same as Rijndael, tapROund is feedback tap, """
        self.tapRound    = tapRound     # <------- !!! change from Rijndael !!!
        self.extraRounds = extraRounds  # <------- !!! change from Rijndael !!!
        self.name        = 'ICEDOLL'
        self.keySize     = keySize
        self.strength    = keySize
        self.blockSize   = blockSize  # blockSize is in bytes
        self.padding     = padding    # change default to noPadding() to get normal ECB behavior

        assert( keySize%4==0 and NrTable[4].has_key(keySize/4)),'key size must be 16,20,24,29 or 32 bytes'
        assert( blockSize%4==0 and NrTable.has_key(blockSize/4)), 'block size must be 16,20,24,29 or 32 bytes'

        self.Nb = self.blockSize/4          # Nb is number of columns of 32 bit words
        self.Nk = keySize/4                 # Nk is the key length in 32-bit words
        self.Nr = NrTable[self.Nb][self.Nk]+extraRounds # <------- !!! change from Rijndael !!!

        if key != None:
            self.setKey(key)

    def setKey(self, key):
        """ Set a key and generate the expanded key """
        assert( len(key) == (self.Nk*4) ), 'Key length must be same as keySize parameter'
        self.__expandedKey = keyExpansion(self, key)
        self.reset()                   # BlockCipher.reset()

    def encryptBlock(self, plainTextBlock):
        """ Encrypt a block, plainTextBlock must be a array of bytes [Nb by 4] """
        self.state = self._toBlock(plainTextBlock)
        if self.encryptBlockCount == 0:   # first call, set frdd back
            self.priorFeedBack = self._toBlock(chr(0)*(4*self.Nb)) # <------- !!! change from Rijndael !!!
        AddRoundKey(self, self.priorFeedBack)                      # <------- !!! change from Rijndael !!!
        AddRoundKey(self, self.__expandedKey[0:self.Nb])
        for round in range(1,self.Nr):          #for round = 1 step 1 to Nr�1
            SubBytes(self)
            ShiftRows(self)
            MixColumns(self)
            if round == self.tapRound:
                        nextFeedBack = deepcopy(self.state)        # <------- !!! change from Rijndael !!!
            AddRoundKey(self, self.__expandedKey[round*self.Nb:(round+1)*self.Nb])
        SubBytes(self)
        ShiftRows(self)
        AddRoundKey(self, self.__expandedKey[self.Nr*self.Nb:(self.Nr+1)*self.Nb])
        AddRoundKey(self, self.priorFeedBack)                      # <------- !!! change from Rijndael !!!
        self.priorFeedBack = nextFeedBack                          # <------- !!! change from Rijndael !!!
        return self._toBString(self.state)

    def decryptBlock(self, encryptedBlock):
        """ decrypt a block (array of bytes) """
        self.state = self._toBlock(encryptedBlock)
        if self.decryptBlockCount == 0:   # first call, set frdd back
            self.priorFeedBack = self._toBlock( chr(0)*(4*self.Nb) ) # <------- !!! change from Rijndael !!!
        AddRoundKey(self, self.priorFeedBack)                        # <------- !!! change from Rijndael !!!
        AddRoundKey(self, self.__expandedKey[self.Nr*self.Nb:(self.Nr+1)*self.Nb])
        for round in range(self.Nr-1,0,-1):
            InvShiftRows(self)
            InvSubBytes(self)
            AddRoundKey(self, self.__expandedKey[round*self.Nb:(round+1)*self.Nb])
            if round == self.tapRound:
                        nextFeedBack = deepcopy(self.state)          # <------- !!! change from Rijndael !!!
            InvMixColumns(self)
        InvShiftRows(self)
        InvSubBytes(self)
        AddRoundKey(self, self.__expandedKey[0:self.Nb])
        AddRoundKey(self, self.priorFeedBack) # <------- !!! change from Rijndael !!!
        self.priorFeedBack = nextFeedBack     # <------- !!! change from Rijndael !!!
        return self._toBString(self.state)


