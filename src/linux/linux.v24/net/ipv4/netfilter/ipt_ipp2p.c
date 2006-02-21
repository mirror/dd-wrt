#if defined(MODVERSIONS)
#include <linux/modversions.h>
#endif
#include <linux/module.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/version.h>
//#include <linux/netfilter_ipv4/ipt_ipp2p.h>
#include "ipt_ipp2p.h"
#include <net/tcp.h>
#include <net/udp.h>

#define get_u8(X,O)  (*(__u8 *)(X + O))
#define get_u16(X,O)  (*(__u16 *)(X + O))
#define get_u32(X,O)  (*(__u32 *)(X + O))

MODULE_AUTHOR("Eicke Friedrich/Klaus Degner <ipp2p@ipp2p.org>");
MODULE_DESCRIPTION("An extension to iptables to identify P2P traffic.");
MODULE_LICENSE("GPL");


/*Search for UDP eDonkey/eMule/Kad commands*/
int
udp_search_edk (unsigned char *haystack, int packet_len)
{
    unsigned char *t = haystack;
    t += 8;

    switch (t[0]) {
    case 0xe3: {	/*edonkey*/
	switch (t[1]) {
			/* e3 9a + 16Bytes Hash | size == 10 + x* 16 byte hashes */
	case 0x9a: if ( ( packet_len - 10) % 16 == 0) return ((IPP2P_EDK * 100) + 50);
			/* e3 96 xx yy zz kk | size == 14 | server status request */
	case 0x96: if (packet_len == 14) return ((IPP2P_EDK * 100) + 51);
			/* e3 94 | size is 30, 50, 70, 90,...*/
	case 0x94: if ((packet_len-10) % 20 == 0) return ((IPP2P_EDK * 100) + 52);
			/* e3 a2 | size == 10 or 14 <-- recheck*/
	}
    }

    case 0xc5: {	/*emule*/
	switch (t[1]) {
			/* c5 91 xx yy | size == 12 (8+4) | xx != 0x00  -- xx yy queue rating */
	case 0x91: if ((packet_len == 12) && (t[2] != 0x00)) return ((IPP2P_EDK * 100) + 53);
			/* c5 90 xx ..  yy | size == 26 (8+2+16) | xx .. yy == hash  -- file ping */
	case 0x90: if ((packet_len == 26) && (t[2] != 0x00)) return ((IPP2P_EDK * 100) + 54);
			/* c5 92 | size == 10 (8+2) -- file not found */
	case 0x92: if (packet_len == 10) return ((IPP2P_EDK * 100) + 55);
			/* c5 93 | size == 10 (8+2) -- queue full */
	case 0x93: if (packet_len == 10) return ((IPP2P_EDK * 100) + 56);
	}
    }

    case 0xe4: {	/*kad*/
	switch (t[1]) {
			/* e4 50 | size == 12 */
	    case 0x50: if (packet_len == 12) return ((IPP2P_EDK * 100) + 57);
			/* e4 58 | size == 14 */
	    case 0x58: if ((packet_len == 14) && (t[2] != 0x00)) return ((IPP2P_EDK * 100) + 58);
			/* e4 59 | size == 10 */
	    case 0x59: if (packet_len == 10) return ((IPP2P_EDK * 100) + 59);
			/* e4 30 .. | t[18] == 0x01 | size > 26 | --> search */
	    case 0x30: if ((packet_len > 26) && (t[18] == 0x01)) return ((IPP2P_EDK * 100) + 60);
			/* e4 28 .. 00 | t[68] == 0x00 | size > 76 */
	    case 0x28: if ((packet_len > 76) && (t[68] == 0x00)) return ((IPP2P_EDK * 100) + 61);
			/* e4 20 .. | size == 43 */
	    case 0x20: if ((packet_len == 43) && (t[2] != 0x00) && (t[34] != 0x00)) return ((IPP2P_EDK * 100) + 62);
			/* e4 00 .. 00 | size == 35 ? */
	    case 0x00: if ((packet_len == 35) && (t[26] == 0x00)) return ((IPP2P_EDK * 100) + 63);
			/* e4 10 .. 00 | size == 35 ? */
	    case 0x10: if ((packet_len == 35) && (t[26] == 0x00)) return ((IPP2P_EDK * 100) + 64);
			/* e4 18 .. 00 | size == 35 ? */
	    case 0x18: if ((packet_len == 35) && (t[26] == 0x00)) return ((IPP2P_EDK * 100) + 65);
			/* e4 40 .. | t[18] == 0x01 | t[19] == 0x00 | size > 40 */
	    case 0x40: if ((packet_len > 40) && (t[18] == 0x01) && (t[19] == 0x00)) return ((IPP2P_EDK * 100) + 66);
	    		/* e4 52 .. | size = 44 */
	    case 0x52: if (packet_len == 44 )return ((IPP2P_EDK * 100) + 67);
	}
    }
    } /* end of switch (t[0]) */
    
    return 0;
}/*udp_search_edk*/


/*Search for UDP Gnutella commands*/
int
udp_search_gnu (unsigned char *haystack, int packet_len)
{
    unsigned char *t = haystack;
    t += 8;
    
    if (memcmp(t, "GND", 3) == 0) return ((IPP2P_GNU * 100) + 51);
    if (memcmp(t, "GNUTELLA ", 9) == 0) return ((IPP2P_GNU * 100) + 52);
    return 0;
}/*udp_search_gnu*/


