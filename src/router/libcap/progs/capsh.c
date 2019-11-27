/*
 * Copyright (c) 2008-11,16 Andrew G. Morgan <morgan@kernel.org>
 *
 * This is a simple 'bash' wrapper program that can be used to
 * raise and lower both the bset and pI capabilities before invoking
 * /bin/bash (hardcoded right now).
 *
 * The --print option can be used as a quick test whether various
 * capability manipulations work as expected (or not).
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <errno.h>
#include <ctype.h>
#include <sys/capability.h>
#include <sys/securebits.h>
#include <sys/wait.h>
#include <sys/prctl.h>

#define MAX_GROUPS       100   /* max number of supplementary groups for user */

static char *binary(unsigned long value)
{
    static char string[8*sizeof(unsigned long) + 1];
    unsigned i;

    i = sizeof(string);
    string[--i] = '\0';
    do {
	string[--i] = (value & 1) ? '1' : '0';
	value >>= 1;
    } while ((i > 0) && value);
    return string + i;
}

static void display_prctl_set(const char *name, int (*fn)(cap_value_t))
{
    unsigned cap;
    const char *sep;
    int set;

    printf("%s set =", name);
    for (sep = "", cap=0; (set = fn(cap)) >= 0; cap++) {
	char *ptr;
	if (!set) {
	    continue;
	}

	ptr = cap_to_name(cap);
	if (ptr == NULL) {
	    printf("%s%u", sep, cap);
	} else {
	    printf("%s%s", sep, ptr);
	    cap_free(ptr);
	}
	sep = ",";
    }
    if (!cap) {
	printf(" <unsupported>\n");
    } else {
	printf("\n");
    }
}

/* arg_print displays the current capability state of the process */
static void arg_print(void)
{
    int set, status, j;
    cap_t all;
    char *text;
    const char *sep;
    struct group *g;
    gid_t groups[MAX_GROUPS], gid;
    uid_t uid;
    struct passwd *u;

    all = cap_get_proc();
    text = cap_to_text(all, NULL);
    printf("Current: %s\n", text);
    cap_free(text);
    cap_free(all);

    display_prctl_set("Bounding", cap_get_bound);
    display_prctl_set("Ambient", cap_get_ambient);
    set = prctl(PR_GET_SECUREBITS);
    if (set >= 0) {
	const char *b;
	b = binary(set);  /* use verilog convention for binary string */
	printf("Securebits: 0%o/0x%x/%u'b%s\n", set, set,
	       (unsigned) strlen(b), b);
	printf(" secure-noroot: %s (%s)\n",
	       (set & SECBIT_NOROOT) ? "yes":"no",
	       (set & SECBIT_NOROOT_LOCKED) ? "locked":"unlocked");
	printf(" secure-no-suid-fixup: %s (%s)\n",
	       (set & SECBIT_NO_SETUID_FIXUP) ? "yes":"no",
	       (set & SECBIT_NO_SETUID_FIXUP_LOCKED) ? "locked":"unlocked");
	printf(" secure-keep-caps: %s (%s)\n",
	       (set & SECBIT_KEEP_CAPS) ? "yes":"no",
	       (set & SECBIT_KEEP_CAPS_LOCKED) ? "locked":"unlocked");
	if (CAP_AMBIENT_SUPPORTED()) {
	    printf(" secure-no-ambient-raise: %s (%s)\n",
		   (set & SECBIT_NO_CAP_AMBIENT_RAISE) ? "yes":"no",
		   (set & SECBIT_NO_CAP_AMBIENT_RAISE_LOCKED) ?
		   "locked":"unlocked");
	}
    } else {
	printf("[Securebits ABI not supported]\n");
	set = prctl(PR_GET_KEEPCAPS);
	if (set >= 0) {
	    printf(" prctl-keep-caps: %s (locking not supported)\n",
		   set ? "yes":"no");
	} else {
	    printf("[Keepcaps ABI not supported]\n");
	}
    }
    uid = getuid();
    u = getpwuid(uid);
    printf("uid=%u(%s)\n", getuid(), u ? u->pw_name : "???");
    gid = getgid();
    g = getgrgid(gid);
    printf("gid=%u(%s)\n", gid, g ? g->gr_name : "???");
    printf("groups=");
    status = getgroups(MAX_GROUPS, groups);
    sep = "";
    for (j=0; j < status; j++) {
	g = getgrgid(groups[j]);
	printf("%s%u(%s)", sep, groups[j], g ? g->gr_name : "???");
	sep = ",";
    }
    printf("\n");
}

