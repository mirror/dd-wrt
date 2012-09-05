#include "links.h"

#if defined(HAVE_GETHOSTBYNAME_BUG) || !defined(HAVE_GETHOSTBYNAME)
#define EXTERNAL_LOOKUP
#endif

struct dnsentry {
	struct dnsentry *next;
	struct dnsentry *prev;
	ttime get_time;
	ip addr;
	unsigned char name[1];
};

#ifndef THREAD_SAFE_LOOKUP
struct dnsquery *dns_queue = NULL;
#endif

struct dnsquery {
#ifndef THREAD_SAFE_LOOKUP
	struct dnsquery *next_in_queue;
#endif
	void (*fn)(void *, int);
	void *data;
	void (*xfn)(struct dnsquery *, int);
	int h;
	struct dnsquery **s;
	ip *addr;
	unsigned char name[1];
};

struct list_head dns_cache = {&dns_cache, &dns_cache};

static int get_addr_byte(unsigned char **ptr, unsigned char *res, unsigned char stp)
{
	unsigned u = 0;
	if (!(**ptr >= '0' && **ptr <= '9')) return -1;
	while (**ptr >= '0' && **ptr <= '9') {
		u = u * 10 + **ptr - '0';
		if (u >= 256) return -1;
		(*ptr)++;
	}
	if (stp != 255 && **ptr != stp) return -1;
	(*ptr)++;
	*res = u;
	return 0;
}

#ifdef EXTERNAL_LOOKUP

static int do_external_lookup(unsigned char *name, ip *host)
{
	unsigned char buffer[1024];
	unsigned char sink[16];
	int rd;
	int pi[2];
	pid_t f;
	unsigned char *n;
	if (pipe(pi) == -1)
		return -1;
	f = fork();
	if (f == -1) {
		close(pi[0]);
		close(pi[1]);
		return -1;
	}
	if (!f) {
		close(pi[0]);
		if (dup2(pi[1], 1) == -1) _exit(1);
		if (dup2(pi[1], 2) == -1) _exit(1);
		close(pi[1]);
		execlp("host", "host", name, NULL);
		execl("/usr/sbin/host", "host", name, NULL);
		_exit(1);
	}
	close(pi[1]);
	rd = hard_read(pi[0], buffer, sizeof buffer - 1);
	if (rd >= 0) buffer[rd] = 0;
	if (rd > 0) {
		while (hard_read(pi[0], sink, sizeof sink) > 0);
	}
	close(pi[0]);
	/* Don't wait for the process, we already have sigchld handler that
	 * does cleanup.
	 * waitpid(f, NULL, 0); */
	if (rd < 0) return -1;
	/*fprintf(stderr, "query: '%s', result: %s\n", name, buffer);*/
	while ((n = strstr(buffer, name))) {
		memset(n, '-', strlen(name));
	}
	for (n = buffer; n < buffer + rd; n++) {
		if (*n >= '0' && *n <= '9') {
			if (get_addr_byte(&n, ((unsigned char *)host + 0), '.')) goto skip_addr;
			if (get_addr_byte(&n, ((unsigned char *)host + 1), '.')) goto skip_addr;
			if (get_addr_byte(&n, ((unsigned char *)host + 2), '.')) goto skip_addr;
			if (get_addr_byte(&n, ((unsigned char *)host + 3), 255)) goto skip_addr;
			return 0;
skip_addr:
			if (n >= buffer + rd) break;
		}
	}
	return -1;
}

#endif

int do_real_lookup(unsigned char *name, ip *host)
{
	unsigned char *n;
	struct hostent *hst;
	if (!*name) return -1;
	for (n = name; *n; n++) if (*n != '.' && (*n < '0' || *n > '9')) goto nogethostbyaddr;
	n = name;
	if (get_addr_byte(&n, ((unsigned char *)host + 0), '.')) goto skip_addr;
	if (get_addr_byte(&n, ((unsigned char *)host + 1), '.')) goto skip_addr;
	if (get_addr_byte(&n, ((unsigned char *)host + 2), '.')) goto skip_addr;
	if (get_addr_byte(&n, ((unsigned char *)host + 3), 0)) goto skip_addr;
	return 0;
	skip_addr:
#ifdef HAVE_GETHOSTBYADDR
	if (!(hst = gethostbyaddr(name, strlen(name), AF_INET)))
#endif
	{
		nogethostbyaddr:
#ifdef HAVE_GETHOSTBYNAME
		if (!(hst = gethostbyname(name)))
#endif
		{
#ifdef EXTERNAL_LOOKUP
			return do_external_lookup(name, host);
#endif
			return -1;
		}
	}
	memcpy(host, hst->h_addr_list[0], sizeof(ip));
	return 0;
}

void lookup_fn(unsigned char *name, int h)
{
	ip host;
	if (do_real_lookup(name, &host)) return;
	write(h, &host, sizeof(ip));
}

void end_real_lookup(struct dnsquery *q)
{
	int r = 1;
	if (!q->addr || read(q->h, q->addr, sizeof(ip)) != sizeof(ip)) goto end;
	r = 0;

	end:
	set_handlers(q->h, NULL, NULL, NULL, NULL);
	close(q->h);
	q->xfn(q, r);
}

