#include "defs.h"

SYS_FUNC(statfs)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		print_struct_statfs(tcp, tcp->u_arg[1]);
	}
	return 0;
}
