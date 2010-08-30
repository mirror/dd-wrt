#ifndef HOTPLUG2_H
#define HOTPLUG2_H 1
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

#include "netlink.h"
#include "settings.h"
#include "uevent.h"
#include "parser/parser.h"
#include "rules/execution.h"
#include "rules/ruleset.h"
#include "workers/loader.h"

#define UEVENT_BUFFER_SIZE		2048


struct options_t {
	char *name;
	int *value;
};

#endif /* ifndef HOTPLUG2_H */

