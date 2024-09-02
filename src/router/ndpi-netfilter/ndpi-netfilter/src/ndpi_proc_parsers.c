#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kref.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/atomic.h>

#include "ndpi_config.h"
#undef HAVE_HYPERSCAN

#include "ndpi_main.h"
#include "ndpi_private.h"
#include "../lib/third_party/include/ndpi_patricia.h"

#include "ndpi_main_common.h"
#include "ndpi_strcol.h"
#include "ndpi_main_netfilter.h"
#include "ndpi_proc_parsers.h"
#include "ndpi_proc_info.h"

/*
 * Syntax: 
 * reset
 * proto:host[,host...][[ \t;]proto:host[,host...]]
 */

int parse_ndpi_hostdef(struct ndpi_net *n,char *cmd) {

    char *pname,*host_match,*nc,*nh,*cstr;
    uint16_t protocol_id;
    int op_del = 0;

    nc = NULL;

    for(; cmd && *cmd ; cmd = nc ) {
	if(*cmd == '#') break;

	while(*cmd && (*cmd == ' ' || *cmd == '\t')) cmd++;
	if(_DBG_TRACE_SPROC_H)
		pr_info("%s: %.100s\n",__func__,cmd);
	if(!strcmp(cmd,"reset")) {

		if(_DBG_TRACE_SPROC_H)
			pr_info("hostdef: clean host_ac %px\n",n->host_ac);

		for(protocol_id = 0; protocol_id < NDPI_NUM_BITS; protocol_id++) {
			if(n->hosts_tmp->p[protocol_id]) {
				kfree(n->hosts_tmp->p[protocol_id]);
				n->hosts_tmp->p[protocol_id] = NULL;
			}
		}
		n->host_error = 0;
		n->host_upd++;

		if(_DBG_TRACE_SPROC_H)
			pr_info("xt_ndpi: reset hosts\n");
		break;
	}
	pname = cmd;
	host_match = strchr(pname,':');

	if(!*pname || !host_match)
		goto bad_cmd;

	*host_match++ = '\0';

	nc = strchr(host_match,' ');
	if(!nc) nc = strchr(host_match,'\t');
	if(!nc) nc = strchr(host_match,';');

	if(nc) {
		*nc++ = '\0';
		while(*nc && (*nc == ' ' || *nc == '\t' || *nc == ';')) nc++;
	}
	if(*pname == '+') {
		pname++;
	} else if(*pname == '-') {
		op_del = 1;
		pname++;
	}
	protocol_id =  ndpi_get_proto_by_name(n->ndpi_struct, pname);

	if(protocol_id == NDPI_PROTOCOL_UNKNOWN) {
		pr_err("xt_ndpi: unknown protocol %s\n",pname);
		goto bad_cmd;
	}
	
	if(protocol_id >= NDPI_NUM_BITS) {
		pr_err("xt_ndpi: bad protoId=%u\n", protocol_id);
		goto bad_cmd;
	}

	if(_DBG_TRACE_SPROC_H)
		pr_err("%s: %c%.100s\n",__func__,op_del ? '-':'+',host_match);

	for(nh = NULL; host_match && *host_match; host_match = nh) {
		size_t sml;
		nh = strchr(host_match,',');
		if(nh) *nh++ = '\0';

		sml = strlen(host_match);

		if(op_del && *host_match == '*' && sml == 1) {
			if(n->hosts_tmp->p[protocol_id]) {
				kfree(n->hosts_tmp->p[protocol_id]);
				n->hosts_tmp->p[protocol_id] = NULL;
				n->host_upd++;
			}
			continue;
		}
		if(str_collect_look(n->hosts_tmp->p[protocol_id],host_match,sml) >= 0) {
			if(op_del) 
				str_collect_del(n->hosts_tmp->p[protocol_id],host_match,sml);
			else {
				pr_err("xt_ndpi: exists '%.60s'\n",host_match);
				continue;
			}
		} else 	{
			cstr = str_collect_add(&n->hosts_tmp->p[protocol_id],host_match,sml);
			if(!cstr) {
				pr_err("xt_ndpi: can't alloc memory for '%.60s'\n",host_match);
				goto bad_cmd;
			}
		}
		n->host_upd++;
	}
	if(_DBG_TRACE_SPROC_H && nc && *nc)
		pr_info("xt_ndpi: next part '%s'\n",nc);
    }
    return 0;

bad_cmd:
    n->host_error++;
    return 1;
}



