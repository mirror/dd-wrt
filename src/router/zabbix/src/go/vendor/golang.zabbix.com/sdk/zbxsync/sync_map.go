/*
** Copyright (C) 2001-2025 Zabbix SIA
**
** Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
** documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
** rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to the following conditions:
**
** The above copyright notice and this permission notice shall be included in all copies or substantial portions
** of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
** WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
** COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
** SOFTWARE.
**/

// Package zbsync contains extensions for workling with concurrent programming.
package zbxsync

import (
	"sync"
)

// SyncMap type represents sync.Map with generic key and value type. Main data manipulation
// functions are mirrored from the underlying sync.Map.
type SyncMap[K comparable, V any] struct {
	m sync.Map
}

// Store function sets the value for a key.
func (s *SyncMap[K, V]) Store(key K, value V) {
	s.m.Store(key, value)
}

// Load function retrieves the value for a key for a given types.
//
//nolint:ireturn
func (s *SyncMap[K, V]) Load(key K) (V, bool) {
	val, ok := s.m.Load(key)
	if !ok {
		var zero V // Default zero value for type V

		return zero, false
	}

	safeVal, ok := val.(V)
	if !ok {
		panic("SyncMap: value has unexpected type")
	}

	return safeVal, true
}

// LoadOrStore function retrieves the value for a key.
//
//nolint:ireturn // value is already type-checked before return
func (s *SyncMap[K, V]) LoadOrStore(key K, value V) (V, bool) {
	a, l := s.m.LoadOrStore(key, value)

	safeVal, ok := a.(V)
	if !ok {
		panic("SyncMap: value has unexpected type")
	}

	return safeVal, l
}

// Delete function deletes the value for a key.
func (s *SyncMap[K, V]) Delete(key K) {
	s.m.Delete(key)
}

// Range function calls f sequentially for each key and value present in the map.
func (s *SyncMap[K, V]) Range(f func(K, V) bool) {
	s.m.Range(func(key, value any) bool {
		safeKey, ok := key.(K)
		if !ok {
			panic("SyncMap: value has unexpected type")
		}

		safeVal, ok := value.(V)
		if !ok {
			panic("SyncMap: value has unexpected type")
		}

		return f(safeKey, safeVal)
	})
}

// Len function returns element count stored in the map.
func (s *SyncMap[K, V]) Len() int {
	count := 0

	s.m.Range(func(_, _ any) bool {
		count++

		return true
	})

	return count
}
