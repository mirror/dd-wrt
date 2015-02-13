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
 *   mme.h -
 *
 *   message header definitions and function declarations;
 *
 *.  Qualcomm Atheros HomePlug AV Powerline Toolkit
 *:  Copyright (c) 2009-2013 by Qualcomm Atheros Inc. ALL RIGHTS RESERVED;
 *;  For demonstration and evaluation only; Not for production use.
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *      Matthieu Poullet <m.poullet@avm.de>
 *
 *--------------------------------------------------------------------*/

#ifndef MME_HEADER
#define MME_HEADER

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../ether/ether.h"
#include "../ether/channel.h"
#include "../mme/homeplug.h"
#include "../mme/qualcomm.h"

/*====================================================================*
 *   manage cross-platform structure packing;
 *--------------------------------------------------------------------*/

#ifndef __packed
#ifdef __GNUC__
#define __packed __attribute__ ((packed))
#else
#define __packed
#endif
#endif

/*====================================================================*
 *   Ethernet, HomePlug and Qualcomm Frame headers;
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed ethernet_hdr

{
	uint8_t ODA [ETHER_ADDR_LEN];
	uint8_t OSA [ETHER_ADDR_LEN];
	uint16_t MTYPE;
}

ethernet_hdr;
typedef struct __packed homeplug_hdr

{
	uint8_t MMV;
	uint16_t MMTYPE;
}

homeplug_hdr;
typedef struct __packed homeplug_fmi

{
	uint8_t MMV;
	uint16_t MMTYPE;
	uint8_t FMSN;
	uint8_t FMID;
}

homeplug_fmi;
typedef struct __packed qualcomm_hdr

{
	uint8_t MMV;
	uint16_t MMTYPE;
	uint8_t OUI [ETHER_ADDR_LEN >> 1];
}

qualcomm_hdr;
typedef struct __packed qualcomm_fmi

{
	uint8_t MMV;
	uint16_t MMTYPE;
	uint8_t FMSN;
	uint8_t FMID;
	uint8_t OUI [ETHER_ADDR_LEN >> 1];
}

qualcomm_fmi;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   Composite message formats;
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push, 1)
#endif

typedef struct __packed message

{
	struct ethernet_hdr ethernet;
	uint8_t content [ETHERMTU];
}

MESSAGE;
typedef struct __packed homeplug

{
	struct ethernet_hdr ethernet;
	struct homeplug_fmi homeplug;
	uint8_t content [ETHERMTU - sizeof (struct homeplug_fmi)];
}

HOMEPLUG;
typedef struct __packed qualcomm

{
	struct ethernet_hdr ethernet;
	struct qualcomm_fmi qualcomm;
	uint8_t content [ETHERMTU - sizeof (struct qualcomm_fmi)];
}

QUALCOMM;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   functions;
 *--------------------------------------------------------------------*/

void MMEPeek (void const * memory, size_t extent, FILE * fp);
void ARPCPeek (void const * memory, size_t extent, FILE * fp);
void ARPCWrite (signed fd, void const * memory, size_t extent);
void ARPCPrint (FILE * fp, void const * memory, size_t extent);

/*====================================================================*
 *   functions;
 *--------------------------------------------------------------------*/

char const * MMEName (uint16_t MMTYPE);
char const * MMEMode (uint16_t MMTYPE);
char const * MMECode (uint16_t MMTYPE, uint8_t MSTATUS);

/*====================================================================*
 *   header encode functions;
 *--------------------------------------------------------------------*/

signed EthernetHeader (void * memory, const uint8_t peer [], const uint8_t host [], uint16_t protocol);
signed HomePlugHeader (struct homeplug_hdr *, uint8_t MMV, uint16_t MMTYPE);
signed QualcommHeader (struct qualcomm_hdr *, uint8_t MMV, uint16_t MMTYPE);
signed HomePlugHeader1 (struct homeplug_fmi *, uint8_t MMV, uint16_t MMTYPE);
signed QualcommHeader1 (struct qualcomm_fmi *, uint8_t MMV, uint16_t MMTYPE);

/*====================================================================*
 *   header decode functions;
 *--------------------------------------------------------------------*/

signed UnwantedMessage (void const * memory, size_t extent, uint8_t MMV, uint16_t MMTYPE);
signed FirmwareMessage (void const * memory);

/*====================================================================*
 *   intermmediate level Ethernet send/receive functions;
 *--------------------------------------------------------------------*/

ssize_t sendmessage (struct channel const *, struct message *, ssize_t length);
ssize_t readmessage (struct channel const *, struct message *, uint8_t MMV, uint16_t MMTYPE);

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#endif