/*************************** ip_def **************************/

uint16_t ndpi_check_ipport(ndpi_patricia_node_t *node,uint16_t port,int l4) {
struct ndpi_port_def *pd;
if(!node) return NDPI_PROTOCOL_UNKNOWN;
pd = node->data;
if(!pd) return NDPI_PROTOCOL_UNKNOWN;
if(pd->count[l4]) {
	int start,end,i;
	ndpi_port_range_t *pt;

	start = !l4 ? 0:pd->count[0];
	end = start + pd->count[l4];

	if(port >= pd->p[start].start &&
	   port <= pd->p[end-1].end) {
		i = start + ((end-start) >> 1);
		do {
			pt = &pd->p[i];
			if(pt->start <= port && port <= pt->end) return pt->proto | (pt->no_dpi ? 0x8000:0);
			if(port < pt->start) {
			    end = i;
			} else {
			    start = i+1;
			}
			i = start + ((end-start) >> 1);
		} while(start < end);
	}
}
return (node->value.u.uv32.user_value & 0x7fff) |
	(node->value.u.uv32.user_value & 0xff0000 ? 0x8000:0);
}

int ndpi_print_port_range(ndpi_port_range_t *pt,
		int count,char *buf,size_t bufsize,
		ndpi_mod_str_t *ndpi_str) {
 const char *t_proto;
 int l = 0;
	for(; count > 0; count--,pt++) {
		if(l && l < bufsize) buf[l++] = ' ';
		l += snprintf(&buf[l],bufsize - l,"%s",
			  pt->l4_proto ? "tcp":"udp");
		if(pt->start == pt->end)
		  l += snprintf(&buf[l],bufsize - l,":%d",pt->start);
		else
		  l += snprintf(&buf[l],bufsize - l,":%d-%d",pt->start,pt->end);

		t_proto = ndpi_get_proto_by_id(ndpi_str,pt->proto & 0xffff);
		l += snprintf(&buf[l],bufsize - l,":%s%s", t_proto ? t_proto : "unknown",
				pt->no_dpi ? "!":"");
		if(l == bufsize) break;
	}
	return l;
}

int parse_n_proto(char *pr,ndpi_port_range_t *np,ndpi_mod_str_t *ndpi_str)
{
int i;
char *lc;

if(!*pr) return 1;

if(!strcmp(pr,"any")) {
	np->proto = NDPI_NUM_BITS;
	return 0;
}
lc = pr + strlen(pr)-1;
if(*lc == '!') {
	np->no_dpi = 1;
	*lc = '\0';
}
i = ndpi_get_proto_by_name(ndpi_str,pr);
if(i != NDPI_PROTOCOL_UNKNOWN) {
	DP("%s: parse_n_proto(%s)=%x\n",pr,i);
	np->proto =i;
	return 0;
}
return 1;
}

int parse_l4_proto(char *pr,ndpi_port_range_t *np)
{
if(!*pr) return 1;

if(!strcmp(pr,"any")) {
	np->l4_proto = 2; return 0;
}
if(!strcmp(pr,"tcp")) {
	np->l4_proto = 1; return 0;
}
if(!strcmp(pr,"udp")) {
	np->l4_proto = 0; return 0;
}
return 1;
}

int parse_port_range(char *pr,ndpi_port_range_t *np)
{
    char *d;
    uint32_t v;
    
    if(!*pr) return 1;

    if(!strcmp(pr,"any")) {
	np->start = 0;
	np->end = 0;
	return 0;
    }
    d = strchr(pr,'-');
    if(d) *d++ = '\0';
    
    if(kstrtou32(pr,10,&v)) return 1;
    if(v > 0xffff) return 1;

    np->start = v;

    if(d) {
    	if(kstrtou32(d,10,&v)) return 1;
	if(v > 0xffff) return 1;
	np->end = v;
    } else {
    	np->end = v;
    }
    if(np->start > np->end) {
	    v = np->start;
	    np->start = np->end;
	    np->end = v;
    }
    if(d) *--d = '-';

    return 0;
}

