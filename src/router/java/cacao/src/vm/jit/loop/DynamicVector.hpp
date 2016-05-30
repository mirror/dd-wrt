/* src/vm/jit/loop/DynamicVector.hpp

   Copyright (C) 1996-2012
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

#ifndef _DYNAMIC_VECTOR_HPP
#define _DYNAMIC_VECTOR_HPP

#include <vector>

template<class T>
class DynamicVector
{
	std::vector<T> _vec;

public:

	typedef typename std::vector<T>::iterator iterator;
	typedef typename std::vector<T>::const_iterator const_iterator;
	
	iterator begin();
	const_iterator begin() const;

	iterator end();
	const_iterator end() const;

	/**
	 * Returns the element at the specified index.
	 * Resizes the vector if the index does not exist yet.
	 */
	T& operator[](size_t index);
	const T& operator[](size_t index) const;

	size_t size() const;

	void resize(size_t size);

/*
	void reserve(size_t size);
	void push_back(const T&);
*/
};

template<class T>
inline typename DynamicVector<T>::iterator DynamicVector<T>::begin()
{
	return _vec.begin();
}

template<class T>
inline typename DynamicVector<T>::const_iterator DynamicVector<T>::begin() const
{
	return _vec.begin();
}

template<class T>
inline typename DynamicVector<T>::iterator DynamicVector<T>::end()
{
	return _vec.end();
}

template<class T>
inline typename DynamicVector<T>::const_iterator DynamicVector<T>::end() const
{
	return _vec.end();
}

template<class T>
inline T& DynamicVector<T>::operator[](size_t index)
{
	if (index >= _vec.size())
		_vec.resize(index + 1);
	return _vec[index];
}

template<class T>
inline const T& DynamicVector<T>::operator[](size_t index) const
{
	assert(index < _vec.size());
	return _vec[index];
}

template<class T>
inline size_t DynamicVector<T>::size() const
{
	return _vec.size();
}

template<class T>
inline void DynamicVector<T>::resize(size_t size)
{
	_vec.resize(size);
}

/*
template<class T>
inline void DynamicVector<T>::reserve(size_t size)
{
	_vec.reserve(size);
}

template<class T>
inline void DynamicVector<T>::push_back(const T& elem)
{
	_vec.push_back(elem);
}*/

#endif

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

