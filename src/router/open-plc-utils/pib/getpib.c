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
 *   getpib.c - PIB Data Extractor
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/error.h"
#include "../tools/types.h"
#include "../tools/flags.h"
#include "../tools/files.h"
#include "../pib/pib.h"
#include "../nvm/nvm.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/uintspec.c"
#include "../tools/basespec.c"
#include "../tools/todigit.c"
#include "../tools/hexout.c"
#include "../tools/error.c"
#include "../tools/checksum32.c"
#include "../tools/fdchecksum32.c"
#endif

#ifndef MAKEFILE
#include "../nvm/nvmseek2.c"
#endif

/*====================================================================*
 *   constants;
 *--------------------------------------------------------------------*/

#define GETPIB_COMMA ' '
#define GETPIB_TOOBIG "object '%s' exceeds extent of " SIZE_T_SPEC " bytes"
#define GETPIB_NOSIZE "object '%s' has no length"

#define GETPIB_VERBOSE (1 << 0)
#define GETPIB_SILENCE (1 << 1)
#define GETPIB_NEWLINE (1 << 2)

/*====================================================================*
 *
 *   void getmemory (byte const * memory, size_t extent, char const * object, size_t length);
 *
 *--------------------------------------------------------------------*/

static void getmemory (byte const * memory, size_t extent, char const * object, size_t length) 

{ 
	if (length > extent) 
	{ 
		error (1, ECANCELED, GETPIB_TOOBIG, object, length); 
	} 
	hexout (memory, length, ':', '\0', stdout); 
	return; 
} 

/*====================================================================*
 *
 *   void getstring (byte const * memory, size_t extent, char const * object, size_t length);
 *
 *--------------------------------------------------------------------*/

static void getstring (byte const * memory, size_t extent, char const * object, size_t length) 

{ 
	char const * string = (char const *) (memory); 
	if (length > extent) 
	{ 
		error (1, ECANCELED, GETPIB_TOOBIG, object, length); 
	} 
	while (isprint (* string) && (length--)) 
	{ 
		putc (* string++, stdout); 
	} 
	return; 
} 

/*====================================================================*
 *
 *   void snatch (int argc, char const * argv [], byte const * memory, size_t extent, char comma);
 *
 *   extract and print the specified data objects from memory; comma
 *   delimits consecutive objects on output; 
 *
 *--------------------------------------------------------------------*/

static void snatch (int argc, char const * argv [], byte const * memory, size_t extent, char comma) 

