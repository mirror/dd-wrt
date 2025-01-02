/***********************license start***************
 * Copyright (c) 2012-2015  Cavium Inc. (support@cavium.com). All rights
 * reserved.
 *
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.

 *   * Neither the name of Cavium Inc. nor the names of
 *     its contributors may be used to endorse or promote products
 *     derived from this software without specific prior written
 *     permission.

 * This Software, including technical data, may be subject to U.S. export  control
 * laws, including the U.S. Export Administration Act and its  associated
 * regulations, and may be subject to export or import  regulations in other
 * countries.

 * TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 * AND WITH ALL FAULTS AND CAVIUM INC. MAKES NO PROMISES, REPRESENTATIONS OR
 * WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 * THE SOFTWARE, INCLUDING ITS CONDITION, ITS CONFORMITY TO ANY REPRESENTATION OR
 * DESCRIPTION, OR THE EXISTENCE OF ANY LATENT OR PATENT DEFECTS, AND CAVIUM
 * SPECIFICALLY DISCLAIMS ALL IMPLIED (IF ANY) WARRANTIES OF TITLE,
 * MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF
 * VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR
 * CORRESPONDENCE TO DESCRIPTION. THE ENTIRE  RISK ARISING OUT OF USE OR
 * PERFORMANCE OF THE SOFTWARE LIES WITH YOU.
 ***********************license end**************************************/

/**
 * @file
 *
 * <hr>$Revision: 115744 $<hr>
 */

#ifdef CVMX_BUILD_FOR_LINUX_KERNEL
#include <linux/types.h>
#include <linux/export.h>
#include "asm/octeon/cvmx-global-resources.h"
#include "asm/octeon/cvmx-bootmem.h"
#include "asm/octeon/cvmx.h"
#include "asm/octeon/cvmx-helper-cfg.h"
#include "asm/octeon/cvmx-range.h"
#else
#include "cvmx.h"
#include "cvmx-platform.h"
#include "cvmx-global-resources.h"
#include "cvmx-bootmem.h"
#include "cvmx-helper-cfg.h"
#include "cvmx-range.h"
#endif

#ifdef CVMX_BUILD_FOR_LINUX_HOST
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#define CVMX_MAX_GLOBAL_RESOURCES 128
#define CVMX_RESOURCES_ENTRIES_SIZE (sizeof(cvmx_global_resource_entry_t) * \
					CVMX_MAX_GLOBAL_RESOURCES)

/**
 * This macro returns a member of the data
 * structure. The argument "field" is the member name of the
 * structure to read. The return type is a uint64_t.
 */
#define CVMX_GLOBAL_RESOURCES_GET_FIELD(field)				\
	__cvmx_struct_get_unsigned_field(__cvmx_global_resources_addr,	\
		offsetof(cvmx_global_resources_t, field),		\
		SIZEOF_FIELD(cvmx_global_resources_t, field))

/**
 * This macro writes a member of the cvmx_global_resources_t
 * structure. The argument "field" is the member name of the
 * cvmx_global_resources_t to write.
 */
#define CVMX_GLOBAL_RESOURCES_SET_FIELD(field, value)			\
	__cvmx_struct_set_unsigned_field(__cvmx_global_resources_addr,	\
		offsetof(cvmx_global_resources_t, field),		\
		SIZEOF_FIELD(cvmx_global_resources_t, field), value)

/**
 * This macro returns a member of the cvmx_global_resource_entry_t.
 * The argument "field" is the member name of this structure.
 * the return type is a uint64_t. The "addr" parameter is the physical
 * address of the structure.
 */
#define CVMX_RESOURCE_ENTRY_GET_FIELD(addr, field)			\
	__cvmx_struct_get_unsigned_field(addr,				\
		offsetof(cvmx_global_resource_entry_t, field),  	\
		SIZEOF_FIELD(cvmx_global_resource_entry_t, field))

/**
 * This macro writes a member of the cvmx_global_resource_entry_t
 * structure. The argument "field" is the member name of the
 * cvmx_global_resource_entry_t to write. The "addr" parameter
 * is the physical address of the structure.
 */
