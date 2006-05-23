/*
 * ipt_osf.c
 *
 * Copyright (c) 2003 Evgeniy Polyakov <johnpol@2ka.mipt.ru>
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/*
 * OS fingerprint matching module.
 * It simply compares various parameters from SYN packet with
 * some hardcoded ones.
 *
 * Original table was created by Michal Zalewski <lcamtuf@coredump.cx>
 * for his p0f.
 */

#include <linux/config.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/string.h>
#include <linux/smp.h>
#include <linux/module.h>
#include <linux/skbuff.h>
#include <linux/file.h>
#include <linux/ip.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/ctype.h>
#include <linux/list.h>
#include <linux/if.h>

#include <net/sock.h>
#include <net/ip.h>

#include <linux/netfilter_ipv4/ip_tables.h>

#include <linux/netfilter_ipv4/ipt_osf.h>

#define OSF_DEBUG

#ifdef OSF_DEBUG
#define log(x...) 		printk(KERN_INFO "ipt_osf: " x)
#define loga(x...) 		printk(x)
#else
#define log(x...) 		do {} while(0)
#define loga(x...) 		do {} while(0)
#endif

#define FMATCH_WRONG		0
#define FMATCH_OK		1
#define FMATCH_OPT_WRONG	2

#define OPTDEL			','
#define OSFPDEL 		':'
#define MAXOPTSTRLEN		128
#define OSFFLUSH		"FLUSH"

static rwlock_t osf_lock = RW_LOCK_UNLOCKED;
static spinlock_t ipt_osf_netlink_lock = SPIN_LOCK_UNLOCKED;
static struct list_head	finger_list;	
static int match(const struct sk_buff *, const struct net_device *, const struct net_device *,
		      const void *, int, 
		      const void *, u_int16_t, 
		      int *);
static int checkentry(const char *, const struct ipt_ip *, void *,
		           unsigned int, unsigned int);

static unsigned long seq, ipt_osf_groups = 1;
static struct sock *nts;

static struct ipt_match osf_match = 
{ 
	{ NULL, NULL }, 
	"osf", 
	&match, 
	&checkentry, 
	NULL, 
	THIS_MODULE 
};

static void ipt_osf_nlsend(struct osf_finger *f, const struct sk_buff *sk)
{
	unsigned int size;
	struct sk_buff *skb;
	struct ipt_osf_nlmsg *data;
	struct nlmsghdr *nlh;

	size = NLMSG_SPACE(sizeof(struct ipt_osf_nlmsg));

	skb = alloc_skb(size, GFP_ATOMIC);
	if (!skb)
	{
		log("skb_alloc() failed.\n");
		return;
	}
	
	nlh = NLMSG_PUT(skb, 0, seq++, NLMSG_DONE, size - sizeof(*nlh));
	
	data = (struct ipt_osf_nlmsg *)NLMSG_DATA(nlh);

	memcpy(&data->f, f, sizeof(struct osf_finger));
	memcpy(&data->ip, sk->nh.iph, sizeof(struct iphdr));
	memcpy(&data->tcp, (struct tcphdr *)((u_int32_t *)sk->nh.iph + sk->nh.iph->ihl), sizeof(struct tcphdr));

	NETLINK_CB(skb).dst_groups = ipt_osf_groups;
	netlink_broadcast(nts, skb, 0, ipt_osf_groups, GFP_ATOMIC);

nlmsg_failure:
	return;
}

static inline int smart_dec(const struct sk_buff *skb, unsigned long flags, unsigned char f_ttl)
{
	struct iphdr *ip = skb->nh.iph;

	if (flags & IPT_OSF_SMART)
	{
		struct in_device *in_dev = in_dev_get(skb->dev);

		for_ifa(in_dev)
		{
			if (inet_ifa_match(ip->saddr, ifa))
			{
				in_dev_put(in_dev);
				return (ip->ttl == f_ttl);
			}
		}
		endfor_ifa(in_dev);
		
		in_dev_put(in_dev);
		return (ip->ttl <= f_ttl);
	}
	else
		return (ip->ttl == f_ttl);
}

