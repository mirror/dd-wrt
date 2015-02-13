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
 *   char const * MMECode (uint16_t MMTYPE, uint8_t MSTATUS)
 *
 *   mme.h
 *
 *   return text for a given message type and status code; search is
 *   performed using two nested binary searches;
 *
D
 *   a given vendor specific status code may mean something different
 *   for each message type; this table maps message types and codes to
 *   strings; it is bulky but takes less space than many small tables;
 *   define MMEPASSFAIL as 0 to suppress the table without disturbing
 *   other code;
 *
 *   the bootloader and firmware share some message types but return
 *   different status codes for the same message type and error; two
 *   MMEs in particular, VS_WR_MEM and VS_ST_MAC are normally used
 *   only with the Bootloader and so we define SOFTLOADER as 1
 *   causing Bootloader/Softloader codes to replace the firmware
 *   codes;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef MMECODE_SOURCE
#define MMECODE_SOURCE

#include <stdio.h>
#include <stdint.h>

#include "../mme/mme.h"
#include "../tools/symbol.h"
#include "../tools/endian.h"

#ifndef MMEPASSFAIL

static struct mme_code

{
	uint16_t type;
	uint8_t code;
	char const * text;
}

mme_codes [] =

{
	{
		0x8005,
		0x01,
		"Read Stopped"
	},
	{
		0x8005,
		0x03,
		"Stopped with Valid Read"
	},
	{
		0x8005,
		0x05,
		"Reached End"
	},
	{
		0x8005,
		0x07,
		"Stopped, Valid Read, Reached End - All Done! Go Away!"
	},
	{
		0x8005,
		0x80,
		"Invalid Index"
	},
	{
		0x8005,
		0x81,
		"Invalid Word Count"
	},
	{
		0x8005,
		0x82,
		"No Stop Before Read"
	},
	{
		0x8005,
		0x83,
		"Invalid MME"
	},

#ifdef SOFTLOADER

	{
		0xA005,
		0x14,
		"Bad Checksum"
	},
	{
		0xA005,
		0x1C,
		"Bad Length"
	},
	{
		0xA005,
		0x38,
		"Bad Address"
	},
	{
		0xA005,
		0x3C,
		"Bad Data Alignment"
	},

#else

	{
		0xA005,
		0x10,
		"Bad Address"
	},
	{
		0xA005,
		0x14,
		"Bad Length"
	},

#endif

	{
		0xA009,
		0x10,
		"Bad Offset"
	},
	{
		0xA009,
		0x14,
		"Bad Length"
	},
	{
		0xA00D,
		0x10,
		"Bad Module ID"
	},

#ifdef SOFTLOADER

	{
		0xA00D,
		0x14,
		"Bad Image Checksum"
	},
	{
		0xA00D,
		0x1C,
		"Bad Image Length"
	},
	{
		0xA00D,
		0x38,
		"Bad Image Load Address"
	},
	{
		0xA00D,
		0x3C,
		"Bad Data Alignment"
	},
	{
		0xA00D,
		0x40,
		"Bad Start Address"
	},

#else

	{
		0xA00D,
		0x38,
		"Bad Command"
	},
	{
		0xA00D,
		0x40,
		"Failed to Lock NVM"
	},

#endif

	{
		0xA011,
		0x10,
		"No Flash Memory"
	},
	{
		0xA01D,
		0x01,
		"Device Failed to Reset"
	},
	{
		0xA01D,
		0x02,
		"Device is Busy"
	},
	{
		0xA021,
		0x10,
		"Bad Module"
	},
	{
		0xA021,
		0x12,
		"Bad Length"
	},
	{
		0xA021,
		0x14,
		"Bad Checksum"
	},
	{
		0xA021,
		0x20,
		"Bad Offset"
	},
	{
		0xA021,
		0x40,
		"Operation Blocked"
	},
	{
		0xA021,
		0x50,
		"Fail to lock NVM"
	},
	{
		0xA025,
		0x10,
		"Bad Module"
	},
	{
		0xA025,
		0x12,
		"Bad Length"
	},
	{
		0xA025,
		0x14,
		"Bad Checksum"
	},
	{
		0xA025,
		0x20,
		"Unexpected Offset"
	},
	{
		0xA025,
		0x50,
		"Fail to lock NVM"
	},
	{
		0xA025,
		0x58,
		"DAK Mismatch"
	},
	{
		0xA029,
		0x10,
		"Bad Module"
	},
	{
		0xA029,
		0x14,
		"No Flash Memory"
	},
	{
		0xA029,
		0x18,
		"Not enough NVRAM"
	},
	{
		0xA029,
		0x1C,
		"Bad Header Checksum"
	},
	{
		0xA029,
		0x20,
		"Bad Image Checksum"
	},
	{
		0xA029,
		0x24,
		"Bad PIB"
	},
	{
		0xA029,
		0x28,
		"Softloader Too Large"
	},
	{
		0xA029,
		0x2C,
		"Firmware Too Large"
	},
	{
		0xA029,
		0x42,
		"Firmware without PIB"
	},
	{
		0xA029,
		0x44,
		"Bad PIB Checksum"
	},
	{
		0xA029,
		0x46,
		"DAK Not Zero"
	},
	{
		0xA029,
		0x48,
		"MAC Mismatch"
	},
	{
		0xA029,
		0x50,
		"MFG HFID Mismatch"
	},
	{
		0xA029,
		0x52,
		"Bad Bind Factory Defaults"
	},
	{
		0xA029,
		0x54,
		"Bad Bind Template PIB"
	},
	{
		0xA029,
		0x56,
		"Bad Bind Working PIB"
	},
	{
		0xA029,
		0x58,
		"Error Computing PIB Checksum"
	},
	{
		0xA029,
		0x5A,
		"Bad Bind Scratch PIB"
	},
	{
		0xA029,
		0x5C,
		"No Firmware Version"
	},
	{
		0xA029,
		0x5E,
		"Version Mismatch"
	},
	{
		0xA029,
		0x60,
		"Bad 01PIB Checksum"
	},
	{
		0xA029,
		0x62,
		"Bad 02PIB Checksum"
	},
	{
		0xA029,
		0x64,
		"Bad WPIB Checksum"
	},
	{
		0xA029,
		0x66,
		"Illegal Firmware Revision"
	},
	{
		0xA031,
		0x01,
		"Bad Control"
	},
	{
		0xA031,
		0x02,
		"Bad Direction"
	},
	{
		0xA031,
		0x10,
		"Bad Link ID"
	},
	{
		0xA031,
		0x20,
		"Bad MAC Address"
	},
	{
		0xA035,
		0x10,
		"Bad Control"
	},
	{
		0xA049,
		0x20,
		"Invalid Duration"
	},
	{
		0xA049,
		0x12,
		"Invalid Length"
	},
	{
		0xA049,
		0x24,
		"Already Set"
	},
	{
		0xA051,
		0x10,
		"Bad PEKS"
	},
	{
		0xA051,
		0x11,
		"Bad PIB"
	},
	{
		0xA051,
		0x12,
		"Bad PEKS Encrypted Payload"
	},
	{
		0xA051,
		0x13,
		"Remote Failure"
	},
	{
		0xA051,
		0x14,
		"Bad Remote Response"
	},
	{
		0xA051,
		0x15,
		"Remote Decryption Failed"
	},
	{
		0xA059,
		0x10,
		"No Flash Memory"
	},
	{
		0xA05D,
		0x30,
		"Bad Checksum"
	},
	{
		0xA05D,
		0x34,
		"BIST Failed"
	},
	{
		0xA065,
		0x10,
		"No Flash Memory"
	},
	{
		0xA069,
		0x01,
		"Failed"
	},
	{
		0xA069,
		0x02,
		"Not Supported"
	},
	{
		0xA06D,
		0x01,
		"Bad Control"
	},
	{
		0xA06D,
		0x02,
		"Success Write"
	},
	{
		0xA06D,
		0x03,
		"Bad Write Parameter"
	},
	{
		0xA071,
		0x01,
		"Bad MAC Address"
	},
	{
		0xA071,
		0x02,
		"Bad TMSLOT"
	},
	{
		0xA081,
		0x10,
		"Bad Module ID"
	},
	{
		0xA081,
		0x14,
		"No Flash Memory"
	},
	{
		0xA081,
		0x18,
		"Not Enough Flash Memory"
	},
	{
		0xA081,
		0x1C,
		"Bad Image Header Checksum"
	},
	{
		0xA081,
		0x20,
		"Bad Image Checksum"
	},
	{
		0xA081,
		0x24,
		"Invalid PIB"
	},
	{
		0xA081,
		0x28,
		"Softloader Too Large"
	},
	{
		0xA081,
		0x2C,
		"Firmware File Too Large"
	},
	{
		0xA081,
		0x42,
		"Firmware without PIB"
	},
	{
		0xA081,
		0x44,
		"Bad PIB Checksum"
	},
	{
		0xA081,
		0x46,
		"DAK Not Zero"
	},
	{
		0xA081,
		0x48,
		"DAC Mismatch"
	},
	{
		0xA081,
		0x50,
		"MFG HFID Mismatch"
	},
	{
		0xA081,
		0x52,
		"Bad Bind Factory Defaults"
	},
	{
		0xA081,
		0x54,
		"Bad Bind Template PIB"
	},
	{
		0xA081,
		0x56,
		"Bad Bind Working PIB"
	},
	{
		0xA081,
		0x58,
		"DAK Mismatch"
	},
	{
		0xA081,
		0x5A,
		"Failed to Lock NVM"
	},
	{
		0xA089,
		0x01,
		"Bad Control"
	},
	{
		0xA089,
		0x02,
		"Bad Volatility"
	},
	{
		0xA089,
		0x03,
		"Bad Classifier"
	},
	{
		0xA089,
		0x04,
		"Classifier table full"
	},
	{
		0xA089,
		0x05,
		"Classifier exists with different action"
	},
	{
		0xA089,
		0x06,
		"Classifier not found"
	},
	{
		0xA08D,
		0x10,
		"Refused"
	},
	{
		0xA08D,
		0x11,
		"Refused"
	},
	{
		0xA08F,
		0x10,
		"Error"
	},
	{
		0xA091,
		0x01,
		"Unknown Peer Address"
	},
	{
		0xA091,
		0x02,
		"Unknown Slot Number"
	},
	{
		0xA099,
		0x01,
		"Destination Address Misaligned"
	},
	{
		0xA099,
		0x02,
		"Entry Point Misaligned"
	},
	{
		0xA099,
		0x04,
		"Entry Point Out-of-Bounds"
	},
	{
		0xA099,
		0x08,
		"Bad Checksum"
	},
	{
		0xA099,
		0x10,
		"Out of Memory"
	},
	{
		0xA099,
		0x20,
		"Offset or Length Mismatch"
	},
	{
		0xA099,
		0x40,
		"Memory not Operational"
	},
	{
		0xA099,
		0x80,
		"Bad Length"
	},
	{
		0xA0B1,
		0x10,
		"Invalid Number of Module Operations"
	},
	{
		0xA0B1,
		0x11,
		"Invalid Module Operation ID"
	},
	{
		0xA0B1,
		0x12,
		"Invalid Session ID"
	},
	{
		0xA0B1,
		0x13,
		"Invalid Num Module Operation Specific Data"
	},
	{
		0xA0B1,
		0x14,
		"Module ID not found"
	},
	{
		0xA0B1,
		0x15,
		"Invalid Module Length"
	},
	{
		0xA0B1,
		0x16,
		"Invalid Module Index"
	},
	{
		0xA0B1,
		0x17,
		"Invalid Data Length"
	},
	{
		0xA0B1,
		0x18,
		"Unexpected Offset"
	},
	{
		0xA0B1,
		0x19,
		"Invalid Commit Action Code"
	},
	{
		0xA0B1,
		0x1A,
		"Operation Block by previous Commit"
	},
	{
		0xA0B1,
		0x1B,
		"Duplicate ModuleID/SubID"
	},
	{
		0xA0B1,
		0x1C,
		"Invalid ModuleID/SubID"
	},
	{
		0xA0B1,
		0x20,
		"No Flash Memory"
	},
	{
		0xA0B1,
		0x21,
		"Not Enough Flash Memory"
	},
	{
		0xA0B1,
		0x22,
		"Unsupported Flash memory Type"
	},
	{
		0xA0B1,
		0x23,
		"Can't Lock Flash Memory"
	},
	{
		0xA0B1,
		0x24,
		"Can't Write Flash Memory"
	},
	{
		0xA0B1,
		0x30,
		"Bad Module Checksum"
	},
	{
		0xA0B1,
		0x31,
		"Individual Module Error"
	},
	{
		0xA0B1,
		0x32,
		"Module not available in NVM or Memory"
	},
	{
		0xA0B1,
		0x40,
		"Invalid Header Checksum"
	},
	{
		0xA0B1,
		0x41,
		"Invalid Firmware Image Checksum"
	},
	{
		0xA0B1,
		0x42,
		"Softloader Image is Too Large"
	},
	{
		0xA0B1,
		0x43,
		"Firmware Image is Too Large"
	},
	{
		0xA0B1,
		0x44,
		"Invalid PIB Checksum"
	},
	{
		0xA0B1,
		0x45,
		"No Firmware Version"
	},
	{
		0xA0B1,
		0x46,
		"FW Commit but no PIB"
	},
	{
		0xA0B1,
		0x47,
		"Major Version Mismatch"
	},
	{
		0xA0B1,
		0x48,
		"Minor Version Mismatch"
	},
	{
		0xA0B1,
		0x50,
		"Invalid PIB"
	},
	{
		0xA0B1,
		0x51,
		"DAK Not Zero"
	},
	{
		0xA0B1,
		0x52,
		"MAC Mismatch"
	},
	{
		0xA0B1,
		0x53,
		"DAK Mismatch"
	},
	{
		0xA0B1,
		0x54,
		"Manufacturer HFID Mismatch"
	},
	{
		0xA0B1,
		0x55,
		"Bad Bind to Factory Default PIB"
	},
	{
		0xA0B1,
		0x56,
		"Bad Bind to Template PIB"
	},
	{
		0xA0B1,
		0x57,
		"Bad Bind to Working PIB"
	},
	{
		0xA0B1,
		0x58,
		"Bad Bind Scratch PIB"
	},
	{
		0xA0B1,
		0x59,
		"Error Generating Checksum Scratch PIB"
	},
	{
		0xA0B1,
		0x5A,
		"Checksum Error O1 PIB"
	},
	{
		0xA0B1,
		0x5B,
		"Checksum Error O2 PIB"
	},
	{
		0xA0B1,
		0x5C,
		"Checksum Error Working PIB"
	},
	{
		0xA0B1,
		0x61,
		"Unexpected Module Operation"
	},
	{
		0xA0B1,
		0x62,
		"Not Enough Resources"
	},
	{
		0xA0B1,
		0x63,
		"Received Module Data Out of Order"
	},
	{
		0xA0B1,
		0x64,
		"No PIB Version"
	},
	{
		0xA0B1,
		0x70,
		"Module Length Mismatch with Module Info"
	},
	{
		0xA0B1,
		0x71,
		"No NVM Softloader Present in Flash Memory"
	},
	{
		0xA0C1,
		0x03,
		"Invalid Classifier"
	},
	{
		0xA0C1,
		0x04,
		"Already too many Classifiers"
	},
	{
		0xA0C1,
		0x05,
		"Classifier exists with different Classification Action"
	},
	{
		0xA0C1,
		0x07,
		"Link Already in Progress"
	},
	{
		0xA0C1,
		0x08,
		"Invalid Configuration"
	},
	{
		0xA0C1,
		0x10,
		"Invalid CSPEC Version"
	},
	{
		0xA0C1,
		0x11,
		"Invalid CSPEC"
	},
	{
		0xA0C1,
		0x20,
		"Out of Local Resources"
	},
	{
		0xA0C1,
		0x30,
		"Invalid Peer"
	},
	{
		0xA0C1,
		0x31,
		"Peer Confirm Timed out"
	},
	{
		0xA0C1,
		0x40,
		"Peer rejection"
	},
	{
		0xA0C5,
		0x01,
		"Invalid Modification Control"
	},
	{
		0xA0C5,
		0x06,
		"CID Not Found"
	},
	{
		0xA0C5,
		0x07,
		"Link Update in Progress, try again later"
	},
	{
		0xA0C9,
		0x06,
		"CID Not Found"
	},
	{
		0xA0C9,
		0x07,
		"Link Update in Progress. Try again later"
	},
	{
		0xA0C9,
		0x31,
		"Peer Confirm Timed out"
	},
	{
		0xA0CD,
		0x01,
		"Invalid Request Type"
	},
	{
		0xA0CD,
		0x04,
		"Too Many Requests",
	},
	{
		0xA0CD,
		0x06,
		"CID Not Found"
	},
	{
		0xA0CD,
		0x10,
		"Invalid CSPEC Version"
	},
	{
		0xA0CD,
		0x31,
		"Peer Confirm Timed out"
	},
	{
		0xA0D1,
		0x01,
		"Unsupported"
	},
	{
		0xA0D1,
		0x02,
		"Process Failed"
	},
	{
		0xA0D1,
		0x02,
		"Process Failed"
	},
	{
		0xA0D9,
		0x01,
		"Bad VersionID or InfoID"
	},
	{
		0xA0DD,
		0x01,
		"No Enumeration Table"
	},
	{
		0xA0F5,
		0x01,
		"Bad Request Type"
	},
	{
		0xA0F5,
		0x02,
		"FMI Not Zero"
	},
	{
		0xA0F5,
		0x03,
		"Invalid Command"
	},
	{
		0xA0F5,
		0x04,
		"Bad Index"
	},
	{
		0xA0F5,
		0x05,
		"Bad Value Count"
	},
	{
		0xA0F5,
		0x06,
		"Capture Already Started"
	},
	{
		0xA0F5,
		0x07,
		"Data Invalid"
	},
	{
		0xA0F5,
		0x08,
		"Bad Gain Value"
	},
	{
		0xA0F5,
		0x10,
		"Bad MAC Address"
	},
	{
		0xA0F5,
		0x11,
		"Bad Tone Map Index"
	},
	{
		0xA0F5,
		0x12,
		"Capture Failed"
	},
	{
		0xA0F5,
		0x13,
		"No Data Available"
	},
	{
		0xA0F9,
		0x01,
		"Get Property Failure"
	},
	{
		0xA0F9,
		0x02,
		"Unsupported Property"
	},
	{
		0xA0F9,
		0x03,
		"Unsupported Property Version"
	},
	{
		0xA101,
		0x01,
		"Set Property Failure"
	},
	{
		0xA101,
		0x02,
		"Unsupported Property"
	},
	{
		0xA101,
		0x03,
		"Unsupported Property Version"
	},
	{
		0xA101,
		0x04,
		"Property Not Persistent"
	},
	{
		0xA101,
		0x05,
		"Flash Memory Failure"
	},
	{
		0xA101,
		0x06,
		"Bad Option for Supported Property"
	},
	{
		0xA101,
		0x07,
		"Invalid Property Data"
	},
	{
		0xA101,
		0x08,
		"Bad Property Data Length"
	},
	{
		0xA105,
		0x01,
		"Buffer Not Available"
	},
	{
		0xA109,
		0x01,
		"No Flash Memory"
	},
	{
		0xA109,
		0x02,
		"Invalid Flash Sector"
	},
	{
		0xA109,
		0x03,
		"Current DAK Mismatch"
	},
	{
		0xA109,
		0x04,
		"Flash is Busy"
	},
	{
		0xA109,
		0x05,
		"No Access to Flash"
	},
	{
		0xA109,
		0x06,
		"Unsupported Chip"
	},
	{
		0xA109,
		0x07,
		"Unspecified Error"
	},
	{
		0xA10D,
		0x01,
		"Option Not Supported"
	},
	{
		0xA10D,
		0x02,
		"No Flash Memory"
	},
	{
		0xA10D,
		0x03,
		"Pending Self Test"
	},
	{
		0xA10D,
		0x04,
		"Flash Activity Conflict"
	},
	{
		0xA10D,
		0x05,
		"Bad Parameter"
	},
	{
		0xA111,
		0x01,
		"Option Not Supported"
	},
	{
		0xA111,
		0x02,
		"Self Test Finished"
	},
	{
		0xA111,
		0x03,
		"Self Test Pending"
	},
	{
		0xA111,
		0x04,
		"Self Test In Progress"
	},
	{
		0xA111,
		0x05,
		"Self Test No Result"
	},
	{
		0xA111,
		0x06,
		"Self Test Aborted"
	},
	{
		0xA111,
		0x07,
		"No Flash Memory"
	},
	{
		0xA115,
		0x01,
		"Interface Not Supported"
	},
	{
		0xA115,
		0x02,
		"Slave Not Found"
	},
	{
		0xA115,
		0x03,
		"Wrong Parameter"
	},
	{
		0xA119,
		0x01,
		"Bad Version"
	},
	{
		0xA119,
		0x02,
		"Bad Argument"
	},
	{
		0xA119,
		0x03,
		"Too Many Slaves"
	},
	{
		0xA119,
		0x04,
		"Too Many VLAN IDs"
	},
	{
		0xA119,
		0x05,
		"No Such Slave"
	},
	{
		0xA119,
		0x06,
		"Flash Operation Blocked"
	},
	{
		0xA119,
		0x07,
		"Unsupported Operation"
	},
	{
		0xA119,
		0x08,
		"Flash Activity Conflict"
	},
	{
		0xA119,
		0x09,
		"Table Busy"
	},
	{
		0xA119,
		0x0A,
		"One Slave Has Duplicate VLAN ID"
	},
	{
		0xA119,
		0x0B,
		"Multiple Slaves Have Same VLAN ID"
	},
	{
		0xA119,
		0x0C,
		"Multiple Configurations for One Slave"
	},
	{
		0xA119,
		0x0D,
		"Can't set VLAN ID for Local Station"
	},
	{
		0xA119,
		0x0E,
		"Bad Data Offset"
	},
	{
		0xA119,
		0x0F,
		"Bad Data Length"
	},
	{
		0xA119,
		0x10,
		"VLAN ID Has Been Configured"
	},
	{
		0xA119,
		0x11,
		"Bad MME Source"
	},
	{
		0xA119,
		0x12,
		"VLAN ID out of range"
	},
	{
		0xA119,
		0x13,
		"Cannot remove nonexistent VLAN ID"
	},
	{
		0xA149,
		0x1,
		"Invalid Key Length"
	},
	{
		0xA149,
		0x2,
		"Invalid Key"
	},
	{
		0xA149,
		0x3,
		"Invalid Access Type"
	},
	{
		0xA149,
		0x4,
		"Invalid Level Control"
	}
};


