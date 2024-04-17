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
#include "ndpi_proc_generic.h"
#include "ndpi_proc_parsers.h"
#include "ndpi_proc_ipdef.h"

int n_ipdef_proc_open(struct inode *inode, struct file *file)
{
        return 0;
}

int n_ipdef_proc_close(struct inode *inode, struct file *file)
{
        struct ndpi_net *n = pde_data(file_inode(file));
	generic_proc_close(n,parse_ndpi_ipdef,W_BUF_IP);
        return 0;
}

int n_ip6def_proc_close(struct inode *inode, struct file *file)
{
        struct ndpi_net *n = pde_data(file_inode(file));
	generic_proc_close(n,parse_ndpi_ip6def,W_BUF_IP);
        return 0;
}

ssize_t
n_ipdef_proc_write(struct file *file, const char __user *buffer,
                     size_t length, loff_t *loff)
{
	return generic_proc_write(pde_data(file_inode(file)), buffer, length, loff,
			parse_ndpi_ipdef, 4060 , W_BUF_IP);
}

ssize_t
n_ip6def_proc_write(struct file *file, const char __user *buffer,
                     size_t length, loff_t *loff)
{
	return generic_proc_write(pde_data(file_inode(file)), buffer, length, loff,
			parse_ndpi_ip6def, 4060 , W_BUF_IP);
}

static ssize_t _n_ipdef_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos,
			      int family)
{
        struct ndpi_net *n = pde_data(file_inode(file));
	ndpi_prefix_t *px;
	ndpi_patricia_tree_t *pt;
	ndpi_patricia_node_t *Xstack[PATRICIA_MAXBITS+1], **Xsp, *node;
	char lbuf[512];
	char ibuf[64];
	int l,bp;
	loff_t cpos;

	cpos = 0; bp = 0;

	pt = family == AF_INET ? n->ndpi_struct->protocols_ptree:n->ndpi_struct->protocols_ptree6;

	Xsp = &Xstack[0];
	node = pt ? pt->head: NULL;
	while (node) {
	    if (node->prefix) {
		l = cpos ? 0: snprintf(lbuf,sizeof(lbuf),"#ip%s              proto\n",
			family == AF_INET ? "":"v6");
		
		px = node->prefix;
		{
		int k;
		inet_ntop(px->family,(void *)&px->add,ibuf,sizeof(ibuf)-1);
		k = strlen(ibuf);
		if((px->family == AF_INET  && px->bitlen < 32 ) ||
		   (px->family == AF_INET6 && px->bitlen < 128 ))
			snprintf(&ibuf[k],sizeof(ibuf)-k,"/%d",px->bitlen);
		}
		{
		uint16_t n_proto,no_dpi;
		n_proto = node->value.u.uv32.user_value & 0xffff;
		no_dpi = node->value.u.uv32.user_value & 0xff0000 ? 1:0;
		if(n_proto != NDPI_PROTOCOL_UNKNOWN)
		    l += snprintf(&lbuf[l],sizeof(lbuf)-l,"%-16s %s%s\n",ibuf,
			n_proto >= NDPI_NUM_BITS ?
				"unknown":ndpi_get_proto_by_id(n->ndpi_struct,n_proto),
				no_dpi ? "!":"");
		}
		if(node->data) {
			struct ndpi_port_def *pd = node->data;
			if(pd->count[0]+pd->count[1] > 0) {
			    l += snprintf(&lbuf[l],sizeof(lbuf)-l,"%-16s ",ibuf);
			    l += ndpi_print_port_range(pd->p,pd->count[0]+pd->count[1],
					&lbuf[l],sizeof(lbuf)-l,n->ndpi_struct);
			    l += snprintf(&lbuf[l],sizeof(lbuf)-l,"\n");
			}
		}
		if(cpos + l <= *ppos) {
			cpos += l;
		} else {
			int offs = 0;
			if(!count) break;
			if(cpos < *ppos) {
				offs = *ppos - cpos;
				l -= offs;
			}
			if(l > count) l = count;
			if (!(ACCESS_OK(VERIFY_WRITE, buf+bp, l) &&
				!__copy_to_user(buf+bp, &lbuf[offs], l))) return -EFAULT;
			count -= l;
			(*ppos) += l;
			cpos += l + offs;
			bp += l;
			if(!count) break;
	    	}

	    } // node->prefix
	    if (node->l) {
		if (node->r) {
		    *Xsp++ = node->r;
		}
		node = node->l;
		continue;
	    }
	    if (node->r) {
		node = node->r;
		continue;
	    }
	    node = Xsp != Xstack ? *(--Xsp): NULL;
	}
	return bp;
}

ssize_t n_ipdef_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos) {
	return _n_ipdef_proc_read(file, buf, count, ppos, AF_INET);
}
ssize_t n_ip6def_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos) {
	return _n_ipdef_proc_read(file, buf, count, ppos, AF_INET6);
}