/*Search for UDP KaZaA commands*/
int
udp_search_kazaa (unsigned char *haystack, int packet_len)
{
    unsigned char *t = haystack;
    
    if (t[packet_len-1] == 0x00){
	t += (packet_len - 6);
	if (memcmp(t, "KaZaA", 5) == 0) return (IPP2P_KAZAA * 100 +50);
    }
    
    return 0;
}/*udp_search_kazaa*/

/*Search for UDP DirectConnect commands*/
int
udp_search_directconnect (unsigned char *haystack, int packet_len)
{
    unsigned char *t = haystack;
    if ((*(t + 8) == 0x24) && (*(t + packet_len - 1) == 0x7c)) {
    	t+=8;
    	if (memcmp(t, "SR ", 3) == 0)	 		return ((IPP2P_DC * 100) + 60);
    	if (memcmp(t, "Ping ", 5) == 0)	 		return ((IPP2P_DC * 100) + 61);
    }
    return 0;
}/*udp_search_directconnect*/



/*Search for UDP BitTorrent commands*/
int
udp_search_bit (unsigned char *haystack, int packet_len)
{
    switch(packet_len)
    {
	case 24:
		/* ^ 00 00 04 17 27 10 19 80 */
		if ((ntohl(get_u32(haystack, 8)) == 0x00000417) && (ntohl(get_u32(haystack, 12)) == 0x27101980)) 
			return (IPP2P_BIT * 100 + 50);
		break;
	case 44:
		if (get_u32(haystack, 16) == __constant_htonl(0x00000400) && get_u32(haystack, 36) == __constant_htonl(0x00000104)) 
			return (IPP2P_BIT * 100 + 51);
		if (get_u32(haystack, 16) == __constant_htonl(0x00000400))
			return (IPP2P_BIT * 100 + 61);
		break;
	case 65:
		if (get_u32(haystack, 16) == __constant_htonl(0x00000404) && get_u32(haystack, 36) == __constant_htonl(0x00000104)) 
			return (IPP2P_BIT * 100 + 52);
		if (get_u32(haystack, 16) == __constant_htonl(0x00000404))
			return (IPP2P_BIT * 100 + 62);
		break;
	case 67:
		if (get_u32(haystack, 16) == __constant_htonl(0x00000406) && get_u32(haystack, 36) == __constant_htonl(0x00000104)) 
			return (IPP2P_BIT * 100 + 53);
		if (get_u32(haystack, 16) == __constant_htonl(0x00000406))
			return (IPP2P_BIT * 100 + 63);
		break;
	case 211:
		if (get_u32(haystack, 8) == __constant_htonl(0x00000405)) 
			return (IPP2P_BIT * 100 + 54);
		break;
	case 29:
		if ((get_u32(haystack, 8) == __constant_htonl(0x00000401))) 
			return (IPP2P_BIT * 100 + 55);
		break;
	default:
		/* this packet doe not have a constant size */
		if (get_u32(haystack, 16) == __constant_htonl(0x00000402) && get_u32(haystack, 36) == __constant_htonl(0x00000104)) 
			return (IPP2P_BIT * 100 + 56);
		break;
    }
    
    /* some extra-bitcomet rules:
     * "d1:" [a|r] "d2:id20:"
     */
    if (packet_len > 30 && get_u8(haystack, 8) == 'd' && get_u8(haystack, 9) == '1' && get_u8(haystack, 10) == ':' )
    {
    	if (get_u8(haystack, 11) == 'a' || get_u8(haystack, 11) == 'r')
	{
		if (memcmp(haystack+12,"d2:id20:",8)==0)
			return (IPP2P_BIT * 100 + 57);
	}
    }
    return 0;
}/*udp_search_bit*/



/*Search for Ares commands*/
//#define IPP2P_DEBUG_ARES

int
search_ares (unsigned char *haystack, int packet_len, int head_len)
{
	const unsigned char *t = haystack + head_len;
	
	/* all ares packets start with  */
	if (t[1] == 0 && (packet_len - head_len - t[0]) == 3)
	{
		const u16 size=packet_len - head_len;
		switch (t[2])
		{
			case 0x5a:
				/* ares connect */
				if ( size == 6 && t[5] == 0x05 ) return ((IPP2P_ARES * 100) + 1);
				break;
			case 0x09:
				/* ares search, min 3 chars --> 14 bytes
				 * lets define a search can be up to 30 chars --> max 34 bytes
				 */
				if ( size >= 14 && size <= 34 ) return ((IPP2P_ARES * 100) + 1);
				break;
#ifdef IPP2P_DEBUG_ARES
			default:
			printk(KERN_DEBUG "Unknown Ares command %x recognized, len: %u \n", (unsigned int) t[2],size);
#endif /* IPP2P_DEBUG_ARES */
		}
	}

#if 0	        
	/* found connect packet: 03 00 5a 04 03 05 */
	/* new version ares 1.8: 03 00 5a xx xx 05 */
    if ((packet_len - head_len) == 6){	/* possible connect command*/
	if ((t[0] == 0x03) && (t[1] == 0x00) && (t[2] == 0x5a) && (t[5] == 0x05))
	    return ((IPP2P_ARES * 100) + 1);
    }
    if ((packet_len - head_len) == 60){	/* possible download command*/
	if ((t[59] == 0x0a) && (t[58] == 0x0a)){
	    if (memcmp(t, "PUSH SHA1:", 10) == 0) /* found download command */
	    	return ((IPP2P_ARES * 100) + 2);
	}
    }
#endif

    return 0;
} /*search_ares*/

