#include "links.h"

#ifndef DISABLE_SMB

#define SMBCLIENT	0
#define SMBC		1
#define N_CLIENTS	2

static int smb_client = 0;

struct smb_connection_info {
	int client;
	int list;
	int cl;
	int ntext;
	unsigned char text[1];
};

static void smb_got_data(struct connection *);
static void smb_got_text(struct connection *);
static void end_smb_connection(struct connection *);

void smb_func(struct connection *c)
{
	int i;
	int po[2];
	int pe[2];
	unsigned char *host, *user, *pass, *port, *data1, *data, *share, *dir;
	int datal;
	unsigned char *p;
	pid_t r;
	struct smb_connection_info *si;
	si = mem_alloc(sizeof(struct smb_connection_info) + 2);
	memset(si, 0, sizeof(struct smb_connection_info));
	c->info = si;
	si->client = smb_client;
	host = get_host_name(c->url);
	if (!host) {
		setcstate(c, S_INTERNAL);
		abort_connection(c);
		return;
	}
	if (!(user = get_user_name(c->url))) user = stracpy("");
	if (!(pass = get_pass(c->url))) pass = stracpy("");
	if (!(port = get_port_str(c->url))) port = stracpy("");
	if (!(data1 = get_url_data(c->url))) data1 = "";
	data = init_str(), datal = 0;
	add_conv_str(&data, &datal, data1, strlen(data1), -2);

	for (i = 0; data[i]; i++) if (data[i] < 32 || data[i] == ';' || (data[i] == '"' && smb_client == SMBCLIENT)) {
/* ';' shouldn't cause security problems but samba doesn't like it */
/* '"' is allowed for smbc */
		mem_free(host);
		mem_free(port);
		mem_free(user);
		mem_free(pass);
		mem_free(data);
		setcstate(c, S_BAD_URL);
		abort_connection(c);
		return;
	}

	if ((p = strchr(data, '/'))) share = memacpy(data, p - data), dir = p + 1;
	else if (*data) {
		if (!c->cache) {
			if (get_cache_entry(c->url, &c->cache)) {
				mem_free(host);
				mem_free(port);
				mem_free(user);
				mem_free(pass);
				mem_free(data);
				setcstate(c, S_OUT_OF_MEM);
				abort_connection(c);
				return;
			}
			c->cache->refcount--;
		}
		if (c->cache->redirect) mem_free(c->cache->redirect);
		c->cache->redirect = stracpy(c->url);
		c->cache->redirect_get = 1;
		add_to_strn(&c->cache->redirect, "/");
		c->cache->incomplete = 0;
		mem_free(host);
		mem_free(port);
		mem_free(user);
		mem_free(pass);
		mem_free(data);
		setcstate(c, S_OK);
		abort_connection(c);
		return;
	} else share = stracpy(""), dir = "";
	if (!*share) si->list = 1;
	else if (!*dir || dir[strlen(dir) - 1] == '/' || dir[strlen(dir) - 1] == '\\') si->list = 2;
	if (c_pipe(po)) {
		int err = errno;
		mem_free(host);
		mem_free(port);
		mem_free(user);
		mem_free(pass);
		mem_free(share);
		mem_free(data);
		setcstate(c, get_error_from_errno(err));
		abort_connection(c);
		return;
	}
	if (c_pipe(pe)) {
		int err = errno;
		mem_free(host);
		mem_free(port);
		mem_free(user);
		mem_free(pass);
		mem_free(share);
		mem_free(data);
		close(po[0]);
		close(po[1]);
		setcstate(c, get_error_from_errno(err));
		abort_connection(c);
		return;
	}
	c->from = 0;
	r = fork();
	if (r == -1) {
		int err = errno;
		mem_free(host);
		mem_free(port);
		mem_free(user);
		mem_free(pass);
		mem_free(share);
		mem_free(data);
		close(po[0]);
		close(po[1]);
		close(pe[0]);
		close(pe[1]);
		setcstate(c, get_error_from_errno(err));
		retry_connection(c);
		return;
	}
	if (!r) {
		int n;
		unsigned char *v[32];
		unsigned char *uphp;
		close_fork_tty();
		close(1);
		if (si->list) dup2(pe[1], 1);
		else dup2(po[1], 1);
		close(2);
		dup2(pe[1], 2);
		close(0);
		open("/dev/null", O_RDONLY);
		close(po[0]);
		close(pe[0]);
		close(po[1]);
		close(pe[1]);
		n = 0;
		switch (si->client) {
		case SMBCLIENT:
			v[n++] = "smbclient";
			if (!*share) {
				v[n++] = "-L";
				v[n++] = host;
			} else {
				unsigned char *s = stracpy("//");
				add_to_strn(&s, host);
				add_to_strn(&s, "/");
				add_to_strn(&s, share);
				v[n++] = s;
				if (*pass && !*user) {
					v[n++] = pass;
				}
			}
			v[n++] = "-N";
			v[n++] = "-E";
			if (*port) {
				v[n++] = "-p";
				v[n++] = port;
			}
			if (*user) {
				v[n++] = "-U";
				if (!*pass) {
					v[n++] = user;
				} else {
					unsigned char *s = stracpy(user);
					add_to_strn(&s, "%");
					add_to_strn(&s, pass);
					v[n++] = s;
				}
			}
			if (*share) {
				if (!*dir || dir[strlen(dir) - 1] == '/' || dir[strlen(dir) - 1] == '\\') {
					if (*dir) {
						v[n++] = "-D";
						v[n++] = dir;
					}
					v[n++] = "-c";
					v[n++] = "ls";
				} else {
					unsigned char *ss;
					unsigned char *s = stracpy("get \"");
					add_to_strn(&s, dir);
					add_to_strn(&s, "\" -");
					while ((ss = strchr(s, '/'))) *ss = '\\';
					v[n++] = "-c";
					v[n++] = s;
				}
			}
			break;
		case SMBC:
			v[n++] = "smbc";
			uphp = stracpy("");
			if (*user) {
				add_to_strn(&uphp, user);
				if (*pass) {
					add_to_strn(&uphp, ":");
					add_to_strn(&uphp, pass);
				}
				add_to_strn(&uphp, "@");
			}
			add_to_strn(&uphp, host);
			if (*port) {
				add_to_strn(&uphp, ":");
				add_to_strn(&uphp, port);
			}
			if (!*share) {
				v[n++] = "-L";
				v[n++] = uphp;
			} else {
				add_to_strn(&uphp, "/");
				add_to_strn(&uphp, share);
				if (!*dir || dir[strlen(dir) - 1] == '/' || dir[strlen(dir) - 1] == '\\') {
					add_to_strn(&uphp, "/");
					add_to_strn(&uphp, dir);
					v[n++] = uphp;
					v[n++] = "-c";
					v[n++] = "ls";
				} else {
					unsigned char *d = init_str();
					int dl = 0;
					unsigned char *dp = dir;
					v[n++] = uphp;
					v[n++] = "-c";
					add_to_str(&d, &dl, "pipe cat ");
					while (*dp) {
						if (*dp <= ' ' || *dp == '\\' || *dp == '"' || *dp == '\'' || *dp == '*' || *dp == '?') add_chr_to_str(&d, &dl, '\\');
						add_chr_to_str(&d, &dl, *dp);
						dp++;
					}
					v[n++] = d;
				}
			}
			break;
		default:
			internal("unsuported smb client");
		}
		v[n++] = NULL;
		execvp(v[0], (void *)v);
		fprintf(stderr, "client not found");
		fflush(stderr);
		_exit(1);
	}
	c->pid = r;
	mem_free(host);
	mem_free(port);
	mem_free(user);
	mem_free(pass);
	mem_free(share);
	mem_free(data);
	c->sock1 = po[0];
	c->sock2 = pe[0];
	close(po[1]);
	close(pe[1]);
	set_handlers(po[0], (void (*)(void *))smb_got_data, NULL, NULL, c);
	set_handlers(pe[0], (void (*)(void *))smb_got_text, NULL, NULL, c);
	setcstate(c, S_CONN);
}

