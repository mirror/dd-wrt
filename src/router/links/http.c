#include "links.h"

struct http_connection_info {
	int bl_flags;
	int http10;
	int close;
	off_t length;
	int version;
	int chunk_remaining;
};

unsigned char *parse_http_header(unsigned char *head, unsigned char *item, unsigned char **ptr)
{
	unsigned char *i, *f, *g, *h;
	if (!head) return NULL;
	h = NULL;
	for (f = head; *f; f++) {
		if (*f != 10) continue;
		f++;
		for (i = item; *i && *f; i++, f++)
			if (upcase(*i) != upcase(*f)) goto cont;
		if (!*f) break;
		if (f[0] == ':') {
			while (f[1] == ' ') f++;
			for (g = ++f; *g >= ' '; g++) ;
			while (g > f && g[-1] == ' ') g--;
			if (h) mem_free(h);
			h = mem_alloc(g - f + 1);
			memcpy(h, f, g - f);
			h[g - f] = 0;
			if (ptr) {
				*ptr = f;
				break;
			}
			return h;
		}
		cont:;
		f--;
	}
	return h;
}

unsigned char *parse_header_param(unsigned char *x, unsigned char *e)
{
	unsigned char u;
	size_t le = strlen(e);
	int lp;
	unsigned char *y = x;
	a:
	if (!(y = strchr(y, ';'))) return NULL;
	while (*y && (*y == ';' || *y <= ' ')) y++;
	if (strlen(y) < le) return NULL;
	if (casecmp(y, e, le)) goto a;
	y += le;
	while (*y && (*y <= ' ' || *y == '=')) y++;
	u = ';';
	if (*y == '\'' || *y == '"') u = *y++;
	lp = 0;
	while (y[lp] >= ' ' && y[lp] != u) {
		lp++;
		if (lp == MAXINT) overalloc();
	}
	return memacpy(y, lp);
}

/*
 Parse string param="value", return value as new string
 or NULL if any error.
*/
unsigned char *get_param(unsigned char *e, unsigned char *name)
{
	unsigned char *n, *start;
	int i = 0;
again:	
	while (*e && upcase(*e++) != upcase(*name));
	if (!*e) return NULL;
	n = name + 1;
	while (*n && upcase(*e) == upcase(*n)) e++, n++;
	if (*n) goto again;
	while (WHITECHAR(*e)) e++;
	if (*e++ != '=') return NULL;
	while (WHITECHAR(*e)) e++;

	start = e;
	if (!U(*e)) while (*e && !WHITECHAR(*e)) e++;
	else {
		char uu = *e++;
		start++;
		while (*e != uu) {
			if (!*e) return NULL;
			e++;
		}
	}
	
	while (start < e && *start == ' ') start++;
	while (start < e && *(e - 1) == ' ') e--;
	if (start == e) return NULL;
	
	n = mem_alloc(e - start + 1);
	while (start < e) {
		if (*start < ' ') n[i] = '.';
		else n[i] = *start;
		i++; start++;
	}
	n[i] = 0;
	return n;
}

static int get_http_code(unsigned char *head, int *code, int *version)
{
	while (head[0] == ' ') head++;
	if (upcase(head[0]) != 'H' || upcase(head[1]) != 'T' || upcase(head[2]) != 'T' ||
	    upcase(head[3]) != 'P') return -1;
	if (head[4] == '/' && head[5] >= '0' && head[5] <= '9'
	 && head[6] == '.' && head[7] >= '0' && head[7] <= '9' && head[8] <= ' ') {
		*version = (head[5] - '0') * 10 + head[7] - '0';
	} else *version = 0;
	for (head += 4; *head > ' '; head++) ;
	if (*head++ != ' ') return -1;
	if (head[0] < '1' || head [0] > '9' || head[1] < '0' || head[1] > '9' ||
	    head[2] < '0' || head [2] > '9') return -1;
	*code = (head[0]-'0')*100 + (head[1]-'0')*10 + head[2]-'0';
	return 0;
}

struct {
	unsigned char *name;
	int bugs;
} buggy_servers[] = {
	{ "mod_czech/3.1.0", BL_HTTP10 },
	{ "Purveyor", BL_HTTP10 },
	{ "Netscape-Enterprise", BL_HTTP10 | BL_NO_ACCEPT_LANGUAGE },
	{ "Apache Coyote", BL_HTTP10 },
	{ "lighttpd", BL_HTTP10 },
	{ "FORPSI", BL_NO_RANGE },
	{ "Sausalito", BL_HTTP10 },
	{ NULL, 0 }
};

