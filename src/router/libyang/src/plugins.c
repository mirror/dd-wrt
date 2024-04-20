/**
 * @file plugins.c
 * @author Radek Krejci <rkrejci@cesnet.cz>
 * @brief Manipulate with the type and extension plugins.
 *
 * Copyright (c) 2021 CESNET, z.s.p.o.
 *
 * This source code is licensed under BSD 3-Clause License (the "License").
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://opensource.org/licenses/BSD-3-Clause
 */

#define _GNU_SOURCE

#include "plugins.h"
#include "plugins_internal.h"

#include <assert.h>
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "config.h"
#include "plugins_exts.h"
#include "plugins_types.h"
#include "set.h"

/*
 * internal type plugins records
 */
extern const struct lyplg_type_record plugins_binary[];
extern const struct lyplg_type_record plugins_bits[];
extern const struct lyplg_type_record plugins_boolean[];
extern const struct lyplg_type_record plugins_decimal64[];
extern const struct lyplg_type_record plugins_empty[];
extern const struct lyplg_type_record plugins_enumeration[];
extern const struct lyplg_type_record plugins_identityref[];
extern const struct lyplg_type_record plugins_instanceid[];
extern const struct lyplg_type_record plugins_integer[];
extern const struct lyplg_type_record plugins_leafref[];
extern const struct lyplg_type_record plugins_string[];
extern const struct lyplg_type_record plugins_union[];

/*
 * ietf-inet-types
 */
extern const struct lyplg_type_record plugins_ipv4_address[];
extern const struct lyplg_type_record plugins_ipv4_address_no_zone[];
extern const struct lyplg_type_record plugins_ipv6_address[];
extern const struct lyplg_type_record plugins_ipv6_address_no_zone[];
extern const struct lyplg_type_record plugins_ipv4_prefix[];
extern const struct lyplg_type_record plugins_ipv6_prefix[];

/*
 * ietf-yang-types
 */
extern const struct lyplg_type_record plugins_date_and_time[];
extern const struct lyplg_type_record plugins_xpath10[];

/*
 * internal extension plugins records
 */
extern struct lyplg_ext_record plugins_metadata[];
extern struct lyplg_ext_record plugins_nacm[];
extern struct lyplg_ext_record plugins_yangdata[];

static pthread_mutex_t plugins_guard = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief Counter for currently present contexts able to refer to the loaded plugins.
 *
 * Plugins are shared among all the created contexts. They are loaded with the creation of the very first context and
 * unloaded with the destroy of the last context. Therefore, to reload the list of plugins, all the contexts must be
 * destroyed and with the creation of a first new context after that, the plugins will be reloaded.
 */
static uint32_t context_refcount = 0;

/**
 * @brief Record describing an implemented extension.
 *
 * Matches ::lyplg_ext_record and ::lyplg_type_record
 */
struct lyplg_record {
    const char *module;          /**< name of the module where the extension/type is defined */
    const char *revision;        /**< optional module revision - if not specified, the plugin applies to any revision,
                                      which is not an optimal approach due to a possible future revisions of the module.
                                      Instead, there should be defined multiple items in the plugins list, each with the
                                      different revision, but all with the same pointer to the plugin functions. The
                                      only valid use case for the NULL revision is the case the module has no revision. */
    const char *name;            /**< name of the extension/typedef */
    int8_t plugin[];             /**< specific plugin type's data - ::lyplg_ext or ::lyplg_type */
};

static struct ly_set plugins_handlers = {0};
static struct ly_set plugins_types = {0};
static struct ly_set plugins_extensions = {0};

/**
 * @brief Just a variadic data to cover extension and type plugins by a single ::plugins_load() function.
 *
 * The values are taken from ::LY_PLUGINS_EXTENSIONS and ::LYPLG_TYPES macros.
 */
