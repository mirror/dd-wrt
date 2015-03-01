/* $Id: */
/*
 * sf_dynamic_plugins.c
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 as
 * published by the Free Software Foundation.  You may not use, modify or
 * distribute this program under any other version of the GNU General
 * Public License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * Copyright (C) 2014 Cisco and/or its affiliates. All rights reserved.
 * Copyright (C) 2005-2013 Sourcefire, Inc.
 *
 * Author: Steven Sturges
 *
 * Dynamic Library Loading for Snort
 *
 */
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef WIN32
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <limits.h>
#include <dirent.h>
#include <dlfcn.h>
#include <fnmatch.h>
#if defined(HPUX)
#define MODULE_EXT "*.sl"
#else
#define MODULE_EXT "*.so*"
#endif
typedef void * PluginHandle;
#else /* !WIN32 */
#include <windows.h>
#define MODULE_EXT "dll"
typedef HANDLE PluginHandle;
/* Of course, WIN32 couldn't do things the unix way...
 * Define a few of these to get around portability issues.
 */
#define getcwd _getcwd
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif
#endif /* !WIN32 */

#include <errno.h>

#include "config.h"
#include "decode.h"
#include "encode.h"
#include "snort_debug.h"
#include "detect.h"
#include "util.h"
#include "snort.h"
#include "sf_dynamic_engine.h"
#include "sf_dynamic_detection.h"
#include "sf_dynamic_preprocessor.h"
#include "sp_dynamic.h"
#include "sp_preprocopt.h"
#include "sp_pcre.h"
#include "util.h"
#include "event_queue.h"
#include "plugbase.h"
#include "sfthreshold.h"
#include "active.h"
#include "mstring.h"
#include "sfsnprintfappend.h"
#include "session_api.h"
#include "stream_api.h"
#include "sf_iph.h"
#include "fpdetect.h"
#include "sfportobject.h"
#include <pcre.h>
#include "parser.h"
#include "event_wrapper.h"
#include "util.h"
#include "detection_util.h"
#include "sfcontrol_funcs.h"
#include "idle_processing_funcs.h"
#include "../dynamic-output/plugins/output.h"
#include "file_api.h"
#include "packet_time.h"

#ifdef TARGET_BASED
#include "target-based/sftarget_protocol_reference.h"
#include "target-based/sftarget_reader.h"
#endif

#ifdef SIDE_CHANNEL
#include "sidechannel.h"
#include "sf_dynamic_side_channel.h"
#endif

#ifndef DEBUG_MSGS
char *no_file = "unknown";
int no_line = 0;
#endif

/* Predeclare this */
void VerifySharedLibUniqueness();
typedef int (*LoadLibraryFunc)(const char * const path, int indent);

typedef struct _DynamicEnginePlugin
{
    PluginHandle handle;
    DynamicPluginMeta metaData;
    InitEngineLibFunc initFunc;
    CompatibilityFunc versCheck;
    struct _DynamicEnginePlugin *next;
    struct _DynamicEnginePlugin *prev;
} DynamicEnginePlugin;

static DynamicEnginePlugin *loadedEngines = NULL;

typedef struct _DynamicDetectionPlugin
{
    PluginHandle handle;
    DynamicPluginMeta metaData;
    InitDetectionLibFunc initFunc;
    struct _DynamicDetectionPlugin *next;
    struct _DynamicDetectionPlugin *prev;
} DynamicDetectionPlugin;

static DynamicDetectionPlugin *loadedDetectionPlugins = NULL;

typedef struct _DynamicPreprocessorPlugin
{
    PluginHandle handle;
    DynamicPluginMeta metaData;
    InitPreprocessorLibFunc initFunc;
    struct _DynamicPreprocessorPlugin *next;
    struct _DynamicPreprocessorPlugin *prev;
} DynamicPreprocessorPlugin;

static DynamicPreprocessorPlugin *loadedPreprocessorPlugins = NULL;

typedef struct _LoadableModule
{
    char *prefix;
    char *name;
    struct _LoadableModule *next;

} LoadableModule;

void CloseDynamicLibrary(PluginHandle handle)
{
#ifndef WIN32
# ifndef DISABLE_DLCLOSE_FOR_VALGRIND_TESTING
    dlclose(handle);
# endif
#else
    FreeLibrary(handle);
#endif
}

#define NONFATAL 0
#define FATAL 1

typedef void (*dlsym_func)(void);

static dlsym_func getSymbol(
    PluginHandle handle, char *symbol, DynamicPluginMeta *meta, int fatal)
{
    dlsym_func symbolPtr = NULL;

    if (!handle)
        return symbolPtr;

#ifndef WIN32
    symbolPtr = (dlsym_func)dlsym(handle, symbol);
#else
    symbolPtr = (dlsym_func)GetProcAddress(handle, symbol);
#endif

    if (!symbolPtr)
    {
        if (fatal)
        {
#ifndef WIN32
            FatalError("Failed to find %s() function in %s: %s\n",
                symbol, meta->libraryPath, dlerror());
#else
            FatalError("Failed to find %s() function in %s: %d\n",
                symbol, meta->libraryPath, GetLastError());
#endif
        }
        else
        {
#ifndef WIN32
            ErrorMessage("Failed to find %s() function in %s: %s\n",
                symbol, meta->libraryPath, dlerror());
#else
            ErrorMessage("Failed to find %s() function in %s: %d\n",
                symbol, meta->libraryPath, GetLastError());
#endif
        }
    }

    return symbolPtr;
}

void GetPluginVersion(PluginHandle handle, DynamicPluginMeta* meta)
{
    LibVersionFunc libVersionFunc = NULL;

    libVersionFunc = (LibVersionFunc)getSymbol(handle, "LibVersion", meta, FATAL);

    if (libVersionFunc != NULL)
    {
        libVersionFunc(meta);
    }
}

PluginHandle openDynamicLibrary(const char * const library_name, int useGlobal)
{
    PluginHandle handle;
#ifndef WIN32
    handle = dlopen(library_name, RTLD_NOW | (useGlobal ? RTLD_GLOBAL : RTLD_LOCAL));
#else
    handle = LoadLibrary(library_name);
#endif
    if (handle == NULL)
    {
#ifndef WIN32
        FatalError("Failed to load %s: %s\n", library_name, dlerror());
#else
        FatalError("Failed to load %s: %d\n", library_name, GetLastError());
#endif
    }
    return handle;
}

void LoadAllLibs(const char * const path, LoadLibraryFunc loadFunc)
{
#ifndef WIN32
    char path_buf[PATH_MAX];
    struct dirent *dir_entry;
    DIR *directory;
    int  count = 0;
    LoadableModule *modules = NULL;

    directory = opendir(path);
    if (directory != NULL)
    {
        dir_entry = readdir(directory);
        while (dir_entry != NULL)
        {
            if (fnmatch(MODULE_EXT, dir_entry->d_name, FNM_PATHNAME | FNM_PERIOD) == 0)
            {
                /* Get the string up until the first dot.  This will be
                 * considered the file prefix. */
                char *dot = strchr(dir_entry->d_name, '.');

                if (dot != NULL)
                {
                    size_t len = (size_t)(dot - dir_entry->d_name);  // len >= 0
                    LoadableModule *tmp = modules;
                    LoadableModule *prev = NULL;

                    while (tmp != NULL)
                    {
                        /* Make sure the prefix lengths are the same */
                        if (strlen(tmp->prefix) == len)
                        {
                            /* And make sure they are the same string */
                            if (strncmp(tmp->prefix, dir_entry->d_name, len) == 0)
                            {
                                /* Take the shorter, since the longer probably
                                 * has version information and the shorter ones
                                 * are generally links to the most recent
                                 * version, e.g.
                                 * libsf_engine.so.0.0.0
                                 * libsf_engine.so.0 -> libsf_engine.so.0.0.0
                                 * libsf_engine.so   -> libsf_engine.so.0.0.0
                                 * Mac seems to do
                                 * libsf_engine.0.so
                                 * libsf_engine.0.0.0.so -> libsf_engine.0.so
                                 * libsf_engine.so       -> libsf_engine.0.so
                                 * We don't want to load the same same thing
                                 * more than once. */
                                if (strlen(dir_entry->d_name) < strlen(tmp->name))
                                {
                                    /* There will be enough space since at
                                     * least the longer of the two will have
                                     * been allocated */
                                    strcpy(tmp->name, dir_entry->d_name);
                                }

                                break;
                            }
                        }

                        prev = tmp;
                        tmp = tmp->next;
                    }

                    if (tmp == NULL)
                    {
                        tmp = SnortAlloc(sizeof(LoadableModule));

                        /* include NULL byte */
                        tmp->prefix = SnortAlloc(len + 1);
                        /* will be NULL terminated because SnortAlloc uses calloc */
                        strncpy(tmp->prefix, dir_entry->d_name, len);

                        /* include NULL byte */
                        tmp->name = SnortAlloc(strlen(dir_entry->d_name) + 1);
                        /* will be NULL terminated because SnortAlloc uses calloc */
                        strncpy(tmp->name, dir_entry->d_name, strlen(dir_entry->d_name));

                        tmp->next = NULL;

                        if (modules == NULL)
                            modules = tmp;
                        else if (prev != NULL)
                            prev->next = tmp;
                    }
                }
            }

            dir_entry = readdir(directory);
        }

        closedir(directory);

        while (modules != NULL)
        {
            LoadableModule *tmp = modules;

            SnortSnprintf(path_buf, PATH_MAX, "%s%s%s", path, "/", modules->name);
            loadFunc(path_buf, 1);
            count++;

            modules = modules->next;

            /* These will all have been allocated together */
            free(tmp->prefix);
            free(tmp->name);
            free(tmp);
        }

        if ( count == 0 )
        {
            LogMessage("WARNING: No dynamic libraries found in directory %s.\n", path);
        }
    }
    else
    {
        LogMessage("WARNING: Directory %s does not exist.\n", path);
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

    if (SnortStrncpy(path_buf, path, PATH_MAX) != SNORT_STRNCPY_SUCCESS)
        FatalError("Path is too long: %s\n", path);

    pathLen = SnortStrnlen(path_buf, PATH_MAX);
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
        if (strcmp(ext, ""))
        {
            FatalError("Improperly formatted directory name: %s\n", path);
        }
        _makepath(path_buf, "", path_buf, "*", MODULE_EXT);
        directory = path;
    }

    fSearch = FindFirstFile(path_buf, &FindFileData);
    while (fSearch != NULL && fSearch != (HANDLE)-1)
    {
        if (useDrive)
            _makepath(dyn_lib_name, drive, directory, FindFileData.cFileName, NULL);
        else
            _makepath(dyn_lib_name, NULL, directory, FindFileData.cFileName, NULL);

        loadFunc(dyn_lib_name, 1);

        if (!FindNextFile(fSearch, &FindFileData))
        {
            break;
        }
    }
    FindClose(fSearch);
#endif
}

