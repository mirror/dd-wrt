
/* $Id: isdn_dwabc.c,v 1.27 2001/12/01 23:18:21 detabc Exp $

 * Linux ISDN subsystem, abc-extension releated funktions.
 *
 * Copyright           by abc GmbH
 *                     written by Detlef Wengorz <detlefw@isdn4linux.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 */

#include <linux/config.h>
#define __NO_VERSION__

#ifdef CONFIG_ISDN_WITH_ABC

static char *dwabcrevison = "$Revision: 1.27 $";

#include <asm/semaphore.h>
#define CONFIG_ISDN_WITH_ABC_NEED_DWSJIFFIES 	1
#include <linux/list.h>
#include <linux/isdn.h>
#include "isdn_common.h"
#include "isdn_net.h"

#include <linux/skbuff.h>

#include <net/udp.h>
#include <net/checksum.h>
#include <linux/isdn_dwabc.h>


#if CONFIG_ISDN_WITH_ABC_RAWIPCOMPRESS && CONFIG_ISDN_PPP
#include <linux/isdn_ppp.h>
extern struct isdn_ppp_compressor *isdn_ippp_comp_head;
#define ipc_head isdn_ippp_comp_head
#ifndef CI_BSD_COMPRESS
#define CI_BSD_COMPRESS 21
#endif
#endif

#define NBYTEORDER_30BYTES      0x1e00 
#define DWABC_TMRES (HZ / 10)

#define VERBLEVEL (dev->net_verbose > 2)

static struct timer_list dw_abc_timer;


#ifdef CONFIG_ISDN_WITH_ABC_LCR_SUPPORT
static ISDN_DWSPINLOCK lcr_spin = ISDN_DWSPIN_UNLOCKED;
#define LCR_LOCK() isdn_dwspin_trylock(&lcr_spin)
#define LCR_ULOCK() isdn_dwspin_unlock(&lcr_spin)

typedef struct ISDN_DW_ABC_LCR {

	struct list_head dll;
	char lcr_printbuf[64 + ISDN_MSNLEN + ISDN_MSNLEN];
	char *lcr_poin;
	char *lcr_epoin;

} ISDN_DW_ABC_LCR;

static LIST_HEAD(lcr_dll);
static atomic_t lcr_open_count		=	ATOMIC_INIT(0);
static volatile  ulong lcr_call_counter	= 0;


static int myjiftime(char *p,u_long nj)
{
	sprintf(p,"%02ld:%02ld.%02ld",
		((nj / 100) / 60) % 100, (nj / 100) % 60,nj % 100);

	return(8);
}


static void dw_lcr_clear_all(void)
{
	struct list_head *lh;

	if(!LCR_LOCK()) {

		while((lh = lcr_dll.next) != &lcr_dll) {

			ISDN_DW_ABC_LCR *p = list_entry(lh,ISDN_DW_ABC_LCR,dll);
			list_del(&p->dll);
			kfree(p);
		}

		LCR_ULOCK();
	}
}

void isdn_dw_abc_lcr_open(void) 
{ atomic_inc(&lcr_open_count); }

void isdn_dw_abc_lcr_close(void) 
{ 
	if(atomic_dec_and_test(&lcr_open_count))
		dw_lcr_clear_all();
}

int isdn_dw_abc_lcr_lock(void) 
{ return(LCR_LOCK()); }

void isdn_dw_abc_lcr_ulock(void) 
{ LCR_ULOCK(); }


size_t isdn_dw_abc_lcr_readstat(char *buf,size_t count) 
{
	size_t  retw = 0;

	while(buf != NULL && count > 0) {

		struct list_head *lh = NULL;
		ISDN_DW_ABC_LCR *p = NULL;
		char *dp = NULL;
		size_t  n;

		if((n = LCR_LOCK())) {

			if(!retw)
				retw = n;

			break;
		}


		while((lh = lcr_dll.next) != &lcr_dll) {

			p = list_entry(lh,ISDN_DW_ABC_LCR,dll);

			if(p->lcr_poin >= p->lcr_epoin) {

				list_del(&p->dll);
				kfree(p);
				p = NULL;

			} else break;
		}

		if(p == NULL) {
				
			LCR_ULOCK();
			break;
		}

		n = p->lcr_epoin - p->lcr_poin;

		if(n > count)
			n = count;

		dp = p->lcr_poin;
		p->lcr_poin += n;
		retw += n;
		LCR_ULOCK();
		copy_to_user(buf,dp,n);
		buf += n;
	}

	return(retw);
}


