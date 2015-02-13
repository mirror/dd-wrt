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
 *   modpib.c -
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/number.h"
#include "../tools/flags.h"
#include "../tools/error.h"
#include "../tools/files.h"
#include "../tools/chars.h"
#include "../plc/plc.h"
#include "../key/HPAVKey.h"
#include "../key/keys.h"
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
#include "../tools/uintspec.c"
#include "../tools/hexencode.c"
#include "../tools/hexdecode.c"
#include "../tools/hexstring.c"
#include "../tools/todigit.c"
#include "../tools/memincr.c"
#include "../tools/error.c"
#endif

#ifndef MAKEFILE
#include "../pib/pibfile.c"
#include "../pib/pibfile1.c"
#include "../pib/pibfile2.c"
#include "../pib/pibpeek1.c"
#include "../pib/pibpeek2.c"
#endif

#ifndef MAKEFILE
#include "../key/HPAVKeyNID.c"
#include "../key/SHA256Reset.c"
#include "../key/SHA256Write.c"
#include "../key/SHA256Block.c"
#include "../key/SHA256Fetch.c"
#include "../key/keys.c"
#endif

/*====================================================================*
 *   constants;
 *--------------------------------------------------------------------*/

#define MEMBERSHIP_OFFSET 0x00001E8C
#define MEMBERSHIP_STATUS 0x00000000

/*====================================================================*
 *
 *   signed pibimage1 (struct _file_ * file, simple_pib *sample_pib, signed level, flag_t flags);
 *
 *   modify selected PIB header values and compute a new checksum;
 *   this function assumes that the file is open and positioned to
 *   the start of the parameter block;
 *
 *
 *   Contributor(s):
 *	Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed pibimage1 (struct _file_ * file, simple_pib * sample_pib, signed level, flag_t flags)

{
	struct simple_pib simple_pib;
	memset (&simple_pib, 0, sizeof (simple_pib));
	if (read (file->file, &simple_pib, sizeof (simple_pib)) != sizeof (simple_pib))
	{
		error (1, errno, FILE_CANTREAD, file->name);
	}
	if (_anyset (flags, PIB_MAC))
	{
		memcpy (simple_pib.MAC, sample_pib->MAC, sizeof (simple_pib.MAC));
	}
	if (_anyset (flags, PIB_MACINC))
	{
		memincr (simple_pib.MAC, sizeof (simple_pib.MAC));
	}
	if (_anyset (flags, PIB_DAK))
	{
		memcpy (simple_pib.DAK, sample_pib->DAK, sizeof (simple_pib.DAK));
	}
	if (_anyset (flags, PIB_NMK))
	{
		memcpy (simple_pib.NMK, sample_pib->NMK, sizeof (simple_pib.NMK));
	}
	if (_anyset (flags, PIB_NETWORK))
	{
		memcpy (simple_pib.NET, sample_pib->NET, sizeof (simple_pib.NET));
	}
	if (_anyset (flags, PIB_MFGSTRING))
	{
		memcpy (simple_pib.MFG, sample_pib->MFG, sizeof (simple_pib.MFG));
	}
	if (_anyset (flags, PIB_USER))
	{
		memcpy (simple_pib.USR, sample_pib->USR, sizeof (simple_pib.USR));
	}
	if (_anyset (flags, PIB_CCO_MODE))
	{
		simple_pib.CCoSelection = sample_pib->CCoSelection;
	}
	if (_anyset (flags, PIB_NMK | PIB_NID))
	{

#if 0

/*
 *	clear the AVLNMembership bit whenever the NMK or NID change; this is an important
 *	step because it prevents false network association;
 */

		if (BE16TOH (simple_pib.PIBVERSION) > 0x0200)
		{
			uint32_t membership = MEMBERSHIP_STATUS;
			if (lseek (file->file, MEMBERSHIP_OFFSET, SEEK_SET) != MEMBERSHIP_OFFSET)
			{
				error (1, errno, FILE_CANTHOME, file->name);
			}
			if (write (file->file, &membership, sizeof (membership)) != sizeof (membership))
			{
				error (1, errno, FILE_CANTSAVE, file->name);
			}
		}

