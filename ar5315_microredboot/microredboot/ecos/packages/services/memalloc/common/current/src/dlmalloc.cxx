//==========================================================================
//
//      dlmalloc.cxx
//
//      Port of Doug Lea's malloc implementation
//
//==========================================================================
//####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Red Hat, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with eCos; if not, write to the Free Software Foundation, Inc.,
// 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
//
// As a special exception, if other files instantiate templates or use macros
// or inline functions from this file, or you compile this file and link it
// with other works to produce a work based on this file, this file does not
// by itself cause the resulting work to be covered by the GNU General Public
// License. However the source code for this file must still be made available
// in accordance with section (3) of the GNU General Public License.
//
// This exception does not invalidate any other reasons why a work based on
// this file might be covered by the GNU General Public License.
//
// Alternative licenses for eCos may be arranged by contacting Red Hat, Inc.
// at http://sources.redhat.com/ecos/ecos-license/
// -------------------------------------------
//####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    Doug Lea (dl at g.oswego.edu), jlarmour
// Contributors: 
// Date:         2000-06-18
// Purpose:      Doug Lea's malloc implementation
// Description:  Doug Lea's malloc has been ported to eCos. This file
//               provides the implementation in a way acceptable to eCos.
//               Substantial amounts of unnecessary bits (to eCos) of the
//               original implementation have been removed to make the
//               code more tractable. Note this may make a number of the
//               comments appear to make little sense, or no longer apply!
//               In particular, mmap support is removed entirely.
//               Also the memory is "sbrked" all at once at the
//               beginning, covering the entire memory region given at
//               construction, and there can be no more afterwards.
// Usage:        #include <cyg/memalloc/dlmalloc.hxx>
//              
//
//####DESCRIPTIONEND####
//
//==========================================================================

// DOCUMENTATION FROM ORIGINAL FILE:
// (some now irrelevant parts elided)

//----------------------------------------------------------------------------

/* 
  A version of malloc/free/realloc written by Doug Lea and released to the 
  public domain.  Send questions/comments/complaints/performance data
  to dl at cs.oswego.edu

* VERSION 2.6.6  Sun Mar  5 19:10:03 2000  Doug Lea  (dl at gee)
  
   Note: There may be an updated version of this malloc obtainable at
           ftp://g.oswego.edu/pub/misc/malloc.c
         Check before installing!

* Why use this malloc?

  This is not the fastest, most space-conserving, most portable, or
  most tunable malloc ever written. However it is among the fastest
  while also being among the most space-conserving, portable and tunable.
  Consistent balance across these factors results in a good general-purpose 
  allocator. For a high-level description, see 
     http://g.oswego.edu/dl/html/malloc.html

* Synopsis of public routines

  (Much fuller descriptions are contained in the program documentation below.)

[ these have of course been renamed in the eCos port ]a

  malloc(size_t n);
     Return a pointer to a newly allocated chunk of at least n bytes, or null
     if no space is available.
  free(Void_t* p);
     Release the chunk of memory pointed to by p, or no effect if p is null.
  realloc(Void_t* p, size_t n);
     Return a pointer to a chunk of size n that contains the same data
     as does chunk p up to the minimum of (n, p's size) bytes, or null
     if no space is available. The returned pointer may or may not be
     the same as p. If p is null, equivalent to malloc. realloc of
     zero bytes calls free(p)

* Vital statistics:

  Alignment:                            8-byte
       8 byte alignment is currently hardwired into the design.  This
       seems to suffice for all current machines and C compilers.

  Assumed pointer representation:       4 or 8 bytes
       Code for 8-byte pointers is untested by me but has worked
       reliably by Wolfram Gloger, who contributed most of the
       changes supporting this.

  Assumed size_t  representation:       4 or 8 bytes
       Note that size_t is allowed to be 4 bytes even if pointers are 8.        

  Minimum overhead per allocated chunk: 4 or 8 bytes
       Each malloced chunk has a hidden overhead of 4 bytes holding size
       and status information.  

  Minimum allocated size: 4-byte ptrs:  16 bytes    (including 4 overhead)
                          8-byte ptrs:  24/32 bytes (including, 4/8 overhead)
                                     
       When a chunk is freed, 12 (for 4byte ptrs) or 20 (for 8 byte
       ptrs but 4 byte size) or 24 (for 8/8) additional bytes are 
       needed; 4 (8) for a trailing size field
       and 8 (16) bytes for free list pointers. Thus, the minimum
       allocatable size is 16/24/32 bytes.

       Even a request for zero bytes (i.e., malloc(0)) returns a
       pointer to something of the minimum allocatable size.

  Maximum allocated size: 4-byte size_t: 2^31 -  8 bytes
                          8-byte size_t: 2^63 - 16 bytes

       It is assumed that (possibly signed) size_t bit values suffice to
       represent chunk sizes. `Possibly signed' is due to the fact
       that `size_t' may be defined on a system as either a signed or
       an unsigned type. To be conservative, values that would appear
       as negative numbers are avoided.  
       Requests for sizes with a negative sign bit when the request
       size is treaded as a long will return null.

  Maximum overhead wastage per allocated chunk: normally 15 bytes

       Alignnment demands, plus the minimum allocatable size restriction
       make the normal worst-case wastage 15 bytes (i.e., up to 15
       more bytes will be allocated than were requested in malloc), with 
       one exception: because requests for zero bytes allocate non-zero space,
       the worst case wastage for a request of zero bytes is 24 bytes.

* Limitations

    Here are some features that are NOT currently supported

    * No user-definable hooks for callbacks and the like.
    * No automated mechanism for fully checking that all accesses
      to malloced memory stay within their bounds.
    * No support for compaction.

* Synopsis of compile-time options:

    People have reported using previous versions of this malloc on all
    versions of Unix, sometimes by tweaking some of the defines
    below. It has been tested most extensively on Solaris and
    Linux. It is also reported to work on WIN32 platforms.
    People have also reported adapting this malloc for use in
    stand-alone embedded systems.

    The implementation is in straight, hand-tuned ANSI C.  Among other
    consequences, it uses a lot of macros.  Because of this, to be at
    all usable, this code should be compiled using an optimizing compiler
    (for example gcc -O2) that can simplify expressions and control
    paths.

  CYGDBG_MEMALLOC_ALLOCATOR_DLMALLOC_DEBUG      (default: NOT defined)
     Define to enable debugging. Adds fairly extensive assertion-based 
     checking to help track down memory errors, but noticeably slows down
     execution.
  MALLOC_LOCK		   (default: NOT defined)
  MALLOC_UNLOCK		   (default: NOT defined)
     Define these to C expressions which are run to lock and unlock
     the malloc data structures.  Calls may be nested; that is,
     MALLOC_LOCK may be called more than once before the corresponding
     MALLOC_UNLOCK calls.  MALLOC_LOCK must avoid waiting for a lock
     that it already holds.
  MALLOC_ALIGNMENT          (default: NOT defined)
     Define this to 16 if you need 16 byte alignment instead of 8 byte alignment
     which is the normal default.
  SIZE_T_SMALLER_THAN_LONG (default: NOT defined)
     Define this when the platform you are compiling has
     sizeof(long) > sizeof(size_t).
     The option causes some extra code to be generated to handle operations
     that use size_t operands and have long results.
  INTERNAL_SIZE_T           (default: size_t)
     Define to a 32-bit type (probably `unsigned int') if you are on a
     64-bit machine, yet do not want or need to allow malloc requests of
     greater than 2^31 to be handled. This saves space, especially for
     very small chunks.

*/