#define CVMX_RESOURCE_ENTRY_SET_FIELD(addr, field, value)		\
	__cvmx_struct_set_unsigned_field(addr,				\
		offsetof(cvmx_global_resource_entry_t, field),  	\
		SIZEOF_FIELD(cvmx_global_resource_entry_t, field), value)

#define CVMX_GET_RESOURCE_ENTRY(count) 						\
		(__cvmx_global_resources_addr + 				\
			offsetof(cvmx_global_resources_t, resource_entry) + 	\
			(count * sizeof(cvmx_global_resource_entry_t)))

#define CVMX_RESOURCE_TAG_SET_FIELD(addr, field, value) 		\
	__cvmx_struct_set_unsigned_field(addr,				\
		offsetof(struct global_resource_tag, field), 		\
		SIZEOF_FIELD(struct global_resource_tag, field), value)

#define CVMX_RESOURCE_TAG_GET_FIELD(addr, field) 		\
	__cvmx_struct_get_unsigned_field(addr, 			\
		offsetof(struct global_resource_tag, field),	\
		SIZEOF_FIELD(struct global_resource_tag, field))




#define MAX_RESOURCE_TAG_LEN 16
#define CVMX_GLOBAL_RESOURCE_NO_LOCKING (1)

typedef struct cvmx_global_resource_entry
{
	struct global_resource_tag tag;
	uint64_t phys_addr;
	uint64_t size;
} cvmx_global_resource_entry_t;

typedef struct cvmx_global_resources
{
#ifdef __LITTLE_ENDIAN_BITFIELD
	uint32_t rlock;
	uint32_t pad;
#else
	uint32_t pad;
	uint32_t rlock;
#endif
	uint64_t entry_cnt;
	cvmx_global_resource_entry_t resource_entry[];
} cvmx_global_resources_t;

/* Not the right place, putting it here for now */
CVMX_SHARED int cvmx_enable_helper_flag;
CVMX_SHARED uint64_t cvmx_app_id;

static const int dbg = 0;

extern int __cvmx_bootmem_phy_free(uint64_t phy_addr, uint64_t size, uint32_t flags);

/*
 * Global named memory can be accessed anywhere even in 32-bit mode
 */
static CVMX_SHARED uint64_t __cvmx_global_resources_addr = 0;

/**
 * This macro returns the size of a member of a structure.
 */
#define SIZEOF_FIELD(s, field) sizeof(((s *)NULL)->field)

/**
 * This function is the implementation of the get macros defined
 * for individual structure members. The argument are generated
 * by the macros inorder to read only the needed memory.
 *
 * @param base   64bit physical address of the complete structure
 * @param offset Offset from the beginning of the structure to the member being
 *               accessed.
 * @param size   Size of the structure member.
 *
 * @return Value of the structure member promoted into a uint64_t.
 */
static inline uint64_t __cvmx_struct_get_unsigned_field(uint64_t base, int offset,
					       		int size)
{
	base = (1ull << 63) | (base + offset);
	switch (size) {
	case 4:
		return cvmx_read64_uint32(base);
	case 8:
		return cvmx_read64_uint64(base);
	default:
		return 0;
	}
}

/**
 * This function is the implementation of the set macros defined
 * for individual structure members. The argument are generated
 * by the macros in order to write only the needed memory.
 *
 * @param base   64bit physical address of the complete structure
 * @param offset Offset from the beginning of the structure to the member being
 *               accessed.
 * @param size   Size of the structure member.
 * @param value  Value to write into the structure
 */
static inline void __cvmx_struct_set_unsigned_field(uint64_t base, int offset, int size,
					   		uint64_t value)
{
	base = (1ull << 63) | (base + offset);
	switch (size) {
	case 4:
		cvmx_write64_uint32(base, value);
		break;
	case 8:
		cvmx_write64_uint64(base, value);
		break;
	default:
		break;
	}
}

