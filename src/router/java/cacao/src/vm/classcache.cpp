/* src/vm/classcache.cpp - loaded class cache and loading constraints

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


#include "config.h"

#include <assert.h>

#include "vm/types.hpp"

#include "mm/memory.hpp"

#include "threads/lock.hpp"
#include "threads/mutex.hpp"

#include "toolbox/hashtable.hpp"
#include "toolbox/logging.hpp"
#include "toolbox/buffer.hpp"

#include "vm/class.hpp"
#include "vm/classcache.hpp"
#include "vm/descriptor.hpp"
#include "vm/exceptions.hpp"
#include "vm/options.hpp"
#include "vm/method.hpp"
#include "vm/utf8.hpp"

/*************************************************************************

  Class Cache

  The classcache has two functions:
  
  	1) caching the resolution of class references
	2) storing and checking loading constraints

  We will use the following terms in this description:

  	N          a class name: a utf string
	(N,L)      a class reference with initiating loader L and class name N
	C          a class (object): the result of resolving a reference (N,L)
               We will write resultion as
			        C = *(N,L)
	(N,L1,L2)  a loading constraint indicating that (N,L1) and (N,L2) must
	           resolve to the same class C. So (N,L1,L2) means
			        *(N,L1) = *(N,L2)

  The functions of the classcache require:

    1) a mapping (N,L) |--> C for looking up prior resolution results.
	2) storing the current set of loading constraints { (N,L1,L2) }

  These functions can be rearranged like that:

    a mapping N |--> (a mapping L |--> C or NULL, 
	                  a set of constraints {(L1,L2)})

  Thus we can treat the mapping and constraints for each name N
  separately. The implementation does this by keeping a hash table
  mapping a name N to a `classcache_name_entry` which contains all
  info with respect to N.

  For a class name N we can define an equivalence relation ~N~ on
  class loaders:

  	L1 ~N~ L2  <==>  *(N,L1) = *(N,L2)

  A loading constraint (N,L1,L2) implies L1 ~N~ L2.

  Also, if two references (N,L1) and (N,L2) resolve to the same class C
  we have L1 ~N~ L2 because class loaders are required to return
  consistent resolutions for a name N [XXX].

  A `classcache_name_entry` keeps a set of tuples { (Cx,IL,CL) },
  where
  		Cx...is a class C or NULL
		IL...is the set of initiating loaders
		CL...is the set of constrained loaders
		
  Such a tuple is called `classcache_class_entry` in the source code.

  The following holds for each tuple (Cx,IL,CL):

    .  (Cx is NULL) implies IL = {}.
	   
	.  If Cx is a class, IL is the set of loaders that have been
	   recorded as initiating loaders for Cx. IL may be the
	   empty set {} in case Cx has already been defined but no
	   initiating loader has been recorded, yet.
  
    .  (IL u CL) is a subset of an equivalence class of ~N~.

		 (This means that all loaders in IL and CL must resolve
		 the name N to the same class.)

  The following holds for the set of tuples { (Cx,IL,CL) }:

    .  For a given class C there is at most one tuple with Cx = C
	   in the set. (There may be an arbitrary number of tuples
	   with Cx = NULL, however.)

	.  For a given loader L there is at most one tuple with
	   L in (IL u CL).

  The implementation stores sets of loaders as linked lists of
  `classcache_loader_entry`s.

  Comments about manipulating the classcache can be found in the
  individual functions below.
 
*************************************************************************/


/* initial number of slots in the classcache hash table */
#define CLASSCACHE_INIT_SIZE  2048

/*============================================================================*/
/* DEBUG HELPERS                                                              */
/*============================================================================*/

/* #define CLASSCACHE_VERBOSE */

/*============================================================================*/
/* STATISTICS                                                                 */
/*============================================================================*/

/*#define CLASSCACHE_STATS*/

#ifdef CLASSCACHE_STATS
static int stat_classnames_stored = 0;
static int stat_classes_stored = 0;
static int stat_trivial_constraints = 0;
static int stat_nontriv_constraints = 0;
static int stat_nontriv_constraints_both = 0;
static int stat_nontriv_constraints_merged = 0;
static int stat_nontriv_constraints_one = 0;
static int stat_nontriv_constraints_none = 0;
static int stat_new_loader_entry = 0;
static int stat_merge_class_entries = 0;
static int stat_merge_loader_entries = 0;
static int stat_lookup = 0;
static int stat_lookup_class_entry_checked = 0;
static int stat_lookup_loader_checked = 0;
static int stat_lookup_name = 0;
static int stat_lookup_name_entry = 0;
static int stat_lookup_name_notfound = 0;
static int stat_lookup_new_name = 0;
static int stat_lookup_new_name_entry = 0;
static int stat_lookup_new_name_collisions = 0;
static int stat_rehash_names = 0;
static int stat_rehash_names_collisions = 0;

#define CLASSCACHE_COUNT(cnt)  (cnt)++
#define CLASSCACHE_COUNTIF(cond,cnt)  do{if(cond) (cnt)++;} while(0)

