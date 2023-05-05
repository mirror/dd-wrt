/* $Id$ */
/****************************************************************************
 *
 * Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2011-2013 Sourcefire, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 ****************************************************************************/

//--------------------------------------------------------------------
// hi stuff
//
// @file    hi_paf.c
// @author  Russ Combs <rcombs@sourcefire.com>

// the goal is to perform the minimal http paf parsing required for
// correctness while maintaining loose coupling with hi proper:

// * distinguish request from response by presence of http version
//   as first token in first header of response
// * identify head request so response is understood to not have body
// * determine length of body from content-length header
// * determine chunking from transfer-endoding header
// * extract chunk lengths for body chunks
// * determine end of chunks from chunk length of zero

// 1XX, 204, or 304 status responses must not have a body per RFC but
// if other headers indicate a body is present we will process that.
// This is different for head responses because content-length or
// transfer-encoding are expected.
//
// TBD:
// * Expect: 100-continue
// * Range, Content-Range, and multipart
//--------------------------------------------------------------------

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


#include "generators.h"
#include "hi_paf.h"
#include "hi_eo_events.h"
#include "decode.h"
#include "snort.h"
#include "stream_api.h"
#include "snort_debug.h"
#include "snort_httpinspect.h"
#include "memory_stats.h"

#ifdef DEBUG_MSGS
#define HI_TRACE     // define for state trace
#endif

#define HIF_REQ 0x0001  // message is request
#define HIF_RSP 0x0002  // message is response
#define HIF_LEN 0x0004  // content-length
#define HIF_CHK 0x0008  // transfer-encoding: chunked
#define HIF_NOB 0x0010  // head (no body in response)
#define HIF_NOF 0x0020  // no flush (chunked body follows)
#define HIF_V09 0x0040  // simple request version 0.9
#define HIF_V10 0x0080  // server response version 1.0
#define HIF_V11 0x0100  // server response version 1.1
#define HIF_ERR 0x0200  // flag error for deferred abort (at eoh)
#define HIF_GET 0x0400  // post (requires content-length or chunks)
#define HIF_PST 0x0800  // post (requires content-length or chunks)
#define HIF_EOL 0x1000  // already saw at least one eol (for v09)
#define HIF_ECD 0x2000  // Content Encoding 
#define HIF_UPG 0x4000  // Http/2 Upgrade:h2c
#define HIF_TST 0x8000  // HTTPv1.0 with chunked header, peek first few bytes of body
#define HIF_END 0x10000  //seen end of header
#define HIF_ALT 0x20000  // alet for no header
#define HIF_CHE 0x40000 //HTTP chunk extension
#define HIF_V1X 0x80000  // invalid HTTP 1.XX version

enum FlowDepthState
{
    HI_FLOW_DEPTH_STATE_NONE = 0,       //start 
    HI_FLOW_DEPTH_STATE_CONT_LEN,       //content length start
    HI_FLOW_DEPTH_STATE_CHUNK_START,    //chunked encoding start
    HI_FLOW_DEPTH_STATE_CHUNK_DISC_SKIP,//chunked encoding discard skip after flow depth
    HI_FLOW_DEPTH_STATE_CHUNK_DISC_CONT,//chunked encoding discard continue till end of PDU
};


typedef struct {
    int flow_depth;
    uint32_t len;
    uint32_t flags;
    uint32_t pipe;
    uint32_t paf_bytes;
    uint8_t msg;
    uint8_t fsm;
    uint8_t flow_depth_state;
    uint8_t char_end_of_header;
    uint8_t junk_chars;
    bool eoh;
    bool disable_chunked;
    bool valid_http;
    bool fast_blocking;
} Hi5State;

// config stuff
static uint32_t hi_cap = 0;

// stats
static uint32_t hi_paf_calls = 0;
static uint32_t hi_paf_bytes = 0;

//Reset flow depth if required ( for chunked transfer encoding )
static bool flow_depth_reset = false;

static uint8_t hi_paf_id = 0;

static bool hi_eoh_found = false;
static bool hi_is_chunked = false;

#ifdef TARGET_BASED
extern int16_t hi_app_protocol_id;
#endif

#define MAX_JUNK_CHAR_BEFORE_RESP_STATUS 127
#define  MAX_CHAR_BEFORE_RESP_STATUS 255
#define MAX_CHAR_AFTER_CHUNK_SIZE 255



//--------------------------------------------------------------------
// char classification stuff per RFC 2616
//--------------------------------------------------------------------

#define CLASS_ERR 0x00
#define CLASS_ANY 0x01
#define CLASS_CHR 0x02
#define CLASS_TOK 0x04
#define CLASS_CRT 0X08
#define CLASS_LWS 0x10
#define CLASS_SEP 0x20
#define CLASS_EOL 0x40
#define CLASS_DIG 0x80

static const uint8_t class_map[256] =
{
//                                                         tab    lf                cr
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x33, 0x43, 0x03, 0x03, 0x0B, 0x03, 0x03,
    0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
//
//   ' ',  '!',  '"',  '#',  '$',  '%',  '&',  ''',  '(',  ')',  '*',  '+',  ',',  '-',  '.',  '/'
    0x33, 0x07, 0x23, 0x07, 0x07, 0x07, 0x07, 0x07, 0x23, 0x23, 0x07, 0x07, 0x23, 0x07, 0x07, 0x23,
//
//   '0',  '1',  '2',  '3',  '4',  '5',  '6',  '7',  '8',  '9',  ':',  ';',  '<',  '=',  '>',  '?'
    0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x87, 0x23, 0x23, 0x23, 0x23, 0x23, 0x23,
//
//   '@',  'A',  'B',  'C',  'D',  'E',  'F',  'G',  'H',  'I',  'J',  'K',  'L',  'M',  'N',  'O' 
    0x23, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
//
//   'P',  'Q',  'R',  'S',  'T',  'U',  'V',  'W',  'X',  'Y',  'Z',  '[',  '\',  ']',  '^',  '_' 
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x23, 0x23, 0x23, 0x07, 0x07,
//
//   '`',  'a',  'b',  'c',  'd',  'e',  'f',  'g',  'h',  'i',  'j',  'k',  'l',  'm',  'n',  'o' 
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
//
//   'p',  'q',  'r',  's',  't',  'u',  'v',  'w',  'x',  'y',  'z',  '{',  '|',  '}',  '~',  '.' 
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x23, 0x07, 0x23, 0x07, 0x03,
//
//  non-ascii
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
    0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
};

// define class strings with ctl bytes to avoid collision
#define LWSS "\1"  // matches LWS (excludes \n)
#define EOLS "\2"  // matches LF (excludes \r)
#define ANYS "\3"  // matches OCTET; other not used so
                   // set other = match for consistency
#define TOKS "\4"  // matches token
#define SEPS "\5"  // matches SEP
#define CHRS "\6"  // matches CHAR
#define DIGS "\7"  // matches DIGIT
#define CRET "\10" // matches CR

static inline bool is_lwspace(const char c)
{
    return (c == ' ' || c == '\t');
}

