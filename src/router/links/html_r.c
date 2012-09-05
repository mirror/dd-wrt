#include "links.h"

#define format format_

static inline int color_distance(struct rgb *c1, struct rgb *c2)
{
	return
		3 * (c1->r - c2->r) * (c1->r - c2->r) +
		4 * (c1->g - c2->g) * (c1->g - c2->g) +
		2 * (c1->b - c2->b) * (c1->b - c2->b);
}

struct rgb palette[] = {
/*	{0x00, 0x00, 0x00, 0},
	{0x80, 0x00, 0x00, 0},
	{0x00, 0x80, 0x00, 0},
	{0x80, 0x80, 0x00, 0},
	{0x00, 0x00, 0x80, 0},
	{0x80, 0x00, 0x80, 0},
	{0x00, 0x80, 0x80, 0},
	{0xC0, 0xC0, 0xC0, 0},
	{0x80, 0x80, 0x80, 0},
	{0xff, 0x00, 0x00, 0},
	{0x00, 0xff, 0x00, 0},
	{0xff, 0xff, 0x00, 0},
	{0x00, 0x00, 0xff, 0},
	{0xff, 0x00, 0xff, 0},
	{0x00, 0xff, 0xff, 0},
	{0xff, 0xff, 0xff, 0},*/
	/*{0x00, 0x00, 0x00, 0},
	{0xaa, 0x00, 0x00, 0},
	{0x00, 0xaa, 0x00, 0},
	{0xaa, 0x55, 0x00, 0},
	{0x00, 0x00, 0xaa, 0},
	{0xaa, 0x00, 0xaa, 0},
	{0x00, 0xaa, 0xaa, 0},
	{0xaa, 0xaa, 0xaa, 0},
	{0x55, 0x55, 0x55, 0},
	{0xff, 0x55, 0x55, 0},
	{0x55, 0xff, 0x55, 0},
	{0xff, 0xff, 0x55, 0},
	{0x55, 0x55, 0xff, 0},
	{0xff, 0x55, 0xff, 0},
	{0x55, 0xff, 0xff, 0},
	{0xff, 0xff, 0xff, 0},*/
	{0x00, 0x00, 0x00, 0},
	{0x80, 0x00, 0x00, 0},
	{0x00, 0x80, 0x00, 0},
	{0xaa, 0x55, 0x00, 0},
	{0x00, 0x00, 0x80, 0},
	{0x80, 0x00, 0x80, 0},
	{0x00, 0x80, 0x80, 0},
	{0xaa, 0xaa, 0xaa, 0},
	{0x55, 0x55, 0x55, 0},
	{0xff, 0x55, 0x55, 0},
	{0x55, 0xff, 0x55, 0},
	{0xff, 0xff, 0x55, 0},
	{0x55, 0x55, 0xff, 0},
	{0xff, 0x55, 0xff, 0},
	{0x55, 0xff, 0xff, 0},
	{0xff, 0xff, 0xff, 0},
	{-1, -1, -1, 0},
};

/*struct rgb rgbcache = {0, 0, 0};
int rgbcache_c = 0;

static inline int find_nearest_color(struct rgb *r, int l)
{
	int dist, dst, min, i;
	if (r->r == rgbcache.r && r->g == rgbcache.g && r->b == rgbcache.b) return rgbcache_c;
	dist = 0xffffff;
	min = 0;
	for (i = 0; i < l; i++) if ((dst = color_distance(r, &palette[i])) < dist)
		dist = dst, min = i;
	return min;
}*/

struct rgb_cache_entry {
	int color;
	int l;
	struct rgb rgb;
};

#define RGB_HASH_SIZE 4096

#define HASH_RGB(r, l) ((((r)->r << 3) + ((r)->g << 2) + (r)->b + (l)) & (RGB_HASH_SIZE - 1))

static inline int find_nearest_color(struct rgb *r, int l)
{
	int dist, dst, min, i;
	static struct rgb_cache_entry rgb_cache[RGB_HASH_SIZE];
	static int cache_init = 0;
	int h;
	if (!cache_init) goto initialize;
	back:
	h = HASH_RGB(r, l);
	if (rgb_cache[h].color != -1 && rgb_cache[h].l == l && rgb_cache[h].rgb.r == r->r && rgb_cache[h].rgb.g == r->g && rgb_cache[h].rgb.b == r->b) return rgb_cache[h].color;
	dist = 0xffffff;
	min = 0;
	for (i = 0; i < l; i++) if ((dst = color_distance(r, &palette[i])) < dist)
		dist = dst, min = i;
	rgb_cache[h].color = min;
	rgb_cache[h].l = l;
	rgb_cache[h].rgb.r = r->r;
	rgb_cache[h].rgb.g = r->g;
	rgb_cache[h].rgb.b = r->b;
	return min;

	initialize:
	for (h = 0; h < RGB_HASH_SIZE; h++) rgb_cache[h].color = -1;
	cache_init = 1;
	goto back;
}

static inline int fg_color(int fg, int bg)
{
	int l = bg < fg ? bg : fg;
	int h = bg < fg ? fg : bg;
	if (l == h || (!l && (h == 4 || h == 8 || h == 12)) ||
	   (l == 1 && (h == 3 || h == 5 || h == 8 || h == 12)) ||
	   (l == 2 && h == 6) || (l == 3 && (h == 5 || h == 12)) ||
	   (l == 4 && (h == 8 || h == 12)) || (l == 5 && (h == 8 || h == 12)))
	   	return (fg == 4 || fg == 12) && (bg == 0 || bg == 8) ? 6 : (7 - 7 * (bg == 2 || bg == 6 || bg == 7));
	return fg;
}

#define XALIGN(x) (((x)+0x7f)&~0x7f)

int nowrap = 0;

static inline void xpand_lines(struct part *p, int y)
{
	/*if (y >= p->y) p->y = y + 1;*/
	if (!p->data) return;
	if (XALIGN((unsigned)y + (unsigned)p->yp) > MAXINT) overalloc();
	y += p->yp;
	if (y >= p->data->y) {			/* !!! FIXME: out of inline */
		int i;
		if (XALIGN(y + 1) > XALIGN(p->data->y)) {
			if (XALIGN((unsigned)y + 1) > MAXINT / sizeof(struct line)) overalloc();
			p->data->data = mem_realloc(p->data->data, XALIGN(y+1)*sizeof(struct line));
		}
		for (i = p->data->y; i <= y; i++) {
			p->data->data[i].l = 0;
			p->data->data[i].c = p->bgcolor;
			p->data->data[i].d = DUMMY;
		}
		p->data->y = i;
	}
}

