#include "action.h"

/**
 * Blindly modprobe the modalias, nothing more.
 *
 * @1 Hotplug settings
 * @2 Event structure
 *
 * Returns: void
 */
static void action_dumb(const struct settings_t *settings, const struct uevent_t *uevent) {
	char *modalias;
	pid_t child;

	modalias = uevent_getvalue(uevent, "MODALIAS");
	if (modalias == NULL)
		return;

	child = fork();
	switch (child) {
		case 0:
			execl(settings->modprobe_command, settings->modprobe_command, "-q", modalias, NULL);
			exit(1);

		case -1:
			return;

		default:
			waitpid(child, NULL, 0);
	}
}

/**
 * Creates a "key=value" string from the given key and value
 *
 * @1 Key
 * @2 Value
 *
 * Returns: Newly allocated string in "key=value" form
 *
 */
char* alloc_env(const char *key, const char *value) {
	size_t keylen, vallen;
	char *combined;

	keylen = strlen(key);
	vallen = strlen(value) + 1;

	combined = xmalloc(keylen + vallen + 1);
	memcpy(combined, key, keylen);
	combined[keylen] = '=';
	memcpy(&combined[keylen + 1], value, vallen);

	return combined;
}

/**
 * Choose what action should be taken according to passed settings.
 *
 * @1 Hotplug settings
 * @2 Event structure
 *
 * Returns: void
 *
 */
void action_perform(struct settings_t *settings, struct uevent_t *event) {
	int i;
	char **env;

	env = xmalloc(sizeof(char *) * event->env_vars_c);

	for (i = 0; i < event->env_vars_c; i++) {
		env[i] = alloc_env(event->env_vars[i].key, event->env_vars[i].value);
		putenv(env[i]);
	}

	if (settings->dumb == 0) {
		ruleset_execute(&settings->rules, event, settings);
	} else {
		action_dumb(settings, event);
	}

	for (i = 0; i < event->env_vars_c; i++) {
		unsetenv(event->env_vars[i].key);
		free(env[i]);
	}

	free(env);
}