int check_http_server_bugs(unsigned char *url, struct http_connection_info *info, unsigned char *head)
{
	unsigned char *server;
	int i, bugs;
	if (!http_bugs.allow_blacklist || info->http10) return 0;
	if (!(server = parse_http_header(head, "Server", NULL))) return 0;
	bugs = 0;
	for (i = 0; buggy_servers[i].name; i++) if (strstr(server, buggy_servers[i].name)) bugs |= buggy_servers[i].bugs;
	mem_free(server);
	if (bugs && (server = get_host_name(url))) {
		add_blacklist_entry(server, bugs);
		mem_free(server);
		return bugs & ~BL_NO_RANGE;
	}
	return 0;	
}

void http_end_request(struct connection *c, int notrunc)
{
	if (c->state == S_OK) {
		if (c->cache) {
			if (!notrunc) truncate_entry(c->cache, c->from, 1);
			c->cache->incomplete = 0;
		}
	}
	if (c->info && !((struct http_connection_info *)c->info)->close 
#ifdef HAVE_SSL
	&& (!c->ssl) /* We won't keep alive ssl connections */
#endif
	&& (!http_bugs.bug_post_no_keepalive || !strchr(c->url, POST_CHAR))) {
		add_keepalive_socket(c, HTTP_KEEPALIVE_TIMEOUT);
	} else abort_connection(c);
}

void http_send_header(struct connection *);

void http_func(struct connection *c)
{
	/*setcstate(c, S_CONN);*/
	/*set_timeout(c);*/
	if (get_keepalive_socket(c)) {
		int p;
		if ((p = get_port(c->url)) == -1) {
			setcstate(c, S_INTERNAL);
			abort_connection(c);
			return;
		}
		make_connection(c, p, &c->sock1, http_send_header);
	} else http_send_header(c);
}

void proxy_func(struct connection *c)
{
	http_func(c);
}

void http_get_header(struct connection *);

