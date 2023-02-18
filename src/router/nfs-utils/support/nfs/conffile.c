/*	$OpenBSD: conf.c,v 1.55 2003/06/03 14:28:16 ho Exp $	*/
/*	$EOM: conf.c,v 1.48 2000/12/04 02:04:29 angelos Exp $	*/

/*
 * Copyright (c) 1998, 1999, 2000, 2001 Niklas Hallqvist.  All rights reserved.
 * Copyright (c) 2000, 2001, 2002 Håkan Olsson.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This code was written under funding by Ericsson Radio Systems.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/param.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <syslog.h>
#include <libgen.h>
#include <sys/file.h>
#include <time.h>
#include <dirent.h>

#include "conffile.h"
#include "xlog.h"

#define CONF_FILE_EXT ".conf"
#define CONF_FILE_EXT_LEN ((int) (sizeof(CONF_FILE_EXT) - 1))

#pragma GCC visibility push(hidden)

static void conf_load_defaults(void);
static char * conf_readfile(const char *path);
static int conf_set(int , const char *, const char *, const char *,
	const char *, int , int );
static void conf_parse(int trans, char *buf,
	char **section, char **subsection, const char *filename);

struct conf_trans {
	TAILQ_ENTRY (conf_trans) link;
	int trans;
	enum conf_op { CONF_SET, CONF_REMOVE, CONF_REMOVE_SECTION } op;
	char *section;
	char *arg;
	char *tag;
	char *value;
	int override;
	int is_default;
};

TAILQ_HEAD (conf_trans_head, conf_trans) conf_trans_queue;

/*
 * Radix-64 Encoding.
 */

static const uint8_t asc2bin[] =
{
  255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255, 255, 255, 255, 255, 255,
  255, 255, 255,  62, 255, 255, 255,  63,
   52,  53,  54,  55,  56,  57,  58,  59,
   60,  61, 255, 255, 255, 255, 255, 255,
  255,   0,   1,   2,   3,   4,   5,   6,
    7,   8,   9,  10,  11,  12,  13,  14,
   15,  16,  17,  18,  19,  20,  21,  22,
   23,  24,  25, 255, 255, 255, 255, 255,
  255,  26,  27,  28,  29,  30,  31,  32,
   33,  34,  35,  36,  37,  38,  39,  40,
   41,  42,  43,  44,  45,  46,  47,  48,
   49,  50,  51, 255, 255, 255, 255, 255
};

struct conf_binding {
  LIST_ENTRY (conf_binding) link;
  char *section;
  char *arg;
  char *tag;
  char *value;
  int is_default;
};

LIST_HEAD (conf_bindings, conf_binding) conf_bindings[256];

const char *modified_by = NULL;

static __inline__ uint8_t
conf_hash(const char *s)
{
	uint8_t hash = 0;

	while (*s) {
		hash = ((hash << 1) | (hash >> 7)) ^ tolower (*s);
		s++;
	}
	return hash;
}

/*
 * free all the component parts of a conf_binding struct
 */
static void free_confbind(struct conf_binding *cb)
{
	if (!cb)
		return;
	if (cb->section)
		free(cb->section);
	if (cb->arg)
		free(cb->arg);
	if (cb->tag)
		free(cb->tag);
	if (cb->value)
		free(cb->value);
	free(cb);
}

static void free_conftrans(struct conf_trans *ct)
{
	if (!ct)
		return;
	if (ct->section)
		free(ct->section);
	if (ct->arg)
		free(ct->arg);
	if (ct->tag)
		free(ct->tag);
	if (ct->value)
		free(ct->value);
	free(ct);
}

/*
 * Insert a tag-value combination from LINE (the equal sign is at POS)
 */
static int
conf_remove_now(const char *section, const char *tag)
{
	struct conf_binding *cb, *next;

	cb = LIST_FIRST(&conf_bindings[conf_hash (section)]);
	for (; cb; cb = next) {
		next = LIST_NEXT(cb, link);
		if (strcasecmp(cb->section, section) == 0
				&& strcasecmp(cb->tag, tag) == 0) {
			LIST_REMOVE(cb, link);
			xlog(LOG_INFO,"[%s]:%s->%s removed", section, tag, cb->value);
			free_confbind(cb);
			return 0;
		}
	}
	return 1;
}

static int
conf_remove_section_now(const char *section)
{
  struct conf_binding *cb, *next;
  int unseen = 1;

	cb = LIST_FIRST(&conf_bindings[conf_hash (section)]);
	for (; cb; cb = next) {
		next = LIST_NEXT(cb, link);
		if (strcasecmp(cb->section, section) == 0) {
			unseen = 0;
			LIST_REMOVE(cb, link);
			xlog(LOG_INFO, "[%s]:%s->%s removed", section, cb->tag, cb->value);
			free_confbind(cb);
			}
		}
	return unseen;
}

/*
 * Insert a tag-value combination from LINE (the equal sign is at POS)
 * into SECTION of our configuration database.
 */
static int
conf_set_now(const char *section, const char *arg, const char *tag,
	const char *value, int override, int is_default)
{
	struct conf_binding *node = 0;

	if (override)
		conf_remove_now(section, tag);
	else if (conf_get_section(section, arg, tag)) {
		if (!is_default) {
			xlog(LOG_INFO, "conf_set: duplicate tag [%s]:%s, ignoring...",
				section, tag);
		}
		return 1;
	}
	node = calloc(1, sizeof *node);
	if (!node) {
		xlog_warn("conf_set: calloc (1, %lu) failed", (unsigned long)sizeof *node);
		return 1;
	}
	node->section = strdup(section);
	if (arg)
		node->arg = strdup(arg);
	node->tag = strdup(tag);
	node->value = strdup(value);
	node->is_default = is_default;
	LIST_INSERT_HEAD(&conf_bindings[conf_hash (section)], node, link);
	return 0;
}

/* Attempt to construct a relative path to the new file */
static char *
relative_path(const char *oldfile, const char *newfile)
{
	char *tmpcopy = NULL;
	char *dir = NULL;
	char *relpath = NULL;
	size_t pathlen;

	if (!newfile)
		return strdup(oldfile);

	if (newfile[0] == '/')
		return strdup(newfile);

	tmpcopy = strdup(oldfile);
	if (!tmpcopy)
		goto mem_err;

	dir = dirname(tmpcopy);

	pathlen = strlen(dir) + strlen(newfile) + 2;
	relpath = calloc(1, pathlen);
	if (!relpath)
		goto mem_err;

	snprintf(relpath, pathlen, "%s/%s", dir, newfile);

	free(tmpcopy);
	return relpath;
mem_err:
	if (tmpcopy)
		free(tmpcopy);
	return NULL;
}


/*
 * Parse the line LINE of SZ bytes.  Skip Comments, recognize section
 * headers and feed tag-value pairs into our configuration database.
 */
