#include "links.h"

struct {
	unsigned char *prot;
	int port;
	void (*func)(struct connection *);
	void (*nc_func)(struct session *, unsigned char *);
	int free_syntax;
	int need_slashes;
	int need_slash_after_host;
	int allow_post;
} protocols[]= {
		{"file", 0, file_func, NULL, 1, 1, 0, 0},
		{"https", 443, https_func, NULL, 0, 1, 1, 1},
		{"http", 80, http_func, NULL, 0, 1, 1, 1},
		{"proxy", 3128, proxy_func, NULL, 0, 1, 1, 1},
		{"ftp", 21, ftp_func, NULL, 0, 1, 1, 0},
		{"finger", 79, finger_func, NULL, 0, 1, 1, 0},
#ifndef DISABLE_SMB
		{"smb", 139, smb_func, NULL, 0, 1, 1, 0},
#endif
		{"mailto", 0, NULL, mailto_func, 0, 0, 0, 0},
		{"telnet", 0, NULL, telnet_func, 0, 0, 0, 0},
		{"tn3270", 0, NULL, tn3270_func, 0, 0, 0, 0},
		{"mms", 0, NULL, mms_func, 1, 0, 1, 0},
		{NULL, 0, NULL, NULL, 0, 0, 0, 0}
};

int check_protocol(unsigned char *p, int l)
{
	int i;
	for (i = 0; protocols[i].prot; i++)
		if (!casecmp(protocols[i].prot, p, l) && (int)strlen(protocols[i].prot) == l) {
			return i;
		}
	return -1;
}

int get_prot_info(unsigned char *prot, int *port, void (**func)(struct connection *), void (**nc_func)(struct session *ses, unsigned char *), int *allow_post)
{
	int i;
	for (i = 0; protocols[i].prot; i++)
		if (!strcasecmp(protocols[i].prot, prot)) {
			if (port) *port = protocols[i].port;
			if (func) *func = protocols[i].func;
			if (nc_func) *nc_func = protocols[i].nc_func;
			if (allow_post) *allow_post = protocols[i].allow_post;
			return 0;
		}
	return -1;
}

int parse_url(unsigned char *url, int *prlen, unsigned char **user, int *uslen, unsigned char **pass, int *palen, unsigned char **host, int *holen, unsigned char **port, int *polen, unsigned char **data, int *dalen, unsigned char **post)
{
	unsigned char *p, *q;
	unsigned char p_c[2];
	int a;
	if (prlen) *prlen = 0;
	if (user) *user = NULL;
	if (uslen) *uslen = 0;
	if (pass) *pass = NULL;
	if (palen) *palen = 0;
	if (host) *host = NULL;
	if (holen) *holen = 0;
	if (port) *port = NULL;
	if (polen) *polen = 0;
	if (data) *data = NULL;
	if (dalen) *dalen = 0;
	if (post) *post = NULL;
	if (!url || !(p = strchr(url, ':'))) return -1;
	if (prlen) *prlen = p - url;
	if ((a = check_protocol(url, p - url)) == -1) return -1;
	if (p[1] != '/' || p[2] != '/') {
		if (protocols[a].need_slashes) return -1;
		p -= 2;
	}
	if (protocols[a].free_syntax) {
		if (data) *data = p + 3;
		if (dalen) *dalen = strlen(p + 3);
		return 0;
	}
	p += 3;
	q = p + strcspn(p, "@/?");
	if (!*q && protocols[a].need_slash_after_host) return -1;
	if (*q == '@') {
		unsigned char *pp;
		while (strcspn(q + 1, "@") < strcspn(q + 1, "/?"))
			q = q + 1 + strcspn(q + 1, "@");
		pp = strchr(p, ':');
		if (!pp || pp > q) {
			if (user) *user = p;
			if (uslen) *uslen = q - p;
		} else {
			if (user) *user = p;
			if (uslen) *uslen = pp - p;
			if (pass) *pass = pp + 1;
			if (palen) *palen = q - pp - 1;
		}
		p = q + 1;
	} 
	q = p + strcspn(p, ":/?");
	if (!*q && protocols[a].need_slash_after_host) return -1;
	if (host) *host = p;
	if (holen) *holen = q - p;
	if (*q == ':') {
		unsigned char *pp = q + strcspn(q, "/");
		int cc;
		if (*pp != '/' && protocols[a].need_slash_after_host) return -1;
		if (port) *port = q + 1;
		if (polen) *polen = pp - q - 1;
		for (cc = 0; cc < pp - q - 1; cc++) if (q[cc+1] < '0' || q[cc+1] > '9') return -1;
		q = pp;
	}
	if (*q && *q != '?') q++;
	p = q;
	p_c[0] = POST_CHAR;
	p_c[1] = 0;
	q = p + strcspn(p, p_c);
	if (data) *data = p;
	if (dalen) *dalen = q - p;
	if (post) *post = *q ? q + 1 : NULL;
	return 0;
}

