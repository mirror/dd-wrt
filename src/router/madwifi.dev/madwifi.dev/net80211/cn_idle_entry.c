/* Copyright TMM (Private) Limited 2007
 * All Rights Reserved.
 *
 * THIS WORK CONTAINS TRADE SECRET AND PROPRIETARY INFORMATION WHICH IS THE
 * PROPERTY OF TMM (PRIVATE) LIMITED OR ITS LICENSORS AND IS SUBJECT
 * TO LICENSE TERMS.
 *
 * Email: info@tmm-ltd.com
 * Website: www.tmm-ltd.com
 *
 * $Id$
 */
#ifdef HAVE_POLLING

#ifndef EXPORT_SYMTAB
#define	EXPORT_SYMTAB
#endif

#ifndef AUTOCONF_INCLUDED
#include <linux/config.h>
#endif
#include <linux/version.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/init.h>
#include <linux/vmalloc.h>
#include <linux/proc_fs.h>

#include "if_media.h"

#include <net80211/ieee80211_var.h>
#include <net80211/cn_policy_entry.h>
#include <net80211/cn_idle_entry.h>

MALLOC_DEFINE(M_80211_CN, "cn", "802.11 CN Wifi Protocol");

/*------------------------------------------------------------------*/
/*
 * Initialize and attach CN Idle Queue Module.
 */

static int cn_idle_attach(struct ieee80211vap *vap)
{
	struct cn_idle_state *cnidle;
#ifndef SINGLE_MODULE
	_MOD_INC_USE(THIS_MODULE, return 0);
#endif

	//printk(KERN_INFO "CN Idle Queue: module attached\n");

	MALLOC(cnidle, struct cn_idle_state *, sizeof(struct cn_idle_state), M_DEVBUF, M_NOWAIT | M_ZERO);

	if (cnidle == NULL) {
#ifndef SINGLE_MODULE
		_MOD_DEC_USE(THIS_MODULE);
#endif
		return 0;
	}

	TAILQ_INIT(&cnidle->idle_list);
	cnidle->idle_clients = 0;
	cnidle->cn_idle_multiplier = 0;
	cnidle->cn_req_multiplier = CN_IDLE_REQ_MULTIPLIER;

	vap->iv_idle = cnidle;
	return 1;
}

/*------------------------------------------------------------------*/
/*
 * Detach CN Idle Queue Module.
 */

static void cn_idle_detach(struct ieee80211vap *vap)
{
//      FREE(cnidle);
	printk(KERN_WARNING "CN Idle Queue: Detached. This is only expected to happen during a reboot, so memory has not been deallocated.\n");
#ifndef SINGLE_MODULE
	_MOD_DEC_USE(THIS_MODULE);
#endif
}

/*------------------------------------------------------------------*/
/*
 * Add clients/entries to CN Idle Queue.
 */

static int cn_idle_client_add(struct ieee80211vap *vap, void *clnt)
{
	struct cn_idle_state *cnidle = vap->iv_idle;
	//spinlock_t                            *lock = &vap->iv_cn_lock;
	struct cn_policy_entry *client = (struct cn_policy_entry *)clnt;
	struct cn_idle_entry *new;

	MALLOC(new, struct cn_idle_entry *, sizeof(struct cn_idle_entry), M_80211_CN, M_NOWAIT | M_ZERO);

	if (new == NULL) {

		//printk(KERN_INFO "CN Idle Queue: add %s failed, no memory\n",
//                      client != NULL ? ether_sprintf(client->mac_address) : "idle client");

		/* XXX statistic */
		return -ENOMEM;
	}
	//spin_lock(lock);

	new->client = client;
	TAILQ_INSERT_TAIL(&cnidle->idle_list, new, entries);
	cnidle->idle_clients++;

	//spin_unlock(lock);

//      printk(KERN_INFO "CN Idle Queue: add %s\n",
//                      client != NULL ? ether_sprintf(client->mac_address) : "idle client");

	return 0;
}

/*------------------------------------------------------------------*/
/*
 * Delete CN Idle Queue.
 */

static int cn_idle_client_del(struct ieee80211vap *vap)
{
	struct cn_idle_state *cnidle = vap->iv_idle;
	//spinlock_t                            *lock = &vap->iv_cn_lock;
	struct cn_idle_entry *idle;

	if (cnidle->idle_clients <= 0) {

//              printk(KERN_INFO "CN Idle Queue: %s\n", "list is empty");
		return ENOENT;
	}
	//spin_lock(lock);

	idle = TAILQ_FIRST(&cnidle->idle_list);
	TAILQ_REMOVE(&cnidle->idle_list, idle, entries);
	cnidle->idle_clients--;

	//spin_unlock(lock);

//      printk(KERN_INFO "CN Idle Queue: remove %s\n",
//              idle->client != NULL ? ether_sprintf(idle->client->mac_address) : "idle client");

	FREE(idle, M_80211_CN);

	return 0;
}

/*------------------------------------------------------------------*/
/*
 * Remove clients/entries from CN Idle Queue based on MAC Address.
 */

static int cn_idle_client_remove(struct ieee80211vap *vap, void *clnt)
{
	struct cn_idle_state *cnidle = vap->iv_idle;
	spinlock_t *lock = &vap->iv_cn_lock;
	struct cn_policy_entry *client = (struct cn_policy_entry *)clnt;
	struct cn_idle_entry *idle;

	if (cnidle == NULL) {

//              printk(KERN_INFO "CN Idle Queue: %s\n", "list is empty");
		return ENOENT;
	}

	spin_lock(lock);

	TAILQ_FOREACH(idle, &cnidle->idle_list, entries) {
		if (idle->client == client) {

			TAILQ_REMOVE(&cnidle->idle_list, idle, entries);
			cnidle->idle_clients--;
			break;
		}
	}
	spin_unlock(lock);

//      printk(KERN_INFO "CN Idle Queue: remove %s\n",
//              idle != NULL ? ether_sprintf(idle->client->mac_address) : "not found");

	if (idle != NULL) {

		FREE(idle, M_80211_CN);
	}

	return 0;
}

