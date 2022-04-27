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
#include <linux/string.h>
#include <asm/uaccess.h>	/* for copy_from_user */

#include "if_media.h"

#include <net80211/ieee80211_var.h>
#include <net80211/cn_policy_entry.h>
#include <net80211/cn_polling_entry.h>
#include <net80211/cn_idle_entry.h>
#include <net80211/if_ethersubr.h>

MALLOC_DEFINE(M_80211_CN, "cn", "802.11 CN Wifi Protocol");

static int cn_select_client_to_poll(struct ieee80211vap *vap, u_int8_t mac[IEEE80211_ADDR_LEN], uint32_t * ctstimeout, u_int8_t index);

static int cn_next_action(struct ieee80211vap *vap, u_int8_t *tx_mac, u_int8_t *cts_mac, uint8_t have_data);

/*------------------------------------------------------------------*/
/*
 * Input an arbitrary length MAC address and convert to binary.
 * Return address size.
 */

static int cn_mac_aton(const char *orig, unsigned char *mac, int macmax)
{
	int count;
	int j[6];

	count = sscanf(orig, "%2X:%2X:%2X:%2X:%2X:%2X", &j[0], &j[1], &j[2], &j[3], &j[4], &j[5]);

	if (count != macmax) {

//      printk(KERN_INFO "cn_mac_aton(%s): invalid ether address! count=%u\n", 
//                      orig, count);                           
		return (0);	/* Error -> non-hex chars */
	}

	for (count = 0; count < macmax; count++) {
		mac[count] = (unsigned char)(j[count] & 0xFF);
	}

	return (macmax);
}

/*------------------------------------------------------------------*/
/*
 * Input an Ethernet address and convert to binary.
 */

static int cn_ether_aton(const char *orig, struct ether_addr *eth)
{
	int maclen;
	maclen = cn_mac_aton(orig, (unsigned char *)eth, ETH_ALEN);

	if ((maclen > 0) && (maclen < ETH_ALEN)) {
		maclen = 0;
	}

	return (maclen);
}

/*------------------------------------------------------------------*/
/*
 * Initialize and attach Cn Policy Queue.
 */

static int cn_policy_attach(struct ieee80211vap *vap)
{
	struct cn_policy_state *cnpolicy;
#ifndef SINGLE_MODULE
	_MOD_INC_USE(THIS_MODULE, return 0);
#endif
//      printk(KERN_INFO "CN Policy Queue: module attached\n");

	MALLOC(cnpolicy, struct cn_policy_state *, sizeof(struct cn_policy_state), M_DEVBUF, M_NOWAIT | M_ZERO);

	if (cnpolicy == NULL) {
#ifndef SINGLE_MODULE
		_MOD_DEC_USE(THIS_MODULE);
#endif
		return 0;
	}

	TAILQ_INIT(&cnpolicy->policy_list);
	cnpolicy->clients = 0;
	vap->iv_policy = cnpolicy;

	return 1;
}

/*------------------------------------------------------------------*/
/*
 * Detach Cn Policy Queue.
 */

static void cn_policy_detach(struct ieee80211vap *vap)
{
	printk(KERN_WARNING "CN Policy Queue: Detached. This is only expected to happen during a reboot, so memory has not been deallocated.\n");
#ifndef SINGLE_MODULE
	_MOD_DEC_USE(THIS_MODULE);
#endif
}

/*------------------------------------------------------------------*/
/*
 * Add entries to Cn Policy Queue.
 */