/* Get the global resource lock. */
static inline void __cvmx_global_resource_lock(void)
{
	uint64_t lock_addr = (1ull << 63) |
			(__cvmx_global_resources_addr + offsetof(cvmx_global_resources_t,
							   rlock));
	unsigned int tmp;

	__asm__ __volatile__(".set noreorder            \n"
			     "1: ll   %[tmp], 0(%[addr])\n"
			     "   bnez %[tmp], 1b        \n"
			     "   li   %[tmp], 1         \n"
			     "   sc   %[tmp], 0(%[addr])\n"
			     "   beqz %[tmp], 1b        \n"
			     "   nop                    \n"
			     ".set reorder              \n"
			     : [tmp] "=&r"(tmp)
			     : [addr] "r"(lock_addr)
			     : "memory");
}

/* Release the global resource lock. */
static inline void __cvmx_global_resource_unlock(void)
{
	uint64_t lock_addr = (1ull << 63) |
		(__cvmx_global_resources_addr + offsetof(cvmx_global_resources_t, rlock));
	CVMX_SYNCW;
	__asm__ __volatile__("sw $0, 0(%[addr])\n"
			     : : [addr] "r"(lock_addr)
			     : "memory");
	CVMX_SYNCW;
}

static uint64_t __cvmx_alloc_bootmem_for_global_resources(int sz)
{
       void *tmp;

       tmp = cvmx_bootmem_alloc_range(sz, CVMX_CACHE_LINE_SIZE, 0, 0);
       return cvmx_ptr_to_phys(tmp);
}

static inline void __cvmx_get_tagname(struct global_resource_tag *rtag,
					char *tagname)
{
	int i, j, k;

	j = 0;
	k = 8;
	for (i = 7 ; i >= 0; i--, j++, k++) {
		tagname[j] = (rtag->lo >> (i * 8)) & 0xff;
		tagname[k] = (rtag->hi >> (i * 8)) & 0xff;
	}
}

static uint64_t __cvmx_global_resources_init(void)
{
	cvmx_bootmem_named_block_desc_t *block_desc;
	int sz =  sizeof(cvmx_global_resources_t) + CVMX_RESOURCES_ENTRIES_SIZE;
	int64_t tmp_phys;
	int count = 0;
	uint64_t base = 0;

	cvmx_bootmem_lock();

	block_desc = (cvmx_bootmem_named_block_desc_t *)
		__cvmx_bootmem_find_named_block_flags(CVMX_GLOBAL_RESOURCES_DATA_NAME,
						      CVMX_BOOTMEM_FLAG_NO_LOCKING);
	if (!block_desc) {
		if (dbg)
			cvmx_dprintf("%s: allocating global resources\n", __func__);

		tmp_phys = cvmx_bootmem_phy_named_block_alloc(sz, 0, 0, CVMX_CACHE_LINE_SIZE,
							      CVMX_GLOBAL_RESOURCES_DATA_NAME,
							      CVMX_BOOTMEM_FLAG_NO_LOCKING);
		if (tmp_phys < 0) {
			cvmx_printf("ERROR: %s: failed to allocate global resource name block. sz=%d\n",
				__func__, sz);
			goto end;
		}
		__cvmx_global_resources_addr = (uint64_t) tmp_phys;

		if (dbg)
			cvmx_dprintf("%s: memset global resources %llu\n", __func__,
				     CAST_ULL(__cvmx_global_resources_addr));

		base = (1ull << 63) | __cvmx_global_resources_addr;
		for (count = 0; count < (sz/8); count++) {
			cvmx_write64_uint64(base, 0);
			base += 8;
		}
	} else {
		if (dbg)
			cvmx_dprintf("%s:found global resource\n", __func__);
		__cvmx_global_resources_addr = block_desc->base_addr;
	}
 end:
	cvmx_bootmem_unlock();
	if (dbg)
		cvmx_dprintf("__cvmx_global_resources_addr=%llu sz=%d \n",
			     CAST_ULL(__cvmx_global_resources_addr), sz);
	return __cvmx_global_resources_addr;
}