//----------------------------------------------------------------------------


/* Preliminaries */

#include <pkgconf/memalloc.h>          // configuration header
#include <pkgconf/infra.h>             // CYGDBG_USE_ASSERTS
#include <cyg/infra/cyg_type.h>        // types
#include <cyg/infra/cyg_ass.h>         // assertions
#include <stddef.h>                    // for size_t
#include <cyg/memalloc/dlmalloc.hxx>

/*
    Debugging:

    Because freed chunks may be overwritten with link fields, this
    malloc will often die when freed memory is overwritten by user
    programs.  This can be very effective (albeit in an annoying way)
    in helping track down dangling pointers.

    If you compile with CYGDBG_MEMALLOC_ALLOCATOR_DLMALLOC_DEBUG enabled, a
    number of assertion checks are
    enabled that will catch more memory errors. You probably won't be
    able to make much sense of the actual assertion errors, but they
    should help you locate incorrectly overwritten memory.  The
    checking is fairly extensive, and will slow down execution
    noticeably. Calling get_status() with DEBUG set will
    attempt to check every allocated and free chunk in the
    course of computing the summmaries. 

    Setting CYGDBG_MEMALLOC_ALLOCATOR_DLMALLOC_DEBUG may also be helpful if you
    are trying to modify this code. The assertions in the check routines
    spell out in more detail the assumptions and invariants underlying
    the algorithms.

*/

#ifdef CYGDBG_MEMALLOC_ALLOCATOR_DLMALLOC_DEBUG
# define ASSERT(x) CYG_ASSERTC( x )
#else
# define ASSERT(x) ((void)0)
#endif


/*
   Define MALLOC_LOCK and MALLOC_UNLOCK to C expressions to run to
   lock and unlock the malloc data structures.  MALLOC_LOCK may be
   called recursively.
 */

#ifndef MALLOC_LOCK
#define MALLOC_LOCK
#endif

#ifndef MALLOC_UNLOCK
#define MALLOC_UNLOCK
#endif

/*
  INTERNAL_SIZE_T is the word-size used for internal bookkeeping
  of chunk sizes. On a 64-bit machine, you can reduce malloc
  overhead by defining INTERNAL_SIZE_T to be a 32 bit `unsigned int'
  at the expense of not being able to handle requests greater than
  2^31. This limitation is hardly ever a concern; you are encouraged
  to set this. However, the default version is the same as size_t.
*/
 
#ifndef INTERNAL_SIZE_T
#define INTERNAL_SIZE_T Cyg_Mempool_dlmalloc_Implementation::Cyg_dlmalloc_size_t
#endif

/*
  Following is needed on implementations whereby long > size_t.
  The problem is caused because the code performs subtractions of
  size_t values and stores the result in long values.  In the case
  where long > size_t and the first value is actually less than
  the second value, the resultant value is positive.  For example,
  (long)(x - y) where x = 0 and y is 1 ends up being 0x00000000FFFFFFFF
  which is 2*31 - 1 instead of 0xFFFFFFFFFFFFFFFF.  This is due to the
  fact that assignment from unsigned to signed won't sign extend.
*/

#ifdef SIZE_T_SMALLER_THAN_LONG
#define long_sub_size_t(x, y) ( (x < y) ? -((long)(y - x)) : (x - y) );
#else
#define long_sub_size_t(x, y) ( (long)(x - y) )
#endif


#ifdef CYGIMP_MEMALLOC_ALLOCATOR_DLMALLOC_USE_MEMCPY

#include <string.h>                    // memcpy, memset

/* The following macros are only invoked with (2n+1)-multiples of
   INTERNAL_SIZE_T units, with a positive integer n. This is exploited
   for fast inline execution when n is small. */

#define MALLOC_ZERO(charp, nbytes)                                            \
do {                                                                          \
  INTERNAL_SIZE_T mzsz = (nbytes);                                        \
  if(mzsz <= 9*sizeof(mzsz)) {                                                \
    INTERNAL_SIZE_T* mz = (INTERNAL_SIZE_T*) (charp);                 \
    if(mzsz >= 5*sizeof(mzsz)) {     *mz++ = 0;                               \
                                     *mz++ = 0;                               \
      if(mzsz >= 7*sizeof(mzsz)) {   *mz++ = 0;                               \
                                     *mz++ = 0;                               \
        if(mzsz >= 9*sizeof(mzsz)) { *mz++ = 0;                               \
                                     *mz++ = 0; }}}                           \
                                     *mz++ = 0;                               \
                                     *mz++ = 0;                               \
                                     *mz   = 0;                               \
  } else memset((charp), 0, mzsz);                                            \
} while(0)

#define MALLOC_COPY(dest,src,nbytes)                                          \
do {                                                                          \
  INTERNAL_SIZE_T mcsz = (nbytes);                                        \
  if(mcsz <= 9*sizeof(mcsz)) {                                                \
    INTERNAL_SIZE_T* mcsrc = (INTERNAL_SIZE_T*) (src);                \
    INTERNAL_SIZE_T* mcdst = (INTERNAL_SIZE_T*) (dest);               \
    if(mcsz >= 5*sizeof(mcsz)) {     *mcdst++ = *mcsrc++;                     \
                                     *mcdst++ = *mcsrc++;                     \
      if(mcsz >= 7*sizeof(mcsz)) {   *mcdst++ = *mcsrc++;                     \
                                     *mcdst++ = *mcsrc++;                     \
        if(mcsz >= 9*sizeof(mcsz)) { *mcdst++ = *mcsrc++;                     \
                                     *mcdst++ = *mcsrc++; }}}                 \
                                     *mcdst++ = *mcsrc++;                     \
                                     *mcdst++ = *mcsrc++;                     \
                                     *mcdst   = *mcsrc  ;                     \
  } else memcpy(dest, src, mcsz);                                             \
} while(0)

#else /* !CYGIMP_MEMALLOC_ALLOCATOR_DLMALLOC_USE_MEMCPY */

/* Use Duff's device for good zeroing/copying performance. */

#define MALLOC_ZERO(charp, nbytes)                                            \
do {                                                                          \
  INTERNAL_SIZE_T* mzp = (INTERNAL_SIZE_T*)(charp);                   \
  long mctmp = (nbytes)/sizeof(INTERNAL_SIZE_T), mcn;                     \
  if (mctmp < 8) mcn = 0; else { mcn = (mctmp-1)/8; mctmp %= 8; }             \
  switch (mctmp) {                                                            \
    case 0: for(;;) { *mzp++ = 0;                                             \
    case 7:           *mzp++ = 0;                                             \
    case 6:           *mzp++ = 0;                                             \
    case 5:           *mzp++ = 0;                                             \
    case 4:           *mzp++ = 0;                                             \
    case 3:           *mzp++ = 0;                                             \
    case 2:           *mzp++ = 0;                                             \
    case 1:           *mzp++ = 0; if(mcn <= 0) break; mcn--; }                \
  }                                                                           \
} while(0)