static void
conf_parse_line(int trans, char *line, const char *filename, int lineno, char **section, char **subsection)
{
	char *val, *ptr;
	char *inc_section = NULL, *inc_subsection = NULL;
	char *relpath, *subconf;

	/* Strip off any leading blanks */
	while (isspace(*line))
		line++;

	/* Ignore blank lines */
	if (*line == '\0')
		return;

	/* Lines starting with '#' or ';' are comments.  */
	if (*line == '#' || *line == ';')
		return;

	/* '[section]' parsing...  */
	if (*line == '[') {
		line++;

		if (*section) {
			free(*section);
			*section = NULL;
		}
		if (*subsection) {
			free(*subsection);
			*subsection = NULL;
		}

		/* Strip off any blanks after '[' */
		while (isblank(*line))
			line++;

		/* find the closing ] */
		ptr = strchr(line, ']');
		if (ptr == NULL) {
			xlog_warn("config error at %s:%d: "
				  "non-matched ']', ignoring until next section",
				  filename, lineno);
			return;
		}

		/* just ignore everything after the closing ] */
		*(ptr--) = '\0';

		/* Strip off any blanks before ']' */
		while (ptr >= line && isblank(*ptr))
			*(ptr--)='\0';

		/* look for an arg to split from the section name */
		val = strchr(line, '"');
		if (val != NULL) {
			ptr = val - 1;
			*(val++) = '\0';

			/* trim away any whitespace before the " */
			while (ptr > line && isblank(*ptr))
				*(ptr--)='\0';
		}

		/* copy the section name */
		*section = strdup(line);
		if (!*section) {
			xlog_warn("config error at %s:%d:"
				  "malloc failed", filename, lineno);
			return;
		}

		/* there is no arg, we are done */
		if (val == NULL) 
			return;

		/* check for the closing " */
		ptr = strchr(val, '"');
		if (ptr == NULL) {
			xlog_warn("config error at %s:%d: "
				  "non-matched '\"', ignoring until next section",
				filename, lineno);
			return;
		}
		*ptr = '\0';
		*subsection = strdup(val);
		if (!*subsection)
			xlog_warn("config error at %s:%d: "
				  "malloc failed", filename, lineno);
		return;
	}

	/* Deal with assignments.  */
	ptr = strchr(line, '=');

	/* not an assignment line */
	if (ptr == NULL) {
		/* Other non-empty lines are weird.  */
		if (line[strspn(line, " \t")])
			xlog_warn("config error at %s:%d: "
				"line not empty and not an assignment",
				filename, lineno);
		return;
	}

	/* If no section, we are ignoring the line.  */
	if (!*section) {
		xlog_warn("config error at %s:%d: "
			  "ignoring line not in a section",
			  filename, lineno);
		return;
	}

	val = ptr + 1;
	*(ptr--) = '\0';

	/* strip spaces before and after the = */
	while (ptr >= line && isblank(*ptr))
		*(ptr--)='\0';
	while (*val != '\0' && isblank(*val))
		val++;

	if (*val == '"') {
		val++;
		ptr = strchr(val, '"');
		if (ptr == NULL) {
			xlog_warn("config error at %s:%d: "
				  "unmatched quotes",filename, lineno);
			return;
		}
		*ptr = '\0';
	} else
	if (*val == '\'') {
		val++;
		ptr = strchr(val, '\'');
		if (ptr == NULL) {
			xlog_warn("config error at %s:%d: "
				  "unmatched quotes", filename, lineno);
			return;
		}
		*ptr = '\0';
	} else {
		/* Trim any trailing spaces and comments */
		if ((ptr=strchr(val, '#'))!=NULL)
			*ptr = '\0';
		if ((ptr=strchr(val, ';'))!=NULL)
			*ptr = '\0';

		ptr = val + strlen(val) - 1;
		while (ptr > val && isspace(*ptr))
			*(ptr--) = '\0';
	}

	if (*line == '\0') {
		xlog_warn("config error at %s:%d: "
			  "missing tag in assignment", filename, lineno);
		return;
	}

	if (strcasecmp(line, "include")==0) {
		/* load and parse subordinate config files */
		_Bool optional = false;

		if (val && *val == '-') {
			optional = true;
			val++;
		}

		relpath = relative_path(filename, val);
		if (relpath == NULL) {
			if (!optional)
				xlog_warn("config error at %s:%d: error loading included config",
					  filename, lineno);
			return;
		}

		subconf = conf_readfile(relpath);
		if (subconf == NULL) {
			if (!optional)
				xlog_warn("config error at %s:%d: error loading included config",
					  filename, lineno);
			if (relpath)
				free(relpath);
			return;
		}

		/* copy the section data so the included file can inherit it
		 * without accidentally changing it for us */
		if (*section != NULL) {
			inc_section = strdup(*section);
			if (*subsection != NULL)
				inc_subsection = strdup(*subsection);
		}

		conf_parse(trans, subconf, &inc_section, &inc_subsection, relpath);

		if (inc_section)
			free(inc_section);
		if (inc_subsection)
			free(inc_subsection);
		if (relpath)
			free(relpath);
		free(subconf);
	} else {
		/* XXX Perhaps should we not ignore errors?  */
		conf_set(trans, *section, *subsection, line, val, 1, 0);
	}
}

/* Parse the mapped configuration file.  */
static void
conf_parse(int trans, char *buf, char **section, char **subsection, const char *filename)
{
	char *cp = buf;
	char *bufend = NULL;
	char *line;
	int lineno = 0;

	line = cp;
	bufend = buf + strlen(buf);
	while (cp < bufend) {
		if (*cp == '\n') {
			/* Check for escaped newlines.  */
			if (cp > buf && *(cp - 1) == '\\')
				*(cp - 1) = *cp = ' ';
			else {
				*cp = '\0';
				lineno++;
				conf_parse_line(trans, line, filename, lineno, section, subsection);
				line = cp + 1;
			}
		}
		cp++;
	}
	if (cp != line)
		xlog_warn("conf_parse: last line non-terminated, ignored.");
}

static void
conf_load_defaults(void)
{
	/* No defaults */
	return;
}

static char *
conf_readfile(const char *path)
{
	struct stat sb;
	if (!path) {
		xlog(L_ERROR, "conf_readfile: no path given");
		return NULL;
	}

	if ((stat (path, &sb) == 0) || (errno != ENOENT)) {
		char *new_conf_addr = NULL;
		off_t sz;
		int fd = open (path, O_RDONLY, 0);

		if (fd == -1) {
			xlog_warn("conf_readfile: open (\"%s\", O_RDONLY) failed", path);
			return NULL;
		}

		/* Grab a shared lock to ensure its not mid-rewrite */
		if (flock(fd, LOCK_SH)) {
			xlog_warn("conf_readfile: attempt to grab read lock failed: %s",
				strerror(errno));
			goto fail;
		}

		/* only after we have the lock, check the file size ready to read it */
		sz = lseek(fd, 0, SEEK_END);
		if (sz < 0) {
			xlog_warn("conf_readfile: unable to determine file size: %s",
				  strerror(errno));
			goto fail;
		}
		lseek(fd, 0, SEEK_SET);

		new_conf_addr = malloc(sz+1);
		if (!new_conf_addr) {
			xlog_warn("conf_readfile: malloc (%lu) failed", (unsigned long)sz);
			goto fail;
		}

		/* XXX I assume short reads won't happen here.  */
		if (read (fd, new_conf_addr, sz) != (int)sz) {
			xlog_warn("conf_readfile: read (%d, %p, %lu) failed",
				fd, new_conf_addr, (unsigned long)sz);
			goto fail;
		}
		close(fd);

		/* XXX Should we not care about errors and rollback?  */
		new_conf_addr[sz] = '\0';
		return new_conf_addr;
	fail:
		close(fd);
		if (new_conf_addr) 
			free(new_conf_addr);
	}
	return NULL;
}

