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
#include "ndpi_flow_info.h"
#include "ndpi_main_netfilter.h"
#include "ndpi_proc_flow.h"
#include "ndpi_proc_generic.h"

void nflow_proc_read_start(struct ndpi_net *n) {
	time64_t tm;

	tm=ktime_get_real_seconds();
	n->acc_end  = 0;
	n->acc_open_time = tm;
	n->flow_l   = NULL;
	memset(n->str_buf,0,NF_STR_LBUF);
	n->str_buf_len = 0;
	n->str_buf_offs = 0;
	n->cnt_view = 0;
	n->cnt_del  = 0;
	n->cnt_out  = 0;
}

size_t ndpi_dump_lost_rec(char *buf,size_t bufsize,
          uint32_t cpi, uint32_t cpo, uint64_t cbi, uint64_t cbo)
{
	struct flow_data_common *c;

	if(bufsize < sizeof(struct flow_data_common)) return 0;
	memset(buf,0,sizeof(struct flow_data_common));
	c = (struct flow_data_common *)buf;
	c->rec_type = 3;
	c->extflag  = 1;
	c->p[0] = cpi;
	c->p[1] = cpo;
	c->b[0] = cbi;
	c->b[1] = cbo;
	return sizeof(struct flow_data_common);
}

size_t ndpi_dump_start_rec(char *buf, size_t bufsize, time64_t tm)
{
	struct flow_data_common *c;

	if(bufsize < 8) return 0;
	memset(buf,0,8);
	c = (struct flow_data_common *)buf;
	c->rec_type = 1;
	c->extflag  = 1;
	c->time_start = (uint32_t)tm; // BUG AFTER YEAR 2105
	return 8;
}

static inline int add_opt_str(char *buf,size_t l,size_t size,char c,char *str) {
	size_t nc = strlen(str) + 2 + (l ? 1:0);
	if(l + nc >= size) return 0;
	buf += l;
	if(l) *buf++ = ' ';
	*buf++ = c;
	*buf++ = '=';
	strcpy(buf,str);
	return nc;
}

static int ndpi_confidence_level(ndpi_confidence_t confidence)
{
  switch(confidence) {
  case NDPI_CONFIDENCE_UNKNOWN:
    return 0;
  case NDPI_CONFIDENCE_MATCH_BY_PORT:
    return 1;
  case NDPI_CONFIDENCE_MATCH_BY_IP:
    return 2;
  case NDPI_CONFIDENCE_CUSTOM_RULE:
    return 3;
  case NDPI_CONFIDENCE_DPI_PARTIAL:
    return 4;
  case NDPI_CONFIDENCE_DPI_PARTIAL_CACHE:
    return 5;
  case NDPI_CONFIDENCE_DPI_CACHE:
    return 6;
  case NDPI_CONFIDENCE_DPI:
    return 7;
  case NDPI_CONFIDENCE_NBPF:
    return 8;
  default:
    return 0;
  }
}
/* 0-99 to str */
static int uint8tostr(char *buf,int v) {
	int i;
	v %= 100;
	i = v % 10;
	v /= 10;
	if(v) {
		buf[0] = '0'+v;
		buf[1] = '0'+i;
		buf[2] = '\0';
		return 2;
	}
	buf[0] = '0'+i;
	buf[1] = '\0';
	return 1;
}

static void risk2str(uint64_t risk,char *buf,size_t len) {
    int ri=0,l=0;
    uint64_t risk_s = risk;
    while(risk != 0 && l < len-4) { // ,XX\0
	if((risk & 0xffffffff) == 0) { ri+=32; risk >>=32; }
	if((risk & 0xffff) == 0) { ri+=16; risk >>=16; }
	if((risk & 0xff) == 0) { ri+=8; risk >>=8; }
	if((risk & 0xf) == 0) { ri+=4; risk >>=4; }
	if((risk & 0x3) == 0) { ri+=2; risk >>=2; }
	if((risk & 0x1) == 0) { ri+=1; risk >>=1; }
	if(l) buf[l++] = ',';
	l += uint8tostr(&buf[l],ri);
	ri++;
	risk >>=1;
    }
    if(risk)
	snprintf(buf,len-1,"0x%llx",risk_s);
    else
	buf[l] = '\0';
}