#define MALLOC_COPY(dest,src,nbytes)                                          \
do {                                                                          \
  INTERNAL_SIZE_T* mcsrc = (INTERNAL_SIZE_T*) src;                    \
  INTERNAL_SIZE_T* mcdst = (INTERNAL_SIZE_T*) dest;                   \
  long mctmp = (nbytes)/sizeof(INTERNAL_SIZE_T), mcn;                     \
  if (mctmp < 8) mcn = 0; else { mcn = (mctmp-1)/8; mctmp %= 8; }             \
  switch (mctmp) {                                                            \
    case 0: for(;;) { *mcdst++ = *mcsrc++;                                    \
    case 7:           *mcdst++ = *mcsrc++;                                    \
    case 6:           *mcdst++ = *mcsrc++;                                    \
    case 5:           *mcdst++ = *mcsrc++;                                    \
    case 4:           *mcdst++ = *mcsrc++;                                    \
    case 3:           *mcdst++ = *mcsrc++;                                    \
    case 2:           *mcdst++ = *mcsrc++;                                    \
    case 1:           *mcdst++ = *mcsrc++; if(mcn <= 0) break; mcn--; }       \
  }                                                                           \
} while(0)

#endif


//----------------------------------------------------------------------------

/*
   malloc_chunk details:

    (The following includes lightly edited explanations by Colin Plumb.)

    Chunks of memory are maintained using a `boundary tag' method as
    described in e.g., Knuth or Standish.  (See the paper by Paul
    Wilson ftp://ftp.cs.utexas.edu/pub/garbage/allocsrv.ps for a
    survey of such techniques.)  Sizes of free chunks are stored both
    in the front of each chunk and at the end.  This makes
    consolidating fragmented chunks into bigger chunks very fast.  The
    size fields also hold bits representing whether chunks are free or
    in use.

    An allocated chunk looks like this:  


    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Size of previous chunk, if allocated            | |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Size of chunk, in bytes                         |P|
      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             User data starts here...                          .
            .                                                               .
            .             (malloc_usable_space() bytes)                     .
            .                                                               |
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Size of chunk                                     |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+


    Where "chunk" is the front of the chunk for the purpose of most of
    the malloc code, but "mem" is the pointer that is returned to the
    user.  "Nextchunk" is the beginning of the next contiguous chunk.

    Chunks always begin on even word boundries, so the mem portion
    (which is returned to the user) is also on an even word boundary, and
    thus double-word aligned.

    Free chunks are stored in circular doubly-linked lists, and look like this:

    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Size of previous chunk                            |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `head:' |             Size of chunk, in bytes                         |P|
      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Forward pointer to next chunk in list             |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Back pointer to previous chunk in list            |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            |             Unused space (may be 0 bytes long)                .
            .                                                               .
            .                                                               |
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `foot:' |             Size of chunk, in bytes                           |
            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    The P (PREV_INUSE) bit, stored in the unused low-order bit of the
    chunk size (which is always a multiple of two words), is an in-use
    bit for the *previous* chunk.  If that bit is *clear*, then the
    word before the current chunk size contains the previous chunk
    size, and can be used to find the front of the previous chunk.
    (The very first chunk allocated always has this bit set,
    preventing access to non-existent (or non-owned) memory.)

    Note that the `foot' of the current chunk is actually represented
    as the prev_size of the NEXT chunk. (This makes it easier to
    deal with alignments etc).

    The exception to all this is the special chunk `top', which doesn't
    bother using the trailing size field since there is no next
    contiguous chunk that would have to index off it. (After
    initialization, `top' is forced to always exist. )

    Available chunks are kept in any of several places (all declared below):

    * `av': An array of chunks serving as bin headers for consolidated
       chunks. Each bin is doubly linked.  The bins are approximately
       proportionally (log) spaced.  There are a lot of these bins
       (128). This may look excessive, but works very well in
       practice.  All procedures maintain the invariant that no
       consolidated chunk physically borders another one. Chunks in
       bins are kept in size order, with ties going to the
       approximately least recently used chunk.

       The chunks in each bin are maintained in decreasing sorted order by
       size.  This is irrelevant for the small bins, which all contain
       the same-sized chunks, but facilitates best-fit allocation for
       larger chunks. (These lists are just sequential. Keeping them in
       order almost never requires enough traversal to warrant using
       fancier ordered data structures.)  Chunks of the same size are
       linked with the most recently freed at the front, and allocations
       are taken from the back.  This results in LRU or FIFO allocation
       order, which tends to give each chunk an equal opportunity to be
       consolidated with adjacent freed chunks, resulting in larger free
       chunks and less fragmentation. 

    * `top': The top-most available chunk (i.e., the one bordering the
       end of available memory) is treated specially. It is never
       included in any bin, is used only if no other chunk is
       available.

    * `last_remainder': A bin holding only the remainder of the
       most recently split (non-top) chunk. This bin is checked
       before other non-fitting chunks, so as to provide better
       locality for runs of sequentially allocated chunks. 

*/

typedef struct Cyg_Mempool_dlmalloc_Implementation::malloc_chunk* mchunkptr;

/*  sizes, alignments */

#define SIZE_SZ                (sizeof(INTERNAL_SIZE_T))

#ifdef CYGNUM_MEMALLOC_ALLOCATOR_DLMALLOC_ALIGNMENT
#define MALLOC_ALIGNMENT (1<<(CYGNUM_MEMALLOC_ALLOCATOR_DLMALLOC_ALIGNMENT))
#endif

#ifndef MALLOC_ALIGNMENT
#define MALLOC_ALIGN           8
#define MALLOC_ALIGNMENT       (SIZE_SZ + SIZE_SZ)
#else
#define MALLOC_ALIGN           MALLOC_ALIGNMENT
#endif
#define MALLOC_ALIGN_MASK      (MALLOC_ALIGNMENT - 1)
#define MINSIZE                \
             (sizeof(struct Cyg_Mempool_dlmalloc_Implementation::malloc_chunk))

/* conversion from malloc headers to user pointers, and back */

#define chunk2mem(p)   ((cyg_uint8*)((char*)(p) + 2*SIZE_SZ))
#define mem2chunk(mem) ((mchunkptr)((char*)(mem) - 2*SIZE_SZ))

/* pad request bytes into a usable size */

#define request2size(req) \
 (((long)((req) + (SIZE_SZ + MALLOC_ALIGN_MASK)) < \
  (long)(MINSIZE + MALLOC_ALIGN_MASK)) ? ((MINSIZE + MALLOC_ALIGN_MASK) & ~(MALLOC_ALIGN_MASK)) : \
   (((req) + (SIZE_SZ + MALLOC_ALIGN_MASK)) & ~(MALLOC_ALIGN_MASK)))

/* Check if m has acceptable alignment */

#define aligned_OK(m)    (((unsigned long)((m)) & (MALLOC_ALIGN_MASK)) == 0)


/* 
  Physical chunk operations  
*/


/* size field is or'ed with PREV_INUSE when previous adjacent chunk in use */

#define PREV_INUSE 0x1 

/* Bits to mask off when extracting size */

#define SIZE_BITS (PREV_INUSE)


/* Ptr to next physical malloc_chunk. */

#define next_chunk(p) ((mchunkptr)( ((char*)(p)) + ((p)->size & ~PREV_INUSE) ))

/* Ptr to previous physical malloc_chunk */

#define prev_chunk(p)\
   ((mchunkptr)( ((char*)(p)) - ((p)->prev_size) ))


/* Treat space at ptr + offset as a chunk */

#define chunk_at_offset(p, s)  ((mchunkptr)(((char*)(p)) + (s)))

/* 
  Dealing with use bits 
*/

/* extract p's inuse bit */

#define inuse(p)\
((((mchunkptr)(((char*)(p))+((p)->size & ~PREV_INUSE)))->size) & PREV_INUSE)

/* extract inuse bit of previous chunk */

#define prev_inuse(p)  ((p)->size & PREV_INUSE)

/* set/clear chunk as in use without otherwise disturbing */

#define set_inuse(p)\
((mchunkptr)(((char*)(p)) + ((p)->size & ~PREV_INUSE)))->size |= PREV_INUSE