void classcache_print_statistics(FILE *file) {
	fprintf(file,"classnames stored   : %8d\n",stat_classnames_stored);
	fprintf(file,"classes stored      : %8d\n",stat_classes_stored);
	fprintf(file,"trivial constraints : %8d\n",stat_trivial_constraints);
	fprintf(file,"non-triv constraints: %8d\n",stat_nontriv_constraints);
	fprintf(file,"   both loaders rec.: %8d\n",stat_nontriv_constraints_both);
	fprintf(file,"       merged       : %8d\n",stat_nontriv_constraints_merged);
	fprintf(file,"   one loader rec.  : %8d\n",stat_nontriv_constraints_one);
	fprintf(file,"   no loaders rec.  : %8d\n",stat_nontriv_constraints_none);
	fprintf(file,"new loader entries  : %8d\n",stat_new_loader_entry);
	fprintf(file,"merge class entries : %8d\n",stat_merge_class_entries);
	fprintf(file,"merge loader entries: %8d\n",stat_merge_loader_entries);
	fprintf(file,"lookups             : %8d\n",stat_lookup);
	fprintf(file,"   class entries ckd: %8d\n",stat_lookup_class_entry_checked);
	fprintf(file,"   loader checked   : %8d\n",stat_lookup_loader_checked);
	fprintf(file,"lookup name         : %8d\n",stat_lookup_name);
	fprintf(file,"   entries checked  : %8d\n",stat_lookup_name_entry);
	fprintf(file,"   not found        : %8d\n",stat_lookup_name_notfound);
	fprintf(file,"lookup (new) name   : %8d\n",stat_lookup_new_name);
	fprintf(file,"   entries checked  : %8d\n",stat_lookup_new_name_entry);
	fprintf(file,"   new collisions   : %8d\n",stat_lookup_new_name_collisions);
	fprintf(file,"names rehashed      : %8d times\n",stat_rehash_names);
	fprintf(file,"    collisions      : %8d\n",stat_rehash_names_collisions);
}
#else
#define CLASSCACHE_COUNT(cnt)
#define CLASSCACHE_COUNTIF(cond,cnt)
#endif

/*============================================================================*/
/* THREAD-SAFE LOCKING                                                        */
/*============================================================================*/

	/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */
	/* CAUTION: The static functions below are */
	/*          NOT synchronized!              */
	/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! */

#define CLASSCACHE_LOCK()      classcache_hashtable_mutex->lock();
#define CLASSCACHE_UNLOCK()    classcache_hashtable_mutex->unlock();


/*============================================================================*/
/* GLOBAL VARIABLES                                                           */
/*============================================================================*/

hashtable hashtable_classcache;

static Mutex *classcache_hashtable_mutex;


/*============================================================================*/
/*                                                                            */
/*============================================================================*/

/* prototypes */

static void classcache_free_class_entry(classcache_class_entry *clsen);
static void classcache_remove_class_entry(classcache_name_entry *en,
										  classcache_class_entry *clsen);


/* classcache_init *************************************************************
 
   Initialize the class cache

   Note: NOT synchronized!
  
*******************************************************************************/

bool classcache_init(void)
{
	TRACESUBSYSTEMINITIALIZATION("classcache_init");

	/* create the hashtable */

	hashtable_create(&hashtable_classcache, CLASSCACHE_INIT_SIZE);

	/* create utf hashtable mutex */

	classcache_hashtable_mutex = new Mutex();

	/* everything's ok */

	return true;
}

/* classcache_new_loader_entry *************************************************
 
   Create a new classcache_loader_entry struct
   (internally used helper function)
  
   IN:
       loader...........the ClassLoader object
	   next.............the next classcache_loader_entry

   RETURN VALUE:
       the new classcache_loader_entry
  
*******************************************************************************/

static classcache_loader_entry * classcache_new_loader_entry(
									classloader_t * loader,
									classcache_loader_entry * next)
{
	classcache_loader_entry *lden;

	lden = NEW(classcache_loader_entry);
	lden->loader = loader;
	lden->next = next;
	CLASSCACHE_COUNT(stat_new_loader_entry);

	return lden;
}

/* classcache_merge_loaders ****************************************************
 
   Merge two lists of loaders into one
   (internally used helper function)
  
   IN:
       lista............first list (may be NULL)
	   listb............second list (may be NULL)

   RETURN VALUE:
       the merged list (may be NULL)

   NOTE:
       The lists given as arguments are destroyed!
  
*******************************************************************************/

static classcache_loader_entry * classcache_merge_loaders(
									classcache_loader_entry * lista,
									classcache_loader_entry * listb)
{
	classcache_loader_entry *result;
	classcache_loader_entry *ldenA;
	classcache_loader_entry *ldenB;
	classcache_loader_entry **chain;

	CLASSCACHE_COUNT(stat_merge_loader_entries);

	/* XXX This is a quadratic algorithm. If this ever
	 * becomes a problem, the loader lists should be
	 * stored as sorted lists and merged in linear time. */

	result = NULL;
	chain = &result;

	for (ldenA = lista; ldenA; ldenA = ldenA->next) {

		for (ldenB = listb; ldenB; ldenB = ldenB->next) {
			if (ldenB->loader == ldenA->loader)
				goto common_element;
		}

		/* this loader is only in lista */
		*chain = ldenA;
		chain = &(ldenA->next);

	  common_element:
		/* XXX free the duplicated element */
		;
	}

	/* concat listb to the result */
	*chain = listb;

	return result;
}