static inline void xpand_line(struct part *p, int y, int x)
{
	if (!p->data) return; /* !!! FIXME: p->x (?) */
	if (XALIGN((unsigned)x + (unsigned)p->xp) > MAXINT) overalloc();
	x += p->xp;
	y += p->yp;
#ifdef DEBUG
	if (y >= p->data->y) {
		internal("line does not exist");
		return;
	}
#endif
	if (x >= p->data->data[y].l) {		/* !!! FIXME: out of inline */
		int i;
		if (XALIGN(x+1) > XALIGN(p->data->data[y].l)) {
			if (XALIGN((unsigned)x + 1) > MAXINT / sizeof(chr)) overalloc();
			p->data->data[y].d = mem_realloc(p->data->data[y].d, XALIGN(x+1)*sizeof(chr));
		}
		for (i = p->data->data[y].l; i <= x; i++)
			p->data->data[y].d[i] = (p->data->data[y].c << 11) | ' ';
		p->data->data[y].c = p->bgcolor;
		p->data->data[y].l = i;
	}
}

void r_xpand_spaces(struct part *p, int l)
{
	unsigned char *c;
	if ((unsigned)l >= MAXINT) overalloc();
	c = mem_realloc(p->spaces, l + 1);
	memset(c + p->spl, 0, l - p->spl + 1);
	p->spl = l + 1;
	p->spaces = c;
}

static inline void xpand_spaces(struct part *p, int l)
{
	if ((unsigned)l >= (unsigned)p->spl) r_xpand_spaces(p, l);
}

#define POS(x, y) (p->data->data[p->yp + (y)].d[p->xp + (x)])
#define LEN(y) (p->data->data[p->yp + (y)].l - p->xp < 0 ? 0 : p->data->data[p->yp + (y)].l - p->xp)
#define SLEN(y, x) p->data->data[p->yp + (y)].l = p->xp + x;
#define X(x) (p->xp + (x))
#define Y(y) (p->yp + (y))

static inline void set_hchar(struct part *p, int x, int y, unsigned c)
{
	xpand_lines(p, y);
	xpand_line(p, y, x);
	POS(x, y) = c;
}

static inline void set_hchars(struct part *p, int x, int y, int xl, unsigned c)
{
	xpand_lines(p, y);
	xpand_line(p, y, x+xl-1);
	for (; xl; xl--, x++) POS(x, y) = c;
}

void xset_hchar(struct part *p, int x, int y, unsigned c)
{
	set_hchar(p, x, y, c);
}

void xset_hchars(struct part *p, int x, int y, int xl, unsigned c)
{
	set_hchars(p, x, y, xl, c);
}

void xxpand_lines(struct part *p, int y)
{
	xpand_lines(p, y);
}

void xxpand_line(struct part *p, int y, int x)
{
	xpand_line(p, y, x);
}

static inline void set_hline(struct part *p, int x, int y, int xl, unsigned char *d, unsigned c, int spc)
{
	xpand_lines(p, y);
	xpand_line(p, y, x+xl-1);
	if (spc) xpand_spaces(p, x+xl-1);
	for (; xl; xl--, x++, d++) {
		if (spc) p->spaces[x] = *d == ' ';
		if (p->data) POS(x, y) = *d | c;
	}
}

int last_link_to_move;
struct tag *last_tag_to_move;
struct tag *last_tag_for_newline;

static inline void move_links(struct part *p, int xf, int yf, int xt, int yt)
{
	int n;
	struct tag *t;
	int w = 0;
	if (!p->data) return;
	xpand_lines(p, yt);
	for (n = last_link_to_move; n < p->data->nlinks; n++) {
		int i;
		struct link *link = &p->data->links[n];
			/*printf("ml: %d %d %d %d",link->pos[0].x,link->pos[0].y,X(xf),Y(yf));fflush(stdout);sleep(1);*/
		/*for (i = 0; i < link->n; i++) fprintf(stderr, "%d.%d -> %d.%d: %d.%d : %d %d\n", X(xf), Y(yf), X(xt), yt != -1 ? Y(yt) : -1, n, i, link->pos[i].x, link->pos[i].y);*/
		for (i = 0; i < link->n; i++) if (link->pos[i].y >= Y(yf)) {
			w = 1;
			if (link->pos[i].y == Y(yf) && link->pos[i].x >= X(xf)) {
				if (yt >= 0) link->pos[i].y = Y(yt), link->pos[i].x += -xf + xt;
				else memmove(&link->pos[i], &link->pos[i+1], (link->n-i-1) * sizeof(struct point)), link->n--, i--;
			}
		}
		/*if (!link->n) {
			if (link->where) mem_free(link->where);
			if (link->target) mem_free(link->target);
			if (link->where_img) mem_free(link->where_img);
			if (link->pos) mem_free(link->pos);
			memmove(link, link + 1, (p->data->nlinks - n - 1) * sizeof(struct link));
			p->data->nlinks --;
			n--;
		}*/
		if (!w /*&& n >= 0*/) last_link_to_move = n;
	}
	w = 0;
	if (yt >= 0) for (t = last_tag_to_move->next; (void *)t != &p->data->tags; t = t->next) {
		if (t->y == Y(yf)) {
			w = 1;
			if (t->x >= X(xf)) {
				t->y = Y(yt), t->x += -xf + xt;
			}
		}
		if (!w) last_tag_to_move = t;
	}
}

static inline void copy_chars(struct part *p, int x, int y, int xl, chr *d)
{
	if (xl <= 0) return;
	xpand_lines(p, y);
	xpand_line(p, y, x+xl-1);
	for (; xl; xl--, x++, d++) POS(x, y) = *d;
}

static inline void move_chars(struct part *p, int x, int y, int nx, int ny)
{
	if (LEN(y) - x <= 0) return;
	copy_chars(p, nx, ny, LEN(y) - x, &POS(x, y));
	SLEN(y, x);
	move_links(p, x, y, nx, ny);
}

static inline void shift_chars(struct part *p, int y, int s)
{
	chr *a;
	int l = LEN(y);
	if ((unsigned)l > MAXINT / sizeof(chr)) overalloc();
	a = mem_alloc(l * sizeof(chr));
	memcpy(a, &POS(0, y), l * sizeof(chr));
	set_hchars(p, 0, y, s, (p->data->data[y].c << 11) | ' ');
	copy_chars(p, s, y, l, a);
	mem_free(a);
	move_links(p, 0, y, s, y);
}

static inline void del_chars(struct part *p, int x, int y)
{
	SLEN(y, x);
	move_links(p, x, y, -1, -1);
}

#define rm(x) ((x).width - (x).rightmargin > 0 ? (x).width - (x).rightmargin : 0)

void line_break(struct part *);

