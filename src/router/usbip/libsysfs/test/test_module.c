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

/**
 ***************************************************************************
 * this will test the module related functions provided by libsysfs.
 *
 * extern void sysfs_close_module(struct sysfs_module *module);
 * extern struct sysfs_module *sysfs_open_module_path(const char *path);
 * extern struct sysfs_module *sysfs_open_module(const char *name);
 * extern struct dlist *sysfs_get_module_attributes
 *	(struct sysfs_module *module);
 * extern struct sysfs_attribute *sysfs_get_module_attr
 *              (struct sysfs_module *module, const char *name);
 * extern struct dlist *sysfs_get_module_parms(struct sysfs_module *module);
 * extern struct dlist *sysfs_get_module_sections(struct sysfs_module *module);
 * extern struct sysfs_attribute *sysfs_get_module_parm
 *        (struct sysfs_module *module, const char *parm);
 * extern struct sysfs_attribute *sysfs_get_module_section
 *        (struct sysfs_module *module, const char *section);
 */

#include "test-defs.h"
#include <errno.h>

/**
 * sysfs_close_module: closes a module.
 * @module: sysfs_module device to close.
 * flag:
 * 	0: path -> valid
 *	1: path -> NULL
 */
int test_sysfs_close_module(int flag)
{
	struct sysfs_module *module;

	switch (flag) {
	case 0:
		module = sysfs_open_module_path(val_mod_path);
		if (module == NULL)
			return 0;
		break;
	case 1:
		module = NULL;
		break;
	default:
		return -1;
	}
	sysfs_close_module(module);

	dbg_print("%s: returns void\n",__FUNCTION__);
	return 0;
}

/**
 * sysfs_open_module_path: Opens and populates the module struct
 * @path: path to module.
 * returns struct sysfs_module with success and NULL with error.
 *
 * flag:
 *	0: path -> valid
 *	1: path -> invalid
 *	2: path -> NULL
 */
int test_sysfs_open_module_path(int flag)
{
	struct sysfs_module *module = NULL;
	char *path = NULL;

	switch (flag) {
	case 0:
		path = val_mod_path;
		break;
	case 1:
		path = inval_path;
		break;
	case 2:
		path = NULL;
		break;
	default:
		return -1;
	}

	module = sysfs_open_module_path(path);

	switch (flag) {
	case 0:
		if (module == NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_module(module);
			dbg_print("\n");
		}
		break;
	case 1:
	case 2:
		if (module != NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
	default:
		break;
	}

	if (module!= NULL)
		sysfs_close_module(module);

	return 0;
}

/**
 * sysfs_open_module: opens specific module on a system
 * returns sysfs_module structure with success or NULL with error.
 * flag:
 *	0:	name -> valid
 *	1:	name -> invalid
 *	2:	name -> NULL
 */
int test_sysfs_open_module(int flag)
{
	struct sysfs_module *module = NULL;
	char *modname = NULL;

	switch (flag) {
	case 0:
		modname = val_mod_name;
		break;
	case 1:
		modname = inval_name;
		break;
	case 2:
		modname = NULL;
		break;
	default:
		return -1;
	}

	module = sysfs_open_module(modname);

	switch (flag) {
	case 0:
		if (module == NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_module(module);
			dbg_print("\n");
		}
		break;
	case 1:
	case 2:
		if (module)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
					__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
						__FUNCTION__, flag);
		break;
	default:
		break;
	}

	if (module)
		sysfs_close_module(module);
	return 0;
}

/**
 * sysfs_get_module_attr: searches module's attributes by name
 * @module: module to look through
 * @name: attribute name to get
 * returns sysfs_attribute reference with success or NULL with error
 * flag:
 * 	0:	name -> valid, attrname -> valid
 * 	1:	name -> valid, attrname -> invalid
 * 	2:	name -> valid, attrname -> NULL
 */
