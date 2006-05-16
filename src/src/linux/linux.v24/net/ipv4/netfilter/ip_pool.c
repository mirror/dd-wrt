/* Kernel module for IP pool management */

#include <linux/module.h>
#include <linux/ip.h>
#include <linux/skbuff.h>
#include <linux/netfilter_ipv4/ip_tables.h>
#include <linux/netfilter_ipv4/ip_pool.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#include <asm/bitops.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>

#define DP(format, args...)

MODULE_LICENSE("GPL");

#define NR_POOL 16
static int nr_pool = NR_POOL;/* overwrite this when loading module */

struct ip_pool {
	u_int32_t first_ip;	/* host byte order, included in range */
	u_int32_t last_ip;	/* host byte order, included in range */
	void *members;		/* the bitmap proper */
	int nr_use;		/* total nr. of tests through this */
	int nr_match;		/* total nr. of matches through this */
	rwlock_t lock;
};

static struct ip_pool *POOL;

static inline struct ip_pool *lookup(ip_pool_t index)
{
	if (index < 0 || index >= nr_pool) {
		DP("ip_pool:lookup: bad index %d\n", index);
		return 0;
	}
	return POOL+index;
}

int ip_pool_match(ip_pool_t index, u_int32_t addr)
{
        struct ip_pool *pool = lookup(index);
	int res = 0;

	if (!pool || !pool->members)
		return 0;
	read_lock_bh(&pool->lock);
	if (pool->members) {
		if (addr >= pool->first_ip && addr <= pool->last_ip) {
			addr -= pool->first_ip;
			if (test_bit(addr, pool->members)) {
				res = 1;
#ifdef CONFIG_IP_POOL_STATISTICS
				pool->nr_match++;
#endif
			}
		}
#ifdef CONFIG_IP_POOL_STATISTICS
		pool->nr_use++;
#endif
	}
	read_unlock_bh(&pool->lock);
	return res;
}

static int pool_change(ip_pool_t index, u_int32_t addr, int isdel)
{
	struct ip_pool *pool;
	int res = -1;

	pool = lookup(index);
	if (    !pool || !pool->members
	     || addr < pool->first_ip || addr > pool->last_ip)
		return -1;
	read_lock_bh(&pool->lock);
	if (pool->members && addr >= pool->first_ip && addr <= pool->last_ip) {
		addr -= pool->first_ip;
		res = isdel
			? (0 != test_and_clear_bit(addr, pool->members))
			: (0 != test_and_set_bit(addr, pool->members));
	}
	read_unlock_bh(&pool->lock);
	return res;
}

int ip_pool_mod(ip_pool_t index, u_int32_t addr, int isdel)
{
	int res = pool_change(index,addr,isdel);

	if (!isdel) res = !res;
	return res;
}

static inline int bitmap_bytes(u_int32_t a, u_int32_t b)
{
	return 4*((((b-a+8)/8)+3)/4);
}

static inline int poolbytes(ip_pool_t index)
{
	struct ip_pool *pool = lookup(index);

	return pool ? bitmap_bytes(pool->first_ip, pool->last_ip) : 0;
}

static int setpool(
	struct sock *sk,
	int optval,
	void *user,
	unsigned int len
) {
	struct ip_pool_request req;

	DP("ip_pool:setpool: optval=%d, user=%p, len=%d\n", optval, user, len);
	if (!capable(CAP_NET_ADMIN))
		return -EPERM;
	if (optval != SO_IP_POOL)
		return -EBADF;
	if (len != sizeof(req))
		return -EINVAL;
	if (copy_from_user(&req, user, sizeof(req)) != 0)
		return -EFAULT;
	printk("obsolete op - upgrade your ippool(8) utility.\n");
	return -EINVAL;
}