void AddEnginePlugin(PluginHandle handle,
                     InitEngineLibFunc initFunc,
                     CompatibilityFunc compatFunc,
                     DynamicPluginMeta *meta)
{
    DynamicEnginePlugin *newPlugin = NULL;
    newPlugin = (DynamicEnginePlugin *)SnortAlloc(sizeof(DynamicEnginePlugin));
    newPlugin->handle = handle;

    if (!loadedEngines)
    {
        loadedEngines = newPlugin;
    }
    else
    {
        newPlugin->next = loadedEngines;
        loadedEngines->prev = newPlugin;
        loadedEngines = newPlugin;
    }

    memcpy(&(newPlugin->metaData), meta, sizeof(DynamicPluginMeta));
    newPlugin->metaData.libraryPath = SnortStrdup(meta->libraryPath);
    newPlugin->initFunc = initFunc;
    newPlugin->versCheck = compatFunc;
}

void RemoveEnginePlugin(DynamicEnginePlugin *plugin)
{
    if (!plugin)
        return;

    if (plugin == loadedEngines)
    {
        loadedEngines = loadedEngines->next;
        loadedEngines->prev = NULL;
    }
    else
    {
        if (plugin->prev)
            plugin->prev->next = plugin->next;
        if (plugin->next)
            plugin->next->prev = plugin->prev;
    }
    CloseDynamicLibrary(plugin->handle);
    if (plugin->metaData.libraryPath != NULL)
        free(plugin->metaData.libraryPath);
    free(plugin);
}

int ValidateDynamicEngines(void)
{
    int testNum = 0;
    DynamicEnginePlugin *curPlugin = loadedEngines;
    CompatibilityFunc versFunc = NULL;

    while( curPlugin != NULL)
    {
        versFunc = (CompatibilityFunc)curPlugin->versCheck;
        /* if compatibility checking func is absent, skip validating */
        if( versFunc != NULL)
        {
            DynamicDetectionPlugin *lib = loadedDetectionPlugins;
            while( lib != NULL)
            {
                if (lib->metaData.type == TYPE_DETECTION)
                {
                    RequiredEngineLibFunc engineFunc;
                    DynamicPluginMeta reqEngineMeta;

                    engineFunc = (RequiredEngineLibFunc) getSymbol(lib->handle, "EngineVersion", &(lib->metaData), 1);
                    if( engineFunc != NULL)
                    {
                        engineFunc(&reqEngineMeta);
                    }
                    testNum = versFunc(&curPlugin->metaData, &reqEngineMeta);
                    if( testNum )
                    {
                        FatalError("The dynamic detection library \"%s\" version "
                                "%d.%d compiled with dynamic engine library "
                                "version %d.%d isn't compatible with the current "
                                "dynamic engine library \"%s\" version %d.%d.\n",
                                lib->metaData.libraryPath, lib->metaData.major,
                                lib->metaData.minor, reqEngineMeta.major,
                                reqEngineMeta.minor, curPlugin->metaData.libraryPath,
                                curPlugin->metaData.major, curPlugin->metaData.minor);
                    }

                }
                lib = lib->next;
            }
        }
        if( testNum ) break;
        curPlugin = curPlugin->next;
    }

    return(testNum);
}

int LoadDynamicEngineLib(const char * const library_name, int indent)
{
    /* Presume here, that library name is full path */
    InitEngineLibFunc engineInit;
    CompatibilityFunc compatFunc;
    DynamicPluginMeta metaData;
    PluginHandle handle;

#if 0
    LogMessage("%sDynamic engine will not be loaded since dynamic detection "
                 "libraries are not yet supported with IPv6.\n",
                indent?"  ":"");
    return 0;
#endif

    LogMessage("%sLoading dynamic engine %s... ",
               indent ? "  " : "", library_name);

    handle = openDynamicLibrary(library_name, 1);
    metaData.libraryPath = (char *) library_name;

    GetPluginVersion(handle, &metaData);

    /* Just to ensure that the function exists */
    engineInit = (InitEngineLibFunc)getSymbol(handle, "InitializeEngine", &metaData, FATAL);
    compatFunc = (CompatibilityFunc)getSymbol(handle, "CheckCompatibility", &metaData, NONFATAL);

    if (metaData.type != TYPE_ENGINE)
    {
        CloseDynamicLibrary(handle);
        LogMessage("failed, not an Engine\n");
        return 0;
    }

    AddEnginePlugin(handle, engineInit, compatFunc, &metaData);

    LogMessage("done\n");
    return 0;
}

void LoadAllDynamicEngineLibs(const char * const path)
{
    LogMessage("Loading all dynamic engine libs from %s...\n", path);
    LoadAllLibs(path, LoadDynamicEngineLib);
    LogMessage("  Finished Loading all dynamic engine libs from %s\n", path);
}

void CloseDynamicEngineLibs(void)
{
    DynamicEnginePlugin *tmpplugin, *plugin = loadedEngines;
    while (plugin)
    {
        tmpplugin = plugin->next;
        //if (!(plugin->metaData.type & TYPE_DETECTION))
        //{
            CloseDynamicLibrary(plugin->handle);
            free(plugin->metaData.libraryPath);
            free(plugin);
        //}
        //else
        //{
        //  HUH?
        //    /* NOP, handle will be closed when we close the DetectionLib */
        //    ;
        //}
        plugin = tmpplugin;
    }
    loadedEngines = NULL;
}

void RemovePreprocessorPlugin(DynamicPreprocessorPlugin *plugin)
{
    if (!plugin)
        return;

    if (plugin == loadedPreprocessorPlugins)
    {
        loadedPreprocessorPlugins = loadedPreprocessorPlugins->next;
        loadedPreprocessorPlugins->prev = NULL;
    }
    else
    {
        if (plugin->prev)
            plugin->prev->next = plugin->next;
        if (plugin->next)
            plugin->next->prev = plugin->prev;
    }
    CloseDynamicLibrary(plugin->handle);
    if (plugin->metaData.libraryPath != NULL)
        free(plugin->metaData.libraryPath);
    free(plugin);
}

void AddPreprocessorPlugin(PluginHandle handle,
                        InitPreprocessorLibFunc initFunc,
                        DynamicPluginMeta *meta)
{
    DynamicPreprocessorPlugin *newPlugin = NULL;
    newPlugin = (DynamicPreprocessorPlugin *)SnortAlloc(sizeof(DynamicPreprocessorPlugin));
    newPlugin->handle = handle;

    if (!loadedPreprocessorPlugins)
    {
        loadedPreprocessorPlugins = newPlugin;
    }
    else
    {
        newPlugin->next = loadedPreprocessorPlugins;
        loadedPreprocessorPlugins->prev = newPlugin;
        loadedPreprocessorPlugins = newPlugin;
    }

    memcpy(&(newPlugin->metaData), meta, sizeof(DynamicPluginMeta));
    newPlugin->metaData.libraryPath = SnortStrdup(meta->libraryPath);
    newPlugin->initFunc = initFunc;
}

void AddDetectionPlugin(PluginHandle handle,
                        InitDetectionLibFunc initFunc,
                        DynamicPluginMeta *meta)
{
    DynamicDetectionPlugin *newPlugin = NULL;
    newPlugin = (DynamicDetectionPlugin *)SnortAlloc(sizeof(DynamicDetectionPlugin));
    newPlugin->handle = handle;

    if (!loadedDetectionPlugins)
    {
        loadedDetectionPlugins = newPlugin;
    }
    else
    {
        newPlugin->next = loadedDetectionPlugins;
        loadedDetectionPlugins->prev = newPlugin;
        loadedDetectionPlugins = newPlugin;
    }

    memcpy(&(newPlugin->metaData), meta, sizeof(DynamicPluginMeta));
    newPlugin->metaData.libraryPath = SnortStrdup(meta->libraryPath);
    newPlugin->initFunc = initFunc;
}

void RemoveDetectionPlugin(DynamicDetectionPlugin *plugin)
{
    if (!plugin)
        return;

    if (plugin == loadedDetectionPlugins)
    {
        loadedDetectionPlugins = loadedDetectionPlugins->next;
        loadedDetectionPlugins->prev = NULL;
    }
    else
    {
        if (plugin->prev)
            plugin->prev->next = plugin->next;
        if (plugin->next)
            plugin->next->prev = plugin->prev;
    }
    LogMessage("Unloading dynamic detection library %s version %d.%d.%d\n",
            plugin->metaData.uniqueName,
            plugin->metaData.major,
            plugin->metaData.minor,
            plugin->metaData.build);
    CloseDynamicLibrary(plugin->handle);
    if (plugin->metaData.libraryPath != NULL)
        free(plugin->metaData.libraryPath);
    free(plugin);
}

int LoadDynamicDetectionLib(const char * const library_name, int indent)
{
    DynamicPluginMeta metaData;
    /* Presume here, that library name is full path */
    InitDetectionLibFunc detectionInit;
    PluginHandle handle;

#if 0
    LogMessage("%sDynamic detection library \"%s\" will not be loaded. Not "
                 "supported with IPv6.\n", indent ? "  " : "", library_name);
    return 0;
#endif

    LogMessage("%sLoading dynamic detection library %s... ",
               indent ? "  " : "", library_name);

    handle = openDynamicLibrary(library_name, 0);
    metaData.libraryPath = (char *) library_name;

    GetPluginVersion(handle, &metaData);

    /* Just to ensure that the function exists */
    detectionInit = (InitDetectionLibFunc)getSymbol(handle, "InitializeDetection", &metaData, FATAL);

    if (!(metaData.type & TYPE_DETECTION))
    {
        CloseDynamicLibrary(handle);
        LogMessage("failed, not a detection library\n");
        return 0;
    }

    if (metaData.type & TYPE_ENGINE)
    {
        /* Do the engine initialization as well */
        InitEngineLibFunc engineInit = (InitEngineLibFunc)getSymbol(handle, "InitializeEngine", &metaData, FATAL);
        CompatibilityFunc compatFunc = (CompatibilityFunc)getSymbol(handle, "CheckCompatibility", &metaData, NONFATAL);

        AddEnginePlugin(handle, engineInit, compatFunc, &metaData);
    }

    AddDetectionPlugin(handle, detectionInit, &metaData);

    LogMessage("done\n");
    return 0;
}

void CloseDynamicDetectionLibs(void)
{
    DynamicDetectionPlugin *tmpplugin, *plugin = loadedDetectionPlugins;
    while (plugin)
    {
        tmpplugin = plugin->next;
        CloseDynamicLibrary(plugin->handle);
        free(plugin->metaData.libraryPath);
        free(plugin);
        plugin = tmpplugin;
    }
    loadedDetectionPlugins = NULL;
}

void LoadAllDynamicDetectionLibs(const char * const path)
{
    LogMessage("Loading all dynamic detection libs from %s...\n", path);
    LoadAllLibs(path, LoadDynamicDetectionLib);
    LogMessage("  Finished Loading all dynamic detection libs from %s\n", path);
}

