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
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <error.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include <typedefs.h>
#include <bcmnvram.h>
#include <nvram_convert.h>

#define PATH_DEV_NVRAM "/dev/nvram"

#ifdef NVRAM_SPACE_256
#define NVRAMSPACE NVRAM_SPACE_256
#else
#define NVRAMSPACE NVRAM_SPACE
#endif



/* Globals */
static int nvram_fd = -1;
static char *nvram_buf = NULL;

int nvram_init(void *unused)
{
	if ((nvram_fd = open(PATH_DEV_NVRAM, O_RDWR)) < 0)
		goto err;

	/* Map kernel string buffer into user space */
	nvram_buf = mmap(NULL, NVRAMSPACE, PROT_READ, MAP_SHARED, nvram_fd, 0);

	if (nvram_buf == MAP_FAILED) {
		close(nvram_fd);
		fprintf(stderr, "nvram_init(): failed\n");
		nvram_fd = -1;
		goto err;
	}
	fcntl(nvram_fd, F_SETFD, FD_CLOEXEC);

	return 0;

err:
	return errno;
}

void lock(void)
{
	FILE *in;
	int lockwait = 0;
	while ((in = fopen("/tmp/.nvlock", "rb")) != NULL) {
		fclose(in);
		fprintf(stderr, "nvram lock, waiting....\n");
		lockwait++;
		if (lockwait == 3)
			unlink("/tmp/.nvlock");	//something crashed, we fix it
		sleep(1);
	}
	in = fopen("/tmp/.nvlock", "wb");
	if (in) {
		fprintf(in, "lock");
		fclose(in);
	}
}

void unlock(void)
{
	unlink("/tmp/.nvlock");
}

char *nvram_get(const char *name)
{
	lock();
	size_t count = strlen(name) + 1;
	char *value;
	unsigned long *off;

	if (nvram_fd < 0) {
#ifdef HAVE_X86
		FILE *in = fopen("/usr/local/nvram/nvram.bin", "rb");
		if (in == NULL) {
			unlock();
			return NULL;
		}
		fclose(in);
#endif
		if (nvram_init(NULL)) {
			unlock();
			return NULL;
		}
	}
	if (!(off = malloc(count))) {
		unlock();
		return NULL;
	}

	/* Get offset into mmap() space */
	strcpy((char *)off, name);
#ifndef HAVE_MICRO
	msync(nvram_buf, NVRAMSPACE, MS_SYNC);
#endif

	count = read(nvram_fd, off, count);
	if (count == sizeof(unsigned long))
		value = &nvram_buf[*off];
	else
		value = NULL;

	if (count < 0)
		perror(PATH_DEV_NVRAM);

	free(off);
	unlock();

	return value;
}

int nvram_getall(char *buf, int count)
{
//lock();
	int ret;

	if (nvram_fd < 0) {
#ifdef HAVE_X86
		FILE *in = fopen("/usr/local/nvram/nvram.bin", "rb");
		if (in == NULL)
			return 0;
		fclose(in);
#endif
		if ((ret = nvram_init(NULL))) {
			//unlock();
			return ret;
		}
	}
	if (count == 0) {
		//unlock();
		return 0;
	}
	/* Get all variables */
	*buf = '\0';

	ret = read(nvram_fd, buf, count);

	if (ret < 0)
		perror(PATH_DEV_NVRAM);
	//unlock();
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
	lock();
	size_t count = strlen(name) + 1;
	char *buf;
	int ret;

	if (nvram_fd < 0)
		if ((ret = nvram_init(NULL))) {
			unlock();
			return ret;
		}

	/* Wolf add - keep nvram varname to sane len - may prevent corruption */
	if (strlen(name) > 64) {
		unlock();
		return -ENOMEM;
	}

	/* Unset if value is NULL */
	if (value)
		count += strlen(value) + 2;

	if (!(buf = malloc(count))) {
		unlock();
		return -ENOMEM;
	}

	if (value)
		sprintf(buf, "%s=%s", name, value);
	else
		strcpy(buf, name);

	count = strlen(buf) + 1;
	ret = write(nvram_fd, buf, count);

	if (ret < 0)
		perror(PATH_DEV_NVRAM);

	free(buf);
	unlock();
	return (ret == count) ? 0 : ret;
}

