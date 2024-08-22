/*
 * NVRAM variable manipulation (Linux user mode half)
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: nvram_linux.c,v 1.9 2003/12/03 10:14:06 honor Exp $
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#define FROM_NVRAM
#include <bcmtypedefs.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <nvram_convert.h>

#define PATH_DEV_NVRAM "/dev/nvram"
#define NVRAM_SPACE_MAGIC			0x50534341	/* 'SPAC' */

static long NVRAMSPACE = NVRAM_SPACE;

/* Globals */
static int nvram_fd = -1;
static char *nvram_buf = NULL;
static int _nvram_init(void)
{
	if (nvram_fd >= 0)
		return 0;

	if ((nvram_fd = open(PATH_DEV_NVRAM, O_RDWR)) < 0) {
		fprintf(stderr, "cannot open /dev/nvram\n");
		goto err;
	}
	fcntl(nvram_fd, F_SETFD, FD_CLOEXEC);
	NVRAMSPACE = ioctl(nvram_fd, NVRAM_SPACE_MAGIC, NULL);
	if (NVRAMSPACE < 0) {
		fprintf(stderr, "nvram driver returns bogus space\n");
		goto err;
	}
	/* Map kernel string buffer into user space */
	nvram_buf = mmap(NULL, NVRAMSPACE, PROT_READ, MAP_SHARED, nvram_fd, 0);
	if (nvram_buf == MAP_FAILED) {
		close(nvram_fd);
		fprintf(stderr, "%s(): failed\n", __func__);
		nvram_fd = -1;
		goto err;
	}
	return 0;

err:
	return errno;
}

char *nvram_get(const char *name)
{
	if (!name) {
		fprintf(stderr, "error in %s: name is NULL\n", __func__);
		return NULL;
	}
	size_t count = strlen(name) + 1;
	char *value = NULL;
	unsigned long *off;
#if defined(HAVE_X86) || defined(HAVE_NEWPORT) || defined(HAVE_RB600) && !defined(HAVE_WDR4900)
	FILE *in = fopen("/usr/local/nvram/nvram.bin", "rb");
	if (in == NULL) {
		return NULL;
	}
	fclose(in);
#endif

	if (_nvram_init())
		return NULL;

	if (!(off = malloc(count))) {
		goto out;
	}

	/* Get offset into mmap() space */
	strcpy((char *)off, name);

	count = read(nvram_fd, off, count);
	if (count == sizeof(unsigned long))
		value = &nvram_buf[*off];
	else
		value = NULL;

	if (count < 0)
		perror(PATH_DEV_NVRAM);

	free(off);
      out:;
	return value;
}

int nvram_getall(char *buf, int count)
{
	int ret;
	if (!buf) {
		fprintf(stderr, "error in %s: buf is NULL\n", __func__);
		return -ENOMEM;
	}
#if defined(HAVE_X86) || defined(HAVE_NEWPORT) || defined(HAVE_RB600) && !defined(HAVE_WDR4900)
	FILE *in = fopen("/usr/local/nvram/nvram.bin", "rb");
	if (in == NULL) {
		return -1;
	}
	fclose(in);
#endif

	if (nvram_fd < 0) {
		if ((ret = _nvram_init())) {
			return ret;
		}
	}
	if (count == 0) {
		return 0;
	}
	/* Get all variables */
	*buf = '\0';
	ret = read(nvram_fd, buf, count);
	if (ret < 0)
		perror(PATH_DEV_NVRAM);
	return (ret == count) ? 0 : ret;
}

void nvram_open(void)		// dummy
{
}

void nvram_close(void)		//dummy
{
}

static int _nvram_set(const char *name, const char *value)
{
	if (!name) {
		fprintf(stderr, "error in %s: name is NULL\n", __func__);
		return -ENOMEM;
	}
	size_t count = strlen(name) + 1;
	char *buf;
	int ret = -1;
	int cnt = 0;

	if (_nvram_init())
		return -1;
	/* Wolf add - keep nvram varname to sane len - may prevent corruption */
	if (strlen(name) > 64) {
		return -ENOMEM;
	}

	/* Unset if value is NULL */
	if (value)
		count += strlen(value) + 2;

	if (!(buf = malloc(count))) {
		return -ENOMEM;
	}

	if (value)
		snprintf(buf, count, "%s=%s", name, value);
	else
		strlcpy(buf, name, count);

	count = 0;
	while (buf[cnt] != 0) {
		if (buf[cnt] != '\r')
			buf[count++] = buf[cnt];
		cnt++;
	}
	buf[count++] = 0;
	ret = write(nvram_fd, buf, count);
	if (ret < 0)
		perror(PATH_DEV_NVRAM);

	free(buf);
	return (ret == count) ? 0 : ret;
}