/* remove and free up any existing config state */
static void conf_free_bindings(void)
{
	unsigned int i;
	for (i = 0; i < sizeof conf_bindings / sizeof conf_bindings[0]; i++) {
		struct conf_binding *cb, *next;

		cb = LIST_FIRST(&conf_bindings[i]);
		for (; cb; cb = next) {
			next = LIST_NEXT(cb, link);
			LIST_REMOVE(cb, link);
			free_confbind(cb);
		}
		LIST_INIT(&conf_bindings[i]);
	}
}

static int
conf_load_files(int trans, const char *conf_file)
{
	char *conf_data;
	char *section = NULL;
	char *subsection = NULL;

	conf_data = conf_readfile(conf_file);
	if (conf_data == NULL)
		return 1;

	/* Load default configuration values.  */
	conf_load_defaults();

	/* Parse config contents into the transaction queue */
	conf_parse(trans, conf_data, &section, &subsection, conf_file);
	if (section) 
		free(section);
	if (subsection) 
		free(subsection);
	free(conf_data);

	return 0;
}
/* Open the config file and map it into our address space, then parse it.  */
static int
conf_load_file(const char *conf_file)
{
	int trans;
	char * conf_data;

	trans = conf_begin();
	conf_data = conf_readfile(conf_file);

	if (conf_data == NULL)
		return 1;

	/* Load default configuration values.  */
	conf_load_defaults();

	/* Parse config contents into the transaction queue */
	char *section = NULL;
	char *subsection = NULL;
	conf_parse(trans, conf_data, &section, &subsection, conf_file);
	if (section) free(section);
	if (subsection) free(subsection);
	free(conf_data);

	/* Free potential existing configuration.  */
	conf_free_bindings();

	/* Apply the new configuration values */
	conf_end(trans, 1);
	return 0;
}

static void 
conf_init_dir(const char *conf_file)
{
	struct dirent **namelist = NULL;
	char *dname, fname[PATH_MAX], *cname;
	int n = 0, nfiles = 0, i, fname_len, dname_len;
	int trans, rv, path_len;

	dname = malloc(strlen(conf_file) + 3);
	if (dname == NULL) {
		xlog(L_WARNING, "conf_init_dir: malloc: %s", strerror(errno));
		return;	
	}
	sprintf(dname, "%s.d", conf_file);

	n = scandir(dname, &namelist, NULL, versionsort);
	if (n < 0) {
		if (errno != ENOENT) {
			xlog(L_WARNING, "conf_init_dir: scandir %s: %s", 
				dname, strerror(errno));
		}
		free(dname);
		return;
	} else if (n == 0) {
		free(dname);
		return;
	}

	trans = conf_begin();
	dname_len = strlen(dname);
	for (i = 0; i < n; i++ ) {
		struct dirent *d = namelist[i];

	 	switch (d->d_type) {
			case DT_UNKNOWN:
			case DT_REG:
			case DT_LNK:
				break;
			default:
				continue;
		}
		if (*d->d_name == '.')
			continue;
		
		fname_len = strlen(d->d_name);
		path_len = (fname_len + dname_len);
		if (!fname_len || path_len > PATH_MAX) {
			xlog(L_WARNING, "conf_init_dir: Too long file name: %s in %s", 
				d->d_name, dname);
			continue; 
		}

		/*
		 * Check the naming of the file. Only process files
		 * that end with CONF_FILE_EXT
		 */
		if (fname_len <= CONF_FILE_EXT_LEN) {
			xlog(D_GENERAL, "conf_init_dir: %s: name too short", 
				d->d_name);
			continue;
		}
		cname = (d->d_name + (fname_len - CONF_FILE_EXT_LEN));
		if (strcmp(cname, CONF_FILE_EXT) != 0) {
			xlog(D_GENERAL, "conf_init_dir: %s: invalid file extension", 
				d->d_name);
			continue;
		}

		rv = snprintf(fname, PATH_MAX, "%s/%s", dname, d->d_name);
		if (rv < path_len) {
			xlog(L_WARNING, "conf_init_dir: file name: %s/%s too short", 
				d->d_name, dname);
			continue;
		}

		if (conf_load_files(trans, fname))
			continue;
		nfiles++;
	}

	if (nfiles) {
		/* Apply the configuration values */
		conf_end(trans, 1);
	}
	for (i = 0; i < n; i++)
		free(namelist[i]);
	free(namelist);
	free(dname);
	
	return;
}

int
conf_init_file(const char *conf_file)
{
	unsigned int i;
	int ret;

	for (i = 0; i < sizeof conf_bindings / sizeof conf_bindings[0]; i++)
		LIST_INIT (&conf_bindings[i]);

	TAILQ_INIT (&conf_trans_queue);

	if (conf_file == NULL) 
		conf_file=NFS_CONFFILE;

	/*
	 * First parse the give config file 
	 * then parse the config.conf.d directory 
	 * (if it exists)
	 *
	 */
	ret = conf_load_file(conf_file);

	/*
	 * When the same variable is set in both files
	 * the conf.d file will override the config file.
	 * This allows automated admin systems to
	 * have the final say.
	 */
	conf_init_dir(conf_file);

	return ret;
}

/*
 * Empty the config and free up any used memory
 */
void
conf_cleanup(void)
{
	conf_free_bindings();

	struct conf_trans *node, *next;
	for (node = TAILQ_FIRST(&conf_trans_queue); node; node = next) {
		next = TAILQ_NEXT(node, link);
		TAILQ_REMOVE (&conf_trans_queue, node, link);
		free_conftrans(node);
	}
	TAILQ_INIT(&conf_trans_queue);
}

/*
 * Return the numeric value denoted by TAG in section SECTION or DEF
 * if that tag does not exist.
 */
int
conf_get_num(const char *section, const char *tag, int def)
{
	char *value = conf_get_str(section, tag);

	if (value)
		return atoi(value);

	return def;
}

/*
 * Return the Boolean value denoted by TAG in section SECTION, or DEF
 * if that tags does not exist.
 * FALSE is returned for case-insensitive comparisons with 0, f, false, n, no, off
 * TRUE is returned for 1, t, true, y, yes, on
 * A failure to match one of these results in DEF
 */
_Bool
conf_get_bool(const char *section, const char *tag, _Bool def)
{
	char *value = conf_get_str(section, tag);

	if (!value)
		return def;
	if (strcasecmp(value, "1") == 0 ||
	    strcasecmp(value, "t") == 0 ||
	    strcasecmp(value, "true") == 0 ||
	    strcasecmp(value, "y") == 0 ||
	    strcasecmp(value, "yes") == 0 ||
	    strcasecmp(value, "on") == 0)
		return true;

	if (strcasecmp(value, "0") == 0 ||
	    strcasecmp(value, "f") == 0 ||
	    strcasecmp(value, "false") == 0 ||
	    strcasecmp(value, "n") == 0 ||
	    strcasecmp(value, "no") == 0 ||
	    strcasecmp(value, "off") == 0)
		return false;
	return def;
}