void RemoveDuplicateEngines(void)
{
    int removed = 0;
    DynamicEnginePlugin *engine1;
    DynamicEnginePlugin *engine2;
    DynamicPluginMeta *meta1;
    DynamicPluginMeta *meta2;

    /* First the Detection Engines */
    do
    {
        removed = 0;
        engine1 = loadedEngines;
        while (engine1 != NULL)
        {
            engine2 = loadedEngines;
            while (engine2 != NULL)
            {
                /* Obviously, the same ones will be the same */
                if (engine1 != engine2)
                {
                    meta1 = &engine1->metaData;
                    meta2 = &engine2->metaData;
                    if (!strcmp(meta1->uniqueName, meta2->uniqueName))
                    {
                        /* Uh, same uniqueName. */
                        if ((meta1->major > meta2->major) ||
                            ((meta1->major == meta2->major) && (meta1->minor > meta2->minor)) ||
                            ((meta1->major == meta2->major) && (meta1->minor == meta2->minor) && (meta1->build > meta2->build)) )
                        {
                            /* Lib1 is newer */
                            RemoveEnginePlugin(engine2);
                            removed = 1;
                        }
                        else if ((meta2->major > meta1->major) ||
                            ((meta2->major == meta1->major) && (meta2->minor > meta1->minor)) ||
                            ((meta2->major == meta1->major) && (meta2->minor == meta1->minor) && (meta2->build > meta1->build)) )
                        {
                            /* Lib2 is newer */
                            RemoveEnginePlugin(engine1);
                            removed = 1;
                        }
                        else if ((meta1->major == meta2->major) && (meta1->minor == meta2->minor) && (meta1->build == meta2->build) )
                        {
                            /* Duplicate */
                            RemoveEnginePlugin(engine2);
                            removed = 1;
                        }
                    }
                }
                /* If we removed anything, start back at the beginning */
                if (removed)
                    break;
                engine2 = engine2->next;
            }
            /* If we removed anything, start back at the beginning */
            if (removed)
                break;
            engine1 = engine1->next;
        }
    } while (removed);
}

void RemoveDuplicateDetectionPlugins(void)
{
    int removed = 0;
    DynamicDetectionPlugin *lib1 = NULL;
    DynamicDetectionPlugin *lib2 = NULL;
    DynamicPluginMeta *meta1;
    DynamicPluginMeta *meta2;

    /* Detection Plugins */
    do
    {
        removed = 0;
        lib1 = loadedDetectionPlugins;
        while (lib1 != NULL)
        {
            lib2 = loadedDetectionPlugins;
            while (lib2 != NULL)
            {
                /* Obviously, the same ones will be the same */
                if (lib1 != lib2)
                {
                    meta1 = &lib1->metaData;
                    meta2 = &lib2->metaData;
                    if (!strcmp(meta1->uniqueName, meta2->uniqueName))
                    {
                        /* Uh, same uniqueName. */
                        if ((meta1->major > meta2->major) ||
                            ((meta1->major == meta2->major) && (meta1->minor > meta2->minor)) ||
                            ((meta1->major == meta2->major) && (meta1->minor == meta2->minor) && (meta1->build > meta2->build)) )
                        {
                            /* Lib1 is newer */
                            RemoveDetectionPlugin(lib2);
                            removed = 1;
                        }
                        else if ((meta2->major > meta1->major) ||
                            ((meta2->major == meta1->major) && (meta2->minor > meta1->minor)) ||
                            ((meta2->major == meta1->major) && (meta2->minor == meta1->minor) && (meta2->build > meta1->build)) )
                        {
                            /* Lib2 is newer */
                            RemoveDetectionPlugin(lib1);
                            removed = 1;
                        }
                        else if ((meta1->major == meta2->major) && (meta1->minor == meta2->minor) && (meta1->build == meta2->build) )
                        {
                            /* Duplicate */
                            RemoveDetectionPlugin(lib2);
                            removed = 1;
                        }
                    }
                }
                /* If we removed anything, start back at the beginning */
                if (removed)
                    break;
                lib2 = lib2->next;
            }
            /* If we removed anything, start back at the beginning */
            if (removed)
                break;
            lib1 = lib1->next;
        }
    } while (removed);
}

void RemoveDuplicatePreprocessorPlugins(void)
{
    int removed = 0;
    DynamicPreprocessorPlugin *pp1 = NULL;
    DynamicPreprocessorPlugin *pp2 = NULL;
    DynamicPluginMeta *meta1;
    DynamicPluginMeta *meta2;

    /* The Preprocessor Plugins */
    do
    {
        removed = 0;
        pp1 = loadedPreprocessorPlugins;
        while (pp1 != NULL)
        {
            pp2 = loadedPreprocessorPlugins;
            while (pp2 != NULL)
            {
                /* Obviously, the same ones will be the same */
                if (pp1 != pp2)
                {
                    meta1 = &pp1->metaData;
                    meta2 = &pp2->metaData;
                    if (!strcmp(meta1->uniqueName, meta2->uniqueName))
                    {
                        /* Uh, same uniqueName. */
                        if ((meta1->major > meta2->major) ||
                            ((meta1->major == meta2->major) && (meta1->minor > meta2->minor)) ||
                            ((meta1->major == meta2->major) && (meta1->minor == meta2->minor) && (meta1->build > meta2->build)) )
                        {
                            /* Lib1 is newer */
                            RemovePreprocessorPlugin(pp2);
                            removed = 1;
                        }
                        else if ((meta2->major > meta1->major) ||
                            ((meta2->major == meta1->major) && (meta2->minor > meta1->minor)) ||
                            ((meta2->major == meta1->major) && (meta2->minor == meta1->minor) && (meta2->build > meta1->build)) )
                        {
                            /* Lib2 is newer */
                            RemovePreprocessorPlugin(pp1);
                            removed = 1;
                        }
                        else if ((meta1->major == meta2->major) && (meta1->minor == meta2->minor) && (meta1->build == meta2->build) )
                        {
                            /* Duplicate */
                            RemovePreprocessorPlugin(pp2);
                            removed = 1;
                        }
                    }
                }
                /* If we removed anything, start back at the beginning */
                if (removed)
                    break;
                pp2 = pp2->next;
            }
            /* If we removed anything, start back at the beginning */
            if (removed)
                break;
            pp1 = pp1->next;
        }
    } while (removed);
}

void VerifyDetectionPluginRequirements(void)
{
    DynamicDetectionPlugin *lib1 = NULL;

    /* Remove all the duplicates */
    RemoveDuplicateDetectionPlugins();

    /* Cycle through all of them, and ensure that the required
     * detection engine is loaded.
     */
    lib1 = loadedDetectionPlugins;
    while (lib1 != NULL)
    {
        /* Do this check if this library is a DETECTION plugin only.
         * If it also has an internal engine, we're fine.
         */
        if (lib1->metaData.type == TYPE_DETECTION)
        {
            RequiredEngineLibFunc engineFunc;
            DynamicPluginMeta reqEngineMeta;
            DynamicEnginePlugin *plugin = loadedEngines;
            int detectionLibOkay = 0;

            engineFunc = (RequiredEngineLibFunc) getSymbol(lib1->handle, "EngineVersion", &(lib1->metaData), FATAL);

            engineFunc(&reqEngineMeta);
            while (plugin != NULL)
            {
                /* Exact match.  Yes! */
                if (!strcmp(plugin->metaData.uniqueName, reqEngineMeta.uniqueName) &&
                    plugin->metaData.major == reqEngineMeta.major &&
                    plugin->metaData.minor == reqEngineMeta.minor)
                {
                    detectionLibOkay = 1;
                    break;
                }

                /* Major match, minor must be >= */
                if (!strcmp(plugin->metaData.uniqueName, reqEngineMeta.uniqueName) &&
                    plugin->metaData.major == reqEngineMeta.major &&
                    plugin->metaData.minor >= reqEngineMeta.minor)
                {
                    detectionLibOkay = 1;
                    break;
                }

                /* Major must be >= -- this assumes newer engine is
                 * bass-ackwards compatabile */
                if (!strcmp(plugin->metaData.uniqueName, reqEngineMeta.uniqueName) &&
                    plugin->metaData.major > reqEngineMeta.major)
                {
                    detectionLibOkay = 1;
                    break;
                }

                plugin = plugin->next;
            }
            if (!detectionLibOkay)
            {
                FatalError("Loaded dynamic detection plugin %s (version %d:%d:%d) "
                           "could not find required engine plugin %s(version %d:%d)\n",
                            lib1->metaData.uniqueName, lib1->metaData.major, lib1->metaData.minor, lib1->metaData.build,
                            reqEngineMeta.uniqueName, reqEngineMeta.major, reqEngineMeta.minor);
            }
        }

        lib1 = lib1->next;
    }
}

int InitDynamicEnginePlugins(DynamicEngineData *info)
{
    DynamicEnginePlugin *plugin;
    RemoveDuplicateEngines();

    plugin = loadedEngines;
    while (plugin)
    {
        if (plugin->initFunc(info))
        {
            FatalError("Failed to initialize dynamic engine: %s version %d.%d.%d\n",
                       plugin->metaData.uniqueName, plugin->metaData.major,
                       plugin->metaData.minor, plugin->metaData.build);
            //return -1;
        }

        plugin = plugin->next;
    }
    return 0;
}

typedef struct _DynamicRuleSessionData
{
    uint32_t sid;
    void *data;
    SessionDataFree cleanupFunc;
    struct _DynamicRuleSessionData *next;

} DynamicRuleSessionData;

static uint32_t so_rule_memory = 0;
/*Only only message will be logged within 60 seconds*/
static ThrottleInfo error_throttleInfo = {0,60,0};

static void * DynamicRuleDataAlloc(size_t size)
{
    size_t alloc_size = size + sizeof(size_t);
    size_t *ret;

    if ((ScSoRuleMemcap() > 0)
            && (so_rule_memory + alloc_size) > ScSoRuleMemcap())
    {
        ErrorMessageThrottled(&error_throttleInfo,"SO rule memcap exceeded: Wanted to allocate "
                "%u bytes (and %d overhead) with memcap: %u and "
                "current memory: %u\n", (uint32_t)size,
                (int)sizeof(size_t), ScSoRuleMemcap(), so_rule_memory);
        return NULL;
    }

    ret = (size_t *)SnortAlloc(alloc_size);
    ret[0] = alloc_size;
    so_rule_memory += alloc_size;
    return (void *)&ret[1];
}

static void DynamicRuleDataFree(void *data)
{
    if (data != NULL)
    {
        size_t *alloc_data = (size_t *)data - 1;
        size_t size = alloc_data[0];

        /* Just in case of an an imbalance of DynamicRuleDataAlloc
         * and this function are used */
        if (size >= so_rule_memory)
            so_rule_memory = 0;
        else
            so_rule_memory -= size;
        free(alloc_data);
    }
}

