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
 *   signed openchannel (struct channel * channel);
 *
 *   channel.h
 *
 *   open a raw ethernet channel;
 *
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *	Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef OPENCHANNEL_SOURCE
#define OPENCHANNEL_SOURCE

#include <unistd.h>
#include <memory.h>
#include <errno.h>

#if defined (__linux__)
#	include <net/if_arp.h>
#	include <netpacket/packet.h>
#	include <sys/ioctl.h>
#elif defined (__APPLE__)
#	include <sys/ioctl.h>
#	include <sys/stat.h>
#	include <fcntl.h>
#	include <stdlib.h>
#elif defined (__OpenBSD__)
#	include <sys/ioctl.h>
#	include <sys/stat.h>
#	include <sys/types.h>
#	include <fcntl.h>
#	include <stdlib.h>
#elif defined (WINPCAP)
#	include <string.h>
#else
#error "Unknown environment"
#endif

#include "../ether/channel.h"
#include "../tools/memory.h"
#include "../tools/flags.h"
#include "../tools/error.h"

#if defined (__APPLE__) || defined (__OpenBSD__)
#	include "../ether/gethwaddr.c"
#endif

signed openchannel (struct channel * channel)

{

#if defined (__linux__)

	struct ifreq ifreq;
	struct sockaddr_ll sockaddr_ll =
	{
		PF_PACKET,
		0x0000,
		0x0000,
		ARPHRD_ETHER,
		PACKET_HOST,
		ETHER_ADDR_LEN,
		{
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00,
			0x00
		}
	};

/*
 *      raw packets require root privileges on linux; one does not have to be
 *      root when this program is installed setuid using 'chown root:root' and
 *      'chmod 4555';
 */

	if (geteuid ())
	{
		error (1, EPERM, ERROR_NOTROOT);
	}

	memset (&ifreq, 0, sizeof (ifreq));
	sockaddr_ll.sll_protocol = htons (channel->type);
	if ((channel->fd = socket (sockaddr_ll.sll_family, SOCK_RAW, sockaddr_ll.sll_protocol)) == -1)
	{
		error (1, errno, "%s", channel->ifname);
	}
	memcpy (ifreq.ifr_name, channel->ifname, sizeof (ifreq.ifr_name));
	if (ioctl (channel->fd, SIOCGIFINDEX, &ifreq) == -1)
	{
		error (1, errno, "%s", ifreq.ifr_name);
	}
	channel->ifindex = sockaddr_ll.sll_ifindex = ifreq.ifr_ifindex;
	if (ioctl (channel->fd, SIOCGIFHWADDR, &ifreq) == -1)
	{
		error (1, errno, "%s", ifreq.ifr_name);
	}
	memcpy (sockaddr_ll.sll_addr, ifreq.ifr_ifru.ifru_hwaddr.sa_data, sizeof (sockaddr_ll.sll_addr));
	if (bind (channel->fd, (struct sockaddr *) (&sockaddr_ll), sizeof (sockaddr_ll)) == -1)
	{
		error (1, errno, "%s", ifreq.ifr_name);
	}
	memcpy (channel->host, sockaddr_ll.sll_addr, sizeof (channel->host));
	if (ioctl (channel->fd, SIOCGIFFLAGS, &ifreq) == -1)
	{
		error (1, errno, "%s", ifreq.ifr_name);
	}
	channel->ifstate = ifreq.ifr_flags;
	_setbits (ifreq.ifr_flags, (IFF_UP | IFF_BROADCAST | IFF_MULTICAST));
	_clrbits (ifreq.ifr_flags, (IFF_ALLMULTI | IFF_PROMISC));
	if (ioctl (channel->fd, SIOCSIFFLAGS, &ifreq) == -1)
	{
		error (1, errno, "%s", ifreq.ifr_name);
	}

#else

	struct bpf_program bpf_program;
	static struct bpf_insn bpf_insn [] =
	{
		{
			BPF_LD + BPF_H + BPF_ABS,
			0,
			0,
			12
		},
		{
			BPF_JMP + BPF_JEQ + BPF_K,
			0,
			18,
			0
		},
		{
			BPF_LD + BPF_B + BPF_ABS,
			0,
			0,
			0
		},
		{
			BPF_JMP + BPF_JEQ + BPF_K,
			0,
			10,
			0
		},
		{
			BPF_LD + BPF_B + BPF_ABS,
			0,
			0,
			1
		},
		{
			BPF_JMP + BPF_JEQ + BPF_K,
			0,
			8,
			0
		},
		{
			BPF_LD + BPF_B + BPF_ABS,
			0,
			0,
			2
		},
		{
			BPF_JMP + BPF_JEQ + BPF_K,
			0,
			6,
			0
		},
		{
			BPF_LD + BPF_B + BPF_ABS,
			0,
			0,
			3
		},
		{
			BPF_JMP + BPF_JEQ + BPF_K,
			0,
			4,
			0
		},
		{
			BPF_LD + BPF_B + BPF_ABS,
			0,
			0,
			4
		},
		{
			BPF_JMP + BPF_JEQ + BPF_K,
			0,
			2,
			0
		},
		{
			BPF_LD + BPF_B + BPF_ABS,
			0,
			0,
			5
		},
		{
			BPF_JMP + BPF_JEQ + BPF_K,
			4,
			0,
			0
		},
		{
			BPF_LD + BPF_W + BPF_ABS,
			0,
			0,
			0
		},
		{
			BPF_JMP + BPF_JEQ + BPF_K,
			0,
			4,
			0xFFFFFFFF
		},
		{
			BPF_LD + BPF_H + BPF_ABS,
			0,
			0,
			4
		},
		{
			BPF_JMP + BPF_JEQ + BPF_K,
			0,
			2,
			0xFFFF
		},
		{
			BPF_LD + BPF_W + BPF_LEN,
			0,
			0,
			0
		},
		{
			BPF_RET + BPF_A,
			0,
			0,
			0
		},
		{
			BPF_RET + BPF_K,
			0,
			0,
			0
		}
	};

#if defined (__APPLE__) || defined (__OpenBSD__)

	struct ifreq ifreq;
	struct timeval timeval;
	struct bpf * bpf;
	char filename [sizeof (CHANNEL_BPFDEVICE) + 1];
	unsigned count;
	unsigned state;
	int stat_errno = 0;
	int open_errno = 0;
	for (count = 0; count < 100; count++)
	{
		struct stat st;
		snprintf (filename, sizeof (filename), CHANNEL_BPFDEVICE, count);
		if (stat(filename, &st) == -1)
		{
			stat_errno = errno;
			continue;
		}
		if ((channel->fd = open (filename, O_RDWR)) != -1)
		{
			break;
		}
		else
		{
			open_errno = errno;
		}
	}
	if (channel->fd == -1)
	{
		if (open_errno)
		{
			error (1, open_errno, "Could not open bpf device");
		}
		else
		{
			error (1, stat_errno, "No bpf device found");
		}
	}
	memcpy (ifreq.ifr_name, channel->ifname, sizeof (ifreq.ifr_name));
	if (ioctl (channel->fd, BIOCSETIF, &ifreq) == -1)
	{
		error (1, errno, "%s", ifreq.ifr_name);
	}
	channel->bpf = bpf = malloc (sizeof (* bpf));
	if (ioctl (channel->fd, BIOCGBLEN, &bpf->bpf_length) == -1)
	{
		error (1, errno, "Can't determine buffer length: %s", ifreq.ifr_name);
	}
	bpf->bpf_bp = bpf->bpf_buffer = malloc (bpf->bpf_length);
	if (bpf->bpf_buffer == NULL)
	{
		error (1, errno, "Can't allocate receive buffer");
	}

#if defined (__APPLE__)

	state = 0;
	if (ioctl (channel->fd, BIOCSSEESENT, &state) == -1)
	{
		error (1, errno, "Can't hide outgoing frames: %s", ifreq.ifr_name);
	}

#elif defined (__OpenBSD__)

	state = BPF_DIRECTION_OUT;
	if (ioctl (channel->fd, BIOCSDIRFILT, &state) == -1)
	{
		error (0, errno, "Can't hide outgoing frames");
	}

#else
#error "Abandon all hope"
#endif

	if (channel->capture > 1000)
	{
		timeval.tv_sec = channel->capture / 1000;
		timeval.tv_usec = 0;
	}
	else
	{

#if defined (__MAC_10_6)

/*
 *	accommodate known bug in BPF on MAC OS X 10.6; shorter times cause socket read
 *	operations to block indefinitely if no frames are waiting because tv_usec gets
 *      clobbered;
 */

		timeval.tv_sec = 1;
		timeval.tv_usec = 0;

#else

		timeval.tv_sec = 0;
		timeval.tv_usec = channel->capture * 1000;

#endif

	}
	if (ioctl (channel->fd, BIOCSRTIMEOUT, &timeval) == -1)
	{
		error (1, errno, "Can't set channel timeout: %s", ifreq.ifr_name);
	}
	state = 1;
	if (ioctl (channel->fd, BIOCIMMEDIATE, &state) == -1)
	{
		error (1, errno, "Can't set immediate mode: %s", ifreq.ifr_name);
	}

#if 1

	state = 1;
	if (ioctl (channel->fd, BIOCSHDRCMPLT, &state) == -1)
	{
		error (1, errno, "Can't set header complete mode: %s", ifreq.ifr_name);
	}

#endif

#if 1

	gethwaddr (channel->host, channel->ifname);

#else

	if (ioctl (channel->fd, SIOCGIFADDR, &ifreq) > 0)
	{
		error (1, errno, "%s", ifreq.ifr_name);
	}
	memcpy (channel->host, LLADDR (ifreq.ifr_ifru.ifru_addr), sizeof (channel->host));

#endif

	bpf_program.bf_len = sizeof (bpf_insn) / sizeof (struct bpf_insn);
	bpf_program.bf_insns = bpf_insn;
	if (channel->type == ETH_P_802_2)
	{
		bpf_insn [1].code = BPF_JMP + BPF_JGT + BPF_K;
		bpf_insn [1].jt = 18;
		bpf_insn [1].jf = 0;
		bpf_insn [1].k = ETHERMTU;
	}
	else
	{
		bpf_insn [1].code = BPF_JMP + BPF_JEQ + BPF_K;
		bpf_insn [1].jt = 0;
		bpf_insn [1].jf = 18;
		bpf_insn [1].k = channel->type;
	}
	bpf_insn [3].k = channel->host [0];
	bpf_insn [5].k = channel->host [1];
	bpf_insn [7].k = channel->host [2];
	bpf_insn [9].k = channel->host [3];
	bpf_insn [11].k = channel->host [4];
	bpf_insn [13].k = channel->host [5];
	if (ioctl (channel->fd, BIOCSETF, &bpf_program) == -1)
	{
		error (1, errno, "Can't store filter: %s", channel->ifname);
	}

#elif defined (WINPCAP) || defined (LIBPCAP)

	channel->ifname = getifname (channel->ifindex);
	gethwaddr (channel->host, channel->ifname);
	channel->socket = pcap_open_live (channel->ifname, 65536, 0, channel->capture, channel->errbuf);
	snprintf ((char *)(channel->ifname), strlen (channel->ifname), "nic%d", channel->ifindex);
	if (!channel->socket)
	{
		error (1, errno, "Can't open interface: %s", channel->ifname);
	}
	bpf_program.bf_len = sizeof (bpf_insn)/sizeof (struct bpf_insn);
	bpf_program.bf_insns = bpf_insn;
	if (channel->type == ETH_P_802_2)
	{
		bpf_insn [1].code = BPF_JMP + BPF_JGT + BPF_K;
		bpf_insn [1].jt = 18;
		bpf_insn [1].jf = 0;
		bpf_insn [1].k = ETHERMTU;
	}
	else
	{
		bpf_insn [1].code = BPF_JMP + BPF_JEQ + BPF_K;
		bpf_insn [1].jt = 0;
		bpf_insn [1].jf = 18;
		bpf_insn [1].k = channel->type;
	}
	bpf_insn [3].k = channel->host [0];
	bpf_insn [5].k = channel->host [1];
	bpf_insn [7].k = channel->host [2];
	bpf_insn [9].k = channel->host [3];
	bpf_insn [11].k = channel->host [4];
	bpf_insn [13].k = channel->host [5];
	if (pcap_setfilter (channel->socket, &bpf_program) < 0)
	{
		error (1, errno, "Can't store filter: %s", channel->ifname);
	}
	if (pcap_setmintocopy (channel->socket, ETHER_MIN_LEN) < 0)
	{
		error (1, errno, "Can't set pcap mintocopy: %s", channel->ifname);
	}

#else
#error "Unknown Environment"
#endif
#endif

	return (0);
}


#endif