int nvram_set(const char *name, const char *value)
{
	extern struct nvram_convert nvram_converts[];
	struct nvram_convert *v;
	int ret;
#ifdef HAVE_NOWIFI
	if (!strcmp(name, "ip_conntrack_max") && value != NULL) {
		int val = atoi(value);
		if (val > 4096) {
			return _nvram_set(name, "4096");
		}

	}
#endif
	ret = _nvram_set(name, value);

	for (v = nvram_converts; v->name; v++) {
		if (!strcmp(v->name, name)) {
			if (strcmp(v->wl0_name, ""))
				_nvram_set(v->wl0_name, value);
		}
	}
	return ret;
}

int nvram_immed_set(const char *name, const char *value)
{
	return nvram_set(name, value);
}

int nvram_unset(const char *name)
{
//lock();
	int v = _nvram_set(name, NULL);
//unlock();
	return v;
}

int nvram_commit(void)
{
	if (nvram_match("flash_active", "1")) {
		fprintf(stderr, "not allowed, flash process in progress");
		exit(1);
	}
#if defined(HAVE_WZRHPG300NH) || defined(HAVE_WHRHPGN) || defined(HAVE_WZRHPAG300NH) || defined(HAVE_DIR825) || defined(HAVE_TEW632BRP) || defined(HAVE_TG2521) || defined(HAVE_WR1043)  || defined(HAVE_WRT400) || defined(HAVE_WZRHPAG300NH) || defined(HAVE_WZRG450) || defined(HAVE_DANUBE) || defined(HAVE_WR741) || defined(HAVE_NORTHSTAR) || defined(HAVE_DIR615I) || defined(HAVE_WDR4900) || defined(HAVE_VENTANA)
	system("/sbin/ledtool 1");
#elif HAVE_LSX
	//nothing
#else
	system("/sbin/ledtool 1");
#endif
	lock();
	int ret;
	if (nvram_fd < 0) {
		if ((ret = nvram_init(NULL))) {
			fprintf(stderr, "nvram_commit(): failed\n");
			unlock();
			return ret;
		}
	}
	ret = ioctl(nvram_fd, NVRAM_MAGIC, NULL);

	if (ret < 0) {
		fprintf(stderr, "nvram_commit(): failed\n");
		perror(PATH_DEV_NVRAM);
	}
	unlock();
	sync();
	return ret;
}

#if 0
int file2nvram(char *filename, char *varname)
{
	FILE *fp;
	int c, count;
	int i = 0, j = 0;
	char mem[10000], buf[30000];

	if (!(fp = fopen(filename, "rb")))
		return 0;

	count = fread(mem, 1, sizeof(mem), fp);
	fclose(fp);
	for (j = 0; j < count; j++) {
		if (i > sizeof(buf) - 3)
			break;
		c = mem[j];
		if (c >= 32 && c <= 126 && c != '~') {
			buf[i++] = (unsigned char)c;
		} else if (c == 13) {
			buf[i++] = (unsigned char)c;
		} else if (c == 0) {
			buf[i++] = '~';
		} else if (c == 10) {
			buf[i++] = (unsigned char)c;
		} else {
			buf[i++] = '\\';
			sprintf(buf + i, "%02X", c);
			i += 2;
		}
	}
	if (i == 0)
		return 0;
	buf[i] = 0;
	//fprintf(stderr,"================ > file2nvram %s = [%s] \n",varname,buf); 
	nvram_set(varname, buf);
	return 0;

}
#endif
int nvram2file(char *varname, char *filename)
{
	FILE *fp;
	int c, tmp;
	int i = 0;
	char *buf;

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

#include "nvram_generics.h"