void http_send_header(struct connection *c)
{
	static unsigned char *accept_charset = NULL;
	struct http_connection_info *info;
	int http10 = http_bugs.http10;
	struct cache_entry *e = NULL;
	unsigned char *hdr;
	unsigned char *h, *u, *uu, *sp;
	int l = 0;
	int la;
	unsigned char *post;
	unsigned char *host = upcase(c->url[0]) != 'P' ? c->url : get_url_data(c->url);
	set_timeout(c);
	info = mem_alloc(sizeof(struct http_connection_info));
	memset(info, 0, sizeof(struct http_connection_info));
	c->info = info;
	if ((h = get_host_name(host))) {
		info->bl_flags = get_blacklist_flags(h);
		mem_free(h);
	}
	if (info->bl_flags & BL_HTTP10) http10 = 1;
	info->http10 = http10;
	post = strchr(c->url, POST_CHAR);
	if (post) post++;
	hdr = init_str();
	if (!post) add_to_str(&hdr, &l, "GET ");
	else {
		add_to_str(&hdr, &l, "POST ");
		c->unrestartable = 2;
	}
	if (upcase(c->url[0]) != 'P') add_to_str(&hdr, &l, "/");
	if (!(u = get_url_data(c->url))) {
		mem_free(hdr);
		setcstate(c, S_BAD_URL);
		http_end_request(c, 0);
		return;
	}
	if (post && post < u) {
		mem_free(hdr);
		setcstate(c, S_BAD_URL);
		http_end_request(c, 0);
		return;
	}
	if (!post) uu = stracpy(u);
	else uu = memacpy(u, post - u - 1);
	a:
	for (sp = uu; *sp; sp++) if (*sp <= ' ') {
		unsigned char *nu = mem_alloc(strlen(uu) + 3);
		memcpy(nu, uu, sp - uu);
		sprintf(nu + (sp - uu), "%%%02X", (int)*sp);
		strcat(nu, sp + 1);
		mem_free(uu);
		uu = nu;
		goto a;
	}
	add_to_str(&hdr, &l, uu);
	mem_free(uu);
	if (!http10) add_to_str(&hdr, &l, " HTTP/1.1\r\n");
	else add_to_str(&hdr, &l, " HTTP/1.0\r\n");
	if ((h = get_host_name(host))) {
		add_to_str(&hdr, &l, "Host: ");
		add_to_str(&hdr, &l, h);
		mem_free(h);
		if ((h = get_port_str(host))) {
			add_to_str(&hdr, &l, ":");
			add_to_str(&hdr, &l, h);
			mem_free(h);
		}
		add_to_str(&hdr, &l, "\r\n");
	}
	add_to_str(&hdr, &l, "User-Agent: Links (" VERSION_STRING "; ");
	add_to_str(&hdr, &l, system_name);
	add_to_str(&hdr, &l, "; ");
	if (!list_empty(terminals)) {
		add_to_str(&hdr, &l, "text");
	} else {
		add_to_str(&hdr, &l, "dump");
	}
	add_to_str(&hdr, &l, ")\r\n");
	add_to_str(&hdr, &l, "Accept: */*\r\n");
	if (!(accept_charset)) {
		int i;
		unsigned char *cs, *ac;
		int aclen = 0;
		ac = init_str();
		for (i = 0; (cs = get_cp_mime_name(i)); i++) {
			if (aclen) add_to_str(&ac, &aclen, ", ");
			else add_to_str(&ac, &aclen, "Accept-Charset: ");
			add_to_str(&ac, &aclen, cs);
		}
		if (aclen) add_to_str(&ac, &aclen, "\r\n");
		if ((accept_charset = malloc(strlen(ac) + 1))) strcpy(accept_charset, ac);
		else accept_charset = "";
		mem_free(ac);
	}
	if (!(info->bl_flags & BL_NO_CHARSET) && !http_bugs.no_accept_charset) add_to_str(&hdr, &l, accept_charset);
	if (!(info->bl_flags & BL_NO_ACCEPT_LANGUAGE)) {
		add_to_str(&hdr, &l, "Accept-Language: ");
		la = l;
		add_to_str(&hdr, &l, _(TEXT_(T__ACCEPT_LANGUAGE), NULL));
		add_to_str(&hdr, &l, ", ");
		if (!strstr(hdr + la, "en,") && !strstr(hdr + la, "en;")) add_to_str(&hdr, &l, "en;q=0.2, ");
		add_to_str(&hdr, &l, "*;q=0.1\r\n");
	}
	if (!http10) {
		if (upcase(c->url[0]) != 'P') add_to_str(&hdr, &l, "Connection: ");
		else add_to_str(&hdr, &l, "Proxy-Connection: ");
		if (!post || !http_bugs.bug_post_no_keepalive) add_to_str(&hdr, &l, "Keep-Alive\r\n");
		else add_to_str(&hdr, &l, "close\r\n");
	}
	if ((e = c->cache)) {
		int code, vers;
		if (get_http_code(e->head, &code, &vers) || code >= 400) goto skip_ifmod_and_range;
		if (!e->incomplete && e->head && c->no_cache <= NC_IF_MOD &&
		    e->last_modified) {
			add_to_str(&hdr, &l, "If-Modified-Since: ");
			add_to_str(&hdr, &l, e->last_modified);
			add_to_str(&hdr, &l, "\r\n");
		}
	}
	if (c->from && (c->est_length == -1 || c->from < c->est_length) && c->no_cache < NC_IF_MOD && !(info->bl_flags & BL_NO_RANGE)) {
		add_to_str(&hdr, &l, "Range: bytes=");
		add_num_to_str(&hdr, &l, c->from);
		add_to_str(&hdr, &l, "-\r\n");
	}
	skip_ifmod_and_range:
	if (c->no_cache >= NC_PR_NO_CACHE) add_to_str(&hdr, &l, "Pragma: no-cache\r\nCache-Control: no-cache\r\n");
	if (post) {
		unsigned char *pd = strchr(post, '\n');
		if (pd) {
			add_to_str(&hdr, &l, "Content-Type: ");
			add_bytes_to_str(&hdr, &l, post, pd - post);
			add_to_str(&hdr, &l, "\r\n");
			post = pd + 1;
		}
		add_to_str(&hdr, &l, "Content-Length: ");
		add_num_to_str(&hdr, &l, strlen(post) / 2);
		add_to_str(&hdr, &l, "\r\n");
	}
	send_cookies(&hdr, &l, host);
	add_to_str(&hdr, &l, "\r\n");
	if (post) {
		while (post[0] && post[1]) {
			int h1, h2;
			h1 = post[0] <= '9' ? post[0] - '0' : post[0] >= 'A' ? upcase(post[0]) - 'A' + 10 : 0;
			if (h1 < 0 || h1 >= 16) h1 = 0;
			h2 = post[1] <= '9' ? post[1] - '0' : post[1] >= 'A' ? upcase(post[1]) - 'A' + 10 : 0;
			if (h2 < 0 || h2 >= 16) h2 = 0;
			add_chr_to_str(&hdr, &l, h1 * 16 + h2);
			post += 2;
		}
	}
	write_to_socket(c, c->sock1, hdr, l, http_get_header);
	mem_free(hdr);
	setcstate(c, S_SENT);
}

