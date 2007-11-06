/** @file msap.h
    OpenLLDP MSAP Cache Header

    Licensed under a dual GPL/Proprietary license.  
    See LICENSE file for more info.
 
    Authors: Terry Simons (terry.simons@gmail.com)
*/

#include "tlv/tlv_common.h"

struct lldp_msap {
  uint8_t *id;
  uint8_t length;
  struct lldp_tlv_list *tlv_list;
  struct lldp_msap *next;
};

struct lldp_msap *create_msap(struct lldp_tlv *tlv1, struct lldp_tlv *tlv2);