uint64_t cvmx_get_global_resource(struct global_resource_tag tag, int no_lock)
{
	uint64_t entry_cnt = 0;
	uint64_t resource_entry_addr  = 0;
	int count = 0;
	uint64_t rphys_addr = 0;
	uint64_t tag_lo = 0, tag_hi = 0;

	if (__cvmx_global_resources_addr == 0)
		__cvmx_global_resources_init();
	if (!no_lock)
		__cvmx_global_resource_lock();

	entry_cnt = CVMX_GLOBAL_RESOURCES_GET_FIELD(entry_cnt);
	while (entry_cnt > 0) {
		resource_entry_addr = CVMX_GET_RESOURCE_ENTRY(count);
		tag_lo = CVMX_RESOURCE_TAG_GET_FIELD(resource_entry_addr, lo);
		tag_hi = CVMX_RESOURCE_TAG_GET_FIELD(resource_entry_addr, hi);

		if (tag_lo == tag.lo && tag_hi == tag.hi) {
			if (dbg)
				cvmx_dprintf("%s: Found global resource entry\n", __func__);
			break;
		}
		entry_cnt--;
		count++;
	}

	if (entry_cnt == 0) {
		if (dbg)
			cvmx_dprintf("%s: no matching global resource entry found\n", __func__);
		if (!no_lock)
			__cvmx_global_resource_unlock();
		return 0;
	}
	rphys_addr = CVMX_RESOURCE_ENTRY_GET_FIELD(resource_entry_addr, phys_addr);
	if (!no_lock)
		__cvmx_global_resource_unlock();

	return rphys_addr;
}

uint64_t cvmx_create_global_resource(struct global_resource_tag tag, uint64_t size,
				     int no_lock, int *new)
{
	uint64_t entry_count = 0;
	uint64_t resource_entry_addr  = 0;
	uint64_t phys_addr;

	if (__cvmx_global_resources_addr == 0)
		__cvmx_global_resources_init();

	if (!no_lock)
		__cvmx_global_resource_lock();

	phys_addr = cvmx_get_global_resource(tag, CVMX_GLOBAL_RESOURCE_NO_LOCKING);
	if (phys_addr != 0) {
		/* we already have the resource, return it */
		*new = 0;
		goto end;
	}

	*new = 1;
	entry_count = CVMX_GLOBAL_RESOURCES_GET_FIELD(entry_cnt);
	if (entry_count >= CVMX_MAX_GLOBAL_RESOURCES) {
		char tagname[MAX_RESOURCE_TAG_LEN+1];

		__cvmx_get_tagname(&tag, tagname);
		cvmx_printf("ERROR: %s: reached global resources limit for %s\n",
			__func__, tagname);
		phys_addr = 0;
		goto end;
	}

        /* Allocate bootmem for the resource*/
	phys_addr = __cvmx_alloc_bootmem_for_global_resources(size);
	if (!phys_addr) {
		char tagname[MAX_RESOURCE_TAG_LEN+1];

		__cvmx_get_tagname(&tag, tagname);
		cvmx_dprintf("ERROR: %s: out of memory %s, size=%d\n",
			__func__, tagname, (int) size);
		goto end;
	}

	resource_entry_addr = CVMX_GET_RESOURCE_ENTRY(entry_count);
	CVMX_RESOURCE_ENTRY_SET_FIELD(resource_entry_addr, phys_addr, phys_addr);
	CVMX_RESOURCE_ENTRY_SET_FIELD(resource_entry_addr, size, size);
	CVMX_RESOURCE_TAG_SET_FIELD(resource_entry_addr, lo, tag.lo);
	CVMX_RESOURCE_TAG_SET_FIELD(resource_entry_addr, hi, tag.hi);
	/* update entry_cnt */
	entry_count += 1;
	CVMX_GLOBAL_RESOURCES_SET_FIELD(entry_cnt, entry_count);

 end:
	if (!no_lock)
		__cvmx_global_resource_unlock();

	return phys_addr;
}

