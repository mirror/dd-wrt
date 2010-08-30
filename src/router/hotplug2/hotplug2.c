#include "hotplug2.h"

/* Controls the loop receiving events. */
int process = 1;
struct settings_t global_settings;

/**
 * Evaluates an argument into a true/false value.
 *
 * @1 argument
 * @2 argument flag
 * @3 pointer to output value
 *
 * Returns: 0 if success, -1 otherwise
 */
static int get_bool_opt(char *argv, char *name, int *value) {
	int rv = -1;
	
	if (!strncmp(argv, "--no-", 5)) {
		rv = 0;
		argv+=5;
	}
	
	if (!strncmp(argv, "--", 2)) {
		rv = 1;
		argv+=2;
	}
	
	if (rv == -1)
		return -1;
	
	if (!strcmp(argv, name)) {
		*value = rv;
		return 0;
	} else {
		return -1;
	}
}

/**
 * Attempt to figure out whether our modprobe command can handle modalias.
 * If not, use our own wrapper.
 *
 * Returns: modprobe command if succesful, NULL otherwise
 */
static char *get_modprobe_command() {
	char buf[18];
	char *modprobe_command;
	int fds[2];
	pid_t p;
	FILE *fp;
	
	pipe(fds);
	p = fork();

	buf[0] = '\0';
	
	switch (p) {
		case -1:
			return NULL;
		
		case 0:
			close(fds[0]);
			close(2);
			dup2(fds[1], 1);
		
			execlp("/sbin/modprobe", "/sbin/modprobe", "--version", NULL);
			exit(1);
			break;
	
		default:
			close(fds[1]);
			fp = fdopen(fds[0], "r");
			fread(buf, 1, 17, fp);
			buf[17]='\0';
		
			/* 
			 * module-init-tools can handle aliases.
			 * If we do not have a match, we use hotplug2-depwrap,
			 * even though our modprobe can do fnmatch aliases,
			 * which is the case of eg. busybox.
			 */
			if (!strcmp(buf, "module-init-tools")) {
				modprobe_command = "/sbin/modprobe";
			} else {
				modprobe_command = "/sbin/hotplug2-depwrap";
			}
			fclose(fp);
			waitpid(p, NULL, 0);
			break;
	}

	return strdup(modprobe_command);
}

static int coldplug(const char *command) {
	int status;
	pid_t p;

	p = fork();
	switch (p) {
		case -1:
			return -1;

		case 0:
			execlp(command, command, NULL);
			exit(1);

		default:
			waitpid(p, &status, 0);
			return WEXITSTATUS(status);
	}
	return -1;
}

/**
 * Loads various settings and rules file.
 *
 * @1 Settings structure
 * @2 Argument count
 * @3 String arguments, terminated by NULL
 *
 * Returns: 0 if success, -1 if non-recoverable error
 */
static int load_settings(struct settings_t *settings, int argc, char *argv[]) {
	int i;
	struct options_t boolopts[] = {
		{"persistent", &settings->persistent},
		{"override", &settings->useflags},	/* compatibility */
		{"useflags", &settings->useflags},
		{"dumb", &settings->dumb},
		{NULL, NULL}
	};

	/*
	 * We parse all the options...
	 */
	settings->worker_argc = argc;
	settings->worker_argv = argv;

	argc--;
	argv++;
	while (argc > 0) {
		for (i = 0; boolopts[i].name != NULL; i++) {
			if (!get_bool_opt(*argv, boolopts[i].name, boolopts[i].value)) {
				/*
				 * Bool options are --option or --no-options. If we handled
				 * it, quit iterating.
				 */
				break;
			}
		}

		if (!strcmp(*argv, "--set-coldplug-cmd")) {
			argv++;
			argc--;
			if (argc <= 0)
				break;
			
			settings->coldplug_command = strdup(*argv);
		} else if (!strcmp(*argv, "--set-modprobe-cmd")) {
			argv++;
			argc--;
			if (argc <= 0)
				break;
			
			settings->modprobe_command = strdup(*argv);
		} else if (!strcmp(*argv, "--set-rules-file")) {
			argv++;
			argc--;
			if (argc <= 0)
				break;
			
			settings->rules_file = strdup(*argv);
		} else if (!strcmp(*argv, "--set-worker")) {
			argv++;
			argc--;
			if (argc <= 0)
				break;
			
			settings->worker_name = strdup(*argv);
		}
		argv++;
		argc--;
	}

	if (settings->modprobe_command == NULL)
		settings->modprobe_command = get_modprobe_command();
	
	if (!settings->dumb && settings->rules_file != NULL) {
		return parser_file(settings->rules_file, &settings->rules);
	}

	return 0;
}