int split_line(struct part *p)
{
	int i;
	/*if (!p->data) goto r;*/
	/*printf("split: %d,%d   , %d,%d,%d\n",p->cx,p->cy,par_format.rightmargin,par_format.leftmargin,p->cx);*/
	for (i = rm(par_format); i >= par_format.leftmargin; i--)
		if (i < p->spl && p->spaces[i]) goto split;
	/*for (i = p->cx - 1; i > rm(par_format) && i > par_format.leftmargin; i--)*/
	for (i = par_format.leftmargin; i < p->cx ; i++)
		if (i < p->spl && p->spaces[i]) goto split;
	/*for (i = rm(par_format); i >= par_format.leftmargin; i--)
		if ((POS(i, p->cy) & 0xff) == ' ') goto split;
	for (i = p->cx - 1; i > rm(par_format) && i > par_format.leftmargin; i--)
		if ((POS(i, p->cy) & 0xff) == ' ') goto split;*/
	if (p->cx + par_format.rightmargin > p->x) p->x = p->cx + par_format.rightmargin;
	/*if (p->y < p->cy + 1) p->y = p->cy + 1;
	p->cy++; p->cx = -1;
	memset(p->spaces, 0, p->spl);
	if (p->data) xpand_lines(p, p->cy + 1);*/
	/*line_break(p);*/
	return 0;
	split:
	if (i + par_format.rightmargin > p->x) p->x = i + par_format.rightmargin;
	if (p->data) {
#ifdef DEBUG
		if ((POS(i, p->cy) & 0xff) != ' ') internal("bad split: %c", (char)POS(i, p->cy));
#endif
		move_chars(p, i+1, p->cy, par_format.leftmargin, p->cy+1);
		del_chars(p, i, p->cy);
	}
	memmove(p->spaces, p->spaces + i + 1, p->spl - i - 1);
	memset(p->spaces + p->spl - i - 1, 0, i + 1);
	memmove(p->spaces + par_format.leftmargin, p->spaces, p->spl - par_format.leftmargin);
	p->cy++; p->cx -= i - par_format.leftmargin + 1;
	/*return 1 + (p->cx == par_format.leftmargin);*/
	if (p->cx == par_format.leftmargin) p->cx = -1;
	if (p->y < p->cy + (p->cx != -1)) p->y = p->cy + (p->cx != -1);
	return 1 + (p->cx == -1);
}

void align_line(struct part *p, int y)
{
	int na;
	if (!p->data) return;
	if (!LEN(y) || par_format.align == AL_LEFT || par_format.align == AL_NO || par_format.align == AL_BLOCK /* !!! fixme! */) return;
	na = rm(par_format) - LEN(y);
	if (par_format.align == AL_CENTER) na /= 2;
	if (na > 0) shift_chars(p, y, na);
}

struct link *new_link(struct f_data *f)
{
	if (!f) return NULL;
	if (!(f->nlinks & (ALLOC_GR - 1))) {
		if ((unsigned)f->nlinks > MAXINT / sizeof(struct link) - ALLOC_GR) overalloc();
		f->links = mem_realloc(f->links, (f->nlinks + ALLOC_GR) * sizeof(struct link));
	}
	memset(&f->links[f->nlinks], 0, sizeof(struct link));
	return &f->links[f->nlinks++];
}

void html_tag(struct f_data *f, unsigned char *t, int x, int y)
{
	struct tag *tag;
	unsigned char *tt;
	int ll;
	if (!f) return;
	tt = init_str();
	ll = 0;
	add_conv_str(&tt, &ll, t, strlen(t), -2);
	tag = mem_alloc(sizeof(struct tag) + strlen(tt) + 1);
	tag->x = x;
	tag->y = y;
	strcpy(tag->name, tt);
	add_to_list(f->tags, tag);
	if ((void *)last_tag_for_newline == &f->tags) last_tag_for_newline = tag;
	mem_free(tt);
}

unsigned char *last_link;
unsigned char *last_target;
unsigned char *last_image;
struct form_control *last_form;

int nobreak;

struct conv_table *convert_table;

void put_chars(struct part *, unsigned char *, int);

#define CH_BUF	256

int put_chars_conv(struct part *p, unsigned char *c, int l)
{
	static char buffer[CH_BUF];
	int bp = 0;
	int pp = 0;
	int total = 0;
	if (format.attr & AT_GRAPHICS) {
		put_chars(p, c, l);
		return l;
	}
	if (!l) put_chars(p, NULL, 0);
	while (pp < l) {
		unsigned char *e;
		if (c[pp] < 128 && c[pp] != '&') {
			put_c:
			buffer[bp++] = c[pp++];
			if (bp < CH_BUF) continue;
			goto flush;
		}
		if (c[pp] != '&') {
			struct conv_table *t;
			int i;
			if (!convert_table) goto put_c;
			t = convert_table;
			i = pp;
			decode:
			if (!t[c[i]].t) {
				e = t[c[i]].u.str;
			} else {
				t = t[c[i++]].u.tbl;
				if (i >= l) goto put_c;
				goto decode;
			}
			pp = i + 1;
		} else {
			int i = pp + 1;
			if (d_opt->plain) goto put_c;
			while (i < l && c[i] != ';' && c[i] != '&' && c[i] > ' ') i++;
			if (!(e = get_entity_string(&c[pp + 1], i - pp - 1, d_opt->cp))) goto put_c;
			pp = i + (i < l && c[i] == ';');
		}
		if (!e[0]) continue;
		if (!e[1]) {
			buffer[bp++] = e[0];
			if (bp < CH_BUF) continue;
			flush:
			e = "";
			goto flush1;
		}
		while (*e) {
			buffer[bp++] = *(e++);
			if (bp < CH_BUF) continue;
			flush1:
			put_chars(p, buffer, bp);
			total += bp;
			bp = 0;
		}
	}
	if (bp) put_chars(p, buffer, bp);
	total += bp;
	return total;
}

