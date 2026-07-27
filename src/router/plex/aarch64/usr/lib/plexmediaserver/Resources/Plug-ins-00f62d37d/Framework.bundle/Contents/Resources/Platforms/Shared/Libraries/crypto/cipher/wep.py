""" crypto.cipher.wep

    The WEP encryption algorithm used in IEEE 802.11

    Copyright (c) 2002 by Paul A. Lambert
    Read LICENSE.txt for license information.

    September 2002
"""
from crypto.cipher.arc4 import ARC4
from crypto.errors import IntegrityCheckError, BadKeySizeError
from zlib   import crc32
from struct import pack

class WEP:
    """ WEP Stream Cipher Algorithm
        Automatically adds and removes IV and KeyId
    """
    def __init__(self, key=None, keyId=None):
        """ key -> octet string for key """
        self.name        = 'WEP'
        self.strength    = None # depends on keySize
        self.arc4 = ARC4()       # base algorithm
        self.__key = [None,None,None,None]  # four possible keys, initialize to invalid keys
        self.encryptHeaderSize = 4
        self.setCurrentKeyId(keyId)
        if key != None:
            self.setKey(key)

    def setKey(self, key, keyId=None):
        """ Set key, key string is typically 5 or 13 octets long
        """
        if not(len(key) in (5,13)):
            raise BadKeySizeError,'Key not valid size of 5 or 13 octets'
        if keyId != None :
            self.setCurrentKeyId(keyId)
        self.__key[self.currentKeyId] = key
        self.keySize = len(key)
        self.strength = self.keySize * 8

    def setCurrentKeyId(self, keyId):
        if keyId == None :
            self.currentKeyId = 0
        elif (0<=keyId<4):
            self.currentKeyId = keyId
        else:
            raise 'WEP keyId must be value 0, 1, 2 or 3'

    def encrypt(self, plainText, iv, keyId=None):
        """ Encrypt a string and return a binary string
            Adds WEP encryption header and crc
        """
        assert(len(iv)==3),'Wrong size WEP IV'
        if keyId != None :
            self.setCurrentKeyId(keyId)
        assert(self.__key[self.currentKeyId]!=None), 'Must set key for specific keyId before encryption'
        self.arc4.setKey( iv + self.__key[self.currentKeyId] )
        crc = pack('<I',crc32(plainText))
        cipherText = self.arc4.encrypt(plainText+crc)
        # add header that contains IV
        cipherText = iv + chr((self.currentKeyId<<6)) + cipherText
        return cipherText

    def decrypt(self, cipherText):
        """ Decrypt a WEP packet, assumes WEP 4 byte header on packet """
        iv = cipherText[:3]
        self.currentKeyId = (ord(cipherText[3])&0xC0)>>6
        assert(self.__key[self.currentKeyId]!=None), 'Must set key for specific keyId before encryption'
        self.arc4.setKey( iv + self.__key[self.currentKeyId] )
        plainText = self.arc4.decrypt(cipherText[self.encryptHeaderSize:])
        if plainText[-self.encryptHeaderSize:] == pack('<I',crc32(plainText)):  # check data integrity
            raise IntegrityCheckError, 'WEP Integrity Check Error'
        return plainText[:-4]


