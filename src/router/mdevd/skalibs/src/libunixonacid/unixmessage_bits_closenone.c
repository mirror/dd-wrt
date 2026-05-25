/* ISC license. */

#include <skalibs/bitarray.h>
#include <skalibs/unixmessage.h>

static unsigned char const unixmessage_bits_closenone_[bitarray_div8(UNIXMESSAGE_MAXFDS)] = { 0 } ;
unsigned char const *const unixmessage_bits_closenone = unixmessage_bits_closenone_ ;
