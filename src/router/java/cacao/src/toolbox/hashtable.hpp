/* src/toolbox/hashtable.hpp - functions for internal hashtables

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


#ifndef HASHTABLE_HPP_
#define HASHTABLE_HPP_ 1

#include "config.h"

#include <cassert>                      // for assert
#include <cstddef>                      // for size_t
#include <new>                          // for operator new

#include "mm/memory.hpp"                // for MemoryAllocator

#include "toolbox/util.hpp"             // for is_power_of_two, etc

#include "vm/types.hpp"                 // for u4
#include "vm/utf8.hpp"                  // for Utf8String

class Mutex;


/***
 *	An instrusive hash table using open addressing.
 *
 *	Entries stored in the table must conform to the following interface:
 *	@code
 *		struct Entry {
 *			Entry();
 *			Entry(const Entry&);
 *
 *			// An Entry constructed with it's default constructor
 *			// must return true for is_empty().
 *			bool is_empty()    const;
 *			bool is_occupied() const;
 *
 *			// make this an  inline function that always
 *			// returns false for an insert only table
 *			bool is_deleted()  const;
 *
 *			void set_empty();
 *			void set_occupied(const T&);
 *
 *			// only required if you want to remove elements from the table,
 *			// an insert-only table will work without it.
 *			void set_deleted();
 *
 *			// there must be an overload for any T that you want to insert
 *			// into the table
 *			void set_occupied(const T&);
 *
 *			// these are only ever called on occupied entries
 *			bool operator==(const T&);
 *			size_t hash() const;
 *		};
 *	@endcode
 *
 *	An entry can be in one of three states: empty, occupied and deleted.
 *
 *	Each table entry must have an unique key that identifies it.
 *	The key must be stored in the entry itself, it is not stored separately in
 *	the table.
 *
 *	After inserting an element its key and the key's hash code may never
 *	change.
 *
 *	Calling find(), insert() or remove() on the table invalidates pointers
 *	to elements in the table, EntryRefs and Iterators.
 *
 *	This hash table is not thread safe.
 *
 *  If you want an insert only hash table Entry should not have a set_deleted()
 *  method. If the is_deleted() method is inline and always returns false
 *  it can be inlined away, and there should be no overhead.
 *
 *	@tparam Entry      The type of element stored in the table.
 *		               Must conform to the interface mentioned above
 *		               Should be small, a pointer is optimal.
 *	@tparam Allocator  STL style allocator for Entries.
 *
 * @note
 *		The design of the hash table is taken from CPython
 *		as presented in the book `Beautiful Code',
 *		and Google's sparsehash (https://code.google.com/p/sparsehash)
 *
 * @Cpp11 Use C++11 move semantics to move entries on resize instead of copying them
 */
template<typename _Entry,
         typename _Allocator = MemoryAllocator<_Entry> >
struct HashTable {
	typedef _Entry     Entry;
	typedef _Allocator Allocator;

	static const size_t DEFAULT_INITIAL_CAPACITY = 256;
	static const size_t DEFAULT_LOAD_FACTOR      = 85;

	/***
	 * Creates an empty hash table.
	 *
	 * @Cpp11 Make this a constexpr
	 */
	HashTable() : entries(0), 
	              capacity(0), 
	              count(0), 
	              deleted(0),
	              threshold(0),
	              load_factor(DEFAULT_LOAD_FACTOR) {}

	HashTable(size_t initial_capacity, size_t load_factor = DEFAULT_LOAD_FACTOR) {
		assert(initial_capacity > 0);

		this->capacity    = initial_capacity;
		this->count       = 0;
		this->load_factor = load_factor;

		allocate_table();
	};

	~HashTable() { destroy(entries, capacity); }

	/***
	 * A reference to an entry in the table.
	 *
	 * Can be obtained via find(), and used with insert() and remove()
	 * for insertion and deletion with a single hash lookup.
	 */
	class EntryRef {
	public:
		/***
		 * Check if entry pointed to by ref is valid.
		 *
		 * Accessing an invalid entry leads to undefined behaviour.
		 */
		operator bool() const { return occupied; }

		/***
		 * Access to refs entry.
		 * Only use this if operator bool() returns true
		 *
		 * You MUST NOT alter the key of entry
		 */
		Entry *operator->() { return  entry; }
		Entry &operator*()  { return *entry; }
	private:
		EntryRef(Entry *e, bool occupied) : entry(e), occupied(occupied) {}