static int smbc_get_num(unsigned char *text, int *ptr, off_t *res)
{
	off_t num;
	int dec, dec_order, unit;
	int was_digit;
	int i = *ptr;
	while (text[i] == ' ' || text[i] == '\t') i++;
	was_digit = 0;
	num = 0;
	while (text[i] >= '0' && text[i] <= '9') {
		num = num * 10 + text[i] - '0';
		i++;
		was_digit = 1;
	}
	dec = 0; dec_order = 1;
	if (text[i] == '.') {
		i++;
		while (text[i] >= '0' && text[i] <= '9') {
			if (dec_order < 1000000) {
				dec = dec * 10 + text[i] - '0';
				dec_order *= 10;
			}
			i++;
			was_digit = 1;
		}
	}
	if (!was_digit) return -1;
	if (upcase(text[i]) == 'B') unit = 1;
	else if (upcase(text[i]) == 'K') unit = 1 << 10;
	else if (upcase(text[i]) == 'M') unit = 1 << 20;
	else if (upcase(text[i]) == 'G') unit = 1 << 30;
	else return -1;
	i++;
	*ptr = i;
	*res = num * unit + (double)dec * ((double)unit / (double)dec_order);
	return 0;
}

static void smb_read_text(struct connection *c, int sock)
{
	int r;
	struct smb_connection_info *si = c->info;
	if ((unsigned)sizeof(struct smb_connection_info) + si->ntext + page_size + 2 > MAXINT) overalloc();
	si = mem_realloc(si, sizeof(struct smb_connection_info) + si->ntext + page_size + 2);
	c->info = si;
	r = read(sock, si->text + si->ntext, page_size);
	if (r == -1) {
		setcstate(c, get_error_from_errno(errno));
		retry_connection(c);
		return;
	}
	if (r == 0) {
		if (!si->cl) {
			si->cl = 1;
			set_handlers(sock, NULL, NULL, NULL, NULL);
			return;
		}
		end_smb_connection(c);
		return;
	}
	si->ntext += r;
	if (!c->from) setcstate(c, S_GETH);
	if (c->from && si->client == SMBC) {
		int lasti = 0;
		int i = 0;
		si->text[si->ntext] = 0;
		for (i = 0; i + 7 < si->ntext; i++) {
			nexti:
			if ((si->text[i] == '\n' || si->text[i] == '\r') && (si->text[i + 1] == ' ' || (si->text[i + 1] >= '0' && si->text[i + 1] <= '9')) && ((si->text[i + 2] == ' ' && si->text[i + 1] == ' ') || (si->text[i + 2] >= '0' && si->text[i + 2] <= '9')) && (si->text[i + 3] >= '0' && si->text[i + 3] <= '9') && si->text[i + 4] == '%' && si->text[i + 5] == ' ' && si->text[i + 6] == '[') {
				off_t position, total = 0; /* against warning */
				i += 7;
				while (si->text[i] != ']') {
					if (!si->text[i] || si->text[i] == '\n' || si->text[i] == '\r') {
						goto nexti;
					}
					i++;
				}
				i++;
				if (smbc_get_num(si->text, &i, &position)) {
					goto nexti;
				}
				while (si->text[i] == ' ' || si->text[i] == '\t') i++;
				if (si->text[i] != '/') {
					goto nexti;
				}
				i++;
				if (smbc_get_num(si->text, &i, &total)) {
					goto nexti;
				}
				if (total < c->from) total = c->from;
				c->est_length = total;
				lasti = i;
			}
		}
		if (lasti) memmove(si->text, si->text + lasti, si->ntext -= lasti);
	}
}