#define clear_inuse(p)\
((mchunkptr)(((char*)(p)) + ((p)->size & ~PREV_INUSE)))->size &= ~(PREV_INUSE)

/* check/set/clear inuse bits in known places */

#define inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((char*)(p)) + (s)))->size & PREV_INUSE)

#define set_inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((char*)(p)) + (s)))->size |= PREV_INUSE)

#define clear_inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((char*)(p)) + (s)))->size &= ~(PREV_INUSE))


/* 
  Dealing with size fields 
*/

/* Get size, ignoring use bits */

#define chunksize(p)          ((p)->size & ~(SIZE_BITS))

/* Set size at head, without disturbing its use bit */

#define set_head_size(p, s)   ((p)->size = (((p)->size & PREV_INUSE) | (s)))

/* Set size/use ignoring previous bits in header */

#define set_head(p, s)        ((p)->size = (s))

/* Set size at footer (only when chunk is not in use) */

#define set_foot(p, s)   (((mchunkptr)((char*)(p) + (s)))->prev_size = (s))


//----------------------------------------------------------------------------

/*
   Bins

    The bins, `av_' are an array of pairs of pointers serving as the
    heads of (initially empty) doubly-linked lists of chunks, laid out
    in a way so that each pair can be treated as if it were in a
    malloc_chunk. (This way, the fd/bk offsets for linking bin heads
    and chunks are the same).

    Bins for sizes < 512 bytes contain chunks of all the same size, spaced
    8 bytes apart. Larger bins are approximately logarithmically
    spaced. (See the table below.) The `av_' array is never mentioned
    directly in the code, but instead via bin access macros.

    Bin layout:

    64 bins of size       8
    32 bins of size      64
    16 bins of size     512
     8 bins of size    4096
     4 bins of size   32768
     2 bins of size  262144
     1 bin  of size what's left

    There is actually a little bit of slop in the numbers in bin_index
    for the sake of speed. This makes no difference elsewhere.

    The special chunks `top' and `last_remainder' get their own bins,
    (this is implemented via yet more trickery with the av_ array),
    although `top' is never properly linked to its bin since it is
    always handled specially.

*/

typedef struct Cyg_Mempool_dlmalloc_Implementation::malloc_chunk* mbinptr;

/* access macros */

#define bin_at(i)      ((mbinptr)((char*)&(av_[2*(i) + 2]) - 2*SIZE_SZ))
#define next_bin(b)    ((mbinptr)((char*)(b) + 2 * sizeof(mbinptr)))
#define prev_bin(b)    ((mbinptr)((char*)(b) - 2 * sizeof(mbinptr)))

/*
   The first 2 bins are never indexed. The corresponding av_ cells are instead
   used for bookkeeping. This is not to save space, but to simplify
   indexing, maintain locality, and avoid some initialization tests.
*/

#define top            (bin_at(0)->fd)   /* The topmost chunk */
#define last_remainder (bin_at(1))       /* remainder from last split */


/* Helper macro to initialize bins */

#define IAV(i)  bin_at(i), bin_at(i)

#ifndef CYGIMP_MEMALLOC_ALLOCATOR_DLMALLOC_SAFE_MULTIPLE
static mbinptr av_[CYGPRI_MEMALLOC_ALLOCATOR_DLMALLOC_NAV * 2 + 2] = {
 0, 0,
 IAV(0),   IAV(1),   IAV(2),   IAV(3),   IAV(4),   IAV(5),   IAV(6),   IAV(7),
 IAV(8),   IAV(9),   IAV(10),  IAV(11),  IAV(12),  IAV(13),  IAV(14),  IAV(15),
 IAV(16),  IAV(17),  IAV(18),  IAV(19),  IAV(20),  IAV(21),  IAV(22),  IAV(23),
 IAV(24),  IAV(25),  IAV(26),  IAV(27),  IAV(28),  IAV(29),  IAV(30),  IAV(31),
 IAV(32),  IAV(33),  IAV(34),  IAV(35),  IAV(36),  IAV(37),  IAV(38),  IAV(39),
 IAV(40),  IAV(41),  IAV(42),  IAV(43),  IAV(44),  IAV(45),  IAV(46),  IAV(47),
 IAV(48),  IAV(49),  IAV(50),  IAV(51),  IAV(52),  IAV(53),  IAV(54),  IAV(55),
 IAV(56),  IAV(57),  IAV(58),  IAV(59),  IAV(60),  IAV(61),  IAV(62),  IAV(63),
 IAV(64),  IAV(65),  IAV(66),  IAV(67),  IAV(68),  IAV(69),  IAV(70),  IAV(71),
 IAV(72),  IAV(73),  IAV(74),  IAV(75),  IAV(76),  IAV(77),  IAV(78),  IAV(79),
 IAV(80),  IAV(81),  IAV(82),  IAV(83),  IAV(84),  IAV(85),  IAV(86),  IAV(87),
 IAV(88),  IAV(89),  IAV(90),  IAV(91),  IAV(92),  IAV(93),  IAV(94),  IAV(95),
 IAV(96),  IAV(97),  IAV(98),  IAV(99),  IAV(100), IAV(101), IAV(102), IAV(103),
 IAV(104), IAV(105), IAV(106), IAV(107), IAV(108), IAV(109), IAV(110), IAV(111),
 IAV(112), IAV(113), IAV(114), IAV(115), IAV(116), IAV(117), IAV(118), IAV(119),
 IAV(120), IAV(121), IAV(122), IAV(123), IAV(124), IAV(125), IAV(126), IAV(127)
};
#endif

/* field-extraction macros */

#define first(b) ((b)->fd)
#define last(b)  ((b)->bk)

/* 
  Indexing into bins
*/

#define bin_index(sz)                                                          \
(((((unsigned long)(sz)) >> 9) ==    0) ?       (((unsigned long)(sz)) >>  3): \
 ((((unsigned long)(sz)) >> 9) <=    4) ?  56 + (((unsigned long)(sz)) >>  6): \
 ((((unsigned long)(sz)) >> 9) <=   20) ?  91 + (((unsigned long)(sz)) >>  9): \
 ((((unsigned long)(sz)) >> 9) <=   84) ? 110 + (((unsigned long)(sz)) >> 12): \
 ((((unsigned long)(sz)) >> 9) <=  340) ? 119 + (((unsigned long)(sz)) >> 15): \
 ((((unsigned long)(sz)) >> 9) <= 1364) ? 124 + (((unsigned long)(sz)) >> 18): \
                                          126)                     
/* 
  bins for chunks < 512 are all spaced SMALLBIN_WIDTH bytes apart, and hold
  identically sized chunks. This is exploited in malloc.
*/

#define MAX_SMALLBIN_SIZE   512
#define SMALLBIN_WIDTH        8
#define SMALLBIN_WIDTH_BITS   3
#define MAX_SMALLBIN        (MAX_SMALLBIN_SIZE / SMALLBIN_WIDTH) - 1

#define smallbin_index(sz)  (((unsigned long)(sz)) >> SMALLBIN_WIDTH_BITS)

/* 
   Requests are `small' if both the corresponding and the next bin are small
*/

#define is_small_request(nb) (nb < MAX_SMALLBIN_SIZE - SMALLBIN_WIDTH)

/*
    To help compensate for the large number of bins, a one-level index
    structure is used for bin-by-bin searching.  `binblocks' is a
    one-word bitvector recording whether groups of BINBLOCKWIDTH bins
    have any (possibly) non-empty bins, so they can be skipped over
    all at once during during traversals. The bits are NOT always
    cleared as soon as all bins in a block are empty, but instead only
    when all are noticed to be empty during traversal in malloc.
*/