		Entry *entry;
		bool   occupied;

		friend struct HashTable;
	};

	/***
	 * An STL style iterator that can be used to traverse all
	 * elements stored in the table
	 */
	class Iterator {
	public:
		bool operator==(Iterator it) const { return pos == it.pos; }
		bool operator!=(Iterator it) const { return pos != it.pos; }

		Iterator& operator++() {
			pos = next_occupied(pos, end);

			return *this;
		}
		Iterator  operator++(int) {
			Iterator it(*this);
			++(*this);
			return it;
		}

		Entry& operator*()  { return *pos; }
		Entry* operator->() { return  pos; }
	private:
		Iterator(Entry *pos, Entry *end) : pos(pos), end(end) {}

		Entry *pos, *end;

		friend struct HashTable;
	};

	/***
	 * Find entry for key.
	 * Iff no such entry exists converting the returned ref to bool yields false.
	 * The returned EntryRef can be used to insert or remove items.
	 *
	 * T must have a member function:
	 *    size_t hash() const;
	 * T must be equality comparable to an Entry
	 */
	template<typename T>
	EntryRef find(const T& t) {
		// make sure table has at least one unoccupied entry
		if ((count + deleted) >= threshold)
			grow_table(capacity + 1);

		EXPENSIVE_ASSERT(has_empty_entries());

		size_t  hash       = t.hash();
		size_t  index      = hash;
		size_t  perturb    = hash;

		Entry  *insert_pos = NULL;
		bool    occupied   = false;

		while (1) {
			Entry *e = entries + fast_modulo(index, capacity);

			if (e->is_empty()) {
				if (insert_pos) {
					// No entry for key found, could be inserted at insert_pos
					break;
				} else {
					// No entry for key found, could be inserted at e
					insert_pos = e;
					break;
				}
			} else if (e->is_deleted() && !insert_pos) {
				// Not the entry we're looking for, but we could insert here
				insert_pos = e;
			} else if (*e == t) {
				// Found entry
				insert_pos = e;
				occupied   = true;
				break;
			}

			// e does not contain the entry we are looking for
			// or a deleted entry, keep searching
			index   = (5 * index) + 1 + perturb;
			perturb >>= 5;
		}

		// single point of return for RVO
		return EntryRef(insert_pos, occupied);
	}

	/***
	 * Insert element into table.
	 * Does nothing if it is already present.
	 *
	 * Returns the newly inserted or already present element.
	 *
	 * T must have a member function:
	 *    size_t hash() const;
	 * T must be equality comparable to an Entry
	 * T must be assignable to an Entry
	 */
	template<typename T>
	Entry& insert(const T& t) {
		return insert(find(t), t);
	}

	/***
	 * Insert into entry pointed to by ref.
	 * Does nothing if it is already present.
	 *
	 * Returns the newly inserted or already present element.
	 *
	 * `e' MUST have the same key as was used to obtain `ref'.
	 *
	 * T must have a member function:
	 *    size_t hash() const;
	 * T must be equality comparable to an Entry
	 * T must be assignable to an Entry
	 */
	template<typename T>
	Entry& insert(EntryRef ref, const T& t) {
		if (ref) {
			return *ref;
		} else {
			return update(ref, t);
		}
	}

	/***
	 * Insert or update element in table.
	 *
	 * Returns the newly inserted element.
	 *
	 * T must have a member function:
	 *    size_t hash() const;
	 * T must be equality comparable to an Entry
	 * T must be assignable to an Entry
	 */
	template<typename T>
	Entry& update(const T& t) {
		return update(find(t), t);
	}

	/***
	 * Insert or update element in table.
	 *
	 * Returns the newly inserted element.
	 *
	 * `e' MUST have the same key as was used to obtain `ref'.
	 *
	 * T must have a member function:
	 *    size_t hash() const;
	 * T must be equality comparable to an Entry
	 * T must be assignable to an Entry
	 */
	template<typename T>
	Entry& update(EntryRef ref, const T& t) {
		if (ref->is_empty()) {
			// insert into empty entry
			count++;
		} else if (ref->is_deleted()) {
			// overwrite deleted entry
			count++;
			deleted--;
		}

		ref->set_occupied(t);

		return *ref;
	}