static int
match(const struct sk_buff *skb, const struct net_device *in, const struct net_device *out,
      const void *matchinfo, int offset,
      const void *hdr, u_int16_t datalen,
      int *hotdrop)
{
	struct ipt_osf_info *info = (struct ipt_osf_info *)matchinfo;
	struct iphdr *ip = skb->nh.iph;
	struct tcphdr *tcp;
	int fmatch = FMATCH_WRONG, fcount = 0;
	unsigned long totlen, optsize = 0, window;
	unsigned char df, *optp = NULL, *_optp = NULL;
	char check_WSS = 0;
	struct list_head *ent;
	struct osf_finger *f;

	if (!ip || !info)
		return 0;
				
	tcp = (struct tcphdr *)((u_int32_t *)ip + ip->ihl);

	if (!tcp->syn)
		return 0;
	
	totlen = ntohs(ip->tot_len);
	df = ((ntohs(ip->frag_off) & IP_DF)?1:0);
	window = ntohs(tcp->window);
	
	if (tcp->doff*4 > sizeof(struct tcphdr))
	{
		_optp = optp = (char *)(tcp+1);
		optsize = tcp->doff*4 - sizeof(struct tcphdr);
	}

	
	/* Actually we can create hash/table of all genres and search
	 * only in appropriate part, but here is initial variant,
	 * so will use slow path.
	 */
	read_lock(&osf_lock);
	list_for_each(ent, &finger_list)
	{
		f = list_entry(ent, struct osf_finger, flist);
	
		if (!(info->flags & IPT_OSF_LOG) && strcmp(info->genre, f->genre)) 
			continue;

		optp = _optp;
		fmatch = FMATCH_WRONG;

		if (totlen == f->ss && df == f->df && 
			smart_dec(skb, info->flags, f->ttl))
		{
			unsigned long foptsize;
			int optnum;
			unsigned short mss = 0;

			check_WSS = 0;

			switch (f->wss.wc)
			{
				case 0:	  check_WSS = 0; break;
				case 'S': check_WSS = 1; break;
				case 'T': check_WSS = 2; break;
				case '%': check_WSS = 3; break;
				default: log("Wrong fingerprint wss.wc=%d, %s - %s\n", 
							 f->wss.wc, f->genre, f->details);
					 check_WSS = 4;
					 break;
			}
			if (check_WSS == 4)
				continue;

			/* Check options */

			foptsize = 0;
			for (optnum=0; optnum<f->opt_num; ++optnum)
				foptsize += f->opt[optnum].length;

				
			if (foptsize > MAX_IPOPTLEN || optsize > MAX_IPOPTLEN || optsize != foptsize)
				continue;

			if (!optp)
			{
				fmatch = FMATCH_OK;
				loga("\tYEP : matching without options.\n");
				if ((info->flags & IPT_OSF_LOG) && 
					info->loglevel == IPT_OSF_LOGLEVEL_FIRST)
					break;
				else
					continue;
			}
			

			for (optnum=0; optnum<f->opt_num; ++optnum)
			{
				if (f->opt[optnum].kind == (*optp)) 
				{
					unsigned char len = f->opt[optnum].length;
					unsigned char *optend = optp + len;
					int loop_cont = 0;

					fmatch = FMATCH_OK;


					switch (*optp)
					{
						case OSFOPT_MSS:
							mss = ntohs(*(unsigned short *)(optp+2));
							break;
						case OSFOPT_TS:
							loop_cont = 1;
							break;
					}
					
					if (loop_cont)
					{
						optp = optend;
						continue;
					}
					
					if (len != 1)
					{
						/* Skip kind and length fields*/
						optp += 2; 

						if (f->opt[optnum].wc.val != 0)
						{
							unsigned long tmp = 0;
							
							/* Hmmm... It looks a bit ugly. :) */
							memcpy(&tmp, optp, 
								(len > sizeof(unsigned long)?
								 	sizeof(unsigned long):len));
							/* 2 + 2: optlen(2 bytes) + 
							 * 	kind(1 byte) + length(1 byte) */
							if (len == 4) 
								tmp = ntohs(tmp);
							else
								tmp = ntohl(tmp);

							if (f->opt[optnum].wc.wc == '%')
							{
								if ((tmp % f->opt[optnum].wc.val) != 0)
									fmatch = FMATCH_OPT_WRONG;
							}
							else if (tmp != f->opt[optnum].wc.val)
								fmatch = FMATCH_OPT_WRONG;
						}
					}

					optp = optend;
				}
				else
					fmatch = FMATCH_OPT_WRONG;

				if (fmatch != FMATCH_OK)
					break;
			}

			if (fmatch != FMATCH_OPT_WRONG)
			{
				fmatch = FMATCH_WRONG;

				switch (check_WSS)
				{
					case 0:
						if (f->wss.val == 0 || window == f->wss.val)
							fmatch = FMATCH_OK;
						break;
					case 1: /* MSS */
/* Lurked in OpenBSD */
#define SMART_MSS	1460
						if (window == f->wss.val*mss || 
							window == f->wss.val*SMART_MSS)
							fmatch = FMATCH_OK;
						break;
					case 2: /* MTU */
						if (window == f->wss.val*(mss+40) ||
							window == f->wss.val*(SMART_MSS+40))
							fmatch = FMATCH_OK;
						break;
					case 3: /* MOD */
						if ((window % f->wss.val) == 0)
							fmatch = FMATCH_OK;
						break;
				}
			}
					

			if (fmatch == FMATCH_OK)
			{
				fcount++;
				log("%s [%s:%s:%s] : %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u hops=%d\n", 
					f->genre, f->version,
					f->subtype, f->details,
					NIPQUAD(ip->saddr), ntohs(tcp->source),
					NIPQUAD(ip->daddr), ntohs(tcp->dest),
					f->ttl - ip->ttl);
				if (info->flags & IPT_OSF_NETLINK)
				{
					spin_lock_bh(&ipt_osf_netlink_lock);
					ipt_osf_nlsend(f, skb);
					spin_unlock_bh(&ipt_osf_netlink_lock);
				}
				if ((info->flags & IPT_OSF_LOG) && 
					info->loglevel == IPT_OSF_LOGLEVEL_FIRST)
					break;
			}
		}
	}
	if (!fcount && (info->flags & (IPT_OSF_LOG | IPT_OSF_NETLINK)))
	{
		unsigned char opt[4 * 15 - sizeof(struct tcphdr)];
		unsigned int i, optsize;
		struct osf_finger fg;

		memset(&fg, 0, sizeof(fg));

		if ((info->flags & IPT_OSF_LOG))
			log("Unknown: %lu:%d:%d:%lu:", window, ip->ttl, df, totlen);
		if (optp)
		{
			optsize = tcp->doff * 4 - sizeof(struct tcphdr);
			if (skb_copy_bits(skb, ip->ihl*4 + sizeof(struct tcphdr),
					  opt, optsize) < 0)
			{
				if (info->flags & IPT_OSF_LOG)
					loga("TRUNCATED");
				if (info->flags & IPT_OSF_NETLINK)
					strcpy(fg.details, "TRUNCATED");
			}
			else
			{
				for (i = 0; i < optsize; i++)
				{
					if (info->flags & IPT_OSF_LOG)
						loga("%02X", opt[i]);
				}
				if (info->flags & IPT_OSF_NETLINK)
					memcpy(fg.details, opt, MAXDETLEN);
			}
		}
		if ((info->flags & IPT_OSF_LOG))
			loga(" %u.%u.%u.%u:%u -> %u.%u.%u.%u:%u\n", 
				NIPQUAD(ip->saddr), ntohs(tcp->source),
				NIPQUAD(ip->daddr), ntohs(tcp->dest));
		
		if (info->flags & IPT_OSF_NETLINK)
		{
			fg.wss.val 	= window;
			fg.ttl		= ip->ttl;
			fg.df		= df;
			fg.ss		= totlen;
			strncpy(fg.genre, "Unknown", MAXGENRELEN);

			spin_lock_bh(&ipt_osf_netlink_lock);
			ipt_osf_nlsend(&fg, skb);
			spin_unlock_bh(&ipt_osf_netlink_lock);
		}
	}

	read_unlock(&osf_lock);

	return (fmatch == FMATCH_OK)?1:0;
}

