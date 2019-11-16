
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/version.h>

#include <linux/ip.h>
#include <linux/ipv6.h>

#include "ndpi_config.h"
#undef HAVE_HYPERSCAN
#include "ndpi_main.h"

#include "ndpi_strcol.h"
#include "ndpi_main_common.h"
#include "ndpi_main_netfilter.h"
#include "ndpi_proc_generic.h"

int
generic_proc_close(struct ndpi_net *n,
		     int (*parse_line)(struct ndpi_net *n,char *cmd),
		     write_buf_id_t id)
{
	struct write_proc_cmd *w_buf;
	int ret = 0;

	spin_lock(&n->w_buff_lock);
	w_buf = n->w_buff[id];
	n->w_buff[id] = NULL;
	spin_unlock(&n->w_buff_lock);

	if(w_buf) {
		if(w_buf->cpos ) {
			if(ndpi_log_debug > 1)
			    pr_info("%s:%s cmd %d:%s\n",__func__,n->ns_name,
					    w_buf->cpos,&w_buf->cmd[0]);
			ret = (parse_line)(n,&w_buf->cmd[0]);
		}
		kfree(w_buf);
	}
	return ret;
}

struct write_proc_cmd * alloc_proc_wbuf(struct ndpi_net *n,
					write_buf_id_t id,size_t cmd_len_max) {
	struct write_proc_cmd *ret;
	spin_lock(&n->w_buff_lock);
	ret = n->w_buff[id];
	if(!ret) {
		ret = kmalloc(sizeof(struct write_proc_cmd) + cmd_len_max + 1,
				GFP_ATOMIC); // under spin_lock
		if(ret) {
			ret->max = cmd_len_max;
			ret->cpos = 0;
			memset(&ret->cmd[0],0,cmd_len_max);
			n->w_buff[id] = ret;
		}
	}
	spin_unlock(&n->w_buff_lock);
	return ret;
}


ssize_t
generic_proc_write(struct ndpi_net *n, const char __user *buffer,
                     size_t length, loff_t *loff, 
		     int (*parse_line)(struct ndpi_net *n,char *cmd),
		     size_t cmd_size,write_buf_id_t id)
{
	char c,buf[1024+1];
	struct write_proc_cmd *w_buf;
	int pos,i,l,r,skip;

	if (length <= 0) return length;
	pos = 0;

	w_buf =  alloc_proc_wbuf(n,id,cmd_size);
	if(!w_buf) return -ENOBUFS;
	skip = w_buf->cpos == w_buf->max - 1;

	while(pos < length) {
		l = min(length-pos,sizeof(buf)-1);
	
		memset(buf,0,sizeof(buf));
		if (!(ACCESS_OK(VERIFY_READ, buffer+pos, l) && 
			!__copy_from_user(&buf[0], buffer+pos, l)))
			        return -EFAULT;
		for(i = 0; i < l; (*loff)++,i++) {
			c = buf[i];
			if(c == '\n' || !c) {
				if(w_buf->cpos) {
					if(ndpi_log_debug > 1)
					    pr_info("%s:%s POS %lld cmd %d:'%s' i %d\n", __func__,n->ns_name,
							   *loff,w_buf->cpos,&w_buf->cmd[0],i);
					r = (parse_line)(n,&w_buf->cmd[0]);
					memset(&w_buf->cmd[0],0,w_buf->cpos);
					skip = 0;
					w_buf->cpos = 0;
					if(r) return -EINVAL;
				}
			} else {
				if(w_buf->cpos < w_buf->max - 1)
					w_buf->cmd[w_buf->cpos++] = c;
				    else {
					    if(!skip)
						pr_err("xt_ndpi:%s Command too long\n",n->ns_name);
					    skip = 1;
					}
			}
		}
		pos += l;
        }
        return length;
}