/* Validate X according to the range denoted by TAG in section SECTION.  */
int
conf_match_num(const char *section, const char *tag, int x)
{
	char *value = conf_get_str (section, tag);
	int val, min, max, n;

	if (!value)
		return 0;
	n = sscanf (value, "%d,%d:%d", &val, &min, &max);
	switch (n) {
	case 1:
		xlog(LOG_INFO, "conf_match_num: %s:%s %d==%d?", section, tag, val, x);
		return x == val;
	case 3:
		xlog(LOG_INFO, "conf_match_num: %s:%s %d<=%d<=%d?", section,
			tag, min, x, max);
		return min <= x && max >= x;
	default:
		xlog(LOG_INFO, "conf_match_num: section %s tag %s: invalid number spec %s",
			section, tag, value);
	}
	return 0;
}

/* Return the string value denoted by TAG in section SECTION.  */
char *
conf_get_str(const char *section, const char *tag)
{
	return conf_get_section(section, NULL, tag);
}

/* Return the string value denoted by TAG in section SECTION,
 * unless it is not set, in which case return def
 */
char *
conf_get_str_with_def(const char *section, const char *tag, char *def)
{
	char * result = conf_get_section(section, NULL, tag);
	if (!result)
		return def;
	return result;
}

/*
 * Retrieve an entry without interpreting its contents
 */
char *
conf_get_entry(const char *section, const char *arg, const char *tag)
{
	struct conf_binding *cb;

	cb = LIST_FIRST (&conf_bindings[conf_hash (section)]);
	for (; cb; cb = LIST_NEXT (cb, link)) {
		if (strcasecmp(section, cb->section) != 0)
			continue;
		if (arg && (cb->arg == NULL || strcasecmp(arg, cb->arg) != 0))
			continue;
		if (!arg && cb->arg)
			continue;
		if (strcasecmp(tag, cb->tag) != 0)
			continue;
		return cb->value;
	}
	return 0;
}

/*
 * Find a section that may or may not have an argument
 */
char *
conf_get_section(const char *section, const char *arg, const char *tag)
{
	struct conf_binding *cb;
retry:
	cb = LIST_FIRST (&conf_bindings[conf_hash (section)]);
	for (; cb; cb = LIST_NEXT (cb, link)) {
		if (strcasecmp(section, cb->section) != 0)
			continue;
		if (arg && (cb->arg == NULL || strcasecmp(arg, cb->arg) != 0))
			continue;
		if (!arg && cb->arg)
			continue;
		if (strcasecmp(tag, cb->tag) != 0)
			continue;
		if (cb->value[0] == '$') {
			/* expand $name from [environment] section,
			 * or from environment
			 */
			char *env = getenv(cb->value+1);
			if (env && *env)
				return env;
			section = "environment";
			tag = cb->value + 1;
			goto retry;
		}
		return cb->value;
	}
	return 0;
}

/*
 * Build a list of string values out of the comma separated value denoted by
 * TAG in SECTION.
 */
struct conf_list *
conf_get_list(const char *section, const char *tag)
{
	char *liststr = 0, *p, *field, *t;
	struct conf_list *list = 0;
	struct conf_list_node *node;

	list = malloc (sizeof *list);
	if (!list)
		goto cleanup;
	TAILQ_INIT (&list->fields);
	list->cnt = 0;
	liststr = conf_get_str(section, tag);
	if (!liststr)
		goto cleanup;
	liststr = strdup (liststr);
	if (!liststr)
		goto cleanup;
	p = liststr;
	while ((field = strsep (&p, ",")) != NULL) {
		/* Skip leading whitespace */
		while (isspace (*field))
			field++;
		/* Skip trailing whitespace */
		if (p) {
			for (t = p - 1; t > field && isspace (*t); t--)
				*t = '\0';
		}
		if (*field == '\0') {
			xlog(LOG_INFO, "conf_get_list: empty field, ignoring...");
			continue;
		}
		list->cnt++;
		node = calloc (1, sizeof *node);
		if (!node)
			goto cleanup;
		node->field = strdup (field);
		if (!node->field) {
			free(node);
			goto cleanup;
		}
		TAILQ_INSERT_TAIL (&list->fields, node, link);
	}
	free (liststr);
	return list;

cleanup:
	if (list)
		conf_free_list(list);
	if (liststr)
		free(liststr);
	return 0;
}

struct conf_list *
conf_get_tag_list(const char *section, const char *arg)
{
	struct conf_list *list = 0;
	struct conf_list_node *node;
	struct conf_binding *cb;

	list = malloc(sizeof *list);
	if (!list)
		goto cleanup;
	TAILQ_INIT(&list->fields);
	list->cnt = 0;
	cb = LIST_FIRST(&conf_bindings[conf_hash (section)]);
	for (; cb; cb = LIST_NEXT(cb, link)) {
		if (strcasecmp (section, cb->section) == 0) {
			if (arg != NULL && strcasecmp(arg, cb->arg) != 0)
				continue;
			list->cnt++;
			node = calloc(1, sizeof *node);
			if (!node)
				goto cleanup;
			node->field = strdup(cb->tag);
			if (!node->field) {
				free(node);
				goto cleanup;
			}
			TAILQ_INSERT_TAIL(&list->fields, node, link);
		}
	}
	return list;

cleanup:
	if (list)
		conf_free_list(list);
	return 0;
}

/* Decode a PEM encoded buffer.  */
int
conf_decode_base64 (uint8_t *out, uint32_t *len, const unsigned char *buf)
{
	uint32_t c = 0;
	uint8_t c1, c2, c3, c4;

	while (*buf) {
		if (*buf > 127 || (c1 = asc2bin[*buf]) == 255)
			return 0;

		buf++;
		if (*buf > 127 || (c2 = asc2bin[*buf]) == 255)
			return 0;

		buf++;
		if (*buf == '=') {
			c3 = c4 = 0;
			c++;

			/* Check last four bit */
			if (c2 & 0xF)
				return 0;

			if (strcmp((char *)buf, "==") == 0)
				buf++;
			else
				return 0;
		} else if (*buf > 127 || (c3 = asc2bin[*buf]) == 255)
			return 0;
		else {
			if (*++buf == '=') {
				c4 = 0;
				c += 2;

				/* Check last two bit */
				if (c3 & 3)
					return 0;

			if (strcmp((char *)buf, "="))
				return 0;
			} else if (*buf > 127 || (c4 = asc2bin[*buf]) == 255)
				return 0;
			else
				c += 3;
		}

		buf++;
		*out++ = (c1 << 2) | (c2 >> 4);
		*out++ = (c2 << 4) | (c3 >> 2);
		*out++ = (c3 << 6) | c4;
	}

	*len = c;
	return 1;
}

void
conf_free_list(struct conf_list *list)
{
	struct conf_list_node *node = TAILQ_FIRST(&list->fields);

	while (node) {
		TAILQ_REMOVE(&list->fields, node, link);
		if (node->field)
			free(node->field);
		free (node);
		node = TAILQ_FIRST(&list->fields);
	}
	free (list);
}

int
conf_begin(void)
{
  static int seq = 0;

  return ++seq;
}

static struct conf_trans *
conf_trans_node(int transaction, enum conf_op op)
{
	struct conf_trans *node;

	node = calloc (1, sizeof *node);
	if (!node) {
		xlog_warn("conf_trans_node: calloc (1, %lu) failed",
		(unsigned long)sizeof *node);
		return 0;
	}
	node->trans = transaction;
	node->op = op;
	TAILQ_INSERT_TAIL (&conf_trans_queue, node, link);
	return node;
}