static uint8_t Classify (const char* s)
{
    if ( !strcmp(s, ANYS) )
        return CLASS_ANY;

    if ( !strcmp(s, CHRS) )
        return CLASS_CHR;

    if ( !strcmp(s, TOKS) )
        return CLASS_TOK;

    if ( !strcmp(s, CRET) )
        return CLASS_CRT;

    if ( !strcmp(s, LWSS) )
        return CLASS_LWS;

    if ( !strcmp(s, SEPS) )
        return CLASS_SEP;

    if ( !strcmp(s, EOLS) )
        return CLASS_EOL;

    if ( !strcmp(s, DIGS) )
        return CLASS_DIG;

    return CLASS_ERR;
}

//--------------------------------------------------------------------
// fsm stuff
//--------------------------------------------------------------------

typedef enum {
    ACT_NOP, ACT_NOB,
    ACT_GET, ACT_PST,
    ACT_V09, ACT_V10, ACT_V11, ACT_V1X,
    ACT_SRL, ACT_REQ, ACT_RSP,
    ACT_SHI, ACT_SHX,
    ACT_LNB, ACT_LNC, ACT_LN0, ACT_ECD,
    ACT_CHK, ACT_CK0, ACT_HDR,
    ACT_H2, ACT_UPG,
    ACT_JNK
} Action;

typedef struct {
    uint8_t action;
    uint8_t next;
} Cell;

typedef struct {
    Cell cell[256];
} State;

static State* hi_fsm = NULL;
static unsigned hi_fsm_size = 0;

#define EOL '\n'  // \r is ignored
#define LWS ' '   // space or tab

typedef struct {
    uint8_t state;
    uint8_t match;
    uint8_t other;
    uint8_t action;
    const char* event;
} HiRule;

// these are just convenient jump points to the start
// of blocks; the states MUST match array index
// more of these make it easier to insert states
#define P0 (0)
#define P1 (P0+8)
#define P2 (P1+11)
#define P3 (P2+2)
#define P4 (P3+2)

#define Q0 (P4+3)
#define Q1 (Q0+15)
#define Q2 (Q1+6)
#define Q3 (Q2+9)

#define R2 (Q3+3)
#define R3 (R2+7)
#define R4 (R3+5)
#define R5 (R4+5)
#define R6 (R5+1)
#define R7 (R6+4)
#define R8 (R7+2)
#define R9 (R8+2)
#define R10 (R9+1)

#define MAX_STATE        (R10+4)
#define RSP_START_STATE  (P0)
#define REQ_START_STATE  (Q0)
#define REQ_V09_STATE_1  (Q1+3)
#define REQ_V09_STATE_2  (Q2)
#define MSG_CHUNK_STATE  (R7)
#define MSG_EOL_STATE    (R10)
#define RSP_ABORT_STATE  (P4)
#define REQ_ABORT_STATE  (Q3)

// -------------------------------------------------------------------
// byte-by-byte FSM
// -- eliminates need to accumulate a copy of the current token
// -- no need to compare prefixes when branching (just current byte)
//
// however, we must split tokens at branch points to distinguish
// between non-match events at the start or later in the string.
// also must split whenever the action differs from prior.
//
// it is possible to eliminate this split but it requires a lot
// of additional complexity in the compiler.
//
// see below for other possible enhancements.
// -------------------------------------------------------------------

