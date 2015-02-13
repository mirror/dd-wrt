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
 *   sdram.c - create INT6300 sdram configuration files;
 *
 *   sdram.h
 *
 *   this program writes two sdram configuration files in the current
 *   working directory; both files are for Linux toolkit programs and
 *   not the Windows Device Manager;
 *
 *   one of these sdram configuration blocks must be sent to the
 *   INT6300 bootloader with VS_SET_SDRAM before downloading NVM
 *   and PIB files;
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>

#include "../tools/memory.h"
#include "../ram/sdram.h"

#ifndef MAKEFILE
#include "../tools/checksum32.c"
#endif

#define FILE1 "sdram16mb.cfg"
#define FILE2 "sdram64mb.cfg"

int main (int argc, char const * argv [])

{
	const uint8_t sdram16mb [32] =
	{
		0x00,
		0x00,
		0x00,
		0x01,
		0x68,
		0x2f,
		0x14,
		0x00,
		0x92,
		0xd4,
		0xe1,
		0x01,
		0xd6,
		0x83,
		0x08,
		0x00,
		0x88,
		0x32,
		0x00,
		0x00,
		0xdb,
		0x06,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00
	};
	const uint8_t sdram64mb [32] =
	{
		0x00,
		0x00,
		0x00,
		0x04,
		0x88,
		0x31,
		0x14,
		0x00,
		0x91,
		0xd4,
		0xe1,
		0x01,
		0xe3,
		0x2b,
		0x01,
		0x00,
		0x89,
		0x30,
		0x00,
		0x00,
		0x66,
		0x03,
		0x00,
		0x00,
		0x00,
		0x01,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00,
		0x00
	};
	int fd;
	uint32_t checksum;
	if ((fd = open (FILE1, O_CREAT|O_WRONLY|O_TRUNC, 0444)) != -1)
	{
		printf ("writing %s\n", FILE1);
		checksum = checksum32 (sdram16mb, sizeof (sdram16mb), 0);
		write (fd, sdram16mb, sizeof (sdram16mb));
		write (fd, &checksum, sizeof (checksum));
		close (fd);
	}
	if ((fd = open (FILE2, O_CREAT|O_WRONLY|O_TRUNC, 0444)) != -1)
	{
		printf ("writing %s\n", FILE2);
		checksum = checksum32 (sdram64mb, sizeof (sdram64mb), 0);
		write (fd, sdram64mb, sizeof (sdram64mb));
		write (fd, &checksum, sizeof (checksum));
		close (fd);
	}
	return (0);
}

