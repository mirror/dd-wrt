/* src/future/unordered_map.hpp - future unordered_map library features

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


#ifndef FUTURE_UNORDERED_MAP_HPP_
#define FUTURE_UNORDERED_MAP_HPP_ 1

#include "config.h"

// get unordered_map
#if HAVE_STD_TR1_UNORDERED_MAP

#include <tr1/unordered_map>

namespace cacao {

namespace _future = std::tr1;
const std::size_t unordered_map_default_bucket_count = 10;

} // end namespace cacao

#elif HAVE_BOOST_UNORDERED_MAP

#include <boost/unordered_map.hpp>

namespace cacao {

namespace _future = boost;
const std::size_t unordered_map_default_bucket_count = boost::unordered::detail::default_bucket_count;

} // end namespace cacao

#else
#error "No implementation of unordered_map available"
#endif

#include "future/future_hash.hpp"

namespace cacao {

#if 0
// we want this:
template<
	class Key,
	class Hash = cacao::hash<Key>,
	class KeyEqual = std::equal_to<Key>,
	class Allocator = std::allocator<std::pair<const Key, T> >
> using unordered_map = public _future::unordered_map<Key,Hash,KeyEqual,Allocator>;
#endif

template<
	class Key,
	class T,
	class Hash = cacao::hash<Key>,
	class KeyEqual = std::equal_to<Key>,
	class Allocator = std::allocator<std::pair<const Key, T> >
>
class unordered_map : public _future::unordered_map<Key,T,Hash,KeyEqual,Allocator> {
	typedef _future::unordered_map<Key,T,Hash,KeyEqual,Allocator> _Base;
public:
	typedef typename _Base::key_type key_type;
	typedef typename _Base::mapped_type mapped_type;
	typedef typename _Base::value_type value_type;
	typedef typename _Base::size_type size_type;
	typedef typename _Base::difference_type difference_type;
	typedef typename _Base::hasher hasher;
	typedef typename _Base::key_equal key_equal;
	typedef typename _Base::allocator_type allocator_type;
	typedef typename _Base::reference reference;
	typedef typename _Base::const_reference const_reference;
	typedef typename _Base::pointer pointer;
	typedef typename _Base::const_pointer const_pointer;
	typedef typename _Base::iterator iterator;
	typedef typename _Base::const_iterator const_iterator;
	typedef typename _Base::local_iterator local_iterator;
	typedef typename _Base::const_local_iterator const_local_iterator;

	explicit unordered_map(
		size_type bucket_count = unordered_map_default_bucket_count,
		const Hash& hash = Hash(),
		const KeyEqual& equal = KeyEqual(),
		const Allocator& alloc = Allocator())
			: _Base(bucket_count, hash, equal, alloc) {}

	explicit unordered_map(
		const Allocator& alloc)
			: _Base(alloc) {}

	template<class InputIt>
	unordered_map(
		InputIt first,
		InputIt last,
		size_type bucket_count = unordered_map_default_bucket_count,
		const Hash& hash = Hash(),
		const KeyEqual& equal = KeyEqual(),
		const Allocator& alloc = Allocator())
			: _Base(first, last, bucket_count, hash, equal, alloc) {}

	unordered_map(const unordered_map& other) : _Base(other) {}

	unordered_map(const unordered_map& other, const Allocator& alloc) : _Base(other,alloc) {}

};

template<
	class Key,
	class T,
	class Hash = cacao::hash<Key>,
	class KeyEqual = std::equal_to<Key>,
	class Allocator = std::allocator<std::pair<const Key, T> >
>
class unordered_multimap : public _future::unordered_multimap<Key,T,Hash,KeyEqual,Allocator> {
	typedef _future::unordered_multimap<Key,T,Hash,KeyEqual,Allocator> _Base;
public:
	typedef typename _Base::key_type key_type;
	typedef typename _Base::mapped_type mapped_type;
	typedef typename _Base::value_type value_type;
	typedef typename _Base::size_type size_type;
	typedef typename _Base::difference_type difference_type;
	typedef typename _Base::hasher hasher;
	typedef typename _Base::key_equal key_equal;
	typedef typename _Base::allocator_type allocator_type;
	typedef typename _Base::reference reference;
	typedef typename _Base::const_reference const_reference;
	typedef typename _Base::pointer pointer;
	typedef typename _Base::const_pointer const_pointer;
	typedef typename _Base::iterator iterator;
	typedef typename _Base::const_iterator const_iterator;
	typedef typename _Base::local_iterator local_iterator;
	typedef typename _Base::const_local_iterator const_local_iterator;

	explicit unordered_multimap(
		size_type bucket_count = unordered_map_default_bucket_count,
		const Hash& hash = Hash(),
		const KeyEqual& equal = KeyEqual(),
		const Allocator& alloc = Allocator())
			: _Base(bucket_count, hash, equal, alloc) {}

	explicit unordered_multimap(
		const Allocator& alloc)
			: _Base(alloc) {}

	template<class InputIt>
	unordered_multimap(
		InputIt first,
		InputIt last,
		size_type bucket_count = unordered_map_default_bucket_count,
		const Hash& hash = Hash(),
		const KeyEqual& equal = KeyEqual(),
		const Allocator& alloc = Allocator())
			: _Base(first, last, bucket_count, hash, equal, alloc) {}

	unordered_multimap(const unordered_multimap& other) : _Base(other) {}

	unordered_multimap(const unordered_multimap& other, const Allocator& alloc) : _Base(other,alloc) {}

};

} // end namespace cacao

#endif /* FUTURE_UNORDERED_MAP_HPP_ */

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
