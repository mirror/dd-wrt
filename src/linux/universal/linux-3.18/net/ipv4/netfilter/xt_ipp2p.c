#include <linux/module.h>
#include <linux/version.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <net/tcp.h>
#include <net/udp.h>
#include <asm/unaligned.h>
#include "xt_ipp2p.h"
#include "compat_xtables.h"

//#define IPP2P_DEBUG_ARES
//#define IPP2P_DEBUG_SOUL
//#define IPP2P_DEBUG_WINMX

#define get_u8(X,  O)  (*(const __u8 *)((X) + O))
#define get_u16(X, O)  get_unaligned((const __u16 *)((X) + O))
#define get_u32(X, O)  get_unaligned((const __u32 *)((X) + O))

MODULE_AUTHOR("Eicke Friedrich/Klaus Degner <ipp2p@ipp2p.org>");
MODULE_DESCRIPTION("An extension to iptables to identify P2P traffic.");
MODULE_LICENSE("GPL");

/* Search for UDP eDonkey/eMule/Kad commands */
static unsigned int
udp_search_edk(const unsigned char *t, const unsigned int packet_len)
{
	if (packet_len < 4)
		return 0;

	switch (t[0]) {
	case 0xe3:
		/* edonkey */
		switch (t[1]) {
		/* client -> server status request */
		case 0x96:
			if (packet_len == 6)
				return IPP2P_EDK * 100 + 50;
			break;

		/* server -> client status request */
		case 0x97:
			if (packet_len == 34)
				return IPP2P_EDK * 100 + 51;
			break;

		/* server description request */
		/* e3 2a ff f0 .. | size == 6 */
		case 0xa2:
			if (packet_len == 6 &&
			    get_u16(t, 2) == __constant_htons(0xfff0))
				return IPP2P_EDK * 100 + 52;
			break;

		/* server description response */
		/* e3 a3 ff f0 ..  | size > 40 && size < 200 */
		/*
		case 0xa3:
			return IPP2P_EDK * 100 + 53;
			break;
		*/

		case 0x9a:
			if (packet_len == 18)
				return IPP2P_EDK * 100 + 54;
			break;

		case 0x92:
			if (packet_len == 10)
				return IPP2P_EDK * 100 + 55;
			break;
		}
		break;

	case 0xe4:
		switch (t[1]) {
		/* e4 20 .. | size == 35 */
		case 0x20:
			if (packet_len == 35 && t[2] != 0x00 && t[34] != 0x00)
				return IPP2P_EDK * 100 + 60;
			break;

		/* e4 00 .. 00 | size == 27 ? */
		case 0x00:
			if (packet_len == 27 && t[26] == 0x00)
				return IPP2P_EDK * 100 + 61;
			break;

		/* e4 10 .. 00 | size == 27 ? */
		case 0x10:
			if (packet_len == 27 && t[26] == 0x00)
				return IPP2P_EDK * 100 + 62;
			break;

		/* e4 18 .. 00 | size == 27 ? */
		case 0x18:
			if (packet_len == 27 && t[26] == 0x00)
				return IPP2P_EDK * 100 + 63;
			break;

		/* e4 52 .. | size = 36 */
		case 0x52:
			if (packet_len == 36)
				return IPP2P_EDK * 100 + 64;
			break;

		/* e4 58 .. | size == 6 */
		case 0x58:
			if (packet_len == 6)
				return IPP2P_EDK * 100 + 65;
			break;

		/* e4 59 .. | size == 2 */
		case 0x59:
			if (packet_len == 2)
				return IPP2P_EDK * 100 + 66;
			break;

		/* e4 28 .. | packet_len == 49,69,94,119... */
		case 0x28:
			if ((packet_len - 44) % 25 == 0)
				return IPP2P_EDK * 100 + 67;
			break;

		/* e4 50 xx xx | size == 4 */
		case 0x50:
			if (packet_len == 4)
				return IPP2P_EDK * 100 + 68;
			break;

		/* e4 40 xx xx | size == 48 */
		case 0x40:
			if (packet_len == 48)
				return IPP2P_EDK * 100 + 69;
			break;
		}
		break;
	}
	return 0;
}