int cvmx_create_global_resource_range(struct global_resource_tag tag, int nelements)
{
	int sz = cvmx_range_memory_size(nelements);
	int new;
	uint64_t addr;
	int rv=0;

	if (__cvmx_global_resources_addr == 0)
		__cvmx_global_resources_init();

	__cvmx_global_resource_lock();
	addr = cvmx_create_global_resource(tag, sz, 1, &new);
	if (!addr) {
		__cvmx_global_resource_unlock();
		return -1;
	}
	if (new) {
		rv = cvmx_range_init(addr, nelements);
	}
	__cvmx_global_resource_unlock();
	return rv;
}


int cvmx_allocate_global_resource_range(struct global_resource_tag tag, uint64_t owner,
					int nelements, int alignment)
{
	uint64_t addr = cvmx_get_global_resource(tag,1);
	int base;

	if (addr == 0) {
		char tagname[256];
		__cvmx_get_tagname(&tag, tagname);
		cvmx_printf("ERROR: %s: cannot find resource %s\n",
			__func__, tagname);
		return -1;
	}
	__cvmx_global_resource_lock();
	base = cvmx_range_alloc(addr, owner, nelements, alignment);
	__cvmx_global_resource_unlock();
	return base;
}

int cvmx_resource_alloc_many(struct global_resource_tag tag,
			     uint64_t owner,
			     int nelements,
			     int allocated_elements[]) {
	uint64_t addr = cvmx_get_global_resource(tag,1);
	int rv;

	if (addr == 0) {
		char tagname[256];
		__cvmx_get_tagname(&tag, tagname);
		cvmx_dprintf("ERROR: cannot find resource %s\n", tagname);
		return -1;
	}
	__cvmx_global_resource_lock();
	rv = cvmx_range_alloc_non_contiguos(addr, owner, nelements, allocated_elements);
	__cvmx_global_resource_unlock();
	return rv;
}

int cvmx_resource_alloc_reverse(struct global_resource_tag tag,
				uint64_t owner)
{
	uint64_t addr = cvmx_get_global_resource(tag, 1);
	int rv;

	if (addr == 0) {
		char tagname[256];
		__cvmx_get_tagname(&tag, tagname);
		cvmx_dprintf("ERROR: cannot find resource %s\n", tagname);
		return -1;
	}
	__cvmx_global_resource_lock();
	rv = cvmx_range_alloc_ordered(addr, owner, 1, 1, 1);
	__cvmx_global_resource_unlock();
	return rv;
}

int cvmx_reserve_global_resource_range(struct global_resource_tag tag,
				       uint64_t owner, int base,
				       int nelements)
{
	uint64_t addr = cvmx_get_global_resource(tag,1);
	int start;

	__cvmx_global_resource_lock();
	start = cvmx_range_reserve(addr, owner, base, nelements);
	__cvmx_global_resource_unlock();
	return start;
}

int cvmx_free_global_resource_range_with_base(struct global_resource_tag tag,
					      int base, int nelements)
{
	uint64_t addr = cvmx_get_global_resource(tag,1);
	int rv;

	/* Resource was not created, nothing to release */
	if (addr == 0)
		return 0;

	__cvmx_global_resource_lock();
	rv = cvmx_range_free_with_base(addr, base, nelements);
	__cvmx_global_resource_unlock();
	return rv;
}

int cvmx_free_global_resource_range_multiple(struct global_resource_tag tag,
					     int bases[], int nelements)
{
	uint64_t addr = cvmx_get_global_resource(tag,1);
	int rv;

	/* Resource was not created, nothing to release */
	if (addr == 0)
		return 0;

	__cvmx_global_resource_lock();
	rv = cvmx_range_free_mutiple(addr, bases, nelements);
	__cvmx_global_resource_unlock();
	return rv;
}

int cvmx_free_global_resource_range_with_owner(struct global_resource_tag tag,
					      int owner)
{
	uint64_t addr = cvmx_get_global_resource(tag,1);
	int rv;

	/* Resource was not created, nothing to release */
	if (addr == 0)
		return 0;

	__cvmx_global_resource_lock();
	rv = cvmx_range_free_with_owner(addr, owner);
	__cvmx_global_resource_unlock();
	return rv;
}

