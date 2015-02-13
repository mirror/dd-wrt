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
 *   signed pibfile1 (struct _file_ const * file);
 *
 *   pib.h
 *
 *   open a thunderbolt/lightning PIB file and validate it by
 *   checking file size, checksum and selected internal parameters;
 *   return a file descriptor on success; terminate the program on
 *   error;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef PIBFILE1_SOURCE
#define PIBFILE1_SOURCE

#include <unistd.h>
#include <errno.h>

#include "../tools/files.h"
#include "../tools/error.h"
#include "../pib/pib.h"

signed pibfile1 (struct _file_ const * file)

{
	struct simple_pib simple_pib;
	if (lseek (file->file, 0, SEEK_SET))
	{
		error (1, errno, FILE_CANTHOME, file->name);
	}
	if (read (file->file, &simple_pib, sizeof (simple_pib)) != sizeof (simple_pib))
	{
		error (1, errno, FILE_CANTREAD, file->name);
	}
	if (lseek (file->file, 0, SEEK_SET))
	{
		error (1, errno, FILE_CANTHOME, file->name);
	}
	if ((simple_pib.RESERVED1) || (simple_pib.RESERVED2))
	{
		error (1, errno, PIB_BADCONTENT, file->name);
	}
	if (fdchecksum32 (file->file, LE16TOH (simple_pib.PIBLENGTH), 0))
	{
		error (1, errno, PIB_BADCHECKSUM, file->name);
	}
	if (lseek (file->file, 0, SEEK_SET))
	{
		error (1, errno, FILE_CANTHOME, file->name);
	}
	return (0);
}


#endif