/**************************************************************/
NDPI_STATIC struct ndpi_port_def *ndpi_port_range_replace(
		struct ndpi_port_def *pd,
		int start, int end,
		ndpi_port_range_t *np,
		int count,
		ndpi_mod_str_t *ndpi_str)
{
struct ndpi_port_def *pd1;

DBGDATA(char dbuf[256])

#ifdef NDPI_IPPORT_DEBUG
ndpi_print_port_range(pd->p,pd->count[0]+pd->count[1],dbuf,sizeof(dbuf),ndpi_str);
DP("%s: old %d,%d %s\n",pd->count[0],pd->count[1],dbuf);
ndpi_print_port_range(np,count,dbuf,sizeof(dbuf),ndpi_str);
DP("%s: on %d,%d,%d,%d %s\n",start,end,count,np->l4_proto,dbuf);
#endif

if( (end-start) == count) {
	if(count) {
	    memcpy(&pd->p[start],np,count*sizeof(ndpi_port_range_t));
	    DP("%s: replaced!\n");
	}
	return pd;
}

pd1 = ndpi_malloc( sizeof(struct ndpi_port_def) + 
		   sizeof(ndpi_port_range_t) * 
		   (count + pd->count[1-np->l4_proto]));
if(!pd1) return pd;

memcpy(pd1,pd,sizeof(struct ndpi_port_def));
if(!np->l4_proto) { // udp
	if(count) 
		memcpy( &pd1->p[0],np,count*sizeof(ndpi_port_range_t));
	if(pd->count[1])
		memcpy( &pd1->p[count], &pd->p[pd->count[0]],
			pd->count[1]*sizeof(ndpi_port_range_t));
	pd1->count[0] = count;
} else { // tcp
	if(pd->count[0])
		memcpy( &pd1->p[0], &pd->p[0],
			pd->count[0]*sizeof(ndpi_port_range_t));
	if(count)
		memcpy( &pd1->p[pd->count[0]], np,
			count*sizeof(ndpi_port_range_t));
	pd1->count[1] = count;
}

#ifdef NDPI_IPPORT_DEBUG
ndpi_print_port_range(pd1->p,pd1->count[0]+pd1->count[1],dbuf,sizeof(dbuf),ndpi_str);
pr_info("%s: res %d,%d %s\n",__func__,pd1->count[0],pd1->count[1],dbuf);
#endif

ndpi_free(pd);
return pd1;
}

NDPI_STATIC struct ndpi_port_def *ndpi_port_range_update(
		struct ndpi_port_def *pd,
		ndpi_port_range_t *np,
		int op,
		ndpi_mod_str_t *ndpi_str)
{
ndpi_port_range_t *tp,*tmp;
int i,n,k;
int start,end;
if(!np->l4_proto) {
	start = 0;
	end   = pd->count[0];
} else {
	start = pd->count[0];
	end   = start + pd->count[1];
}
n = end-start;
DP("%s: %s:%d-%d:%d %d %d %d %s\n",
	np->l4_proto ? (np->l4_proto == 1 ? "tcp":"any"):"udp",
	np->start,np->end,np->proto,start,end,np->l4_proto,
	op ? "set":"del");

if(!n) {
	if(!op) return pd;
	return ndpi_port_range_replace(pd,start,end,np,1,ndpi_str); // create 1 range
}

tmp = ndpi_malloc(sizeof(ndpi_port_range_t)*(n+2));
if(!tmp) return pd;
memset((char *)tmp,0,sizeof(ndpi_port_range_t)*(n+2));

i = start;
tp = &pd->p[i];
k = 0;

#ifdef NDPI_IPPORT_DEBUG
#define DD1 pr_info("%s: k=%d tmp %d-%d i=%d tp %d-%d np %d-%d \n", \
		__func__,k,tmp[k].start,tmp[k].end,i,tp->start,tp->end,np->start,np->end);
