/*
 * Copyright (C) 2001-2004 Sistina Software, Inc. All rights reserved.  
 * Copyright (C) 2004-2011 Red Hat, Inc. All rights reserved.
 *
 * This file is part of the device-mapper userspace tools.
 *
 * This copyrighted material is made available to anyone wishing to use,
 * modify, copy, or redistribute it subject to the terms and conditions
 * of the GNU Lesser General Public License v.2.1.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include "libdm/misc/dmlib.h"

#ifdef VALGRIND_POOL
#include "memcheck.h"
#endif
#include <assert.h>
#include <stdarg.h>

void *dm_malloc_aux(size_t s, const char *file, int line)
        __attribute__((__malloc__)) __attribute__((__warn_unused_result__));
void *dm_malloc_aux_debug(size_t s, const char *file, int line)
        __attribute__((__malloc__)) __attribute__((__warn_unused_result__));
static void *_dm_malloc_aligned_aux(size_t s, size_t a, const char *file, int line)
        __attribute__((__malloc__)) __attribute__((__warn_unused_result__));
void *dm_zalloc_aux(size_t s, const char *file, int line)
        __attribute__((__malloc__)) __attribute__((__warn_unused_result__));
void *dm_zalloc_aux_debug(size_t s, const char *file, int line)
        __attribute__((__malloc__)) __attribute__((__warn_unused_result__));
void *dm_realloc_aux(void *p, unsigned int s, const char *file, int line)
        __attribute__((__warn_unused_result__));
void dm_free_aux(void *p);
char *dm_strdup_aux(const char *str, const char *file, int line)
        __attribute__((__warn_unused_result__));
int dm_dump_memory_debug(void);
void dm_bounds_check_debug(void);

char *dm_strdup_aux(const char *str, const char *file, int line)
{
	char *ret;

	if (!str) {
		log_error(INTERNAL_ERROR "dm_strdup called with NULL pointer");
		return NULL;
	}

	if ((ret = dm_malloc_aux_debug(strlen(str) + 1, file, line)))
		strcpy(ret, str);

	return ret;
}

struct memblock {
	struct memblock *prev, *next;	/* All allocated blocks are linked */
	size_t length;		/* Size of the requested block */
	int id;			/* Index of the block */
	const char *file;	/* File that allocated */
	int line;		/* Line that allocated */
	void *magic;		/* Address of this block */
} __attribute__((aligned(8)));

static struct {
	unsigned block_serialno;/* Non-decreasing serialno of block */
	unsigned blocks_allocated; /* Current number of blocks allocated */
	unsigned blocks_max;	/* Max no of concurrently-allocated blocks */
	unsigned int bytes, mbytes;

} _mem_stats = {
0, 0, 0, 0, 0};

static struct memblock *_head = 0;
static struct memblock *_tail = 0;

void *dm_malloc_aux_debug(size_t s, const char *file, int line)
{
	struct memblock *nb;
	size_t tsize = s + sizeof(*nb) + sizeof(unsigned long);

	if (s > 50000000) {
		log_error("Huge memory allocation (size %" PRIsize_t
			  ") rejected - metadata corruption?", s);
		return 0;
	}

	if (!(nb = malloc(tsize))) {
		log_error("couldn't allocate any memory, size = %" PRIsize_t,
			  s);
		return 0;
	}

	/* set up the file and line info */
	nb->file = file;
	nb->line = line;

	dm_bounds_check();

	/* setup fields */
	nb->magic = nb + 1;
	nb->length = s;
	nb->id = ++_mem_stats.block_serialno;
	nb->next = 0;

	/* stomp a pretty pattern across the new memory
	   and fill in the boundary bytes */
	{
		char *ptr = (char *) (nb + 1);
		size_t i;
		for (i = 0; i < s; i++)
			*ptr++ = i & 0x1 ? (char) 0xba : (char) 0xbe;

		for (i = 0; i < sizeof(unsigned long); i++)
			*ptr++ = (char) nb->id;
	}

	nb->prev = _tail;

	/* link to tail of the list */
	if (!_head)
		_head = _tail = nb;
	else {
		_tail->next = nb;
		_tail = nb;
	}

	_mem_stats.blocks_allocated++;
	if (_mem_stats.blocks_allocated > _mem_stats.blocks_max)
		_mem_stats.blocks_max = _mem_stats.blocks_allocated;

	_mem_stats.bytes += s;
	if (_mem_stats.bytes > _mem_stats.mbytes)
		_mem_stats.mbytes = _mem_stats.bytes;

	/* log_debug_mem("Allocated: %u %u %u", nb->id, _mem_stats.blocks_allocated,
		  _mem_stats.bytes); */
#ifdef VALGRIND_POOL
	VALGRIND_MAKE_MEM_UNDEFINED(nb + 1, s);
#endif
	return nb + 1;
}

