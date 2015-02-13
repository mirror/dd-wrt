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
 *   void HPAVKeySpec (const char * string);
 *
 *   HPAVKey.h
 *
 *   confirm that a string is a legal HomePlug AV pass phrase; return
 *   on success; exit the program on failure; legal pass phrases have
 *   12 to 64 characters ranging from 0x20 thru 0x7F;
 *
 *   this function is intended to check pass phrases entered from the
 *   command line as arguments therefore it explains why it failed;
 *
 *
 *   Contributor(s);
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef HPAVKEYSPEC_SOURCE
#define HPAVKEYSPEC_SOURCE

#include <ctype.h>

#include "../tools/error.h"
#include "../key/HPAVKey.h"

void HPAVKeySpec (const char * string)

{
	const char * sp = string;
	while (isprint (*sp))
	{
		sp++;
	}
	if (*sp)
	{
		error (1, ENOTSUP, "Phrase \"%s\" has illegal characters", string);
	}
	if ((sp - string) < HPAVKEY_PHRASE_MIN)
	{
		error (1, ENOTSUP, "Phrase \"%s\" less than %d characters", string, HPAVKEY_PHRASE_MIN);
	}
	if ((sp - string) > HPAVKEY_PHRASE_MAX)
	{
		error (1, ENOTSUP, "Phrase \"%s\" more than %d characters", string, HPAVKEY_PHRASE_MAX);
	}
	return;
}


#endif

