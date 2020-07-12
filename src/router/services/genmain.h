#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>


static void handle_procdeps(int name, void (*start)(void), char *(*deps_func)(void), char *(*proc_func)(void), int force)
{
	char *deps = NULL;
	int state;
	if (deps_func) {
		deps = deps_func();
		dd_debug(DEBUG_SERVICE, "%s_deps exists, check nvram params %s\n", functiontable[name], deps);
		state = nvram_states(deps);
	}

	if (!state) {
		if (proc_func) {
			dd_debug(DEBUG_SERVICE, "%s_proc exists, check process\n", functiontable[name]);
			char *proc = proc_func();
			int pid = pidof(proc);
			dd_debug(DEBUG_SERVICE, "process name is %s, pid is %d\n", proc, pidof(proc));
			if (pid == -1)
				state = 1;
		}
	}
	if (force || state) {
		start();
		if (deps)
			nvram_states(deps);
	}

}

#define HANDLE_START(name, sym) \
	if (name == function) { \
		start_##sym(); \
		return 0; \
	}

#define HANDLE_START_DEPS(name, sym) \
	if (name == function) { \
		char * (*deps)(void) = sym##_deps; \
		void (*start)(void) = start_##sym; \
		handle_procdeps(name,start, deps,  NULL, force); \
		return 0; \
	}

#define HANDLE_START_PROC(name, sym) \
	if (name == function) { \
		char * (*proc)(void) = sym##_proc; \
		void (*start)(void) = start_##sym; \
		handle_procdeps(name,start,  NULL, proc, force); \
		return 0; \
	}

#define HANDLE_START_DEPS_PROC(name, sym) \
	if (name == function) { \
		char * (*deps)(void) = sym##_deps; \
		char * (*proc)(void) = sym##_proc; \
		void (*start)(void) = start_##sym; \
		handle_procdeps(name,start, deps, proc, force); \
		return 0; \
	}

#define HANDLE_STOP(name, sym) \
	if (name == function) { \
		stop_##sym(); \
		return 0; \
	}

#define HANDLE_MAIN(name, sym) \
	if (name == function) { \
		return sym##_main(argc-2, args); \
	}

#include <stdlib.h>
static char **buildargs(int argc, char *argv[])
{
	char **args = (char **)malloc(sizeof(char **) * argc);
	int i;
	for (i = 0; i < argc - 3; i++)
		args[i + 1] = argv[i + 3];
	args[0] = argv[1];
	return args;
}

void check_arguments(int argc, char *argv[], int *f)
{
	if (argc < 3) {
		fprintf(stderr, "%s servicename start|stop [-f]", argv[0]);
		fprintf(stderr, "list of services:\n");
		int i;
		for (i=0;i<sizeof(functiontable) / sizeof(char*);i++) {
		fprintf(stderr, "\t%s\n", functiontable[i]);		
		}
		exit(-1);
	}
	int i;
	for (i=0;i<sizeof(functiontable) / sizeof(char*);i++) {
		if (!strcmp(functiontable[i], argv[1])) {
		    *f = i;
		    return;
		}
	}
	dd_debug(DEBUG_SERVICE, "function %s_%s not found\n", argv[2], argv[1]);
	exit(-1);
}