/*Search for SoulSeek commands*/
int
search_soul (unsigned char *haystack, int packet_len, int head_len)
{
    unsigned char *t = haystack;
    t += head_len;
//#define IPP2P_DEBUG_SOUL
    /* match: xx xx xx xx | xx = sizeof(payload) - 4 */
    if (get_u32(t, 0) == (packet_len - head_len - 4)){
	const __u32 m=get_u32(t, 4);
	/* match 00 yy yy 00, yy can be everything */
        if ( get_u8(t, 4) == 0x00 && get_u8(t, 7) == 0x00 )
	{
#ifdef IPP2P_DEBUG_SOUL
	printk(KERN_DEBUG "0: Soulseek command 0x%x recognized\n",get_u32(t, 4));
#endif /* IPP2P_DEBUG_SOUL */
		return ((IPP2P_SOUL * 100) + 1);
	}
	
        /* next match: 01 yy 00 00 | yy can be everything */
        if ( get_u8(t, 4) == 0x01 && get_u16(t, 6) == 0x0000 )
	{
#ifdef IPP2P_DEBUG_SOUL
	printk(KERN_DEBUG "1: Soulseek command 0x%x recognized\n",get_u16(t, 4));
#endif /* IPP2P_DEBUG_SOUL */
		return ((IPP2P_SOUL * 100) + 2);
	}
	
	/* other soulseek commandos are: 1-5,7,9,13-18,22,23,26,28,35-37,40-46,50,51,60,62-69,91,92,1001 */
	/* try to do this in an intelligent way */
	/* get all small commandos */
	switch(m)
	{
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
		printk(KERN_DEBUG "2: Soulseek command 0x%x recognized\n",get_u16(t, 4));
#endif /* IPP2P_DEBUG_SOUL */
		return ((IPP2P_SOUL * 100) + 3);
	}
	
	if (m > 0 && m < 6 ) 
	{
#ifdef IPP2P_DEBUG_SOUL
		printk(KERN_DEBUG "3: Soulseek command 0x%x recognized\n",get_u16(t, 4));
#endif /* IPP2P_DEBUG_SOUL */
		return ((IPP2P_SOUL * 100) + 4);
	}
	if (m > 12 && m < 19 )
	{
#ifdef IPP2P_DEBUG_SOUL
		printk(KERN_DEBUG "4: Soulseek command 0x%x recognized\n",get_u16(t, 4));
#endif /* IPP2P_DEBUG_SOUL */
		return ((IPP2P_SOUL * 100) + 5);
	}

	if (m > 34 && m < 38 )
	{
#ifdef IPP2P_DEBUG_SOUL
		printk(KERN_DEBUG "5: Soulseek command 0x%x recognized\n",get_u16(t, 4));
#endif /* IPP2P_DEBUG_SOUL */
		return ((IPP2P_SOUL * 100) + 6);
	}

	if (m > 39 && m < 47 )
	{
#ifdef IPP2P_DEBUG_SOUL
		printk(KERN_DEBUG "6: Soulseek command 0x%x recognized\n",get_u16(t, 4));
#endif /* IPP2P_DEBUG_SOUL */
		return ((IPP2P_SOUL * 100) + 7);
	}

	if (m > 61 && m < 70 ) 
	{
#ifdef IPP2P_DEBUG_SOUL
		printk(KERN_DEBUG "7: Soulseek command 0x%x recognized\n",get_u16(t, 4));
#endif /* IPP2P_DEBUG_SOUL */
		return ((IPP2P_SOUL * 100) + 8);
	}

#ifdef IPP2P_DEBUG_SOUL
	printk(KERN_DEBUG "unknown SOULSEEK command: 0x%x, first 16 bit: 0x%x, first 8 bit: 0x%x ,soulseek ???\n",get_u32(t, 4),get_u16(t, 4) >> 16,get_u8(t, 4) >> 24);
#endif /* IPP2P_DEBUG_SOUL */
    }
	
	/* match 14 00 00 00 01 yy 00 00 00 STRING(YY) 01 00 00 00 00 46|50 00 00 00 00 */
	/* without size at the beginning !!! */
	if ( get_u32(t, 0) == 0x14 && get_u8(t, 4) == 0x01 )
	{
		__u32 y=get_u32(t, 5);
		/* we need 19 chars + string */
		if ( (y + 19) <= (packet_len - head_len) )
		{
			__u8 *w=t+9+y;
			if (get_u32(w, 0) == 0x01 && ( get_u16(w, 4) == 0x4600 || get_u16(w, 4) == 0x5000) && get_u32(w, 6) == 0x00);
#ifdef IPP2P_DEBUG_SOUL
	    		printk(KERN_DEBUG "Soulssek special client command recognized\n");
#endif /* IPP2P_DEBUG_SOUL */
	    		return ((IPP2P_SOUL * 100) + 9);
		}
	}
    return 0;
}


