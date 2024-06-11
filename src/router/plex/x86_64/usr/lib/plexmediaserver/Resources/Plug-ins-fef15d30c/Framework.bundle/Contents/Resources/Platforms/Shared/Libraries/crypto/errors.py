""" errors
    Error classes for cryptographic modules

    Copyright (c) 2002 by Paul A. Lambert
    Read LICENSE.txt for license information.
"""

class CryptoError(Exception):
    """ Base class for crypto exceptions """
    def __init__(self,errorMessage='Error!'):
        self.message = errorMessage
    def __str__(self):
        return self.message

class InitCryptoError(CryptoError):
    """ Crypto errors during algorithm initialization """
class BadKeySizeError(InitCryptoError):
    """ Bad key size error """
class EncryptError(CryptoError):
    """ Error in encryption processing """
class DecryptError(CryptoError):
    """ Error in decryption processing """
class DecryptNotBlockAlignedError(DecryptError):
    """ Error in decryption processing """
class IntegrityCheckError(DecryptError):
    """ Bad integrity detected during decryption (integrity aware algorithms) """