static int cn_policy_add(struct ieee80211vap *vap, const u_int8_t mac[IEEE80211_ADDR_LEN], uint16_t mytimeout, uint8_t idle, uint8_t mul)
{
	struct cn_policy_state *cnpolicy = vap->iv_policy;
	const struct cn_idle *cnidle = vap->iv_idle_glue;
	const struct cn_polling *cnpoll_glu = vap->iv_poll_glue;
	spinlock_t *lock = &vap->iv_cn_lock;
	u_int8_t mac_to_send[IEEE80211_ADDR_LEN];
	uint32_t ctstimeout;
	struct cn_policy_entry *new;

	MALLOC(new, struct cn_policy_entry *, sizeof(struct cn_policy_entry), M_80211_CN, M_NOWAIT | M_ZERO);

	if (new == NULL) {

//              printk(KERN_INFO "CN Policy Queue: add %s failed, no memory\n",
//                      ether_sprintf( mac ));

		/* XXX statistic */
		return -ENOMEM;
	}

	spin_lock(lock);

	memcpy(new->mac_address, mac, IEEE80211_ADDR_LEN);
	new->timeout = mytimeout;
	new->idle_multiplier = idle;
	new->multiplier = mul;
	new->is_idle = 1;
	new->idle_polls = 0;
	new->current_multiplier = 0;
	TAILQ_INSERT_TAIL(&cnpolicy->policy_list, new, entries);
	cnpolicy->clients++;

	/* Add to idle client list. */
	cnidle->cn_add(vap, new);

	if (cnpolicy->clients == 1) {

		cnpoll_glu->cn_add(vap, NULL);
		spin_unlock(lock);

#define CN_SEND_CTS_ON_TIMER 1
#if CN_SEND_CTS_ON_TIMER
		cn_select_client_to_poll(vap, mac_to_send, &ctstimeout, 0);

		cn_next_action(vap, NULL, mac_to_send, 0);

		ieee80211_send_cts(vap, mac_to_send, ctstimeout, NULL);

#ifdef CN_DEBUG
		printk(KERN_INFO "%s: CTS sent %s\n", __func__, ether_sprintf(mac_to_send));
#endif
#endif

	} else {
		spin_unlock(lock);
	}

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_CN, "CN Policy Queue: add %s\n", ether_sprintf(mac));

	return 0;
}

/*------------------------------------------------------------------*/
/*
 * Remove entries from Cn Policy Queue and their correponding entries from CN Idle & CN Polling Queues.
 */

static int cn_policy_del(struct ieee80211vap *vap, u_int8_t mac[IEEE80211_ADDR_LEN])
{
	int cmp;
	struct cn_policy_state *cnpolicy = vap->iv_policy;
	const struct cn_idle *cnidle = vap->iv_idle_glue;
	const struct cn_polling *cnpoll = vap->iv_poll_glue;
	spinlock_t *lock = &vap->iv_cn_lock;
	struct cn_policy_entry *policy;

	if (cnpolicy == NULL) {

//              printk(KERN_INFO "CN Policy Queue: %s\n", "list is empty");
		return ENOENT;
	}

	spin_lock(lock);

	TAILQ_FOREACH(policy, &cnpolicy->policy_list, entries) {
		cmp = memcmp(policy->mac_address, mac, IEEE80211_ADDR_LEN);

		if (cmp == 0) {

			TAILQ_REMOVE(&cnpolicy->policy_list, policy, entries);
			break;
		}
	}

	cnpolicy->clients--;

	spin_unlock(lock);

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_CN, "CN Policy Queue: remove %s\n", policy != NULL ? ether_sprintf(policy->mac_address) : "not found");

	if (policy != NULL) {

		/* Remove from Idle and polling lists. */
		cnpoll->cn_remove(vap, policy);
		cnidle->cn_remove(vap, policy);

		FREE(policy, M_80211_CN);
	}

	return 0;
}

/*------------------------------------------------------------------*/
/*
 * Delete all entries from Cn Policy Queue, CN Idle Queue and CN Polling Queue.
 */