/*Search for WinMX commands*/
int
search_winmx (unsigned char *haystack, int packet_len, int head_len)
{
    unsigned char *t = haystack;
    int c;
    t += head_len;

    if (((packet_len - head_len) == 4) && (memcmp(t, "SEND", 4) == 0))  return ((IPP2P_WINMX * 100) + 1);
    if (((packet_len - head_len) == 3) && (memcmp(t, "GET", 3) == 0))  return ((IPP2P_WINMX * 100) + 2);
    if (packet_len < (head_len + 10)) return 0;

    if ((memcmp(t, "SEND", 4) == 0) || (memcmp(t, "GET", 3) == 0)){
        c = head_len + 4;
	t += 4;
	while (c < packet_len - 5) {
	    if ((t[0] == 0x20) && (t[1] == 0x22)){
		c += 2;
		t += 2;
		while (c < packet_len - 2) {
		    if ((t[0] == 0x22) && (t[1] == 0x20)) return ((IPP2P_WINMX * 100) + 3);
		    t++;
		    c++;
		}
	    }
	    t++;
	    c++;
	}    
    }
    return 0;
} /*search_winmx*/


/*Search for appleJuice commands*/
int
search_apple (unsigned char *haystack, int packet_len, int head_len)
{
    unsigned char *t = haystack;
    t += head_len;
    
    if ((memcmp(t, "ajprot", 6) == 0) && (t[6] == 0x0d) && (t[7] == 0x0a))  return (IPP2P_APPLE * 100);
    
    return 0;
}


/*Search for BitTorrent commands*/
int
search_bittorrent (unsigned char *haystack, int packet_len, int head_len)
{
    const unsigned char *t = haystack+head_len;
    
    /* test for match 0x13+"BitTorrent protocol" */
    if (t[0] == 0x13) 
    {
        if (memcmp(t+1, "BitTorrent protocol", 19) == 0) return (IPP2P_BIT * 100);
    }
    
    /* get tracker commandos, all starts with GET /
     * then it can follow: scrape| announce
     * and then ?hash_info=
     */
    if (memcmp(t,"GET /",5) == 0)
    {
	/* message scrape */
	if ( memcmp(t+5,"scrape?info_hash=",17)==0 ) return (IPP2P_BIT * 100 + 1);
	/* message announce */
	if ( memcmp(t+5,"announce?info_hash=",19)==0 ) return (IPP2P_BIT * 100 + 2);
    }
    return 0;
}



/*check for Kazaa get command*/
int
search_kazaa (unsigned char *haystack, int packet_len, int head_len)
{
    unsigned char *t = haystack;

    if (!((*(haystack + packet_len - 2) == 0x0d) && (*(haystack + packet_len - 1) == 0x0a))) return 0;    

    t += head_len;
    if (memcmp(t, "GET /.hash=", 11) == 0)
	return (IPP2P_DATA_KAZAA * 100);
    else
	return 0;
}


/*check for gnutella get command*/
int
search_gnu (unsigned char *haystack, int packet_len, int head_len)
{
    unsigned char *t = haystack;

    if (!((*(haystack + packet_len - 2) == 0x0d) && (*(haystack + packet_len - 1) == 0x0a))) return 0;    

    t += head_len;
    if (memcmp(t, "GET /get/", 9) == 0)	return ((IPP2P_DATA_GNU * 100) + 1);
    if (memcmp(t, "GET /uri-res/", 13) == 0) return ((IPP2P_DATA_GNU * 100) + 2); 
    
    return 0;
}


/*check for gnutella get commands and other typical data*/
int
search_all_gnu (unsigned char *haystack, int packet_len, int head_len)
{
    unsigned char *t = haystack;
    int c;    

    if (!((*(haystack + packet_len - 2) == 0x0d) && (*(haystack + packet_len - 1) == 0x0a))) return 0;

    t += head_len;

    if (memcmp(t, "GNUTELLA CONNECT/", 17) == 0) return ((IPP2P_GNU * 100) + 1);
    if (memcmp(t, "GNUTELLA/", 9) == 0) return ((IPP2P_GNU * 100) + 2);    

    if ((memcmp(t, "GET /get/", 9) == 0) || (memcmp(t, "GET /uri-res/", 13) == 0))
    {        
        c = head_len + 8;
	t += 8;
	while (c < packet_len - 22) {
	    if ((t[0] == 0x0d) && (t[1] == 0x0a)) {
		    t += 2;
		    c += 2;
		    if ((memcmp(t, "X-Gnutella-", 11) == 0) || (memcmp(t, "X-Queue:", 8) == 0)) return ((IPP2P_GNU * 100) + 3);
	    } else {
		t++;
		c++;
	    }    
	}
    }
    return 0;
}


/*check for KaZaA download commands and other typical data*/
int
search_all_kazaa (unsigned char *haystack, int packet_len, int head_len)
{
    unsigned char *t = haystack;
    int c;    
    
    if (!((*(haystack + packet_len - 2) == 0x0d) && (*(haystack + packet_len - 1) == 0x0a))) return 0;

    t += head_len;
    if (memcmp(t, "GIVE ", 5) == 0) return ((IPP2P_KAZAA * 100) + 1);
    
    if (memcmp(t, "GET /", 5) == 0) {
        c = head_len + 8;
	t += 8;
	while (c < packet_len - 22) {
	    if ((t[0] == 0x0d) && (t[1] == 0x0a)) {
		    t += 2;
		    c += 2;
		    if ( memcmp(t, "X-Kazaa-Username: ", 18) == 0 ) return ((IPP2P_KAZAA * 100) + 2);
		    if ( memcmp(t, "User-Agent: PeerEnabler/", 24) == 0 ) return ((IPP2P_KAZAA * 100) + 3);
	    } else {
		t++;
		c++;
	    }    
	}
    }
    return 0;
}

