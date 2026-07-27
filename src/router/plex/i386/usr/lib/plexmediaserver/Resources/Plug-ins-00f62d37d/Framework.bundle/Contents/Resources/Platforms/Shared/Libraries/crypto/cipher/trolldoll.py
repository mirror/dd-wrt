""" crypto.cipher.trolldoll

    Modification to Icedoll to take advantage of the better error extension
    and provide a IV for randomization of the output and integrity checking.

    IV is simply prepended to plaintext.
    Integrity check is appended to the end of the plain text as
    zeros with count of the blocks in the ciphertext.

    Note !!!! auto IV uses python default random :-(
    should not be 'too bad' (tm) for this applicaiton

    ALso ... currently just IV .... in test ..

    Copyright (c) 2002 by Paul A. Lambert
    Read LICENSE.txt for license information.
"""

from crypto.cipher.icedoll import  Icedoll
from crypto.errors      import IntegrityCheckError
from random import Random  # should change to crypto.random!!!

class Trolldoll(Icedoll):
    """ Trolldoll encryption algorithm
        based on Icedoll, which is based on Rijndael
        Trolldoll adds an 'IV' and integrity checking to Icedoll
    """
    def __init__(self,key=None,keySize=32,blockSize=32,tapRound=6,extraRounds=6,micSize=16,ivSize=16):
        """  """
        Icedoll.__init__(self,key=None,keySize=32,blockSize=32,tapRound=6,extraRounds=6)
        self.name    = 'TROLLDOLL'
        self.micSize = micSize
        self.ivSize  = ivSize
        self.r       = Random()            # for IV generation
        import time
        newSeed = time.ctime()+str(self.r)    # seed with instance location
        self.r.seed(newSeed)                  # to make unique
        self.reset()

    def reset(self):
        Icedoll.reset(self)
        self.hasIV = None

    def _makeIV(self):
        return self.ivSize*'a'

    def _makeIC(self):
        """ Make the integrity check """
        return self.micSize*chr(0x00)

    def _verifyIC(self,integrityCheck):
        """ Verify the integrity check """
        if self.micSize*chr(0x00) == integrityCheck :
            return 1  # matches
        else:
            return 0  # fails

    def encrypt(self, plainText, more=None):
        """ """
        if not(self.hasIV):  # On first call to encrypt put in an IV
            plainText = self._makeIV() + plainText # add the 'IV'
            self.hasIV = 1
        if more == None:    # on last call to encrypt append integrity check
            plainText = plainText + self._makeIC()
        return Icedoll.encrypt(self, plainText, more=more)

    def decrypt(self, cipherText, more=None):
        """ Decrypt cipher text, Icedoll automatically removes
            prepended random bits used as IV.
            Note - typically IV is directly used as the first
            cipher text.  Here the IV is prepended to the plaintext
            prior to encryption and removed on decryption.
        """
        plainText = Icedoll.decrypt(self, cipherText, more=more)
        if not(self.hasIV):  # on first call to decrypt remove IV
            plainText = plainText[self.ivSize:] # remove the IV
            self.hasIV = 1
        if more == None:    # on last call to encrypt append integrity check
            if not(self._verifyIC(plainText[-self.micSize:])) :
                raise IntegrityCheckError, 'Trolldoll MIC Failure, bad key or modified data'
            plainText = plainText[:-self.micSize]  # trim off the integrity check
        return plainText


