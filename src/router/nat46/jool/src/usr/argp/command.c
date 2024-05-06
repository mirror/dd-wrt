#include "usr/argp/command.h"

#include "common/xlat.h"
#include "usr/argp/xlator_type.h"

bool cmdopt_is_hidden(struct cmd_option *option)
{
	return option->hidden || !(xt_get() & option->xt);
}
