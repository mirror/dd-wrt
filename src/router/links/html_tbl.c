#include "links.h"

#define format format_

#define AL_TR		-1

#define VAL_TR		-1
#define VAL_TOP		0
#define VAL_MIDDLE	1
#define VAL_BOTTOM	2

#define W_AUTO		-1
#define W_REL		-2

#define F_VOID		0
#define F_ABOVE		1
#define F_BELOW		2
#define F_HSIDES	3
#define F_LHS		4
#define F_RHS		8
#define F_VSIDES	12
#define F_BOX		15

#define R_NONE		0
#define R_ROWS		1
#define R_COLS		2
#define R_ALL		3
#define R_GROUPS	4

void get_align(char *attr, int *a)
{
	char *al;
	if ((al = get_attr_val(attr, "align"))) {
		if (!(strcasecmp(al, "left"))) *a = AL_LEFT;
		if (!(strcasecmp(al, "right"))) *a = AL_RIGHT;
		if (!(strcasecmp(al, "center"))) *a = AL_CENTER;
		if (!(strcasecmp(al, "justify"))) *a = AL_BLOCK;
		if (!(strcasecmp(al, "char"))) *a = AL_RIGHT; /* NOT IMPLEMENTED */
		mem_free(al);
	}
}

void get_valign(char *attr, int *a)
{
	char *al;
	if ((al = get_attr_val(attr, "valign"))) {
		if (!(strcasecmp(al, "top"))) *a = VAL_TOP;
		if (!(strcasecmp(al, "middle"))) *a = VAL_MIDDLE;
		if (!(strcasecmp(al, "bottom"))) *a = VAL_BOTTOM;
		if (!(strcasecmp(al, "baseline"))) *a = VAL_TOP; /* NOT IMPLEMENTED */
		mem_free(al);
	}
}

void get_c_width(char *attr, int *w, int sh)
{
	char *al;
	if ((al = get_attr_val(attr, "width"))) {
		if (*al && al[strlen(al) - 1] == '*') {
			char *en;
			unsigned long n;
			al[strlen(al) - 1] = 0;
			n = strtoul(al, &en, 10);
			if (n < 10000 && !*en) *w = W_REL - n;
		} else {
			int p = get_width(attr, "width", sh);
			if (p >= 0) *w = p;
		}
		mem_free(al);
	}
}

#define INIT_X		8
#define INIT_Y		8

struct table_cell {
	int used;
	int spanned;
	int mx, my;
	unsigned char *start;
	unsigned char *end;
	int align;
	int valign;
	int b;
	struct rgb bgcolor;
	int group;
	int colspan;
	int rowspan;
	int min_width;
	int max_width;
	int x_width;
	int height;
	int xpos, ypos, xw, yw;
	int link_num;
};

struct table_column {
	int group;
	int align;
	int valign;
	int width;
};

struct table {
	struct part *p;
	int x, y;
	int rx, ry;
	int border, cellpd, vcellpd, cellsp;
	int frame, rules, width, wf;
	int *min_c, *max_c;
	int *w_c;
	int rw;
	int min_t, max_t;
	struct table_cell *cells;
	int c, rc;
	struct table_column *cols;
	int xc;
	int *xcols;
	int *r_heights;
	int rh;
	int link_num;
};

#define CELL(t, x, y) (&(t)->cells[(y) * (t)->rx + (x)])

unsigned char frame_table[81] = {
	0x00, 0xb3, 0xba,	0xc4, 0xc0, 0xd3,	0xcd, 0xd4, 0xc8,
	0xc4, 0xd9, 0xbd,	0xc4, 0xc1, 0xd0,	0xcd, 0xd4, 0xc8,
	0xcd, 0xbe, 0xbc,	0xcd, 0xbe, 0xbc,	0xcd, 0xcf, 0xca,

	0xb3, 0xb3, 0xba,	0xda, 0xc3, 0xd3,	0xd5, 0xc6, 0xc8,
	0xbf, 0xb4, 0xbd,	0xc2, 0xc5, 0xd0,	0xd5, 0xc6, 0xc8,
	0xb8, 0xb5, 0xbc,	0xb8, 0xb5, 0xbc,	0xd1, 0xd8, 0xca,

	0xba, 0xba, 0xba,	0xd6, 0xd6, 0xc7,	0xc9, 0xc9, 0xcc,
	0xb7, 0xb7, 0xb6,	0xd2, 0xd2, 0xd7,	0xc9, 0xc9, 0xcc,
	0xbb, 0xbb, 0xb9,	0xbb, 0xbb, 0xb9,	0xcb, 0xcb, 0xce,
};

unsigned char hline_table[3] = { 0x20, 0xc4, 0xcd };
unsigned char vline_table[3] = { 0x20, 0xb3, 0xba };

struct table *new_table()
{
	struct table *t;
	t = mem_alloc(sizeof(struct table));
	memset(t, 0, sizeof(struct table));
	t->p = NULL;
	t->x = t->y = 0;
	t->rx = INIT_X;
	t->ry = INIT_Y;
	t->cells = mem_alloc(INIT_X * INIT_Y * sizeof(struct table_cell));
	memset(t->cells, 0, INIT_X * INIT_Y * sizeof(struct table_cell));
	t->c = 0;
	t->rc = INIT_X;
	t->cols = mem_alloc(INIT_X * sizeof(struct table_column));
	memset(t->cols, 0, INIT_X * sizeof(struct table_column));
	t->xcols = DUMMY;
	t->xc = 0;
	t->r_heights = DUMMY;
	return t;
}

void free_table(struct table *t)
{
	if (t->min_c) mem_free(t->min_c);
	if (t->max_c) mem_free(t->max_c);
	if (t->w_c) mem_free(t->w_c);
	mem_free(t->r_heights);
	mem_free(t->cols);
	mem_free(t->xcols);
	mem_free(t->cells);
	mem_free(t);
}

void expand_cells(struct table *t, int x, int y)
{
	int i, j;
	if (x >= t->x) {
		if (t->x) {
			for (i = 0; i < t->y; i++) if (CELL(t, t->x - 1, i)->colspan == -1) {
				for (j = t->x; j <= x; j++) {
					CELL(t, j, i)->used = 1;
					CELL(t, j, i)->spanned = 1;
					CELL(t, j, i)->rowspan = CELL(t, t->x - 1, i)->rowspan;
					CELL(t, j, i)->colspan = -1;
					CELL(t, j, i)->mx = CELL(t, t->x - 1, i)->mx;
					CELL(t, j, i)->my = CELL(t, t->x - 1, i)->my;
				}
			}
		}
		t->x = x + 1;
	}
	if (y >= t->y) {
		if (t->y) {
			for (i = 0; i < t->x; i++) if (CELL(t, i, t->y - 1)->rowspan == -1) {
				for (j = t->y; j <= y; j++) {
					CELL(t, i, j)->used = 1;
					CELL(t, i, j)->spanned = 1;
					CELL(t, i, j)->rowspan = -1;
					CELL(t, i, j)->colspan = CELL(t, i, t->y - 1)->colspan;
					CELL(t, i, j)->mx = CELL(t, i, t->y - 1)->mx;
					CELL(t, i, j)->my = CELL(t, i, t->y - 1)->my;
				}
			}
		}
		t->y = y + 1;
	}
}