size_t ndpi_dump_opt(char *buf, size_t bufsize,
		struct nf_ct_ext_ndpi *ct)
{
	size_t l = 0,i,flag=0;

	buf[0] = '\0';

	for(i=0; i < NDPI_FLOW_OPT_MAX && ndpi_flow_opt[i]; i++) {
	   if(ndpi_flow_opt[i] != 'L' && ndpi_flow_opt[i] != 'R' && !ct->flow_opt) continue;
	   switch(ndpi_flow_opt[i]) {
		case 'R':
			if(!(flag & 0x20) && ct->risk && l < bufsize-20) {
			    char hbuf[22];
			    risk2str(ct->risk,hbuf,sizeof(hbuf));
			    l += add_opt_str(buf,l,bufsize,'R',hbuf);
			    flag |= 0x20;
			}
			break;
		case 'L':
			if(!(flag & 0x10) && ct->confidence != NDPI_CONFIDENCE_UNKNOWN && l < bufsize-4) {
			    if(l) buf[l++] = ' ';
			    buf[l] = 'L'; buf[l+1] = '=';
			    buf[l+2] = (char)('0' + ndpi_confidence_level(ct->confidence));
			    l += 3;
			    flag |= 0x10;
			}
			break;
		case 'S':
			if(!(flag & 1) && ct->ja3s) {
			    l += add_opt_str(buf,l,bufsize,'S',&ct->flow_opt[ct->ja3s-1]);
			    flag |= 1;
			}
			break;
		case 'C':
			if(!(flag & 2) && ct->ja3c) {
			    l += add_opt_str(buf,l,bufsize,'C',&ct->flow_opt[ct->ja3c-1]);
			    flag |= 2;
			}
			break;
		case 'c':
			if(!(flag & 0x40) && ct->ja4c) {
			    l += add_opt_str(buf,l,bufsize,'c',&ct->flow_opt[ct->ja4c-1]);
			    flag |= 0x40;
			}
			break;
		case 'F':
			if(!(flag & 4) && ct->tlsfp) {
			    l += add_opt_str(buf,l,bufsize,'F',&ct->flow_opt[ct->tlsfp-1]);
			    flag |= 4;
			}
			break;
		case 'V':
			if(!(flag & 8) && ct->tlsv) {
			    l += add_opt_str(buf,l,bufsize,'V',&ct->flow_opt[ct->tlsv-1]);
			    flag |= 8;
			}
			break;
		default:
			i=NDPI_FLOW_OPT_MAX;
	   }
	}
	buf[l] = '\0';
	return l;
}
ssize_t ndpi_dump_acct_info_bin(struct ndpi_net *n,int v6,
		char *buf, size_t buflen, struct nf_ct_ext_ndpi *ct)
{
	struct flow_data_common *c;
	struct flow_data *d;
	struct flow_info *i;
	ssize_t ret_len;
	int o_len,h_len;
	char buf_opt[512];

	ret_len = v6 ? flow_data_v6_size : flow_data_v4_size;
	h_len = ct->host ? strlen(ct->host):0;
	o_len = ndpi_dump_opt(buf_opt,sizeof(buf_opt)-1,ct);
	if(h_len != 0 && o_len != 0)
		o_len++; // Add space before opttions

	if(o_len + h_len > NF_STR_OPTLEN) { /* FIXME */
		if(h_len > 255) h_len = 255;
		if(o_len > 255) o_len = NF_STR_OPTLEN - h_len;
	}
	ret_len += o_len + h_len;

	if(buflen < ret_len) return 0;

	memset(buf,0,ret_len);
	d = (struct flow_data *)buf;
	c = &d->c;
	i = &ct->flinfo;

	c->rec_type	= 2;
	c->family	= v6 != 0;
	c->extflag      = 1;
	c->proto	= ct->l4_proto;
	if( o_len > 255 || h_len > 255) {
	  c->opt_len	= o_len+h_len-255;
	  c->host_len	= 255;
	} else {
	  c->opt_len	= o_len;
	  c->host_len	= h_len;
	}
	c->time_start	= i->time_start;
	if((n->acc_read_mode & 0x3) != 2) {
		c->p[0]		= i->p[0]-i->p[2];
		c->p[1]		= i->p[1]-i->p[3];
		c->b[0]		= i->b[0]-i->b[2];
		c->b[1]		= i->b[1]-i->b[3];
	} else {
		c->p[0]		= i->p[0];
		c->p[1]		= i->p[1];
		c->b[0]		= i->b[0];
		c->b[1]		= i->b[1];
	}
	c->time_end	= i->time_end;
	c->ifidx	= i->ifidx;
	c->ofidx	= i->ofidx;
	c->proto_master	= ct->proto.master_protocol;
	c->proto_app	= ct->proto.app_protocol;
	c->connmark	= ct->flinfo.connmark;
	if(!v6) {
		struct flow_data_v4 *a = &d->d.v4;
		a->ip_s = i->ip_s.ip;
		a->ip_d = i->ip_d.ip;
		a->ip_snat = i->ip_snat;
		a->ip_dnat = i->ip_dnat;
		a->sport = i->sport;
		a->dport = i->dport;
		a->sport_nat = i->sport_nat;
		a->dport_nat = i->dport_nat;
		c->nat_flags = ct->flags >> f_snat; // f_snat,f_dnat,f_userid
	} else {
		struct flow_data_v6 *a = &d->d.v6;
		memcpy(a->ip_s,(char *)&i->ip_s,16);
		memcpy(a->ip_d,(char *)&i->ip_d,16);
		a->sport = i->sport;
		a->dport = i->dport;
	}
	if(o_len + h_len) {
		char *s = (char *)c + (v6 ? flow_data_v6_size : flow_data_v4_size);
		if(h_len) {
			memcpy(s,ct->host,h_len);
			s += h_len;
			if(o_len)
			   *s++ = ' ';
		}
		if(o_len)
			memcpy(s,buf_opt,o_len);
	}
	n->str_buf_len = ret_len; 
	return ret_len;
}