static void DynamicRuleDataFreeSession(void *data)
{
    DynamicRuleSessionData *drsd = (DynamicRuleSessionData *)data;

    while (drsd != NULL)
    {
        DynamicRuleSessionData *tmp = drsd;
        drsd = drsd->next;

        if (tmp->data && tmp->cleanupFunc)
            tmp->cleanupFunc(tmp->data);
        DynamicRuleDataFree(tmp);
    }
}

int DynamicSetRuleData(void *p, void *data, uint32_t sid, SessionDataFree sdf)
{
    Packet *pkt = (Packet *)p;
    if (stream_api && pkt && pkt->ssnptr)
    {
        DynamicRuleSessionData *head =
            (DynamicRuleSessionData *)session_api->get_application_data(pkt->ssnptr, PP_SHARED_RULES);
        DynamicRuleSessionData *tmp = head;
        DynamicRuleSessionData *tail = NULL;

        /* Can't reset head without setting application data again which
         * will free what's there already, so have to iterate to end of list
         * Also need to iterate for duplicates */
        while (tmp != NULL)
        {
            if (tmp->sid == sid)
            {
                /* Not the same data */
                if (tmp->data != data)
                {
                    /* Cleanup the old and replace with the new */
                    if (tmp->data && tmp->cleanupFunc)
                        tmp->cleanupFunc(tmp->data);
                    tmp->data = data;
                }

                tmp->cleanupFunc = sdf;
                return 0;
            }

            tail = tmp;
            tmp = tmp->next;
        }

        tmp = (DynamicRuleSessionData *)DynamicRuleDataAlloc(sizeof(DynamicRuleSessionData));
        if (tmp == NULL)
            return -1;

        tmp->data = data;
        tmp->sid = sid;
        tmp->cleanupFunc = sdf;

        if (head == NULL)
        {
            if (session_api->set_application_data(pkt->ssnptr, PP_SHARED_RULES,
                        (void *)tmp, DynamicRuleDataFreeSession) != 0)
            {
                DynamicRuleDataFree(tmp);
                return -1;
            }
        }
        else
        {
            tail->next = tmp;
        }

        return 0;
    }

    return -1;
}

void * DynamicGetRuleData(void *p, uint32_t sid)
{
    Packet *pkt = (Packet *)p;

    if (stream_api && pkt && pkt->ssnptr)
    {
        DynamicRuleSessionData *head =
            (DynamicRuleSessionData *)session_api->get_application_data(pkt->ssnptr, PP_SHARED_RULES);

        while (head != NULL)
        {
            if (head->sid == sid)
                return head->data;

            head = head->next;
        }
    }

    return NULL;
}

void *pcreCompile(const char *pattern, int options, const char **errptr, int *erroffset, const unsigned char *tableptr)
{
    options &= ~SNORT_PCRE_OVERRIDE_MATCH_LIMIT;
    return (void *)pcre_compile(pattern, options, errptr, erroffset, tableptr);
}

void *pcreStudy(const void *code, int options, const char **errptr)
{
    pcre_extra *extra_extra;
    int snort_options = options & SNORT_PCRE_OVERRIDE_MATCH_LIMIT;

    extra_extra = pcre_study((const pcre*)code, 0, errptr);

    if (extra_extra)
    {
        if ((ScPcreMatchLimit() != -1) && !(snort_options & SNORT_PCRE_OVERRIDE_MATCH_LIMIT))
        {
            if (extra_extra->flags & PCRE_EXTRA_MATCH_LIMIT)
            {
                extra_extra->match_limit = ScPcreMatchLimit();
            }
            else
            {
                extra_extra->flags |= PCRE_EXTRA_MATCH_LIMIT;
                extra_extra->match_limit = ScPcreMatchLimit();
            }
        }

#ifdef PCRE_EXTRA_MATCH_LIMIT_RECURSION
        if ((ScPcreMatchLimitRecursion() != -1) && !(snort_options & SNORT_PCRE_OVERRIDE_MATCH_LIMIT))
        {
            if (extra_extra->flags & PCRE_EXTRA_MATCH_LIMIT_RECURSION)
            {
                extra_extra->match_limit_recursion = ScPcreMatchLimitRecursion();
            }
            else
            {
                extra_extra->flags |= PCRE_EXTRA_MATCH_LIMIT_RECURSION;
                extra_extra->match_limit_recursion = ScPcreMatchLimitRecursion();
            }
        }
#endif
    }
    else
    {
        if (!(snort_options & SNORT_PCRE_OVERRIDE_MATCH_LIMIT) &&
            ((ScPcreMatchLimit() != -1) || (ScPcreMatchLimitRecursion() != -1)))
        {
            extra_extra = (pcre_extra *)SnortAlloc(sizeof(pcre_extra));
            if (ScPcreMatchLimit() != -1)
            {
                extra_extra->flags |= PCRE_EXTRA_MATCH_LIMIT;
                extra_extra->match_limit = ScPcreMatchLimit();
            }

#ifdef PCRE_EXTRA_MATCH_LIMIT_RECURSION
            if (ScPcreMatchLimitRecursion() != -1)
            {
                extra_extra->flags |= PCRE_EXTRA_MATCH_LIMIT_RECURSION;
                extra_extra->match_limit_recursion = ScPcreMatchLimitRecursion();
            }
#endif
        }
    }

    return extra_extra;
}

/* pcreOvectorInfo
 *
 * Get the Ovector configuration for PCRE from the snort.conf
 */
void pcreOvectorInfo(int **ovector, int *ovector_size)
{
    *ovector = snort_conf->pcre_ovector;
    *ovector_size = snort_conf->pcre_ovector_size;
}

int pcreExec(const void *code, const void *extra, const char *subj,
             int len, int start, int options, int *ovec, int ovecsize)
{
    return pcre_exec((const pcre *)code, (const pcre_extra *)extra, subj, len, start, options, ovec, ovecsize);
}

static int setFlowId(const void* p, uint32_t id)
{
    return DAQ_ModifyFlow(p, id);
}

static const uint8_t* getHttpBuffer (HTTP_BUFFER hb_type, unsigned* len)
{
    const HttpBuffer* hb = GetHttpBuffer(hb_type);
    if ( !hb )
        return NULL;

    *len = hb->length;
    return hb->buf;
}

int InitDynamicEngines(char *dynamic_rules_path)
{
    DynamicEngineData engineData;

    engineData.version = ENGINE_DATA_VERSION;
    engineData.altBuffer = (SFDataBuffer *)&DecodeBuffer;
    engineData.altDetect = (SFDataPointer *)&DetectBuffer;
    engineData.fileDataBuf = (SFDataPointer *)&file_data_ptr;

    /* This is defined in dynamic-plugins/sp_dynamic.h */
    engineData.ruleRegister = &RegisterDynamicRule;
    engineData.flowbitRegister = &DynamicFlowbitRegister;
    engineData.flowbitCheck = &DynamicFlowbitCheck;
    engineData.asn1Detect = &DynamicAsn1Detect;

    if (dynamic_rules_path != NULL)
        engineData.dataDumpDirectory = SnortStrdup(dynamic_rules_path);
    else
        engineData.dataDumpDirectory = NULL;

    engineData.logMsg = &LogMessage;
    engineData.errMsg = &ErrorMessage;
    engineData.fatalMsg = &FatalError;

    engineData.preprocRuleOptInit = &DynamicPreprocRuleOptInit;

    engineData.setRuleData = &DynamicSetRuleData;
    engineData.getRuleData = &DynamicGetRuleData;

    engineData.sfUnfold = &DynamicsfUnfold;
    engineData.sfbase64decode = &Dynamicsfbase64decode;
    engineData.GetAltDetect = &DynamicGetAltDetect;
    engineData.SetAltDetect = &DynamicSetAltDetect;
    engineData.Is_DetectFlag = &DynamicIsDetectFlag;
    engineData.DetectFlag_Disable = &DynamicDetectFlagDisable;

    engineData.debugMsg = &DebugMessageFunc;
#ifdef SF_WCHAR
    engineData.debugWideMsg = &DebugWideMessageFunc;
#endif
#ifdef DEBUG_MSGS
    engineData.debugMsgFile = &DebugMessageFile;
    engineData.debugMsgLine = &DebugMessageLine;
#else
    engineData.debugMsgFile = &no_file;
    engineData.debugMsgLine = &no_line;
#endif

    engineData.pcreStudy = &pcreStudy;
    engineData.pcreCompile = &pcreCompile;
    engineData.pcreExec = &pcreExec;

    engineData.allocRuleData = &DynamicRuleDataAlloc;
    engineData.freeRuleData = &DynamicRuleDataFree;

    engineData.flowbitUnregister = &DynamicFlowbitUnregister;

    engineData.pcreCapture = &PcreCapture;
    engineData.pcreOvectorInfo = &pcreOvectorInfo;
    engineData.getHttpBuffer = getHttpBuffer;

    return InitDynamicEnginePlugins(&engineData);
}

int InitDynamicPreprocessorPlugins(DynamicPreprocessorData *info)
{
    DynamicPreprocessorPlugin *plugin;
    RemoveDuplicatePreprocessorPlugins();

    plugin = loadedPreprocessorPlugins;
    while (plugin)
    {
        int i = plugin->initFunc(info);
        if (i)
        {
            FatalError("Failed to initialize dynamic preprocessor: %s version %d.%d.%d (%d)\n",
                       plugin->metaData.uniqueName, plugin->metaData.major,
                       plugin->metaData.minor, plugin->metaData.build, i);
            //return -1;
        }

        plugin = plugin->next;
    }
    return 0;
}

/* Do this to avoid exposing Packet & PreprocessFuncNode from
 * snort to non-GPL code */
typedef void (*SnortPacketProcessFunc)(Packet *, void *);
void *AddPreprocessor(struct _SnortConfig *sc, void (*pp_func)(void *, void *), uint16_t priority,
                      uint32_t preproc_id, uint32_t proto_mask)
{
    SnortPacketProcessFunc preprocessorFunc = (SnortPacketProcessFunc)pp_func;
    return (void *)AddFuncToPreprocList(sc, preprocessorFunc, priority, preproc_id, proto_mask);
}

void *AddPreprocessorAllPolicies(struct _SnortConfig *sc, void (*pp_func)(void *, void *),
                                 uint16_t priority, uint32_t preproc_id, uint32_t proto_mask)
{
    SnortPacketProcessFunc preprocessorFunc = (SnortPacketProcessFunc)pp_func;
    AddFuncToPreprocListAllNapPolicies(sc, preprocessorFunc, priority, preproc_id, proto_mask);
    return NULL;
}

