#! /usr/bin/awk -f

# This script is used to process the Keccak testvectors, originally
# we used http://keccak.noekeon.org/KeccakKAT-3.zip.
# For the updated NIST version, test vectors can be found at
# https://github.com/gvanas/KeccakCodePackage/tree/master/TestVectors

/^Len/ { len = $3 }
/^Msg/ { msg = $3 }
/^Squeezed/ { md = $3;
  if (len % 8 == 0)
    printf("test_hash_extendable(&nettle_shakexxx, /* %d octets */\nSHEX(\"%s\"),\nSHEX(\"%s\"));\n",
	   len / 8, len ? msg : "", md);
}
