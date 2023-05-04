/*
 ** Copyright (C) 2014-2022 Cisco and/or its affiliates. All rights reserved.
 ** Copyright (C) 2012-2013 Sourcefire, Inc.
 **
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License Version 2 as
 ** published by the Free Software Foundation.  You may not use, modify or
 ** distribute this program under any other version of the GNU General
 ** Public License.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU General Public License for more details.
 **
 ** You should have received a copy of the GNU General Public License
 ** along with this program; if not, write to the Free Software
 ** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 **
 ** Date: 01-27-2012
 ** Author: Hui Cao <hcao@sourcefire.com>
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef WIN32
#include <dirent.h>
#include <dlfcn.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#if defined(HPUX)
#define MODULE_EXT ".sl"
#else
#define MODULE_EXT ".so"
#endif

#define dlclose_func "dlclose"
#define dlopen_func_name "dlopen"
#define dlsym_func_name "dlsym"

#else /* !WIN32 */
#include <windows.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#define MODULE_EXT "dll"

#define dlclose(args) FreeLibrary(args)
#define dlclose_func_name "FreeLibrary"
#define dlopen(path, arg2) LoadLibrary(path)
#define dlopen_func_name "LoadLibrary"
#define dlsym(handle, func) GetProcAddress(handle, func)
#define dlsym_func_name "GetProcAddress"
#define dlerror() GetLastError()

#define getcwd _getcwd
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif

#endif
#include "sf_types.h"
#include "plugbase.h"
#include "output.h"
#include "output_api.h"
#include "output_lib.h"
#include "util.h"

#define NAME_SIZE       512

#ifdef STATIC_MODULE_LIST
extern const Output_Module_t *static_modules[];
extern const int num_static_modules;
#endif


typedef struct _output_list_node
{
    const Output_Module_t *module;
    void *dl_handle;
    struct _output_list_node *next;
} Output_ListNode_t;

static Output_ListNode_t *module_list = NULL;
static int num_modules = 0;
static int loaded = 0;

void * GetNextOutputModule(void *p_node)
{
    Output_ListNode_t *node = (Output_ListNode_t *)p_node;
    if (node)
        return node->next;

    return module_list;
}

const char * GetOutputModuleName(void *p_node)
{
    Output_ListNode_t *node = (Output_ListNode_t *)p_node;
    if (node)
        return node->module->name;

    return NULL;
}

uint32_t GetOutputModuleVersion(void *p_node)
{
    Output_ListNode_t *node = (Output_ListNode_t *)p_node;
    if (node)
        return node->module->module_version;

    return 0;
}

const Output_Module_t *output_find_module(const char *name)
{
    Output_ListNode_t *node;

    for (node = module_list; node; node = node->next)
    {
        if (!strcmp(name, node->module->name))
            return node->module;
    }

    return NULL;
}

static int register_plugin(const Output_Module_t *dm)
{

    /* Check to make sure that the required function pointers are populated. */
    if (!dm->parse_args || !dm->shutdown )
    {
        fprintf(stderr, "%s: Module definition is missing function pointer(s)!\n", dm->name);
        return OUTPUT_ERROR;
    }

    RegisterOutputPlugin((char*)dm->name, dm->type, dm->load);

    DEBUG_WRAP(DebugMessage(DEBUG_INIT, "Registered dynamic output plugin: %s\n", dm->name););

    return OUTPUT_SUCCESS;
}

