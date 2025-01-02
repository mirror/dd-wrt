#define DEBUG
/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2012-2013 Cavium, Inc.
 */

#define pr_fmt(fmt) "octeon-hw-status: %s:%d " fmt, __func__, __LINE__

#include <linux/interrupt.h>
#include <linux/irqdomain.h>
#include <linux/notifier.h>
#include <linux/export.h>
#include <linux/slab.h>
#include <linux/moduleparam.h>
#include <linux/irqdesc.h>

#include <asm/octeon/octeon-hw-status.h>
#include <asm/octeon/octeon.h>

static RAW_NOTIFIER_HEAD(octeon_hw_status_notifiers);

int octeon_hw_status_notifier_register(struct notifier_block *nb)
{
	return raw_notifier_chain_register(&octeon_hw_status_notifiers, nb);
}
EXPORT_SYMBOL(octeon_hw_status_notifier_register);

int octeon_hw_status_notifier_unregister(struct notifier_block *nb)
{
	return raw_notifier_chain_unregister(&octeon_hw_status_notifiers, nb);
}
EXPORT_SYMBOL(octeon_hw_status_notifier_unregister);

#ifdef DEBUG
static int count_debug;
module_param(count_debug, int, 0644);
#else
# define count_debug 0
#endif

/*
 * top-level of hw-status is a list of "roots", each corresponding to
 * an irq-summary word.  Each bit of an irq-summary can correspond to
 * a detailed 64bit summary/enable register pair.
 * Each of these can potentially be registered for hw-status notifier.
 * Count is inc'd during creation allowing concurrent add/remove in leaves
 * while dropping lock for kmalloc().
 * See INT_SUM1 for high-count example, where GMX/PCS/PCSX all add several bits.
 */
#define ref_max_users ((64 * 64) + 64 + 1)

struct octeon_hw_status_node {
	struct octeon_hw_status_node *next; /* Child list */
	struct octeon_hw_status_node *child;
	struct octeon_hw_status_node *parent;
	union {
		struct {
			u64 reg;
			u64 mask_reg;
		};
		struct {
			u32 hwint;
			int irq;
		};
	};
	u16 users;  /* Reference count. */
	u8 bit;
	u8 is_hwint:1;
	u8 ack_w1c:1;
	u8 own_irq:1;
};

/* Protects octeon_hw_status_roots & their trees */
static DEFINE_RWLOCK(octeon_hw_status_lock);
static struct octeon_hw_status_node *octeon_hw_status_roots;

/*
 * when constructing tree, _lock may be dropped to kmalloc, but new parent
 * does .users++ to reflect _intended_ child, to avoid a sibling'd dealloc
 * causing the parent.users-- to free the parent.
 * This state is visible to deflect a WARN() by passing scans while unlocked.
 */
static bool constructing;

/* call cb on each node.  stop if cb returns true. */
static int visit_leaves(struct octeon_hw_status_node *s, bool visit_sibs,
			  int (*cb)(struct octeon_hw_status_node *, void *),
			  void *arg)
{
	int depth = 0;
	struct octeon_hw_status_node *w = s;
	bool visit_roots = visit_sibs; /* should they ever differ? bitmask? */
	int r;

	while (w && depth >= 0) {
		if (visit_roots && !depth) {
			r = cb(w, arg);
			if (r)
				return r;
		}

		if (w->child) {
			/* Go out to the leaves */
			w = w->child;
			depth++;
			continue;
		}

		/* At a leaf, make the callback. */
		r = cb(w, arg);
		if (r)
			return r;

		if (w->next) {
			/* Go to next leaf */
			w = w->next;
			continue;
		}
		/* back toward the root*/
		for (;;) {
			depth--;
			if (!w->parent)
				return 0;
			if (!visit_sibs && w->parent == s)
				return 0;
			if (w->parent->next) {
				w = w->parent->next;
				break;
			}
			w = w->parent;
		}
	}
	return 0;
}

struct find_node_cb_data {
	struct octeon_hw_status_node *r;
	struct octeon_hw_status_reg *sr;
	bool warn:1;
	bool count:1;
	u32 found;
};

static void warn_mismatch(struct find_node_cb_data *cbd)
{
	WARN(cbd->warn,
		pr_fmt("Mismatched properties"
			" %llx:%llx %p/%d %d/%d\n"),
		 cbd->r->reg, cbd->sr->mask_reg,
		 cbd->r->child, cbd->sr->has_child,
		 cbd->r->ack_w1c, cbd->sr->ack_w1c);
}

