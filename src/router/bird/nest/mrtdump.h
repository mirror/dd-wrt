/*
 *	BIRD -- MRTdump handling
 *
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef MRTDUMP_H
#define MRTDUMP_H
#include "nest/protocol.h"

/* MRTDump values */

#define MRTDUMP_HDR_LENGTH	12

/* MRTdump types */

#define BGP4MP			16

/* MRTdump subtypes */

#define BGP4MP_MESSAGE		1
#define BGP4MP_MESSAGE_AS4	4
#define BGP4MP_STATE_CHANGE_AS4	5


/* implemented in sysdep */
void mrt_dump_message(struct proto *p, u16 type, u16 subtype, byte *buf, u32 len);

#endif

