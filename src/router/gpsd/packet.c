/* $Id: packet.c 4103 2006-12-07 15:46:44Z esr $ */
/****************************************************************************

NAME:
   packet.c -- a packet-sniffing engine for reading from GPS devices

DESCRIPTION:

Initial conditions of the problem:

1. We have a file descriptor open for (possibly non-blocking) read. The device 
   on the other end is sending packets at us.  

2. It may require more than one read to gather a packet.  Reads may span packet
   boundaries.
  
3. There may be leading garbage before the first packet.  After the first
   start-of-packet, the input should be well-formed.

The problem: how do we recognize which kind of packet we're getting?

No need to handle Garmin USB binary, we know that type by the fact we're 
connected to the Garmin kernel driver.  But we need to be able to tell the 
others apart and distinguish them from baud barf.

***************************************************************************/
#include <sys/types.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "gpsd_config.h"
#include "gpsd.h"

/* 
 * The packet-recognition state machine.  It can be fooled by garbage
 * that looks like the head of a binary packet followed by a NMEA
 * packet; in that case it won't reset until it notices that the
 * binary trailer is not where it should be, and the NMEA packet will
 * be lost.  The reverse scenario is not possible because none of the
 * binary leader characters can occur in an NMEA packet.  Caller should
 * consume a packet when it sees one of the *_RECOGNIZED states.
 * It's good practice to follow the _RECOGNIZED transition with one
 * that recognizes a leader of the same packet type rather than
 * dropping back to ground state -- this for example will prevent
 * the state machine from hopping between recognizing TSIP and
 * EverMore packets that both start with a DLE.
 *
 * Error handling is brutally simple; any time we see an unexpected
 * character, go to GROUND_STATE and reset the machine (except that a
 * $ in an NMEA payload only resets back to NMEA_DOLLAR state).  Because
 * another good packet will be usually along in less than a second
 * repeating the same data, Boyer-Moore-like attempts to do parallel
 * recognition beyond the headers would make no sense in this
 * application, they'd just add complexity.
 *
 * This state machine allows the following talker IDs:
 *      GP -- Global Positioning System.
 *      II -- Integrated Instrumentation (Raytheon's SeaTalk system).
 *	IN -- Integrated Navigation (Garmin uses this).
 *
 */

enum {
#include "packet_states.h"
};

#define DLE	0x10
#define STX	0x02
#define ETX	0x03