#endif

		if (_allclr (flags, PIB_NID))
		{
			level = simple_pib.PreferredNID [HPAVKEY_NID_LEN-1] >> 4;
		}
		HPAVKeyNID (simple_pib.PreferredNID, simple_pib.NMK, level & 1);
	}
	if (lseek (file->file, (off_t)(0) - sizeof (simple_pib), SEEK_CUR) == -1)
	{
		error (1, errno, FILE_CANTHOME, file->name);
	}
	if (write (file->file, &simple_pib, sizeof (simple_pib)) != sizeof (simple_pib))
	{
		error (1, errno, FILE_CANTSAVE, file->name);
	}
	if (lseek (file->file, (off_t)(0) - sizeof (simple_pib), SEEK_CUR) == -1)
	{
		error (1, errno, FILE_CANTHOME, file->name);
	}
	simple_pib.CHECKSUM = fdchecksum32 (file->file, LE16TOH (simple_pib.PIBLENGTH), simple_pib.CHECKSUM);
	if (lseek (file->file, (off_t)(0) - LE16TOH (simple_pib.PIBLENGTH), SEEK_CUR) == -1)
	{
		error (1, errno, FILE_CANTHOME, file->name);
	}
	if (write (file->file, &simple_pib, sizeof (simple_pib)) != sizeof (simple_pib))
	{
		error (1, errno, FILE_CANTSAVE, file->name);
	}
	if (_anyset (flags, PIB_VERBOSE))
	{
		pibpeek1 (&simple_pib);
	}
	return (0);
}


/*====================================================================*
 *
 *   signed pibimage2 (struct _file_ * file, simple_pib *sample_pib, signed level, flag_t flags);
 *
 *   modify selected PIB header values but do not compute a checksum;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed pibimage2 (struct _file_ * file, simple_pib * sample_pib, signed level, flag_t flags)

{
	struct simple_pib simple_pib;
	memset (&simple_pib, 0, sizeof (simple_pib));
	if (read (file->file, &simple_pib, sizeof (simple_pib)) != sizeof (simple_pib))
	{
		error (1, errno, FILE_CANTREAD, file->name);
	}
	if (_anyset (flags, PIB_MAC))
	{
		memcpy (simple_pib.MAC, sample_pib->MAC, sizeof (simple_pib.MAC));
	}
	if (_anyset (flags, PIB_MACINC))
	{
		memincr (simple_pib.MAC, sizeof (simple_pib.MAC));
	}
	if (_anyset (flags, PIB_DAK))
	{
		memcpy (simple_pib.DAK, sample_pib->DAK, sizeof (simple_pib.DAK));
	}
	if (_anyset (flags, PIB_NMK))
	{
		memcpy (simple_pib.NMK, sample_pib->NMK, sizeof (simple_pib.NMK));
	}
	if (_anyset (flags, PIB_NETWORK))
	{
		memcpy (simple_pib.NET, sample_pib->NET, sizeof (simple_pib.NET));
	}
	if (_anyset (flags, PIB_MFGSTRING))
	{
		memcpy (simple_pib.MFG, sample_pib->MFG, sizeof (simple_pib.MFG));
	}
	if (_anyset (flags, PIB_USER))
	{
		memcpy (simple_pib.USR, sample_pib->USR, sizeof (simple_pib.USR));
	}
	if (_anyset (flags, PIB_CCO_MODE))
	{
		simple_pib.CCoSelection = sample_pib->CCoSelection;
	}
	if (_anyset (flags, PIB_NMK | PIB_NID))
	{

#if 0

/*
 *	clear the AVLNMembership bit whenever the NMK or NID change; this is an important
 *	step because it prevents false network association;
 */

		if (BE16TOH (simple_pib.PIBVERSION) > 0x0200)
		{
			uint32_t membership = MEMBERSHIP_STATUS;
			if (lseek (file->file, MEMBERSHIP_OFFSET, SEEK_SET) != MEMBERSHIP_OFFSET)
			{
				error (1, errno, FILE_CANTHOME, file->name);
			}
			if (write (file->file, &membership, sizeof (membership)) != sizeof (membership))
			{
				error (1, errno, FILE_CANTSAVE, file->name);
			}
		}

#endif

		if (_allclr (flags, PIB_NID))
		{
			level = simple_pib.PreferredNID [HPAVKEY_NID_LEN-1] >> 4;
		}
		HPAVKeyNID (simple_pib.PreferredNID, simple_pib.NMK, level & 1);
	}
	if (lseek (file->file, (off_t)(0) - sizeof (simple_pib), SEEK_CUR) == -1)
	{
		error (1, errno, FILE_CANTHOME, file->name);
	}
	if (write (file->file, &simple_pib, sizeof (simple_pib)) != sizeof (simple_pib))
	{
		error (1, errno, FILE_CANTSAVE, file->name);
	}
	if (lseek (file->file, (off_t)(0) - sizeof (simple_pib), SEEK_CUR) == -1)
	{
		error (1, errno, FILE_CANTHOME, file->name);
	}
	if (_anyset (flags, PIB_VERBOSE))
	{
		pibpeek2 (&simple_pib);
	}
	return (0);
}