static int cn_policy_del_all(struct ieee80211vap *vap)
{
	struct cn_policy_state *cnpolicy = vap->iv_policy;
	const struct cn_idle *cnidle = vap->iv_idle_glue;
	const struct cn_polling *cnpoll = vap->iv_poll_glue;
	spinlock_t *lock = &vap->iv_cn_lock;
	struct cn_policy_entry *policy;

	if (cnpolicy == NULL) {

//              printk(KERN_INFO "CN Policy Queue: %s\n", "list is empty");
		return ENOENT;
	}

	cnpoll->cn_remove_all(vap);
	cnidle->cn_remove_all(vap);

	spin_lock(lock);

	while (cnpolicy->clients > 0) {

		policy = TAILQ_FIRST(&cnpolicy->policy_list);
		TAILQ_REMOVE(&cnpolicy->policy_list, policy, entries);
		FREE(policy, M_80211_CN);

		cnpolicy->clients--;
	}

	spin_unlock(lock);

	IEEE80211_DPRINTF(vap, IEEE80211_MSG_CN, "CN Policy Queue: %s\n", "Remove All");

	return 0;
}

static int cn_select_client_to_poll(struct ieee80211vap *vap, u_int8_t mac[IEEE80211_ADDR_LEN], uint32_t * ctstimeout, u_int8_t index)
{
	struct cn_polling_state *cnpoll = vap->iv_poll;
	struct cn_idle_state *cnidle = vap->iv_idle;
	const struct cn_polling *cnpoll_glu = vap->iv_poll_glue;
	struct cn_polling_entry *client_to_poll;
	struct cn_idle_entry *idle_client;
	spinlock_t *lock = &vap->iv_cn_lock;
	int status = -1;
	int last_idle;

	spin_lock(lock);

	do {

		/* Look at the top of the polling queue, for the next client to poll. */
		client_to_poll = TAILQ_FIRST(&cnpoll->polling_list);

		if (index)
			printk(KERN_INFO "%s: index is greater than 0\n", __func__);

#if (0)
		/* In case we want to lookahead to the second element, we will need to
		 * know if the first element was an idle client. This is because, if we
		 * have two idle clients, then we will have to do a look ahead on the 
		 * idle queue as well.
		 */
		if (client_to_poll->client == NULL)
			last_idle = 1;
		else
			last_idle = 0;

		/* If we are interested in the second client in the polling list and
		 * there is more than one client in the list, then we select the next
		 * client in the list.
		 */
		if (index && cnpoll->poll_clients > 1) {
			client_to_poll = TAILQ_NEXT(client_to_poll, entries);
/*			
			if ( client_to_poll->client != NULL )
				printk(KERN_INFO "Next polling: %s\n", 
     					ether_sprintf( client_to_poll->client->mac_address ) );
*/
		}
#endif

		if (client_to_poll != NULL) {

			/* If the head of queue refers us to the idle client. */
			if (client_to_poll->client == NULL) {

//                              printk(KERN_INFO "idle client sending: %s\n", 
//                                      ether_sprintf( mac ) );

				/* Look at the top of the idle queue. */
				idle_client = TAILQ_FIRST(&cnidle->idle_list);

#if (0)
				/* If we are interested in the second client and the first
				 * was also an idle client, then we do a look ahead in
				 * the idle queue (if it has more than one clients).
				 */
				if (index && last_idle && cnidle->idle_clients > 1) {
					idle_client = TAILQ_NEXT(idle_client, entries);

/*					printk(KERN_INFO "Next polling Idle: %s\n", 
     					ether_sprintf( idle_client->client->mac_address ) );
*/
				}
#endif

				if (idle_client != NULL) {
					memcpy(mac, idle_client->client->mac_address, IEEE80211_ADDR_LEN);
					*ctstimeout = idle_client->client->timeout;

					status = 0;
				} else {

#if (0)
//                                      printk(KERN_INFO "idle client should not happen: \n" /*, 
//                                      ether_sprintf( mac ) */);                               

					/* This should not happen. We have no idle clients but
					 * the polling list still has a reference to us.
					 */
					IEEE80211_DPRINTF(vap, IEEE80211_MSG_CN, "%s: Polling list has reference to an empty Idle list\n", __func__);
#endif
					//cnpoll_glu->cn_delete( vap );
					TAILQ_REMOVE(&cnpoll->polling_list, client_to_poll, entries);
					TAILQ_INSERT_TAIL(&cnpoll->polling_list, client_to_poll, entries);
				}
			} else {

				/* If the client has been marked as idle. */
				if (client_to_poll->client->is_idle) {

//                                      printk(KERN_INFO "%s: Polling list has reference to a client already marked as idle\n", 
//                                        __func__);

//                                      printk(KERN_INFO "idle client removed from polling queue: %s\n", 
//                                      ether_sprintf( client_to_poll->client->mac_address ) );

					cnpoll_glu->cn_delete(vap);

				} else {

					memcpy(mac, client_to_poll->client->mac_address, IEEE80211_ADDR_LEN);

					*ctstimeout = client_to_poll->client->timeout;

					status = 0;
				}

			}
		} else {

//                      printk(KERN_INFO "list is empty!!!: %s\n", 
//                                      ether_sprintf( mac ) );

			IEEE80211_DPRINTF(vap, IEEE80211_MSG_CN, "%s: Polling list is empty\n", __func__);

			break;
		}

//              printk(KERN_INFO "Next polling: %s\n", 
//                                      ether_sprintf( mac ) );

		//break;

	} while ((status != 0) && (client_to_poll));

	spin_unlock(lock);

	return (status);
}

