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
 *   int sdramfileA (int fd, char const * filename, flag_t flags);
 *
 *   sdram.h
 *
 *   open an SDRAM configuration file and validate it by checking the
 *   file length; unfortunately there are not many things to check;
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef SDRAMFILEA_SOURCE
#define SDRAMFILEA_SOURCE

#include <unistd.h>
#include <memory.h>
#include <fcntl.h>
#include <errno.h>

#include "../ram/sdram.h"
#include "../tools/memory.h"
#include "../tools/error.h"
#include "../tools/flags.h"

int sdramfileA (int fd, char const * filename, flag_t flags)

{
	uint32_t checksum;
	struct config_ram config_ram;
	memset (&config_ram, 0, sizeof (config_ram));
	if (lseek (fd, 0, SEEK_SET) == -1)
	{
		if (_allclr (flags, SDRAM_SILENCE))
		{
			error (0, errno, "Can't rewind file: %s", filename);
		}
		return (-1);
	}
	if (read (fd, &config_ram, sizeof (config_ram)) != sizeof (config_ram))
	{
		if (_allclr (flags, SDRAM_SILENCE))
		{
			error (0, errno, "Wrong file size: %s", filename);
		}
		return (-1);
	}
	if (read (fd, &checksum, sizeof (checksum)) != sizeof (checksum))
	{
		if (_allclr (flags, SDRAM_SILENCE))
		{
			error (0, errno, "Can't read checksum: %s", filename);
		}
		return (-1);
	}
	if (checksum32 ((uint32_t *)(&config_ram), sizeof (config_ram) >> 2, checksum))
	{
		if (_allclr (flags, SDRAM_SILENCE))
		{
			error (0, 0, "Bad checksum: %s", filename);
		}
		return (-1);
	}
	if (_anyset (flags, SDRAM_VERBOSE))
	{
		if ((filename) && (*filename))
		{
			printf ("------- %s -------\n", filename);
		}
		sdrampeek (&config_ram);
	}
	if (lseek (fd, 0, SEEK_SET) == -1)
	{
		if (_allclr (flags, SDRAM_SILENCE))
		{
			error (0, errno, "Can't rewind file: %s", filename);
		}
		return (-1);
	}
	return (0);
}


#endif