	/**
	 * Utility function for hashtables that store key value pairs
	 */
	template<typename Key, typename Val>
	Entry& insert(const Key& k, const Val& v) {
		return insert(find(k), Entry(k, v));
	}

	/**
	 * Utility function for hashtables that store key value pairs
	 */
	template<typename Key, typename Val>
	Entry& insert(EntryRef ref, const Key& k, const Val& v) {
		return insert(ref, Entry(k, v));
	}

	/**
	 * Utility function for hashtables that store key value pairs
	 */
	template<typename Key, typename Val>
	Entry& update(const Key& k, const Val& v) {
		return update(find(k), Entry(k, v));
	}

	/**
	 * Utility function for hashtables that store key value pairs
	 */
	template<typename Key, typename Val>
	Entry& update(EntryRef ref, const Key& k, const Val& v) {
		return update(ref, Entry(k, v));
	}

	/***
	 * Remove entry pointed to by ref.
	 *
	 * `e' MUST have the same key as was used to obtain `ref'.
	 *
	 * T must have a member function:
	 *    size_t hash() const;
	 * T must be equality comparable to an Entry
	 */
	void remove(EntryRef ref) {
		if (ref) {
			ref->set_deleted();

			count--;
			deleted++;
		}
	}

	/***
	 * Remove element from table.
	 *
	 * T must have a member function:
	 *    size_t hash() const;
	 * T must be equality comparable to an Entry
	 */
	template<typename T>
	void remove(const T& t) {
		remove(find(t));
	}

	/***
	 * Remove all entries from the table.
	 */
	void clear() {
		count   = 0;
		deleted = 0;

		for (Entry *e = entries, *end = entries + capacity; e != end; ++e) {
			e->set_empty();
		}
	}

	/***
	 * Makes sure the table has enough space for at least n entries
	 */
	void reserve(size_t n) {
		if (n <= capacity)
			return;

		grow_table(n);
	}

	/***
	 * Change the tables load factor.
	 * This never triggers a resizing, a resize may be triggered by the next
	 * call to find, insert, update or erase.
	 */
	void set_load_factor(size_t load_factor) {
		assert(load_factor > 0);
		assert(load_factor < 100);

		this->load_factor = load_factor;
		update_threshold();
	}

	/***
	 * Access to all occupied entries in table.
	 */
	Iterator begin() {
		Entry *begin = entries;
		Entry *end   = entries + capacity;

		// skip to first entry that is occupied
		if (begin != end && !begin->is_occupied())
			begin = next_occupied(begin, end);

		return Iterator(begin, end);
	}

	Iterator end() {
		return Iterator(entries + capacity, entries + capacity);
	}

	size_t size()  { return count; }
	bool   empty() { return count == 0; }
private:
	HashTable(const HashTable&);            // non-copyable
	HashTable& operator=(const HashTable&); // non-assignable

	bool has_empty_entries() {
		return capacity > (count + deleted);
	}

	/**
	 * Increase capacity, re-allocate table and copy over all old entries
	 */
	void grow_table(size_t new_capacity) {
		Entry *old_entries  = entries;
		size_t old_capacity = capacity;

		capacity = next_power_of_two(new_capacity);

		assert(capacity > 0);
		assert(capacity > old_capacity);
		allocate_table();

		// re-insert all entries
		for (Entry *it = old_entries, *end = old_entries + old_capacity; it != end; ++it) {
			if (it->is_occupied()) {
				// We use an optimized version of the hash lookup here.
				// We can omit some checks since we know that,
				//  a) the table contains no deleted entries,
				//  b) every entry we insert has a unique key

				size_t  hash       = it->hash();
				size_t  index      = hash;
				size_t  perturb    = hash;

				while (1) {
					Entry& e = entries[fast_modulo(index, capacity)];

					if (e.is_empty()) {
						e.set_occupied(*it);
						break;
					}

					// e does not contain the entry we are looking for
					// or a deleted entry, keep searching
					index   = (5 * index) + 1 + perturb;
					perturb >>= 5;
				}
			}
		}

		// free old table
		destroy(old_entries, old_capacity);
	}