static int cn_new_event(struct ieee80211vap *vap, uint8_t event, u_int8_t mac[IEEE80211_ADDR_LEN])
{
	struct cn_polling_state *cnpoll = vap->iv_poll;
	struct cn_idle_state *cnidle = vap->iv_idle;
	const struct cn_idle *cnidle_glu = vap->iv_idle_glue;
	const struct cn_polling *cnpoll_glu = vap->iv_poll_glue;
	struct cn_polling_entry *client_to_poll;
	struct cn_idle_entry *idle_client;
	spinlock_t *lock = &vap->iv_cn_lock;
	int status = -1;

	spin_lock(lock);

	/* Look at the top of the polling queue, for the next client to poll. */
	client_to_poll = TAILQ_FIRST(&cnpoll->polling_list);

	if (client_to_poll != NULL) {

		/* If the head of queue refers us to the idle client. */
		if (client_to_poll->client == NULL) {

//                      printk(KERN_INFO "idle client evemt: %u\n", 
//                                      event );

			/* Look at the top of the idle queue. */
			idle_client = TAILQ_FIRST(&cnidle->idle_list);

			if (idle_client != NULL) {

				if (event != CN_CTS_DATA_RCVD) {

//                                      printk(KERN_INFO "placing idle client at the end evemt: %u\n", 
//                                      event );

					/* Put this client at the end of the queue. */
					TAILQ_REMOVE(&cnidle->idle_list, idle_client, entries);
					TAILQ_INSERT_TAIL(&cnidle->idle_list, idle_client, entries);
				} else if (1) {	/*mac == NULL || 
						   strcmp(ether_sprintf(idle_client->client->mac_address),
						   ether_sprintf( mac ) ) == 0 ) */
					//del_timer( &vap->iv_cn_ctstimer );

					idle_client->client->is_idle = 0;
					idle_client->client->idle_polls = 0;

					cnpoll->pkt_rcvd++;

					while (idle_client->client->current_multiplier < idle_client->client->multiplier) {

						cnpoll_glu->cn_add(vap, idle_client->client);

//                                              printk(KERN_INFO "%s: Client has become active: %s\n", 
//                                              __func__, ether_sprintf(idle_client->client->mac_address));
					}

//                                      printk(KERN_INFO "%s: Client has become active: %s\n", __func__, ether_sprintf(idle_client->client->mac_address));

					cnidle_glu->cn_delete(vap);

					status = 0;

				} else {

//                                      printk(KERN_INFO "%s: Idle Client mac did not match. mine: %s. his: %s\n", 
//                                      __func__, ether_sprintf(idle_client->client->mac_address), 
//                                      ether_sprintf( mac ) );

					IEEE80211_DPRINTF(vap, IEEE80211_MSG_CN, "%s: Idle Client mac did not match: %s\n", __func__, ether_sprintf(idle_client->client->mac_address));

					spin_unlock(lock);

					return (status);
				}

				if (cnidle->cn_idle_multiplier > cnidle->cn_req_multiplier) {

//                                      printk(KERN_INFO "We have more idle clients!!!:\n" );

					/* We have more idle entries in the polling queue, then
					 * what we need.
					 */
					cnpoll_glu->cn_delete(vap);
				} else {

//                                      printk(KERN_INFO "Putting the client at the end of the poll queue:\n");

					TAILQ_REMOVE(&cnpoll->polling_list, client_to_poll, entries);
					TAILQ_INSERT_TAIL(&cnpoll->polling_list, client_to_poll, entries);

					while (cnidle->cn_idle_multiplier != cnidle->cn_req_multiplier)
						cnpoll_glu->cn_add(vap, NULL);
				}

				status = 0;
			} else {

				/* This should not happen. We have no idle clients but
				 * the polling list still has a reference to us.
				 */
				IEEE80211_DPRINTF(vap, IEEE80211_MSG_CN, "%s: Polling list has reference to an empty Idle list\n", __func__);

				cnpoll_glu->cn_delete(vap);
			}
		} else {

			if (event == CN_CTS_TIMEOUT) {

				client_to_poll->client->idle_polls++;

				if (client_to_poll->client->idle_polls >= client_to_poll->client->idle_multiplier) {

					client_to_poll->client->is_idle = 1;

//                                      printk(KERN_INFO 
//                                      "%s: Client CTS timed out. Client is in Idle mode now. polls: %u, multiplier: %u, mac: %s\n", 
//                                      __func__, client_to_poll->client->idle_polls, client_to_poll->client->idle_multiplier, ether_sprintf(client_to_poll->client->mac_address) );

					IEEE80211_DPRINTF(vap, IEEE80211_MSG_CN,
							  "%s: Client CTS timed out. Client is in Idle mode now. polls: %u, multiplier: %u\n",
							  __func__, client_to_poll->client->idle_polls, client_to_poll->client->idle_multiplier);

					cnidle_glu->cn_add(vap, client_to_poll->client);
					cnpoll_glu->cn_delete(vap);

				} else {
//                                      
//                                      printk(KERN_INFO 
//                                      "%s: Client CTS timed out. polls: %u, multiplier: %u, mac: %s\n", 
//                                      __func__, client_to_poll->client->idle_polls, client_to_poll->client->idle_multiplier, ether_sprintf(client_to_poll->client->mac_address) );

					TAILQ_REMOVE(&cnpoll->polling_list, client_to_poll, entries);
					TAILQ_INSERT_TAIL(&cnpoll->polling_list, client_to_poll, entries);
				}

				status = 0;

			} else if ((event == CN_CTS_DATA_RCVD)) {	/*&& 
									   ( mac == NULL || 
									   memcmp( mac, client_to_poll->client->mac_address, 
									   IEEE80211_ADDR_LEN) == 0 ) ) */
				//del_timer( &vap->iv_cn_ctstimer );
				client_to_poll->client->idle_polls = 0;

				cnpoll->pkt_rcvd++;

//                              printk(KERN_INFO "%s: Received data: %s\n", 
//                                      __func__, ether_sprintf(client_to_poll->client->mac_address) );

				TAILQ_REMOVE(&cnpoll->polling_list, client_to_poll, entries);
				TAILQ_INSERT_TAIL(&cnpoll->polling_list, client_to_poll, entries);

				while (client_to_poll->client->current_multiplier < client_to_poll->client->multiplier)
					cnpoll_glu->cn_add(vap, client_to_poll->client);

				status = 0;
			}	/*else if ( event == CN_CTS_DATA_RCVD ) {

				   cnpoll->pkt_rcvd++;

				   printk(KERN_INFO "%s: Client mac did not match: %s\n", 
				   __func__, ether_sprintf(idle_client->client->mac_address) );

				   IEEE80211_DPRINTF(vap, IEEE80211_MSG_CN,
				   "%s: Client mac did not match: %s\n", 
				   __func__, ether_sprintf(client_to_poll->client->mac_address));

				   } */
			else {

//                              printk(KERN_INFO "%s: Unhandled event: %u\n", 
//                                __func__, event);
			}
		}

	} else {

//              printk(KERN_INFO "%s: Polling list is empty\n", __func__);
	}

	spin_unlock(lock);

	return (status);
}

