/*
** Copyright (C) 2010 Sourcefire, Inc.
** Author: Michael R. Altizer <maltizer@sourcefire.com>
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
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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

#include "daq.h"
#include "daq_api.h"

#define NAME_SIZE       512

#ifdef STATIC_MODULE_LIST
extern const DAQ_Module_t *static_modules[];
extern const int num_static_modules;
#endif

static int daq_verbosity = 0;

#ifdef WIN32
inline void DEBUG(char *fmt, ...)
{

    if (daq_verbosity > 0)
    {
        va_list ap;
        va_start(ap, fmt);

        printf(fmt, ap);

        va_end(ap);
    }
}
#else
#define DEBUG(...) do { if (daq_verbosity > 0) { printf(__VA_ARGS__); } } while (0)
#endif

typedef struct _daq_list_node
{
    const DAQ_Module_t *module;
    void *dl_handle;
    struct _daq_list_node *next;
} DAQ_ListNode_t;

static DAQ_ListNode_t *module_list = NULL;
static int num_modules = 0;

static const char *daq_verdict_strings[MAX_DAQ_VERDICT] = {
    "pass",         // DAQ_VERDICT_PASS
    "block",        // DAQ_VERDICT_BLOCK
    "replace",      // DAQ_VERDICT_REPLACE
    "whitelist",    // DAQ_VERDICT_WHITELIST
    "blacklist",    // DAQ_VERDICT_BLACKLIST
    "ignore"        // DAQ_VERDICT_IGNORE
};

static const char *daq_mode_strings[MAX_DAQ_MODE] = {
    "passive",      // DAQ_MODE_PASSIVE
    "inline",       // DAQ_MODE_INLINE
    "read-file"     // DAQ_MODE_READ_FILE
};

static const char *daq_state_strings[MAX_DAQ_STATE] = {
    "uninitialized",    // DAQ_STATE_UNINITIALIZED
    "initialized",      // DAQ_STATE_INITIALIZED
    "started",          // DAQ_STATE_STARTED
    "stopped",          // DAQ_STATE_STOPPED
    "unknown"           // DAQ_STATE_UNKNOWN
};

DAQ_LINKAGE const char *daq_verdict_string(DAQ_Verdict verdict)
{
    if (verdict >= MAX_DAQ_VERDICT)
        return NULL;

    return daq_verdict_strings[verdict];
}

DAQ_LINKAGE const char *daq_mode_string(DAQ_Mode mode)
{
    if (mode >= MAX_DAQ_MODE)
        return NULL;

    return daq_mode_strings[mode];
}

DAQ_LINKAGE const char *daq_state_string(DAQ_State state)
{
    if (state >= MAX_DAQ_STATE)
        return NULL;

    return daq_state_strings[state];
}

DAQ_LINKAGE const DAQ_Module_t *daq_find_module(const char *name)
{
    DAQ_ListNode_t *node;

    for (node = module_list; node; node = node->next)
    {
        if (!strcmp(name, node->module->name))
            return node->module;
    }

    return NULL;
}

static int register_module(const DAQ_Module_t *dm, void *dl_handle)
{
    DAQ_ListNode_t *node;

    /* Check to make sure the module's API version matches ours. */
    if (dm->api_version != DAQ_API_VERSION)
    {
        fprintf(stderr, "%s: Module API version (0x%x) differs from expected version (0x%x)\n",
                dm->name, dm->api_version, DAQ_API_VERSION);
        return DAQ_ERROR;
    }

    /* Check to make sure that ALL of the function pointers are populated. */
    if (!dm->initialize || !dm->set_filter || !dm->start || !dm->acquire || !dm->inject || !dm->breakloop ||
        !dm->stop || !dm->shutdown || !dm->check_status || !dm->get_stats || !dm->reset_stats ||
        !dm->get_snaplen || !dm->get_capabilities || !dm->get_errbuf || !dm->set_errbuf || !dm->get_device_index)
    {
        fprintf(stderr, "%s: Module definition is missing function pointer(s)!\n", dm->name);
        return DAQ_ERROR;
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
            DEBUG("DAQ module with name '%s' was already loaded with version %u (versus %u)!\n",
                    node->module->name, node->module->module_version, dm->module_version);
            return DAQ_ERROR_EXISTS;
        }
        if (node->dl_handle)
        {
            dlclose(node->dl_handle);
        }
    }
    else
    {
        node = calloc(1, sizeof(DAQ_ListNode_t));
        if (!node)
            return DAQ_ERROR_NOMEM;
        node->next = module_list;
        module_list = node;
        num_modules++;
    }

    DEBUG("Registered daq module: %s\n", dm->name);
    node->module = dm;
    node->dl_handle = dl_handle;

    return DAQ_SUCCESS;
}

