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
#include <malloc.h>
#include <stdarg.h>

#define PATH_DEV_NVRAM "/dev/nvram"
#define NVRAM_SPACE_MAGIC 0x50534341 /* 'SPAC' */

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

void nvram_open(void) // dummy
{
}

void nvram_close(void) //dummy
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
		return 0; //ignore if already set
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
	stores = getc(in); // count of unique values

	unsigned char *index;
	index = malloc(sizeof(char) * defaultnum);
	fread(index, defaultnum, 1, in); // read string index table

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

void nvram_store_collection(const char *name, char *buf)
{
	char *chain;
	char *n;
	int size = strlen(buf);
	int chaincount = size / 1024;
	int c = 0;
	int i, offset;
	offset = 0;
	for (i = 0; i < chaincount; i++) {
		n = malloc(strlen(name) + 16);
		sprintf(n, "%s%d", name, c++);
		chain = malloc(1025);
		memcpy(chain, &buf[offset], 1024);
		chain[1024] = 0;
		nvram_set(n, chain);
		offset += 1024;
		free(n);
		free(chain);
	}
	int rest = size % 1024;
	if (rest) {
		n = malloc(strlen(name) + 16);
		sprintf(n, "%s%d", name, c);
		chain = malloc(rest + 1);
		memcpy(chain, &buf[offset], rest);
		chain[rest] = 0;
		nvram_set(n, chain);
		free(n);
		free(chain);
	}
}

/*
    do not forget to free the returned result
*/
char *nvram_get_collection(const char *name)
{
	char *chains = NULL;
	int offset = 0;
	int c = 0;
	char n[65];

	snprintf(n, sizeof(n), "%s%d", name, c++);
	while (nvram_get(n) != NULL) {
		char *chain = nvram_get(n);
		if (chains == NULL)
			chains = malloc(strlen(chain) + 1);
		chains = (char *)realloc(chains, chains != NULL ? strlen(chains) + strlen(chain) + 1 : strlen(chain) + 1);
		memcpy(&chains[offset], chain, strlen(chain));
		offset += strlen(chain);
		chains[offset] = 0;
		snprintf(n, sizeof(n), "%s%d", name, c++);
	}
	return chains;
}

/*
 * Match an NVRAM variable.
 * @param	name	name of variable to match
 * @param	match	value to compare against value of variable
 * @return	TRUE if variable is defined and its value is string equal
 *		to match or FALSE otherwise
 */
int nvram_match(const char *name, const char *match)
{
	const char *value = nvram_get(name);
	return (value && !strcmp(value, match));
}

int nvram_matchi(const char *name, const int match)
{
	char tmp[100];
	snprintf(tmp, sizeof(tmp), "%d", match);
	return nvram_match(name, tmp);
}

/*
 * Inversely match an NVRAM variable.
 * @param	name	name of variable to match
 * @param	match	value to compare against value of variable
 * @return	TRUE if variable is defined and its value is not string
 *		equal to invmatch or FALSE otherwise
 */
int nvram_invmatch(const char *name, const char *invmatch)
{
	const char *value = nvram_get(name);
	return (value && strcmp(value, invmatch));
}

int nvram_invmatchi(const char *name, const int invmatch)
{
	char tmp[100];
	snprintf(tmp, sizeof(tmp), "%d", invmatch);
	return nvram_invmatch(name, tmp);
}

char *nvram_prefix_get(const char *name, const char *prefix)
{
	char p[64];
	snprintf(p, sizeof(p), "%s_%s", prefix, name);
	return nvram_safe_get(p);
}

int nvram_prefix_match(const char *name, const char *prefix, const char *match)
{
	char p[64];
	snprintf(p, sizeof(p), "%s_%s", prefix, name);
	return nvram_match(p, match);
}

#define NVGETARGS(varbuf)                                     \
	{                                                     \
		va_list args;                                 \
		va_start(args, (char *)fmt);                  \
		vsnprintf(varbuf, sizeof(varbuf), fmt, args); \
		va_end(args);                                 \
	}

