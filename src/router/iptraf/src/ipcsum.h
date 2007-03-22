/***

ipcsum.h - prototype declaration for the standard IP checksum calculation
routine

***/

#include <sys/types.h>

int in_cksum(u_short * addr, int len);