static HiRule hi_rule[] =
{
    // P* are resPonse specific
    // http version starts response (8 chars)
    { P0+ 0, P0+ 1, P3   , ACT_SRL, "H"  },
    { P0+ 1, P0+ 2, P3   , ACT_NOP, "TTP/1." },
    { P0+ 2, P0+ 5, P0+ 3, ACT_V10, "0"  },
    { P0+ 3, P0+ 5, P0+ 4, ACT_V11, "1"  },
    { P0+ 4, P0+ 5, P0+ 6, ACT_V11, DIGS },
    { P0+ 5, P0+ 5, P0+ 6, ACT_V1X, DIGS },
    { P0+ 6, P0+ 7, P4   , ACT_NOP, LWSS },
    { P0+ 7, P0+ 7, P1   , ACT_NOP, LWSS },

    // now get status code (3 chars)
    { P1+ 0, P1+ 6, P1+ 1, ACT_NOB, "1"  },
    { P1+ 1, P1+ 3, P1+ 2, ACT_NOP, "2"  },
    { P1+ 2, P1+ 3, P1+ 5, ACT_NOP, "3"  },
    { P1+ 3, P1+ 4, P1+ 6, ACT_NOP, "0"  },
    { P1+ 4, P1+ 8, P1+ 7, ACT_NOB, "4"  },
    { P1+ 5, P1+ 6, P1+ 6, ACT_NOP, DIGS },
    { P1+ 6, P1+ 7, P1+ 7, ACT_NOP, DIGS },
    { P1+ 7, P1+ 8, P4+ 2, ACT_NOP, DIGS },
    { P1+ 8, P1+ 10, P1+ 9, ACT_NOP, LWSS }, 
    // no status message (http response status code followed by \r\n)    
    { P1+ 9, R2+ 0, P4+ 2, ACT_RSP, EOLS },
    { P1+ 10, P1+ 10, P2, ACT_NOP, LWSS },

    // now get status message (variable)
    { P2+ 0, R2+ 0, P2+ 1, ACT_RSP, EOLS },
    { P2+ 1, P2+ 0, P2+ 0, ACT_NOP, ANYS },

    { P3+ 0, P0   , P3+ 1,   ACT_JNK, EOLS },
    { P3+ 1, P3+ 0, P3+ 0,   ACT_JNK, ANYS },

    // resync state
    { P4+ 0, P0   , P4+ 1, ACT_NOP, LWSS },
    { P4+ 1, P0   , P4+ 2, ACT_NOP, EOLS },
    { P4+ 2, P4   , P4+ 0, ACT_NOP, ANYS },

    // Q* are reQuest specific
    // get method required for 0.9 request
    { Q0+ 0, Q0+ 1, Q0+ 3, ACT_SRL, "G"  },
    { Q0+ 1, Q0+ 2, Q0+10, ACT_NOP, "ET" },
    { Q0+ 2, Q1+ 0, Q0+10, ACT_GET, LWSS },
    // head method signals no body in response
    { Q0+ 3, Q0+ 4, Q0+ 6, ACT_SRL, "H"  },
    { Q0+ 4, Q0+ 5, Q0+10, ACT_NOP, "EAD" },
    { Q0+ 5, Q1+ 0, Q0+10, ACT_NOB, LWSS },
    // post method must have content-length or chunks
    { Q0+ 6, Q0+ 7, Q0+12, ACT_SRL, "P"  },
    { Q0+ 7, Q0+ 8, Q0+10, ACT_NOP, "O" },
    { Q0+ 8, Q0+ 9, Q0+13, ACT_NOP, "ST" },
    { Q0+ 9, Q1+ 0, Q0+13, ACT_PST, LWSS },

    { Q0+ 10, Q0+ 11, Q0+ 13, ACT_NOP, "R"  },
    { Q0+ 11, R8+ 1, Q0+ 13, ACT_H2, "I * HTTP/2.0"  }, // R8+0 or R8+1

    // other method
    { Q0+ 12, Q0+13, Q3+ 0, ACT_SRL, TOKS },
    { Q0+ 13, Q0+13, Q0+14, ACT_NOP, TOKS },
    { Q0+ 14, Q1+ 0, Q3+ 2, ACT_NOP, LWSS },

    // this gets required URI / next token
    { Q1+ 0, R10+ 0, Q1+ 1, ACT_NOP, EOLS },
    { Q1+ 1, Q1+ 0, Q1+ 2, ACT_NOP, LWSS },
    { Q1+ 2, Q1+ 3, Q1+ 3, ACT_NOP, ANYS },
    { Q1+ 3, R10+ 0, Q1+ 4, ACT_V09, EOLS },
    { Q1+ 4, Q2+ 0, Q1+ 5, ACT_NOP, LWSS },
    { Q1+ 5, Q1+ 3, Q1+ 3, ACT_NOP, ANYS },

    // this gets version, allowing extra tokens
    // if start line doesn't end with version,
    // assume 0.9 SimpleRequest (1 line header)
    { Q2+ 0, R10+ 0, Q2+ 1, ACT_V09, EOLS },
    { Q2+ 1, Q2+ 0, Q2+ 2, ACT_NOP, LWSS },
    { Q2+ 2, Q2+ 3, Q2+ 8, ACT_NOP, "H"  },
    { Q2+ 3, Q2+ 4, Q2+ 8, ACT_NOP, "TTP/1." },
    { Q2+ 4, Q2+ 6, Q2+ 5, ACT_V10, "0"  },
    { Q2+ 5, Q2+ 6, Q2+ 8, ACT_V11, "1"  },
    { Q2+ 6, R2+ 0, Q2+ 7, ACT_REQ, EOLS },
    { Q2+ 7, Q2+ 6, Q2+ 8, ACT_NOP, LWSS },
    { Q2+ 8, Q2+ 0, Q2+ 0, ACT_NOP, ANYS },

    // resync state
    { Q3+ 0, Q0   , Q3+ 1, ACT_NOP, LWSS },
    { Q3+ 1, Q0   , Q3+ 2, ACT_NOP, EOLS },
    { Q3+ 2, Q3   , Q3+ 0, ACT_NOP, ANYS },

    // R* are request or response
    // for simplicity, we don't restrict headers to allowed
    // direction of transfer (eg cookie only from client).
    // content-length is optional
    { R2+ 0, R2+ 1, R3   , ACT_HDR, "C"  },
    { R2+ 1, R2+ 2, R10  , ACT_NOP, "ONTENT-"   },
    { R2+ 2, R2+ 4, R2+3 , ACT_NOP, "LENGTH"    },
    { R2+ 3, R2+ 4, R10  , ACT_ECD, "ENCODING"  },
    { R2+ 4, R2+ 4, R2+5 , ACT_NOP, LWSS },
    { R2+ 5, R2+ 6, R10   , ACT_NOP, ":"  },
    { R2+ 6, R2+ 6, R6   , ACT_LN0, LWSS }, 
    // Upgrade parameter
    { R3+ 0, R3+ 1, R4   , ACT_HDR, "U"  },
    { R3+ 1, R3+ 2, R10   , ACT_NOP, "PGRADE"  },
    { R3+ 2, R3+ 2, R3+ 3, ACT_NOP, LWSS },
    { R3+ 3, R3+ 4, R10   , ACT_NOP, ":"  },
    { R3+ 4, R3+ 4, R9   , ACT_NOP, LWSS },

    // transfer-encoding required for chunks
    { R4+ 0, R4+ 1, R10   , ACT_HDR, "T"  },
    { R4+ 1, R4+ 2, R10   , ACT_NOP, "RANSFER-ENCODING"  },
    { R4+ 2, R4+ 2, R4+ 3, ACT_NOP, LWSS },
    { R4+ 3, R4+ 4, R10   , ACT_NOP, ":"  },
    { R4+ 4, R4+ 4, R5   , ACT_NOP, LWSS },

    // only recognized encoding
    { R5+ 0, R10   , R10   , ACT_CHK, "CHUNKED" },

    // extract decimal content length
    { R6+ 0, R2   , R6+ 1, ACT_LNB, EOLS },
    { R6+ 1, R6   , R6+ 2, ACT_SHI, DIGS },
    { R6+ 2, R6   , R6+ 3, ACT_SHI, LWSS },
    { R6+ 3, R10   , R10   , ACT_SHI, ANYS },

    // extract hex chunk length
    { R7+ 0, R8   , R7+ 1, ACT_LNC, EOLS },
    { R7+ 1, R7   , R7   , ACT_SHX, ANYS },

    // skip to end of line after chunk data
    { R8+ 0, R7   , R8+ 1, ACT_LN0, EOLS },
    { R8+ 1, R8   , R8   , ACT_NOP, ANYS },

    { R9+ 0, R10   , R10   , ACT_UPG, "h2c" },

    // skip to end of line
    { R10+ 0, R2    , R10+1  , ACT_NOP, EOLS },
    { R10+ 1, R10+3 , R10+2  , ACT_NOP, CRET },
    { R10+ 2, R10   , R10    , ACT_NOP, ANYS },
    { R10+ 3, R2    , R10+2  , ACT_NOP, EOLS },
};
static void hi_update_flow_depth_state(Hi5State *hip, uint32_t* fp, PAF_Status *paf );
static void get_flow_depth(void *ssn, uint64_t flags, Hi5State *hip);
static void get_fast_blocking_status(Hi5State *hip);


//--------------------------------------------------------------------
// trace stuff
//--------------------------------------------------------------------

#ifdef HI_TRACE
static void get_state (int s, char* buf, int max)
{
    int nbase;
    const char* sbase;

    if ( s >= MAX_STATE ) { sbase = "X"; nbase = MAX_STATE; }

    else if ( s >= R8 ) { sbase = "R8"; nbase = R8; }
    else if ( s >= R7 ) { sbase = "R7"; nbase = R7; }
    else if ( s >= R6 ) { sbase = "R6"; nbase = R6; }
    else if ( s >= R5 ) { sbase = "R5"; nbase = R5; }
    else if ( s >= R4 ) { sbase = "R4"; nbase = R4; }
    else if ( s >= R3 ) { sbase = "R3"; nbase = R3; }
    else if ( s >= R2 ) { sbase = "R2"; nbase = R2; }
    //else if ( s >= R1 ) { sbase = "R1"; nbase = R1; }
    //else if ( s >= R0 ) { sbase = "R0"; nbase = R0; }

    else if ( s >= Q3 ) { sbase = "Q3"; nbase = Q3; }
    else if ( s >= Q2 ) { sbase = "Q2"; nbase = Q2; }
    else if ( s >= Q1 ) { sbase = "Q1"; nbase = Q1; }
    else if ( s >= Q0 ) { sbase = "Q0"; nbase = Q0; }

    else if ( s >= P4 ) { sbase = "P4"; nbase = P4; }
    else if ( s >= P3 ) { sbase = "P3"; nbase = P3; }
    else if ( s >= P2 ) { sbase = "P2"; nbase = P2; }
    else if ( s >= P1 ) { sbase = "P1"; nbase = P1; }
    else                { sbase = "P0"; nbase = P0; }

    snprintf(buf, max, "%s+%d", sbase, (s-nbase));
}

#if 1
static inline bool dump_it (int c)
{
    if ( !c ) return false;
    return ( isupper(c) || isdigit(c) || strchr(":-/. \t\n", c) );
}
#endif

