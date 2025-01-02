/*
 * Driver for the Octeon III Free Pool Unit (fpa).
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2015 Cavium Networks, Inc.
 */

#include <linux/module.h>

#include <asm/octeon/octeon.h>

static DEFINE_MUTEX(octeon_fpa3_lock);

/*
 * octeon_fpa3_init:		Initialize the fpa to default values.
 *
 *  node:			Node of fpa to initialize.
 *
 *  Returns:			Zero on success, error otherwise.
 */
int octeon_fpa3_init(int node)
{
	union cvmx_fpa_gen_cfg	fpa_cfg;
	static int		init_done;
	int			i;
	int			aura_cnt;

	mutex_lock(&octeon_fpa3_lock);

	if (init_done)
		goto done;

	aura_cnt = cvmx_fpa3_num_auras();
	for (i = 0; i < aura_cnt; i++) {
		cvmx_write_csr_node(node, CVMX_FPA_AURAX_CNT(i),
				    0x100000000ull);
		cvmx_write_csr_node(node, CVMX_FPA_AURAX_CNT_LIMIT(i),
				    0xfffffffffull);
		cvmx_write_csr_node(node, CVMX_FPA_AURAX_CNT_THRESHOLD(i),
				    0xffffffffeull);
	}

	fpa_cfg.u64 = cvmx_read_csr_node(node, CVMX_FPA_GEN_CFG);
	fpa_cfg.s.lvl_dly = 3;
	cvmx_write_csr_node(node, CVMX_FPA_GEN_CFG, fpa_cfg.u64);

	init_done = 1;
 done:
	mutex_unlock(&octeon_fpa3_lock);
	return 0;
}
EXPORT_SYMBOL(octeon_fpa3_init);

/*
 * octeon_fpa3_pool_init:	Initialize a pool.
 *
 *  node:			Node to initialize pool on.
 *  pool_num:			Requested pool number (-1 for don't care).
 *  pool:			Updated with the initialized pool number.
 *  pool_stack:			Updated with the base of the memory allocated
 *				for the pool stack.
 *  num_ptrs:			Number of pointers to allocated on the stack.
 *
 *  Returns:			Zero on success, error otherwise.
 */
int octeon_fpa3_pool_init(int			node,
			  int			pool_num,
			  cvmx_fpa3_pool_t	*pool,
			  void			**pool_stack,
			  int			num_ptrs)
{
	u64				pool_stack_start;
	u64				pool_stack_end;
	union cvmx_fpa_poolx_end_addr	limit_addr;
	union cvmx_fpa_poolx_cfg	cfg;
	int				stack_size;
	int				rc = 0;

	mutex_lock(&octeon_fpa3_lock);

	*pool = cvmx_fpa3_reserve_pool(node, pool_num);
	if (!__cvmx_fpa3_pool_valid(*pool)) {
		pr_err("Failed to reserve pool=%d\n", pool_num);
		rc = -ENODEV;
		goto error;
	}

	stack_size = (DIV_ROUND_UP(num_ptrs, 29) + 1) * 128;

	cvmx_write_csr_node(node, CVMX_FPA_POOLX_CFG(pool->lpool), 0);
	cvmx_write_csr_node(node, CVMX_FPA_POOLX_START_ADDR(pool->lpool), 128);
	limit_addr.u64 = 0;
	limit_addr.cn78xx.addr = ~0ll;
	cvmx_write_csr_node(node, CVMX_FPA_POOLX_END_ADDR(pool->lpool),
			    limit_addr.u64);

	*pool_stack = kmalloc_node(stack_size, GFP_KERNEL, node);
	if (!*pool_stack) {
		pr_err("Failed to allocate pool stack memory pool=%d\n",
		       pool_num);
		rc = -ENOMEM;
		goto error_stack;
	}

	pool_stack_start = virt_to_phys(*pool_stack);
	pool_stack_end = round_down(pool_stack_start + stack_size, 128);
	pool_stack_start = round_up(pool_stack_start, 128);

	cvmx_write_csr_node(node, CVMX_FPA_POOLX_STACK_BASE(pool->lpool),
			    pool_stack_start);
	cvmx_write_csr_node(node, CVMX_FPA_POOLX_STACK_ADDR(pool->lpool),
			    pool_stack_start);
	cvmx_write_csr_node(node, CVMX_FPA_POOLX_STACK_END(pool->lpool),
			    pool_stack_end);

	cfg.u64 = 0;
	cfg.s.l_type = 2; /* Load with DWB */
	cfg.s.ena = 1;
	cvmx_write_csr_node(node, CVMX_FPA_POOLX_CFG(pool->lpool), cfg.u64);

	mutex_unlock(&octeon_fpa3_lock);
	return 0;

 error_stack:
	cvmx_fpa3_release_pool(*pool);
 error:
	mutex_unlock(&octeon_fpa3_lock);
	return rc;
}
EXPORT_SYMBOL(octeon_fpa3_pool_init);

