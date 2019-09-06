#ifndef __BACKPORT_MM_H
#define __BACKPORT_MM_H
#include_next <linux/mm.h>
#include <linux/page_ref.h>
#include <linux/sched.h>

#ifndef VM_NODUMP
/*
 * defined here to allow things to compile but technically
 * using this for memory regions will yield in a no-op on newer
 * kernels but on older kernels (v3.3 and older) this bit was used
 * for VM_ALWAYSDUMP. The goal was to remove this bit moving forward
 * and since we can't skip the core dump on old kernels we just make
 * this bit name now a no-op.
 *
 * For details see commits: 909af7 accb61fe cdaaa7003
 */
#define VM_NODUMP      0x0
#endif

#ifndef VM_DONTDUMP
#define VM_DONTDUMP    VM_NODUMP
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,15,0)
#define kvfree LINUX_BACKPORT(kvfree)
void kvfree(const void *addr);
#endif /* < 3.15 */

#if (LINUX_VERSION_CODE < KERNEL_VERSION(3,20,0))
#define get_user_pages_locked LINUX_BACKPORT(get_user_pages_locked)
long get_user_pages_locked(unsigned long start, unsigned long nr_pages,
		    int write, int force, struct page **pages, int *locked);
#define get_user_pages_unlocked LINUX_BACKPORT(get_user_pages_unlocked)
long get_user_pages_unlocked(unsigned long start, unsigned long nr_pages,
		    int write, int force, struct page **pages);
#elif (LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0))
static inline
long backport_get_user_pages_locked(unsigned long start, unsigned long nr_pages,
		    int write, int force, struct page **pages, int *locked)
{
	return get_user_pages_locked(current, current->mm, start, nr_pages,
		    write, force, pages, locked);
}

#define get_user_pages_locked LINUX_BACKPORT(get_user_pages_locked)

static inline
long backport_get_user_pages_unlocked(unsigned long start, unsigned long nr_pages,
				      int write, int force, struct page **pages)
{
	return get_user_pages_unlocked(current, current->mm, start,  nr_pages,
		    write, force, pages);
}
#define get_user_pages_unlocked LINUX_BACKPORT(get_user_pages_unlocked)
#endif

#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0))
static inline
long backport_get_user_pages(unsigned long start, unsigned long nr_pages,
			    int write, int force, struct page **pages,
			    struct vm_area_struct **vmas)
{
	return get_user_pages(current, current->mm, start,  nr_pages,
		    write, force, pages, vmas);
}
#define get_user_pages LINUX_BACKPORT(get_user_pages)
#endif

#ifndef FOLL_TRIED
#define FOLL_TRIED	0x800	/* a retry, previous pass started an IO */
#endif

#ifdef CPTCFG_BPAUTO_BUILD_FRAME_VECTOR
/* Container for pinned pfns / pages */
struct frame_vector {
	unsigned int nr_allocated;	/* Number of frames we have space for */
	unsigned int nr_frames;	/* Number of frames stored in ptrs array */
	bool got_ref;		/* Did we pin pages by getting page ref? */
	bool is_pfns;		/* Does array contain pages or pfns? */
	void *ptrs[0];		/* Array of pinned pfns / pages. Use
				 * pfns_vector_pages() or pfns_vector_pfns()
				 * for access */
};

struct frame_vector *frame_vector_create(unsigned int nr_frames);
void frame_vector_destroy(struct frame_vector *vec);
int get_vaddr_frames(unsigned long start, unsigned int nr_pfns,
		     bool write, bool force, struct frame_vector *vec);
void put_vaddr_frames(struct frame_vector *vec);
int frame_vector_to_pages(struct frame_vector *vec);
void frame_vector_to_pfns(struct frame_vector *vec);

static inline unsigned int frame_vector_count(struct frame_vector *vec)
{
	return vec->nr_frames;
}

static inline struct page **frame_vector_pages(struct frame_vector *vec)
{
	if (vec->is_pfns) {
		int err = frame_vector_to_pages(vec);

		if (err)
			return ERR_PTR(err);
	}
	return (struct page **)(vec->ptrs);
}

static inline unsigned long *frame_vector_pfns(struct frame_vector *vec)
{
	if (!vec->is_pfns)
		frame_vector_to_pfns(vec);
	return (unsigned long *)(vec->ptrs);
}
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,1,9) && \
     LINUX_VERSION_CODE >= KERNEL_VERSION(3,6,0)
#define page_is_pfmemalloc LINUX_BACKPORT(page_is_pfmemalloc)
static inline bool page_is_pfmemalloc(struct page *page)
{
	return page->pfmemalloc;
}
#endif /* < 4.2 */

#endif /* __BACKPORT_MM_H */