static void isdn_dw_abc_lcr_clear_helper(isdn_net_local *lp)
{
	if(lp != NULL) {

		void *a,*b;

		a = lp->dw_abc_lcr_cmd;  
		b = lp->dw_abc_lcr_io;
		lp->dw_abc_lcr_io = NULL;
		lp->dw_abc_lcr_cmd = NULL;
		lp->dw_abc_lcr_callid = 
		lp->dw_abc_lcr_start_request =
		lp->dw_abc_lcr_end_request = 0;
		
		if(a) kfree(a);
		if(b) kfree(b);
	}
}

void isdn_dw_abc_lcr_clear(isdn_net_local *lp)
{
	if(!LCR_LOCK()) {
		isdn_dw_abc_lcr_clear_helper(lp);
		LCR_ULOCK();
	}
}


u_long isdn_dw_abc_lcr_call_number( isdn_net_local *lp,isdn_ctrl *call_cmd)
{
	u_long mid = 0;

	if(LCR_LOCK())
		return(0);

	isdn_dw_abc_lcr_clear_helper(lp);

	if( atomic_read(&lcr_open_count) > 0 && 
		lp != NULL 						&& 
		call_cmd != NULL) {

		ISDN_DW_ABC_LCR  *lc = NULL;
		int ab = 0;

		if((lp->dw_abc_lcr_cmd = 
			( isdn_ctrl *)kmalloc(sizeof(isdn_ctrl),GFP_ATOMIC)) == NULL) {

no_mem_out:;
			isdn_dw_abc_lcr_clear_helper(lp);
			LCR_ULOCK();
			printk(KERN_DEBUG "%s %d : LCR no memory\n",__FILE__,__LINE__);
			return(0);
		}

		memcpy(lp->dw_abc_lcr_cmd,call_cmd,sizeof(*call_cmd));
		while(!(lp->dw_abc_lcr_callid = mid = lcr_call_counter++));
		
		lp->dw_abc_lcr_end_request = lp->dw_abc_lcr_start_request = jiffies;
		lp->dw_abc_lcr_end_request += HZ * 3;

		if((lc = (ISDN_DW_ABC_LCR  *)kmalloc(sizeof(*lc),GFP_KERNEL)) == NULL)
			goto no_mem_out;

		lc->lcr_poin = lc->lcr_epoin = lc->lcr_printbuf;
		lc->lcr_epoin += myjiftime(lc->lcr_epoin,jiffies);

		sprintf(lc->lcr_epoin," DW_ABC_LCR\t%lu\t%.*s\t%.*s\n",
			mid,
			(int)ISDN_MSNLEN,
			call_cmd->parm.setup.eazmsn,
			(int)ISDN_MSNLEN,
			call_cmd->parm.setup.phone);

		lc->lcr_epoin += strlen(lc->lcr_epoin);
		ab = lc->lcr_epoin - lc->lcr_poin;

		list_add_tail(&lc->dll,&lcr_dll);
		LCR_ULOCK();

		if(ab > 0) {

			if(dev->drv[0] != NULL ) {

				dev->drv[0]->stavail += ab;
				wake_up_interruptible(&dev->drv[0]->st_waitq);
			}
		}

	} else LCR_ULOCK();

	return(mid);
}


int isdn_dw_abc_lcr_ioctl(u_long arg)
{
	struct ISDN_DWABC_LCR_IOCTL	i;
	int need = sizeof(struct ISDN_DWABC_LCR_IOCTL); 
	isdn_net_dev *p; 

	memset(&i,0,sizeof(struct ISDN_DWABC_LCR_IOCTL));
	copy_from_user(&i,(char *)arg,sizeof(int));

	if(i.lcr_ioctl_sizeof < need)
		need = i.lcr_ioctl_sizeof;

	if(need > 0) 
		copy_from_user(&i,(char *)arg,need);

	 if(LCR_LOCK())
	 	return(-EAGAIN);

	 p = dev->netdev; 

	 for(;p ; p = p->next) {

	 	isdn_net_local *lp = p->local;

	 	if(	lp->dw_abc_lcr_callid != i.lcr_ioctl_callid)
			continue;

		if(lp->dw_abc_lcr_cmd == NULL) 
			continue;

		if(lp->dw_abc_lcr_io == NULL)
			lp->dw_abc_lcr_io = (struct ISDN_DWABC_LCR_IOCTL *)
				kmalloc(sizeof(struct ISDN_DWABC_LCR_IOCTL),GFP_ATOMIC);

		if(lp->dw_abc_lcr_io == NULL) {

			printk(KERN_DEBUG "%s %d : no memory\n",__FILE__,__LINE__);
			continue;
		}

		memcpy(lp->dw_abc_lcr_io,&i,sizeof(struct ISDN_DWABC_LCR_IOCTL));

		if(i.lcr_ioctl_flags & DWABC_LCR_FLG_NEWNUMBER) {

			char *xx = i.lcr_ioctl_nr;
			char *exx = xx + sizeof(i.lcr_ioctl_nr);
			char *d = lp->dw_abc_lcr_cmd->parm.setup.phone;
			char *ed = d + sizeof(lp->dw_abc_lcr_cmd->parm.setup.phone) - 1;

			while(d < ed && xx < exx && *xx) *(d++) = *(xx++);
			while(d < ed) *(d++) = 0;
			*d = 0;
		}
	 }

	 LCR_ULOCK();
	 return(0);
}