/*
 * octeon_fpa3_aura_init:	Initialize an aura.
 *
 *  pool:			Pool the aura belongs to.
 *  aura_num:			Requested aura number (-1 for don't care).
 *  aura:			Updated with the initialized aura number.
 *  num_bufs:			Number of buffers in the aura.
 *  limit:			Limit for the aura.
 *
 *  Returns:			Zero on success, error otherwise.
 */
int octeon_fpa3_aura_init(cvmx_fpa3_pool_t	pool,
			  int			aura_num,
			  cvmx_fpa3_gaura_t	*aura,
			  int			num_bufs,
			  unsigned int		limit)
{
	union cvmx_fpa_aurax_cnt_levels	cnt_levels;
	int				shift;
	unsigned int			drop;
	unsigned int			pass;
	int				rc = 0;

	mutex_lock(&octeon_fpa3_lock);

	*aura = cvmx_fpa3_reserve_aura(pool.node, aura_num);
	if (!__cvmx_fpa3_aura_valid(*aura)) {
		pr_err("Failed to reserved aura=%d\n", aura_num);
		rc = -ENODEV;
		goto error;
	}

	limit *= 2; /* Allow twice the limit before saturation at zero. */

	cvmx_write_csr_node(aura->node, CVMX_FPA_AURAX_CFG(aura->laura), 0);
	cvmx_write_csr_node(aura->node, CVMX_FPA_AURAX_CNT_LIMIT(aura->laura),
			    limit);
	cvmx_write_csr_node(aura->node, CVMX_FPA_AURAX_CNT(aura->laura), limit);
	cvmx_write_csr_node(aura->node, CVMX_FPA_AURAX_POOL(aura->laura),
			    pool.lpool);
	cvmx_write_csr_node(aura->node, CVMX_FPA_AURAX_POOL_LEVELS(aura->laura),
			    0); /* No per-pool RED/Drop */

	shift = 0;
	while ((limit >> shift) > 255)
		shift++;

	drop = (limit - num_bufs / 20) >> shift;	/* 95% */
	pass = (limit - (num_bufs * 3) / 20) >> shift;	/* 85% */

	cnt_levels.u64 = 0;
	cnt_levels.s.shift = shift;
	cnt_levels.s.red_ena = 1;
	cnt_levels.s.drop = drop;
	cnt_levels.s.pass = pass;
	cvmx_write_csr_node(aura->node, CVMX_FPA_AURAX_CNT_LEVELS(aura->laura),
			    cnt_levels.u64);

 error:
	mutex_unlock(&octeon_fpa3_lock);
	return rc;
}
EXPORT_SYMBOL(octeon_fpa3_aura_init);

/*
 * octeon_mem_fill_fpa3:	Add buffers to an aura.
 *
 *  node:			Node to get memory from.
 *  cache:			Memory cache to allocate from.
 *  aura:			Aura to add buffers to.
 *  num_bufs:			Number of buffers to add to the aura.
 *
 *  Returns:			Zero on success, error otherwise.
 */
int octeon_mem_fill_fpa3(int			node,
			 struct kmem_cache	*cache,
			 cvmx_fpa3_gaura_t	aura,
			 int			num_bufs)
{
	void	*mem;
	int	i;
	int	rc = 0;

	mutex_lock(&octeon_fpa3_lock);

	for (i = 0; i < num_bufs; i++) {
		mem = kmem_cache_alloc_node(cache, GFP_KERNEL, node);
		if (!mem) {
			pr_err("Failed to allocate memory for aura=%d\n",
			       aura.laura);
			rc = -ENOMEM;
			break;
		}
		cvmx_fpa3_free(mem, aura, 0);
	}

	mutex_unlock(&octeon_fpa3_lock);
	return rc;
}
EXPORT_SYMBOL(octeon_mem_fill_fpa3);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Cavium, Inc. Octeon III FPA manager.");
