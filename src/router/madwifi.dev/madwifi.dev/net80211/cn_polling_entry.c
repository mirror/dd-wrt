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
#include <net80211/cn_polling_entry.h>
#include <net80211/cn_idle_entry.h>

MALLOC_DEFINE(M_80211_CN, "cn", "802.11 CN Wifi Protocol");

static int cn_poll_client_add(struct ieee80211vap *vap, void *client);

/*------------------------------------------------------------------*/
/*
 * Initialize and attach CN Polling Queue.
 */

static int cn_polling_attach(struct ieee80211vap *vap)
{
	struct cn_polling_state *cnpoll;

#ifndef SINGLE_MODULE
	_MOD_INC_USE(THIS_MODULE, return 0);
#endif
//      printk(KERN_INFO "CN Polling Queue: module attached\n");

	MALLOC(cnpoll, struct cn_polling_state *, sizeof(struct cn_polling_state), M_DEVBUF, M_NOWAIT | M_ZERO);

	if (cnpoll == NULL) {
#ifndef SINGLE_MODULE
		_MOD_DEC_USE(THIS_MODULE);
#endif
		return 0;
	}

	TAILQ_INIT(&cnpoll->polling_list);
	cnpoll->poll_clients = 0;
	cnpoll->pkt_rcvd = 0;
	cnpoll->polling_started = 0;
	vap->iv_poll = cnpoll;

	return 1;
}

/*------------------------------------------------------------------*/
/*
 * Detach CN Polling Queue.
 */

static void cn_polling_detach(struct ieee80211vap *vap)
{
	printk(KERN_WARNING "CN Polling Queue: Detached. This is only expected to happen during a reboot, so memory has not been deallocated.\n");
#ifndef SINGLE_MODULE
	_MOD_DEC_USE(THIS_MODULE);
#endif
}

/*------------------------------------------------------------------*/
/*
 * Add entries to CN Polling Queue.
 */

static int cn_poll_client_add(struct ieee80211vap *vap, void *clnt)
{
	struct cn_polling_state *cnpoll = vap->iv_poll;
	struct cn_idle_state *cnidle = vap->iv_idle;
	//spinlock_t                            *lock = &vap->iv_cn_lock;
	struct cn_policy_entry *client = (struct cn_policy_entry *)clnt;
	struct cn_polling_entry *new;

	MALLOC(new, struct cn_polling_entry *, sizeof(struct cn_polling_entry), M_80211_CN, M_NOWAIT | M_ZERO);

	if (new == NULL) {

//              printk(KERN_INFO "CN Polling Queue: add %s failed, no memory\n", 
//                      client != NULL ? ether_sprintf(client->mac_address) : "idle client");

		/* XXX statistic */
		return -ENOMEM;
	}
	//spin_lock(lock);

	new->client = client;
	TAILQ_INSERT_TAIL(&cnpoll->polling_list, new, entries);
	cnpoll->poll_clients++;

	/* If this is a normal client, then increment its multiplier. If it is
	 * and idle client, then increment the idle client multiplier.
	 */
	if (client != NULL)
		client->current_multiplier++;
	else
		cnidle->cn_idle_multiplier++;

	//spin_unlock(lock);

//      printk(KERN_INFO "CN Polling Queue: add %s\n", 
//                      client != NULL ? ether_sprintf(client->mac_address) : "idle client");

	return 0;
}

/*------------------------------------------------------------------*/
/*
 * Delete CN Polling Queue.
 */

static int cn_poll_client_del(struct ieee80211vap *vap)
{
	struct cn_polling_state *cnpoll = vap->iv_poll;
	struct cn_idle_state *cnidle = vap->iv_idle;
	//spinlock_t                            *lock = &vap->iv_cn_lock;
	struct cn_polling_entry *poll;

	if (cnpoll->poll_clients <= 0) {

//              printk(KERN_INFO "CN Polling Queue: %s\n", "list is empty");
		return ENOENT;
	}
	//spin_lock(lock);

	poll = TAILQ_FIRST(&cnpoll->polling_list);
	TAILQ_REMOVE(&cnpoll->polling_list, poll, entries);
	cnpoll->poll_clients--;

	/* If this is a normal client, then decrement its multiplier. If it is
	 * and idle client, then decrement the idle client multiplier.
	 */
	if (poll->client != NULL)
		poll->client->current_multiplier--;
	else
		cnidle->cn_idle_multiplier--;

	//spin_unlock(lock);

//      printk(KERN_INFO "CN Polling Queue: remove %s\n", 
//              poll->client != NULL ? ether_sprintf(poll->client->mac_address) : "idle client");

	FREE(poll, M_80211_CN);

	return 0;
}

/*------------------------------------------------------------------*/
/*
 * Remove an entry from CN Polling Queue based on MAC Address.
 */