/* Search for UDP Gnutella commands */
static unsigned int
udp_search_gnu(const unsigned char *t, const unsigned int packet_len)
{
	if (packet_len >= 3 && memcmp(t, "GND", 3) == 0)
		return IPP2P_GNU * 100 + 51;
	if (packet_len >= 9 && memcmp(t, "GNUTELLA ", 9) == 0)
		return IPP2P_GNU * 100 + 52;
	return 0;
}

/* Search for UDP KaZaA commands */
static unsigned int
udp_search_kazaa(const unsigned char *t, const unsigned int packet_len)
{
	if (packet_len < 6)
		return 0;
	if (memcmp(t + packet_len - 6, "KaZaA\x00", 6) == 0)
		return IPP2P_KAZAA * 100 + 50;
	return 0;
}

/* Search for UDP DirectConnect commands */
static unsigned int udp_search_directconnect(const unsigned char *t,
                                             const unsigned int packet_len)
{
	if (packet_len < 5)
		return 0;
	if (t[0] == 0x24 && t[packet_len-1] == 0x7c) {
		if (memcmp(&t[1], "SR ", 3) == 0)
			return IPP2P_DC * 100 + 60;
		if (packet_len >= 7 && memcmp(&t[1], "Ping ", 5) == 0)
			return IPP2P_DC * 100 + 61;
	}
	return 0;
}

/* Search for UDP BitTorrent commands */
static unsigned int
udp_search_bit(const unsigned char *haystack, const unsigned int packet_len)
{
	switch (packet_len) {
	case 16:
		/* ^ 00 00 04 17 27 10 19 80 */
		if (ntohl(get_u32(haystack, 0)) == 0x00000417 &&
		    ntohl(get_u32(haystack, 4)) == 0x27101980)
			return IPP2P_BIT * 100 + 50;
		break;
	case 36:
		if (get_u32(haystack, 8) == __constant_htonl(0x00000400) &&
		    get_u32(haystack, 28) == __constant_htonl(0x00000104))
			return IPP2P_BIT * 100 + 51;
		if (get_u32(haystack, 8) == __constant_htonl(0x00000400))
			return IPP2P_BIT * 100 + 61;
		break;
	case 57:
		if (get_u32(haystack, 8) == __constant_htonl(0x00000404) &&
		    get_u32(haystack, 28) == __constant_htonl(0x00000104))
			return IPP2P_BIT * 100 + 52;
		if (get_u32(haystack, 8) == __constant_htonl(0x00000404))
			return IPP2P_BIT * 100 + 62;
		break;
	case 59:
		if (get_u32(haystack, 8) == __constant_htonl(0x00000406) &&
		    get_u32(haystack, 28) == __constant_htonl(0x00000104))
			return (IPP2P_BIT * 100 + 53);
		if (get_u32(haystack, 8) == __constant_htonl(0x00000406))
			return (IPP2P_BIT * 100 + 63);
		break;
	case 203:
		if (get_u32(haystack, 0) == __constant_htonl(0x00000405))
			return IPP2P_BIT * 100 + 54;
		break;
	case 21:
		if (get_u32(haystack, 0) == __constant_htonl(0x00000401))
			return IPP2P_BIT * 100 + 55;
		break;
	case 44:
		if (get_u32(haystack, 0)  == __constant_htonl(0x00000827) &&
		    get_u32(haystack, 4) == __constant_htonl(0x37502950))
			return IPP2P_BIT * 100 + 80;
		break;
	default:
		/* this packet does not have a constant size */
		if (packet_len >= 32 &&
		    get_u32(haystack, 8) == __constant_htonl(0x00000402) &&
		    get_u32(haystack, 28) == __constant_htonl(0x00000104))
			return IPP2P_BIT * 100 + 56;
		break;
	}

	/* some extra-bitcomet rules: "d1:" [a|r] "d2:id20:" */
	if (packet_len > 22 && get_u8(haystack, 0) == 'd' &&
	    get_u8(haystack, 1) == '1' && get_u8(haystack, 2) == ':')
		if (get_u8(haystack, 3) == 'a' ||
		    get_u8(haystack, 3) == 'r')
			if (memcmp(haystack + 4, "d2:id20:", 8) == 0)
				return IPP2P_BIT * 100 + 57;

#if 0
	/* bitlord rules */
	/* packetlen must be bigger than 32 */
	/* first 4 bytes are zero */
	if (packet_len > 32 && get_u32(haystack, 0) == 0x00000000) {
		/* first rule: 00 00 00 00 01 00 00 xx xx xx xx 00 00 00 00*/
		if (get_u32(haystack, 4) == 0x00000000 &&
		    get_u32(haystack, 8) == 0x00010000 &&
		    get_u32(haystack, 16) == 0x00000000)
			return IPP2P_BIT * 100 + 71;

		/* 00 01 00 00 0d 00 00 xx xx xx xx 00 00 00 00*/
		if (get_u32(haystack, 4) == 0x00000001 &&
		    get_u32(haystack, 8) == 0x000d0000 &&
		    get_u32(haystack, 16) == 0x00000000)
			return IPP2P_BIT * 100 + 71;
	}
#endif

	return 0;
}