#else
#define DD1
#endif
for(;i < end && np->start > tp->end; tp++,i++) {
	DD1
	tmp[k++] = *tp;
}
DD1
if(i < end ) {
	DD1;
	if(np->start > tp->start && np->start < tp->end) {
	    tmp[k] = *tp;
	    tmp[k].end = np->start-1;
	    k++;
	    DP("%s: P0 k %d\n",k);
	}
	tmp[k] = *np;
	DD1;
      v0:
	if(np->end < tp->start) {
	    // insert before old ranges
	    DP("%s: P1\n");
	    goto copy_tail;
	}
	if(np->end < tp->end) {
	    k++;
	    tmp[k] = *tp++; i++;
	    tmp[k].start = np->end+1;
	    DP("%s: P2\n");
	    goto copy_tail;
	}
	if(np->end == tp->end) { // override first old range
	    tp++; i++;
	    DP("%s: P3\n");
	    goto copy_tail;
	}
	// np->end > tp->end
	for(; i < end; i++,tp++) {
	    if(np->end > tp->end) continue;
	    goto v0;
	}
	k++;
	// override all old ranges!
	goto check_eq_proto;
} else {
	// append new range
	tmp[k++] = *np;
	DP("%s: P4 k=%d\n",k);
	goto check_eq_proto; // OK
}

copy_tail:
    k++;
    for(; i < end; i++,tp++) tmp[k++] = *tp;

// k - new length of range
check_eq_proto:

#ifdef NDPI_IPPORT_DEBUG
{
char dbuf[128];
ndpi_print_port_range(tmp,k,dbuf,sizeof(dbuf),ndpi_str);
pr_info("%s: k=%d: %s\n",__func__,k,dbuf);
}
#endif

if(k <= 1) {
    if(!op) k = 0;
} else {
    int l;
    if(!op) {
	for(l = 0 , i = 0; i < k; i++) {
	    if(tmp[i].start == np->start &&
		tmp[i].end   == np->end &&
		tmp[i].proto == np->proto) {
		continue;	
	    }
	    if(i != l) tmp[l++] = tmp[i];
	}
	k = l;
    }

    for(l = 0; l < k-1; ) {
	i = l+1;
	DP("%s: l=%d %d-%d.%d i=%d %d-%d.%d n=%d\n",
		l,tmp[l].start,tmp[l].end,tmp[l].proto,
		i,tmp[i].start,tmp[i].end,tmp[i].proto,n);

	if(tmp[l].proto != tmp[i].proto ||
	   tmp[l].end+1 != tmp[i].start) {
	    l++;
	    continue;
	}
	tmp[l].end = tmp[i].end;
	for(;i < k-1; i++) tmp[i] = tmp[i+1];
	k--;
    }
    k = l+1;
}
DP("%s: k %d\n",k);

pd = ndpi_port_range_replace(pd,start,end,tmp,k,ndpi_str);
ndpi_free(tmp);
return pd;
}

static inline void _node_free_data(ndpi_patricia_node_t *node) {
	if(!node->data) return;
	ndpi_free(node->data);
	node->data = NULL;
}

static void pt_node_free_data(void *data) {
	_node_free_data((ndpi_patricia_node_t *)data);
}

NDPI_STATIC void *ndpi_port_add_one_range(void *data, ndpi_port_range_t *np,int op,
		ndpi_mod_str_t *ndpi_str)
{
struct ndpi_port_def *pd = data;
int i;
if(!pd) {
	if(!op) return data;

	i = np->l4_proto != 2 ? 1:2;
	pd = ndpi_malloc(sizeof(struct ndpi_port_def) + sizeof(ndpi_port_range_t)*i);
	if(!pd) return pd;

	pd->p[0] = *np;
	pd->count[0] = np->l4_proto != 1;
	pd->count[1] = np->l4_proto > 0;
	if(np->l4_proto == 2) {
		pd->p[1] = *np;
		pd->p[0].l4_proto = 0;
		pd->p[1].l4_proto = 1;
	}
	return pd;
}
switch(np->l4_proto) {
    case 2: // any
	np->l4_proto = 0;
	pd = ndpi_port_range_update(pd,np,op,ndpi_str);
	np->l4_proto = 1;
	pd = ndpi_port_range_update(pd,np,op,ndpi_str);
	break;
    case 1:
	pd = ndpi_port_range_update(pd,np,op,ndpi_str);
	break;
    case 0:
	pd = ndpi_port_range_update(pd,np,op,ndpi_str);
}
if(pd && (pd->count[0] + pd->count[1]) == 0) {
	ndpi_free(pd);
	pd = NULL;
}
return pd;
}