static void hi_fsm_dump ()
{
#if 0
    unsigned i;
    printf("%s", "s\\e");

    for ( i = 0; i < 256; i++ )
    {
        if ( dump_it(i) )
        {
            if ( strchr(" \t\n", i) )
                printf(",%02X", i);
            else
                printf(",%c", i);
        }
    }
    printf("\n");

    for ( i = 0; i < hi_fsm_size; i++ )
    {
        unsigned c;
        char buf[16];
        get_state(i, buf, sizeof(buf));
        printf("%s", buf);

        for ( c = 0; c < 256; c++ )
        {
            if ( dump_it(c) )
            {
                get_state(hi_fsm[i].cell[c].next, buf, sizeof(buf));
                printf(",%s", buf);
            }
        }
        printf("\n");
    }
#endif
}
#endif

//--------------------------------------------------------------------
// fsm build
//--------------------------------------------------------------------

#define TBD 0xFF

static void hi_load (State* state, int event, HiRule* rule)
{
    //assert(state->cell[event].next == TBD);
    state->cell[event].action = rule->action;
    state->cell[event].next = rule->match;
}

static void hi_reload (State* state, int event, int next)
{
    state->cell[event].action = ACT_NOP;
    state->cell[event].next = next;
}

static void hi_link (State* state, const char* event, HiRule* rule)
{
    bool not_done;

    do
    {
        int i;
        uint8_t class = Classify(event);
        not_done = strcmp(event, ANYS) != 0;

        if ( !class )
        {
            hi_load(state, toupper(*event), rule);
            hi_load(state, tolower(*event), rule);
        }
        else for ( i = 0; i < 256; i++ )
        {
            if ( (state->cell[i].next == TBD) && (class_map[i] & class) )
            {
                hi_load(state, toupper(i), rule);
                hi_load(state, tolower(i), rule);
            }
        }
        rule = hi_rule + rule->other;
        event = rule->event;
    }
    while ( not_done );
}

static void hi_relink (State* state, const char* event, int next)
{
    hi_reload(state, toupper(*event), next);
    hi_reload(state, tolower(*event), next);
}

static void hi_link_check (void)
{
    unsigned i, j;

    for ( i = 0; i < hi_fsm_size; i++ )
    {
        for ( j = 0; j < 256; j++ )
            assert(hi_fsm[i].cell[j].next != TBD);

        for ( j = 'A'; j <= 'Z'; j++ )
            assert(hi_fsm[i].cell[j].next == hi_fsm[i].cell[tolower(j)].next);
    }
}

static bool hi_fsm_compile (void)
{
    unsigned i = 0, j;
    unsigned max = sizeof(hi_rule) / sizeof(hi_rule[0]);
    unsigned next, extra = 0;

    while ( i < max )
    {
        // verify that HiRule.state corresponds to array index
        // HiRule.state is used solely for this sanity check.
        assert(hi_rule[i].state == i);

        if ( strlen(hi_rule[i].event) > 1 )
            extra += strlen(hi_rule[i].event) - 1;

        i++;
    }
    hi_fsm_size = max + extra;
    assert(hi_fsm_size < TBD);  // using uint8_t for Cell.next and Hi5State.fsm

    hi_fsm = SnortPreprocAlloc(1, hi_fsm_size*sizeof(*hi_fsm), PP_HTTPINSPECT, 
                  PP_MEM_CATEGORY_SESSION);
    if ( hi_fsm == NULL )
    {
        DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF, "Unable to allocate memory for hi_fsm."););
        return false;
    }
    next = max;

    for ( i = 0; i < hi_fsm_size; i++ )
        for ( j = 0; j < 256; j++ )
            hi_fsm[i].cell[j].next = TBD;

    for ( i = 0; i < max; i++ )
    {
        int prev = i, j, n = strlen(hi_rule[i].event);
        const char* event = hi_rule[i].event;

        hi_link(hi_fsm+i, event, hi_rule+i);

#if 0
        if ( n > 1 )
            printf("Expanding %s at %u\n", hi_rule[i].event, next);
#endif

        for ( j = 1; j < n; j++ )
        {
            event = hi_rule[i].event + j;
            hi_link(hi_fsm+next, event, hi_rule+i);

            event = hi_rule[i].event + j - 1;
            hi_relink(hi_fsm+prev, event, next);

            prev = next++;
        }
    }
    hi_link_check();
    assert(max + extra == next);
    return true;
}

//--------------------------------------------------------------------
// actions
//--------------------------------------------------------------------

static inline int dton (int c)
{
    return c - '0';
}

static inline int xton (int c)
{
    if ( isdigit(c) )
        return c - '0';

    if ( isupper(c) )
        return c - 'A' + 10;

    return c - 'a' + 10;
}

static inline void hi_paf_event_post ()
{
    SnortEventqAdd(
        GENERATOR_SPP_HTTP_INSPECT_CLIENT,
        HI_EO_CLIENT_UNBOUNDED_POST+1, 1, 0, 3,
        HI_EO_CLIENT_UNBOUNDED_POST_STR, NULL);
}

static inline void hi_paf_event_simple ()
{
    SnortEventqAdd(
        GENERATOR_SPP_HTTP_INSPECT_CLIENT,
        HI_EO_CLIENT_SIMPLE_REQUEST+1, 1, 0, 3,
        HI_EO_CLIENT_SIMPLE_REQUEST_STR, NULL);
}

static inline void hi_paf_event_msg_size ()
{
    SnortEventqAdd(
        GENERATOR_SPP_HTTP_INSPECT,
        HI_EO_CLISRV_MSG_SIZE_EXCEPTION+1, 1, 0, 3,
        HI_EO_CLISRV_MSG_SIZE_EXCEPTION_STR, NULL);
}

static inline void hi_paf_invalid_chunked ()
{
    SnortEventqAdd(
        GENERATOR_SPP_HTTP_INSPECT,
        HI_EO_CLISRV_INVALID_CHUNKED_ENCODING+1, 1, 0, 3,
        HI_EO_CLISRV_INVALID_CHUNKED_EXCEPTION_STR, NULL);
}

static inline void hi_paf_event_pipe ()
{
    SnortEventqAdd(
        GENERATOR_SPP_HTTP_INSPECT_CLIENT,
        HI_EO_CLIENT_PIPELINE_MAX+1, 1, 0, 3,
        HI_EO_CLIENT_PIPELINE_MAX_STR, NULL);
}

static inline void hi_paf_response_junk_line ()
{
    SnortEventqAdd(
        GENERATOR_SPP_HTTP_INSPECT,
        HI_EO_SERVER_JUNK_LINE_BEFORE_RESP_HEADER+1, 1, 0, 2,
        HI_EO_SERVER_JUNK_LINE_BEFORE_RESP_HEADER_STR, NULL);
}

static inline void hi_paf_response_invalid_chunksize ()
{
    SnortEventqAdd(
        GENERATOR_SPP_HTTP_INSPECT,
        HI_EO_SERVER_INVALID_CHUNK_SIZE+1, 1, 0, 2,
        HI_EO_SERVER_INVALID_CHUNK_SIZE_STR, NULL);
}

static inline void hi_paf_response_invalid_version ()
{
    SnortEventqAdd(
        GENERATOR_SPP_HTTP_INSPECT,
        HI_EO_SERVER_INVALID_VERSION_RESP_HEADER+1, 1, 0, 2,
        HI_EO_SERVER_INVALID_VERSION_RESP_HEADER_STR, NULL);
}