/* Search for Ares commands */
static unsigned int
search_ares(const unsigned char *payload, const unsigned int plen)
{
	if (plen < 3)
		return 0;
	/* all ares packets start with  */
	if (payload[1] == 0 && plen - payload[0] == 3) {
		switch (payload[2]) {
		case 0x5a:
			/* ares connect */
			if (plen == 6 && payload[5] == 0x05)
				return IPP2P_ARES * 100 + 1;
			break;
		case 0x09:
			/*
			 * ares search, min 3 chars --> 14 bytes
			 * lets define a search can be up to 30 chars
			 * --> max 34 bytes
			 */
			if (plen >= 14 && plen <= 34)
				return IPP2P_ARES * 100 + 1;
			break;
#ifdef IPP2P_DEBUG_ARES
		default:
			printk(KERN_DEBUG "Unknown Ares command %x "
			       "recognized, len: %u\n",
			       (unsigned int)payload[2], plen);
#endif
		}
	}

#if 0
	/* found connect packet: 03 00 5a 04 03 05 */
	/* new version ares 1.8: 03 00 5a xx xx 05 */
	if (plen == 6)
		/* possible connect command */
		if (payload[0] == 0x03 && payload[1] == 0x00 &&
		    payload[2] == 0x5a && payload[5] == 0x05)
			return IPP2P_ARES * 100 + 1;

	if (plen == 60)
		/* possible download command*/
		if (payload[59] == 0x0a && payload[58] == 0x0a)
			if (memcmp(t, "PUSH SHA1:", 10) == 0)
				/* found download command */
				return IPP2P_ARES * 100 + 2;
#endif

	return 0;
}