static const cap_value_t raise_setpcap[1] = { CAP_SETPCAP };
static const cap_value_t raise_chroot[1] = { CAP_SYS_CHROOT };

static void push_pcap(cap_t *orig_p, cap_t *raised_for_setpcap_p)
{
    /*
     * We need to do this here because --inh=XXX may have reset
     * orig and it isn't until we are within the --drop code that
     * we know what the prevailing (orig) pI value is.
     */
    *orig_p = cap_get_proc();
    if (NULL == *orig_p) {
	perror("Capabilities not available");
	exit(1);
    }

    *raised_for_setpcap_p = cap_dup(*orig_p);
    if (NULL == *raised_for_setpcap_p) {
	fprintf(stderr, "modification requires CAP_SETPCAP\n");
	exit(1);
    }
    if (cap_set_flag(*raised_for_setpcap_p, CAP_EFFECTIVE, 1,
		     raise_setpcap, CAP_SET) != 0) {
	perror("unable to select CAP_SETPCAP");
	exit(1);
    }
}

static void pop_pcap(cap_t orig, cap_t raised_for_setpcap)
{
    cap_free(raised_for_setpcap);
    cap_free(orig);
}

static void arg_drop(const char *arg_names)
{
    char *ptr;
    cap_t orig, raised_for_setpcap;
    char *names;

    push_pcap(&orig, &raised_for_setpcap);
    if (strcmp("all", arg_names) == 0) {
	unsigned j = 0;
	while (CAP_IS_SUPPORTED(j)) {
	    int status;
	    if (cap_set_proc(raised_for_setpcap) != 0) {
		perror("unable to raise CAP_SETPCAP for BSET changes");
		exit(1);
	    }
	    status = cap_drop_bound(j);
	    if (cap_set_proc(orig) != 0) {
		perror("unable to lower CAP_SETPCAP post BSET change");
		exit(1);
	    }
	    if (status != 0) {
		char *name_ptr;

		name_ptr = cap_to_name(j);
		fprintf(stderr, "Unable to drop bounding capability [%s]\n",
			name_ptr);
		cap_free(name_ptr);
		exit(1);
	    }
	    j++;
	}
	pop_pcap(orig, raised_for_setpcap);
	return;
    }

    names = strdup(arg_names);
    if (NULL == names) {
	fprintf(stderr, "failed to allocate names\n");
	exit(1);
    }
    for (ptr = names; (ptr = strtok(ptr, ",")); ptr = NULL) {
	/* find name for token */
	cap_value_t cap;
	int status;

	if (cap_from_name(ptr, &cap) != 0) {
	    fprintf(stderr, "capability [%s] is unknown to libcap\n", ptr);
	    exit(1);
	}
	if (cap_set_proc(raised_for_setpcap) != 0) {
	    perror("unable to raise CAP_SETPCAP for BSET changes");
	    exit(1);
	}
	status = cap_drop_bound(cap);
	if (cap_set_proc(orig) != 0) {
	    perror("unable to lower CAP_SETPCAP post BSET change");
	    exit(1);
	}
	if (status != 0) {
	    fprintf(stderr, "failed to drop [%s=%u]\n", ptr, cap);
	    exit(1);
	}
    }
    pop_pcap(orig, raised_for_setpcap);
    free(names);
}

