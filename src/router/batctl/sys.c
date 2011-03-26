/*
 * Copyright (C) 2009-2011 B.A.T.M.A.N. contributors:
 *
 * Marek Lindner <lindner_marek@yahoo.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of version 2 of the GNU General Public
 * License as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 *
 */


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>

#include "main.h"
#include "sys.h"
#include "functions.h"

#define PATH_BUFF_LEN 200

const char *sysfs_param_enable[] = {
	"enable",
	"disable",
	"1",
	"0",
	NULL,
};

const char *sysfs_param_server[] = {
	"off",
	"client",
	"server",
	NULL,
};

static void interface_usage(void)
{
	printf("Usage: batctl interface [options] [add|del iface(s)] \n");
	printf("options:\n");
	printf(" \t -h print this help\n");
}

static int print_interfaces(char *mesh_iface)
{
	DIR *iface_base_dir;
	struct dirent *iface_dir;
	char *path_buff;
	int res;

	path_buff = malloc(PATH_BUFF_LEN);
	if (!path_buff) {
		printf("Error - could not allocate path buffer: out of memory ?\n");
		goto err;
	}

	iface_base_dir = opendir(SYS_IFACE_PATH);
	if (!iface_base_dir) {
		printf("Error - the directory '%s' could not be read: %s\n",
		       SYS_IFACE_PATH, strerror(errno));
		printf("Is the batman-adv module loaded and sysfs mounted ?\n");
		goto err_buff;
	}

	while ((iface_dir = readdir(iface_base_dir)) != NULL) {
		snprintf(path_buff, PATH_BUFF_LEN, SYS_MESH_IFACE_FMT, iface_dir->d_name);
		res = read_file("", path_buff, SINGLE_READ | USE_READ_BUFF | SILENCE_ERRORS, 0, 0);
		if (res != EXIT_SUCCESS)
			continue;

		if (line_ptr[strlen(line_ptr) - 1] == '\n')
			line_ptr[strlen(line_ptr) - 1] = '\0';

		if (strcmp(line_ptr, "none") == 0)
			goto free_line;

		if (strcmp(line_ptr, mesh_iface) != 0)
			goto free_line;

		free(line_ptr);
		line_ptr = NULL;

		snprintf(path_buff, PATH_BUFF_LEN, SYS_IFACE_STATUS_FMT, iface_dir->d_name);
		res = read_file("", path_buff, SINGLE_READ | USE_READ_BUFF | SILENCE_ERRORS, 0, 0);
		if (res != EXIT_SUCCESS) {
			printf("<error reading status>\n");
			continue;
		}

		printf("%s: %s", iface_dir->d_name, line_ptr);

free_line:
		free(line_ptr);
		line_ptr = NULL;
	}

	free(path_buff);
	closedir(iface_base_dir);
	return EXIT_SUCCESS;

err_buff:
	free(path_buff);
err:
	return EXIT_FAILURE;
}

int interface(char *mesh_iface, int argc, char **argv)
{
	char *path_buff;
	int i, res, optchar;

	while ((optchar = getopt(argc, argv, "h")) != -1) {
		switch (optchar) {
		case 'h':
			interface_usage();
			return EXIT_SUCCESS;
		default:
			interface_usage();
			return EXIT_FAILURE;
		}
	}

	if (argc == 1)
		return print_interfaces(mesh_iface);

	if ((strcmp(argv[1], "add") != 0) && (strcmp(argv[1], "a") != 0) &&
	    (strcmp(argv[1], "del") != 0) && (strcmp(argv[1], "d") != 0)) {
		printf("Error - unknown argument specified: %s\n", argv[1]);
		interface_usage();
		goto err;
	}

	path_buff = malloc(PATH_BUFF_LEN);
	if (!path_buff) {
		printf("Error - could not allocate path buffer: out of memory ?\n");
		goto err;
	}

	for (i = 2; i < argc; i++) {
		snprintf(path_buff, PATH_BUFF_LEN, SYS_MESH_IFACE_FMT, argv[i]);

		if (argv[1][0] == 'a')
			res = write_file("", path_buff, mesh_iface, NULL);
		else
			res = write_file("", path_buff, "none", NULL);

		if (res != EXIT_SUCCESS)
			goto err_buff;
	}

	free(path_buff);
	return EXIT_SUCCESS;

err_buff:
	free(path_buff);
err:
	return EXIT_FAILURE;
}