int test_sysfs_get_module_attr(int flag)
{
	char *name, *attrname;
	struct sysfs_attribute *attr;
	struct sysfs_module *module;

	switch (flag) {
	case 0:
		name = val_mod_path;
		module = sysfs_open_module_path(name);
		if (!module) {
			dbg_print("%s: failed opening module at %s\n",
						__FUNCTION__, name);
			return 0;
		}
		attrname = val_mod_attr_name;
		break;
	case 1:
		name = val_mod_path;
		module = sysfs_open_module_path(name);
		if (!module) {
			dbg_print("%s: failed opening module at %s\n",
						__FUNCTION__, name);
			return 0;
		}
		attrname = inval_name;
		break;
	case 2:
		name = val_mod_path;
		module = sysfs_open_module_path(name);
		if (!module) {
			dbg_print("%s: failed opening module at %s\n",
						__FUNCTION__, name);
			return 0;
		}
		attrname = NULL;
		break;
	default:
		return -1;
	}

	attr = sysfs_get_module_attr(module,attrname);

	switch (flag) {
	case 0:
		if(!attr) {
			if (errno == EACCES)
				dbg_print("%s: attribute %s does not support "
					"READ\n", __FUNCTION__, attrname);
			else if (errno == ENOENT)
				dbg_print("%s: attribute %s not defined for "
					"module at %s\n", __FUNCTION__,
							attrname, name);
			else if (errno == 0)
				dbg_print("%s: module at %s does not export "
				"attributes\n", __FUNCTION__, val_drv_path);
			else
				dbg_print("%s: FAILED with flag = %d errno = "
					"%d\n", __FUNCTION__, flag, errno);
		} else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
					__FUNCTION__, flag);
			show_attribute(attr);
			dbg_print("\n");
		}
		break;

	case 1:
	case 2:
		if (attr)
			dbg_print("%s: FAILED with flag = %d errno = "
				"%d\n", __FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
							__FUNCTION__, flag);
		dbg_print("******************\n");
	default:
		break;
	}

	if (module)
		sysfs_close_module(module);
	return 0;
}

/**
 * sysfs_get_module_attributes: returns a dlist of attributes for
 *     the requested sysfs_module
 * @cdev: sysfs_module for which attributes are needed
 * returns a dlist of attributes if exists, NULL otherwise
 * flag:
 * 	0:	name -> valid
 * 	1:	name -> invalid
 * 	2:	name -> NULL
 */
int test_sysfs_get_module_attributes(int flag)
{
	struct dlist *list;
	struct sysfs_module *module;
	char *name;

	switch (flag) {
	case 0:
		name = val_mod_path;
		module = sysfs_open_module_path(name);
		if (!module) {
			dbg_print("%s: failed opening module at %s\n",
						__FUNCTION__, name);
			return 0;
		}
		break;
	case 1:
		name = inval_name;
		module = sysfs_open_module_path(name);
		if (!module) {
			return 0;
		}
		break;
	case 2:
		name = NULL;
		module = sysfs_open_module_path(name);
		if (!module) {
			return 0;
		}
	default:
		return -1;
	}

	list = sysfs_get_module_attributes(module);

	switch (flag) {
	case 0:
		if (!list) {
			if (errno == 0)
				dbg_print("%s: No attributes are defined for "
				"the module at %s\n", __FUNCTION__, name);
			else
				dbg_print("%s: FAILED with flag = %d errno = "
					"%d\n", __FUNCTION__, flag, errno);
		} else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_attribute_list(list);
			dbg_print("\n");
		}
		break;
	case 1:
	case 2:
		if (list != NULL)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
							__FUNCTION__, flag);
		break;
	default:
		break;
	}

	if (module)
		sysfs_close_module(module);
	return 0;
}

/**
 * sysfs_get_module_parms: Get modules list of parameters
 * @module: sysfs_module whose parmameter list is required
 * Returns dlist of parameters on SUCCESS and NULL on error
 * flag:
 * 	0:	name -> valid
 * 	1:	name -> invalid
 * 	2:	name -> NULL
 */