static void arg_change_amb(const char *arg_names, cap_flag_value_t set)
{
    char *ptr;
    cap_t orig, raised_for_setpcap;
    char *names;

    push_pcap(&orig, &raised_for_setpcap);
    if (strcmp("all", arg_names) == 0) {
	unsigned j = 0;
	while (CAP_IS_SUPPORTED(j)) {
	    int status;
	    if (cap_set_proc(raised_for_setpcap) != 0) {
		perror("unable to raise CAP_SETPCAP for AMBIENT changes");
		exit(1);
	    }
	    status = cap_set_ambient(j, set);
	    if (cap_set_proc(orig) != 0) {
		perror("unable to lower CAP_SETPCAP post AMBIENT change");
		exit(1);
	    }
	    if (status != 0) {
		char *name_ptr;

		name_ptr = cap_to_name(j);
		fprintf(stderr, "Unable to %s ambient capability [%s]\n",
			set == CAP_CLEAR ? "clear":"raise", name_ptr);
		cap_free(name_ptr);
		exit(1);
	    }
	    j++;
	}
	pop_pcap(orig, raised_for_setpcap);
	return;
    }

    names = strdup(arg_names);
    if (NULL == names) {
	fprintf(stderr, "failed to allocate names\n");
	exit(1);
    }
    for (ptr = names; (ptr = strtok(ptr, ",")); ptr = NULL) {
	/* find name for token */
	cap_value_t cap;
	int status;

	if (cap_from_name(ptr, &cap) != 0) {
	    fprintf(stderr, "capability [%s] is unknown to libcap\n", ptr);
	    exit(1);
	}
	if (cap_set_proc(raised_for_setpcap) != 0) {
	    perror("unable to raise CAP_SETPCAP for AMBIENT changes");
	    exit(1);
	}
	status = cap_set_ambient(cap, set);
	if (cap_set_proc(orig) != 0) {
	    perror("unable to lower CAP_SETPCAP post AMBIENT change");
	    exit(1);
	}
	if (status != 0) {
	    fprintf(stderr, "failed to %s ambient [%s=%u]\n",
		    set == CAP_CLEAR ? "clear":"raise", ptr, cap);
	    exit(1);
	}
    }
    pop_pcap(orig, raised_for_setpcap);
    free(names);
}