void put_chars(struct part *p, unsigned char *c, int l)
{
	static struct text_attrib_beginning ta_cache = { -1, { 0, 0, 0, 0 }, { 0, 0, 0, 0 } };
	static int bg_cache;
	static int fg_cache;

	int bg, fg;
	int i;
	struct link *link;
	struct point *pt;
	if (l < 0) overalloc();
	/*printf("%d-", p->cx);for (i=0; i<l; i++) printf("%c", c[i]); printf("-\n");sleep(1);*/
	while (par_format.align != AL_NO && p->cx == -1 && l && *c == ' ') c++, l--;
	if (!l) return;
	if (c[0] != ' ' || (c[1] && c[1] != ' ')) {
		last_tag_for_newline = (void *)&p->data->tags;
	}
	if (p->cx < par_format.leftmargin) p->cx = par_format.leftmargin;
	if (last_link || last_image || last_form || format.link || format.image || format.form) goto process_link;
	no_l:
	/*printf("%d %d\n",p->cx, p->cy);*/
	if (memcmp(&ta_cache, &format, sizeof(struct text_attrib_beginning))) goto format_change;
	bg = bg_cache, fg = fg_cache;
	end_format_change:
	if (p->cx == par_format.leftmargin && *c == ' ' && par_format.align != AL_NO) c++, l--;
	if (p->y < p->cy + 1) p->y = p->cy + 1;
	if (nowrap && p->cx + l > rm(par_format)) return;
	set_hline(p, p->cx, p->cy, l, c, (((fg&0x08)<<3)|(bg<<3)|(fg&0x07))<<8, 1);
	p->cx += l;
	nobreak = 0;
	if (par_format.align != AL_NO)
		while (p->cx > rm(par_format) && p->cx > par_format.leftmargin) {
			int x;
			/*if (p->cx > p->x) {
				p->x = p->cx + par_format.rightmargin;
				if (c[l - 1] == ' ') p->x--;
			}*/
			if (!(x = split_line(p))) break;
			/*if (LEN(p->cy-1) > p->x) p->x = LEN(p->cy-1);*/
			align_line(p, p->cy - 1);
			nobreak = x - 1;
		}
	if ((p->xa += l) - (c[l-1] == ' ' && par_format.align != AL_NO) + par_format.leftmargin + par_format.rightmargin > p->xmax) p->xmax = p->xa - (c[l-1] == ' ' && par_format.align != AL_NO) + par_format.leftmargin + par_format.rightmargin;
	return;
	process_link:
	if ((last_link /*|| last_target*/ || last_image || last_form) &&
	    !xstrcmp(format.link, last_link) && !xstrcmp(format.target, last_target) &&
	    !xstrcmp(format.image, last_image) && format.form == last_form) {
		if (!p->data) goto x;
		link = &p->data->links[p->data->nlinks - 1];
		if (!p->data->nlinks) {
			internal("no link");
			goto no_l;
		}
		goto set_link;
		x:;
	} else {
		if (last_link) mem_free(last_link);	/* !!! FIXME: optimize */
		if (last_target) mem_free(last_target);
		if (last_image) mem_free(last_image);
		last_link = last_target = last_image = NULL;
		last_form = NULL;
		if (!(format.link || format.image || format.form)) goto no_l;
		if (d_opt->num_links) {
			unsigned char s[64];
			unsigned char *fl = format.link, *ft = format.target, *fi = format.image;
			struct form_control *ff = format.form;
			format.link = format.target = format.image = NULL;
			format.form = NULL;
			s[0] = '[';
			snzprint(s + 1, 62, p->link_num);
			strcat(s, "]");
			put_chars(p, s, strlen(s));
			if (ff && ff->type == FC_TEXTAREA) line_break(p);
			if (p->cx == -1) p->cx = par_format.leftmargin;
			format.link = fl, format.target = ft, format.image = fi;
			format.form = ff;
		}
		p->link_num++;
		last_link = stracpy(format.link);
		last_target = stracpy(format.target);
		last_image = stracpy(format.image);
		last_form = format.form;
		if (!p->data) goto no_l;
		if (!(link = new_link(p->data))) goto no_l;
		link->num = p->link_num - 1;
		link->pos = DUMMY;
		if (!last_form) {
			link->type = L_LINK;
			link->where = stracpy(last_link);
			link->target = stracpy(last_target);
		} else {
			link->type = last_form->type == FC_TEXT || last_form->type == FC_PASSWORD || last_form->type == FC_FILE ? L_FIELD : last_form->type == FC_TEXTAREA ? L_AREA : last_form->type == FC_CHECKBOX || last_form->type == FC_RADIO ? L_CHECKBOX : last_form->type == FC_SELECT ? L_SELECT : L_BUTTON;
			link->form = last_form;
			link->target = stracpy(last_form->target);
		}
		link->where_img = stracpy(last_image);
		if (link->type != L_FIELD && link->type != L_AREA) {
			bg = find_nearest_color(&format.clink, 8);
			fg = find_nearest_color(&format.bg, 8);
			fg = fg_color(fg, bg);
		} else {
			fg = find_nearest_color(&format.fg, 8);
			bg = find_nearest_color(&format.bg, 8);
			fg = fg_color(fg, bg);
		}
		link->sel_color = ((fg & 8) << 3) | (fg & 7) | (bg << 3);
		link->n = 0;
		set_link:
		if ((unsigned)link->n + (unsigned)l > MAXINT / sizeof(struct point)) overalloc();
		pt = mem_realloc(link->pos, (link->n + l) * sizeof(struct point));
		link->pos = pt;
		for (i = 0; i < l; i++) pt[link->n + i].x = X(p->cx) + i,
					pt[link->n + i].y = Y(p->cy);
		link->n += l;
	}
	goto no_l;

		format_change:
		bg = find_nearest_color(&format.bg, 8);
		fg = find_nearest_color(&format.fg, 16);
		fg = fg_color(fg, bg);
		if (format.attr & AT_ITALIC) fg = fg ^ 0x01;
		if (format.attr & AT_UNDERLINE) fg = (fg ^ 0x04) | 0x08;
		if (format.attr & AT_BOLD) fg = fg | 0x08;
		fg = fg_color(fg, bg);
		if (format.attr & AT_GRAPHICS) bg = bg | 0x10;
		memcpy(&ta_cache, &format, sizeof(struct text_attrib_beginning));
		fg_cache = fg; bg_cache = bg;
		goto end_format_change;
}

void line_break(struct part *p)
{
	struct tag *t;
	/*printf("-break-\n");*/
	if (p->cx + par_format.rightmargin > p->x) p->x = p->cx + par_format.rightmargin;
	if (nobreak) {
		/*if (p->y < p->cy) p->y = p->cy;*/
		nobreak = 0;
		p->cx = -1;
		p->xa = 0;
		return;
	}
	if (!p->data) goto e;
	/*move_links(p, p->cx, p->cy, 0, p->cy + 1);*/
	xpand_lines(p, p->cy + 1);
	if (p->cx > par_format.leftmargin && LEN(p->cy) > p->cx - 1 && (POS(p->cx-1, p->cy) & 0xff) == ' ') del_chars(p, p->cx-1, p->cy), p->cx--;
	/*if (LEN(p->cy) > p->x) p->x = LEN(p->cy);*/
	if (p->cx > 0) align_line(p, p->cy);
	if (p->data) for (t = last_tag_for_newline; t && (void *)t != &p->data->tags; t = t->prev) {
		t->x = X(0);
		t->y = Y(p->cy + 1);
	}
	e:
	p->cy++; p->cx = -1; p->xa = 0; /*if (p->y < p->cy) p->y = p->cy;*/
	memset(p->spaces, 0, p->spl);
}

int g_ctrl_num;

void html_form_control(struct part *p, struct form_control *fc)
{
	if (!p->data) {
		/*destroy_fc(fc);
		mem_free(fc);*/
		add_to_list(p->uf, fc);
		return;
	}
	fc->g_ctrl_num = g_ctrl_num++;
	if (fc->type == FC_TEXT || fc->type == FC_PASSWORD || fc->type == FC_TEXTAREA) {
		unsigned char *dv = convert_string(convert_table, fc->default_value, strlen(fc->default_value));
		if (dv) {
			mem_free(fc->default_value);
			fc->default_value = dv;
		}
		/*
		for (i = 0; i < fc->nvalues; i++) if ((dv = convert_string(convert_table, fc->values[i], strlen(fc->values[i])))) {
			mem_free(fc->values[i]);
			fc->values[i] = dv;
		}
		*/
	}
	if (fc->type == FC_TEXTAREA) {
		unsigned char *p;
		for (p = fc->default_value; p[0]; p++) if (p[0] == '\r') {
			if (p[1] == '\n') memmove(p, p + 1, strlen(p)), p--;
			else p[0] = '\n';
		}
	}
	add_to_list(p->data->forms, fc);
}

void add_frameset_entry(struct frameset_desc *fsd, struct frameset_desc *subframe, unsigned char *name, unsigned char *url)
{
	if (fsd->yp >= fsd->y) return;
	fsd->f[fsd->xp + fsd->yp * fsd->x].subframe = subframe;
	fsd->f[fsd->xp + fsd->yp * fsd->x].name = stracpy(name);
	fsd->f[fsd->xp + fsd->yp * fsd->x].url = stracpy(url);
	if (++fsd->xp >= fsd->x) fsd->xp = 0, fsd->yp++;
}

