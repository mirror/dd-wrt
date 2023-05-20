""" crypto.cipher.base


    Base 'BlockCipher' and Pad classes for cipher instances.
    BlockCipher supports automatic padding and type conversion. The BlockCipher
    class was written to make the actual algorithm code more readable and
    not for performance.

    Copyright (c) 2002 by Paul A. Lambert
    Read LICENSE.txt for license information.

    2002-04-25   changed block input
"""
from crypto.errors import DecryptNotBlockAlignedError
from crypto.keyedHash.pbkdf2 import pbkdf2

class BlockCipher:
    """ Block ciphers """
    def __init__(self):
        self.reset()

    def reset(self):
        self.resetEncrypt()
        self.resetDecrypt()        
    def resetEncrypt(self):
        self.encryptBlockCount = 0
        self.bytesToEncrypt = ''
    def resetDecrypt(self):
        self.decryptBlockCount = 0
        self.bytesToDecrypt = ''

    def setPassphrase(self,passphrase):
        """ Use pbkdf2 to hash passphrase into a key """
        self.setKey(  pbkdf2( passphrase, self.name, 4096, self.keySize) )

    def encrypt(self, plainText, more = None):
        """ Encrypt a string and return a binary string """
        self.bytesToEncrypt += plainText  # append plainText to any bytes from prior encrypt
        numBlocks, numExtraBytes = divmod(len(self.bytesToEncrypt), self.blockSize)
        cipherText = ''
        for i in range(numBlocks):
            bStart = i*self.blockSize
            ctBlock = self.encryptBlock(self.bytesToEncrypt[bStart:bStart+self.blockSize])
            self.encryptBlockCount += 1
            cipherText += ctBlock
        if numExtraBytes > 0:        # save any bytes that are not block aligned
            self.bytesToEncrypt = self.bytesToEncrypt[-numExtraBytes:]
        else:
            self.bytesToEncrypt = ''

        if more == None:   # no more data expected from caller
            finalBytes = self.padding.addPad(self.bytesToEncrypt,self.blockSize)
            if len(finalBytes) > 0:
                ctBlock = self.encryptBlock(finalBytes)
                self.encryptBlockCount += 1
                cipherText += ctBlock
            self.resetEncrypt()
        return cipherText
    
    def decrypt(self, cipherText, more = None):
        """ Decrypt a string and return a string """
        self.bytesToDecrypt += cipherText  # append to any bytes from prior decrypt

        numBlocks, numExtraBytes = divmod(len(self.bytesToDecrypt), self.blockSize)
        if more == None:  # no more calls to decrypt, should have all the data
            if numExtraBytes  != 0:
                raise DecryptNotBlockAlignedError, 'Data not block aligned on decrypt'

        # hold back some bytes in case last decrypt has zero len
        if (more != None) and (numExtraBytes == 0) and (numBlocks >0) :
            numBlocks -= 1
            numExtraBytes = self.blockSize

        plainText = ''
        for i in range(numBlocks):
            bStart = i*self.blockSize
            ptBlock = self.decryptBlock(self.bytesToDecrypt[bStart : bStart+self.blockSize])
            self.decryptBlockCount += 1
            plainText += ptBlock

        if numExtraBytes > 0:        # save any bytes that are not block aligned
            self.bytesToEncrypt = self.bytesToEncrypt[-numExtraBytes:]
        else:
            self.bytesToEncrypt = ''

        if more == None:         # last decrypt remove padding
            plainText = self.padding.removePad(plainText, self.blockSize)
            self.resetDecrypt()
        return plainText

class BlockCipherWithIntegrity(BlockCipher):
    """ Base class for encryption with integrity checking
        just a holding place for now ... """
    def __init__(self, authData, plainText):
        self.reset()

class Pad:
   def __init__(self):
       pass              # eventually could put in calculation of min and max size extension

class padWithPadLen(Pad):
    """ Pad a binary string with the length of the padding """

    def addPad(self, extraBytes, blockSize):
        """ Add padding to a binary string to make it an even multiple
            of the block size """
        blocks, numExtraBytes = divmod(len(extraBytes), blockSize)
        padLength = blockSize - numExtraBytes
        return extraBytes + padLength*chr(padLength)
    
    def removePad(self, paddedBinaryString, blockSize):
        """ Remove padding from a binary string """
        if not(0<len(paddedBinaryString)):
            raise DecryptNotBlockAlignedError, 'Expected More Data'
        return paddedBinaryString[:-ord(paddedBinaryString[-1])]

class noPadding(Pad):
    """ No padding. Use this to get ECB behavior from encrypt/decrypt """

    def addPad(self, extraBytes, blockSize):
        """ Add no padding """
        return extraBytes

    def removePad(self, paddedBinaryString, blockSize):
        """ Remove no padding """
        return paddedBinaryString

class padWithZeros(Pad):
    """ Zero padding. Used in CBC_MAC processing """

    def addPad(self, extraBytes, blockSize):
        """ Add padding to a binary string to make it an even multiple
            of the block size """
        blocks, numExtraBytes = divmod(len(extraBytes), blockSize)
        padLength = blockSize - numExtraBytes
        return extraBytes + padLength*chr(0x00)

    def removePad(self, paddedBinaryString, blockSize):
        """ Remove no padding, you really should, but no way to tell padding size """
        return paddedBinaryString
