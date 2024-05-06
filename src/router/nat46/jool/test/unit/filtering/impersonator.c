#include "mod/common/joold.h"
#include "framework/unit_test.h"

static struct fake {
	int junk;
} dummy;

void joold_add(struct xlator *jool, struct session_entry *entry)
{
	/* No code. */
}

struct joold_queue *joold_alloc(void)
{
	return (struct joold_queue *)&dummy;
}

void joold_get(struct joold_queue *queue)
{
	/* No code. */
}

void joold_put(struct joold_queue *queue)
{
	/* No code. */
}

int foreach_ifa(struct net *ns, int (*cb)(struct in_ifaddr *, void const *),
		void const *args)
{
	return broken_unit_call(__func__);
}
