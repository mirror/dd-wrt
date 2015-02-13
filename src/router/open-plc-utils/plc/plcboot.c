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

/*====================================================================*"
 *
 *   plcboot.c -
 *
 *
 *--------------------------------------------------------------------*/

/*====================================================================*"
 *   system header files;
 *--------------------------------------------------------------------*/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/stat.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/memory.h"
#include "../tools/symbol.h"
#include "../tools/number.h"
#include "../tools/types.h"
#include "../tools/flags.h"
#include "../tools/files.h"
#include "../tools/error.h"
#include "../ram/nvram.h"
#include "../ram/sdram.h"
#include "../plc/plc.h"
#include "../nvm/nvm.h"
#include "../pib/pib.h"
#include "../mme/mme.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../plc/BootDevice2.c"
#include "../plc/BootFirmware2.c"
#include "../plc/BootParameters2.c"
#include "../plc/chipset.c"
#include "../plc/Confirm.c"
#include "../plc/Devices.c"
#include "../plc/Display.c"
#include "../plc/Failure.c"
#include "../plc/FlashDevice2.c"
#include "../plc/FlashNVM.c"
#include "../plc/FlashSoftloader.c"
#include "../plc/FlashParameters.c"
#include "../plc/FlashFirmware.c"
#include "../plc/InitDevice2.c"
#include "../plc/ModuleCommit.c"
#include "../plc/ModuleSession.c"
#include "../plc/ModuleSpec.c"
#include "../plc/ModuleWrite.c"
#include "../plc/NVMSelect.c"
#include "../plc/ReadMME.c"
#include "../plc/Request.c"
#include "../plc/ResetDevice.c"
#include "../plc/SendMME.c"
#include "../plc/WaitForReset.c"
#include "../plc/WaitForStart.c"
#include "../plc/WriteExecuteApplet2.c"
#include "../plc/WriteExecuteFirmware2.c"
#include "../plc/WriteExecuteParameters2.c"
#include "../plc/WriteFirmware2.c"
#include "../plc/WriteMEM.c"
#include "../plc/WriteNVM.c"
#include "../plc/WritePIB.c"
#include "../plc/WriteParameters2.c"
#endif

#ifndef MAKEFILE
#include "../tools/basespec.c"
#include "../tools/checkfilename.c"
#include "../tools/checksum32.c"
#include "../tools/error.c"
#include "../tools/fdchecksum32.c"
#include "../tools/getoptv.c"
#include "../tools/hexdecode.c"
#include "../tools/hexstring.c"
#include "../tools/hexdump.c"
#include "../tools/putoptv.c"
#include "../tools/strfbits.c"
#include "../tools/todigit.c"
#include "../tools/typename.c"
#include "../tools/uintspec.c"
#include "../tools/version.c"
#endif

#ifndef MAKEFILE
#include "../ether/channel.c"
#include "../ether/closechannel.c"
#include "../ether/openchannel.c"
#include "../ether/readpacket.c"
#include "../ether/sendpacket.c"
#endif

#ifndef MAKEFILE
#include "../nvm/nvmfile2.c"
#endif

#ifndef MAKEFILE
#include "../pib/pibfile2.c"
#endif

#ifndef MAKEFILE
#include "../mme/EthernetHeader.c"
#include "../mme/QualcommHeader.c"
#include "../mme/UnwantedMessage.c"
#include "../mme/MMECode.c"
#endif

#ifndef MAKEFILE
#include "../key/keys.c"
#endif