struct frameset_desc *create_frameset(struct f_data *fda, struct frameset_param *fp)
{
	int i;
	struct frameset_desc *fd;
	if (!fp->x || !fp->y) {
		internal("zero size of frameset");
		return NULL;
	}
	if (fp->x && (unsigned)fp->x * (unsigned)fp->y / (unsigned)fp->x != (unsigned)fp->y) overalloc();
	if ((unsigned)fp->x * (unsigned)fp->y > (MAXINT - sizeof(struct frameset_desc)) / sizeof(struct frame_desc)) overalloc();
	fd = mem_alloc(sizeof(struct frameset_desc) + fp->x * fp->y * sizeof(struct frame_desc));
	memset(fd, 0, sizeof(struct frameset_desc) + fp->x * fp->y * sizeof(struct frame_desc));
	fd->n = fp->x * fp->y;
	fd->x = fp->x;
	fd->y = fp->y;
	for (i = 0; i < fd->n; i++) {
		fd->f[i].xw = fp->xw[i % fp->x];
		fd->f[i].yw = fp->yw[i / fp->x];
	}
	if (fp->parent) add_frameset_entry(fp->parent, fd, NULL, NULL);
	else if (!fda->frame_desc) fda->frame_desc = fd;
	     else mem_free(fd), fd = NULL;
	return fd;
}

void create_frame(struct frame_param *fp)
{
	add_frameset_entry(fp->parent, NULL, fp->name, fp->url);
}

void *html_special(struct part *p, int c, ...)
{
	va_list l;
	unsigned char *t;
	struct form_control *fc;
	struct frameset_param *fsp;
	struct frame_param *fp;
	va_start(l, c);
	switch (c) {
		case SP_TAG:
			t = va_arg(l, unsigned char *);
			va_end(l);
			html_tag(p->data, t, X(p->cx), Y(p->cy));
			break;
		case SP_CONTROL:
			fc = va_arg(l, struct form_control *);
			va_end(l);
			html_form_control(p, fc);
			break;
		case SP_TABLE:
			va_end(l);
			return convert_table;
		case SP_USED:
			va_end(l);
			return (void *)!!p->data;
		case SP_FRAMESET:
			fsp = va_arg(l, struct frameset_param *);
			va_end(l);
			return create_frameset(p->data, fsp);
		case SP_FRAME:
			fp = va_arg(l, struct frame_param *);
			va_end(l);
			create_frame(fp);
			break;
		case SP_NOWRAP:
			nowrap = va_arg(l, int);
			va_end(l);
			break;
		default:
			internal("html_special: unknown code %d", c);
			va_end(l);
	}
	return NULL;
}

void do_format(char *start, char *end, struct part *part, unsigned char *head)
{
	parse_html(start, end, (int (*)(void *, unsigned char *, int)) put_chars_conv, (void (*)(void *)) line_break, (void *(*)(void *, int, ...)) html_special, part, head);
	/*if ((part->y -= line_breax) < 0) part->y = 0;*/
}

int margin;

struct table_cache_entry {
	struct table_cache_entry *next;
	struct table_cache_entry *prev;
	struct table_cache_entry *hash_next;
	unsigned char *start;
	unsigned char *end;
	int align;
	int m;
	int width;
	int xs;
	int link_num;
	struct part p;
};

struct list_head table_cache = { &table_cache, &table_cache };

#define TC_HASH_SIZE	4096

struct table_cache_entry *table_cache_hash[TC_HASH_SIZE];

void free_table_cache()
{
	struct table_cache_entry *tce;
	foreach(tce, table_cache) {
		int hash = ((int)(unsigned long)tce->start + tce->xs) & (TC_HASH_SIZE - 1);
		table_cache_hash[hash] = NULL;
	}
	free_list(table_cache);
}

struct part *format_html_part(unsigned char *start, unsigned char *end, int align, int m, int width, struct f_data *data, int xs, int ys, unsigned char *head, int link_num)
{
	struct part *p;
	struct html_element *e;
	int llm = last_link_to_move;
	struct tag *ltm = last_tag_to_move;
	/*struct tag *ltn = last_tag_for_newline;*/
	int lm = margin;
	int ef = empty_format;
	struct form_control *fc;
	struct table_cache_entry *tce;
	if (!data) {
		tce = table_cache_hash[((int)(unsigned long)start + xs) & (TC_HASH_SIZE - 1)];
		while (tce) {
			if (tce->start == start && tce->end == end && tce->align == align && tce->m == m && tce->width == width && tce->xs == xs && tce->link_num == link_num) {
				p = mem_alloc(sizeof(struct part));
				memcpy(p, &tce->p, sizeof(struct part));
				return p;
			}
			tce = tce->hash_next;
		}
	}
	if (ys < 0) {
		internal("format_html_part: ys == %d", ys);
		return NULL;
	}
	if (data) {
		struct node *n;
		n = mem_alloc(sizeof(struct node));
		n->x = xs;
		n->y = ys;
		n->xw = !table_level ? MAXINT : width;
		add_to_list(data->nodes, n);
		/*sdbg(data);*/
	}
	last_link_to_move = data ? data->nlinks : 0;
	last_tag_to_move = data ? (void *)&data->tags : NULL;
	last_tag_for_newline = data ? (void *)&data->tags: NULL;
	margin = m;
	empty_format = !data;
	if (last_link) mem_free(last_link);
	if (last_image) mem_free(last_image);
	if (last_target) mem_free(last_target);
	last_link = last_image = last_target = NULL;
	last_form = NULL;
	nobreak = 1;
	p = mem_alloc(sizeof(struct part));
	p->x = p->y = 0;
	p->data = data;
	p->xp = xs; p->yp = ys;
	p->xmax = p->xa = 0;
	p->bgcolor = find_nearest_color(&par_format.bgcolor, 8);
	p->spaces = DUMMY;
	p->spl = 0;
	p->link_num = link_num;
	init_list(p->uf);
	html_stack_dup();
	e = &html_top;
	html_top.dontkill = 2;
	html_top.namelen = 0;
	par_format.align = align;
	par_format.leftmargin = m;
	par_format.rightmargin = m;
	par_format.width = width;
	par_format.list_level = 0;
	par_format.list_number = 0;
	par_format.dd_margin = 0;
	p->cx = -1;
	p->cy = 0;
	do_format(start, end, p, head);
	if (p->xmax < p->x) p->xmax = p->x;
	nobreak = 0;
	line_breax = 1;
	if (last_link) mem_free(last_link);
	if (last_image) mem_free(last_image);
	if (last_target) mem_free(last_target);
	while (&html_top != e) {
		kill_html_stack_item(&html_top);
		if (!&html_top || (void *)&html_top == (void *)&html_stack) {
			internal("html stack trashed");
			break;
		}
	}
	html_top.dontkill = 0;
	kill_html_stack_item(&html_top);
	mem_free(p->spaces);
	if (data) {
		struct node *n = data->nodes.next;
		n->yw = ys - n->y + p->y;
	}
	foreach(fc, p->uf) destroy_fc(fc);
	free_list(p->uf);
	last_link_to_move = llm;
	last_tag_to_move = ltm;
	/*last_tag_for_newline = ltn;*/
	margin = lm;
	empty_format = ef;
	last_link = last_image = last_target = NULL;
	last_form = NULL;
	if (table_level > 1 && !data) {
		int hash;
		tce = mem_alloc(sizeof(struct table_cache_entry));
		tce->start = start;
		tce->end = end;
		tce->align = align;
		tce->m = m;
		tce->width = width;
		tce->xs = xs;
		tce->link_num = link_num;
		memcpy(&tce->p, p, sizeof(struct part));
		add_to_list(table_cache, tce);
		hash = ((int)(unsigned long)start + xs) & (TC_HASH_SIZE - 1);
		tce->hash_next = table_cache_hash[hash];
		table_cache_hash[hash] = tce;
	}
	return p;
}