/* Search for SoulSeek commands */
static unsigned int
search_soul(const unsigned char *payload, const unsigned int plen)
{
	if (plen < 8)
		return 0;
	/* match: xx xx xx xx | xx = sizeof(payload) - 4 */
	if (get_u32(payload, 0) == plen - 4) {
		const uint32_t m = get_u32(payload, 4);

		/* match 00 yy yy 00, yy can be everything */
		if (get_u8(payload, 4) == 0x00 && get_u8(payload, 7) == 0x00) {
#ifdef IPP2P_DEBUG_SOUL
			printk(KERN_DEBUG "0: Soulseek command 0x%x "
			       "recognized\n", get_u32(payload, 4));
#endif
			return IPP2P_SOUL * 100 + 1;
		}

		/* next match: 01 yy 00 00 | yy can be everything */
		if (get_u8(payload, 4) == 0x01 && get_u16(payload, 6) == 0x0000) {
#ifdef IPP2P_DEBUG_SOUL
			printk(KERN_DEBUG "1: Soulseek command 0x%x "
			       "recognized\n", get_u16(payload, 4));
#endif
			return IPP2P_SOUL * 100 + 2;
		}

		/* other soulseek commandos are: 1-5,7,9,13-18,22,23,26,28,35-37,40-46,50,51,60,62-69,91,92,1001 */
		/* try to do this in an intelligent way */
		/* get all small commandos */
		switch (m) {
		case 7:
		case 9:
		case 22:
		case 23:
		case 26:
		case 28:
		case 50:
		case 51:
		case 60:
		case 91:
		case 92:
		case 1001:
#ifdef IPP2P_DEBUG_SOUL
			printk(KERN_DEBUG "2: Soulseek command 0x%x "
			       "recognized\n", get_u16(payload, 4));
#endif
			return IPP2P_SOUL * 100 + 3;
		}

		if (m > 0 && m < 6) {
#ifdef IPP2P_DEBUG_SOUL
			printk(KERN_DEBUG "3: Soulseek command 0x%x "
			       "recognized\n", get_u16(payload, 4));
#endif
			return IPP2P_SOUL * 100 + 4;
		}

		if (m > 12 && m < 19) {
#ifdef IPP2P_DEBUG_SOUL
			printk(KERN_DEBUG "4: Soulseek command 0x%x "
			       "recognized\n", get_u16(payload, 4));
#endif
			return IPP2P_SOUL * 100 + 5;
		}

		if (m > 34 && m < 38) {
#ifdef IPP2P_DEBUG_SOUL
			printk(KERN_DEBUG "5: Soulseek command 0x%x "
			       "recognized\n", get_u16(payload, 4));
#endif
			return IPP2P_SOUL * 100 + 6;
		}

		if (m > 39 && m < 47) {
#ifdef IPP2P_DEBUG_SOUL
			printk(KERN_DEBUG "6: Soulseek command 0x%x "
			       "recognized\n", get_u16(payload, 4));
#endif
			return IPP2P_SOUL * 100 + 7;
		}

		if (m > 61 && m < 70) {
#ifdef IPP2P_DEBUG_SOUL
			printk(KERN_DEBUG "7: Soulseek command 0x%x "
			       "recognized\n", get_u16(payload, 4));
#endif
			return IPP2P_SOUL * 100 + 8;
		}

#ifdef IPP2P_DEBUG_SOUL
		printk(KERN_DEBUG "unknown SOULSEEK command: 0x%x, first "
		       "16 bit: 0x%x, first 8 bit: 0x%x ,soulseek ???\n",
		       get_u32(payload, 4), get_u16(payload, 4) >> 16,
		       get_u8(payload, 4) >> 24);
#endif
	}

	/* match 14 00 00 00 01 yy 00 00 00 STRING(YY) 01 00 00 00 00 46|50 00 00 00 00 */
	/* without size at the beginning !!! */
	if (get_u32(payload, 0) == 0x14 && get_u8(payload, 4) == 0x01) {
		uint32_t y = get_u32(payload, 5);

		/* we need 19 chars + string */
		if (y + 19 <= plen) {
			const unsigned char *w = payload + 9 + y;
			if (get_u32(w, 0) == 0x01 &&
			    (get_u16(w, 4) == 0x4600 ||
			    get_u16(w, 4) == 0x5000) &&
			    get_u32(w, 6) == 0x00)
				;
#ifdef IPP2P_DEBUG_SOUL
	    		printk(KERN_DEBUG "Soulssek special client command recognized\n");
#endif
	    		return IPP2P_SOUL * 100 + 9;
		}
	}
	return 0;
}

