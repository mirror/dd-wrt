
/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2008 Henning Rogge <rogge@fgan.de>
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

#ifndef LQPLUGIN_H_
#define LQPLUGIN_H_

#include "tc_set.h"
#include "link_set.h"
#include "olsr_spf.h"
#include "lq_packet.h"
#include "packet.h"
#include "common/avl.h"

#define LINK_COST_BROKEN (1<<22)
#define ROUTE_COST_BROKEN (0xffffffff)
#define ZERO_ROUTE_COST 0

#define MINIMAL_USEFUL_LQ 0.1
#define LQ_PLUGIN_RELEVANT_COSTCHANGE 16

#define LQ_QUICKSTART_STEPS 12
#define LQ_QUICKSTART_AGING 0.25

struct lqtextbuffer {
  char buf[16];
};

struct lq_handler {
  void (*initialize)(void);
  
  olsr_linkcost(*calc_hello_cost) (const void *lq);
  olsr_linkcost(*calc_tc_cost) (const void *lq);

  olsr_bool(*is_relevant_costchange) (olsr_linkcost c1, olsr_linkcost c2);

  olsr_linkcost(*packet_loss_handler) (struct link_entry * entry, void *lq,
				       olsr_bool lost);

  void (*memorize_foreign_hello) (void *local, void *foreign);
  void (*copy_link_lq_into_tc) (void *target, void *source);
  void (*clear_hello) (void *target);
  void (*clear_tc) (void *target);

  int (*serialize_hello_lq) (unsigned char *buff, void *lq);
  int (*serialize_tc_lq) (unsigned char *buff, void *lq);
  void (*deserialize_hello_lq) (const olsr_u8_t ** curr, void *lq);
  void (*deserialize_tc_lq) (const olsr_u8_t ** curr, void *lq);

  const char *(*print_hello_lq) (void *ptr, char separator, struct lqtextbuffer * buffer);
  const char *(*print_tc_lq) (void *ptr, char separator, struct lqtextbuffer * buffer);
  const char *(*print_cost) (olsr_linkcost cost, struct lqtextbuffer * buffer);

  size_t hello_lq_size;
  size_t tc_lq_size;
};

struct lq_handler_node {
  struct avl_node node;
  struct lq_handler *handler; 
  char name[0];
};

AVLNODE2STRUCT(lq_handler_tree2lq_handler_node, struct lq_handler_node, node);

#define OLSR_FOR_ALL_LQ_HANDLERS(lq) \
{ \
  struct avl_node *lq_tree_node, *next_lq_tree_node; \
  for (lq_tree_node = avl_walk_first(&lq_handler_tree); \
    lq_tree_node; lq_tree_node = next_lq_tree_node) { \
    next_lq_tree_node = avl_walk_next(lq_tree_node); \
    lq = lq_handler_tree2lq_handler_node(lq_tree_node);
#define OLSR_FOR_ALL_LQ_HANDLERS_END(tc) }}

int avl_strcasecmp(const void *str1, const void *str2);
void init_lq_handler_tree(void);

void register_lq_handler(struct lq_handler *handler, const char *name);
int activate_lq_handler(const char *name);

olsr_linkcost olsr_calc_tc_cost(const struct tc_edge_entry *);
olsr_bool olsr_is_relevant_costchange(olsr_linkcost c1, olsr_linkcost c2);

int olsr_serialize_hello_lq_pair(unsigned char *buff,
				 struct lq_hello_neighbor *neigh);
void olsr_deserialize_hello_lq_pair(const olsr_u8_t ** curr,
				    struct hello_neighbor *neigh);
int olsr_serialize_tc_lq_pair(unsigned char *buff, struct tc_mpr_addr *neigh);
void olsr_deserialize_tc_lq_pair(const olsr_u8_t ** curr,
				 struct tc_edge_entry *edge);

void olsr_update_packet_loss_worker(struct link_entry *entry, olsr_bool lost);
void olsr_memorize_foreign_hello_lq(struct link_entry *local,
				    struct hello_neighbor *foreign);

const char *get_link_entry_text(struct link_entry *entry,
				char separator,
				struct lqtextbuffer *buffer);
const char *get_tc_edge_entry_text(struct tc_edge_entry *entry,
				   char separator,
				   struct lqtextbuffer *buffer);
const char *get_linkcost_text(olsr_linkcost cost, olsr_bool route,
			      struct lqtextbuffer *buffer);

void olsr_copy_hello_lq(struct lq_hello_neighbor *target,
			struct link_entry *source);
void olsr_copylq_link_entry_2_tc_mpr_addr(struct tc_mpr_addr *target,
					  struct link_entry *source);
void olsr_copylq_link_entry_2_tc_edge_entry(struct tc_edge_entry *target,
					    struct link_entry *source);
void olsr_clear_tc_lq(struct tc_mpr_addr *target);

struct hello_neighbor *olsr_malloc_hello_neighbor(const char *id);
struct tc_mpr_addr *olsr_malloc_tc_mpr_addr(const char *id);
struct lq_hello_neighbor *olsr_malloc_lq_hello_neighbor(const char *id);
struct link_entry *olsr_malloc_link_entry(const char *id);

/* Externals. */
extern struct lq_handler *active_lq_handler;

#endif /*LQPLUGIN_H_ */

/*
 * Local Variables:
 * c-basic-offset: 2
 * End:
 */
