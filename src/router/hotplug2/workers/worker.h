#ifndef WORKER_H
#define WORKER_H 1

#include "../uevent.h"
#include "../settings.h"

typedef void*(*worker_init)(struct settings_t *);
typedef void(*worker_deinit)(void *);
typedef int(*worker_process)(void *, struct uevent_t *);

struct worker_module_t {
	char			*modinfo;

	worker_init		init;
	worker_deinit	deinit;
	worker_process	process;
};

#endif

