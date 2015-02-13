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
 *   unsigned memencode (void * memory, size_t extent, char const * format, char const * string);
 *
 *   memory.h
 *
 *--------------------------------------------------------------------*/

#ifndef MEMENCODE_SOURCE
#define MEMENCODE_SOURCE

#include <string.h>

#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/error.h"
#include "../pib/pib.h"

static size_t memstring (void * memory, size_t extent, char const * format, char const * string, size_t length)

{
	if (extent < length)
	{
		error (1, ECANCELED, "Overflow at %s %s", format, string);
	}
	memset (memory, 0, length);
	strncpy (memory, string, length-1);
	return (length);
}

size_t memencode (void * memory, size_t extent, char const * format, char const * string)

{
	if (!strcmp (format, "byte"))
	{
		uint8_t * number = (uint8_t *)(memory);
		if (extent < sizeof (* number))
		{
			error (1, ECANCELED, "Overflow at %s %s", format, string);
		}
		* number = (uint8_t)(basespec (string, 0, sizeof (* number)));
		return (sizeof (* number));
	}
	if (!strcmp (format, "word"))
	{
		uint16_t * number = (uint16_t *)(memory);
		if (extent < sizeof (* number))
		{
			error (1, ECANCELED, "Overflow at %s %s", format, string);
		}
		* number = (uint16_t)(basespec (string, 0, sizeof (* number)));
		* number = HTOLE16 (* number);
		return (sizeof (* number));
	}
	if (!strcmp (format, "long"))
	{
		uint32_t * number = (uint32_t *)(memory);
		if (extent < sizeof (* number))
		{
			error (1, ECANCELED, "Overflow at %s %s", format, string);
		}
		* number = (uint32_t)(basespec (string, 0, sizeof (* number)));
		* number = HTOLE32 (* number);
		return (sizeof (* number));
	}
	if (!strcmp (format, "huge"))
	{
		uint64_t * number = (uint64_t *)(memory);
		if (extent < sizeof (* number))
		{
			error (1, ECANCELED, "Overflow at %s %s", format, string);
		}
		* number = (uint64_t)(basespec (string, 0, sizeof (* number)));
		* number = HTOLE64 (* number);
		return (sizeof (* number));
	}
	if (!strcmp (format, "text"))
	{
		extent = (unsigned)(strlen (string));
		memcpy (memory, string, extent);
		return (extent);
	}
	if (!strcmp (format, "data"))
	{
		extent = (unsigned)(dataspec (string, memory, extent));
		return (extent);
	}
	if (!strcmp (format, "fill"))
	{
		extent = (unsigned)(uintspec (string, 0, extent));
		memset (memory, ~0, extent);
		return (extent);
	}
	if (!strcmp (format, "zero"))
	{
		extent = (unsigned)(uintspec (string, 0, extent));
		memset (memory, 0, extent);
		return (extent);
	}
	if (!strcmp (format, "skip"))
	{
		extent = (unsigned)(uintspec (string, 0, extent));
		return (extent);
	}

#if 1

/*
 *   tr-069 specific fields that don't really belong in the PIB;
 */

	if (!strcmp (format, "adminusername") || !strcmp (format, "adminpassword") || !strcmp (format, "accessusername"))
	{
		return (memstring (memory, extent, format, string, PIB_NAME_LEN + 1));
	}
	if (!strcmp (format, "accesspassword"))
	{
		return (memstring (memory, extent, format, string, PIB_HFID_LEN + 1));
	}
	if (!strcmp (format, "username") || !strcmp (format, "password") || !strcmp (format, "url"))
	{
		return (memstring (memory, extent, format, string, PIB_TEXT_LEN + 1));
	}

#endif

#if 1

/*
 *   HPAV specific fields that belong in the PIB;
 */

	if (!strcmp (format, "hfid"))
	{
		return (memstring (memory, extent, format, string, PIB_HFID_LEN));
	}
	if (!strcmp (format, "mac"))
	{
		return (bytespec (string, memory, ETHER_ADDR_LEN));
	}
	if (!strcmp (format, "key"))
	{
		return (bytespec (string, memory, PIB_KEY_LEN));
	}

#endif

	error (1, ENOTSUP, "%s", format);
	return (0);
}


#endif

