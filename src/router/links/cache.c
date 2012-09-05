#include "links.h"

struct list_head cache = {&cache, &cache};

long cache_size;

int cache_count = 0;

long cache_info(int type)
{
	int i = 0;
	struct cache_entry *ce;
	switch (type) {
		case CI_BYTES:
			return cache_size;
		case CI_FILES:
			foreach(ce, cache) i++;
			return i;
		case CI_LOCKED:
			foreach(ce, cache) i += !!ce->refcount;
			return i;
		case CI_LOADING:
			foreach(ce, cache) i += is_entry_used(ce);
			return i;
		case CI_LIST:
			return (long) &cache;
		default:
			internal("cache_info: bad request");
	}
	return 0;
}

unsigned char *extract_proxy(unsigned char *url)
{
	char *a;
	if (strlen(url) < 8 || casecmp(url, "proxy://", 8)) return url;
	if (!(a = strchr(url + 8, '/'))) return url;
	return a + 1;
}

int find_in_cache(unsigned char *url, struct cache_entry **f)
{
	struct cache_entry *e;
	url = extract_proxy(url);
	foreach(e, cache) if (!strcmp(e->url, url)) {
		e->refcount++;
		del_from_list(e);
		add_to_list(cache, e);
		*f = e;
		return 0;
	}
	return -1;
}

int get_cache_entry(unsigned char *url, struct cache_entry **f)
{
	struct cache_entry *e;
	if (!find_in_cache(url, f)) return 0;
	shrink_memory(0);
	url = extract_proxy(url);
	e = mem_alloc(sizeof(struct cache_entry));
	memset(e, 0, sizeof(struct cache_entry));
	e->url = mem_alloc(strlen(url) + 1);
	strcpy(e->url, url);
	e->length = 0;
	e->incomplete = 1;
	e->data_size = 0;
	init_list(e->frag);
	e->count = cache_count++;
	e->refcount = 1;
	add_to_list(cache, e);
	*f = e;
	return 0;
}

#define sf(x) e->data_size += (x), cache_size += (x)

int page_size = 4096;

#define C_ALIGN(x) ((((x) + sizeof(struct fragment) + 64) | (page_size - 1)) - sizeof(struct fragment) - 64)

int add_fragment(struct cache_entry *e, off_t offset, unsigned char *data, off_t length)
{
	struct fragment *f;
	struct fragment *nf;
	int a = 0;
	int trunc = 0;
	volatile off_t ca;
	if (!length) return 0;
	e->incomplete = 1;
	if (offset + length < 0 || offset + length < offset) overalloc();
	if (offset + (off_t)C_ALIGN(length) < 0 || offset + (off_t)C_ALIGN(length) < offset) overalloc();
	if (e->length < offset + length) e->length = offset + length;
	e->count = cache_count++;
	foreach(f, e->frag) {
		if (f->offset > offset) break;
		if (f->offset <= offset && f->offset+f->length >= offset) {
			if (offset+length > f->offset+f->length) {
				if (memcmp(f->data+offset-f->offset, data, f->offset+f->length-offset)) trunc = 1;
				a = 1; /* !!! FIXME */
				if (offset-f->offset+length <= f->real_length) {
					sf((offset+length) - (f->offset+f->length));
					f->length = offset-f->offset+length;
				}
				else {
					sf(-(f->offset+f->length-offset));
					f->length = offset-f->offset;
					f = f->next;
					break;
				}
			} else {
				if (memcmp(f->data+offset-f->offset, data, length)) trunc = 1;
			}
			memcpy(f->data+offset-f->offset, data, length);
			goto ch_o;
		}
	}
/* Intel C 9 has a bug and miscompiles this statement (< 0 test is true) */
	/*if (C_ALIGN(length) > MAXINT - sizeof(struct fragment) || C_ALIGN(length) < 0) overalloc();*/
	ca = C_ALIGN(length);
	if (ca > MAXINT - (int)sizeof(struct fragment) || ca < 0) overalloc();
	nf = mem_alloc(sizeof(struct fragment) + ca);
	a = 1;
	sf(length);
	nf->offset = offset;
	nf->length = length;
	nf->real_length = C_ALIGN(length);
	memcpy(nf->data, data, length);
	add_at_pos(f->prev, nf);
	f = nf;
	ch_o:
	while ((void *)f->next != &e->frag && f->offset+f->length > f->next->offset) {
		if (f->offset+f->length < f->next->offset+f->next->length) {
			nf = mem_realloc(f, sizeof(struct fragment)+f->next->offset-f->offset+f->next->length);
			nf->prev->next = nf;
			nf->next->prev = nf;
			f = nf;
			if (memcmp(f->data+f->next->offset-f->offset, f->next->data, f->offset+f->length-f->next->offset)) trunc = 1;
			memcpy(f->data+f->length, f->next->data+f->offset+f->length-f->next->offset, f->next->offset+f->next->length-f->offset-f->length);
			sf(f->next->offset+f->next->length-f->offset-f->length);
			f->length = f->real_length = f->next->offset+f->next->length-f->offset;
		} else {
			if (memcmp(f->data+f->next->offset-f->offset, f->next->data, f->next->length)) trunc = 1;
		}
		nf = f->next;
		del_from_list(nf);
		sf(-nf->length);
		mem_free(nf);
	}
	if (trunc) truncate_entry(e, offset + length, 0);
	/*{
		foreach(f, e->frag) fprintf(stderr, "%d, %d, %d\n", f->offset, f->length, f->real_length);
		debug("a-");
	}*/
	return a;
}