#endif


#ifdef CONFIG_ISDN_WITH_ABC_UDP_CHECK
int dw_abc_udp_test(struct sk_buff *skb,struct net_device *ndev)
{
	if(ndev != NULL && skb != NULL && skb->protocol == htons(ETH_P_IP)) {

		struct iphdr *iph = (struct iphdr *)skb->data;
		isdn_net_local *lp = (isdn_net_local *) ndev->priv;
		int rklen = skb->len;

		if (skb->nh.raw > skb->data && skb->nh.raw < skb->tail) {

			rklen -= (char *)skb->nh.raw - (char *)skb->data;
			iph = (struct iphdr *)skb->nh.raw;
		}

		if(rklen >= 20 && iph->version == 4 && 
			!(lp->dw_abc_flags & ISDN_DW_ABC_FLAG_NO_UDP_CHECK)) {

			if(	iph->tot_len == NBYTEORDER_30BYTES	&& 
				iph->protocol == IPPROTO_UDP) {

				struct udphdr *udp = 
					(struct udphdr *)((char *)iph + (iph->ihl << 2));

				ushort usrc = ntohs(udp->source);

				if(	udp->dest == htons(25001) && 
					usrc >= 20000 && usrc < 25000) {

					char *p = (char *)(udp + 1);

					if(p[0] == p[1]) {

						char mc = 0;

						switch(*p) {
						case 0x30:

							mc = *p;

							if((lp->flags & ISDN_NET_CONNECTED) && (!lp->dialstate))
								mc++;

							break;

						case 0x32:

							mc = *p;
#ifdef CONFIG_ISDN_WITH_ABC_UDP_CHECK_DIAL
							if((lp->flags & ISDN_NET_CONNECTED) && (!lp->dialstate)) {

								mc++;
								break;
							}

							if(!isdn_net_force_dial_lp(lp)) mc++;
#endif
							break;

						case 0x11:
							mc = *p + 1;
							isdn_dw_abc_reset_interface(lp,1);
							break;

						case 0x28:	mc = *p + 1;	break;
						case 0x2a:
						case 0x2c:

							mc = *p;
#ifdef CONFIG_ISDN_WITH_ABC_UDP_CHECK_HANGUP
							if(!(lp->dw_abc_flags & ISDN_DW_ABC_FLAG_NO_UDP_HANGUP)) {

								if(lp->isdn_device >= 0) {

									isdn_net_hangup(ndev);
									mc = *p + 1;
								}
							}
#endif
							break;
						}

						if(mc) {

							struct sk_buff *nskb;
							int need = 2+sizeof(struct iphdr)+sizeof(struct udphdr);
							int hneed = need + ndev->hard_header_len;

							if((nskb = (struct sk_buff *)dev_alloc_skb(hneed)) != NULL) {

								ushort n = sizeof(struct udphdr) + 2;
								struct iphdr *niph;
								struct udphdr *nup;
								skb_reserve(nskb,ndev->hard_header_len);

								if((niph = (struct iphdr *)skb_put(nskb,need))==NULL){

									printk(KERN_DEBUG "%s: skb_put failt (%d bytes)\n", lp->name,hneed);
									dev_kfree_skb(nskb);
									return(0);
								}

								nup = (struct udphdr *)(niph + 1);
								((char *)(nup + 1))[0] = mc;
								((char *)(nup + 1))[1] = mc;
								nup->source=udp->dest;
								nup->dest=udp->source;
								nup->len=htons(n);
								nup->check=0; /* dont need checksum */
								memset((void *)niph,0,sizeof(*niph));
								niph->version=4;
								niph->ihl=5;
								niph->tot_len=NBYTEORDER_30BYTES;
								niph->ttl = 32;
								niph->protocol = IPPROTO_UDP;
								niph->saddr=iph->daddr;
								niph->daddr=iph->saddr;
								niph->id=iph->id;
								niph->check=ip_fast_csum((unsigned char *)niph,niph->ihl);
								nskb->dev = ndev;
								nskb->pkt_type = PACKET_HOST;
								nskb->protocol = htons(ETH_P_IP);
								nskb->mac.raw = nskb->data;
								netif_rx(nskb);
							}

							return(1);
						}
					}
				}
			}
		}
	}

	return(0);
}
#endif