	/***
	 * Allocate table with given capacity.
	 * Also recomputes resize threshold
	 */
	void allocate_table() {
		assert(is_power_of_two(capacity));

		deleted = 0;
		update_threshold();

		// allocate new table
		entries = allocator.allocate(capacity);
		assert(capacity == 0 || entries != NULL);

		// initialize table entries
		new (entries) Entry[capacity]();
	}

	void destroy(Entry *es, size_t sz) {
		for (Entry *it = es, *end = it + sz; it != end; ++it) {
			it->~Entry();
		}

		allocator.deallocate(es, sz);
	}

	void update_threshold() {
		threshold = (capacity * load_factor) / 100;
	}

	static Entry *next_occupied(Entry *current, Entry *end) {
		if (current == end) return current;

		do {
			++current;
		} while (current != end && !current->is_occupied());

		return current;
	}

	Entry       *entries;     // the table of entries
	size_t       capacity;    // number of elements the table can hold
	size_t       count;       // number of elements stored in the table
	size_t       deleted;     // number of deleted elements in the table
	size_t       threshold;   // table will be resized when `count' gets
	                          // greater than `threshold'
	size_t       load_factor; // used to compute threshold
	Allocator    allocator;   // used to allocate table
};


/***
 * A HashTable entry that uses Utf8Strings as key.
 * Useful for using a HashTable as a hash map.
 * 
 * @code
 *   typedef HashTable<NameValueEntry<int> > HashMap;
 *
 *   HashMap    map;
 *   Utf8String key = ...;
 *
 *   map.insert(key, 5);
 *
 *   int i = *map.find(key);
 * @endcode
 *
 * The NULL string is forbidden as key
 *
 * @Cpp11 Add a templated typedef of HashTable that makes using it as a hash map easier.
 */
template<typename Value>
struct NameValuePair {
	NameValuePair()                                                  {}
	NameValuePair(Utf8String name, Value v) : _name(name), _value(v) {}

	Utf8String   name()  const { return _name;  }
	Value&       value()       { return _value; }
	const Value& value() const { return _value; }
private:
	Utf8String _name;
	Value      _value;

	/// Interface to HashTable

	size_t hash() const { return _name.hash(); }

	bool is_empty()    const { return _name == (utf*) 0; }
	bool is_deleted()  const { return _name == (utf*) 1; }
	bool is_occupied() const { return _name >  (utf*) 1; }

	void set_empty()   { _name = (utf*) 0; }
	void set_deleted() { _name = (utf*) 1; }
	void set_occupied(const NameValuePair& n) { (*this) = n; }

	bool operator==(Utf8String u)           const { return _name == u;       }
	bool operator==(const NameValuePair& n) const { return _name == n._name; }

	template<typename T, typename A>
	friend struct HashTable;
};

/***
 * Same as NameValuePair, but for insert only hash tables.
 */
template<typename Value>
struct InsertOnlyNameValuePair {
	InsertOnlyNameValuePair()                                                  {}
	InsertOnlyNameValuePair(Utf8String name, Value v) : _name(name), _value(v) {}

	Utf8String   key()   const { return _name;  }
	Value&       value()       { return _value; }
	const Value& value() const { return _value; }

	Value *operator->() { return  _value; }
	Value &operator*()  { return *_value; }
private:
	Utf8String _name;
	Value      _value;

	/// Interface to HashTable

	size_t hash() const { return _name.hash(); }

	bool is_empty()    const { return _name == (utf*) 0; }
	bool is_deleted()  const { return _name == (utf*) 1; }
	bool is_occupied() const { return _name >  (utf*) 1; }

	void set_empty()   { _name = (utf*) 0; }
	void set_deleted() { _name = (utf*) 1; }
	void set_occupied(const InsertOnlyNameValuePair& n) { (*this) = n; }

	bool operator==(Utf8String u)                     const { return _name == u;       }
	bool operator==(const InsertOnlyNameValuePair& n) const { return _name == n._name; }

	template<typename T, typename A>
	friend struct HashTable;
};


/***
 * A HashTable entry that uses a Utf8String as key.
 * Useful for using a HashTable as an intrusive hash map.
 *
 * The key is not stored separately in the entry, Value must store
 * and provide access to the key.
 *
 * The type Value must have members for accessing the key:
 * @code
 * 	 Utf8String key() const;
 *   void set_key(Utf8String);
 * @endcode
 *
 * The NULL string is forbidden as key
 */