int nvram_nmatch(const char *match, const char *fmt, ...)
{
	char varbuf[64];
	NVGETARGS(varbuf);
	return nvram_match(varbuf, match);
}

int nvram_default_nmatch(const char *match, const char *def, const char *fmt, ...)
{
	char varbuf[64];
	NVGETARGS(varbuf);
	return nvram_default_match(varbuf, match, def);
}

int nvram_default_nmatchi(const int match, const int def, const char *fmt, ...)
{
	char varbuf[64];
	NVGETARGS(varbuf);
	return nvram_default_matchi(varbuf, match, def);
}

int nvram_nmatchi(const int match, const char *fmt, ...)
{
	char varbuf[64];
	NVGETARGS(varbuf);
	char tmp[100];
	snprintf(tmp, sizeof(tmp), "%d", match);
	return nvram_match(varbuf, tmp);
}

char *nvram_nget(const char *fmt, ...)
{
	char varbuf[64];
	NVGETARGS(varbuf);
	return nvram_safe_get(varbuf);
}

char *nvram_default_nget(const char *def, const char *fmt, ...)
{
	char varbuf[64];
	NVGETARGS(varbuf);
	return nvram_default_get(varbuf, def);
}

int nvram_default_ngeti(const int def, const char *fmt, ...)
{
	char varbuf[64];
	NVGETARGS(varbuf);
	return nvram_default_geti(varbuf, def);
}

int nvram_ngeti(const char *fmt, ...)
{
	char varbuf[64];
	NVGETARGS(varbuf);
	return nvram_default_geti(varbuf, 0);
}

int nvram_nset(const char *value, const char *fmt, ...)
{
	char varbuf[64];
	NVGETARGS(varbuf);
	return nvram_set(varbuf, value);
}

int nvram_nseti(const int value, const char *fmt, ...)
{
	char varbuf[64];
	NVGETARGS(varbuf);
	char tmp[100];
	snprintf(tmp, sizeof(tmp), "%d", value);
	return nvram_set(varbuf, tmp);
}

char *nvram_safe_get(const char *name)
{
	return nvram_get(name) ?: "";
}

int nvram_exists(const char *name)
{
	return nvram_get(name) ? 1 : 0;
}

int nvram_empty(const char *name)
{
	return strlen(nvram_safe_get(name)) == 0;
}

int nvram_nexists(const char *fmt, ...)
{
	char varbuf[64];
	NVGETARGS(varbuf);
	return nvram_get(varbuf) ? 1 : 0;
}

int nvram_geti(const char *name)
{
	return atoi(nvram_safe_get(name));
}

void nvram_seti(const char *name, const int value)
{
	char tmp[100];
	snprintf(tmp, sizeof(tmp), "%d", value);
	nvram_set(name, tmp);
}

void nvram_safe_unset(const char *name)
{
	if (nvram_get(name))
		nvram_unset(name);
}

void nvram_safe_set(const char *name, char *value)
{
	if (!nvram_get(name) || strcmp(nvram_safe_get(name), value))
		nvram_set(name, value);
}

int nvram_default_match(const char *var, const char *match, const char *def)
{
	const char *v = nvram_get(var);
	if (v == NULL || !*v) {
		nvram_set(var, def);
		v = def;
	}
	return !strcmp(v, match);
}

int nvram_default_matchi(const char *var, const int match, const int def)
{
	char m[32];
	char d[32];
	snprintf(m, sizeof(m), "%d", match);
	snprintf(d, sizeof(d), "%d", def);
	return nvram_default_match(var, m, d);
}

char *nvram_default_get(const char *var, const char *def)
{
	char *v = nvram_get(var);
	if (v == NULL || !*v) {
		nvram_set(var, def);
		return (char *)def;
	}
	return v;
}

int nvram_default_geti(const char *var, const int def)
{
	char tmp[100];
	char *v = nvram_get(var);
	if (v == NULL || !*v) {
		snprintf(tmp, sizeof(tmp), "%d", def);
		nvram_set(var, tmp);
		return def;
	}
	return atoi(v);
}

