""" crypto.cipher.ccm

    CCM block cipher mode

    The CCM class can wrap any BlockCipher to create a 'CCM' mode
    that provides encryption with a strong integrity check.  The
    integrity check can optionally include unencrypted 'addAuthData'.
    CCM requires a nonce that MUST NEVER repeat for a given key.

    Copyright (c) 2002 by Paul A. Lambert
    Read LICENSE.txt for license information.
"""
from crypto.cipher.base import BlockCipherWithIntegrity, noPadding
from crypto.common      import xor
from struct             import unpack, pack
from crypto.errors      import InitCryptoError, EncryptError, DecryptError, IntegrityCheckError

class CCM(BlockCipherWithIntegrity):
    """ The CCM class wraps block ciphers to provide integrity and encryption.

        CCM provides both encryption and a strong integrity check. The
        integrity check can optionally include "additional authentication
        data" that is included in the message integrity check, but is not encrypted.

        CCM is composed of two passes of the same base cipher, first
        the instance calculates a CBC Message Authentication Check,
        and then the same algorithm instance is used for the CTR
        (counter) mode encryption.

        This algorithm mode does NOT support streams of data (moreData flag)
        since a full packet must be available for the two pass CBC_MAC
        and CTR encryption process.

        When decrypting, a 'DecryptIntegrityError' exception is raised
        if the integrity check fails.

            >> aes_ccm = CCM(AES(key))
            >> cipherText = aes_ccm.encrypt(plainText, nonce)
            >> try:
            >>     decryptedText = aes_ccm.decrypt(cipherText, nonce)
            >> except IntegrityCheckError:
            >>     print 'failed integrity check'
            or ...
            >> cipherText = aes_ccm.encrypt(plainText, nonce, addAuthData=header)
            >> try:
            >>     decryptedText = aes_ccm.decrypt(cipherText, nonce, addAuthData=header)
            >> except IntegrityCheckError:
            >>     print 'failed integrity check'
    """
    def __init__(self, blockCipherInstance, autoNonce=None, macSize=8, nonceSize=13):
        """ CCM algorithms are created by initializing with a BlockCipher instance
                blockCipherInstance -> typically AES_ECB
                autoNonce -> sets the intial value of a nonce for automatic nonce
                             creation (not available yet)
                macSize   -> size of MAC field can be = 4, 6, 8, 10, 12, 14, or 16
                nonceSize -> size of nonce in bytes (default 13)
                the counter size is blockSize-nonceSize-1
        """
        self.baseCipher = blockCipherInstance
        self.name       = self.baseCipher.name + '_CCM'
        self.blockSize  = self.baseCipher.blockSize
        self.keySize    = self.baseCipher.keySize

        self.baseCipher.padding = noPadding()   # baseCipher should NOT pad!!


        self.M = macSize        # Number of octets
        if  not((3 < self.M < 17) and (macSize%2==0)) :
            raise InitCryptoError, 'CCM, M (size of auth field) is out of bounds'

        self.nonceSize  = nonceSize
        self.L = self.baseCipher.blockSize - self.nonceSize - 1
        if not(1 < self.L < 9) :
            raise InitCryptoError, 'CCM, L (size of length field) is out of bounds'
        self.reset()

    def setKey(self, key):
        self.baseCipher.setKey(key)
        self.reset()

    # Overload to reset both CCM state and the wrapped baseCipher
    def resetEncrypt(self):
        BlockCipherWithIntegrity.resetEncrypt(self)  # reset CCM encrypt state (super class)
        self.baseCipher.resetEncrypt()  # reset base cipher encrypt state

    def resetDecrypt(self):
        BlockCipherWithIntegrity.resetDecrypt(self)  # reset CBC state (super class)
        self.baseCipher.resetEncrypt()  # CCM uses encryption of base cipher to decrypt!

    def encrypt(self, plainText, nonce, addAuthData=''):
        """  CCM encryption of plainText
                nonce must be unique for each encryption, if set to none
                   it will maintain it's own nonce creation
                addAuthData is optional  """
        # construct authentication block zero
        #     flag byte fields
        Adata      = ((len(addAuthData))>0) << 6  # bit 6 is 1 if auth
        Mfield     = ((self.M-2)/2) << 3          # bits 5,4,3 encode macSize
        Lfield     =  self.L-1                    # bits 2,1,0 encode L size = blockSize-nonceSize-1
        flagsByte  =  chr(Adata^Mfield^Lfield)

        if len(nonce) != self.nonceSize :
            raise EncryptError, 'wrong sized nonce'

        lenMessage = len(plainText)
        if lenMessage >= 1L<<(8*self.L):
            raise EncryptError, 'CCM plainText too long for given L field size'
        packedLenMessage = pack('!Q', lenMessage)[-self.L:]  # pack and truncate to L bytes

        blockZero = flagsByte+nonce+packedLenMessage
        if len(blockZero) != self.baseCipher.blockSize:
            raise EncryptError, 'CCM bad size of first block'

        authLengthField = self._encodeAuthLength(len(addAuthData))
        cbcInput     = blockZero+authLengthField+addAuthData
        authPadSize  = self.baseCipher.blockSize-((len(cbcInput)-1)%self.baseCipher.blockSize)-1
        cbcInput     = cbcInput + authPadSize*chr(0)    # pad to block size with zeros
        cbcInput     = cbcInput + plainText
        cbcEndPad    = chr(0x00)*((self.blockSize-((len(cbcInput))%self.blockSize))%self.blockSize)
        cbcInput     = cbcInput + cbcEndPad

        # Calculate CBC_MAC
        numCbcBlocks,extra = divmod(len(cbcInput),self.blockSize)
        assert (extra==0), 'bad block size on cbc_mac calculation'

        cbcMicValue = self.blockSize*chr(0x00)
        for i in range(numCbcBlocks) :
            cbcBlock    = cbcInput[i*self.blockSize:(i+1)*self.blockSize]
            cbcMicValue = self.baseCipher.encrypt( xor(cbcMicValue, cbcBlock) )
        counter   = 0L
        # the counter mode preload with counter starting at zero
        ctrModePl = chr(self.L-1)+ nonce + pack('>Q', counter)[-self.L:]
        ccmMIC = xor(self.baseCipher.encrypt(ctrModePl),cbcMicValue)[:self.M] # first M bytes of xor

        ct = ''
        numCtrBlocks,extra = divmod(len(plainText)+self.blockSize,self.blockSize)
        while counter < numCtrBlocks :
            counter   = counter + 1L
            ctrModePl = chr(self.L-1) + nonce + pack('>Q', counter)[-self.L:]
            ct = ct + xor(self.baseCipher.encrypt(ctrModePl), plainText[(counter-1)*16:counter*16] )
        ct = ct + ccmMIC
        return  ct

    def decrypt(self, cipherText, nonce, addAuthData=''):
        """  CCM decryption of cipherText
                nonce must be unique for each encryption, if set to none
                   it will maintain it's own nonce creation
                   the nonce is then included in the cipher text
                addAuthData is option """
        # construct authentication block zero
        #     flag byte fields
        Adata      = ((len(addAuthData))>0) << 6  # bit 6 is 1 if auth
        Mfield     = ((self.M-2)/2) << 3          # bits 5,4,3 encode macSize
        Lfield     =  self.L-1                    # bits 2,1,0 encode L size = blockSize-nonceSize-1
        flagsByte  =  chr(Adata^Mfield^Lfield)

        if len(nonce) != self.nonceSize :
            raise DecryptError, 'wrong sized nonce'

        lenMessage = len(cipherText)-self.M
        if lenMessage >= 1L<<(8*self.L):
            raise DecryptError, 'CCM cipherText too long for given L field size'
        if lenMessage < 0 :
            raise DecryptError, 'Too small of cipherText for MIC size'
        packedLenMessage = pack('!Q', lenMessage)[-self.L:]  # pack and truncate to L bytes

        pt = ''
        ct = cipherText[:-self.M]      # trim of MIC field

        numCtrBlocks,extra = divmod(len(ct)+self.blockSize,self.blockSize)
        for counter in range(1, numCtrBlocks+1) :
            ctrModePl = chr(self.L-1) + nonce + pack('>Q', counter)[-self.L:]
            ctr     = self.baseCipher.encrypt(ctrModePl)
            ctBlock = ct[(counter-1)*self.blockSize:counter*self.blockSize]
            pt = pt + xor( ctr, ctBlock )
        #------- CBC Mac Calculation
        blockZero = flagsByte+nonce+packedLenMessage
        if len(blockZero) != self.baseCipher.blockSize:
            raise DecryptError, 'CCM bad size of first block'

        authLengthField = self._encodeAuthLength(len(addAuthData))
        cbcInput     = blockZero+authLengthField+addAuthData
        authPadSize  = self.baseCipher.blockSize-((len(cbcInput)-1)%self.baseCipher.blockSize)-1
        cbcInput     = cbcInput + authPadSize*chr(0)    # pad to block size with zeros
        cbcInput     = cbcInput + pt
        cbcEndPad    = chr(0x00)*((self.blockSize-((len(cbcInput))%self.blockSize))%self.blockSize)
        cbcInput     = cbcInput + cbcEndPad

        # Calculate CBC_MAC
        numCbcBlocks,extra = divmod(len(cbcInput),self.blockSize)
        assert (extra==0), 'bad block size on cbc_mac calculation'
        cbcMicValue = self.blockSize*chr(0x00)
        for i in range(numCbcBlocks) :
            cbcBlock    = cbcInput[i*self.blockSize:(i+1)*self.blockSize]
            cbcMicValue = self.baseCipher.encrypt( xor(cbcMicValue, cbcBlock) )

        ctrModePl0 = chr(self.L-1)+ nonce + pack('>Q', 0)[-self.L:]
        ccmMIC = xor(self.baseCipher.encrypt(ctrModePl0),cbcMicValue)[:self.M] # first 8 bytes of xor

        if ccmMIC != cipherText[-self.M:] :
            raise IntegrityCheckError, 'CCM Integrity check failed on decrypt'

        return  pt

    def _encodeAuthLength(self, length):
        """ construct byte string representing length, returns 2 to 10 bytes """
        if length < 0 :
            raise EncryptError, 'CCM illegal length value'
        elif 0 <= length < 0xFF00:
            byteString = pack('!H', length)         # pack into two bytes
        elif 0xFF00 <= length < 0x100000000L:
            byteString = pack('!HI',0xFFFE, length) # pack into 0xFFFE + four bytes
        elif 0x100000000L <= length < 0x10000000000000000L:
            byteString = pack('!HQ',0xFFFF, length) # pack into 0xFFFF + eigth bytes
        else:
            raise EncryptError, 'CCM length error'
        return byteString

    def _decodeAuthLength(self, byteString):
        """ decode byte string representing length, returns length
            Only the first 2 to 10 bytes of the byte string are examined """
        firstTwoOctets == unpack('!H',bytesString[0:2])   # two bytes used for length
        if firstTwoOctets == 0:
            raise DecryptError, 'CCM auth length zero with auth bit set'
        elif 0 < firstTwoOctets < 0xFEFF:
            messageLength == firstTwoOctets
        elif 0xFEFF < firstTwoOctets < 0xFFFE:
            raise DecryptError, 'CCM auth length illegal values'
        elif firstTwoOctets == 0xFFFE:
            messageLength = unpack('!I',byteString[2:6])  # four bytes used for length
        elif firstTwoOctets == 0xFFFF:
            messageLength = unpack('!Q',byteString[2:10]) # eight bytes used for length
        else:
            raise DecryptError, 'CCM auth length error'
        return messageLength







