/*
 * The olsr.org Optimized Link-State Routing daemon(olsrd)
 * Copyright (c) 2004, Andreas Tønnesen(andreto@olsr.org)
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
 * $Id: plugin_loader.c,v 1.18 2005/03/06 19:33:35 kattemat Exp $
 */

#include "plugin_loader.h"
#include "defs.h"
#include "olsr.h"
/* Local functions */

static void
init_olsr_plugin(struct olsr_plugin *);

static int
olsr_load_dl(char *, struct plugin_param *);

static struct olsr_plugin *olsr_plugins;


/**
 *Function that loads all registered plugins
 *
 *@return the number of plugins loaded
 */
int
olsr_load_plugins()
{
  struct plugin_entry *entry;
  int loaded;

  entry = olsr_cnf->plugins;
  loaded = 0;

  OLSR_PRINTF(1, "Loading plugins...\n\n")

  while(entry)
    {  
      if(olsr_load_dl(entry->name, entry->params) < 0)
	OLSR_PRINTF(1, "-- PLUGIN LOADING FAILED! --\n\n")
      else
	loaded ++;

      entry = entry->next;
    }
  return loaded;
}


/**
 *Try to load a shared library and extract
 *the required information
 *
 *@param libname the name of the library(file)
 *
 *@return negative on error
 */
int
olsr_load_dl(char *libname, struct plugin_param *params)
{
  struct olsr_plugin new_entry, *entry;
  int (*get_interface_version)(void);
  int *interface_version;

  OLSR_PRINTF(1, "---------- Plugin loader ----------\nLibrary: %s\n", libname)

  if((new_entry.dlhandle = dlopen(libname, RTLD_NOW)) == NULL)
    {
      OLSR_PRINTF(1, "DL loading failed: \"%s\"!\n", dlerror())
      return -1;
    }

  /* Fetch the interface version function */
  OLSR_PRINTF(1, "Checking plugin interface version....")
  if((get_interface_version = dlsym(new_entry.dlhandle, "get_plugin_interface_version")) == NULL)
    {
      OLSR_PRINTF(1, "trying v1 detection...")
      if((interface_version = dlsym(new_entry.dlhandle, "plugin_interface_version")) == NULL)
	{
	  OLSR_PRINTF(1, "FAILED: \"%s\"\n", dlerror())
	  dlclose(new_entry.dlhandle);
	  return -1;
	}
      else
	{
	  OLSR_PRINTF(1, " %d - ", *interface_version)
	  if(*interface_version != PLUGIN_INTERFACE_VERSION)
	    OLSR_PRINTF(1, "WARNING: VERSION MISSMATCH!\n")
	  else
	    OLSR_PRINTF(1, "OK\n")
	}
    }
  else
    {
      OLSR_PRINTF(1, " %d - ", get_interface_version())
      if(get_interface_version() != PLUGIN_INTERFACE_VERSION)
	OLSR_PRINTF(1, "WARNING: VERSION MISSMATCH!\n")
      else
	OLSR_PRINTF(1, "OK\n")
    }

  OLSR_PRINTF(1, "Trying to fetch register function....")
  
  if((new_entry.register_olsr_data = dlsym(new_entry.dlhandle, "register_olsr_data")) == NULL)
    {
      /* This function must be present */
      OLSR_PRINTF(1, "\nCould not find function registration function in plugin!\n%s\nCRITICAL ERROR - aborting!\n", dlerror())
      dlclose(new_entry.dlhandle);
      return -1;
    }
  OLSR_PRINTF(1, "OK\n")


  /* Fetch the multipurpose function */
  OLSR_PRINTF(1, "Trying to fetch plugin IO function....")
  if((new_entry.plugin_io = dlsym(new_entry.dlhandle, "plugin_io")) == NULL)
    OLSR_PRINTF(1, "FAILED: \"%s\"\n", dlerror())
  else
    OLSR_PRINTF(1, "OK\n")

  /* Fetch the parameter function */
  OLSR_PRINTF(1, "Trying to fetch param function....")
  if((new_entry.register_param = dlsym(new_entry.dlhandle, "register_olsr_param")) == NULL)
    OLSR_PRINTF(1, "FAILED: \"%s\"\n", dlerror())
  else
    OLSR_PRINTF(1, "OK\n")


  entry = olsr_malloc(sizeof(struct olsr_plugin), "Plugin entry");

  memcpy(entry, &new_entry, sizeof(struct olsr_plugin));

  entry->params = params;

  /* Initialize the plugin */
  init_olsr_plugin(entry);

  /* queue */
  entry->next = olsr_plugins;
  olsr_plugins = entry;

  OLSR_PRINTF(1, "---------- LIBRARY LOADED ----------\n\n")

  return 0;
}



/**
 *Initialize a loaded plugin
 *This includes sending information
 *from olsrd to the plugin and
 *register the functions from the flugin with olsrd
 *
 *@param entry the plugin to initialize
 *
 *@return nada
 */
void
init_olsr_plugin(struct olsr_plugin *entry)
{
  struct olsr_plugin_data plugin_data;
  struct plugin_param *params = entry->params;
  int retval;

  if(entry->register_param)
    {
      OLSR_PRINTF(1, "Sending parameters...\n")
      while(params)
	{
	  OLSR_PRINTF(1, "\"%s\"/\"%s\".... ", params->key, params->value)
	  if((retval = entry->register_param(params->key, params->value)) < 0)
	    {
	      fprintf(stderr, "\nFatal error in plugin parameter \"%s\"/\"%s\"\n", params->key, params->value);
	      exit(EXIT_FAILURE);
	    }
	  OLSR_PRINTF(1, "%s\n", retval == 0 ? "FAILED" : "OK")

	  params = params->next;
	}
    }

  OLSR_PRINTF(1, "Running registration function...\n")
  /* Fill struct */
  plugin_data.ipversion = olsr_cnf->ip_version;
  plugin_data.main_addr = &main_addr;

  plugin_data.olsr_plugin_io = &olsr_plugin_io;

  /* Register data with plugin */
  entry->register_olsr_data(&plugin_data);

}



/**
 *Close all loaded plugins
 */
void
olsr_close_plugins()
{
  struct olsr_plugin *entry;

  OLSR_PRINTF(1, "Closing plugins...\n")
  for(entry = olsr_plugins; 
      entry != NULL ; 
      entry = entry->next)
    {
      dlclose(&entry->dlhandle);
    }

}