{ 
	size_t length = 0; 
	size_t offset = 0; 
	if (! (argc) || ! (* argv)) 
	{ 
		error (1, ECANCELED, "Need an offset"); 
	} 
	offset = (size_t) (basespec (* argv, 16, sizeof (uint32_t))); 
	if (offset > extent) 
	{ 
		error (1, ECANCELED, "offset " SIZE_T_SPEC " exceeds extent of " SIZE_T_SPEC " bytes", offset, extent); 
	} 
	memory += offset; 
	extent -= offset; 
	argc--; 
	argv++; 
	while ((argc) && (* argv)) 
	{ 
		char const * object = * argv; 
		argc--; 
		argv++; 
		if (! strcmp (object, "byte")) 
		{ 
			uint8_t * number = (uint8_t *) (memory); 
			if (sizeof (* number) > extent) 
			{ 
				error (1, ECANCELED, GETPIB_TOOBIG, object, extent); 
			} 
			printf ("%u", * number); 
			memory += sizeof (* number); 
			extent -= sizeof (* number); 
		} 
		else if (! strcmp (object, "word")) 
		{ 
			uint16_t * number = (uint16_t *) (memory); 
			if (sizeof (* number) > extent) 
			{ 
				error (1, ECANCELED, GETPIB_TOOBIG, object, extent); 
			} 
			printf ("%u", LE16TOH (* number)); 
			memory += sizeof (* number); 
			extent -= sizeof (* number); 
		} 
		else if (! strcmp (object, "long")) 
		{ 
			uint32_t * number = (uint32_t *) (memory); 
			if (sizeof (* number) > extent) 
			{ 
				error (1, ECANCELED, GETPIB_TOOBIG, object, extent); 
			} 
			printf ("%u", LE32TOH (* number)); 
			memory += sizeof (* number); 
			extent -= sizeof (* number); 
		} 
		else if (! strcmp (object, "huge")) 
		{ 
			uint64_t * number = (uint64_t *) (memory); 
			if (sizeof (* number) > extent) 
			{ 
				error (1, ECANCELED, GETPIB_TOOBIG, object, extent); 
			} 
			printf ("%llu", LE64TOH (* number)); 
			memory += sizeof (* number); 
			extent -= sizeof (* number); 
		} 

#if 1

		else if (! strcmp (object, "xbyte")) 
		{ 
			uint8_t * number = (uint8_t *) (memory); 
			if (sizeof (* number) > extent) 
			{ 
				error (1, ECANCELED, GETPIB_TOOBIG, object, extent); 
			} 
			printf ("0x%02X", * number); 
			memory += sizeof (* number); 
			extent -= sizeof (* number); 
		} 
		else if (! strcmp (object, "xword")) 
		{ 
			uint16_t * number = (uint16_t *) (memory); 
			if (sizeof (* number) > extent) 
			{ 
				error (1, ECANCELED, GETPIB_TOOBIG, object, extent); 
			} 
			printf ("0x%04X", LE16TOH (* number)); 
			memory += sizeof (* number); 
			extent -= sizeof (* number); 
		} 
		else if (! strcmp (object, "xlong")) 
		{ 
			uint32_t * number = (uint32_t *) (memory); 
			if (sizeof (* number) > extent) 
			{ 
				error (1, ECANCELED, GETPIB_TOOBIG, object, extent); 
			} 
			printf ("0x%08X", LE32TOH (* number)); 
			memory += sizeof (* number); 
			extent -= sizeof (* number); 
		} 
		else if (! strcmp (object, "xhuge")) 
		{ 
			uint64_t * number = (uint64_t *) (memory); 
			if (sizeof (* number) > extent) 
			{ 
				error (1, ECANCELED, GETPIB_TOOBIG, object, extent); 
			} 
			printf ("0x%016llX", LE64TOH (* number)); 
			memory += sizeof (* number); 
			extent -= sizeof (* number); 
		} 

#endif

		else if (! strcmp (object, "mac")) 
		{ 
			length = ETHER_ADDR_LEN; 
			if (length > extent) 
			{ 
				error (1, ECANCELED, GETPIB_TOOBIG, object, extent); 
			} 
			getmemory (memory, extent, object, length); 
			memory += length; 
			extent -= length; 
		} 
		else if (! strcmp (object, "key")) 
		{ 
			length = PIB_KEY_LEN; 
			if (length > extent) 
			{ 
				error (1, ECANCELED, GETPIB_TOOBIG, object, extent); 
			} 
			getmemory (memory, extent, object, length); 
			memory += length; 
			extent -= length; 
		} 
		else if (! strcmp (object, "hfid")) 
		{ 
			length = PIB_HFID_LEN; 
			if (length > extent) 
			{ 
				error (1, ECANCELED, GETPIB_TOOBIG, object, extent); 
			} 
			getstring (memory, extent, object, length); 
			memory += length; 
			extent -= length; 
		} 

#if 1

		else if (! strcmp (object, "adminusername") || ! strcmp (object, "adminpassword") || ! strcmp (object, "accessusername")) 
		{ 
			length = PIB_NAME_LEN +  1; 
			if (length > extent) 
			{ 
				error (1, ECANCELED, GETPIB_TOOBIG, object, extent); 
			} 
			getstring (memory, extent, object, length); 
			memory += length; 
			extent -= length; 
		} 
		else if (! strcmp (object, "accesspassword")) 
		{ 
			length = PIB_HFID_LEN +  1; 
			if (length > extent) 
			{ 
				error (1, ECANCELED, GETPIB_TOOBIG, object, extent); 
			} 
			getstring (memory, extent, object, length); 
			memory += length; 
			extent -= length; 
		} 
		else if (! strcmp (object, "username") || ! strcmp (object, "password") || ! strcmp (object, "url")) 
		{ 
			length = PIB_TEXT_LEN +  1; 
			getstring (memory, extent, object, length); 
			memory += length; 
			extent -= length; 
		} 

#endif

		else if (! strcmp (object, "data")) 
		{ 
			if (! * argv) 
			{ 
				error (1, EINVAL, GETPIB_NOSIZE, object); 
			} 
			length = (unsigned) (uintspec (* argv, 1, extent)); 
			hexout (memory, length, 0, 0, stdout); 
			memory += length; 
			extent -= length; 
			argc--; 
			argv++; 
		} 
		else if (! strcmp (object, "text")) 
		{ 
			if (! * argv) 
			{ 
				error (1, EINVAL, GETPIB_NOSIZE, object); 
			} 
			length = (unsigned) (uintspec (* argv, 1, extent)); 
			getstring (memory, extent, object, length); 
			memory += length; 
			extent -= length; 
			argc--; 
			argv++; 
		} 
		else if (! strcmp (object, "skip")) 
		{ 
			if (! * argv) 
			{ 
				error (1, EINVAL, GETPIB_NOSIZE, object); 
			} 
			length = (unsigned) (uintspec (* argv, 1, extent)); 
			memory += length; 
			extent -= length; 
			argc--; 
			argv++; 
			continue; 
		} 
		else 
		{ 
			error (1, ENOTSUP, "%s", object); 
		} 
		if ((argc) && (* argv)) 
		{ 
			putc (comma, stdout); 
		} 
	} 
	return; 
} 

