#include "log.h"

#include <string.h>
#include <syslog.h>

int pr_result(struct jool_result *result)
{
	int error = result->error;

	if (error)
		syslog(LOG_ERR, "%s", result->msg);

	result_cleanup(result);
	return error;
}

void pr_perror(char *prefix, int error)
{
	char buffer[256];

	if (strerror_r(error, buffer, sizeof(buffer))) {
		syslog(LOG_ERR, "%s: %d", prefix, error);
		syslog(LOG_ERR, "(Sorry. I tried to stringify that but it didn't work.)");
	} else {
		syslog(LOG_ERR, "%s: %s", prefix, buffer);
	}
}
