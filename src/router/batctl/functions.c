/*
 * Copyright (C) 2007-2011 B.A.T.M.A.N. contributors:
 *
 * Andreas Langer <an.langer@gmx.de>, Marek Lindner <lindner_marek@yahoo.de>
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


#define _GNU_SOURCE
#include <netinet/ether.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>

#include "main.h"
#include "functions.h"
#include "bat-hosts.h"

static struct timeval start_time;
static char *host_name;
char *line_ptr = NULL;

void start_timer(void)
{
	gettimeofday(&start_time, NULL);
}

double end_timer(void)
{
	struct timeval end_time, diff;

	gettimeofday(&end_time, NULL);
	diff.tv_sec = end_time.tv_sec - start_time.tv_sec;
	diff.tv_usec = end_time.tv_usec - start_time.tv_usec;

	if (diff.tv_usec < 0) {
		diff.tv_sec--;
		diff.tv_usec += 1000000;
	}

	return (((double)diff.tv_sec * 1000) + ((double)diff.tv_usec / 1000));
}

char *ether_ntoa_long(const struct ether_addr *addr)
{
	static char asc[18];

	sprintf(asc, "%02x:%02x:%02x:%02x:%02x:%02x",
		addr->ether_addr_octet[0], addr->ether_addr_octet[1],
		addr->ether_addr_octet[2], addr->ether_addr_octet[3],
		addr->ether_addr_octet[4], addr->ether_addr_octet[5]);

	return asc;
}

char *get_name_by_macaddr(struct ether_addr *mac_addr, int read_opt)
{
	struct bat_host *bat_host = NULL;

	if (read_opt & USE_BAT_HOSTS)
		bat_host = bat_hosts_find_by_mac((char *)mac_addr);

	if (!bat_host)
		host_name = ether_ntoa_long((struct ether_addr *)mac_addr);
	else
		host_name = bat_host->name;

	return host_name;
}

char *get_name_by_macstr(char *mac_str, int read_opt)
{
	struct ether_addr *mac_addr;

	mac_addr = ether_aton(mac_str);
	if (!mac_addr)
		return mac_str;

	return get_name_by_macaddr(mac_addr, read_opt);
}

static int check_sys_dir(char *dir)
{
	struct stat st;

	if (stat("/sys/", &st) != 0) {
		printf("Error - the folder '/sys/' was not found on the system\n");
		printf("Please make sure that the sys filesystem is properly mounted\n");
		return EXIT_FAILURE;
	}

	if (stat(dir, &st) == 0)
		return EXIT_SUCCESS;

	printf("Error - the folder '%s' was not found within the sys filesystem\n", dir);
	printf("Please make sure that the batman-adv kernel module is loaded\n");
	return EXIT_FAILURE;
}

int read_file(char *dir, char *fname, int read_opt,
	      float orig_timeout, float watch_interval)
{
	struct ether_addr *mac_addr;
	struct bat_host *bat_host;
	int res = EXIT_FAILURE;
	float last_seen;
	char full_path[500], *buff_ptr, *space_ptr, extra_char;
	size_t len = 0;
	FILE *fp = NULL;

	if (read_opt & USE_BAT_HOSTS)
		bat_hosts_init(read_opt);

	if (strstr(dir, "/sys/")) {
		if (check_sys_dir(dir) != EXIT_SUCCESS)
			goto out;
	}

	strncpy(full_path, dir, strlen(dir));
	full_path[strlen(dir)] = '\0';
	strncat(full_path, fname, sizeof(full_path) - strlen(full_path));

open:
	fp = fopen(full_path, "r");

	if (!fp) {
		if (!(read_opt & SILENCE_ERRORS))
			printf("Error - can't open file '%s': %s\n", full_path, strerror(errno));
		goto out;
	}

	if (read_opt & CLR_CONT_READ)
		system("clear");

read:
	while (getline(&line_ptr, &len, fp) != -1) {
		/* the buffer will be handled elsewhere */
		if (read_opt & USE_READ_BUFF)
			break;

		/* skip timed out originators */
		if (read_opt & NO_OLD_ORIGS)
			if (sscanf(line_ptr, "%*s %f", &last_seen)
			    && (last_seen > orig_timeout))
				continue;

		if (!(read_opt & USE_BAT_HOSTS)) {
			printf("%s", line_ptr);
			continue;
		}

		/* replace mac addresses with bat host names */
		buff_ptr = line_ptr;

		while ((space_ptr = strchr(buff_ptr, ' ')) != NULL) {

			*space_ptr = '\0';
			extra_char = '\0';

			if (strlen(buff_ptr) == ETH_STR_LEN + 1) {
				extra_char = buff_ptr[ETH_STR_LEN];
				switch (extra_char) {
				case ',':
				case ')':
					buff_ptr[ETH_STR_LEN] = '\0';
					break;
				default:
					extra_char = '\0';
					break;
				}
			}

			if (strlen(buff_ptr) != ETH_STR_LEN)
				goto print_plain_buff;

			mac_addr = ether_aton(buff_ptr);

			if (!mac_addr)
				goto print_plain_buff;

			bat_host = bat_hosts_find_by_mac((char *)mac_addr);

			if (!bat_host)
				goto print_plain_buff;

			if (read_opt & LOG_MODE)
				printf("%s", bat_host->name);
			else
				/* keep table format */
				printf("%17s", bat_host->name);

			goto written;

print_plain_buff:
			printf("%s", buff_ptr);

written:
			if (extra_char != '\0')
				printf("%c", extra_char);

			printf(" ");
			buff_ptr = space_ptr + 1;
		}

		printf("%s", buff_ptr);
	}

	if (read_opt & CONT_READ) {
		usleep(1000000 * watch_interval);
		goto read;
	}

	if (read_opt & CLR_CONT_READ) {
		if (fp)
			fclose(fp);
		usleep(1000000 * watch_interval);
		goto open;
	}

	res = EXIT_SUCCESS;

out:
	if (fp)
		fclose(fp);

	if (read_opt & USE_BAT_HOSTS)
		bat_hosts_free();

	return res;
}

int write_file(char *dir, char *fname, char *arg1, char *arg2)
{
	int fd = 0, res = EXIT_FAILURE;
	char full_path[500];
	ssize_t write_len;

	if (strstr(dir, "/sys/")) {
		if (check_sys_dir(dir) != EXIT_SUCCESS)
			goto out;
	}

	strncpy(full_path, dir, strlen(dir));
	full_path[strlen(dir)] = '\0';
	strncat(full_path, fname, sizeof(full_path) - strlen(full_path));

	fd = open(full_path, O_WRONLY);

	if (fd < 0) {
		printf("Error - can't open file '%s': %s\n", full_path, strerror(errno));
		goto out;
	}

	if (arg2)
		write_len = dprintf(fd, "%s %s", arg1, arg2);
	else
		write_len = write(fd, arg1, strlen(arg1) + 1);

	if (write_len < 0) {
		printf("Error - can't write to file '%s': %s\n", full_path, strerror(errno));
		goto out;
	}

	res = EXIT_SUCCESS;

out:
	if (fd)
		close(fd);
	return res;
}
