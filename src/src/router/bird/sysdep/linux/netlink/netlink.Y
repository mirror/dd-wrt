/*
 *	BIRD -- Linux Netlink Configuration
 *
 *	(c) 1999--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

CF_HDR

CF_DECLS

CF_KEYWORDS(ASYNC, KERNEL, TABLE)

CF_GRAMMAR

CF_ADDTO(kern_proto, kern_proto nl_item ';')

nl_item:
   KERNEL TABLE expr {
#ifndef IPV6
	if ($3 <= 0 || $3 >= NL_NUM_TABLES)
	  cf_error("Kernel routing table number out of range");
#else
	if ($3 != 254)
	  cf_error("Linux implementation of IPv6 doesn't support multiple routing tables");
#endif
	THIS_KRT->scan.table_id = $3;
   }
 ;

CF_CODE

CF_END
