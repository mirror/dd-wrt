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

#include <nmea/parser.h>

#include <nmea/context.h>
#include <nmea/parse.h>
#include <nmea/sentence.h>
#include <nmea/conversions.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
 * high level
 */

/**
 * Initialise the parser.
 * Allocates a buffer.
 *
 * @param parser a pointer to the parser
 * @return true (1) - success or false (0) - fail
 */
int nmea_parser_init(nmeaPARSER *parser) {
	int resv = 0;
	int buff_size = nmea_context_get_buffer_size();

	assert(parser);

	memset(parser, 0, sizeof(nmeaPARSER));

	if (!(parser->buffer = malloc(buff_size)))
		nmea_error("nmea_parser_init: insufficient memory");
	else {
		parser->buff_size = buff_size;
		resv = 1;
	}

	return resv;
}

/**
 * Destroy the parser.
 * Frees a buffer.
 *
 * @param parser a pointer to the parser
 */
void nmea_parser_destroy(nmeaPARSER *parser) {
	assert(parser);

	if (parser->buffer) {
		free(parser->buffer);
		parser->buffer = NULL;
	}
	nmea_parser_queue_clear(parser);
	memset(parser, 0, sizeof(nmeaPARSER));
}

/**
 * Parse a string and store the results in the nmeaINFO structure
 *
 * @param parser a pointer to the parser
 * @param s the string
 * @param len the length of the string
 * @param info a pointer to the nmeaINFO structure
 * @return the number of packets that were parsed
 */
int nmea_parse(nmeaPARSER *parser, const char *s, const int len, nmeaINFO *info) {
	int packetType;
	int packetsParsed = 0;
	void *packet = 0;

	assert(parser);

	nmea_parser_push(parser, s, len);

	while (GPNON != (packetType = nmea_parser_pop(parser, &packet))) {
		packetsParsed++;

		switch (packetType) {
		case GPGGA:
			nmea_GPGGA2info((nmeaGPGGA *) packet, info);
			break;
		case GPGSA:
			nmea_GPGSA2info((nmeaGPGSA *) packet, info);
			break;
		case GPGSV:
			nmea_GPGSV2info((nmeaGPGSV *) packet, info);
			break;
		case GPRMC:
			nmea_GPRMC2info((nmeaGPRMC *) packet, info);
			break;
		case GPVTG:
			nmea_GPVTG2info((nmeaGPVTG *) packet, info);
			break;
		default:
			break;
		};

		free(packet);
	}

	return packetsParsed;
}

/*
 * low level
 */

/**
 * Do the actual parsing of a string and store the results in the parser.
 * This function is used to parse (broken up parts) of a complete string.
 *
 * @param parser a pointer to the parser
 * @param s the string
 * @param len the length of the string
 * @return the number of bytes that were parsed, -1 on error
 */
static int nmea_parser_real_push(nmeaPARSER *parser, const char *s, int len) {
	int charsParsed = 0;
	int crc;
	int sentenceLength;
	int sentenceType;
	nmeaParserNODE *node = NULL;

	assert(parser);
	assert(parser->buffer);

	if (!s || !len)
		return 0;

	/* clear the buffer if the string is too large */
	if ((parser->buff_use + len) >= parser->buff_size)
		nmea_parser_buff_clear(parser);

	/* check that the string will fit in the buffer */
	if ((parser->buff_use + len) >= parser->buff_size) {
		nmea_error("nmea_parser_real_push: string too long to fit in parser buffer");
		return 0;
	}

	/* put the string in the buffer */
	memcpy(parser->buffer + parser->buff_use, s, len);
	parser->buff_use += len;

	/* parse */
	for (;; node = NULL) {
		sentenceLength = nmea_parse_get_sentence_length(parser->buffer + charsParsed, parser->buff_use - charsParsed,
				&crc);

		if (!sentenceLength) {
			if (charsParsed)
				memmove(parser->buffer, parser->buffer + charsParsed, parser->buff_use -= charsParsed);
			break;
		} else if (crc >= 0) {
			sentenceType = nmea_parse_get_sentence_type(parser->buffer + charsParsed + 1,
					parser->buff_use - charsParsed - 1);

			if (!(node = malloc(sizeof(nmeaParserNODE))))
				goto mem_fail;

			node->pack = NULL;

			switch (sentenceType) {
			case GPGGA:
				if (!(node->pack = malloc(sizeof(nmeaGPGGA))))
					goto mem_fail;
				node->packType = GPGGA;
				if (!nmea_parse_GPGGA(parser->buffer + charsParsed, sentenceLength, (nmeaGPGGA *) node->pack)) {
					free(node->pack);
					free(node);
					node = NULL;
				}
				break;
			case GPGSA:
				if (!(node->pack = malloc(sizeof(nmeaGPGSA))))
					goto mem_fail;
				node->packType = GPGSA;
				if (!nmea_parse_GPGSA(parser->buffer + charsParsed, sentenceLength, (nmeaGPGSA *) node->pack)) {
					free(node->pack);
					free(node);
					node = NULL;
				}
				break;
			case GPGSV:
				if (!(node->pack = malloc(sizeof(nmeaGPGSV))))
					goto mem_fail;
				node->packType = GPGSV;
				if (!nmea_parse_GPGSV(parser->buffer + charsParsed, sentenceLength, (nmeaGPGSV *) node->pack)) {
					free(node->pack);
					free(node);
					node = NULL;
				}
				break;
			case GPRMC:
				if (!(node->pack = malloc(sizeof(nmeaGPRMC))))
					goto mem_fail;
				node->packType = GPRMC;
				if (!nmea_parse_GPRMC(parser->buffer + charsParsed, sentenceLength, (nmeaGPRMC *) node->pack)) {
					free(node->pack);
					free(node);
					node = NULL;
				}
				break;
			case GPVTG:
				if (!(node->pack = malloc(sizeof(nmeaGPVTG))))
					goto mem_fail;
				node->packType = GPVTG;
				if (!nmea_parse_GPVTG(parser->buffer + charsParsed, sentenceLength, (nmeaGPVTG *) node->pack)) {
					free(node->pack);
					free(node);
					node = NULL;
				}
				break;
			default:
				free(node);
				node = NULL;
				break;
			};

			if (node) {
				if (parser->end_node)
					parser->end_node->next_node = node;
				parser->end_node = node;
				if (!parser->top_node)
					parser->top_node = node;
				node->next_node = NULL;
			}
		}

		charsParsed += sentenceLength;
	}

	return charsParsed;

	mem_fail: if (node)
		free(node);
	nmea_error("Insufficient memory!");

	return -1;
}

