/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#include "plugin_loader.h"
#include "olsrd_plugin.h"
#include "plugin_util.h"
#include "defs.h"
#include "olsr.h"

#include <dlfcn.h>

/* Local functions */
static int init_olsr_plugin(struct olsr_plugin *);
static int olsr_load_dl(char *, struct plugin_param *);
static int olsr_add_dl(struct olsr_plugin *);

static struct olsr_plugin *olsr_plugins = NULL;

/**
 *Function that loads all registered plugins
 *
 *@return the number of plugins loaded
 */
void
olsr_load_plugins(void)
{
  struct plugin_entry *entry = olsr_cnf->plugins;
  int rv = 0;
  for (entry = olsr_cnf->plugins; entry != NULL; entry = entry->next) {
    if (olsr_load_dl(entry->name, entry->params) < 0) {
      rv = 1;
    }
  }
  if (rv != 0) {
    olsr_exit("-- PLUGIN LOADING FAILED --", EXIT_FAILURE);
  }
  OLSR_PRINTF(0, "-- ALL PLUGINS LOADED! --\n\n");
}

/**
 *Try to load a shared library and extract
 *the required information
 *
 *@param libname the name of the library(file)
 *@param params plugin parameters
 *
 *@return negative on error
 */
static int
olsr_load_dl(char *libname, struct plugin_param *params)
{
#if defined TESTLIB_PATH && TESTLIB_PATH
  char path[256] = "/usr/testlib/";
#endif /* defined TESTLIB_PATH && TESTLIB_PATH */
  struct olsr_plugin *plugin = olsr_malloc(sizeof(struct olsr_plugin), "Plugin entry");
  int rv;

  OLSR_PRINTF(0, "---------- LOADING LIBRARY %s ----------\n", libname);

#if defined TESTLIB_PATH && TESTLIB_PATH
  strcat(path, libname);
  plugin->dlhandle = dlopen(path, RTLD_NOW);
#else /* defined TESTLIB_PATH && TESTLIB_PATH */
  plugin->dlhandle = dlopen(libname, RTLD_NOW);
#endif /* defined TESTLIB_PATH && TESTLIB_PATH */
  if (plugin->dlhandle == NULL) {
    const int save_errno = errno;
    OLSR_PRINTF(0, "DL loading failed: \"%s\"!\n", dlerror());
    free(plugin);
    errno = save_errno;
    return -1;
  }

  rv = olsr_add_dl(plugin);
  if (rv == -1) {
    const int save_errno = errno;
    dlclose(plugin->dlhandle);
    free(plugin);
    errno = save_errno;
  } else {
    plugin->params = params;

    /* Initialize the plugin */
    if (init_olsr_plugin(plugin) != 0) {
      rv = -1;
    }

    /* queue */
    plugin->next = olsr_plugins;
    olsr_plugins = plugin;
  }
  OLSR_PRINTF(0, "---------- LIBRARY %s %s ----------\n\n", libname, rv == 0 ? "LOADED" : "FAILED");
  return rv;
}

#if defined SUPPORT_OLD_PLUGIN_VERSIONS && SUPPORT_OLD_PLUGIN_VERSIONS
static int
try_old_versions(const struct olsr_plugin *plugin)
{
  get_interface_version_func get_interface_version;
  int *interface_version;

  OLSR_PRINTF(1, "trying v2 detection... ");
  get_interface_version = dlsym(plugin->dlhandle, "get_plugin_interface_version");
  if (get_interface_version != NULL) {
    return get_interface_version();
  }

  OLSR_PRINTF(1, "trying v1 detection... ");
  interface_version = dlsym(plugin->dlhandle, "plugin_interface_version");
  if (interface_version != NULL) {
    return *interface_version;
  }
  OLSR_PRINTF(0, "FAILED: \"%s\"\n", dlerror());
  return -1;
}
#else /* defined SUPPORT_OLD_PLUGIN_VERSIONS && SUPPORT_OLD_PLUGIN_VERSIONS */
#define try_old_versions(plugin) -1
#endif /* defined SUPPORT_OLD_PLUGIN_VERSIONS && SUPPORT_OLD_PLUGIN_VERSIONS */