static void nextstate(struct gps_packet_t *lexer, 
		      unsigned char c)
{
#ifdef RTCM104_ENABLE
    enum isgpsstat_t	isgpsstat;    
#endif /* RTCM104_ENABLE */
/*@ +charint */
    switch(lexer->state)
    {
    case GROUND_STATE:
	if (c == '#') {
	    lexer->state = COMMENT_BODY;
	    break;
	}
#ifdef NMEA_ENABLE
	if (c == '$') {
	    lexer->state = NMEA_DOLLAR;
	    break;
	}
#endif /* NMEA_ENABLE */
#ifdef TNT_ENABLE
        if (c == '@') {
	    lexer->state = TNT_LEADER;
	    break;
	}
#endif
#ifdef SIRF_ENABLE
        if (c == 0xa0) {
	    lexer->state = SIRF_LEADER_1;
	    break;
	}
#endif /* SIRF_ENABLE */
#if defined(TSIP_ENABLE) || defined(EVERMORE_ENABLE) || defined(GARMIN_ENABLE)
        if (c == DLE) {
	    lexer->state = DLE_LEADER;
	    break;
	}
#endif /* TSIP_ENABLE || EVERMORE_ENABLE || GARMIN_ENABLE */
#ifdef TRIPMATE_ENABLE
        if (c == 'A') {
#ifdef RTCM104_ENABLE
	    if (rtcm_decode(lexer, c) == ISGPS_MESSAGE) {
		lexer->state = RTCM_RECOGNIZED;
		break;
	    }
#endif /* RTCM104_ENABLE */
	    lexer->state = ASTRAL_1;
	    break;
	}
#endif /* TRIPMATE_ENABLE */
#ifdef EARTHMATE_ENABLE
        if (c == 'E') {
#ifdef RTCM104_ENABLE
	    if (rtcm_decode(lexer, c) == ISGPS_MESSAGE) {
		lexer->state = RTCM_RECOGNIZED;
		break;
	    }
#endif /* RTCM104_ENABLE */
	    lexer->state = EARTHA_1;
	    break;
	}
#endif /* EARTHMATE_ENABLE */
#ifdef ZODIAC_ENABLE
	if (c == 0xff) {
	    lexer->state = ZODIAC_LEADER_1;
	    break;
	}
#endif /* ZODIAC_ENABLE */
#ifdef ITALK_ENABLE
	if (c == '<') {
	    lexer->state = ITALK_LEADER_1;
	    break;
	}
#endif /* ITALK_ENABLE */
#ifdef RTCM104_ENABLE
	if ((isgpsstat = rtcm_decode(lexer, c)) == ISGPS_SYNC) {
	    lexer->state = RTCM_SYNC_STATE;
	    break;
	} else if (isgpsstat == ISGPS_MESSAGE) {
	    lexer->state = RTCM_RECOGNIZED;
	    break;
	}
#endif /* RTCM104_ENABLE */
	break;
	/*@ +casebreak @*/
    case COMMENT_BODY:
	if (c == '\n')
	    lexer->state = COMMENT_RECOGNIZED;
	else if (!isprint(c))
	    lexer->state = GROUND_STATE;
	break;	    
#ifdef NMEA_ENABLE
    case NMEA_DOLLAR:
	if (c == 'G')
	    lexer->state = NMEA_PUB_LEAD;
	else if (c == 'P')	/* vendor sentence */
	    lexer->state = NMEA_LEADER_END;
	else if (c =='I')	/* Seatalk */
	    lexer->state = SEATALK_LEAD_1;
	else if (c =='A')	/* SiRF Ack */
	    lexer->state = SIRF_ACK_LEAD_1;
	else
	    lexer->state = GROUND_STATE;
	break;
    case NMEA_PUB_LEAD:
	if (c == 'P')
	    lexer->state = NMEA_LEADER_END;
	else
	    lexer->state = GROUND_STATE;
	break;
#ifdef TNT_ENABLE
    case TNT_LEADER:
          lexer->state = NMEA_LEADER_END;
        break;
#endif
    case NMEA_LEADER_END:
	if (c == '\r')
	    lexer->state = NMEA_CR;
	else if (c == '\n')
	    /* not strictly correct, but helps for interpreting logfiles */
	    lexer->state = NMEA_RECOGNIZED;
	else if (c == '$')
	    /* faster recovery from missing sentence trailers */
	    lexer->state = NMEA_DOLLAR;
	else if (!isprint(c))
	    lexer->state = GROUND_STATE;
	break;
    case NMEA_CR:
	if (c == '\n')
	    lexer->state = NMEA_RECOGNIZED;
	else
	    lexer->state = GROUND_STATE;
	break;
    case NMEA_RECOGNIZED:
	if (c == '$')
	    lexer->state = NMEA_DOLLAR;
	else
	    lexer->state = GROUND_STATE;
	break;
    case SEATALK_LEAD_1:
	if (c == 'I' || c == 'N')	/* II or IN are accepted */
	    lexer->state = NMEA_LEADER_END;
	else
	    lexer->state = GROUND_STATE;
	break;
#ifdef TRIPMATE_ENABLE
    case ASTRAL_1:
	if (c == 'S') {
#ifdef RTCM104_ENABLE
	    if (rtcm_decode(lexer, c) == ISGPS_MESSAGE) {
		lexer->state = RTCM_RECOGNIZED;
		break;
	    }
#endif /* RTCM104_ENABLE */
	    lexer->state = ASTRAL_2;
	} else
	    lexer->state = GROUND_STATE;
	break;
    case ASTRAL_2:
	if (c == 'T') {
#ifdef RTCM104_ENABLE
	    if (rtcm_decode(lexer, c) == ISGPS_MESSAGE) {
		lexer->state = RTCM_RECOGNIZED;
		break;
	    }
#endif /* RTCM104_ENABLE */
	    lexer->state = ASTRAL_3;
	} else
	    lexer->state = GROUND_STATE;
	break;
    case ASTRAL_3:
	if (c == 'R') {
#ifdef RTCM104_ENABLE
	    if (rtcm_decode(lexer, c) == ISGPS_MESSAGE) {
		lexer->state = RTCM_RECOGNIZED;
		break;
	    }
#endif /* RTCM104_ENABLE */
	    lexer->state = ASTRAL_5;
	} else
	    lexer->state = GROUND_STATE;
	break;
    case ASTRAL_4:
	if (c == 'A') {
#ifdef RTCM104_ENABLE
	    if (rtcm_decode(lexer, c) == ISGPS_MESSAGE) {
		lexer->state = RTCM_RECOGNIZED;
		break;
	    }
#endif /* RTCM104_ENABLE */
	    lexer->state = ASTRAL_2;
	} else
	    lexer->state = GROUND_STATE;
	break;
    case ASTRAL_5:
	if (c == 'L') {
#ifdef RTCM104_ENABLE
	    if (rtcm_decode(lexer, c) == ISGPS_MESSAGE) {
		lexer->state = RTCM_RECOGNIZED;
		break;
	    }
#endif /* RTCM104_ENABLE */
	    lexer->state = NMEA_RECOGNIZED;
	} else
	    lexer->state = GROUND_STATE;
	break;
#endif /* TRIPMATE_ENABLE */
#ifdef EARTHMATE_ENABLE
    case EARTHA_1:
	if (c == 'A') {
#ifdef RTCM104_ENABLE
	    if (rtcm_decode(lexer, c) == ISGPS_MESSAGE) {
		lexer->state = RTCM_RECOGNIZED;
		break;
	    }
#endif /* RTCM104_ENABLE */
	    lexer->state = EARTHA_2;
	} else
	    lexer->state = GROUND_STATE;
	break;
    case EARTHA_2:
	if (c == 'R') {
#ifdef RTCM104_ENABLE
	    if (rtcm_decode(lexer, c) == ISGPS_MESSAGE) {
		lexer->state = RTCM_RECOGNIZED;
		break;
	    }
#endif /* RTCM104_ENABLE */
	    lexer->state = EARTHA_3;
	} else
	    lexer->state = GROUND_STATE;
	break;
    case EARTHA_3:
	if (c == 'T') {
#ifdef RTCM104_ENABLE
	    if (rtcm_decode(lexer, c) == ISGPS_MESSAGE) {
		lexer->state = RTCM_RECOGNIZED;
		break;
	    }
#endif /* RTCM104_ENABLE */
	    lexer->state = EARTHA_4;
	} else
	    lexer->state = GROUND_STATE;
	break;
    case EARTHA_4:
	if (c == 'H') {
#ifdef RTCM104_ENABLE
	    if (rtcm_decode(lexer, c) == ISGPS_MESSAGE) {
		lexer->state = RTCM_RECOGNIZED;
		break;
	    }
#endif /* RTCM104_ENABLE */
	    lexer->state = EARTHA_5;
	} else
	    lexer->state = GROUND_STATE;
	break;
    case EARTHA_5:
	if (c == 'A') {
#ifdef RTCM104_ENABLE
	    if (rtcm_decode(lexer, c) == ISGPS_MESSAGE) {
		lexer->state = RTCM_RECOGNIZED;
		break;
	    }
#endif /* RTCM104_ENABLE */
	    lexer->state = NMEA_RECOGNIZED;
	} else
	    lexer->state = GROUND_STATE;
	break; 
#endif /* EARTHMATE_ENABLE */
    case SIRF_ACK_LEAD_1:
	if (c == 'c')
	    lexer->state = SIRF_ACK_LEAD_2;
	else
	    lexer->state = GROUND_STATE;
	break;
   case SIRF_ACK_LEAD_2:
	if (c == 'k')
	    lexer->state = NMEA_LEADER_END;
	else
	    lexer->state = GROUND_STATE;
	break;
#endif /* NMEA_ENABLE */
#ifdef SIRF_ENABLE
    case SIRF_LEADER_1:
	if (c == 0xa2)
	    lexer->state = SIRF_LEADER_2;
	else
	    lexer->state = GROUND_STATE;
	break;
    case SIRF_LEADER_2:
	lexer->length = (size_t)(c << 8);
	lexer->state = SIRF_LENGTH_1;
	break;
    case SIRF_LENGTH_1:
	lexer->length += c + 2;
	if (lexer->length <= MAX_PACKET_LENGTH)
	    lexer->state = SIRF_PAYLOAD;
	else
	    lexer->state = GROUND_STATE;
	break;
    case SIRF_PAYLOAD:
	if (--lexer->length == 0)
	    lexer->state = SIRF_DELIVERED;
	break;
    case SIRF_DELIVERED:
	if (c == 0xb0)
	    lexer->state = SIRF_TRAILER_1;
	else
	    lexer->state = GROUND_STATE;
	break;
    case SIRF_TRAILER_1:
	if (c == 0xb3)
	    lexer->state = SIRF_RECOGNIZED;
	else
	    lexer->state = GROUND_STATE;
	break;
    case SIRF_RECOGNIZED:
        if (c == 0xa0)
	    lexer->state = SIRF_LEADER_1;
	else
	    lexer->state = GROUND_STATE;
	break;
#endif /* SIRF_ENABLE */
#if defined(TSIP_ENABLE) || defined(EVERMORE_ENABLE) || defined(GARMIN_ENABLE)
    case DLE_LEADER:
#ifdef EVERMORE_ENABLE
	if (c == STX)
	    lexer->state = EVERMORE_LEADER_2;
	else
#endif /* EVERMORE_ENABLE */
#if defined(TSIP_ENABLE) || defined(GARMIN_ENABLE)
	/* garmin is special case of TSIP */
	/* check last because there's no checksum */
	if (c >= 0x13)
	    lexer->state = TSIP_PAYLOAD;
	else
#endif /* TSIP_ENABLE */
	    lexer->state = GROUND_STATE;
	break;
#endif /* TSIP_ENABLE || EVERMORE_ENABLE || GARMIN_ENABLE */
#ifdef ZODIAC_ENABLE
    case ZODIAC_EXPECTED:
    case ZODIAC_RECOGNIZED:
	if (c == 0xff)
	    lexer->state = ZODIAC_LEADER_1;
	else
	    lexer->state = GROUND_STATE;
	break;
    case ZODIAC_LEADER_1:
	if (c == 0x81)
	    lexer->state = ZODIAC_LEADER_2;
	else
	    lexer->state = GROUND_STATE;
	break;
    case ZODIAC_LEADER_2:
	lexer->state = ZODIAC_ID_1;
	break;
    case ZODIAC_ID_1:
	lexer->state = ZODIAC_ID_2;
	break;
    case ZODIAC_ID_2:
	lexer->length = (size_t)c;
	lexer->state = ZODIAC_LENGTH_1;
	break;
    case ZODIAC_LENGTH_1:
	lexer->length += (c << 8);
	lexer->state = ZODIAC_LENGTH_2;
	break;
    case ZODIAC_LENGTH_2:
	lexer->state = ZODIAC_FLAGS_1;
	break;
    case ZODIAC_FLAGS_1:
	lexer->state = ZODIAC_FLAGS_2;
	break;
    case ZODIAC_FLAGS_2:
	lexer->state = ZODIAC_HSUM_1;
	break;
    case ZODIAC_HSUM_1:
	{
 #define getword(i) (short)(lexer->inbuffer[2*(i)] | (lexer->inbuffer[2*(i)+1] << 8))
	    short sum = getword(0) + getword(1) + getword(2) + getword(3);
	    sum *= -1;
	    if (sum != getword(4)) {
		gpsd_report(LOG_IO, "Zodiac Header checksum 0x%hx expecting 0x%hx\n", 
		       sum, getword(4));
		lexer->state = GROUND_STATE;
		break;
	    }
	}
	gpsd_report(LOG_RAW+1,"Zodiac header id=%hd len=%hd flags=%hx\n", getword(1), getword(2), getword(3));
 #undef getword
	if (lexer->length == 0) {
	    lexer->state = ZODIAC_RECOGNIZED;
	    break;
	}
	lexer->length *= 2;		/* word count to byte count */
	lexer->length += 2;		/* checksum */
	/* 10 bytes is the length of the Zodiac header */
	if (lexer->length <= MAX_PACKET_LENGTH - 10)
	    lexer->state = ZODIAC_PAYLOAD;
	else
	    lexer->state = GROUND_STATE;
	break;
    case ZODIAC_PAYLOAD:
	if (--lexer->length == 0)
	    lexer->state = ZODIAC_RECOGNIZED;
	break;
#endif /* ZODIAC_ENABLE */
#ifdef EVERMORE_ENABLE
    case EVERMORE_LEADER_1:
	if (c == STX)
	    lexer->state = EVERMORE_LEADER_2;
	else
	    lexer->state = GROUND_STATE;
	break;
    case EVERMORE_LEADER_2:
	lexer->length = (size_t)c;
	if (c == DLE)
	    lexer->state = EVERMORE_PAYLOAD_DLE;
	else
	    lexer->state = EVERMORE_PAYLOAD;
	break;
    case EVERMORE_PAYLOAD:
	if (c == DLE)
	    lexer->state = EVERMORE_PAYLOAD_DLE;
	else if (--lexer->length == 0)
	    lexer->state = GROUND_STATE;
	break;
    case EVERMORE_PAYLOAD_DLE:
        switch (c) {
           case DLE: lexer->state = EVERMORE_PAYLOAD; break;
           case ETX: lexer->state = EVERMORE_RECOGNIZED; break;
           default: lexer->state = GROUND_STATE;
        }
    break;
    case EVERMORE_RECOGNIZED:
        if (c == DLE)
	    lexer->state = EVERMORE_LEADER_1;
	else
	    lexer->state = GROUND_STATE;
	break;
#endif /* EVERMORE_ENABLE */
#ifdef ITALK_ENABLE
    case ITALK_LEADER_1:
        if (c == '!')
	    lexer->state = ITALK_LEADER_2;
	else
	    lexer->state = GROUND_STATE;
	break;
    case ITALK_LEADER_2:
	lexer->length = (size_t)(c & 0xff);
	lexer->state = ITALK_LENGTH;
	break;
    case ITALK_LENGTH:
	lexer->length += 1;	/* fix number of words in payload */
	lexer->length *= 2;	/* convert to number of bytes */
	lexer->state = ITALK_PAYLOAD;
	break;
    case ITALK_PAYLOAD:
	/* lookahead for "<!" because sometimes packets are short but valid */
	if ((c == '>') && (lexer->inbufptr[0] == '<') && (lexer->inbufptr[1] == '!'))
	    lexer->state = ITALK_RECOGNIZED;
	else if (--lexer->length == 0)
	    lexer->state = ITALK_DELIVERED;
	break;
    case ITALK_DELIVERED:
	if (c == '>')
	    lexer->state = ITALK_RECOGNIZED;
	else
	    lexer->state = GROUND_STATE;
	break;
    case ITALK_RECOGNIZED:
        if (c == '<')
	    lexer->state = ITALK_LEADER_1;
	else
	    lexer->state = GROUND_STATE;
	break;
#endif /* ITALK_ENABLE */
#ifdef TSIP_ENABLE
    case TSIP_LEADER:
        /* unused case */
	if (c >= 0x13)
	    lexer->state = TSIP_PAYLOAD;
	else
	    lexer->state = GROUND_STATE;
	break;
    case TSIP_PAYLOAD:
	if (c == DLE)
	    lexer->state = TSIP_DLE;
	break;
    case TSIP_DLE:
	switch (c)
	{
	case ETX:
	    lexer->state = TSIP_RECOGNIZED;
	    break;
	case DLE:
	    lexer->state = TSIP_PAYLOAD;
	    break;
	default:
	    lexer->state = GROUND_STATE;
	    break;
	}
	break;
    case TSIP_RECOGNIZED:
        if (c == DLE)
	    /*
	     * Don't go to TSIP_LEADER state -- TSIP packets aren't
	     * checksummed, so false positives are easy.  We might be
	     * looking at another DLE-stuffed protocol like EverMore
             * or Garmin streaming binary.
	     */
	    lexer->state = DLE_LEADER;
	else
	    lexer->state = GROUND_STATE;
	break;
#endif /* TSIP_ENABLE */
#ifdef RTCM104_ENABLE
    case RTCM_SYNC_STATE:
    case RTCM_SKIP_STATE:
	if ((isgpsstat = rtcm_decode(lexer, c)) == ISGPS_MESSAGE) {
	    lexer->state = RTCM_RECOGNIZED;
	    break;
	} else if (isgpsstat == ISGPS_NO_SYNC)
	    lexer->state = GROUND_STATE;
	break;

    case RTCM_RECOGNIZED:
	if (rtcm_decode(lexer, c) == ISGPS_SYNC) {
	    lexer->state = RTCM_SYNC_STATE;
	    break;
	} else
	    lexer->state = GROUND_STATE;
	break;
#endif /* RTCM104_ENABLE */
    }
/*@ -charint */
}

