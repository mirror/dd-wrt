/* ISC license. */

#include <skalibs/bitarray.h>
#include <skalibs/unixmessage.h>

static unsigned char const unixmessage_bits_closeall_[bitarray_div8(UNIXMESSAGE_MAXFDS)] =
"\377" "\377" "\377" "\377" "\377" "\377" "\377" "\377"
"\377" "\377" "\377" "\377" "\377" "\377" "\377" "\377"
"\377" "\377" "\377" "\377" "\377" "\377" "\377" "\377"
"\377" "\377" "\377" "\377" "\377" "\377" "\377" "\377"
 ;
unsigned char const *const unixmessage_bits_closeall = unixmessage_bits_closeall_ ;