static int
checkentry(const char *tablename,
           const struct ipt_ip *ip,
           void *matchinfo,
           unsigned int matchsize,
           unsigned int hook_mask)
{
       if (matchsize != IPT_ALIGN(sizeof(struct ipt_osf_info)))
               return 0;
       if (ip->proto != IPPROTO_TCP)
	       return 0;

       return 1;
}

static char * osf_strchr(char *ptr, char c)
{
	char *tmp;

	tmp = strchr(ptr, c);

	while (tmp && tmp+1 && isspace(*(tmp+1)))
		tmp++;

	return tmp;
}

static struct osf_finger * finger_alloc(void)
{
	struct osf_finger *f;

	f = kmalloc(sizeof(struct osf_finger), GFP_KERNEL);
	if (f)
		memset(f, 0, sizeof(struct osf_finger));
	
	return f;
}

static void finger_free(struct osf_finger *f)
{
	memset(f, 0, sizeof(struct osf_finger));
	kfree(f);
}


static void osf_parse_opt(struct osf_opt *opt, int *optnum, char *obuf, int olen)
{
	int i, op;
	char *ptr, wc;
	unsigned long val;

	ptr = &obuf[0];
	i = 0;
	while (ptr != NULL && i < olen)
	{
		val = 0;
		op = 0;
		wc = 0;
		switch (obuf[i])
		{
			case 'N': 
				op = OSFOPT_NOP;
				ptr = osf_strchr(&obuf[i], OPTDEL);
				if (ptr)
				{
					*ptr = '\0';
					ptr++;
					i += (int)(ptr-&obuf[i]);

				}
				else
					i++;
				break;
			case 'S': 
				op = OSFOPT_SACKP;
				ptr = osf_strchr(&obuf[i], OPTDEL);
				if (ptr)
				{
					*ptr = '\0';
					ptr++;
					i += (int)(ptr-&obuf[i]);

				}
				else
					i++;
				break;
			case 'T': 
				op = OSFOPT_TS;
				ptr = osf_strchr(&obuf[i], OPTDEL);
				if (ptr)
				{
					*ptr = '\0';
					ptr++;
					i += (int)(ptr-&obuf[i]);

				}
				else
					i++;
				break;
			case 'W': 
				op = OSFOPT_WSO;
				ptr = osf_strchr(&obuf[i], OPTDEL);
				if (ptr)
				{
					switch (obuf[i+1])
					{
						case '%':	wc = '%'; break;
						case 'S':	wc = 'S'; break;
						case 'T':	wc = 'T'; break;
						default:	wc = 0; break;
					}
					
					*ptr = '\0';
					ptr++;
					if (wc)
						val = simple_strtoul(&obuf[i+2], NULL, 10);
					else
						val = simple_strtoul(&obuf[i+1], NULL, 10);
					i += (int)(ptr-&obuf[i]);

				}
				else
					i++;
				break;
			case 'M': 
				op = OSFOPT_MSS;
				ptr = osf_strchr(&obuf[i], OPTDEL);
				if (ptr)
				{
					if (obuf[i+1] == '%')
						wc = '%';
					*ptr = '\0';
					ptr++;
					if (wc)
						val = simple_strtoul(&obuf[i+2], NULL, 10);
					else
						val = simple_strtoul(&obuf[i+1], NULL, 10);
					i += (int)(ptr-&obuf[i]);

				}
				else
					i++;
				break;
			case 'E': 
				op = OSFOPT_EOL;
				ptr = osf_strchr(&obuf[i], OPTDEL);
				if (ptr)
				{
					*ptr = '\0';
					ptr++;
					i += (int)(ptr-&obuf[i]);

				}
				else
					i++;
				break;
			default:
				ptr = osf_strchr(&obuf[i], OPTDEL);
				if (ptr)
				{
					ptr++;
					i += (int)(ptr-&obuf[i]);

				}
				else
					i++;
				break;
		}

		opt[*optnum].kind 	= IANA_opts[op].kind;
		opt[*optnum].length 	= IANA_opts[op].length;
		opt[*optnum].wc.wc 	= wc;
		opt[*optnum].wc.val	= val;

		(*optnum)++;
	}
}