/*fast check for edonkey file segment transfer command*/
int
search_edk (unsigned char *haystack, int packet_len, int head_len)
{
    if (*(haystack+head_len) != 0xe3) 
	return 0;
    else {
	if (*(haystack+head_len+5) == 0x47) 
	    return (IPP2P_DATA_EDK * 100);
	else 	
	    return 0;
    }
}



/*intensive but slower search for some edonkey packets including size-check*/
int
search_all_edk (unsigned char *haystack, int packet_len, int head_len)
{
    unsigned char *t = haystack;
    int cmd;

// this stuff almost matches nothing
#if 0    
    if (*(haystack+head_len) == 0xd4) {
	t += head_len;	
	cmd = get_u16(t, 1);	
	if (cmd == (packet_len - head_len - 5))	{
	    switch (t[5]) {
		case 0x82: return ((IPP2P_EDK * 100) + 42);
		case 0x15: return ((IPP2P_EDK * 100) + 43);
		default: return 0;
	    }
	}
	return 0;    
    }
    
    
    if (*(haystack+head_len) == 0xc5) {	/*search for additional eMule packets*/
	t += head_len;	
	cmd = get_u16(t, 1);	

	if (cmd == (packet_len - head_len - 5))	{
	    switch (t[5]) {
		case 0x01: return ((IPP2P_EDK * 100) + 30);
		case 0x02: return ((IPP2P_EDK * 100) + 31);
		case 0x60: return ((IPP2P_EDK * 100) + 32);
		case 0x81: return ((IPP2P_EDK * 100) + 33);
		case 0x82: return ((IPP2P_EDK * 100) + 34);
		case 0x85: return ((IPP2P_EDK * 100) + 35);
		case 0x86: return ((IPP2P_EDK * 100) + 36);
		case 0x87: return ((IPP2P_EDK * 100) + 37);
		case 0x40: return ((IPP2P_EDK * 100) + 38);
		case 0x92: return ((IPP2P_EDK * 100) + 39);
		case 0x93: return ((IPP2P_EDK * 100) + 40);
		case 0x12: return ((IPP2P_EDK * 100) + 41);
		default: return 0;
	    }
	}
	
	return 0;
    }
#endif

    if (*(haystack+head_len) != 0xe3) 
	return 0;
    else {
	t += head_len;	
	cmd = get_u16(t, 1);
	if (cmd == (packet_len - head_len - 5)) {
	    switch (t[5]) {
		case 0x01: return ((IPP2P_EDK * 100) + 1);	/*Client: hello or Server:hello*/
//		case 0x50: return ((IPP2P_EDK * 100) + 2);	/*Client: file status*/
//		case 0x16: return ((IPP2P_EDK * 100) + 3);	/*Client: search*/
//		case 0x58: return ((IPP2P_EDK * 100) + 4);	/*Client: file request*/
//		case 0x48: return ((IPP2P_EDK * 100) + 5);	/*???*/
//		case 0x54: return ((IPP2P_EDK * 100) + 6);	/*???*/
//		case 0x47: return ((IPP2P_EDK * 100) + 7);	/*Client: file segment request*/
//		case 0x46: return ((IPP2P_EDK * 100) + 8); 	/*Client: download segment*/
		case 0x4c: return ((IPP2P_EDK * 100) + 9);	/*Client: Hello-Answer*/
//		case 0x4f: return ((IPP2P_EDK * 100) + 10);	/*Client: file status request*/
//		case 0x59: return ((IPP2P_EDK * 100) + 11);	/*Client: file request answer*/
//		case 0x65: return ((IPP2P_EDK * 100) + 12);	/*Client: ???*/
//		case 0x66: return ((IPP2P_EDK * 100) + 13);	/*Client: ???*/
//		case 0x51: return ((IPP2P_EDK * 100) + 14);	/*Client: ???*/
//		case 0x52: return ((IPP2P_EDK * 100) + 15);	/*Client: ???*/
//		case 0x4d: return ((IPP2P_EDK * 100) + 16);	/*Client: ???*/
//		case 0x5c: return ((IPP2P_EDK * 100) + 17);	/*Client: ???*/
//		case 0x38: return ((IPP2P_EDK * 100) + 18);	/*Client: ???*/
//		case 0x69: return ((IPP2P_EDK * 100) + 19);	/*Client: ???*/
//		case 0x19: return ((IPP2P_EDK * 100) + 20);	/*Client: ???*/
//		case 0x42: return ((IPP2P_EDK * 100) + 21);	/*Client: ???*/
//		case 0x34: return ((IPP2P_EDK * 100) + 22);	/*Client: ???*/
//		case 0x94: return ((IPP2P_EDK * 100) + 23);	/*Client: ???*/
//		case 0x1c: return ((IPP2P_EDK * 100) + 24);	/*Client: ???*/
//		case 0x6a: return ((IPP2P_EDK * 100) + 25);	/*Client: ???*/
		default: return 0;
	    }
	} else {
	    if (cmd > packet_len - head_len - 5) {
		if ((t[3] == 0x00) && (t[4] == 0x00)) {
		    if (t[5] == 0x01) return ((IPP2P_EDK * 100) + 26);
//		    if (t[5] == 0x4c) return ((IPP2P_EDK * 100) + 27);
		} 
		return 0;
		
	    }	/*non edk packet*/
//	    if (t[cmd+5] == 0xe3) return ((IPP2P_EDK * 100) + 28);/*found another edk-command*/
//	    if (t[cmd+5] == 0xc5) return ((IPP2P_EDK * 100) + 29);/*found an emule-command*/	    
	    return 0;
	}
    }
}


