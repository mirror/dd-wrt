/*
 *  nfsidmap_common.c
 *
 *  nfs idmapping library, primarily for nfs4 client/server kernel idmapping
 *  and for userland nfs4 idmapping by acl libraries.
 *
 *  Code common to libnfsidmap and some of its bundled plugins
 *
 *  If you make use of these functions you must initialise your own
 *  copy of the config file data using: conf_init_file(nfsidmap_conf_path)
 *  failure to do so will appear as if the config was empty
 */

#include "config.h"

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "nfsidmap.h"
#include "nfsidmap_private.h"
#include "nfsidmap_plugin.h"
#include "conffile.h"

#pragma GCC visibility push(hidden)

static char * toupper_str(char *s)
{
        size_t i;
        for (i=0; i < strlen(s); i++)
                s[i] = toupper(s[i]);
        return s;
}

static struct conf_list *local_realms = NULL;

void free_local_realms(void)
{
	if (local_realms) {
		conf_free_list(local_realms);
		local_realms = NULL;
	}
}

/* Get list of "local equivalent" realms.  Meaning the list of realms
 * where john@REALM.A is considered the same user as john@REALM.B
 * If not specified, default to upper-case of local domain name */
struct conf_list *get_local_realms(void)
{
	if (local_realms) return local_realms;

	local_realms = conf_get_list("General", "Local-Realms");
	if (local_realms == NULL) {
		struct conf_list_node *node;

		local_realms = malloc(sizeof *local_realms);
		if (local_realms == NULL)
			return NULL;
		local_realms->cnt = 0;
		TAILQ_INIT(&local_realms->fields);

		node = calloc(1, sizeof *node);
		if (node == NULL)
			return NULL;

		node->field = calloc(1, NFS4_MAX_DOMAIN_LEN);
		if (node->field == NULL) {
			free(node);
			return NULL;
		}

		nfs4_get_default_domain(NULL, node->field, NFS4_MAX_DOMAIN_LEN);
		toupper_str(node->field);

		TAILQ_INSERT_TAIL(&local_realms->fields, node, link);
		local_realms->cnt++;
	}
	return local_realms;
}

static int no_strip = -1;
static int reformat_group = 0;

int get_nostrip(void)
{
	if (no_strip != -1) return no_strip;

	char * nostrip = conf_get_str_with_def("General", "No-Strip", "none");
	if (strcasecmp(nostrip, "both") == 0)
		no_strip = IDTYPE_USER|IDTYPE_GROUP;
	else if (strcasecmp(nostrip, "group") == 0)
		no_strip = IDTYPE_GROUP;
	else if (strcasecmp(nostrip, "user") == 0)
		no_strip = IDTYPE_USER;
	else
		no_strip = 0;

	if (no_strip & IDTYPE_GROUP) {
		char * reformatgroup = conf_get_str_with_def("General", "Reformat-Group", "false");
		if ((strcasecmp(reformatgroup, "true") == 0) ||
		    (strcasecmp(reformatgroup, "on") == 0) ||
		    (strcasecmp(reformatgroup, "yes") == 0))
			reformat_group = 1;
		else
			reformat_group = 0;
	}

	return no_strip;
}

int get_reformat_group(void)
{
	if (no_strip != -1) return reformat_group;

	return reformat_group;
}