ssize_t ndpi_dump_acct_info(struct ndpi_net *n,
		struct nf_ct_ext_ndpi *ct) {
	const char *t_proto;
	ssize_t l = 0;
	char *buf = n->str_buf;
	size_t buflen = NF_STR_LBUF-1;
	int is_ipv6 = test_ipv6(ct);

	n->str_buf_offs = 0;
	n->str_buf_len = 0;

	if(n->acc_read_mode >= 4) {
		return ndpi_dump_acct_info_bin(n, is_ipv6, buf, buflen, ct);
	}

	*buf = 0;

	buflen -= 2; // space for \n \0
	l = snprintf(buf,buflen,"%u %u %c %d ",
		ct->flinfo.time_start,ct->flinfo.time_end,
		is_ipv6 ? '6':'4', ct->l4_proto);
	if(is_ipv6) {
	    l += snprintf(&buf[l],buflen-l,"%pI6c %d %pI6c %d",
		&ct->flinfo.ip_s, htons(ct->flinfo.sport),
		&ct->flinfo.ip_d, htons(ct->flinfo.dport));
	} else {
	    l += snprintf(&buf[l],buflen-l,"%pI4n %d %pI4n %d",
		&ct->flinfo.ip_s, htons(ct->flinfo.sport),
		test_dnat(ct) ? (char*)&ct->flinfo.ip_dnat : 
				(char*)&ct->flinfo.ip_d,
		test_dnat(ct) ? htons(ct->flinfo.dport_nat):
				htons(ct->flinfo.dport) );
	}
	if((n->acc_read_mode & 0x3) != 2) {
	   l += snprintf(&buf[l],buflen-l," %llu %llu %u %u",
		ct->flinfo.b[0]-ct->flinfo.b[2],
		ct->flinfo.b[1]-ct->flinfo.b[3],
		ct->flinfo.p[0]-ct->flinfo.p[2],
		ct->flinfo.p[1]-ct->flinfo.p[3]);
	} else {
	   l += snprintf(&buf[l],buflen-l," %llu %llu %u %u",
		ct->flinfo.b[0],
		ct->flinfo.b[1],
		ct->flinfo.p[0],
		ct->flinfo.p[1]);
	}

	if(ct->flinfo.ifidx != ct->flinfo.ofidx)
		l += snprintf(&buf[l],buflen-l," I=%d,%d",ct->flinfo.ifidx,ct->flinfo.ofidx);
	else
		l += snprintf(&buf[l],buflen-l," I=%d",ct->flinfo.ifidx);
#if defined(CONFIG_NF_CONNTRACK_MARK)
	if(ct->flinfo.connmark)
	    l += snprintf(&buf[l],buflen-l," CM=%x",ct->flinfo.connmark);
#endif
	if(!is_ipv6) {
	    if(test_snat(ct)) {
		l += snprintf(&buf[l],buflen-l," SN=%pI4n,%d",
				&ct->flinfo.ip_snat,htons(ct->flinfo.sport_nat));
	    }
	    if(test_dnat(ct)) {
		l += snprintf(&buf[l],buflen-l," DN=%pI4n,%d",
				&ct->flinfo.ip_d,htons(ct->flinfo.dport));
	    }
#ifdef USE_HACK_USERID
	    if(test_userid(ct)) {
		l += snprintf(&buf[l],buflen-l," UI=%pI4n,%d",
				&ct->flinfo.ip_snat,htons(ct->flinfo.sport_nat));
	    }
#endif
	}
	t_proto = ndpi_get_proto_by_id(n->ndpi_struct,ct->proto.app_protocol);
	l += snprintf(&buf[l],buflen-l," P=%s",t_proto);
	if(ct->proto.master_protocol != NDPI_PROTOCOL_UNKNOWN &&
	   ct->proto.master_protocol != ct->proto.app_protocol) {
	    t_proto = ndpi_get_proto_by_id(n->ndpi_struct,ct->proto.master_protocol);
	    l += snprintf(&buf[l],buflen-l,",%s",t_proto);
	}
	{
	  int sl_host = 0; 
	  int sl_opt = 0; 
	  if(ct->host) {
	    sl_host = strlen(ct->host);
	    if(sl_host > 250) sl_host = 250;
	    if(l + sl_host + 6 > buflen)
		   sl_host = buflen - 6 - l;
	    l += snprintf(&buf[l],buflen-l," H=%.*s",sl_host,ct->host);
	  }
	  {
	    char opt_buf[512];
	    sl_opt = ndpi_dump_opt(opt_buf,sizeof(opt_buf)-1,ct);
	    if(sl_opt) {
	      if(sl_opt + sl_host > NF_STR_OPTLEN ) sl_opt = NF_STR_OPTLEN - 4 - sl_host;
	      if(l + sl_opt + 6 > buflen)
		   sl_opt = buflen - 6 - l;
	      l += snprintf(&buf[l],buflen-l," %.*s",sl_opt,opt_buf);
	    }
	  }
	}
	buf[l++] = '\n';
	buf[l] = 0;
	n->str_buf_len = l; 
	return l;
}