struct table_cell *new_cell(struct table *t, int x, int y)
{
	struct table nt;
	int i, j;
	if (x < t->x && y < t->y) goto ret;
	rep:
	if (x < t->rx && y < t->ry) {
		expand_cells(t, x, y);
		goto ret;
	}
	nt.rx = t->rx;
	nt.ry = t->ry;
	while (x >= nt.rx) {
		if ((unsigned)nt.rx > MAXINT / 2) overalloc();
		nt.rx *= 2;
	}
	while (y >= nt.ry) {
		if ((unsigned)nt.ry > MAXINT / 2) overalloc();
		nt.ry *= 2;
	}
	if ((unsigned)nt.rx * (unsigned)nt.ry / (unsigned)nt.rx != (unsigned)nt.ry) overalloc();
	if ((unsigned)nt.rx * (unsigned)nt.ry > MAXINT / sizeof(struct table_cell)) overalloc();
	nt.cells = mem_alloc(nt.rx * nt.ry * sizeof(struct table_cell));
	memset(nt.cells, 0, nt.rx * nt.ry * sizeof(struct table_cell));
	for (i = 0; i < t->x; i++)
		for (j = 0; j < t->y; j++)
			memcpy(CELL(&nt, i, j), CELL(t, i, j), sizeof(struct table_cell));
	mem_free(t->cells);
	t->cells = nt.cells;
	t->rx = nt.rx;
	t->ry = nt.ry;
	goto rep;

	ret:
	return CELL(t, x, y);
}

void new_columns(struct table *t, int span, int width, int align, int valign, int group)
{
	if ((unsigned)t->c + (unsigned)span > MAXINT) overalloc();
	if (t->c + span > t->rc) {
		int n = t->rc;
		struct table_column *nc;
		while (t->c + span > n) {
			if ((unsigned)n > MAXINT / 2) overalloc();
			n *= 2;
		}
		if ((unsigned)n > MAXINT / sizeof(struct table_column)) overalloc();
		nc = mem_realloc(t->cols, n * sizeof(struct table_column));
		t->rc = n;
		t->cols = nc;
	}
	while (span--) {
		t->cols[t->c].align = align;
		t->cols[t->c].valign = valign;
		t->cols[t->c].width = width;
		t->cols[t->c++].group = group;
		group = 0;
	}
}

void set_td_width(struct table *t, int x, int width, int f)
{
	if (x >= t->xc) {
		int n = t->xc ? t->xc : 1;
		int i;
		int *nc;
		while (x >= n) {
			if ((unsigned)n > MAXINT / 2) overalloc();
			n *= 2;
		}
		if ((unsigned)n > MAXINT / sizeof(int)) overalloc();
		nc = mem_realloc(t->xcols, n * sizeof(int));
		for (i = t->xc; i < n; i++) nc[i] = W_AUTO;
		t->xc = n;
		t->xcols = nc;
	}
	if (t->xcols[x] == W_AUTO || f) {
		set:
		t->xcols[x] = width;
		return;
	}
	if (width == W_AUTO) return;
	if (width < 0 && t->xcols[x] >= 0) goto set;
	if (width >= 0 && t->xcols[x] < 0) return;
	t->xcols[x] = (t->xcols[x] + width) / 2;
}

unsigned char *skip_table(unsigned char *html, unsigned char *eof)
{
	int level = 1;
	unsigned char *name;
	int namelen;
	r:
	while (html < eof && (*html != '<' || parse_element(html, eof, &name, &namelen, NULL, &html))) html++;
	if (html >= eof) return eof;
	if (namelen == 5 && !casecmp(name, "TABLE", 5)) level++;
	if (namelen == 6 && !casecmp(name, "/TABLE", 6)) if (!--level) return html;
	goto r;
}

struct s_e {
	unsigned char *s, *e;
};