#define BINBLOCKWIDTH     4   /* bins per block */

#define binblocks      (bin_at(0)->size) /* bitvector of nonempty blocks */

/* bin<->block macros */

#define idx2binblock(ix)    ((unsigned long)1 << (ix / BINBLOCKWIDTH))
#define mark_binblock(ii)   (binblocks |= idx2binblock(ii))
#define clear_binblock(ii)  (binblocks &= ~(idx2binblock(ii)))


//----------------------------------------------------------------------------

/* 
  Debugging support 
*/

#ifdef CYGDBG_MEMALLOC_ALLOCATOR_DLMALLOC_DEBUG

/*
  These routines make a number of assertions about the states
  of data structures that should be true at all times. If any
  are not true, it's very likely that a user program has somehow
  trashed memory. (It's also possible that there is a coding error
  in malloc. In which case, please report it!)
*/

void
Cyg_Mempool_dlmalloc_Implementation::do_check_chunk( mchunkptr p ) 
{ 
  INTERNAL_SIZE_T sz = p->size & ~PREV_INUSE;

  /* Check for legal address ... */
  ASSERT((cyg_uint8 *)p >= arenabase);
  if (p != top) 
    ASSERT((cyg_uint8 *)p + sz <= (cyg_uint8 *)top);
  else
    ASSERT((cyg_uint8 *)p + sz <= arenabase + arenasize);

} // Cyg_Mempool_dlmalloc_Implementation::do_check_chunk()


void
Cyg_Mempool_dlmalloc_Implementation::do_check_free_chunk(mchunkptr p) 
{ 
  INTERNAL_SIZE_T sz = p->size & ~PREV_INUSE;
  mchunkptr next = chunk_at_offset(p, sz);

  do_check_chunk(p);

  /* Check whether it claims to be free ... */
  ASSERT(!inuse(p));

  /* Unless a special marker, must have OK fields */
  if ((long)sz >= (long)MINSIZE)
  {
    ASSERT((sz & MALLOC_ALIGN_MASK) == 0);
    ASSERT(aligned_OK(chunk2mem(p)));
    /* ... matching footer field */
    ASSERT(next->prev_size == sz);
    /* ... and is fully consolidated */
    ASSERT(prev_inuse(p));
    ASSERT (next == top || inuse(next));
    
    /* ... and has minimally sane links */
    ASSERT(p->fd->bk == p);
    ASSERT(p->bk->fd == p);
  }
  else /* markers are always of size SIZE_SZ */
    ASSERT(sz == SIZE_SZ); 
} // Cyg_Mempool_dlmalloc_Implementation::do_check_free_chunk()

void
Cyg_Mempool_dlmalloc_Implementation::do_check_inuse_chunk(mchunkptr p) 
{ 
  mchunkptr next = next_chunk(p);
  do_check_chunk(p);

  /* Check whether it claims to be in use ... */
  ASSERT(inuse(p));

  /* ... and is surrounded by OK chunks.
    Since more things can be checked with free chunks than inuse ones,
    if an inuse chunk borders them and debug is on, it's worth doing them.
  */
  if (!prev_inuse(p)) 
  {
    mchunkptr prv = prev_chunk(p);
    ASSERT(next_chunk(prv) == p);
    do_check_free_chunk(prv);
  }
  if (next == top)
  {
    ASSERT(prev_inuse(next));
    ASSERT(chunksize(next) >= MINSIZE);
  }
  else if (!inuse(next))
    do_check_free_chunk(next);

} // Cyg_Mempool_dlmalloc_Implementation::do_check_inuse_chunk(

void
Cyg_Mempool_dlmalloc_Implementation::do_check_malloced_chunk(mchunkptr p,
                                              INTERNAL_SIZE_T s) 
{
  INTERNAL_SIZE_T sz = p->size & ~PREV_INUSE;
  long room = long_sub_size_t(sz, s);

  do_check_inuse_chunk(p);

  /* Legal size ... */
  ASSERT((long)sz >= (long)MINSIZE);
  ASSERT((sz & MALLOC_ALIGN_MASK) == 0);
  ASSERT(room >= 0);
  ASSERT(room < (long)MINSIZE);

  /* ... and alignment */
  ASSERT(aligned_OK(chunk2mem(p)));


  /* ... and was allocated at front of an available chunk */
  ASSERT(prev_inuse(p));

} // Cyg_Mempool_dlmalloc_Implementation::do_check_malloced_chunk(


#define check_free_chunk(P)  do_check_free_chunk(P)
#define check_inuse_chunk(P) do_check_inuse_chunk(P)
#define check_chunk(P) do_check_chunk(P)
#define check_malloced_chunk(P,N) do_check_malloced_chunk(P,N)
#else
#define check_free_chunk(P) 
#define check_inuse_chunk(P)
#define check_chunk(P)
#define check_malloced_chunk(P,N)
#endif


//----------------------------------------------------------------------------

/* 
  Macro-based internal utilities
*/


/*  
  Linking chunks in bin lists.
  Call these only with variables, not arbitrary expressions, as arguments.
*/

/* 
  Place chunk p of size s in its bin, in size order,
  putting it ahead of others of same size.
*/


#define frontlink(P, S, IDX, BK, FD)                                          \
{                                                                             \
  if (S < MAX_SMALLBIN_SIZE)                                                  \
  {                                                                           \
    IDX = smallbin_index(S);                                                  \
    mark_binblock(IDX);                                                       \
    BK = bin_at(IDX);                                                         \
    FD = BK->fd;                                                              \
    P->bk = BK;                                                               \
    P->fd = FD;                                                               \
    FD->bk = BK->fd = P;                                                      \
  }                                                                           \
  else                                                                        \
  {                                                                           \
    IDX = bin_index(S);                                                       \
    BK = bin_at(IDX);                                                         \
    FD = BK->fd;                                                              \
    if (FD == BK) mark_binblock(IDX);                                         \
    else                                                                      \
    {                                                                         \
      while (FD != BK && S < chunksize(FD)) FD = FD->fd;                      \
      BK = FD->bk;                                                            \
    }                                                                         \
    P->bk = BK;                                                               \
    P->fd = FD;                                                               \
    FD->bk = BK->fd = P;                                                      \
  }                                                                           \
}


/* take a chunk off a list */

#define unlink(P, BK, FD)                                                     \
{                                                                             \
  BK = P->bk;                                                                 \
  FD = P->fd;                                                                 \
  FD->bk = BK;                                                                \
  BK->fd = FD;                                                                \
}                                                                             \

/* Place p as the last remainder */

#define link_last_remainder(P)                                                \
{                                                                             \
  last_remainder->fd = last_remainder->bk =  P;                               \
  P->fd = P->bk = last_remainder;                                             \
}

/* Clear the last_remainder bin */

#define clear_last_remainder \
  (last_remainder->fd = last_remainder->bk = last_remainder)


//----------------------------------------------------------------------------

