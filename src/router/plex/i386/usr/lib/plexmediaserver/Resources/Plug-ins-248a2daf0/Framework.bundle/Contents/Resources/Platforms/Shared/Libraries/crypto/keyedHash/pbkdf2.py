""" crypto.keyedHash.pbkdf2


	Password Based Key Derivation Function 2
	References: RFC2898, B. Kaliski, September 2000, PKCS #5

	This function is used for IEEE 802.11/WPA passphrase to key hashing

	Copyright (c) 2002 by Paul A. Lambert
	Read LICENSE.txt for license information.
"""
from crypto.keyedHash.hmacHash import HMAC_SHA1
from crypto.common import xor
from math import ceil
from struct import pack

def pbkdf2(password, salt, iterations, keySize, PRF=HMAC_SHA1):
	""" Create key of size keySize from password and salt """
	if len(password)>63:
		raise 'Password too long for pbkdf2'
	#if len(password)<8 : raise 'Password too short for pbkdf2'
	if (keySize > 10000):		  # spec says >4294967295L*digestSize
		raise 'keySize too long for PBKDF2'

	prf = PRF(key=password)  # HMAC_SHA1
	numBlocks = ceil(1.*keySize/prf.digest_size) # ceiling function
	key = ''
	for block in range(1,numBlocks+1):
		# Calculate F(P, salt, iterations, i)
		F = prf(salt+pack('>i',block)) # i is packed into 4 big-endian bytes
		U = prf(salt+pack('>i',block)) # i is packed into 4 big-endian bytes
		for count in range(2,iterations+1):
			U = prf(U)
			F = xor(F,U)
		key = key + F
	return key[:keySize]

def dot11PassPhraseToPSK(passPhrase,ssid):
	""" The 802.11 TGi recommended pass-phrase-to-preshared-key mapping.
		This function simply uses pbkdf2 with interations=4096 and keySize=32
	"""
	assert( 7<len(passPhrase)<64 ), 'Passphrase must be greater than 7 or less than 64 characters'
	return pbkdf2(passPhrase, ssid, iterations=4096, keySize=32)