void isdn_dw_clear_if(ulong pm,isdn_net_local *lp)
{
	if(lp != NULL) {
#ifdef CONFIG_ISDN_WITH_ABC_LCR_SUPPORT
		isdn_dw_abc_lcr_clear(lp);
#endif
	}
}



static void dw_abc_timer_func(u_long dont_need_yet)
{
	register u_long t;

	if(!((t = ++isdn_dwabc_jiffies.msec_100) & 1))
		if(isdn_dwabc_jiffies.msec_200++ & 1)
			isdn_dwabc_jiffies.msec_400++;
	
	if(!(t % 5)) 
		if(isdn_dwabc_jiffies.msec_500++ & 1)
			isdn_dwabc_jiffies.msec_1000++;

	dw_abc_timer.expires = jiffies + DWABC_TMRES;
	add_timer(&dw_abc_timer);
}


void isdn_dw_abc_init_func(void)
{

	init_timer(&dw_abc_timer);
	dw_abc_timer.function = dw_abc_timer_func;


	printk( KERN_INFO
		"abc-extension %s Kernel 0x%06X\n"
		"written by\nDetlef Wengorz <detlefw@isdn4linux.de>\n"
		"Installed options:\n"
#ifdef CONFIG_ISDN_WITH_ABC_CALLB
		"CONFIG_ISDN_WITH_ABC_CALLB\n"
#endif
#ifdef CONFIG_ISDN_WITH_ABC_UDP_CHECK
		"CONFIG_ISDN_WITH_ABC_UDP_CHECK\n"
#endif
#ifdef CONFIG_ISDN_WITH_ABC_UDP_CHECK_HANGUP
		"CONFIG_ISDN_WITH_ABC_UDP_CHECK_HANGUP\n"
#endif
#ifdef CONFIG_ISDN_WITH_ABC_UDP_CHECK_DIAL
		"CONFIG_ISDN_WITH_ABC_UDP_CHECK_DIAL\n"
#endif
#ifdef CONFIG_ISDN_WITH_ABC_OUTGOING_EAZ
		"CONFIG_ISDN_WITH_ABC_OUTGOING_EAZ\n"
#endif
#ifdef CONFIG_ISDN_WITH_ABC_LCR_SUPPORT
		"CONFIG_ISDN_WITH_ABC_LCR_SUPPORT\n"
#endif
#ifdef CONFIG_ISDN_WITH_ABC_RCV_NO_HUPTIMER
		"CONFIG_ISDN_WITH_ABC_RCV_NO_HUPTIMER\n"
#endif
#ifdef CONFIG_ISDN_WITH_ABC_CH_EXTINUSE
		"CONFIG_ISDN_WITH_ABC_CH_EXTINUSE\n"
#endif
#ifdef CONFIG_ISDN_WITH_ABC_CONN_ERROR
		"CONFIG_ISDN_WITH_ABC_CONN_ERROR\n"
#endif
#ifdef CONFIG_ISDN_WITH_ABC_RAWIPCOMPRESS
		"CONFIG_ISDN_WITH_ABC_RAWIPCOMPRESS\n"
#endif
		"loaded\n",
		dwabcrevison,LINUX_VERSION_CODE);
		dwsjiffies = 0;
		dw_abc_timer.expires = jiffies + DWABC_TMRES;
		add_timer(&dw_abc_timer);
}