void cvmx_show_global_resource_range(struct global_resource_tag tag)
{
	uint64_t addr = cvmx_get_global_resource(tag,1);

	cvmx_range_show(addr);
}


int free_global_resources(void)
{
	int rc;
	int i, entry_cnt;
	uint64_t resource_entry_addr, phys_addr, size;

	if (__cvmx_global_resources_addr == 0)
		__cvmx_global_resources_init();

	__cvmx_global_resource_lock();

	entry_cnt = CVMX_GLOBAL_RESOURCES_GET_FIELD(entry_cnt);

	/* get and free all the global resources */
	for (i = 0; i < entry_cnt; i++) {
		resource_entry_addr = CVMX_GET_RESOURCE_ENTRY(i);
		phys_addr = CVMX_RESOURCE_ENTRY_GET_FIELD(resource_entry_addr, phys_addr);
		size = CVMX_RESOURCE_ENTRY_GET_FIELD(resource_entry_addr, size);
		/* free the resource */
		rc = __cvmx_bootmem_phy_free(phys_addr, size, 0);
		if (!rc) {
			cvmx_dprintf("ERROR: %s: could not free memory to bootmem\n", __func__);
		}
	}

	__cvmx_global_resource_unlock();

	rc = cvmx_bootmem_free_named(CVMX_GLOBAL_RESOURCES_DATA_NAME);
	if (dbg)
		cvmx_dprintf("freed global resources named block rc=%d \n",rc);

	__cvmx_global_resources_addr = 0;

	return 0;
}

uint64_t cvmx_get_global_resource_owner(struct global_resource_tag tag, int base)
{
	uint64_t addr = cvmx_get_global_resource(tag, 1);

	/* Resource was not created, return "available" special owner code */
	if (addr == 0)
		return -88LL;

	return cvmx_range_get_owner(addr, base);
}

void cvmx_global_resources_show(void)
{
	uint64_t entry_cnt;
	uint64_t p;
	char tagname[MAX_RESOURCE_TAG_LEN+1];
	struct global_resource_tag rtag;
	uint64_t count;
	uint64_t phys_addr;

	if (__cvmx_global_resources_addr == 0)
		__cvmx_global_resources_init();

	__cvmx_global_resource_lock();

	entry_cnt = CVMX_GLOBAL_RESOURCES_GET_FIELD(entry_cnt);
	memset (tagname, 0, MAX_RESOURCE_TAG_LEN + 1);

	for (count = 0; count < entry_cnt; count++) {
		p = CVMX_GET_RESOURCE_ENTRY(count);
		phys_addr = CVMX_RESOURCE_ENTRY_GET_FIELD(p, phys_addr);
		rtag.lo = CVMX_RESOURCE_TAG_GET_FIELD(p, lo);
		rtag.hi = CVMX_RESOURCE_TAG_GET_FIELD(p, hi);
		__cvmx_get_tagname(&rtag, tagname);
		cvmx_dprintf("Global Resource tag name: %s Resource Address: %llx\n",
			     tagname, CAST_ULL(phys_addr));
	}
	cvmx_dprintf("<End of Global Resources>\n");
	__cvmx_global_resource_unlock();

}
EXPORT_SYMBOL(free_global_resources);

void cvmx_app_id_init(void *bootmem)
{
	uint64_t *p = (uint64_t *) bootmem;

	*p = 0;
}

uint64_t cvmx_allocate_app_id(void)
{
	uint64_t *vptr;

	vptr = (uint64_t *)cvmx_bootmem_alloc_named_range_once(
		sizeof(cvmx_app_id), 0, 1<<31, 128,
		"cvmx_app_id", cvmx_app_id_init);

	cvmx_app_id = __atomic_add_fetch(vptr, 1, __ATOMIC_SEQ_CST);

	if (dbg)
		cvmx_dprintf("CVMX_APP_ID = %lx.\n",
			     (unsigned long)cvmx_app_id);
	return cvmx_app_id;
}

uint64_t cvmx_get_app_id(void)
{
	if (cvmx_app_id == 0)
		cvmx_allocate_app_id();
	return cvmx_app_id;
}
