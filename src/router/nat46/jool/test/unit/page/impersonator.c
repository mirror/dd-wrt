#include "mod/common/dev.h"
#include "mod/common/db/bib/db.h"
#include "mod/common/db/pool4/db.h"
#include "framework/unit_test.h"

int foreach_ifa(struct net *ns, int (*cb)(struct in_ifaddr *, void const *),
		void const *args)
{
	return 0;
}

int bib_foreach(struct bib *db, l4_protocol proto,
		bib_foreach_entry_cb cb, void *cb_arg,
		const struct ipv4_transport_addr *offset)
{
	return broken_unit_call("bib_foreach");
}

bool pool4db_contains(struct pool4 *pool, struct net *ns, l4_protocol proto,
		struct ipv4_transport_addr const *addr)
{
	broken_unit_call("pool4db_contains");
	return false;
}