/* Queue a set operation.  */
static int
conf_set(int transaction, const char *section, const char *arg,
	const char *tag, const char *value, int override, int is_default)
{
	struct conf_trans *node;

	if (!value || !*value)
		return 0;
	node = conf_trans_node(transaction, CONF_SET);
	if (!node)
		return 1;
	node->section = strdup(section);
	if (!node->section) {
		xlog_warn("conf_set: strdup(\"%s\") failed", section);
		goto fail;
	}
	/* Make Section names case-insensitive */
	upper2lower(node->section);

	if (arg) {
		node->arg = strdup(arg);
		if (!node->arg) {
			xlog_warn("conf_set: strdup(\"%s\") failed", arg);
			goto fail;
		}
	} else
		node->arg = NULL;

	node->tag = strdup(tag);
	if (!node->tag) {
		xlog_warn("conf_set: strdup(\"%s\") failed", tag);
		goto fail;
	}
	node->value = strdup(value);
	if (!node->value) {
		xlog_warn("conf_set: strdup(\"%s\") failed", value);
		goto fail;
	}
	node->override = override;
	node->is_default = is_default;
	return 0;

fail:
	free_conftrans(node);
	return 1;
}

/* Queue a remove operation.  */
int
conf_remove(int transaction, const char *section, const char *tag)
{
	struct conf_trans *node;

	node = conf_trans_node(transaction, CONF_REMOVE);
	if (!node)
		goto fail;
	node->section = strdup(section);
	if (!node->section) {
		xlog_warn("conf_remove: strdup(\"%s\") failed", section);
		goto fail;
	}
	node->tag = strdup(tag);
	if (!node->tag) {
		xlog_warn("conf_remove: strdup(\"%s\") failed", tag);
		goto fail;
	}
	return 0;

fail:
	free_conftrans(node);
	return 1;
}

/* Queue a remove section operation.  */
int
conf_remove_section(int transaction, const char *section)
{
	struct conf_trans *node;

	node = conf_trans_node(transaction, CONF_REMOVE_SECTION);
	if (!node)
		goto fail;
	node->section = strdup(section);
	if (!node->section) {
		xlog_warn("conf_remove_section: strdup(\"%s\") failed", section);
		goto fail;
	}
	return 0;

fail:
	free_conftrans(node);
	return 1;
}

/* Execute all queued operations for this transaction.  Cleanup.  */
int
conf_end(int transaction, int commit)
{
	struct conf_trans *node, *next;

	for (node = TAILQ_FIRST(&conf_trans_queue); node; node = next) {
		next = TAILQ_NEXT(node, link);
		if (node->trans == transaction) {
			if (commit) {
				switch (node->op) {
				case CONF_SET:
					conf_set_now(node->section, node->arg,
						node->tag, node->value, node->override,
						node->is_default);
					break;
				case CONF_REMOVE:
					conf_remove_now(node->section, node->tag);
					break;
				case CONF_REMOVE_SECTION:
					conf_remove_section_now(node->section);
					break;
				default:
					xlog(LOG_INFO, "conf_end: unknown operation: %d", node->op);
				}
			}
			TAILQ_REMOVE (&conf_trans_queue, node, link);
			free_conftrans(node);
		}
	}
	return 0;
}

/*
 * Dump running configuration upon SIGUSR1.
 * Configuration is "stored in reverse order", so reverse it again.
 */
struct dumper {
	char *section;
	char *arg;
	char *tag;
	char *value;
	struct dumper *next;
};

/*
 * Test if two nodes belong to the same (sub)sections
 */
static int
dumper_section_compare(const struct dumper *nodea, const struct dumper *nodeb)
{
	int ret;

	/* missing node, shouldnt happen */
	if (!nodea || !nodeb)
		return -1;

	/* no section names at all, they are equal */
	if (!nodea->section && !nodeb->section)
		return 0;

	/* if only one has a section name, the blank one goes first */
	if (!nodea->section && nodeb->section)
		return -1;

	if (nodea->section && !nodeb->section)
		return 1;

	/* both have section names, but do they match */
	ret = strcmp(nodea->section, nodeb->section);

	/* section names differ, that was easy */
	if (ret != 0)
		return ret;

	/* sections matched, but how about sub-sections,
	 * again, if only one has a value the blank goes first
	 */
	if (!nodea->arg && nodeb->arg)
		return -1;

	if (nodea->arg && !nodeb->arg)
		return  1;

	/* both have sub-section args and they differ */
	if (nodea->arg && nodeb->arg
	&& (ret=strcmp(nodea->arg, nodeb->arg))!=0)
		return ret;

	return 0;
}

/* If a string starts or ends with a space it should be quoted */
static bool
should_escape(const char *text)
{
	int len;

	/* no string, no escaping needed */
	if (!text)
		return false;

	/* first character is a space */
	if (isspace(text[0]))
		return true;

	/* last character is a space */
	len = strlen(text);
	if (isspace(text[len-1]))
		return true;

	return false;
}

static void
conf_report_dump_text(struct dumper *head, FILE *ff)
{
	const struct dumper *node = head;
	const struct dumper *last = NULL;

	for (node=head; node!=NULL; node=node->next) {
		/* starting a new section, print the section header */
		if (dumper_section_compare(last, node)!=0) {
			if (node != head)
				fprintf(ff, "\n");
			if (node->arg)
				fprintf(ff, "[%s \"%s\"]\n", node->section, node->arg);
			else
				fprintf(ff, "[%s]\n", node->section);
		}

		/* now print the tag and its value */
		fprintf(ff, " %s", node->tag);
		if (node->value) {
			if (should_escape(node->value))
				fprintf(ff, " = \"%s\"", node->value);
			else
				fprintf(ff, " = %s", node->value);
		}
		fprintf(ff, "\n");

		last = node;
	}
}

/* sort by tag compare function */
static int
dumper_compare(const void *a, const void *b)
{
	const struct dumper *nodea = *(struct dumper **)a;
	const struct dumper *nodeb = *(struct dumper **)b;
	int ret;

	/* missing node, shouldnt happen */
	if (!nodea || !nodeb)
		return -1;

	/* are the two nodes in different (sub)sections */
	ret = dumper_section_compare(nodea, nodeb);
	if (ret != 0)
		return ret;

	/* sub-sections match (both blank, or both same)
	 * so we compare the tag names
	 */

	/* blank tags shouldnt happen, but paranoia */
	if (!nodea->tag && !nodeb->tag)
		return  0;

	/* still shouldnt happen, but use the blank-goes-first logic */
	if (!nodea->tag &&  nodeb->tag)
		return -1;
	if ( nodea->tag && !nodeb->tag)
		return  1;

	/* last test, compare the tags directly */
	ret = strcmp(nodea->tag, nodeb->tag);
	return ret;
}

