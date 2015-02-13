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

/*====================================================================*"
 *
 *   chkpib.c -
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/flags.h"
#include "../tools/error.h"
#include "../tools/files.h"
#include "../key/HPAVKey.h"
#include "../nvm/nvm.h"
#include "../pib/pib.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/checksum32.c"
#include "../tools/fdchecksum32.c"
#include "../tools/checksum32.c"
#include "../tools/hexstring.c"
#include "../tools/hexdecode.c"
#include "../tools/strfbits.c"
#include "../tools/error.c"
#endif

#ifndef MAKEFILE
#include "../key/SHA256Reset.c"
#include "../key/SHA256Block.c"
#include "../key/SHA256Write.c"
#include "../key/SHA256Fetch.c"
#include "../key/HPAVKeyNID.c"
#include "../key/keys.c"
#endif

#ifndef MAKEFILE
#include "../pib/pibpeek1.c"
#include "../pib/pibpeek2.c"
#endif

#ifndef MAKEFILE
#include "../nvm/manifest.c"
#include "../nvm/fdmanifest.c"
#endif

/*====================================================================*
 *
 *   signed pibimage1 (signed fd, char const * filename, flag_t flags);
 *
 *   validate a disk-resident lightning/thunderbolt PIB image; read
 *   the header then verify the checksum and preferred Network
 *   Identifier (NID); return 0 on success or -1 on error;
 *
 *   this is not a thorough check but it detects non-PIB images;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed pibimage1 (signed fd, char const * filename, flag_t flags)

{
	struct simple_pib simple_pib;
	uint8_t NID [HPAVKEY_NID_LEN];
	if (read (fd, &simple_pib, sizeof (simple_pib)) != sizeof (simple_pib))
	{
		if (_allclr (flags, PIB_SILENCE))
		{
			error (0, errno, FILE_CANTREAD, filename);
		}
		return (-1);
	}
	if (_anyset (flags, PIB_VERBOSE))
	{
		printf ("------- %s -------\n", filename);
		if (pibpeek1 (&simple_pib))
		{
			if (_allclr (flags, PIB_SILENCE))
			{
				error (0, 0, PIB_BADVERSION, filename);
			}
			return (-1);
		}
	}
	if (lseek (fd, (off_t)(0)-sizeof (simple_pib), SEEK_CUR) == -1)
	{
		if (_allclr (flags, PIB_SILENCE))
		{
			error (0, errno, FILE_CANTSEEK, filename);
		}
		return (-1);
	}
	if (fdchecksum32 (fd, LE16TOH (simple_pib.PIBLENGTH), 0))
	{
		if (_allclr (flags, PIB_SILENCE))
		{
			error (0, 0, PIB_BADCHECKSUM, filename);
		}
		return (-1);
	}
	HPAVKeyNID (NID, simple_pib.NMK, simple_pib.PreferredNID [HPAVKEY_NID_LEN-1] >> 4);
	if (memcmp (NID, simple_pib.PreferredNID, sizeof (NID)))
	{
		if (_allclr (flags, PIB_SILENCE))
		{
			error (0, 0, PIB_BADNID, filename);
		}
		return (-1);
	}
	return (0);
}


/*====================================================================*
 *
 *   signed pibimage2 (signed fd, char const * filename, uint32_t length, uint32_t checksum, flag_t flags);
 *
 *   validate a disk-resident panther/lynxPIB image; read the header
 *   then verify the checksum and preferred Network Identifuier (NID);
 *   return 0 on success or -1 on error;
 *
 *   this is not a thorough check but it detects non-PIB images;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed pibimage2 (signed fd, char const * filename, uint32_t length, uint32_t checksum, flag_t flags)

{
	struct simple_pib simple_pib;
	struct pib_header pib_header;
	uint8_t NID [HPAVKEY_NID_LEN];
	memset (&pib_header, 0, sizeof (pib_header));
	if (read (fd, &simple_pib, sizeof (simple_pib)) != sizeof (simple_pib))
	{
		if (_allclr (flags, PIB_SILENCE))
		{
			error (0, errno, FILE_CANTREAD, filename);
		}
		return (-1);
	}
	if (_anyset (flags, PIB_VERBOSE))
	{
		struct pib_header * pib_header = (struct pib_header *)(&simple_pib);
		pib_header->PIBLENGTH = HTOLE16((uint16_t)(length));
		printf ("------- %s -------\n", filename);
		if (pibpeek2 (&simple_pib))
		{
			if (_allclr (flags, PIB_SILENCE))
			{
				error (0, 0, PIB_BADVERSION, filename);
			}
			return (-1);
		}
		memset (pib_header, 0, sizeof (* pib_header));
	}
	if (lseek (fd, (off_t)(0)-sizeof (simple_pib), SEEK_CUR) == -1)
	{
		if (_allclr (flags, PIB_SILENCE))
		{
			error (0, errno, FILE_CANTHOME, filename);
		}
		return (-1);
	}
	if (fdchecksum32 (fd, length, checksum))
	{
		if (_allclr (flags, PIB_SILENCE))
		{
			error (0, 0, PIB_BADCHECKSUM, filename);
		}
		return (-1);
	}
	HPAVKeyNID (NID, simple_pib.NMK, simple_pib.PreferredNID [HPAVKEY_NID_LEN-1] >> 4);
	if (memcmp (NID, simple_pib.PreferredNID, sizeof (NID)))
	{
		if (_allclr (flags, PIB_SILENCE))
		{
			error (0, 0, PIB_BADNID, filename);
		}
		return (-1);
	}
	return (0);
}


/*====================================================================*
 *
 *   signed pibchain2 (signed fd, char const * filename, flag_t flags);
 *
 *   open a panther/lynx parameter file and validate it;
 *
 *   traverse a panther/lynx image file looking for PIB images and
 *   validate each one; return 0 on success or -1 on error; errors
 *   occur due to an invalid image chain or a bad parameter block;
 *
 *   this implementation checks the parameter block without reading
 *   the entire block into memory;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed pibchain2 (signed fd, char const * filename, flag_t flags)

{
	struct nvm_header2 nvm_header;
	uint32_t origin = ~0;
	uint32_t offset = 0;
	unsigned module = 0;
	do
	{
		if (read (fd, &nvm_header, sizeof (nvm_header)) != sizeof (nvm_header))
		{
			if (_allclr (flags, PIB_SILENCE))
			{
				error (0, errno, NVM_HDR_CANTREAD, filename, module);
			}
			return (-1);
		}
		if (LE16TOH (nvm_header.MajorVersion) != 1)
		{
			if (_allclr (flags, PIB_SILENCE))
			{
				error (0, 0, NVM_HDR_VERSION, filename, module);
			}
			return (-1);
		}
		if (LE16TOH (nvm_header.MinorVersion) != 1)
		{
			if (_allclr (flags, PIB_SILENCE))
			{
				error (0, 0, NVM_HDR_VERSION, filename, module);
			}
			return (-1);
		}
		if (checksum32 (&nvm_header, sizeof (nvm_header), 0))
		{
			if (_allclr (flags, PIB_SILENCE))
			{
				error (0, 0, NVM_HDR_CHECKSUM, filename, module);
			}
			return (-1);
		}
		if (LE32TOH (nvm_header.PrevHeader) != origin)
		{
			if (_allclr (flags, PIB_SILENCE))
			{
				error (0, 0, NVM_HDR_LINK, filename, module);
			}
			return (-1);
		}
		if (LE32TOH (nvm_header.ImageType) == NVM_IMAGE_PIB)
		{
			return (pibimage2 (fd, filename, LE32TOH (nvm_header.ImageLength), nvm_header.ImageChecksum, flags));
		}
		if (LE32TOH (nvm_header.ImageType) == NVM_IMAGE_MANIFEST)
		{
			if (_anyset (flags, PIB_MANIFEST))
			{
				fdmanifest (fd, filename, &nvm_header, module);
			}
		}
		if (fdchecksum32 (fd, LE32TOH (nvm_header.ImageLength), nvm_header.ImageChecksum))
		{
			if (_allclr (flags, PIB_SILENCE))
			{
				error (0, 0, NVM_IMG_CHECKSUM, filename, module);
			}
			return (-1);
		}
		origin = offset;
		offset = LE32TOH (nvm_header.NextHeader);
		module++;
	}
	while (~nvm_header.NextHeader);
	if (lseek (fd, 0, SEEK_CUR) != lseek (fd, 0, SEEK_END))
	{
		if (_allclr (flags, NVM_SILENCE))
		{
			error (0, errno, NVM_HDR_LINK, filename, module);
		}
		return (-1);
	}
	error (0, 0, "%s has no PIB", filename);
	return (-1);
}


/*====================================================================*
 *
 *   signed chkpib (char const * filename, flag_t flags);
 *
 *   open a named file and determine if it is a valid thunderbolt,
 *   lightning, panther or lynx PIB;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed chkpib (char const * filename, flag_t flags)

{
	uint32_t version;
	signed status = 0;
	signed fd;
	if ((fd = open (filename, O_BINARY|O_RDONLY)) == -1)
	{
		if (_allclr (flags, PIB_SILENCE))
		{
			error (0, errno, FILE_CANTOPEN, filename);
		}
		return (-1);
	}
	if (read (fd, &version, sizeof (version)) != sizeof (version))
	{
		if (_allclr (flags, PIB_SILENCE))
		{
			error (0, errno, FILE_CANTREAD, filename);
		}
		return (-1);
	}
	if (lseek (fd, 0, SEEK_SET))
	{
		if (_allclr (flags, PIB_SILENCE))
		{
			error (0, errno, FILE_CANTHOME, filename);
		}
		return (-1);
	}
	if (LE32TOH (version) == 0x60000000)
	{
		if (_allclr (flags, PIB_SILENCE))
		{
			error (0, errno, FILE_WONTREAD, filename);
		}
		status = -1;;
	}
	else if (LE32TOH (version) == 0x00010001)
	{
		status = pibchain2 (fd, filename, flags);
	}
	else
	{
		status = pibimage1 (fd, filename, flags);
	}
	close (fd);
	return (status);
}


/*====================================================================*
 *
 *   int main (int argc, char const * argv []);
 *
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv [])

{
	static char const * optv [] =
	{
		"mqv",
		"file [file] [...]",
		"Qualcomm Atheros PLC Parameter File Inspector",
		"m\tdisplay manifest",
		"q\tquiet",
		"v\tverbose messages",
		(char const *) (0)
	};
	flag_t flags = (flag_t)(0);
	signed state = 0;
	signed c;
	optind = 1;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		case 'm':
			_setbits (flags, PIB_MANIFEST);
			break;
		case 'q':
			_setbits (flags, PIB_SILENCE);
			break;
		case 'v':
			_setbits (flags, PIB_VERBOSE);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	while ((argc) && (* argv))
	{
		errno = 0;
		if (chkpib (* argv, flags))
		{
			state = 1;
		}
		else if (_allclr (flags, (PIB_VERBOSE | PIB_SILENCE | PIB_MANIFEST)))
		{
			printf ("%s looks good\n", * argv);
		}
		argc--;
		argv++;
	}
	return (state);
}