struct table *parse_table(unsigned char *html, unsigned char *eof, unsigned char **end, struct rgb *bgcolor, int sh, struct s_e **bad_html, int *bhp)
{
	int qqq;
	struct table *t;
	struct table_cell *cell;
	unsigned char *t_name, *t_attr, *en;
	int t_namelen;
	int x = 0, y = -1;
	int p = 0;
	unsigned char *lbhp = NULL;
	int l_al = AL_LEFT;
	int l_val = VAL_MIDDLE;
	int csp, rsp;
	int group = 0;
	int i, j, k;
	struct rgb l_col;
	int c_al = AL_TR, c_val = VAL_TR, c_width = W_AUTO, c_span = 0;
	memcpy(&l_col, bgcolor, sizeof(struct rgb));
	*end = html;
	if (bad_html) {
		*bad_html = DUMMY;
		*bhp = 0;
	}
	if (!(t = new_table())) return NULL;
	se:
	en = html;
	see:
	html = en;
	if (bad_html && !p && !lbhp) {
		if (!(*bhp & (ALLOC_GR-1))) {
			if ((unsigned)*bhp > MAXINT / sizeof(struct s_e) - ALLOC_GR) overalloc();
			*bad_html = mem_realloc(*bad_html, (*bhp + ALLOC_GR) * sizeof(struct s_e));
		}
		lbhp = (*bad_html)[(*bhp)++].s = html;
	}
	while (html < eof && *html != '<') html++;
	if (html >= eof) {
		if (p) CELL(t, x, y)->end = html;
		if (lbhp) (*bad_html)[*bhp-1].e = html;
		goto scan_done;
	}
	if (html + 2 <= eof && (html[1] == '!' || html[1] == '?')) {
		html = skip_comment(html, eof);
		goto se;
	}
	if (parse_element(html, eof, &t_name, &t_namelen, &t_attr, &en)) {
		html++;
		goto se;
	}
	if (t_namelen == 5 && !casecmp(t_name, "TABLE", 5)) {
		en = skip_table(en, eof);
		goto see;
	}
	if (t_namelen == 6 && !casecmp(t_name, "/TABLE", 6)) {
		if (c_span) new_columns(t, c_span, c_width, c_al, c_val, 1);
		if (p) CELL(t, x, y)->end = html;
		if (lbhp) (*bad_html)[*bhp-1].e = html;
		goto scan_done;
	}
	if (t_namelen == 8 && !casecmp(t_name, "COLGROUP", 8)) {
		if (c_span) new_columns(t, c_span, c_width, c_al, c_val, 1);
		if (lbhp) (*bad_html)[*bhp-1].e = html, lbhp = NULL;
		c_al = AL_TR;
		c_val = VAL_TR;
		c_width = W_AUTO;
		get_align(t_attr, &c_al);
		get_valign(t_attr, &c_val);
		get_c_width(t_attr, &c_width, sh);
		if ((c_span = get_num(t_attr, "span")) == -1) c_span = 1;
		goto see;
	}
	if (t_namelen == 9 && !casecmp(t_name, "/COLGROUP", 9)) {
		if (c_span) new_columns(t, c_span, c_width, c_al, c_val, 1);
		if (lbhp) (*bad_html)[*bhp-1].e = html, lbhp = NULL;
		c_span = 0;
		c_al = AL_TR;
		c_val = VAL_TR;
		c_width = W_AUTO;
		goto see;
	}
	if (t_namelen == 3 && !casecmp(t_name, "COL", 3)) {
		int sp, wi, al, val;
		if (lbhp) (*bad_html)[*bhp-1].e = html, lbhp = NULL;
		if ((sp = get_num(t_attr, "span")) == -1) sp = 1;
		wi = c_width;
		al = c_al;
		val = c_val;
		get_align(t_attr, &al);
		get_valign(t_attr, &val);
		get_c_width(t_attr, &wi, sh);
		new_columns(t, sp, wi, al, val, !!c_span);
		c_span = 0;
		goto see;
	}
	if (t_namelen == 3 && (!casecmp(t_name, "/TR", 3) || !casecmp(t_name, "/TD", 3) || !casecmp(t_name, "/TH", 3))) {
		if (c_span) new_columns(t, c_span, c_width, c_al, c_val, 1);
		if (p) CELL(t, x, y)->end = html, p = 0;
		if (lbhp) (*bad_html)[*bhp-1].e = html, lbhp = NULL;
	}
	if (t_namelen == 2 && !casecmp(t_name, "TR", 2)) {
		if (c_span) new_columns(t, c_span, c_width, c_al, c_val, 1);
		if (p) CELL(t, x, y)->end = html, p = 0;
		if (lbhp) (*bad_html)[*bhp-1].e = html, lbhp = NULL;
		if (group) group--;
		l_al = AL_LEFT;
		l_val = VAL_MIDDLE;
		memcpy(&l_col, bgcolor, sizeof(struct rgb));
		get_align(t_attr, &l_al);
		get_valign(t_attr, &l_val);
		get_bgcolor(t_attr, &l_col);
		y++, x = 0;
		goto see;
	}
	if (t_namelen == 5 && ((!casecmp(t_name, "THEAD", 5)) || (!casecmp(t_name, "TBODY", 5)) || (!casecmp(t_name, "TFOOT", 5)))) {
		if (c_span) new_columns(t, c_span, c_width, c_al, c_val, 1);
		if (lbhp) (*bad_html)[*bhp-1].e = html, lbhp = NULL;
		group = 2;
	}
	if (t_namelen != 2 || (casecmp(t_name, "TD", 2) && casecmp(t_name, "TH", 2))) goto see;
	if (c_span) new_columns(t, c_span, c_width, c_al, c_val, 1);
	if (lbhp) (*bad_html)[*bhp-1].e = html, lbhp = NULL;
	if (p) CELL(t, x, y)->end = html, p = 0;
	if (y == -1) y = 0, x = 0;
	nc:
	cell = new_cell(t, x, y);
	if (cell->used) {
		if (cell->colspan == -1) goto see;
		x++;
		goto nc;
	}
	cell->mx = x;
	cell->my = y;
	cell->used = 1;
	cell->start = en;
	p = 1;
	cell->align = l_al;
	cell->valign = l_val;
	if ((cell->b = upcase(t_name[1]) == 'H')) cell->align = AL_CENTER;
	if (group == 1) cell->group = 1;
	if (x < t->c) {
		if (t->cols[x].align != AL_TR) cell->align = t->cols[x].align;
		if (t->cols[x].valign != VAL_TR) cell->valign = t->cols[x].valign;
	}
	memcpy(&cell->bgcolor, &l_col, sizeof(struct rgb));
	get_align(t_attr, &cell->align);
	get_valign(t_attr, &cell->valign);
	get_bgcolor(t_attr, &cell->bgcolor);
	if ((csp = get_num(t_attr, "colspan")) == -1) csp = 1;
	if (!csp) csp = -1;
	if ((rsp = get_num(t_attr, "rowspan")) == -1) rsp = 1;
	if (!rsp) rsp = -1;
	if (csp >= 0 && rsp >= 0 && csp * rsp > 100000) {
		if (csp > 10) csp = -1;
		if (rsp > 10) rsp = -1;
	}
	cell->colspan = csp;
	cell->rowspan = rsp;
	if (csp == 1) {
		int w = W_AUTO;
		get_c_width(t_attr, &w, sh);
		if (w != W_AUTO) set_td_width(t, x, w, 0);
	}
	qqq = t->x;
	for (i = 1; csp != -1 ? i < csp : x + i < qqq; i++) {
		struct table_cell *sc = new_cell(t, x + i, y);
		if (sc->used) {
			csp = i;
			for (k = 0; k < i; k++) CELL(t, x + k, y)->colspan = csp;
			break;
		}
		sc->used = sc->spanned = 1;
		sc->rowspan = rsp;
		sc->colspan = csp;
		sc->mx = x;
		sc->my = y;
	}
	qqq = t->y;
	for (j = 1; rsp != -1 ? j < rsp : y + j < qqq; j++) {
		for (k = 0; k < i; k++) {
			struct table_cell *sc = new_cell(t, x + k, y + j);
			if (sc->used) {
				int l, m;
				if (sc->mx == x && sc->my == y) continue;
				/*internal("boo");*/
				for (l = 0; l < k; l++) memset(CELL(t, x + l, y + j), 0, sizeof(struct table_cell));
				rsp = j;
				for (l = 0; l < i; l++) for (m = 0; m < j; m++) CELL(t, x + l, y + m)->rowspan = j;
				goto brk;
			}
			sc->used = sc->spanned = 1;
			sc->rowspan = rsp;
			sc->colspan = csp;
			sc->mx = x;
			sc->my = y;
		}
	}
	brk:
	goto see;

	scan_done:
	*end = html;