static int osf_proc_read(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	struct list_head *ent;
	struct osf_finger *f = NULL;
	int i;
	
	*eof = 1;
	count = 0;

	read_lock_bh(&osf_lock);
	list_for_each(ent, &finger_list)
	{
		f = list_entry(ent, struct osf_finger, flist);

		log("%s [%s]", f->genre, f->details);
		
		count += sprintf(buf+count, "%s - %s[%s] : %s", 
					f->genre, f->version,
					f->subtype, f->details);
		
		if (f->opt_num)
		{
			loga(" OPT: ");
			//count += sprintf(buf+count, " OPT: ");
			for (i=0; i<f->opt_num; ++i)
			{
				//count += sprintf(buf+count, "%d.%c%lu; ", 
				//	f->opt[i].kind, (f->opt[i].wc.wc)?f->opt[i].wc.wc:' ', f->opt[i].wc.val);
				loga("%d.%c%lu; ", 
					f->opt[i].kind, (f->opt[i].wc.wc)?f->opt[i].wc.wc:' ', f->opt[i].wc.val);
			}
		}
		loga("\n");
		count += sprintf(buf+count, "\n");
	}
	read_unlock_bh(&osf_lock);

	return count;
}

static int osf_proc_write(struct file *file, const char *buffer, unsigned long count, void *data)
{
	int cnt;
	unsigned long i;
	char obuf[MAXOPTSTRLEN];
	struct osf_finger *finger;
	struct list_head *ent, *n;

	char *pbeg, *pend;

	if (count == strlen(OSFFLUSH) && !strncmp(buffer, OSFFLUSH, strlen(OSFFLUSH)))
	{
		int i = 0;
		write_lock_bh(&osf_lock);
		list_for_each_safe(ent, n, &finger_list)
		{
			i++;
			finger = list_entry(ent, struct osf_finger, flist);
			list_del(&finger->flist);
			finger_free(finger);
		}
		write_unlock_bh(&osf_lock);
	
		log("Flushed %d entries.\n", i);
		
		return count;
	}

	
	cnt = 0;
	for (i=0; i<count && buffer[i] != '\0'; ++i)
		if (buffer[i] == ':')
			cnt++;

	if (cnt != 8 || i != count)
	{
		log("Wrong input line cnt=%d[8], len=%lu[%lu]\n", 
			cnt, i, count);
		return count;
	}

	memset(obuf, 0, sizeof(obuf));
	
	finger = finger_alloc();
	if (!finger)
	{
		log("Failed to allocate new fingerprint entry.\n");
		return -ENOMEM;
	}

	pbeg = (char *)buffer;
	pend = osf_strchr(pbeg, OSFPDEL);
	if (pend)
	{
		*pend = '\0';
		if (pbeg[0] == 'S')
		{
			finger->wss.wc = 'S';
			if (pbeg[1] == '%')
				finger->wss.val = simple_strtoul(pbeg+2, NULL, 10);
			else if (pbeg[1] == '*')
				finger->wss.val = 0;
			else 
				finger->wss.val = simple_strtoul(pbeg+1, NULL, 10);
		}
		else if (pbeg[0] == 'T')
		{
			finger->wss.wc = 'T';
			if (pbeg[1] == '%')
				finger->wss.val = simple_strtoul(pbeg+2, NULL, 10);
			else if (pbeg[1] == '*')
				finger->wss.val = 0;
			else 
				finger->wss.val = simple_strtoul(pbeg+1, NULL, 10);
		}
		else if (pbeg[0] == '%')
		{
			finger->wss.wc = '%';
			finger->wss.val = simple_strtoul(pbeg+1, NULL, 10);
		}
		else if (isdigit(pbeg[0]))
		{
			finger->wss.wc = 0;
			finger->wss.val = simple_strtoul(pbeg, NULL, 10);
		}

		pbeg = pend+1;
	}
	pend = osf_strchr(pbeg, OSFPDEL);
	if (pend)
	{
		*pend = '\0';
		finger->ttl = simple_strtoul(pbeg, NULL, 10);
		pbeg = pend+1;
	}
	pend = osf_strchr(pbeg, OSFPDEL);
	if (pend)
	{
		*pend = '\0';
		finger->df = simple_strtoul(pbeg, NULL, 10);
		pbeg = pend+1;
	}
	pend = osf_strchr(pbeg, OSFPDEL);
	if (pend)
	{
		*pend = '\0';
		finger->ss = simple_strtoul(pbeg, NULL, 10);
		pbeg = pend+1;
	}

	pend = osf_strchr(pbeg, OSFPDEL);
	if (pend)
	{
		*pend = '\0';
		cnt = snprintf(obuf, sizeof(obuf), "%s", pbeg);
		pbeg = pend+1;
	}

	pend = osf_strchr(pbeg, OSFPDEL);
	if (pend)
	{
		*pend = '\0';
		if (pbeg[0] == '@' || pbeg[0] == '*')
			cnt = snprintf(finger->genre, sizeof(finger->genre), "%s", pbeg+1);
		else
			cnt = snprintf(finger->genre, sizeof(finger->genre), "%s", pbeg);
		pbeg = pend+1;
	}
	
	pend = osf_strchr(pbeg, OSFPDEL);
	if (pend)
	{
		*pend = '\0';
		cnt = snprintf(finger->version, sizeof(finger->version), "%s", pbeg);
		pbeg = pend+1;
	}
	
	pend = osf_strchr(pbeg, OSFPDEL);
	if (pend)
	{
		*pend = '\0';
		cnt = snprintf(finger->subtype, sizeof(finger->subtype), "%s", pbeg);
		pbeg = pend+1;
	}

	cnt = snprintf(finger->details, 
			((count - (pbeg - buffer)+1) > MAXDETLEN)?MAXDETLEN:(count - (pbeg - buffer)+1), 
			"%s", pbeg);
	
	log("%s - %s[%s] : %s\n", 
		finger->genre, finger->version,
		finger->subtype, finger->details);
	
	osf_parse_opt(finger->opt, &finger->opt_num, obuf, sizeof(obuf));
	

	write_lock_bh(&osf_lock);
	list_add_tail(&finger->flist, &finger_list);
	write_unlock_bh(&osf_lock);

	return count;
}

