/*
 * $Id: match_http.c,v 1.21 2004/02/13 01:04:27 liquidk Exp $
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */


#define __NO_VERSION__

#include <linux/config.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/file.h>
#include <net/sock.h>

#include <linux/netfilter_ipv4/ipt_p2p.h>

/*****************************************************************************/

/* Ugly short-hand to avoid costly initialization */
#define STRING_MATCH(strobj) (strobj), (sizeof (strobj) - 1)

#define EOH -2 /* End of Headers */

struct string_match
{
	const char *name;
	size_t      len;
};

static struct string_match methods[] =
{
#define MM_GET_GET      0
	{ STRING_MATCH("GET /get/")               },

#define MM_GET_URIRES   1
	{ STRING_MATCH("GET /uri-res/")           },

#define MM_GET_HASH     2
	{ STRING_MATCH("GET /.hash=")             },

#define MM_GET_FILE     3
	{ STRING_MATCH("GET /.file")             },

#define MM_GET_SIG      4
	{ STRING_MATCH("GET /.sig")             },

#define MM_GET_POISONED 5
	{ STRING_MATCH("GET /PoisonedDownloads/") },

#define MM_GET          6
	{ STRING_MATCH("GET /")                   },

#define MM_GIVE         7
	{ STRING_MATCH("GIVE ")                   },

#define MM_HTTP11       8
	{ STRING_MATCH("HTTP/1.1")                },

	{ NULL, 0                                 }
};

#define METHODS_LEN (((sizeof(methods))/(sizeof(methods[0]))) - 1)

struct string_match headers[] =
{
#define HM_X_KAZAA          0
	{ STRING_MATCH("X-Kazaa-")       },

#define HM_X_GNUTELLA       1
	{ STRING_MATCH("X-Gnutella-")    },

#define HM_X_P2P_MESSAGE    2
	{ STRING_MATCH("X-P2P-Message:") },

#define HM_X_OPENFTALIAS    3
	{ STRING_MATCH("X-OpenftAlias:") },

#define HM_CONTENT_URN      4
	{ STRING_MATCH("Content-URN:")   },

#define HM_X_QUEUE          5
	{ STRING_MATCH("X-Queue:")       },

#define HM_X_TIGER_THREE    6
	{ STRING_MATCH("X-TigerTree")    },

	{ NULL, 0                        }
};

#define HEADERS_LEN (((sizeof(headers))/(sizeof(headers[0]))) - 1)

/*****************************************************************************/

static inline const unsigned char *
next_line(const unsigned char *data,
          const unsigned char *end)
{
	while (data <= end)
	{
	 	if (*data++ == '\n')
			return data;
	}

	return NULL;
}

static inline int
string_matchlist(const unsigned char *data,
                 const unsigned char *end,
                 const struct string_match *strings)
{
	int i;

	if(*data == '\r' || *data == '\n')
			return EOH;

	for (i = 0; strings[i].name != NULL; i++)
	{
		/* avoid overflow */
		if (data + strings[i].len > end)
			continue;

		if (memcmp(data, strings[i].name, strings[i].len) == 0)
			return i;
	}

	return -1;
}

#define MM(x) (method_matched == x)
#define HM(x) headers_matched[x]

int
match_http(const unsigned char *data,
           const unsigned char *end)
{
	unsigned int method_matched;               /* Methods matched */
	unsigned int headers_matched[HEADERS_LEN]; /* Headers matched */

	/* Match method */
	method_matched = string_matchlist(data, end, methods);

	if (method_matched == -1)
		return 0;

	memset(headers_matched, 0, sizeof(headers_matched));

	/* Match in headers */
	while ((data = next_line(data, end)))
	{
		int header;

		header = string_matchlist(data, end, headers);

		if (header == EOH)
				break;

		if (header != -1)
			headers_matched[header] = 1;
	}


/*
 * FastTrack
 */
	/* KaZaa < 2.6 */
	if ((MM(MM_GET_HASH) || MM(MM_HTTP11)) && HM(HM_X_KAZAA))
		return IPT_P2P_PROTO_FASTTRACK;

	/* KaZaa >= 2.6 (TODO: needs testing) */
	if ((MM(MM_GET_FILE) || MM(MM_GET_SIG) || MM(MM_HTTP11)) &&
		HM(HM_X_P2P_MESSAGE))
		return IPT_P2P_PROTO_FASTTRACK;

	/* KaZaa passive mode (TODO: Check if methos GIVE is used anywhere else) */
	if (MM(MM_GIVE))
		return IPT_P2P_PROTO_FASTTRACK;

/*
 * Gnutella
 */
	/* Gnutella 1 */
	if ((MM(MM_GET_GET) || MM(MM_GET_URIRES) || MM(MM_HTTP11)) &&
	    (HM(HM_X_GNUTELLA)))
		return IPT_P2P_PROTO_GNUTELLA;

	/* Shareaza/Gnutella 2 */
	if ((MM(MM_GET_URIRES) && (HM(HM_CONTENT_URN) || HM(HM_X_QUEUE))))
		return IPT_P2P_PROTO_GNUTELLA;

	if (MM(MM_HTTP11) && HM(HM_X_TIGER_THREE))
		return IPT_P2P_PROTO_GNUTELLA;

	/* OpenFT */
	if ((MM(MM_GET) || MM(MM_HTTP11)) && (HM(HM_X_OPENFTALIAS)))
		return IPT_P2P_PROTO_OPENFT;

	if (MM(MM_GET_POISONED))
		return IPT_P2P_PROTO_OPENFT;

	return 0;
}
