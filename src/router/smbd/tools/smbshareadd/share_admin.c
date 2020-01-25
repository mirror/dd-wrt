// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2019 Samsung Electronics Co., Ltd.
 *
 *   linux-cifsd-devel@lists.sourceforge.net
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <config_parser.h>
#include <usmbdtools.h>

#include <management/share.h>

#include <linux/usmbd_server.h>
#include <share_admin.h>

static int conf_fd = -1;
static char wbuf[16384];
static size_t wsz;

#define AUX_GROUP_PREFIX	"_a_u_x_grp_"

static char *new_group_name(char *name)
{
	char *gn;

	if (strchr(name, '['))
		return name;

	gn = malloc(strlen(name) + 3);
	if (gn)
		sprintf(gn, "[%s]", name);
	return gn;
}

static char *aux_group_name(char *name)
{
	char *gn;

	gn = malloc(strlen(name) + 3 + strlen(AUX_GROUP_PREFIX));
	if (gn)
		sprintf(gn, "[%s%s]", AUX_GROUP_PREFIX, name);
	return gn;
}

static int __open_smbconf(char *smbconf)
{
	conf_fd = open(smbconf, O_WRONLY);
	if (conf_fd == -1) {
		pr_err("%s %s\n", strerr(errno), smbconf);
		return -EINVAL;
	}

	if (ftruncate(conf_fd, 0)) {
		pr_err("%s %s\n", strerr(errno), smbconf);
		close(conf_fd);
		return -EINVAL;
	}

	return 0;
}

static void __write(void)
{
	int nr = 0;
	int ret;

	while (wsz && (ret = write(conf_fd, wbuf + nr, wsz)) != 0) {
		if (ret == -1) {
			if (errno == EINTR)
				continue;
			pr_err("%s\n", strerr(errno));
			exit(EXIT_FAILURE);
		}

		nr += ret;
		wsz -= ret;
	}
}

static void __write_share(void *value, unsigned long long key, void *buf)
{
	char *k = (char *)list_fromkey(key);
	char *v = (char *)value;

	wsz = snprintf(wbuf, sizeof(wbuf), "\t%s = %s\n", k, v);
	if (wsz > sizeof(wbuf)) {
		pr_err("smb.conf entry size is above the limit: %zu > %zu\n",
			wsz,
			sizeof(wbuf));
		exit(EXIT_FAILURE);
	}
	__write();
}

static void write_share(struct smbconf_group *g)
{
	int ret, nr = 0;

	wsz = snprintf(wbuf, sizeof(wbuf), "[%s]\n", g->name);
	__write();
	list_foreach(&g->kv, __write_share, NULL);
}

static void write_share_cb(void *value, unsigned long long key, void *share_data)
{
	struct smbconf_group *g = (struct smbconf_group *)value;

	/*
	 * Do not write AUX group
	 */
	if (!strstr(g->name, AUX_GROUP_PREFIX))
		write_share(g);
}

static void write_remove_share_cb(void *value,
				  unsigned long long key,
				  void *name)
{
	struct smbconf_group *g = (struct smbconf_group *)value;

	if (!strcasecmp(g->name, name)) {
		pr_info("share '%s' removed\n", g->name);
		return;
	}

	write_share(g);
}

static void update_share_cb(void *value,
			    unsigned long long key,
			    void *g)
{
	struct LIST *list = (struct LIST *) g;
	char *nk, *nv;

	nk = strdup(list_fromkey(key));
	nv = strdup(value);
	if (!nk || !nv)
		exit(EXIT_FAILURE);

	/* This will call .dtor for already existing key/value pairs */
	list_add_str(&list, nv, nk);
}

int command_add_share(char *smbconf, char *name, char *opts)
{
	char *new_name = NULL;

	if (list_get(&parser.groups, list_tokey(name))) {
		pr_err("Share already exists: %s\n", name);
		return -EEXIST;
	}

	new_name = new_group_name(name);
	if (cp_parse_external_smbconf_group(new_name, opts))
		goto error;

	if (__open_smbconf(smbconf))
		goto error;
	list_foreach(&parser.groups, write_share_cb, NULL);
	close(conf_fd);
	free(new_name);
	return 0;

error:
	free(new_name);
	return -EINVAL;
}

int command_update_share(char *smbconf, char *name, char *opts)
{
	struct smbconf_group *existing_group;
	struct smbconf_group *update_group;
	char *aux_name = NULL;

	existing_group = list_get(&parser.groups, list_tokey(name));
	if (!existing_group) {
		pr_err("Unknown share: %s\n", name);
		goto error;
	}

	aux_name = aux_group_name(name);
	if (cp_parse_external_smbconf_group(aux_name, opts))
		goto error;

	/* get rid of [] */
	sprintf(aux_name, "%s%s", AUX_GROUP_PREFIX, name);
	update_group = list_get(&parser.groups, list_tokey(aux_name));
	if (!update_group) {
		pr_err("Cannot find the external group\n");
		goto error;
	}

	list_foreach(&update_group->kv,
			     update_share_cb,
			     existing_group->kv);

	if (__open_smbconf(smbconf))
		goto error;

	list_foreach(&parser.groups, write_share_cb, NULL);
	close(conf_fd);
	free(aux_name);
	return 0;

error:
	free(aux_name);
	return -EINVAL;
}

int command_del_share(char *smbconf, char *name)
{
	if (__open_smbconf(smbconf))
		return -EINVAL;

	list_foreach(&parser.groups,
			     write_remove_share_cb,
			     (void *)name);
	close(conf_fd);
	return 0;
}