/* classcache_merge_class_entries **********************************************
 
   Merge two `classcache_class_entry`s into one.
   (internally used helper function)
  
   IN:
       en...............the classcache_name_entry containing both class entries
       clsenA...........first class entry, will receive the result
	   clsenB...........second class entry

   PRE-CONDITION:
       Either both entries must have the same classobj, or one of them has
	   classobj == NULL.

   NOTE:
       clsenB is freed by this function!
  
*******************************************************************************/

static void classcache_merge_class_entries(classcache_name_entry *en,
										   classcache_class_entry *clsenA,
										   classcache_class_entry *clsenB)
{
#ifdef CLASSCACHE_VERBOSE
	Buffer<> logbuffer;
#endif
	
	assert(en);
	assert(clsenA);
	assert(clsenB);
	assert(!clsenA->classobj || !clsenB->classobj || clsenA->classobj == clsenB->classobj);

#ifdef CLASSCACHE_VERBOSE
	logbuffer.writef("classcache_merge_class_entries(%p,%p->%p,%p->%p) ", 
			(void*)en,(void*)clsenA,(void*)clsenA->classobj,(void*)clsenB,(void*)clsenB->classobj);
	if (clsenA->classobj)
		logbuffer.write_slash_to_dot(clsenA->classobj->name);
	if (clsenB->classobj)
		logbuffer.write_slash_to_dot(clsenB->classobj->name);
	log_println(logbuffer);
#endif

	CLASSCACHE_COUNT(stat_merge_class_entries);

	/* clsenB will be merged into clsenA */
	clsenA->loaders = classcache_merge_loaders(clsenA->loaders, clsenB->loaders);
	clsenB->loaders = NULL; /* these have been freed or reused */

	clsenA->constraints = classcache_merge_loaders(clsenA->constraints,
												   clsenB->constraints);
	clsenB->constraints = NULL; /* these have been freed or reused */

	if (!clsenA->classobj)
		clsenA->classobj = clsenB->classobj;

	/* remove clsenB from the list of class entries */
	classcache_remove_class_entry(en, clsenB);
}


/* classcache_lookup_name ******************************************************
 
   Lookup a name in the first level of the cache
   (internally used helper function)
   
   IN:
       name.............the name to look up
  
   RETURN VALUE:
       a pointer to the classcache_name_entry for this name, or
       null if no entry was found.
	   
*******************************************************************************/

static classcache_name_entry *classcache_lookup_name(Utf8String name)
{
	classcache_name_entry *c;           /* hash table element                 */
	u4 key;                             /* hashkey computed from classname    */
	u4 slot;                            /* slot in hashtable                  */

	CLASSCACHE_COUNT(stat_lookup_name);

	key  = name.hash();
	slot = key & (hashtable_classcache.size - 1);
	c    = (classcache_name_entry*) hashtable_classcache.ptr[slot];

	/* search external hash chain for the entry */

	while (c) {
		/* entry found in hashtable */
		CLASSCACHE_COUNT(stat_lookup_name_entry);

		if (c->name == name)
			return c;

		c = c->hashlink;                    /* next element in external chain */
	}

	/* not found */

	CLASSCACHE_COUNT(stat_lookup_name_notfound);
	return NULL;
}


/* classcache_new_name *********************************************************
 
   Return a classcache_name_entry for the given name. The entry is created
   if it is not already in the cache.
   (internally used helper function)
   
   IN:
       name.............the name to look up / create an entry for
  
   RETURN VALUE:
       a pointer to the classcache_name_entry for this name
	   
*******************************************************************************/

static classcache_name_entry *classcache_new_name(Utf8String name)
{
	classcache_name_entry *c;	/* hash table element */
	u4 key;						/* hashkey computed from classname */
	u4 slot;					/* slot in hashtable               */
	u4 i;

	CLASSCACHE_COUNT(stat_lookup_new_name);

	key  = name.hash();
	slot = key & (hashtable_classcache.size - 1);
	c    = (classcache_name_entry*) hashtable_classcache.ptr[slot];

	/* search external hash chain for the entry */

	while (c) {
		/* entry found in hashtable */
		CLASSCACHE_COUNT(stat_lookup_new_name_entry);

		if (c->name == name)
			return c;

		c = c->hashlink;                    /* next element in external chain */
	}

	/* location in hashtable found, create new entry */

	c = NEW(classcache_name_entry);

	c->name = name;
	c->classes = NULL;

	/* insert entry into hashtable */
	c->hashlink = (classcache_name_entry *) hashtable_classcache.ptr[slot];
	CLASSCACHE_COUNTIF(c->hashlink,stat_lookup_new_name_collisions);
	hashtable_classcache.ptr[slot] = c;

	/* update number of hashtable-entries */
	hashtable_classcache.entries++;
	CLASSCACHE_COUNT(stat_classnames_stored);

	if ((hashtable_classcache.entries*2) > hashtable_classcache.size) {
		/* reorganization of hashtable */ 

		classcache_name_entry *c2;
		hashtable newhash;		/* the new hashtable */

		CLASSCACHE_COUNT(stat_rehash_names);

		/* create new hashtable, double the size */

		hashtable_create(&newhash, hashtable_classcache.size * 2);
		newhash.entries = hashtable_classcache.entries;

		/* transfer elements to new hashtable */

		for (i = 0; i < hashtable_classcache.size; i++) {
			c2 = (classcache_name_entry *) hashtable_classcache.ptr[i];
			while (c2) {
				classcache_name_entry *nextc = c2->hashlink;
				u4 newslot = c2->name.hash() & (newhash.size - 1);

				c2->hashlink = (classcache_name_entry *) newhash.ptr[newslot];
				CLASSCACHE_COUNTIF(c2->hashlink,stat_rehash_names_collisions);
				newhash.ptr[newslot] = c2;

				c2 = nextc;
			}
		}

		/* dispose old table */

		MFREE(hashtable_classcache.ptr, void *, hashtable_classcache.size);
		hashtable_classcache = newhash;
	}

	return c;
}


