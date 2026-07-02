"""  prf_dot11.py

The PRF function is used in a number of places in 802.11i
to create psuedo random octet strings.

The parameter 'prefix' is a string that shall be a unique label for each
different purpose of the PRF.
"""

import hmac, sha        # available in any Python 2.x

def PRF( key, prefix, data, number_of_bits):
    """ Key, prefix and data are arbitrary strings .
        number_of_bits must be a multiple of 8
        HMAC_SHA1 generates 20 byte blocks.  Enough are generated to get the
        requested number of octets and the reslut is truncated to the requested size.
    """
    number_of_octets, remainder = divmod(number_of_bits,8)
    if remainder != 0:
        raise ValueError, 'requested bits not multiple of 8'
    R = ''
    i = 0
    while len(R) <= number_of_octets :
        hmac_sha_1= hmac.new( key, prefix + chr(0x00) + data + chr(i), sha )
        i = i + 1
        R = R + hmac_sha_1.digest()       # concatenate latest hash to result string
    return R[:number_of_octets]           # return R truncated to 'number_of_octets'

def PRF_128(key,A,B): return PRF(key,A,B,128)
def PRF_192(key,A,B): return PRF(key,A,B,192)
def PRF_256(key,A,B): return PRF(key,A,B,256)
def PRF_384(key,A,B): return PRF(key,A,B,384)
def PRF_512(key,A,B): return PRF(key,A,B,512)
def PRF_768(key,A,B): return PRF(key,A,B,768)