void fwritenvram(const char *var, FILE *fp)
{
	int i;
	if (fp == NULL)
		return;
	const char *host_key = nvram_safe_get(var);
	int len = strlen(host_key);
	for (i = 0; i < len; i++)
		if (host_key[i] != '\r')
			fprintf(fp, "%c", host_key[i]);
}

void writenvram(const char *var, char *file)
{
	FILE *fp = fopen(file, "wb");
	if (fp == NULL)
		return;
	fwritenvram(var, fp);
	fclose(fp);
}

int write_nvram(char *name, char *nv)
{
	if (nvram_invmatch(nv, "")) {
		FILE *fp = fopen(name, "wb");

		if (fp) {
			fwritenvram(nv, fp);
			fprintf(fp, "\n");
			fclose(fp);
		}
	} else
		return -1;
	return 0;
}

#define foreach_int(word, wordlist, next)                                                                                 \
	for (next = &wordlist[strspn(wordlist, " ")], strncpy(word, next, sizeof(word)), word[strcspn(word, " ")] = '\0', \
	    word[sizeof(word) - 1] = '\0', next = strchr(next, ' ');                                                      \
	     word[0]; next = next ? &next[strspn(next, " ")] : "", strncpy(word, next, sizeof(word)),                     \
	    word[strcspn(word, " ")] = '\0', word[sizeof(word) - 1] = '\0', next = strchr(next, ' '))

static int update_state(char *file, char *nvram)
{
	char *nv = nvram_safe_get(nvram);
	FILE *fp = fopen(file, "wb");
	if (fp) {
		fwrite(nv, strlen(nv), 1, fp);
		fclose(fp);
	}
	return 1;
}

/*
 * returns 1 if nvram content has changed
 */
static int internal_nvram_state(char *nvram, int zerofirstrun)
{
	char file[128];
	char *nv = nvram_safe_get(nvram);
	snprintf(file, sizeof(file), "/tmp/nvstate/%s.state", nvram);
	//	fprintf(stderr, "open %s\n", file);
	FILE *fp = fopen(file, "rb");
	if (!fp) {
		//		fprintf(stderr, "no exist\n");
		mkdir("/tmp/nvstate", 0700);
		int ret = update_state(file, nvram);
		//		fprintf(stderr, "update states returns %s: %d zero %d\n",nvram, ret, zerofirstrun);
		if (zerofirstrun)
			return 0;
		return ret;
	}
	fseek(fp, 0, SEEK_END);
	size_t len = ftell(fp);
	if (!len) {
		fclose(fp);
		if (!*nv) {
			//			fprintf(stderr, "nv %s is empty\n",nvram);
			return 0;
		}
		return update_state(file, nvram);
	}
	rewind(fp);
	char *checkbuf = malloc(len + 1);
	fread(checkbuf, len, 1, fp);
	fclose(fp);
	checkbuf[len] = 0;
	if (!strcmp(nv, checkbuf)) {
		//		fprintf(stderr, "%s is equal (checkbuf %s)\n", nvram, checkbuf);
		free(checkbuf);
		return 0;
	}
	//	fprintf(stderr, "%s is not equal (checkbuf %s)\n", nvram, checkbuf);
	free(checkbuf);
	return update_state(file, nvram);
}

int nvram_state(char *nvram)
{
	return internal_nvram_state(nvram, 0);
}

int nvram_state_change(char *nvram)
{
	return internal_nvram_state(nvram, 1);
}

int nvram_states(char *list)
{
	char *next;
	char var[128];
	int retval = 0;
	foreach_int(var, list, next)
	{
		if (nvram_state(var))
			retval = 1;
	}
	return retval;
}

int nvram_delstates(char *list)
{
	char *next;
	char var[128];
	int retval = 0;
	foreach_int(var, list, next)
	{
		char p[64];
		snprintf(p, sizeof(p), "/tmp/nvstate/%s.state", var);
		unlink(p);
	}
	return retval;
}

int nvhas(char *nvname, char *key)
{
	char *next;
	char var[32];
	char *list = nvram_safe_get(nvname);
	foreach_int(var, list, next)
	{
		if (!strcmp(var, key))
			return 1;
	}
	return 0;
}