static void sighandler(int signum) {
	switch (signum) {
		case SIGINT:
		case SIGTERM:
			/* Terminate the loop. */
			process = 0;
			break;

		case SIGUSR1:
			/* Switch persistency. */
			global_settings.persistent = !global_settings.persistent;
			break;
	}
}

/*
 * Check for terminating conditions.
 */
static int terminate_condition(struct settings_t *settings, struct uevent_t *uevent) {
	event_seqnum_t current_seqnum;
	int rv;
	fd_set fdrset;
	struct timeval tv;

	/*
	 * If this is non-persistent invokation and last event
	 * has been processed, terminate.
	 */
	if (settings->persistent == 0) {
		rv = seqnum_get(&current_seqnum);
		if (rv == 0 && current_seqnum == uevent->seqnum) {
			/* Confirm value of seqnum after grace time. */
			usleep(settings->grace);
			rv = seqnum_get(&current_seqnum);
			if (rv == 0 && current_seqnum == uevent->seqnum)
				return 1;
		} else if (rv != 0) {
			/* If it has failed, let's try and guess. */
			tv.tv_sec = 0;
			tv.tv_usec = settings->grace;
			FD_ZERO(&fdrset);
			FD_SET(settings->netlink_socket, &fdrset);
			select(settings->netlink_socket+1, &fdrset, NULL, NULL, &tv);
			/* Did anything arrive within grace time? */
			return !FD_ISSET(settings->netlink_socket, &fdrset);
		}
	}

	return 0;
}

int main(int argc, char *argv[]) {
	char buffer[UEVENT_BUFFER_SIZE+512];
	ssize_t size;
	struct sigaction sigact;
	struct settings_t *settings;
	struct uevent_t *uevent;
	void *worker_ctx;

	settings = &global_settings;
	settings_init(settings);
	if (load_settings(settings, argc, argv)) {
		settings_clear(settings);
		exit(1);
	}

	/* Load the worker. */
#ifndef STATIC_WORKER
	if (settings->worker_name == NULL) {
		fprintf(stderr, "Missing worker name.\n");
		settings_clear(settings);
		exit(1);
	}
#endif
	settings->worker = worker_load(settings->worker_name);
#ifndef STATIC_WORKER
	if (settings->worker == NULL) {
		fprintf(stderr, "Unable to load worker: %s\n", settings->worker_name);
		settings_clear(settings);
		exit(1);
	}
#endif
	
	/* Prepare a netlink connection to the kernel. */
	settings->netlink_socket = netlink_init();
	if (netlink_bind(settings->netlink_socket) == -1) {
		fprintf(stderr, "Unable to connect to netlink socket.\n");
		settings_clear(settings);
		exit(1);
	}

	if (settings->coldplug_command != NULL)
		coldplug(settings->coldplug_command);

	/* 
	 * Catch relevant signals. We use sigaction instead
	 * of signal, becaue signal implies SA_RESTART, which
	 * is undesirable.
	 */
	sigact.sa_handler = sighandler;
	sigact.sa_flags = 0;
	sigact.sa_restorer = NULL;
	sigemptyset(&sigact.sa_mask);
	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGTERM, &sigact, NULL);
	sigaction(SIGUSR1, &sigact, NULL);

	worker_ctx = settings->worker->module->init(settings);
	while (process) {
		size = recv(settings->netlink_socket, &buffer, sizeof(buffer), 0);
		if (size < 0)
			continue;

		uevent = uevent_deserialize(buffer, size);
		
		if (uevent == NULL)
			continue;

		/* Update highest seqnum. */
		if (uevent->seqnum > settings->highest_seqnum)
			settings->highest_seqnum = uevent->seqnum;

		/* Pass the uevent to the loaded worker. */
		if (settings->worker->module->process(worker_ctx, uevent)) {
			fprintf(stderr, "Unable to pass event to worker.\n");
			process = 0;
		}

		/* If we are supposed to terminate, let's do it. */
		if (terminate_condition(settings, uevent))
			process = 0;

		uevent_free(uevent);
	}

	/* Unload worker. This may take a while (waiting for children etc.). */
	settings->worker->module->deinit(worker_ctx);
	worker_free(settings->worker);

	/* Close socket, free remaining resources. */
	close(settings->netlink_socket);
	settings_clear(settings);

	return 0;
}
