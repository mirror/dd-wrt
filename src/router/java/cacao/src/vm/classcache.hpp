/* src/vm/classcache.hpp - loaded class cache and loading constraints

   Copyright (C) 1996-2013
   CACAOVM - Verein zur Foerderung der freien virtuellen Maschine CACAO

   This file is part of CACAO.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2, or (at
   your option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
   02110-1301, USA.

*/


#ifndef CLASSCACHE_HPP_
#define CLASSCACHE_HPP_ 1

#include "config.h"

#include "vm/types.hpp"

#include <stdio.h>  /* for FILE */

#include "toolbox/hashtable.hpp"

#include "vm/loader.hpp"

struct classinfo;

/* forward declarations *******************************************************/

typedef struct classcache_name_entry classcache_name_entry;
typedef struct classcache_class_entry classcache_class_entry;
typedef struct classcache_loader_entry classcache_loader_entry;

/* global variables ***********************************************************/

extern hashtable hashtable_classcache;


/* structs ********************************************************************/

/*----------------------------------------------------------------------------*/
/* The Loaded Class Cache                                                     */
/*                                                                            */
/* The loaded class cache is implemented as a two-level data structure.       */
/*                                                                            */
/* The first level is a hash table indexed by class names. For each class     */
/* name in the cache there is a classcache_name_entry, which collects all     */
/* information about classes with this class name.                            */
/*                                                                            */
/* Second level: For each classcache_name_entry there is a list of            */
/* classcache_class_entry:s representing the possible different resolutions   */
/* of the class name.                                                         */
/*                                                                            */
/* A classcache_class_entry records the following:                            */
/*                                                                            */
/* - the loaded class object, if this entry has been resolved, otherwise NULL */
/* - the list of initiating loaders which have resolved the class name to     */
/*   this class object                                                        */
/* - the list of initiating loaders which are constrained to resolve this     */
/*   class name to this class object in the future                            */
/*                                                                            */
/* The classcache_class_entry:s approximate the equivalence classes created   */
/* by the loading constraints and the equivalence of loaded classes.          */
/*                                                                            */
/* When a loading constraint (loaderA,loaderB,NAME) is added, then the        */
/* classcache_class_entry:s for NAME containing loaderA and loaderB resp.     */
/* must be merged into one entry. If this is impossible, because the entries  */
/* have already been resolved to different class objects, then the constraint */
/* is violated and an expception must be thrown.                              */
/*----------------------------------------------------------------------------*/


/* classcache_name_entry
 *
 * For each classname a classcache_name_entry struct is created.
 */

struct classcache_name_entry
{
	Utf8String               name;        /* class name                       */
	classcache_name_entry   *hashlink;    /* link for external chaining       */
	classcache_class_entry  *classes;     /* equivalence classes for this name*/
};

struct classcache_class_entry
{
	classinfo               *classobj;    /* the loaded class object, or NULL */
	classcache_loader_entry *loaders;
	classcache_loader_entry *constraints;
	classcache_class_entry  *next;        /* next class entry for same name   */
};

struct classcache_loader_entry
{
	classloader_t            *loader;     /* class loader object              */
	classcache_loader_entry  *next;       /* next loader entry in the list    */
};


/* callback function type for  classcache_foreach_loaded_class */

typedef void (*classcache_foreach_functionptr_t)(classinfo *, void *);


/* function prototypes ********************************************************/

/* initialize the loaded class cache */
bool classcache_init(void);
void classcache_free(void);

classinfo * classcache_lookup(classloader_t *initloader,Utf8String classname);
classinfo * classcache_lookup_defined(classloader_t *defloader,Utf8String classname);
classinfo * classcache_lookup_defined_or_initiated(classloader_t *loader,Utf8String classname);

bool classcache_store_unique(classinfo *cls);
classinfo * classcache_store(classloader_t *initloader,classinfo *cls,bool mayfree);
classinfo * classcache_store_defined(classinfo *cls);

#if defined(ENABLE_VERIFIER)
bool classcache_add_constraint(classloader_t *a,classloader_t *b,Utf8String classname);
bool classcache_add_constraints_for_params(classloader_t *a,classloader_t *b,
										   methodinfo *m);
#endif

s4 classcache_get_loaded_class_count(void);

void classcache_foreach_loaded_class(classcache_foreach_functionptr_t func,
									 void *data);

#ifndef NDEBUG
void classcache_debug_dump(FILE *file,Utf8String only);
#endif

#endif // CLASSCACHE_HPP_


/*
 * These are local overrides for various environment variables in Emacs.
 * Please do not remove this and leave it at the end of the file, where
 * Emacs will automagically detect them.
 * ---------------------------------------------------------------------
 * Local variables:
 * mode: c++
 * indent-tabs-mode: t
 * c-basic-offset: 4
 * tab-width: 4
 * End:
 * vim:noexpandtab:sw=4:ts=4:
 */