/*====================================================================*
 *
 *   signed pibchain2 (struct _file_ * file, simple_pib * sample_pib, signed level, flag_t flags);
 *
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed pibchain2 (struct _file_ * file, simple_pib * sample_pib, signed level, flag_t flags)

{
	unsigned module = 0;
	struct nvm_header2 nvm_header;
	uint32_t prev = ~0;
	uint32_t next = 0;
	if (lseek (file->file, 0, SEEK_SET))
	{
		error (1, errno, NVM_IMG_CHECKSUM, file->name, module);
	}
	do
	{
		if (read (file->file, &nvm_header, sizeof (nvm_header)) != sizeof (nvm_header))
		{
			error (1, errno, NVM_HDR_CANTREAD, file->name, module);
		}
		if (LE16TOH (nvm_header.MajorVersion) != 1)
		{
			error (1, errno, NVM_HDR_VERSION, file->name, module);
		}
		if (LE16TOH (nvm_header.MinorVersion) != 1)
		{
			error (1, errno, NVM_HDR_VERSION, file->name, module);
		}
		if (checksum32 (&nvm_header, sizeof (nvm_header), 0))
		{
			error (1, errno, NVM_HDR_CHECKSUM, file->name, module);
		}
		if (LE32TOH (nvm_header.PrevHeader) != prev)
		{
			error (1, errno, NVM_HDR_LINK, file->name, module);
		}
		if (LE32TOH (nvm_header.ImageType) == NVM_IMAGE_PIB)
		{
			pibimage2 (file, sample_pib, level, flags);
			nvm_header.ImageChecksum = fdchecksum32 (file->file, LE32TOH (nvm_header.ImageLength), 0);
			if (lseek (file->file, (off_t)(0) - LE32TOH (nvm_header.ImageLength), SEEK_CUR) == -1)
			{
				error (1, errno, FILE_CANTHOME, file->name);
			}
			nvm_header.HeaderChecksum = checksum32 (&nvm_header, sizeof (nvm_header), nvm_header.HeaderChecksum);
			if (lseek (file->file, (off_t)(0) - sizeof (nvm_header), SEEK_CUR) == -1)
			{
				error (1, errno, FILE_CANTHOME, file->name);
			}
			if (write (file->file, &nvm_header, sizeof (nvm_header)) != sizeof (nvm_header))
			{
				error (1, errno, FILE_CANTSAVE, file->name);
			}
			if (lseek (file->file, (off_t)(0) - sizeof (nvm_header), SEEK_CUR) == -1)
			{
				error (1, errno, FILE_CANTHOME, file->name);
			}
			break;
		}
		if (fdchecksum32 (file->file, LE32TOH (nvm_header.ImageLength), nvm_header.ImageChecksum))
		{
			error (1, errno, NVM_IMG_CHECKSUM, file->name, module);
		}
		prev = next;
		next = LE32TOH (nvm_header.NextHeader);
		module++;
	}
	while (~nvm_header.NextHeader);
	return (0);
}


/*====================================================================*
 *
 *   signed function (char const * filename, struct simple_pib * simple_pib, unsigned level, flag_t flags);
 *
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

static signed function (char const * filename, struct simple_pib * sample_pib, unsigned level, flag_t flags)

{
	uint32_t version;
	signed status;
	struct _file_ file;
	file.name = filename;
	if ((file.file = open (file.name, O_BINARY|O_RDWR, FILE_FILEMODE)) == -1)
	{
		if (_allclr (flags, PIB_SILENCE))
		{
			error (0, errno, FILE_CANTOPEN, file.name);
		}
		return (-1);
	}
	if (read (file.file, &version, sizeof (version)) != sizeof (version))
	{
		if (_allclr (flags, PIB_SILENCE))
		{
			error (0, errno, FILE_CANTREAD, file.name);
		}
		return (-1);
	}
	if (lseek (file.file, 0, SEEK_SET))
	{
		error (1, errno, FILE_CANTHOME, file.name);
	}
	if (LE32TOH (version) == 0x00010001)
	{
		status = pibchain2 (&file, sample_pib, level, flags);
	}
	else
	{
		status = pibimage1 (&file, sample_pib, level, flags);
	}
	close (file.file);
	return (status);
}


/*====================================================================*
 *
 *   int main (int argc, char const * argv [])
 *
 *
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv [])

{
	extern const struct key keys [];
	static char const * optv [] =
	{
		"C:D:L:M:N:S:T:U:v",
		"file [file] [...]",
		"modify selected PIB parameters and update checksum",
		"C n\tCCo Selection is n",
		"D x\tDAK as xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx",
		"L n\tsecurity level is n",
		"M x\tMAC as xx:xx:xx:xx:xx:xx",
		"N x\tNMK as xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx:xx",
		"S s\tMFG string is s",
		"T s\tNET string is s",
		"U s\tUSR string is s",
		"v\tverbose messages",
		(char const *) (0)
	};
	struct simple_pib sample_pib;
	signed level = 0;
	signed state = 0;
	flag_t flags = (flag_t)(0);
	signed c;
	optind = 1;
	memset (&sample_pib, 0, sizeof (sample_pib));
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		char const * sp;
		switch ((char) (c))
		{
		case 'C':
			_setbits (flags, PIB_CCO_MODE);
			sample_pib.CCoSelection = (uint8_t)(uintspec (optarg, 0, 4));
			break;
		case 'D':
			_setbits (flags, PIB_DAK);
			if (!strcmp (optarg, "none"))
			{
				memcpy (sample_pib.DAK, keys [0].DAK, sizeof (sample_pib.DAK));
				break;
			}
			if (!strcmp (optarg, "key1"))
			{
				memcpy (sample_pib.DAK, keys [1].DAK, sizeof (sample_pib.DAK));
				break;
			}
			if (!strcmp (optarg, "key2"))
			{
				memcpy (sample_pib.DAK, keys [2].DAK, sizeof (sample_pib.DAK));
				break;
			}
			if (!hexencode (sample_pib.DAK, sizeof (sample_pib.DAK), optarg))
			{
				error (1, errno, PLC_BAD_DAK, optarg);
			}
			break;
		case 'L':
			level = (uint8_t)(uintspec (optarg, 0, 1));
			_setbits (flags, PIB_NID);
			break;
		case 'M':
			if (!strcmp (optarg, "auto"))
			{
				_setbits (flags, PIB_MACINC);
				break;
			}
			if (!strcmp (optarg, "next"))
			{
				_setbits (flags, PIB_MACINC);
				break;
			}
			if (!strcmp (optarg, "plus"))
			{
				_setbits (flags, PIB_MACINC);
				break;
			}
			if (!hexencode (sample_pib.MAC, sizeof (sample_pib.MAC), optarg))
			{
				error (1, errno, PLC_BAD_MAC, optarg);
			}
			_setbits (flags, PIB_MAC);
			break;
		case 'N':
			_setbits (flags, PIB_NMK);
			if (!strcmp (optarg, "key0"))
			{
				memcpy (sample_pib.NMK, keys [0].NMK, sizeof (sample_pib.NMK));
				break;
			}
			if (!strcmp (optarg, "key1"))
			{
				memcpy (sample_pib.NMK, keys [1].NMK, sizeof (sample_pib.NMK));
				break;
			}
			if (!strcmp (optarg, "key2"))
			{
				memcpy (sample_pib.NMK, keys [2].NMK, sizeof (sample_pib.NMK));
				break;
			}
			if (!hexencode (sample_pib.NMK, sizeof (sample_pib.NMK), optarg))
			{
				error (1, errno, PLC_BAD_NMK, optarg);
			}
			break;
		case 'S':
			for (sp = optarg; isprint (*sp); ++sp);
			if (*sp)
			{
				error (1, EINVAL, "NMK contains illegal characters");
			}
			if ((sp - optarg) > (signed)(sizeof (sample_pib.MFG) - 1))
			{
				error (1, 0, "Manufacturing string is at most %u chars", (unsigned)(sizeof (sample_pib.MFG) - 1));
			}
			memcpy (sample_pib.MFG, optarg, strlen (optarg));
			_setbits (flags, PIB_MFGSTRING);
			break;
		case 'T':
			for (sp = optarg; isprint (*sp); ++sp);
			if (*sp)
			{
				error (1, EINVAL, "NMK contains illegal characters");
			}
			if ((sp - optarg) > (signed)(sizeof (sample_pib.NET) - 1))
			{
				error (1, 0, "NET string is at most %u chars", (unsigned)(sizeof (sample_pib.NET) - 1));
			}
			memcpy (sample_pib.NET, optarg, strlen (optarg));
			_setbits (flags, PIB_NETWORK);
			break;
		case 'U':
			for (sp = optarg; isprint (*sp); ++sp);
			if (*sp)
			{
				error (1, EINVAL, "NMK contains illegal characters");
			}
			if ((sp - optarg) > (signed)(sizeof (sample_pib.USR) - 1))
			{
				error (1, 0, "USR string is at most %u chars", (unsigned)(sizeof (sample_pib.USR) - 1));
			}
			memcpy (sample_pib.USR, optarg, strlen (optarg));
			_setbits (flags, PIB_USER);
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
		if (function (* argv, &sample_pib, level, flags))
		{
			state = 1;
		}
		argc--;
		argv++;
	}
	exit (state);
}