static void smb_got_data(struct connection *c)
{
	struct smb_connection_info *si = c->info;
	char *buffer = mem_alloc(page_size);
	int r;
	if (si->list) {
		smb_read_text(c, c->sock1);
		mem_free(buffer);
		return;
	}
	r = read(c->sock1, buffer, page_size);
	if (r == -1) {
		setcstate(c, get_error_from_errno(errno));
		retry_connection(c);
		mem_free(buffer);
		return;
	}
	if (r == 0) {
		mem_free(buffer);
		if (!si->cl) {
			si->cl = 1;
			set_handlers(c->sock1, NULL, NULL, NULL, NULL);
			return;
		}
		end_smb_connection(c);
		return;
	}
	setcstate(c, S_TRANS);
	if (!c->cache) {
		if (get_cache_entry(c->url, &c->cache)) {
			setcstate(c, S_OUT_OF_MEM);
			abort_connection(c);
			mem_free(buffer);
			return;
		}
		c->cache->refcount--;
	}
	if ((off_t)(0UL + c->from + r) < 0) {
		setcstate(c, S_LARGE_FILE);
		abort_connection(c);
		mem_free(buffer);
		return;
	}
	c->received += r;
	if (add_fragment(c->cache, c->from, buffer, r) == 1) c->tries = 0;
	c->from += r;
	mem_free(buffer);
}

