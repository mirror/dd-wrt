/*
 * duplicate_handler.c
 *
 *  Created on: 11.01.2010
 *      Author: henning
 */

#include "common/list.h"
#include "defs.h"
#include "ipcalc.h"
#include "log.h"
#include "olsr.h"
#include "scheduler.h"
#include "duplicate_handler.h"

#ifndef NO_DUPLICATE_DETECTION_HANDLER

static struct list_node duplicate_handler_head;
static uint32_t spam_orig_counter, spam_hna_counter, spam_mid_counter;

static struct timer_entry *duplicate_spam_timer;

static void handle_duplicate_spam_timer(void __attribute__ ((unused)) *no) {
  if (spam_orig_counter > MAX_SYSLOG_EACH_HOUR) {
    olsr_syslog(OLSR_LOG_INFO, "Skipped %u originator duplicate warnings.", spam_orig_counter - MAX_SYSLOG_EACH_HOUR);
  }
  if (spam_hna_counter > MAX_SYSLOG_EACH_HOUR) {
    olsr_syslog(OLSR_LOG_INFO, "Skipped %u hna duplicate warnings.", spam_hna_counter - MAX_SYSLOG_EACH_HOUR);
  }
  if (spam_mid_counter > MAX_SYSLOG_EACH_HOUR) {
    olsr_syslog(OLSR_LOG_INFO, "Skipped %u mid duplicate warnings.", spam_mid_counter - MAX_SYSLOG_EACH_HOUR);
  }
  spam_orig_counter = 0;
  spam_hna_counter = 0;
  spam_mid_counter = 0;
}


void olsr_duplicate_handler_init(void) {
  list_head_init(&duplicate_handler_head);

  spam_orig_counter = 0;
  spam_hna_counter = 0;
  spam_mid_counter = 0;

  olsr_set_timer(&duplicate_spam_timer, 3600*1000, 0, OLSR_TIMER_PERIODIC,
                 &handle_duplicate_spam_timer, NULL, 0);
}

void olsr_add_duplicate_handler(struct duplicate_handler *h) {
  list_add_before(&duplicate_handler_head, &h->node);
}

void olsr_remove_duplicate_handler(struct duplicate_handler *h) {
  list_remove(&h->node);
}

void olsr_test_originator_collision(uint8_t msgType, uint16_t seqno) {
  struct list_node *n;

  if (!olsr_is_bad_duplicate_msg_seqno(seqno)) {
    return;
  }

  if (++spam_orig_counter < MAX_SYSLOG_EACH_HOUR) {
    struct ipaddr_str buf;

    olsr_syslog(OLSR_LOG_INFO, "You might have another node with main ip %s in the mesh!",
        olsr_ip_to_string(&buf, &olsr_cnf->main_addr));
  }

  for (n=duplicate_handler_head.next; n != &duplicate_handler_head; n = n->next) {
    struct duplicate_handler *h = (struct duplicate_handler *)n;

    h->originator_collision(msgType);
  }
}

void olsr_handle_hna_collision(union olsr_ip_addr *hna, union olsr_ip_addr *orig) {
  struct list_node *n;

  if (++spam_hna_counter < MAX_SYSLOG_EACH_HOUR) {
    struct ipaddr_str buf1, buf2;

    olsr_syslog(OLSR_LOG_INFO, "Node %s is publishing your ip %s as HNA!",
        olsr_ip_to_string(&buf1, orig), olsr_ip_to_string(&buf2, hna));
  }

  for (n=duplicate_handler_head.next; n != &duplicate_handler_head; n = n->next) {
    struct duplicate_handler *h = (struct duplicate_handler *)n;

    h->hna_collision(hna, orig);
  }
}

void olsr_handle_mid_collision(union olsr_ip_addr *mid, union olsr_ip_addr *orig) {
  struct list_node *n;

  if (++spam_mid_counter < MAX_SYSLOG_EACH_HOUR) {
    struct ipaddr_str buf1, buf2;

    olsr_syslog(OLSR_LOG_INFO, "Node %s is publishing your ip %s as MID!",
        olsr_ip_to_string(&buf1, orig), olsr_ip_to_string(&buf2, mid));
  }

  for (n=duplicate_handler_head.next; n != &duplicate_handler_head; n = n->next) {
    struct duplicate_handler *h = (struct duplicate_handler *)n;

    h->mid_collision(mid, orig);
  }
}

#endif