/* classcache_lookup ***********************************************************
 
   Lookup a possibly loaded class
  
   IN:
       initloader.......initiating loader for resolving the class name
       classname........class name to look up
  
   RETURN VALUE:
       The return value is a pointer to the cached class object,
       or NULL, if the class is not in the cache.

   Note: synchronized with global tablelock
   
*******************************************************************************/

classinfo *classcache_lookup(classloader_t *initloader, Utf8String classname)
{
	classcache_name_entry *en;
	classcache_class_entry *clsen;
	classcache_loader_entry *lden;
	classinfo *cls = NULL;

	CLASSCACHE_LOCK();

	CLASSCACHE_COUNT(stat_lookup);
	en = classcache_lookup_name(classname);

	if (en) {
		/* iterate over all class entries */

		for (clsen = en->classes; clsen; clsen = clsen->next) {
			CLASSCACHE_COUNT(stat_lookup_class_entry_checked);
			/* check if this entry has been loaded by initloader */

			for (lden = clsen->loaders; lden; lden = lden->next) {
				CLASSCACHE_COUNT(stat_lookup_loader_checked);
				if (lden->loader == initloader) {
					/* found the loaded class entry */

					assert(clsen->classobj);
					cls = clsen->classobj;
					goto found;
				}
			}
		}
	}

  found:
	CLASSCACHE_UNLOCK();
	return cls;
}


/* classcache_lookup_defined ***************************************************
 
   Lookup a class with the given name and defining loader
  
   IN:
       defloader........defining loader
       classname........class name
  
   RETURN VALUE:
       The return value is a pointer to the cached class object,
       or NULL, if the class is not in the cache.
   
*******************************************************************************/

classinfo *classcache_lookup_defined(classloader_t *defloader, Utf8String classname)
{
	classcache_name_entry *en;
	classcache_class_entry *clsen;
	classinfo *cls = NULL;

	CLASSCACHE_LOCK();

	en = classcache_lookup_name(classname);

	if (en) {
		/* iterate over all class entries */
		for (clsen = en->classes; clsen; clsen = clsen->next) {
			if (!clsen->classobj)
				continue;

			/* check if this entry has been defined by defloader */
			if (clsen->classobj->classloader == defloader) {
				cls = clsen->classobj;
				goto found;
			}
		}
	}

  found:
	CLASSCACHE_UNLOCK();
	return cls;
}


/* classcache_lookup_defined_or_initiated **************************************
 
   Lookup a class that has been defined or initiated by the given loader
  
   IN:
       loader...........defining or initiating loader
       classname........class name to look up
  
   RETURN VALUE:
       The return value is a pointer to the cached class object,
       or NULL, if the class is not in the cache.

   Note: synchronized with global tablelock
   
*******************************************************************************/

classinfo *classcache_lookup_defined_or_initiated(classloader_t *loader, 
												  Utf8String classname)
{
	classcache_name_entry *en;
	classcache_class_entry *clsen;
	classcache_loader_entry *lden;
	classinfo *cls = NULL;

	CLASSCACHE_LOCK();

	en = classcache_lookup_name(classname);

	if (en) {
		/* iterate over all class entries */

		for (clsen = en->classes; clsen; clsen = clsen->next) {

			/* check if this entry has been defined by loader */
			if (clsen->classobj && clsen->classobj->classloader == loader) {
				cls = clsen->classobj;
				goto found;
			}
			
			/* check if this entry has been initiated by loader */
			for (lden = clsen->loaders; lden; lden = lden->next) {
				if (lden->loader == loader) {
					/* found the loaded class entry */

					assert(clsen->classobj);
					cls = clsen->classobj;
					goto found;
				}
			}
		}
	}

  found:
	CLASSCACHE_UNLOCK();
	return cls;
}


/* classcache_store ************************************************************
   
   Store a loaded class. If a class of the same name has already been stored
   with the same initiating loader, then the given class CLS is freed (if
   possible) and the previously stored class is returned.
  
   IN:
       initloader.......initiating loader used to load the class
	                    (may be NULL indicating the bootstrap loader)
       cls..............class object to cache
	   mayfree..........true if CLS may be freed in case another class is
	                    returned
  
   RETURN VALUE:
       cls..............everything ok, the class was stored in the cache,
	   other classinfo..another class with the same (initloader,name) has been
	                    stored earlier. CLS has been freed[1] and the earlier
						stored class is returned.
       NULL.............an exception has been thrown.
   
   Note: synchronized with global tablelock

   [1]...in case MAYFREE is true
   
*******************************************************************************/

