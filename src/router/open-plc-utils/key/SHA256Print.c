/*====================================================================*
 *
 *   void SHA256Print (const uint8_t digest [], const char * string);
 *
 *   HPAVKey.h
 *
 *   print a digest in hexadecimal on stdout followed by string if
 *   string is non-empty;
 *
 *   Motley Tools by Charles Maier <cmaier@cmassoc.net>;
 *   Copyright (c) 2001-2006 by Charles Maier Associates;
 *   Licensed under the Internet Software Consortium License;
 *
 *--------------------------------------------------------------------*/

#ifndef SHA256PRINT_SOURCE
#define SHA256PRINT_SOURCE

#include <stdio.h>

#include "../key/SHA256.h"
#include "../tools/number.h"

void SHA256Print (const uint8_t digest [], const char * string)

{
	unsigned length = SHA256_DIGEST_LENGTH;
	while (length--)
	{
		putc (DIGITS_HEX [(* digest >> 4) & 0x0F], stdout);
		putc (DIGITS_HEX [(* digest >> 0) & 0x0F], stdout);
		digest++;
	}
	if (string) for (putc (' ', stdout); *string; string++)
	{
		putc (*string, stdout);
	}
	printf ("\n");
	return;
}


#endif