#if LINUX_VERSION_CODE < KERNEL_VERSION(3,18,0)
static inline int unsigned_offsets(struct file *file)
{
	return file->f_mode & FMODE_UNSIGNED_OFFSET;
}

static loff_t vfs_setpos(struct file *file, loff_t offset, loff_t maxsize)
{
	if (offset < 0 && !unsigned_offsets(file))
		return -EINVAL;
	if (offset > maxsize)
		return -EINVAL;

	if (offset != file->f_pos) {
		file->f_pos = offset;
		file->f_version = 0;
	}
	return offset;
}
#endif

ssize_t nflow_proc_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos)
{
        struct ndpi_net *n = pde_data(file_inode(file));
	if(n->acc_last_op != 1) { // seek 0 after write command
		n->acc_last_op = 1;
		nflow_proc_read_start(n);
		vfs_setpos(file,0,OFFSET_MAX);
		*ppos = 0;
	}
	return nflow_read(n, buf, count, ppos);
}

static char *read_modes[8]= {
	"read_all",		// 0
	"read_closed",		// 1
	"read_flows",		// 2
	"",			// 3 unused
	"read_all_bin",		// 4
	"read_closed_bin",	// 5
	"read_flows_bin",	// 6
	NULL
};

static int parse_ndpi_flow(struct ndpi_net *n,char *buf)
{
	int idx;

	if(!ndpi_enable_flow) return -EINVAL;

        if(!buf[0]) return 0;

	if(sscanf(buf,"timeout=%d",&idx) == 1) {
		if(idx < 1 || idx > 600) return -EINVAL;
		n->acc_wait = idx;
		if(flow_read_debug)
			pr_info("%s:%s set timeout=%d\n",__func__,n->ns_name,n->acc_wait);
		return 0;
	}
	if(sscanf(buf,"limit=%d",&idx) == 1) {
		if(idx < atomic_read(&n->acc_work) || idx > ndpi_flow_limit)
			return -EINVAL;
		n->acc_limit = idx;
		if(flow_read_debug)
			pr_info("%s:%s set limit=%d\n",__func__,n->ns_name,n->acc_limit);
		return 0;
	}

	for(idx=0; read_modes[idx]; idx++) {
		if(*read_modes[idx] && !strcmp(buf,read_modes[idx])) {
			if(n->acc_end || !n->flow_l) {
				n->acc_read_mode = idx;
			} else return -EINVAL;
			if(flow_read_debug)
				pr_info("%s:%s set read_mode=%d\n",__func__,n->ns_name,n->acc_read_mode);
			return 0;
		}
	}
	return -EINVAL;
}

