// SPDX-License-Identifier: GPL-2.0-or-later
/*
 *   Copyright (C) 2018 Samsung Electronics Co., Ltd.
 */

#include <linux/kernel.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/vmalloc.h>
#include <linux/rwlock.h>

#include "glob.h"
#include "buffer_pool.h"
#include "connection.h"
#include "mgmt/ksmbd_ida.h"

static struct kmem_cache *filp_cache;
static int threads; 
struct wm {
	struct list_head	list;
	size_t			sz;
	size_t			realsize;
	char			buffer[0];
};

struct wm_list {
	struct list_head	list;
	size_t			sz;

	spinlock_t		wm_lock;
	int			avail_wm;
	struct list_head	idle_wm;
	wait_queue_head_t	wm_wait;
};

static LIST_HEAD(wm_lists);
static DEFINE_RWLOCK(wm_lists_lock);

#if LINUX_VERSION_CODE <= KERNEL_VERSION(5, 0, 0)
/*
 * A simple kvmalloc()/kvfree() implementation.
 */
static inline void *__alloc(size_t size, gfp_t flags)
{
	void *ret;
	gfp_t kmalloc_flags = flags;

	/*
	 * We want to attempt a large physically contiguous block first because
	 * it is less likely to fragment multiple larger blocks and therefore
	 * contribute to a long term fragmentation less than vmalloc fallback.
	 * However make sure that larger requests are not too disruptive - no
	 * OOM killer and no allocation failure warnings as we have a fallback.
	 */
	if (size > PAGE_SIZE) {
		kmalloc_flags |= __GFP_NORETRY | __GFP_NOWARN;
		if (kmalloc_flags & GFP_KERNEL) {
			kmalloc_flags &= ~GFP_KERNEL;
			kmalloc_flags |= GFP_NOWAIT;
		}
	}
	
	ret = kmalloc(size,  kmalloc_flags);

	/*
	 * It doesn't really make sense to fallback to vmalloc for sub page
	 * requests
	 */
	if (ret || size <= PAGE_SIZE)
		return ret;
	
	return __vmalloc(size, flags, PAGE_KERNEL);
}

static inline void __free(void *addr)
{
	if (is_vmalloc_addr(addr))
		vfree(addr);
	else
		kfree(addr);
}

#endif

void *_ksmbd_alloc(size_t size, const char *func, int line)
{
	void *p;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(5, 0, 0)
 	p = __alloc(size, GFP_KERNEL);
#else
	p = kvmalloc(size, GFP_KERNEL);
#endif
	if (!p) {
		printk(KERN_WARNING "%s: allocation failed at %s:%d\n", __func__, func, line);
	}
	return p;
}

void *_ksmbd_zalloc(size_t size, const char *func, int line)
{
	void *p;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(5, 0, 0)
 	p = __alloc(size, GFP_KERNEL | __GFP_ZERO);
#else
	p = kvmalloc(size, GFP_KERNEL | __GFP_ZERO);
#endif
	if (!p) {
		printk(KERN_WARNING "%s: allocation failed at %s:%d\n", __func__, func, line);
	}
	return p;
}

void ksmbd_free(void *ptr)
{
	__free(ptr);
}

static struct wm *wm_alloc(size_t sz)
{
	struct wm *wm;
	size_t alloc_sz = sz + sizeof(struct wm);

	if (sz > SIZE_MAX - sizeof(struct wm))
		return NULL;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(5, 0, 0)
 	wm = __alloc(alloc_sz, GFP_KERNEL);
#else
	wm = kvmalloc(alloc_sz, GFP_KERNEL);
#endif
	if (!wm)
		return NULL;
	wm->sz = sz;
	wm->realsize = sz;
	return wm;
}