static int daq_load_module(const char *filename)
{
    const DAQ_Module_t *dm;
    struct stat fs;
    void *dl_handle;
    int rval;

    if (filename == NULL)
        return DAQ_ERROR_INVAL;

    if ((stat(filename, &fs)) != 0 || !(fs.st_mode & S_IFREG))
    {
        fprintf(stderr, "%s: File does not exist.\n", filename);
        return DAQ_ERROR;
    }

    if ((dl_handle = dlopen(filename, RTLD_NOW)) == NULL)
    {
        fprintf(stderr, "%s: %s: %s\n", filename, dlopen_func_name, dlerror());
        return DAQ_ERROR;
    }

    if ((dm = (const DAQ_Module_t*)dlsym(dl_handle, "DAQ_MODULE_DATA")) == NULL)
    {
        fprintf(stderr, "%s: %s: %s\n", filename, dlsym_func_name, dlerror());
        dlclose(dl_handle);
        return DAQ_ERROR;
    }

    if ((rval = register_module(dm, dl_handle)) != DAQ_SUCCESS)
    {
        if (rval != DAQ_ERROR_EXISTS)
            fprintf(stderr, "%s: Failed to register DAQ module.\n", filename);
        dlclose(dl_handle);
        return DAQ_ERROR;
    }

    return DAQ_SUCCESS;
}

#ifdef STATIC_MODULE_LIST
static void load_static_modules(void)
{
    const DAQ_Module_t *dm;
    int i;

    DEBUG("Static modules: %d\n", num_static_modules);
    for (i = 0; i < num_static_modules; i++)
    {
        dm = static_modules[i];
        if (register_module(dm, NULL) != DAQ_SUCCESS)
            fprintf(stderr, "%s (%d): Failed to register static DAQ module.\n", dm->name, i);
    }
}
#endif

DAQ_LINKAGE int daq_load_modules(const char *directory_list[])
{
    static const char *extension = MODULE_EXT;
#ifndef WIN32
    char dirpath[NAME_SIZE];
    DIR *dirp;
    struct dirent *de;
    char *p;
    int ret;

#ifdef STATIC_MODULE_LIST
    load_static_modules();
#endif

    for (; directory_list && *directory_list; directory_list++)
    {
        if (!(**directory_list))
            continue;
        if ((dirp = opendir(*directory_list)) == NULL)
        {
            fprintf(stderr,"Unable to open directory \"%s\"\n", *directory_list);
            continue;
        }

        DEBUG("Loading modules in: %s\n", *directory_list);

        while((de = readdir(dirp)) != NULL)
        {
            if (de->d_name == NULL)
                continue;
            p = strrchr(de->d_name, '.');
            if (!p || strcmp(p, extension))
                continue;
            snprintf(dirpath, sizeof(dirpath), "%s/%s", *directory_list, de->d_name);

            ret = daq_load_module(dirpath);
            if (ret == DAQ_SUCCESS)
            {
                DEBUG("Found module %s\n", de->d_name);
            }
            else if (ret == DAQ_ERROR_NOMEM)
            {
                closedir(dirp);
                return DAQ_ERROR_NOMEM;
            }
        }
        closedir(dirp);
    }
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
    const char *directory;
    int useDrive = 0;

#ifdef STATIC_MODULE_LIST
    load_static_modules();
#endif

    for (; directory_list && *directory_list; directory_list++)
    {
        if (!(**directory_list))
            continue;

        if ((strncpy(path_buf, *directory_list, PATH_MAX) == NULL) ||
            (strlen(path_buf) != strlen(*directory_list)))
        {
            fprintf(stderr, "Path is too long: %s\n", *directory_list);
            continue;
        }

        pathLen = strlen(path_buf);
        if ((path_buf[pathLen - 1] == '\\') ||
            (path_buf[pathLen - 1] == '/'))
        {
            /* A directory was specified with trailing dir character */
            _splitpath(path_buf, drive, dir, fname, ext);
            _makepath(path_buf, drive, dir, "*", MODULE_EXT);
            directory = &dir[0];
            useDrive = 1;
        }
        else /* A directory was specified */
        {
            _splitpath(path_buf, drive, dir, fname, ext);
            if (strcmp(extension, ""))
            {
                fprintf(stderr, "Improperly formatted directory name: %s\n", *directory_list);
                continue;
            }

            _makepath(path_buf, "", path_buf, "*", MODULE_EXT);
            directory = *directory_list;
        }

        fSearch = FindFirstFile(path_buf, &FindFileData);
        while (fSearch != NULL && fSearch != (HANDLE)-1)
        {
            if (useDrive)
                _makepath(dyn_lib_name, drive, directory, FindFileData.cFileName, NULL);
            else
                _makepath(dyn_lib_name, NULL, directory, FindFileData.cFileName, NULL);

            daq_load_module(dyn_lib_name);

            if (!FindNextFile(fSearch, &FindFileData))
            {
                break;
            }
        }
        FindClose(fSearch);
    }
#endif
    return DAQ_SUCCESS;
}

DAQ_LINKAGE void daq_unload_modules(void)
{
    DAQ_ListNode_t *node;

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
}