static int find_node_cb(struct octeon_hw_status_node *n, void *arg)
{
	struct find_node_cb_data *d = arg;
	struct octeon_hw_status_reg *sr = d->sr;

	if (n->is_hwint == sr->reg_is_hwint) {
		if (n->is_hwint) {
			if (n->hwint == sr->reg)
				goto found;
		} else {
			if (n->reg == sr->reg && n->bit == sr->bit)
				goto found;
		}
	}
	return 0;

found:
	d->r = n;
	d->warn = (!constructing && !!n->child != sr->has_child);
	d->warn |= (n->ack_w1c != sr->ack_w1c);

	if (d->count) {
		/* continue, summing hits */
		d->found++;
		return 0;
	}

	return 1;
}

static struct octeon_hw_status_node *find_node(struct octeon_hw_status_node *r,
					       struct octeon_hw_status_reg *sr,
					       struct find_node_cb_data *np)
{
	struct find_node_cb_data d = { .warn  = false, };

	if (!np)
		np = &d;

	np->r = NULL;
	np->sr = sr;

	visit_leaves(r, true, find_node_cb, np);

	return np->r;
}

struct irq_cb_data {
	bool handled_one;
};

static int irq_cb(struct octeon_hw_status_node *n, void *arg)
{
	u64 csr, en, bit_mask;
	struct octeon_hw_status_data ohsd;
	struct irq_cb_data *d = arg;

	memset(&ohsd, 0, sizeof(ohsd));

	if (n->is_hwint) {
		ohsd.reg = n->hwint;
		ohsd.reg_is_hwint = 1;
	} else {
		bit_mask = 1ull << n->bit;
		csr = cvmx_read_csr(n->reg);
		if (!(csr & bit_mask))
			return 0;

		if (n->mask_reg) {
			en = cvmx_read_csr(n->mask_reg);
			if (!(en & bit_mask))
				return 0;
		}
		/* Found an enabled source. */
		if (n->ack_w1c)
			cvmx_write_csr(n->reg, bit_mask);

		ohsd.reg = n->reg;
		ohsd.bit = n->bit;
	}
	raw_notifier_call_chain(&octeon_hw_status_notifiers,
				OCTEON_HW_STATUS_SOURCE_ASSERTED, &ohsd);
	d->handled_one = true;
	return 0;
}

static irqreturn_t octeon_hw_status_irq(int irq, void *dev)
{
	struct irq_cb_data d;
	struct octeon_hw_status_node *root = dev;

	d.handled_one = false;
	read_lock(&octeon_hw_status_lock);
	visit_leaves(root, false, irq_cb, &d);
	read_unlock(&octeon_hw_status_lock);
	return d.handled_one ? IRQ_HANDLED : IRQ_NONE;
}

/* find the root node containing a given child */
static struct octeon_hw_status_node *find_child(struct octeon_hw_status_node *r,
						struct octeon_hw_status_reg *sr,
						struct find_node_cb_data  *np)
{
	struct octeon_hw_status_node *cw = r->child;
	struct find_node_cb_data cbd;

	if (!np)
		np = &cbd;
	np->r = NULL;
	np->sr = sr;
	np->warn = false;
	while (cw) {
		if (find_node_cb(cw, np))
			break;
		cw = cw->next;
	}
	return cw;
}

static bool irq_handled(int irq)
{
	struct irq_desc *desc = irq_to_desc(irq);

	if (!desc || !desc->action)
		return false;
	return (desc->action->handler || desc->action->thread_fn);
}