int nflow_proc_open(struct inode *inode, struct file *file) {
        struct ndpi_net *n = pde_data(file_inode(file));

	if(!ndpi_enable_flow) return -EINVAL;

	mutex_lock(&n->rem_lock);
	n->acc_read_mode = 0;
	if(!n->acc_wait) n->acc_wait = 60;
	n->acc_last_op = 1;
	nflow_proc_read_start(n);
	return 0;
}

int nflow_proc_close(struct inode *inode, struct file *file)
{
        struct ndpi_net *n = pde_data(file_inode(file));
	if(!ndpi_enable_flow) return -EINVAL;
	generic_proc_close(n,parse_ndpi_flow,W_BUF_FLOW);
	if(flow_read_debug)
		pr_info("%s:%s view %ld dumped %ld deleted %ld\n",
			__func__,n->ns_name,n->cnt_view,n->cnt_out,n->cnt_del);
	n->acc_gc = jiffies + n->acc_wait*HZ;
	mutex_unlock(&n->rem_lock);
        return 0;
}

ssize_t
nflow_proc_write(struct file *file, const char __user *buffer,
		                     size_t length, loff_t *loff)
{
struct ndpi_net *n = pde_data(file_inode(file));
	if(!ndpi_enable_flow) return -EINVAL;
	if(n->flow_l) return -EINVAL; // Reading in progress!
	n->acc_last_op = 2;
	return generic_proc_write(pde_data(file_inode(file)), buffer, length, loff,
				  parse_ndpi_flow, 4060 , W_BUF_FLOW);
}

loff_t nflow_proc_llseek(struct file *file, loff_t offset, int whence) {
	if(whence == SEEK_SET) {
		struct ndpi_net *n = pde_data(file_inode(file));
		if(offset == 0) {
			if(flow_read_debug)
				pr_info("%s:%s seek start\n", __func__,n->ns_name);
			nflow_proc_read_start(n);
			return vfs_setpos(file,offset,OFFSET_MAX);
		}
		if(offset >= atomic_read(&n->acc_work)) {
			n->acc_end = 1;
			n->flow_l = NULL;
			n->str_buf_len = 0;
			n->str_buf_offs = 0;
			if(flow_read_debug)
				pr_info("%s:%s seek EOF\n", __func__,n->ns_name);
			return vfs_setpos(file,atomic_read(&n->acc_work),OFFSET_MAX);
		}
	}
	if(whence == SEEK_CUR && offset == 0) {
		return noop_llseek(file,offset,whence);
	}
	return -EINVAL;
}