/*====================================================================*
 *
 *   signed pibimage1 (int argc, char const * argv [], char comma);
 * 
 *   read an entire flat parameter file into memory, edit it, save 
 *   it and display it;
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed pibimage1 (int argc, char const * argv [], char comma) 

{ 
	signed fd; 
	off_t extent; 
	byte * memory; 
	if ((fd = open (* argv, O_BINARY | O_RDWR)) == - 1) 
	{ 
		error (1, errno, FILE_CANTOPEN, * argv); 
	} 
	if ((extent = lseek (fd, 0, SEEK_END)) == - 1) 
	{ 
		error (1, errno, FILE_CANTSIZE, * argv); 
	} 
	if (lseek (fd, 0, SEEK_SET)) 
	{ 
		error (1, errno, FILE_CANTHOME, * argv); 
	} 
	if (! (memory = malloc (extent))) 
	{ 
		error (1, errno, FILE_CANTLOAD, * argv); 
	} 
	if (read (fd, memory, extent) != extent) 
	{ 
		error (1, errno, FILE_CANTREAD, * argv); 
	} 
	close (fd); 
	snatch (argc - 1, argv +  1, memory, extent, comma); 
	free (memory); 
	return (0); 
} 

/*====================================================================*
 *
 *   signed pibimage2 (int argc, char const * argv [], char comma);
 * 
 *   read an entire flat parameter file into memory, edit it, save 
 *   it and display it;
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed pibimage2 (int argc, char const * argv [], char comma) 

{ 
	struct nvm_header2 header; 
	signed fd; 
	off_t extent; 
	byte * memory; 
	if ((fd = open (* argv, O_BINARY | O_RDWR)) == - 1) 
	{ 
		error (1, errno, FILE_CANTOPEN, * argv); 
	} 
	if (nvmseek2 (fd, * argv, & header, NVM_IMAGE_PIB)) 
	{ 
		error (1, errno, "Can't find PIB image in %s", * argv); 
	} 
	extent = LE32TOH (header.ImageLength); 
	if (! (memory = malloc (extent))) 
	{ 
		error (1, errno, FILE_CANTLOAD, * argv); 
	} 
	if (read (fd, memory, extent) != extent) 
	{ 
		error (1, errno, FILE_CANTREAD, * argv); 
	} 
	close (fd); 
	snatch (argc - 1, argv +  1, memory, extent, comma); 
	free (memory); 
	return (0); 
} 

/*====================================================================*
 *
 *   signed function (int argc, char const * argv [], char comma);
 *
 *   call an appropriate parameter edit function based on the file 
 *   header;
 *
 *   older parameter files are flat with their own header; newer ones
 *   are image chains where one of image contains the parameter block;
 *
 *   
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed function (int argc, char const * argv [], char comma) 

{ 
	uint32_t version; 
	signed status; 
	signed fd; 
	if ((fd = open (* argv, O_BINARY | O_RDWR)) == - 1) 
	{ 
		error (1, errno, FILE_CANTOPEN, * argv); 
	} 
	if (read (fd, & version, sizeof (version)) != sizeof (version)) 
	{ 
		error (1, errno, FILE_CANTREAD, * argv); 
	} 
	close (fd); 
	if (LE32TOH (version) == 0x00010001) 
	{ 
		status = pibimage2 (argc, argv, comma); 
	} 
	else 
	{ 
		status = pibimage1 (argc, argv, comma); 
	} 
	return (status); 
} 

/*====================================================================*
 *
 *   int main (int argc, char const * argv []);
 *   
 *
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv []) 

{ 
	static char const * optv [] = 
	{ 
		"c:qvn", 
		"file offset type [size]\n\n\tstandard-length types are 'byte'|'word'|'long'|'huge'|'hfid'|'mac'|'key'\n\tvariable-length types are 'data'|'text'|'skip' and need a size", 
		"PIB Data Extractor", 
		"c c\tobject separator is (c) [" LITERAL (GETPIB_COMMA) "]", 
		"n\tappend newline", 
		"q\tquiet mode", 
		"v\tverbose mode", 
		(char const *) (0)
	}; 
	flag_t flags = (flag_t) (0); 
	char comma = GETPIB_COMMA; 
	signed c; 
	optind = 1; 
	opterr = 1; 
	while (~ (c = getoptv (argc, argv, optv))) 
	{ 
		switch (c) 
		{ 
		case 'c': 
			comma = * optarg; 
			break; 
		case 'n': 
			_setbits (flags, GETPIB_NEWLINE); 
			break; 
		case 'q': 
			_setbits (flags, GETPIB_SILENCE); 
			break; 
		case 'v': 
			_setbits (flags, GETPIB_VERBOSE); 
			break; 
		default: 
			break; 
		} 
	} 
	argc -= optind; 
	argv += optind; 
	if ((argc) && (* argv)) 
	{ 
		function (argc, argv, comma); 
		if (_anyset (flags, GETPIB_NEWLINE)) 
		{ 
			putc ('\n', stdout); 
		} 
	} 
	return (0); 
} 