int is_line_in_buffer(struct read_buffer *rb)
{
	int l;
	for (l = 0; l < rb->len; l++) {
		if (rb->data[l] == 10) return l + 1;
		if (l < rb->len - 1 && rb->data[l] == 13 && rb->data[l + 1] == 10) return l + 2;
		if (l == rb->len - 1 && rb->data[l] == 13) return 0;
		if (rb->data[l] < ' ') return -1;
	}
	return 0;
}

void read_http_data(struct connection *c, struct read_buffer *rb)
{
	struct http_connection_info *info = c->info;
	set_timeout(c);
	if (rb->close == 2) {
		setcstate(c, S_OK);
		http_end_request(c, 0);
		return;
	}
	if (info->length != -2) {
		int l = rb->len;
		if (info->length >= 0 && info->length < l) l = info->length;
		if ((off_t)(0UL + c->from + l) < 0) {
			setcstate(c, S_LARGE_FILE);
			abort_connection(c);
			return;
		}
		c->received += l;
		if (add_fragment(c->cache, c->from, rb->data, l) == 1) c->tries = 0;
		if (info->length >= 0) info->length -= l;
		c->from += l;
		kill_buffer_data(rb, l);
		if (!info->length && !rb->close) {
			setcstate(c, S_OK);
			http_end_request(c, 0);
			return;
		}
	} else {
		next_chunk:
		if (info->chunk_remaining == -2) {
			int l;
			if ((l = is_line_in_buffer(rb))) {
				if (l == -1) {
					setcstate(c, S_HTTP_ERROR);
					abort_connection(c);
					return;
				}
				kill_buffer_data(rb, l);
				if (l <= 2) {
					setcstate(c, S_OK);
					http_end_request(c, 0);
					return;
				}
				goto next_chunk;
			}
		} else if (info->chunk_remaining == -1) {
			int l;
			if ((l = is_line_in_buffer(rb))) {
				unsigned char *de;
				long n = 0;	/* warning, go away */
				if (l != -1) n = strtol(rb->data, (char **)(void *)&de, 16);
				if (l == -1 || n < 0 || n >= MAXINT || de == rb->data) {
					setcstate(c, S_HTTP_ERROR);
					abort_connection(c);
					return;
				}
				kill_buffer_data(rb, l);
				if (!(info->chunk_remaining = n)) info->chunk_remaining = -2;
				goto next_chunk;
			}
		} else {
			int l = info->chunk_remaining;
			if (l > rb->len) l = rb->len;
			if ((off_t)(0UL + c->from + l) < 0) {
				setcstate(c, S_LARGE_FILE);
				abort_connection(c);
				return;
			}
			c->received += l;
			if (add_fragment(c->cache, c->from, rb->data, l) == 1) c->tries = 0;
			info->chunk_remaining -= l;
			c->from += l;
			kill_buffer_data(rb, l);
			if (!info->chunk_remaining && rb->len >= 1) {
				if (rb->data[0] == 10) kill_buffer_data(rb, 1);
				else {
					if (rb->data[0] != 13 || (rb->len >= 2 && rb->data[1] != 10)) {
						setcstate(c, S_HTTP_ERROR);
						abort_connection(c);
						return;
					}
					if (rb->len < 2) goto read_more;
					kill_buffer_data(rb, 2);
				}
				info->chunk_remaining = -1;
				goto next_chunk;
			}
		}
				
	}
	read_more:
	read_from_socket(c, c->sock1, rb, read_http_data);
	setcstate(c, S_TRANS);
}

