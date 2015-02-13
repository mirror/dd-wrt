# file: key/key.mak

# ====================================================================
# programs;
# --------------------------------------------------------------------

hpavkey.o: hpavkey.c HPAVKey.h HPAVKeyDAK.c HPAVKeyNID.c HPAVKeyNMK.c HPAVKeyOut.c HPAVKeySHA.c SHA256.c SHA256.h error.c error.h flags.h getoptv.c getoptv.h hexout.c number.h putoptv.c putoptv.h todigit.c types.h uintspec.c version.c version.h
hpavkeys.o: hpavkeys.c HPAVKey.h HPAVKeyDAK.c HPAVKeyNID.c HPAVKeyNMK.c HPAVKeyOut.c HPAVKeySHA.c SHA256.c SHA256.h error.c error.h flags.h getoptv.c getoptv.h hexout.c number.h putoptv.c putoptv.h todigit.c types.h uintspec.c version.c version.h
keys.o: keys.c keys.h
mac2pw.o: mac2pw.c MACPasswords.c RNDPasswords.c strnpwd.o error.c error.h flags.h getoptv.c getoptv.h keys.h memory.h number.h putoptv.c putoptv.h todigit.c types.h uintspec.c version.c
mac2pwd.o: mac2pwd.c MACPasswords.c RNDPasswords.c strnpwd.o error.c error.h flags.h getoptv.c getoptv.h keys.h memory.h number.h putoptv.c putoptv.h todigit.c types.h uintspec.c version.c
rkey.o: rkey.c HPAVKey.h HPAVKeyDAK.c HPAVKeyNID.c HPAVKeyNMK.c HPAVKeyOut.c HPAVKeySHA.c SHA256.h SHA256Block.c SHA256Fetch.c SHA256Reset.c SHA256Write.c error.c error.h files.h flags.h getoptv.c getoptv.h hexout.c memory.h number.h putoptv.c putoptv.h strincr.c todigit.c uintspec.c version.c

# ====================================================================
# modules;
# --------------------------------------------------------------------

HPAVKeyDAK.o: HPAVKeyDAK.c HPAVKey.h SHA256.h
HPAVKeyNID.o: HPAVKeyNID.c HPAVKey.h SHA256.h
HPAVKeyNMK.o: HPAVKeyNMK.c HPAVKey.h SHA256.h
HPAVKeyOut.o: HPAVKeyOut.c HPAVKey.h flags.h memory.h number.h types.h
HPAVKeySHA.o: HPAVKeySHA.c HPAVKey.h SHA256.h
HPAVKeySpec.o: HPAVKeySpec.c HPAVKey.h error.h
MACPasswords.o: MACPasswords.c flags.h keys.h types.h
RNDPasswords.o: RNDPasswords.c flags.h keys.h types.h
SHA256.o: SHA256.c SHA256Block.c SHA256Fetch.c SHA256Reset.c SHA256Write.c
SHA256Block.o: SHA256Block.c SHA256.h
SHA256Fetch.o: SHA256Fetch.c SHA256.h
SHA256Ident.o: SHA256Ident.c SHA256.h
SHA256Match.o: SHA256Match.c SHA256.h number.h
SHA256Print.o: SHA256Print.c SHA256.h number.h
SHA256Reset.o: SHA256Reset.c SHA256.h
SHA256Write.o: SHA256Write.c SHA256.h
strnpwd.o: strnpwd.c flags.h types.h keys.h

# ====================================================================
# headers;
# --------------------------------------------------------------------

keys.h: HPAVKey.h types.h

