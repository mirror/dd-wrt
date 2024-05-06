#include "mod/common/xlator.h"

#include "mod/common/db/global.h"
#include "mod/common/db/bib/db.h"

/*
 * xlator impersonator for BIB unit tests.
 */

int xlator_init(struct xlator *jool, struct net *ns, char *iname,
		xlator_flags flags, struct ipv6_prefix *pool6)
{
	int error;

	memset(jool, 0, sizeof(*jool));

	jool->ns = ns;
	jool->flags = flags;
	strcpy(jool->iname, iname);

	error = globals_init(&jool->globals, XT_NAT64, pool6);
	if (error)
		return error;
	jool->nat64.bib = bib_alloc();

	return jool->nat64.bib ? 0 : -ENOMEM;
}

void xlator_put(struct xlator *jool)
{
	bib_put(jool->nat64.bib);
}