int get_header(struct read_buffer *rb)
{
	int i;
	for (i = 0; i < rb->len; i++) {
		unsigned char a = rb->data[i];
		if (/*a < ' ' && a != 10 && a != 13*/!a) return -1;
		if (i < rb->len - 1 && a == 10 && rb->data[i + 1] == 10) return i + 2;
		if (i < rb->len - 3 && a == 13) {
			if (rb->data[i + 1] != 10) return -1;
			if (rb->data[i + 2] == 13) {
				if (rb->data[i + 3] != 10) return -1;
				return i + 4;
			}
		}
	}
	return 0;
}

void http_got_header(struct connection *c, struct read_buffer *rb)
{
	off_t cf;
	int state = c->state != S_PROC ? S_GETH : S_PROC;
	unsigned char *head;
	unsigned char *cookie, *ch;
	int a, h, version;
	unsigned char *d;
	struct cache_entry *e;
	struct http_connection_info *info;
	unsigned char *host = upcase(c->url[0]) != 'P' ? c->url : get_url_data(c->url);
	set_timeout(c);
	info = c->info;
	if (rb->close == 2) {
		unsigned char *h;
		if (!c->tries && (h = get_host_name(host))) {
			if (info->bl_flags & BL_NO_CHARSET) {
				del_blacklist_entry(h, BL_NO_CHARSET);
			} else {
				add_blacklist_entry(h, BL_NO_CHARSET);
				c->tries = -1;
			}
			mem_free(h);
		}
		setcstate(c, S_CANT_READ);
		retry_connection(c);
		return;
	}
	rb->close = 0;
	again:
	if ((a = get_header(rb)) == -1) {
		setcstate(c, S_HTTP_ERROR);
		abort_connection(c);
		return;
	}
	if (!a) {
		read_from_socket(c, c->sock1, rb, http_got_header);
		setcstate(c, state);
		return;
	}
	if (get_http_code(rb->data, &h, &version) || h == 101) {
		setcstate(c, S_HTTP_ERROR);
		abort_connection(c);
		return;
	}
	head = mem_alloc(a + 1);
	memcpy(head, rb->data, a); head[a] = 0;
	if (check_http_server_bugs(host, c->info, head) && is_connection_restartable(c)) {
		mem_free(head);
		setcstate(c, S_RESTART);
		retry_connection(c);
		return;
	}
	ch = head;
	while ((cookie = parse_http_header(ch, "Set-Cookie", &ch))) {
		unsigned char *host = upcase(c->url[0]) != 'P' ? c->url : get_url_data(c->url);
		set_cookie(NULL, host, cookie);
		mem_free(cookie);
	}
	if (h == 100) {
		mem_free(head);
		state = S_PROC;
		kill_buffer_data(rb, a);
		goto again;
	}
	if (h < 200) {
		mem_free(head);
		setcstate(c, S_HTTP_ERROR);
		abort_connection(c);
		return;
	}
	if (h == 304) {
		mem_free(head);
		setcstate(c, S_OK);
		http_end_request(c, 1);
		return;
	}
	if (h == 204) {
		mem_free(head);
		setcstate(c, S_HTTP_204);
		http_end_request(c, 0);
		return;
	}
	if (!c->cache) {
		if (get_cache_entry(c->url, &c->cache)) {
			mem_free(head);
			setcstate(c, S_OUT_OF_MEM);
			abort_connection(c);
			return;
		}
		c->cache->refcount--;
	}
	e = c->cache;
	if (e->head) mem_free(e->head);
	e->head = head;
#ifdef HAVE_SSL
	if (c->ssl) {
		int l = 0;
		if (e->ssl_info) mem_free(e->ssl_info);
		e->ssl_info = init_str();
		add_num_to_str(&e->ssl_info, &l, SSL_get_cipher_bits(c->ssl, NULL));
		add_to_str(&e->ssl_info, &l, "-bit ");
		add_to_str(&e->ssl_info, &l, SSL_get_cipher_version(c->ssl));
		add_to_str(&e->ssl_info, &l, " ");
		add_to_str(&e->ssl_info, &l, (unsigned  char *)SSL_get_cipher_name(c->ssl));
	}
#endif
	if (e->redirect) mem_free(e->redirect), e->redirect = NULL;
	if (h == 301 || h == 302 || h == 303 || h == 307) {
		if ((d = parse_http_header(e->head, "Location", NULL))) {
			if (e->redirect) mem_free(e->redirect);
			e->redirect = d;
			e->redirect_get = h == 303;
		}
	}
	kill_buffer_data(rb, a);
	info->close = 0;
	info->length = -1;
	info->version = version;
	if ((d = parse_http_header(e->head, "Connection", NULL)) || (d = parse_http_header(e->head, "Proxy-Connection", NULL))) {
		if (!strcasecmp(d, "close")) info->close = 1;
		mem_free(d);
	} else if (version < 11) info->close = 1;
	cf = c->from;
	c->from = 0;
	if ((d = parse_http_header(e->head, "Content-Range", NULL))) {
		if (strlen(d) > 6) {
			d[5] = 0;
			if (!(strcasecmp(d, "bytes")) && d[6] >= '0' && d[6] <= '9') {
#if defined(HAVE_STRTOLL)
				long long f = strtoll(d + 6, NULL, 10);
#elif defined(HAVE_STRTOQ)
				longlong f = strtoq(d + 6, NULL, 10);
#else
				long f = strtol(d + 6, NULL, 10);
				if (f == MAXLONG) f = -1;
#endif
				if (f >= 0 && (off_t)f >= 0 && (off_t)f == f) c->from = f;
			}
		}
		mem_free(d);
	} else if (h == 206) {
/* Hmm ... some servers send 206 partial but don't send Content-Range */
		c->from = cf;
	}
	if (cf && !c->from && !c->unrestartable) c->unrestartable = 1;
	if (c->from > cf || c->from < 0) {
		setcstate(c, S_HTTP_ERROR);
		abort_connection(c);
		return;
	}
	if ((d = parse_http_header(e->head, "Content-Length", NULL))) {
		unsigned char *ep;
#if defined(HAVE_STRTOLL)
		long long l = strtoll(d, (char **)(void *)&ep, 10);
#elif defined(HAVE_STRTOQ)
		longlong l = strtoq(d, (char **)(void *)&ep, 10);
#else
		long l = strtol(d, (char **)(void *)&ep, 10);
		if (l == MAXLONG) l = -1;
#endif
		if (!*ep && l >= 0 && (off_t)l >= 0 && (off_t)l == l) {
			if (!info->close || version >= 11) info->length = l;
			if (c->from + l >= 0) c->est_length = c->from + l;
		}
		mem_free(d);
	}
	if ((d = parse_http_header(e->head, "Accept-Ranges", NULL))) {
		if (!strcasecmp(d, "none") && !c->unrestartable) c->unrestartable = 1;
		mem_free(d);
	} else {
		if (!c->unrestartable && !c->from) c->unrestartable = 1;
	}
	if (info->bl_flags & BL_NO_RANGE && !c->unrestartable) c->unrestartable = 1;
	if ((d = parse_http_header(e->head, "Transfer-Encoding", NULL))) {
		if (!strcasecmp(d, "chunked")) {
			info->length = -2;
			info->chunk_remaining = -1;
		}
		mem_free(d);
	}
	if (!info->close && info->length == -1) info->close = 1;
	if ((d = parse_http_header(e->head, "Last-Modified", NULL))) {
		if (e->last_modified && strcasecmp(e->last_modified, d)) {
			delete_entry_content(e);
			if (c->from) {
				c->from = 0;
				mem_free(d);
				setcstate(c, S_MODIFIED);
				retry_connection(c);
				return;
			}
		}
		if (!e->last_modified) e->last_modified = d;
		else mem_free(d);
	}
	if (!e->last_modified && (d = parse_http_header(e->head, "Date", NULL)))
		e->last_modified = d;
	if (info->length == -1 || (version < 11 && info->close)) rb->close = 1;
	read_http_data(c, rb);
}

void http_get_header(struct connection *c)
{
	struct read_buffer *rb;
	set_timeout(c);
	if (!(rb = alloc_read_buffer(c))) return;
	rb->close = 1;
	read_from_socket(c, c->sock1, rb, http_got_header);
}
