#include "settings.h"

void settings_init(struct settings_t *settings) {
	/* Nothing processed yet.  */
	settings->highest_seqnum = 0;
	settings->netlink_socket = -1;

	/* Various defaults... */
	settings->persistent = 1;
	settings->useflags = 0;
	settings->dumb = 0;

	/* Modprobe command will be autodetected if not set. */
	settings->modprobe_command = NULL;

	/* Coldplug will not be used if not set. */
	settings->coldplug_command = NULL;

	/* Worker will be set by argv */
	settings->worker_name = NULL;
	settings->worker = NULL;

	/* Worker argv is initially empty */
	settings->worker_argv = NULL;
	settings->worker_argc = 0;

	/* Rules will be processed in the application, too. */
	ruleset_init(&settings->rules);
	settings->rules_file = NULL;

	/* This is currently constant, but might be user-specified eventually */
	settings->grace = 100000;
}

void settings_clear(struct settings_t *settings) {
	/*
	 * We don't close netlink socket here, nor
	 * do we unload worker.
	 */

	if (settings->modprobe_command != NULL)
		free(settings->modprobe_command);

	if (settings->coldplug_command != NULL)
		free(settings->coldplug_command);

	if (settings->rules_file != NULL) {
		ruleset_clear(&settings->rules);
		free(settings->rules_file);
	}
	
	if (settings->worker_name != NULL)
		free(settings->worker_name);
}
