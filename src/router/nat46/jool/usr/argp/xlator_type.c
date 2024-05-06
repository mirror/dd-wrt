#include "usr/argp/xlator_type.h"

static xlator_type xtype = -1;

void xt_set(xlator_type xt)
{
	xtype = xt;
}

xlator_type xt_get(void)
{
	return xtype;
}