	for (x = 0; x < t->x; x++) for (y = 0; y < t->y; y++) {
		struct table_cell *c = CELL(t, x, y);
		if (!c->spanned) {
			if (c->colspan == -1) c->colspan = t->x - x;
			if (c->rowspan == -1) c->rowspan = t->y - y;
		}
	}

	if ((unsigned)t->y > MAXINT / sizeof(int)) overalloc();
	t->r_heights = mem_alloc(t->y * sizeof(int));
	memset(t->r_heights, 0, t->y * sizeof(int));

	for (x = 0; x < t->c; x++) if (t->cols[x].width != W_AUTO) set_td_width(t, x, t->cols[x].width, 1);
	set_td_width(t, t->x, W_AUTO, 0);

	return t;
}

void get_cell_width(char *start, char *end, int cellpd, int w, int a, int *min, int *max, int n_link, int *n_links)
{
	struct part *p;
	if (min) *min = -1;
	if (max) *max = -1;
	if (n_links) *n_links = n_link;
	if (!(p = format_html_part(start, end, AL_LEFT, cellpd, w, NULL, !!a, !!a, NULL, n_link))) return;
	if (min) *min = p->x;
	if (max) *max = p->xmax;
	if (n_links) *n_links = p->link_num;
	/*if (min && max && *min > *max) internal("get_cell_width: %d > %d", *min, *max);*/
	mem_free(p);
}

static inline void check_cell_widths(struct table *t)
{
	int i, j;
	for (j = 0; j < t->y; j++) for (i = 0; i < t->x; i++) {
		int min, max;
		struct table_cell *c = CELL(t, i, j);
		if (!c->start) continue;
		get_cell_width(c->start, c->end, t->cellpd, 0, 0, &min, &max, c->link_num, NULL);
		/*if (min != c->min_width || max < c->max_width) internal("check_cell_widths failed");*/
	}
}

#define g_c_w(cc)							\
do {									\
		struct table_cell *c = cc;				\
		if (!c->start) continue;				\
		c->link_num = nl;					\
		get_cell_width(c->start, c->end, t->cellpd, 0, 0, &c->min_width, &c->max_width, nl, &nl);\
} while (0)

void get_cell_widths(struct table *t)
{
	int nl = t->p->link_num;
	int i, j;
	if (!d_opt->table_order)
		for (j = 0; j < t->y; j++) for (i = 0; i < t->x; i++) g_c_w(CELL(t, i, j));
	else
		for (i = 0; i < t->x; i++) for (j = 0; j < t->y; j++) g_c_w(CELL(t, i, j));
	t->link_num = nl;
}

void dst_width(int *p, int n, int w, int *lim)
{
	int i, s = 0, d, r;
	for (i = 0; i < n; i++) s += p[i];
	if (s >= w) return;
	if (!n) return;
	again:
	d = (w - s) / n;
	r = (w - s) % n;
	w = 0;
	for (i = 0; i < n; i++) {
		p[i] += d + (i < r);
		if (lim && p[i] > lim[i]) w += p[i] - lim[i], p[i] = lim[i];
	}
	if (w) {
		/*if (!lim) internal("bug in dst_width");*/
		lim = NULL;
		s = 0;
		goto again;
	}
}

int get_vline_width(struct table *t, int col)
{			/* return: -1 none, 0, space, 1 line, 2 double */
	int w = 0;
	if (!col) return -1;
	if (t->rules == R_COLS || t->rules == R_ALL) w = t->cellsp;
	else if (t->rules == R_GROUPS) w = col < t->c && t->cols[col].group;
	if (!w && t->cellpd) w = -1;
	return w;
}

int get_hline_width(struct table *t, int row)
{
	int w = 0;
	if (!row) return -1;
	if (t->rules == R_ROWS || t->rules == R_ALL) {
		x:
		if (t->cellsp || t->vcellpd) return t->cellsp;
		return -1;
	}
	else if (t->rules == R_GROUPS) {
		int q;
		for (q = 0; q < t->x; q++) if (CELL(t, q, row)->group) goto x;
		return t->vcellpd ? 0 : -1;
	}
	if (!w && !t->vcellpd) w = -1;
	return w;
}
	
int get_column_widths(struct table *t)
{
	int i, j, s, ns;
	if ((unsigned)t->x > MAXINT / sizeof(int)) overalloc();
	if (!t->min_c) t->min_c = mem_alloc(t->x * sizeof(int));
	if (!t->max_c) t->max_c = mem_alloc(t->x * sizeof(int));
	if (!t->w_c) t->w_c = mem_alloc(t->x * sizeof(int));
	memset(t->min_c, 0, t->x * sizeof(int));
	memset(t->max_c, 0, t->x * sizeof(int));
	s = 1;
	do {
		ns = MAXINT;
		for (i = 0; i < t->x; i++) for (j = 0; j < t->y; j++) {
			struct table_cell *c = CELL(t, i, j);
			if (c->spanned || !c->used) continue;
			if (c->colspan + i > t->x) {
				/*internal("colspan out of table");
				return -1;*/
				continue;
			}
			if (c->colspan == s) {
				int k, p = 0;
				/*int pp = t->max_c[i];*/
				int m = 0;
				for (k = 1; k < s; k++) {
					p += get_vline_width(t, i + k) >= 0;
					/*pp += t->max_c[i + k];*/
				}
				/*if (0 && s > 1 && (t->p->data || t->p->xp)) {
					int d, cc = (!!(t->frame & F_LHS) + !!(t->frame & F_RHS)) * !!t->border;
					for (d = 0; d < t->c; c++) {
						cc += t->max_c[d];
						if (d > 0) d += get_vline_width(t, d) >= 0;
					}
					if (cc >= t->width) goto nd;
					if (cc + c->max_width - p - pp >= t->width) {
						m = cc + c->max_width - p - pp - t->width;
					}
				}*/
				dst_width(t->min_c + i, s, c->min_width - p, t->max_c + i);
				dst_width(t->max_c + i, s, c->max_width - p - m, NULL);
				for (k = 0; k < s; k++) if (t->min_c[i + k] > t->max_c[i + k]) t->max_c[i + k] = t->min_c[i + k];
			} else if (c->colspan > s && c->colspan < ns) ns = c->colspan;
		}
	} while ((s = ns) != MAXINT);
	return 0;
}

void get_table_width(struct table *t)
{
	int i, vl;
	int min = 0, max = 0;
	for (i = 0; i < t->x; i++) {
		vl = get_vline_width(t, i) >= 0;
		min += vl, max += vl;
		min += t->min_c[i];
		if (t->xcols[i] > t->max_c[i]) max += t->xcols[i];
		max += t->max_c[i];
	}
	vl = (!!(t->frame & F_LHS) + !!(t->frame & F_RHS)) * !!t->border;
	min += vl, max += vl;
	t->min_t = min;
	t->max_t = max;
	/*if (min > max) internal("min(%d) > max(%d)", min, max);*/
}