static void smb_got_text(struct connection *c)
{
	smb_read_text(c, c->sock2);
}

static void end_smb_connection(struct connection *c)
{
	struct smb_connection_info *si = c->info;
	if (!c->cache) {
		if (get_cache_entry(c->url, &c->cache)) {
			setcstate(c, S_OUT_OF_MEM);
			abort_connection(c);
			return;
		}
		c->cache->refcount--;
	}
	if (!c->from) {
		int sdir;
		if (si->ntext && si->text[si->ntext - 1] != '\n') si->text[si->ntext++] = '\n';
		si->text[si->ntext] = 0;
		if (!strcmp(si->text, "client not found\n")) {
			setcstate(c, S_NO_SMB_CLIENT);
			if (++si->client < N_CLIENTS) {
				if (si->client > smb_client) smb_client = si->client;
				c->tries = -1;
				retry_connection(c);
			} else {
				smb_client = 0;
				abort_connection(c);
			}
			return;
		}
		sdir = 0;
		if (si->client == SMBC) {
			unsigned char *st = si->text;
			if (!memcmp(st, "ServerName", 10) && strchr(st, '\n')) st = strchr(st, '\n') + 1;
			if (!memcmp(st, "Logged", 6) && strchr(st, '\n')) st = strchr(st, '\n') + 1;
			if (!strstr(st, "ERR")) sdir = 1;
		}
		if (!si->list && *c->url && c->url[strlen(c->url) - 1] != '/' && c->url[strlen(c->url) - 1] != '\\' && (strstr(si->text, "NT_STATUS_FILE_IS_A_DIRECTORY") || strstr(si->text, "NT_STATUS_ACCESS_DENIED") || strstr(si->text, "ERRbadfile") || sdir)) {
			if (c->cache->redirect) mem_free(c->cache->redirect);
			c->cache->redirect = stracpy(c->url);
			c->cache->redirect_get = 1;
			add_to_strn(&c->cache->redirect, "/");
			c->cache->incomplete = 0;
		} else {
			unsigned char *ls, *le, *le2;
			unsigned char *ud;
			unsigned char *t = init_str();
			int l = 0;
			int type = 0;
			int pos = 0;
			add_to_str(&t, &l, "<html><head><title>");
			ud = stracpy(c->url);
			if (strchr(ud, POST_CHAR)) *strchr(ud, POST_CHAR) = 0;
			add_conv_str(&t, &l, ud, strlen(ud), -1);
			mem_free(ud);
			add_to_str(&t, &l, "</title></head><body><pre>");
			if (si->list == 1 && si->client == SMBC) {
/* smbc has a nasty bug that it writes field descriptions to stderr and data to
   stdout. Because of stdout buffer, they'll get mixed in the output. Try to
   demix them. */
#define SERVER	"Server              Comment\n------              -------\n"
#define WORKGR	"Workgroup           Master\n---------           ------\n"
				unsigned char *spos = strstr(si->text, SERVER);
				unsigned char *gpos;
				unsigned char *p, *pp, *ppp;
				if (spos) memmove(spos, spos + strlen(SERVER), strlen(spos) - strlen(SERVER) + 1);
				gpos = strstr(si->text, WORKGR);
				if (gpos) memmove(gpos, gpos + strlen(WORKGR), strlen(gpos) - strlen(WORKGR) + 1);
				if (!spos && !gpos) goto sc;
				pp = NULL, ppp = NULL, p = si->text;
				while ((p = strstr(p, "\n\n"))) ppp = pp, pp = p + 2, p++;
				if (!pp) goto sc;
				if (!spos || !gpos) ppp = NULL;
				if (spos) {
					if (!ppp) ppp = pp, pp = NULL;
					memmove(ppp + strlen(SERVER), ppp, strlen(ppp) + 1);
					memcpy(ppp, SERVER, strlen(SERVER));
					if (pp) pp += strlen(SERVER);
				}
				if (gpos && pp) {
					memmove(pp + strlen(WORKGR), pp, strlen(pp) + 1);
					memcpy(pp, WORKGR, strlen(WORKGR));
				}
				goto sc;
			}
			sc:
			ls = si->text;
			while ((le = strchr(ls, '\n'))) {
				unsigned char *lx;
				le2 = strchr(ls, '\r');
				if (!le2 || le2 > le) le2 = le;
				lx = memacpy(ls, le2 - ls);
				if (si->list == 1) {
					unsigned char *ll, *lll;
					if (!*lx) type = 0;
					if (strstr(lx, "Sharename") && strstr(lx, "Type")) {
						if (strstr(lx, "Type")) pos = (unsigned char *)strstr(lx, "Type") - lx;
						else pos = 0;
						type = 1;
						goto af;
					}
					if (strstr(lx, "Server") && strstr(lx, "Comment")) {
						type = 2;
						goto af;
					}
					if (strstr(lx, "Workgroup") && strstr(lx, "Master")) {
						pos = (unsigned char *)strstr(lx, "Master") - lx;
						type = 3;
						goto af;
					}
					if (!type) goto af;
					for (ll = lx; *ll; ll++) if (!WHITECHAR(*ll) && *ll != '-') goto np;
					goto af;
					np:
					for (ll = lx; *ll; ll++) if (!WHITECHAR(*ll)) break;
					for (lll = ll; *lll/* && lll[1]*/; lll++) if (WHITECHAR(*lll)/* && WHITECHAR(lll[1])*/) break;
					if (type == 1) {
						unsigned char *llll;
						if (!strstr(lll, "Disk")) goto af;
						if (pos && (size_t)pos < strlen(lx) && WHITECHAR(*(llll = lx + pos - 1)) && llll > ll) {
							while (llll > ll && WHITECHAR(*llll)) llll--;
							if (!WHITECHAR(*llll)) lll = llll + 1;
						}
						add_conv_str(&t, &l, lx, ll - lx, 0);
						add_to_str(&t, &l, "<a href=\"/");
						add_conv_str(&t, &l, ll, lll - ll, 1);
						add_to_str(&t, &l, "/\">");
						add_conv_str(&t, &l, ll, lll - ll, 0);
						add_to_str(&t, &l, "</a>");
						add_conv_str(&t, &l, lll, strlen(lll), 0);
					} else if (type == 2) {
						sss:
						add_conv_str(&t, &l, lx, ll - lx, 0);
						add_to_str(&t, &l, "<a href=\"smb://");
						add_conv_str(&t, &l, ll, lll - ll, 1);
						add_to_str(&t, &l, "/\">");
						add_conv_str(&t, &l, ll, lll - ll, 0);
						add_to_str(&t, &l, "</a>");
						add_conv_str(&t, &l, lll, strlen(lll), 0);
					} else if (type == 3) {
						if ((size_t)pos < strlen(lx) && pos && WHITECHAR(lx[pos - 1]) && !WHITECHAR(lx[pos])) ll = lx + pos;
						else for (ll = lll; *ll; ll++) if (!WHITECHAR(*ll)) break;
						for (lll = ll; *lll; lll++) if (WHITECHAR(*lll)) break;
						goto sss;
					} else goto af;
				} else if (si->list == 2 && si->client == SMBCLIENT) {
					if (strstr(lx, "NT_STATUS")) {
						le[1] = 0;
						goto af;
					}
					if (le2 - ls >= 5 && ls[0] == ' ' && ls[1] == ' ' && ls[2] != ' ') {
						int dir;
						unsigned char *pp;
						unsigned char *p = ls + 3;
						while (p + 2 <= le2) {
							if (p[0] == ' ' && p[1] == ' ') goto o;
							p++;
						}
						goto af;
						o:
						dir = 0;
						pp = p;
						while (pp < le2 && *pp == ' ') pp++;
						while (pp < le2 && *pp != ' ') {
							if (*pp == 'D') {
								dir = 1;
								break;
							}
							pp++;
						}
						add_to_str(&t, &l, "  <a href=\"./");
						add_conv_str(&t, &l, ls + 2, p - (ls + 2), 1);
						if (dir) add_chr_to_str(&t, &l, '/');
						add_to_str(&t, &l, "\">");
						add_conv_str(&t, &l, ls + 2, p - (ls + 2), 0);
						add_to_str(&t, &l, "</a>");
						add_conv_str(&t, &l, p, le - p, 0);
					} else goto af;
				} else if (si->list == 2 && si->client == SMBC) {
					unsigned char *d;
					if (le2 - ls <= 17) goto af;
					d = ls + 17;
					smbc_next_chr:
					if (d + 9 >= le2) goto af;
					if (!(d[0] == ':' && d[1] >= '0' && d[1] <= '9' && d[2] >= '0' && d[2] <= '9' && d[3] == ' ' && ((d[4] == '1' && d[5] == '9') || (d[4] == '2' && d[5] >= '0' && d[5] <= '9')) && d[6] >= '0' && d[6] <= '9' && d[7] >= '0' && d[7] <= '9' && d[8] == ' ')) {
						d++;
						goto smbc_next_chr;
					}
					d += 9;
					add_conv_str(&t, &l, ls, d - ls, 0);
					add_to_str(&t, &l, "<a href=\"./");
					add_conv_str(&t, &l, d, le2 - d, 1);
					if (ls[4] == 'D') add_chr_to_str(&t, &l, '/');
					add_to_str(&t, &l, "\">");
					add_conv_str(&t, &l, d, le2 - d, 0);
					add_to_str(&t, &l, "</a>");
				} else af: add_conv_str(&t, &l, ls, le2 - ls, 0);
				add_chr_to_str(&t, &l, '\n');
				ls = le + 1;
				mem_free(lx);
			}
			/*add_to_str(&t, &l, si->text);*/
			add_fragment(c->cache, 0, t, l);
			c->from += l;
			truncate_entry(c->cache, l, 1);
			c->cache->incomplete = 0;
			mem_free(t);
			if (!c->cache->head) c->cache->head = stracpy("\r\n");
			add_to_strn(&c->cache->head, "Content-Type: text/html\r\n");
		}
	} else {
		truncate_entry(c->cache, c->from, 1);
		c->cache->incomplete = 0;
	}
	close_socket(&c->sock1);
	close_socket(&c->sock2);
	setcstate(c, S_OK);
	abort_connection(c);
	return;
}

#endif