static int cn_next_action(struct ieee80211vap *vap, u_int8_t *tx_mac, u_int8_t *cts_mac, uint8_t have_data)
{
	struct cn_polling_state *cnpoll = vap->iv_poll;
	spinlock_t *lock = &vap->iv_cn_lock;
	uint32_t ctstimeout;
	int status;
	static int sent_data = 0;
	static u_int8_t pending_cts = 0;
	u_int8_t temp_cts;

	spin_lock(lock);

	del_timer(&vap->iv_cn_ctstimer);
	//TIMER hrtimer_cancel( &vap->iv_cn_ctstimer );

//      printk(KERN_INFO "%s: Beginning!: %u, %u, %u, %u\n", 
//               __func__, have_data, sent_data, pending_cts, cnpoll->pkt_rcvd );

	if (cnpoll->pkt_rcvd) {

		if (cnpoll->pkt_rcvd > 1) {
			printk(KERN_INFO "%s: We have received more than one packet: %u\n", __func__, cnpoll->pkt_rcvd);
		}

		/* We have receivd data since we last sent a CTS 
		 * packet. We therefore indicate that one CTS has already
		 * been handled.
		 */
		if (pending_cts)
			pending_cts--;

		cnpoll->pkt_rcvd = 0;
	}

	if (pending_cts > 2 || sent_data > 3) {
		printk(KERN_INFO "%s: We have unexpected values pending_cts: %u, sent_data: %u\n", __func__, pending_cts, sent_data);
	}

	/* If we have two CTS outstanding, or we have one outstanding CTS 
	 * and we have already sent a data packet, then our first CTS
	 * has timed out.
	 */
	if ((pending_cts >= 1 /*2 */ ) || ((pending_cts == 1) && (sent_data >= 1))) {

//              printk(KERN_INFO "%s: pending_cts: %u, sent_data: %u\n",
//                                      __func__, pending_cts, sent_data );

		pending_cts--;
		spin_unlock(lock);
		cn_new_event(vap, CN_CTS_TIMEOUT, NULL);
		spin_lock(lock);
		/* Indicate that a CTS has been handled. */

	}

	//      printk(KERN_INFO "%s: Current situation pending_cts: %u, sent_data: %u, have_data: %u\n",
//                                      __func__, pending_cts, sent_data, have_data );

	/* If we have data to send and we have not exceeded our sending 
	 * limit for a single phase, then send the data.
	 */
	if ((sent_data < 3) && have_data) {
		sent_data++;

		/* Let the calling function know that we want to send data. */
		status = 1;
	} else {

		temp_cts = pending_cts++;
		sent_data = 0;

		spin_unlock(lock);
		status = cn_select_client_to_poll(vap, cts_mac, &ctstimeout, temp_cts);
		spin_lock(lock);

		/* Let the calling function know that we want to send CTS packet. */
		//status = 0;

//              printk(KERN_INFO "%s: In sending cts!: %d\n", 
//               __func__, status );

	}

	vap->iv_cn_ctstimer.expires = jiffies + msecs_to_jiffies(1 /*ctstimeout */ );
	add_timer(&vap->iv_cn_ctstimer);
	//TIMER time = ktime_set(1, 500000);    
	//TIMER hrtimer_start(&vap->iv_cn_ctstimer, time, HRTIMER_MODE_REL);

	spin_unlock(lock);

//      printk(KERN_INFO "%s: Status!: %d, mac: %s\n", 
//               __func__, status, ether_sprintf(cts_mac) );

	return (status);
}