static int register_wm_size_class(size_t sz)
{
	struct wm_list *l, *nl;

#if LINUX_VERSION_CODE <= KERNEL_VERSION(5, 0, 0)
	nl = __alloc(sizeof(struct wm_list), GFP_KERNEL);
#else
	nl = kvmalloc(sizeof(struct wm_list), GFP_KERNEL);
#endif
	if (!nl) {
		printk(KERN_ERR "Out of memory in %s:%d\n", __func__,__LINE__);
		return -ENOMEM;
	}

	nl->sz = sz;
	spin_lock_init(&nl->wm_lock);
	INIT_LIST_HEAD(&nl->idle_wm);
	INIT_LIST_HEAD(&nl->list);
	init_waitqueue_head(&nl->wm_wait);
	nl->avail_wm = 0;

	write_lock(&wm_lists_lock);
	list_for_each_entry(l, &wm_lists, list) {
		if (l->sz == sz) {
			write_unlock(&wm_lists_lock);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(5, 0, 0)
			__free(nl);
#else
			kvfree(nl);
#endif
			return 0;
		}
	}

	list_add(&nl->list, &wm_lists);
	write_unlock(&wm_lists_lock);
	return 0;
}

static struct wm_list *match_wm_list(size_t size)
{
	struct wm_list *l, *rl = NULL;

	read_lock(&wm_lists_lock);
	list_for_each_entry(l, &wm_lists, list) {
		if (l->sz == size) {
			rl = l;
			break;
		}
	}
	read_unlock(&wm_lists_lock);
	return rl;
}

static struct wm_list *search_wm_list(size_t size, size_t *realsize)
{
	struct wm_list *l, *rl = NULL;
	size_t last_size = (size_t)-1;
	read_lock(&wm_lists_lock);
	list_for_each_entry(l, &wm_lists, list) {
		if (l->sz == size) {
			rl = l;
			break;
		}

		if (l->sz > size && l->sz < last_size) {
			last_size = l->sz;
			rl = l;
		}
	}
	if (rl)
	    *realsize = rl->sz;
	read_unlock(&wm_lists_lock);
	return rl;
}

static struct wm *find_wm(size_t size)
{
	struct wm_list *wm_list;
	struct wm *wm;
	size_t realsize = size;

	wm_list = search_wm_list(size, &realsize);
	if (!wm_list) {
		if (register_wm_size_class(size))
			return NULL;
		wm_list = match_wm_list(size);
	}

	if (!wm_list)
		return NULL;

	while (1) {
		spin_lock(&wm_list->wm_lock);
		if (!list_empty(&wm_list->idle_wm)) {
			wm = list_entry(wm_list->idle_wm.next,
					struct wm,
					list);
			list_del(&wm->list);
			spin_unlock(&wm_list->wm_lock);
			wm->realsize = realsize;
			return wm;
		}

		if (wm_list->avail_wm > threads) {
			spin_unlock(&wm_list->wm_lock);
			wait_event(wm_list->wm_wait,
				   !list_empty(&wm_list->idle_wm));
			continue;
		}

		wm_list->avail_wm++;
		spin_unlock(&wm_list->wm_lock);

		wm = wm_alloc(realsize);
		if (!wm) {
			spin_lock(&wm_list->wm_lock);
			wm_list->avail_wm--;
			spin_unlock(&wm_list->wm_lock);
			wait_event(wm_list->wm_wait,
				   !list_empty(&wm_list->idle_wm));
			continue;
		}
		break;
	}

	wm->realsize = realsize;
	return wm;
}

static void release_wm(struct wm *wm, struct wm_list *wm_list)
{
	if (!wm)
		return;

	spin_lock(&wm_list->wm_lock);
	if (wm_list->avail_wm <= threads) {
		list_add(&wm->list, &wm_list->idle_wm);
		spin_unlock(&wm_list->wm_lock);
		wake_up(&wm_list->wm_wait);
		return;
	}

	wm_list->avail_wm--;
	spin_unlock(&wm_list->wm_lock);
	ksmbd_free(wm);
}