static int getpool(
	struct sock *sk,
	int optval,
	void *user,
	int *len
) {
	struct ip_pool_request req;
	struct ip_pool *pool;
	ip_pool_t i;
	int newbytes;
	void *newmembers;
	int res;

	DP("ip_pool:getpool: optval=%d, user=%p\n", optval, user);
	if (!capable(CAP_NET_ADMIN))
		return -EINVAL;
	if (optval != SO_IP_POOL)
		return -EINVAL;
	if (*len != sizeof(req)) {
		return -EFAULT;
	}
	if (copy_from_user(&req, user, sizeof(req)) != 0)
		return -EFAULT;
	DP("ip_pool:getpool op=%d, index=%d\n", req.op, req.index);
	if (req.op < IP_POOL_BAD001) {
		printk("obsolete op - upgrade your ippool(8) utility.\n");
		return -EFAULT;
	}
	switch(req.op) {
	case IP_POOL_HIGH_NR:
		DP("ip_pool HIGH_NR\n");
		req.index = IP_POOL_NONE;
		for (i=0; i<nr_pool; i++)
			if (POOL[i].members)
				req.index = i;
		return copy_to_user(user, &req, sizeof(req));
	case IP_POOL_LOOKUP:
		DP("ip_pool LOOKUP\n");
		pool = lookup(req.index);
		if (!pool)
			return -EINVAL;
		if (!pool->members)
			return -EBADF;
		req.addr = htonl(pool->first_ip);
		req.addr2 = htonl(pool->last_ip);
		return copy_to_user(user, &req, sizeof(req));
	case IP_POOL_USAGE:
		DP("ip_pool USE\n");
		pool = lookup(req.index);
		if (!pool)
			return -EINVAL;
		if (!pool->members)
			return -EBADF;
		req.addr = pool->nr_use;
		req.addr2 = pool->nr_match;
		return copy_to_user(user, &req, sizeof(req));
	case IP_POOL_TEST_ADDR:
		DP("ip_pool TEST 0x%08x\n", req.addr);
		pool = lookup(req.index);
		if (!pool)
			return -EINVAL;
		res = 0;
		read_lock_bh(&pool->lock);
		if (!pool->members) {
			DP("ip_pool TEST_ADDR no members in pool\n");
			res = -EBADF;
			goto unlock_and_return_res;
		}
		req.addr = ntohl(req.addr);
		if (req.addr < pool->first_ip) {
			DP("ip_pool TEST_ADDR address < pool bounds\n");
			res = -ERANGE;
			goto unlock_and_return_res;
		}
		if (req.addr > pool->last_ip) {
			DP("ip_pool TEST_ADDR address > pool bounds\n");
			res = -ERANGE;
			goto unlock_and_return_res;
		}
		req.addr = (0 != test_bit((req.addr - pool->first_ip),
					pool->members));
		read_unlock_bh(&pool->lock);
		return copy_to_user(user, &req, sizeof(req));
	case IP_POOL_FLUSH:
		DP("ip_pool FLUSH not yet implemented.\n");
		return -EBUSY;
	case IP_POOL_DESTROY:
		DP("ip_pool DESTROY not yet implemented.\n");
		return -EBUSY;
	case IP_POOL_INIT:
		DP("ip_pool INIT 0x%08x-0x%08x\n", req.addr, req.addr2);
		pool = lookup(req.index);
		if (!pool)
			return -EINVAL;
		req.addr = ntohl(req.addr);
		req.addr2 = ntohl(req.addr2);
		if (req.addr > req.addr2) {
			DP("ip_pool INIT bad ip range\n");
			return -EINVAL;
		}
		newbytes = bitmap_bytes(req.addr, req.addr2);
		newmembers = kmalloc(newbytes, GFP_KERNEL);
		if (!newmembers) {
			DP("ip_pool INIT out of mem for %d bytes\n", newbytes);
			return -ENOMEM;
		}
		memset(newmembers, 0, newbytes);
		write_lock_bh(&pool->lock);
		if (pool->members) {
			DP("ip_pool INIT pool %d exists\n", req.index);
			kfree(newmembers);
			res = -EBUSY;
			goto unlock_and_return_res;
		}
		pool->first_ip = req.addr;
		pool->last_ip = req.addr2;
		pool->nr_use = 0;
		pool->nr_match = 0;
		pool->members = newmembers;
		write_unlock_bh(&pool->lock);
		return 0;
	case IP_POOL_ADD_ADDR:
		DP("ip_pool ADD_ADDR 0x%08x\n", req.addr);
		req.addr = pool_change(req.index, ntohl(req.addr), 0);
		return copy_to_user(user, &req, sizeof(req));
	case IP_POOL_DEL_ADDR:
		DP("ip_pool DEL_ADDR 0x%08x\n", req.addr);
		req.addr = pool_change(req.index, ntohl(req.addr), 1);
		return copy_to_user(user, &req, sizeof(req));
	default:
		DP("ip_pool:getpool bad op %d\n", req.op);
		return -EINVAL;
	}
	return -EINVAL;

unlock_and_return_res:
	if (pool)
		read_unlock_bh(&pool->lock);
	return res;
}

static struct nf_sockopt_ops so_pool
= { { NULL, NULL }, PF_INET,
    SO_IP_POOL, SO_IP_POOL+1, &setpool,
    SO_IP_POOL, SO_IP_POOL+1, &getpool,
    0, NULL };

MODULE_PARM(nr_pool, "i");

static int __init init(void)
{
	ip_pool_t i;
	int res;

	if (nr_pool < 1) {
		printk("ip_pool module init: bad nr_pool %d\n", nr_pool);
		return -EINVAL;
	}
	POOL = kmalloc(nr_pool * sizeof(*POOL), GFP_KERNEL);
	if (!POOL) {
		printk("ip_pool module init: out of memory for nr_pool %d\n",
			nr_pool);
		return -ENOMEM;
	}
	for (i=0; i<nr_pool; i++) {
		POOL[i].first_ip = 0;
		POOL[i].last_ip = 0;
		POOL[i].members = 0;
		POOL[i].nr_use = 0;
		POOL[i].nr_match = 0;
		POOL[i].lock = RW_LOCK_UNLOCKED;
	}
	res = nf_register_sockopt(&so_pool);
	DP("ip_pool:init %d pools, result %d\n", nr_pool, res);
	if (res != 0) {
		kfree(POOL);
		POOL = 0;
	}
	return res;
}

static void __exit fini(void)
{
	ip_pool_t i;

	DP("ip_pool:fini BYEBYE\n");
	nf_unregister_sockopt(&so_pool);
	for (i=0; i<nr_pool; i++) {
		if (POOL[i].members) {
			kfree(POOL[i].members);
			POOL[i].members = 0;
		}
	}
	kfree(POOL);
	POOL = 0;
	DP("ip_pool:fini these are the famous last words\n");
	return;
}

module_init(init);
module_exit(fini);