int parse_ndpi_ipdef_cmd(struct ndpi_net *n, int f_op, ndpi_prefix_t *prefix, char *arg,
		int family, const char *paddr) {
char *d1,*d2;
int f_op2=0;
int ret = 0;
ndpi_patricia_tree_t *pt;
ndpi_port_range_t np = { .start=0, .end=0, .l4_proto = 2, .proto = NDPI_PROTOCOL_UNKNOWN };
ndpi_patricia_node_t *node;

pt = family == AF_INET ? 
	n->ndpi_struct->protocols->v4:n->ndpi_struct->protocols->v6;

if(*arg == '-') {
	f_op2=1;
	arg++;
}
d1 = strchr(arg,':');
if(d1) *d1++='\0';
d2 = d1 ? strchr(d1,':'):NULL;
if(d2) *d2++='\0';
DP("%s(op=%d, %s %s %s)\n",f_op,arg,d1 ? d1: "(NULL)",d2 ? d2:"(NULL)");
if(d1) {
    if(d2) { // full spec
	if(parse_l4_proto(arg,&np)) return 1;
	if(parse_port_range(d1,&np)) return 1;
	if(parse_n_proto(d2,&np,n->ndpi_struct)) return 1;
	DP("%s:3 proto %d start %d end %d l4 %d\n",
			np.proto, np.start, np.end, np.l4_proto);
    } else {
	// port:protocol
	if(parse_port_range(arg,&np)) {
		//l4:proto
		if(parse_l4_proto(arg,&np)) return 1;
		np.end=65535;
	}
	if(parse_n_proto(d1,&np,n->ndpi_struct)) return 1;
	DP("%s:2 proto %d start %d end %d l4 %d\n",
			np.proto, np.start, np.end, np.l4_proto);
    }
} else {
	if(parse_n_proto(arg,&np,n->ndpi_struct)) return 1;
	DP("%s:1 proto %d\n",np.proto);
}

spin_lock_bh (&n->ipq_lock);

do {
node = ndpi_patricia_search_exact(pt,prefix);
DP(node ? "%s: Found node\n":"%s: Node not found\n");
if(f_op || f_op2) { // delete
	if(!node && np.proto >= NDPI_NUM_BITS && // -xxxx any
		np.l4_proto == 2 && !np.start && !np.end) {
		if(!prefix->bitlen && pt->head) {
			ndpi_Clear_Patricia(pt,NULL,pt_node_free_data);
			break;
		}
		node = ndpi_patricia_lookup(pt, prefix);
		if(node) {
			pr_err("prefix len %d node %d\n",prefix->bitlen,node?1:0);
			ndpi_Clear_Patricia(pt,node,pt_node_free_data);
			break;
		}
		break;
	}
	if(!node) {
		pr_err("Can't delete %s with tcp or udp\n",paddr);
		break;
	}
	// -xxxx any
	if(np.proto >= NDPI_NUM_BITS && 
		np.l4_proto == 2 && !np.start && !np.end) {
		_node_free_data(node);
		ndpi_patricia_remove(pt,node);
		break;
	}
	if(!np.start && !np.end) {
	    // -xxxx proto
	    if(node->value.u.uv32.user_value == np.proto) {
		if(!node->data) {
		    _node_free_data(node);
		    ndpi_patricia_remove(pt,node);
		} else {
		  node->value.u.uv32.user_value = 0;
		}
		break;
	    }
	}
	node->data = ndpi_port_add_one_range(node->data,&np,0,n->ndpi_struct);
	break;
}
// add or change
if(!node) {
	node = ndpi_patricia_lookup(pt, prefix);
	if(!node) {
		ret = 1; break;
	}
}

if(np.proto == NDPI_PROTOCOL_UNKNOWN || 
   np.proto >= NDPI_NUM_BITS) {
	ret = 1; break;
}

if(!np.start && !np.end) {
	if(np.l4_proto == 2) {
	    // any:proto
	    node->value.u.uv32.user_value = np.proto | (np.no_dpi << 16);
	    break;
	}
	// (tcp|udp):any:proto
	np.start=1;
	np.end = 65535;
}
node->data = ndpi_port_add_one_range(node->data,&np,1,n->ndpi_struct);
} while (0);

spin_unlock_bh (&n->ipq_lock);

return ret;
}