Cyg_Mempool_dlmalloc_Implementation::Cyg_Mempool_dlmalloc_Implementation(
                                            cyg_uint8 *base, cyg_int32 size,
                                            CYG_ADDRWORD /* argthru */ )
{
    arenabase = base;
    arenasize = size;

    CYG_ADDRESS front_misalign;
    cyg_int32 correction;

#ifdef CYGIMP_MEMALLOC_ALLOCATOR_DLMALLOC_SAFE_MULTIPLE
    cyg_ucount16 i;
    av_[0] = av_[1] = 0;
    for (i=0; i < CYGPRI_MEMALLOC_ALLOCATOR_DLMALLOC_NAV; i++) {
        av_[ i*2+2 ] = av_[ i*2+3 ] = bin_at(i);
    } // for
    
#elif defined(CYGDBG_USE_ASSERTS)
    static int instances;
    if ( ++instances > 1 )
        CYG_FAIL( "Multiple dlmalloc instances but "
                  "CYGIMP_MEMALLOC_ALLOCATOR_DLMALLOC_SAFE_MULTIPLE "
                  "not defined" );
#endif

    front_misalign = (CYG_ADDRESS)chunk2mem(base) & MALLOC_ALIGN_MASK;

    if ( front_misalign > 0 ) {
        correction = (MALLOC_ALIGNMENT) - front_misalign;
    } else {
        correction = 0;
    }

    // too small to be useful?
    if ( correction + 2*(unsigned)MALLOC_ALIGNMENT > (unsigned) size )
        // help catch errors. Don't fail now.
        arenabase = NULL; 
    else {
        top = (mchunkptr)(base + correction);
        set_head(top, arenasize | PREV_INUSE);
    }
}

//----------------------------------------------------------------------------

/* Main public routines */

/*
  Malloc Algorithm:

    The requested size is first converted into a usable form, `nb'.
    This currently means to add 4 bytes overhead plus possibly more to
    obtain 8-byte alignment and/or to obtain a size of at least
    MINSIZE (currently 16 bytes), the smallest allocatable size.
    (All fits are considered `exact' if they are within MINSIZE bytes.)

    From there, the first successful of the following steps is taken:

      1. The bin corresponding to the request size is scanned, and if
         a chunk of exactly the right size is found, it is taken.

      2. The most recently remaindered chunk is used if it is big
         enough.  This is a form of (roving) first fit, used only in
         the absence of exact fits. Runs of consecutive requests use
         the remainder of the chunk used for the previous such request
         whenever possible. This limited use of a first-fit style
         allocation strategy tends to give contiguous chunks
         coextensive lifetimes, which improves locality and can reduce
         fragmentation in the long run.

      3. Other bins are scanned in increasing size order, using a
         chunk big enough to fulfill the request, and splitting off
         any remainder.  This search is strictly by best-fit; i.e.,
         the smallest (with ties going to approximately the least
         recently used) chunk that fits is selected.

      4. If large enough, the chunk bordering the end of memory
         (`top') is split off. (This use of `top' is in accord with
         the best-fit search rule.  In effect, `top' is treated as
         larger (and thus less well fitting) than any other available
         chunk since it can be extended to be as large as necessary
         (up to system limitations).

      All allocations are made from the the `lowest' part of any found
      chunk. (The implementation invariant is that prev_inuse is
      always true of any allocated chunk; i.e., that each allocated
      chunk borders either a previously allocated and still in-use chunk,
      or the base of its memory arena.)

*/

cyg_uint8 *
Cyg_Mempool_dlmalloc_Implementation::try_alloc( cyg_int32 bytes )
{
  mchunkptr victim;                  /* inspected/selected chunk */
  INTERNAL_SIZE_T victim_size;   /* its size */
  int       idx;                     /* index for bin traversal */
  mbinptr   bin;                     /* associated bin */
  mchunkptr remainder;               /* remainder from a split */
  long      remainder_size;          /* its size */
  int       remainder_index;         /* its bin index */
  unsigned long block;               /* block traverser bit */
  int       startidx;                /* first bin of a traversed block */
  mchunkptr fwd;                     /* misc temp for linking */
  mchunkptr bck;                     /* misc temp for linking */
  mbinptr q;                         /* misc temp */

  INTERNAL_SIZE_T nb;

  /*  Allow uninitialised (zero sized) heaps because they could exist as a
   *  quirk of the MLT setup where a dynamically sized heap is at the top of
   *  memory. */
  if (NULL==arenabase) return NULL;

  if ((long)bytes < 0) return 0;

  nb = request2size(bytes);  /* padded request size; */

  MALLOC_LOCK;

  /* Check for exact match in a bin */

  if (is_small_request(nb))  /* Faster version for small requests */
  {
    idx = smallbin_index(nb); 

    /* No traversal or size check necessary for small bins.  */

    q = bin_at(idx);
    victim = last(q);

#if MALLOC_ALIGN != 16
    /* Also scan the next one, since it would have a remainder < MINSIZE */
    if (victim == q)
    {
      q = next_bin(q);
      victim = last(q);
    }
#endif
    if (victim != q)
    {
      victim_size = chunksize(victim);
      unlink(victim, bck, fwd);
      set_inuse_bit_at_offset(victim, victim_size);
      check_malloced_chunk(victim, nb);
      MALLOC_UNLOCK;
      return chunk2mem(victim);
    }

    idx += 2; /* Set for bin scan below. We've already scanned 2 bins. */

  }
  else
  {
    idx = bin_index(nb);
    bin = bin_at(idx);

    for (victim = last(bin); victim != bin; victim = victim->bk)
    {
      victim_size = chunksize(victim);
      remainder_size = long_sub_size_t(victim_size, nb);
      
      if (remainder_size >= (long)MINSIZE) /* too big */
      {
        --idx; /* adjust to rescan below after checking last remainder */
        break;   
      }

      else if (remainder_size >= 0) /* exact fit */
      {
        unlink(victim, bck, fwd);
        set_inuse_bit_at_offset(victim, victim_size);
        check_malloced_chunk(victim, nb);
	MALLOC_UNLOCK;
        return chunk2mem(victim);
      }
    }

    ++idx; 

  }

  /* Try to use the last split-off remainder */

  if ( (victim = last_remainder->fd) != last_remainder)
  {
    victim_size = chunksize(victim);
    remainder_size = long_sub_size_t(victim_size, nb);

    if (remainder_size >= (long)MINSIZE) /* re-split */
    {
      remainder = chunk_at_offset(victim, nb);
      set_head(victim, nb | PREV_INUSE);
      link_last_remainder(remainder);
      set_head(remainder, remainder_size | PREV_INUSE);
      set_foot(remainder, remainder_size);
      check_malloced_chunk(victim, nb);
      MALLOC_UNLOCK;
      return chunk2mem(victim);
    }

    clear_last_remainder;

    if (remainder_size >= 0)  /* exhaust */
    {
      set_inuse_bit_at_offset(victim, victim_size);
      check_malloced_chunk(victim, nb);
      MALLOC_UNLOCK;
      return chunk2mem(victim);
    }

    /* Else place in bin */

    frontlink(victim, victim_size, remainder_index, bck, fwd);
  }

  /* 
     If there are any possibly nonempty big-enough blocks, 
     search for best fitting chunk by scanning bins in blockwidth units.
  */

  if ( (block = idx2binblock(idx)) <= binblocks)  
  {

    /* Get to the first marked block */

    if ( (block & binblocks) == 0) 
    {
      /* force to an even block boundary */
      idx = (idx & ~(BINBLOCKWIDTH - 1)) + BINBLOCKWIDTH;
      block <<= 1;
      while ((block & binblocks) == 0)
      {
        idx += BINBLOCKWIDTH;
        block <<= 1;
      }
    }
      
    /* For each possibly nonempty block ... */
    for (;;)  
    {
      startidx = idx;          /* (track incomplete blocks) */
      q = bin = bin_at(idx);

      /* For each bin in this block ... */
      do
      {
        /* Find and use first big enough chunk ... */

        for (victim = last(bin); victim != bin; victim = victim->bk)
        {
          victim_size = chunksize(victim);
          remainder_size = long_sub_size_t(victim_size, nb);

          if (remainder_size >= (long)MINSIZE) /* split */
          {
            remainder = chunk_at_offset(victim, nb);
            set_head(victim, nb | PREV_INUSE);
            unlink(victim, bck, fwd);
            link_last_remainder(remainder);
            set_head(remainder, remainder_size | PREV_INUSE);
            set_foot(remainder, remainder_size);
            check_malloced_chunk(victim, nb);
	    MALLOC_UNLOCK;
            return chunk2mem(victim);
          }

          else if (remainder_size >= 0)  /* take */
          {
            set_inuse_bit_at_offset(victim, victim_size);
            unlink(victim, bck, fwd);
            check_malloced_chunk(victim, nb);
	    MALLOC_UNLOCK;
            return chunk2mem(victim);
          }

        }

       bin = next_bin(bin);

#if MALLOC_ALIGN == 16
       if (idx < MAX_SMALLBIN)
         {
           bin = next_bin(bin);
           ++idx;
         }
#endif
      } while ((++idx & (BINBLOCKWIDTH - 1)) != 0);

      /* Clear out the block bit. */

      do   /* Possibly backtrack to try to clear a partial block */
      {
        if ((startidx & (BINBLOCKWIDTH - 1)) == 0)
        {
          binblocks &= ~block;
          break;
        }
        --startidx;
       q = prev_bin(q);
      } while (first(q) == q);

      /* Get to the next possibly nonempty block */

      if ( (block <<= 1) <= binblocks && (block != 0) ) 
      {
        while ((block & binblocks) == 0)
        {
          idx += BINBLOCKWIDTH;
          block <<= 1;
        }
      }
      else
        break;
    }
  }


  /* Try to use top chunk */

  /* Require that there be a remainder, ensuring top always exists  */
  remainder_size = long_sub_size_t(chunksize(top), nb);
  if (chunksize(top) < nb || remainder_size < (long)MINSIZE)
  {
      //diag_printf("chunksize(top)=%ld, nb=%d, remainder=%ld\n", chunksize(top),
      //            nb, remainder_size);
      MALLOC_UNLOCK;
      CYG_MEMALLOC_FAIL(bytes);
      return NULL; /* propagate failure */
  }

  victim = top;
  set_head(victim, nb | PREV_INUSE);
  top = chunk_at_offset(victim, nb);
  set_head(top, remainder_size | PREV_INUSE);
  check_malloced_chunk(victim, nb);
  MALLOC_UNLOCK;
  return chunk2mem(victim);

} // Cyg_Mempool_dlmalloc_Implementation::try_alloc()