void failed_real_lookup(struct dnsquery *q)
{
	set_handlers(q->h, NULL, NULL, NULL, NULL);
	close(q->h);
	q->xfn(q, -1);
}

int do_lookup(struct dnsquery *q, int force_async)
{
	/*debug("starting lookup for %s", q->name);*/
#ifndef NO_ASYNC_LOOKUP
	if (!async_lookup && !force_async) {
#endif
		int r;
#ifndef NO_ASYNC_LOOKUP
		sync_lookup:
#endif
		r = do_real_lookup(q->name, q->addr);
		q->xfn(q, r);
		return 0;
#ifndef NO_ASYNC_LOOKUP
	} else {
		if ((q->h = start_thread((void (*)(void *, int))lookup_fn, q->name, strlen(q->name) + 1)) == -1) goto sync_lookup;
		set_handlers(q->h, (void (*)(void *))end_real_lookup, NULL, (void (*)(void *))failed_real_lookup, q);
		return 1;
	}
#endif
}

int do_queued_lookup(struct dnsquery *q)
{
#ifndef THREAD_SAFE_LOOKUP
	q->next_in_queue = NULL;
	if (!dns_queue) {
		dns_queue = q;
		/*debug("direct lookup");*/
#endif
		return do_lookup(q, 0);
#ifndef THREAD_SAFE_LOOKUP
	} else {
		/*debug("queuing lookup for %s", q->name);*/
		if (dns_queue->next_in_queue) internal("DNS queue corrupted");
		dns_queue->next_in_queue = q;
		dns_queue = q;
		return 1;
	}
#endif
}

int find_in_dns_cache(unsigned char *name, struct dnsentry **dnsentry)
{
	struct dnsentry *e;
	foreach(e, dns_cache)
		if (!strcasecmp(e->name, name)) {
			del_from_list(e);
			add_to_list(dns_cache, e);
			*dnsentry=e;
			return 0;
		}
	return -1;
}

void end_dns_lookup(struct dnsquery *q, int a)
{
	struct dnsentry *dnsentry;
	void (*fn)(void *, int);
	void *data;
	/*debug("end lookup %s", q->name);*/
#ifndef THREAD_SAFE_LOOKUP
	if (q->next_in_queue) {
		/*debug("processing next in queue: %s", q->next_in_queue->name);*/
		do_lookup(q->next_in_queue, 1);
	} else dns_queue = NULL;
#endif
	if (!q->fn || !q->addr) {
		free(q);
		return;
	}
	if (!find_in_dns_cache(q->name, &dnsentry)) {
		if (a) {
			memcpy(q->addr, &dnsentry->addr, sizeof(ip));
			a = 0;
			goto e;
		}
		del_from_list(dnsentry);
		mem_free(dnsentry);
	}
	if (a) goto e;
	dnsentry = mem_alloc(sizeof(struct dnsentry) + strlen(q->name) + 1);
	strcpy(dnsentry->name, q->name);
	memcpy(&dnsentry->addr, q->addr, sizeof(ip));
	dnsentry->get_time = get_time();
	add_to_list(dns_cache, dnsentry);
	e:
	if (q->s) *q->s = NULL;
	fn = q->fn;
	data = q->data;
	free(q);
	fn(data, a);
}

int find_host_no_cache(unsigned char *name, ip *addr, void **qp, void (*fn)(void *, int), void *data)
{
	struct dnsquery *q;
	if (!(q = (struct dnsquery *)malloc(sizeof(struct dnsquery) + strlen(name) + 1))) {
		fn(data, -1);
		return 0;
	}
	q->fn = fn;
	q->data = data;
	q->s = (struct dnsquery **)qp;
	q->addr = addr;
	strcpy(q->name, name);
	if (qp) *(struct dnsquery **) qp = q;
	q->xfn = end_dns_lookup;
	return do_queued_lookup(q);
}

int find_host(unsigned char *name, ip *addr, void **qp, void (*fn)(void *, int), void *data)
{
	struct dnsentry *dnsentry;
	if (qp) *qp = NULL;
	if (!find_in_dns_cache(name, &dnsentry)) {
		if ((uttime)get_time() - (uttime)dnsentry->get_time > DNS_TIMEOUT) goto timeout;
		memcpy(addr, &dnsentry->addr, sizeof(ip));
		fn(data, 0);
		return 0;
	}
	timeout:
	return find_host_no_cache(name, addr, qp, fn, data);
}

void kill_dns_request(void **qp)
{
	struct dnsquery *q = *qp;
	/*set_handlers(q->h, NULL, NULL, NULL, NULL);
	close(q->h);
	mem_free(q);*/
	q->fn = NULL;
	q->addr = NULL;
	*qp = NULL;
}

void shrink_dns_cache(int u)
{
	struct dnsentry *d, *e;
	foreach(d, dns_cache) if (u || (uttime)get_time() - (uttime)d->get_time > DNS_TIMEOUT) {
		e = d;
		d = d->prev;
		del_from_list(e);
		mem_free(e);
	}
}