void distribute_widths(struct table *t, int width)
{
	int i;
	int d = width - t->min_t;
	int om = 0;
	char *u;
	int *w, *mx;
	int mmax_c = 0;
	if (!t->x) return;
	if (d < 0) {
		/*internal("too small width %d, required %d", width, t->min_t);*/
		return;
	}
	for (i = 0; i < t->x; i++) if (t->max_c[i] > mmax_c) mmax_c = t->max_c[i];
	memcpy(t->w_c, t->min_c, t->x * sizeof(int));
	t->rw = width;
	if ((unsigned)t->x > MAXINT / sizeof(int)) overalloc();
	u = mem_alloc(t->x);
	w = mem_alloc(t->x * sizeof(int));
	mx = mem_alloc(t->x * sizeof(int));
	while (d) {
		int mss, mii;
		int p = 0;
		int wq;
		int dd;
		memset(w, 0, t->x * sizeof(int));
		memset(mx, 0, t->x * sizeof(int));
		for (i = 0; i < t->x; i++) {
			switch (om) {
				case 0:
					if (t->w_c[i] < t->xcols[i]) {
						w[i] = 1, mx[i] = (t->xcols[i] > t->max_c[i] ? t->max_c[i] : t->xcols[i]) - t->w_c[i];
						if (mx[i] <= 0) w[i] = 0;
					}
					break;
				case 1:
					if (t->xcols[i] < -1 && t->xcols[i] != -2) {
						w[i] = t->xcols[i] <= -2 ? -2 - t->xcols[i] : 1;
						mx[i] = t->max_c[i] - t->w_c[i];
						if (mx[i] <= 0) w[i] = 0;
					}
					break;
				case 2:
				case 3:
					if (t->w_c[i] < t->max_c[i] && (om == 3 || t->xcols[i] == W_AUTO)) {
						mx[i] = t->max_c[i] - t->w_c[i];
						if (mmax_c) w[i] = 5 + t->max_c[i] * 10 / mmax_c;
						else w[i] = 1;
					}
					break;
				case 4:
					if (t->xcols[i] >= 0) {
						w[i] = 1, mx[i] = t->xcols[i] - t->w_c[i];
						if (mx[i] <= 0) w[i] = 0;
					}
					break;
				case 5:
					if (t->xcols[i] < 0) w[i] = t->xcols[i] <= -2 ? -2 - t->xcols[i] : 1, mx[i] = MAXINT;
					break;
				case 6:
					w[i] = 1, mx[i] = MAXINT;
					break;
				default:
					/*internal("could not expand table");*/
					goto end2;
			}
			p += w[i];
		}
		if (!p) {
			om++;
			continue;
		}
		wq = 0;
		if (u) memset(u, 0, t->x);
		dd = d;
		a:
		mss = 0; mii = -1;
		for (i = 0; i < t->x; i++) if (w[i]) {
			int ss;
			if (u && u[i]) continue;
			if (!(ss = dd * w[i] / p)) ss = 1;
			if (ss > mx[i]) ss = mx[i];
			if (ss > mss) mss = ss, mii = i;
		}
		if (mii != -1) {
			int q = t->w_c[mii];
			if (u) u[mii] = 1;
			t->w_c[mii] += mss;
			d -= t->w_c[mii] - q;
			while (d < 0) t->w_c[mii]--, d++;
			if (t->w_c[mii] < q) {
				/*internal("shrinking cell");*/
				t->w_c[mii] = q;
			}
			wq = 1;
			if (d) goto a;
		} else if (!wq) om++;
	}
	end2:
	mem_free(mx);
	mem_free(w);
	if (u) mem_free(u);
}

#ifdef HTML_TABLE_2ND_PASS
void check_table_widths(struct table *t)
{
	int *w;
	int i, j;
	int s, ns;
	int m, mi = 0; /* go away, warning! */
	if ((unsigned)t->x > MAXINT / sizeof(int)) overalloc();
	w = mem_alloc(t->x * sizeof(int));
	memset(w, 0, t->x * sizeof(int));
	for (j = 0; j < t->y; j++) for (i = 0; i < t->x; i++) {
		struct table_cell *c = CELL(t, i, j);
		int k, p = 0;
		if (!c->start) continue;
		for (k = 1; k < c->colspan; k++) p += get_vline_width(t, i + k) >= 0;
		for (k = 0; k < c->colspan; k++) p += t->w_c[i + k];
		get_cell_width(c->start, c->end, t->cellpd, p, 1, &c->x_width, NULL, c->link_num, NULL);
		if (c->x_width > p) {
			/*int min, max;
			get_cell_width(c->start, c->end, t->cellpd, 0, 0, &min, &max, c->link_num, NULL);
			internal("cell is now wider (%d > %d) min = %d, max = %d, now_min = %d, now_max = %d", c->x_width, p, t->min_c[i], t->max_c[i], min, max);*/
			/* sbohem, internale. chytl jsi mi spoustu chyb v tabulkovaci, ale ted je proste cas jit ... ;-( */
			c->x_width = p;
		}
	}
	s = 1;
	do {
		ns = MAXINT;
		for (i = 0; i < t->x; i++) for (j = 0; j < t->y; j++) {
			struct table_cell *c = CELL(t, i, j);
			if (!c->start) continue;
			if (c->colspan + i > t->x) {
				/*internal("colspan out of table");*/
				mem_free(w);
				return;
			}
			if (c->colspan == s) {
				int k, p = 0;
				for (k = 1; k < s; k++) p += get_vline_width(t, i + k) >= 0;
				dst_width(w + i, s, c->x_width - p, t->max_c + i);
				/*for (k = i; k < i + s; k++) if (w[k] > t->w_c[k]) {
					int l;
					int c;
					ag:
					c = 0;
					for (l = i; l < i + s; l++) if (w[l] < t->w_c[k]) w[l]++, w[k]--, c = 1;
					if (w[k] > t->w_c[k]) {
						if (!c) internal("can't shrink cell");
						else goto ag;
					}
				}*/
			} else if (c->colspan > s && c->colspan < ns) ns = c->colspan;
		}
	} while ((s = ns) != MAXINT);

	s = 0; ns = 0;
	for (i = 0; i < t->x; i++) {
		s += t->w_c[i], ns += w[i];
		/*if (w[i] > t->w_c[i]) {
			int k;
			for (k = 0; k < t->x; k++) debug("%d, %d", t->w_c[k], w[k]);
			debug("column %d: new width(%d) is larger than previous(%d)", i, w[i], t->w_c[i]);
		}*/
	}
	if (ns > s) {
		/*internal("new width(%d) is larger than previous(%d)", ns, s);*/
		mem_free(w);
		return;
	}
	m = -1;
	for (i = 0; i < t->x; i++) {
		/*if (table_level == 1) debug("%d: %d %d %d %d", i, t->max_c[i], t->min_c[i], t->w_c[i], w[i]);*/
		if (t->max_c[i] > m) m = t->max_c[i], mi = i;
	}
	/*if (table_level == 1) debug("%d %d", mi, s - ns);*/
	if (m != -1) {
		w[mi] += s - ns;
		if (w[mi] <= t->max_c[mi]) {
			mem_free(t->w_c);
			t->w_c = w;
			return;
		}
	}
	mem_free(w);
}
#endif

