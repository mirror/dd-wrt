/** @file 00120f.h 
    Authors: Terry Simons (terry.simons@gmail.com)
*/

#ifndef LLDP_TLV_00120F_H
#define LLDP_TLV_00120F_H

/* 802.1AB Annex G IEEE 802.3 Organizationally SPecific TLVs */
/* 0 reserved */
#define MAC_PHY_CONFIGURATION_STATUS_TLV 1 /* Subclause Reference: G.2 */
#define POWER_VIA_MDI_TLV                2 /* Subclause Reference: G.3 */
#define LINK_AGGREGATION_TLV             3 /* Subclause Reference: G.4 */
#define MAXIMUM_FRAME_SIZE_TLV           4 /* Subclause Reference: G.5 */
/* 5 - 255 reseved */
/* End 802.1AB Annex G IEEE 802.3 Organizationally Specific TLVs */

/* IEEE 802.3 auto-negotiation Support/Status Masks */
#define AUTO_NEGOTIATION_SUPPORT 1 /* bit 0 */
#define AUTO_NEGOTIATION_STATUS  2 /* bit 1 */
/* bits 2 - 7 reserved */
/* End IEEE 802.3 auto-negotiation Support/Status Masks */

/* IEEE 802.3 Aggregation Capability/Status Masks */
#define AGGREGATION_CAPABILITY 1 /* bit 0 */
#define AGGREGATION_STATUS     2 /* bit 1 */
/* bits 2 - 7 reserved */
/* End IEEE 802.3 Aggregation Capability/Status Masks */

#define bOther   0
#define bAUI     1
#define b10base5 2

#endif /* LLDP_TLV_00120F_H */
