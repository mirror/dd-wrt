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
 *   amptool.c -
 *
 *
 *   Contributor(s):
 *      Charles Maier <cmaier@qca.qualcomm.com>
 *      Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/


/*====================================================================*"
 *   system header files;
 *--------------------------------------------------------------------*/

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

/*====================================================================*
 *   custom header files;
 *--------------------------------------------------------------------*/

#include "../tools/getoptv.h"
#include "../tools/putoptv.h"
#include "../tools/memory.h"
#include "../tools/number.h"
#include "../tools/symbol.h"
#include "../tools/types.h"
#include "../tools/flags.h"
#include "../tools/files.h"
#include "../tools/error.h"
#include "../ether/channel.h"
#include "../key/HPAVKey.h"
#include "../key/keys.h"
#include "../ram/sdram.h"
#include "../pib/pib.h"
#include "../nvm/nvm.h"
#include "../plc/plc.h"

/*====================================================================*
 *   custom source files;
 *--------------------------------------------------------------------*/

#ifndef MAKEFILE
#include "../plc/Attributes1.c"
#include "../plc/chipset.c"
#include "../plc/NVRAMInfo.c"
#include "../plc/SDRAMInfo.c"
#include "../plc/Devices.c"
#include "../plc/Confirm.c"
#include "../plc/Display.c"
#include "../plc/FlashNVM.c"
#include "../plc/FactoryDefaults.c"
#include "../plc/HostActionResponse.c"
#include "../plc/Identity1.c"
#include "../plc/NetInfo2.c"
#include "../plc/PushButton.c"
#include "../plc/SetNMK.c"
#include "../plc/Request.c"
#include "../plc/Failure.c"
#include "../plc/PLCSelect.c"
#include "../plc/ReadFirmware1.c"
#include "../plc/ReadMME.c"
#include "../plc/ReadMFG.c"
#include "../plc/ReadParameters1.c"
#include "../plc/RemoteHosts.c"
#include "../plc/ResetDevice.c"
#include "../plc/SendMME.c"
#include "../plc/StationRole.c"
#include "../plc/VersionInfo1.c"
#include "../plc/WriteNVM.c"
#include "../plc/WritePIB.c"
#include "../plc/WaitForReset.c"
#include "../plc/WaitForStart.c"
#include "../plc/WatchdogReport.c"
#include "../plc/StartFirmware1.c"
#endif

#ifndef MAKEFILE
#include "../tools/error.c"
#include "../tools/getoptv.c"
#include "../tools/putoptv.c"
#include "../tools/synonym.c"
#include "../tools/uintspec.c"
#include "../tools/version.c"
#include "../tools/hexdump.c"
#include "../tools/hexencode.c"
#include "../tools/hexdecode.c"
#include "../tools/hexstring.c"
#include "../tools/hexout.c"
#include "../tools/todigit.c"
#include "../tools/checkfilename.c"
#include "../tools/checksum32.c"
#include "../tools/fdchecksum32.c"
#include "../tools/strfbits.c"
#include "../tools/typename.c"
#endif

#ifndef MAKEFILE
#include "../ether/openchannel.c"
#include "../ether/closechannel.c"
#include "../ether/readpacket.c"
#include "../ether/sendpacket.c"
#include "../ether/channel.c"
#endif

#ifndef MAKEFILE
#include "../ram/nvram.c"
#include "../ram/sdramfile.c"
#include "../ram/sdrampeek.c"
#endif

#ifndef MAKEFILE
#include "../nvm/nvmfile1.c"
#endif

#ifndef MAKEFILE
#include "../pib/pibfile1.c"
#include "../pib/pibpeek1.c"
#endif

#ifndef MAKEFILE
#include "../mme/EthernetHeader.c"
#include "../mme/QualcommHeader.c"
#include "../mme/QualcommHeader1.c"
#include "../mme/UnwantedMessage.c"
#include "../mme/MMECode.c"
#endif

#ifndef MAKEFILE
#include "../key/keys.c"
#endif

/*====================================================================*
 *   program constants;
 *--------------------------------------------------------------------*/

#define AMPTOOL_WAIT 0
#define AMPTOOL_LOOP 1

/*====================================================================*
 *   program variables;
 *--------------------------------------------------------------------*/

static const struct _term_ modules [] =

{
	{
		"nvm",
		"1"
	},
	{
		"pib",
		"2"
	},
	{
		"both",
		"3"
	}
};

static const struct _term_ buttons [] =

{
	{
		"join",
		"1"
	},
	{
		"leave",
		"2"
	},
	{
		"status",
		"3"
	}
};


/*====================================================================*
 *
 *   void manager (struct plc * plc, signed count, signed pause);
 *
 *   perform operations in logical order despite any order specfied
 *   on the command line; for example read PIB before writing PIB;
 *
 *   operation order is controlled by the order of "if" statements
 *   shown here; the entire operation sequence can be repeated with
 *   an optional pause between each iteration;
 *
 *
 *--------------------------------------------------------------------*/