static inline PAF_Status hi_exec (Hi5State* s, Action a, int c, void* ssn)
{
    switch ( a )
    {
        case ACT_HDR:
            break;
        case ACT_SRL:
            break;
        case ACT_NOP:
            break;
        case ACT_V09:
            s->flags |= HIF_REQ|HIF_V09|HIF_ERR;
            break;
        case ACT_V10:
            s->flags |= HIF_V10;
            break;
        case ACT_V11:
            s->flags |= HIF_V11;
            break;
        case ACT_V1X:
            if ( !(s->flags & HIF_V1X) )
            {
                s->flags |= HIF_V1X;
                hi_paf_response_invalid_version();
            }
            break;
        case ACT_NOB:
            s->flags |= HIF_NOB;
            break;
        case ACT_GET:
            s->flags |= HIF_GET;
            break;
        case ACT_PST:
            s->flags |= HIF_PST;
            break;
        case ACT_REQ:
            s->flags |= HIF_REQ;
            break;
        case ACT_RSP:
            s->flags |= HIF_RSP;
            break;
        case ACT_SHI:
            if ( s->flags & HIF_ERR )
                break;
            if ( isdigit(c) && (s->len < 429496728) )
            {
                s->len = (10 * s->len) + dton(c);
            }
            else if (!is_lwspace(c))
            {
                hi_paf_event_msg_size();
                s->flags |= HIF_ERR;
            }
            break;
        case ACT_SHX:
            if ( s->flags & HIF_ERR )
                break;
            if ( isxdigit(c) && !(s->len & 0xF8000000) && !(s->flags & HIF_CHE) )
            {
                s->len = (s->len << 4) + xton(c);
            }
            else if((s->len & 0xF8000000)) 
            {
                hi_paf_event_msg_size();
                s->flags |= HIF_ERR;
                return PAF_FLUSH;
            }
            else
            {
                if( !(s->flags & HIF_CHE) )
                {
                    /*A correct chunk extension starts with semi-colon*/
                    if( c != ';' )
                    {
                        hi_paf_response_invalid_chunksize();
                    }
                    else
                    {
                        s->flags |= HIF_CHE;
                        s->junk_chars = 1;
                    }
                }
                else if( s->flags & HIF_CHE )
                {
                    s->junk_chars++;
                    if(s->junk_chars >= MAX_CHAR_AFTER_CHUNK_SIZE )
                    {
                        s->flags |= HIF_ERR;
                        s->junk_chars = 0;
                        return PAF_FLUSH;
                    }
                }
            }
            break;
        case ACT_LNB:
            s->flags |= HIF_LEN;
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
                "%s: lnb=%u\n", __FUNCTION__, s->len);)
            break;
        case ACT_ECD:
           s->flags |=HIF_ECD;
           break;
        case ACT_LNC:
            s->flags |= HIF_LEN;
            s->flags &= ~HIF_CHE;
            s->junk_chars = 0;
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
                "%s: lnc=%u\n", __FUNCTION__, s->len);)
            if ( s->len )
                return PAF_SKIP;
            if( s->flags & HIF_NOF )
                flow_depth_reset = true;
            s->flags &= ~HIF_NOF;
            s->msg = 3;
            break;
        case ACT_LN0:
            s->len = 0;
            break;
        case ACT_CHK:
            s->flags |= HIF_CHK;
            hi_is_chunked = true;
            break;
        case ACT_UPG:
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
            "%s: Http/1 Connection upgrade possible to Http/2\n", __FUNCTION__);)
            s->flags |= HIF_UPG;
            break;
        case ACT_H2:
            DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
            "%s: Http/2 Connection preface received\n", __FUNCTION__);)
            stream_api->set_session_http2(ssn);
            return PAF_ABORT;
        case ACT_CK0:
            s->flags |= HIF_NOF;
            s->flags &= ~HIF_CHK;
            s->fsm = MSG_CHUNK_STATE;
            s->len = 0;
            break;
        case ACT_JNK:
            if(++(s->junk_chars) >= MAX_JUNK_CHAR_BEFORE_RESP_STATUS)
            {
                s->valid_http = false;
                return PAF_ABORT;
            }  
            break;
    }
    return PAF_SEARCH;
}

//--------------------------------------------------------------------
// pipeline
//--------------------------------------------------------------------

#define MAX_PIPELINE       24
#define PIPELINE_RUPTURED 255

static void hi_pipe_push (Hi5State* s_req, void* ssn)
{
    uint32_t nreq = s_req->pipe & 0xFF;
    uint32_t pipe = s_req->pipe >> 8;

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
        "%s: nreq=%d, pipe=0x%X\n", __FUNCTION__, nreq, pipe);)

    if ( nreq == MAX_PIPELINE )
    {
        if ( stream_api->is_paf_active(ssn, 0) )
            hi_paf_event_pipe();
    }
    else if ( nreq < MAX_PIPELINE )
    {
        if ( s_req->flags & HIF_NOB )
            pipe |= (0x1 << nreq);
    }
    if ( nreq == PIPELINE_RUPTURED )
        return;

    s_req->pipe = (pipe << 8) | ++nreq;
}

static void hi_pipe_pop (Hi5State* s_rsp, void* ssn)
{
    Hi5State* s_req;
    uint32_t nreq, pipe;

    void** pv = stream_api->get_paf_user_data(ssn, 1, hi_paf_id);

    if ( !*pv )
        return;

    s_req = *pv;

    nreq = s_req->pipe & 0xFF;
    pipe = s_req->pipe >> 8;

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
        "%s: nreq=%d, pipe=0x%X\n", __FUNCTION__, nreq, pipe);)

    if ( nreq == 0 || nreq == PIPELINE_RUPTURED )
        return;

    if ( --nreq < MAX_PIPELINE )
    {
        if ( pipe & 0x1 )
            s_rsp->flags |= HIF_NOB;
    }
    pipe >>= 1;
    s_req->pipe = (pipe << 8) | nreq;
}

//--------------------------------------------------------------------
// control
//--------------------------------------------------------------------

static inline bool simple_allowed (Hi5State* s)
{
    return ( (s->fsm == REQ_V09_STATE_1 || s->fsm == REQ_V09_STATE_2)
        && s->flags & HIF_GET );
}

static inline bool have_pdu (Hi5State* s)
{
    return ( s->fsm != REQ_START_STATE && s->fsm != RSP_START_STATE );
}

static inline bool paf_abort (Hi5State* s)
{
    return ( s->fsm == REQ_ABORT_STATE || s->fsm == RSP_ABORT_STATE );
}

// this is the 2nd step of stateful scanning, which executes
// the fsm.
static PAF_Status hi_scan_fsm (Hi5State* s, int c, void* ssn)
{
    PAF_Status status;
    State* m = hi_fsm + s->fsm;
    Cell* cell = &m->cell[c];

#ifdef HI_TRACE
    char before[16], after[16];
    uint8_t prev = s->fsm;
#endif

    s->fsm = cell->next;

#ifdef HI_TRACE
    get_state(prev, before, sizeof(before));
    get_state(s->fsm, after, sizeof(after));
    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
        "%s: %s(%u)[0x%2X, '%c'] -> %d,%s(%u)\n",
        __FUNCTION__, before, prev, c, isgraph(c) ? c : '.',
        cell->action, after, s->fsm);)