void get_table_heights(struct table *t)
{
	int s, ns;
	int i, j;
	for (j = 0; j < t->y; j++) {
		for (i = 0; i < t->x; i++) {
			struct table_cell *cell = CELL(t, i, j);
			struct part *p;
			int xw = 0, sp;
			if (!cell->used || cell->spanned) continue;
			for (sp = 0; sp < cell->colspan; sp++) {
				xw += t->w_c[i + sp];
				if (sp < cell->colspan - 1) xw += get_vline_width(t, i + sp + 1) >= 0;
			}
			if (!(p = format_html_part(cell->start, cell->end, cell->align, t->cellpd, xw, NULL, 2, 2, NULL, cell->link_num))) return;
			cell->height = p->y;
				/*debug("%d, %d.",xw, cell->height);*/
			mem_free(p);
		}
	}
	s = 1;
	do {
		ns = MAXINT;
		for (j = 0; j < t->y; j++) {
			for (i = 0; i < t->x; i++) {
				struct table_cell *cell = CELL(t, i, j);
				if (!cell->used || cell->spanned) continue;
				if (cell->rowspan == s) {
					int k, p = 0;
					for (k = 1; k < s; k++) p += get_hline_width(t, j + k) >= 0;
					dst_width(t->r_heights + j, s, cell->height - p, NULL);
				} else if (cell->rowspan > s && cell->rowspan < ns) ns = cell->rowspan;
			}
		}
	} while ((s = ns) != MAXINT);
	t->rh = (!!(t->frame & F_ABOVE) + !!(t->frame & F_BELOW)) * !!t->border;
	for (j = 0; j < t->y; j++) {
		t->rh += t->r_heights[j];
		if (j) t->rh += get_hline_width(t, j) >= 0;
	}
}

void display_complicated_table(struct table *t, int x, int y, int *yy)
{
	int i, j;
	struct f_data *f = t->p->data;
	int yp, xp = x + ((t->frame & F_LHS) && t->border);
	for (i = 0; i < t->x; i++) {
		yp = y + ((t->frame & F_ABOVE) && t->border);
		for (j = 0; j < t->y; j++) {
			struct table_cell *cell = CELL(t, i, j);
			if (cell->start) {
				int yt;
				struct part *p = NULL;
				int xw = 0, yw = 0, s;
				for (s = 0; s < cell->colspan; s++) {
					xw += t->w_c[i + s];
					if (s < cell->colspan - 1) xw += get_vline_width(t, i + s + 1) >= 0;
				}
				for (s = 0; s < cell->rowspan; s++) {
					yw += t->r_heights[j + s];
					if (s < cell->rowspan - 1) yw += get_hline_width(t, j + s + 1) >= 0;
				}
				html_stack_dup();
				html_top.dontkill = 1;
				if (cell->b) format.attr |= AT_BOLD;
				memcpy(&format.bg, &cell->bgcolor, sizeof(struct rgb));
				memcpy(&par_format.bgcolor, &cell->bgcolor, sizeof(struct rgb));
				p = format_html_part(cell->start, cell->end, cell->align, t->cellpd, xw, f, t->p->xp + xp, t->p->yp + yp + (cell->valign != VAL_MIDDLE && cell->valign != VAL_BOTTOM ? 0 : (yw - cell->height) / (cell->valign == VAL_MIDDLE ? 2 : 1)), NULL, cell->link_num);
				cell->xpos = xp;
				cell->ypos = yp;
				cell->xw = xw;
				cell->yw = yw;
				for (yt = 0; yt < p->y; yt++) {
					xxpand_lines(t->p, yp + yt);
					xxpand_line(t->p, yp + yt, xp + t->w_c[i]);
				}
				kill_html_stack_item(&html_top);
				mem_free(p);
			}
			cell->xpos = xp;
			cell->ypos = yp;
			cell->xw = t->w_c[i];
			yp += t->r_heights[j];
			if (j < t->y - 1) yp += (get_hline_width(t, j + 1) >= 0);
		}
		if (i < t->x - 1) xp += t->w_c[i] + (get_vline_width(t, i + 1) >= 0);
	}
	yp = y;
	for (j = 0; j < t->y; j++) {
		yp += t->r_heights[j];
		if (j < t->y - 1) yp += (get_hline_width(t, j + 1) >= 0);
	}
	*yy = yp + (!!(t->frame & F_ABOVE) + !!(t->frame & F_BELOW)) * !!t->border;
}

/* !!! FIXME: background */
#define draw_frame_point(xx, yy, ii, jj)	\
if (H_LINE_X((ii-1), (jj)) >= 0 || H_LINE_X((ii), (jj)) >= 0 || V_LINE_X((ii), (jj-1)) >= 0 || V_LINE_X((ii), (jj)) >= 0) xset_hchar(t->p, (xx), (yy), frame_table[V_LINE((ii),(jj)-1)+3*H_LINE((ii),(jj))+9*H_LINE((ii)-1,(jj))+27*V_LINE((ii),(jj))] | ATTR_FRAME)

#define draw_frame_hline(xx, yy, ll, ii, jj)	\
if (H_LINE_X((ii), (jj)) >= 0) xset_hchars(t->p, (xx), (yy), (ll), hline_table[H_LINE((ii), (jj))] | ATTR_FRAME)

#define draw_frame_vline(xx, yy, ll, ii, jj)	\
{						\
	int qq;					\
	if (V_LINE_X((ii), (jj)) >= 0) for (qq = 0; qq < (ll); qq++) xset_hchar(t->p, (xx), (yy) + qq, vline_table[V_LINE((ii), (jj))] | ATTR_FRAME); }