static int
olsr_add_dl(struct olsr_plugin *plugin)
{
  get_interface_version_func get_interface_version;
  get_plugin_parameters_func get_plugin_parameters;
  int plugin_interface_version;

  /* Fetch the interface version function, 3 different ways */
  OLSR_PRINTF(0, "Checking plugin interface version: ");
  get_interface_version = dlsym(plugin->dlhandle, "olsrd_plugin_interface_version");
  if (get_interface_version == NULL) {
    plugin_interface_version = try_old_versions(plugin);
  } else {
    plugin_interface_version = get_interface_version();
  }
  if (plugin_interface_version == -1) {
    OLSR_PRINTF(0, "FAILED: \"%s\"\n", dlerror());
    return -1;
  }
  OLSR_PRINTF(0, " %d - OK\n", plugin_interface_version);

  if (plugin_interface_version < 5) {
    /* old plugin interface */
    OLSR_PRINTF(0,
                "\nWARNING: YOU ARE USING AN OLD DEPRECATED PLUGIN INTERFACE!\n"
                "DETECTED VERSION %d AND THE CURRENT VERSION IS %d\n" "PLEASE UPGRADE YOUR PLUGIN!\n", plugin_interface_version,
                MOST_RECENT_PLUGIN_INTERFACE_VERSION);
#if defined SUPPORT_OLD_PLUGIN_VERSIONS && SUPPORT_OLD_PLUGIN_VERSIONS
    OLSR_PRINTF(0, "WILL CONTINUE IN 5 SECONDS...\n\n");
    sleep(5);
#else /* defined SUPPORT_OLD_PLUGIN_VERSIONS && SUPPORT_OLD_PLUGIN_VERSIONS */
    return -1;
#endif /* defined SUPPORT_OLD_PLUGIN_VERSIONS && SUPPORT_OLD_PLUGIN_VERSIONS */
  }
#if defined SUPPORT_OLD_PLUGIN_VERSIONS && SUPPORT_OLD_PLUGIN_VERSIONS
  /* new plugin interface */
  if (plugin_interface_version < LAST_SUPPORTED_PLUGIN_INTERFACE_VERSION) {
    OLSR_PRINTF(0,
                "\n\nWARNING: VERSION MISMATCH!\n" "DETECTED %d AND LAST SUPPORTED VERSION IS %d\n"
                "THIS CAN CAUSE UNEXPECTED BEHAVIOUR AND CRASHES!\n" "WILL CONTINUE IN 5 SECONDS...\n\n", get_interface_version(),
                LAST_SUPPORTED_PLUGIN_INTERFACE_VERSION);
    sleep(5);
  }
#endif /* defined SUPPORT_OLD_PLUGIN_VERSIONS && SUPPORT_OLD_PLUGIN_VERSIONS */

  /* Fetch the init function */
  OLSR_PRINTF(1, "Trying to fetch plugin init function: ");
  plugin->plugin_init = dlsym(plugin->dlhandle, "olsrd_plugin_init");
  if (plugin->plugin_init == NULL) {
    OLSR_PRINTF(0, "FAILED: \"%s\"\n", dlerror());
    return -1;
  }
  OLSR_PRINTF(1, "OK\n");

  OLSR_PRINTF(1, "Trying to fetch parameter table and it's size... \n");

  get_plugin_parameters = dlsym(plugin->dlhandle, "olsrd_get_plugin_parameters");
  if (get_plugin_parameters != NULL) {
    (*get_plugin_parameters) (&plugin->plugin_parameters, &plugin->plugin_parameters_size);
  } else {
#if defined SUPPORT_OLD_PLUGIN_VERSIONS && SUPPORT_OLD_PLUGIN_VERSIONS
    /* Fetch the parameter function */
    OLSR_PRINTF(1, "Trying to fetch param function: ");

    plugin->register_param = dlsym(plugin->dlhandle, "olsrd_plugin_register_param");
    if (plugin->register_param == NULL) {
      OLSR_PRINTF(0, "FAILED: \"%s\"\n", dlerror());
      return -1;
    } else {
      OLSR_PRINTF(1, "OK\n");
    }

    plugin->plugin_parameters = NULL;
    plugin->plugin_parameters_size = 0;
#else /* defined SUPPORT_OLD_PLUGIN_VERSIONS && SUPPORT_OLD_PLUGIN_VERSIONS */
    OLSR_PRINTF(0, "Old plugin interfaces are not supported\n");
    return -1;
#endif /* defined SUPPORT_OLD_PLUGIN_VERSIONS && SUPPORT_OLD_PLUGIN_VERSIONS */
  }
  return 0;
}