#define SKIP_SPACE_C while(!!(c = *cmd) && (c == ' ' || c == '\t')) cmd++
#define SKIP_NONSPACE_C while(!!(c = *cmd) && (c != ' ') && (c != '\t')) cmd++

/*
 * [-]prefix ([[(tcp|udp|any):]port[-port]:]protocol)+
 */

static int _parse_ndpi_ipdef(struct ndpi_net *n,char *cmd, int family) {
int f_op = 0; // 1 if delete
char *addr,c;
ndpi_prefix_t *prefix;
int res = 0;


SKIP_SPACE_C;
if(*cmd == '#') return 0;
if(*cmd == '-') {
    f_op = 1; cmd++;
} else 
    if(*cmd == '+') cmd++;

addr = cmd;
SKIP_NONSPACE_C;
if (*cmd) *cmd++ = 0;
SKIP_SPACE_C;
if(!*addr) return 1;
DP("%s: prefix %s\n",addr);
prefix = ndpi_ascii2prefix(family,addr);
if(!prefix) {
	DP("%s: bad IP '%s'\n",addr);
	return 1;
}

while(*cmd && !res) {
	char *t = cmd;
	SKIP_NONSPACE_C;
	if(*cmd) *cmd++ = 0;
	SKIP_SPACE_C;
	if(parse_ndpi_ipdef_cmd(n,f_op,prefix,t,family,addr)) res=1;
}

ndpi_Deref_Prefix(prefix);

return res;
}

int parse_ndpi_ipdef(struct ndpi_net *n,char *cmd) {
	return _parse_ndpi_ipdef(n,cmd,AF_INET);
}
int parse_ndpi_ip6def(struct ndpi_net *n,char *cmd) {
	return _parse_ndpi_ipdef(n,cmd,AF_INET6);
}

/********************* ndpi proto *********************************/

