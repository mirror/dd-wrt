/*
 * simple string collections.
 */

#include "ndpi_config.h"
#undef HAVE_HYPERSCAN
#include "ndpi_main.h"

#include <linux/in6.h>

#include "ndpi_strcol.h"

void str_hosts_done(hosts_str_t *h) {
int i;

    if(!h) return;
    for(i=0; i < NDPI_NUM_BITS+1; i++) {
	if(h->p[i]) kfree(h->p[i]);
    }
    kfree(h);
}


str_collect_t *str_collect_init(size_t num_start) {
str_collect_t *c;

    if(!num_start)
	    num_start =  64;

    c = (str_collect_t *)kmalloc(sizeof(str_collect_t) + num_start + 1, GFP_KERNEL);
    if(!c) 
	return c;

    c->max  = num_start;
    c->last = 0;
    c->s[0] = '\0';
    return c;
}

str_collect_t *str_collect_copy(str_collect_t *c,int add_size) {

    str_collect_t *n = str_collect_init(c->last + 1 + add_size);
    if(n) {
	memcpy((char *)n->s,(char *)c->s, c->last + 1 );
	n->last = c->last;
    }
    return n;
}

hosts_str_t *str_collect_clone( hosts_str_t *h) {
int i,s0,s1;
hosts_str_t *r;
str_collect_t *t,*n;

    if(!h) return h;
    r = str_hosts_alloc();
    if(!r) return r;

    s0 = s1 = 0;
    for(i=0; i < NDPI_NUM_BITS+1; i++) {
	t = h->p[i];
	if(!t) continue;
	if(!t->last) continue;
	n = str_collect_copy(t,0);
	if(!n) {
		str_hosts_done(r);
		return NULL;
	}
	s0++;
	s1 += n->last;
	r->p[i] = n;
    }
// pr_info("%s: protos %d, strlen sum %d\n",__func__,s0,s1);
    return r;
}

int str_collect_look(str_collect_t *c,char *str,size_t slen) {
uint32_t ln,i,ni;

if(!c || !str || !slen) return -1;
for(i=0; i < c->last;i = ni) {
	ln = (uint8_t)c->s[i];
	if(!ln) break;
	ni = i+ln+2;
	if(ln == slen && !strncmp(&c->s[i+1],str,slen)) {
		return i;
	}
}
return -1;
}

#if 0
void str_collect_dump(str_collect_t *c) {
uint32_t ln,i,ni;
char buf[256];
int li = 0;

if(!c) return;

for(i=0; i < c->last;i = ni) {
	ln = (uint8_t)c->s[i];
	if(!ln) break;
	ni = i+ln+2;
	li += snprintf(&buf[li],sizeof(buf)-li-15,"%c%u:%.30s",
			li ? ',':' ',ln,&c->s[i+1]);
	if(li >= sizeof(buf)-15) {
		snprintf(&buf[li],sizeof(buf)-li-1,",...");
		break;
	}
}
pr_info("str_collect_dump '%s' last %lu free %lu last %lu\n",
		buf,c->last, c->max-c->last, c->max);
return;
}
#endif

char *str_collect_add(str_collect_t **pc,char *str,size_t slen) {
uint32_t nsn;
str_collect_t *nc,*c = *pc;
char *rstr;

    if(slen >= 255) {
	pr_err("xt_ndpi: hostname length > 255 chars : %.60s...\n", str);
	return NULL;
    }

    nsn = slen + 3;
    if(nsn < 128) nsn = 128;

    if(c) {
	if(c->last + nsn >= c->max-1) {
		nc = str_collect_copy(c,nsn);
		if(!nc) return NULL;
		kfree(c);
		*pc = nc;
		c = nc;
	}
    } else {
	nc = str_collect_init(nsn);
	if(!nc) return NULL;
	*pc = nc;
	c = nc;
    }

    rstr = &c->s[c->last];
    *rstr++ = slen;
    strncpy(rstr,str,slen+1);
    c->last += slen + 2;
    c->s[c->last] = '\0'; // eol
    return rstr;
}

void str_collect_del(str_collect_t *c,char *str, size_t slen) {
uint32_t i,ln,ni;

    if(!c || !str) return;

/* deleting last string ? */
    if(slen < c->last) {
	i = c->last - slen - 2;
	if((uint8_t)c->s[i] == slen && !strcmp(&c->s[i+1],str)) {
		c->s[i] = '\0';
		c->last = i;
		return;
	}
    }
/* check all strings :(  */ 
    i = str_collect_look(c,str,slen);
    if(i >= 0) {
	ln = (uint8_t)c->s[i];
	ni = i+ln+2;
	if(c->last != ni)
		memmove(&c->s[i],&c->s[ni],c->last - ni);
	c->last -= ln + 2;
	c->s[c->last] = '\0';
    }
}