static void manager (struct plc * plc, signed count, signed pause)

{
	while (count--)
	{
		if (_anyset (plc->flags, PLC_VERSION))
		{
			VersionInfo1 (plc);
		}
		if (_anyset (plc->flags, PLC_ATTRIBUTES))
		{
			Attributes1 (plc);
		}
		if (_anyset (plc->flags, PLC_WATCHDOG_REPORT))
		{
			WatchdogReport (plc);
		}
		if (_anyset (plc->flags, PLC_NVRAM_INFO))
		{
			NVRAMInfo (plc);
		}
		if (_anyset (plc->flags, PLC_SDRAM_INFO))
		{
			SDRAMInfo (plc);
		}
		if (_anyset (plc->flags, PLC_READ_IDENTITY))
		{
			Identity1 (plc);
		}
		if (_anyset (plc->flags, PLC_REMOTEHOSTS))
		{
			RemoteHosts (plc);
		}
		if (_anyset (plc->flags, PLC_NETWORK))
		{
			NetInfo2 (plc);
		}
		if (_anyset (plc->flags, PLC_WRITE_MAC))
		{
			WriteNVM (plc);
		}
		if (_anyset (plc->flags, PLC_READ_MAC))
		{
			ReadFirmware1 (plc);
		}
		if (_anyset (plc->flags, PLC_WRITE_PIB))
		{
			WritePIB (plc);
		}
		if (_anyset (plc->flags, PLC_READ_PIB))
		{
			ReadParameters1 (plc);
		}
		if (_anyset (plc->flags, PLC_HOST_ACTION))
		{
			HostActionResponse (plc);
		}
		if (_anyset (plc->flags, PLC_PUSH_BUTTON))
		{
			PushButton (plc);
		}
		if (_anyset (plc->flags, (PLC_SETLOCALKEY | PLC_SETREMOTEKEY)))
		{
			SetNMK (plc);
		}
		if (_anyset (plc->flags, PLC_FACTORY_DEFAULTS))
		{
			FactoryDefaults (plc);
		}
		if (_anyset (plc->flags, PLC_FLASH_DEVICE))
		{
			FlashNVM (plc);
		}
		if (_anyset (plc->flags, PLC_RESET_DEVICE))
		{
			ResetDevice (plc);
		}
		sleep (pause);
	}
	return;
}


/*====================================================================*
 *
 *   int main (int argc, char const * argv[]);
 *
 *   parse command line, populate plc structure and perform selected
 *   operations; show help summary if asked; see getoptv and putoptv
 *   to understand command line parsing and help summary display; see
 *   plc.h for the definition of struct plc;
 *
 *   the command line accepts multiple MAC addresses and the program
 *   performs the specified operations on each address, in turn; the
 *   address order is significant but the option order is not; the
 *   default address is a local broadcast that causes all devices on
 *   the local H1 interface to respond but not those at the remote
 *   end of the powerline;
 *
 *   the default address is 00:B0:52:00:00:01; omitting the address
 *   will automatically address the local device; some options will
 *   cancel themselves if this makes no sense;
 *
 *   the default interface is eth1 because most people use eth0 as
 *   their principle network connection; you can specify another
 *   interface with -i or define environment string PLC to make
 *   that the default interface and save typing;
 *
 *
 *--------------------------------------------------------------------*/

int main (int argc, char const * argv [])