static int cn_poll_client_remove(struct ieee80211vap *vap, void *clnt)
{
	struct cn_polling_state *cnpoll = vap->iv_poll;
	struct cn_idle_state *cnidle = vap->iv_idle;
	spinlock_t *lock = &vap->iv_cn_lock;
	struct cn_policy_entry *client = (struct cn_policy_entry *)clnt;
	struct cn_polling_entry *polling;

	if (cnpoll == NULL) {

//              printk(KERN_INFO "CN Polling Queue: %s\n", "list is empty");
		return ENOENT;
	}

	spin_lock(lock);

	TAILQ_FOREACH(polling, &cnpoll->polling_list, entries) {
		if (polling->client == client) {

			TAILQ_REMOVE(&cnpoll->polling_list, polling, entries);
			cnpoll->poll_clients--;
			 if (client != NULL)
				polling->client->current_multiplier--;
			
			else
				cnidle->cn_idle_multiplier--;
			break;
		}
	}
	spin_unlock(lock);

//      printk(KERN_INFO "CN Polling Queue: remove %s\n",
//              polling != NULL ? ether_sprintf(polling->client->mac_address) : "not found");

	if (polling != NULL) {

		FREE(polling, M_80211_CN);
	}

	return 0;
}

/*------------------------------------------------------------------*/
/*
 * Remove all entries from CN Polling Queue.
 */

static int cn_poll_client_remove_all(struct ieee80211vap *vap)
{
	struct cn_polling_state *cnpoll = vap->iv_poll;
	spinlock_t *lock = &vap->iv_cn_lock;
	struct cn_polling_entry *polling;

	if (cnpoll == NULL) {

//              printk(KERN_INFO "CN Polling Queue: %s\n", "list is empty");
		return ENOENT;
	}

	spin_lock(lock);

	while (cnpoll->poll_clients > 0) {

		polling = TAILQ_FIRST(&cnpoll->polling_list);
		TAILQ_REMOVE(&cnpoll->polling_list, polling, entries);
		FREE(polling, M_80211_CN);

		cnpoll->poll_clients--;
	}

	spin_unlock(lock);

//      printk(KERN_INFO "CN Polling Queue: %s\n", "Remove All");

	return 0;
}

/*------------------------------------------------------------------*/
/*
 * Open and read CN Polling Queue.
 */

#ifdef CONFIG_SYSCTL

int proc_cn_polling_open(struct inode *inode, struct file *file)
{
	struct proc_ieee80211_priv *pv = NULL;
	struct proc_dir_entry *dp = PDE(inode);
	struct ieee80211vap *vap = dp->data;
	struct cn_polling_state *cnpoll = vap->iv_poll;
	spinlock_t *lock = &vap->iv_cn_lock;
	struct cn_polling_entry *poll;
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
	p += sprintf(p, "\nPolling Queue (%u elements)\n", cnpoll->poll_clients);

	spin_lock(lock);

	count = 0;
	TAILQ_FOREACH(poll, &cnpoll->polling_list, entries) {

		p += sprintf(p, "%u: %s\n", count, poll->client != NULL ? ether_sprintf(poll->client->mac_address) : "idle client");
		count++;
	}

	spin_unlock(lock);

	pv->rlen = (p - pv->rbuf);

	return (0);
}

/*------------------------------------------------------------------*/
/*
 * File operations sructure for CN Polling Queue.
 */

static struct file_operations proc_cn_polling = {
	.read = NULL,
	.write = NULL,
	.open = proc_cn_polling_open,
	.release = NULL,
};

/*------------------------------------------------------------------*/
/*
 * Register CN Polling Queue.
 */

void ath_cn_polling_proc_register(struct ieee80211vap *vap)
{
	ieee80211_proc_vcreate(vap, &proc_cn_polling, "cn_polling");
}

EXPORT_SYMBOL(ath_cn_polling_proc_register);

#endif				/* CONFIG_SYSCTL */

/*
 * Module glue.
 */

/*------------------------------------------------------------------*/
/*
 * CN Polling Queue structure.
 */

static const struct cn_polling cn_poll = {
	.cn_attach = cn_polling_attach,
	.cn_detach = cn_polling_detach,
	.cn_add = cn_poll_client_add,
	.cn_delete = cn_poll_client_del,
	.cn_remove = cn_poll_client_remove,
	.cn_remove_all = cn_poll_client_remove_all,
};

/*------------------------------------------------------------------*/
/*
 * Initialize CN Polling Queue.
 */

int __init init_ieee80211_cn_polling(void)
{
	cn_polling_register(&cn_poll);
	return 0;
}

#ifndef SINGLE_MODULE
module_init(init_ieee80211_cn_polling);
#endif

/*------------------------------------------------------------------*/
/*
 * Exit CN Polling Queue.
 */

void __exit exit_ieee80211_cn_polling(void)
{
	cn_polling_unregister(&cn_poll);
}

#ifndef SINGLE_MODULE
module_exit(exit_ieee80211_cn_polling);
#endif

#endif