void isdn_dw_abc_release_func(void)
{
	del_timer(&dw_abc_timer);
#ifdef CONFIG_ISDN_WITH_ABC_LCR_SUPPORT
	dw_lcr_clear_all();
#endif
	printk( KERN_INFO
		"abc-extension %s  Kernel 0x%06X\n"
		"written by\n"
		"Detlef Wengorz <detlefw@isdn4linux.de>\n"
		"unloaded\n",
		dwabcrevison,LINUX_VERSION_CODE);
}


void isdn_dwabc_test_phone(isdn_net_local *lp) 
{
	if(lp != NULL) {

		isdn_net_phone *h = lp->phone[0];
		ulong oflags = lp->dw_abc_flags;
		int secure = 0;

		lp->dw_abc_flags = 0;
#ifdef CONFIG_ISDN_WITH_ABC_OUTGOING_EAZ
		*lp->dw_out_msn = 0;
#endif

		for(;h != NULL && secure < 1000;secure++,h = h->next) {

			char *p 	= 	h->num;
			char *ep 	= 	p + ISDN_MSNLEN;

			for(;p < ep && *p && (*p <= ' ' || *p == '"' || *p == '\'');p++);

			if(p >= ep)
				continue;

#ifdef CONFIG_ISDN_WITH_ABC_OUTGOING_EAZ
			if(*p == '>') {

				if(++p < ep && *p != '<' && *p != '>') {

					char *d = lp->dw_out_msn;

					for(;*p && (p < ep) && (*p == ' ' || *p == '\t');p++);
					for(ep--;*p && (p < ep);) *(d++) = *(p++);
					*d = 0;
					continue;
				}
			}
#endif

			if(*p == '~') {

				/* abc switch's */

				for(p++;p < ep && *p;p++) switch(*p) {
				case 'u':	lp->dw_abc_flags |= ISDN_DW_ABC_FLAG_NO_UDP_CHECK;			break;
				case 'h':	lp->dw_abc_flags |= ISDN_DW_ABC_FLAG_NO_UDP_HANGUP;			break;
				case 'd':	lp->dw_abc_flags |= ISDN_DW_ABC_FLAG_NO_UDP_DIAL;			break;
				case 'c':	lp->dw_abc_flags |= ISDN_DW_ABC_FLAG_NO_CH_EXTINUSE;		break;
				case 'e':   lp->dw_abc_flags |= ISDN_DW_ABC_FLAG_NO_CONN_ERROR;			break;
				case 'l':   lp->dw_abc_flags |= ISDN_DW_ABC_FLAG_NO_LCR;				break;

				case 'x':
				case 'X':	lp->dw_abc_flags |= ISDN_DW_ABC_FLAG_RCV_NO_HUPTIMER;		break;

				case 'B':	lp->dw_abc_flags |= ISDN_DW_ABC_FLAG_BSD_COMPRESS;			break;
				case 'L': 	lp->dw_abc_flags |= ISDN_DW_ABC_FLAG_LEASED_LINE;			break;

				case '"':
				case ' ':
				case '\t':
				case '\'':	break;

				default:	
					printk(KERN_DEBUG"isdn_net: %s abc-switch <~%c> unknown\n",lp->name,*p);
					break;
				}
			}
		}

		if(lp->dw_abc_flags & ISDN_DW_ABC_FLAG_LEASED_LINE) {

			lp->dw_abc_flags |= 
					ISDN_DW_ABC_FLAG_NO_UDP_CHECK		|
					ISDN_DW_ABC_FLAG_NO_UDP_HANGUP		|
					ISDN_DW_ABC_FLAG_NO_UDP_DIAL		|
					ISDN_DW_ABC_FLAG_NO_CH_EXTINUSE		|
					ISDN_DW_ABC_FLAG_NO_CONN_ERROR		|
					ISDN_DW_ABC_FLAG_NO_LCR;
		}

		if(dev->net_verbose  && (lp->dw_abc_flags != oflags || dev->net_verbose > 4))
			printk(KERN_DEBUG "isdn_net %s abc-flags 0x%lx\n",lp->name,lp->dw_abc_flags);

	}
}


int isdn_dw_abc_reset_interface(isdn_net_local *lp,int with_message)
{
	int r = -EINVAL;

	if(lp != NULL) {

		r = 0;

		lp->dw_abc_bchan_last_connect = 0;
		lp->dw_abc_dialstart = 0;
		lp->dw_abc_inuse_secure = 0;
#ifdef CONFIG_ISDN_WITH_ABC_CONN_ERROR
		lp->dw_abc_bchan_errcnt = 0;
#endif

		if(with_message && dev->net_verbose > 0)
			printk(KERN_INFO
				"%s: NOTE: reset (clear) abc-interface-secure-counter\n",
				lp->name);
	}

	return(r);
}

	
#if CONFIG_ISDN_WITH_ABC_RAWIPCOMPRESS && CONFIG_ISDN_PPP