#define STATE_DEBUG

static void packet_accept(struct gps_packet_t *lexer, int packet_type)
/* packet grab succeeded, move to output buffer */
{
    size_t packetlen = lexer->inbufptr-lexer->inbuffer;
    if (packetlen < sizeof(lexer->outbuffer)) {
	memcpy(lexer->outbuffer, lexer->inbuffer, packetlen);
	lexer->outbuflen = packetlen;
	lexer->outbuffer[packetlen] = '\0';
	lexer->type = packet_type;
#ifdef STATE_DEBUG
	gpsd_report(LOG_RAW+1, "Packet type %d accepted %d = %s\n",
		packet_type, packetlen,
		gpsd_hexdump(lexer->outbuffer, lexer->outbuflen));
#endif /* STATE_DEBUG */
    } else {
	gpsd_report(LOG_ERROR, "Rejected too long packet type %d len %d\n",
		packet_type,packetlen);
    }
}

static void packet_discard(struct gps_packet_t *lexer)
/* shift the input buffer to discard all data up to current input pointer */
{
    size_t discard = lexer->inbufptr - lexer->inbuffer;
    size_t remaining = lexer->inbuflen - discard;
    lexer->inbufptr = memmove(lexer->inbuffer,
				lexer->inbufptr,
				remaining);
    lexer->inbuflen = remaining;
#ifdef STATE_DEBUG
    gpsd_report(LOG_RAW+1, "Packet discard of %d, chars remaining is %d = %s\n",
		discard, remaining,
		gpsd_hexdump(lexer->inbuffer, lexer->inbuflen));
#endif /* STATE_DEBUG */
}