void defrag_entry(struct cache_entry *e)
{
	struct fragment *f, *g, *h, *n, *x;
	off_t l;
	if (list_empty(e->frag)) return;
	f = e->frag.next;
	if (f->offset) return;
	for (g = f->next; g != (void *)&e->frag && g->offset <= g->prev->offset+g->prev->length; g = g->next) if (g->offset < g->prev->offset+g->prev->length) {
		internal("fragments overlay");
		return;
	}
	if (g == f->next && f->length == f->real_length) return;
	for (l = 0, h = f; h != g; h = h->next) l += h->length;
	if (l > MAXINT - (int)sizeof(struct fragment) || l < 0) overalloc();
	n = mem_alloc(sizeof(struct fragment) + l);
	n->offset = 0;
	n->length = l;
	n->real_length = l;
	/*{
		struct fragment *f;
		foreach(f, e->frag) fprintf(stderr, "%d, %d, %d\n", f->offset, f->length, f->real_length);
		debug("d1-");
	}*/
	for (l = 0, h = f; h != g; h = h->next) {
		memcpy(n->data + l, h->data, h->length);
		l += h->length;
		x = h;
		h = h->prev;
		del_from_list(x);
		mem_free(x);
	}
	add_to_list(e->frag, n);
	/*{
		foreach(f, e->frag) fprintf(stderr, "%d, %d, %d\n", f->offset, f->length, f->real_length);
		debug("d-");
	}*/
}

void truncate_entry(struct cache_entry *e, off_t off, int final)
{
	int modified = 0;
	struct fragment *f, *g;
	if (e->length > off) e->length = off, e->incomplete = 1;
	foreach(f, e->frag) {
		if (f->offset >= off) {
			del:
			while ((void *)f != &e->frag) {
				modified = 1;
				sf(-f->length);
				g = f->next;
				del_from_list(f);
				mem_free(f);
				f = g;
			}
			goto ret;
		}
		if (f->offset + f->length > off) {
			modified = 1;
			sf(-(f->offset + f->length - off));
			f->length = off - f->offset;
			if (final) {
				g = mem_realloc(f, sizeof(struct fragment) + f->length);
				g->next->prev = g;
				g->prev->next = g;
				f = g;
				f->real_length = f->length;
			}
			f = f->next;
			goto del;
		}
	}
	ret:
	if (modified) {
		e->count = cache_count++;
	}
}