static const struct {
    const char *id;          /**< string identifier: type/extension */
    const char *apiver_var;  /**< expected variable name holding API version value */
    const char *plugins_var; /**< expected variable name holding plugin records */
    const char *envdir;      /**< environment variable containing directory with the plugins */
    const char *dir;         /**< default directory with the plugins (has less priority than envdir) */
    uint32_t apiver;         /**< expected API version */
} plugins_load_info[] = {
    {   /* LYPLG_TYPE */
        .id = "type",
        .apiver_var = "plugins_types_apiver__",
        .plugins_var = "plugins_types__",
        .envdir = "LIBYANG_TYPES_PLUGINS_DIR",
        .dir = LYPLG_TYPE_DIR,
        .apiver = LYPLG_TYPE_API_VERSION
    }, {/* LYPLG_EXTENSION */
        .id = "extension",
        .apiver_var = "plugins_extensions_apiver__",
        .plugins_var = "plugins_extensions__",
        .envdir = "LIBYANG_EXTENSIONS_PLUGINS_DIR",
        .dir = LYPLG_EXT_DIR,
        .apiver = LYPLG_EXT_API_VERSION
    }
};

/**
 * @brief Iterate over list of loaded plugins of the given @p type.
 *
 * @param[in] type Type of the plugins to iterate.
 * @param[in,out] index The iterator - set to 0 for the first call.
 * @return The plugin records, NULL if no more record is available.
 */
static struct lyplg_record *
plugins_iter(enum LYPLG type, uint32_t *index)
{
    struct ly_set *plugins;

    assert(index);

    if (type == LYPLG_EXTENSION) {
        plugins = &plugins_extensions;
    } else {
        plugins = &plugins_types;
    }

    if (*index == plugins->count) {
        return NULL;
    }

    *index += 1;
    return plugins->objs[*index - 1];
}

void *
lyplg_find(enum LYPLG type, const char *module, const char *revision, const char *name)
{
    uint32_t i = 0;
    struct lyplg_record *item;

    assert(module);
    assert(name);

    while ((item = plugins_iter(type, &i)) != NULL) {
        if (!strcmp(item->module, module) && !strcmp(item->name, name)) {
            if (item->revision && revision && strcmp(item->revision, revision)) {
                continue;
            } else if (!revision && item->revision) {
                continue;
            }

            return &item->plugin;
        }
    }

    return NULL;
}

/**
 * @brief Insert the provided extension plugin records into the internal set of extension plugins for use by libyang.
 *
 * @param[in] recs An array of plugin records provided by the plugin implementation. The array must be terminated by a zeroed
 * record.
 * @return LY_SUCCESS in case of success
 * @return LY_EINVAL for invalid information in @p recs.
 * @return LY_EMEM in case of memory allocation failure.
 */
static LY_ERR
plugins_insert(enum LYPLG type, const void *recs)
{
    if (!recs) {
        return LY_SUCCESS;
    }

    if (type == LYPLG_EXTENSION) {
        const struct lyplg_ext_record *rec = (const struct lyplg_ext_record *)recs;

        for (uint32_t i = 0; rec[i].name; i++) {
            LY_CHECK_RET(ly_set_add(&plugins_extensions, (void *)&rec[i], 0, NULL));
        }
    } else { /* LYPLG_TYPE */
        const struct lyplg_type_record *rec = (const struct lyplg_type_record *)recs;

        for (uint32_t i = 0; rec[i].name; i++) {
            LY_CHECK_RET(ly_set_add(&plugins_types, (void *)&rec[i], 0, NULL));
        }
    }

    return LY_SUCCESS;
}

static void
lyplg_close_cb(void *handle)
{
    dlclose(handle);
}

static void
lyplg_clean_(void)
{
    if (--context_refcount) {
        /* there is still some other context, do not remove the plugins */
        return;
    }

    ly_set_erase(&plugins_types, NULL);
    ly_set_erase(&plugins_extensions, NULL);
    ly_set_erase(&plugins_handlers, lyplg_close_cb);
}

