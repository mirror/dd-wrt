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
#include "ndpi_private.h"

#include "ndpi_main_common.h"
#include "ndpi_strcol.h"
#include "ndpi_main_netfilter.h"
#include "ndpi_proc_generic.h"
#include "ndpi_proc_parsers.h"
#include "ndpi_proc_hostdef.h"

int n_hostdef_proc_open(struct inode *inode, struct file *file)
{
        struct ndpi_net *n = pde_data(file_inode(file));
	int ret = 0;

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
	ac_automata_feature(n->host_ac,AC_FEATURE_LC);
	ac_automata_name(n->host_ac,"host",AC_FEATURE_DEBUG);

	if(_DBG_TRACE_SPROC_H)
		pr_info("host_open:%s %px new\n",n->ns_name,n->host_ac);

	n->host_error = 0;
	n->host_upd   = 0;

} while(0);

	if(_DBG_TRACE_SPROC_H)
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
        struct ndpi_net *n = pde_data(file_inode(file));
	char lbuf[256+32],unkproto[16],*host;
	const char *t_proto;
	str_collect_t *ph;
	int i, l=0, p=0, bpos = 0, hl, pl;
	int hdp = 0, hdh = 0;
	loff_t cpos = 0;

	if(_DBG_TRACE_GPROC_H)
		pr_info("read: start ppos %lld\n",*ppos);

	l = snprintf(lbuf,sizeof(lbuf),"#Proto:host\n");

	while(hdp < NDPI_NUM_BITS ) {
		ph = n->hosts_tmp ? n->hosts_tmp->p[hdp]:
				    n->hosts->p[hdp];
		host = NULL;
		if(ph && ph->last && hdh < ph->last ) {
			t_proto = ndpi_get_proto_by_id(n->ndpi_struct,hdp);
			if(!t_proto) {
				snprintf(unkproto,sizeof(unkproto)-1,"custom%.5d",hdp);
				t_proto = unkproto;
			}
			pl = strlen(t_proto);
			for(i = 0, p = 0; (hl = (uint8_t)ph->s[hdh]) != 0; hdh += hl + 2) {
				host = &ph->s[hdh+1];

				if(i && l - p + hl > 132) break;

				if(hl + 1 + (!i ? pl:0) + l + 5 > sizeof(lbuf)) {
					if(hl + pl + 3 > sizeof(lbuf)) {
						pr_err("ndpi: lbuf too small\n");
						continue;
					}
					if(_DBG_TRACE_GPROC_H2) 
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
			if(_DBG_TRACE_GPROC_H2) 
				pr_info("read:4 lbuf:%d '%s'\n",l,lbuf);

			if(cpos + l <= *ppos) {
				cpos += l;
			} else {
				if(!count) {
					if(_DBG_TRACE_GPROC_H2) 
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
				if(_DBG_TRACE_GPROC_H2) 
					pr_info("read:5 copy bpos %d p %d l %d\n",bpos,p,l);
				(*ppos) += l;
				bpos  += l;
				cpos  += l+p;
				count -= l;
				if(!count) {
					if(_DBG_TRACE_GPROC_H2) 
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
		if(_DBG_TRACE_GPROC_H2) 
			pr_info("read:7 next\n");
	}
	if(_DBG_TRACE_GPROC_H2) 
		pr_info("read:8 return bpos %d\n",bpos);
	return bpos;
}

int n_hostdef_proc_close(struct inode *inode, struct file *file)
{
        struct ndpi_net *n = pde_data(file_inode(file));
	ndpi_mod_str_t *nstr = n->ndpi_struct;

	generic_proc_close(n,parse_ndpi_hostdef,W_BUF_HOST);

	if(n->host_ac) { // open for write
	    if(n->host_upd) {
		if(!n->host_error && str_coll_to_automata(nstr,n->host_ac,n->hosts_tmp))
			n->host_error++;
		if(!n->host_error) {
			spin_lock_bh(&nstr->host_automa_lock);
			XCHGP(nstr->host_automa.ac_automa,n->host_ac);
			spin_unlock_bh(&nstr->host_automa_lock);

			XCHGP(n->hosts,n->hosts_tmp);

		} else {
			pr_err("xt_ndpi:%s Can't update host_proto with errors\n",n->ns_name);
		}

		if(_DBG_TRACE_GPROC_H)
			pr_info("host_open:%s release host_ac %px\n",n->ns_name,n->host_ac);
	    }
	    ac_automata_release((AC_AUTOMATA_t*)n->host_ac,1);
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
	return generic_proc_write(pde_data(file_inode(file)), buffer, length, loff,
			parse_ndpi_hostdef, 4060, W_BUF_HOST);
}

