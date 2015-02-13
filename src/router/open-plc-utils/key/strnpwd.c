/*====================================================================*
 *
 *   Copyright (c) 2015 Qualcomm Atheros, Inc.
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
 *   char * strnpwd (char buffer [], unsigned length, unsigned count, unsigned group, char space);
 *
 *   keys.h
 *
 *   encode a buffer with a password containing the specified number
 *   of random letters and digits; optionally, group the letters and
 *   digits and separate groups with a space character; return the
 *   address of the byte following the encoded password;
 *
 *   alphabet is an array of 32 printable password characters; 
 *   count is the number of characters to be selected from alphabet; 
 *   group is the grouping factor; 
 *   space is the group separator;
 *
 *   grouping is suppressed when group is zero or greater than or
 *   equal to count;
 *
 *   Contributors:
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef STRNPWD_SOURCE
#define STRNPWD_SOURCE

#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "../tools/types.h"
#include "../tools/error.h"
#include "../key/keys.h"

char * strnpwd (char buffer [], unsigned length, unsigned count, unsigned group, char space)

{
	signed fd;
	if ((fd = open ("/dev/urandom", O_RDONLY)) == -1)
	{
		error (1, errno, "can't open /dev/urandom");
	}
	while (count--)
	{
		static const char alphabet [] =
		{
			'2',
			'3',
			'4',
			'5',
			'6',
			'7',
			'8',
			'9',
			'A',
			'B',
			'C',
			'D',
			'E',
			'F',
			'G',
			'H',
			'J',
			'K',
			'L',
			'M',
			'N',
			'P',
			'Q',
			'R',
			'S',
			'T',
			'U',
			'V',
			'W',
			'X',
			'Y',
			'Z'
		};
		unsigned member;
		if (read (fd, & member, sizeof (member)) != sizeof (member))
		{
			error (1, errno, "can't read /dev/urandom");
		}
		member &= 0x1F;
		if (length)
		{
			*buffer = alphabet [member % sizeof (alphabet)];
			buffer++;
			length--;
		}
		if ((count) && (group) && ! (count % group))
		{
			if (length)
			{
				*buffer = space;
				buffer++;
				length--;
			}
		}
	}
	close (fd);
	return (buffer);
}

#endif



