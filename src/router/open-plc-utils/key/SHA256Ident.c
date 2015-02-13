/*====================================================================*
 *
 *   void SHA256Ident (signed fd,  uint8_t digest []);
 *
 *   SHA256.h
 *
 *   compute the SHA256 digest of file content; the digest becomes
 *   the fingerprint that can be used to identify the file despite
 *   filename changes;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef SHA256IDENT_SOURCE
#define SHA256IDENT_SOURCE

#include <unistd.h>

#include "../key/SHA256.h"

void SHA256Ident (signed fd, uint8_t digest [])

{
	struct sha256 sha256;
	uint8_t buffer [1024];
	signed length;
	SHA256Reset (&sha256);
	while ((length = read (fd, buffer, sizeof (buffer))) > 0)
	{
		SHA256Write (&sha256, buffer, length);
	}
	SHA256Fetch (&sha256, digest);
	return;
}


#endif