#endif

    status = hi_exec(s, cell->action, c, ssn);

    return status;
}

static PAF_Status hi_eoh (Hi5State* s, void* ssn)
{
    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
        "%s: flags=0x%X, len=%u\n", __FUNCTION__, s->flags, s->len);)

    s->eoh = true;
    s->flags &= ~HIF_ALT;

    if ( (s->flags & HIF_REQ) )
        hi_pipe_push(s, ssn);
    else
        hi_pipe_pop(s, ssn);

    if( (s->flags & HIF_V10 &&
        s->flags & HIF_CHK))
    {
        hi_paf_invalid_chunked();
        s->flags |= HIF_TST;
    }
    if ( (s->flags & HIF_PST) &&
        !(s->flags & (HIF_CHK|HIF_LEN)) )
    {
        hi_paf_event_post();
        s->flags |= HIF_ERR;
    }

    if( s->junk_chars && (s->flags & HIF_RSP))
    {
        hi_paf_response_junk_line();
        s->junk_chars = 0;
    }

    if ( (s->flags & HIF_ERR) ||
        ((s->flags & HIF_NOB) && (s->flags & HIF_RSP))
    ) {
        if ( s->flags & HIF_V09 )
            hi_paf_event_simple();

        hi_exec(s, ACT_LN0, 0, ssn);
        return PAF_FLUSH;
    }
    if ( s->flags & HIF_CHK )
    {
        if( !(s->flags & HIF_TST) )
        {
            uint32_t fp;
            PAF_Status paf = PAF_SEARCH;
            hi_exec(s, ACT_CK0, 0, ssn);
            hi_update_flow_depth_state(s, &fp, &paf );
            return paf;
        }
         return PAF_SEARCH;
    }
    if ( (s->flags & (HIF_REQ|HIF_LEN)) )
        return PAF_FLUSH;

    if ( (s->flags & HIF_V11) && (s->flags & HIF_RSP) )
    {
        hi_exec(s, ACT_LN0, 0, ssn);
        hi_paf_event_msg_size();
        return PAF_FLUSH;
    }
    if ( (s->flags & HIF_V10) && (s->flags & HIF_ECD) && !(s->flags & HIF_LEN))
        return PAF_FLUSH; 
    return PAF_ABORT;
}

// http messages are scanned statefully, char-by-char, in
// two steps.  this is the 1st step, which figures out
// end-of-line (eol) and end-of-headers (eoh) from the byte
// stream.  also unfolds headers before fsm scanning.  this
// simplified version ignores \r (in the spirit of send strict,
// recv tolerant, but it would only take 2 more states to check
// for \r).  the 2nd step is hi_scan_fsm().
static inline PAF_Status hi_scan_msg (
    Hi5State* s, int c, uint32_t* fp, void* ssn)
{
    PAF_Status paf = PAF_SEARCH;
    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
        "%s[%d]: 0x%2X, '%c'\n", __FUNCTION__, s->msg, c, isgraph(c) ? c : '.');)

    if ( c == '\r' )
    {
        if ( hi_is_chunked  && hi_eoh_found  )
        {
            hi_paf_response_invalid_chunksize();
            hi_is_chunked = false;
        }
        hi_eoh_found = false;

        if ( s->msg != 1 && s->msg != 5 )
        {
            *fp = 0;
            return paf;
        }
    }
    hi_eoh_found = false;

    switch ( s->msg )
    {
    case 0:
        if ( c == '\n' )
        {
            if ( !(s->flags & HIF_EOL) )
            {
                s->flags |= HIF_EOL;

                if ( simple_allowed(s) )
                {
                    hi_scan_fsm(s, EOL, ssn);
                    paf = hi_eoh(s, ssn);
                }
                else
                    s->msg = 1;
            }
            else if ( s->flags & HIF_NOF )
                paf = hi_scan_fsm(s, EOL, ssn);
            else
                s->msg = 1;
        }
        else
            paf = hi_scan_fsm(s, c, ssn);
        break;

    case 1:
        if ( c == '\n' )
        {
            hi_scan_fsm(s, EOL, ssn);

            if ( have_pdu(s) )
                paf = hi_eoh(s, ssn);
            s->msg = 0;
        }
        else if (c == ' ' || c == '\t' || c == 0xb || c == 0xc)
        {
            if(s->fsm == MSG_EOL_STATE)
            {
             /* This s->char_end_of_header + 3 to avoid \n\r\n and \n\r\r\n */
               s->char_end_of_header = s->char_end_of_header + 3; 
               hi_scan_fsm(s, c, ssn);
               s->msg = 5;
            }
            else if(!(s->flags & HIF_RSP))
        {
            paf = hi_scan_fsm(s, LWS, ssn);
            s->msg = 0;
        }
            else
            {
               *fp = 0;
               return paf;
            }
        }
        else if ( c == '\r')
        {
            if(s->fsm == MSG_EOL_STATE)
            {
               s->char_end_of_header++;
               hi_scan_fsm(s, c, ssn);
               s->msg = 5;
            }
            else
            {
               *fp = 0;
               return paf;
            }
        }
        else
        {
            paf = hi_scan_fsm(s, EOL, ssn);

            if ( paf == PAF_SEARCH )
                paf = hi_scan_fsm(s, c, ssn);
            s->msg = 0;
        }
        break;

    case 3:
        if ( c == '\n' )
            paf = hi_eoh(s, ssn);
        else
        {
            s->msg = 4;
        }
        break;

    case 4:
        if ( c == '\n' )
        {
            s->msg = 3;
        }
        break;
    case 5:
        if ( c == '\n' )
        {
            hi_eoh_found = true;
            if ( s->char_end_of_header >= 2)
               s->flags |= HIF_END;
            s->char_end_of_header =0;
            hi_scan_fsm(s, EOL, ssn);

            if ( have_pdu(s) )
                paf = hi_eoh(s, ssn);
            s->msg = 0;
        }
        else if (c == '\r' ||c == '\t' || c == ' ' ||  c == 0xb || c == 0xc)
        {
          s->char_end_of_header++;
          hi_scan_fsm(s, c, ssn);
        }
        else
        {
            s->msg = 0;
            hi_scan_fsm(s, c, ssn);
            s->char_end_of_header =0;
        }
        break;

    }

    if ( paf_abort(s) )
        paf = PAF_ABORT;

    else if ( paf != PAF_SEARCH ) {
        /*
         * If the flush point is set based on content-length,
         * then the payload is eligible for pseudo flush for
         * early detection and file processing.
         */
        *fp = s->len;
        if (paf == PAF_FLUSH && s->fast_blocking &&
            !(stream_api->is_session_decrypted(ssn)) )
        {
            paf = PAF_PSEUDO_FLUSH_SEARCH;
        }
    }

    if( paf != PAF_SEARCH && paf != PAF_ABORT )
    {
        hi_update_flow_depth_state(s, fp, &paf );
    }

    return paf;
}

//--------------------------------------------------------------------
// utility
//--------------------------------------------------------------------