static void wm_list_free(struct wm_list *l)
{
	struct wm *wm;

	while (!list_empty(&l->idle_wm)) {
		wm = list_entry(l->idle_wm.next, struct wm, list);
		list_del(&wm->list);
#if LINUX_VERSION_CODE <= KERNEL_VERSION(5, 0, 0)
		__free(wm);
#else
		kvfree(wm);
#endif
	}
#if LINUX_VERSION_CODE <= KERNEL_VERSION(5, 0, 0)
	__free(l);
#else
	kvfree(l);
#endif
}

static void wm_lists_destroy(void)
{
	struct wm_list *l;

	while (!list_empty(&wm_lists)) {
		l = list_entry(wm_lists.next, struct wm_list, list);
		list_del(&l->list);
		wm_list_free(l);
	}
}

void ksmbd_free_request(void *addr)
{
#if LINUX_VERSION_CODE <= KERNEL_VERSION(5, 0, 0)
	__free(addr);
#else
	kvfree(addr);
#endif
}

void *_ksmbd_alloc_request(size_t size, const char *func, int line)
{
	void *p;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(5, 0, 0)
	p = __alloc(size, GFP_KERNEL);
#else
	p = kvmalloc(size, GFP_KERNEL);
#endif
	if (!p) {
		printk(KERN_WARNING "%s: allocation failed at %s:%d\n", __func__, func, line);
	}
	return p;
}

void ksmbd_free_response(void *buffer)
{
#if LINUX_VERSION_CODE <= KERNEL_VERSION(5, 0, 0)
	__free(buffer);
#else
	kvfree(buffer);
#endif
}

void *_ksmbd_alloc_response(size_t size, const char *func, int line)
{
	void *p;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(5, 0, 0)
	p = __alloc(size, GFP_KERNEL | __GFP_ZERO);
#else
	p = kvmalloc(size, GFP_KERNEL | __GFP_ZERO);
#endif
	if (!p) {
		printk(KERN_WARNING "%s: allocation failed at %s:%d\n", __func__, func, line);
	}
	return p;
}

void *_ksmbd_find_buffer(size_t size, const char *func, int line)
{
	struct wm *wm;

	wm = find_wm(size);

	WARN_ON(!wm);
	if (wm)
		return wm->buffer;
	printk(KERN_WARNING "%s: allocation failed at %s:%d\n", __func__, func, line);
	return NULL;
}

void ksmbd_release_buffer(void *buffer)
{
	struct wm_list *wm_list;
	struct wm *wm;

	if (!buffer)
		return;

	wm = container_of(buffer, struct wm, buffer);
	wm_list = match_wm_list(wm->realsize);
	WARN_ON(!wm_list);
	if (wm_list)
		release_wm(wm, wm_list);
}

void *_ksmbd_realloc_response(void *ptr, size_t old_sz, size_t new_sz, const char *func, int line)
{
	size_t sz = min(old_sz, new_sz);
	void *nptr;

	nptr = _ksmbd_alloc_response(new_sz, func, line);
	if (!nptr)
		return ptr;
	memcpy(nptr, ptr, sz);
	ksmbd_free_response(ptr);
	return nptr;
}

void ksmbd_free_file_struct(void *filp)
{
	kmem_cache_free(filp_cache, filp);
}

void *ksmbd_alloc_file_struct(void)
{
	return kmem_cache_zalloc(filp_cache, 0);
}

void ksmbd_destroy_buffer_pools(void)
{
	wm_lists_destroy();
	ksmbd_work_pool_destroy();
	kmem_cache_destroy(filp_cache);
}

int ksmbd_init_buffer_pools(void)
{
	threads = num_online_cpus() * 2;
	if (ksmbd_work_pool_init())
		goto out;
	
	filp_cache = kmem_cache_create("ksmbd_file_cache",
				       sizeof(struct ksmbd_file), 0,
				       SLAB_HWCACHE_ALIGN, NULL);
	if (!filp_cache)
		goto out;

	return 0;

out:
	pr_err("failed to allocate memory\n");
	ksmbd_destroy_buffer_pools();
	return -ENOMEM;
}
