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
 *   nvm.h - nvm file format definitions and declarations;
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef NVM_HEADER
#define NVM_HEADER

/*====================================================================*
 *   system header files;
 *--------------------------------------------------------------------*/

#include <unistd.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/types.h"

/*====================================================================*
 *   nvm program constants;
 *--------------------------------------------------------------------*/

#define NVM_IMAGETYPES 18
#define NVM_PLATFORMS 5

/*====================================================================*
 *   nvm program flags;
 *--------------------------------------------------------------------*/

#define NVM_SILENCE     (1 << 0)
#define NVM_VERBOSE     (1 << 1)
#define NVM_MANIFEST    (1 << 2)
#define NVM_FIRMWARE    (1 << 3)
#define NVM_IDENTITY    (1 << 4)
#define NVM_SDRAM       (1 << 5)

/*====================================================================*
 *   nvm header execute masks;
 *--------------------------------------------------------------------*/

#define NVM_IGNORE_MASK_INT6000         (1 << 0)
#define NVM_IGNORE_MASK_INT6300         (1 << 1)
#define NVM_IGNORE_MASK_INT6400         (1 << 2)
#define NVM_IGNORE_MASK_AR7400          (1 << 4)
#define NVM_IGNORE_MASK_QCA7420         (1 << 8)

/*====================================================================*
 *   image chain types;
 *--------------------------------------------------------------------*/

#define NVM_CHAIN_SOFTLOADER            0
#define NVM_CHAIN_FIRMWARE              1
#define NVM_CHAIN_PIB                   2
#define NVM_CHAIN_CUSTOM_MODULE         3
#define NVM_CHAIN_MODULE_OP_SESSION     4

/*====================================================================*
 *   nvm build types;
 *--------------------------------------------------------------------*/

#define NVM_BUILD_NIGHTLY               0
#define NVM_BUILD_RELEASE               1
#define NVM_BUILD_CUSTOM                2

/*====================================================================*
 *   nvm image types;
 *--------------------------------------------------------------------*/

#define NVM_IMAGE_GENERIC               0x0000
#define NVM_IMAGE_CONFIG_SYNOPSIS       0x0001
#define NVM_IMAGE_CONFIG_DENALI         0x0002
#define NVM_IMAGE_APPLET_DENALI         0x0003
#define NVM_IMAGE_FIRMWARE              0x0004
#define NVM_IMAGE_OASCLIENT             0x0005
#define NVM_IMAGE_CUSTOM                0x0006
#define NVM_IMAGE_MEMCTL                0x0007
#define NVM_IMAGE_ADVPWRMGMT            0x0008
#define NVM_IMAGE_OAS_CLIENT_IP_STK     0x0009
#define NVM_IMAGE_OAS_CLIENT_TR069      0x000A
#define NVM_IMAGE_NVM_SOFTLOADER        0x000B
#define NVM_IMAGE_FLASH_LAYOUT          0x000C
#define NVM_IMAGE_MANIFEST              0x000E
#define NVM_IMAGE_PIB                   0x000F

/*====================================================================*
 *   manifest field types;
 *--------------------------------------------------------------------*/

#define NVM_FIELD_SIGNATURE             0
#define NVM_FIELD_HARDWARE_COMPAT       1
#define NVM_FIELD_CHAIN_MAJOR_VERSION   2
#define NVM_FIELD_CHAIN_MINOR_VERSION   3
#define NVM_FIELD_CHAIN_TYPE            4
#define NVM_FIELD_BUILD_MAJOR_VERSION   5
#define NVM_FIELD_BUILD_MINOR_VERSION   6
#define NVM_FIELD_BUILD_TYPE            7
#define NVM_FIELD_MANIFEST_VERSION      8
#define NVM_FIELD_BUILD_NUMBER          9
#define NVM_FIELD_BUILD_DATE            10
#define NVM_FIELD_BUILD_TIME            11
#define NVM_FIELD_DEVICE_TYPE           12
#define NVM_FIELD_BUILD_HOSTNAME        13
#define NVM_FIELD_BUILD_USERNAME        14
#define NVM_FIELD_BUILD_DESCRIPTION     15
#define NVM_FIELD_BUILD_VERSION_STRING  16
#define NVM_FIELD_BUILD_SUSTAINING_RELEASE 17
#define NVM_FIELD_BUILD_MAJOR_SUBVERSION 18
#define NVM_FIELD_GENERIC_ID0           19
#define NVM_FIELD_GENERIC_ID1           20
#define NVM_FIELD_SL_MAJOR_VERSION      21
#define NVM_FIELD_SL_MINOR_VERSION      22
#define NVM_FIELD_FREE_SPACE            23