static void hi_reset (Hi5State* s, uint64_t flags, void *ssn)
{
    s->len = s->msg = 0;

    if ( flags & PKT_FROM_CLIENT )
    {
        s->fsm = REQ_START_STATE;
    }
    else
    {
        s->fsm = RSP_START_STATE;
    }
    s->flags = 0;
    s->junk_chars = 0;
    s->flow_depth_state = HI_FLOW_DEPTH_STATE_NONE;

    if( flow_depth_reset )
    {
        get_flow_depth(ssn, flags, s);
        flow_depth_reset = false;
    }

    get_fast_blocking_status(s);

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
        "%s: fsm=%u, flags=0x%X\n", __FUNCTION__, s->fsm, s->flags);)
}

/*peek into first few bytes of response body to guess if this is chunked.*/ 
bool  peek_chunk_size(const uint8_t* data, uint32_t len)
{
    bool chunk_len = false;
    int n = 0;
    while ( n < len )
    {
        char c = data[n];
        if(isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))
        {
            chunk_len = true;
            //Too big a chunk, probably not chunk size
            if( n > 8 )
                return false;
        }
        else if( c == '\r' || c == '\n' || c == ';')
        {
            if( chunk_len )
                return true;
            else
                return false;
        }
        else
        {
            //other character,cant be chunk size
            return false;
        }
        n++;
    }
    return true;
}

//--------------------------------------------------------------------
// callback for stateful scanning of in-order raw payload
//--------------------------------------------------------------------

static PAF_Status hi_paf (
    void* ssn, void** pv, const uint8_t* data, uint32_t len,
    uint64_t *flags, uint32_t* fp, uint32_t *fp_eoh)
{
    Hi5State* hip = *pv;
    PAF_Status paf = PAF_SEARCH;

    uint32_t n = 0;
    *fp = 0;

#ifdef TARGET_BASED
    if ( session_api )
    {
        int16_t proto_id = session_api->get_application_protocol_id(ssn);
    
        if ( (proto_id > 0) && (proto_id != hi_app_protocol_id) )
        {
             DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
                        "%s: its not HTTP, no need to detect\n",
                         __FUNCTION__);)
             return PAF_ABORT;
        }
    }
#endif

    if ( !hip )
    {
        // beware - we allocate here but s5 calls free() directly
        // so no pointers allowed
        hip = SnortPreprocAlloc(1, sizeof(Hi5State), PP_HTTPINSPECT, 
                   PP_MEM_CATEGORY_SESSION);

        if ( !hip )
        {
            *flags |= PKT_H1_ABORT;
            return PAF_ABORT;
        }
        *pv = hip;

        flow_depth_reset = true;

        hi_reset(hip, *flags, ssn );
    }

    hip->eoh = false;
hip->valid_http = true;
    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
        "%s: len=%u\n", __FUNCTION__, len);)

    if ( hip->flags & HIF_ERR )
    {
        *flags |= PKT_H1_ABORT;
        return PAF_ABORT;
    }

    if ( hi_cap && (hi_paf_bytes > hi_cap) )
    {
        *flags |= PKT_H1_ABORT;
        return PAF_ABORT;
    }

    hi_update_flow_depth_state(hip, fp, &paf);

    if( paf != PAF_SEARCH )
    {
        hi_paf_calls++;

        if ( paf == PAF_DISCARD_END )
            hi_reset(hip, *flags, ssn);

        hi_paf_bytes += *fp;
        
        if (paf == PAF_ABORT)
            *flags |= PKT_H1_ABORT;
        return paf;
    }


    while ( n < len )
    {
        if( !(hip->flags & HIF_ERR) && hip->flags & HIF_TST )
        {
            if( n != len )
            {
                if(peek_chunk_size(data + n, len - n))
                {
                    hi_exec(hip, ACT_CK0, 0, ssn);
                    hip->flags &= ~HIF_LEN;
                    hi_update_flow_depth_state(hip, fp, &paf );
                }
                else
                {
                     if( hip->flags & HIF_LEN )
                     {
                          paf = PAF_FLUSH;
                          *fp = hip->len;
                          hip->flags &= ~HIF_CHK;
                          hi_update_flow_depth_state(hip, fp, &paf );
                     }
                     else
                     {
                         if ( (hip->flags & HIF_PST))
                             hi_paf_event_post();
                         paf = PAF_ABORT;
                     }
                     hip->disable_chunked = true;
                }
                hip->flags &= ~HIF_TST;
            }
        }

        // jump ahead to next linefeed when possible
        if ( (hip->msg == 0) && (hip->fsm == MSG_EOL_STATE) )
        {
            uint8_t* lf = memchr(data+n, '\n', len-n);
            if ( !lf )
            {
                n = len;
                break;
            }
            n += (lf - (data + n));
        }
        if( paf == PAF_SEARCH)
        {
            paf = hi_scan_msg(hip, data[n++], fp, ssn);
            if( (hip->flags & HIF_RSP) && hip->flags &  HIF_END)
            {
                SnortEventqAdd(
                GENERATOR_SPP_HTTP_INSPECT,
                HI_EO_SERVER_NO_RESP_HEADER_END+1, 1, 0, 2,
                HI_EO_SERVER_NO_RESP_HEADER_END_STR, NULL);
                hip->flags |= HIF_ALT;
                hip->flags &= ~HIF_END;
                hip->char_end_of_header = 0; 
            }
            if (hip->flags & HIF_ALT)
            {
                hip->char_end_of_header++;
                if (hip->char_end_of_header  >= MAX_CHAR_BEFORE_RESP_STATUS )
                { 
                      paf = PAF_ABORT;
                      hip->flags &= ~HIF_ALT;
                      hip->char_end_of_header = 0;
                 }
             }
        }
        
        if ( hip->flags & HIF_UPG)
        {
            *flags |= PKT_UPGRADE_PROTO;
            //Before a client preface is received server can start
            //sending messages. From next packet h2_paf will take over.
            if (*flags & PKT_FROM_SERVER)
            {
                stream_api->set_session_http2(ssn);
                stream_api->set_session_http2_upg(ssn);
            }
        }

        if ( paf != PAF_SEARCH )
        {
            if ( hip->flags & HIF_ERR )
            {
                *fp = len;
                break;
            }

            *fp += n;
            if( hip->eoh && (( hip->flags & HIF_PST ))){
                *fp_eoh = n;
                if(*fp <= len )
                    *fp_eoh = 0;
            }

            if ( paf != PAF_SKIP && paf != PAF_DISCARD_START )
                hi_reset(hip, *flags, ssn);

            if ( hip->fast_blocking && (paf == PAF_SKIP) &&
                 !(stream_api->is_session_decrypted(ssn)) )
                paf = PAF_PSEUDO_FLUSH_SKIP;

            break;
        }
    }

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
        "%s: paf=%d, rfp=%u\n", __FUNCTION__, paf, *fp);)

    hi_paf_calls++;
    hi_paf_bytes += n;

    hip->paf_bytes += n;
    if ( paf == PAF_ABORT)
        *flags |= PKT_H1_ABORT;

    return paf;
}

//--------------------------------------------------------------------
// public stuff
//--------------------------------------------------------------------

int hi_paf_register_port (
    struct _SnortConfig *sc, uint16_t port, bool client, bool server, tSfPolicyId pid, bool auto_on)
{
    if ( !ScPafEnabled() )
        return 0;

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
        "%s: policy %u, port %u\n", __FUNCTION__, pid, port);)

    if ( !stream_api )
        return -1;

    if ( client )
        hi_paf_id = stream_api->register_paf_port(sc, pid, port, true, hi_paf, auto_on);

    if ( server )
        hi_paf_id = stream_api->register_paf_port(sc, pid, port, false, hi_paf, auto_on);

    return 0;
}

