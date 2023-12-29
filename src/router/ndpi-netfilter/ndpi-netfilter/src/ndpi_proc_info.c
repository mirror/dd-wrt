#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>

#include <linux/ip.h>
#include <linux/ipv6.h>

#include "ndpi_config.h"
#undef HAVE_HYPERSCAN
#include "ndpi_main.h"
#include "ndpi_private.h"

#include "ndpi_strcol.h"
#include "ndpi_main_common.h"
#include "ndpi_main_netfilter.h"
#include "ndpi_proc_parsers.h"
#include "ndpi_proc_generic.h"
#include "ndpi_proc_info.h"

ssize_t _ninfo_proc_read(struct ndpi_net *n, char __user *buf,
                              size_t count, loff_t *ppos,int family)
{
        struct ndpi_detection_module_struct *ndpi_struct = n->ndpi_struct;
	struct hash_ip4p_table *ht,*ht4 = ndpi_struct->bt_ht;
#ifdef NDPI_DETECTION_SUPPORT_IPV6
	struct hash_ip4p_table *ht6 = ndpi_struct->bt6_ht;
#endif
	char lbuf[128];
	struct hash_ip4p *t;
	size_t p;
	int l;
	ht = 
#ifdef NDPI_DETECTION_SUPPORT_IPV6
		family == AF_INET6 ? ht6:
#endif
		ht4;
	if(!ht) {
	    if(!*ppos) {
	        l =  snprintf(lbuf,sizeof(lbuf)-1, "hash disabled\n");
		if (!(ACCESS_OK(VERIFY_WRITE, buf, l) &&
				! __copy_to_user(buf, lbuf, l))) return -EFAULT;
		(*ppos)++;
		return l;
	    }
	    return 0;
	}
	if(n->n_hash < 0 || n->n_hash >= ht->size-1) {
	    int tmin,tmax,i;

	    if(!*ppos) {
		tmin = 0x7fffffff;
		tmax = 0;
		t = &ht->tbl[0];

		for(i = ht->size-1; i >= 0 ;i--,t++) {
			if(t->len > 0 && t->len < tmin) tmin = t->len;
			if(t->len > tmax) tmax = t->len;
		}
		if(!atomic_read(&ht->count)) tmin = 0;
	        l =  snprintf(lbuf,sizeof(lbuf)-1,
			"hash_size %lu hash timeout %lus count %u min %d max %d gc %d\n",
				(family == AF_INET6 ? bt6_hash_size:bt_hash_size)*1024,
				bt_hash_tmo, atomic_read(&ht->count),tmin,tmax,n->gc_count );

		if (!(ACCESS_OK(VERIFY_WRITE, buf, l) &&
				! __copy_to_user(buf, lbuf, l))) return -EFAULT;
		(*ppos)++;
		return l;
	    }
	    /* ppos > 0 */
#define BSS1 144
#define BSS2 12
	    if(*ppos * BSS1 >= (family == AF_INET6 ? bt6_hash_size:bt_hash_size)*1024) return 0;

	    t = &ht->tbl[(*ppos-1)*BSS1];
	    p=0;
	    for(i=0; i < BSS1;i++,t++) {
		if(!(i % BSS2)) {
		        l = snprintf(lbuf,sizeof(lbuf)-1, "%d:\t",(int)(i+(*ppos-1)*BSS1));
			if (!(ACCESS_OK(VERIFY_WRITE, buf+p, l) && !__copy_to_user(buf+p, lbuf, l)))
				return -EFAULT;
			p += l;
		}
	        l = snprintf(lbuf,sizeof(lbuf)-1, "%5zu%c",
				t->len, (i % BSS2) == (BSS2-1) ? '\n':' ');
		
		if (!(ACCESS_OK(VERIFY_WRITE, buf+p, l) &&
				!__copy_to_user(buf+p, lbuf, l)))
			return -EFAULT;
		p += l;
	    }
	    (*ppos)++;
	    return p;
	}
	t = &ht->tbl[n->n_hash];
	if(!*ppos) {
	        l =  snprintf(lbuf,sizeof(lbuf)-1, "index %d len %zu\n",
				n->n_hash,t->len);
		if (!(ACCESS_OK(VERIFY_WRITE, buf, l) &&
				!__copy_to_user(buf, lbuf, l))) return -EFAULT;
		(*ppos)++;
		return l;
	}
	if(*ppos > 1) return 0;
	p = 0;
	spin_lock_bh(&t->lock);
	do {
		struct hash_ip4p_node *x = t->top;
	 	time64_t tm;

	        tm=ktime_get_real_seconds();
		while(x && p < count - 128) {
		        l =  inet_ntop_port(family,&x->ip,x->port,lbuf,sizeof(lbuf)-2);
			l += snprintf(&lbuf[l],sizeof(lbuf)-l-1, " %d %x %u\n",
				(int)(tm - x->lchg),x->flag,x->count);

			if (!(ACCESS_OK(VERIFY_WRITE, buf+p, l) &&
				!__copy_to_user(buf+p, lbuf, l))) return -EFAULT;
			p += l;
			x = x->next;
		}
	} while(0);
	spin_unlock_bh(&t->lock);
	(*ppos)++;
	return p;
}


