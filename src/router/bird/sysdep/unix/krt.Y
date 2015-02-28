/*
 *	BIRD -- UNIX Kernel Syncer Configuration
 *
 *	(c) 1998--2000 Martin Mares <mj@ucw.cz>
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

CF_HDR

#include "lib/krt.h"

CF_DEFINES

#define THIS_KRT ((struct krt_config *) this_proto)
#define THIS_KIF ((struct kif_config *) this_proto)

CF_DECLS

CF_KEYWORDS(KERNEL, PERSIST, SCAN, TIME, LEARN, DEVICE, ROUTES, GRACEFUL, RESTART, KRT_SOURCE, KRT_METRIC)

CF_GRAMMAR

/* Kernel syncer protocol */

CF_ADDTO(proto, kern_proto '}')

kern_proto_start: proto_start KERNEL { this_proto = krt_init_config($1); }
 ;

CF_ADDTO(kern_proto, kern_proto_start proto_name '{')
CF_ADDTO(kern_proto, kern_proto proto_item ';')
CF_ADDTO(kern_proto, kern_proto kern_item ';')

kern_item:
   PERSIST bool { THIS_KRT->persist = $2; }
 | SCAN TIME expr {
      /* Scan time of 0 means scan on startup only */
      THIS_KRT->scan_time = $3;
   }
 | LEARN bool {
      THIS_KRT->learn = $2;
#ifndef KRT_ALLOW_LEARN
      if ($2)
	cf_error("Learning of kernel routes not supported in this configuration");
#endif
   }
 | DEVICE ROUTES bool { THIS_KRT->devroutes = $3; }
 | GRACEFUL RESTART bool { THIS_KRT->graceful_restart = $3; }
 ;

/* Kernel interface protocol */

CF_ADDTO(proto, kif_proto '}')

kif_proto_start: proto_start DEVICE { this_proto = kif_init_config($1); }
 ;

CF_ADDTO(kif_proto, kif_proto_start proto_name '{')
CF_ADDTO(kif_proto, kif_proto proto_item ';')
CF_ADDTO(kif_proto, kif_proto kif_item ';')

kif_item:
   SCAN TIME expr {
      /* Scan time of 0 means scan on startup only */
      THIS_KIF->scan_time = $3;
   }
 | PRIMARY text_or_none prefix_or_ipa {
     struct kif_primary_item *kpi = cfg_alloc(sizeof (struct kif_primary_item));
     kpi->pattern = $2;
     kpi->prefix = $3.addr;
     kpi->pxlen = $3.len;
     add_tail(&THIS_KIF->primary, &kpi->n);
   }
 ;

CF_ADDTO(dynamic_attr, KRT_SOURCE { $$ = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_KRT_SOURCE); })
CF_ADDTO(dynamic_attr, KRT_METRIC { $$ = f_new_dynamic_attr(EAF_TYPE_INT | EAF_TEMP, T_INT, EA_KRT_METRIC); })

CF_CODE

CF_END