int parse_ndpi_proto(struct ndpi_net *n,char *cmd) {
	char *v,*m,*hid;
	struct ndpi_detection_module_struct *ndpi_str = n->ndpi_struct;
	v = cmd;
	if(!*v) return 0;
/*
 * hexID hexmark/mask name
 * hexID debug 0..3
 * hexID disable
 * hexID enable (ToDo)
 * add_custom name
 * netns name
 */
	while(*v && (*v == ' ' || *v == '\t')) v++;
	if(*v == '#') return 0;
	// first word (hid)
	hid = v;
	while(*v && !(*v == ' ' || *v == '\t')) v++; // first word
	if(*v) *v++ = '\0';
	while(*v && (*v == ' ' || *v == '\t')) v++; // space
	// second word (v)
	for(m = v; *m ; m++) {
		if(*m != '#') continue;
		// remove comment 
		*m = '\0';
		// remove spaces before comment
		while (m--, m > v && (*m == ' ' || *m == '\t')) {
			*m = '\0';
		}
		break;
	}
	if(!strcmp(hid,"netns")) {
		char *e;
		// v -> name of custom protocol
		for(e = v; *e && *e != ' ' && *e != '\t'; e++) { // first space or eol
			if(*e < ' ' || strchr("/&^:;\\\"'",*e)) {
				if(*e < ' ')
				    pr_err("NDPI: can't use '\\0x%x' in NS  name\n",*e & 0xff);
				  else
				    pr_err("NDPI: can't use '%c' in NS name\n",*e & 0xff);
				return 1;
			}
		}
		if(*e) *e = '\0';
		strncpy(n->ns_name,v,sizeof(n->ns_name)-1);
		return 0;
	}

	for(m = v; *m && *m != '/';m++);

	if(*m) {
		char *x;
		*m++ = '\0';
		x = m;
		while(*x && !(*x == ' ' || *x == '\t')) x++;
		if(*x) *x++ = '\0';
	}
	if(*v) {
		u_int32_t mark,mask;

		int id=-1;
		int i,any,all,ok;

		any = !strcmp(hid,"any");
		all = !strcmp(hid,"all");
		mark = 0;
		mask = 0xffff;

		if(!strcmp(hid,"add_custom")) {
			u_int16_t e_proto;
			char *e;
			// v -> name of custom protocol
			for(e = v; *e && *e != ' ' && *e != '\t'; e++) { // first space or eol
				if(*e < ' ' || strchr("/&^:;\\\"'",*e)) {
					if(*e < ' ')
					    pr_err("NDPI: can't use '\\0x%x' in protocol name\n",*e & 0xff);
					  else
					    pr_err("NDPI: can't use '%c' in protocol name\n",*e & 0xff);
					return 1;
				}
			}
			if(*e) *e = '\0';
			e_proto = ndpi_get_proto_by_name(ndpi_str,v);
			if(e_proto != NDPI_PROTOCOL_UNKNOWN) {
				pr_err("NDPI: '%s' exists\n",v);
				return 0;
			}
			v--;
			*v = '@';
			id = ndpi_handle_rule(ndpi_str, v);
			if(id < 0) {
				pr_err("NDPI: ndpi_handle_rule error %d\n",id);
				return 1;
			}
			e_proto = ndpi_get_proto_by_name(ndpi_str,v+1);
			if(_DBG_TRACE_SPROC)
				pr_info("NDPI: add custom protocol %x\n",e_proto);
			n->mark[e_proto].mark = e_proto;
			n->mark[e_proto].mask = 0x1ff;
			return 0;
		}
		if(!any && !all) {
		    if(kstrtoint(hid,16,&id)) {
			id = ndpi_get_proto_by_name(ndpi_str,hid);
			if(id == NDPI_PROTOCOL_UNKNOWN && 
			   strcasecmp(ndpi_str->proto_defaults[id].protoName,hid)) {
				pr_err("NDPI: '%s' unknown protocol or not hexID\n",hid);
				return 1;
			}
		    } else {
			if(id < 0 || id >= NDPI_NUM_BITS) {
				pr_err("NDPI: bad id %d\n",id);
				id = -1;
			}
		    }
		}
		if(!strncmp(v,"debug",5)) {
#ifdef NDPI_ENABLE_DEBUG_MESSAGES
			u_int8_t dbg_lvl = 0;
			m = v+5;
			if(*m && *m != ' ' && *m != '\t') {
				pr_err("NDPI: invalid debug settings\n");
				return 1;
			}
			while(*m && (*m == ' ' || *m == '\t')) m++; // space
			if(*m < '0' || *m > '4') {
				pr_err("NDPI: debug level must be 0..4\n");
				return 1;
			}

			dbg_lvl = *m - '0';

			if(all || any) {
				for(i=0; i < NDPI_NUM_BITS; i++)
						n->debug_level[i] = dbg_lvl;
				set_debug_trace(n);
				return 0;
			}
			if(id >= 0 && id < NDPI_NUM_BITS) {
				n->debug_level[id] = dbg_lvl;
				set_debug_trace(n);
				return 0;
			}
			pr_err("%s:%d BUG! id %s\n",__func__,__LINE__,hid);
			return 1;
#else
			pr_err("NDPI: debug not enabled.\n");
#endif
		}
		/* FIXME enable */
		if(!strncmp(v,"disable",7)) {
			mark = 0;
			mask = 0;
			m = v;
			if(any || all) {
				pr_err("NDPI: can't disable all\n");
				return 1;
			}
		} else {
		    /* set mark/mask */
		    if(kstrtou32(v,16,&mark)) {
			pr_err("NDPI: bad mark '%s'\n",v);
			return 1;
		    }
		    if(*m) {
			if(kstrtou32(m,16,&mask)) {
				pr_err("NDPI: bad mask '%s'\n",m);
				return 1;
			}
		    }
		}
//		pr_info("NDPI: proto %s id %d mark %x mask %s\n",
//				hid,id,mark,m);
		if(atomic64_read(&n->protocols_cnt[0]) &&
			!mark && !mask) {
			pr_err("NDPI: iptables in use! Can't disable protocol\n");
			return 1;
		}
		if(id >= 0) {
			n->mark[id].mark = mark;
			if(*m) 	n->mark[id].mask = mask;
			return 0;
		}
		/* all or any */
		for(i=0; i < NDPI_NUM_BITS; i++) {
			const char *t_proto = ndpi_get_proto_by_id(ndpi_str,i);
			if(!t_proto) continue;
			if(any && !i) continue;

			n->mark[i].mark = mark;
			if(*m) 	n->mark[i].mask = mask;
			ok++;
//			pr_info("Proto %s id %02x mark %08x/%08x\n",
//					cmd,i,n->mark[i].mark,n->mark[i].mask);
		}
		return 0;
	}
	if(!strcmp(hid,"init")) {
		int i;
		for(i=0; i < NDPI_NUM_BITS; i++) {
			n->mark[i].mark = i;
			n->mark[i].mask = 0x1ff;
		}
		return 0;
	}
	pr_err("NDPI: bad cmd %s\n",hid);
	return *v ? 0:1;
}