classinfo *classcache_store(classloader_t *initloader, classinfo *cls,
							bool mayfree)
{
	classcache_name_entry *en;
	classcache_class_entry *clsen;
	classcache_class_entry *clsenB;
	classcache_loader_entry *lden;
#ifdef CLASSCACHE_VERBOSE
	Buffer<> logbuffer;
#endif
	
	assert(cls);
	assert(cls->state & CLASS_LOADED);

	CLASSCACHE_LOCK();

#ifdef CLASSCACHE_VERBOSE
	logbuffer.writef("classcache_store (%p,%d,%p=", (void*)initloader,mayfree,(void*)cls)
	         .write_slash_to_dot(cls->name)
	         .write(")");
	log_println(logbuffer);
#endif

	en = classcache_new_name(cls->name);

	assert(en);

	/* iterate over all class entries */
	for (clsen = en->classes; clsen; clsen = clsen->next) {

		/* check if this entry has already been loaded by initloader */
		for (lden = clsen->loaders; lden; lden = lden->next) {
			if (lden->loader == initloader) {
			   if (clsen->classobj != cls) {
					/* A class with the same (initloader,name) pair has been stored already. */
					/* We free the given class and return the earlier one.                   */
#ifdef CLASSCACHE_VERBOSE
					log_println("replacing %p with earlier loaded class %p",cls,clsen->classobj);
#endif
					assert(clsen->classobj);
					if (mayfree)
						class_free(cls);
					cls = clsen->classobj;
			   }
			   goto return_success;
			}
		}

		/* {This entry has not been resolved with initloader} */

		/* check if initloader is constrained to this entry */
		for (lden = clsen->constraints; lden; lden = lden->next) {
			if (lden->loader == initloader) {
				/* we have to use this entry. check if it has been resolved */
				if (clsen->classobj) {
					/* check if is has already been resolved to another class */
					if (clsen->classobj != cls) {
						/* a loading constraint is violated */
						exceptions_throw_linkageerror("loading constraint violated: ", cls);
						goto return_exception;
					}

					/* record initloader as initiating loader */
					clsen->loaders = classcache_new_loader_entry(initloader, clsen->loaders);
					goto return_success;
				}

				/* {this is the first resolution for this entry} */
				/* record initloader as initiating loader */
				clsen->loaders = classcache_new_loader_entry(initloader, clsen->loaders);

				/* maybe we can merge this entry with another one */
				for (clsenB = en->classes; clsenB; clsenB = clsenB->next) {
					/* we dont want the entry that we have already */
					if (clsenB->classobj == cls) {
						/* this entry has the same classobj. let's merge them */
						classcache_merge_class_entries(en,clsen,clsenB);
						goto return_success;
					}
				}

				/* record the loaded class object */
				clsen->classobj = cls;
				CLASSCACHE_COUNT(stat_classes_stored);

				/* done */
				goto return_success;
			}
		}

	}

	/* {There is no class entry containing initloader as initiating 
	 *  or constrained loader.} */

	/* we look for a class entry with the same classobj we want to store */
	for (clsen = en->classes; clsen; clsen = clsen->next) {
		if (clsen->classobj == cls) {
			/* this entry is about the same classobj. let's use it */
			/* check if this entry has already been loaded by initloader */
			for (lden = clsen->loaders; lden; lden = lden->next) {
				if (lden->loader == initloader)
					goto return_success;
			}
			clsen->loaders = classcache_new_loader_entry(initloader, clsen->loaders);
			goto return_success;
		}
	}

	/* create a new class entry for this class object with */
	/* initiating loader initloader                        */

	clsen = NEW(classcache_class_entry);
	clsen->classobj = cls;
	clsen->loaders = classcache_new_loader_entry(initloader, NULL);
	clsen->constraints = NULL;

	clsen->next = en->classes;
	en->classes = clsen;
	CLASSCACHE_COUNT(stat_classes_stored);

  return_success:
#ifdef CLASSCACHE_VERBOSE
	classcache_debug_dump(stdout,cls->name);
#endif
	CLASSCACHE_UNLOCK();
	return cls;

  return_exception:
	CLASSCACHE_UNLOCK();
	return NULL;				/* exception */
}

/* classcache_store_unique *****************************************************
   
   Store a loaded class as loaded by the bootstrap loader. This is a wrapper 
   aroung classcache_store that throws an exception if a class with the same 
   name has already been loaded by the bootstrap loader.

   This function is used to register a few special classes during startup.
   It should not be used otherwise.
  
   IN:
       cls..............class object to cache
  
   RETURN VALUE:
       true.............everything ok, the class was stored.
       false............an exception has been thrown.
   
   Note: synchronized with global tablelock
   
*******************************************************************************/

bool classcache_store_unique(classinfo *cls)
{
	classinfo *result;

	result = classcache_store(NULL,cls,false);
	if (result == NULL)
		return false;

	if (result != cls) {
		exceptions_throw_internalerror("class already stored in the class cache");
		return false;
	}

	return true;
}

