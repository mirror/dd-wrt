/*====================================================================*
 *
 *   Copyright (c) 2013 Qualcomm Atheros, Inc.
 *
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or 
 *   without modification, are permitted (subject to the limitations 
 *   in the disclaimer below) provided that the following conditions 
 *   are met:
 *
 *   * Redistributions of source code must retain the above copyright 
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above 
 *     copyright notice, this list of conditions and the following 
 *     disclaimer in the documentation and/or other materials 
 *     provided with the distribution.
 *
 *   * Neither the name of Qualcomm Atheros nor the names of 
 *     its contributors may be used to endorse or promote products 
 *     derived from this software without specific prior written 
 *     permission.
 *
 *   NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE 
 *   GRANTED BY THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE 
 *   COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
 *   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 *   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 *   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER 
 *   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
 *   NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 *   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) 
 *   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 *   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE 
 *   OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 *   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *
 *   signed SHA256Match (signed fd, const uint8_t digest[]);
 *
 *   SHA256.h
 *
 *   search a fingerprint file for the given digest; return true
 *   on success and false on failure; digests in the fingerprint
 *   file must be upper case because comparison is binary;
 *
 *
 *--------------------------------------------------------------------*/

#ifndef SHA256MATCH_SOURCE
#define SHA256MATCH_SOURCE

#include <unistd.h>
#include <errno.h>

#include "../tools/number.h"
#include "../key/SHA256.h"

signed SHA256Match (signed fd, const uint8_t digest [])

{
	char string [SHA256_DIGEST_LENGTH << 1];
	char buffer [SHA256_DIGEST_LENGTH << 1];
	signed length = 0;
	while (length < (signed)(sizeof (string)))
	{
		string [length++] = DIGITS_HEX [(*digest >> 4) & 0x0F];
		string [length++] = DIGITS_HEX [(*digest >> 0) & 0x0F];
		digest++;
	}
	while (read (fd, buffer, sizeof (buffer)) == sizeof (buffer))
	{
		char c;
		if (!memcmp (string, buffer, sizeof (string)))
		{
			return (1);
		}
		while (read (fd, &c, sizeof (c)) == sizeof (c))
		{
			if (c == '\n')
			{
				break;
			}
		}
	}
	return (0);
}


#endif

