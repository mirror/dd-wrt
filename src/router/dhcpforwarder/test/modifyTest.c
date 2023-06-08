/*	--*- c -*--
 * Copyright (C) 2014 Enrico Scholz <enrico.scholz@ensc.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#define main	main_
#include "../src/main.c"
#undef main

static struct InterfaceInfo const	iface0;

static struct DHCPSubOption		subopt0[] = {
	[0] = {
		.code	= agREMOTEID,
		.data	= iface0.aid,
		.len	= 4,
	},
	[1] = {
		.code	= agREPLACESERVER,
		.data	= &subopt0[1].val.ip,
		.len	= 4,
		.val	= { .test = { 10, 11, 12, 13 } },
	}
};

static struct InterfaceInfo const	iface0 = {
	.suboptions	= {
		.dta	= subopt0,
		.len	= sizeof subopt0 / sizeof subopt0[0]
	},
	.aid		= "test",
};

static struct InterfaceInfo const	iface1 = {
	.suboptions	= {
		.dta	= NULL,
		.len	= 0,
	},
};

static uint8_t const		dhcp_msg0[] = {
	0x1, 0,
	0x2, 1, 1,
	0x3, 2, 'a', 'b',
	0xff
};
static uint8_t const		dhcp_msg0_codes[] = { 0x1, 0x2, 0x3, 0xff };

static uint8_t const		dhcp_msg1[] = {
	0x1, 0,
	0x2, 1, 1,
	0x3, 2, 'a', 'b',
	82, 12,
	2, 4, 't', 'e', 's', 't',
	11, 4, 10, 11, 12, 13,
	0xff
};
static uint8_t const		dhcp_msg1_codes[] = { 0x1, 0x2, 0x3, 82, 0xff };

static uint8_t const		dhcp_msg2[] = {
	0x1, 0,
	0x2, 1, 1,
	0x3, 2, 'a', 'b',
	82, 4,
	2, 2, 'x', 'y',
	0xff
};
static uint8_t const		dhcp_msg2_codes[] = { 0x1, 0x2, 0x3, 82, 0xff };

static uint8_t const		dhcp_msg3[] = {
	0x1, 0,
	0x2, 1, 1,
	0x3, 2, 'a', 'b',
	82, 4,
	2, 2, 'x', 'y',
	0x4, 0,
	82, 6,
	2, 4, 't', 'e', 's', 't',
	0x5, 0,
	0xff
};
static uint8_t const		dhcp_msg3_codes[] = { 0x1, 0x2, 0x3, 82, 0x4,
						      82, 0x5, 0xff };

static uint8_t const		dhcp_msg3_0[] = {
	0x1, 0,
	0x2, 1, 1,
	0x3, 2, 'a', 'b',
	0x4, 0,
	0x5, 0,
	0xff
};
static uint8_t const		dhcp_msg3_0_codes[] = { 0x1, 0x2, 0x3, 0x4,
							0x5, 0xff };

static uint8_t const		dhcp_msg3_1[] = {
	0x1, 0,
	0x2, 1, 1,
	0x3, 2, 'a', 'b',
	0x4, 0,
	0x5, 0,
	82, 12,
	2, 4, 't', 'e', 's', 't',
	11, 4, 10, 11, 12, 13,
	0xff
};
static uint8_t const		dhcp_msg3_1_codes[] = { 0x1, 0x2, 0x3, 0x4,
							0x5, 82, 0xff };

static uint8_t const		dhcp_msg4[] = {
	0xff
};
static uint8_t const		dhcp_msg4_codes[] = { 0xff };

static uint8_t const		dhcp_msg4_1[] = {
	82, 12,
	2, 4, 't', 'e', 's', 't',
	11, 4, 10, 11, 12, 13,
	0xff
};
static uint8_t const		dhcp_msg4_1_codes[] = { 82, 0xff };


static uint8_t const		dhcp_msg5[500] = {
	[0] = 0x1, 254,
	[256] = 0x2, 241,
	[499] = 0xff
};
static uint8_t const		dhcp_msg5_codes[] = { 1, 2, 0xff };

static uint8_t const		dhcp_msg6[526] = {
	[0] = 0x1, 254,
	[256] = 0x2, 241,
	[499] = 82, 12,
	2, 4, 't', 'e', 's', 't',
	11, 4, 10, 11, 12, 13,
	0x4, 0,
	82, 6,
	2, 4, 't', 'e', 's', 't',
	0x5, 0,
	0xff
};
static uint8_t const		dhcp_msg6_codes[] = { 1, 2, 82, 4, 82, 5, 0xff };

static uint8_t const		dhcp_msg6_0[504] = {
	[0] = 0x1, 254,
	[256] = 0x2, 241,
	[499] = 0x4, 0,
	0x5, 0,
	0xff
};
static uint8_t const		dhcp_msg6_0_codes[] = { 1, 2, 4, 5, 0xff };


#define TEST(_msg, _iface, _mode, _exp)					\
	do {								\
		unsigned char buf[1024];				\
		memset(buf, 0xfe, sizeof buf);				\
		memcpy(buf+1, _msg, sizeof(_msg));			\
		assert(fillOptions(_iface, buf+1, _mode) == sizeof(_exp)); \
		assert(memcmp(buf+1, _exp, sizeof(_exp)) == 0);		\
		assert(buf[0] == 0xfe);					\
		switch (_mode) {					\
		case acIGNORE:						\
		case acADD_AGENT_INFO:					\
			assert(buf[sizeof(_exp)+1] == 0xfe);		\
			break;						\
		case acREMOVE_AGENT_INFO:				\
			assert(buf[sizeof(_msg)+1] == 0xfe);		\
			break;						\
		}							\
	} while (0)

#define VALIDATE_OPTS(_msg, _exp)					\
	do {								\
		unsigned char			tmpbuf[sizeof(_msg) + 4]; \
		bool res;						\
		tmpbuf[0] = 0x63;					\
		tmpbuf[1] = 0x82;					\
		tmpbuf[2] = 0x53;					\
		tmpbuf[3] = 0x63;					\
		memcpy(tmpbuf+4, _msg, sizeof(_msg));			\
		res = isValidOptions((void const *)(tmpbuf), sizeof tmpbuf); \
		assert(!!res == !!(_exp));				\
	} while (0)

#define VALIDATE(_msg, _codes)						\
	do {								\
		struct DHCPSingleOption	const	*opt = (void const *)(_msg); \
		uint8_t const *c = (_codes);				\
		while (*c != 0xff) {					\
			assert(opt->code == *c);			\
			opt = DHCP_nextSingleOptionConst(opt);		\
			++c;						\
		}							\
		assert(opt->code == 0xff);				\
		assert((uintptr_t)(opt) + 1 - (uintptr_t)(_msg) == sizeof(_msg)); \
	} while (0);							\
	VALIDATE_OPTS(_msg, true)					\

int main()
{
	VALIDATE(dhcp_msg0, dhcp_msg0_codes);
	VALIDATE(dhcp_msg1, dhcp_msg1_codes);
	VALIDATE(dhcp_msg2, dhcp_msg2_codes);
	VALIDATE(dhcp_msg3, dhcp_msg3_codes);
	VALIDATE(dhcp_msg3_0, dhcp_msg3_0_codes);
	VALIDATE(dhcp_msg3_1, dhcp_msg3_1_codes);
	VALIDATE(dhcp_msg4, dhcp_msg4_codes);
	VALIDATE(dhcp_msg4_1, dhcp_msg4_1_codes);
	VALIDATE(dhcp_msg5, dhcp_msg5_codes);
	VALIDATE(dhcp_msg6, dhcp_msg6_codes);
	VALIDATE(dhcp_msg6_0, dhcp_msg6_0_codes);

	/**********/

	TEST(dhcp_msg0, &iface0, acIGNORE, dhcp_msg0);
	TEST(dhcp_msg1, &iface0, acIGNORE, dhcp_msg1);
	TEST(dhcp_msg2, &iface0, acIGNORE, dhcp_msg2);
	TEST(dhcp_msg3, &iface0, acIGNORE, dhcp_msg3);
	TEST(dhcp_msg4, &iface0, acIGNORE, dhcp_msg4);
	TEST(dhcp_msg5, &iface0, acIGNORE, dhcp_msg5);
	TEST(dhcp_msg6, &iface0, acIGNORE, dhcp_msg6);

	TEST(dhcp_msg0, &iface0, acADD_AGENT_INFO, dhcp_msg1);
	TEST(dhcp_msg1, &iface0, acADD_AGENT_INFO, dhcp_msg1);
	TEST(dhcp_msg2, &iface0, acADD_AGENT_INFO, dhcp_msg2);
	TEST(dhcp_msg3, &iface0, acADD_AGENT_INFO, dhcp_msg3);
	TEST(dhcp_msg4, &iface0, acADD_AGENT_INFO, dhcp_msg4_1);
	TEST(dhcp_msg5, &iface0, acADD_AGENT_INFO, dhcp_msg5);
	TEST(dhcp_msg6, &iface0, acADD_AGENT_INFO, dhcp_msg6);

	TEST(dhcp_msg0, &iface0, acREMOVE_AGENT_INFO, dhcp_msg0);
	TEST(dhcp_msg1, &iface0, acREMOVE_AGENT_INFO, dhcp_msg0);
	TEST(dhcp_msg2, &iface0, acREMOVE_AGENT_INFO, dhcp_msg0);
	TEST(dhcp_msg3, &iface0, acREMOVE_AGENT_INFO, dhcp_msg3_0);
	TEST(dhcp_msg4, &iface0, acREMOVE_AGENT_INFO, dhcp_msg4);
	TEST(dhcp_msg5, &iface0, acREMOVE_AGENT_INFO, dhcp_msg5);
	TEST(dhcp_msg6, &iface0, acREMOVE_AGENT_INFO, dhcp_msg6_0);

	TEST(dhcp_msg0, &iface0, acREPLACE_AGENT_INFO, dhcp_msg1);
	TEST(dhcp_msg1, &iface0, acREPLACE_AGENT_INFO, dhcp_msg1);
	TEST(dhcp_msg2, &iface0, acREPLACE_AGENT_INFO, dhcp_msg1);
	TEST(dhcp_msg3, &iface0, acREPLACE_AGENT_INFO, dhcp_msg3_1);
	TEST(dhcp_msg4, &iface0, acREPLACE_AGENT_INFO, dhcp_msg4_1);
	TEST(dhcp_msg5, &iface0, acREPLACE_AGENT_INFO, dhcp_msg5);
	TEST(dhcp_msg6, &iface0, acREPLACE_AGENT_INFO, dhcp_msg6_0);

	/**********/

	TEST(dhcp_msg0, &iface1, acIGNORE, dhcp_msg0);
	TEST(dhcp_msg1, &iface1, acIGNORE, dhcp_msg1);
	TEST(dhcp_msg2, &iface1, acIGNORE, dhcp_msg2);
	TEST(dhcp_msg3, &iface1, acIGNORE, dhcp_msg3);
	TEST(dhcp_msg4, &iface1, acIGNORE, dhcp_msg4);
	TEST(dhcp_msg5, &iface1, acIGNORE, dhcp_msg5);
	TEST(dhcp_msg6, &iface1, acIGNORE, dhcp_msg6);

	TEST(dhcp_msg0, &iface1, acADD_AGENT_INFO, dhcp_msg0);
	TEST(dhcp_msg1, &iface1, acADD_AGENT_INFO, dhcp_msg1);
	TEST(dhcp_msg2, &iface1, acADD_AGENT_INFO, dhcp_msg2);
	TEST(dhcp_msg3, &iface1, acADD_AGENT_INFO, dhcp_msg3);
	TEST(dhcp_msg4, &iface1, acADD_AGENT_INFO, dhcp_msg4);
	TEST(dhcp_msg5, &iface1, acADD_AGENT_INFO, dhcp_msg5);
	TEST(dhcp_msg6, &iface1, acADD_AGENT_INFO, dhcp_msg6);

	TEST(dhcp_msg0, &iface1, acREMOVE_AGENT_INFO, dhcp_msg0);
	TEST(dhcp_msg1, &iface1, acREMOVE_AGENT_INFO, dhcp_msg0);
	TEST(dhcp_msg2, &iface1, acREMOVE_AGENT_INFO, dhcp_msg0);
	TEST(dhcp_msg3, &iface1, acREMOVE_AGENT_INFO, dhcp_msg3_0);
	TEST(dhcp_msg4, &iface1, acREMOVE_AGENT_INFO, dhcp_msg4);
	TEST(dhcp_msg5, &iface1, acREMOVE_AGENT_INFO, dhcp_msg5);
	TEST(dhcp_msg6, &iface1, acREMOVE_AGENT_INFO, dhcp_msg6_0);

	TEST(dhcp_msg0, &iface1, acREPLACE_AGENT_INFO, dhcp_msg0);
	TEST(dhcp_msg1, &iface1, acREPLACE_AGENT_INFO, dhcp_msg0);
	TEST(dhcp_msg2, &iface1, acREPLACE_AGENT_INFO, dhcp_msg0);
	TEST(dhcp_msg3, &iface1, acREPLACE_AGENT_INFO, dhcp_msg3_0);
	TEST(dhcp_msg4, &iface1, acREPLACE_AGENT_INFO, dhcp_msg4);
	TEST(dhcp_msg5, &iface1, acREPLACE_AGENT_INFO, dhcp_msg5);
	TEST(dhcp_msg6, &iface1, acREPLACE_AGENT_INFO, dhcp_msg6_0);

	return 0;
}