/* classcache_store_defined ****************************************************
   
   Store a loaded class after it has been defined. If the class has already
   been defined by the same defining loader in another thread, free the given
   class and returned the one which has been defined earlier.
  
   IN:
       cls..............class object to store. classloader must be set
	                    (classloader may be NULL, for bootloader)
  
   RETURN VALUE:
       cls..............everything ok, the class was stored the cache,
	   other classinfo..the class had already been defined, CLS was freed, the
	                    class which was defined earlier is returned,
       NULL.............an exception has been thrown.
   
*******************************************************************************/

classinfo *classcache_store_defined(classinfo *cls)
{
	classcache_name_entry *en;
	classcache_class_entry *clsen;
#ifdef CLASSCACHE_VERBOSE
	Buffer<> logbuffer;
#endif

	assert(cls);
	assert(cls->state & CLASS_LOADED);

	CLASSCACHE_LOCK();

#ifdef CLASSCACHE_VERBOSE
	logbuffer.writef("classcache_store_defined (%p,", (void*)cls->classloader)
	         .write_slash_to_dot(cls->name)
	         .write(")");
	log_println(logbuffer);
#endif

	en = classcache_new_name(cls->name);

	assert(en);

	/* iterate over all class entries */
	for (clsen = en->classes; clsen; clsen = clsen->next) {
		
		/* check if this class has been defined by the same classloader */
		if (clsen->classobj && clsen->classobj->classloader == cls->classloader) {
			/* we found an earlier definition, delete the newer one */
			/* (if it is a different classinfo)                     */
			if (clsen->classobj != cls) {
#ifdef CLASSCACHE_VERBOSE
				log_println("replacing %p with earlier defined class %p",cls,clsen->classobj);
#endif
				class_free(cls);
				cls = clsen->classobj;
			}
			goto return_success;
		}
	}

	/* create a new class entry for this class object */
	/* the list of initiating loaders is empty at this point */

	clsen = NEW(classcache_class_entry);
	clsen->classobj = cls;
	clsen->loaders = NULL;
	clsen->constraints = NULL;

	clsen->next = en->classes;
	en->classes = clsen;
	CLASSCACHE_COUNT(stat_classes_stored);

return_success:
#ifdef CLASSCACHE_VERBOSE
	classcache_debug_dump(stdout,cls->name);
#endif
	CLASSCACHE_UNLOCK();
	return cls;
}

/* classcache_find_loader ******************************************************
 
   Find the class entry loaded by or constrained to a given loader
   (internally used helper function)
  
   IN:
       entry............the classcache_name_entry
       loader...........the loader to look for
  
   RETURN VALUE:
       the classcache_class_entry for the given loader, or
	   NULL if no entry was found
   
*******************************************************************************/

#if defined(ENABLE_VERIFIER)
static classcache_class_entry * classcache_find_loader(
									classcache_name_entry * entry,
									classloader_t * loader)
{
	classcache_class_entry *clsen;
	classcache_loader_entry *lden;

	assert(entry);

	/* iterate over all class entries */
	for (clsen = entry->classes; clsen; clsen = clsen->next) {

		/* check if this entry has already been loaded by initloader */
		for (lden = clsen->loaders; lden; lden = lden->next) {
			if (lden->loader == loader)
				return clsen;	/* found */
		}

		/* check if loader is constrained to this entry */
		for (lden = clsen->constraints; lden; lden = lden->next) {
			if (lden->loader == loader)
				return clsen;	/* found */
		}
	}

	/* not found */
	return NULL;
}
#endif

/* classcache_free_class_entry *************************************************
 
   Free the memory used by a class entry
  
   IN:
       clsen............the classcache_class_entry to free  
	   
*******************************************************************************/

static void classcache_free_class_entry(classcache_class_entry * clsen)
{
	classcache_loader_entry *lden;
	classcache_loader_entry *next;

	assert(clsen);

	for (lden = clsen->loaders; lden; lden = next) {
		next = lden->next;
		FREE(lden, classcache_loader_entry);
	}
	for (lden = clsen->constraints; lden; lden = next) {
		next = lden->next;
		FREE(lden, classcache_loader_entry);
	}

	FREE(clsen, classcache_class_entry);
}

/* classcache_remove_class_entry ***********************************************
 
   Remove a classcache_class_entry from the list of possible resolution of
   a name entry
   (internally used helper function)
  
   IN:
       entry............the classcache_name_entry
       clsen............the classcache_class_entry to remove
  
*******************************************************************************/

static void classcache_remove_class_entry(classcache_name_entry * entry,
										  classcache_class_entry * clsen)
{
	classcache_class_entry **chain;

	assert(entry);
	assert(clsen);

	chain = &(entry->classes);
	while (*chain) {
		if (*chain == clsen) {
			*chain = clsen->next;
			classcache_free_class_entry(clsen);
			return;
		}
		chain = &((*chain)->next);
	}
}

/* classcache_free_name_entry **************************************************
 
   Free the memory used by a name entry
  
   IN:
       entry............the classcache_name_entry to free  
	   
*******************************************************************************/

static void classcache_free_name_entry(classcache_name_entry * entry)
{
	classcache_class_entry *clsen;
	classcache_class_entry *next;

	assert(entry);

	for (clsen = entry->classes; clsen; clsen = next) {
		next = clsen->next;
		classcache_free_class_entry(clsen);
	}

	FREE(entry, classcache_name_entry);
}

