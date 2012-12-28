/*
 * This file is part of nmealib.
 *
 * Copyright (c) 2008 Timur Sinitsyn
 * Copyright (c) 2011 Ferry Huberts
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __NMEA_PARSER_H__
#define __NMEA_PARSER_H__

#include <nmea/info.h>

#ifdef  __cplusplus
extern "C" {
#endif

/**
 * Description of a parser node / packet
 */
typedef struct _nmeaParserNODE {
	int packType;						/**< the type of the packet (see nmeaPACKTYPE) */
	void *pack;                         /**< the packet (a pointer to a malloced sentence sctucture) */
	struct _nmeaParserNODE *next_node;  /**< pointer to the next node / packet */
} nmeaParserNODE;

/**
 * The parser data.
 */
typedef struct _nmeaPARSER {
	nmeaParserNODE *top_node; /**< the first node / packet */
	nmeaParserNODE *end_node; /**< the last node / packet */
	char *buffer;             /**< the buffer containing the string to parse */
	int buff_size;            /**< the size of the buffer */
	int buff_use;             /**< the number of bytes in the buffer */
} nmeaPARSER;

int nmea_parser_init(nmeaPARSER *parser);
void nmea_parser_destroy(nmeaPARSER *parser);
int nmea_parse(nmeaPARSER *parser, const char *buff, const int buff_sz, nmeaINFO *info);

int nmea_parser_push(nmeaPARSER *parser, const char *buff, int buff_sz);
int nmea_parser_top(const nmeaPARSER *parser);
int nmea_parser_pop(nmeaPARSER *parser, void **pack_ptr);
int nmea_parser_peek(const nmeaPARSER *parser, void **pack_ptr);
int nmea_parser_drop(nmeaPARSER *parser);
void nmea_parser_buff_clear(nmeaPARSER *parser);
void nmea_parser_queue_clear(nmeaPARSER *parser);

#ifdef  __cplusplus
}
#endif

#endif /* __NMEA_PARSER_H__ */
