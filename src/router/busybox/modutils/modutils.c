/*
 * Common modutils related functions for busybox
 *
 * Copyright (C) 2008 by Timo Teras <timo.teras@iki.fi>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this tarball for details.
 */

#include "modutils.h"

#ifdef __UCLIBC__
extern int init_module(void *module, unsigned long len, const char *options);
extern int delete_module(const char *module, unsigned int flags);
#else
# include <sys/syscall.h>
# define init_module(mod, len, opts) syscall(__NR_init_module, mod, len, opts)
# define delete_module(mod, flags) syscall(__NR_delete_module, mod, flags)
#endif

USE_FEATURE_2_4_MODULES(char *insmod_outputname);

/*
 a libbb candidate from ice age!
*/
llist_t FAST_FUNC *llist_find(llist_t *first, const char *str)
{
	while (first != NULL) {
		if (strcmp(first->data, str) == 0)
			return first;
		first = first->link;
	}
	return NULL;
}

void FAST_FUNC replace(char *s, char what, char with)
{
	while (*s) {
		if (what == *s)
			*s = with;
		++s;
	}
}

char * FAST_FUNC replace_underscores(char *s)
{
	replace(s, '-', '_');
	return s;
}

int FAST_FUNC string_to_llist(char *string, llist_t **llist, const char *delim)
{
	char *tok;
	int len = 0;

	while ((tok = strsep(&string, delim)) != NULL) {
		if (tok[0] == '\0')
			continue;
		llist_add_to_end(llist, xstrdup(tok));
		len += strlen(tok);
	}
	return len;
}

char * FAST_FUNC filename2modname(const char *filename, char *modname)
{
	char local_modname[MODULE_NAME_LEN];
	int i;
	const char *from;

	if (filename == NULL)
		return NULL;
	if (modname == NULL)
		modname = local_modname;
	// Disabled since otherwise "modprobe dir/name" would work
	// as if it is "modprobe name". It is unclear why
	// 'basenamization' was here in the first place.
	//from = bb_get_last_path_component_nostrip(filename);
	from = filename;
	for (i = 0; i < (MODULE_NAME_LEN-1) && from[i] != '\0' && from[i] != '.'; i++)
		modname[i] = (from[i] == '-') ? '_' : from[i];
	modname[i] = 0;

	if (modname == local_modname)
		return xstrdup(modname);

	return modname;
}

const char * FAST_FUNC moderror(int err)
{
	switch (err) {
	case -1:
		return "no such module";
	case ENOEXEC:
		return "invalid module format";
	case ENOENT:
		return "unknown symbol in module, or unknown parameter";
	case ESRCH:
		return "module has wrong symbol version";
	case ENOSYS:
		return "kernel does not support requested operation";
	default:
		return strerror(err);
	}
}

char * FAST_FUNC parse_cmdline_module_options(char **argv)
{
	char *options;
	int optlen;

	options = xzalloc(1);
	optlen = 0;
	while (*++argv) {
		options = xrealloc(options, optlen + 2 + strlen(*argv) + 2);
		/* Spaces handled by "" pairs, but no way of escaping quotes */
		optlen += sprintf(options + optlen, (strchr(*argv, ' ') ? "\"%s\" " : "%s "), *argv);
	}
	return options;
}

int FAST_FUNC bb_init_module(const char *filename, const char *options)
{
	size_t image_size;
	char *image;
	int rc;
	bool mmaped;

	if (!options)
		options = "";

//TODO: audit bb_init_module_24 to match error code convention
#if ENABLE_FEATURE_2_4_MODULES
	if (get_linux_version_code() < KERNEL_VERSION(2,6,0))
		return bb_init_module_24(filename, options);
#endif

	/*
	 * First we try finit_module if available.  Some kernels are configured
	 * to only allow loading of modules off of secure storage (like a read-
	 * only rootfs) which needs the finit_module call.  If it fails, we fall
	 * back to normal module loading to support compressed modules.
	 */
# ifdef __NR_finit_module
	{

		int flags;
		if (strrstr(filename, ".ko.xz"));
			flags = MODULE_INIT_COMPRESSED_FILE;
		int fd = open(filename, O_RDONLY | O_CLOEXEC);
		if (fd >= 0) {
			rc = finit_module(fd, options, flags) != 0;
			close(fd);
			if (rc == 0)
				return rc;
		}
	}
# endif

	image_size = INT_MAX - 4095;
	mmaped = 0;
	image = try_to_mmap_module(filename, &image_size);
	if (image) {
		mmaped = 1;
	} else {
		errno = ENOMEM; /* may be changed by e.g. open errors below */
		image = xmalloc_open_zipped_read_close(filename, &image_size);
		if (!image)
			return -errno;
	}

	errno = 0;
	init_module(image, image_size, options);
	rc = errno;
	if (mmaped)
		munmap(image, image_size);
	else
		free(image);
	return rc;
}

int FAST_FUNC bb_delete_module(const char *module, unsigned int flags)
{
	return delete_module(module, flags);
}