unsigned char *get_protocol_name(unsigned char *url)
{
	int l;
	if (parse_url(url, &l, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)) return NULL;
	return memacpy(url, l);
}

unsigned char *get_host_and_pass(unsigned char *url)
{
	unsigned char *u, *h, *p, *z, *k;
	int hl, pl;
	if (parse_url(url, NULL, &u, NULL, NULL, NULL, &h, &hl, &p, &pl, NULL, NULL, NULL)) return NULL;
	z = u ? u : h;
	k = p ? p + pl : h + hl;
	return memacpy(z, k - z);
}

unsigned char *get_host_name(unsigned char *url)
{
	unsigned char *h;
	int hl;
	if (parse_url(url, NULL, NULL, NULL, NULL, NULL, &h, &hl, NULL, NULL, NULL, NULL, NULL)) return stracpy("");
	return memacpy(h, hl);
}

unsigned char *get_user_name(unsigned char *url)
{
	unsigned char *h;
	int hl;
	if (parse_url(url, NULL, &h, &hl, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)) return NULL;
	return memacpy(h, hl);
}

unsigned char *get_pass(unsigned char *url)
{
	unsigned char *h;
	int hl;
	if (parse_url(url, NULL,NULL,  NULL, &h, &hl, NULL, NULL, NULL, NULL, NULL, NULL, NULL)) return NULL;
	return memacpy(h, hl);
}

unsigned char *get_port_str(unsigned char *url)
{
	unsigned char *h;
	int hl;
	if (parse_url(url, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &h, &hl, NULL, NULL, NULL)) return NULL;
	return hl ? memacpy(h, hl) : NULL;
}

int get_port(unsigned char *url)
{
	unsigned char *h;
	int hl;
	long n = -1;
	if (parse_url(url, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &h, &hl, NULL, NULL, NULL)) return -1;
	if (h) {
		n = strtol(h, NULL, 10);
		if (n && n < MAXINT) return n;
	}
	if ((h = get_protocol_name(url))) {
		int nn = -1;	/* against warning */
		get_prot_info(h, &nn, NULL, NULL, NULL);
		mem_free(h);
		n = nn;
	}
	return n;
}

void (*get_protocol_handle(unsigned char *url))(struct connection *)
{
	unsigned char *p;
	void (*f)(struct connection *) = NULL;
	int post = 0;
	if (!(p = get_protocol_name(url))) return NULL;
	get_prot_info(p, NULL, &f, NULL, &post);
	mem_free(p);
	if (!post && strchr(url, POST_CHAR)) return NULL;
	return f;
}

void (*get_external_protocol_function(unsigned char *url))(struct session *, unsigned char *)
{
	unsigned char *p;
	void (*f)(struct session *, unsigned char *) = NULL;
	int post = 0;
	if (!(p = get_protocol_name(url))) return NULL;
	get_prot_info(p, NULL, NULL, &f, &post);
	mem_free(p);
	if (!post && strchr(url, POST_CHAR)) return NULL;
	return f;
}

unsigned char *get_url_data(unsigned char *url)
{
	unsigned char *d;
	if (parse_url(url, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &d, NULL, NULL)) return NULL;
	return d;
}

#define dsep(x) (lo ? dir_sep(x) : (x) == '/')

void translate_directories(unsigned char *url)
{
	unsigned char *dd = get_url_data(url);
	unsigned char *s, *d;
	int lo = !casecmp(url, "file://", 7);
	if (!dd || dd == url/* || *--dd != '/'*/) return;
	if (!dsep(*dd)) dd--;
	s = dd;
	d = dd;
	r:
	if (end_of_dir(s[0])) {
		memmove(d, s, strlen(s) + 1);
		return;
	}
	if (dsep(s[0]) && s[1] == '.' && dsep(s[2])) {
		/**d++ = s[0];*/
		if (s == dd && !s[3]) goto p;
		s += 2;
		goto r;
	}
	if (dsep(s[0]) && s[1] == '.' && s[2] == '.' && (dsep(s[3]) || !s[3])) {
		while (d > dd) {
			d--;
			if (dsep(*d)) goto b;
		}
		b:
		if (!s[3]) *d++ = *s;
		s += 3;
		goto r;
	}
	p:
	if ((*d++ = *s++)) goto r;
}