int test_sysfs_get_module_parms(int flag)
{
	struct dlist *params;
	char *name;
	struct sysfs_module *module;
	switch (flag) {
	case 0:
		name = val_mod_path;
		module = sysfs_open_module_path(name);
		if (!module) {
			dbg_print("%s: failed opening module at %s\n",
						__FUNCTION__, name);
			return 0;
		}
		break;
	case 1:
		name = inval_name;
		module = sysfs_open_module_path(name);
		if (!module) {
			return 0;
		}
		break;
	case 2:
		name = NULL;
		module = sysfs_open_module_path(name);
		if (!module) {
			return 0;
		}
		break;
	default:
		return -1;
	}

	params = sysfs_get_module_parms(module);

	switch (flag) {
	case 0:
		if (!params) {
			if (errno == 0)
				dbg_print("%s: No parameters are passed for "
				"the module at %s\n", __FUNCTION__, name);
			else
				dbg_print("%s: FAILED with flag = %d errno = "
					"%d\n", __FUNCTION__, flag, errno);
		} else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_parm_list(params);
			dbg_print("\n");
		}
		break;
	case 1:
	case 2:
		if (params)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
							__FUNCTION__, flag);
		break;
	default:
		break;
	}

	if (module)
		sysfs_close_module(module);
	return 0;
}

/**
 * sysfs_get_module_sections: Get the set of sections for this module
 * @module: sysfs_module whose list of sections is required
 * Returns dlist of sections on SUCCESS and NULL on error
 * 	0:	name -> valid
 * 	1:	name -> invalid
 * 	2:	name -> NULL
 */
int test_sysfs_get_module_sections(int flag)
{
	struct dlist *sections;
	char *name;
	struct sysfs_module *module;

	switch (flag) {
	case 0:
		name = val_mod_path;
		module = sysfs_open_module_path(name);
		if (!module) {
			dbg_print("%s: failed opening module at %s\n",
						__FUNCTION__, name);
			return 0;
		}
		break;
	case 1:
		name = inval_name;
		module = sysfs_open_module_path(name);
		if (!module) {
			return 0;
		}
		break;
	case 2:
		name = NULL;
		module = sysfs_open_module_path(name);
		if (!module) {
			return 0;
		}
		break;
	default:
		return -1;
	}

	sections = sysfs_get_module_sections(module);

	switch (flag) {
	case 0:
		if (!sections) {
			if (errno == 0)
				dbg_print("%s: No sections for the module at "
						"%s\n", __FUNCTION__, name);
			else
				dbg_print("%s: FAILED with flag = %d errno = "
					"%d\n", __FUNCTION__, flag, errno);
		} else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
						__FUNCTION__, flag);
			show_section_list(sections);
			dbg_print("\n");
		}
		break;
	case 1:
	case 2:
		if (sections)
			dbg_print("%s: FAILED with flag = %d errno = %d\n",
						__FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
							__FUNCTION__, flag);
		break;
	default:
		break;
	}

	if (module)
		sysfs_close_module(module);
	return 0;
}

/**
 * sysfs_get_module_parm:
 * @module: sysfs_module to look through
 * @parm: name of the parameter to look for
 * Returns sysfs_attribute * on SUCCESS and NULL on error
 * flag:
 * 	0:	name -> valid, paramname -> valid
 * 	1:	name -> valid, paramname -> invalid
 * 	2:	name -> valid, paramname -> NULL
 */
