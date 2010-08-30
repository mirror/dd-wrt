#ifndef LOADER_H
#define LOADER_H 1

#include <dlfcn.h>

#include "worker.h"

struct loader_ctx_t {
	void *dl_handle;
	struct worker_module_t *module;
};

struct loader_ctx_t *worker_load(const char *);
void worker_free(struct loader_ctx_t *);

#endif /* LOADER_H */