/* Search for WinMX commands */
static unsigned int
search_winmx(const unsigned char *payload, const unsigned int plen)
{
	if (plen == 4 && memcmp(payload, "SEND", 4) == 0)
		return IPP2P_WINMX * 100 + 1;
	if (plen == 3 && memcmp(payload, "GET", 3) == 0)
		return IPP2P_WINMX * 100 + 2;
	/*
	if (packet_len < head_len + 10)
		return 0;
	*/
	if (plen < 10)
		return 0;

	if (memcmp(payload, "SEND", 4) == 0 || memcmp(payload, "GET", 3) == 0) {
		uint16_t c = 4;
		const uint16_t end = plen - 2;
		uint8_t count = 0;

		while (c < end) {
			if (payload[c] == 0x20 && payload[c+1] == 0x22) {
				c++;
				count++;
				if (count >= 2)
					return IPP2P_WINMX * 100 + 3;
			}
			c++;
		}
	}

	if (plen == 149 && payload[0] == '8') {
#ifdef IPP2P_DEBUG_WINMX
		printk(KERN_INFO "maybe WinMX\n");
#endif
		if (get_u32(payload, 17) == 0 && get_u32(payload, 21) == 0 &&
		    get_u32(payload, 25) == 0 &&
//		    get_u32(payload, 33) == __constant_htonl(0x71182b1a) &&
//		    get_u32(payload, 37) == __constant_htonl(0x05050000) &&
//		    get_u32(payload, 133) == __constant_htonl(0x31097edf) &&
//		    get_u32(payload, 145) == __constant_htonl(0xdcb8f792))
		    get_u16(payload, 39) == 0 &&
		    get_u16(payload, 135) == __constant_htons(0x7edf) &&
		    get_u16(payload,147) == __constant_htons(0xf792))
		{
#ifdef IPP2P_DEBUG_WINMX
			printk(KERN_INFO "got WinMX\n");
#endif
			return IPP2P_WINMX * 100 + 4;
		}
	}
	return 0;
}

/* Search for appleJuice commands */
static unsigned int
search_apple(const unsigned char *payload, const unsigned int plen)
{
	if (plen > 7 && payload[6] == 0x0d && payload[7] == 0x0a &&
	    memcmp(payload, "ajprot", 6) == 0)
		return IPP2P_APPLE * 100;

	return 0;
}

/* Search for BitTorrent commands */
static unsigned int
search_bittorrent(const unsigned char *payload, const unsigned int plen)
{
	if (plen > 20) {
		/* test for match 0x13+"BitTorrent protocol" */
		if (payload[0] == 0x13)
			if (memcmp(payload + 1, "BitTorrent protocol", 19) == 0)
				return IPP2P_BIT * 100;
		/*
		 * Any tracker command starts with GET / then *may be* some file on web server
		 * (e.g. announce.php or dupa.pl or whatever.cgi or NOTHING for tracker on root dir)
		 * but *must have* one (or more) of strings listed below (true for scrape and announce)
		 */
		if (memcmp(payload, "GET /", 5) == 0) {
			if (HX_memmem(payload, plen, "info_hash=", 9) != NULL)
				return IPP2P_BIT * 100 + 1;
			if (HX_memmem(payload, plen, "peer_id=", 8) != NULL)
				return IPP2P_BIT * 100 + 2;
			if (HX_memmem(payload, plen, "passkey=", 8) != NULL)
				return IPP2P_BIT * 100 + 4;
		}
	} else {
	    	/* bitcomet encryptes the first packet, so we have to detect another
	    	 * one later in the flow */
		/* first try failed, too many false positives */
	    	/*
		if (size == 5 && get_u32(t, 0) == __constant_htonl(1) &&
		    t[4] < 3)
			return IPP2P_BIT * 100 + 3;
		*/

	    	/* second try: block request packets */
	    	if (plen == 17 &&
		    get_u32(payload, 0) == __constant_htonl(0x0d) &&
		    payload[4] == 0x06 &&
		    get_u32(payload,13) == __constant_htonl(0x4000))
			return IPP2P_BIT * 100 + 3;
	}

	return 0;
}

/* check for Kazaa get command */
static unsigned int
search_kazaa(const unsigned char *payload, const unsigned int plen)
{
	if (plen < 13)
		return 0;
	if (payload[plen-2] == 0x0d && payload[plen-1] == 0x0a &&
	    memcmp(payload, "GET /.hash=", 11) == 0)
		return IPP2P_DATA_KAZAA * 100;

	return 0;
}

/* check for gnutella get command */
static unsigned int
search_gnu(const unsigned char *payload, const unsigned int plen)
{
	if (plen < 11)
		return 0;
	if (payload[plen-2] == 0x0d && payload[plen-1] == 0x0a) {
		if (memcmp(payload, "GET /get/", 9) == 0)
			return IPP2P_DATA_GNU * 100 + 1;
		if (plen >= 15 && memcmp(payload, "GET /uri-res/", 13) == 0)
			return IPP2P_DATA_GNU * 100 + 2;
	}
	return 0;
}