/*fast check for Direct Connect send command*/
int
search_dc (unsigned char *haystack, int packet_len, int head_len)
{
    unsigned char *t = haystack;

    if (*(haystack+head_len) != 0x24 ) 
	return 0;
    else {
	t += head_len + 1;
        if (memcmp(t, "Send|", 5) == 0)
	    return (IPP2P_DATA_DC * 100);
	else
	    return 0;
    }	

}


/*intensive but slower check for all direct connect packets*/
int
search_all_dc (unsigned char *haystack, int packet_len, int head_len)
{
    unsigned char *t = haystack;

    if ((*(haystack + head_len) == 0x24) && (*(haystack + packet_len - 1) == 0x7c)) 
    {
    	t += head_len + 1;
	/* Client-Hub-Protocol */
	if (memcmp(t, "Lock ", 5) == 0)	 		return ((IPP2P_DC * 100) + 1);
#if 0	
	if (memcmp(t, "Key ", 4) == 0)			 return ((IPP2P_DC * 100) + 2);
	if (memcmp(t, "HubName ", 8) == 0) 		return ((IPP2P_DC * 100) + 3);
	if (memcmp(t, "HubIsFull ", 10) == 0) 		return ((IPP2P_DC * 100) + 4);
	if (memcmp(t, "ValidateNick ", 13) == 0) 	return ((IPP2P_DC * 100) + 5);
	if (memcmp(t, "ValidateDenide ", 15) == 0) 	return ((IPP2P_DC * 100) + 6);
	if (memcmp(t, "GetPass ", 8) == 0) 		return ((IPP2P_DC * 100) + 7);
	if (memcmp(t, "MyPass ", 7) == 0) 		return ((IPP2P_DC * 100) + 8);
	if (memcmp(t, "BadPass ", 8) == 0) 		return ((IPP2P_DC * 100) + 9);

	if (memcmp(t, "LogedIn ", 8) == 0) 		return ((IPP2P_DC * 100) + 10);
	if (memcmp(t, "Version ", 8) == 0) 		return ((IPP2P_DC * 100) + 11);
	if (memcmp(t, "Hello ", 6) == 0) 		return ((IPP2P_DC * 100) + 12);
	if (memcmp(t, "Quit ", 5) == 0) 		return ((IPP2P_DC * 100) + 13);
	if (memcmp(t, "GetNickList ", 12) == 0) 	return ((IPP2P_DC * 100) + 14);
	if (memcmp(t, "MyINFO ", 7) == 0) 		return ((IPP2P_DC * 100) + 15);
	if (memcmp(t, "GetINFO ", 8) == 0) 		return ((IPP2P_DC * 100) + 16);
	if (memcmp(t, "NickList ", 9) == 0) 		return ((IPP2P_DC * 100) + 17);
	if (memcmp(t, "OpList ", 7) == 0) 		return ((IPP2P_DC * 100) + 18);
	if (memcmp(t, "Search ", 7) == 0) 		return ((IPP2P_DC * 100) + 19);
	if (memcmp(t, "MultiSearch ", 12) == 0) 	return ((IPP2P_DC * 100) + 20);
	if (memcmp(t, "SR ", 3) == 0) 			return ((IPP2P_DC * 100) + 21);
	if (memcmp(t, "ConnectToMe ", 12) == 0) 	return ((IPP2P_DC * 100) + 22);
	if (memcmp(t, "MultiConnectToMe ", 17) == 0) 	return ((IPP2P_DC * 100) + 23);
	if (memcmp(t, "RevConnectToMe ", 15) == 0) 	return ((IPP2P_DC * 100) + 24);
	if (memcmp(t, "Chat ", 5) == 0) 		return ((IPP2P_DC * 100) + 25);
	if (memcmp(t, "To ", 3) == 0) 			return ((IPP2P_DC * 100) + 26);
	
	if (memcmp(t, "OpForceMove ", 12) == 0) 	return ((IPP2P_DC * 100) + 27);
	if (memcmp(t, "ForceMove ", 10) == 0) 		return ((IPP2P_DC * 100) + 28);
	if (memcmp(t, "Kick ", 5) == 0) 		return ((IPP2P_DC * 100) + 29);
	if (memcmp(t, "Close ", 6) == 0) 		return ((IPP2P_DC * 100) + 30);
	if (memcmp(t, "UserIP ", 7) == 0) 		return ((IPP2P_DC * 100) + 31);
	if (memcmp(t, "BotINFO ", 8) == 0) 		return ((IPP2P_DC * 100) + 32);
	if (memcmp(t, "HubINFO ", 8) == 0) 		return ((IPP2P_DC * 100) + 33);
	if (memcmp(t, "UserCommand ", 12) == 0) 	return ((IPP2P_DC * 100) + 34);
	if (memcmp(t, "QuickList ", 10) == 0) 		return ((IPP2P_DC * 100) + 35);
	if (memcmp(t, "Supports ", 9) == 0) 		return ((IPP2P_DC * 100) + 36);
	if (memcmp(t, "QuickList ", 10) == 0) 		return ((IPP2P_DC * 100) + 37);

#endif
	/* Client-Client-Protocol, some are already recognized by client-hub (like lock) */
	if (memcmp(t, "MyNick ", 7) == 0)	 	return ((IPP2P_DC * 100) + 38); 
#if 0
	if (memcmp(t, "Lock ", 5) == 0)		 	return ((IPP2P_DC * 100) + 39); 
	if (memcmp(t, "Direction ", 10) == 0)		return ((IPP2P_DC * 100) + 40); 
//	if (memcmp(t, "Key ", 4) == 0)	 		return ((IPP2P_DC * 100) + 2); 
	if (memcmp(t, "GetListLen ", 11) == 0)	 	return ((IPP2P_DC * 100) + 41); 
	if (memcmp(t, "MaxedOut ", 9) == 0)	 	return ((IPP2P_DC * 100) + 42); 
	if (memcmp(t, "Error ", 7) == 0)	 	return ((IPP2P_DC * 100) + 43); 
	if (memcmp(t, "Get ", 4) == 0)	 		return ((IPP2P_DC * 100) + 44); 
	if (memcmp(t, "FileLength ", 11) == 0)	 	return ((IPP2P_DC * 100) + 45);
	if (memcmp(t, "Send ", 5) == 0)	 		return ((IPP2P_DC * 100) + 46); 
	if (memcmp(t, "Cancel ", 7) == 0)	 	return ((IPP2P_DC * 100) + 47); 
	if (memcmp(t, "Canceled ", 9) == 0)	 	return ((IPP2P_DC * 100) + 48);
//	if (memcmp(t, "Supports ", 4) == 0)	 	return ((IPP2P_DC * 100) + 2); 
	
	if (memcmp(t, "BZList ", 7) == 0)	 	return ((IPP2P_DC * 100) + 49); 
	if (memcmp(t, "GetZBlock ", 10) == 0)	 	return ((IPP2P_DC * 100) + 50); 
	if (memcmp(t, "Sending ", 8) == 0)	 	return ((IPP2P_DC * 100) + 51); 
	if (memcmp(t, "Failed ", 7) == 0)	 	return ((IPP2P_DC * 100) + 52); 
	if (memcmp(t, "MiniSlots ", 10) == 0)	 	return ((IPP2P_DC * 100) + 53); 
	if (memcmp(t, "ADCGET ", 7) == 0)	 	return ((IPP2P_DC * 100) + 54); 
	if (memcmp(t, "Meta ", 5) == 0)	 		return ((IPP2P_DC * 100) + 55); 
	if (memcmp(t, "GetMeta ", 8) == 0)	 	return ((IPP2P_DC * 100) + 56); 
#endif
	

	// old ipp2p 0.7.4 detecting
#if 0	
	if (memcmp(t, "Send", 4) == 0)	 return ((IPP2P_DC * 100) + 6); /*client: start download*/
	if (memcmp(t, "Lock ", 5) == 0)	 return ((IPP2P_DC * 100) + 1); /*hub: hello*/
	if (memcmp(t, "Key ", 4) == 0)	 return ((IPP2P_DC * 100) + 2); /*client: hello*/
	if (memcmp(t, "Hello ", 6) == 0) return ((IPP2P_DC * 100) + 3); /*hub:connected*/
	if (memcmp(t, "MyNick ", 7) == 0) return ((IPP2P_DC * 100) + 4); /*client-client: hello*/
	if (memcmp(t, "Search ", 7) == 0) return ((IPP2P_DC * 100) + 5); /*client: search*/
	if (memcmp(t, "Send", 4) == 0)	 return ((IPP2P_DC * 100) + 6); /*client: start download*/
#endif
	return 0;
    } else
	return 0;
}


