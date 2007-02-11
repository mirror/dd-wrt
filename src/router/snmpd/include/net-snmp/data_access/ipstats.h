/*
 * ipstats data access header
 *
 * $Id: ipstats.h,v 1.2 2004/09/14 02:29:16 rstory Exp $
 */
#ifndef NETSNMP_ACCESS_IPSTATS_H
#define NETSNMP_ACCESS_IPSTATS_H

# ifdef __cplusplus
extern          "C" {
#endif

/**---------------------------------------------------------------------*/
/*
 * structure definitions
 */

/*
 * netsnmp_ipstats_entry
 */
typedef struct netsnmp_ipstats_s {

   U64             HCInReceives;
   U64             HCInOctets;
   u_long          InHdrErrors;
   u_long          InNoRoutes;
   u_long          InAddrErrors;
   u_long          InUnknownProtos;
   u_long          InTruncatedPkts;
   U64             HCInForwDatagrams;
   u_long          ReasmReqds;
   u_long          ReasmOKs;
   u_long          ReasmFails;
   u_long          InDiscards;
   U64             HCInDelivers;
   U64             HCOutRequests;
   u_long          OutNoRoutes;
   U64             HCOutForwDatagrams;
   u_long          OutDiscards;
   u_long          OutFragReqds;
   u_long          OutFragOKs;
   u_long          OutFragFails;
   u_long          OutFragCreates;
   U64             HCOutTransmits;
   U64             HCOutOctets;
   U64             HCInMcastPkts;
   U64             HCInMcastOctets;
   U64             HCOutMcastPkts;
   U64             HCOutMcastOctets;
   U64             HCInBcastPkts;
   U64             HCOutBcastPkts;

} netsnmp_ipstats;


# ifdef __cplusplus
}
#endif

#endif /* NETSNMP_ACCESS_IPSTATS_H */
