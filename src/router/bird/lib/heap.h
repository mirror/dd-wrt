/*
 *	UCW Library -- Universal Heap Macros
 *
 *	(c) 2001 Martin Mares <mj@ucw.cz>
 *	(c) 2005 Tomas Valla <tom@ucw.cz>
 *
 *	This software may be freely distributed and used according to the terms
 *	of the GNU Lesser General Public License.
 */

/**
 * [[intro]]
 * Introduction
 * ------------
 *
 * Binary heap is a simple data structure, which for example supports efficient insertions, deletions
 * and access to the minimal inserted item. We define several macros for such operations.
 * Note that because of simplicity of heaps, we have decided to define direct macros instead
 * of a <<generic:,macro generator>> as for several other data structures in the Libucw.
 *
 * A heap is represented by a number of elements and by an array of values. Beware that we
 * index this array from one, not from zero as do the standard C arrays.
 *
 * Most macros use these parameters:
 *
 * - @type - the type of elements
 * - @num - a variable (signed or unsigned integer) with the number of elements
 * - @heap - a C array of type @type; the heap is stored in `heap[1] .. heap[num]`; `heap[0]` is unused
 * - @less - a callback to compare two element values; `less(x, y)` shall return a non-zero value iff @x is lower than @y
 * - @swap - a callback to swap two array elements; `swap(heap, i, j, t)` must swap `heap[i]` with `heap[j]` with possible help of temporary variable @t (type @type).
 *
 * A valid heap must follow these rules:
 *
 * - `num >= 0`
 * - `heap[i] >= heap[i / 2]` for each `i` in `[2, num]`
 *
 * The first element `heap[1]` is always lower or equal to all other elements.
 *
 * [[macros]]
 * Macros
 * ------
 */

/* For internal usage. */
#define HEAP_BUBBLE_DOWN_J(heap,num,less,swap)						\
  for (;;)										\
    {											\
      _l = 2*_j;									\
      if (_l > num)									\
	break;										\
      if (less(heap[_j],heap[_l]) && (_l == num || less(heap[_j],heap[_l+1])))		\
	break;										\
      if (_l != num && less(heap[_l+1],heap[_l]))					\
	_l++;										\
      swap(heap,_j,_l,x);								\
      _j = _l;										\
    }

/* For internal usage. */
#define HEAP_BUBBLE_UP_J(heap,num,less,swap)						\
  while (_j > 1)									\
    {											\
      _u = _j/2;									\
      if (less(heap[_u], heap[_j]))							\
	break;										\
      swap(heap,_u,_j,x);								\
      _j = _u;										\
    }

/**
 * Shuffle the unordered array @heap of @num elements to become a valid heap. The time complexity is linear.
 **/
#define HEAP_INIT(heap,num,type,less,swap)						\
  do {											\
    uint _i = num;									\
    uint _j, _l;										\
    type x;										\
    while (_i >= 1)									\
      {											\
	_j = _i;									\
        HEAP_BUBBLE_DOWN_J(heap,num,less,swap)						\
	_i--;										\
      }											\
  } while(0)

/**
 * Delete the minimum element `heap[1]` in `O(log(n))` time.
 * The removed value is moved just after the resulting heap (`heap[num + 1]`).
 **/
#define HEAP_DELMIN(heap,num,type,less,swap)						\
  do {											\
    uint _j, _l;										\
    type x;										\
    swap(heap,1,num,x);									\
    num--;										\
    _j = 1;										\
    HEAP_BUBBLE_DOWN_J(heap,num,less,swap);						\
  } while(0)

/**
 * Insert `heap[num]` in `O(log(n))` time. The value of @num must be increased before.
 **/
#define HEAP_INSERT(heap,num,type,less,swap)						\
  do {											\
    uint _j, _u;										\
    type x;										\
    _j = num;										\
    HEAP_BUBBLE_UP_J(heap,num,less,swap);						\
  } while(0)

/**
 * If you need to increase the value of `heap[pos]`, just do it and then call this macro to rebuild the heap.
 * Only `heap[pos]` can be changed, the rest of the array must form a valid heap.
 * The time complexity is `O(log(n))`.
 **/
#define HEAP_INCREASE(heap,num,type,less,swap,pos)					\
  do {											\
    uint _j, _l;										\
    type x;										\
    _j = pos;										\
    HEAP_BUBBLE_DOWN_J(heap,num,less,swap);						\
  } while(0)

/**
 * If you need to decrease the value of `heap[pos]`, just do it and then call this macro to rebuild the heap.
 * Only `heap[pos]` can be changed, the rest of the array must form a valid heap.
 * The time complexity is `O(log(n))`.
 **/
#define HEAP_DECREASE(heap,num,type,less,swap,pos)					\
  do {											\
    uint _j, _u;										\
    type x;										\
    _j = pos;										\
    HEAP_BUBBLE_UP_J(heap,num,less,swap);						\
  } while(0)

/**
 * Delete `heap[pos]` in `O(log(n))` time.
 **/
#define HEAP_DELETE(heap,num,type,less,swap,pos)					\
  do {											\
    uint _j, _l, _u;									\
    type x;										\
    _j = pos;										\
    swap(heap,_j,num,x);								\
    num--;										\
    if (less(heap[_j], heap[num+1]))							\
      HEAP_BUBBLE_UP_J(heap,num,less,swap)						\
    else										\
      HEAP_BUBBLE_DOWN_J(heap,num,less,swap);						\
  } while(0)

/**
 * Default swapping macro.
 **/
#define HEAP_SWAP(heap,a,b,t) (t=heap[a], heap[a]=heap[b], heap[b]=t)