int octeon_hw_status_add_source(struct octeon_hw_status_reg *chain0)
{
	struct octeon_hw_status_reg *chain = chain0;
	struct octeon_hw_status_data ohsd;
	struct octeon_hw_status_node *root = NULL;
	struct octeon_hw_status_node *w;
	struct octeon_hw_status_node *new_root = NULL;
	struct find_node_cb_data match = {.warn = false, };
	bool oflow = false;
	bool root_created = false;
	bool created = false;
	int rv = 0;

	if (!chain->reg_is_hwint)
		return -EINVAL;

	write_lock(&octeon_hw_status_lock);

	/*
	 * Find-or-create matching root object,
	 * repeating scan if must drop lock to alloc,
	 * as other players could insert root while unlocked.
	 */
	do {
		for (root = octeon_hw_status_roots; root; root = root->next) {
			BUG_ON(!root->is_hwint);
			if (root->hwint == chain->reg)
				break;
		}

		if (!(root || new_root)) {
			write_unlock(&octeon_hw_status_lock);
			new_root =
			    kzalloc(sizeof(struct octeon_hw_status_node),
				    GFP_KERNEL);
			WARN(!new_root,
			     pr_fmt("ENOMEM inserting root %llx/%llx:%u\n"),
			     chain->reg, chain->mask_reg, chain->bit);
			if (!new_root)
				return -ENOMEM;
			write_lock(&octeon_hw_status_lock);
			/* rescan, a new root may have appeared when unlocked */
			continue;
		}

		if (!root) {
			root = new_root;
			new_root = NULL;

			root->is_hwint = chain->reg_is_hwint;
			root->ack_w1c = chain->ack_w1c;

			if (root->is_hwint) {
				root->hwint = chain->reg;
				if (irq_handled(chain->reg))
					root->irq = chain->reg;
			} else {
				root->reg = chain->reg;
				root->mask_reg = chain->mask_reg;
			}

			/* hold root until child attached */
			root->users = 1;

			root->next = octeon_hw_status_roots;
			octeon_hw_status_roots = root;
			root_created = true;
		}
	} while (!root);

	/* now lay out the children under that root ... */
	w = root;
	while (chain->has_child) {
		struct octeon_hw_status_node *n;
		struct octeon_hw_status_node *n2 = NULL;

		chain++;
		created = false;

		do {
			/* existing root containing this chain? */
			n = find_child(w, chain, &match);

			/* somebody added it while sleeping on alloc? */
			if (n && n2) {
				kfree(n2);
				n2 = NULL;
				break;
			}

			if (!n && !n2) {
				/*
				 * drop lock to alloc, could potentially collide like
				 * the root case, but simpler.
				 * Bump the use-count while unlocked to avoid teardown.
				 */
				constructing = true;
				w->users++;
				write_unlock(&octeon_hw_status_lock);
				n2 = kzalloc(sizeof
					     (struct octeon_hw_status_node),
					     GFP_KERNEL);
				WARN(!n2, pr_fmt("ENOMEM inserting child"
						 " %llx/%llx:%u\n"),
					 chain->reg, chain->mask_reg,
					 chain->bit);
				write_lock(&octeon_hw_status_lock);
				w->users--;
				constructing = false;

				if (!n2) {
					rv = -ENOMEM;
					goto unlock;
				}
			} else if (!n) {
				/* create node: n2 alloc'd, no match in tree */
				n = n2;
				match.r = n;
				n->is_hwint = chain->reg_is_hwint;
				n->bit = chain->bit;
				n->ack_w1c = chain->ack_w1c;
				n->users = 1;

				if (n->is_hwint) {
					n->hwint = chain->reg;
					if (irq_handled(chain->reg))
						n->irq = chain->reg;
				} else {
					n->reg = chain->reg;
					n->mask_reg = chain->mask_reg;
				}

				/* attach to parent */
				n->next = w->child;
				w->child = n;
				n->parent = w;

				/* up parent ref-count, unless created u=1 */
				oflow = (w->users == ref_max_users);
				if (!oflow && !root_created)
					w->users++;
				created = true;
			}
		} while (!n);
		w = n;
	}

	write_unlock(&octeon_hw_status_lock);

	if (count_debug && w) {
		/* log last match only, not parents */
		pr_debug("%llx/%llx:%d i%d refcount"
			" parent %s%d @%p, child %d @%p\n",
		       w->reg, w->mask_reg, w->bit, w->is_hwint,
		       (created ? "++" : ""), w->parent->users,
		       w->parent, (w ? w->users : 0), w);
	}

	WARN(oflow, pr_fmt("Reference count overflowed!\n"));
	warn_mismatch(&match);

	if (root_created) {
		if (root->irq) {
			WARN(rv, pr_fmt("handler for irq %d already set\n"),
				root->irq);
		} else {
			/* register an interrupt handler */
			root->irq = irq_create_mapping(NULL, root->hwint);

			if (!root->irq) {
				WARN(rv, pr_fmt("cannot map handler"
					" for hwint %d\n"),
					(int) root->hwint);
				rv = -ENXIO;
				goto bye;
			}

			rv = request_threaded_irq(root->irq, NULL,
					octeon_hw_status_irq, IRQF_ONESHOT,
					"octeon-hw-status", root);
			if (!rv)
				root->own_irq = 1;
			else {
				write_lock(&octeon_hw_status_lock);
				octeon_hw_status_roots = root->next;
				write_unlock(&octeon_hw_status_lock);
				w = root->child;
				while(w) {
					new_root = w->next;
					kfree(w);
					w = new_root;
				}
				kfree(root);
				return rv;
			}
		}

		if (count_debug)
			pr_debug("%d/%d created root\n",
			       (int)root->hwint, root->irq);
	}

	/* notifies on leaf creation, not intermediate nodes */
	ohsd.reg = w->reg;
	ohsd.bit = w->bit;
	raw_notifier_call_chain(&octeon_hw_status_notifiers,
				OCTEON_HW_STATUS_SOURCE_ADDED, &ohsd);
	rv = 0;
bye:
	if (new_root)
		kfree(new_root);
	if (rv && count_debug)
		pr_debug("%llx/%llx:%d err %d\n",
		       chain->reg, chain->mask_reg, chain->bit, rv);

	/* final sanity-check to ensure all are findable */
	for (; count_debug && chain0; chain0++) {
		struct octeon_hw_status_reg targ = *chain0;
		struct find_node_cb_data counter = {.count = true, };

		if (count_debug > 1)
			pr_debug("check %llx/%llx:%d i%d\n",
			       targ.reg, targ.mask_reg,
			       targ.bit, targ.reg_is_hwint);

		find_node(octeon_hw_status_roots, &targ, &counter);

		if (counter.found != 1)
			pr_debug("%llx/%llx:%d inserted %d times\n",
			       targ.reg, targ.mask_reg, targ.bit,
			       counter.found);

		if (!chain0->has_child)
			break;
	}

	return rv;
unlock:
	write_unlock(&octeon_hw_status_lock);
	goto bye;
}
EXPORT_SYMBOL(octeon_hw_status_add_source);