/**
 * Parse a string and store the results in the parser
 *
 * @param parser a pointer to the parser
 * @param s the string
 * @param len the length of the string
 * @return the number of bytes that were parsed
 */
int nmea_parser_push(nmeaPARSER *parser, const char *s, int len) {
	int charsParsed = 0;

	assert(parser);

	if (!s || !len)
		return 0;

	do {
		int charsToParse;

		if (len > parser->buff_size)
			charsToParse = parser->buff_size;
		else
			charsToParse = len;

		charsParsed += nmea_parser_real_push(parser, s, charsToParse);
		len -= charsToParse;
	} while (len);

	return charsParsed;
}

/**
 * Get the type of top packet
 *
 * @param parser a pointer to the parser
 * @return the type of the top packet (see nmeaPACKTYPE)
 */
int nmea_parser_top(const nmeaPARSER *parser) {
	int retval = GPNON;
	nmeaParserNODE *node;

	assert(parser);

	node = parser->top_node;
	if (node)
		retval = node->packType;

	return retval;
}

/**
 * Remove the top packet from the parser
 *
 * @param parser a pointer to the parser
 * @param pack_ptr a pointer to the location where to store a pointer to the packet
 * @return the type of the top packet (see nmeaPACKTYPE)
 */
int nmea_parser_pop(nmeaPARSER *parser, void **pack_ptr) {
	int retval = GPNON;
	nmeaParserNODE *node;

	assert(parser);

	node = parser->top_node;
	if (node) {
		retval = node->packType;
		if (pack_ptr)
			*pack_ptr = node->pack;
		parser->top_node = node->next_node;
		if (!parser->top_node)
			parser->end_node = 0;
		free(node);
	}

	return retval;
}

/**
 * Get the top packet from the parser without removing it
 *
 * @param parser a pointer to the parser
 * @param pack_ptr a pointer to the location where to store a pointer to the packet
 * @return the type of the top packet (see nmeaPACKTYPE)
 */
int nmea_parser_peek(const nmeaPARSER *parser, void **pack_ptr) {
	int retval = GPNON;
	nmeaParserNODE *node;

	assert(parser);

	node = parser->top_node;
	if (node) {
		retval = node->packType;
		if (pack_ptr)
			*pack_ptr = node->pack;
	}

	return retval;
}

/**
 * Remove the top packet from the parser
 *
 * @param parser a pointer to the parser
 * @return the type of the removed packet (see nmeaPACKTYPE)
 */
int nmea_parser_drop(nmeaPARSER *parser) {
	int retval = GPNON;
	nmeaParserNODE *node;

	assert(parser);

	node = parser->top_node;
	if (node) {
		retval = node->packType;
		if (node->pack)
			free(node->pack);
		parser->top_node = node->next_node;
		if (!parser->top_node)
			parser->end_node = NULL;
		free(node);
	}

	return retval;
}

/**
 * Clear the cache of the parser
 *
 * @param parser a pointer to the parser
 */
void nmea_parser_buff_clear(nmeaPARSER *parser) {
	assert(parser);
	parser->buff_use = 0;
}

/**
 * Clear the packets queue in the parser
 *
 * @param parser a pointer to the parser
 */
void nmea_parser_queue_clear(nmeaPARSER *parser) {
	assert(parser);
	while (parser->top_node)
		nmea_parser_drop(parser);
}