static int register_module(const Output_Module_t *dm, void *dl_handle)
{
    Output_ListNode_t *node;
    Output_Module_t *current_dm = (Output_Module_t *)dm;

    /* Check to make sure the module's API version matches ours. */
    if (dm->api_major_version != OUTPUT_API_MAJOR_VERSION)
    {
        fprintf(stderr, "%s: Module API major version (0x%x) differs from expected version (0x%x)\n",
                dm->name, dm->api_major_version, OUTPUT_API_MAJOR_VERSION);
        return OUTPUT_ERROR;
    }

    if (dm->api_minor_version < OUTPUT_API_MINOR_VERSION)
    {
        fprintf(stderr, "%s: Module API minor version (0x%x) differs from expected version (0x%x)\n",
                dm->name, dm->api_minor_version, OUTPUT_API_MINOR_VERSION);
        return OUTPUT_ERROR;
    }

    /* Output modules should have a non-NULL name. */
    if (dm->name == NULL)
    {
        fprintf(stderr, "Module name is NULL\n");
        return OUTPUT_ERROR;
    }

    /* Do we already have a module with the same name loaded? */
    for (node = module_list; node; node = node->next)
    {
        if (!strcmp(node->module->name, dm->name))
            break;
    }

    /* If so, and this version is newer, use it instead.  Otherwise, create a new node. */
    if (node)
    {
        if (node->module->module_version >= dm->module_version)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_INIT, "OUTPUT module with name '%s' was already loaded"
                    " with version %u (versus %u)!\n",
                    node->module->name, node->module->module_version, dm->module_version););
            return OUTPUT_ERROR_EXISTS;
        }
        if (node->dl_handle)
        {
            dlclose(node->dl_handle);
            RemoveOutputPlugin((char *)dm->name);
        }
    }
    else
    {
        node = calloc(1, sizeof(Output_ListNode_t));
        if (!node)
            return OUTPUT_ERROR_NOMEM;
        node->next = module_list;
        module_list = node;
        num_modules++;
    }

    /*add all the plugins within the module*/
    while(current_dm)
    {
        int rval;
        if ((rval = register_plugin(current_dm)) != OUTPUT_SUCCESS)
        {
            if (rval != OUTPUT_ERROR_EXISTS)
                fprintf(stderr, "%s: Failed to register OUTPUT plugin.\n", current_dm->name);
            return OUTPUT_ERROR;
        }
        current_dm = current_dm->next;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Registered output module: %s\n", dm->name););
    node->module = dm;
    node->dl_handle = dl_handle;

    return OUTPUT_SUCCESS;
}

int output_load_module(const char *filename)
{
    const Output_Module_t *dm;
    void *outputInit;
    struct stat fs;
    void *dl_handle;
    int rval;

    if (filename == NULL)
        return OUTPUT_ERROR_INVAL;

    if ((stat(filename, &fs)) != 0 || !(fs.st_mode & S_IFREG))
    {
        fprintf(stderr, "%s: File does not exist.\n", filename);
        return OUTPUT_ERROR;
    }

    if ((dl_handle = dlopen(filename, RTLD_NOW)) == NULL)
    {
        fprintf(stderr, "%s: %s: %s\n", filename, dlopen_func_name, dlerror());
        return OUTPUT_ERROR;
    }

    /* Initiate _dod functions*/
    if ((outputInit = dlsym(dl_handle, "initOutputPlugins")) == NULL)
    {
        fprintf(stderr, "%s: %s: %s\n", filename, dlsym_func_name, dlerror());
        dlclose(dl_handle);
        return OUTPUT_ERROR;
    }

    if (initOutputPlugin(outputInit) !=OUTPUT_SUCCESS)
    {
        fprintf(stderr, "%s: Failed to load OUTPUT module.\n", filename);
        dlclose(dl_handle);
        return OUTPUT_ERROR;
    }

    if ((dm = (const Output_Module_t*)dlsym(dl_handle, "OUTPUT_MODULE_DATA")) == NULL)
    {
        fprintf(stderr, "%s: %s: %s\n", filename, dlsym_func_name, dlerror());
        dlclose(dl_handle);
        return OUTPUT_ERROR;
    }


    if ((rval = register_module(dm, dl_handle)) != OUTPUT_SUCCESS)
    {
        if (rval != OUTPUT_ERROR_EXISTS)
            fprintf(stderr, "%s: Failed to register OUTPUT module.\n", filename);
        dlclose(dl_handle);
        return OUTPUT_ERROR;
    }

    return OUTPUT_SUCCESS;
}

#ifdef STATIC_MODULE_LIST
static void load_static_modules(void)
{
    const Output_Module_t *dm;
    int i;

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Static modules: %d\n", num_static_modules););
    for (i = 0; i < num_static_modules; i++)
    {
        dm = static_modules[i];
        if (register_module(dm, NULL) != OUTPUT_SUCCESS)
            fprintf(stderr, "%s (%d): Failed to register static OUTPUT module.\n", dm->name, i);
    }
}
#endif

