/*
 *	BIRD -- Linux Netlink Configuration
 *
 *	(c) 1999--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

CF_HDR

CF_DECLS

CF_KEYWORDS(KERNEL, TABLE, KRT_PREFSRC, KRT_REALM)

CF_GRAMMAR

CF_ADDTO(kern_proto, kern_proto kern_sys_item ';')

kern_sys_item:
   KERNEL TABLE expr {
	if ($3 <= 0 || $3 >= NL_NUM_TABLES)
	  cf_error("Kernel routing table number out of range");
	THIS_KRT->sys.table_id = $3;
   }
 ;

CF_ADDTO(dynamic_attr, KRT_PREFSRC { $$ = f_new_dynamic_attr(EAF_TYPE_IP_ADDRESS, T_IP, EA_KRT_PREFSRC); })
CF_ADDTO(dynamic_attr, KRT_REALM { $$ = f_new_dynamic_attr(EAF_TYPE_INT, T_INT, EA_KRT_REALM); })

CF_CODE

CF_END