/* check for gnutella get commands and other typical data */
static unsigned int
search_all_gnu(const unsigned char *payload, const unsigned int plen)
{
	if (plen < 11)
		return 0;
	if (payload[plen-2] == 0x0d && payload[plen-1] == 0x0a) {
		if (plen >= 19 && memcmp(payload, "GNUTELLA CONNECT/", 17) == 0)
			return IPP2P_GNU * 100 + 1;
		if (memcmp(payload, "GNUTELLA/", 9) == 0)
			return IPP2P_GNU * 100 + 2;

		if (plen >= 22 && (memcmp(payload, "GET /get/", 9) == 0 ||
		    memcmp(payload, "GET /uri-res/", 13) == 0))
		{
			unsigned int c;

			for (c = 0; c < plen - 22; ++c)
				if (payload[c] == 0x0d &&
				    payload[c+1] == 0x0a &&
				    (memcmp(&payload[c+2], "X-Gnutella-", 11) == 0 ||
				    memcmp(&payload[c+2], "X-Queue:", 8) == 0))
					return IPP2P_GNU * 100 + 3;
		}
	}
	return 0;
}

/* check for KaZaA download commands and other typical data */
/* plen is guaranteed to be >= 5 (see @matchlist) */
static unsigned int
search_all_kazaa(const unsigned char *payload, const unsigned int plen)
{
	uint16_t c, end, rem;

	if (plen < 7)
		/* too short for anything we test for - early bailout */
		return 0;

	if (payload[plen-2] != 0x0d || payload[plen-1] != 0x0a)
		return 0;

	if (memcmp(payload, "GIVE ", 5) == 0)
		return IPP2P_KAZAA * 100 + 1;

	if (memcmp(payload, "GET /", 5) != 0)
		return 0;

	if (plen < 18)
		/* The next tests would not succeed anyhow. */
		return 0;

	end = plen - 18;
	rem = plen - 5;
	for (c = 5; c < end; ++c, --rem) {
		if (payload[c] != 0x0d)
			continue;
		if (payload[c+1] != 0x0a)
			continue;
		if (rem >= 18 &&
		    memcmp(&payload[c+2], "X-Kazaa-Username: ", 18) == 0)
			return IPP2P_KAZAA * 100 + 2;
		if (rem >= 24 &&
		    memcmp(&payload[c+2], "User-Agent: PeerEnabler/", 24) == 0)
			return IPP2P_KAZAA * 100 + 2;
	}

	return 0;
}

/* fast check for edonkey file segment transfer command */
static unsigned int
search_edk(const unsigned char *payload, const unsigned int plen)
{
	if (plen < 6)
		return 0;
	if (payload[0] != 0xe3) {
		return 0;
	} else {
		if (payload[5] == 0x47)
			return IPP2P_DATA_EDK * 100;
		else
			return 0;
	}
}

/* intensive but slower search for some edonkey packets including size-check */
static unsigned int
search_all_edk(const unsigned char *payload, const unsigned int plen)
{
	if (plen < 6)
		return 0;
	if (payload[0] != 0xe3) {
		return 0;
	} else {
		unsigned int cmd = get_u16(payload, 1);

		if (cmd == plen - 5) {
			switch (payload[5]) {
			case 0x01:
				/* Client: hello or Server:hello */
			return IPP2P_EDK * 100 + 1;
				case 0x4c:
				/* Client: Hello-Answer */
				return IPP2P_EDK * 100 + 9;
			}
		}
		return 0;
	}
}

/* fast check for Direct Connect send command */
static unsigned int
search_dc(const unsigned char *payload, const unsigned int plen)
{
	if (plen < 6)
		return 0;
	if (payload[0] != 0x24) {
		return 0;
	} else {
		if (memcmp(&payload[1], "Send|", 5) == 0)
			return IPP2P_DATA_DC * 100;
		else
			return 0;
	}
}