static void log_level_usage(void)
{
	printf("Usage: batctl [options] loglevel [level]\n");
	printf("options:\n");
	printf(" \t -h print this help\n");
}

int handle_loglevel(char *mesh_iface, int argc, char **argv)
{
	int optchar, res;
	char *path_buff;

	while ((optchar = getopt(argc, argv, "h")) != -1) {
		switch (optchar) {
		case 'h':
			log_level_usage();
			return EXIT_SUCCESS;
		default:
			log_level_usage();
			return EXIT_FAILURE;
		}
	}

	path_buff = malloc(PATH_BUFF_LEN);
	snprintf(path_buff, PATH_BUFF_LEN, SYS_BATIF_PATH_FMT, mesh_iface);

	if (argc != 1) {
		res = write_file(path_buff, SYS_LOG_LEVEL, argv[1], NULL);
		goto out;
	}

	res = read_file(path_buff, SYS_LOG_LEVEL, SINGLE_READ | USE_READ_BUFF, 0, 0);

	if (res != EXIT_SUCCESS)
		goto out;

	printf("[%c] %s (%d)\n", (line_ptr[0] == '0') ? 'x' : ' ',
	       "all debug output disabled", 0);
	printf("[%c] %s (%d)\n", (line_ptr[0] == '1') ? 'x' : ' ',
	       "messages related to routing / flooding / broadcasting", 1);
	printf("[%c] %s (%d)\n", (line_ptr[0] == '2') ? 'x' : ' ',
	       "messages related to route or hna added / changed / deleted", 2);
	printf("[%c] %s (%d)\n", (line_ptr[0] == '3') ? 'x' : ' ',
	       "all debug messages", 3);

out:
	if (errno == ENOENT)
		printf("To increase the log level you need to compile the module with debugging enabled (see the README)\n");

	free(path_buff);
	return res;
}

void aggregation_usage(void)
{
	printf("Usage: batctl [options] aggregation [0|1]\n");
	printf("options:\n");
	printf(" \t -h print this help\n");
}

void bonding_usage(void)
{
	printf("Usage: batctl [options] bonding [0|1]\n");
	printf("options:\n");
	printf(" \t -h print this help\n");
}

void gw_mode_usage(void)
{
	printf("Usage: batctl [options] gw_mode [mode] [sel_class|bandwidth]\n");
	printf("options:\n");
	printf(" \t -h print this help\n");
}

void vis_mode_usage(void)
{
	printf("Usage: batctl [options] vis_mode [mode]\n");
	printf("options:\n");
	printf(" \t -h print this help\n");
}

void orig_interval_usage(void)
{
	printf("Usage: batctl [options] interval \n");
	printf("options:\n");
	printf(" \t -h print this help\n");
}

void fragmentation_usage(void)
{
	printf("Usage: batctl [options] fragmentation [0|1]\n");
	printf("options:\n");
	printf(" \t -h print this help\n");
}

int handle_sys_setting(char *mesh_iface, int argc, char **argv,
		       char *file_path, void setting_usage(void),
		       const char *sysfs_param[])
{
	int optchar, res = EXIT_FAILURE;
	char *path_buff;
	const char **ptr;

	while ((optchar = getopt(argc, argv, "h")) != -1) {
		switch (optchar) {
		case 'h':
			setting_usage();
			return EXIT_SUCCESS;
		default:
			setting_usage();
			return EXIT_FAILURE;
		}
	}

	path_buff = malloc(PATH_BUFF_LEN);
	snprintf(path_buff, PATH_BUFF_LEN, SYS_BATIF_PATH_FMT, mesh_iface);

	if (argc == 1) {
		res = read_file(path_buff, file_path, SINGLE_READ, 0, 0);
		goto out;
	}

	if (!sysfs_param)
		goto write_file;

	ptr = sysfs_param;
	while (*ptr) {
		if (strcmp(*ptr, argv[1]) == 0)
			goto write_file;

		ptr++;
	}

	printf("Error - the supplied argument is invalid: %s\n", argv[1]);
	printf("The following values are allowed:\n");

	ptr = sysfs_param;
	while (*ptr) {
		printf(" * %s\n", *ptr);
		ptr++;
	}

	goto out;

write_file:
	res = write_file(path_buff, file_path, argv[1], argc > 2 ? argv[2] : NULL);

out:
	free(path_buff);
	return res;
}