static void character_discard(struct gps_packet_t *lexer)
/* shift the input buffer to discard one character and reread data */
{
    memmove(lexer->inbuffer, lexer->inbuffer+1, (size_t)--lexer->inbuflen);
    lexer->inbufptr = lexer->inbuffer;
#ifdef STATE_DEBUG
    gpsd_report(LOG_RAW+1, "Character discarded, buffer %d chars = %s\n",
		lexer->inbuflen,
		gpsd_hexdump(lexer->inbuffer, lexer->inbuflen));
#endif /* STATE_DEBUG */
}


/* get 0-origin big-endian words relative to start of packet buffer */
#define getword(i) (short)(lexer->inbuffer[2*(i)] | (lexer->inbuffer[2*(i)+1] << 8))

/* entry points begin here */

ssize_t packet_parse(struct gps_packet_t *lexer, size_t fix)
/* grab a packet; returns either BAD_PACKET or the length */
{
#ifdef STATE_DEBUG
    gpsd_report(LOG_RAW+1, "Read %d chars to buffer offset %d (total %d): %s\n",
		fix,
		lexer->inbuflen,
		lexer->inbuflen+fix,
		gpsd_hexdump(lexer->inbufptr, fix));
#endif /* STATE_DEBUG */

    lexer->outbuflen = 0;
    lexer->inbuflen += fix;
    while (lexer->inbufptr < lexer->inbuffer + lexer->inbuflen) {
	/*@ -modobserver @*/
	unsigned char c = *lexer->inbufptr++;
	/*@ +modobserver @*/
	char *state_table[] = {
#include "packet_names.h"
	};
	nextstate(lexer, c);
	gpsd_report(LOG_RAW+2, "%08ld: character '%c' [%02x], new state: %s\n",
		    lexer->char_counter, 
		    (isprint(c)?c:'.'), 
		    c, 
		    state_table[lexer->state]);
	lexer->char_counter++;

	if (lexer->state == GROUND_STATE) {
	    character_discard(lexer);
	}
	else if (lexer->state == COMMENT_RECOGNIZED) {
	    packet_accept(lexer, COMMENT_PACKET);
	    packet_discard(lexer);
	    lexer->state = GROUND_STATE;
	    break;
	}
#ifdef NMEA_ENABLE
	else if (lexer->state == NMEA_RECOGNIZED) {
	    bool checksum_ok = true;
	    char csum[3];
	    char *trailer = (char *)lexer->inbufptr-5;
	    if (*trailer == '*') {
		unsigned int n, crc = 0;
		for (n = 1; (char *)lexer->inbuffer + n < trailer; n++)
		    crc ^= lexer->inbuffer[n];
		(void)snprintf(csum, sizeof(csum), "%02X", crc);
		checksum_ok = (csum[0]==toupper(trailer[1])
				&& csum[1]==toupper(trailer[2]));
	    }
	    if (checksum_ok)
		packet_accept(lexer, NMEA_PACKET);
	    else
		lexer->state = GROUND_STATE;
	    packet_discard(lexer);
            break;
	}
#endif /* NMEA_ENABLE */
#ifdef SIRF_ENABLE
	else if (lexer->state == SIRF_RECOGNIZED) {
	    unsigned char *trailer = lexer->inbufptr-4;
	    unsigned int checksum = (unsigned)((trailer[0] << 8) | trailer[1]);
	    unsigned int n, crc = 0;
	    for (n = 4; n < (unsigned)(trailer - lexer->inbuffer); n++)
		crc += (int)lexer->inbuffer[n];
	    crc &= 0x7fff;
	    if (checksum == crc)
		packet_accept(lexer, SIRF_PACKET);
	    else
		lexer->state = GROUND_STATE;
	    packet_discard(lexer);
            break;
	}
#endif /* SIRF_ENABLE */
#if defined(TSIP_ENABLE) || defined(GARMIN_ENABLE)
	else if (lexer->state == TSIP_RECOGNIZED) {
            size_t packetlen = lexer->inbufptr - lexer->inbuffer;
	    if ( packetlen < 5) {
		lexer->state = GROUND_STATE;
            } else {
		unsigned int pkt_id, len;
		size_t n;
#ifdef GARMIN_ENABLE
		unsigned int ch, chksum;
		n = 0;
		/*@ +charint */
		if (lexer->inbuffer[n++] != DLE) 
		    goto not_garmin;
		pkt_id = lexer->inbuffer[n++]; /* packet ID */
		len = lexer->inbuffer[n++];
		chksum = len + pkt_id;
		if (len == DLE) {
		    if (lexer->inbuffer[n++] != DLE)
			goto not_garmin;
		}
		for (; len > 0; len--) {
		    chksum += lexer->inbuffer[n];
		    if (lexer->inbuffer[n++] == DLE) {
			if (lexer->inbuffer[n++] != DLE)
			    goto not_garmin;
		    }
		}
		/* check sum byte */
		ch = lexer->inbuffer[n++];
		chksum += ch;
		if (ch == DLE) {
		    if (lexer->inbuffer[n++] != DLE)
			goto not_garmin;
		}
		if (lexer->inbuffer[n++] != DLE)
		    goto not_garmin;
		if (lexer->inbuffer[n++] != ETX) 
		    goto not_garmin;
		/*@ +charint */
		chksum &= 0xff;
		if (chksum) {
		    gpsd_report(LOG_IO,
				"Garmin checksum failed: %02x!=0\n",chksum);
		    goto not_garmin;
		}
		/* Debug
		   gpsd_report(LOG_IO, "Garmin n= %#02x\n %s\n", n,
		   gpsd_hexdump(lexer->inbuffer, packetlen));
		*/
		packet_accept(lexer, GARMIN_PACKET);
		packet_discard(lexer);
		break;
	    not_garmin:;
	        gpsd_report(LOG_RAW+1,"Not a Garmin packet\n");
#endif /* GARMIN_ENABLE */
#ifdef TSIP_ENABLE
		/* check for some common TSIP packet types:
		 * 0x41, GPS time, data length 10
		 * 0x42, Single Precision Fix, data length 16
		 * 0x43, Velocity Fix, data length 20
		 * 0x45, Software Version Information, data length 10
		 * 0x46, Health of Receiver, data length 2
		 * 0x4a, LLA Position, data length 20
		 * 0x4b, Machine Code Status, data length 3
		 * 0x56, Velocity Fix (ENU), data length 20
		 * 0x5c, Satellite Tracking Status, data length 24
		 * 0x6d, All-In-View Satellite Selection, data length 16+numSV
		 * 0x82, Differential Position Fix Mode, data length 1
		 * 0x83, Double Precision XYZ, data length 36
		 * 0x84, Double Precision LLA, data length 36
		 *
		 * <DLE>[pkt id] [data] <DLE><ETX>
		 */
		/*@ +charint @*/
		pkt_id = lexer->inbuffer[1]; /* packet ID */
		if ((0x41 > pkt_id) || (0x8f < pkt_id)) {
		    gpsd_report(LOG_RAW+1, "Packet ID out of range for TSIP\n");
		    goto not_tsip;
		}
		/*@ -ifempty */
		if ((0x41 == pkt_id) && (0x0c == packetlen))
		    /* pass */;
		else if ((0x42 == pkt_id) && (0x12 == packetlen ))
		    /* pass */;
		else if ((0x43 == pkt_id) && (0x16 == packetlen))
		    /* pass */;
		else if ((0x45 == pkt_id) && (0x0c == packetlen))
		    /* pass */;
		else if ((0x46 == pkt_id) && (0x04 == packetlen))
		    /* pass */;
		else if ((0x4a == pkt_id) && (0x16 == packetlen))
		    /* pass */;
		else if ((0x4b == pkt_id) && (0x05 == packetlen))
		    /* pass */;
		else if ((0x56 == pkt_id) && (0x16 == packetlen))
		    /* pass */;
		else if ((0x5c == pkt_id) && (0x1a == packetlen))
		    /* pass */;
		else if ((0x6d == pkt_id) && ((0x12 <= packetlen) && (0x1e >= packetlen) ))
		    /* pass */;
		else if ((0x82 == pkt_id) && (0x03 == packetlen))
		    /* pass */;
		else if ((0x8f == pkt_id))
		    /* pass */;
		else {
		    gpsd_report(LOG_IO,
			"TSIP REJECT pkt_id = %#02x, packetlen= %#02x\n",
			pkt_id, packetlen); 
		    goto not_tsip;
		}
		/* Debug */
		gpsd_report(LOG_RAW,
		    "TSIP pkt_id = %#02x, packetlen= %#02x\n",
		    pkt_id, packetlen); 
		/*@ -charint +ifempty @*/
		packet_accept(lexer, TSIP_PACKET);
		packet_discard(lexer);
		break;
	    not_tsip:
	        gpsd_report(LOG_RAW+1,"Not a TSIP packet\n");
		/*
		 * More attempts to recognize ambiguous TSIP-like
		 * packet types could go here.
		 */
		lexer->state = GROUND_STATE;
		packet_discard(lexer);
		break;
#endif /* TSIP_ENABLE */
	    }
	}
#endif /* TSIP_ENABLE || GARMIN_ENABLE */
#ifdef ZODIAC_ENABLE
	else if (lexer->state == ZODIAC_RECOGNIZED) {
	    short len, n, sum;
	    len = getword(2);
	    for (n = sum = 0; n < len; n++)
		sum += getword(5+n);
	    sum *= -1;
	    if (len == 0 || sum == getword(5 + len)) {
		packet_accept(lexer, ZODIAC_PACKET);
	    } else {
		gpsd_report(LOG_IO,
		    "Zodiac data checksum 0x%hx over length %hd, expecting 0x%hx\n",
			sum, len, getword(5 + len));
		lexer->state = GROUND_STATE;
	    }
	    packet_discard(lexer);
            break;
	}
#endif /* ZODIAC_ENABLE */
#ifdef EVERMORE_ENABLE
	else if (lexer->state == EVERMORE_RECOGNIZED) {
	    unsigned int n, crc, checksum, len;
	    n = 0;
	    /*@ +charint */
	    if (lexer->inbuffer[n++] != DLE) 
		goto not_evermore;
	    if (lexer->inbuffer[n++] != STX)
		goto not_evermore;
	    len = lexer->inbuffer[n++];
	    if (len == DLE) {
		if (lexer->inbuffer[n++] != DLE)
		    goto not_evermore;
	    }
	    len -= 2;
	    crc = 0;
	    for (; len > 0; len--) {
		crc += lexer->inbuffer[n];
		if (lexer->inbuffer[n++] == DLE) {
		    if (lexer->inbuffer[n++] != DLE)
			goto not_evermore;
		}
	    }
	    checksum = lexer->inbuffer[n++];
	    if (checksum == DLE) {
		if (lexer->inbuffer[n++] != DLE)
		    goto not_evermore;
	    }
	    if (lexer->inbuffer[n++] != DLE)
		goto not_evermore;
	    if (lexer->inbuffer[n++] != ETX)
		goto not_evermore;
	    crc &= 0xff;
	    if (crc != checksum) {
		gpsd_report(LOG_IO, 
			    "EverMore checksum failed: %02x != %02x\n", 
			    crc, checksum);
		goto not_evermore;
	    }
	    /*@ +charint */
	    packet_accept(lexer, EVERMORE_PACKET);
	    packet_discard(lexer);
	    break;
	not_evermore:
	    lexer->state = GROUND_STATE;
	    packet_discard(lexer);
            break;
	}
#endif /* EVERMORE_ENABLE */
#ifdef ITALK_ENABLE
	else if (lexer->state == ITALK_RECOGNIZED) {
	    u_int16_t len, n, sum;
	    len = (unsigned short)(lexer->length / 2 - 1);
	    /*
	     * Skip first 9 words so we compute checksum only over data
	     * portion of packet.
	     */
	    for (n = sum = 0; n < (unsigned short)(len - 9); n++)
		sum += getword(9 + n);
	    if (len == 0 || sum == (u_int16_t)getword(len+1)) {
		gpsd_report(LOG_RAW, "italk checksum ok\n");
		packet_accept(lexer, ITALK_PACKET);
	    } else {
		gpsd_report(LOG_RAW, "italk checksum failed\n");
		lexer->state = GROUND_STATE;
	    }
	    packet_discard(lexer);
            break;
	}
#endif /* ITALK_ENABLE */
#ifdef RTCM104_ENABLE
	else if (lexer->state == RTCM_RECOGNIZED) {
	    /*
	     * RTCM packets don't have checksums.  The six bits of parity 
	     * per word and the preamble better be good enough.
	     */
	    packet_accept(lexer, RTCM_PACKET);
	    lexer->state = RTCM_SYNC_STATE;
	    packet_discard(lexer);
            break;
	}
#endif /* RTCM104_ENABLE */
    } /* while */

    return (ssize_t)fix;
}
#undef getword

