/*
 * Copyright (c) 1999,2007 Andrew G. Morgan <morgan@kernel.org>
 *
 * The purpose of this module is to enforce inheritable capability sets
 * for a specified user.
 */

/* #define DEBUG */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <syslog.h>

#include <sys/capability.h>

#include <security/pam_modules.h>
#include <security/_pam_macros.h>

#define USER_CAP_FILE           "/etc/security/capability.conf"
#define CAP_FILE_BUFFER_SIZE    4096
#define CAP_FILE_DELIMITERS     " \t\n"
#define CAP_COMBINED_FORMAT     "%s all-i %s+i"
#define CAP_DROP_ALL            "%s all-i"

struct pam_cap_s {
    int debug;
    const char *user;
    const char *conf_filename;
};

/* obtain the inheritable capabilities for the current user */

static char *read_capabilities_for_user(const char *user, const char *source)
{
    char *cap_string = NULL;
    char buffer[CAP_FILE_BUFFER_SIZE], *line;
    FILE *cap_file;

    cap_file = fopen(source, "r");
    if (cap_file == NULL) {
	D(("failed to open capability file"));
	return NULL;
    }

    while ((line = fgets(buffer, CAP_FILE_BUFFER_SIZE, cap_file))) {
	int found_one = 0;
	const char *cap_text;

	cap_text = strtok(line, CAP_FILE_DELIMITERS);

	if (cap_text == NULL) {
	    D(("empty line"));
	    continue;
	}
	if (*cap_text == '#') {
	    D(("comment line"));
	    continue;
	}

	while ((line = strtok(NULL, CAP_FILE_DELIMITERS))) {

	    if (strcmp("*", line) == 0) {
		D(("wildcard matched"));
		found_one = 1;
		cap_string = strdup(cap_text);
		break;
	    }

	    if (strcmp(user, line) == 0) {
		D(("exact match for user"));
		found_one = 1;
		cap_string = strdup(cap_text);
		break;
	    }

	    D(("user is not [%s] - skipping", line));
	}

	cap_text = NULL;
	line = NULL;

	if (found_one) {
	    D(("user [%s] matched - caps are [%s]", user, cap_string));
	    break;
	}
    }

    fclose(cap_file);

    memset(buffer, 0, CAP_FILE_BUFFER_SIZE);

    return cap_string;
}

/*
 * Set capabilities for current process to match the current
 * permitted+executable sets combined with the configured inheritable
 * set.
 */

static int set_capabilities(struct pam_cap_s *cs)
{
    cap_t cap_s;
    ssize_t length = 0;
    char *conf_icaps;
    char *proc_epcaps;
    char *combined_caps;
    int ok = 0;

    cap_s = cap_get_proc();
    if (cap_s == NULL) {
	D(("your kernel is capability challenged - upgrade: %s",
	   strerror(errno)));
	return 0;
    }

    conf_icaps =
	read_capabilities_for_user(cs->user,
				   cs->conf_filename
				   ? cs->conf_filename:USER_CAP_FILE );
    if (conf_icaps == NULL) {
	D(("no capabilities found for user [%s]", cs->user));
	goto cleanup_cap_s;
    }

    proc_epcaps = cap_to_text(cap_s, &length);
    if (proc_epcaps == NULL) {
	D(("unable to convert process capabilities to text"));
	goto cleanup_icaps;
    }

    /*
     * This is a pretty inefficient way to combine
     * capabilities. However, it seems to be the most straightforward
     * one, given the limitations of the POSIX.1e draft spec. The spec
     * is optimized for applications that know the capabilities they
     * want to manipulate at compile time.
     */

    combined_caps = malloc(1+strlen(CAP_COMBINED_FORMAT)
			   +strlen(proc_epcaps)+strlen(conf_icaps));
    if (combined_caps == NULL) {
	D(("unable to combine capabilities into one string - no memory"));
	goto cleanup_epcaps;
    }

    if (!strcmp(conf_icaps, "none")) {
	sprintf(combined_caps, CAP_DROP_ALL, proc_epcaps);
    } else if (!strcmp(conf_icaps, "all")) {
	/* no change */
	sprintf(combined_caps, "%s", proc_epcaps);
    } else {
	sprintf(combined_caps, CAP_COMBINED_FORMAT, proc_epcaps, conf_icaps);
    }
    D(("combined_caps=[%s]", combined_caps));

    cap_free(cap_s);
    cap_s = cap_from_text(combined_caps);
    _pam_overwrite(combined_caps);
    _pam_drop(combined_caps);

#ifdef DEBUG
    {
        char *temp = cap_to_text(cap_s, NULL);
	D(("abbreviated caps for process will be [%s]", temp));
	cap_free(temp);
    }
#endif /* DEBUG */

    if (cap_s == NULL) {
	D(("no capabilies to set"));
    } else if (cap_set_proc(cap_s) == 0) {
	D(("capabilities were set correctly"));
	ok = 1;
    } else {
	D(("failed to set specified capabilities: %s", strerror(errno)));
    }

cleanup_epcaps:
    cap_free(proc_epcaps);

cleanup_icaps:
    _pam_overwrite(conf_icaps);
    _pam_drop(conf_icaps);

cleanup_cap_s:
    if (cap_s) {
	cap_free(cap_s);
	cap_s = NULL;
    }

    return ok;
}