void insert_wd(unsigned char **up, unsigned char *cwd)
{
	unsigned char *url = *up;
	if (!url || !cwd || !*cwd) return;
	if (casecmp(url, "file://", 7)) return;
	if (dir_sep(url[7])) return;
#ifdef DOS_FS
	if (upcase(url[7]) >= 'A' && upcase(url[7]) <= 'Z' && url[8] == ':' && dir_sep(url[9])) return;
#endif
#ifdef SPAD
	if (_is_absolute(url + 7) != _ABS_NO) return;
#endif
	url = mem_alloc(strlen(*up) + strlen(cwd) + 2);
	memcpy(url, *up, 7);
	strcpy(url + 7, cwd);
	if (!dir_sep(cwd[strlen(cwd) - 1])) strcat(url, "/");
	strcat(url, *up + 7);
	mem_free(*up);
	*up = url;
}

unsigned char *join_urls(unsigned char *base, unsigned char *rel)
{
	unsigned char *p, *n, *pp;
	int l;
	int lo = !casecmp(base, "file://", 7);
	if (rel[0] == '#' || !rel[0]) {
		n = stracpy(base);
		for (p = n; *p && *p != POST_CHAR && *p != '#'; p++) ;
		*p = 0;
		add_to_strn(&n, rel);
		translate_directories(n);
		return n;
	}
	if (rel[0] == '?' || rel[0] == '&') {
		unsigned char rj[3];
		unsigned char *d = get_url_data(base);
		if (!d) goto bad_base;
		rj[0] = rel[0];
		rj[1] = POST_CHAR;
		rj[2] = 0;
		d += strcspn(d, rj);
		n = memacpy(base, d - base);
		add_to_strn(&n, rel);
		translate_directories(n);
		return n;
	}
	if (rel[0] == '/' && rel[1] == '/') {
		unsigned char *s, *n;
		if (!(s = strstr(base, "//"))) {
			if (!(s = strchr(base, ':'))) {
				bad_base:
				internal("bad base url: %s", base);
				return NULL;
			}
			s++;
		}
		n = memacpy(base, s - base);
		add_to_strn(&n, rel);
		if (!parse_url(n, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)) {
			translate_directories(n);
			return n;
		}
		mem_free(n);
	}
	if (!casecmp("proxy://", rel, 8)) goto prx;
	if (!parse_url(rel, &l, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)) {
		n = stracpy(rel);
		translate_directories(n);
		return n;
	}
	n = stracpy(rel);
	while (n[0] && n[strlen(n) - 1] <= ' ') n[strlen(n) - 1] = 0;
	add_to_strn(&n, "/");
	if (!parse_url(n, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)) {
		translate_directories(n);
		return n;
	}
	mem_free(n);
	prx:
	if (parse_url(base, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &p, NULL, NULL) || !p) {
		goto bad_base;
	}
	if (!dsep(*p)) p--;
	if (end_of_dir(rel[0])) for (; *p; p++) {
		if (end_of_dir(*p)) break;
	} else if (!dsep(rel[0])) for (pp = p; *pp; pp++) {
		if (end_of_dir(*pp)) break;
		if (dsep(*pp)) p = pp + 1;
	}
	n = mem_alloc(p - base + strlen(rel) + 1);
	memcpy(n, base, p - base);
	strcpy(n + (p - base), rel);
	translate_directories(n);
	return n;
}

