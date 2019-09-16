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
        struct ndpi_net *n = PDE_DATA(file_inode(file));
	generic_proc_close(n,parse_ndpi_ipdef,W_BUF_IP);
        return 0;
}

ssize_t
n_ipdef_proc_write(struct file *file, const char __user *buffer,
                     size_t length, loff_t *loff)
{
	return generic_proc_write(PDE_DATA(file_inode(file)), buffer, length, loff,
			parse_ndpi_ipdef, 4060 , W_BUF_IP);
}

ssize_t n_ipdef_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos)
{
        struct ndpi_net *n = PDE_DATA(file_inode(file));
	patricia_tree_t *pt;
	prefix_t *px;
	patricia_node_t *Xstack[PATRICIA_MAXBITS+1], **Xsp, *node;
	char lbuf[512];
	char ibuf[64];
	int l,bp;
	loff_t cpos;

	cpos = 0; bp = 0;
	pt = n->ndpi_struct->protocols_ptree;
	Xsp = &Xstack[0];
	node = pt->head;
	while (node) {
	    if (node->prefix) {
		l = cpos ? 0: snprintf(lbuf,sizeof(lbuf),"#ip              proto\n");
		
		px = node->prefix;
		{
		int k;
		inet_ntop(px->family,(void *)&px->add,ibuf,sizeof(ibuf)-1);
		k = strlen(ibuf);
		if((px->family == AF_INET  && px->bitlen < 32 ) ||
		   (px->family == AF_INET6 && px->bitlen < 128 ))
			snprintf(&ibuf[k],sizeof(ibuf)-k,"/%d",px->bitlen);
		}
		if(node->value.user_value != NDPI_PROTOCOL_UNKNOWN)
		    l += snprintf(&lbuf[l],sizeof(lbuf)-l,"%-16s %s\n",ibuf,
			node->value.user_value >= NDPI_NUM_BITS ?
				"unknown":ndpi_get_proto_by_id(n->ndpi_struct,node->value.user_value));
		if(node->data) {
			struct ndpi_port_def *pd = node->data;
			ndpi_port_range_t *pt = pd->p;
			if(pd->count[0]+pd->count[1] > 0) {
			    l += snprintf(&lbuf[l],sizeof(lbuf)-l,"%-16s ",ibuf);
			    l += ndpi_print_port_range(pt,pd->count[0]+pd->count[1],
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