void *dm_zalloc_aux_debug(size_t s, const char *file, int line)
{
	void *ptr = dm_malloc_aux_debug(s, file, line);

	if (ptr)
		memset(ptr, 0, s);

	return ptr;
}

void dm_free_aux(void *p)
{
	char *ptr;
	size_t i;
	struct memblock *mb = ((struct memblock *) p) - 1;
	if (!p)
		return;

	dm_bounds_check();

	/* sanity check */
	assert(mb->magic == p);
#ifdef VALGRIND_POOL
	VALGRIND_MAKE_MEM_DEFINED(p, mb->length);
#endif
	/* check data at the far boundary */
	ptr = (char *) p + mb->length;
	for (i = 0; i < sizeof(unsigned long); i++)
		if (ptr[i] != (char) mb->id)
			assert(!"Damage at far end of block");

	/* have we freed this before ? */
	assert(mb->id != 0);

	/* unlink */
	if (mb->prev)
		mb->prev->next = mb->next;
	else
		_head = mb->next;

	if (mb->next)
		mb->next->prev = mb->prev;
	else
		_tail = mb->prev;

	mb->id = 0;

	/* stomp a different pattern across the memory */
	ptr = p;
	for (i = 0; i < mb->length; i++)
		ptr[i] = i & 1 ? (char) 0xde : (char) 0xad;

	assert(_mem_stats.blocks_allocated);
	_mem_stats.blocks_allocated--;
	_mem_stats.bytes -= mb->length;

	/* free the memory */
	free(mb);
}

void *dm_realloc_aux(void *p, unsigned int s, const char *file, int line)
{
	void *r;
	struct memblock *mb = ((struct memblock *) p) - 1;

	r = dm_malloc_aux_debug(s, file, line);

	if (r && p) {
		memcpy(r, p, mb->length);
		dm_free_aux(p);
	}

	return r;
}

int dm_dump_memory_debug(void)
{
	unsigned long tot = 0;
	struct memblock *mb;
	char str[32];

	if (_head)
		log_very_verbose("You have a memory leak:");

	for (mb = _head; mb; mb = mb->next) {
#ifdef VALGRIND_POOL
		/*
		 * We can't look at the memory in case it has had
		 * VALGRIND_MAKE_MEM_NOACCESS called on it.
		 */
		str[0] = '\0';
#else
		size_t c;

		for (c = 0; c < sizeof(str) - 1; c++) {
			if (c >= mb->length)
				str[c] = ' ';
			else if (((char *)mb->magic)[c] == '\0')
				str[c] = '\0';
			else if (((char *)mb->magic)[c] < ' ')
				str[c] = '?';
			else
				str[c] = ((char *)mb->magic)[c];
		}
		str[sizeof(str) - 1] = '\0';
#endif

		LOG_MESG(_LOG_INFO, mb->file, mb->line, 0,
			 "block %d at %p, size %" PRIsize_t "\t [%s]",
			 mb->id, mb->magic, mb->length, str);
		tot += mb->length;
	}

	if (_head)
		log_very_verbose("%ld bytes leaked in total", tot);

	return 1;
}