#define DWBSD_PKT_FIRST_LEN 16
#define DWBSD_PKT_SWITCH	165
#define DWBSD_PKT_BSD		189

#define DWBSD_VERSION 		0x2

void dwabc_bsd_first_gen(isdn_net_local *lp)
{
	if(lp != NULL && lp->p_encap == ISDN_NET_ENCAP_RAWIP && 
		(lp->dw_abc_flags & ISDN_DW_ABC_FLAG_BSD_COMPRESS)) { 
		
		struct sk_buff *skb = NULL;
		char *p = NULL;
		char *ep = NULL;

		if((skb =(struct sk_buff *)dev_alloc_skb(128)) == NULL) {

			printk(KERN_INFO "%s: dwabc: alloc-skb failed for 128 bytes\n",lp->name);
			return;
		}

		skb_reserve(skb,64);
		p = skb_put(skb,DWBSD_PKT_FIRST_LEN);
		ep = p + DWBSD_PKT_FIRST_LEN;

		*(p++) = DWBSD_PKT_SWITCH;
		*(p++) = DWBSD_VERSION;
		for(;p < ep;p++)	*(p++) = 0;

		isdn_net_write_super(lp, skb);

		if(dev->net_verbose > 2)
			printk(KERN_INFO "%s: dwabc: sending comm-header version 0x%x\n",lp->name,DWBSD_VERSION);
	}
}


void dwabc_bsd_free(isdn_net_local *lp)
{
	if(lp != NULL) {

		if(lp->dw_abc_bsd_stat_rx || lp->dw_abc_bsd_stat_tx) {

			struct isdn_ppp_compressor *c = NULL;

			if(!(c = (struct isdn_ppp_compressor *)lp->dw_abc_bsd_compressor)) {

				printk(KERN_WARNING
				"%s: PANIC: freeing bsd compressmemory without compressor\n",
					lp->name);

			} else {

				if(lp->dw_abc_bsd_stat_rx) (*c->free)(lp->dw_abc_bsd_stat_rx);
				if(lp->dw_abc_bsd_stat_tx) (*c->free)(lp->dw_abc_bsd_stat_tx);

				if(dev->net_verbose > 2)
					printk(KERN_INFO
						"%s: free bsd compress-memory\n",
						lp->name);
			}
		}

		lp->dw_abc_bsd_compressor = NULL;
		lp->dw_abc_bsd_stat_rx = NULL;
		lp->dw_abc_bsd_stat_tx = NULL;
		lp->dw_abc_if_flags &= ~ISDN_DW_ABC_IFFLAG_BSDAKTIV;

		if(dev->net_verbose > 0) {

			if(lp->dw_abc_bsd_rcv != lp->dw_abc_bsd_bsd_rcv) {

				printk(KERN_INFO "%s: Receive %lu<-%lu kb\n",lp->name,
					lp->dw_abc_bsd_rcv >> 10 , lp->dw_abc_bsd_bsd_rcv >> 10);
			}


			if(lp->dw_abc_bsd_snd != lp->dw_abc_bsd_bsd_snd) {

				printk(KERN_INFO "%s: Send  %lu->%lu kb\n",lp->name,
					lp->dw_abc_bsd_snd >> 10 , lp->dw_abc_bsd_bsd_snd >> 10);
			}
		}

		lp->dw_abc_bsd_rcv 		=
		lp->dw_abc_bsd_bsd_rcv	=
		lp->dw_abc_bsd_snd 		=
		lp->dw_abc_bsd_bsd_snd 	= 0;
	}
}


