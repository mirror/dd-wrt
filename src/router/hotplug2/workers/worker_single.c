#include "worker_single.h"

/*
 * Exported module functions
 */
static void *worker_single_init(struct settings_t *settings) {
	/* We don't need any special context, so we use settings as ctx. */
	return settings;
}

static void worker_single_deinit(void *settings) {
	/* We don't free settigns here. */
	return;
}

static int worker_single_process(void *settings, struct uevent_t *uevent) {
	action_perform((struct settings_t *)settings, uevent);
	return 0;
}

struct worker_module_t worker_module = {
	"Hotplug2 single process module",
	worker_single_init,
	worker_single_deinit,
	worker_single_process
};