{
	extern struct channel channel;
	extern struct key const keys [];
	static char const * optv [] =
	{
		"abB:C:d:D:efFgHi:IJ:K:l:mMn:N:p:P:QqrsRt:Tvw:x",
		"device [device] [...]",
		"Qualcomm Atheros AR7x00 Powerline Device Manager",
		"a\tread Device Attributes using VS_OP_ATTRIBUTES",
		"b\tread device enumeration ID table using VS_EM_ID_TABLE",
		"B n\tperform pushbutton action (n) using MS_PB_ENC [1|2|3|'join'|'leave'|'status']",
		"C n\tflash NVRAM with module (n) using VS_MOD_NVM [1|2|3|'nvm'|'pib'|'both']",
		"C n\tflash NVRAM with module (n) using VS_MOD_NVM",
		"d f\tdump and clear watchdog report to file (f) using VS_WD_RPT",
		"D x\tDevice Access Key (DAK) is (x) [" DAK1 "]",
		"e\tredirect stderr to stdout",
		"f\tread NVRAM Configuration using VS_GET_NVM",
		"F[F]\tflash [force] NVRAM with PIB and firmware using VS_MOD_NVM",
		"g\tdisplay multicast group information using VS_MULTICAST_INFO",
		"H\tstop host action requests messages with VS_HOST_ACTION.IND",

#if defined (WINPCAP) || defined (LIBPCAP)

		"i n\thost interface is (n) [" LITERAL (CHANNEL_ETHNUMBER) "]",

#else

		"i s\thost interface is (s) [" LITERAL (CHANNEL_ETHDEVICE) "]",

#endif

		"I\tread device identity using VS_RD_MOD",
		"J x\tset NMK on remote device (x) via local device using VS_SET_KEY (see -K)",
		"K x\tNetwork Membership Key (NMK) is (x) [" NMK1 "]",
		"l n\tloop (n) times [" LITERAL (AMPTOOL_LOOP) "]",
		"m\tread network membership information using VS_NW_INFO",
		"M\tset NMK on local device using VS_SET_KEY (see -K)",
		"n f\tread NVM from SDRAM to file (f) using VS_RD_MOD",
		"N f\twrite NVM file (f) to SDRAM using VS_WR_MOD",
		"p f\tread PIB from SDRAM to file (f) using VS_RD_MOD",
		"P f\twrite PIB file (f) to SDRAM using VS_WR_MOD",
		"q\tquiet mode",
		"Q\tquick flash (return immediately)",
		"r\tread hardware and firmware revision using VS_SW_VER",
		"R\treset device using VS_RS_DEV",
		"s\tread SDRAM Configuration using VS_RD_CBLOCK",
		"t n\tread timeout is (n) milliseconds [" LITERAL (CHANNEL_TIMEOUT) "]",
		"T\trestore factory defaults using VS_FAC_DEFAULTS",
		"v\tverbose mode",
		"w n\tpause (n) seconds [" LITERAL (AMPTOOL_WAIT) "]",
		"x\texit on error",
		(char const *) (0)
	};

#include "../plc/plc.c"

	signed loop = AMPTOOL_LOOP;
	signed wait = AMPTOOL_WAIT;
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
		case 'a':
			_setbits (plc.flags, PLC_ATTRIBUTES);
			break;
		case 'B':
			_setbits (plc.flags, PLC_PUSH_BUTTON);
			plc.pushbutton = (unsigned)(uintspec (synonym (optarg, buttons, SIZEOF (buttons)), 0, UCHAR_MAX));
			break;
		case 'b':
			_setbits (plc.flags, PLC_REMOTEHOSTS);
			break;
		case 'C':
			_setbits (plc.flags, PLC_FLASH_DEVICE);
			plc.module = (unsigned)(uintspec (synonym (optarg, modules, SIZEOF (modules)), 0, UCHAR_MAX));
			break;
		case 'd':
			_setbits (plc.flags, PLC_WATCHDOG_REPORT);
			if (!checkfilename (optarg))
			{
				error (1, EINVAL, "%s", optarg);
			}
			if ((plc.rpt.file = open (plc.rpt.name = optarg, O_BINARY|O_CREAT|O_RDWR|O_TRUNC, FILE_FILEMODE)) == -1)
			{
				error (1, errno, "%s", plc.rpt.name);
			}

#ifndef WIN32

			chown (optarg, getuid (), getgid ());

#endif

			plc.readaction = 3;
			break;
		case 'D':
			if (!strcmp (optarg, "none"))
			{
				memcpy (plc.DAK, keys [0].DAK, sizeof (plc.DAK));
				break;
			}
			if (!strcmp (optarg, "key1"))
			{
				memcpy (plc.DAK, keys [1].DAK, sizeof (plc.DAK));
				break;
			}
			if (!strcmp (optarg, "key2"))
			{
				memcpy (plc.DAK, keys [2].DAK, sizeof (plc.DAK));
				break;
			}
			if (!hexencode (plc.DAK, sizeof (plc.DAK), (char const *)(optarg)))
			{
				error (1, errno, PLC_BAD_DAK, optarg);
			}
			break;
		case 'e':
			dup2 (STDOUT_FILENO, STDERR_FILENO);
			break;
		case 'f':
			_setbits (plc.flags, PLC_NVRAM_INFO);
			break;
		case 'F':
			_setbits (plc.module, (VS_MODULE_MAC | VS_MODULE_PIB));
			if (_anyset (plc.flags, PLC_FLASH_DEVICE))
			{
				_setbits (plc.module, VS_MODULE_FORCE);
			}
			_setbits (plc.flags, PLC_FLASH_DEVICE);
			break;
		case 'H':
			_setbits (plc.flags, PLC_HOST_ACTION);
			break;
		case 'I':
			_setbits (plc.flags, PLC_READ_IDENTITY);
			break;
		case 'i':

#if defined (WINPCAP) || defined (LIBPCAP)

			channel.ifindex = atoi (optarg);

#else

			channel.ifname = optarg;

#endif

			break;
		case 'J':
			if (!hexencode (plc.RDA, sizeof (plc.RDA), (char const *)(optarg)))
			{
				error (1, errno, PLC_BAD_MAC, optarg);
			}
			_setbits (plc.flags, PLC_SETREMOTEKEY);
			break;
		case 'K':
			if (!strcmp (optarg, "none"))
			{
				memcpy (plc.NMK, keys [0].NMK, sizeof (plc.NMK));
				break;
			}
			if (!strcmp (optarg, "key1"))
			{
				memcpy (plc.NMK, keys [1].NMK, sizeof (plc.NMK));
				break;
			}
			if (!strcmp (optarg, "key2"))
			{
				memcpy (plc.NMK, keys [2].NMK, sizeof (plc.NMK));
				break;
			}
			if (!hexencode (plc.NMK, sizeof (plc.NMK), (char const *)(optarg)))
			{
				error (1, errno, PLC_BAD_NMK, optarg);
			}
			break;
		case 'M':
			_setbits (plc.flags, PLC_SETLOCALKEY);
			break;
		case 'l':
			loop = (unsigned)(uintspec (optarg, 0, UINT_MAX));
			break;
		case 'm':
			_setbits (plc.flags, PLC_NETWORK);
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
			if (nvmfile1 (&plc.NVM))
			{
				error (1, errno, "Bad firmware file: %s", plc.NVM.name);
			}
			_setbits (plc.flags, PLC_WRITE_MAC);
			break;
		case 'n':
			if (!checkfilename (optarg))
			{
				error (1, EINVAL, "%s", optarg);
			}
			if ((plc.nvm.file = open (plc.nvm.name = optarg, O_BINARY|O_CREAT|O_RDWR|O_TRUNC, FILE_FILEMODE)) == -1)
			{
				error (1, errno, "%s", plc.nvm.name);
			}

#ifndef WIN32

			chown (optarg, getuid (), getgid ());

#endif

			_setbits (plc.flags, PLC_READ_MAC);
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
			if (pibfile1 (&plc.PIB))
			{
				error (1, errno, "Bad parameter file: %s", plc.PIB.name);
			}
			_setbits (plc.flags, PLC_WRITE_PIB);
			break;
		case 'p':
			if (!checkfilename (optarg))
			{
				error (1, EINVAL, "%s", optarg);
			}
			if ((plc.pib.file = open (plc.pib.name = optarg, O_BINARY|O_CREAT|O_RDWR|O_TRUNC, FILE_FILEMODE)) == -1)
			{
				error (1, errno, "%s", plc.pib.name);
			}

#ifndef WIN32

			chown (optarg, getuid (), getgid ());

#endif

			_setbits (plc.flags, PLC_READ_PIB);
			break;
		case 'Q':
			_setbits (plc.flags, PLC_QUICK_FLASH);
			break;
		case 'q':
			_setbits (channel.flags, CHANNEL_SILENCE);
			_setbits (plc.flags, PLC_SILENCE);
			break;
		case 'R':
			_setbits (plc.flags, PLC_RESET_DEVICE);
			break;
		case 'r':
			_setbits (plc.flags, PLC_VERSION);
			break;
		case 's':
			_setbits (plc.flags, PLC_SDRAM_INFO);
			break;
		case 't':
			channel.timeout = (signed)(uintspec (optarg, 0, UINT_MAX));
			break;
		case 'T':
			_setbits (plc.flags, PLC_FACTORY_DEFAULTS);
			break;
		case 'v':
			_setbits (channel.flags, CHANNEL_VERBOSE);
			_setbits (plc.flags, PLC_VERBOSE);
			break;
		case 'V':
			_setbits (plc.flags, PLC_SNIFFER);
			plc.action = (uint8_t)(uintspec (optarg, 0, UCHAR_MAX));
			break;
		case 'w':
			wait = (unsigned)(uintspec (optarg, 0, 3600));
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
	if (argc != 1)
	{
		if (plc.nvm.file != -1)
		{
			error (1, ECANCELED, PLC_NODEVICE);
		}
		if (plc.pib.file != -1)
		{
			error (1, ECANCELED, PLC_NODEVICE);
		}
		if (plc.rpt.file != -1)
		{
			error (1, ECANCELED, PLC_NODEVICE);
		}
	}
	openchannel (&channel);
	if (!(plc.message = malloc (sizeof (* plc.message))))
	{
		error (1, errno, PLC_NOMEMORY);
	}
	if (!argc)
	{
		manager (&plc, loop, wait);
	}
	while ((argc) && (* argv))
	{
		if (!hexencode (channel.peer, sizeof (channel.peer), synonym (* argv, devices, SIZEOF (devices))))
		{
			error (1, errno, PLC_BAD_MAC, * argv);
		}
		manager (&plc, loop, wait);
		argc--;
		argv++;
	}
	free (plc.message);
	closechannel (&channel);
	exit (0);
}