typedef void (*MetadataProcessFunc)(int, const uint8_t *);
void *AddMetaEval(struct _SnortConfig *sc, void (*meta_eval_func)(int, const uint8_t *), uint16_t priority,
                      uint32_t preproc_id)
{
    MetadataProcessFunc metaEvalFunc = (MetadataProcessFunc)meta_eval_func;
    return (void *)AddFuncToPreprocMetaEvalList(sc, metaEvalFunc, priority, preproc_id);
}

void *AddDetection(struct _SnortConfig *sc, void (*det_func)(void *, void *), uint16_t priority,
                      uint32_t det_id, uint32_t proto_mask)
{
    SnortPacketProcessFunc detectionFunc = (SnortPacketProcessFunc)det_func;
    return (void *)AddFuncToDetectionList(sc, detectionFunc, priority, det_id, proto_mask);
}

void AddPreprocessorCheck(struct _SnortConfig *sc, int (*pp_chk_func)(struct _SnortConfig *sc))
{
    AddFuncToConfigCheckList(sc, pp_chk_func);
}

void DynamicDisableDetection( void *p )
{
    DisableDetect( ( Packet * ) p );
}

void DynamicDisableAllDetection( void *p )
{
    DisableAllDetect( ( Packet * ) p );
}

void DynamicDisablePacketAnalysis( void *p )
{
    DisablePacketAnalysis( ( Packet * ) p );
}

int DynamicDetect(void *p)
{
    return Detect((Packet *)p);
}

int DynamicEnablePreprocessor(void *p, uint32_t preprocId)
{
    return EnablePreprocessor((Packet *)p, preprocId);
}

void DynamicDropReset(void *p)
{
    Active_DropSession((Packet*)p);
}

void DynamicForceDropPacket(void *p)
{
    Active_ForceDropSession();
}

void DynamicForceDropReset(void *p)
{
    Active_ForceDropResetAction((Packet *)p);
}

#ifdef ACTIVE_RESPONSE
void DynamicActiveSetEnabled(int on_off)
{
     Active_SetEnabled(on_off);
}
#endif

void *DynamicGetRuleClassByName(char *name)
{
    return (void *)ClassTypeLookupByType(snort_conf, name);
}

void *DynamicGetRuleClassById(int id)
{
    return (void *)ClassTypeLookupById(snort_conf, id);
}

void DynamicRegisterPreprocessorProfile(const char *keyword, void *stats, int layer, void *parent)
{
#ifdef PERF_PROFILING
    RegisterPreprocessorProfile(keyword, (PreprocStats *)stats, layer, (PreprocStats *)parent);
#endif
}

int DynamicProfilingPreprocs(void)
{
#ifdef PERF_PROFILING
    return ScProfilePreprocs();
#else
    return 0;
#endif
}

int DynamicPreprocess(void *packet)
{
    return Preprocess( ( Packet * ) packet );
}

void DynamicDisablePreprocessors(void *p)
{
    DisableAppPreprocessors( ( Packet * ) p );
}

void DynamicIP6Build(void *p, const void *hdr, int family)
{
    sfiph_build((Packet *)p, hdr, family);
}

static inline void DynamicIP6SetCallbacks(void *p, int family, char orig)
{
    set_callbacks((Packet *)p, family, orig);
}

int DynamicSnortEventqLog(void *p)
{
    return SnortEventqLog(snort_conf->event_queue, (Packet *)p);
}

tSfPolicyId DynamicGetParserPolicy(struct _SnortConfig *sc)
{
    return getParserPolicy(sc);
}

tSfPolicyId DynamicGetNapRuntimePolicy(void)
{
    return getNapRuntimePolicy();
}

tSfPolicyId DynamicGetIpsRuntimePolicy(void)
{
    return getIpsRuntimePolicy();
}

tSfPolicyId DynamicGetDefaultPolicy(void)
{
    return getDefaultPolicy();
}

tSfPolicyId DynamicGetPolicyFromId(uint16_t id)
{
    return sfPolicyIdGetBinding(snort_conf->policy_config, id);
}

void DynamicChangeNapRuntimePolicy(tSfPolicyId new_id, void *scb)
{
    session_api->set_runtime_policy( scb, SNORT_NAP_POLICY, new_id );
    setNapRuntimePolicy(new_id);
}

void DynamicChangeIpsRuntimePolicy(tSfPolicyId new_id, void *p)
{
    Packet *pkt = (Packet *) p;

    session_api->set_runtime_policy( pkt->ssnptr, SNORT_IPS_POLICY, new_id );
    setIpsRuntimePolicy(new_id);
    pkt->configPolicyId = snort_conf->targeted_policies[new_id]->configPolicyId;
}

static void* DynamicEncodeNew (void)
{
    return (void*)Encode_New();
}

static void DynamicEncodeDelete (void *p)
{
    Encode_Delete((Packet*)p);
}

static void *DynamicNewGrinderPkt(void *p, void *phdr, uint8_t *pkt)
{
    return (void*)NewGrinderPkt((Packet *)p, (DAQ_PktHdr_t *)phdr, pkt);
}

static void DynamicDeleteGrinderPkt(void *p)
{
    DeleteGrinderPkt((Packet*)p);
}

static int DynamicEncodeFormat (uint32_t f, const void* p, void *c, int t)
{
    return Encode_Format(f, (Packet*)p, (Packet*)c, (PseudoPacketType)t);
}

static void DynamicEncodeUpdate (void* p)
{
    Encode_Update((Packet*)p);
}

#ifdef ACTIVE_RESPONSE
void DynamicSendBlockResponseMsg(void *p, const uint8_t* buffer, uint32_t buffer_len, unsigned flags)
{
    Packet *packet = (Packet *)p;
    EncodeFlags df = (packet->packet_flags & PKT_FROM_SERVER) ? ENC_FLAG_FWD:0;

    if ( !packet->data || packet->dsize == 0 )
        return;

    if (flags & SND_BLK_RESP_FLAG_DO_CLIENT)
        df |= ENC_FLAG_RST_CLNT;
    if (flags & SND_BLK_RESP_FLAG_DO_SERVER)
        df |= ENC_FLAG_RST_SRVR;
    if (packet->packet_flags & PKT_STREAM_EST)
        Active_SendData(packet, df, buffer, buffer_len);
}

void DynamicActiveInjectData(void *p, uint32_t flags, const uint8_t *buf, uint32_t blen)
{
    Active_InjectData((Packet *)p, (EncodeFlags)flags, buf, blen);
}
#endif

void DynamicDropPacket(void *p)
{
    Active_DropPacket((Packet*)p);
}

void DynamicSetParserPolicy(SnortConfig *sc, tSfPolicyId id)
{
    setParserPolicy(sc, id);
}

void DynamicSetFileDataPtr(uint8_t *ptr, uint16_t decode_size)
{
    setFileDataPtr(ptr, decode_size);
}

void DynamicDetectResetPtr(uint8_t *ptr, uint16_t decode_size)
{
    DetectReset(ptr, decode_size);
}


void DynamicSetAltDecode(uint16_t altLen)
{
    SetAltDecode(altLen);
}

int DynamicGetNapInlineMode(void)
{
    return ScNapInlineMode();
}

int DynamicGetIpsInlineMode(void)
{
    return ScIpsInlineMode();
}

long DynamicSnortStrtol(const char *nptr, char **endptr, int base)
{
    return SnortStrtol(nptr,endptr,base);
}

unsigned long DynamicSnortStrtoul(const char *nptr, char **endptr, int base)
{
    return SnortStrtoul(nptr,endptr,base);
}

const char *DynamicSnortStrnStr(const char *s, int slen, const char *accept)
{
    return SnortStrnStr(s, slen, accept);
}


const char *DynamicSnortStrcasestr(const char *s, int slen, const char *accept)
{
    return SnortStrcasestr(s, slen, accept);
}

int DynamicSnortStrncpy(char *dst, const char *src, size_t dst_size)
{
    return SnortStrncpy(dst, src, dst_size);
}

const char *DynamicSnortStrnPbrk(const char *s, int slen, const char *accept)
{
    return SnortStrnPbrk(s, slen, accept);
}

int DynamicEvalRTN(void *rtn, void *p, int check_ports)
{
    return fpEvalRTN((RuleTreeNode *)rtn, (Packet *)p, check_ports);
}

char *DynamicGetLogDirectory(void)
{
    return SnortStrdup(snort_conf->log_dir);
}

uint32_t DynamicGetSnortInstance(void)
{
    return (snort_conf->event_log_id >> 16);
}

bool DynamicIsPafEnabled(void)
{
    return ScPafEnabled();
}

time_t DynamicPktTime(void)
{
    return packet_time();
}

void DynamicGetPktTimeOfDay(struct timeval *tv)
{
    packet_gettimeofday(tv);
}

#ifdef SIDE_CHANNEL
bool DynamicIsSCEnabled(void)
{
    return ScSideChannelEnabled();
}

int DynamicSCRegisterRXHandler(uint16_t type, SCMProcessMsgFunc processMsgFunc, void *data)
{
    return SideChannelRegisterRXHandler(type, processMsgFunc, data);
}

int DynamicSCPreallocMessageTX(uint32_t length, SCMsgHdr **hdr_ptr, uint8_t **msg_ptr, void **msg_handle)
{
    return SideChannelPreallocMessageTX(length, hdr_ptr, msg_ptr, msg_handle);
}

int DynamicSCEnqueueMessageTX(SCMsgHdr *hdr, const uint8_t *msg, uint32_t length, void *msg_handle, SCMQMsgFreeFunc msgFreeFunc)
{
    return SideChannelEnqueueMessageTX(hdr, msg, length, msg_handle, msgFreeFunc);
}
#endif

int DynamicCanWhitelist(void)
{
    return DAQ_CanWhitelist();
}

int DynamicSnortIsStrEmpty(const char *s)
{
    return IsEmptyStr((char*)s);
}

static void DynamicDisableAllPolicies(struct _SnortConfig *sc)
{
    DisableAllPolicies(sc);
}

static int DynamicReenablePreprocBitFunc(struct _SnortConfig *sc, unsigned int preproc_id)
{
    return ReenablePreprocBit(sc, preproc_id);
}

#ifdef SIDE_CHANNEL
static sigset_t DynamicSnortSignalMask(void)
{
    sigset_t mask;

    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGQUIT);
    sigaddset(&mask, SIGPIPE);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGNAL_SNORT_RELOAD);
    sigaddset(&mask, SIGNAL_SNORT_DUMP_STATS);
    sigaddset(&mask, SIGUSR1);
    sigaddset(&mask, SIGUSR2);
    sigaddset(&mask, SIGNAL_SNORT_ROTATE_STATS);
    sigaddset(&mask, SIGNAL_SNORT_CHILD_READY);
#ifdef TARGET_BASED
    sigaddset(&mask, SIGNAL_SNORT_READ_ATTR_TBL);
    sigaddset(&mask, SIGVTALRM);
#endif
    pthread_sigmask(SIG_SETMASK, &mask, NULL);

    return mask;
}
#endif

static SslAppIdLookupFunc sslAppIdLookupFnPtr;