int dwabc_bsd_init(isdn_net_local *lp)
{
	int r = 1;

	if(lp != NULL) {

		dwabc_bsd_free(lp);

		if(lp->p_encap == ISDN_NET_ENCAP_RAWIP) {

			void *rx = NULL;
			void *tx = NULL;
			struct isdn_ppp_comp_data *cp = NULL;
			struct isdn_ppp_compressor *c = NULL;

			if(lp->dw_abc_flags & ISDN_DW_ABC_FLAG_BSD_COMPRESS) do {

				for(c = ipc_head ;
					c != NULL && c->num != CI_BSD_COMPRESS; c = c->next);

				if(c == NULL) {

					printk(KERN_INFO
						"%s: Module isdn_bsdcompress not loaded\n",
						lp->name);

					break;
				}

				cp = (struct isdn_ppp_comp_data *)
					kmalloc(sizeof(struct isdn_ppp_comp_data),GFP_ATOMIC);

				if(cp == NULL) {

					printk(KERN_INFO
						"%s: allocation of isdn_ppp_comp_data failed\n",
						lp->name);

					break;
				}

				memset(cp,0,sizeof(*cp));
				cp->num = CI_BSD_COMPRESS;
				cp->optlen = 1;
					
				/*
				** set BSD_VERSION 1 and 12 bits compressmode
				*/
				*cp->options = (1 << 5) | 12;

				if((rx = (*c->alloc)(cp)) == NULL) {

					printk(KERN_INFO
						"%s: allocation of bsd rx-memory failed\n",
						lp->name);

					break;
				}
					
				if(!(*c->init)(rx,cp,0,1)) {

					printk(KERN_INFO 
						"%s: init of bsd rx-stream  failed\n",lp->name);

					break;
				}

				cp->flags = IPPP_COMP_FLAG_XMIT;
						
				if((tx = (*c->alloc)(cp)) == NULL) {

					printk(KERN_INFO
						"%s: allocation of bsd tx-memory failed\n",
						lp->name);

					break;
				}

				if(!(*c->init)(tx,cp,0,1)) {

					printk(KERN_INFO
						"%s: init of bsd tx-stream  failed\n",
						lp->name);

					break;
				}

				lp->dw_abc_bsd_compressor = (void *)c;
				lp->dw_abc_bsd_stat_rx = rx;
				lp->dw_abc_bsd_stat_tx = tx;
				rx = tx = NULL;
				r = 0;

				if(dev->net_verbose > 2)
					printk(KERN_INFO
						"%s: bsd compress-memory and init ok\n",
						lp->name);

			} while(0);

			if(cp != NULL)
				kfree(cp);

			if(c != NULL) {

				if(tx != NULL) (*c->free)(tx);
				if(rx != NULL) (*c->free)(rx);
			}

		} else if(lp->dw_abc_flags & ISDN_DW_ABC_FLAG_BSD_COMPRESS) {
		
			printk(KERN_INFO
				"%s: bsd-compress only with encapsulation rawip allowed\n",
				lp->name);
		}
	}

	return(r);
}

struct sk_buff *dwabc_bsd_compress(	isdn_net_local *lp,
									struct sk_buff *skb,
									struct net_device *ndev)
{
	if(lp != NULL && lp->p_encap == ISDN_NET_ENCAP_RAWIP 	&& 
		(lp->dw_abc_flags & ISDN_DW_ABC_FLAG_BSD_COMPRESS)	&&
		(lp->dw_abc_if_flags & ISDN_DW_ABC_IFFLAG_BSDAKTIV)) {

		if(lp->dw_abc_bsd_stat_tx != NULL && lp->dw_abc_bsd_compressor) {

			struct isdn_ppp_compressor *cp = 
				(struct isdn_ppp_compressor *)lp->dw_abc_bsd_compressor;

			struct sk_buff *nskb = (struct sk_buff *)
				dev_alloc_skb(skb->len * 2 + ndev->hard_header_len);

			int l = 0;

			if(nskb == NULL) {

				(void)(*cp->reset)(lp->dw_abc_bsd_stat_tx,0,0,NULL,0,NULL);
				printk(KERN_INFO "%s: dwabc-compress no memory\n",lp->name);

			} else {

				skb_reserve(nskb,ndev->hard_header_len);
				*(unsigned char *)skb_put(nskb,1) = DWBSD_PKT_BSD;
				l = (*cp->compress)(lp->dw_abc_bsd_stat_tx,skb,nskb,0x21);

				if(l < 1 || l > skb->len) {

					(void)(*cp->reset)(lp->dw_abc_bsd_stat_tx,0,0,NULL,0,NULL);
					dev_kfree_skb(nskb);

				} else {

					u_short sqnr;

					dev_kfree_skb(skb);
					skb = nskb;
					sqnr = ((*(u_char *)skb->data) << 8) + 
							((u_char)skb->data[1]);

					if(sqnr > 65500)
						(void)(*cp->reset)
							(lp->dw_abc_bsd_stat_tx,0,0,NULL,0,NULL);
				}
			}
		}
	}

