/* module.c - Module management for libgcrypt.
 * Copyright (C) 2003 Free Software Foundation, Inc.
 *
 * This file is part of Libgcrypt.
 *
 * Libgcrypt is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser general Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * Libgcrypt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

#include <assert.h>
#include <config.h>
#include <errno.h>
#include "g10lib.h"

#define MODULE_ID_MIN 600

/* Internal function.  Generate a new, unique module ID for a module
   that should be inserted into the module chain starting at
   MODULES.  */
static gcry_err_code_t
_gcry_module_id_new (gcry_module_t modules, unsigned int *id_new)
{
  /* FIXME, what should be the ID of the first module registered by
     the user?  */
  unsigned int id_min = MODULE_ID_MIN, id_max = (unsigned int) -1, mod_id;
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  gcry_module_t module;

  /* Search for unused ID.  */
  for (mod_id = id_min; mod_id < id_max; mod_id++)
    {
      /* Search for a module with the current ID.  */
      for (module = modules; module; module = module->next)
	if (mod_id == module->mod_id)
	  break;

      if (! module)
	/* None found -> the ID is available for use.  */
	break;
    }

  if (mod_id < id_max)
    /* Done.  */
    *id_new = mod_id;
  else
    /* No free ID found.  */
    err = GPG_ERR_INTERNAL;

  return err;
}

/* Add a module specification to the list ENTRIES.  The new module has
   it's use-counter set to one.  */
gcry_err_code_t
_gcry_module_add (gcry_module_t *entries, unsigned int mod_id,
		  void *spec, gcry_module_t *module)
{
  gcry_err_code_t err = 0;
  gcry_module_t entry;

  if (! mod_id)
    err = _gcry_module_id_new (*entries, &mod_id);

  if (! err)
    {
      entry = gcry_malloc (sizeof (struct gcry_module));
      if (! entry)
	err = gpg_err_code_from_errno (errno);
    }

  if (! err)
    {
      /* Fill new module entry.  */
      entry->flags = 0;
      entry->counter = 1;
      entry->spec = spec;
      entry->mod_id = mod_id;

      /* Link it into the list.  */
      entry->next = *entries;
      entry->prevp = entries;
      if (*entries)
	(*entries)->prevp = &entry->next;
      *entries = entry;

      /* And give it to the caller.  */
      if (module)
	*module = entry;
    }
  return err;
}

/* Internal function.  Unlink CIPHER_ENTRY from the list of registered
   ciphers and destroy it.  */
static void
_gcry_module_drop (gcry_module_t entry)
{
  *entry->prevp = entry->next;
  if (entry->next)
    entry->next->prevp = entry->prevp;

  gcry_free (entry);
}

/* Lookup a module specification by it's ID.  After a successfull
   lookup, the module has it's resource counter incremented.  */
gcry_module_t 
_gcry_module_lookup_id (gcry_module_t entries, unsigned int mod_id)
{
  gcry_module_t entry;

  for (entry = entries; entry; entry = entry->next)
    if (entry->mod_id == mod_id)
      {
	entry->counter++;
	break;
      }

  return entry;
}

/* Lookup a module specification.  After a successfull lookup, the
   module has it's resource counter incremented.  FUNC is a function
   provided by the caller, which is responsible for identifying the
   wanted module.  */
gcry_module_t 
_gcry_module_lookup (gcry_module_t entries, void *data,
		     gcry_module_lookup_t func)
{
  gcry_module_t entry;

  for (entry = entries; entry; entry = entry->next)
    if ((*func) (entry->spec, data))
      {
	entry->counter++;
	break;
      }

  return entry;
}

/* Release a module.  In case the use-counter reaches zero, destroy
   the module.  Passing MODULE as NULL is a dummy operation (similar
   to free()). */
void
_gcry_module_release (gcry_module_t module)
{
  if (module && ! --module->counter)
    _gcry_module_drop (module);
}

/* Add a reference to a module.  */
void
_gcry_module_use (gcry_module_t module)
{
  ++module->counter;
}

/* If LIST is zero, write the number of modules identified by MODULES
   to LIST_LENGTH and return.  If LIST is non-zero, the first
   *LIST_LENGTH algorithm IDs are stored in LIST, which must be of
   according size.  In case there are less cipher modules than
   *LIST_LENGTH, *LIST_LENGTH is updated to the correct number.  */
gcry_err_code_t
_gcry_module_list (gcry_module_t modules,
		   int *list, int *list_length)
{
  gcry_err_code_t err = GPG_ERR_NO_ERROR;
  gcry_module_t module;
  int length, i;

  for (module = modules, length = 0; module; module = module->next, length++);

  if (list)
    {
      if (length > *list_length)
	length = *list_length;

      for (module = modules, i = 0; i < length; module = module->next, i++)
	list[i] = module->mod_id;

      if (length < *list_length)
	*list_length = length;
    }
  else
    *list_length = length;

  return err;
}