/*
 * unuse_node():
 * Decrement use count, freeing when zero,
 * recursing up parent chain as children freed.
 * Count generations freed into *levp.
 *
 * Called with lock held.
 * Returns 'locked', false if had to unlock to free irq-node.
 * Work is complete at return, even in the unlocked-return case,
 * because an irq-node never has a parent.
 */
static bool unuse_node(struct octeon_hw_status_node *n, bool *oflow, int *levp)
{
	bool locked = true;
	struct octeon_hw_status_node *parent;
	struct octeon_hw_status_node **pw;

	for (; locked && n; n = parent) {
		/* "Can't Happen": count over/underflowed, so unsafe */
		*oflow |= (n->users == ref_max_users);
		*oflow |= !n->users;

		if (*oflow)
			return locked;

		if (--(n->users) > 0)
			return locked;

		/* n will be freed, but from which chain? */
		parent = n->parent;
		if (parent)
			pw = &n->parent->child;
		else
			pw = &octeon_hw_status_roots;

		/* walk parent's chain, to find unlink point */
		for (; *pw; pw = &(*pw)->next)
			if (*pw == n)
				break;

		/* if sum/mask, remove from mask */
		if (!n->is_hwint && n->mask_reg) {
			u64 mask = 1ull << n->bit;
			u64 csr = cvmx_read_csr(n->mask_reg);
			csr &= ~mask;
			cvmx_write_csr(n->mask_reg, csr);
		}

		/* edit self out of chain */
		if (pw)
			*pw = n->next;

		if (n->is_hwint && n->own_irq) {
			write_unlock(&octeon_hw_status_lock);
			locked = false;
			free_irq(n->irq, n);
		}

		if (!pw) {
			/* "Can't Happen", so log */
			if (locked)
				write_unlock(&octeon_hw_status_lock);
			locked = false;
			pr_err("%s(%p .i%d .r%llx) but no parent!\n",
				__func__, n, n->is_hwint, n->reg);
		}

		kfree(n);
		levp[0]++;
	}

	return locked;
}

int octeon_hw_status_remove_source(struct octeon_hw_status_reg *leaf)
{
	int rv = 0;
	bool locked = true;
	struct octeon_hw_status_node *n;
	struct find_node_cb_data d = { .warn  = false, };
	bool oflow = false;
	int levels = 0;

	write_lock(&octeon_hw_status_lock);

	n = find_node(octeon_hw_status_roots, leaf, &d);
	if (!n) {
		rv = -ENODEV;
		goto out;
	}

	locked = unuse_node(n, &oflow, &levels);
out:
	if (locked)
		write_unlock(&octeon_hw_status_lock);

	warn_mismatch(&d);

	if (count_debug) {
		if (!n)
			pr_err("%s(i%d %llx:%d) ENOENT\n",
				__func__, leaf->reg_is_hwint,
				leaf->reg, leaf->bit);
		else if (levels)
			/* n freed, but can safely print _address_ for cross-ref */
			pr_debug("%llx:%d refcount --0 @%p, %d levels freed\n",
				leaf->reg, leaf->bit, n, levels);
		else
			pr_debug("%llx:%d refcount parent --%d @%p, child %d @%p\n",
				n->reg, n->bit,
				(n->parent ? n->parent->users : 999),
				n->parent, n->users, n);
	}

	return rv;
}
EXPORT_SYMBOL(octeon_hw_status_remove_source);


