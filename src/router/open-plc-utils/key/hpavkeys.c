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

#define _GETOPT_H

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/version.h"
#include "../tools/number.h"
#include "../tools/types.h"
#include "../tools/flags.h"
#include "../tools/error.h"
#include "../key/HPAVKey.h"
#include "../key/SHA256.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/version.c"
#include "../tools/uintspec.c"
#include "../tools/todigit.c"
#include "../tools/hexout.c"
#include "../tools/error.c"
#endif

#ifndef MAKEFILE
#include "../key/HPAVKeyDAK.c"
#include "../key/HPAVKeyNMK.c"
#include "../key/HPAVKeyNID.c"
#include "../key/HPAVKeySHA.c"
#include "../key/HPAVKeyOut.c"
#include "../key/SHA256.c"
#endif

/*====================================================================*
 *
 *   void generate (signed type, signed level, flag_t flags);
 *
 *   read pass phrases from stdin, compute the digest for each and
 *   print both on stdout; ignore illegal pass phrases;
 *
 *   a pass phrase consists of consecutive ASCII characters in the
 *   range 0x20 through 0x7F; other characters are noise and serve
 *   to delimit the phrase; phrases less than HPAVKEY_PHRASE_MIN characters
 *   or more than HPAVKEY_PHRASE_MAX characters are also illegal;
 *
 *   effectively, each text line is a candidate phrase where spaces
 *   are legal and significant; tabs characters are illegal and act
 *   as line breaks;
 *
 *   detected errors are reported along with the input line number;
 *
 *
 *--------------------------------------------------------------------*/

void generate (signed class, signed level, flag_t flags)

{
	uint8_t digest [SHA256_DIGEST_LENGTH];
	char phrase [BUFSIZ];
	char * sp = phrase;
	unsigned line = 1;
	signed c = getc (stdin);
	while (c != EOF)
	{
		if (!isprint (c))
		{
			if (c == '\n')
			{
				line++;
			}
			c = getc (stdin);
			continue;
		}
		sp = phrase;
		while (isprint (c))
		{
			if ((sp - phrase) < (signed)(sizeof (phrase) - 1))
			{
				*sp++ = c;
			}
			c = getc (stdin);
		}
		if ((c != '\r') && (c != '\n') && (c != EOF))
		{
			error (0, ENOTSUP, "Illegal characters on line %d", line);
			continue;
		}
		*sp = (char)(0);
		if (_anyset (flags, HPAVKEY_ENFORCE))
		{
			if ((sp - phrase) < HPAVKEY_PHRASE_MIN)
			{
				error (0, ENOTSUP, "Less than %d characters on line %d", HPAVKEY_PHRASE_MIN, line);
				continue;
			}
			if ((sp - phrase) > HPAVKEY_PHRASE_MAX)
			{
				error (0, ENOTSUP, "More than %d characters on line %d", HPAVKEY_PHRASE_MAX, line);
				continue;
			}
		}
		if (class == HPAVKEY_DAK)
		{
			HPAVKeyDAK (digest, phrase);
			HPAVKeyOut (digest, HPAVKEY_DAK_LEN, phrase, flags);
			continue;
		}
		if (class == HPAVKEY_NMK)
		{
			HPAVKeyNMK (digest, phrase);
			HPAVKeyOut (digest, HPAVKEY_NMK_LEN, phrase, flags);
			continue;
		}
		if (class == HPAVKEY_NID)
		{
			HPAVKeyNMK (digest, phrase);
			HPAVKeyNID (digest, digest, level);
			HPAVKeyOut (digest, HPAVKEY_NID_LEN, phrase, flags);
			continue;
		}
		HPAVKeySHA (digest, phrase);
		HPAVKeyOut (digest, HPAVKEY_SHA_LEN, phrase, flags);
	}
	return;
}


/*====================================================================*
 *
 *   int main (int argc, const char * argv []);
 *
 *
 *--------------------------------------------------------------------*/

#define DEFAULT_LEVEL 0

int main (int argc, const char * argv [])

{
	static const char * optv [] =
	{
		"DeL:MNqv",
		"file [file] [...]",
		"HomePlug AV key generator",
		"D\tconvert password to Device Access Key",
		"e\tenforce HomePlug AV password rules",
		"M\tconvert password to Network Membership Key",
		"N\tconvert password to Network Identification Key",
		"L n\tSecurity Level is (n) [" LITERAL (DEFAULT_LEVEL) "]",
		"q\tquiet mode",
		"v\tverbose mode",
		(const char *) (0)
	};
	flag_t flags = (flag_t)(0);
	signed type = 0;
	signed level = DEFAULT_LEVEL;
	signed c;
	optind = 1;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch ((char) (c))
		{
		case 'D':
			type = HPAVKEY_DAK;
			break;
		case 'M':
			type = HPAVKEY_NMK;
			break;
		case 'N':
			type = HPAVKEY_NID;
			break;
		case 'L':
			level = (signed)(uintspec (optarg, 0, 1));
			break;
		case 'q':
			_setbits (flags, HPAVKEY_SILENCE);
			break;
		case 'v':
			_setbits (flags, HPAVKEY_VERBOSE);
			break;
		case 'e':
			_setbits (flags, HPAVKEY_ENFORCE);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if (!argc)
	{
		generate (type, level, flags);
	}
	while ((argc) && (* argv))
	{
		if (!freopen (* argv, "r", stdin))
		{
			error (0, errno, "%s", * argv);
		}
		else
		{
			generate (type, level, flags);
		}
		argv++;
		argc--;
	}
	return (0);
}