//----------------------------------------------------------------------------

/*
  free() algorithm :

    cases:

       1. free(NULL) has no effect.  

       2. Chunks are consolidated as they arrive, and
          placed in corresponding bins. (This includes the case of
          consolidating with the current `last_remainder').
*/

cyg_bool
Cyg_Mempool_dlmalloc_Implementation::free( cyg_uint8 *mem, cyg_int32 )
{
  mchunkptr p;         /* chunk corresponding to mem */
  INTERNAL_SIZE_T hd;  /* its head field */
  INTERNAL_SIZE_T sz;  /* its size */
  int       idx;       /* its bin index */
  mchunkptr next;      /* next contiguous chunk */
  INTERNAL_SIZE_T nextsz; /* its size */
  INTERNAL_SIZE_T prevsz; /* size of previous contiguous chunk */
  mchunkptr bck;       /* misc temp for linking */
  mchunkptr fwd;       /* misc temp for linking */
  int       islr;      /* track whether merging with last_remainder */

  if (mem == NULL)                              /* free(NULL) has no effect */
    return false;

  MALLOC_LOCK;

  p = mem2chunk(mem);
  hd = p->size;

  check_inuse_chunk(p);
  
  sz = hd & ~PREV_INUSE;
  next = chunk_at_offset(p, sz);
  nextsz = chunksize(next);
  
  if (next == top)                            /* merge with top */
  {
    sz += nextsz;

    if (!(hd & PREV_INUSE))                    /* consolidate backward */
    {
      prevsz = p->prev_size;
      p = chunk_at_offset(p, -((long) prevsz));
      sz += prevsz;
      unlink(p, bck, fwd);
    }

    set_head(p, sz | PREV_INUSE);
    top = p;
    MALLOC_UNLOCK;
    return true;
  }

  set_head(next, nextsz);                    /* clear inuse bit */

  islr = 0;

  if (!(hd & PREV_INUSE))                    /* consolidate backward */
  {
    prevsz = p->prev_size;
    p = chunk_at_offset(p, -((long) prevsz));
    sz += prevsz;
    
    if (p->fd == last_remainder)             /* keep as last_remainder */
      islr = 1;
    else
      unlink(p, bck, fwd);
  }
  
  if (!(inuse_bit_at_offset(next, nextsz)))   /* consolidate forward */
  {
    sz += nextsz;
    
    if (!islr && next->fd == last_remainder)  /* re-insert last_remainder */
    {
      islr = 1;
      link_last_remainder(p);   
    }
    else
      unlink(next, bck, fwd);
  }


  set_head(p, sz | PREV_INUSE);
  set_foot(p, sz);
  if (!islr)
    frontlink(p, sz, idx, bck, fwd);  

  MALLOC_UNLOCK;

  return true;
} // Cyg_Mempool_dlmalloc_Implementation::free()

//----------------------------------------------------------------------------

// resize existing allocation, if oldsize is non-NULL, previous
// allocation size is placed into it. If previous size not available,
// it is set to 0. NB previous allocation size may have been rounded up.
// Occasionally the allocation can be adjusted *backwards* as well as,
// or instead of forwards, therefore the address of the resized
// allocation is returned, or NULL if no resizing was possible.
// Note that this differs from ::realloc() in that no attempt is
// made to call malloc() if resizing is not possible - that is left
// to higher layers. The data is copied from old to new though.
// The effects of alloc_ptr==NULL or newsize==0 are undefined


// DOCUMENTATION FROM ORIGINAL FILE:
// (some now irrelevant parts elided)
/*
  Realloc algorithm:

    If the reallocation is for additional space, and the
    chunk can be extended, it is, else a malloc-copy-free sequence is
    taken.  There are several different ways that a chunk could be
    extended. All are tried:

       * Extending forward into following adjacent free chunk.
       * Shifting backwards, joining preceding adjacent space
       * Both shifting backwards and extending forward.

    If the reallocation is for less space, and the new request is for
    a `small' (<512 bytes) size, then the newly unused space is lopped
    off and freed.

    The old unix realloc convention of allowing the last-free'd chunk
    to be used as an argument to realloc is no longer supported.
    I don't know of any programs still relying on this feature,
    and allowing it would also allow too many other incorrect 
    usages of realloc to be sensible.
*/