void push_base_format(unsigned char *url, struct document_options *opt)
{
	struct html_element *e;
	if (html_stack.next != &html_stack) {
		internal("something on html stack");
		init_list(html_stack);
	}
	e = mem_alloc(sizeof(struct html_element));
	memset(e, 0, sizeof(struct html_element));
	add_to_list(html_stack, e);
	format.attr = 0;
	format.fontsize = 3;
	format.link = format.target = format.image = format.select = NULL;
	format.form = NULL;
	memcpy(&format.fg, &opt->default_fg, sizeof(struct rgb));
	memcpy(&format.bg, &opt->default_bg, sizeof(struct rgb));
	memcpy(&format.clink, &opt->default_link, sizeof(struct rgb));
	memcpy(&format.vlink, &opt->default_vlink, sizeof(struct rgb));
	format.href_base = stracpy(url);
	format.target_base = stracpy(opt->framename);
	par_format.align = opt->plain ? AL_NO : AL_LEFT;
	par_format.leftmargin = opt->plain ? 0 : opt->margin;
	par_format.rightmargin = opt->plain ? 0 : opt->margin;
	par_format.width = opt->xw;
	par_format.list_level = par_format.list_number = 0;
	par_format.dd_margin = opt->margin;
	par_format.flags = 0;
	memcpy(&par_format.bgcolor, &opt->default_bg, sizeof(struct rgb));
	html_top.invisible = 0;
	html_top.name = NULL; html_top.namelen = 0; html_top.options = NULL;
	html_top.linebreak = 1;
	html_top.dontkill = 1;
}

struct conv_table *get_convert_table(unsigned char *head, int to, int def, int *frm, int *aa, int hard)
{
	int from = -1;
	unsigned char *a, *b;
	unsigned char *p = head;
	while (from == -1 && p && (a = parse_http_header(p, "Content-Type", &p))) {
		if ((b = parse_header_param(a, "charset"))) {
			from = get_cp_index(b);
			mem_free(b);
		}
		mem_free(a);
	}
	if (from == -1 && head && (a = parse_http_header(head, "Content-Charset", NULL))) {
		from = get_cp_index(a);
		mem_free(a);
	}
	if (from == -1 && head && (a = parse_http_header(head, "Charset", NULL))) {
		from = get_cp_index(a);
		mem_free(a);
	}
	if (aa) {
		*aa = from == -1;
		if (hard && !*aa) *aa = 2;
	}
	if (hard || from == -1) from = def;
	if (frm) *frm = from;
	return get_translation_table(from, to);
}

struct document_options *d_opt;

void format_html(struct cache_entry *ce, struct f_data *screen)
{
	unsigned char *url = ce->url;
	struct fragment *fr;
	struct part *rp;
	unsigned char *start, *end;
	unsigned char *head, *t;
	int hdl;
	int i;
	memset(table_cache_hash, 0, sizeof(table_cache_hash));
	d_opt = &screen->opt;
	screen->use_tag = ce->count;
	defrag_entry(ce);
	fr = ce->frag.next;
	if ((void *)fr == &ce->frag || fr->offset || !fr->length) start = NULL, end = NULL;
	else start = fr->data, end = fr->data + fr->length;
	startf = start;
	eofff = end;
	head = init_str(), hdl = 0;
	if (ce->head) add_to_str(&head, &hdl, ce->head);
	scan_http_equiv(start, end, &head, &hdl, &t);
	convert_table = get_convert_table(head, screen->opt.cp, screen->opt.assume_cp, &screen->cp, &screen->ass, screen->opt.hard_assume);
	screen->opt.real_cp = screen->cp;
	i = d_opt->plain; d_opt->plain = 0;
	screen->title = convert_string(convert_table, t, strlen(t));
	d_opt->plain = i;
	mem_free(t);
	push_base_format(url, &screen->opt);
	table_level = 0;
	g_ctrl_num = 0;
	last_form_tag = NULL;
	last_form_attr = NULL;
	last_input_tag = NULL;
	if ((rp = format_html_part(start, end, par_format.align, par_format.leftmargin, screen->opt.xw, screen, 0, 0, head, 1))) mem_free(rp);
	mem_free(head);
	screen->x = 0;
	for (i = screen->y - 1; i >= 0; i--) {
		if (!screen->data[i].l) mem_free(screen->data[i].d), screen->y--;
		else break;
	}
	for (i = 0; i < screen->y; i++) if (screen->data[i].l > screen->x) screen->x = screen->data[i].l;
	if (form.action) mem_free(form.action), form.action = NULL;
	if (form.target) mem_free(form.target), form.target = NULL;
	kill_html_stack_item(html_stack.next);
	if (html_stack.next != &html_stack) {
		internal("html stack not empty after operation");
		init_list(html_stack);
	}
	screen->bg = 007 << 8; /* !!! FIXME */
	sort_links(screen);
	if (screen->frame_desc) screen->frame = 1;
	/*{
		FILE *f = fopen("forms", "a");
		struct form_control *form;
		unsigned char *qq;
		fprintf(f,"FORM:\n");
		foreach(form, screen->forms) fprintf(f, "g=%d f=%d c=%d t:%d\n", form->g_ctrl_num, form->form_num, form->ctrl_num, form->type);
		fprintf(f,"fragment: \n");
		for (qq = start; qq < end; qq++) fprintf(f, "%c", *qq);
		fprintf(f,"----------\n\n");
		fclose(f);
	}*/
}

static inline int compare_opt(struct document_options *o1, struct document_options *o2)
{
	if (o1->xw == o2->xw &&
	    o1->yw == o2->yw &&
	    o1->xp == o2->xp &&
	    o1->yp == o2->yp &&
	    o1->col == o2->col &&
	    o1->cp == o2->cp &&
	    o1->assume_cp == o2->assume_cp &&
	    o1->hard_assume == o2->hard_assume &&
	    o1->tables == o2->tables &&
	    o1->frames == o2->frames &&
	    o1->images == o2->images &&
	    o1->margin == o2->margin &&
	    o1->plain == o2->plain &&
	    !memcmp(&o1->default_fg, &o2->default_fg, sizeof(struct rgb)) &&
	    !memcmp(&o1->default_bg, &o2->default_bg, sizeof(struct rgb)) &&
	    !memcmp(&o1->default_link, &o2->default_link, sizeof(struct rgb)) &&
	    !memcmp(&o1->default_vlink, &o2->default_vlink, sizeof(struct rgb)) &&
	    o1->num_links == o2->num_links &&
	    o1->table_order == o2->table_order &&
	    !strcasecmp(o1->framename, o2->framename)) return 0;
	return 1;
}