void display_table_frames(struct table *t, int x, int y)
{
	signed char *fh, *fv;
	int i, j;
	int cx, cy;
	if ((unsigned)t->x > MAXINT) overalloc();
	if ((unsigned)t->y > MAXINT) overalloc();
	if (((unsigned)t->x + 2) * ((unsigned)t->y + 2) / ((unsigned)t->x + 2) != ((unsigned)t->y + 2)) overalloc();
	if (((unsigned)t->x + 2) * ((unsigned)t->y + 2) > MAXINT) overalloc();
	fh = mem_alloc((t->x + 2) * (t->y + 1));
	memset(fh, -1, (t->x + 2) * (t->y + 1));
	fv = mem_alloc((t->x + 1) * (t->y + 2));
	memset(fv, -1, (t->x + 1) * (t->y + 2));
#ifndef DEBUG
#define H_LINE_X(xx, yy) fh[(xx) + 1 + (t->x + 2) * (yy)]
#define V_LINE_X(xx, yy) fv[(yy) + 1 + (t->y + 2) * (xx)]
#else
#define H_LINE_X(xx, yy) (*(xx < -1 || xx > t->x + 1 || yy < 0 || yy > t->y ? (signed char *)NULL : &fh[(xx) + 1 + (t->x + 2) * (yy)]))
#define V_LINE_X(xx, yy) (*(xx < 0 || xx > t->x || yy < -1 || yy > t->y + 1 ? (signed char *)NULL : &fv[(yy) + 1 + (t->y + 2) * (xx)]))
#endif
#define H_LINE(xx, yy) (H_LINE_X((xx), (yy)) == -1 ? 0 : H_LINE_X((xx), (yy)))
#define V_LINE(xx, yy) (V_LINE_X((xx), (yy)) == -1 ? 0 : V_LINE_X((xx), (yy)))
	for (j = 0; j < t->y; j++) for (i = 0; i < t->x; i++) {
		int x, y;
		int xsp, ysp;
		struct table_cell *cell = CELL(t, i, j);
		if (!cell->used || cell->spanned) continue;
		if ((xsp = cell->colspan) == 0) xsp = t->x - i;
		if ((ysp = cell->rowspan) == 0) ysp = t->y - j;
		if (t->rules != R_NONE && t->rules != R_COLS) for (x = 0; x < xsp; x++) {H_LINE_X(i + x, j) = t->cellsp; H_LINE_X(i + x, j + ysp) = t->cellsp;}
		if (t->rules != R_NONE && t->rules != R_ROWS) for (y = 0; y < ysp; y++) {V_LINE_X(i, j + y) = t->cellsp; V_LINE_X(i + xsp, j + y) = t->cellsp;}
	}
	if (t->rules == R_GROUPS) {
		for (i = 1; i < t->x; i++) {
			if (/*i < t->xc &&*/ t->xcols[i]) continue;
			for (j = 0; j < t->y; j++) V_LINE_X(i, j) = 0;
		}
		for (j = 1; j < t->y; j++) {
			for (i = 0; i < t->x; i++) if (CELL(t, i, j)->group) goto c;
			for (i = 0; i < t->x; i++) H_LINE_X(i, j) = 0;
			c:;
		}
	}
	for (i = 0; i < t->x; i++) {
		H_LINE_X(i, 0) = t->border * !!(t->frame & F_ABOVE);
		H_LINE_X(i, t->y) = t->border * !!(t->frame & F_BELOW);
	}
	for (j = 0; j < t->y; j++) {
		V_LINE_X(0, j) = t->border * !!(t->frame & F_LHS);
		V_LINE_X(t->x, j) = t->border * !!(t->frame & F_RHS);
	}

	cy = y;
	for (j = 0; j <= t->y; j++) {
		cx = x;
		if ((j > 0 && j < t->y && get_hline_width(t, j) >= 0) || (j == 0 && t->border && (t->frame & F_ABOVE)) || (j == t->y && t->border && (t->frame & F_BELOW))) {
			for (i = 0; i < t->x; i++) {
				int w;
				if (i > 0) w = get_vline_width(t, i);
				else w = t->border && (t->frame & F_LHS) ? t->border : -1;
				if (w >= 0) {
					draw_frame_point(cx, cy, i, j);
					if (j < t->y) draw_frame_vline(cx, cy + 1, t->r_heights[j], i, j);
					cx++;
				}
				w = t->w_c[i];
				draw_frame_hline(cx, cy, w, i, j);
				cx += w;
			}
			if (t->border && (t->frame & F_RHS)) {
				draw_frame_point(cx, cy, i, j);
				if (j < t->y) draw_frame_vline(cx, cy + 1, t->r_heights[j], i, j);
				cx++;
			}
			cy++;
		} else if (j < t->y) {
			for (i = 0; i <= t->x; i++) {
				if ((i > 0 && i < t->x && get_vline_width(t, i) >= 0) || (i == 0 && t->border && (t->frame & F_LHS)) || (i == t->x && t->border && (t->frame & F_RHS))) {
					draw_frame_vline(cx, cy, t->r_heights[j], i, j);
					cx++;
				}
				if (i < t->x) cx += t->w_c[i];
			}
		}
		if (j < t->y) cy += t->r_heights[j];
		/*for (cyy = cy1; cyy < cy; cyy++) xxpand_line(t->p, cyy, cx - 1);*/
	}
	mem_free(fh);
	mem_free(fv);
}