#endif

char const * MMECode (uint16_t MMTYPE, uint8_t MSTATUS)

{

#ifndef MMEPASSFAIL

	size_t lower = 0;
	size_t upper = SIZEOF (mme_codes);
	MMTYPE = LE16TOH (MMTYPE);
	while (lower < upper)
	{
		size_t index = (lower + upper) >> 1;
		signed order = MMTYPE - mme_codes [index].type;
		if (order < 0)
		{
			upper = index - 0;
			continue;
		}
		if (order > 0)
		{
			lower = index + 1;
			continue;
		}
		for (lower = index; lower > 0; lower--)
		{
			if (mme_codes [lower - 1].type != mme_codes [index].type)
			{
				break;
			}
		}
		for (upper = index; upper < SIZEOF (mme_codes); upper++)
		{
			if (mme_codes [upper + 0].type != mme_codes [index].type)
			{
				break;
			}
		}
		while (lower < upper)
		{
			index = (lower + upper) >> 1;
			order = MSTATUS - mme_codes [index].code;
			if (order < 0)
			{
				upper = index - 0;
				continue;
			}
			if (order > 0)
			{
				lower = index + 1;
				continue;
			}
			return (mme_codes [index].text);
		}
	}

#endif

	return ((MSTATUS)? ("Failure"):("Success"));
}


#endif