/* classcache_free *************************************************************
 
   Free the memory used by the class cache

   NOTE:
       The class cache may not be used any more after this call, except
	   when it is reinitialized with classcache_init.
  
   Note: NOT synchronized!
  
*******************************************************************************/

void classcache_free(void)
{
	u4 slot;
	classcache_name_entry *entry;
	classcache_name_entry *next;

	for (slot = 0; slot < hashtable_classcache.size; ++slot) {
		for (entry = (classcache_name_entry *) hashtable_classcache.ptr[slot]; entry; entry = next) {
			next = entry->hashlink;
			classcache_free_name_entry(entry);
		}
	}

	MFREE(hashtable_classcache.ptr, void*, hashtable_classcache.size);
	hashtable_classcache.size = 0;
	hashtable_classcache.entries = 0;
	hashtable_classcache.ptr = NULL;
}

/* classcache_add_constraint ***************************************************
 
   Add a loading constraint
  
   IN:
       a................first initiating loader
       b................second initiating loader
       classname........class name
  
   RETURN VALUE:
       true.............everything ok, the constraint has been added,
       false............an exception has been thrown.
   
   Note: synchronized with global tablelock
   
*******************************************************************************/

#if defined(ENABLE_VERIFIER)
bool classcache_add_constraint(classloader_t * a,
							   classloader_t * b,
							   Utf8String  classname)
{
	classcache_name_entry *en;
	classcache_class_entry *clsenA;
	classcache_class_entry *clsenB;

	assert(classname);

#ifdef CLASSCACHE_VERBOSE
	log_start();
	log_print("classcache_add_constraint(%p,%p,", (void *) a, (void *) b);
	utf_fprint_printable_ascii_classname(stdout, classname);
	log_print(")\n");
	log_finish();
#endif

	/* a constraint with a == b is trivially satisfied */
	if (a == b) {
		CLASSCACHE_COUNT(stat_trivial_constraints);
		return true;
	}

	CLASSCACHE_LOCK();

	en = classcache_new_name(classname);

	assert(en);
	CLASSCACHE_COUNT(stat_nontriv_constraints);

	/* find the entry loaded by / constrained to each loader */
	clsenA = classcache_find_loader(en, a);
	clsenB = classcache_find_loader(en, b);

	if (clsenA && clsenB) {
		/* { both loaders have corresponding entries } */
		CLASSCACHE_COUNT(stat_nontriv_constraints_both);

		/* if the entries are the same, the constraint is already recorded */
		if (clsenA == clsenB)
			goto return_success;

		/* check if the entries can be merged */
		if (clsenA->classobj && clsenB->classobj
			&& clsenA->classobj != clsenB->classobj) {
			/* no, the constraint is violated */
			exceptions_throw_linkageerror("loading constraint violated: ",
										  clsenA->classobj);
			goto return_exception;
		}

		/* yes, merge the entries */
		classcache_merge_class_entries(en,clsenA,clsenB);
		CLASSCACHE_COUNT(stat_nontriv_constraints_merged);
	}
	else {
		/* { at most one of the loaders has a corresponding entry } */

		/* set clsenA to the single class entry we have */
		if (!clsenA)
			clsenA = clsenB;

		if (!clsenA) {
			/* { no loader has a corresponding entry } */
			CLASSCACHE_COUNT(stat_nontriv_constraints_none);

			/* create a new class entry with the constraint (a,b,en->name) */
			clsenA = NEW(classcache_class_entry);
			clsenA->classobj = NULL;
			clsenA->loaders = NULL;
			clsenA->constraints = classcache_new_loader_entry(b, NULL);
			clsenA->constraints = classcache_new_loader_entry(a, clsenA->constraints);

			clsenA->next = en->classes;
			en->classes = clsenA;
		}
		else {
			CLASSCACHE_COUNT(stat_nontriv_constraints_one);

			/* make b the loader that has no corresponding entry */
			if (clsenB)
				b = a;

			/* loader b must be added to entry clsenA */
			clsenA->constraints = classcache_new_loader_entry(b, clsenA->constraints);
		}
	}

  return_success:
	CLASSCACHE_UNLOCK();
	return true;

  return_exception:
	CLASSCACHE_UNLOCK();
	return false;				/* exception */
}
#endif /* defined(ENABLE_VERIFIER) */

/* classcache_add_constraints_for_params ***************************************
 
   Add loading constraints for the parameters and return type of 
   the given method.
  
   IN:
       a................first initiating loader
       b................second initiating loader
       m................methodinfo 
  
   RETURN VALUE:
       true.............everything ok, the constraints have been added,
       false............an exception has been thrown.
   
   Note: synchronized with global tablelock
   
*******************************************************************************/