int main(int argc, char *argv[], char *envp[])
{
    pid_t child;
    unsigned i;

    child = 0;

    for (i=1; i<argc; ++i) {
	if (!strncmp("--drop=", argv[i], 7)) {
	    arg_drop(argv[i]+7);
	} else if (!strcmp("--has-ambient", argv[i])) {
	    if (!CAP_AMBIENT_SUPPORTED()) {
		fprintf(stderr, "ambient set not supported\n");
		exit(1);
	    }
	} else if (!strncmp("--addamb=", argv[i], 9)) {
	    arg_change_amb(argv[i]+9, CAP_SET);
	} else if (!strncmp("--delamb=", argv[i], 9)) {
	    arg_change_amb(argv[i]+9, CAP_CLEAR);
	} else if (!strncmp("--noamb", argv[i], 7)) {
	    if (cap_reset_ambient() != 0) {
		fprintf(stderr, "failed to reset ambient set\n");
		exit(1);
	    }
	} else if (!strncmp("--inh=", argv[i], 6)) {
	    cap_t all, raised_for_setpcap;
	    char *text;
	    char *ptr;

	    all = cap_get_proc();
	    if (all == NULL) {
		perror("Capabilities not available");
		exit(1);
	    }
	    if (cap_clear_flag(all, CAP_INHERITABLE) != 0) {
		perror("libcap:cap_clear_flag() internal error");
		exit(1);
	    }

	    raised_for_setpcap = cap_dup(all);
	    if ((raised_for_setpcap != NULL)
		&& (cap_set_flag(raised_for_setpcap, CAP_EFFECTIVE, 1,
				 raise_setpcap, CAP_SET) != 0)) {
		cap_free(raised_for_setpcap);
		raised_for_setpcap = NULL;
	    }

	    text = cap_to_text(all, NULL);
	    cap_free(all);
	    if (text == NULL) {
		perror("Fatal error concerning process capabilities");
		exit(1);
	    }
	    ptr = malloc(10 + strlen(argv[i]+6) + strlen(text));
	    if (ptr == NULL) {
		perror("Out of memory for inh set");
		exit(1);
	    }
	    if (argv[i][6] && strcmp("none", argv[i]+6)) {
		sprintf(ptr, "%s %s+i", text, argv[i]+6);
	    } else {
		strcpy(ptr, text);
	    }

	    all = cap_from_text(ptr);
	    if (all == NULL) {
		perror("Fatal error internalizing capabilities");
		exit(1);
	    }
	    cap_free(text);
	    free(ptr);

	    if (raised_for_setpcap != NULL) {
		/*
		 * This is only for the case that pP does not contain
		 * the requested change to pI.. Failing here is not
		 * indicative of the cap_set_proc(all) failing (always).
		 */
		(void) cap_set_proc(raised_for_setpcap);
		cap_free(raised_for_setpcap);
		raised_for_setpcap = NULL;
	    }

	    if (cap_set_proc(all) != 0) {
		perror("Unable to set inheritable capabilities");
		exit(1);
	    }
	    /*
	     * Since status is based on orig, we don't want to restore
	     * the previous value of 'all' again here!
	     */

	    cap_free(all);
	} else if (!strncmp("--caps=", argv[i], 7)) {
	    cap_t all, raised_for_setpcap;

	    raised_for_setpcap = cap_get_proc();
	    if (raised_for_setpcap == NULL) {
		perror("Capabilities not available");
		exit(1);
	    }

	    if ((raised_for_setpcap != NULL)
		&& (cap_set_flag(raised_for_setpcap, CAP_EFFECTIVE, 1,
				 raise_setpcap, CAP_SET) != 0)) {
		cap_free(raised_for_setpcap);
		raised_for_setpcap = NULL;
	    }

	    all = cap_from_text(argv[i]+7);
	    if (all == NULL) {
		fprintf(stderr, "unable to interpret [%s]\n", argv[i]);
		exit(1);
	    }

	    if (raised_for_setpcap != NULL) {
		/*
		 * This is only for the case that pP does not contain
		 * the requested change to pI.. Failing here is not
		 * indicative of the cap_set_proc(all) failing (always).
		 */
		(void) cap_set_proc(raised_for_setpcap);
		cap_free(raised_for_setpcap);
		raised_for_setpcap = NULL;
	    }

	    if (cap_set_proc(all) != 0) {
		fprintf(stderr, "Unable to set capabilities [%s]\n", argv[i]);
		exit(1);
	    }
	    /*
	     * Since status is based on orig, we don't want to restore
	     * the previous value of 'all' again here!
	     */

	    cap_free(all);
	} else if (!strncmp("--keep=", argv[i], 7)) {
	    unsigned value;
	    int set;

	    value = strtoul(argv[i]+7, NULL, 0);
	    set = prctl(PR_SET_KEEPCAPS, value);
	    if (set < 0) {
		fprintf(stderr, "prctl(PR_SET_KEEPCAPS, %u) failed: %s\n",
			value, strerror(errno));
		exit(1);
	    }
	} else if (!strncmp("--chroot=", argv[i], 9)) {
	    int status;
	    cap_t orig, raised_for_chroot;

	    orig = cap_get_proc();
	    if (orig == NULL) {
		perror("Capabilities not available");
		exit(1);
	    }

	    raised_for_chroot = cap_dup(orig);
	    if (raised_for_chroot == NULL) {
		perror("Unable to duplicate capabilities");
		exit(1);
	    }

	    if (cap_set_flag(raised_for_chroot, CAP_EFFECTIVE, 1, raise_chroot,
			     CAP_SET) != 0) {
		perror("unable to select CAP_SET_SYS_CHROOT");
		exit(1);
	    }

	    if (cap_set_proc(raised_for_chroot) != 0) {
		perror("unable to raise CAP_SYS_CHROOT");
		exit(1);
	    }
	    cap_free(raised_for_chroot);

	    status = chroot(argv[i]+9);
	    if (cap_set_proc(orig) != 0) {
		perror("unable to lower CAP_SYS_CHROOT");
		exit(1);
	    }
	    /*
	     * Given we are now in a new directory tree, its good practice
	     * to start off in a sane location
	     */
	    status = chdir("/");

	    cap_free(orig);

	    if (status != 0) {
		fprintf(stderr, "Unable to chroot/chdir to [%s]", argv[i]+9);
		exit(1);
	    }
	} else if (!strncmp("--secbits=", argv[i], 10)) {
	    unsigned value;
	    int status;

	    value = strtoul(argv[i]+10, NULL, 0);
	    status = prctl(PR_SET_SECUREBITS, value);
	    if (status < 0) {
		fprintf(stderr, "failed to set securebits to 0%o/0x%x\n",
			value, value);
		exit(1);
	    }
	} else if (!strncmp("--forkfor=", argv[i], 10)) {
	    unsigned value;

	    value = strtoul(argv[i]+10, NULL, 0);
	    if (value == 0) {
		goto usage;
	    }
	    child = fork();
	    if (child < 0) {
		perror("unable to fork()");
	    } else if (!child) {
		sleep(value);
		exit(0);
	    }
	} else if (!strncmp("--killit=", argv[i], 9)) {
	    int retval, status;
	    pid_t result;
	    unsigned value;

	    value = strtoul(argv[i]+9, NULL, 0);
	    if (!child) {
		fprintf(stderr, "no forked process to kill\n");
		exit(1);
	    }
	    retval = kill(child, value);
	    if (retval != 0) {
		perror("Unable to kill child process");
		exit(1);
	    }
	    result = waitpid(child, &status, 0);
	    if (result != child) {
		fprintf(stderr, "waitpid didn't match child: %u != %u\n",
			child, result);
		exit(1);
	    }
	    if (WTERMSIG(status) != value) {
		fprintf(stderr, "child terminated with odd signal (%d != %d)\n"
			, value, WTERMSIG(status));
		exit(1);
	    }
	} else if (!strncmp("--uid=", argv[i], 6)) {
	    unsigned value;
	    int status;

	    value = strtoul(argv[i]+6, NULL, 0);
	    status = setuid(value);
	    if (status < 0) {
		fprintf(stderr, "Failed to set uid=%u: %s\n",
			value, strerror(errno));
		exit(1);
	    }
	} else if (!strncmp("--gid=", argv[i], 6)) {
	    unsigned value;
	    int status;

	    value = strtoul(argv[i]+6, NULL, 0);
	    status = setgid(value);
	    if (status < 0) {
		fprintf(stderr, "Failed to set gid=%u: %s\n",
			value, strerror(errno));
		exit(1);
	    }
        } else if (!strncmp("--groups=", argv[i], 9)) {
	  char *ptr, *buf;
	  long length, max_groups;
	  gid_t *group_list;
	  int g_count;

	  length = sysconf(_SC_GETGR_R_SIZE_MAX);
	  buf = calloc(1, length);
	  if (NULL == buf) {
	    fprintf(stderr, "No memory for [%s] operation\n", argv[i]);
	    exit(1);
	  }

	  max_groups = sysconf(_SC_NGROUPS_MAX);
	  group_list = calloc(max_groups, sizeof(gid_t));
	  if (NULL == group_list) {
	    fprintf(stderr, "No memory for gid list\n");
	    exit(1);
	  }

	  g_count = 0;
	  for (ptr = argv[i] + 9; (ptr = strtok(ptr, ","));
	       ptr = NULL, g_count++) {
	    if (max_groups <= g_count) {
	      fprintf(stderr, "Too many groups specified (%d)\n", g_count);
	      exit(1);
	    }
	    if (!isdigit(*ptr)) {
	      struct group *g, grp;
	      getgrnam_r(ptr, &grp, buf, length, &g);
	      if (NULL == g) {
		fprintf(stderr, "Failed to identify gid for group [%s]\n", ptr);
		exit(1);
	      }
	      group_list[g_count] = g->gr_gid;
	    } else {
	      group_list[g_count] = strtoul(ptr, NULL, 0);
	    }
	  }
	  free(buf);
	  if (setgroups(g_count, group_list) != 0) {
	    fprintf(stderr, "Failed to setgroups.\n");
	    exit(1);
	  }
	  free(group_list);
	} else if (!strncmp("--user=", argv[i], 7)) {
	    struct passwd *pwd;
	    const char *user;
	    gid_t groups[MAX_GROUPS];
	    int status, ngroups;

	    user = argv[i] + 7;
	    pwd = getpwnam(user);
	    if (pwd == NULL) {
	      fprintf(stderr, "User [%s] not known\n", user);
	      exit(1);
	    }
	    ngroups = MAX_GROUPS;
	    status = getgrouplist(user, pwd->pw_gid, groups, &ngroups);
	    if (status < 1) {
	      perror("Unable to get group list for user");
	      exit(1);
	    }
	    status = setgroups(ngroups, groups);
	    if (status != 0) {
	      perror("Unable to set group list for user");
	      exit(1);
	    }
	    status = setgid(pwd->pw_gid);
	    if (status < 0) {
		fprintf(stderr, "Failed to set gid=%u(user=%s): %s\n",
			pwd->pw_gid, user, strerror(errno));
		exit(1);
	    }
	    status = setuid(pwd->pw_uid);
	    if (status < 0) {
		fprintf(stderr, "Failed to set uid=%u(user=%s): %s\n",
			pwd->pw_uid, user, strerror(errno));
		exit(1);
	    }
	} else if (!strncmp("--decode=", argv[i], 9)) {
	    unsigned long long value;
	    unsigned cap;
	    const char *sep = "";

	    /* Note, if capabilities become longer than 64-bits we'll need
	       to fixup the following code.. */
	    value = strtoull(argv[i]+9, NULL, 16);
	    printf("0x%016llx=", value);

	    for (cap=0; (cap < 64) && (value >> cap); ++cap) {
		if (value & (1ULL << cap)) {
		    char *ptr;

		    ptr = cap_to_name(cap);
		    if (ptr != NULL) {
			printf("%s%s", sep, ptr);
			cap_free(ptr);
		    } else {
			printf("%s%u", sep, cap);
		    }
		    sep = ",";
		}
	    }
	    printf("\n");
        } else if (!strncmp("--supports=", argv[i], 11)) {
	    cap_value_t cap;

	    if (cap_from_name(argv[i] + 11, &cap) < 0) {
		fprintf(stderr, "cap[%s] not recognized by library\n",
			argv[i] + 11);
		exit(1);
	    }
	    if (!CAP_IS_SUPPORTED(cap)) {
		fprintf(stderr, "cap[%s=%d] not supported by kernel\n",
			argv[i] + 11, cap);
		exit(1);
	    }
	} else if (!strcmp("--print", argv[i])) {
	    arg_print();
	} else if ((!strcmp("--", argv[i])) || (!strcmp("==", argv[i]))) {
	    argv[i] = strdup(argv[i][0] == '-' ? "/bin/bash" : argv[0]);
	    argv[argc] = NULL;
	    execve(argv[i], argv+i, envp);
	    fprintf(stderr, "execve /bin/bash failed!\n");
	    exit(1);
	} else {
	usage:
	    printf("usage: %s [args ...]\n"
		   "  --help         this message (or try 'man capsh')\n"
		   "  --print        display capability relevant state\n"
		   "  --decode=xxx   decode a hex string to a list of caps\n"
		   "  --supports=xxx exit 1 if capability xxx unsupported\n"
		   "  --drop=xxx     remove xxx,.. capabilities from bset\n"
		   "  --addamb=xxx   add xxx,... capabilities to ambient set\n"
		   "  --delamb=xxx   remove xxx,... capabilities from ambient\n"
		   "  --noamb=xxx    reset the ambient capabilities\n"
		   "  --caps=xxx     set caps as per cap_from_text()\n"
		   "  --inh=xxx      set xxx,.. inheritiable set\n"
		   "  --secbits=<n>  write a new value for securebits\n"
		   "  --keep=<n>     set keep-capabability bit to <n>\n"
		   "  --uid=<n>      set uid to <n> (hint: id <username>)\n"
		   "  --gid=<n>      set gid to <n> (hint: id <username>)\n"
		   "  --groups=g,... set the supplemental groups\n"
                   "  --user=<name>  set uid,gid and groups to that of user\n"
		   "  --chroot=path  chroot(2) to this path\n"
		   "  --killit=<n>   send signal(n) to child\n"
		   "  --forkfor=<n>  fork and make child sleep for <n> sec\n"
		   "  ==             re-exec(capsh) with args as for --\n"
		   "  --             remaing arguments are for /bin/bash\n"
		   "                 (without -- [%s] will simply exit(0))\n",
		   argv[0], argv[0]);

	    exit(strcmp("--help", argv[i]) != 0);
	}
    }

    exit(0);
}
