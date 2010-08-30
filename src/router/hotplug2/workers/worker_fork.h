#ifndef WORKER_FORK_H
#define WORKER_FORK_H 1

#define _XOPEN_SOURCE 500
#include <sys/types.h>
#include <sys/select.h>
#include <unistd.h>

#include "../rules/execution.h"

#include "../action.h"
#include "../settings.h"
#include "../uevent.h"

#include "worker.h"

/*#define PRINTFUNC() printf("[%5d] Function: %s, %s:%d\n", getpid(), __func__, __FILE__, __LINE__)*/
#define PRINTFUNC() 

#define CURRENT_CHILD(ctx) (ctx)->children[(ctx)->children_count - 1]

struct worker_fork_child_t {
	pid_t		pid;
	int			busy;
	int			event_fd, feedback_fd;
	uint32_t	processed;
};

struct worker_fork_ctx_t {
	struct worker_fork_child_t 	**children;
	int							children_count;

	int							max_children;
	int							always_fork;
	struct settings_t			*settings;
};

struct worker_fork_uevent_t {
	struct uevent_t *uevent;
	struct worker_fork_uevent_t *next;
};

#endif
