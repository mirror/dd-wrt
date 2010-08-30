#include "worker_example.h"

/*
 * Hotplug2 example worker module
 *
 * This is a heavily commented example Hotplug2 worker module,
 * used for demonstration purposes as to how to write your own
 * module tailored to your needs.
 */

static void *worker_example_init(struct settings_t *settings) {
	/*
	 * In this function, you initialize the worker. This function
	 * gets called only once in the life of Hotplug2, and you probably
	 * want to eg. prepare threading, or prepare data structures to
	 * keep track of child processes (which you may also pre-spawn).
	 * 
	 * This function receives complete settings (or context) of Hotplug2,
	 * including parsed rules and netlink socket, as the first argument.
	 *
	 * It returns the context of the worker, that is passed as first
	 * argument to deinit and process functions.
	 */
	return NULL;
}

static void worker_example_deinit(void *ctx) {
	/*
	 * This function gets called when Hotplug2 terminates - either by not
	 * being invoked as persistent, or by being killed by some handled signal.
	 *
	 * Here you should eg. wait for all children processes (but preferably
	 * not indefinitely) and release all memory of the worker context.
	 */
	return;
}

static int worker_example_process(void *ctx, struct uevent_t *uevent) {
	/*
	 * This function is invoked whenever a new uevent is read from the
	 * kernel. The event gets passed to child process or a thread to
	 * be properly handled. Dynamic child spawning should take place
	 * here, along with methods of throthling based on various inputs,
	 * such as:
	 *  * number of events waiting (kernel seqnum vs event seqnum)
	 *  * resources available
	 *  * estimate of time intensivity of the applied rules (eg. using flags)
	 *
	 *
	 * This is how an event is usually processed:
	 * action_perform(ctx->settings, uevent);
	 *
	 * In this example, we only print out the event.
	 */

	int i;

	for (i = 0; i < uevent->env_vars_c; i++)
		printf("%s=%s\n", uevent->env_vars[i].key, uevent->env_vars[i].value);
	printf("\n");

	return 0;
}

struct worker_module_t worker_module = {
	"Hotplug2 example module",
	worker_example_init,
	worker_example_deinit,
	worker_example_process
};