int nvram_set(const char *name, const char *value)
{
#ifndef HAVE_MADWIFI
	extern struct nvram_convert nvram_converts[];
#endif
	struct nvram_convert *v;
	int ret;
	if (!name) {
		fprintf(stderr, "error in %s: name is NULL\n", __func__);
		return -ENOMEM;
	}
#ifdef HAVE_NOWIFI
	if (!strcmp(name, "ip_conntrack_max") && value != NULL) {
		int val = atoi(value);
		if (val > 4096) {
			return _nvram_set(name, "4096");
		}

	}
#endif
	if (!strcmp(name, "et0macaddr_safe") && nvram_get("et0macaddr_safe"))
		return 0;	//ignore if already set
	ret = _nvram_set(name, value);

#ifndef HAVE_MADWIFI
	for (v = nvram_converts; v->name; v++) {
		if (!strcmp(v->name, name)) {
			if (strcmp(v->wl0_name, ""))
				_nvram_set(v->wl0_name, value);
		}
	}
#endif
	return ret;
}

int nvram_immed_set(const char *name, const char *value)
{
	return nvram_set(name, value);
}

int nvram_unset(const char *name)
{
	int v = _nvram_set(name, NULL);
	return v;
}

int _nvram_commit(void)
{
	if (nvram_match("flash_active", "1")) {
		fprintf(stderr, "not allowed, flash process in progress");
		return 1;
	}
	int ret = -1;
	if (_nvram_init())
		return ret;

	ret = ioctl(nvram_fd, NVRAM_MAGIC, NULL);

	if (ret < 0) {
		fprintf(stderr, "%s(): failed\n", __func__);
		perror(PATH_DEV_NVRAM);
	}
	return ret;
}

int nvram2file(char *varname, char *filename)
{
	FILE *fp;
	int i = 0;
	const char *buf;

	if (!(fp = fopen(filename, "wb")))
		return 0;

	buf = nvram_safe_get(varname);
	//fprintf(stderr,"=================> nvram2file %s = [%s] \n",varname,buf);
	while (buf[i]) {
		if (buf[i] == '~') {
			putc(0, fp);
		} else {
			putc(buf[i], fp);
		}
		i++;
	}
	fclose(fp);
	return i;
}

int nvram_size(void)
{
	if (_nvram_init())
		return -1;
	return NVRAMSPACE;
}

int nvram_used(int *space)
{
	char *name, *buf;
	if (_nvram_init())
		return -1;

	*space = NVRAMSPACE;

	buf = malloc(NVRAMSPACE);
	nvram_getall(buf, NVRAMSPACE);

	name = buf;

	while (*name) {
		name += strlen(name) + 1;
	}

	int used = (sizeof(struct nvram_header) + (long)name - (long)buf);
	free(buf);
	return used;
}

static unsigned char **values;
static unsigned int defaultnum;
static unsigned int stores;
struct nvram_param *load_defaults(void)
{
	struct nvram_param *srouter_defaults = NULL;
	FILE *in = fopen("/etc/defaults.bin", "rb");
	if (in == NULL)
		return NULL;
	unsigned int i;
	defaultnum = (unsigned int)getc(in);
	defaultnum |= (unsigned int)getc(in) << 8;
	defaultnum |= (unsigned int)getc(in) << 16;
	defaultnum |= (unsigned int)getc(in) << 24;
	stores = getc(in);	// count of unique values

	unsigned char *index;
	index = malloc(sizeof(char) * defaultnum);
	fread(index, defaultnum, 1, in);	// read string index table

	values = malloc(sizeof(char *) * stores);
	for (i = 0; i < stores; i++) {
		char temp[4096];
		int c;
		int a = 0;
		while ((c = getc(in)) != 0) {
			temp[a++] = c;
		}
		temp[a] = 0;

		values[i] = strdup(temp);
	}
	srouter_defaults = (struct nvram_param *)malloc(sizeof(struct nvram_param) * (defaultnum + 1));
	memset(srouter_defaults, 0, sizeof(struct nvram_param) * (defaultnum + 1));
	for (i = 0; i < defaultnum; i++) {
		char temp[4096];
		int c;
		int a = 0;
		while ((c = getc(in)) != 0) {
			temp[a++] = c;
		}
		temp[a++] = 0;
		srouter_defaults[i].name = strdup(temp);
		srouter_defaults[i].value = values[index[i]];
	}
	free(index);
	fclose(in);
	return srouter_defaults;
}

void free_defaults(struct nvram_param *srouter_defaults)
{
	int i;
	for (i = defaultnum - 1; i > -1; i--) {
		if (srouter_defaults[i].name) {
			free(srouter_defaults[i].name);
		}
	}
	free(srouter_defaults);
	for (i = stores - 1; i > -1; i--) {
		free(values[i]);
	}
	free(values);

}

#include "nvram_generics.h"