template<typename Value>
struct NamedEntry : Value {
	NamedEntry() {}
	NamedEntry(const Value& v) : Value(v) {}
private:
	using Value::key;
	using Value::set_key;

	/// Interface to HashTable

	size_t hash() const { return key().hash(); }

	bool is_empty()    const { return key() == (utf*) 0; }
	bool is_deleted()  const { return key() == (utf*) 1; }
	bool is_occupied() const { return key() >  (utf*) 1; }

	void set_empty()   { set_key((utf*) 0); }
	void set_deleted() { set_key((utf*) 1); }
	void set_occupied(const NamedEntry& e) { (*this) = e; }

	bool operator==(Utf8String u)   const { return key() == u;       }
	bool operator==(const Value& v) const { return key() == v.key(); }

	template<typename T, typename A>
	friend struct HashTable;
};


/***
 * Same as NamedEntry, but for insert only hash tables.
 */
template<typename Value>
struct InsertOnlyNamedEntry : Value {
	InsertOnlyNamedEntry() {}
	InsertOnlyNamedEntry(const Value& v) : Value(v) {}
//	InsertOnlyNamedEntry(Utf8String key, const Value& v) : Value(v) { set_key(key); }
private:
	using Value::key;
	using Value::set_key;

	/// Interface to HashTable

	size_t hash() const { return key().hash(); }

	bool is_empty()    const { return key() == (utf*) 0; }
	bool is_deleted()  const { return false;             }
	bool is_occupied() const { return key() != (utf*) 0; }

	void set_empty() { set_key((utf*) 0); }
	void set_occupied(const Value& v) { (*this) = v; }

	bool operator==(Utf8String u)   const { return key() == u;       }
	bool operator==(const Value& v) const { return key() == v.key(); }

	template<typename T, typename A>
	friend struct HashTable;
};


/* data structures for hashtables ********************************************

   All utf-symbols, javastrings and classes are stored in global
   hashtables, so every symbol exists only once. Equal symbols have
   identical pointers.  The functions for adding hashtable elements
   search the table for the element with the specified name/text and
   return it on success. Otherwise a new hashtable element is created.

   The hashtables use external linking for handling collisions. The
   hashtable structure contains a pointer <ptr> to the array of
   hashtable slots. The number of hashtable slots and therefore the
   size of this array is specified by the element <size> of hashtable
   structure. <entries> contains the number of all hashtable elements
   stored in the table, including those in the external chains.  The
   hashtable element structures (utf, literalstring, classinfo)
   contain both a pointer to the next hashtable element as a link for
   the external hash chain and the key of the element. The key is
   computed from the text of the string or the classname by using up
   to 8 characters.

   If the number of entries in the hashtable exceeds twice the size of
   the hashtableslot-array it is supposed that the average length of
   the external chains has reached a value beyond 2. Therefore the
   functions for adding hashtable elements (utf_new, class_new,
   literalstring_new) double the hashtableslot-array. In this
   restructuring process all elements have to be inserted into the new
   hashtable and new external chains must be built.

   Example for the layout of a hashtable:

hashtable.ptr-->+-------------------+
                |                   |
                         ...
                |                   |
                +-------------------+   +-------------------+   +-------------------+
                | hashtable element |-->| hashtable element |-->| hashtable element |-->NULL
                +-------------------+   +-------------------+   +-------------------+
                | hashtable element |
                +-------------------+   +-------------------+
                | hashtable element |-->| hashtable element |-->NULL
                +-------------------+   +-------------------+
                | hashtable element |-->NULL
                +-------------------+
                |                   |
                         ...
                |                   |
                +-------------------+

*/

/* hashtable ******************************************************************/

struct hashtable {
	Mutex              *mutex;          /* required for locking               */
	u4                  size;           /* current size of the hashtable      */
	u4                  entries;        /* number of entries in the table     */
	void              **ptr;            /* pointer to hashtable               */
};


/* function prototypes ********************************************************/

/* create hashtable */
void hashtable_create(hashtable *hash, u4 size);

/* creates and resizes a hashtable */
hashtable *hashtable_resize(hashtable *hash, u4 size);

/* frees a hashtable */
void hashtable_free(hashtable *hash);

#endif // HASHTABLE_HPP_


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
