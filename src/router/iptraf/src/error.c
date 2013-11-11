/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/***

error.c - Error-handling subroutines

***/

#include "iptraf-ng-compat.h"

#include "log.h"
#include "tui/msgboxes.h"

void write_error(char *msg, ...)
{
	va_list vararg;

	va_start(vararg, msg);
	if (daemonized)
		write_daemon_err(msg, vararg);
	else
		tui_error_va(ANYKEY_MSG, msg, vararg);
	va_end(vararg);
}