/* intensive but slower check for all direct connect packets */
static unsigned int
search_all_dc(const unsigned char *payload, const unsigned int plen)
{
	if (plen < 7)
		return 0;
	if (payload[0] == 0x24 && payload[plen-1] == 0x7c) {
		const unsigned char *t = &payload[1];

		/* Client-Hub-Protocol */
		if (memcmp(t, "Lock ", 5) == 0)
			return IPP2P_DC * 100 + 1;

		/*
		 * Client-Client-Protocol, some are already recognized by
		 * client-hub (like lock)
		 */
		if (plen >= 9 && memcmp(t, "MyNick ", 7) == 0)
			return IPP2P_DC * 100 + 38;
	}
	return 0;
}

/* check for mute */
static unsigned int
search_mute(const unsigned char *payload, const unsigned int plen)
{
	if (plen == 209 || plen == 345 || plen == 473 || plen == 609 ||
	    plen == 1121) {
		//printk(KERN_DEBUG "size hit: %u", size);
		if (memcmp(payload,"PublicKey: ", 11) == 0) {
			return IPP2P_MUTE * 100 + 0;
			/*
			if (memcmp(t + size - 14, "\x0aEndPublicKey\x0a", 14) == 0)
				printk(KERN_DEBUG "end pubic key hit: %u", size);
			*/
		}
	}
	return 0;
}

/* check for xdcc */
static unsigned int
search_xdcc(const unsigned char *payload, const unsigned int plen)
{
	/* search in small packets only */
	if (plen > 20 && plen < 200 && payload[plen-1] == 0x0a &&
	    payload[plen-2] == 0x0d && memcmp(payload, "PRIVMSG ", 8) == 0)
	{
		uint16_t x = 10;
		const uint16_t end = plen - 13;

		/*
		 * is seems to be a irc private massage, chedck for
		 * xdcc command
		 */
		while (x < end)	{
			if (payload[x] == ':')
				if (memcmp(&payload[x+1], "xdcc send #", 11) == 0)
					return IPP2P_XDCC * 100 + 0;
			x++;
		}
	}
	return 0;
}

/* search for waste */
static unsigned int
search_waste(const unsigned char *payload, const unsigned int plen)
{
	if (plen >= 8 && memcmp(payload, "GET.sha1:", 9) == 0)
		return IPP2P_WASTE * 100 + 0;

	return 0;
}

static const struct {
	unsigned int command;
	unsigned int packet_len;
	unsigned int (*function_name)(const unsigned char *, const unsigned int);
} matchlist[] = {
	{IPP2P_EDK,         20, search_all_edk},
	{IPP2P_DATA_KAZAA, 200, search_kazaa}, /* exp */
	{IPP2P_DATA_EDK,    60, search_edk}, /* exp */
	{IPP2P_DATA_DC,     26, search_dc}, /* exp */
	{IPP2P_DC,           5, search_all_dc},
	{IPP2P_DATA_GNU,    40, search_gnu}, /* exp */
	{IPP2P_GNU,          5, search_all_gnu},
	{IPP2P_KAZAA,        5, search_all_kazaa},
	{IPP2P_BIT,         20, search_bittorrent},
	{IPP2P_APPLE,        5, search_apple},
	{IPP2P_SOUL,         5, search_soul},
	{IPP2P_WINMX,        2, search_winmx},
	{IPP2P_ARES,         5, search_ares},
	{IPP2P_MUTE,       200, search_mute},
	{IPP2P_WASTE,        5, search_waste},
	{IPP2P_XDCC,         5, search_xdcc},
	{0},
};

static const struct {
	unsigned int command;
	unsigned int packet_len;
	unsigned int (*function_name)(const unsigned char *, const unsigned int);
} udp_list[] = {
	{IPP2P_KAZAA, 14, udp_search_kazaa},
	{IPP2P_BIT,   23, udp_search_bit},
	{IPP2P_GNU,   11, udp_search_gnu},
	{IPP2P_EDK,    9, udp_search_edk},
	{IPP2P_DC,    12, udp_search_directconnect},
	{0},
};

