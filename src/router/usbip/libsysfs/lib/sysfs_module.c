/*
 * sysfs_module.c
 *
 * Generic module utility functions for libsysfs
 *
 * Copyright (C) IBM Corp. 2003-2005
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */
#include "libsysfs.h"
#include "sysfs.h"

/**
 * mod_name_equal: compares modules' name
 * @a: module_name looking for
 * @b: sysfs_module being compared
 */
static int mod_name_equal(void *a, void *b)
{
	if (a == NULL || b == NULL)
		return 0;

	if (strcmp(((char *)a), ((struct sysfs_module *)b)->name) == 0)
		return 1;

	return 0;
}

/**
 * sysfs_close_module: closes a module.
 * @module: sysfs_module device to close.
 */
void sysfs_close_module(struct sysfs_module *module)
{
	/*
	 * since both parms and sections are attribs _under_ the
	 * subdir of module->directory, they will get closed by
	 * this single call
	 */
	if (module != NULL) {
		if (module->attrlist != NULL)
			dlist_destroy(module->attrlist);
		if (module->parmlist != NULL)
			dlist_destroy(module->parmlist);
		if (module->sections != NULL)
			dlist_destroy(module->sections);
		free(module);
	}
}

/**
 * alloc_module: callocs and initializes new module struct.
 * returns sysfs_module or NULL.
 */
static struct sysfs_module *alloc_module(void)
{
	return (struct sysfs_module *)calloc(1, sizeof(struct sysfs_module));
}

/**
 * sysfs_open_module_path: Opens and populates the module struct
 * @path: path to module.
 * returns struct sysfs_module with success and NULL with error.
 */
struct sysfs_module *sysfs_open_module_path(const char *path)
{
	struct sysfs_module *mod = NULL;

	if (path == NULL) {
		errno = EINVAL;
		return NULL;
	}
	if ((sysfs_path_is_dir(path)) != 0) {
		dprintf("%s is not a valid path to a module\n", path);
		return NULL;
	}
	mod = alloc_module();
	if (mod == NULL) {
		dprintf("calloc failed\n");
		return NULL;
	}
	if ((sysfs_get_name_from_path(path, mod->name, SYSFS_NAME_LEN)) != 0) {
		errno = EINVAL;
		dprintf("Error getting module name\n");
		sysfs_close_module(mod);
		return NULL;
	}

	safestrcpy(mod->path, path);
	if ((sysfs_remove_trailing_slash(mod->path)) != 0) {
		dprintf("Invalid path to module %s\n", mod->path);
		sysfs_close_module(mod);
		return NULL;
	}

	return mod;
}

/**
 * sysfs_open_module: opens specific module on a system
 * returns sysfs_module structure with success or NULL with error.
 */
struct sysfs_module *sysfs_open_module(const char *name)
{
	struct sysfs_module *mod = NULL;
	char modpath[SYSFS_PATH_MAX];

	if (name == NULL) {
		errno = EINVAL;
		return NULL;
	}

	memset(modpath, 0, SYSFS_PATH_MAX);
	if ((sysfs_get_mnt_path(modpath, SYSFS_PATH_MAX)) != 0) {
		dprintf("Sysfs not supported on this system\n");
		return NULL;
	}

	safestrcat(modpath, "/");
	safestrcat(modpath, SYSFS_MODULE_NAME);
	safestrcat(modpath, "/");
	safestrcat(modpath, name);

	if ((sysfs_path_is_dir(modpath)) != 0) {
		dprintf("Module %s not found on the system\n", name);
		return NULL;
	}

	mod = alloc_module();
	if (mod == NULL) {
		dprintf("calloc failed\n");
		return NULL;
	}
	safestrcpy(mod->name, name);
	safestrcpy(mod->path, modpath);
	if ((sysfs_remove_trailing_slash(mod->path)) != 0) {
		dprintf("Invalid path to module %s\n", mod->path);
		sysfs_close_module(mod);
		return NULL;
	}

	return mod;
}

/**
 * sysfs_get_module_attributes: returns a dlist of attributes for
 *     the requested sysfs_module
 * @cdev: sysfs_module for which attributes are needed
 * returns a dlist of attributes if exists, NULL otherwise
 */
struct dlist *sysfs_get_module_attributes(struct sysfs_module *module)
{
	if (module == NULL) {
		errno = EINVAL;
		return NULL;
	}
	return get_dev_attributes_list(module);
}

/**
 * sysfs_get_module_attr: searches module's attributes by name
 * @module: module to look through
 * @name: attribute name to get
 * returns sysfs_attribute reference with success or NULL with error
 */
struct sysfs_attribute *sysfs_get_module_attr
		(struct sysfs_module *module, const char *name)
{
	if (module == NULL || name == NULL) {
		errno = EINVAL;
		return NULL;
	}

	return get_attribute(module, (char *)name);
}

/**
 * sysfs_get_module_parms: Get modules list of parameters
 * @module: sysfs_module whose parmameter list is required
 * Returns dlist of parameters on SUCCESS and NULL on error
 */
struct dlist *sysfs_get_module_parms(struct sysfs_module *module)
{
	char ppath[SYSFS_PATH_MAX];

	if (module == NULL) {
		errno = EINVAL;
		return NULL;
	}
	memset(ppath, 0, SYSFS_PATH_MAX);
	safestrcpy(ppath, module->path);
	safestrcat(ppath,"/");
	safestrcat(ppath, SYSFS_MOD_PARM_NAME);

	return (get_attributes_list(module->parmlist, ppath));
}

/**
 * sysfs_get_module_sections: Get the set of sections for this module
 * @module: sysfs_module whose list of sections is required
 * Returns dlist of sections on SUCCESS and NULL on error
 */
struct dlist *sysfs_get_module_sections(struct sysfs_module *module)
{
	char ppath[SYSFS_PATH_MAX];

	if (module == NULL) {
		errno = EINVAL;
		return NULL;
	}

	memset(ppath, 0, SYSFS_PATH_MAX);
	safestrcpy(ppath, module->path);
	safestrcat(ppath,"/");
	safestrcat(ppath, SYSFS_MOD_SECT_NAME);

	return (get_attributes_list(module->sections, ppath));
}

/**
 * sysfs_get_module_parm:
 * @module: sysfs_module to look through
 * @parm: name of the parameter to look for
 * Returns sysfs_attribute * on SUCCESS and NULL on error
 */
struct sysfs_attribute *sysfs_get_module_parm
		(struct sysfs_module *module, const char *parm)
{
	struct dlist *parm_list = NULL;

	if (module == NULL || parm == NULL) {
		errno = EINVAL;
		return NULL;
	}

	parm_list = sysfs_get_module_parms(module);
	if (parm_list == NULL)
		return NULL;

	return (struct sysfs_attribute *)dlist_find_custom(parm_list,
		(void *)parm, mod_name_equal);
}

/**
 * sysfs_get_module_section
 * @module: sysfs_module to look through
 * @section: name of the section to look for
 * Returns sysfs_attribute * on SUCCESS and NULL on error
 */
struct sysfs_attribute *sysfs_get_module_section
		(struct sysfs_module *module, const char *section)
{
	struct dlist *sect_list = NULL;

	if (module == NULL || section == NULL) {
		errno = EINVAL;
		return NULL;
	}

	sect_list = sysfs_get_module_sections(module);
	if (sect_list == NULL)
		return NULL;

	return (struct sysfs_attribute *)dlist_find_custom(sect_list,
		(void *)section, mod_name_equal);
}
