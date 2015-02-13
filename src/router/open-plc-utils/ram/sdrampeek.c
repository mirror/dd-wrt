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
 *   void sdrampeek(const struct config_ram * config_ram);
 *
 *   sdram.h
 *
 *   print SDRAM configuration on stdout in human-readable format;
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef SDRAMPEEK_SOURCE
#define SDRAMPEEK_SOURCE

#include <stdio.h>

#include "../ram/sdram.h"
#include "../tools/memory.h"

void sdrampeek (struct config_ram * config_ram)

{
	printf ("\tSIZE=0x%08X (%dmb)\n", LE32TOH (config_ram->SDRAMSIZE), LE32TOH (config_ram->SDRAMSIZE)>>20);
	printf ("\tCONF=0x%08X\n", LE32TOH (config_ram->SDRAMCONF));
	printf ("\tTIM0=0x%08X\n", LE32TOH (config_ram->SDRAMTIM0));
	printf ("\tTIM1=0x%08X\n", LE32TOH (config_ram->SDRAMTIM1));
	printf ("\tCNTRL=0x%08X\n", LE32TOH (config_ram->SDRAMCNTRL));
	printf ("\tREF=0x%08X\n", LE32TOH (config_ram->SDRAMREF));
	printf ("\tCLOCK=0x%08X\n", LE32TOH (config_ram->MACCLOCK));
	return;
}


#endif

