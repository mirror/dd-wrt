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

/*
 * A simple kvmalloc()/kvfree() implementation.
 */
static inline void *__alloc(size_t size, gfp_t flags)
{
	void *ret;

	/*
	 * We want to attempt a large physically contiguous block first because
	 * it is less likely to fragment multiple larger blocks and therefore
	 * contribute to a long term fragmentation less than vmalloc fallback.
	 * However make sure that larger requests are not too disruptive - no
	 * OOM killer and no allocation failure warnings as we have a fallback.
	 */

	ret = kmalloc(size,  flags | GFP_NOWAIT | __GFP_NORETRY | __GFP_NOWARN);

	/*
	 * It doesn't really make sense to fallback to vmalloc for sub page
	 * requests
	 */
	if (ret || size <= PAGE_SIZE) {
		if (!ret)
			printk(KERN_EMERG "alloc fail %d\n", size);
		return ret;
	} else {
		printk(KERN_EMERG "alloc success %d\n", size);
	
	}

	return __vmalloc(size, flags, PAGE_KERNEL);
}

static inline void __free(void *addr)
{
	if (is_vmalloc_addr(addr))
		vfree(addr);
	else
		kfree(addr);
}

void *ksmbd_alloc(size_t size)
{
	return __alloc(size, 0);
}

void *ksmbd_zalloc(size_t size)
{
	return __alloc(size, __GFP_ZERO);
}

void ksmbd_free(void *ptr)
{
	__free(ptr);
}

static struct wm *wm_alloc(size_t sz, gfp_t flags)
{
	struct wm *wm;
	size_t alloc_sz = sz + sizeof(struct wm);

	wm = __alloc(alloc_sz, flags);
	if (!wm)
		return NULL;
	wm->sz = sz;
	return wm;
}

static int register_wm_size_class(size_t sz)
{
	struct wm_list *l, *nl;

	nl = __alloc(sizeof(struct wm_list), 0);
	if (!nl)
		return -ENOMEM;

	nl->sz = sz;
	spin_lock_init(&nl->wm_lock);
	INIT_LIST_HEAD(&nl->idle_wm);
	init_waitqueue_head(&nl->wm_wait);
	nl->avail_wm = 0;

	write_lock(&wm_lists_lock);
	list_for_each_entry(l, &wm_lists, list) {
		if (l->sz == sz) {
			write_unlock(&wm_lists_lock);
			__free(nl);
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

static struct wm *find_wm(size_t size, gfp_t flags)
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

		if (wm_list->avail_wm > num_online_cpus()) {
			spin_unlock(&wm_list->wm_lock);
			wait_event(wm_list->wm_wait,
				   !list_empty(&wm_list->idle_wm));
			continue;
		}

		wm_list->avail_wm++;
		spin_unlock(&wm_list->wm_lock);

		wm = wm_alloc(realsize, flags);
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
	if (flags & __GFP_ZERO)
	    memset(wm->buffer, 0, wm->sz);

	return wm;
}

static void release_wm(struct wm *wm, struct wm_list *wm_list)
{
	if (!wm)
		return;

	spin_lock(&wm_list->wm_lock);
	if (wm_list->avail_wm <= num_online_cpus()) {
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
		__free(wm);
	}
	__free(l);
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
	__free(addr);
}

void *ksmbd_alloc_request(size_t size)
{
	return __alloc(size, 0);
}

void ksmbd_free_response(void *buffer)
{
	__free(buffer);
}

void *ksmbd_alloc_response(size_t size)
{
	return __alloc(size, 0);
}

void *ksmbd_find_buffer(size_t size)
{
	struct wm *wm;

	wm = find_wm(size, 0);

	WARN_ON(!wm);
	if (wm)
		return wm->buffer;
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

void *ksmbd_realloc_response(void *ptr, size_t old_sz, size_t new_sz)
{
	size_t sz = min(old_sz, new_sz);
	void *nptr;

	nptr = ksmbd_alloc_response(new_sz);
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
	if (ksmbd_work_pool_init())
		goto out;

	filp_cache = kmem_cache_create("ksmbd_file_cache",
					sizeof(struct ksmbd_file), 0,
					SLAB_HWCACHE_ALIGN, NULL);
	if (!filp_cache)
		goto out;

	return 0;

out:
	ksmbd_err("failed to allocate memory\n");
	ksmbd_destroy_buffer_pools();
	return -ENOMEM;
}