int handle_gw_setting(char *mesh_iface, int argc, char **argv)
{
	int optchar, res = EXIT_FAILURE;
	char *path_buff, gw_mode;
	const char **ptr;

	while ((optchar = getopt(argc, argv, "h")) != -1) {
		switch (optchar) {
		case 'h':
			gw_mode_usage();
			return EXIT_SUCCESS;
		default:
			gw_mode_usage();
			return EXIT_FAILURE;
		}
	}

	path_buff = malloc(PATH_BUFF_LEN);
	snprintf(path_buff, PATH_BUFF_LEN, SYS_BATIF_PATH_FMT, mesh_iface);

	if (argc == 1) {
		res = read_file(path_buff, SYS_GW_MODE, SINGLE_READ | USE_READ_BUFF, 0, 0);

		if (res != EXIT_SUCCESS)
			goto out;

		if (line_ptr[strlen(line_ptr) - 1] == '\n')
			line_ptr[strlen(line_ptr) - 1] = '\0';

		if (strcmp(line_ptr, "client") == 0)
			gw_mode = GW_MODE_CLIENT;
		else if (strcmp(line_ptr, "server") == 0)
			gw_mode = GW_MODE_SERVER;
		else
			gw_mode = GW_MODE_OFF;

		free(line_ptr);
		line_ptr = NULL;

		switch (gw_mode) {
		case GW_MODE_CLIENT:
			res = read_file(path_buff, SYS_GW_SEL, SINGLE_READ | USE_READ_BUFF, 0, 0);
			break;
		case GW_MODE_SERVER:
			res = read_file(path_buff, SYS_GW_BW, SINGLE_READ | USE_READ_BUFF, 0, 0);
			break;
		default:
			printf("off\n");
			goto out;
		}

		if (res != EXIT_SUCCESS)
			goto out;

		if (line_ptr[strlen(line_ptr) - 1] == '\n')
			line_ptr[strlen(line_ptr) - 1] = '\0';

		switch (gw_mode) {
		case GW_MODE_CLIENT:
			printf("client (selection class: %s)\n", line_ptr);
			break;
		case GW_MODE_SERVER:
			printf("server (announced bw: %s)\n", line_ptr);
			break;
		default:
			goto out;
		}

		free(line_ptr);
		line_ptr = NULL;
		goto out;
	}

	if (strcmp(argv[1], "client") == 0)
		gw_mode = GW_MODE_CLIENT;
	else if (strcmp(argv[1], "server") == 0)
		gw_mode = GW_MODE_SERVER;
	else if (strcmp(argv[1], "off") == 0)
		gw_mode = GW_MODE_OFF;
	else
		goto opt_err;

	res = write_file(path_buff, SYS_GW_MODE, argv[1], NULL);
	if (res != EXIT_SUCCESS)
		goto out;

	if (argc == 2)
		goto out;

	switch (gw_mode) {
	case GW_MODE_CLIENT:
		res = write_file(path_buff, SYS_GW_SEL, argv[2], NULL);
		break;
	case GW_MODE_SERVER:
		res = write_file(path_buff, SYS_GW_BW, argv[2], NULL);
		break;
	default:
		goto out;
	}

opt_err:
	printf("Error - the supplied argument is invalid: %s\n", argv[1]);
	printf("The following values are allowed:\n");

	ptr = sysfs_param_server;
	while (*ptr) {
		printf(" * %s\n", *ptr);
		ptr++;
	}

out:
	free(path_buff);
	return res;
}