void free_entry_to(struct cache_entry *e, off_t off)
{
	struct fragment *f, *g;
	e->incomplete = 1;
	foreach(f, e->frag) {
		if (f->offset + f->length <= off) {
			sf(-f->length);
			g = f;
			f = f->prev;
			del_from_list(g);
			mem_free(g);
		} else if (f->offset < off) {
			sf(f->offset - off);
			memmove(f->data, f->data + (off - f->offset), f->length -= off - f->offset);
			f->offset = off;
		} else break;
	}
}

void delete_entry_content(struct cache_entry *e)
{
	e->count = cache_count++;
	free_list(e->frag);
	e->length = 0;
	e->incomplete = 1;
	if ((cache_size -= e->data_size) < 0) {
		internal("cache_size underflow: %ld", cache_size);
		cache_size = 0;
	}
	e->data_size = 0;
	if (e->last_modified) {
		mem_free(e->last_modified);
		e->last_modified = NULL;
	}
}

void delete_cache_entry(struct cache_entry *e)
{
	if (e->refcount) internal("deleteing locked cache entry");
#ifdef DEBUG
	if (is_entry_used(e)) internal("deleting loading cache entry");
#endif
	delete_entry_content(e);
	del_from_list(e);
	mem_free(e->url);
	if (e->head) mem_free(e->head);
	if (e->last_modified) mem_free(e->last_modified);
	if (e->redirect) mem_free(e->redirect);
#ifdef HAVE_SSL
	if (e->ssl_info) mem_free(e->ssl_info);
#endif
	mem_free(e);
}

void garbage_collection(int u)
{
	struct cache_entry *e, *f;
	long ncs = cache_size;
	long ccs = 0;
	if (!u && cache_size <= memory_cache_size) return;
	foreach(e, cache) {
		if (e->refcount || is_entry_used(e)) if ((ncs -= e->data_size) < 0) {
			internal("cache_size underflow: %ld", ncs);
			ncs = 0;
		}
		ccs += e->data_size;
	}
	if (ccs != cache_size) internal("cache size badly computed: %ld != %ld", cache_size, ccs), cache_size = ccs;
	if (!u && ncs <= memory_cache_size) return;
	for (e = cache.prev; (void *)e != &cache; e = e->prev) {
		if (!u && ncs <= (longlong)memory_cache_size * MEMORY_CACHE_GC_PERCENT) goto g;
		if (e->refcount || is_entry_used(e)) {
			e->tgc = 0;
			continue;
		}
		e->tgc = 1;
		if ((ncs -= e->data_size) < 0) {
			internal("cache_size underflow: %ld", ncs);
			ncs = 0;
		}
	}
	if (/*!no &&*/ ncs) internal("cache_size(%ld) is larger than size of all objects(%ld)", cache_size, cache_size - ncs);
	g:
	if ((void *)(e = e->next) == &cache) return;
	if (!u) for (f = e; (void *)f != &cache; f = f->next) {
		if (ncs + f->data_size <= (longlong)memory_cache_size * MEMORY_CACHE_GC_PERCENT) {
			ncs += f->data_size;
			f->tgc = 0;
		}
	}
	for (f = e; (void *)f != &cache;) {
		f = f->next;
		if (f->prev->tgc) delete_cache_entry(f->prev);
	}
	/*if (!no && cache_size > (longlong)memory_cache_size * MEMORY_CACHE_GC_PERCENT) {
		internal("garbage collection doesn't work, cache size %ld", cache_size);
	}*/
}

void init_cache(void)
{
#ifdef HAVE_GETPAGESIZE
	int getpg = getpagesize();
	if (getpg > 0 && getpg < 0x10000 && !(getpg & (getpg - 1))) page_size = getpg;
#endif
}

