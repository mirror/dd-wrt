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
 *   void HPAVKeyNID (uint8_t NID [], const uint8_t NMK [], signed level);
 *
 *   HPAVKey.h
 *
 *   compute the HomePlugAV compliant Network Identification Key (NID)
 *   for a given Network Membership Key (NMK); return the key in buffer
 *   NID []; the key will be HPAVKEY_NID_LEN bytes as defined in
 *   HPAVKey.h;
 *
 *   unlike the NMK, the NID is 54-bits and includes a 2-bit security
 *   level; See the HomePlug AV Specification for more info;
 *
 *   hash the NMK then rehash the digest 4 times per HomePlug AV
 *   Specification; no salt is used;
 *
 *
 *   Contributor(s);
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef HPAVKEYNID_SOURCE
#define HPAVKEYNID_SOURCE

#include <memory.h>

#include "../key/HPAVKey.h"
#include "../key/SHA256.h"

void HPAVKeyNID (uint8_t NID [], const uint8_t NMK [], uint8_t level)

{
	struct sha256 sha256;
	uint8_t digest [SHA256_DIGEST_LENGTH];
	unsigned rehash = 4;
	SHA256Reset (&sha256);
	SHA256Write (&sha256, NMK, HPAVKEY_NMK_LEN);
	SHA256Fetch (&sha256, digest);
	while (rehash--)
	{
		SHA256Reset (&sha256);
		SHA256Write (&sha256, digest, sizeof (digest));
		SHA256Fetch (&sha256, digest);
	}

#if 1

	level <<= 4;
	digest [HPAVKEY_NID_LEN - 1] >>= 4;
	digest [HPAVKEY_NID_LEN - 1] |= level;

#else

	digest [HPAVKEY_NID_LEN - 1] &= ~0xC0;
	digest [HPAVKEY_NID_LEN - 1] |= level << 6;

#endif

	memcpy (NID, digest, HPAVKEY_NID_LEN);
	return;
}


#endif