static inline void copy_opt(struct document_options *o1, struct document_options *o2)
{
	memcpy(o1, o2, sizeof(struct document_options));
	o1->framename = stracpy(o2->framename);
}

struct list_head format_cache = {&format_cache, &format_cache};
int format_cache_entries = 0;

void shrink_format_cache(int u)
{
	struct f_data *ce;
	delete_unused_format_cache_entries();
	if (format_cache_entries < 0) {
		internal("format_cache_entries underflow");
		format_cache_entries = 0;
	}
 	ce = format_cache.prev;
	while ((u || format_cache_entries > max_format_cache_entries) && (void *)ce != &format_cache) {
		if (ce->refcount) {
			ce = ce->prev;
			continue;
		}
		ce = ce->prev;
		destroy_formatted(ce->next);
		format_cache_entries--;
	}
}

void count_format_cache()
{
	struct f_data *ce;
	format_cache_entries = 0;
	foreach(ce, format_cache) if (!ce->refcount) format_cache_entries++;
}

void delete_unused_format_cache_entries()
{
	struct f_data *ce;
	foreach(ce, format_cache) {
		struct cache_entry *cee;
		if (find_in_cache(ce->url, &cee))
			internal("file %s disappeared from cache", ce->url);
		cee->refcount--;
		if (!ce->refcount && cee->count != ce->use_tag) {
			ce = ce->prev;
			destroy_formatted(ce->next);
			format_cache_entries--;
		}
	}
}

void format_cache_reactivate(struct f_data *ce)
{
	del_from_list(ce);
	add_to_list(format_cache, ce);
}

void cached_format_html(struct view_state *vs, struct f_data_c *screen, struct document_options *opt)
{
	unsigned char *n;
	struct f_data *ce;
	struct cache_entry *cee;
	if (!vs) return;
	n = screen->name; screen->name = NULL;
	detach_formatted(screen);
	screen->name = n;
	screen->link_bg = NULL;
	screen->link_bg_n = 0;
	screen->vs = vs;
	screen->xl = screen->yl = -1;
	screen->f_data = NULL;
	foreach(ce, format_cache) if (!strcmp(ce->url, vs->url) && !compare_opt(&ce->opt, opt)) {
		cee = NULL;
		if (find_in_cache(ce->url, &cee))
			internal("file %s disappeared from cache", ce->url);
		cee->refcount--;
		if (cee->count != ce->use_tag) {
			if (!ce->refcount) {
				ce = ce->prev;
				destroy_formatted(ce->next);
				format_cache_entries--;
			}
			continue;
		}
		format_cache_reactivate(ce);
		if (!ce->refcount++) format_cache_entries--;
		screen->f_data = ce;
		goto sx;
	}
	if (find_in_cache(vs->url, &cee)) {
		internal("document to format not found");
		return;
	}
	/*cee->refcount++;*/
	shrink_memory(0);
	ce = mem_alloc(SIZEOF_F_DATA);
	init_formatted(ce);
	ce->refcount = 1;
	ce->url = stracpy(vs->url);
	copy_opt(&ce->opt, opt);
	add_to_list(format_cache, ce);
	screen->f_data = ce;
	ce->time_to_get = -get_time();
	format_html(cee, ce);
	ce->time_to_get += get_time();
	sx:
	screen->xw = ce->opt.xw;
	screen->yw = ce->opt.yw;
	screen->xp = ce->opt.xp;
	screen->yp = ce->opt.yp;
}

long formatted_info(int type)
{
	int i = 0;
	struct f_data *ce;
	switch (type) {
		case CI_FILES:
			foreach(ce, format_cache) i++;
			return i;
		case CI_LOCKED:
			foreach(ce, format_cache) i += !!ce->refcount;
			return i;
		default:
			internal("formatted_info: bad request");
	}
	return 0;
}

void add_frame_to_list(struct session *ses, struct f_data_c *fd)
{
	struct f_data_c *f, *fp;
	foreach(f, ses->scrn_frames) {
		if (f->yp > fd->yp || (f->yp == fd->yp && f->xp > fd->xp)) {
			add_at_pos(f->prev, fd);
			return;
		}
	}
	fp = (struct f_data_c *)ses->scrn_frames.prev;
	add_at_pos(fp, fd);
}

struct f_data_c *find_fd(struct session *ses, unsigned char *name, int depth, int x, int y)
{
	struct f_data_c *fd;
	foreachback(fd, ses->scrn_frames) if (!strcasecmp(fd->name, name) && !fd->used) {
		fd->used = 1;
		fd->depth = depth;
		return fd;
	}
	fd = mem_alloc(sizeof(struct f_data_c));
	memset(fd, 0, sizeof(struct f_data_c));
	fd->used = 1;
	fd->name = stracpy(name);
	fd->depth = depth;
	fd->xp = x, fd->yp = y;
	fd->search_word = &ses->search_word;
	/*add_to_list(ses->scrn_frames, fd);*/
	add_frame_to_list(ses, fd);
	return fd;
}

struct f_data_c *format_frame(struct session *ses, unsigned char *name, unsigned char *url, struct document_options *o, int depth)
{
	struct cache_entry *ce;
	struct view_state *vs;
	struct f_data_c *fd;
	struct frame *fr;
	repeat:
	if (!(fr = ses_find_frame(ses, name))) return NULL;
	vs = &fr->vs;
	if (find_in_cache(vs->url, &ce)) return NULL;
	ce->refcount--;
	if (ce->redirect && fr->redirect_cnt < MAX_REDIRECTS) {
		unsigned char *u;
		if ((u = join_urls(vs->url, ce->redirect))) {
			fr->redirect_cnt++;
			ses_change_frame_url(ses, name, u);
			mem_free(u);
			goto repeat;
		}
	}
	if (!(fd = find_fd(ses, name, depth, o->xp, o->yp))) return NULL;
	cached_format_html(vs, fd, o);
	return fd;
}