/*------------------------------------------------------------------*/
/*
 * Open CN Policy Queue.
 */

#ifdef CONFIG_SYSCTL

int proc_cn_policy_open(struct inode *inode, struct file *file)
{
	struct proc_ieee80211_priv *pv = NULL;
	struct proc_dir_entry *dp = PDE(inode);
	struct ieee80211vap *vap = dp->data;
	spinlock_t *lock = &vap->iv_cn_lock;
	struct cn_policy_state *cnpolicy = vap->iv_policy;
	struct cn_policy_entry *policy;
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

	pv->vap = vap;

	memset(pv->wbuf, 0, MAX_PROC_IEEE80211_SIZE);
	memset(pv->rbuf, 0, MAX_PROC_IEEE80211_SIZE);
	pv->max_wlen = MAX_PROC_IEEE80211_SIZE;
	pv->max_rlen = MAX_PROC_IEEE80211_SIZE;

	/* now read the data into the buffer */
	p = pv->rbuf;
	p += sprintf(p, "\nPolicy Queue (%u elements)\n", cnpolicy->clients);

	spin_lock(lock);

	count = 0;

	p += sprintf(p, "\tMac Address\t\tTimeout\tIdle Multiplier\tMultiplier\tCurrent Multiplier\tIs idle\n");

	TAILQ_FOREACH(policy, &cnpolicy->policy_list, entries) {

		p += sprintf(p, "%u:\t%s", count, ether_sprintf(policy->mac_address));
		p += sprintf(p, "\t%u\t%u\t\t%u\t\t%u\t\t\t%s\n", policy->timeout, policy->idle_multiplier, policy->multiplier, policy->current_multiplier, (policy->is_idle) ? "true" : "false");
		count++;
	}

	spin_unlock(lock);

	pv->rlen = (p - pv->rbuf);
	return (0);
}

