#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/atomic.h>
#include <linux/init.h>
#include <linux/proc_fs.h>

#include <linux/ip.h>
#include <linux/ipv6.h>

#include "ndpi_config.h"
#undef HAVE_HYPERSCAN
#include "ndpi_main.h"

#include "ndpi_main_common.h"
#include "ndpi_strcol.h"
#include "ndpi_main_netfilter.h"
#include "ndpi_proc_generic.h"
#include "ndpi_proc_parsers.h"
#include "ndpi_proc_hostdef.h"

int n_hostdef_proc_open(struct inode *inode, struct file *file)
{
        struct ndpi_net *n = PDE_DATA(file_inode(file));
	str_collect_t *ph;
	char *host;
	AC_PATTERN_t ac_pattern;
	AC_ERROR_t r;
	int np,nh,ret = 0;

	mutex_lock(&n->host_lock);
	n->host_ac = NULL;
	n->hosts_tmp = NULL;

do {
	if((file->f_mode & (FMODE_READ|FMODE_WRITE)) == FMODE_READ)
		break;

	n->hosts_tmp = str_collect_clone(n->hosts);
	if(!n->hosts_tmp) {
		return ENOMEM; break;
	}

	n->host_ac = ndpi_init_automa();

	if(!n->host_ac) {
		str_hosts_done(n->hosts_tmp);
		n->hosts_tmp = NULL;
		ret = ENOMEM; break;
	}
	if(ndpi_log_debug > 1)
		pr_info("host_open:%s %px new\n",n->ns_name,n->host_ac);

	n->host_error = 0;

	for(np = 0; np < NDPI_NUM_BITS; np++) {
		ph = n->hosts_tmp->p[np];
		if(ph) {
			nh = 0;
			for(nh = 0 ; nh < ph->last && ph->s[nh] ;
					nh += (uint8_t)ph->s[nh] + 2) {
				host = &ph->s[nh+1];
				ac_pattern.astring = host;
				ac_pattern.length = strlen(host);
				ac_pattern.rep.number = np;
				r = ac_automata_add(n->host_ac, &ac_pattern);
				if(r != ACERR_SUCCESS) {
					pr_err("%s:%s host add '%s' : %s : skipped\n",
						__func__,n->ns_name,host,acerr2txt(r));
				}
			}
		}
	}
} while(0);

	if(ndpi_log_debug > 1)
		pr_info("host_open:%s host_ac %px old %px %s\n",
				n->ns_name,(void *)n->host_ac,
				ndpi_automa_host(n->ndpi_struct),
				ret ? "ERROR":"OK");
	if(ret)
		mutex_unlock(&n->host_lock);

        return ret;
}