static struct {
    int command;
    __u8 short_hand;			/*for fucntions included in short hands*/
    int packet_len;
    int (*function_name) (unsigned char *, int, int);
} matchlist[] = {
    {IPP2P_EDK,SHORT_HAND_IPP2P,40, &search_all_edk},
//    {IPP2P_DATA_KAZAA,SHORT_HAND_DATA,200, &search_kazaa},
//    {IPP2P_DATA_EDK,SHORT_HAND_DATA,60, &search_edk},
//    {IPP2P_DATA_DC,SHORT_HAND_DATA,26, &search_dc},
    {IPP2P_DC,SHORT_HAND_IPP2P,25, search_all_dc},
//    {IPP2P_DATA_GNU,SHORT_HAND_DATA,40, &search_gnu},
    {IPP2P_GNU,SHORT_HAND_IPP2P,35, &search_all_gnu},
    {IPP2P_KAZAA,SHORT_HAND_IPP2P,35, &search_all_kazaa},
    {IPP2P_BIT,SHORT_HAND_IPP2P,40, &search_bittorrent},
    {IPP2P_APPLE,SHORT_HAND_IPP2P,20, &search_apple},
    {IPP2P_SOUL,SHORT_HAND_IPP2P,25, &search_soul},
    {IPP2P_WINMX,SHORT_HAND_IPP2P,20, &search_winmx},
    {IPP2P_ARES,SHORT_HAND_IPP2P,25, &search_ares},
    {0,0,0,NULL}
};