unsigned char *translate_url(unsigned char *url, unsigned char *cwd)
{
	unsigned char *ch;
	unsigned char *nu, *da;
	unsigned char *prefix;
	int sl;
	while (*url == ' ') url++;
	if (!casecmp("proxy://", url, 8)) return NULL;
	if (!parse_url(url, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &da, NULL, NULL)) {
		nu = stracpy(url);
		insert_wd(&nu, cwd);
		translate_directories(nu);
		return nu;
	}
	if (strchr(url, POST_CHAR)) return NULL;
	if (strstr(url, "://")) {
		nu = stracpy(url);
		add_to_strn(&nu, "/");
		if (!parse_url(nu, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)) {
			insert_wd(&nu, cwd);
			translate_directories(nu);
			return nu;
		}
		mem_free(nu);
	}
	ch = url + strcspn(url, ".:/@");
	prefix = "file://";
	sl = 0;
	if (*ch != ':' || *(url + strcspn(url, "/@")) == '@') {
		if (*url != '.' && *ch == '.') {
			unsigned char *f, *e;
			int i;
			for (e = ch + 1; *(f = e + strcspn(e, ".:/")) == '.'; e = f + 1) ;
			for (i = 0; i < f - e; i++) if (e[i] >= '0' && e[i] <= '9') goto http;
			if (f - e == 2) {
				http:
				prefix = "http://", sl = 1;
			} else {
				unsigned char *tld[] = { "com", "edu", "net", "org", "gov", "mil", "int", "arpa", "aero", "biz", "coop", "info", "museum", "name", "pro", "cat", "jobs", "mobi", "travel", "tel", NULL };
				for (i = 0; tld[i]; i++) if ((size_t)(f - e) == strlen(tld[i]) && !casecmp(tld[i], e, f - e)) goto http;
			}
		}
		if (*ch == '@' || *ch == ':' || !cmpbeg(url, "ftp.")) prefix = "ftp://", sl = 1;
		goto set_prefix;
		set_prefix:
		nu = stracpy(prefix);
		add_to_strn(&nu, url);
		if (sl && !strchr(url, '/')) add_to_strn(&nu, "/");
		if (parse_url(nu, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)) mem_free(nu), nu = NULL;
		else {
			insert_wd(&nu, cwd);
			translate_directories(nu);
		}
		return nu;
	}
#ifdef DOS_FS
	if (ch == url + 1) goto set_prefix;
#endif
#ifdef SPAD
	if (_is_local(url)) goto set_prefix;
#endif
	nu = memacpy(url, ch - url + 1);
	add_to_strn(&nu, "//");
	add_to_strn(&nu, ch + 1);
	if (!parse_url(nu, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)) {
		insert_wd(&nu, cwd);
		translate_directories(nu);
		return nu;
	}
	add_to_strn(&nu, "/");
	if (!parse_url(nu, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL)) {
		insert_wd(&nu, cwd);
		translate_directories(nu);
		return nu;
	}
	mem_free(nu);
	return NULL;
}

unsigned char *extract_position(unsigned char *url)
{
	unsigned char *u, *uu, *r;
	if ((u = get_url_data(url))) url = u;
	if (!(u = strchr(url, POST_CHAR))) u = url + strlen(url);
	if (!(uu = memchr(url, '#', u - url))) return NULL;
	r = mem_alloc(u - uu);
	memcpy(r, uu + 1, u - uu - 1);
	r[u - uu - 1] = 0;
	memmove(uu, u, strlen(u) + 1);
	return r;
}

void get_filename_from_url(unsigned char *url, unsigned char **s, int *l)
{
	int lo = !casecmp(url, "file://", 7);
	unsigned char *uu;
	if ((uu = get_url_data(url))) url = uu;
	*s = url;
	while (*url && !end_of_dir(*url)) {
		if (dsep(*url)) *s = url + 1;
		url++;
	}
	*l = url - *s;
}

#define accept_char(x)	((x) != '"' && (x) != '\'' && (x) != '&' && (x) != '<' && (x) != '>')
#define special_char(x)	((x) == '%' || (x) == '#')

void add_conv_str(unsigned char **s, int *l, unsigned char *b, int ll, int encode_special)
{
	for (; ll; ll--, b++) {
		if ((unsigned char)*b < ' ') continue;
		if (special_char(*b) && encode_special == 1) {
			unsigned char h[4];
			sprintf(h, "%%%02X", (unsigned)*b & 0xff);
			add_to_str(s, l, h);
		} else if (*b == '%' && encode_special <= -1 && ll > 2 && ((b[1] >= '0' && b[1] <= '9') || (b[1] >= 'A' && b[1] <= 'F') || (b[1] >= 'a' && b[1] <= 'f'))) {
			unsigned char h = 0;
			int i;
			for (i = 1; i < 3; i++) {
				if (b[i] >= '0' && b[i] <= '9') h = h * 16 + b[i] - '0';
				if (b[i] >= 'A' && b[i] <= 'F') h = h * 16 + b[i] - 'A' + 10;
				if (b[i] >= 'a' && b[i] <= 'f') h = h * 16 + b[i] - 'a' + 10;
			}
			if (h >= ' ') add_chr_to_str(s, l, h);
			ll -= 2;
			b += 2;
		} else if (accept_char(*b) || encode_special == -2) {
			add_chr_to_str(s, l, *b);
		} else {
			add_to_str(s, l, "&#");
			add_num_to_str(s, l, (int)*b);
			add_chr_to_str(s, l, ';');
		}
	}
}
