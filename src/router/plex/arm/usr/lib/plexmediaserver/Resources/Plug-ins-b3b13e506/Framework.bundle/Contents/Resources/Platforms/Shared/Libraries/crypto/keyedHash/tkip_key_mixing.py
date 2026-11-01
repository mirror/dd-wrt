""" crypto.keyedHash.tkip_key_mixing.py
    TKIP Temporal Key Mixing Function reference implementation

    2002-11-04

"""
from struct import pack, unpack
from binascii_plus import *

def S(word):
    """ TKIP S-Box non-linear substitution of a 16 bit word """
    return (tkipSbox[0][word & 0x00FF] ^ tkipSbox[1][(word>>8) & 0x00FF])

""" tkipSbox consists of two 256 word arrays
    The tkip Sbox is formed from the AES/Rijndael Sbox
"""

from crypto.cipher.rijndael import Sbox
tkipSbox = [range(256),range(256)] # arbitrary initialization
for i in range(256):
    k  = Sbox[i]           # the rijndael S box (imported)
    if k & 0x80 :          # calculate k*2 polynomial math
        k2 = (k<<1)^0x11b  # reduce by rijndael modulas
    else:
        k2 = k<<1
    k3 = k ^ k2
    tkipSbox[0][i] = (k2<<8)^k3
    tkipSbox[1][i] = (k3<<8)^k2  # second array is just byte swap of first array

def rotR1(v16):
    """ circular right rotate on 16 bits """
    return    ((((v16) >> 1) & 0x7FFF) ^ (((v16) & 1) << 15))

class TKIP_Mixer:
    """ The TKIP_Mixer class generates dynamic keys for TKIP based on the
        TK (temporal key), TA and PN
    """
    def __init__(self, tk1=None, transmitterAddress=None, pnBytes=6*chr(0)):
        """ The TKIP Mixer is initialized with tk1 and TA
            tk1 is a temporal key (16 octet string)
            transmitterAddress is a 6 octet MAC address
            pn is the packet number, here as an integer < (1<<8*6)
        """
        self.tk = None
        self.ta = None
        self.setPnBytes(pnBytes)   # sets self.pnBytes and validates input
        self.upper4SequenceOctets = self.pnBytes[-4:]
        if tk1 != None :
            self.setKey(tk1)
        if transmitterAddress != None :
            self.setTA(transmitterAddress)

    def setKey(self, key):
        """ Set the temporal key (tk1) for key mixing """
        if len(key)!= 16: raise 'Wrong key size'
        # for readability of subroutines, make tk a list of 1 octet ints
        self.tk = [ord(byte) for byte in key]
        if self.ta != None : # reset phase1 value
            self.phase1Key = phase1KeyMixing( self.tk, self.ta, self.pn)

    def setTA(self, taBytes):
        """ Set the transmitter address """
        if len(taBytes) != 6: raise 'Bad size for transmitterAddress'
        self.ta = [ord(byte) for byte in taBytes]
        if self.tk != None : # reset phase1 value
            self.phase1Key = phase1KeyMixing( self.tk, self.ta, self.pn )

    def setPnBytes(self, pnBytes):
        """ Set the pnBytes from the packet number (int) """
        assert( len(pnBytes)==6 ), 'pnBytes must be 6 octets'
        self.pnBytes = pnBytes
        self.pn = [ord(byte) for byte in pnBytes] # int list for readability

    def newKey(self, pnBytes):
        """ return a new 'mixed' key (16 octets) based on
            the pn in 6 octets, also know as TSC
        """
        assert(self.ta != None), 'No TA'
        assert(self.tk != None), 'No TK'
        self.setPnBytes(pnBytes)
        if self.pnBytes[-4:] != self.upper4SequenceOctets: # check if upper bits change
            # calculate phase1 key only when upper bytes change
            self.upper4SequenceOctets = pnBytes[-4:]
            self.phase1Key = phase1KeyMixing( self.tk, self.ta, self.pn )
        return  phase2KeyMixing( self.tk, self.phase1Key, self.pn )