cyg_uint8 *
Cyg_Mempool_dlmalloc_Implementation::resize_alloc( cyg_uint8 *oldmem,
                                                   cyg_int32 bytes,
                                                   cyg_int32 *poldsize )
{

  INTERNAL_SIZE_T nb;             /* padded request size */

  mchunkptr oldp;                 /* chunk corresponding to oldmem */
  INTERNAL_SIZE_T oldsize;        /* its size */

  mchunkptr newp;                 /* chunk to return */
  INTERNAL_SIZE_T newsize;        /* its size */
  cyg_uint8*   newmem;            /* corresponding user mem */

  mchunkptr next;                 /* next contiguous chunk after oldp */
  INTERNAL_SIZE_T nextsize;       /* its size */

  mchunkptr prev;                 /* previous contiguous chunk before oldp */
  INTERNAL_SIZE_T prevsize;       /* its size */

  mchunkptr remainder;            /* holds split off extra space from newp */
  INTERNAL_SIZE_T remainder_size; /* its size */

  mchunkptr bck;                  /* misc temp for linking */
  mchunkptr fwd;                  /* misc temp for linking */

  MALLOC_LOCK;

  newp    = oldp    = mem2chunk(oldmem);
  newsize = oldsize = chunksize(oldp);

  if (NULL != poldsize)
      *poldsize = oldsize - SIZE_SZ;

  nb = request2size(bytes);

  check_inuse_chunk(oldp);

  if ((long)(oldsize) < (long)(nb))  
  {

    /* Try expanding forward */

    next = chunk_at_offset(oldp, oldsize);
    if (next == top || !inuse(next)) 
    {
      nextsize = chunksize(next);

      /* Forward into top only if a remainder */
      if (next == top)
      {
        if ((long)(nextsize + newsize) >= (long)(nb + MINSIZE))
        {
          newsize += nextsize;
          top = chunk_at_offset(oldp, nb);
          set_head(top, (newsize - nb) | PREV_INUSE);
          set_head_size(oldp, nb);
	  MALLOC_UNLOCK;
          return chunk2mem(oldp);
        }
      }

      /* Forward into next chunk */
      else if (((long)(nextsize + newsize) >= (long)(nb)))
      { 
        unlink(next, bck, fwd);
        newsize  += nextsize;
        goto split;
      }
    }
    else
    {
      next = 0;
      nextsize = 0;
    }

    /* Try shifting backwards. */

    if (!prev_inuse(oldp))
    {
      prev = prev_chunk(oldp);
      prevsize = chunksize(prev);

      /* try forward + backward first to save a later consolidation */

      if (next != 0)
      {
        /* into top */
        if (next == top)
        {
          if ((long)(nextsize + prevsize + newsize) >= (long)(nb + MINSIZE))
          {
            unlink(prev, bck, fwd);
            newp = prev;
            newsize += prevsize + nextsize;
            newmem = chunk2mem(newp);
            MALLOC_COPY(newmem, oldmem, oldsize - SIZE_SZ);
            top = chunk_at_offset(newp, nb);
            set_head(top, (newsize - nb) | PREV_INUSE);
            set_head_size(newp, nb);
	    MALLOC_UNLOCK;
            return newmem;
          }
        }

        /* into next chunk */
        else if (((long)(nextsize + prevsize + newsize) >= (long)(nb)))
        {
          unlink(next, bck, fwd);
          unlink(prev, bck, fwd);
          newp = prev;
          newsize += nextsize + prevsize;
          newmem = chunk2mem(newp);
          MALLOC_COPY(newmem, oldmem, oldsize - SIZE_SZ);
          goto split;
        }
      }
      
      /* backward only */
      if (prev != 0 && (long)(prevsize + newsize) >= (long)nb)  
      {
        unlink(prev, bck, fwd);
        newp = prev;
        newsize += prevsize;
        newmem = chunk2mem(newp);
        MALLOC_COPY(newmem, oldmem, oldsize - SIZE_SZ);
        goto split;
      }
    }

    // couldn't resize the allocation any direction, so return failure
    MALLOC_UNLOCK;
    return NULL;
  }


 split:  /* split off extra room in old or expanded chunk */

  remainder_size = long_sub_size_t(newsize, nb);

  if (remainder_size >= (long)MINSIZE) /* split off remainder */
  {
    remainder = chunk_at_offset(newp, nb);
    set_head_size(newp, nb);
    set_head(remainder, remainder_size | PREV_INUSE);
    set_inuse_bit_at_offset(remainder, remainder_size);
    /* let free() deal with it */
    Cyg_Mempool_dlmalloc_Implementation::free( chunk2mem(remainder) );
  }
  else
  {
    set_head_size(newp, newsize);
    set_inuse_bit_at_offset(newp, newsize);
  }

  check_inuse_chunk(newp);
  MALLOC_UNLOCK;
  return chunk2mem(newp);

} // Cyg_Mempool_dlmalloc_Implementation::resize_alloc()

//----------------------------------------------------------------------------

// Get memory pool status
// flags is a bitmask of requested fields to fill in. The flags are
// defined in common.hxx
void
Cyg_Mempool_dlmalloc_Implementation::get_status( 
                                  cyg_mempool_status_flag_t flags,
                                  Cyg_Mempool_Status &status )
{
    if (0 != (flags&(CYG_MEMPOOL_STAT_FREEBLOCKS|CYG_MEMPOOL_STAT_TOTALFREE|
                     CYG_MEMPOOL_STAT_TOTALALLOCATED|CYG_MEMPOOL_STAT_MAXFREE)))
    {
        int i;
        mbinptr b;
        mchunkptr p;
        cyg_int32 chunksizep;
        cyg_int32 maxfree;
#ifdef CYGDBG_MEMALLOC_ALLOCATOR_DLMALLOC_DEBUG
        mchunkptr q;
#endif
        
        INTERNAL_SIZE_T avail = chunksize(top);
        int   navail = ((long)(avail) >= (long)MINSIZE)? 1 : 0; 
        maxfree = avail;
        
        for (i = 1; i < CYGPRI_MEMALLOC_ALLOCATOR_DLMALLOC_NAV; ++i) {
            b = bin_at(i);
            for (p = last(b); p != b; p = p->bk) {
#ifdef CYGDBG_MEMALLOC_ALLOCATOR_DLMALLOC_DEBUG
                check_free_chunk(p);
                for (q = next_chunk(p); 
                     (q < top) && inuse(q) && 
                         (long)(chunksize(q)) >= (long)MINSIZE; 
                     q = next_chunk(q))
                    check_inuse_chunk(q);
#endif
                chunksizep = chunksize(p);
                avail += chunksizep;
                if ( chunksizep > maxfree )
                    maxfree = chunksizep;
                navail++;
            }
        }
    
        if ( 0 != (flags & CYG_MEMPOOL_STAT_TOTALALLOCATED) )
            status.totalallocated = arenasize - avail;
        // as quick or quicker to just set most of these, rather than
        // test flag first
        status.totalfree = (avail & ~(MALLOC_ALIGN_MASK)) - SIZE_SZ - MINSIZE;
        CYG_ASSERT( ((avail + SIZE_SZ + MALLOC_ALIGN_MASK) & ~MALLOC_ALIGN_MASK)
                    >= MINSIZE, "free mem negative!" );
        status.freeblocks = navail;
        status.maxfree = (maxfree & ~(MALLOC_ALIGN_MASK)) - SIZE_SZ - MINSIZE;
        //diag_printf("raw mf: %d, ret mf: %d\n", maxfree, status.maxfree);
        CYG_ASSERT( ((maxfree + SIZE_SZ + MALLOC_ALIGN_MASK) & 
                    ~MALLOC_ALIGN_MASK) >= MINSIZE, 
                    "max free block size negative!" );
    } // if

    // as quick or quicker to just set most of these, rather than
    // test flag first
    status.arenabase = status.origbase = arenabase;
    status.arenasize = status.origsize = arenasize;
    status.maxoverhead = MINSIZE + MALLOC_ALIGNMENT;

} // Cyg_Mempool_dlmalloc_Implementation::get_status()


//----------------------------------------------------------------------------

// EOF dlmalloc.cxx