/********************* ndpi debug *********************************/

int parse_ndpi_debug(struct ndpi_net *n,char *cmd) {
#ifdef NDPI_ENABLE_DEBUG_MESSAGES
	char *v;
	uint32_t mask;
	v = cmd;
	if(!*v) return 0;
/*
 * opt [yY1+]
 */
	while(v && *v != ' ' && *v != '\t') v++;
	if(*v) {
		*v = '\0';
		mask = dbg_ipt_opt_get(cmd);
		if(!mask) {
			pr_err("NDPI: bad debug opt %s\n",cmd);
			return 1;
		}
		*v = ' ';
		while(*v && (*v == ' ' || *v == '\t')) v++;
		if(*v) {
			if(*v == 'y' || *v == 'Y' || *v == '1' || *v == '+')
				ndpi_log_debug |= mask;
			  else 
			    if(*v == 'n' || *v == 'N' || *v == '0' || *v == '-')
				ndpi_log_debug &= ~mask;
			      else 
				return 1;
			return 0;
		}
	}
	pr_err("NDPI: bad cmd %s\n",cmd);
	return *v ? 0:1;
#else
	pr_err("NDPI: Debug is not enabled\n");
	return 1;
#endif
}

int parse_ndpi_risk(struct ndpi_net *n,char *cmd) {
	char *v;
	uint64_t risk_mask_old = n->risk_mask;
	int risk;
	v = cmd;
	if(!*v) return 0;
/*
 * num e|d
 */
	while(v && *v != ' ' && *v != '\t') v++;
	if(*v) {
		*v = '\0';
		if(kstrtoint(cmd,10,&risk)) return 1;
		if(risk < (int)NDPI_NO_RISK || risk >= (int)NDPI_MAX_RISK ) {
			pr_err("NDPI: bad risk number %s\n",cmd);
			return 1;
		}
		*v = ' ';
		while(*v && (*v == ' ' || *v == '\t')) v++;
		if(v[1]) return 1;
		if(*v) {
			if(*v == 'd')
				n->risk_mask &= ~(1ULL << risk);
			  else 
			    if(*v == 'a')
				n->risk_mask |= (1ULL << risk);
			      else 
				return 1;

			if(n->risk_mask != risk_mask_old && n->risk_names)
				risk_names(n, n->risk_names, n->risk_names_len+1);
			return 0;
		}
	}
	pr_err("NDPI: bad cmd %s\n",cmd);
	return 1;
}

/*
 * [proto] param value
 *
 */
int parse_ndpi_cfg(struct ndpi_net *n,char *cmd) {
	ndpi_cfg_error rc = NDPI_CFG_NOT_FOUND;
	int i;
	char *v, *word[4] = { NULL, NULL, NULL, NULL };
	v = cmd;
	if(!*v) return 0;
	i = 0;
	for(i = 0; *v && i < 3; i++) {
	    while(*v && (*v == ' ' || *v == '\t')) v++;
	    word[i] = v;
            while (*v && *v != ' ' && *v != '\t') v++;
	    if(*v)
		*v++ = '\0';
	}
	/* if(i == 1) { // TODO:reset settings
	    } */
	if(i == 2) {
		rc = ndpi_set_config(n->ndpi_struct, NULL, word[0], word[1]);
		if(rc != NDPI_CFG_OK)
			pr_err("NDPI: bad config rc:%d options %s = %s\n",(int)rc, word[0], word[1]);
		return rc != NDPI_CFG_OK;
	}
	if(i == 3) {
		rc = ndpi_set_config(n->ndpi_struct, word[0], word[1], word[2]);
		if(rc != NDPI_CFG_OK)
			pr_err("NDPI: bad config rc:%d options [%s] %s = %s\n",(int)rc, word[0], word[1], word[2]);
		return rc != NDPI_CFG_OK;
	}
	pr_err("NDPI: bad config options %s\n",cmd);
	return 1;
}