/**
 * This function is called with the /proc file is written
 *
 */

static int proc_cn_policy_write(struct file *file, const char *buffer, size_t len, loff_t * off)
{
	struct proc_ieee80211_priv *pv = file->private_data;
	struct ieee80211vap *vap = pv->vap;
	struct ether_addr ethernet_address;
	uint32_t timeout = 0;
	uint32_t idle_multiplier = 0;
	uint32_t multiplier = 0;
	char *read;
	int status = len;

	if (len > MAX_PROC_IEEE80211_SIZE) {
		len = MAX_PROC_IEEE80211_SIZE;
	}

	if (copy_from_user(pv->wbuf, buffer, len)) {

//              printk(KERN_INFO "Data not written! Buffer = %s, len=%u\n", 
//                              pv->wbuf, len );

		return -EFAULT;
	}

	read = pv->wbuf;

	while (read < pv->wbuf + len) {

		if (memcmp(read, "add ", 4) == 0) {

			read += 4;

			/* Skip all white spaces. */
			while ((*read == ' ' || *read == '\t') && *read != '\n' && *read != '\0')
				read++;

			if (cn_ether_aton(read, &ethernet_address) != ETH_ALEN) {
				status = -EFAULT;
				break;
			}

			read += (ETH_ALEN * 3);

			if (sscanf(read, "%u", &timeout) != 1) {
				status = -EFAULT;
				break;
			}

			while (*read != ' ' && *read != '\t' && *read != '\n' && *read != '\0')
				read++;

			while ((*read == ' ' || *read == '\t') && *read != '\n' && *read != '\0')
				read++;

			if (sscanf(read, "%u", &idle_multiplier) != 1) {
				status = -EFAULT;
				break;
			}

			while (*read != ' ' && *read != '\t' && *read != '\n' && *read != '\0')
				read++;

			while (*read == ' ' || *read == '\t')
				read++;

			if (sscanf(read, "%u", &multiplier) != 1) {
				status = -EFAULT;
				break;
			}

			cn_policy_add(vap, ethernet_address.octet, (uint16_t) (timeout & 0x0000FFFF), (uint8_t) (idle_multiplier & 0x000000FF), (uint8_t) (multiplier & 0x000000FF));

			while (*read != ' ' && *read != '\t' && *read != '\n' && *read != '\0')
				read++;

		} else if (memcmp(read, "rm ", 3) == 0) {

			read += 3;

			/* Skip all white spaces. */
			while ((*read == ' ' || *read == '\t') && *read != '\n' && *read != '\0')
				read++;

			if (memcmp(read, "ALL", 3) == 0) {

				read += 3;

				cn_policy_del_all(vap);

			} else {

				if (cn_ether_aton(read, &ethernet_address) != ETH_ALEN) {
					status = -EFAULT;
					break;
				}

				read += (ETH_ALEN * 3);

				cn_policy_del(vap, ethernet_address.octet);
			}

/*		} else if ( memcmp( read, "idle ", 5 ) == 0 ) {
			
			read += 5;

			while ( ( *read == ' ' || *read == '\t' ) && *read != '\n' &&  *read != '\0' )
				read++;
				
			if ( memcmp( read, "multiplier ", 10 ) == 0 ) {
				
				read += 3;
				
				cn_policy_del_all( vap );
				
			} else {
			
				if ( cn_ether_aton( read, &ethernet_address ) != ETH_ALEN ) {				
					status = -EFAULT;
					break;
				}
			
				read += ( ETH_ALEN * 3 );
				
				cn_policy_del( vap, ethernet_address.octet );
			}			
*/
		} else {

			IEEE80211_DPRINTF(vap, IEEE80211_MSG_CN, "CN Policy Queue: unknown command in string: %s\n", read);

			read++;
		}

		while (read < pv->wbuf && (*read == ' ' || *read == '\t' || *read != '\n' || *read == '\0'))
			read++;
	}

	return status;
}