/*------------------------------------------------------------------*/
/*
 * Remove all clients/entries from CN Idle Queue.
 */

static int cn_idle_client_remove_all(struct ieee80211vap *vap)
{
	struct cn_idle_state *cnidle = vap->iv_idle;
	spinlock_t *lock = &vap->iv_cn_lock;
	struct cn_idle_entry *idle;

	if (cnidle == NULL) {

		IEEE80211_DPRINTF(vap, IEEE80211_MSG_CN, "CN Idle Queue: %s\n", "list is empty");
		return ENOENT;
	}

	spin_lock(lock);

	while (cnidle->idle_clients > 0) {

		idle = TAILQ_FIRST(&cnidle->idle_list);
		TAILQ_REMOVE(&cnidle->idle_list, idle, entries);
		FREE(idle, M_80211_CN);

		cnidle->idle_clients--;
	}

	spin_unlock(lock);

//      printk(KERN_INFO "CN Idle Queue: %s\n", "Remove All");

	return 0;
}

/*------------------------------------------------------------------*/
/*
 * Open CN Idle Queue.
 */

#ifdef CONFIG_SYSCTL

int proc_cn_idle_open(struct inode *inode, struct file *file)
{
	struct proc_ieee80211_priv *pv = NULL;
	struct proc_dir_entry *dp = PDE(inode);
	struct ieee80211vap *vap = dp->data;
	struct cn_idle_state *cnidle = vap->iv_idle;
	spinlock_t *lock = &vap->iv_cn_lock;
	struct cn_idle_entry *idle;
	uint16_t count;

	char *p;

	if (!(file->private_data = kmalloc(sizeof(struct proc_ieee80211_priv), GFP_KERNEL)))
		return -ENOMEM;

	pv = (struct proc_ieee80211_priv *)file->private_data;
	memset(pv, 0, sizeof(struct proc_ieee80211_priv));
	pv->rbuf = vmalloc(MAX_PROC_IEEE80211_SIZE);

	if (!pv->rbuf) {
		kfree(pv);
		return -ENOMEM;
	}

	pv->wbuf = vmalloc(MAX_PROC_IEEE80211_SIZE);

	if (!pv->wbuf) {
		vfree(pv->rbuf);
		kfree(pv);
		return -ENOMEM;
	}

	memset(pv->wbuf, 0, MAX_PROC_IEEE80211_SIZE);
	memset(pv->rbuf, 0, MAX_PROC_IEEE80211_SIZE);
	pv->max_wlen = MAX_PROC_IEEE80211_SIZE;
	pv->max_rlen = MAX_PROC_IEEE80211_SIZE;

	/* now read the data into the buffer */
	p = pv->rbuf;
	p += sprintf(p, "\nIdle Queue (%u elements)\n", cnidle->idle_clients);
	p += sprintf(p, "\nSet Multiplier: %u\n", cnidle->cn_req_multiplier);
	p += sprintf(p, "\nCurrent Multiplier: %u\n", cnidle->cn_idle_multiplier);

	spin_lock(lock);

	count = 0;
	TAILQ_FOREACH(idle, &cnidle->idle_list, entries) {

		p += sprintf(p, "%u: %s\n", count, idle->client != NULL ? ether_sprintf(idle->client->mac_address) : "idle client");
		count++;

	}

	spin_unlock(lock);

	pv->rlen = (p - pv->rbuf);

	return (0);
}

/*------------------------------------------------------------------*/
/*
 * Initialize /proc functions of CN Idle Queue.
 */

static struct file_operations proc_cn_idle = {
	.read = NULL,
	.write = NULL,
	.open = proc_cn_idle_open,
	.release = NULL,
};

/*------------------------------------------------------------------*/
/*
 * Register CN Idle Queue.
 */

void ath_cn_idle_proc_register(struct ieee80211vap *vap)
{
	ieee80211_proc_vcreate(vap, &proc_cn_idle, "cn_idle");
}

EXPORT_SYMBOL(ath_cn_idle_proc_register);

#endif				/* CONFIG_SYSCTL */

/*
 * Module glue.
 */

/*------------------------------------------------------------------*/
/*
 * CN Idle Queue structure.
 */

static const struct cn_idle cn_idle = {
	.cn_attach = cn_idle_attach,
	.cn_detach = cn_idle_detach,
	.cn_add = cn_idle_client_add,
	.cn_delete = cn_idle_client_del,
	.cn_remove = cn_idle_client_remove,
	.cn_remove_all = cn_idle_client_remove_all,
};

/*------------------------------------------------------------------*/
/*
 * Initialize CN Idle Queue.
 */

int __init init_ieee80211_cn_idle(void)
{
	cn_idle_register(&cn_idle);
	return 0;
}

#ifndef SINGLE_MODULE
module_init(init_ieee80211_cn_idle);
#endif

/*------------------------------------------------------------------*/
/*
 * Exit CN Idle Queue.
 */

void __exit exit_ieee80211_cn_idle(void)
{
	cn_idle_unregister(&cn_idle);
}

#ifndef SINGLE_MODULE
module_exit(exit_ieee80211_cn_idle);
#endif

#endif