	return(skb);
}

struct sk_buff *dwabc_bsd_rx_pkt(	isdn_net_local *lp,
									struct sk_buff *skb,
									struct net_device *ndev)
{
	struct sk_buff *r = skb;

	if(lp != NULL && lp->p_encap == ISDN_NET_ENCAP_RAWIP && 
		(lp->dw_abc_flags & ISDN_DW_ABC_FLAG_BSD_COMPRESS)) { 

		unsigned char *p = (unsigned char *)skb->data;
		struct isdn_ppp_compressor *cp = 
			(struct isdn_ppp_compressor *)lp->dw_abc_bsd_compressor;

		if(*p == DWBSD_PKT_SWITCH) {

			if(skb->len == DWBSD_PKT_FIRST_LEN) {

				if((lp->dw_abc_remote_version = p[1]) < 0x2) {

					printk(KERN_INFO 
						"%s: I can't really talk with remote version 0x%x\n"
						"Please upgrade remote or disable rawip-compression\n",
						lp->name,
						p[1]);
				}

				lp->dw_abc_if_flags |= ISDN_DW_ABC_IFFLAG_BSDAKTIV;
				dev_kfree_skb(skb);

				if(cp && lp->dw_abc_bsd_stat_tx) 
					(void)(*cp->reset)(lp->dw_abc_bsd_stat_tx,0,0,NULL,0,NULL);

				if(dev->net_verbose > 2)
					printk(KERN_INFO 
						"%s: receive comm-header rem-version 0x%02x\n",
						lp->name,
						lp->dw_abc_remote_version);

				return(NULL);
			}

		} else if(*p == DWBSD_PKT_BSD && lp->dw_abc_bsd_stat_rx != NULL && cp) {

			struct sk_buff *nskb = NULL;

			if(test_and_set_bit(ISDN_DW_ABC_BITLOCK_RECEIVE,
				&lp->dw_abc_bitlocks)) {

				printk(KERN_INFO "%s: bsd-decomp called recursivly\n",lp->name);
				dev_kfree_skb(skb);
				dwabc_bsd_first_gen(lp);
				return(NULL);
			} 
			
			nskb = (struct sk_buff *)
				dev_alloc_skb(2048 + ndev->hard_header_len);

			if(nskb != NULL) {

				int l = 0;
				u_short sqnr;

				skb_reserve(nskb,ndev->hard_header_len);
				skb_pull(skb, 1);
				sqnr = ((*(u_char *)skb->data) << 8) | ((u_char)skb->data[1]);

				if(!sqnr && cp && lp->dw_abc_bsd_stat_rx)
					(void)(*cp->reset)(lp->dw_abc_bsd_stat_rx,0,0,NULL,0,NULL);

				if((l = (*cp->decompress)
					(lp->dw_abc_bsd_stat_rx,skb,nskb,NULL)) < 1 || l>8000) {

					printk(KERN_INFO "%s: abc-decomp failed\n",lp->name);
					dev_kfree_skb(nskb);
					dev_kfree_skb(skb);
					nskb = NULL;
					dwabc_bsd_first_gen(lp);

				} else {

					if (nskb->data[0] & 0x1)
						skb_pull(nskb, 1);   /* protocol ID is only 8 bit */
					else
						skb_pull(nskb, 2);

					nskb->dev = skb->dev;
					nskb->pkt_type = skb->pkt_type;
					nskb->mac.raw = nskb->data;
					dev_kfree_skb(skb);
				}

			} else {

				printk(KERN_INFO "%s: PANIC abc-decomp no memory\n",lp->name);
				dev_kfree_skb(skb);
				dwabc_bsd_first_gen(lp);
			}

			clear_bit(ISDN_DW_ABC_BITLOCK_RECEIVE,&lp->dw_abc_bitlocks);
			r = nskb;
		}
	}

	return(r);
}

#else
int dwabc_bsd_init(isdn_net_local *lp) { return(1); }
void dwabc_bsd_free(isdn_net_local *lp) { return; }
void dwabc_bsd_first_gen(isdn_net_local *lp) { return ; }

struct sk_buff *dwabc_bsd_compress(isdn_net_local *lp,struct sk_buff *skb,struct net_device *ndev)
{ return(skb); }

struct sk_buff *dwabc_bsd_rx_pkt(isdn_net_local *lp,struct sk_buff *skb,struct net_device *ndev)
{ return(skb); }
#endif
#endif