#if defined(ENABLE_VERIFIER)
bool classcache_add_constraints_for_params(classloader_t * a,
										   classloader_t * b,
										   methodinfo *m)
{
	methoddesc *md;
	typedesc *td;
	s4 i;

	/* a constraint with a == b is trivially satisfied */

	if (a == b) {
		return true;
	}

	/* get the parsed descriptor */

	assert(m);
	md = m->parseddesc;
	assert(md);

	/* constrain the return type */

	if (md->returntype.type == TYPE_ADR) {
		if (!classcache_add_constraint(a, b, md->returntype.classref->name))
			return false; /* exception */
	}

	/* constrain each reference type used in the parameters */

	td = md->paramtypes;
	i = md->paramcount;
	for (; i--; td++) {
		if (td->type != TYPE_ADR)
			continue;

		if (!classcache_add_constraint(a, b, td->classref->name))
			return false; /* exception */
	}

	/* everything ok */
	return true;
}
#endif /* defined(ENABLE_VERIFIER) */


/* classcache_number_of_loaded_classes *****************************************

   Counts the number of loaded classes and returns it.

   Note: This function assumes that the CLASSCACHE_LOCK is held by the
   caller!

*******************************************************************************/

static s4 classcache_number_of_loaded_classes(void)
{
	classcache_name_entry  *en;
	classcache_class_entry *clsen;
	s4                      number;
	u4                      i;

	/* initialize class counter */

	number = 0;

	for (i = 0; i < hashtable_classcache.size; i++) {
		/* iterate over hashlink */

		for (en = (classcache_name_entry*) hashtable_classcache.ptr[i]; en != NULL; en = en->hashlink) {
			/* filter pseudo classes $NEW$, $NULL$, $ARRAYSTUB$ out */

			if (en->name[0] == '$')
				continue;

			/* iterate over classes with same name */

			for (clsen = en->classes; clsen != NULL; clsen = clsen->next) {
				/* get only loaded classes */

				if (clsen->classobj != NULL)
					number++;
			}
		}
	}

	return number;
}


/* classcache_get_loaded_class_count *******************************************

   Counts the number of loaded classes and returns it.

*******************************************************************************/

s4 classcache_get_loaded_class_count(void)
{
	s4 count;

	CLASSCACHE_LOCK();

	count = classcache_number_of_loaded_classes();
	
	CLASSCACHE_UNLOCK();

	return count;
}


/* classcache_foreach_loaded_class *********************************************

   Calls the given function for each loaded class.

*******************************************************************************/

void classcache_foreach_loaded_class(classcache_foreach_functionptr_t func,
									 void *data)
{
	classcache_name_entry   *en;
	classcache_class_entry  *clsen;
	u4                       i;

	CLASSCACHE_LOCK();

	/* look in every slot of the hashtable */

	for (i = 0; i < hashtable_classcache.size; i++) {
		/* iterate over hashlink */

		for (en = (classcache_name_entry*) hashtable_classcache.ptr[i]; en != NULL; en = en->hashlink) {
			/* filter pseudo classes $NEW$, $NULL$, $ARRAYSTUB$ out */

			if (en->name[0] == '$')
				continue;

			/* iterate over classes with same name */

			for (clsen = en->classes; clsen != NULL; clsen = clsen->next) {
				/* get only loaded classes */

				if (clsen->classobj != NULL) {
					(*func)(clsen->classobj, data);
				}
			}
		}
	}

	CLASSCACHE_UNLOCK();
}


/*============================================================================*/
/* DEBUG DUMPS                                                                */
/*============================================================================*/

/* classcache_debug_dump *******************************************************
 
   Print the contents of the loaded class cache to a stream
  
   IN:
       file.............output stream
	   only.............if != NULL, only print entries for this name
	                    (Currently we print also the rest of the hash chain to
						 get a feel for the average length of hash chains.)
  
   Note: synchronized with global tablelock
   
*******************************************************************************/

#ifndef NDEBUG
void classcache_debug_dump(FILE * file,Utf8String only)
{
	classcache_name_entry *c;
	classcache_class_entry *clsen;
	classcache_loader_entry *lden;
	u4 slot;

	CLASSCACHE_LOCK();

	log_println("=== [loaded class cache] =====================================");
	log_println("hash size   : %d", (int) hashtable_classcache.size);
	log_println("hash entries: %d", (int) hashtable_classcache.entries);
	log_println("");

	if (only) {
		c = classcache_lookup_name(only);
		slot = 0; /* avoid compiler warning */
		goto dump_it;
	}

	for (slot = 0; slot < hashtable_classcache.size; ++slot) {
		c = (classcache_name_entry *) hashtable_classcache.ptr[slot];

dump_it:
		for (; c; c = c->hashlink) {
			utf_fprint_printable_ascii_classname(file, c->name);
			fprintf(file, "\n");

			/* iterate over all class entries */
			for (clsen = c->classes; clsen; clsen = clsen->next) {
				if (clsen->classobj) {
					log_println("    loaded %p", (void *) clsen->classobj);
				}
				else {
					log_println("    unresolved");
				}

				log_start();
				log_print("        loaders: ");
				for (lden = clsen->loaders; lden; lden = lden->next) {
					log_print("<%p> %p ", (void *) lden, (void *) lden->loader);
				}
				log_finish();

				log_start();
				log_print("        constraints: ");
				for (lden = clsen->constraints; lden; lden = lden->next) {
					log_print("<%p> %p ", (void *) lden, (void *) lden->loader);
				}
				log_finish();
			}
		}

		if (only)
			break;
	}
	fprintf(file, "\n==============================================================\n\n");

	CLASSCACHE_UNLOCK();
}

#endif /* NDEBUG */

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
