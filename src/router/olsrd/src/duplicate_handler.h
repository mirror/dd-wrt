/*
 * duplicate_handler.h
 *
 *  Created on: 11.01.2010
 *      Author: henning
 */

#ifndef DUPLICATE_HANDLER_H_
#define DUPLICATE_HANDLER_H_

#include "defs.h"
#include "olsr.h"
#include "common/list.h"

#ifndef NO_DUPLICATE_DETECTION_HANDLER
#define MAX_SYSLOG_EACH_HOUR 10

struct duplicate_handler {
  struct list_node node;

  void (*originator_collision)(uint8_t msgType);
  void (*hna_collision)(union olsr_ip_addr *hna, union olsr_ip_addr *orig);
  void (*mid_collision)(union olsr_ip_addr *mid, union olsr_ip_addr *orig);
};

void olsr_duplicate_handler_init(void);

void olsr_add_duplicate_handler(struct duplicate_handler *);
void olsr_remove_duplicate_handler(struct duplicate_handler *);

void olsr_test_originator_collision(uint8_t msgType, uint16_t seqno);
void olsr_handle_hna_collision(union olsr_ip_addr *hna, union olsr_ip_addr *orig);
void olsr_handle_mid_collision(union olsr_ip_addr *mid, union olsr_ip_addr *orig);
#endif
#endif /* DUPLICATE_HANDLER_H_ */