ssize_t n_hostdef_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos)
{
        struct ndpi_net *n = PDE_DATA(file_inode(file));
	char lbuf[256+32],*host;
	const char *t_proto;
	str_collect_t *ph;
	int i, l=0, p=0, bpos = 0, hl, pl;
	int hdp = 0, hdh = 0;
	loff_t cpos = 0;

	if(ndpi_log_debug > 1)
		pr_info("read: start ppos %lld\n",*ppos);

	while(hdp < NDPI_NUM_BITS ) {
		if(!cpos) {
			strcpy(lbuf,"#Proto:host\n");
			l = strlen(lbuf);
		}

		ph = n->hosts_tmp ? n->hosts_tmp->p[hdp]:
				    n->hosts->p[hdp];
		host = NULL;
		if(ph && ph->last && hdh < ph->last ) {
			t_proto = ndpi_get_proto_by_id(n->ndpi_struct,hdp);
			pl = strlen(t_proto);
			i = 0; p = 0;
			for( ; (uint8_t)ph->s[hdh] ;
					hdh += (uint8_t)ph->s[hdh] + 2) {
				host = &ph->s[hdh+1];
				hl = strlen(host);

				if(i && l - p + hl > 80) break;

				if(hl + 1 + (!i ? pl:0) + l + 5 > sizeof(lbuf)) {
					if(hl + pl + 3 > sizeof(lbuf)) {
						pr_err("ndpi: lbuf too small\n");
						continue;
					}
					if(ndpi_log_debug > 1) 
						pr_info("read:3 lbuff full\n");
					break;
				}
				if(!i) { // start line from protocol name
				    if(p) lbuf[l++] = '\n';
				    strcpy(&lbuf[l],t_proto);
				    l += pl;
				    lbuf[l] = '\0';
				}
				lbuf[l++] = i ? ',':':';
				strcpy(&lbuf[l],host);
				l += hl;
				lbuf[l] = '\0';

				i++;
			}
			lbuf[l++] = '\n'; lbuf[l] = '\0';

			if(hdh == ph->last) // last hostdef for current protocol
				host = NULL;
			if(ndpi_log_debug > 1) 
				pr_info("read:4 lbuf:%d '%s'\n",l,lbuf);

			if(cpos + l <= *ppos) {
				cpos += l;
			} else {
				if(!count) {
					if(ndpi_log_debug > 1) 
						pr_info("read:6 buf full, bpos %d\n",bpos);
					return bpos;
				}
				p = 0;
				// ppos: buf + count, cpos: lbuf + l
				if(cpos < *ppos) {
					p = *ppos - cpos;
					l -= p;
				}
				if( l > count) l = count;
				if (!(ACCESS_OK(VERIFY_WRITE, buf+bpos, l) &&
					!__copy_to_user(buf+bpos, lbuf+p, l))) return -EFAULT;
				if(ndpi_log_debug > 1) 
					pr_info("read:5 copy bpos %d p %d l %d\n",bpos,p,l);
				(*ppos) += l;
				bpos  += l;
				cpos  += l+p;
				count -= l;
				if(!count) {
					if(ndpi_log_debug > 1) 
						pr_info("read:6 buf full, bpos %d\n",bpos);
					return bpos;
				}
			}
			l = 0;
		}

		if(!host) {
			hdp++;
			hdh = 0;
			continue;
		}
		if(ndpi_log_debug > 1) 
			pr_info("read:7 next\n");
	}
	if(ndpi_log_debug > 1) 
		pr_info("read:8 return bpos %d\n",bpos);
	return bpos;
}

int n_hostdef_proc_close(struct inode *inode, struct file *file)
{
        struct ndpi_net *n = PDE_DATA(file_inode(file));
	ndpi_mod_str_t *nstr = n->ndpi_struct;

	generic_proc_close(n,parse_ndpi_hostdef,W_BUF_HOST);

	if(n->host_ac) { // open for write
		if(!n->host_error) {

			ac_automata_finalize((AC_AUTOMATA_t*)n->host_ac);

			spin_lock_bh(&nstr->host_automa_lock);
			XCHGP(nstr->host_automa.ac_automa,n->host_ac);
			spin_unlock_bh(&nstr->host_automa_lock);

			XCHGP(n->hosts,n->hosts_tmp);

		} else {
			pr_err("xt_ndpi:%s Can't update host_proto with errors\n",n->ns_name);
		}

		if(ndpi_log_debug > 1)
			pr_info("host_open:%s release host_ac %px\n",n->ns_name,n->host_ac);

		ac_automata_release((AC_AUTOMATA_t*)n->host_ac);
		n->host_ac = NULL;
		str_hosts_done(n->hosts_tmp);
		n->hosts_tmp = NULL;
	}

	mutex_unlock(&n->host_lock);
        return 0;
}

ssize_t
n_hostdef_proc_write(struct file *file, const char __user *buffer,
                     size_t length, loff_t *loff)
{
	return generic_proc_write(PDE_DATA(file_inode(file)), buffer, length, loff,
			parse_ndpi_hostdef, 4060, W_BUF_HOST);
}