void dm_bounds_check_debug(void)
{
	struct memblock *mb = _head;
	while (mb) {
		size_t i;
		char *ptr = ((char *) (mb + 1)) + mb->length;
		for (i = 0; i < sizeof(unsigned long); i++)
			if (*ptr++ != (char) mb->id)
				assert(!"Memory smash");

		mb = mb->next;
	}
}

void *dm_malloc_aux(size_t s, const char *file __attribute__((unused)),
		    int line __attribute__((unused)))
{
	if (s > 50000000) {
		log_error("Huge memory allocation (size %" PRIsize_t
			  ") rejected - metadata corruption?", s);
		return 0;
	}

	return malloc(s);
}

/* Allocate size s with alignment a (or page size if 0) */
static void *_dm_malloc_aligned_aux(size_t s, size_t a, const char *file __attribute__((unused)),
				    int line __attribute__((unused)))
{
	void *memptr;
	int r;

	if (!a)
		a = getpagesize();

	if (s > 50000000) {
		log_error("Huge memory allocation (size %" PRIsize_t
			  ") rejected - metadata corruption?", s);
		return 0;
	}

	if ((r = posix_memalign(&memptr, a, s))) {
		log_error("Failed to allocate %" PRIsize_t " bytes aligned to %" PRIsize_t ": %s", s, a, strerror(r));
		return 0;
	}

	return memptr;
}

void *dm_zalloc_aux(size_t s, const char *file, int line)
{
	void *ptr = dm_malloc_aux(s, file, line);

	if (ptr)
		memset(ptr, 0, s);

	return ptr;
}

#ifdef DEBUG_MEM

void *dm_malloc_wrapper(size_t s, const char *file, int line)
{
	return dm_malloc_aux_debug(s, file, line);
}

void *dm_malloc_aligned_wrapper(size_t s, size_t a, const char *file, int line)
{
	/* FIXME Implement alignment when debugging - currently just ignored */
	return _dm_malloc_aux_debug(s, file, line);
}

void *dm_zalloc_wrapper(size_t s, const char *file, int line)
{
	return dm_zalloc_aux_debug(s, file, line);
}

char *dm_strdup_wrapper(const char *str, const char *file, int line)
{
	return dm_strdup_aux(str, file, line);
}

void dm_free_wrapper(void *ptr)
{
	dm_free_aux(ptr);
}

void *dm_realloc_wrapper(void *p, unsigned int s, const char *file, int line)
{
	return dm_realloc_aux(p, s, file, line);
}

int dm_dump_memory_wrapper(void)
{
	return dm_dump_memory_debug();
}

void dm_bounds_check_wrapper(void)
{
	dm_bounds_check_debug();
}

#else /* !DEBUG_MEM */

void *dm_malloc_wrapper(size_t s, const char *file, int line)
{
	return dm_malloc_aux(s, file, line);
}

void *dm_malloc_aligned_wrapper(size_t s, size_t a, const char *file, int line)
{
	return _dm_malloc_aligned_aux(s, a, file, line);
}

void *dm_zalloc_wrapper(size_t s, const char *file, int line)
{
	return dm_zalloc_aux(s, file, line);
}

char *dm_strdup_wrapper(const char *str,
			const char *file __attribute__((unused)),
			int line __attribute__((unused)))
{
	return strdup(str);
}

void dm_free_wrapper(void *ptr)
{
	free(ptr);
}

void *dm_realloc_wrapper(void *p, unsigned int s, 
			 const char *file __attribute__((unused)),
			 int line __attribute__((unused)))
{
	return realloc(p, s);
}

int dm_dump_memory_wrapper(void)
{
	return 1;
}

void dm_bounds_check_wrapper(void)
{
}

#endif /* DEBUG_MEM */