static void registerSslAppIdLookup(SslAppIdLookupFunc fnptr)
{
    sslAppIdLookupFnPtr = fnptr;
}

static int sslAppIdLookup(void *ssnptr, const char * serverName, const char * commonName, int32_t *serviceAppId, int32_t *clientAppId, int32_t *payloadAppId)
{
    if (sslAppIdLookupFnPtr)
        return (sslAppIdLookupFnPtr)(ssnptr, serverName, commonName, serviceAppId, clientAppId, payloadAppId);
    return 0;
}

static GetAppIdFunc getAppIdFnPtr = NULL;

static void registerGetAppId(GetAppIdFunc fnptr)
{
    getAppIdFnPtr = fnptr;
}

static int32_t getAppId(void *ssnptr)
{
    if(getAppIdFnPtr)
        return (getAppIdFnPtr)(ssnptr);
    return 0;
}


static UrlQueryCreateFunc urlQueryCreateFnPtr;
static UrlQueryDestroyFunc urlQueryDestroyFnPtr;
static UrlQueryMatchFunc urlQueryMatchFnPtr;
static UserGroupIdGetFunc userGroupIdGetFnPtr;
static GeoIpAddressLookupFunc geoIpAddressLookupFnPtr;
static UpdateSSLSSnLogDataFunc updateSSLSSnLogDataFnPtr;
static EndSSLSSnLogDataFunc endSSLSSnLogDataFnPtr;
static GetIntfDataFunc getIntfDataFnPtr;

void registerUrlQuery(UrlQueryCreateFunc createFn, UrlQueryDestroyFunc destroyFn, UrlQueryMatchFunc matchFn)
{
    urlQueryCreateFnPtr = createFn;
    urlQueryDestroyFnPtr = destroyFn;
    urlQueryMatchFnPtr = matchFn;
}
static struct urlQueryContext* urlQueryCreate(const char *url)
{
    if (urlQueryCreateFnPtr)
    {
        return ((urlQueryCreateFnPtr)(url));
    }

    return NULL;
}
static void urlQueryDestroy(struct urlQueryContext *context)
{
    if (urlQueryDestroyFnPtr)
        (urlQueryDestroyFnPtr)(context);
}
static int urlQueryMatch(struct urlQueryContext *context, uint16_t inUrlCat, uint16_t inUrlMinRep, uint16_t inUrlMaxRep)
{
    if (urlQueryMatchFnPtr)
        return (urlQueryMatchFnPtr)(context, inUrlCat, inUrlMinRep, inUrlMaxRep);
    return -1;
}

static void registerUserGroupIdGet(UserGroupIdGetFunc userIdFn)
{
    userGroupIdGetFnPtr = userIdFn;
}
static int userGroupIdGet(const snort_ip *snortIp, uint32_t *userId, unsigned *groupIdArray, unsigned groupIdArrayLen)
{
    if (userGroupIdGetFnPtr)
        return (userGroupIdGetFnPtr)(snortIp, userId, groupIdArray, groupIdArrayLen);
    return -1;
}

static void registerGeoIpAddressLookup(GeoIpAddressLookupFunc fn)
{
    geoIpAddressLookupFnPtr = fn;
}
static int geoIpAddressLookup(const snort_ip *snortIp, uint16_t* geo)
{
    if (geoIpAddressLookupFnPtr)
        return (geoIpAddressLookupFnPtr)(snortIp, geo);
    return -1;
}

static void registerGetIntfData(GetIntfDataFunc fn)
{
    getIntfDataFnPtr = fn;
}

static void getIntfData(void *ssnptr, int32_t *ingressIntfIndex, int32_t *egressIntfIndex,
                int32_t *ingressZoneIndex, int32_t *egressZoneIndex)
{
    if (getIntfDataFnPtr)
    {
        (getIntfDataFnPtr)(ssnptr, ingressIntfIndex, egressIntfIndex, ingressZoneIndex, egressZoneIndex);
    }
}

static void registerUpdateSSLSSnLogData(UpdateSSLSSnLogDataFunc fn)
{
    updateSSLSSnLogDataFnPtr = fn;
}

static void updateSSLSSnLogData(void *ssnptr, uint8_t logging_on, uint8_t action_is_block, const char *ssl_cert_fingerprint,
    uint32_t ssl_cert_fingerprint_len, uint16_t ssl_cert_status, uint8_t *ssl_policy_id,
    uint32_t ssl_policy_id_len, uint32_t ssl_rule_id, uint16_t ssl_cipher_suite, uint8_t ssl_version,
    uint16_t ssl_actual_action, uint16_t ssl_expected_action, uint32_t ssl_url_category,
    uint16_t ssl_flow_status, uint32_t ssl_flow_error, uint32_t ssl_flow_messages,
    uint64_t ssl_flow_flags, char *ssl_server_name, uint8_t *ssl_session_id, uint8_t session_id_len,
    uint8_t *ssl_ticket_id, uint8_t ticket_id_len)
{
    if (updateSSLSSnLogDataFnPtr)
    {
        (updateSSLSSnLogDataFnPtr)(ssnptr, logging_on, action_is_block, ssl_cert_fingerprint,
                ssl_cert_fingerprint_len, ssl_cert_status, ssl_policy_id,
                ssl_policy_id_len, ssl_rule_id, ssl_cipher_suite, ssl_version,
                ssl_actual_action, ssl_expected_action, ssl_url_category,
                ssl_flow_status, ssl_flow_error, ssl_flow_messages,
                ssl_flow_flags, ssl_server_name, ssl_session_id, session_id_len, ssl_ticket_id, ticket_id_len);
    }
}


static void registerEndSSLSSnLogData(EndSSLSSnLogDataFunc fn)
{
    endSSLSSnLogDataFnPtr = fn;
}

static void endSSLSSnLogData(void *ssnptr, uint32_t ssl_flow_messages, uint64_t ssl_flow_flags)
{
    if (endSSLSSnLogDataFnPtr)
    {
        (endSSLSSnLogDataFnPtr)(ssnptr, ssl_flow_messages, ssl_flow_flags);
    }
}
static inline bool DynamicReadyForProcess (void* pkt)
{
    Packet *p = (Packet *)pkt;

    if ( ScPafEnabled() )
        return PacketHasPAFPayload(p);

    return !(p->packet_flags & PKT_STREAM_INSERT);
}

void DynamicSetSSLCallback(void *p)
{
    SetSSLCallback(p);
}

void *DynamicGetSSLCallback(void)
{
    return GetSSLCallback();
}
bool DynamicIsSSLPolicyEnabled(void)
{
    tSfPolicyId policy = getNapRuntimePolicy();
    return (snort_conf->targeted_policies[policy]->ssl_policy_enabled );
}

void DynamicSetSSLPolicyEnabled(struct _SnortConfig *sc, tSfPolicyId policy, bool value)
{
    sc->targeted_policies[policy]->ssl_policy_enabled = value;
}