/**
 *Initialize a loaded plugin
 *This includes sending information
 *from olsrd to the plugin and
 *register the functions from the plugin with olsrd
 *
 *@param entry the plugin to initialize
 *
 *@return -1 if there was an error
 */
static int
init_olsr_plugin(struct olsr_plugin *entry)
{
  int rv = 0;
  struct plugin_param *params;
  OLSR_PRINTF(1, "Sending parameters...\n");
  for (params = entry->params; params != NULL; params = params->next) {
    OLSR_PRINTF(1, "\"%s\"/\"%s\"... ", params->key, params->value);
    if (entry->plugin_parameters_size != 0) {
      unsigned int i;
      int rc = 0;
      for (i = 0; i < entry->plugin_parameters_size; i++) {
        if (0 == entry->plugin_parameters[i].name[0] || 0 == strcasecmp(entry->plugin_parameters[i].name, params->key)) {
          /* we have found it! */
          rc =
            entry->plugin_parameters[i].set_plugin_parameter(params->value, entry->plugin_parameters[i].data,
                                                             0 == entry->plugin_parameters[i].name[0]
                                                             ? (set_plugin_parameter_addon)
                                                             params->key : entry->plugin_parameters[i].addon);
          if (rc != 0) {
            fprintf(stderr, "\nFatal error in plugin parameter \"%s\"/\"%s\"\n", params->key, params->value);
            rv = -1;
          }
          break;
        }
      }
      if (i >= entry->plugin_parameters_size) {
        OLSR_PRINTF(0, "Ignored parameter \"%s\"\n", params->key);
      } else {
        OLSR_PRINTF(1, "%s: %s\n", params->key, rc == 0 ? "OK" : "FAILED");
        if (rc != 0) {
          rv = -1;
        }
      }
#if defined SUPPORT_OLD_PLUGIN_VERSIONS && SUPPORT_OLD_PLUGIN_VERSIONS
    } else if (entry->register_param != NULL) {
      int rc;
      OLSR_PRINTF(0, "Registering parameter \"%s\": ", params->key);
      rc = entry->register_param(params->key, params->value);
      if (rc < 0) {
        char buf[1024];
        snprintf(buf, sizeof(buf), "Fatal error in plugin parameter \"%s\"/\"%s\"", params->key, params->value);
        olsr_exit(buf, EXIT_FAILURE);
      }
      OLSR_PRINTF(0, "%s\n", rc == 0 ? "FAILED" : "OK");
#endif /* defined SUPPORT_OLD_PLUGIN_VERSIONS && SUPPORT_OLD_PLUGIN_VERSIONS */
    } else {
      OLSR_PRINTF(0, "I don't know what to do with \"%s\"!\n", params->key);
      rv = -1;
    }
  }

  OLSR_PRINTF(1, "Running plugin_init function...\n");
  entry->plugin_init();
  return rv;
}

/**
 *Close all loaded plugins
 */
void
olsr_close_plugins(void)
{
  struct olsr_plugin *entry;

  OLSR_PRINTF(0, "Closing plugins...\n");
  for (entry = olsr_plugins; entry != NULL; entry = entry->next) {
    dlclose(entry->dlhandle);
    entry->dlhandle = NULL;
  }
}

/*
 * Local Variables:
 * mode: c
 * style: linux
 * c-basic-offset: 4
 * indent-tabs-mode: nil
 * End:
 */