static void hi_paf_cleanup(void *pafData)
{
    if (pafData)
        SnortPreprocFree(pafData, sizeof(Hi5State), PP_HTTPINSPECT, PP_MEM_CATEGORY_SESSION);
}

int hi_paf_register_service (
    struct _SnortConfig *sc, uint16_t service, bool client, bool server, tSfPolicyId pid, bool auto_on)
{
    if ( !ScPafEnabled() )
        return 0;

    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
        "%s: policy %u, service %u\n", __FUNCTION__, pid, service);)

    if ( !stream_api )
        return -1;

    if ( client )
    {
        hi_paf_id = stream_api->register_paf_service(sc, pid, service, true, hi_paf, auto_on);
        stream_api->register_paf_free(hi_paf_id, hi_paf_cleanup);
    }
    if ( server )
    {
        hi_paf_id = stream_api->register_paf_service(sc, pid, service, false, hi_paf, auto_on);
        stream_api->register_paf_free(hi_paf_id, hi_paf_cleanup);
    }
    return 0;
}

//--------------------------------------------------------------------

bool hi_paf_init (uint32_t cap)
{
    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
        "%s: cap=%u\n",  __FUNCTION__, cap);)

    hi_cap = cap;

    if ( !hi_fsm_compile() )
        return false;

#ifdef HI_TRACE
    hi_fsm_dump();
#endif

    return true;
}

void hi_paf_term (void)
{
    SnortPreprocFree(hi_fsm, hi_fsm_size*sizeof(*hi_fsm), PP_HTTPINSPECT, 
         PP_MEM_CATEGORY_SESSION);
    DEBUG_WRAP(DebugMessage(DEBUG_STREAM_PAF,
        "%s: calls=%u, bytes=%u\n",  __FUNCTION__,
        hi_paf_calls, hi_paf_bytes);)

    hi_fsm = NULL;
    hi_fsm_size = 0;
}

//--------------------------------------------------------------------

bool hi_paf_simple_request (void* ssn)
{
    if ( ssn )
    {
        Hi5State** s = (Hi5State **)stream_api->get_paf_user_data(ssn, 1, hi_paf_id);

        if ( s && *s )
            return ( (*s)->flags & HIF_V09 );
    }
    return false;
}

static void get_fast_blocking_status(Hi5State *hip)
{
    hip->fast_blocking = GetHttpFastBlockingStatus();
}

static void  get_flow_depth(void *ssn, uint64_t flags, Hi5State *hip)
{
    hip->flow_depth = GetHttpFlowDepth(ssn, flags);
}

static void hi_update_flow_depth_state(Hi5State *hip, uint32_t* fp, PAF_Status *paf )
{
    //sanity check
    assert( hip );

    if( !hip->flow_depth )
        return;

    if ( (hip->flags & HIF_ERR) ||
            ((hip->flags & HIF_NOB) ))
        return;

    if( hip->flags & HIF_V09 )
        return;

    //Apply flow depth for POST in case of request
    if( (hip->flags & HIF_REQ ) &&
            !(hip->flags & HIF_PST) )
        return;

    switch( hip->flow_depth_state )
    {
        case HI_FLOW_DEPTH_STATE_NONE:
            if( hip->flags & HIF_LEN )
            {
                if( hip->len > 0 )
                {
                    if( hip->flow_depth == -1 )
                    {
                        *paf = PAF_DISCARD_START;
                        *fp = 0;
                        hip->flow_depth_state = HI_FLOW_DEPTH_STATE_CONT_LEN;
                    }
                    else if( hip->flow_depth > 0 )
                    {
                        if( hip->len > hip->flow_depth )
                        {
                            *paf = PAF_DISCARD_START;
                            *fp = hip->flow_depth;
                            hip->flow_depth_state = HI_FLOW_DEPTH_STATE_CONT_LEN;
                        }
                    }
                }
            }
            else if( hip->flags & HIF_NOF )
            {
                if( hip->flow_depth == -1 )
                {
                    *paf = PAF_DISCARD_START;
                    *fp = 0;
                    hip->flow_depth_state = HI_FLOW_DEPTH_STATE_CHUNK_DISC_CONT;
                }
                else if( hip->flow_depth > 0 )
                {
                    hip->flow_depth_state = HI_FLOW_DEPTH_STATE_CHUNK_START;
                }

            }
            break;

        case HI_FLOW_DEPTH_STATE_CONT_LEN:
            *paf = PAF_DISCARD_END;
            if( hip->flow_depth > 0 )
                *fp = hip->len - hip->flow_depth;
            else
                *fp = hip->len;
            break;

        case HI_FLOW_DEPTH_STATE_CHUNK_START:
            if( ( *paf == PAF_SKIP ) && ( hip->flow_depth <= hip->len ) )
            {
                *paf = PAF_DISCARD_START;
                *fp = hip->flow_depth;
                if( hip->flow_depth == hip->len )
                    hip->flow_depth_state = HI_FLOW_DEPTH_STATE_CHUNK_DISC_CONT;
                else
                    hip->flow_depth_state = HI_FLOW_DEPTH_STATE_CHUNK_DISC_SKIP;
                hip->len -= hip->flow_depth;
            }
            else if( *paf == PAF_SKIP )
            {
                hip->flow_depth -= hip->len;
            }
            break;

        case HI_FLOW_DEPTH_STATE_CHUNK_DISC_SKIP:
            if( hip->len )
            {
                *paf = PAF_SKIP;
                *fp = hip->len;
                hip->len = 0;
            }
            hip->flow_depth_state = HI_FLOW_DEPTH_STATE_CHUNK_DISC_CONT;
            break;

        case HI_FLOW_DEPTH_STATE_CHUNK_DISC_CONT:
            if( !( hip->flags & HIF_NOF) )
            {
                *paf = PAF_DISCARD_END;
            }
            break;

        default:
            break;
    }
}

bool hi_paf_resp_eoh(void* ssn)
{
    if ( ssn )
    {
        Hi5State** s = (Hi5State **)stream_api->get_paf_user_data(ssn, 0, hi_paf_id);

        if ( s && *s )
            return ( (*s)->eoh );
    }
    return false;
}

uint32_t hi_paf_resp_bytes_processed(void* ssn)
{
    if ( ssn )
    {
        Hi5State** s = (Hi5State **)stream_api->get_paf_user_data(ssn, 0, hi_paf_id);

        if ( s && *s )
            return ( (*s)->paf_bytes );
    }
    return 0;
}

bool hi_paf_valid_http(void* ssn)
{
    if ( ssn )
    {   
        Hi5State** s = (Hi5State **)stream_api->get_paf_user_data(ssn, 0, hi_paf_id);
         
        if ( s && *s )
            return ((*s)->valid_http);
    }
    return 1;
}
bool hi_paf_disable_te(void* ssn, bool to_server)
{
    if ( ssn )
    {
        Hi5State** s = (Hi5State **)stream_api->get_paf_user_data(ssn, to_server?1:0, hi_paf_id);

        if ( s && *s )
            return ( (*s)->disable_chunked);
    }
    return false;
}

uint32_t hi_paf_get_size()
{
    return sizeof(Hi5State);
}