int InitDynamicPreprocessors(void)
{
    DynamicPreprocessorData preprocData;

    preprocData.version = PREPROCESSOR_DATA_VERSION;
    preprocData.size = sizeof(DynamicPreprocessorData);

    preprocData.altBuffer = (SFDataBuffer *)&DecodeBuffer;
    preprocData.altDetect = (SFDataPointer *)&DetectBuffer;
    preprocData.fileDataBuf = (SFDataPointer *)&file_data_ptr;

    preprocData.logMsg = &LogMessage;
    preprocData.errMsg = &ErrorMessage;
    preprocData.fatalMsg = &FatalError;
    preprocData.debugMsg = &DebugMessageFunc;
#ifdef SF_WCHAR
    preprocData.debugWideMsg = &DebugWideMessageFunc;
#endif

    preprocData.registerPreproc = &RegisterPreprocessor;
#ifdef SNORT_RELOAD
    preprocData.getRelatedReloadData = GetRelatedReloadData;
#endif
    preprocData.addPreproc = &AddPreprocessor;
    preprocData.addPreprocAllPolicies = &AddPreprocessorAllPolicies;
    preprocData.addMetaEval = &AddMetaEval;
    preprocData.getSnortInstance = DynamicGetSnortInstance;
    preprocData.addPreprocExit = &AddFuncToPreprocCleanExitList;
    preprocData.addPreprocConfCheck = &AddPreprocessorCheck;
    preprocData.preprocOptRegister = &RegisterPreprocessorRuleOption;
    preprocData.addPreprocProfileFunc = &DynamicRegisterPreprocessorProfile;
    preprocData.profilingPreprocsFunc = &DynamicProfilingPreprocs;
#ifdef PERF_PROFILING
    preprocData.totalPerfStats = &totalPerfStats;
#else
    preprocData.totalPerfStats = NULL;
#endif

    preprocData.alertAdd = &SnortEventqAdd;
    preprocData.genSnortEvent = &GenerateSnortEvent;
    preprocData.thresholdCheck = &sfthreshold_test;
    preprocData.inlineDropAndReset = &DynamicDropReset;

    preprocData.detect = &DynamicDetect;
    preprocData.disableDetect = &DynamicDisableDetection;
    preprocData.disableAllDetect = &DynamicDisableAllDetection;
    preprocData.disablePacketAnalysis = &DynamicDisablePacketAnalysis;
    preprocData.enablePreprocessor = &DynamicEnablePreprocessor;

    preprocData.sessionAPI = session_api;
    preprocData.streamAPI = stream_api;
    preprocData.searchAPI = search_api;

    preprocData.config_file = &file_name;
    preprocData.config_line = &file_line;
    preprocData.printfappend = &sfsnprintfappend;
    preprocData.tokenSplit = &mSplit;
    preprocData.tokenFree = &mSplitFree;

    preprocData.getRuleInfoByName = &DynamicGetRuleClassByName;
    preprocData.getRuleInfoById = &DynamicGetRuleClassById;

    preprocData.preprocess = &DynamicPreprocess;

#ifdef DEBUG_MSGS
    preprocData.debugMsgFile = &DebugMessageFile;
    preprocData.debugMsgLine = &DebugMessageLine;
#else
    preprocData.debugMsgFile = &no_file;
    preprocData.debugMsgLine = &no_line;
#endif

    preprocData.registerPreprocStats = &RegisterPreprocStats;
    preprocData.addPreprocReset = &AddFuncToPreprocResetList;
    preprocData.addPreprocResetStats = &AddFuncToPreprocResetStatsList;
    preprocData.disablePreprocessors = &DynamicDisablePreprocessors;

    preprocData.ip6Build = &DynamicIP6Build;
    preprocData.ip6SetCallbacks = &DynamicIP6SetCallbacks;

    preprocData.logAlerts = &DynamicSnortEventqLog;
    preprocData.resetAlerts = &SnortEventqReset;
    preprocData.pushAlerts = SnortEventqPush;
    preprocData.popAlerts = SnortEventqPop;

#ifdef TARGET_BASED
    preprocData.findProtocolReference = &FindProtocolReference;
    preprocData.addProtocolReference = &AddProtocolReference;
#if defined(FEAT_OPEN_APPID)
    preprocData.findProtocolName = &FindProtocolName;
#endif /* defined(FEAT_OPEN_APPID) */
    preprocData.isAdaptiveConfigured = &IsAdaptiveConfigured;
    preprocData.isAdaptiveConfiguredForSnortConfig = &IsAdaptiveConfiguredForSnortConfig;
#endif

    preprocData.preprocOptOverrideKeyword = &RegisterPreprocessorRuleOptionOverride;
    preprocData.preprocOptByteOrderKeyword = &RegisterPreprocessorRuleOptionByteOrder;
    preprocData.isPreprocEnabled = &IsPreprocEnabled;

    preprocData.getNapRuntimePolicy = DynamicGetNapRuntimePolicy;
    preprocData.getIpsRuntimePolicy = DynamicGetIpsRuntimePolicy;
    preprocData.getParserPolicy = DynamicGetParserPolicy;
    preprocData.getDefaultPolicy = DynamicGetDefaultPolicy;
    preprocData.setParserPolicy = DynamicSetParserPolicy;
    preprocData.setFileDataPtr = DynamicSetFileDataPtr;
    preprocData.DetectReset = DynamicDetectResetPtr;
    preprocData.SetAltDecode = &DynamicSetAltDecode;
    preprocData.GetAltDetect = &DynamicGetAltDetect;
    preprocData.SetAltDetect = &DynamicSetAltDetect;
    preprocData.Is_DetectFlag = &DynamicIsDetectFlag;
    preprocData.DetectFlag_Disable = &DynamicDetectFlagDisable;
    preprocData.SnortStrtol = DynamicSnortStrtol;
    preprocData.SnortStrtoul = DynamicSnortStrtoul;
    preprocData.SnortStrnStr = DynamicSnortStrnStr;
    preprocData.SnortStrncpy = DynamicSnortStrncpy;
    preprocData.SnortStrnPbrk = DynamicSnortStrnPbrk;
    preprocData.SnortStrcasestr = DynamicSnortStrcasestr;

    preprocData.portObjectCharPortArray = PortObjectCharPortArray;
    preprocData.fpEvalRTN = DynamicEvalRTN;

    preprocData.obApi = obApi;

    preprocData.encodeNew = DynamicEncodeNew;
    preprocData.encodeDelete = DynamicEncodeDelete;
    preprocData.encodeFormat = DynamicEncodeFormat;
    preprocData.encodeUpdate = DynamicEncodeUpdate;

    preprocData.newGrinderPkt = DynamicNewGrinderPkt;
    preprocData.deleteGrinderPkt = DynamicDeleteGrinderPkt;

    preprocData.portObjectCharPortArray = PortObjectCharPortArray;

    preprocData.addDetect = &AddDetection;

    preprocData.getLogDirectory = DynamicGetLogDirectory;

    preprocData.controlSocketRegisterHandler = &ControlSocketRegisterHandler;

    preprocData.registerIdleHandler = &IdleProcessingRegisterHandler;

    preprocData.isPafEnabled = DynamicIsPafEnabled;

    preprocData.pktTime = DynamicPktTime;
    preprocData.getPktTimeOfDay = DynamicGetPktTimeOfDay;
#ifdef SIDE_CHANNEL
    preprocData.isSCEnabled = DynamicIsSCEnabled;
    preprocData.scRegisterRXHandler = &DynamicSCRegisterRXHandler;
    preprocData.scAllocMessageTX = &DynamicSCPreallocMessageTX;
    preprocData.scEnqueueMessageTX = &DynamicSCEnqueueMessageTX;
#endif

    preprocData.getPolicyFromId = &DynamicGetPolicyFromId;
    preprocData.changeNapRuntimePolicy = &DynamicChangeNapRuntimePolicy;
    preprocData.changeIpsRuntimePolicy = &DynamicChangeIpsRuntimePolicy;

    preprocData.inlineForceDropPacket = &DynamicForceDropPacket;
    preprocData.inlineForceDropAndReset = &DynamicForceDropReset;
#ifdef ACTIVE_RESPONSE
    preprocData.activeSetEnabled = &DynamicActiveSetEnabled;
#endif
    preprocData.SnortIsStrEmpty = DynamicSnortIsStrEmpty;
#ifdef ACTIVE_RESPONSE
    preprocData.dynamicSendBlockResponse = &DynamicSendBlockResponseMsg;
#endif
    preprocData.dynamicSetFlowId = &setFlowId;
    preprocData.addPeriodicCheck = &AddFuncToPeriodicCheckList;
    preprocData.addPostConfigFunc = &AddFuncToPreprocPostConfigList;
    preprocData.addFuncToPostConfigList = &AddFuncToPostConfigList;
    preprocData.snort_conf_dir = &snort_conf_dir;
    preprocData.addOutputModule = &output_load_module;
    preprocData.canWhitelist = DynamicCanWhitelist;
    preprocData.fileAPI = file_api;
    preprocData.disableAllPolicies = &DynamicDisableAllPolicies;
    preprocData.reenablePreprocBit = &DynamicReenablePreprocBitFunc;
    preprocData.checkValueInRange = &CheckValueInRange;

    preprocData.setHttpBuffer = SetHttpBuffer;
    preprocData.getHttpBuffer = getHttpBuffer;

#ifdef ACTIVE_RESPONSE
    preprocData.activeInjectData = &DynamicActiveInjectData;
#endif
    preprocData.inlineDropPacket = &DynamicDropPacket;
    preprocData.readyForProcess = &DynamicReadyForProcess;

    preprocData.getSSLCallback = &DynamicGetSSLCallback;
    preprocData.setSSLCallback = &DynamicSetSSLCallback;

    preprocData.sslAppIdLookup = &sslAppIdLookup;
    preprocData.registerSslAppIdLookup = &registerSslAppIdLookup;

    preprocData.getAppId = &getAppId;
    preprocData.registerGetAppId = &registerGetAppId;

    preprocData.urlQueryCreate = &urlQueryCreate;
    preprocData.urlQueryDestroy = &urlQueryDestroy;
    preprocData.urlQueryMatch = &urlQueryMatch;
    preprocData.registerUrlQuery = &registerUrlQuery;

    preprocData.userGroupIdGet = &userGroupIdGet;
    preprocData.registerUserGroupIdGet = &registerUserGroupIdGet;

    preprocData.geoIpAddressLookup = &geoIpAddressLookup;
    preprocData.registerGeoIpAddressLookup = &registerGeoIpAddressLookup;
    preprocData.updateSSLSSnLogData = &updateSSLSSnLogData;
    preprocData.registerUpdateSSLSSnLogData = &registerUpdateSSLSSnLogData;
    preprocData.endSSLSSnLogData = &endSSLSSnLogData;
    preprocData.registerEndSSLSSnLogData = &registerEndSSLSSnLogData;
    preprocData.getIntfData = &getIntfData;
    preprocData.registerGetIntfData = &registerGetIntfData;
    preprocData.isSSLPolicyEnabled = &DynamicIsSSLPolicyEnabled;
    preprocData.setSSLPolicyEnabled = &DynamicSetSSLPolicyEnabled;
    return InitDynamicPreprocessorPlugins(&preprocData);
}

int InitDynamicDetectionPlugins(SnortConfig *sc)
{
    DynamicDetectionPlugin *plugin;

    if (sc == NULL)
        return -1;

    VerifyDetectionPluginRequirements();

    plugin = loadedDetectionPlugins;
    while (plugin)
    {
        if (plugin->initFunc(sc))
        {
            ErrorMessage("Failed to initialize dynamic detection library: "
                    "%s version %d.%d.%d\n",
                    plugin->metaData.uniqueName,
                    plugin->metaData.major,
                    plugin->metaData.minor,
                    plugin->metaData.build);

            return -1;
        }

        plugin = plugin->next;
    }

    return 0;
}

int DumpDetectionLibRules(void)
{
    DynamicDetectionPlugin *plugin = loadedDetectionPlugins;
    DumpDetectionRules ruleDumpFunc = NULL;
    int retVal = 0;
    int dumped = 0;

    LogMessage("Dumping dynamic rules...\n");
    while (plugin)
    {
        ruleDumpFunc = (DumpDetectionRules) getSymbol(plugin->handle, "DumpSkeletonRules", &(plugin->metaData), NONFATAL);

        LogMessage("Dumping dynamic rules for Library %s %d.%d.%d\n",
            plugin->metaData.uniqueName,
            plugin->metaData.major,
            plugin->metaData.minor,
            plugin->metaData.build);
        if (ruleDumpFunc != NULL)
        {
            if (ruleDumpFunc())
            {
                LogMessage("Failed to dump the rules for Library %s %d.%d.%d\n",
                    plugin->metaData.uniqueName,
                    plugin->metaData.major,
                    plugin->metaData.minor,
                    plugin->metaData.build);
                dumped = 1;
            }
        }
        plugin = plugin->next;
    }
    if( dumped == 0)
    {
        LogMessage("  Finished dumping dynamic rules.\n");
    }
    return retVal;
}

int LoadDynamicPreprocessor(const char * const library_name, int indent)
{
    DynamicPluginMeta metaData;
    /* Presume here, that library name is full path */
    InitPreprocessorLibFunc preprocInit;
    PluginHandle handle;

    LogMessage("%sLoading dynamic preprocessor library %s... ",
               indent ? "  " : "", library_name);

    handle = openDynamicLibrary(library_name, 0);
    metaData.libraryPath = (char *) library_name;

    GetPluginVersion(handle, &metaData);

    /* Just to ensure that the function exists */
    preprocInit = (InitPreprocessorLibFunc) getSymbol(handle, "InitializePreprocessor", &metaData, FATAL);

    if (metaData.type != TYPE_PREPROCESSOR)
    {
        CloseDynamicLibrary(handle);
        LogMessage("failed, not a preprocessor library\n");
        return 0;
    }

    AddPreprocessorPlugin(handle, preprocInit, &metaData);

    LogMessage("done\n");
    return 0;
}

void LoadAllDynamicPreprocessors(const char * const path)
{
    LogMessage("Loading all dynamic preprocessor libs from %s...\n", path);
    LoadAllLibs(path, LoadDynamicPreprocessor);
    LogMessage("  Finished Loading all dynamic preprocessor libs from %s\n", path);
}

void CloseDynamicPreprocessorLibs(void)
{
    DynamicPreprocessorPlugin *tmpplugin, *plugin = loadedPreprocessorPlugins;
    while (plugin)
    {
        tmpplugin = plugin->next;
        CloseDynamicLibrary(plugin->handle);
        free(plugin->metaData.libraryPath);
        free(plugin);
        plugin = tmpplugin;
    }
    loadedPreprocessorPlugins = NULL;
}