void
lyplg_clean(void)
{
    pthread_mutex_lock(&plugins_guard);
    lyplg_clean_();
    pthread_mutex_unlock(&plugins_guard);
}

/**
 * @brief Get the expected plugin objects from the loaded dynamic object and add the defined plugins into the lists of
 * available extensions/types plugins.
 *
 * @param[in] dlhandler Loaded dynamic library handler.
 * @param[in] pathname Path of the loaded library for logging.
 * @param[in] type Type of the plugins to get from the dynamic library. Note that a single library can hold both types
 * and extensions plugins implementations, so this function should be called twice (once for each plugin type) with
 * different @p type values
 * @return LY_ERR values.
 */
static LY_ERR
plugins_load(void *dlhandler, const char *pathname, enum LYPLG type)
{
    const void *plugins;
    uint32_t *version;

    /* type plugin */
    version = dlsym(dlhandler, plugins_load_info[type].apiver_var);
    if (version) {
        /* check version ... */
        if (*version != plugins_load_info[type].apiver) {
            LOGERR(NULL, LY_EINVAL, "Processing user %s plugin \"%s\" failed, wrong API version - %d expected, %d found.",
                    plugins_load_info[type].id, pathname, plugins_load_info[type].apiver, *version);
            return LY_EINVAL;
        }

        /* ... get types plugins information ... */
        if (!(plugins = dlsym(dlhandler, plugins_load_info[type].plugins_var))) {
            char *errstr = dlerror();
            LOGERR(NULL, LY_EINVAL, "Processing user %s plugin \"%s\" failed, missing %s plugins information (%s).",
                    plugins_load_info[type].id, pathname, plugins_load_info[type].id, errstr);
            return LY_EINVAL;
        }

        /* ... and load all the types plugins */
        LY_CHECK_RET(plugins_insert(type, plugins));
    }

    return LY_SUCCESS;
}

static LY_ERR
plugins_load_module(const char *pathname)
{
    LY_ERR ret = LY_SUCCESS;
    void *dlhandler;
    uint32_t types_count = 0, extensions_count = 0;

    dlerror();    /* Clear any existing error */

    dlhandler = dlopen(pathname, RTLD_NOW);
    if (!dlhandler) {
        LOGERR(NULL, LY_ESYS, "Loading \"%s\" as a plugin failed (%s).", pathname, dlerror());
        return LY_ESYS;
    }

    if (ly_set_contains(&plugins_handlers, dlhandler, NULL)) {
        /* the plugin is already loaded */
        LOGVRB("Plugin \"%s\" already loaded.", pathname);

        /* keep the correct refcount */
        dlclose(dlhandler);
        return LY_SUCCESS;
    }

    /* remember the current plugins lists for recovery */
    types_count = plugins_types.count;
    extensions_count = plugins_extensions.count;

    /* type plugin */
    ret = plugins_load(dlhandler, pathname, LYPLG_TYPE);
    LY_CHECK_GOTO(ret, error);

    /* extension plugin */
    ret = plugins_load(dlhandler, pathname, LYPLG_EXTENSION);
    LY_CHECK_GOTO(ret, error);

    /* remember the dynamic plugin */
    ret = ly_set_add(&plugins_handlers, dlhandler, 1, NULL);
    LY_CHECK_GOTO(ret, error);

    return LY_SUCCESS;

error:
    dlclose(dlhandler);

    /* revert changes in the lists */
    while (plugins_types.count > types_count) {
        ly_set_rm_index(&plugins_types, plugins_types.count - 1, NULL);
    }
    while (plugins_extensions.count > extensions_count) {
        ly_set_rm_index(&plugins_extensions, plugins_extensions.count - 1, NULL);
    }

    return ret;
}

