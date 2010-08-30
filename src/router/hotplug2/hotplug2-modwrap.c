#include "hotplug2-modwrap.h"

static int execute(char **argv) {
	pid_t p;
	int status;
	
	p = fork();
	switch (p) {
		case -1:
			return -1;
		case 0:
			execvp(argv[0], argv);
			exit(1);
			break;
		default:
			waitpid(p, &status, 0);
			break;
	}

	return WEXITSTATUS(status);
}

static FILE *modalias_open(void) {
	char *filename;
	FILE *fp;
	size_t len;
	struct utsname unamebuf;

	
	if (uname(&unamebuf))
		return NULL;
	
	len = strlen(MODULES_PATH) + strlen(unamebuf.release) + strlen(MODULES_ALIAS) + 1;
	filename = malloc(len);
	snprintf(filename, len, "%s%s%s", MODULES_PATH, unamebuf.release, MODULES_ALIAS);

	fp = fopen(filename, "r");
	free(filename);

	return fp;
}

static int modalias_read_line(FILE *fp, char **modalias, char **modname) {
	char line[512];
	char *sptr, *eptr;

	if (fgets(line, 512, fp) == NULL)
		return -1;
	
	/* "alias" */
	sptr = eptr = line;
	while (!isspace(*eptr) && *eptr != '\0') {
		eptr++;
	}
	if (*eptr == '\0')
		return -1;
	*eptr='\0';

	if (strcmp("alias", sptr))
		return -1;

	/* modalias */
	sptr = eptr+1;
	while (isspace(*sptr) && *sptr != '\0') {
		sptr++;
	}
	if (*sptr == '\0')
		return -1;
	eptr = sptr;
	while (!isspace(*eptr) && *eptr != '\0') {
		eptr++;
	}
	if (*eptr == '\0')
		return -1;
	*eptr='\0';
	*modalias = strdup(sptr);

	/* modname */
	sptr = eptr+1;
	while (isspace(*sptr) && *sptr != '\0') {
		sptr++;
	}
	if (*sptr == '\0') {
		free(*modalias);
		*modalias = NULL;
		return -1;
	}
	eptr = sptr;
	while (!isspace(*eptr) && *eptr != '\0') {
		eptr++;
	}
	if (*eptr == '\0') {
		free(*modalias);
		*modalias = NULL;
		return -1;
	}
	*eptr='\0';
	*modname = strdup(sptr);
	
	return 0;
}

int main(int argc, char *argv[]) {
	FILE *fp;
	char *match_modalias;
	char *modalias;
	char *modname;

	if (argc < 2) {
		fprintf(stderr, "Usage: hotplug2-modwrap [options for modprobe] <modalias>.\n");
		return 1;
	}

	argv[0] = getenv("MODPROBE_COMMAND");
	if (argv[0] == NULL)
		argv[0] = "/sbin/modprobe";

	fp = modalias_open();
	if (fp == NULL) {
		fprintf(stderr, "Unable to open modules.alias.\n");
		return 1;
	}

	match_modalias = argv[argc-1];
	while (!feof(fp)) {
		modalias = NULL;
		modname = NULL;

		if (modalias_read_line(fp, &modalias, &modname))
			continue;

		if (!fnmatch(modalias, match_modalias, 0)) {
			argv[argc - 1] = modname;
			if (execute(argv)) {
				fclose(fp);
				return 1;
			}
		}

		free(modalias);
		free(modname);
	}
	fclose(fp);
	return 0;
}