void *GetNextEnginePluginVersion(void *p)
{
    DynamicEnginePlugin *lib = (DynamicEnginePlugin *) p;

    if ( lib != NULL )
    {
        lib = lib->next;
    }
    else
    {
        lib = loadedEngines;
    }

    if ( lib == NULL )
    {
        return lib;
    }

    return (void *) lib;
}

void *GetNextDetectionPluginVersion(void *p)
{
    DynamicDetectionPlugin *lib = (DynamicDetectionPlugin *) p;

    if ( lib != NULL )
    {
        lib = lib->next;
    }
    else
    {
        lib = loadedDetectionPlugins;
    }

    if ( lib == NULL )
    {
        return lib;
    }

    return (void *) lib;
}

void *GetNextPreprocessorPluginVersion(void *p)
{
    DynamicPreprocessorPlugin *lib = (DynamicPreprocessorPlugin *) p;

    if ( lib != NULL )
    {
        lib = lib->next;
    }
    else
    {
        lib = loadedPreprocessorPlugins;
    }

    if ( lib == NULL )
    {
        return lib;
    }

    return (void *) lib;
}

DynamicPluginMeta *GetDetectionPluginMetaData(void *p)
{
    DynamicDetectionPlugin *lib = (DynamicDetectionPlugin *) p;
    DynamicPluginMeta *meta;

    meta = &(lib->metaData);

    return meta;
}

DynamicPluginMeta *GetEnginePluginMetaData(void *p)
{
    DynamicEnginePlugin *lib = (DynamicEnginePlugin *) p;
    DynamicPluginMeta *meta;

    meta = &(lib->metaData);

    return meta;
}

DynamicPluginMeta *GetPreprocessorPluginMetaData(void *p)
{
    DynamicPreprocessorPlugin *lib = (DynamicPreprocessorPlugin *) p;
    DynamicPluginMeta *meta;

    meta = &(lib->metaData);

    return meta;
}

#ifdef SIDE_CHANNEL

/*
 * Dynamic Side Channel Plugin Support
 */

typedef struct _DynamicSideChannelPlugin
{
    PluginHandle handle;
    DynamicPluginMeta metaData;
    InitSideChannelLibFunc initFunc;
    struct _DynamicSideChannelPlugin *next;
    struct _DynamicSideChannelPlugin *prev;
} DynamicSideChannelPlugin;

static DynamicSideChannelPlugin *loadedSideChannelPlugins = NULL;

void AddSideChannelPlugin(PluginHandle handle,
                        InitSideChannelLibFunc initFunc,
                        DynamicPluginMeta *meta)
{
    DynamicSideChannelPlugin *newPlugin = NULL;
    newPlugin = (DynamicSideChannelPlugin *)SnortAlloc(sizeof(DynamicSideChannelPlugin));
    newPlugin->handle = handle;

    if (!loadedSideChannelPlugins)
    {
        loadedSideChannelPlugins = newPlugin;
    }
    else
    {
        newPlugin->next = loadedSideChannelPlugins;
        loadedSideChannelPlugins->prev = newPlugin;
        loadedSideChannelPlugins = newPlugin;
    }

    memcpy(&(newPlugin->metaData), meta, sizeof(DynamicPluginMeta));
    newPlugin->metaData.libraryPath = SnortStrdup(meta->libraryPath);
    newPlugin->initFunc = initFunc;
}

void RemoveSideChannelPlugin(DynamicSideChannelPlugin *plugin)
{
    if (!plugin)
        return;

    if (plugin == loadedSideChannelPlugins)
    {
        loadedSideChannelPlugins = loadedSideChannelPlugins->next;
        loadedSideChannelPlugins->prev = NULL;
    }
    else
    {
        if (plugin->prev)
            plugin->prev->next = plugin->next;
        if (plugin->next)
            plugin->next->prev = plugin->prev;
    }
    LogMessage("Unloading dynamic side channel library %s version %d.%d.%d\n",
            plugin->metaData.uniqueName,
            plugin->metaData.major,
            plugin->metaData.minor,
            plugin->metaData.build);
    CloseDynamicLibrary(plugin->handle);
    if (plugin->metaData.libraryPath != NULL)
        free(plugin->metaData.libraryPath);
    free(plugin);
}

int LoadDynamicSideChannelLib(const char * const library_name, int indent)
{
    DynamicPluginMeta metaData;
    /* Presume here, that library name is full path */
    InitSideChannelLibFunc sideChannelInit;
    PluginHandle handle;

    LogMessage("%sLoading dynamic side channel library %s... ",
               indent ? "  " : "", library_name);

    handle = openDynamicLibrary(library_name, 0);
    metaData.libraryPath = (char *) library_name;

    GetPluginVersion(handle, &metaData);

    /* Just to ensure that the function exists */
    sideChannelInit = (InitSideChannelLibFunc)getSymbol(handle, "InitializeSideChannel", &metaData, FATAL);

    if (!(metaData.type & TYPE_SIDE_CHANNEL))
    {
        CloseDynamicLibrary(handle);
        LogMessage("failed, not a side channel library\n");
        return 0;
    }

    AddSideChannelPlugin(handle, sideChannelInit, &metaData);

    LogMessage("done\n");
    return 0;
}

void CloseDynamicSideChannelLibs(void)
{
    DynamicSideChannelPlugin *tmpplugin, *plugin = loadedSideChannelPlugins;
    while (plugin)
    {
        tmpplugin = plugin->next;
        CloseDynamicLibrary(plugin->handle);
        free(plugin->metaData.libraryPath);
        free(plugin);
        plugin = tmpplugin;
    }
    loadedSideChannelPlugins = NULL;
}

void LoadAllDynamicSideChannelLibs(const char * const path)
{
    LogMessage("Loading all dynamic side channel libs from %s...\n", path);
    LoadAllLibs(path, LoadDynamicSideChannelLib);
    LogMessage("  Finished Loading all dynamic side channel libs from %s\n", path);
}

int InitDynamicSideChannelPlugins(void)
{
    DynamicSideChannelPlugin *plugin;
    DynamicSideChannelData sideChannelData;

    RemoveDuplicateSideChannelPlugins();

    sideChannelData.version = SIDE_CHANNEL_DATA_VERSION;
    sideChannelData.size = sizeof(DynamicSideChannelData);
    sideChannelData.registerModule = &RegisterSideChannelModule;
    sideChannelData.registerRXHandler = &SideChannelRegisterRXHandler;
    sideChannelData.registerTXHandler = &SideChannelRegisterTXHandler;
    sideChannelData.unregisterRXHandler = &SideChannelUnregisterRXHandler;
    sideChannelData.unregisterTXHandler = &SideChannelUnregisterTXHandler;
    sideChannelData.allocMessageRX = &SideChannelPreallocMessageRX;
    sideChannelData.allocMessageTX = &SideChannelPreallocMessageTX;
    sideChannelData.discardMessageRX = &SideChannelDiscardMessageRX;
    sideChannelData.discardMessageTX = &SideChannelDiscardMessageTX;
    sideChannelData.enqueueMessageRX = &SideChannelEnqueueMessageRX;
    sideChannelData.enqueueMessageTX = &SideChannelEnqueueMessageTX;
    sideChannelData.enqueueDataRX = &SideChannelEnqueueDataRX;
    sideChannelData.enqueueDataTX = &SideChannelEnqueueDataTX;

    sideChannelData.getSnortInstance = &DynamicGetSnortInstance;
    sideChannelData.snortSignalMask = &DynamicSnortSignalMask;

    sideChannelData.logMsg = &LogMessage;
    sideChannelData.errMsg = &ErrorMessage;
    sideChannelData.fatalMsg = &FatalError;
    sideChannelData.debugMsg = &DebugMessageFunc;

    plugin = loadedSideChannelPlugins;
    while (plugin)
    {
        if (plugin->initFunc(&sideChannelData))
        {
            ErrorMessage("Failed to initialize dynamic side channel library: %s version %d.%d.%d\n",
                    plugin->metaData.uniqueName,
                    plugin->metaData.major,
                    plugin->metaData.minor,
                    plugin->metaData.build);

            return -1;
        }

        plugin = plugin->next;
    }

    return 0;
}

void *GetNextSideChannelPluginVersion(void *p)
{
    DynamicSideChannelPlugin *lib = (DynamicSideChannelPlugin *) p;

    if (lib != NULL)
        lib = lib->next;
    else
        lib = loadedSideChannelPlugins;

    if (lib == NULL)
        return lib;

    return (void *) lib;
}

DynamicPluginMeta *GetSideChannelPluginMetaData(void *p)
{
    DynamicSideChannelPlugin *lib = (DynamicSideChannelPlugin *) p;
    DynamicPluginMeta *meta;

    meta = &(lib->metaData);

    return meta;
}

void RemoveDuplicateSideChannelPlugins(void)
{
    int removed = 0;
    DynamicSideChannelPlugin *lib1 = NULL;
    DynamicSideChannelPlugin *lib2 = NULL;
    DynamicPluginMeta *meta1;
    DynamicPluginMeta *meta2;

    /* Side Channel Plugins */
    do
    {
        removed = 0;
        lib1 = loadedSideChannelPlugins;
        while (lib1 != NULL)
        {
            lib2 = loadedSideChannelPlugins;
            while (lib2 != NULL)
            {
                /* Obviously, the same ones will be the same */
                if (lib1 != lib2)
                {
                    meta1 = &lib1->metaData;
                    meta2 = &lib2->metaData;
                    if (!strcmp(meta1->uniqueName, meta2->uniqueName))
                    {
                        /* Uh, same uniqueName. */
                        if ((meta1->major > meta2->major) ||
                            ((meta1->major == meta2->major) && (meta1->minor > meta2->minor)) ||
                            ((meta1->major == meta2->major) && (meta1->minor == meta2->minor) && (meta1->build > meta2->build)) )
                        {
                            /* Lib1 is newer */
                            RemoveSideChannelPlugin(lib2);
                            removed = 1;
                            break;
                        }
                        else if ((meta2->major > meta1->major) ||
                            ((meta2->major == meta1->major) && (meta2->minor > meta1->minor)) ||
                            ((meta2->major == meta1->major) && (meta2->minor == meta1->minor) && (meta2->build > meta1->build)) )
                        {
                            /* Lib2 is newer */
                            RemoveSideChannelPlugin(lib1);
                            removed = 1;
                            break;
                        }
                        else if ((meta1->major == meta2->major) && (meta1->minor == meta2->minor) && (meta1->build == meta2->build) )
                        {
                            /* Duplicate */
                            RemoveSideChannelPlugin(lib2);
                            removed = 1;
                            break;
                        }
                    }
                }
                /* If we removed anything, start back at the beginning */
                if (removed)
                    break;
                lib2 = lib2->next;
            }
            /* If we removed anything, start back at the beginning */
            if (removed)
                break;
            lib1 = lib1->next;
        }
    } while (removed);
}

#endif /* SIDE_CHANNEL */