def phase1KeyMixing(tk,ta,pn):
    """ Create a p1k (5 integers) from TK, TA and upper 4 octets of sequence number pn"""
    p1k = [0,0,0,0,0]           # array of 5 integers (each 2 octets)
    p1k[0] = pn[3]*256 + pn[2]
    p1k[1] = pn[5]*256 + pn[4]
    p1k[2] = ta[1]*256 + ta[0]  # 2 octets of MAC as an integer (little-endian)
    p1k[3] = ta[3]*256 + ta[2]  # 2 octets of MAC as an integer (little-endian)
    p1k[4] = ta[5]*256 + ta[4]  # 2 octets of MAC as an integer (little-endian)
    for i in range(8):
        j = 2*(i&1)
        p1k[0] = ( p1k[0] + S( p1k[4]^(tk[j+ 1]*256 + tk[j+ 0])))   & 0xFFFF
        p1k[1] = ( p1k[1] + S( p1k[0]^(tk[j+ 5]*256 + tk[j+ 4])))   & 0xFFFF
        p1k[2] = ( p1k[2] + S( p1k[1]^(tk[j+ 9]*256 + tk[j+ 8])))   & 0xFFFF
        p1k[3] = ( p1k[3] + S( p1k[2]^(tk[j+13]*256 + tk[j+12])))   & 0xFFFF
        p1k[4] = ( p1k[4] + S( p1k[3]^(tk[j+ 1]*256 + tk[j])) + i ) & 0xFFFF
    return p1k

def phase2KeyMixing(tk,p1k,pn):
    """ Create a 16 octet key from the phase1Key (p1k)
        and 2 octets of sequence counter """
    ppk = [i for i in p1k]
    ppk.append( p1k[4] + pn[1]*256 + pn[0] ) # append value for ppk[5]
    # Bijective non-linear mixing of the 96 bits of ppk
    ppk[0] = (ppk[0] + S(ppk[5] ^ (tk[1]*256 + tk[0]) ))  & 0xFFFF
    ppk[1] = (ppk[1] + S(ppk[0] ^ (tk[3]*256 + tk[2]) ))  & 0xFFFF
    ppk[2] = (ppk[2] + S(ppk[1] ^ (tk[5]*256 + tk[4]) ))  & 0xFFFF
    ppk[3] = (ppk[3] + S(ppk[2] ^ (tk[7]*256 + tk[6]) ))  & 0xFFFF
    ppk[4] = (ppk[4] + S(ppk[3] ^ (tk[9]*256 + tk[8]) ))  & 0xFFFF
    ppk[5] = (ppk[5] + S(ppk[4] ^ (tk[11]*256+ tk[10]) )) & 0xFFFF
    # Final sweep
    ppk[0] = (ppk[0] + rotR1(ppk[5] ^ (tk[13]*256+tk[12]))) & 0xFFFF
    ppk[1] = (ppk[1] + rotR1(ppk[0] ^ (tk[15]*256+tk[14]))) & 0xFFFF
    ppk[2] = (ppk[2] + rotR1(ppk[1])) & 0xFFFF
    ppk[3] = (ppk[3] + rotR1(ppk[2])) & 0xFFFF
    ppk[4] = (ppk[4] + rotR1(ppk[3])) & 0xFFFF
    ppk[5] = (ppk[5] + rotR1(ppk[4])) & 0xFFFF

    rc4Key = range(16)
    rc4Key[0] =  pn[0]
    rc4Key[1] = (pn[0] | 0x20) & 0x7F
    rc4Key[2] =  pn[1]
    rc4Key[3] = 0xFF &((ppk[5]^(tk[1]*256+tk[0]))>>1)
    for i in range(6):
        rc4Key[4+2*i] = ppk[i] & 0xff
        rc4Key[5+2*i] = (ppk[i]>>8) & 0xff
    wepSeed = ''.join([chr(i) for i in rc4Key]) # convert to string
    return wepSeed


