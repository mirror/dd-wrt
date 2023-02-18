/*
 * configfile.c -- mount configuration file manipulation
 * Copyright (C) 2008 Red Hat, Inc <nfs@redhat.com>
 *
 * - Routines use to create mount options from the mount
 *   configuration file.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "xlog.h"
#include "mount.h"
#include "parse_opt.h"
#include "network.h"
#include "conffile.h"
#include "mount_config.h"

#define KBYTES(x)     ((x) * (1024))
#define MEGABYTES(x)  ((x) * (1048576))
#define GIGABYTES(x)  ((x) * (1073741824))

#ifndef NFSMOUNT_GLOBAL_OPTS
#define NFSMOUNT_GLOBAL_OPTS "NFSMount_Global_Options"
#endif

#ifndef NFSMOUNT_MOUNTPOINT
#define NFSMOUNT_MOUNTPOINT "MountPoint"
#endif

#ifndef NFSMOUNT_SERVER
#define NFSMOUNT_SERVER "Server"
#endif

enum {
	MNT_NOARG=0,
	MNT_INTARG,
	MNT_STRARG,
	MNT_SPEC,
	MNT_UNSET
};
struct mnt_alias {
	char *alias;
	char *opt;
	int  argtype;
} mnt_alias_tab[] = {
	{"background", "bg", MNT_NOARG},
	{"foreground", "fg", MNT_NOARG},
	{"sloppy", "sloppy", MNT_NOARG},
};
int mnt_alias_sz = (sizeof(mnt_alias_tab)/sizeof(mnt_alias_tab[0]));

static const char *version_keys[] = {
	"v3", "v4", "vers", "nfsvers", "minorversion", NULL
};

static int strict;

static int is_version(const char *field)
{
	int i;
	for (i = 0; version_keys[i] ; i++)
		if (strcmp(version_keys[i], field) == 0)
			return 1;
	if (strncmp(field, "v4.", 3) == 0)
		return 1;
	return 0;
}

/*
 * See if the option is an alias, if so return the
 * real mount option along with the argument type.
 */
inline static
char *mountopts_alias(char *opt, int *argtype)
{
	int i;

	*argtype = MNT_UNSET;
	for (i=0; i < mnt_alias_sz; i++) {
		if (strcasecmp(opt, mnt_alias_tab[i].alias) != 0)
			continue;
		*argtype = mnt_alias_tab[i].argtype;
		return mnt_alias_tab[i].opt;
	}
	/* Make option names case-insensitive */
	upper2lower(opt);

	return opt;
}
/*
 * Convert numeric strings that end with 'k', 'm' or 'g'
 * into numeric strings with the real value.
 * Meaning '8k' becomes '8094'.
 */
static char *mountopts_convert(char *value)
{
	unsigned long long factor, num;
	static char buf[64];
	char *ch;

	ch = &value[strlen(value)-1];
	switch (tolower(*ch)) {
	case 'k':
		factor = KBYTES(1);
		break;
	case 'm':
		factor = MEGABYTES(1);
		break;
	case 'g':
		factor = GIGABYTES(1);
		break;
	default:
		return value;
	}
	*ch = '\0';
	if (strncmp(value, "0x", 2) == 0) {
		num = strtol(value, (char **)NULL, 16);
	} else if (strncmp(value, "0", 1) == 0) {
		num = strtol(value, (char **)NULL, 8);
	} else {
		num = strtol(value, (char **)NULL, 10);
	}
	num *= factor;
	snprintf(buf, 64, "%lld", num);

	return buf;
}

struct nfs_version config_default_vers;
unsigned long config_default_proto;
extern sa_family_t config_default_family;

/*
 * Check to see if a default value is being set.
 * If so, set the appropriate global value which will
 * be used as the initial value in the server negation.
 */
static int
default_value(char *mopt)
{
	struct mount_options *options = NULL;
	int dftlen = strlen("default");
	char *field;

	if (strncasecmp(mopt, "default", dftlen) != 0)
		return 0;

	field = mopt + dftlen;
	if (strncasecmp(field, "proto", strlen("proto")) == 0) {
		if ((options = po_split(field)) != NULL) {
			if (!nfs_nfs_protocol(options, &config_default_proto)) {
				xlog_warn("Unable to set default protocol : %s",
					strerror(errno));
			}
			if (!nfs_nfs_proto_family(options, &config_default_family)) {
				xlog_warn("Unable to set default family : %s",
					strerror(errno));
			}
		} else {
			xlog_warn("Unable to alloc memory for default protocol");
		}
	} else if (strncasecmp(field, "vers", strlen("vers")) == 0) {
		if ((options = po_split(field)) != NULL) {
			if (!nfs_nfs_version("nfs", options, &config_default_vers)) {
				xlog_warn("Unable to set default version: %s",
					strerror(errno));
			}
		} else {
			xlog_warn("Unable to alloc memory for default version");
		}
	} else
		xlog_warn("Invalid default setting: '%s'", mopt);

	if (options)
		po_destroy(options);

	return 1;
}
/*
 * Parse the given section of the configuration
 * file to if there are any mount options set.
 * If so, added them to link list.
 */
