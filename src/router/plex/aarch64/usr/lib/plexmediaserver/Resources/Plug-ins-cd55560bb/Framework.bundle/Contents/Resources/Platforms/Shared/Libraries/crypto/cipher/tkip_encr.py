""" crypto.cipher.tkip_encr

    TKIP encryption from IEEE 802.11 TGi

    TKIP uses WEP (ARC4 with crc32) and key mixing
    This is only the encryption and not the Michael integrity check!

    Copyright (c) 2002 by Paul A. Lambert
    Read LICENSE.txt for license information.

    November 2002
"""
from crypto.cipher.arc4 import ARC4
from zlib   import crc32
from struct import pack
from crypto.keyedHash.tkip_key_mixing import TKIP_Mixer
from crypto.errors import BadKeySizeError, IntegrityCheckError
from binascii_plus import *

class TKIP_encr:
    """ TKIP Stream Cipher Algorithm without the Michael integrity check

        TKIP encryption on an MPDU using WEP with a longer 'iv'
        and the TKIP key mixing algorithm .  This does NOT include
        the Michael integrity algorithm that operates on the MSDU data.
    """
    def __init__(self, key=None, transmitterAddress=None, keyID=None):
        """ Initialize TKIP_encr, key -> octet string for key """
        assert(keyID==0 or keyID==None), 'keyID should be zero in TKIP'
        self.keyId = 0
        self.name  = 'TKIP_encr'
        self.strength    = 128
        self.encryptHeaderSize = 8        # used to skip octets on decrypt
        self.arc4 = ARC4()                # base algorithm
        self.keyMixer = TKIP_Mixer(key,transmitterAddress)
        if key != None:                   # normally in base init, uses adjusted keySize
            self.setKey(key)
        if transmitterAddress!=None:
            self.setTA(transmitterAddress)

    def setKey(self, key, ta=None):
        """ Set key, key string is 16 octets long """
        self.keyMixer.setKey(key)
        if ta!=None:
            self.setTA(ta)

    def setTA(self, transmitterAddress):
        """ Set the transmitter address """
        self.keyMixer.setTA(transmitterAddress)

    def _getIVandKeyID(self, cipherText):
        """ Parse the TKIP header to get iv and set KeyID
            iv is returned as octet string and is little-endian!!!
        """
        assert(ord(cipherText[3])& 0x20),'extIV SHOULD be set in TKIP header'
        self.setCurrentKeyID = (ord(cipherText[3])&0xC0)>>6
        return cipherText[:3] + cipherText[5:9] # note iv octets are little-endian!!!

    def _makeARC4key(self, tscOctets, keyID=0):
        """ Make an ARC4 key from TKIP Sequence Counter Octets (little-endian) """
        if keyID!=0 :
            raise 'TKIP expects keyID of zero'
        print "tscOctets in tkmixer=",b2a_p(tscOctets)
        newKey = self.keyMixer.newKey(tscOctets)
        print "newKey=", b2a_p(newKey)
        return newKey

    def encrypt(self, plainText, iv):
        """ Encrypt a string and return a binary string
            iv is 6 octets of little-endian encoded pn
        """
        assert(len(iv)==6),'TKIP bad IV size on encryption'
        self.pnField = iv
        self.arc4.setKey( self._makeARC4key(iv) )
        eh1 = chr((ord(iv[0])|0x20)&0x7f)
        encryptionHeader = iv[0] + eh1 + iv[1] + chr((self.keyId<<6)|0x20) + iv[2:]
        crc = pack('<I', crc32(plainText) )
        cipherText = encryptionHeader + self.arc4.encrypt(plainText+crc)
        return cipherText

    def decrypt(self, cipherText):
        """ Decrypt a WEP packet, assumes WEP 4 byte header on packet """
        assert(ord(cipherText[3])& 0x20),'extIV SHOULD be set in TKIP header'
        self.setCurrentKeyID = (ord(cipherText[3])&0xC0)>>6
        iv = cipherText[0]+cipherText[2]+cipherText[4:8]
        self.pnField = iv
        self.arc4.setKey( self._makeARC4key(iv) )
        plainText = self.arc4.decrypt(cipherText[self.encryptHeaderSize:])
        if plainText[-4:] != pack('<I',crc32(plainText[:-4])):  # check data integrity
            raise IntegrityCheckError, 'WEP CRC Integrity Check Error'
        return plainText[:-4]