ssize_t ninfo_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos)
{
return _ninfo_proc_read(pde_data(file_inode(file)),buf,count,ppos,AF_INET);
}

#ifdef NDPI_DETECTION_SUPPORT_IPV6
ssize_t ninfo6_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos)
{
return _ninfo_proc_read(pde_data(file_inode(file)),buf,count,ppos,AF_INET6);
}
#endif

ssize_t ninfo_proc_write(struct file *file, const char __user *buffer,
                     size_t length, loff_t *loff)
{
        struct ndpi_net *n = pde_data(file_inode(file));
	char buf[32];
	int idx;

        if (length > 0) {
		memset(buf,0,sizeof(buf));
		if (!(ACCESS_OK(VERIFY_READ, buffer, length) && 
			!__copy_from_user(&buf[0], buffer, min(length,sizeof(buf)-1))))
			        return -EFAULT;
		if(sscanf(buf,"%d",&idx) != 1) return -EINVAL;
		n->n_hash = idx;
        }
        return length;
}

#ifdef BT_ANNOUNCE
ssize_t nann_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos)
{
        struct ndpi_net *n = pde_data(file_inode(file));
        struct ndpi_detection_module_struct *ndpi_struct = n->ndpi_struct;
	struct bt_announce *b = ndpi_struct->bt_ann;
	int  bt_len = ndpi_struct->bt_ann_len;
	char lbuf[512],ipbuf[64];
	int i,l,p;

	for(i = 0,p = 0; i < bt_len; i++,b++) {
		if(!b->time) break;

		if(i < *ppos ) continue;
		if(!b->ip[0] && !b->ip[1] && b->ip[2] == 0xfffffffful)
			inet_ntop_port(AF_INET,&b->ip[3],b->port,ipbuf,sizeof(ipbuf));
		    else
			inet_ntop_port(AF_INET6,&b->ip,b->port,ipbuf,sizeof(ipbuf));
	        l =  snprintf(lbuf,sizeof(lbuf)-1, "%08x%08x%08x%08x%08x %s %u '%.*s'\n",
				htonl(b->hash[0]),htonl(b->hash[1]),
				htonl(b->hash[2]),htonl(b->hash[3]),htonl(b->hash[4]),
				ipbuf,b->time,b->name_len,b->name);

		if(count < l) break;

		if (!(ACCESS_OK(VERIFY_WRITE, buf+p, l) &&
				!__copy_to_user(buf+p, lbuf, l))) return -EFAULT;
		p += l;
		count -= l;
		(*ppos)++;
	}
	return p;
}
#endif

ssize_t nproto_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos)
{
        struct ndpi_net *n = pde_data(file_inode(file));
	char lbuf[128];
	char c_buf[32];
	int i,l,p,ro;
	loff_t i_pos = 0;

	for(i = 0,p = 0; i < NDPI_NUM_BITS; i++) {
		const char *t_proto = ndpi_get_proto_by_id(n->ndpi_struct,i);
		if(!t_proto) {
			snprintf(c_buf,sizeof(c_buf)-1,"custom%d",i);
		} else {
			char *cb = c_buf;
			l = sizeof(c_buf)-1;
			for(;*t_proto && l > 0; t_proto++,l--) {
				char c = *t_proto;
				if(c != '_' && c & 0x40) c |= 0x20;
				*cb++ = c;
			}
			*cb = '\0';
		}

		l = i ? 0: snprintf(lbuf,sizeof(lbuf),
				"#id     mark ~mask     name   # count #version %s\n",
				NDPI_GIT_RELEASE);
		if(!n->mark[i].mark && !n->mark[i].mask)
		    l += snprintf(&lbuf[l],sizeof(lbuf)-l,"%02x  %17s %-16s # %lld \n",
				i,"disabled",c_buf,
				(long long int)atomic64_read(&n->protocols_cnt[i]));
		else
		    l += snprintf(&lbuf[l],sizeof(lbuf)-l,"%02x  %8x/%08x %-16s # %lld debug=%d \n",
				i,n->mark[i].mark,n->mark[i].mask,c_buf,
				(long long int)atomic64_read(&n->protocols_cnt[i]),
#ifdef NDPI_ENABLE_DEBUG_MESSAGES
					n->debug_level[i]
#else
					0
#endif
					);

		if(i_pos + l <= *ppos ) {
			i_pos += l;
			continue;
		}
		if(!count) break;
		ro = 0;
		if(i_pos < *ppos) {
			ro = *ppos - i_pos;
			l -= ro;
		}
		if(count < l) l = count;

		if (!(ACCESS_OK(VERIFY_WRITE, buf+p, l) &&
				!__copy_to_user(buf+p, &lbuf[ro], l))) return -EFAULT;
		p += l;
		count -= l;
		(*ppos) += l;
		i_pos += l + ro;
		if(!count) break;
	}
	return p;
}

