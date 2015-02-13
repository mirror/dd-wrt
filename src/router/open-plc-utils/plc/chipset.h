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

#ifndef CHIPSET_HEADER
#define CHIPSET_HEADER

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <stdint.h>

/*====================================================================*
 *   constants;
 *--------------------------------------------------------------------*/

#define PLCHIPSET "PLCDEVICE"

/*====================================================================*
 *   chipset codes returned in VS_SW_VER MDEVICE_CLASS field;
 *--------------------------------------------------------------------*/

#define CHIPSET_UNKNOWN 0x00
#define CHIPSET_INT6000A1 0x01
#define CHIPSET_INT6300A0 0x02
#define CHIPSET_INT6400A0 0x03
#define CHIPSET_AR7400A0 0x04
#define CHIPSET_AR6405A0 0x05
#define CHIPSET_PANTHER_LYNX 0x06
#define CHIPSET_QCA7450A0 0x07
#define CHIPSET_QCA7451A0 0x08
#define CHIPSET_QCA7452A0 0x09
#define CHIPSET_QCA7420A0 0x20
#define CHIPSET_QCA6410A0 0x21
#define CHIPSET_QCA6411A0 0x21
#define CHIPSET_QCA7000A0 0x22
#define CHIPSET_QCA7000I 0x22
#define CHIPSET_QCA7005A0 0x22
#define CHIPSET_QCA7500A0 0x30

/*
 * the following definitions define older constants in terms of
 * newer constants to avoid compiler errors; there is nothing
 * magic about the A0/A1 suffixes;
 */

#define CHIPSET_INT6000 CHIPSET_INT6000A1
#define CHIPSET_INT6300 CHIPSET_INT6300A0
#define CHIPSET_INT6400 CHIPSET_INT6400A0
#define CHIPSET_AR7400  CHIPSET_AR7400A0
#define CHIPSET_INT6405 CHIPSET_AR6405A0
#define CHIPSET_AR6405  CHIPSET_AR6405A0
#define CHIPSET_QCA7450 CHIPSET_QCA7450A0
#define CHIPSET_QCA7451 CHIPSET_QCA7451A0
#define CHIPSET_QCA7452 CHIPSET_QCA7452A0
#define CHIPSET_QCA7420 CHIPSET_QCA7420A0
#define CHIPSET_QCA6410 CHIPSET_QCA6410A0
#define CHIPSET_QCA6411 CHIPSET_QCA6411A0
#define CHIPSET_QCA7000 CHIPSET_QCA7000A0
#define CHIPSET_QCA7005 CHIPSET_QCA7005A0
#define CHIPSET_QCA7500 CHIPSET_QCA7500A0

/*====================================================================*
 *   chipset functions;
 *--------------------------------------------------------------------*/

char const * chipsetname (uint8_t chipset);
void chipset (void const * memory);

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#endif



