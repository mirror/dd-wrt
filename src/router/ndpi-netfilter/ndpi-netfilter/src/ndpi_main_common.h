
struct ndpi_net;

int inet_ntop_port(int family,void *ip, u_int16_t port, char *lbuf, size_t bufsize);
int ndpi_delete_acct(struct ndpi_net *n,int all);
ssize_t nflow_read(struct ndpi_net *n, char __user *buf,
	            size_t count, loff_t *ppos);

extern unsigned long  ndpi_flow_limit;
extern unsigned long  bt_hash_size;
extern unsigned long  bt6_hash_size;
extern unsigned long  bt_hash_tmo;
extern unsigned long  ndpi_enable_flow;
extern unsigned long  flow_read_debug;
extern struct kmem_cache *ct_info_cache;

#define XCHGP(a,b) { void *__c = a; a = b; b = __c; }

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5,0,0)
#define ACCESS_OK(a,b,c) access_ok(b,c)
#else
#define ACCESS_OK(a,b,c) access_ok(a,b,c)
#endif