int nproto_proc_close(struct inode *inode, struct file *file)
{
        struct ndpi_net *n = pde_data(file_inode(file));
	generic_proc_close(n,parse_ndpi_proto,W_BUF_PROTO);
        return 0;
}

ssize_t
nproto_proc_write(struct file *file, const char __user *buffer,
                     size_t length, loff_t *loff)
{
	return generic_proc_write(pde_data(file_inode(file)), buffer, length, loff,
			parse_ndpi_proto, 256, W_BUF_PROTO);
}


ssize_t ndebug_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos)
{
	char lbuf[1024];
	int l,p,ro;
	loff_t i_pos = 0;

	p = 0;
	memset(lbuf,0,sizeof(lbuf));
#ifdef NDPI_ENABLE_DEBUG_MESSAGES
	l = dbg_ipt_opt(lbuf,sizeof(lbuf));
#else
	l = snprintf(lbuf,sizeof(lbuf)-1,"Debug is not enabled\n");
#endif

	if(i_pos + l <= *ppos )	return 0; // EOF

	ro = 0;
	if(i_pos < *ppos) {
		ro = *ppos - i_pos;
		l -= ro;
	}
	if(count < l) l = count;
	if(!count) return 0;

	if (!(ACCESS_OK(VERIFY_WRITE, buf+p, l) &&
			!__copy_to_user(buf+p, &lbuf[ro], l))) return -EFAULT;
	(*ppos) += l;
	p += l;
	return p;
}
int ndebug_proc_close(struct inode *inode, struct file *file)
{
        struct ndpi_net *n = pde_data(file_inode(file));
	generic_proc_close(n,parse_ndpi_debug,W_BUF_DEBUG);
        return 0;
}

ssize_t
ndebug_proc_write(struct file *file, const char __user *buffer,
                     size_t length, loff_t *loff)
{
	return generic_proc_write(pde_data(file_inode(file)), buffer, length, loff,
			parse_ndpi_debug, 256, W_BUF_DEBUG);
}


int risk_names(struct ndpi_net *n, char *lbuf,size_t count) {
	ndpi_risk_enum r;
	char rbuf[128];
	size_t l = 0, i;

	for(r = NDPI_NO_RISK; r < NDPI_MAX_RISK; r++) {
		const char *ra = ndpi_risk2str(r);
		if(ra[0] >= '0' && ra[0] <= '9') continue; // unknown risk
		i = snprintf(rbuf,sizeof(rbuf)-1,"%d %c %s\n",
				(int)r, n->risk_mask & (1ULL << r) ? 'a':'d',
				ra);
		if(lbuf && l + i > count) return -1;
		if(lbuf)
			strncpy(&lbuf[l],rbuf,i);
		l += i;
	}
	return l;
}

int nrisk_proc_open(struct inode *inode, struct file *file) {
        struct ndpi_net *n = pde_data(file_inode(file));
	char *tmp;

	if(READ_ONCE(n->risk_names)) return -EBUSY;

	tmp = kmalloc(n->risk_names_len+4,GFP_KERNEL);
	if(tmp) {
		risk_names(n,tmp,n->risk_names_len+1);
		WRITE_ONCE(n->risk_names, tmp);
	} else
		return -ENOMEM;
	return 0;
}

ssize_t nrisk_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos)
{
        struct ndpi_net *n = pde_data(file_inode(file));

	int l,ro;

	if(!n->risk_names_len || !n->risk_names) return 0;
	l = n->risk_names_len;

	if( l <= *ppos ) return 0; // EOF

	ro = 0;
	if(*ppos > 0) {
		ro = *ppos;
		l -= ro;
	}
	if(count < l) l = count;
	if(!count) return 0;

	if (!(ACCESS_OK(VERIFY_WRITE, buf, l) &&
			!__copy_to_user(buf, n->risk_names+ro, l))) return -EFAULT;
	(*ppos) += l;
	return l;
}

ssize_t
nrisk_proc_write(struct file *file, const char __user *buffer,
                     size_t length, loff_t *loff)
{
	return generic_proc_write(pde_data(file_inode(file)), buffer, length, loff,
			parse_ndpi_risk, 256, W_BUF_RISK);
}

int nrisk_proc_close(struct inode *inode, struct file *file)
{
        struct ndpi_net *n = pde_data(file_inode(file));
	char *tmp = NULL;
	generic_proc_close(n,parse_ndpi_risk,W_BUF_RISK);
	XCHGP(n->risk_names,tmp);
	if(tmp) kfree(tmp);
        return 0;
}


