#include "links.h"

#define ACCEPT_NONE	0
#define ACCEPT_ASK	1
#define ACCEPT_ALL	2

int accept_cookies = ACCEPT_ALL;

tcount cookie_id = 0;

struct cookie {
	struct cookie *next;
	struct cookie *prev;
	unsigned char *name, *value;
	unsigned char *server;
	unsigned char *path, *domain;
	time_t expires; /* zero means undefined */
	int secure;
	int id;
};

struct list_head cookies = { &cookies, &cookies };

struct c_domain {
	struct c_domain *next;
	struct c_domain *prev;
	unsigned char domain[1];
};

struct list_head c_domains = { &c_domains, &c_domains };

struct c_server {
	struct c_server *next;
	struct c_server *prev;
	int accept;
	unsigned char server[1];
};

struct list_head c_servers = { &c_servers, &c_servers };

void accept_cookie(struct cookie *);
void delete_cookie(struct cookie *);

void free_cookie(struct cookie *c)
{
	mem_free(c->name);
	if (c->value) mem_free(c->value);
	if (c->server) mem_free(c->server);
	if (c->path) mem_free(c->path);
	if (c->domain) mem_free(c->domain);
}

int check_domain_security(unsigned char *server, unsigned char *domain)
{
	size_t i, j, dl;
	int nd;
	if (domain[0] == '.') domain++;
	dl = strlen(domain);
	if (dl > strlen(server)) return 1;
	for (i = strlen(server) - dl, j = 0; server[i]; i++, j++)
		if (upcase(server[i]) != upcase(domain[j])) return 1;
	nd = 2;
	if (dl > 4 && domain[dl - 4] == '.') {
		unsigned char *tld[] = { "com", "edu", "net", "org", "gov", "mil", "int", NULL };
		for (i = 0; tld[i]; i++) if (!casecmp(tld[i], &domain[dl - 3], 3)) {
			nd = 1;
			break;
		}
	}
	for (i = 0; domain[i]; i++) if (domain[i] == '.') if (!--nd) break;
	if (nd > 0) return 1;
	return 0;
}

int set_cookie(struct terminal *term, unsigned char *url, unsigned char *str)
{
	int noval = 0;
	struct cookie *cookie;
	struct c_server *cs;
	unsigned char *p, *q, *s, *server, *date;
	if (accept_cookies == ACCEPT_NONE) return 0;
	for (p = str; *p != ';' && *p; p++) /*if (WHITECHAR(*p)) return 0*/;
	for (q = str; *q != '='; q++) if (!*q || q >= p) {
		noval = 1;
		break;
	}
	if (str == q || q + 1 == p) return 0;
	cookie = mem_alloc(sizeof(struct cookie));
	server = get_host_name(url);
	cookie->name = memacpy(str, q - str);
	cookie->value = !noval ? memacpy(q + 1, p - q - 1) : NULL;
	cookie->server = stracpy(server);
	date = parse_header_param(str, "expires");
	if (date) {
		cookie->expires = parse_http_date(date);
		/* kdo tohle napsal a proc ?? */
		/*if (! cookie->expires) cookie->expires++;*/ /* no harm and we can use zero then */
		mem_free(date);
	} else
		cookie->expires = 0;
	if (!(cookie->path = parse_header_param(str, "path"))) {
		/*unsigned char *w;*/
		cookie->path = stracpy("/");
		/*
		add_to_strn(&cookie->path, document);
		for (w = cookie->path; *w; w++) if (end_of_dir(*w)) {
			*w = 0;
			break;
		}
		for (w = cookie->path + strlen(cookie->path) - 1; w >= cookie->path; w--)
			if (*w == '/') {
				w[1] = 0;
				break;
			}
		*/
	} else {
		if (cookie->path[0] != '/') {
			add_to_strn(&cookie->path, "x");
			memmove(cookie->path + 1, cookie->path, strlen(cookie->path) - 1);
			cookie->path[0] = '/';
		}
	}
	if (!(cookie->domain = parse_header_param(str, "domain"))) cookie->domain = stracpy(server);
	if (cookie->domain[0] == '.') memmove(cookie->domain, cookie->domain + 1, strlen(cookie->domain));
	if ((s = parse_header_param(str, "secure"))) {
		cookie->secure = 1;
		mem_free(s);
	} else cookie->secure = 0;
	if (check_domain_security(server, cookie->domain)) {
		mem_free(cookie->domain);
		cookie->domain = stracpy(server);
	}
	cookie->id = cookie_id++;
	foreach (cs, c_servers) if (!strcasecmp(cs->server, server)) {
		if (cs->accept) goto ok;
		else {
			free_cookie(cookie);
			mem_free(cookie);
			mem_free(server);
			return 0;
		}
	}
	if (accept_cookies != ACCEPT_ALL) {
		free_cookie(cookie);
		mem_free(cookie);
		mem_free(server);
		return 1;
	}
	ok:
	accept_cookie(cookie);
	mem_free(server);
	return 0;
}

