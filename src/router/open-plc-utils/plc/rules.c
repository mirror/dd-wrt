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
 *   rules.c - Classification Rules Lookup Tables;
 *
 *   rules.h
 *
 *   QoS related symbol tables used by function ParseRule;
 *
 *   Contributor(s):
 *	Charles Maier <cmaier@qca.qualcomm.com>
 *	Nathaniel Houghton <nhoughto@qca.qualcomm.com>
 *
 *--------------------------------------------------------------------*/

#ifndef RULES_SOURCE
#define RULES_SOURCE

#include "../plc/rules.h"

struct _code_ const controls [] =

{
	{
		CONTROL_ADD,
		"Add"
	},
	{
		CONTROL_REM,
		"Rem"
	},
	{
		CONTROL_REMOVE,
		"Remove"
	}
};

struct _code_ const volatilities [] =

{
	{
		VOLATILITY_TEMP,
		"Temp"
	},
	{
		VOLATILITY_PERM,
		"Perm"
	}
};

struct _code_ const actions [] =

{
	{
		ACTION_CAP0,
		"CAP0"
	},
	{
		ACTION_CAP1,
		"CAP1"
	},
	{
		ACTION_CAP2,
		"CAP2"
	},
	{
		ACTION_CAP3,
		"CAP3"
	},
	{
		ACTION_BOOST,
		"Boost"
	},
	{
		ACTION_DROP,
		"Drop"
	},
	{
		ACTION_DROPTX,
		"DropTX"
	},
	{
		ACTION_DROPRX,
		"DropRX"
	},
	{
		ACTION_AUTOCONNECT,
		"AutoConnect"
	},
	{
		ACTION_STRIPTX,
		"StripTX"
	},
	{
		ACTION_STRIPRX,
		"StripRX"
	},
	{
		ACTION_TAGTX,
		"TagTX"
	},
	{
		ACTION_TAGRX,
		"TagRX"
	}
};

struct _code_ const operands [] =

{
	{
		OPERAND_ALL,
		"All"
	},
	{
		OPERAND_ANY,
		"Any"
	},
	{
		OPERAND_ALWAYS,
		"Always"
	}
};

struct _code_ const fields [] =

{
	{
		FIELD_ETH_DA,
		"EthDA"
	},
	{
		FIELD_ETH_SA,
		"EthSA"
	},
	{
		FIELD_VLAN_UP,
		"VLANUP"
	},
	{
		FIELD_VLAN_ID,
		"VLANID"
	},
	{
		FIELD_IPV4_TOS,
		"IPv4TOS"
	},
	{
		FIELD_IPV4_PROT,
		"IPv4PROT"
	},
	{
		FIELD_IPV4_SA,
		"IPv4SA"
	},
	{
		FIELD_IPV4_DA,
		"IPv4DA"
	},
	{
		FIELD_IPV6_TC,
		"IPv6TC"
	},
	{
		FIELD_IPV6_FL,
		"IPv6FL"
	},
	{
		FIELD_IPV6_SA,
		"IPv6SA"
	},
	{
		FIELD_IPV6_DA,
		"IPv6DA"
	},
	{
		FIELD_TCP_SP,
		"TCPSP"
	},
	{
		FIELD_TCP_DP,
		"TCPDP"
	},
	{
		FIELD_UDP_SP,
		"UDPSP"
	},
	{
		FIELD_UDP_DP,
		"UDPDP"
	},
	{
		FIELD_IP_SP,
		"IPSP"
	},
	{
		FIELD_IP_DP,
		"IPDP"
	},
	{
		FIELD_HPAV_MME,
		"MME"
	},
	{
		FIELD_ETH_TYPE,
		"ET"
	},
	{
		FIELD_TCP_ACK,
		"TCPAck"
	},
	{
		FIELD_VLAN_TAG,
		"VLANTag"
	}
};

struct _code_ const operators [] =

{
	{
		OPERATOR_IS,
		"Is"
	},
	{
		OPERATOR_NOT,
		"Not"
	}
};

struct _code_ const states [] =

{
	{
		OPERATOR_IS,
		"True"
	},
	{
		OPERATOR_NOT,
		"False"
	},
	{
		OPERATOR_IS,
		"On"
	},
	{
		OPERATOR_NOT,
		"Off"
	},
	{
		OPERATOR_IS,
		"Yes"
	},
	{
		OPERATOR_NOT,
		"No"
	},
	{
		OPERATOR_IS,
		"Present"
	},
	{
		OPERATOR_NOT,
		"Missing"
	}
};

#endif