void format_frames(struct session *ses, struct frameset_desc *fsd, struct document_options *op, int depth)
{
	int i, j, n;
	struct document_options o;
	if (depth > HTML_MAX_FRAME_DEPTH) return;
	memcpy(&o, op, sizeof(struct document_options));
	if (o.margin) o.margin = 1;
	n = 0;
	for (j = 0; j < fsd->y; j++) {
		o.xp = op->xp;
		for (i = 0; i < fsd->x; i++) {
			struct f_data_c *fdc;
			o.xw = fsd->f[n].xw;
			o.yw = fsd->f[n].yw;
			o.framename = fsd->f[n].name;
			if (fsd->f[n].subframe) format_frames(ses, fsd->f[n].subframe, &o, depth + 1);
			else if (fsd->f[n].name) {
				fdc = format_frame(ses, fsd->f[n].name, fsd->f[n].url, &o, depth);
				if (fdc && fdc->f_data && fdc->f_data->frame) format_frames(ses, fdc->f_data->frame_desc, &o, depth + 1);
			}
			o.xp += o.xw + 1;
			n++;
		}
		o.yp += o.yw + 1;
	}
			
	/*for (i = 0; i < fsd->n; i++) {
		if (!fsd->horiz) o.xw = fsd->f[i].width;
		else o.yw = fsd->f[i].width;
		o.framename = fsd->f[i].name;
		if (fsd->f[i].subframe) format_frames(ses, fsd->f[i].subframe, &o);
		else format_frame(ses, fsd->f[i].name, fsd->f[i].url, &o);
		if (!fsd->horiz) o.xp += fsd->f[i].width + 1;
		else o.yp += fsd->f[i].width + 1;
	}*/
}

void html_interpret(struct session *ses)
{
	struct view_state *l;
	struct document_options o;
	struct f_data_c *fd, *cf = NULL;
	/*debug("I");*/
	memset(&o, 0, sizeof(struct document_options));
	if (!ses->screen) {
		ses->screen = mem_alloc(sizeof(struct f_data_c));
		memset(ses->screen, 0, sizeof(struct f_data_c));
		ses->screen->search_word = &ses->search_word;
	}
	if (!list_empty(ses->history)) l = &cur_loc(ses)->vs;
	else l = NULL;
	o.xp = 0;
	o.yp = 1;
	o.xw = ses->term->x;
	if (ses->term->y < 2) o.yw = 0;
	else o.yw = ses->term->y - 2;
	o.col = ses->term->spec->col;
	o.cp = ses->term->spec->charset;
	ds2do(&ses->ds, &o);
	if ((o.plain = l ? l->plain : 1) == -1) o.plain = 0;
	if (l) l->plain = o.plain;
	memcpy(&o.default_fg, &default_fg, sizeof(struct rgb));
	memcpy(&o.default_bg, &default_bg, sizeof(struct rgb));
	memcpy(&o.default_link, &default_link, sizeof(struct rgb));
	memcpy(&o.default_vlink, &default_vlink, sizeof(struct rgb));
	o.framename = "";
	foreach(fd, ses->scrn_frames) fd->used = 0;
	cached_format_html(l, ses->screen, &o);
	if (ses->screen->f_data && ses->screen->f_data->frame) {
		cf = current_frame(ses);
		format_frames(ses, ses->screen->f_data->frame_desc, &o, 0);
	}
	foreach(fd, ses->scrn_frames) if (!fd->used) {
		struct f_data_c *fdp = fd->prev;
		detach_formatted(fd);
		del_from_list(fd);
		mem_free(fd);
		fd = fdp;
	}
	if (cf) {
		int n = 0;
		foreach(fd, ses->scrn_frames) {
			if (fd->f_data && fd->f_data->frame) continue;
			if (fd == cf) {
				cur_loc(ses)->vs.current_link = n;
				break;
			}
			n++;
		}
	}
}

void add_srch_chr(struct f_data *f, unsigned char c, int x, int y, int nn)
{
	int n = f->nsearch;
	if (c == ' ' && (!n || f->search[n - 1].c == ' ')) return;
	f->search[n].c = c;
	f->search[n].x = x;
	f->search[n].y = y;
	f->search[n].n = nn;
	f->nsearch++;
}

/*void sdbg(struct f_data *f)
{
	struct node *n;
	foreachback(n, f->nodes) {
		int xm = n->x + n->xw, ym = n->y + n->yw;
		printf("%d %d - %d %d\n", n->x, n->y, xm, ym);
		fflush(stdout);
	}
	debug("!");
}*/

void sort_srch(struct f_data *f)
{
	int i;
	int *min, *max;
	if ((unsigned)f->y > MAXINT / sizeof(struct search *)) overalloc();
	if ((unsigned)f->y > MAXINT / sizeof(int)) overalloc();
	f->slines1 = mem_alloc(f->y * sizeof(struct search *));
	f->slines2 = mem_alloc(f->y * sizeof(struct search *));
	min = mem_alloc(f->y * sizeof(int));
	max = mem_alloc(f->y * sizeof(int));
	memset(f->slines1, 0, f->y * sizeof(struct search *));
	memset(f->slines2, 0, f->y * sizeof(struct search *));
	for (i = 0; i < f->y; i++) min[i] = MAXINT, max[i] = 0;
	for (i = 0; i < f->nsearch; i++) {
		struct search *s = &f->search[i];
		if (s->x < min[s->y]) min[s->y] = s->x, f->slines1[s->y] = s;
		if (s->x + s->n > max[s->y]) max[s->y] = s->x + s->n, f->slines2[s->y] = s;
	}
	mem_free(min);
	mem_free(max);
}

static inline int is_spc(chr c)
{
	return (unsigned char)c <= ' ' || c & ATTR_FRAME;
}

int get_srch(struct f_data *f)
{
	struct node *n;
	int cnt = 0;
	int cc = !f->search;
	foreachback(n, f->nodes) {
		int x, y;
		int xm = n->x + n->xw, ym = n->y + n->yw;
		/*printf("%d %d - %d %d\n", n->x, n->y, xm, ym);
		fflush(stdout);*/
		for (y = n->y; y < ym && y < f->y; y++) {
			int ns = 1;
			for (x = n->x; x < xm && x < f->data[y].l; x++) {
				unsigned char c = f->data[y].d[x];
				if (is_spc(f->data[y].d[x])) c = ' ';
				if (c == ' ' && ns) continue;
				c = charset_upcase(c, f->opt.cp);
				if (ns) {
					if (!cc) add_srch_chr(f, c, x, y, 1);
					else cnt++;
					ns = 0;
					continue;
				}
				if (c != ' ')  if (!cc) add_srch_chr(f, c, x, y, 1);
					       else cnt++;
				else {
					int xx;
					for (xx = x + 1; xx < xm && xx < f->data[y].l; xx++) if (!is_spc(f->data[y].d[xx])) goto ja_uz_z_toho_programovani_asi_zcvoknu;
					xx = x;
					ja_uz_z_toho_programovani_asi_zcvoknu:
				/* uz jsem zcvoknul, trpim poruchou osobnosti */
					if (!cc) add_srch_chr(f, ' ', x, y, xx - x);
					else cnt++;
					if (xx == x) break;
					x = xx - 1;
				}
			}
			if (!cc) add_srch_chr(f, ' ', x, y, 0);
			else cnt++;
		}
	}
	return cnt;
	
}

void get_search_data(struct f_data *f)
{
	int n;
	if (f->search) return;
	n = get_srch(f);
	f->nsearch = 0;
	if ((unsigned)n > MAXINT / sizeof(struct search)) overalloc();
	f->search = mem_alloc(n * sizeof(struct search));
	get_srch(f);
	while (f->nsearch && f->search[f->nsearch - 1].c == ' ') f->nsearch--;
	/*debug("!");*/
	sort_srch(f);
}
