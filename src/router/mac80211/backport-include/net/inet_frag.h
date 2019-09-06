#ifndef __BACKPORT__NET_FRAG_H__
#define __BACKPORT__NET_FRAG_H__
#include_next <net/inet_frag.h>
#include <linux/version.h>

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,9,0)
/* Memory Tracking Functions. */
#define frag_mem_limit LINUX_BACKPORT(frag_mem_limit)
static inline int frag_mem_limit(struct netns_frags *nf)
{
	return atomic_read(&nf->mem);
}

#define init_frag_mem_limit LINUX_BACKPORT(init_frag_mem_limit)
static inline void init_frag_mem_limit(struct netns_frags *nf)
{
	atomic_set(&nf->mem, 0);
}

#define sum_frag_mem_limit LINUX_BACKPORT(sum_frag_mem_limit)
static inline int sum_frag_mem_limit(struct netns_frags *nf)
{
	return atomic_read(&nf->mem);
}

#define inet_frag_maybe_warn_overflow LINUX_BACKPORT(inet_frag_maybe_warn_overflow)
void inet_frag_maybe_warn_overflow(struct inet_frag_queue *q,
				   const char *prefix);
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(3,9,0) */

/* the type of the paramater changed with kernel 4.3 */
#if LINUX_VERSION_CODE < KERNEL_VERSION(3,9,0)
#define sub_frag_mem_limit LINUX_BACKPORT(sub_frag_mem_limit)
static inline void sub_frag_mem_limit(struct netns_frags *nf, int i)
{
	atomic_sub(i, &nf->mem);
}

#define add_frag_mem_limit LINUX_BACKPORT(add_frag_mem_limit)
static inline void add_frag_mem_limit(struct netns_frags *nf, int i)
{
	atomic_add(i, &nf->mem);
}
#elif LINUX_VERSION_CODE < KERNEL_VERSION(4,3,0)
#define sub_frag_mem_limit LINUX_BACKPORT(sub_frag_mem_limit)
static inline void sub_frag_mem_limit(struct netns_frags *nf, int i)
{
	__percpu_counter_add(&nf->mem, -i, frag_percpu_counter_batch);
}

#define add_frag_mem_limit LINUX_BACKPORT(add_frag_mem_limit)
static inline void add_frag_mem_limit(struct netns_frags *nf, int i)
{
	__percpu_counter_add(&nf->mem, i, frag_percpu_counter_batch);
}
#endif /* LINUX_VERSION_CODE < KERNEL_VERSION(4,3,0) */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0) && \
    LINUX_VERSION_CODE >= KERNEL_VERSION(3,9,0)
#define inet_frags_uninit_net LINUX_BACKPORT(inet_frags_uninit_net)
static inline void inet_frags_uninit_net(struct netns_frags *nf)
{
	percpu_counter_destroy(&nf->mem);
}
#endif /* < 4.4 && >= 3.9 */

#if LINUX_VERSION_CODE < KERNEL_VERSION(4,4,0)
static inline int backport_inet_frags_init_net(struct netns_frags *nf)
{
	inet_frags_init_net(nf);
	return 0;
}
#define inet_frags_init_net LINUX_BACKPORT(inet_frags_init_net)
#endif /* < 4.4 */

#endif /* __BACKPORT__NET_FRAG_H__ */
