#ifndef SRC_MOD_NAT64_JOOLD_H_
#define SRC_MOD_NAT64_JOOLD_H_

#include "common/config.h"
#include "mod/common/xlator.h"
#include "mod/common/db/bib/entry.h"

struct joold_queue;

/*
 * Note: "flush" in this context means "send sessions to userspace." The queue
 * is emptied as a result.
 */

/* joold_setup() not needed. */
void joold_teardown(void);

struct joold_queue *joold_alloc(void);
void joold_get(struct joold_queue *queue);
void joold_put(struct joold_queue *queue);

int joold_sync(struct xlator *jool, struct nlattr *root);
void joold_add(struct xlator *jool, struct session_entry *entry);

int joold_advertise(struct xlator *jool);
void joold_ack(struct xlator *jool);

void joold_clean(struct xlator *jool);

#endif /* SRC_MOD_NAT64_JOOLD_H_ */