/* log errors */

static void _pam_log(int err, const char *format, ...)
{
    va_list args;

    va_start(args, format);
    openlog("pam_cap", LOG_CONS|LOG_PID, LOG_AUTH);
    vsyslog(err, format, args);
    va_end(args);
    closelog();
}

static void parse_args(int argc, const char **argv, struct pam_cap_s *pcs)
{
    int ctrl=0;

    /* step through arguments */
    for (ctrl=0; argc-- > 0; ++argv) {

	if (!strcmp(*argv, "debug")) {
	    pcs->debug = 1;
	} else if (!strncmp(*argv, "config=", 7)) {
	    pcs->conf_filename = 7 + *argv;
	} else {
	    _pam_log(LOG_ERR, "unknown option; %s", *argv);
	}

    }
}

int pam_sm_authenticate(pam_handle_t *pamh, int flags,
			int argc, const char **argv)
{
    int retval;
    struct pam_cap_s pcs;
    char *conf_icaps;

    memset(&pcs, 0, sizeof(pcs));

    parse_args(argc, argv, &pcs);

    retval = pam_get_user(pamh, &pcs.user, NULL);

    if (retval == PAM_CONV_AGAIN) {
	D(("user conversation is not available yet"));
	memset(&pcs, 0, sizeof(pcs));
	return PAM_INCOMPLETE;
    }

    if (retval != PAM_SUCCESS) {
	D(("pam_get_user failed: %s", pam_strerror(pamh, retval)));
	memset(&pcs, 0, sizeof(pcs));
	return PAM_AUTH_ERR;
    }

    conf_icaps =
	read_capabilities_for_user(pcs.user,
				   pcs.conf_filename
				   ? pcs.conf_filename:USER_CAP_FILE );

    memset(&pcs, 0, sizeof(pcs));

    if (conf_icaps) {
	D(("it appears that there are capabilities for this user [%s]",
	   conf_icaps));

	/* We could also store this as a pam_[gs]et_data item for use
	   by the setcred call to follow. As it is, there is a small
	   race associated with a redundant read. Oh well, if you
	   care, send me a patch.. */

	_pam_overwrite(conf_icaps);
	_pam_drop(conf_icaps);

	return PAM_SUCCESS;

    } else {

	D(("there are no capabilities restrctions on this user"));
	return PAM_IGNORE;

    }
}

int pam_sm_setcred(pam_handle_t *pamh, int flags,
		   int argc, const char **argv)
{
    int retval;
    struct pam_cap_s pcs;

    if (!(flags & PAM_ESTABLISH_CRED)) {
	D(("we don't handle much in the way of credentials"));
	return PAM_IGNORE;
    }

    memset(&pcs, 0, sizeof(pcs));

    parse_args(argc, argv, &pcs);

    retval = pam_get_item(pamh, PAM_USER, (const void **)&pcs.user);
    if ((retval != PAM_SUCCESS) || (pcs.user == NULL) || !(pcs.user[0])) {

	D(("user's name is not set"));
	return PAM_AUTH_ERR;
    }

    retval = set_capabilities(&pcs);

    memset(&pcs, 0, sizeof(pcs));

    return (retval ? PAM_SUCCESS:PAM_IGNORE );
}
