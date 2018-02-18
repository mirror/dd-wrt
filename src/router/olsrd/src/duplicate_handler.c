/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
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

#endif /* NO_DUPLICATE_DETECTION_HANDLER */