/* sort all of the report nodes */
static struct dumper *
conf_report_sort(struct dumper *start)
{
	struct dumper **list;
	struct dumper *node;
	unsigned int count = 0;
	unsigned int i=0;

	/* how long is this list */
	for (node=start; node!=NULL; node=node->next)
		count++;

	/* no need to sort a list with less than 2 items */
	if (count < 2)
		return start;

	/* build an array of all the nodes */
	list = calloc(count, sizeof(struct dumper *));
	if (!list)
		goto mem_err;

	for (node=start,i=0; node!=NULL; node=node->next) {
		list[i++] = node;
	}

	/* sort the array alphabetically by section and tag */
	qsort(list, count, sizeof(struct dumper *), dumper_compare);

	/* rebuild the linked list in sorted order */
	for (i=0; i<count-1; i++) {
		list[i]->next = list[i+1];
	}
	list[count-1]->next = NULL;

	/* remember the new head of list and discard the sorting array */
	node = list[0];
	free(list);

	/* return the new head of list */
	return node;

mem_err:
	free(list);
	return NULL;
}

/* Output a copy of the current configuration to file */
void
conf_report(FILE *outfile)
{
	struct conf_binding *cb = NULL;
	unsigned int i;
	struct dumper *dumper = NULL, *dnode = NULL;

	xlog(LOG_INFO, "conf_report: dumping running configuration");

	/* build a linked list of all the config nodes */
	for (i = 0; i < sizeof conf_bindings / sizeof conf_bindings[0]; i++) {
		for (cb = LIST_FIRST(&conf_bindings[i]); cb; cb = LIST_NEXT(cb, link)) {
			struct dumper *newnode = calloc(1, sizeof (struct dumper));
			if (!newnode)
				goto mem_fail;

			newnode->next = dumper;
			dumper = newnode;

			newnode->section = cb->section;
			newnode->arg = cb->arg;
			newnode->tag = cb->tag;
			newnode->value = cb->value;
		}
	}

	/* sort the list then print it */
	dumper = conf_report_sort(dumper);
	conf_report_dump_text(dumper, outfile);
	goto cleanup;

mem_fail:
	xlog_warn("conf_report: malloc/calloc failed");
cleanup:
	/* traverse the linked list freeing all the nodes */
	while ((dnode = dumper) != 0) {
		dumper = dumper->next;
		free(dnode);
	}
	return;
}

/* struct and queue for buffing output lines */
TAILQ_HEAD(tailhead, outbuffer);

struct outbuffer {
	TAILQ_ENTRY(outbuffer) link;
	char *text;
};

static struct outbuffer *
make_outbuffer(char *line)
{
	struct outbuffer *new;

	if (line == NULL)
		return NULL;

	new = calloc(1, sizeof(struct outbuffer));
	if (new == NULL) {
		xlog(L_ERROR, "malloc error creating outbuffer");
		return NULL;
	}
	new->text = line;
	return new;
}

/* compose a properly escaped tag=value line */
static char *
make_tagline(const char *tag, const char *value)
{
	char *line;
	int ret;

	if (!value)
		return NULL;

	if (should_escape(value))
		ret = asprintf(&line, "%s = \"%s\"\n", tag, value);
	else
		ret = asprintf(&line, "%s = %s\n", tag, value);

	if (ret == -1) {
		xlog(L_ERROR, "malloc error composing a tag line");
		return NULL;
	}
	return line;
}

/* compose a section header line */
static char *
make_section(const char *section, const char *arg)
{
	char *line;
	int ret;

	if (arg)
		ret = asprintf(&line, "[%s \"%s\"]\n", section, arg);
	else
		ret = asprintf(&line, "[%s]\n", section);

	if (ret == -1) {
		xlog(L_ERROR, "malloc error composing section header");
		return NULL;
	}
	return line;
}

/* compose a comment line (with or without tag) */
static char *
make_comment(const char *tag, const char *comment)
{
	char *line;
	int ret;

	if (tag == NULL || *tag == '\0') {
		ret = asprintf(&line, "# %s\n", comment);
	} else {
		ret = asprintf(&line, "# %s: %s\n", tag, comment);
	}

	if (ret == -1) {
		xlog(L_ERROR, "malloc error composing header");
		return NULL;
	}

	return line;
}
		
/* compose a 'file modified' comment */
static char *
make_timestamp(const char *tag, time_t when)
{
	struct tm *tstamp;
	char datestr[80];
	char *result = NULL, *tmpstr = NULL;
	int ret;

	tstamp = localtime(&when);
	if (strftime(datestr, sizeof(datestr), "%b %d %Y %H:%M:%S", tstamp) == 0) {
		xlog(L_ERROR, "error composing date");
		datestr[0] = '\0';
	}

	if (modified_by) {
		ret = asprintf(&tmpstr, "%s on %s", modified_by, datestr);
		if (ret == -1) {
			xlog(L_ERROR, "malloc error composing a time stamp");
			return NULL;
		}
		result = make_comment(tag, tmpstr);
		free(tmpstr);
	} else {
		result = make_comment(tag, datestr);
	}
	return result;
}

/* does the supplied line contain the named section header */
static bool
is_section(const char *line, const char *section, const char *arg)
{
	char *end;
	char *name;
	char *sub;
	bool found = false;

	/* Not a valid section name  */
	if (strcmp(section, "#") == 0)
		return false;

	/* skip leading white space */
	while (*line == '[' || isspace(*line))
		line++;

	name = strdup(line);
	if (name == NULL) {
		xlog_warn("conf_write: malloc failed ");
		return false;
	}

	/* find the end */
	end = strchr(name, ']');

	/* malformed line */
	if (end == NULL) {
		xlog_warn("conf_write: warning: malformed section name");
		goto cleanup;
	}

	while (*end && ( *end == ']' || isblank(*end)))
		*(end--) = '\0';

	/* is there a subsection name (aka arg) */
	sub = strchr(name, '"');
	if (sub) {
		end = sub - 1;
		*(sub++) = '\0';

		/* trim whitespace between section name and arg */
		while (end > name && isblank(*end))
			*(end--) = '\0';

		/* trim off closing quote */
		end = strchr(sub, '"');
		if (end == NULL) {
			xlog_warn("conf_write: warning: malformed sub-section name");
			goto cleanup;
		}
		*end = '\0';
	}

	/* ready to compare */
	if (strcasecmp(section, name)!=0)
		goto cleanup;

	if (arg != NULL) {
		if (sub == NULL || strcasecmp(arg, sub)!=0)
			goto cleanup;
	} else {
		if (sub != NULL)
			goto cleanup;
	}

	found = true;

cleanup:
	free(name);
	return found;
}

/* check that line contains the specified tag assignment */
static bool
is_tag(const char *line, const char *tagname)
{
	char *end;
	char *name;
	bool found = false;

	/* quick check, is this even an assignment line */
	end = strchr(line, '=');
	if (end == NULL)
		return false;

	/* skip leading white space before tag name */
	while (isblank(*line))
		line++;

	name = strdup(line);
	if (name == NULL) {
		xlog_warn("conf_write: malloc failed");
		return false;
	}

	/* trim any newline characters */
	end = strchr(name, '\n');
	if (end)
		*end = '\0';
	end = strchr(name, '\r');
	if (end)
		*end = '\0';

	/* find the assignment equals sign */
	end = strchr(name, '=');

	/* malformed line, i swear the equals was there earlier */
	if (end == NULL) {
		xlog_warn("conf_write: warning: malformed tag name");
		goto cleanup;
	}

	/* trim trailing whitespace after tag name */
	do {
		*(end--) = '\0';
	}while (end > name && *end && isblank(*end));

	/* quoted string, take contents of quotes only */
	if (*name == '"') {
		char * new = strdup(name+1);
		end = strchr(new, '"');
		if (end != NULL) {
			*end = 0;
			free(name);
			name = new;
		} else {
			free(new);
		}
	}

	/* now compare */
	if (strcasecmp(tagname, name) == 0)
		found = true;

cleanup:
	free(name);
	return found;
}

