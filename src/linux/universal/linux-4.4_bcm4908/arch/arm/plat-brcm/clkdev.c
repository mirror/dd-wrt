/*
 *  arch/arm/common/clkdev.c
 *
 *  Copyright (C) 2008 Russell King.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Helper for the clk API to assist looking up a struct clk.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/clk.h>
#include <linux/slab.h>

#include <asm/clkdev.h>
#include <mach/clkdev.h>

static LIST_HEAD(clocks);
static DEFINE_MUTEX(clocks_mutex);

static struct clk_hw *__clk_get_hw(struct clk *clk)
{
	return (struct clk_hw *)clk;
}

/*
 * Find the correct struct clk for the device and connection ID.
 * We do slightly fuzzy matching here:
 *  An entry with a NULL ID is assumed to be a wildcard.
 *  If an entry has a device ID, it must match
 *  If an entry has a connection ID, it must match
 * Then we take the most specific entry - with the following
 * order of precedence: dev+con > dev only > con only.
 */
static struct clk *clk_find(const char *dev_id, const char *con_id)
{
	struct clk_lookup *p;
	struct clk *clk = NULL;
	int match, best = 0;

	list_for_each_entry(p, &clocks, node) {
		match = 0;
		if (p->dev_id) {
			if (!dev_id || strcmp(p->dev_id, dev_id))
				continue;
			match += 2;
		}
		if (p->con_id) {
			if (!con_id || strcmp(p->con_id, con_id))
				continue;
			match += 1;
		}

		if (match > best) {
			clk = p->clk;
			if (match != 3)
				best = match;
			else
				break;
		}
	}
	return clk;
}

struct clk *clk_get_sys(const char *dev_id, const char *con_id)
{
	struct clk *clk;

	mutex_lock(&clocks_mutex);
	clk = clk_find(dev_id, con_id);
	if (clk && !__clk_get(clk))
		clk = NULL;
	mutex_unlock(&clocks_mutex);

	return clk ? clk : ERR_PTR(-ENOENT);
}
EXPORT_SYMBOL(clk_get_sys);

struct clk *clk_get(struct device *dev, const char *con_id)
{
	const char *dev_id = dev ? dev_name(dev) : NULL;

	return clk_get_sys(dev_id, con_id);
}
EXPORT_SYMBOL(clk_get);

void clk_put(struct clk *clk)
{
	__clk_put(clk);
}
EXPORT_SYMBOL(clk_put);
static void __clkdev_add(struct clk_lookup *cl)
{
	mutex_lock(&clocks_mutex);
	list_add_tail(&cl->node, &clocks);
	mutex_unlock(&clocks_mutex);
}

void clkdev_add(struct clk_lookup *cl)
{
	if (!cl->clk_hw)
		cl->clk_hw = __clk_get_hw(cl->clk);
	__clkdev_add(cl);
}
EXPORT_SYMBOL(clkdev_add);


void __init clkdev_add_table(struct clk_lookup *cl, size_t num)
{
	mutex_lock(&clocks_mutex);
	while (num--) {
		list_add_tail(&cl->node, &clocks);
		cl++;
	}
	mutex_unlock(&clocks_mutex);
}

#define MAX_DEV_ID	20
#define MAX_CON_ID	16

struct clk_lookup_alloc {
	struct clk_lookup cl;
	char	dev_id[MAX_DEV_ID];
	char	con_id[MAX_CON_ID];
};

static struct clk_lookup * __init_refok
vclkdev_alloc(struct clk_hw *hw, const char *con_id, const char *dev_fmt,
	va_list ap)
{
	struct clk_lookup_alloc *cla;

	cla = __clkdev_alloc(sizeof(*cla));
	if (!cla)
		return NULL;

	cla->cl.clk_hw = hw;
	if (con_id) {
		strlcpy(cla->con_id, con_id, sizeof(cla->con_id));
		cla->cl.con_id = cla->con_id;
	}

	if (dev_fmt) {
		vscnprintf(cla->dev_id, sizeof(cla->dev_id), dev_fmt, ap);
		cla->cl.dev_id = cla->dev_id;
	}

	return &cla->cl;
}

static struct clk_lookup *
vclkdev_create(struct clk_hw *hw, const char *con_id, const char *dev_fmt,
	va_list ap)
{
	struct clk_lookup *cl;

	cl = vclkdev_alloc(hw, con_id, dev_fmt, ap);
	if (cl)
		__clkdev_add(cl);

	return cl;
}


struct clk_lookup *clkdev_alloc(struct clk *clk, const char *con_id,
	const char *dev_fmt, ...)
{
	struct clk_lookup_alloc *cla;

	cla = kzalloc(sizeof(*cla), GFP_KERNEL);
	if (!cla)
		return NULL;

	cla->cl.clk = clk;
	if (con_id) {
		strlcpy(cla->con_id, con_id, sizeof(cla->con_id));
		cla->cl.con_id = cla->con_id;
	}

	if (dev_fmt) {
		va_list ap;

		va_start(ap, dev_fmt);
		vscnprintf(cla->dev_id, sizeof(cla->dev_id), dev_fmt, ap);
		cla->cl.dev_id = cla->dev_id;
		va_end(ap);
	}

	return &cla->cl;
}
EXPORT_SYMBOL(clkdev_alloc);

struct clk_lookup *clkdev_create(struct clk *clk, const char *con_id,
	const char *dev_fmt, ...)
{
	struct clk_lookup *cl;
	va_list ap;

	va_start(ap, dev_fmt);
	cl = vclkdev_create(__clk_get_hw(clk), con_id, dev_fmt, ap);
	va_end(ap);

	return cl;
}
EXPORT_SYMBOL_GPL(clkdev_create);


int clk_add_alias(const char *alias, const char *alias_dev_name,
	const char *con_id, struct device *dev)
{
	struct clk *r = clk_get(dev, con_id);
	struct clk_lookup *l;

	if (IS_ERR(r))
		return PTR_ERR(r);

	l = clkdev_create(r, alias, alias_dev_name ? "%s" : NULL,
			  alias_dev_name);
	clk_put(r);

	return l ? 0 : -ENODEV;
}
EXPORT_SYMBOL(clk_add_alias);

/*
 * clkdev_drop - remove a clock dynamically allocated
 */
void clkdev_drop(struct clk_lookup *cl)
{
	mutex_lock(&clocks_mutex);
	list_del(&cl->node);
	mutex_unlock(&clocks_mutex);
	kfree(cl);
}
EXPORT_SYMBOL(clkdev_drop);