/*====================================================================*
 *   toolkit nvm error messages;
 *--------------------------------------------------------------------*/

#define NVM_HDR_VERSION "%s found bad NVM header version in %s module %d", __func__
#define NVM_HDR_CHECKSUM "%s found bad NVM header checksum in %s module %d", __func__
#define NVM_HDR_LINK "%s found bad NVM header link in %s module %d", __func__
#define NVM_IMG_CHECKSUM "%s found bad image checksum in %s module %d", __func__
#define NVM_HDR_CANTREAD "%s can't read image header in %s module %d", __func__
#define NVM_HDR_CANTSAVE "%s can't save image header in %s module %d", __func__
#define NVM_IMG_CANTREAD "%s can't read image in %s module %d", __func__
#define NVM_IMG_CANTSAVE "%s can't save image in %s module %d", __func__
#define NVM_IMG_RUN "%s applet %d is still running"

/*====================================================================*
 *   old nvm image header (44 bytes);
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push,1)
#endif

typedef struct __packed nvm_header1

{
	uint32_t HEADERVERSION;
	uint32_t IMAGEROMADDR;
	uint32_t IMAGEADDRESS;
	uint32_t IMAGELENGTH;
	uint32_t IMAGECHECKSUM;
	uint32_t ENTRYPOINT;
	uint8_t HEADERMINORVERSION;
	uint8_t IMAGETYPE;
	uint16_t IGNOREMASK;
	uint32_t MODULEID;
	uint32_t MODULESUBID;
	uint32_t NEXTHEADER;
	uint32_t HEADERCHECKSUM;
}

nvm_header1;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   new nvm image header (96 bytes);
 *--------------------------------------------------------------------*/

#ifndef __GNUC__
#pragma pack (push,1)
#endif

typedef struct __packed nvm_header2

{
	uint16_t MajorVersion;
	uint16_t MinorVersion;
	uint32_t ExecuteMask;
	uint32_t ImageNvmAddress;
	uint32_t ImageAddress;
	uint32_t ImageLength;
	uint32_t ImageChecksum;
	uint32_t EntryPoint;
	uint32_t NextHeader;
	uint32_t PrevHeader;
	uint32_t ImageType;
	uint16_t ModuleID;
	uint16_t ModuleSubID;
	uint16_t AppletEntryVersion;
	uint16_t Reserved0;
	uint32_t Reserved1;
	uint32_t Reserved2;
	uint32_t Reserved3;
	uint32_t Reserved4;
	uint32_t Reserved5;
	uint32_t Reserved6;
	uint32_t Reserved7;
	uint32_t Reserved8;
	uint32_t Reserved9;
	uint32_t Reserved10;
	uint32_t Reserved11;
	uint32_t HeaderChecksum;
}

nvm_header2;

#ifndef __GNUC__
#pragma pack (pop)
#endif

/*====================================================================*
 *   nvm functions;
 *--------------------------------------------------------------------*/

signed manifest (void const * memory, size_t extent);
void * manifetch (void const * memory, size_t extent, uint32_t type);
signed nvmfile (struct _file_ const *);
signed nvmfile1 (struct _file_ const *);
signed nvmfile2 (struct _file_ const *);
signed fdmanifest (signed fd, char const * filename, struct nvm_header2 *, unsigned module);
signed nvmseek1 (signed fd, char const * filename, struct nvm_header1 *, uint32_t imagetype);
signed nvmseek2 (signed fd, char const * filename, struct nvm_header2 *, uint32_t imagetype);
void nvmpeek (void const *);
void nvmpeek1 (void const *);
void nvmpeek2 (void const *);

/*====================================================================*
 *
 *--------------------------------------------------------------------*/

#endif



