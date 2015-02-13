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
 *   void HPAVKeyDAK (uint8_t DAK [], const char * string);
 *
 *   HPAVKey.h
 *
 *   compute the HomePlugAV compliant Device Access Key (DAK) for a
 *   NUL terminated string; return the key in buffer DAK[]; the key
 *   will be HPAVKEY_DAK_LEN bytes as defined in HPAVKey.h;
 *
 *   seed the digest then salt the digest then rehash the digest 999
 *   times per HomePlug AV Specification;
 *
 *
 *   Contributor(s);
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef HPAVKEYDAK_SOURCE
#define HPAVKEYDAK_SOURCE

#include <memory.h>

#include "../key/HPAVKey.h"
#include "../key/SHA256.h"

void HPAVKeyDAK (uint8_t DAK [], const char * string)

{
	struct sha256 sha256;
	uint8_t digest [SHA256_DIGEST_LENGTH];
	const uint8_t secret [] =
	{
		0x08,
		0x85,
		0x6D,
		0xAF,
		0x7C,
		0xF5,
		0x81,
		0x85
	};
	unsigned rehash = 999;
	SHA256Reset (&sha256);
	SHA256Write (&sha256, string, strlen (string));
	SHA256Write (&sha256, secret, sizeof (secret));
	SHA256Fetch (&sha256, digest);
	while (rehash--)
	{
		SHA256Reset (&sha256);
		SHA256Write (&sha256, digest, sizeof (digest));
		SHA256Fetch (&sha256, digest);
	}
	memcpy (DAK, digest, HPAVKEY_DAK_LEN);
	return;
}


#endif