/* is this an empty line ? */
static bool
is_empty(const char *line)
{
	const char *p = line;

	if (line == NULL)
		return true;
	if (*line == '\0')
		return true;

	while (*p != '\0' && isspace(*p))
		p++;

	if (*p == '\0')
		return true;

	return false;
}

/* is this line just a comment ? */
static bool
is_comment(const char *line)
{
	if (line == NULL)
		return false;

	while (isblank(*line))
		line++;

	if (*line == '#')
		return true;

	return false;
}

/* check that line contains the specified comment header */
static bool
is_taggedcomment(const char *line, const char *field)
{
	char *end;
	char *name;
	bool found = false;

	if (line == NULL)
		return false;

	while (isblank(*line))
		line++;

	if (*line != '#')
		return false;

	line++;

	/* quick check, is this even a likely formatted line */
	end = strchr(line, ':');
	if (end == NULL)
		return false;

	/* skip leading white space before field name */
	while (isblank(*line))
		line++;

	name = strdup(line);
	if (name == NULL) {
		xlog_warn("conf_write: malloc failed");
		return false;
	}

	/* strip trailing spaces from the name */
	end = strchr(name, ':');
	if (end) *(end--) = 0;
	while (end && end > name && isblank(*end))
		*(end--)=0;

	if (strcasecmp(name, field)==0) 
		found = true;

	free(name);
	return found;
}


/* delete a buffer queue whilst optionally outputting to file */
static int
flush_outqueue(struct tailhead *queue, FILE *fout)
{
	int ret = 0;
	while (queue->tqh_first != NULL) {
		struct outbuffer *ob = queue->tqh_first;
		TAILQ_REMOVE(queue, ob, link);
		if (ob->text) {
			if (fout) {
				ret = fprintf(fout, "%s", ob->text);
				if (ret == -1) {
					xlog(L_ERROR, "Error writing to config file: %s",
						 strerror(errno));
					fout = NULL;
				}
			}
			free(ob->text);
		}
		free(ob);
	}
	if (ret == -1)
		return 1;
	return 0;
}

/* append one queue to another */
static void
append_queue(struct tailhead *inq, struct tailhead *outq)
{
	while (inq->tqh_first != NULL) {
		struct outbuffer *ob = inq->tqh_first;
		TAILQ_REMOVE(inq, ob, link);
		TAILQ_INSERT_TAIL(outq, ob, link);
	}
}

/* read one line of text from a file, growing the buffer as necessary */
static int
read_line(char **buff, int *buffsize, FILE *in)
{
	char *readp;
	int used = 0;
	bool again = false;

	/* make sure we have a buffer to read into */
	if (*buff == NULL) {
		*buffsize = 4096;
		*buff = calloc(1, *buffsize);
		if (*buff == NULL) {
			xlog(L_ERROR, "malloc error for read buffer");
			return -1;
		}
	}

	readp = *buff;

	do {
		int len;

		/* read in a chunk */
		if (fgets(readp, *buffsize-used, in)==NULL)
			return -1;

		len = strlen(*buff);
		if (len == 0)
			return -1;

		/* was this the end of a line, or partial read */
		readp = *buff + len - 1;

		if (*readp != '\n' && *readp !='\r') {
			/* no nl/cr must be partial read, go again */
			readp++;
			again = true;
		} else {
			/* that was a normal end of line */
			again = false;
		}

		/* do we need more space */
		if (again && *buffsize - len < 1024) {
			int offset = readp - *buff;
			char *newbuff;
			*buffsize += 4096;
			newbuff = realloc(*buff, *buffsize);
			if (newbuff == NULL) {
				xlog(L_ERROR, "malloc error reading line");
				return -1;
			}
			*buff = newbuff;
			readp = newbuff + offset;
		}
	} while(again);
	return 0;
}

/* append a line to the given location in the queue */
static int
append_line(struct tailhead *queue, struct outbuffer *entry, char *line)
{
	int ret = 0;
	char *end;
	bool splitmode = false;
	char *start = line;

	if (line == NULL)
		return -1;

	/* if there are \n's in the middle of the string
	 * then we need to split it into folded lines */
	do {
		char *thisline;
		struct outbuffer *qbuff;

		end = strchr(start, '\n');
		if (end && *(end+1) != '\0') {
			*end = '\0';

			ret = asprintf(&thisline, "%s\\\n", start);
			if (ret == -1) {
				xlog(L_ERROR, "malloc error composing output");
				return -1;
			}
			splitmode = true;
			start = end+1;
		} else {
			end = NULL;
			if (splitmode) {
				thisline = strdup(start);
				if (thisline == NULL)
					return -1;
			} else {
				thisline = start;
			}
		}

		qbuff = make_outbuffer(thisline);
		if (qbuff == NULL)
			return -1;

		if (entry) {
			TAILQ_INSERT_AFTER(queue, entry, qbuff, link);
			entry = TAILQ_NEXT(entry, link);
		} else {
			TAILQ_INSERT_TAIL(queue, qbuff, link);
		}
	}while (end != NULL);

	/* we malloced copies of this, so free the original */
	if (splitmode)
		free(line);

	return 0;
}

/* is this a "folded" line, i.e. ends in backslash */
static bool
is_folded(const char *line)
{
	const char *end;
	if (line == NULL)
		return false;

	end = line + strlen(line);
	while (end > line) {
		end--;
		if (*end != '\n' && *end != '\r')
			break;
	}

	if (*end == '\\')
		return true;

	return false;
}

static int
lock_file(FILE *f)
{
	int ret;
	ret = flock(fileno(f), LOCK_EX);
	if (ret) 
		xlog(L_ERROR, "Error could not lock the file");
	return ret;
}

/***
 * Write a value to an nfs.conf style filename
 *
 * create the file if it doesnt already exist
 * if value==NULL removes the setting (if present)
 */
