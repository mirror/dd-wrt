/*
 * distribution.h        distribution
 *
 * Copyright (c) 2001-2004 Thomas Graf <tgraf@suug.ch>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef __BMON_DISTRIBUTION_H_
#define __BMON_DISTRIBUTION_H_

#include <bmon/bmon.h>

#define BMON_MAGIC 0xFA
#define BMON_VERSION 0x02

enum {
	BMON_GRP_IF
};

/*
 *                           NODE MESSAGE
 * 
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +---------------+---------------+---------------+---------------+
 * | Magic Number  |    Version    |     Offset    |      Flags    |
 * +---------------+---------------+---------------+---------------+
 * |         Total Length          |           Checksum            |
 * +-------------------------------+-------------------------------+
 * |                      Timestamp (seconds)                      |
 * +---------------------------------------------------------------+
 * |                       Timestamp (usecs)                       |
 * +---------------------------------------------------------------+
 * .                                                               .
 * .                         Name of Node                          .
 * .                                                               .
 * +---------------------------------------------------------------+
 * .                                                               .
 * .                        List of Groups                         .
 * .                                                               .
 * +---------------------------------------------------------------+
 *
 * 
 *                          GROUP MESSAGE
 *  0                   1                   2                   3
 *  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 * +-------------------------------+-------------------------------+
 * |             Type              |             Offset            |
 * +-------------------------------+-------------------------------+
 * .                                                               .
 * .                      Type specific data                       .
 * .                                                               .
 * +---------------------------------------------------------------+
 *
 * 
 *                            ITEM MESSAGE
 * 
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-------------------------------+-------------------------------+
 *  |           Index               |            Offset             |
 *  +---------------+---------------+-------------------------------+
 *  |  Name Length  |  Opts Length  |        Reserved             |C|
 *  +---------------+---------------+-------------------------------+
 *  .                                                               .
 *  .                      Name of Interface                        .
 *  .                                                               .
 *  +---------------------------------------------------------------+
 *  .                                                               .
 *  .                           Options                             .
 *  .                                                               .
 *  +---------------------------------------------------------------+
 *  .                                                               .
 *  .                      List of Attributes                       .
 *  .                                                               .
 *  +---------------------------------------------------------------+
 *
 *                          INTERFACE OPTION
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +---------------+---------------+ - - - - - - - - - - - - - - - +
 *  |     Type      |     Length    |         Data or padding       |
 *  +---------------+---------------+ - - - - - - - - - - - - - - - +
 *  .                                                               .
 *  .                               Data                            .
 *  .                                                               .
 *  + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
 *
 *                     DEFINED INTERFACE OPTIONS
 *
 *          +----------------+--------+---------------------+
 *          | Type           | Length |   Description       |
 *          +----------------+--------+---------------------+
 *          | IFOPT_HANDLE   | 4      | Interface handle    |
 *          | IFOPT_PARENT   | 2      | Parent index        |
 *          | IFOPT_LINK     | 2      | Master link         |
 *          | IFOPT_LEVEL    | 2      | Level (tc trees)    |
 *          | IFOPT_RX_USAGE | 4      | RX Usage            |
 *          | IFOPT_TX_USAGE | 4      | TX Usage            |
 *          +----------------+--------+---------------------+
 *
 * 
 *                          ATTRIBUTE MESSAGE
 *                          
 *   0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-------------------------------+---------------------------+-+-+
 *  |             Type              |                           |R|T|
 *  +-------------------------------+---------------------------+-+-+
 *  |          RX Overflows         |          TX Overflows         |
 *  +-------------------------------+-------------------------------+
 *  |                            RX Value                           |
 *  |                                                               |
 *  +---------------------------------------------------------------+
 *  |                            TX Value                           |
 *  |                                                               |
 *  +---------------------------------------------------------------+
 */

struct distr_msg_hdr
{
	uint8_t    h_magic;
	uint8_t    h_ver;
	uint8_t    h_offset;
	uint8_t    h_flags;
	uint16_t   h_len;
	uint16_t   h_csum;
	uint32_t   h_ts_sec;
	uint32_t   h_ts_usec;
};

struct distr_msg_grp
{
	uint16_t  g_type;
	uint16_t  g_offset;
};

struct distr_msg_item
{	
	uint16_t  i_index;
	uint16_t  i_offset;
	uint8_t   i_namelen;
	uint8_t   i_optslen;
	uint16_t  i_flags;
};

#define IF_IS_CHILD (1<<0)

enum {
	IFOPT_END,
	IFOPT_HANDLE,
	IFOPT_PARENT,
	IFOPT_LINK,
	IFOPT_LEVEL,
	IFOPT_RX_USAGE,
	IFOPT_TX_USAGE,
	IFOPT_DESC,
};

struct distr_msg_ifopt
{
	uint8_t   io_type;
	uint8_t   io_len;
	uint16_t  io_pad;
};

#define ATTR_TX_PROVIDED (1<<0)
#define ATTR_RX_PROVIDED (1<<1)

struct distr_msg_attr
{
	uint16_t  a_type;
	uint16_t  a_flags;
	uint16_t  a_rx_overflows;
	uint16_t  a_tx_overflows;
	uint64_t  a_rx;
	uint64_t  a_tx;
};

#endif