int test_sysfs_get_module_parm(int flag)
{
	char *name, *paramname;
	struct sysfs_attribute *attr;
	struct sysfs_module *module;

	switch (flag) {
	case 0:
		name = val_mod_path;
		module = sysfs_open_module_path(name);
		if (!module) {
			dbg_print("%s: failed opening module at %s\n",
						__FUNCTION__, name);
			return 0;
		}
		paramname = val_mod_param;
		break;
	case 1:
		name = val_mod_path;
		module = sysfs_open_module_path(name);
		if (!module) {
			dbg_print("%s: failed opening module at %s\n",
						__FUNCTION__, name);
			return 0;
		}
		paramname = inval_name;
		break;
	case 2:
		name = val_mod_path;
		module = sysfs_open_module_path(name);
		if (!module) {
			dbg_print("%s: failed opening module at %s\n",
						__FUNCTION__, name);
			return 0;
		}
		paramname = NULL;
		break;
	default:
		return -1;
	}

	attr = sysfs_get_module_parm(module,paramname);

	switch (flag) {
	case 0:
		if(!attr) {
			if (errno == EACCES)
				dbg_print("%s: parameter %s not used by module"
					, __FUNCTION__, paramname);
			else if (errno == ENOENT)
				dbg_print("%s: attribute %s not defined for "
					"module at %s\n", __FUNCTION__,
							paramname, name);
			else if (errno == 0)
				dbg_print("%s: module at %s does not use "
				"parameter\n", __FUNCTION__, val_mod_path);
			else
				dbg_print("%s: FAILED with flag = %d errno = "
					"%d\n", __FUNCTION__, flag, errno);
		} else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
					__FUNCTION__, flag);
			show_attribute(attr);
			dbg_print("\n");
		}
		break;

	case 1:
	case 2:
		if (attr)
			dbg_print("%s: FAILED with flag = %d errno = "
				"%d\n", __FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
							__FUNCTION__, flag);
	default:
		break;
	}

	if (module)
		sysfs_close_module(module);
	return 0;
}

/**
 * sysfs_get_module_section
 * @module: sysfs_module to look through
 * @section: name of the section to look for
 * Returns sysfs_attribute * on SUCCESS and NULL on error
 * flag:
 * 	0:	name -> valid, sectname -> valid
 * 	1:	name -> valid, sectname -> invalid
 * 	2:	name -> valid, sectname -> NULL
 */
int test_sysfs_get_module_section(int flag)
{
	char *name, *sectname;
	struct sysfs_attribute *attr;
	struct sysfs_module *module;

	switch (flag) {
	case 0:
		name = val_mod_path;
		module = sysfs_open_module_path(name);
		if (!module) {
			dbg_print("%s: failed opening module at %s\n",
						__FUNCTION__, name);
			return 0;
		}
		sectname = val_mod_section;
		break;
	case 1:
		name = val_mod_path;
		module = sysfs_open_module_path(name);
		if (!module) {
			dbg_print("%s: failed opening module at %s\n",
						__FUNCTION__, name);
			return 0;
		}
		sectname = inval_name;
		break;
	case 2:
		name = val_mod_path;
		module = sysfs_open_module_path(name);
		if (!module) {
			dbg_print("%s: failed opening module at %s\n",
						__FUNCTION__, name);
			return 0;
		}
		sectname = NULL;
		break;
	default:
		return -1;
	}

	attr = sysfs_get_module_section(module,sectname);

	switch (flag) {
	case 0:
		if(!attr) {
			if (errno == EACCES)
				dbg_print("%s: section %s not used by module"
					, __FUNCTION__, sectname);
			else if (errno == ENOENT)
				dbg_print("%s: section %s not defined for "
					"module at %s\n", __FUNCTION__,
							sectname, name);
			else if (errno == 0)
				dbg_print("%s: module at %s does not use "
				"section\n", __FUNCTION__, val_mod_path);
			else
				dbg_print("%s: FAILED with flag = %d errno = "
					"%d\n", __FUNCTION__, flag, errno);
		} else {
			dbg_print("%s: SUCCEEDED with flag = %d\n\n",
					__FUNCTION__, flag);
			show_attribute(attr);
			dbg_print("\n");
		}
		break;

	case 1:
	case 2:
		if (attr)
			dbg_print("%s: FAILED with flag = %d errno = "
				"%d\n", __FUNCTION__, flag, errno);
		else
			dbg_print("%s: SUCCEEDED with flag = %d\n",
							__FUNCTION__, flag);
	default:
		break;
	}

	if (module)
		sysfs_close_module(module);
	return 0;
}
