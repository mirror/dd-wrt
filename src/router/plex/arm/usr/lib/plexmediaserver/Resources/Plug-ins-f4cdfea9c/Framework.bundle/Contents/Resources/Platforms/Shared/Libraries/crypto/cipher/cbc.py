""" crypto.cipher.cbc

    CBC mode of encryption for block ciphers.

    This algorithm mode wraps any BlockCipher to make a
    Cipher Block Chaining mode.

    Note !!!! auto IV uses python default random :-(
    should not be 'too bad' (tm) for this cbc applicaiton

    Copyright (c) 2002 by Paul A. Lambert
    Read LICENSE.txt for license information.
"""
from crypto.cipher.base import BlockCipher, padWithPadLen, noPadding
from crypto.errors      import EncryptError
from crypto.common      import xor
from random             import Random  # should change to crypto.random!!!


class CBC(BlockCipher):
    """ The CBC class wraps block ciphers to make cipher block chaining (CBC) mode
        algorithms.  The initialization (IV) is automatic if set to None.  Padding
        is also automatic based on the Pad class used to initialize the algorithm
    """
    def __init__(self, blockCipherInstance, padding = padWithPadLen()):
        """ CBC algorithms are created by initializing with a BlockCipher instance """
        self.baseCipher = blockCipherInstance
        self.name       = self.baseCipher.name + '_CBC'
        self.blockSize  = self.baseCipher.blockSize
        self.keySize    = self.baseCipher.keySize
        self.padding    = padding
        self.baseCipher.padding = noPadding()   # baseCipher should NOT pad!!
        self.r          = Random()            # for IV generation, currently uses
                                              # mediocre standard distro version     <----------------
        import time
        newSeed = time.ctime()+str(self.r)    # seed with instance location
        self.r.seed(newSeed)                  # to make unique
        self.reset()

    def setKey(self, key):
        self.baseCipher.setKey(key)

    # Overload to reset both CBC state and the wrapped baseCipher
    def resetEncrypt(self):
        BlockCipher.resetEncrypt(self)  # reset CBC encrypt state (super class)
        self.baseCipher.resetEncrypt()  # reset base cipher encrypt state

    def resetDecrypt(self):
        BlockCipher.resetDecrypt(self)  # reset CBC state (super class)
        self.baseCipher.resetDecrypt()  # reset base cipher decrypt state

    def encrypt(self, plainText, iv=None, more=None):
        """ CBC encryption - overloads baseCipher to allow optional explicit IV
            when iv=None, iv is auto generated!
        """
        if self.encryptBlockCount == 0:
            self.iv = iv
        else:
            assert(iv==None), 'IV used only on first call to encrypt'

        return BlockCipher.encrypt(self,plainText, more=more)

    def decrypt(self, cipherText, iv=None, more=None):
        """ CBC decryption - overloads baseCipher to allow optional explicit IV
            when iv=None, iv is auto generated!
        """
        if self.decryptBlockCount == 0:
            self.iv = iv
        else:
            assert(iv==None), 'IV used only on first call to decrypt'

        return BlockCipher.decrypt(self, cipherText, more=more)

    def encryptBlock(self, plainTextBlock):
        """ CBC block encryption, IV is set with 'encrypt' """
        auto_IV = ''
        if self.encryptBlockCount == 0:
            if self.iv == None:
                # generate IV and use
                self.iv = ''.join([chr(self.r.randrange(256)) for i in range(self.blockSize)])
                self.prior_encr_CT_block = self.iv
                auto_IV = self.prior_encr_CT_block    # prepend IV if it's automatic
            else:                       # application provided IV
                assert(len(self.iv) == self.blockSize ),'IV must be same length as block'
                self.prior_encr_CT_block = self.iv
        """ encrypt the prior CT XORed with the PT """
        ct = self.baseCipher.encryptBlock( xor(self.prior_encr_CT_block, plainTextBlock) )
        self.prior_encr_CT_block = ct
        return auto_IV+ct

    def decryptBlock(self, encryptedBlock):
        """ Decrypt a single block """

        if self.decryptBlockCount == 0:   # first call, process IV
            if self.iv == None:    # auto decrypt IV?
                self.prior_CT_block = encryptedBlock
                return ''
            else:
                assert(len(self.iv)==self.blockSize),"Bad IV size on CBC decryption"
                self.prior_CT_block = self.iv

        dct = self.baseCipher.decryptBlock(encryptedBlock)
        """ XOR the prior decrypted CT with the prior CT """
        dct_XOR_priorCT = xor( self.prior_CT_block, dct )

        self.prior_CT_block = encryptedBlock

        return dct_XOR_priorCT