static int __init osf_init(void)
{
	int err;
	struct proc_dir_entry *p;

	log("Startng OS fingerprint matching module.\n");

	INIT_LIST_HEAD(&finger_list);
	
	err = ipt_register_match(&osf_match);
	if (err)
	{
		log("Failed to register OS fingerprint matching module.\n");
		return -ENXIO;
	}

	p = create_proc_entry("sys/net/ipv4/osf", S_IFREG | 0644, NULL);
	if (!p)
	{
		ipt_unregister_match(&osf_match);
		return -ENXIO;
	}

	p->write_proc = osf_proc_write;
	p->read_proc  = osf_proc_read;
	
	nts = netlink_kernel_create(NETLINK_NFLOG, NULL);
	if (!nts)
	{
		log("netlink_kernel_create() failed\n");
		remove_proc_entry("sys/net/ipv4/osf", NULL);
		ipt_unregister_match(&osf_match);
		return -ENOMEM;
	}

	return 0;
}

static void __exit osf_fini(void)
{
	struct list_head *ent, *n;
	struct osf_finger *f;
	
	remove_proc_entry("sys/net/ipv4/osf", NULL);
	ipt_unregister_match(&osf_match);
	if (nts && nts->socket)
		sock_release(nts->socket);

	list_for_each_safe(ent, n, &finger_list)
	{
		f = list_entry(ent, struct osf_finger, flist);
		list_del(&f->flist);
		finger_free(f);
	}
	
	log("OS fingerprint matching module finished.\n");
}

module_init(osf_init);
module_exit(osf_fini);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Evgeniy Polyakov <johnpol@2ka.mipt.ru>");
MODULE_DESCRIPTION("Passive OS fingerprint matching.");