/*Load all the output modules in the directory*/
int output_load(const char *directory)
{

#ifndef WIN32
    char dirpath[NAME_SIZE];
    DIR *dirp;
    struct dirent *de;
    char *p;
    int ret;

#ifdef STATIC_MODULE_LIST
    load_static_modules();
#endif

    if (!(*directory))
        return OUTPUT_ERROR;

    if ((dirp = opendir(directory)) == NULL)
    {
        fprintf(stderr,"Unable to open directory \"%s\"\n", directory);
        return OUTPUT_ERROR;
    }

    DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Loading modules in: %s\n", directory););

    while((de = readdir(dirp)) != NULL)
    {
        p = strrchr(de->d_name, '.');
        if (!p || strcmp(p, MODULE_EXT))
            continue;
        snprintf(dirpath, sizeof(dirpath), "%s/%s", directory, de->d_name);
        dirpath[NAME_SIZE-1] = '\0';
        ret = output_load_module(dirpath);
        if (ret == OUTPUT_SUCCESS)
        {
            DEBUG_WRAP(DebugMessage(DEBUG_INIT,"Found module %s\n", de->d_name););
        }
        else if (ret == OUTPUT_ERROR_NOMEM)
        {
            closedir(dirp);
            return OUTPUT_ERROR_NOMEM;
        }
    }
    closedir(dirp);

#else
    /* Find all shared library files in path */
    char path_buf[PATH_MAX];
    char dyn_lib_name[PATH_MAX];
    char drive[_MAX_DRIVE];
    char dir[_MAX_DIR];
    char fname[_MAX_FNAME];
    char ext[_MAX_EXT];
    HANDLE fSearch;
    WIN32_FIND_DATA FindFileData;
    int pathLen = 0;
    const char *full_directory;
    int useDrive = 0;

#ifdef STATIC_MODULE_LIST
    load_static_modules();
#endif

    if (!(*directory))
        return OUTPUT_ERROR;

    if ((strncpy(path_buf, directory, PATH_MAX) == NULL) ||
            (strlen(path_buf) != strlen(directory)))
    {
        fprintf(stderr, "Path is too long: %s\n", directory);
        return OUTPUT_ERROR;
    }

    pathLen = strlen(path_buf);
    if ((path_buf[pathLen - 1] == '\\') ||
            (path_buf[pathLen - 1] == '/'))
    {
        /* A directory was specified with trailing dir character */
        _splitpath(path_buf, drive, dir, fname, ext);
        _makepath(path_buf, drive, dir, "*", MODULE_EXT);
        full_directory = &dir[0];
        useDrive = 1;
    }
    else /* A directory was specified */
    {
        _splitpath(path_buf, drive, dir, fname, ext);
        if (strcmp(MODULE_EXT, ""))
        {
            fprintf(stderr, "Improperly formatted directory name: %s\n", directory);
            return OUTPUT_ERROR;
        }

        _makepath(path_buf, "", path_buf, "*", MODULE_EXT);
        full_directory = directory;
    }

    fSearch = FindFirstFile(path_buf, &FindFileData);
    while (fSearch != NULL && fSearch != (HANDLE)-1)
    {
        if (useDrive)
            _makepath(dyn_lib_name, drive, full_directory, FindFileData.cFileName, NULL);
        else
            _makepath(dyn_lib_name, NULL, full_directory, FindFileData.cFileName, NULL);

        output_load_module(dyn_lib_name);

        if (!FindNextFile(fSearch, &FindFileData))
        {
            break;
        }
    }
    FindClose(fSearch);

#endif
    loaded = 1;
    return OUTPUT_SUCCESS;
}

void output_unload(void)
{
    Output_ListNode_t *node;

    while (module_list)
    {
        node = module_list;
        module_list = node->next;
        if (node->dl_handle)
        {
            dlclose(node->dl_handle);
        }
        free(node);
        num_modules--;
    }
    loaded = 0;
}