/*====================================================================*
 *
 *   int main (int argc, char const * argv[]);
 *
 *
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv [])

{
	extern struct channel channel;
	static char const * optv [] =
	{
		"eFi:N:p:P:qS:t:vx",
		"-N file -P file [device] [device] [...]",
		"Qualcomm Atheros Panther/Lynx Powerline Device Bootstrapper",
		"e\tredirect stderr to stdout",
		"F[F]\tFlash [Force] non-volatile memory after boot",

#if defined (WINPCAP) || defined (LIBPCAP)

		"i n\thost interface is (n) [" LITERAL (CHANNEL_ETHNUMBER) "]",

#else

		"i s\thost interface is (s) [" LITERAL (CHANNEL_ETHDEVICE) "]",

#endif

		"N f\tfirmware file is (f)",
		"P f\tparameter file is (f)",
		"q\tquiet mode",
		"S f\tsoftloader file is (f)",
		"t n\tread timeout is (n) milliseconds [" LITERAL (CHANNEL_TIMEOUT) "]",
		"v\tverbose mode",
		"x\texit on error",
		(char const *) (0)
	};

#include "../plc/plc.c"

	char firmware [PLC_VERSION_STRING];
	signed c;
	if (getenv (PLCDEVICE))
	{

#if defined (WINPCAP) || defined (LIBPCAP)

		channel.ifindex = atoi (getenv (PLCDEVICE));

#else

		channel.ifname = strdup (getenv (PLCDEVICE));

#endif

	}
	optind = 1;
	while ((c = getoptv (argc, argv, optv)) != -1)
	{
		switch (c)
		{
		case 'e':
			dup2 (STDOUT_FILENO, STDERR_FILENO);
			break;
		case 'F':
			_setbits (plc.module, (VS_MODULE_MAC | VS_MODULE_PIB));
			if (_anyset (plc.flags, PLC_FLASH_DEVICE))
			{
				_setbits (plc.module, VS_MODULE_FORCE);
			}
			_setbits (plc.flags, PLC_FLASH_DEVICE);
			break;
		case 'i':

#if defined (WINPCAP) || defined (LIBPCAP)

			channel.ifindex = atoi (optarg);

#else

			channel.ifname = optarg;

#endif

			break;
		case 'N':
			if (!checkfilename (optarg))
			{
				error (1, EINVAL, "%s", optarg);
			}
			if ((plc.NVM.file = open (plc.NVM.name = optarg, O_BINARY|O_RDONLY)) == -1)
			{
				error (1, errno, "%s", plc.NVM.name);
			}
			if (nvmfile2 (&plc.NVM))
			{
				error (1, errno, "Bad firmware file: %s", plc.NVM.name);
			}
			_setbits (plc.flags, PLC_WRITE_MAC);
			break;
		case 'P':
			if (!checkfilename (optarg))
			{
				error (1, EINVAL, "%s", optarg);
			}
			if ((plc.PIB.file = open (plc.PIB.name = optarg, O_BINARY|O_RDONLY)) == -1)
			{
				error (1, errno, "%s", plc.PIB.name);
			}
			if (pibfile2 (&plc.PIB))
			{
				error (1, errno, "Bad parameter file: %s", plc.PIB.name);
			}
			_setbits (plc.flags, PLC_WRITE_PIB);
			break;
		case 'q':
			_setbits (channel.flags, CHANNEL_SILENCE);
			_setbits (plc.flags, PLC_SILENCE);
			break;
		case 'S':
			if (!checkfilename (optarg))
			{
				error (1, EINVAL, "%s", optarg);
			}
			if ((plc.CFG.file = open (plc.CFG.name = optarg, O_BINARY|O_RDONLY)) == -1)
			{
				error (1, errno, "%s", plc.CFG.name);
			}
			if (nvmfile2 (&plc.CFG))
			{
				error (1, errno, "Bad softloader file: %s", plc.CFG.name);
			}
			break;
		case 't':
			channel.timeout = (signed)(uintspec (optarg, 0, UINT_MAX));
			break;
		case 'v':
			_setbits (channel.flags, CHANNEL_VERBOSE);
			_setbits (plc.flags, PLC_VERBOSE);
			break;
		case 'x':
			_setbits (plc.flags, PLC_BAILOUT);
			break;
		default:
			break;
		}
	}
	argc -= optind;
	argv += optind;
	if (argc)
	{
		error (1, ENOTSUP, ERROR_TOOMANY);
	}
	openchannel (&channel);
	if (!(plc.message = malloc (sizeof (* plc.message))))
	{
		error (1, errno, PLC_NOMEMORY);
	}
	if (WaitForStart (&plc, firmware, sizeof (firmware)))
	{
		Failure (&plc, PLC_NODETECT);
		exit (1);
	}
	if (strcmp (firmware, "BootLoader"))
	{
		Failure (&plc, "Bootloader must be running");
		exit (1);
	}
	if (plc.hardwareID < CHIPSET_INT6400)
	{
		Failure (&plc, "Device must be %s or later; Use program int6kboot or ampboot instead.", chipsetname (CHIPSET_INT6400));
		exit (1);
	}
	if (plc.PIB.file == -1)
	{
		error (1, ECANCELED, "No Parameter file named");
	}
	if (plc.NVM.file == -1)
	{
		error (1, ECANCELED, "No Firmware file named");
	}
	if (plc.CFG.file == -1)
	{
		if (_anyset (plc.flags, PLC_FLASH_DEVICE))
		{
			error (1, ECANCELED, "No Softloader file named");
		}
	}
	if (!InitDevice2 (&plc))
	{
		if (!BootDevice2 (&plc))
		{
			if (_anyset (plc.flags, PLC_FLASH_DEVICE))
			{
				FlashDevice2 (& plc, (PLC_COMMIT_FORCE | PLC_COMMIT_NORESET | PLC_COMMIT_FACTPIB));
			}
		}
	}
	free (plc.message);
	closechannel (&channel);
	exit (0);
}