ssize_t packet_get(int fd, struct gps_packet_t *lexer)
/* grab a packet; returns either BAD_PACKET or the length */
{
    ssize_t recvd;

    /*@ -modobserver @*/
    recvd = read(fd, lexer->inbuffer+lexer->inbuflen,
			sizeof(lexer->inbuffer)-(lexer->inbuflen));
#ifdef STATEDEBUG
    gpsd_report(LOG_RAW+1, "%d raw bytes read: %s\n",
		recvd, gpsd_hexdump(lexer->inbuffer+lexer->inbuflen, recvd));
#endif /* STATEDEBUG */
    /*@ +modobserver @*/
    if (recvd == -1) {
        if ((errno == EAGAIN) || (errno == EINTR)) {
	    return 0;
        } else {
#ifdef STATEDEBUG
	    gpsd_report(LOG_RAW+1, "errno: %s\n", strerror(errno));
#endif /* STATEDEBUG */
	    return BAD_PACKET;
        }
    }

    if (recvd == 0)
	return 0;
    return packet_parse(lexer, (size_t)recvd);
}

void packet_reset(struct gps_packet_t *lexer)
/* return the packet machine to the ground state */
{
    lexer->type = BAD_PACKET;
    lexer->state = GROUND_STATE;
    lexer->inbuflen = 0;
    lexer->inbufptr = lexer->inbuffer;
#ifdef BINARY_ENABLE
    isgps_init(lexer);
#endif /* BINARY_ENABLE */
}


#ifdef __UNUSED__
void packet_pushback(struct gps_packet_t *lexer)
/* push back the last packet grabbed */
{
    if (lexer->outbuflen + lexer->inbuflen < MAX_PACKET_LENGTH) {
	memmove(lexer->inbuffer+lexer->outbuflen,
		lexer->inbuffer,
		lexer->inbuflen);
	memmove(lexer->inbuffer,
		lexer->outbuffer,
		lexer->outbuflen);
	lexer->inbuflen += lexer->outbuflen;
	lexer->inbufptr += lexer->outbuflen;
	lexer->outbuflen = 0;
    }
}
#endif /* __UNUSED */