void format_table(unsigned char *attr, unsigned char *html, unsigned char *eof, unsigned char **end, void *f)
{
	struct part *p = f;
	int border, cellsp, vcellpd, cellpd, align;
	int frame, rules, width, wf;
	struct rgb bgcolor;
	struct table *t;
	char *al;
	int cye;
	int x;
	int i;
	/*int llm = last_link_to_move;*/
	struct s_e *bad_html;
	int bad_html_n;
	struct node *n, *nn;
	int cpd_pass, cpd_width, cpd_last;
	/*if (!p->data) {
		debug("nested tables not supported");
		return;
	}*/
	table_level++;
	memcpy(&bgcolor, &par_format.bgcolor, sizeof(struct rgb));
	get_bgcolor(attr, &bgcolor);
	if ((border = get_num(attr, "border")) == -1) border = has_attr(attr, "border") || has_attr(attr, "rules") || has_attr(attr, "frame");
	/*if (!border) border = 1;*/

	if ((cellsp = get_num(attr, "cellspacing")) == -1) cellsp = 1;
	if ((cellpd = get_num(attr, "cellpadding")) == -1) {
		vcellpd = 0;
		cellpd = !!border;
	} else {
		vcellpd = cellpd >= HTML_CHAR_HEIGHT / 2 + 1;
		cellpd = cellpd >= HTML_CHAR_WIDTH / 2 + 1;
	}
	if (!border) cellsp = 0;
	else if (!cellsp) cellsp = 1;
	if (border > 2) border = 2;
	if (cellsp > 2) cellsp = 2;
	align = par_format.align;
	if (align == AL_NO || align == AL_BLOCK) align = AL_LEFT;
	if ((al = get_attr_val(attr, "align"))) {
		if (!strcasecmp(al, "left")) align = AL_LEFT;
		if (!strcasecmp(al, "center")) align = AL_CENTER;
		if (!strcasecmp(al, "right")) align = AL_RIGHT;
		mem_free(al);
	}
	frame = F_BOX;
	if ((al = get_attr_val(attr, "frame"))) {
		if (!strcasecmp(al, "void")) frame = F_VOID;
		if (!strcasecmp(al, "above")) frame = F_ABOVE;
		if (!strcasecmp(al, "below")) frame = F_BELOW;
		if (!strcasecmp(al, "hsides")) frame = F_HSIDES;
		if (!strcasecmp(al, "vsides")) frame = F_VSIDES;
		if (!strcasecmp(al, "lhs")) frame = F_LHS;
		if (!strcasecmp(al, "rhs")) frame = F_RHS;
		if (!strcasecmp(al, "box")) frame = F_BOX;
		if (!strcasecmp(al, "border")) frame = F_BOX;
		mem_free(al);
	}
	rules = border ? R_ALL : R_NONE;
	if ((al = get_attr_val(attr, "rules"))) {
		if (!strcasecmp(al, "none")) rules = R_NONE;
		if (!strcasecmp(al, "groups")) rules = R_GROUPS;
		if (!strcasecmp(al, "rows")) rules = R_ROWS;
		if (!strcasecmp(al, "cols")) rules = R_COLS;
		if (!strcasecmp(al, "all")) rules = R_ALL;
		mem_free(al);
	}
	if (!border) frame = F_VOID;
	wf = 0;
	if ((width = get_width(attr, "width", p->data || p->xp)) == -1) {
		width = par_format.width - par_format.leftmargin - par_format.rightmargin;
		if (width < 0) width = 0;
		wf = 1;
	}
	if (!(t = parse_table(html, eof, end, &bgcolor, p->data || p->xp, &bad_html, &bad_html_n))) {
		mem_free(bad_html);
		goto ret0;
	}
	for (i = 0; i < bad_html_n; i++) {
		while (bad_html[i].s < bad_html[i].e && WHITECHAR(*bad_html[i].s)) bad_html[i].s++;
		while (bad_html[i].s < bad_html[i].e && WHITECHAR(bad_html[i].e[-1])) bad_html[i].e--;
		if (bad_html[i].s < bad_html[i].e) parse_html(bad_html[i].s, bad_html[i].e, put_chars_f, line_break_f, special_f, p, NULL);
	}
	mem_free(bad_html);
	html_stack_dup();
	html_top.dontkill = 1;
	par_format.align = AL_LEFT;
	t->p = p;
	t->border = border;
	t->cellpd = cellpd;
	t->vcellpd = vcellpd;
	t->cellsp = cellsp;
	t->frame = frame;
	t->rules = rules;
	t->width = width;
	t->wf = wf;
	cpd_pass = 0;
	cpd_last = t->cellpd;
	cpd_width = 0;	/* not needed, but let the warning go away */
	again:
	get_cell_widths(t);
	if (get_column_widths(t)) goto ret2;
	get_table_width(t);
	if (!p->data && !p->xp) {
		if (!wf && t->max_t > width) t->max_t = width;
		if (t->max_t < t->min_t) t->max_t = t->min_t;
		if (t->max_t + par_format.leftmargin + par_format.rightmargin > p->xmax) p->xmax = t->max_t + par_format.leftmargin + par_format.rightmargin;
		if (t->min_t + par_format.leftmargin + par_format.rightmargin > p->x) p->x = t->min_t + par_format.leftmargin + par_format.rightmargin;
		goto ret2;
	}
	if (!cpd_pass && t->min_t > width && t->cellpd) {
		t->cellpd = 0;
		cpd_pass = 1;
		cpd_width = t->min_t;
		goto again;
	}
	if (cpd_pass == 1 && t->min_t > cpd_width) {
		t->cellpd = cpd_last;
		cpd_pass = 2;
		goto again;
	}
	/*debug("%d %d %d", t->min_t, t->max_t, width);*/
	if (t->min_t >= width) distribute_widths(t, t->min_t);
	else if (t->max_t < width && wf) distribute_widths(t, t->max_t);
	else distribute_widths(t, width);
	if (!p->data && p->xp == 1) {
		int ww = t->rw + par_format.leftmargin + par_format.rightmargin;
		if (ww > par_format.width) ww = par_format.width;
		if (ww < t->rw) ww = t->rw;
		if (ww > p->x) p->x = ww;
		p->cy += t->rh;
		goto ret2;
	}
#ifdef HTML_TABLE_2ND_PASS
	check_table_widths(t);
#endif
	x = par_format.leftmargin;
	if (align == AL_CENTER) x = (par_format.width + par_format.leftmargin - par_format.rightmargin - t->rw) / 2;
	if (align == AL_RIGHT) x = par_format.width - par_format.rightmargin - t->rw;
	if (x + t->rw > par_format.width) x = par_format.width - t->rw;
	if (x < 0) x = 0;
	/*display_table(t, x, p->cy, &cye);*/
	get_table_heights(t);
	if (!p->data) {
		if (t->rw + par_format.leftmargin + par_format.rightmargin > p->x) p->x = t->rw + par_format.leftmargin + par_format.rightmargin;
		p->cy += t->rh;
		goto ret2;
	}
	n = p->data->nodes.next;
	n->yw = p->yp - n->y + p->cy;
	display_complicated_table(t, x, p->cy, &cye);
	display_table_frames(t, x, p->cy);
	nn = mem_alloc(sizeof(struct node));
	nn->x = n->x;
	nn->y = p->yp + cye;
	nn->xw = n->xw;
	add_to_list(p->data->nodes, nn);
	/*sdbg(p->data);*/
	/*for (y = p->cy; y < cye; y++) {
		last_link_to_move = llm;
		align_line(p, y);
	}*/
	/*if (p->cy + t->rh != cye) internal("size does not match; 1:%d, 2:%d", p->cy + t->rh, cye);*/
	p->cy = cye;
	p->cx = -1;

	ret2:
	p->link_num = t->link_num;
	if (p->cy > p->y) p->y = p->cy;
	/*ret1:*/
	free_table(t);
	kill_html_stack_item(&html_top);
	ret0:
	/*ret:*/
	table_level--;
	if (!table_level) free_table_cache();
}