static struct {
    int command;
    __u8 short_hand;			/*for fucntions included in short hands*/
    int packet_len;
    int (*function_name) (unsigned char *, int);
} udp_list[] = {
    {IPP2P_KAZAA,SHORT_HAND_IPP2P,14, &udp_search_kazaa},
    {IPP2P_BIT,SHORT_HAND_IPP2P,23, &udp_search_bit},
    {IPP2P_GNU,SHORT_HAND_IPP2P,11, &udp_search_gnu},
    {IPP2P_EDK,SHORT_HAND_IPP2P,9, &udp_search_edk},
    {IPP2P_DC,SHORT_HAND_IPP2P,12, &udp_search_directconnect},    
    {0,0,0,NULL}
};


static int
match(const struct sk_buff *skb,
      const struct net_device *in,
      const struct net_device *out,
      const void *matchinfo,
      int offset,

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
      const void *hdr,
      u_int16_t datalen,
#endif

      int *hotdrop)
{
    const struct ipt_p2p_info *info = matchinfo;
    unsigned char  *haystack;
    struct iphdr *ip = skb->nh.iph;
    int p2p_result = 0, i = 0;
    int head_len;
    int hlen = ntohs(ip->tot_len)-(ip->ihl*4);	/*hlen = packet-data length*/

    /*must not be a fragment*/
    if (offset) {
	if (info->debug) printk("IPP2P.match: offset found %i \n",offset);
	return 0;
    }
    
    /*make sure that skb is linear*/
    if(skb_is_nonlinear(skb)){
	if (info->debug) printk("IPP2P.match: nonlinear skb found\n");
	return 0;
    }


    haystack=(char *)ip+(ip->ihl*4);		/*haystack = packet data*/

    switch (ip->protocol){
	case IPPROTO_TCP:		/*what to do with a TCP packet*/
	{
	    struct tcphdr *tcph = (void *) ip + ip->ihl * 4;
	    
	    if (tcph->fin) return 0;  /*if FIN bit is set bail out*/
	    if (tcph->syn) return 0;  /*if SYN bit is set bail out*/
	    if (tcph->rst) return 0;  /*if RST bit is set bail out*/
	    head_len = tcph->doff * 4; /*get TCP-Header-Size*/
	    while (matchlist[i].command) {
		if ((((info->cmd & matchlist[i].command) == matchlist[i].command) ||
		    ((info->cmd & matchlist[i].short_hand) == matchlist[i].short_hand)) &&
		    (hlen > matchlist[i].packet_len)) {
			    p2p_result = matchlist[i].function_name(haystack, hlen, head_len);
			    if (p2p_result) 
			    {
				if (info->debug) printk("IPP2P.debug:TCP-match: %i from: %u.%u.%u.%u:%i to: %u.%u.%u.%u:%i Length: %i\n", 
				    p2p_result, NIPQUAD(ip->saddr),ntohs(tcph->source), NIPQUAD(ip->daddr),ntohs(tcph->dest),hlen);
				return p2p_result;
    			    }
    		}
	    i++;
	    }
	    return p2p_result;
	}
	
	case IPPROTO_UDP:		/*what to do with an UDP packet*/
	{
	    struct udphdr *udph = (void *) ip + ip->ihl * 4;
	    
	    while (udp_list[i].command){
		if ((((info->cmd & udp_list[i].command) == udp_list[i].command) ||
		    ((info->cmd & udp_list[i].short_hand) == udp_list[i].short_hand)) &&
		    (hlen > udp_list[i].packet_len)) {
			    p2p_result = udp_list[i].function_name(haystack, hlen);
			    if (p2p_result){
				if (info->debug) printk("IPP2P.debug:UDP-match: %i from: %u.%u.%u.%u:%i to: %u.%u.%u.%u:%i Length: %i\n", 
				    p2p_result, NIPQUAD(ip->saddr),ntohs(udph->source), NIPQUAD(ip->daddr),ntohs(udph->dest),hlen);
				return p2p_result;
			    }
		}
	    i++;
	    }			
	    return p2p_result;
	}
    
	default: return 0;
    }
}



static int
checkentry(const char *tablename,
            const struct ipt_ip *ip,
	    void *matchinfo,
	    unsigned int matchsize,
	    unsigned int hook_mask)
{
        /* Must specify -p tcp */
/*    if (ip->proto != IPPROTO_TCP || (ip->invflags & IPT_INV_PROTO)) {
 *	printk("ipp2p: Only works on TCP packets, use -p tcp\n");
 *	return 0;
 *    }*/
    return 1;
}
									    



static struct ipt_match ipp2p_match = { 
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
	{ NULL, NULL }, 
	"ipp2p", 
	&match, 
	&checkentry, 
	NULL, 
	THIS_MODULE
#endif
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,0)
	.name		= "ipp2p",
	.match		= &match,
	.checkentry	= &checkentry,
	.me		= THIS_MODULE,
#endif
};


static int __init init(void)
{
    printk(KERN_INFO "IPP2P v%s loading\n", IPP2P_VERSION);
    return ipt_register_match(&ipp2p_match);
}
	
static void __exit fini(void)
{
    ipt_unregister_match(&ipp2p_match);
    printk(KERN_INFO "IPP2P v%s unloaded\n", IPP2P_VERSION);    
}
	
module_init(init);
module_exit(fini);