int
conf_write(const char *filename, const char *section, const char *arg,
	   const char *tag, const char *value)
{
	FILE *infile = NULL;
	int ret = 1;
	struct tailhead outqueue;
	struct tailhead inqueue;
	char * buff = NULL;
	int buffsize = 0;
	time_t now = time(NULL);

	TAILQ_INIT(&inqueue);
	TAILQ_INIT(&outqueue);

	if (!filename) {
		xlog_warn("conf_write: no filename supplied");
		return ret;
	}

	if (!section || !tag) {
		xlog_warn("conf_write: section or tag name missing");
		return ret;
	}

	infile = fopen(filename, "r+");
	if (!infile) {
		if (!value) {
			xlog_warn("conf_write: config file \"%s\" not found, nothing to do", filename);
			ret = 0;
			goto cleanup;
		}

		xlog_warn("conf_write: config file \"%s\" not found, creating.", filename);
		infile = fopen(filename, "wx");
		if (!infile) {
			xlog(L_ERROR, "conf_write: Error creating config file \"%s\".", filename);
			goto cleanup;
		}

		if (lock_file(infile))
			goto cleanup;

		if (strcmp(section, "#") == 0) {
			if (append_line(&inqueue, NULL, make_comment(tag, value)))
				goto cleanup;
		} else {
			if (append_line(&inqueue, NULL, make_section(section, arg)))
				goto cleanup;

			if (append_line(&inqueue, NULL, make_tagline(tag, value)))
				goto cleanup;
		}

		append_queue(&inqueue, &outqueue);
	} else 
	if (strcmp(section, "#") == 0) {
		/* Adding a comment line */
		struct outbuffer *where = NULL;
		struct outbuffer *next = NULL;
		bool found = false;
		int err = 0;

		if (lock_file(infile))
			goto cleanup;

		buffsize = 4096;
		buff = calloc(1, buffsize);
		if (buff == NULL) {
			xlog(L_ERROR, "malloc error for read buffer");
			goto cleanup;
		}
		buff[0] = '\0';

		/* read in the file */
		do {
			if (*buff != '\0' 
			&& !is_taggedcomment(buff, "Modified")) {
				if (append_line(&inqueue, NULL, strdup(buff)))
					goto cleanup;
			}

			err = read_line(&buff, &buffsize, infile);
		} while (err == 0);

		/* if a tagged comment, look for an existing instance */
		if (tag && *tag != '\0') {
			where = TAILQ_FIRST(&inqueue);
			while (where != NULL) {
				next = TAILQ_NEXT(where, link);
				struct outbuffer *prev = TAILQ_PREV(where, tailhead, link);
				if (is_taggedcomment(where->text, tag)) {
					TAILQ_REMOVE(&inqueue, where, link);
					free(where->text);
					free(where);
					found = true;
					if (append_line(&inqueue, prev, make_comment(tag, value)))
						goto cleanup;
				}
				where = next;
			}
		}
		/* it wasn't tagged or we didn't find it */
		if (!found) {
			/* does the file end in a blank line or a comment */
			if (!TAILQ_EMPTY(&inqueue)) {
				struct outbuffer *tail = TAILQ_LAST(&inqueue, tailhead);
				if (tail && !is_empty(tail->text) && !is_comment(tail->text)) {
					/* no, so add one for clarity */
					if (append_line(&inqueue, NULL, strdup("\n")))
						goto cleanup;
				}
			}
			/* add the new comment line */
			if (append_line(&inqueue, NULL, make_comment(tag, value)))
				goto cleanup;
		}
		/* move everything over to the outqueue for writing */
		append_queue(&inqueue, &outqueue);
	} else {
		bool found = false;
		int err = 0;

		if (lock_file(infile))
			goto cleanup;

		buffsize = 4096;
		buff = calloc(1, buffsize);
		if (buff == NULL) {
			xlog(L_ERROR, "malloc error for read buffer");
			goto cleanup;
		}

		buff[0] = '\0';
		do {
			struct outbuffer *where = NULL;

			/* read in one section worth of lines */
			do {
				if (*buff != '\0' 
				&& !is_taggedcomment(buff, "Modified")) {
					if (append_line(&inqueue, NULL, strdup(buff)))
						goto cleanup;
				}

				err = read_line(&buff, &buffsize, infile);
			} while (err == 0 && buff[0] != '[');

			/* find the section header */
			where = TAILQ_FIRST(&inqueue);
			while (where != NULL) {
				if (where->text != NULL && where->text[0] == '[')
					break;
				where = TAILQ_NEXT(where, link);
			}

			/* this is the section we care about */
			if (where != NULL && is_section(where->text, section, arg)) {
				struct outbuffer *section_start = where;

				/* is there an existing assignment */
				while ((where = TAILQ_NEXT(where, link)) != NULL) {
					if (is_tag(where->text, tag)) {
						found = true;
						break;
					}
				}

				/* no active assignment, but is there a commented one */
				if (!found) {
					where = section_start;
					while ((where = TAILQ_NEXT(where, link)) != NULL) {
						if (is_comment(where->text)) {
							char *cline = where->text;
							while (isspace(*cline)) 
								cline++;

							if (*cline != '#') 
								continue;
							cline++;

							if (is_tag(cline, tag)) {
								found = true;
								break;
							}
						}
					}
				}

				/* replace the located tag with an updated one */
				if (found) {
					struct outbuffer *prev = TAILQ_PREV(where, tailhead, link);
					bool again = false;

					/* remove current tag */
					do {
						struct outbuffer *next = TAILQ_NEXT(where, link);
						TAILQ_REMOVE(&inqueue, where, link);
						if (is_folded(where->text))
							again = true;
						else
							again = false;
						free(where->text);
						free(where);
						where = next;
					} while(again && where != NULL);

					/* insert new tag */
					if (value) {
						if (append_line(&inqueue, prev, make_tagline(tag, value)))
							goto cleanup;
					}
				} else
				/* no existing assignment found and we need to add one */
				if (value) {
					/* rewind past blank lines and comments */
					struct outbuffer *tail = TAILQ_LAST(&inqueue, tailhead);

					/* comments immediately before a section usually relate
					 * to the section below them */
					while (tail != NULL && is_comment(tail->text))
						tail = TAILQ_PREV(tail, tailhead, link);

					/* there is usually blank line(s) between sections */
					while (tail != NULL && is_empty(tail->text))
						tail = TAILQ_PREV(tail, tailhead, link);

					/* now add the tag here */
					if (append_line(&inqueue, tail, make_tagline(tag, value)))
						goto cleanup;

					found = true;
				}
			}

			/* EOF and correct section not found, so add one */
			if (err && !found && value) {
				/* did the last section end in a blank line */
				struct outbuffer *tail = TAILQ_LAST(&inqueue, tailhead);
				if (tail && !is_empty(tail->text)) {
					/* no, so add one for clarity */
					if (append_line(&inqueue, NULL, strdup("\n")))
						goto cleanup;
				}

				/* add the new section header */
				if (append_line(&inqueue, NULL, make_section(section, arg)))
					goto cleanup;

				/* now add the tag */
				if (append_line(&inqueue, NULL, make_tagline(tag, value)))
					goto cleanup;
			}

			/* we are done with this section, move it to the out queue */
			append_queue(&inqueue, &outqueue);
		} while(err == 0);
	}

	if (modified_by) {
		/* check for and update the Modified header */
		/* does the file end in a blank line or a comment */
		if (!TAILQ_EMPTY(&outqueue)) {
			struct outbuffer *tail = TAILQ_LAST(&outqueue, tailhead);
			if (tail && !is_empty(tail->text) && !is_comment(tail->text)) {
				/* no, so add one for clarity */
				if (append_line(&outqueue, NULL, strdup("\n")))
					goto cleanup;
			}
		}

		/* now append the modified date comment */
		if (append_line(&outqueue, NULL, make_timestamp("Modified", now)))
			goto cleanup;
	}

	/* now rewind and overwrite the file with the updated data */
	rewind(infile);

	if (ftruncate(fileno(infile), 0)) {
		xlog(L_ERROR, "Error truncating config file");
		goto cleanup;
	}

	if (flush_outqueue(&outqueue, infile))
		goto cleanup;

	if (infile) {
		fclose(infile);
		infile = NULL;
	}

	ret = 0;

cleanup:
	flush_outqueue(&inqueue, NULL);
	flush_outqueue(&outqueue, NULL);

	if (buff)
		free(buff);
	if (infile)
		fclose(infile);
	return ret;
}
