#ifndef MSTRING_ENGINE
#define MSTRING_ENGINE

// typedef unsigned char u_int8_t;
// typedef unsigned short int u_int16_t;

struct mstring_node {
	u_int16_t n,		/* N-char cmp */
	 s,			/* offset in sbuf */
	 l,			/* left  node */
	 r;			/* right node */
	u_int16_t e;		/* eq    node or exit code */
};
struct mstring_exec {
	u_int16_t smin;		/* min string length */
	u_int16_t start, last;	/* start node, last node */
	u_int16_t sbuf_len;
	u_int16_t nstr;		/* number of strings */
	char *sbuf;
	u_int16_t *soffs;	/* start offset in sbuf */
	struct mstring_node *N;
};
static int mstring_find(struct mstring_exec *s, char *str, int l)
{
	int c, i, n, x;
	char *cs;
	struct mstring_node *p;

	if (!l)
		l = strlen(str);
	if (l < s->smin)
		return 0;
	i = s->start;
	if (!i)
		return 0;

	p = &s->N[i];
	n = p->n;
	cs = &s->sbuf[p->s];
	while (l > 0 && n > 0) {
		c = *str++;
		l--;
		if (c >= 'A' && c <= 'Z')
			c |= 0x20;
		do {
			x = c - (int)*cs;
			if (!x) {
				cs++;
				n--;
				if (n > 0)
					break;
				if (p->e >= s->last)
					return p->e - s->last;
				i = p->e;
			} else {
				if (n != p->n)
					return 0;
				i = x < 0 ? p->l : p->r;
			}
			if (!i)
				return 0;
			p = &s->N[i];
			n = p->n;
			cs = &s->sbuf[p->s];
		} while (x);
	}
	return 0;
}
#endif
/* Source http_hdr
  mstring http_hdr "host:" "x-forwarded-for:" "referer:" \
	"content-type:" "accept:" "user-agent:" "content-encoding:" \
	"transfer-encoding:" "content-length:" "cookie:" \
	"x-session-type:" "server:" "origin:" 

	"host:",1
	"x-forwarded-for:",2
	"referer:",3
	"content-type:",4
	"accept:",5
	"user-agent:",6
	"content-encoding:",7
	"transfer-encoding:",8
	"content-length:",9
	"cookie:",10
	"x-session-type:",11
	"server:",12
	"origin:",13
 */
static char mstring_sbuf_http_hdr[] = "#host:x-forwarded-for:referer:content-type:accept:user-agent:content-encoding:transfer-encoding:content-length:cookie:x-session-type:server:origin:";
static u_int16_t mstring_soffs_http_hdr[15] = {
	0, 1, 6, 22, 30, 43, 50, 61, 78, 96, 111, 118, 133, 140, 147
};

static struct mstring_node mstring_node_http_hdr[18] = {
	{0,},
	{.n = 7,.s = 43,.l = 0,.r = 0,.e = 22},
	{.n = 2,.s = 30,.l = 1,.r = 0,.e = 14},
	{.n = 5,.s = 1,.l = 2,.r = 4,.e = 18},
	{.n = 7,.s = 140,.l = 0,.r = 0,.e = 30},
	{.n = 8,.s = 22,.l = 3,.r = 7,.e = 20},
	{.n = 7,.s = 133,.l = 0,.r = 0,.e = 29},
	{.n = 18,.s = 78,.l = 6,.r = 8,.e = 25},
	{.n = 11,.s = 50,.l = 0,.r = 9,.e = 23},
	{.n = 2,.s = 6,.l = 0,.r = 0,.e = 11},
	{.n = 13,.s = 120,.l = 0,.r = 0,.e = 28},
	{.n = 14,.s = 8,.l = 0,.r = 10,.e = 19},
	{.n = 5,.s = 113,.l = 0,.r = 0,.e = 27},
	{.n = 7,.s = 104,.l = 16,.r = 0,.e = 26},
	{.n = 6,.s = 32,.l = 0,.r = 12,.e = 15},
	{.n = 5,.s = 38,.l = 13,.r = 0,.e = 21},
	{.n = 9,.s = 69,.l = 0,.r = 0,.e = 24}
};

static struct mstring_exec mstring_data_http_hdr = {
	.smin = 5,
	.start = 5,.last = 17,
	.sbuf_len = 147,
	.sbuf = &mstring_sbuf_http_hdr[0],
	.soffs = mstring_soffs_http_hdr,
	.N = &mstring_node_http_hdr[0]
};
