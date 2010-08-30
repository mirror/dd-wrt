#include "loader.h"

#ifdef STATIC_WORKER

extern struct worker_module_t worker_module;
static struct loader_ctx_t static_ctx = {
	.module = &worker_module
};

struct loader_ctx_t *worker_load(const char *name)
{
	return &static_ctx;
}

void worker_free(struct loader_ctx_t *ctx)
{
}

#else

struct loader_ctx_t *worker_load(const char *name) {
	struct loader_ctx_t *ctx;

	ctx = malloc(sizeof(struct loader_ctx_t));

	ctx->dl_handle = dlopen(name, RTLD_NOW);
	if (ctx->dl_handle == NULL) {
		fprintf(stderr, "Loader error: %s\n", dlerror());
		worker_free(ctx);
		return NULL;
	}
	
	ctx->module = dlsym(ctx->dl_handle, "worker_module");
	if (ctx->module == NULL) {
		fprintf(stderr, "Loader error: %s\n", dlerror());
		worker_free(ctx);
		return NULL;
	}

	return ctx;
}

void worker_free(struct loader_ctx_t *ctx) {
	if (ctx == NULL)
		return;
	
	if (ctx->dl_handle != NULL)
		dlclose(ctx->dl_handle);
	
	free(ctx);
}

#endif