DAQ_LINKAGE void daq_print_stats(DAQ_Stats_t *stats, FILE *fp)
{
    if (!stats)
        return;

    if (!fp)
        fp = stdout;

    fprintf(fp, "*DAQ Module Statistics*\n");
    fprintf(fp, "  Hardware Packets Received:  %" PRIu64 "\n", stats->hw_packets_received);
    fprintf(fp, "  Hardware Packets Dropped:   %" PRIu64 "\n", stats->hw_packets_dropped);
    fprintf(fp, "  Packets Received:   %" PRIu64 "\n", stats->packets_received);
    fprintf(fp, "  Packets Filtered:   %" PRIu64 "\n", stats->packets_filtered);
    fprintf(fp, "  Packets Passed:     %" PRIu64 "\n", stats->verdicts[DAQ_VERDICT_PASS]);
    fprintf(fp, "  Packets Replaced:   %" PRIu64 "\n", stats->verdicts[DAQ_VERDICT_REPLACE]);
    fprintf(fp, "  Packets Blocked:    %" PRIu64 "\n", stats->verdicts[DAQ_VERDICT_BLOCK]);
    fprintf(fp, "  Packets Injected:   %" PRIu64 "\n", stats->packets_injected);
    fprintf(fp, "  Flows Whitelisted:  %" PRIu64 "\n", stats->verdicts[DAQ_VERDICT_WHITELIST]);
    fprintf(fp, "  Flows Blacklisted:  %" PRIu64 "\n", stats->verdicts[DAQ_VERDICT_BLACKLIST]);
    fprintf(fp, "  Flows Ignored:      %" PRIu64 "\n", stats->verdicts[DAQ_VERDICT_IGNORE]);
}

DAQ_LINKAGE int daq_get_module_list(DAQ_Module_Info_t *list[])
{
    DAQ_Module_Info_t *info;
    DAQ_ListNode_t *node;
    int idx;

    if (!list)
        return DAQ_ERROR_INVAL;

    info = calloc(num_modules, sizeof(DAQ_Module_Info_t));
    if (!info)
        return DAQ_ERROR_NOMEM;

    idx = 0;
    for (node = module_list; node; node = node->next)
    {
        info[idx].name = strdup(node->module->name);
        if (info[idx].name == NULL)
        {
            daq_free_module_list(info, idx);
            return DAQ_ERROR_NOMEM;
        }
        info[idx].version = node->module->module_version;
        info[idx].type = node->module->type;
        idx++;
    }

    *list = info;

    return num_modules;
}

DAQ_LINKAGE void daq_free_module_list(DAQ_Module_Info_t *list, int size)
{
    int idx;

    if (!list || size <= 0)
        return;

    for (idx = 0; idx < size; idx++)
    {
        if (list[idx].name)
            free(list[idx].name);
    }

    free(list);
}

DAQ_LINKAGE void daq_set_verbosity(int level)
{
    daq_verbosity = level;
    DEBUG("DAQ verbosity level is set to %d.\n", daq_verbosity);
}

DAQ_LINKAGE const char *daq_config_get_value(DAQ_Config_t *config, const char *key)
{
    DAQ_Dict *entry;

    if (!config || !key)
        return NULL;

    for (entry = config->values; entry; entry = entry->next)
    {
        if (!strcmp(entry->key, key))
            return entry->value;
    }

    return NULL;
}

DAQ_LINKAGE void daq_config_set_value(DAQ_Config_t *config, const char *key, const char *value)
{
    DAQ_Dict *entry;

    if (!config || !key)
        return;

    for (entry = config->values; entry; entry = entry->next)
    {
        if (!strcmp(entry->key, key))
            break;
    }

    if (!entry)
    {
        entry = calloc(1, sizeof(struct _daq_dict_entry));
        if (!entry)
        {
            fprintf(stderr, "%s: Could not allocate %lu bytes for a dictionary entry!\n",
                    __FUNCTION__, (unsigned long) sizeof(struct _daq_dict_entry));
            return;
        }
        entry->key = strdup(key);
        if (!entry->key)
        {
            fprintf(stderr, "%s: Could not allocate %lu bytes for a dictionary entry key!\n",
                    __FUNCTION__, (unsigned long) (strlen(key) + 1));
            return;
        }
        entry->next = config->values;
        config->values = entry;
    }
    free(entry->value);
    if (value)
    {
        entry->value = strdup(value);
        if (!entry->value)
        {
            fprintf(stderr, "%s: Could not allocate %lu bytes for a dictionary entry value!\n",
                    __FUNCTION__, (unsigned long) (strlen(value) + 1));
            return;
        }
    }
    DEBUG("Set config dictionary entry '%s' => '%s'.\n", entry->key, entry->value);
}

DAQ_LINKAGE void daq_config_clear_value(DAQ_Config_t *config, const char *key)
{
    DAQ_Dict *entry, *prev = NULL;

    if (!config || !key)
        return;

    for (entry = config->values; entry; entry = entry->next)
    {
        if (!strcmp(entry->key, key))
        {
            if (prev)
                prev->next = entry->next;
            else
                config->values = entry->next;
            free(entry->key);
            free(entry->value);
            free(entry);
            return;
        }
        prev = entry;
    }
}

DAQ_LINKAGE void daq_config_clear_values(DAQ_Config_t *config)
{
    DAQ_Dict *entry;

    if (!config)
        return;

    while (config->values)
    {
        entry = config->values;
        config->values = entry->next;
        free(entry->key);
        free(entry->value);
        free(entry);
    }
}