static bool
ipp2p_mt(const struct sk_buff *skb, struct xt_action_param *par)
{
	const struct ipt_p2p_info *info = par->matchinfo;
	const unsigned char  *haystack;
	const struct iphdr *ip = ip_hdr(skb);
	bool p2p_result = false;
	int i = 0;
	unsigned int hlen = ntohs(ip->tot_len) - ip_hdrlen(skb);	/* hlen = packet-data length */

	/* must not be a fragment */
	if (par->fragoff != 0) {
		if (info->debug)
			printk("IPP2P.match: offset found %d\n", par->fragoff);
		return 0;
	}

	/* make sure that skb is linear */
	if (skb_is_nonlinear(skb)) {
		if (info->debug)
			printk("IPP2P.match: nonlinear skb found\n");
		return 0;
	}

	haystack = skb_network_header(skb) + ip_hdrlen(skb);

	switch (ip->protocol) {
	case IPPROTO_TCP:	/* what to do with a TCP packet */
	{
		const struct tcphdr *tcph = (const void *)ip + ip_hdrlen(skb);

		if (tcph->fin) return 0;  /* if FIN bit is set bail out */
		if (tcph->syn) return 0;  /* if SYN bit is set bail out */
		if (tcph->rst) return 0;  /* if RST bit is set bail out */

		haystack += tcph->doff * 4; /* get TCP-Header-Size */
		if (tcph->doff * 4 > hlen) {
			if (info->debug)
				pr_info("TCP header indicated packet larger than it is\n");
			hlen = 0;
		} else {
			hlen -= tcph->doff * 4;
		}
		while (matchlist[i].command) {
			if ((info->cmd & matchlist[i].command) == matchlist[i].command &&
			    hlen > matchlist[i].packet_len)
			{
				p2p_result = matchlist[i].function_name(haystack, hlen);
				if (p2p_result)	{
					if (info->debug)
						printk("IPP2P.debug:TCP-match: %i from: %u.%u.%u.%u:%i to: %u.%u.%u.%u:%i Length: %i\n",
						       p2p_result, NIPQUAD(ip->saddr),ntohs(tcph->source), NIPQUAD(ip->daddr),ntohs(tcph->dest),hlen);
					return p2p_result;
				}
			}
			i++;
		}
		return p2p_result;
	}

	case IPPROTO_UDP:	/* what to do with an UDP packet */
	case IPPROTO_UDPLITE:
	{
		const struct udphdr *udph = (const void *)ip + ip_hdrlen(skb);

		haystack += sizeof(*udph);
		if (sizeof(*udph) > hlen) {
			if (info->debug)
				pr_info("UDP header indicated packet larger than it is\n");
			hlen = 0;
		} else {
			hlen -= sizeof(*udph);
		}

		while (udp_list[i].command) {
			if ((info->cmd & udp_list[i].command) == udp_list[i].command &&
			    hlen > udp_list[i].packet_len)
			{
				p2p_result = udp_list[i].function_name(haystack, hlen);
				if (p2p_result) {
					if (info->debug)
						printk("IPP2P.debug:UDP-match: %i from: %u.%u.%u.%u:%i to: %u.%u.%u.%u:%i Length: %i\n",
						       p2p_result, NIPQUAD(ip->saddr), ntohs(udph->source), NIPQUAD(ip->daddr), ntohs(udph->dest), hlen);
					return p2p_result;
				}
			}
			i++;
		}
		return p2p_result;
	}

	default:
		return 0;
	}
}

static struct xt_match ipp2p_mt_reg __read_mostly = {
	.name       = "ipp2p",
	.revision   = 1,
	.family     = NFPROTO_IPV4,
	.match      = ipp2p_mt,
	.matchsize  = sizeof(struct ipt_p2p_info),
	.me         = THIS_MODULE,
};

static int __init ipp2p_mt_init(void)
{
	return xt_register_match(&ipp2p_mt_reg);
}

static void __exit ipp2p_mt_exit(void)
{
	xt_unregister_match(&ipp2p_mt_reg);
}

module_init(ipp2p_mt_init);
module_exit(ipp2p_mt_exit);
MODULE_ALIAS("ipt_ipp2p");