/*------------------------------------------------------------------*/
/*
 * Define /proc file opeations.
 */

static struct file_operations proc_cn_policy = {
	.read = NULL,
	.write = proc_cn_policy_write,
	.open = proc_cn_policy_open,
	.release = NULL,
};

/*------------------------------------------------------------------*/
/*
 * Register Cn Policy Queue.
 */

void ath_cn_policy_proc_register(struct ieee80211vap *vap)
{
	ieee80211_proc_vcreate(vap, &proc_cn_policy, "cn_policy");
}

EXPORT_SYMBOL(ath_cn_policy_proc_register);

#endif				/* CONFIG_SYSCTL */

/*
 * Module glue.
 */

/*------------------------------------------------------------------*/
/*
 * Cn Policy Queue structure.
 */

static const struct cn_policy cn_pol = {
	.cn_attach = cn_policy_attach,
	.cn_detach = cn_policy_detach,
	.cn_add = cn_policy_add,
	.cn_delete = cn_policy_del,
	.cn_select = cn_select_client_to_poll,
	.cn_event = cn_new_event,
	.cn_action = cn_next_action,
};

/*------------------------------------------------------------------*/
/*
 * Initialize Cn Policy Queue.
 */

int __init init_ieee80211_cn_policy(void)
{
	cn_policy_register(&cn_pol);
	return 0;
}

#ifndef SINGLE_MODULE

module_init(init_ieee80211_cn_policy);
#endif

/*------------------------------------------------------------------*/
/*
 * Exit Cn Policy Queue.
 */

void __exit exit_ieee80211_cn_policy(void)
{
	cn_policy_unregister(&cn_pol);
}

#ifndef SINGLE_MODULE
module_exit(exit_ieee80211_cn_policy);
#endif

#endif