static void
conf_parse_mntopts(char *section, char *arg, struct mount_options *options)
{
	struct conf_list *list;
	struct conf_list_node *node;
	char buf[BUFSIZ], *value, *field;
	char *nvalue, *ptr;
	int argtype;
	int have_version = 0;

	if (po_rightmost(options, version_keys) >= 0 ||
	    po_contains_prefix(options, "v4.", NULL, 0) == PO_FOUND)
		have_version = 1;

	list = conf_get_tag_list(section, arg);
	TAILQ_FOREACH(node, &list->fields, link) {
		/*
		 * Do not overwrite options if already exists
		 */
		field = mountopts_alias(node->field, &argtype);
		if (po_contains(options, field) == PO_FOUND)
			continue;
		/* Some options can be inverted by a "no" prefix.
		 * Check for these.
		 * "no" prefixes are unlikely in the config file as
		 * "option=false" is preferred, but still possible.
		 */
		if (strncmp(field, "no", 2) == 0 &&
		    po_contains(options, field+2) == PO_FOUND)
			continue;
		if (strlen(field) < BUFSIZ-3) {
			strcat(strcpy(buf, "no"), field);
			if (po_contains(options, buf) == PO_FOUND)
				continue;
		}

		/* If fg or bg already present, ignore bg or fg */
		if (strcmp(field, "fg") == 0 &&
		    po_contains(options, "bg") == PO_FOUND)
			continue;
		if (strcmp(field, "bg") == 0 &&
		    po_contains(options, "fg") == PO_FOUND)
			continue;

		if (is_version(field)) {
			if (have_version)
				continue;
			have_version = 1;
		}

		buf[0] = '\0';
		value = conf_get_section(section, arg, node->field);
		if (value == NULL)
			continue;
		if (strcasecmp(value, "false") == 0) {
			if (argtype != MNT_NOARG)
				snprintf(buf, BUFSIZ, "no%s", field);
			else if (strcasecmp(field, "bg") == 0)
				snprintf(buf, BUFSIZ, "fg");
			else if (strcasecmp(field, "fg") == 0)
				snprintf(buf, BUFSIZ, "bg");
			else if (strcasecmp(field, "sloppy") == 0)
				strict = 1;
		} else if (strcasecmp(value, "true") == 0) {
			if ((strcasecmp(field, "sloppy") == 0) && strict)
				continue;
			snprintf(buf, BUFSIZ, "%s", field);
		} else {
			nvalue = strdup(value);
			ptr = mountopts_convert(nvalue);
			snprintf(buf, BUFSIZ, "%s=%s", field, ptr);
			free(nvalue);
		}
		if (buf[0] == '\0')
			continue;

		po_append(options, buf);
		default_value(buf);
	}
	conf_free_list(list);
}

/*
 * Concatenate options from the configuration file with the
 * given options by building a link list of options from the
 * different sections in the conf file. Options that exists
 * in the either the given options or link list are not
 * overwritten so it matter which when each section is
 * parsed.
 */
char *conf_get_mntopts(char *spec, char *mount_point,
			      char *mount_opts)
{
	struct mount_options *options;
	char *ptr, *server;

	strict = 0;
	options = po_split(mount_opts);
	if (!options) {
		xlog_warn("conf_get_mountops: Unable calloc memory for options");
		return mount_opts;
	}
	/*
	 * First see if there are any mount options relative
	 * to the mount point.
	 */
	conf_parse_mntopts(NFSMOUNT_MOUNTPOINT, mount_point, options);

	/*
	 * Next, see if there are any mount options relative
	 * to the server
	 */
	server = strdup(spec);
	if (server == NULL) {
		xlog_warn("conf_get_mountops: Unable calloc memory for server");
		po_destroy(options);
		return mount_opts;
	}
	if ((ptr = strchr(server, ':')) != NULL)
		*ptr='\0';
	conf_parse_mntopts(NFSMOUNT_SERVER, server, options);
	free(server);

	/*
	 * Finally process all the global mount options.
	 */
	conf_parse_mntopts(NFSMOUNT_GLOBAL_OPTS, NULL, options);

	/*
	 * Strip out defaults, which have already been handled,
	 * then join the rest and return.
	 */
	while (po_contains_prefix(options, "default", &ptr, 0) == PO_FOUND) {
		ptr = strdup(ptr);
		po_remove_all(options, ptr);
		free(ptr);
	}

	po_join(options, &mount_opts);
	po_destroy(options);

	return mount_opts;
}