static LY_ERR
plugins_insert_dir(enum LYPLG type)
{
    LY_ERR ret = LY_SUCCESS;
    const char *pluginsdir;
    DIR *dir;
    ly_bool default_dir = 0;

    /* try to get the plugins directory from environment variable */
    pluginsdir = getenv(plugins_load_info[type].envdir);
    if (!pluginsdir) {
        /* remember that we are going to a default dir and do not print warning if the directory doesn't exist */
        default_dir = 1;
        pluginsdir = plugins_load_info[type].dir;
    }

    dir = opendir(pluginsdir);
    if (!dir) {
        /* no directory (or no access to it), no extension plugins */
        if (!default_dir || (errno != ENOENT)) {
            LOGWRN(NULL, "Failed to open libyang %s plugins directory \"%s\" (%s).", plugins_load_info[type].id,
                    pluginsdir, strerror(errno));
        }
    } else {
        struct dirent *file;

        while ((file = readdir(dir))) {
            size_t len;
            char pathname[PATH_MAX];

            /* required format of the filename is *LYPLG_SUFFIX */
            len = strlen(file->d_name);
            if ((len < LYPLG_SUFFIX_LEN + 1) || strcmp(&file->d_name[len - LYPLG_SUFFIX_LEN], LYPLG_SUFFIX)) {
                continue;
            }

            /* and construct the filepath */
            snprintf(pathname, PATH_MAX, "%s/%s", pluginsdir, file->d_name);

            ret = plugins_load_module(pathname);
            if (ret) {
                break;
            }
        }
        closedir(dir);
    }

    return ret;
}

LY_ERR
lyplg_init(void)
{
    LY_ERR ret;

    pthread_mutex_lock(&plugins_guard);
    /* let only the first context to initiate plugins, but let others wait for finishing the initiation */
    if (context_refcount++) {
        /* already initiated */
        pthread_mutex_unlock(&plugins_guard);
        return LY_SUCCESS;
    }

    /* internal types */
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_binary), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_bits), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_boolean), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_decimal64), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_empty), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_enumeration), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_identityref), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_instanceid), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_integer), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_leafref), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_string), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_union), error);

    /* ietf-inet-types */
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_ipv4_address), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_ipv4_address_no_zone), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_ipv6_address), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_ipv6_address_no_zone), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_ipv4_prefix), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_ipv6_prefix), error);

    /* ietf-yang-types */
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_date_and_time), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_TYPE, plugins_xpath10), error);

    /* internal extensions */
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_EXTENSION, plugins_metadata), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_EXTENSION, plugins_nacm), error);
    LY_CHECK_GOTO(ret = plugins_insert(LYPLG_EXTENSION, plugins_yangdata), error);

    /* external types */
    LY_CHECK_GOTO(ret = plugins_insert_dir(LYPLG_TYPE), error);

    /* external extensions */
    LY_CHECK_GOTO(ret = plugins_insert_dir(LYPLG_EXTENSION), error);

    /* initiation done, wake-up possibly waiting threads creating another contexts */
    pthread_mutex_unlock(&plugins_guard);

    return LY_SUCCESS;

error:
    /* initiation was not successful - cleanup (and let others to try) */
    lyplg_clean_();
    pthread_mutex_unlock(&plugins_guard);

    if (ret == LY_EINVAL) {
        /* all the plugins here are internal, invalid record actually means an internal libyang error */
        ret = LY_EINT;
    }
    return ret;
}

API LY_ERR
lyplg_add(const char *pathname)
{
    LY_ERR ret = LY_SUCCESS;

    LY_CHECK_ARG_RET(NULL, pathname, LY_EINVAL);

    /* works only in case a context exists */
    pthread_mutex_lock(&plugins_guard);
    if (!context_refcount) {
        /* no context */
        pthread_mutex_unlock(&plugins_guard);
        LOGERR(NULL, LY_EDENIED, "To add a plugin, at least one context must exists.");
        return LY_EDENIED;
    }

    ret = plugins_load_module(pathname);

    pthread_mutex_unlock(&plugins_guard);

    return ret;
}