void accept_cookie(struct cookie *c)
{
	struct c_domain *cd;
	struct cookie *d, *e;
	foreach(d, cookies) if (!strcasecmp(d->name, c->name) && !strcasecmp(d->domain, c->domain)) {
		e = d;
		d = d->prev;
		del_from_list(e);
		free_cookie(e);
		mem_free(e);
	}
	if (c->value && !strcasecmp(c->value, "deleted")) {
		free_cookie(c);
		mem_free(c);
		return;
	}
	add_to_list(cookies, c);
	foreach(cd, c_domains) if (!strcasecmp(cd->domain, c->domain)) return;
	cd = mem_alloc(sizeof(struct c_domain) + strlen(c->domain) + 1);
	strcpy(cd->domain, c->domain);
	add_to_list(c_domains, cd);
}

void delete_cookie(struct cookie *c)
{
	struct c_domain *cd;
	struct cookie *d;
	foreach(d, cookies) if (!strcasecmp(d->domain, c->domain)) goto x;
	foreach(cd, c_domains) if (!strcasecmp(cd->domain, c->domain)) {
		del_from_list(cd);
		mem_free(cd);
		break;
	}
	x:
	del_from_list(c);
	free_cookie(c);
	mem_free(c);
}

struct cookie *find_cookie_id(void *idp)
{
	int id = (int)idp;
	struct cookie *c;
	foreach(c, cookies) if (c->id == id) return c;
	return NULL;
}

void reject_cookie(void *idp)
{
	struct cookie *c;
	if (!(c = find_cookie_id(idp))) return;
	delete_cookie(c);
}

void cookie_default(void *idp, int a)
{
	struct cookie *c;
	struct c_server *s;
	if (!(c = find_cookie_id(idp))) return;
	foreach(s, c_servers) if (!strcasecmp(s->server, c->server)) goto found;
	s = mem_alloc(sizeof(struct c_server) + strlen(c->server) + 1);
	strcpy(s->server, c->server);
	add_to_list(c_servers, s);
	found:
	s->accept = a;
}

void accept_cookie_always(void *idp)
{
	cookie_default(idp, 1);
}

void accept_cookie_never(void *idp)
{
	cookie_default(idp, 0);
	reject_cookie(idp);
}

int is_in_domain(unsigned char *d, unsigned char *s)
{
	int dl = strlen(d);
	int sl = strlen(s);
	if (dl > sl) return 0;
	if (dl == sl) return !strcasecmp(d, s);
	if (s[sl - dl - 1] != '.') return 0;
	return !casecmp(d, s + sl - dl, dl);
}

int is_path_prefix(unsigned char *d, unsigned char *s)
{
	int dl = strlen(d);
	int sl = strlen(s);
	if (!dl) return 1;
	if (dl > sl) return 0;
	if (memcmp(d, s, dl)) return 0;
	return d[dl - 1] == '/' || !s[dl] || s[dl] == '/' || s[dl] == POST_CHAR || s[dl] == '?' || s[dl] == '&';
}

int cookie_expired(struct cookie *c)	/* parse_http_date is broken */
{
  	return 0 && (c->expires && c->expires < time(NULL));
}

void send_cookies(unsigned char **s, int *l, unsigned char *url)
{
	int nc = 0;
	struct c_domain *cd;
	struct cookie *c, *d;
	unsigned char *server = get_host_name(url);
	unsigned char *data = get_url_data(url);
	if (data > url) data--;
	foreach (cd, c_domains) if (is_in_domain(cd->domain, server)) goto ok;
	mem_free(server);
	return;
	ok:
	foreach (c, cookies) if (is_in_domain(c->domain, server)) if (is_path_prefix(c->path, data)) {
		if (cookie_expired(c)) {
			d = c;
			c = c->prev;
			del_from_list(d);
			free_cookie(d);
			mem_free(d);
			continue;
		}
		if (c->secure && casecmp(url, "https://", 8)) continue;
		if (!nc) add_to_str(s, l, "Cookie: "), nc = 1;
		else add_to_str(s, l, "; ");
		add_to_str(s, l, c->name);
		if (c->value) {
			add_to_str(s, l, "=");
			add_to_str(s, l, c->value);
		}
	}
	if (nc) add_to_str(s, l, "\r\n");
	mem_free(server);
}

void init_cookies()
{
	/* !!! FIXME: read cookies */
}

void cleanup_cookies()
{
	struct cookie *c;
	free_list(c_domains);
	/* !!! FIXME: save cookies */
	foreach (c, cookies) free_cookie(c);
	free_list(cookies);
}