struct enable_cb_data {
	u64 reg;
	u64 requested_mask;
	u64 mask_reg;
	u64 valid_mask;
	u64 warn_mask;
};

static int enable_cb(struct octeon_hw_status_node *n, void *arg)
{
	struct enable_cb_data *d = arg;

	if (n->reg == d->reg && (d->requested_mask & (1ul << n->bit))) {
		d->valid_mask |= (1ul << n->bit);
		if (n->mask_reg) {
			if (d->mask_reg)
				d->warn_mask |= (d->mask_reg ^ n->mask_reg);
			d->mask_reg = n->mask_reg;
		}
	}
	return 0;
}

int octeon_hw_status_enable(u64 reg, u64 bit_mask)
{
	struct enable_cb_data cbd;

	memset(&cbd, 0, sizeof(cbd));
	cbd.reg = reg;
	cbd.requested_mask = bit_mask;

	read_lock(&octeon_hw_status_lock);

	visit_leaves(octeon_hw_status_roots, true, enable_cb, &cbd);

	if (cbd.valid_mask && cbd.mask_reg) {
		u64 csr = cvmx_read_csr(cbd.mask_reg);
		csr |= cbd.valid_mask;
		cvmx_write_csr(cbd.mask_reg, csr);
	}

	read_unlock(&octeon_hw_status_lock);

	WARN(cbd.warn_mask, pr_fmt("mask reg mismatch %llx %llx\n"),
		cbd.reg, cbd.warn_mask);

	return 0;
}
EXPORT_SYMBOL(octeon_hw_status_enable);

int octeon_hw_status_disable(u64 reg, u64 bit_mask)
{
	struct enable_cb_data cbd;

	memset(&cbd, 0, sizeof(cbd));
	cbd.reg = reg;
	cbd.requested_mask = bit_mask;

	read_lock(&octeon_hw_status_lock);

	visit_leaves(octeon_hw_status_roots, true, enable_cb, &cbd);

	if (cbd.valid_mask && cbd.mask_reg) {
		u64 csr = cvmx_read_csr(cbd.mask_reg);
		csr &= ~cbd.valid_mask;
		cvmx_write_csr(cbd.mask_reg, csr);
	}

	read_unlock(&octeon_hw_status_lock);

	WARN(cbd.warn_mask, pr_fmt("mask reg mismatch %llx %llx\n"),
		cbd.reg, cbd.warn_mask);

	return 0;
}
EXPORT_SYMBOL(octeon_hw_status_disable);

#ifdef CONFIG_DEBUG_FS

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/init.h>
#include <linux/namei.h>
#include <linux/debugfs.h>
#include <linux/semaphore.h>

static DEFINE_SEMAPHORE(hwstat_sem, 1); /* single open */

static int hwstat_show_node(struct octeon_hw_status_node *n, void *data)
{
	struct seq_file *s = (struct seq_file *)data;

	if (count_debug)
		seq_printf(s, "%p@%p: u%d i%d ",
			n, n->parent, n->users, n->is_hwint);

	if (n->is_hwint)
		seq_printf(s, "%d/%d\n",
			(int)n->hwint, n->irq);
	else
		seq_printf(s, "%llx/%llx:%d a%d\n",
			n->reg, n->mask_reg, n->bit, n->ack_w1c);
	return 0;
}

static int hwstat_show(struct seq_file *s, void *data)
{
	/* print heading */
	read_lock(&octeon_hw_status_lock);
	visit_leaves(octeon_hw_status_roots, true, hwstat_show_node, s);
	read_unlock(&octeon_hw_status_lock);

	return 0;
}

static int hwstat_open(struct inode *inode, struct file *file)
{
	down(&hwstat_sem);
	return single_open(file, hwstat_show, NULL);
}

static int hwstat_release(struct inode *inode, struct file *file)
{
	up(&hwstat_sem);
	return single_release(inode, file);
}

static const struct file_operations hwstat_operations = {
	.open		= hwstat_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= hwstat_release,
};

static int __init hwstat_debugfs_init(void)
{
	/* /sys/kernel/debug/hwstat */
	(void) debugfs_create_file("hwstat", S_IFREG | S_IRUGO, NULL, NULL, &hwstat_operations);
	return 0;
}
postcore_initcall(hwstat_debugfs_init);

#endif /* CONFIG_DEBUG_FS */
