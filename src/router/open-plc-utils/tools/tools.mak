# ====================================================================
# program;
# --------------------------------------------------------------------

__bswap.o: __bswap.c endian.h
assist.o: assist.c error.h symbol.h
b64dump.o: b64dump.c base64.h types.h
basespec.o: basespec.c error.h number.h
binout.o: binout.c memory.h number.h
bytespec.o: bytespec.c error.h memory.h number.h
checkfilename.o: checkfilename.c files.h
checksum32.o: checksum32.c memory.h
chrout.o: chrout.c memory.h
clr32bitmap.o: clr32bitmap.c endian.h flags.h
codelist.o: codelist.c symbol.h
codename.o: codename.c symbol.h 
config.o: config.c config.h types.h
dataspec.o: dataspec.c error.h memory.h number.h
debug.o: debug.c error.h types.h
decdecode.o: decdecode.c memory.h number.h
decout.o: decout.c memory.h number.h
decstring.o: decstring.c memory.h
efreopen.o: efreopen.c error.h files.h
emalloc.o: emalloc.c error.h
error.o: error.c error.h types.h
extra.o: extra.c error.h types.h
fdchecksum32.o: fdchecksum32.c memory.h
getoptv.o: getoptv.c error.h getoptv.h putoptv.h version.h
getargv.o: getargv.c chars.h  symbol.h
gettimeofday.o: gettimeofday.c
hexdecode.o: hexdecode.c memory.h number.h
hexdump.o: hexdump.c memory.h number.h
hexencode.o: hexencode.c memory.h number.h
hexload.o: hexload.c chars.h error.h memory.h
hexoffset.o: hexoffset.c memory.h number.h
hexout.o: hexout.c memory.h number.h
hexpeek.o: hexpeek.c memory.h
hexcopy.o: hexcopy.c memory.h
hexstring.o: hexstring.c memory.h
hexview.o: hexview.c memory.h number.h
hexwrite.o: hexwrite.c number.h types.h
ipv4spec.o: ipv4spec.c error.c error.h hexdecode.c memory.h number.h todigit.c
ipv6spec.o: ipv6spec.c error.c error.h hexdecode.c memory.h number.h todigit.c
lookup.o: lookup.c symbol.h
memdecr.o: memdecr.c memory.h
memencode.o: memencode.c error.h memory.h number.h pib.h
memincr.o: memincr.c memory.h
memout.o: memout.c memory.h
memswap.o: memswap.c memory.h
output.o: output.c format.h
putoptv.o: putoptv.c getoptv.h putoptv.h
regview32.o: regview32.c endian.h memory.h
reverse.o: reverse.c memory.h
reword.o: reword.c symbol.h
set32bitmap.o: set32bitmap.c endian.h flags.h
strdecr.o: strdecr.c memory.h
strfbits.o: strfbits.c flags.h memory.h
strincr.o: strincr.c memory.h
synonym.o: synonym.c types.h
todigit.o: todigit.c number.h
typelist.o: typelist.c symbol.h
typename.o: typename.c symbol.h 
uintspec.o: uintspec.c error.h number.h types.h
version.o: version.c version.h

# ====================================================================
# header files;
# --------------------------------------------------------------------

error.h: types.h
files.h: types.h
markup.h: version.h
memory.h: endian.h types.h
symbol.h: types.h

