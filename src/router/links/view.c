#include "links.h"

void init_vs(struct view_state *vs, unsigned char *url)
{
	memset(vs, 0, sizeof(struct view_state));
	vs->current_link = -1;
	vs->orig_link = -1;
	vs->plain = -1;
	vs->form_info = DUMMY;
	vs->form_info_len = 0;
	strcpy(vs->url, url);
}

void destroy_vs(struct view_state *vs)
{
	int i;
	if (vs->goto_position) mem_free(vs->goto_position);
	if (vs->goto_position_end) mem_free(vs->goto_position_end);
	for (i = 0; i < vs->form_info_len; i++) if (vs->form_info[i].value) mem_free(vs->form_info[i].value);
	mem_free(vs->form_info);
}

void init_formatted(struct f_data *scr)
{
	memset(((struct f_data **)scr) + 2, 0, SIZEOF_F_DATA - 2 * sizeof(struct f_data *));
	scr->data = DUMMY;
	scr->nlinks = 0;
	scr->links = DUMMY;
	init_list(scr->forms);
	init_list(scr->tags);
	init_list(scr->nodes);
}

void destroy_fc(struct form_control *fc)
{
	int i;
	if (fc->action) mem_free(fc->action);
	if (fc->target) mem_free(fc->target);
	if (fc->name) mem_free(fc->name);
	if (fc->alt) mem_free(fc->alt);
	if (fc->default_value) mem_free(fc->default_value);
	for (i = 0; i < fc->nvalues; i++) {
		if (fc->values[i]) mem_free(fc->values[i]);
		if (fc->labels[i]) mem_free(fc->labels[i]);
	}
	if (fc->values) mem_free(fc->values);
	if (fc->labels) mem_free(fc->labels);
	if (fc->menu) free_menu(fc->menu);
}

void free_frameset_desc(struct frameset_desc *fd)
{
	int i;
	for (i = 0; i < fd->n; i++) {
		if (fd->f[i].subframe) free_frameset_desc(fd->f[i].subframe);
		if (fd->f[i].name) mem_free(fd->f[i].name);
		if (fd->f[i].url) mem_free(fd->f[i].url);
	}
	mem_free(fd);
}

void clear_formatted(struct f_data *scr)
{
	int n;
	int y;
	struct cache_entry *ce;
	struct form_control *fc;
	if (!scr) return;
	if (find_in_cache(scr->url, &ce)) internal("no cache entry for document");
	ce->refcount -= 2;
	if (scr->url) mem_free(scr->url);
	if (scr->title) mem_free(scr->title);
	if (scr->frame_desc) {
		free_frameset_desc(scr->frame_desc);
	}
	for (n = 0; n < scr->nlinks; n++) {
		struct link *l = &scr->links[n];
		if (l->where) mem_free(l->where);
		if (l->target) mem_free(l->target);
		if (l->where_img) mem_free(l->where_img);
		if (l->pos) mem_free(l->pos);
	}
	mem_free(scr->links);
	for (y = 0; y < scr->y; y++) mem_free(scr->data[y].d);
	mem_free(scr->data);
	if (scr->lines1) mem_free(scr->lines1);
	if (scr->lines2) mem_free(scr->lines2);
	mem_free(scr->opt.framename);
	foreach(fc, scr->forms) {
		destroy_fc(fc);
	}
	free_list(scr->forms);
	free_list(scr->tags);
	free_list(scr->nodes);
	if (scr->search) mem_free(scr->search);
	if (scr->slines1) mem_free(scr->slines1);
	if (scr->slines2) mem_free(scr->slines2);
	init_formatted(scr);
}

void destroy_formatted(struct f_data *scr)
{
	if (scr->refcount) {
		internal("trying to free locked formatted data");
		return;
	}
	clear_formatted(scr);
	del_from_list(scr);
	mem_free(scr);
}

void detach_formatted(struct f_data_c *scr)
{
	if (scr->f_data) {
		format_cache_reactivate(scr->f_data);
		if (!--scr->f_data->refcount) {
			format_cache_entries++;
			/*shrink_format_cache();*/
		}
		if (scr->f_data->refcount < 0) {
			internal("format_cache refcount underflow");
			scr->f_data->refcount = 0;
		}
		scr->f_data = NULL;
	}
	scr->vs = NULL;
	if (scr->link_bg) mem_free(scr->link_bg), scr->link_bg = NULL, scr->link_bg_n = 0;
	if (scr->name) mem_free(scr->name), scr->name = NULL;
}

void copy_vs(struct view_state *dst, struct view_state *src)
{
	int i;
	memcpy(dst, src, sizeof(struct view_state));
	strcpy(dst->url, src->url);
	dst->goto_position = stracpy(src->goto_position);
	dst->goto_position_end = stracpy(src->goto_position_end);
	if ((unsigned)src->form_info_len > MAXINT / sizeof(struct form_state)) overalloc();
	dst->form_info = mem_alloc(src->form_info_len * sizeof(struct form_state));
	memcpy(dst->form_info, src->form_info, src->form_info_len * sizeof(struct form_state));
	for (i = 0; i < src->form_info_len; i++) if (src->form_info[i].value) dst->form_info[i].value = stracpy(src->form_info[i].value);
}

void copy_location(struct location *dst, struct location *src)
{
	struct frame *f, *nf;
	init_list(dst->frames);
	foreachback(f, src->frames) {
		nf = mem_alloc(sizeof(struct frame) + strlen(f->vs.url) + 1);
		nf->name = stracpy(f->name);
		nf->redirect_cnt = 0;
		copy_vs(&nf->vs, &f->vs);
		add_to_list(dst->frames, nf);
	}
	copy_vs(&dst->vs, &src->vs);
}

static inline int c_in_view(struct f_data_c *);
void set_pos_x(struct f_data_c *, struct link *);
void set_pos_y(struct f_data_c *, struct link *);
void find_link(struct f_data_c *, int, int);
void next_frame(struct session *, int);

void check_vs(struct f_data_c *f)
{
	struct view_state *vs = f->vs;
	int ovx = vs->orig_view_posx, ovy = vs->orig_view_pos, ol = vs->orig_link;
	if (vs->current_link >= f->f_data->nlinks) vs->current_link = f->f_data->nlinks - 1;
	if (vs->current_link != -1 && !c_in_view(f)) {
		set_pos_x(f, &f->f_data->links[f->vs->current_link]);
		set_pos_y(f, &f->f_data->links[f->vs->current_link]);
	}
	if (vs->current_link == -1) find_link(f, 1, 0);
	vs->orig_view_posx = ovx, vs->orig_view_pos = ovy, vs->orig_link = ol;
}

void set_link(struct f_data_c *f)
{
	if (c_in_view(f)) return;
	find_link(f, 1, 0);
}

int find_tag(struct f_data *f, unsigned char *name)
{
	struct tag *tag;
	unsigned char *tt;
	int ll;
	tt = init_str();
	ll = 0;
	add_conv_str(&tt, &ll, name, strlen(name), -2);
	foreachback(tag, f->tags) if (!strcasecmp(tag->name, tt) || (tag->name[0] == '#' && !strcasecmp(tag->name + 1, tt))) {
		mem_free(tt);
		return tag->y;
	}
	mem_free(tt);
	return -1;
}

LIBC_CALLBACK int comp_links(struct link *l1, struct link *l2)
{
	return l1->num - l2->num;
}

void sort_links(struct f_data *f)
{
	int i;
	if (f->nlinks) qsort(f->links, f->nlinks, sizeof(struct link), (void *)comp_links);
	if ((unsigned)f->y > MAXINT / sizeof(struct link *)) overalloc();
	f->lines1 = mem_alloc(f->y * sizeof(struct link *));
	f->lines2 = mem_alloc(f->y * sizeof(struct link *));
	memset(f->lines1, 0, f->y * sizeof(struct link *));
	memset(f->lines2, 0, f->y * sizeof(struct link *));
	for (i = 0; i < f->nlinks; i++) {
		int p, q, j;
		struct link *link = &f->links[i];
		if (!link->n) {
			if (d_opt->num_links) continue;
			if (link->where) mem_free(link->where);
			if (link->target) mem_free(link->target);
			if (link->where_img) mem_free(link->where_img);
			if (link->pos) mem_free(link->pos);
			memmove(link, link + 1, (f->nlinks - i - 1) * sizeof(struct link));
			f->nlinks --;
			i--;
			continue;
		}
		p = f->y - 1;
		q = 0;
		for (j = 0; j < link->n; j++) {
			if (link->pos[j].y < p) p = link->pos[j].y;
			if (link->pos[j].y > q) q = link->pos[j].y;
		}
		/*
		p = link->pos[0].y;
		q = link->pos[link->n - 1].y;
		*/
		if (p > q) j = p, p = q, q = j;
		for (j = p; j <= q; j++) {
			if (j >= f->y) {
				internal("link out of screen");
				continue;
			}
			f->lines2[j] = &f->links[i];
			if (!f->lines1[j]) f->lines1[j] = &f->links[i];
		}
	}
}

struct form_state *find_form_state(struct f_data_c *, struct form_control *);

struct line_info {
	unsigned char *st;
	unsigned char *en;
};

struct line_info *format_text(unsigned char *text, int width, int wrap)
{
	struct line_info *ln = DUMMY;
	int lnn = 0;
	unsigned char *b = text;
	int sk, ps = 0;
	while (*text) {
		unsigned char *s;
		if (*text == '\n') {
			sk = 1;
			put:
			if (!(lnn & (ALLOC_GR-1))) {
				if ((unsigned)lnn > MAXINT / sizeof(struct line_info) - ALLOC_GR) overalloc();
				ln = mem_realloc(ln, (lnn + ALLOC_GR) * sizeof(struct line_info));
			}
			ln[lnn].st = b;
			ln[lnn++].en = text;
			b = text += sk;
			continue;
		}
		if (!wrap || text - b < width) {
			text++;
			continue;
		}
		for (s = text; s >= b; s--) if (*s == ' ') {
			text = s;
			if (wrap == 2) {
				*s = '\n';
				for (s++; *s; s++) if (*s == '\n') {
					if (s[1] != '\n') *s = ' ';
					break;
				}
			}
			sk = 1;
			goto put;
		}
		sk = 0;
		goto put;
	}
	if (ps < 2) {
		ps++;
		sk = 0;
		goto put;
	}
	ln[lnn - 1].st = ln[lnn - 1].en = NULL;
	return ln;
}

int _area_cursor(struct form_control *form, struct form_state *fs)
{
	struct line_info *ln;
	int q = 0;
	if ((ln = format_text(fs->value, form->cols, form->wrap))) {
		int x, y;
		for (y = 0; ln[y].st; y++) if (fs->value + fs->state >= ln[y].st && fs->value + fs->state < ln[y].en + (ln[y+1].st != ln[y].en)) {
			x = fs->value + fs->state - ln[y].st;
			if (form->wrap && x == form->cols) x--;
			if (x >= form->cols + fs->vpos) fs->vpos = x - form->cols + 1;
			if (x < fs->vpos) fs->vpos = x;
			if (y >= form->rows + fs->vypos) fs->vypos = y - form->rows + 1;
			if (y < fs->vypos) fs->vypos = y;
			x -= fs->vpos;
			y -= fs->vypos;
			q = y * form->cols + x;
			break;
		}
		mem_free(ln);
	}
	return q;
}

void draw_link(struct terminal *t, struct f_data_c *scr, int l)
{
	struct link *link = &scr->f_data->links[l];
	int xp = scr->xp;
	int yp = scr->yp;
	int xw = scr->xw;
	int yw = scr->yw;
	int vx, vy;
	struct view_state *vs = scr->vs;
	int f = 0;
	vx = vs->view_posx;
	vy = vs->view_pos;
	if (scr->link_bg) {
		internal("link background not empty");
		mem_free(scr->link_bg);
	}
	if (l == -1) return;
	switch (link->type) {
		int i;
		int q;
		case L_LINK:
		case L_CHECKBOX:
		case L_BUTTON:
		case L_SELECT:
		case L_FIELD:
		case L_AREA:
			q = 0;
			if (link->type == L_FIELD) {
				struct form_state *fs = find_form_state(scr, link->form);
				if (fs) q = fs->state - fs->vpos;
				/*else internal("link has no form control");*/
			} else if (link->type == L_AREA) {
				struct form_state *fs = find_form_state(scr, link->form);
				if (fs) q = _area_cursor(link->form, fs);
				/*else internal("link has no form control");*/
			}
			if ((unsigned)link->n > MAXINT / sizeof(struct link_bg)) overalloc();
			scr->link_bg = mem_alloc(link->n * sizeof(struct link_bg));
			scr->link_bg_n = link->n;
			for (i = 0; i < link->n; i++) {
				int x = link->pos[i].x + xp - vx;
				int y = link->pos[i].y + yp - vy;
				if (x >= xp && y >= yp && x < xp+xw && y < yp+yw) {
					unsigned co;
					co = get_char(t, x, y);
					if (scr->link_bg) scr->link_bg[i].x = x,
							  scr->link_bg[i].y = y,
							  scr->link_bg[i].c = co;
					if (!f || (link->type == L_CHECKBOX && i == 1) || (link->type == L_BUTTON && i == 2) || ((link->type == L_FIELD || link->type == L_AREA) && i == q)) {
						int xx = x, yy = y;
						if (link->type != L_FIELD && link->type != L_AREA) {
							if (((co >> 8) & 0x38) != (link->sel_color & 0x38)) xx = xp + xw - 1, yy = yp + yw - 1;
						}
						set_cursor(t, x, y, xx, yy);
						set_window_ptr(get_root_window(t), x, y);
						f = 1;
					}
					set_color(t, x, y, /*((link->sel_color << 3) | (co >> 11 & 7)) << 8*/ link->sel_color << 8);
				} else scr->link_bg[i].x = scr->link_bg[i].y = scr->link_bg[i].c = -1;
			}
			break;
		default: internal("bad link type");
	}
}

void free_link(struct f_data_c *scr)
{
	if (scr->link_bg) {
		mem_free(scr->link_bg);
		scr->link_bg = NULL;
	}
	scr->link_bg_n = 0;
}

void clear_link(struct terminal *t, struct f_data_c *scr)
{
	if (scr->link_bg) {
		int i;
		for (i = scr->link_bg_n - 1; i >= 0; i--)
			set_char(t, scr->link_bg[i].x, scr->link_bg[i].y, scr->link_bg[i].c);
		free_link(scr);
	}
}

int get_range(struct f_data *f, int y, int yw, int l, struct search **s1, struct search **s2)
{
	int i;
	*s1 = *s2 = NULL;
	for (i = y < 0 ? 0 : y; i < y + yw && i < f->y; i++) {
		if (f->slines1[i] && (!*s1 || f->slines1[i] < *s1)) *s1 = f->slines1[i];
		if (f->slines2[i] && (!*s2 || f->slines2[i] > *s2)) *s2 = f->slines2[i];
	}

	if (l > f->nsearch) *s1 = *s2 = NULL;
	if (!*s1 || !*s2) return -1;

	if (*s1 - f->search < l) *s1 = f->search;
	else *s1 -= l;

	if (f->search + f->nsearch - *s2 < l) *s2 = f->search + f->nsearch - l;

	if (*s1 > *s2) *s1 = *s2 = NULL;
	if (!*s1 || !*s2) return -1;

	return 0;
}

int is_in_range(struct f_data *f, int y, int yw, unsigned char *txt, int *min, int *max)
{
	int found = 0;
	int l = strlen(txt);
	struct search *s1, *s2;
	if (min || max) *min = MAXINT, *max = 0;
	if (get_range(f, y, yw, l, &s1, &s2)) return 0;
	for (; s1 <= s2; s1++) {
		int i;
		if (s1->c != txt[0]) {
			unable_to_handle_kernel_paging_request___oops:
			continue;
		}
		for (i = 1; i < l; i++) if (s1[i].c != txt[i]) goto unable_to_handle_kernel_paging_request___oops;
		for (i = 0; i < l; i++) if (s1[i].y >= y && s1[i].y < y + yw && s1[i].n) goto in_view;
		continue;
		in_view:
		if (!min && !max) return 1;
		found = 1;
		for (i = 0; i < l; i++) if (s1[i].n) {
			if (s1[i].x < *min) *min = s1[i].x;
			if (s1[i].x + s1[i].n > *max) *max = s1[i].x + s1[i].n;
		}
	}
	return found;
}

void get_searched(struct f_data_c *scr, struct point **pt, int *pl)
{
	int xp = scr->xp;
	int yp = scr->yp;
	int xw = scr->xw;
	int yw = scr->yw;
	int vx = scr->vs->view_posx;
	int vy = scr->vs->view_pos;
	struct search *s1, *s2;
	int l;
	unsigned char c;
	struct point *points = DUMMY;
	int len = 0;
	if (!scr->search_word || !*scr->search_word || !(*scr->search_word)[0]) return;
	get_search_data(scr->f_data);
	l = strlen(*scr->search_word);
	c = (*scr->search_word)[0];
	if (get_range(scr->f_data, scr->vs->view_pos, scr->yw, l, &s1, &s2)) goto ret;
	for (; s1 <= s2; s1++) {
		int i, j;
		if (s1->c != c) {
			c:continue;
		}
		for (i = 1; i < l; i++) if (s1[i].c != (*scr->search_word)[i]) goto c;
		for (i = 0; i < l; i++) for (j = 0; j < s1[i].n; j++) {
			int x = s1[i].x + j + xp - vx;
			int y = s1[i].y + yp - vy;
			if (x >= xp && y >= yp && x < xp + xw && y < yp + yw) {
				/*unsigned co;
				co = get_char(t, x, y);
				co = ((co >> 3) & 0x0700) | ((co << 3) & 0x3800);
				set_color(t, x, y, co);*/
				if (!(len & (ALLOC_GR - 1))) {
					if ((unsigned)len > MAXINT / sizeof(struct point) - ALLOC_GR) overalloc();
					points = mem_realloc(points, sizeof(struct point) * (len + ALLOC_GR));
				}
				points[len].x = s1[i].x + j;
				points[len++].y = s1[i].y;
			}
		}
	}
	ret:
	*pt = points;
	*pl = len;
}

void draw_searched(struct terminal *t, struct f_data_c *scr)
{
	int xp = scr->xp;
	int yp = scr->yp;
	int vx = scr->vs->view_posx;
	int vy = scr->vs->view_pos;
	struct point *pt;
	int len, i;
	if (!scr->search_word || !*scr->search_word || !(*scr->search_word)[0]) return;
	get_searched(scr, &pt, &len);
	for (i = 0; i < len; i++) {
		int x = pt[i].x + xp - vx, y = pt[i].y + yp - vy;
		unsigned co;
		co = get_char(t, x, y);
		co = ((co >> 3) & 0x0700) | ((co << 3) & 0x3800);
		set_color(t, x, y, co);
	}
	mem_free(pt);
}

void draw_current_link(struct terminal *t, struct f_data_c *scr)
{
	draw_link(t, scr, scr->vs->current_link);
	draw_searched(t, scr);
}

struct link *get_first_link(struct f_data_c *f)
{
	int i;
	struct link *l = f->f_data->links + f->f_data->nlinks;
	for (i = f->vs->view_pos; i < f->vs->view_pos + f->yw; i++)
		if (i >= 0 && i < f->f_data->y && f->f_data->lines1[i] && f->f_data->lines1[i] < l)
			l = f->f_data->lines1[i];
	if (l == f->f_data->links + f->f_data->nlinks) l = NULL;
	return l;
}

struct link *get_last_link(struct f_data_c *f)
{
	int i;
	struct link *l = NULL;
	for (i = f->vs->view_pos; i < f->vs->view_pos + f->yw; i++)
		if (i >= 0 && i < f->f_data->y && f->f_data->lines2[i] > l)
			l = f->f_data->lines2[i];
	return l;
}

void init_ctrl(struct form_control *form, struct form_state *fs);

void fixup_select_state(struct form_control *fc, struct form_state *fs)
{
	int inited = 0;
	int i;
	retry:
	if (fs->state >= 0 && fs->state < fc->nvalues && !strcmp(fc->values[fs->state], fs->value)) return;
	for (i = 0; i < fc->nvalues; i++) {
		if (!strcmp(fc->values[i], fs->value)) {
			fs->state = i;
			return;
		}
	}
	if (!inited) {
		init_ctrl(fc, fs);
		inited = 1;
		goto retry;
	}
	fs->state = 0;
	if (fs->value) mem_free(fs->value);
	if (fc->nvalues) fs->value = stracpy(fc->values[0]);
	else fs->value = stracpy("");
}

void init_ctrl(struct form_control *form, struct form_state *fs)
{
	if (fs->value) mem_free(fs->value), fs->value = NULL;
	switch (form->type) {
		case FC_TEXT:
		case FC_PASSWORD:
		case FC_TEXTAREA:
			fs->value = stracpy(form->default_value);
			fs->state = strlen(form->default_value);
			fs->vpos = 0;
			break;
		case FC_FILE:
			fs->value = stracpy("");
			fs->state = 0;
			fs->vpos = 0;
			break;
		case FC_CHECKBOX:
		case FC_RADIO:
			fs->state = form->default_state;
			break;
		case FC_SELECT:
			fs->value = stracpy(form->default_value);
			fs->state = form->default_state;
			fixup_select_state(form, fs);
			break;
	}
}

struct form_state *find_form_state(struct f_data_c *f, struct form_control *form)
{
	struct view_state *vs = f->vs;
	struct form_state *fs;
	int n = form->g_ctrl_num;
	if (n < vs->form_info_len) fs = &vs->form_info[n];
	else {
		if ((unsigned)n > MAXINT / sizeof(struct form_state) - 1) overalloc();
		fs = mem_realloc(vs->form_info, (n + 1) * sizeof(struct form_state));
		vs->form_info = fs;
		memset(fs + vs->form_info_len, 0, (n + 1 - vs->form_info_len) * sizeof(struct form_state));
		vs->form_info_len = n + 1;
		fs = &vs->form_info[n];
	}
	if (fs->form_num == form->form_num && fs->ctrl_num == form->ctrl_num && fs->g_ctrl_num == form->g_ctrl_num && fs->position == form->position && fs->type == form->type) return fs;
	if (fs->value) mem_free(fs->value);
	memset(fs, 0, sizeof(struct form_state));
	fs->form_num = form->form_num;
	fs->ctrl_num = form->ctrl_num;
	fs->g_ctrl_num = form->g_ctrl_num;
	fs->position = form->position;
	fs->type = form->type;
	init_ctrl(form, fs);
	return fs;
}

void draw_form_entry(struct terminal *t, struct f_data_c *f, struct link *l)
{
	int xp = f->xp;
	int yp = f->yp;
	int xw = f->xw;
	int yw = f->yw;
	struct view_state *vs = f->vs;
	int vx = vs->view_posx;
	int vy = vs->view_pos;
	struct form_state *fs;
	struct form_control *form = l->form;
	int i, x, y;
	if (!form) {
		internal("link %d has no form", (int)(l - f->f_data->links));
		return;
	}
	if (!(fs = find_form_state(f, form))) return;
	switch (form->type) {
		unsigned char *s;
		struct line_info *ln, *lnx;
		int sl;
		case FC_TEXT:
		case FC_PASSWORD:
		case FC_FILE:
			if (fs->state >= fs->vpos + form->size) fs->vpos = fs->state - form->size + 1;
			if (fs->state < fs->vpos) fs->vpos = fs->state;
			if (!l->n) break;
			x = l->pos[0].x + xp - vx; y = l->pos[0].y + yp - vy;
			for (i = 0; i < form->size; i++, x++)
				if (x >= xp && y >= yp && x < xp+xw && y < yp+yw) {
					if (fs->value && i >= -fs->vpos && i < (int)strlen(fs->value) - fs->vpos) set_only_char(t, x, y, form->type != FC_PASSWORD ? fs->value[i + fs->vpos] : '*');
					else set_only_char(t, x, y, '_');
				}
			break;
		case FC_TEXTAREA:
			if (!l->n) break;
			x = l->pos[0].x + xp - vx; y = l->pos[0].y + yp - vy;
			_area_cursor(form, fs);
			if (!(lnx = format_text(fs->value, form->cols, form->wrap))) break;
			ln = lnx;
			sl = fs->vypos;
			while (ln->st && sl) sl--, ln++;
			for (; ln->st && y < l->pos[0].y + yp - vy + form->rows; ln++, y++) {
				for (i = 0; i < form->cols; i++) {
					if (x+i >= xp && y >= yp && x+i < xp+xw && y < yp+yw) {
						if (fs->value && i >= -fs->vpos && i + fs->vpos < ln->en - ln->st) set_only_char(t, x+i, y, ln->st[i + fs->vpos]);
						else set_only_char(t, x+i, y, '_');
					}
				}
			}
			for (; y < l->pos[0].y + yp - vy + form->rows; y++) {
				for (i = 0; i < form->cols; i++) {
					if (x+i >= xp && y >= yp && x+i < xp+xw && y < yp+yw)
						set_only_char(t, x+i, y, '_');
				}
			}
			
			mem_free(lnx);
			break;
		case FC_CHECKBOX:
		case FC_RADIO:
			if (l->n < 2) break;
			x = l->pos[1].x + xp - vx;
			y = l->pos[1].y + yp - vy;
			if (x >= xp && y >= yp && x < xp+xw && y < yp+yw)
				set_only_char(t, x, y, fs->state ? 'X' : ' ');
			break;
		case FC_SELECT:
			fixup_select_state(form, fs);
			s = fs->state < form->nvalues ? form->labels[fs->state] : (unsigned char *)"";
			sl = s ? strlen(s) : 0;
			for (i = 0; i < l->n; i++) {
				x = l->pos[i].x + xp - vx;
				y = l->pos[i].y + yp - vy;
				if (x >= xp && y >= yp && x < xp+xw && y < yp+yw)
					set_only_char(t, x, y, i < sl ? s[i] : '_');
			}
			break;
		case FC_SUBMIT:
		case FC_IMAGE:
		case FC_RESET:
		case FC_HIDDEN:
			break;
	}
}

void draw_forms(struct terminal *t, struct f_data_c *f)
{
	struct link *l1 = get_first_link(f);
	struct link *l2 = get_last_link(f);
	if (!l1 || !l2) {
		if (l1 || l2) internal("get_first_link == %p, get_last_link == %p", l1, l2);
		return;
	}
	do {
		if (l1->type != L_LINK) draw_form_entry(t, f, l1);
	} while (l1++ < l2);
}

/* 0 -> 1 <- 2 v 3 ^ */

unsigned char fr_trans[2][4] = {{0xb3, 0xc3, 0xb4, 0xc5}, {0xc4, 0xc2, 0xc1, 0xc5}};

void set_xchar(struct terminal *t, int x, int y, unsigned dir)
{
	unsigned c;
	if (x < 0 || x >= t->x || y < 0 || y >= t->y) return;
	c = get_char(t, x, y);
	if (!(c & ATTR_FRAME)) return;
	c &= 0xff;
	if (c == fr_trans[dir / 2][0]) set_only_char(t, x, y, fr_trans[dir / 2][1 + (dir & 1)] | ATTR_FRAME);
	else if (c == fr_trans[dir / 2][2 - (dir & 1)]) set_only_char(t, x, y, fr_trans[dir / 2][3] | ATTR_FRAME);
}

void draw_frame_lines(struct terminal *t, struct frameset_desc *fsd, int xp, int yp)
{
	int i, j;
	int x, y;
	if (!fsd) return;
	y = yp - 1;
	for (j = 0; j < fsd->y; j++) {
		int wwy = fsd->f[j * fsd->x].yw;
		x = xp - 1;
		for (i = 0; i < fsd->x; i++) {
			int wwx = fsd->f[i].xw;
			if (i) {
				fill_area(t, x, y + 1, 1, wwy, 179 | ATTR_FRAME);
				if (j == fsd->y - 1) set_xchar(t, x, y + wwy + 1, 3);
			} else if (j) set_xchar(t, x, y, 0);
			if (j) {
				fill_area(t, x + 1, y, wwx, 1, 196 | ATTR_FRAME);
				if (i == fsd->x - 1) set_xchar(t, x + wwx + 1, y, 1);
			} else if (i) set_xchar(t, x, y, 2);
			if (i && j) set_char(t, x, y, 197 | ATTR_FRAME);
			x += wwx + 1;
		}
		y += wwy + 1;
	}
	y = yp - 1;
	for (j = 0; j < fsd->y; j++) {
		int wwy = fsd->f[j * fsd->x].yw;
		x = xp - 1;
		for (i = 0; i < fsd->x; i++) {
			int wwx = fsd->f[i].xw;
			if (fsd->f[j * fsd->x + i].subframe) {
				draw_frame_lines(t, fsd->f[j * fsd->x + i].subframe, x + 1, y + 1);
			}
			x += wwx + 1;
		}
		y += wwy + 1;
	}
}

void draw_doc(struct terminal *t, struct f_data_c *scr, int active)
{
	int y;
	int xp = scr->xp;
	int yp = scr->yp;
	int xw = scr->xw;
	int yw = scr->yw;
	struct view_state *vs;
	int vx, vy;
	struct cache_entry *ce;
	if (active) {
		set_cursor(t, xp + xw - 1, yp + yw - 1, xp + xw - 1, yp + yw - 1);
		set_window_ptr(get_root_window(t), xp, yp);
	}
	if (!scr->vs) {
		fill_area(t, xp, yp, xw, yw, scr->f_data->y ? scr->f_data->bg : ' ');
		return;
	}
	if (scr->f_data->frame) {
	 	fill_area(t, xp, yp, xw, yw, scr->f_data->y ? scr->f_data->bg : ' ');
		draw_frame_lines(t, scr->f_data->frame_desc, xp, yp);
		if (scr->vs && scr->vs->current_link == -1) scr->vs->current_link = 0, scr->vs->orig_link = scr->vs->current_link;
		return;
	}
	check_vs(scr);
	vs = scr->vs;
	if (vs->goto_position && !get_cache_entry(vs->url, &ce)) {
		ce->refcount--;
		if ((vy = find_tag(scr->f_data, vs->goto_position)) != -1) {
			if (vs->goto_position_end) mem_free(vs->goto_position_end);
			if (ce->incomplete) {
				vs->goto_position_end = vs->goto_position;
			} else {
				vs->goto_position_end = NULL;
				mem_free(vs->goto_position);
			}
			vs->goto_position = NULL;
			goto goto_vy;
		}
	}
	if (vs->goto_position_end && !get_cache_entry(vs->url, &ce)) {
		ce->refcount--;
		if (!ce->incomplete && (vy = find_tag(scr->f_data, vs->goto_position_end)) != -1) {
			mem_free(vs->goto_position_end);
			vs->goto_position_end = NULL;
			goto_vy:
			if (vy > scr->f_data->y) vy = scr->f_data->y - 1;
			if (vy < 0) vy = 0;
			vs->view_pos = vy;
			vs->view_posx = 0;
			vs->orig_view_pos = vs->view_pos;
			vs->orig_view_posx = vs->view_posx;
			set_link(scr);
		}
	}
	if (vs->view_pos != vs->orig_view_pos || vs->view_posx != vs->orig_view_posx || vs->current_link != vs->orig_link) {
		int ol;
		vs->view_pos = vs->orig_view_pos;
		vs->view_posx = vs->orig_view_posx;
		ol = vs->orig_link;
		if (ol < scr->f_data->nlinks) vs->current_link = ol;
		while (vs->view_pos >= scr->f_data->y) vs->view_pos -= yw ? yw : 1;
		if (vs->view_pos < 0) vs->view_pos = 0;
		set_link(scr);
		check_vs(scr);
		vs->orig_link = ol;
	}
	vx = vs->view_posx;
	vy = vs->view_pos;
	if (scr->xl == vx && scr->yl == vy && scr->xl != -1 && (!scr->search_word || !*scr->search_word || !(*scr->search_word)[0])) {
		clear_link(t, scr);
		draw_forms(t, scr);
		if (active) draw_current_link(t, scr);
		return;
	}
	free_link(scr);
	scr->xl = vx;
	scr->yl = vy;
	fill_area(t, xp, yp, xw, yw, scr->f_data->y ? scr->f_data->bg : ' ');
	if (!scr->f_data->y) return;
	while (vs->view_pos >= scr->f_data->y) vs->view_pos -= yw ? yw : 1;
	if (vs->view_pos < 0) vs->view_pos = 0;
	if (vy != vs->view_pos) vy = vs->view_pos, check_vs(scr);
	for (y = vy <= 0 ? 0 : vy; y < (-vy + scr->f_data->y <= yw ? scr->f_data->y : yw + vy); y++) {
		int st = vx <= 0 ? 0 : vx;
		int en = -vx + scr->f_data->data[y].l <= xw ? scr->f_data->data[y].l : xw + vx;
		set_line(t, xp + st - vx, yp + y - vy, en - st, &scr->f_data->data[y].d[st]);
	}
	draw_forms(t, scr);
	if (active) draw_current_link(t, scr);
	if (scr->search_word && *scr->search_word && (*scr->search_word)[0]) scr->xl = scr->yl = -1;
}

void draw_frames(struct session *ses)
{
	int n;
	int d, more;
	int *l;
	struct f_data_c *f, *cf;
	if (!ses->screen->f_data->frame) return;
	n = 0;
	foreach(f, ses->scrn_frames) f->xl = f->yl = -1, n++;
	l = &cur_loc(ses)->vs.current_link;
	if (*l < 0) *l = 0;
	if (!n) n = 1;
	*l %= n;
	cf = current_frame(ses);
	d = 0;
	do {
		more = 0;
		foreach(f, ses->scrn_frames) {
			if (f->depth == d) draw_doc(ses->term, f, f == cf);
			else if (f->depth > d) more = 1;
		}
		d++;
	} while (more);
}

void draw_formatted(struct session *ses)
{
	if (!ses->screen || !ses->screen->f_data) {
		/*internal("document not formatted");*/
		fill_area(ses->term, 0, 1, ses->term->x, ses->term->y - 2, ' ');
		return;
	}
	if (!ses->screen->vs && !list_empty(ses->history))
		ses->screen->vs = &cur_loc(ses)->vs;
	ses->screen->xl = ses->screen->yl = -1;
	draw_doc(ses->term, ses->screen, 1);
	draw_frames(ses);
	print_screen_status(ses);
	redraw_from_window(ses->win);
}

#define D_BUF	65536

extern unsigned char frame_dumb[];

int dump_to_file(struct f_data *fd, int h)
{
	int x, y;
	unsigned char *buf;
	int bptr = 0;
	buf = mem_alloc(D_BUF);
	for (y = 0; y < fd->y; y++) for (x = 0; x <= fd->data[y].l; x++) {
		int c;
		if (x == fd->data[y].l) c = '\n';
		else {
			if (((c = fd->data[y].d[x]) & 0xff) == 1) c += ' ' - 1;
			if ((c >> 15) && (c & 0xff) >= 176 && (c & 0xff) < 224) c = frame_dumb[(c & 0xff) - 176];
		}
		buf[bptr++] = c;
		if (bptr >= D_BUF) {
			if (hard_write(h, buf, bptr) != bptr) goto fail;
			bptr = 0;
		}
	}
	if (hard_write(h, buf, bptr) != bptr) {
		fail:
		mem_free(buf);
		return -1;
	}
	mem_free(buf);
	if (fd->opt.num_links && fd->nlinks) {
		static char head[] = "\nLinks:\n";
		int i;
		if ((int)hard_write(h, head, strlen(head)) != (int)strlen(head)) return -1;
		for (i = 0; i < fd->nlinks; i++) {
			struct link *lnk = &fd->links[i];
			unsigned char *s = init_str();
			int l = 0;
			add_num_to_str(&s, &l, i + 1);
			add_to_str(&s, &l, ". ");
			if (lnk->where) {
				add_to_str(&s, &l, lnk->where);
			} else if (lnk->where_img) {
				add_to_str(&s, &l, "Image: ");
				add_to_str(&s, &l, lnk->where_img);
			} else if (lnk->type == L_BUTTON) {
				struct form_control *fc = lnk->form;
				if (fc->type == FC_RESET) add_to_str(&s, &l, "Reset form");
				else if (!fc->action) add_to_str(&s, &l, "Button");
				else {
					if (!fc->method == FM_GET) add_to_str(&s, &l, "Submit form: ");
					else add_to_str(&s, &l, "Post form: ");
					add_to_str(&s, &l, fc->action);
				}
			} else if (lnk->type == L_CHECKBOX || lnk->type == L_SELECT || lnk->type == L_FIELD || lnk->type == L_AREA) {
				struct form_control *fc = lnk->form;
				if (fc->type == FC_RADIO) add_to_str(&s, &l, "Radio button");
				else if (fc->type == FC_CHECKBOX) add_to_str(&s, &l, "Checkbox");
				else if (fc->type == FC_SELECT) add_to_str(&s, &l, "Select field");
				else if (fc->type == FC_TEXT) add_to_str(&s, &l, "Text field");
				else if (fc->type == FC_TEXTAREA) add_to_str(&s, &l, "Text area");
				else if (fc->type == FC_FILE) add_to_str(&s, &l, "File upload");
				else if (fc->type == FC_PASSWORD) add_to_str(&s, &l, "Password field");
				else goto unknown;
				if (fc->name && fc->name[0]) add_to_str(&s, &l, ", Name "), add_to_str(&s, &l, fc->name);
				if ((fc->type == FC_CHECKBOX || fc->type == FC_RADIO) && fc->default_value && fc->default_value[0]) add_to_str(&s, &l, ", Value "), add_to_str(&s, &l, fc->default_value);
			}
			unknown:
			add_to_str(&s, &l, "\n");
			if (hard_write(h, s, l) != l) {
				mem_free(s);
				return -1;
			}
			mem_free(s);
		}
	}
	return 0;
}

int in_viewx(struct f_data_c *f, struct link *l)
{
	int i;
	for (i = 0; i < l->n; i++) {
		if (l->pos[i].x >= f->vs->view_posx && l->pos[i].x < f->vs->view_posx + f->xw)
			return 1;
	}
	return 0;
}

int in_viewy(struct f_data_c *f, struct link *l)
{
	int i;
	for (i = 0; i < l->n; i++) {
		if (l->pos[i].y >= f->vs->view_pos && l->pos[i].y < f->vs->view_pos + f->yw)
		return 1;
	}
	return 0;
}

int in_view(struct f_data_c *f, struct link *l)
{
	return in_viewy(f, l) && in_viewx(f, l);
}

static inline int c_in_view(struct f_data_c *f)
{
	return f->vs->current_link != -1 && in_view(f, &f->f_data->links[f->vs->current_link]);
}

int next_in_view(struct f_data_c *f, int p, int d, int (*fn)(struct f_data_c *, struct link *), void (*cntr)(struct f_data_c *, struct link *))
{
	int p1 = f->f_data->nlinks - 1;
	int p2 = 0;
	int y;
	int yl = f->vs->view_pos + f->yw;
	if (yl > f->f_data->y) yl = f->f_data->y;
	for (y = f->vs->view_pos < 0 ? 0 : f->vs->view_pos; y < yl; y++) {
		if (f->f_data->lines1[y] && f->f_data->lines1[y] - f->f_data->links < p1) p1 = f->f_data->lines1[y] - f->f_data->links;
		if (f->f_data->lines2[y] && f->f_data->lines2[y] - f->f_data->links > p2) p2 = f->f_data->lines2[y] - f->f_data->links;
	}
	/*while (p >= 0 && p < f->f_data->nlinks) {*/
	while (p >= p1 && p <= p2) {
		if (fn(f, &f->f_data->links[p])) {
			f->vs->current_link = p;
			f->vs->orig_link = f->vs->current_link;
			if (cntr) cntr(f, &f->f_data->links[p]);
			return 1;
		}
		p += d;
	}
	f->vs->current_link = -1;
	f->vs->orig_link = f->vs->current_link;
	return 0;
}

void set_pos_x(struct f_data_c *f, struct link *l)
{
	int i;
	int xm = 0;
	int xl = MAXINT;
	for (i = 0; i < l->n; i++) {
		if (l->pos[i].y >= f->vs->view_pos && l->pos[i].y < f->vs->view_pos + f->yw) {
			if (l->pos[i].x >= xm) xm = l->pos[i].x + 1;
			if (l->pos[i].x < xl) xl = l->pos[i].x;
		}
	}
	if (xl == MAXINT) return;
	/*if ((f->vs->view_posx = xm - f->xw) > xl) f->vs->view_posx = xl;*/
	if (f->vs->view_posx + f->xw < xm) f->vs->view_posx = xm - f->xw;
	if (f->vs->view_posx > xl) f->vs->view_posx = xl;
	f->vs->orig_view_posx = f->vs->view_posx;
}

void set_pos_y(struct f_data_c *f, struct link *l)
{
	int i;
	int ym = 0;
	int yl = f->f_data->y;
	for (i = 0; i < l->n; i++) {
		if (l->pos[i].y >= ym) ym = l->pos[i].y + 1;
		if (l->pos[i].y < yl) yl = l->pos[i].y;
	}
	if ((f->vs->view_pos = (ym + yl) / 2 - f->f_data->opt.yw / 2) > f->f_data->y - f->f_data->opt.yw) f->vs->view_pos = f->f_data->y - f->f_data->opt.yw;
	if (f->vs->view_pos < 0) f->vs->view_pos = 0;
	f->vs->orig_view_pos = f->vs->view_pos;
}

void find_link(struct f_data_c *f, int p, int s)
{ /* p=1 - top, p=-1 - bottom, s=0 - pgdn, s=1 - down */
	int y;
	int l;
	struct link *link;
	struct link **line = p == -1 ? f->f_data->lines2 : f->f_data->lines1;
	if (p == -1) {
		y = f->vs->view_pos + f->yw - 1;
		if (y >= f->f_data->y) y = f->f_data->y - 1;
	} else {
		y = f->vs->view_pos;
		if (y < 0) y = 0;
	}
	if (y < 0 || y >= f->f_data->y) goto nolink;
	link = NULL;
	do {
		if (line[y] && (!link || (p > 0 ? line[y] < link : line[y] > link))) link = line[y];
		y += p;
	} while (!(y < 0 || y < f->vs->view_pos || y >= f->vs->view_pos + f->f_data->opt.yw || y >= f->f_data->y));
	if (!link) goto nolink;
	l = link - f->f_data->links;
	if (s == 0) {
		next_in_view(f, l, p, in_view, NULL);
		return;
	}
	f->vs->current_link = l;
	f->vs->orig_link = f->vs->current_link;
	set_pos_x(f, link);
	return;
	nolink:
	f->vs->current_link = -1;
	f->vs->orig_link = f->vs->current_link;
}

void page_down(struct session *ses, struct f_data_c *f, int a)
{
	if (f->vs->view_pos + f->f_data->opt.yw < f->f_data->y) f->vs->view_pos += f->f_data->opt.yw, f->vs->orig_view_pos = f->vs->view_pos, find_link(f, 1, a);
	else find_link(f, -1, a);
}

void page_up(struct session *ses, struct f_data_c *f, int a)
{
	f->vs->view_pos -= f->yw;
	find_link(f, -1, a);
	if (f->vs->view_pos < 0) f->vs->view_pos = 0/*, find_link(f, 1, a)*/;
	f->vs->orig_view_pos = f->vs->view_pos;
}

void set_textarea(struct session *, struct f_data_c *, int);

void down(struct session *ses, struct f_data_c *f, int a)
{
	int l = f->vs->current_link;
	/*if (f->vs->current_link >= f->nlinks - 1) return;*/
	if (f->vs->current_link == -1 || !next_in_view(f, f->vs->current_link+1, 1, in_viewy, set_pos_x)) page_down(ses, f, 1);
	if (l != f->vs->current_link) set_textarea(ses, f, KBD_UP);
}

void up(struct session *ses, struct f_data_c *f, int a)
{
	int l = f->vs->current_link;
	/*if (f->vs->current_link == 0) return;*/
	if (f->vs->current_link == -1 || !next_in_view(f, f->vs->current_link-1, -1, in_viewy, set_pos_x)) page_up(ses, f, 1);
	if (l != f->vs->current_link) set_textarea(ses, f, KBD_DOWN);
}

void scroll(struct session *ses, struct f_data_c *f, int a)
{
	if (f->vs->view_pos + f->f_data->opt.yw >= f->f_data->y && a > 0) return;
	f->vs->view_pos += a;
	if (f->vs->view_pos > f->f_data->y - f->f_data->opt.yw && a > 0) f->vs->view_pos = f->f_data->y - f->f_data->opt.yw;
	if (f->vs->view_pos < 0) f->vs->view_pos = 0;
	f->vs->orig_view_pos = f->vs->view_pos;
	if (c_in_view(f)) return;
	find_link(f, a < 0 ? -1 : 1, 0);
}

void hscroll(struct session *ses, struct f_data_c *f, int a)
{
	f->vs->view_posx += a;
	if (f->vs->view_posx >= f->f_data->x) f->vs->view_posx = f->f_data->x - 1;
	if (f->vs->view_posx < 0) f->vs->view_posx = 0;
	f->vs->orig_view_posx = f->vs->view_posx;
	if (c_in_view(f)) return;
	find_link(f, 1, 0);
	/* !!! FIXME: check right margin */
}

void home(struct session *ses, struct f_data_c *f, int a)
{
	f->vs->view_pos = f->vs->view_posx = 0;
	f->vs->orig_view_posx = f->vs->view_posx;
	f->vs->orig_view_pos = f->vs->view_pos;
	find_link(f, 1, 0);
}

void x_end(struct session *ses, struct f_data_c *f, int a)
{
	f->vs->view_posx = 0;
	if (f->vs->view_pos < f->f_data->y - f->f_data->opt.yw) f->vs->view_pos = f->f_data->y - f->f_data->opt.yw;
	if (f->vs->view_pos < 0) f->vs->view_pos = 0;
	f->vs->orig_view_pos = f->vs->view_pos;
	f->vs->orig_view_posx = f->vs->view_posx;
	find_link(f, -1, 0);
}

int has_form_submit(struct f_data *f, struct form_control *form)
{
	struct form_control *i;
	int q = 0;
	foreach (i, f->forms) if (i->form_num == form->form_num) {
		if ((i->type == FC_SUBMIT || i->type == FC_IMAGE)) return 1;
		q = 1;
	}
	if (!q) internal("form is not on list");
	return 0;
}

void decrement_fc_refcount(struct f_data *f)
{
	if (!--f->refcount) format_cache_entries++;
}

struct submitted_value {
	struct submitted_value *next;
	struct submitted_value *prev;
	int type;
	unsigned char *name;
	unsigned char *value;
	void *file_content;
	int fc_len;
	int position;
};

void free_succesful_controls(struct list_head *submit)
{
	struct submitted_value *v;
	foreach(v, *submit) {
		if (v->name) mem_free(v->name);
		if (v->value) mem_free(v->value);
		if (v->file_content) mem_free(v->file_content);
	}
	free_list(*submit);
}

unsigned char *encode_textarea(unsigned char *t)
{
	int len = 0;
	unsigned char *o = init_str();
	for (; *t; t++) {
		if (*t != '\n') add_chr_to_str(&o, &len, *t);
		else add_to_str(&o, &len, "\r\n");
	}
	return o;
}

int compare_submitted(struct submitted_value *sub1, struct submitted_value *sub2);
int compare_submitted(struct submitted_value *sub1, struct submitted_value *sub2)
{
	/*int c = (sub1->type == FC_IMAGE) - (sub2->type == FC_IMAGE);
	if (c) return c;*/
	return sub1->position - sub2->position;
}

void get_succesful_controls(struct f_data_c *f, struct form_control *fc, struct list_head *subm)
{
	int ch;
	struct form_control *form;
	init_list(*subm);
	foreach(form, f->f_data->forms) {
		if (form->form_num == fc->form_num && ((form->type != FC_SUBMIT && form->type != FC_IMAGE && form->type != FC_RESET) || form == fc) && form->name && form->name[0] && form->ro != 2) {
			struct submitted_value *sub;
			struct form_state *fs;
			int fi = form->type == FC_IMAGE && form->default_value && *form->default_value ? -1 : 0;;
			if (!(fs = find_form_state(f, form))) continue;
			if ((form->type == FC_CHECKBOX || form->type == FC_RADIO) && !fs->state) continue;
			if (form->type == FC_SELECT && !form->nvalues) continue;
			fi_rep:
			sub = mem_alloc(sizeof(struct submitted_value));
			memset(sub, 0, sizeof(struct submitted_value));
			sub->type = form->type;
			sub->name = stracpy(form->name);
			switch (form->type) {
				case FC_TEXT:
				case FC_PASSWORD:
				case FC_FILE:
					sub->value = stracpy(fs->value);
					break;
				case FC_TEXTAREA:
					sub->value = encode_textarea(fs->value);
					break;
				case FC_CHECKBOX:
				case FC_RADIO:
				case FC_SUBMIT:
				case FC_HIDDEN:
					sub->value = encode_textarea(form->default_value);
					break;
				case FC_SELECT:
					fixup_select_state(form, fs);
					sub->value = encode_textarea(fs->value);
					break;
				case FC_IMAGE:
					if (fi == -1) {
						sub->value = encode_textarea(form->default_value);
						break;
					}
					add_to_strn(&sub->name, fi ? ".x" : ".y");
					sub->value = stracpy("1");
					break;
				default:
					internal("bad form control type");
					mem_free(sub);
					continue;
			}
			sub->position = form->form_num + form->ctrl_num;
			add_to_list(*subm, sub);
			if (form->type == FC_IMAGE && fi < 1) {
				fi++;
				goto fi_rep;
			}
		}
	}
	do {
		struct submitted_value *sub, *nx;
		ch = 0;
		foreach(sub, *subm) if (sub->next != (void *)subm)
			if (sub->next->position < sub->position) {
				nx = sub->next;
				del_from_list(sub);
				add_at_pos(nx, sub);
				sub = nx;
				ch = 1;
			}
		foreachback(sub, *subm) if (sub->next != (void *)subm)
			if (sub->next->position < sub->position) {
				nx = sub->next;
				del_from_list(sub);
				add_at_pos(nx, sub);
				sub = nx;
				ch = 1;
			}
	} while (ch);
			
}

unsigned char *strip_file_name(unsigned char *f)
{
	unsigned char *n;
	unsigned char *l = f - 1;
	for (n = f; *n; n++) if (dir_sep(*n)) l = n;
	return l + 1;
}

static inline int safe_char(unsigned char c)
{
	return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c== '.' || c == '-' || c == '_';
}

void encode_string(unsigned char *name, unsigned char **data, int *len)
{
	for (; *name; name++) {
		if (*name == ' ') add_chr_to_str(data, len, '+');
		else if (safe_char(*name)) add_chr_to_str(data, len, *name);
		else {
			unsigned char n[4];
			sprintf(n, "%%%02X", *name);
			add_to_str(data, len, n);
		}
	}
}

void encode_controls(struct list_head *l, unsigned char **data, int *len,
		     int cp_from, int cp_to)
{
	struct submitted_value *sv;
	int lst = 0;
	char *p2;
	struct conv_table *convert_table = get_translation_table(cp_from, cp_to);
	*len = 0;
	*data = init_str();
	foreach(sv, *l) {
		unsigned char *p = sv->value;
		struct document_options o;
		memset(&o, 0, sizeof o);
		o.plain = 1;
		d_opt = &o;
		/*if (sv->type == FC_TEXTAREA) p = encode_textarea(sv->value);*/
		if (lst) add_to_str(data, len, "&"); else lst = 1;
		encode_string(sv->name, data, len);
		add_to_str(data, len, "=");
		if (sv->type == FC_TEXT || sv->type == FC_PASSWORD || sv->type == FC_TEXTAREA)
			p2 = convert_string(convert_table, p, strlen(p));
		else p2 = stracpy(p);
		encode_string(p2, data, len);
		mem_free(p2);
		/*if (sv->type == FC_TEXTAREA) mem_free(p);*/
	}
}

#define BL	56
#define BL1	27

void encode_multipart(struct session *ses, struct list_head *l, unsigned char **data, int *len,
		      unsigned char *bound, int cp_from, int cp_to)
{
	int errn;
	int *bound_ptrs = DUMMY;
	int nbound_ptrs = 0;
	unsigned char *m1, *m2;
	struct submitted_value *sv;
	int i, j;
	int flg = 0;
	unsigned char *p;
	struct conv_table *convert_table = get_translation_table(cp_from, cp_to);
	memset(bound, 'x', BL);
	*len = 0;
	*data = init_str();
	foreach(sv, *l) {
		unsigned char *ct;
		bnd:
		add_to_str(data, len, "--");
		if (!(nbound_ptrs & (ALLOC_GR-1))) {
			if ((unsigned)nbound_ptrs > MAXINT / sizeof(int) - ALLOC_GR) overalloc();
			bound_ptrs = mem_realloc(bound_ptrs, (nbound_ptrs + ALLOC_GR) * sizeof(int));
		}
		bound_ptrs[nbound_ptrs++] = *len;
		add_bytes_to_str(data, len, bound, BL);
		if (flg) break;
		add_to_str(data, len, "\r\nContent-Disposition: form-data; name=\"");
		add_to_str(data, len, sv->name);
		add_to_str(data, len, "\"");
		if (sv->type == FC_FILE) {
			add_to_str(data, len, "; filename=\"");
			add_to_str(data, len, strip_file_name(sv->value));
				/* It sends bad data if the file name contains ", but
				   Netscape does the same */
			add_to_str(data, len, "\"");
			if (*sv->value) if ((ct = get_content_type(NULL, sv->value))) {
				add_to_str(data, len, "\r\nContent-Type: ");
				add_to_str(data, len, ct);
				if (strlen(ct) >= 4 && !casecmp(ct, "text", 4)) {
					add_to_str(data, len, "; charset=");
					add_to_str(data, len, get_cp_mime_name(ses->term->spec->charset));
				}
				mem_free(ct);
			}
		}
		add_to_str(data, len, "\r\n\r\n");
		if (sv->type != FC_FILE) {
			struct document_options o;
			o.plain = 1;
			d_opt = &o;
			if (sv->type == FC_TEXT || sv->type == FC_PASSWORD || sv->type == FC_TEXTAREA)
				p = convert_string(convert_table, sv->value, strlen(sv->value));
			else p = stracpy(sv->value);
			add_to_str(data, len, p);
			mem_free(p);
		} else {
			int fh, rd;
#define F_BUFLEN 1024
			unsigned char buffer[F_BUFLEN];
			/*if (!check_file_name(sv->value)) {
				errn = errno;
				err = "File access forbidden";
				goto error;
			}*/
			if (*sv->value) {
				if (anonymous) {
					goto not_allowed;
				}
				if ((fh = open(sv->value, O_RDONLY)) == -1) {
					errn = errno;
					goto error;
				}
				set_bin(fh);
				do {
					if ((rd = read(fh, buffer, F_BUFLEN)) == -1) {
						errn = errno;
						close(fh);
						goto error;
					}
					if (rd) add_bytes_to_str(data, len, buffer, rd);
				} while (rd);
				close(fh);
			}
		}
		add_to_str(data, len, "\r\n");
	}
	if (!flg) {
		flg = 1;
		goto bnd;
	}
	add_to_str(data, len, "--\r\n");
	memset(bound, '-', BL1);
	memset(bound + BL1, '0', BL - BL1);
	again:
	for (i = 0; i <= *len - BL; i++) {
		for (j = 0; j < BL; j++) if ((*data)[i + j] != bound[j]) goto nb;
		for (j = BL - 1; j >= 0; j--)
			if (bound[j] < '0') bound[j] = '0' - 1;
			if (bound[j]++ >= '9') bound[j] = '0';
			else goto again;
		internal("Counld not assing boundary");
		nb:;
	}
	for (i = 0; i < nbound_ptrs; i++) memcpy(*data + bound_ptrs[i], bound, BL);
	mem_free(bound_ptrs);
	return;

	error:
	mem_free(bound_ptrs);
	mem_free(*data);
	*data = NULL;
	m1 = stracpy(sv->value);
	m2 = stracpy(strerror(errn));
	msg_box(ses->term, getml(m1, m2, NULL), TEXT_(T_ERROR_WHILE_POSTING_FORM), AL_CENTER | AL_EXTD_TEXT, TEXT_(T_COULD_NOT_GET_FILE), " ", m1, ": ", m2, NULL, ses, 1, TEXT_(T_CANCEL), NULL, B_ENTER | B_ESC);

	not_allowed:
	mem_free(bound_ptrs);
	mem_free(*data);
	*data = NULL;
	msg_box(ses->term, NULL, TEXT_(T_ERROR_WHILE_POSTING_FORM), AL_CENTER | AL_EXTD_TEXT, TEXT_(T_READING_FILES_IS_NOT_ALLOWED), NULL, ses, 1, TEXT_(T_CANCEL), NULL, B_ENTER | B_ESC);
}

void reset_form(struct f_data_c *f, int form_num)
{
	struct form_control *form;
	foreach(form, f->f_data->forms) if (form->form_num == form_num) {
		struct form_state *fs;
		if ((fs = find_form_state(f, form))) init_ctrl(form, fs);
	}
}
		
unsigned char *get_form_url(struct session *ses, struct f_data_c *f, struct form_control *form)
{
	struct list_head submit;
	unsigned char *data;
	unsigned char bound[BL];
	int len;
	unsigned char *go = NULL;
	int cp_from, cp_to;
	if (!form) return NULL;
	if (form->type == FC_RESET) {
		reset_form(f, form->form_num);
		return NULL;
	}
	if (!form->action) return NULL;
	get_succesful_controls(f, form, &submit);
	cp_from = ses->term->spec->charset;
	cp_to = f->f_data->cp;
	if (form->method == FM_GET || form->method == FM_POST)
		encode_controls(&submit, &data, &len, cp_from, cp_to);
	else
		encode_multipart(ses, &submit, &data, &len, bound, cp_from, cp_to);
	if (!data) goto ff;
	if (form->method == FM_GET) {
		unsigned char *pos;
		int q;
		if (strlen(form->action) + 2 + len < (unsigned)len) overalloc();
		go = mem_alloc(strlen(form->action) + 1 + len + 1);
		strcpy(go, form->action);
		if ((pos = strchr(go, '#'))) {
			unsigned char *poss = pos;
			pos = stracpy(pos);
			*poss = 0;
		}
		q = strlen(go);
		if (q && (go[q - 1] == '&' || go[q - 1] == '?')) ;
		else if (strchr(go, '?')) strcat(go, "&");
		else strcat(go, "?");
		strcat(go, data);
		if (pos) strcat(go, pos), mem_free(pos);
	} else {
		int l = 0;
		int i;
		go = init_str();
		add_to_str(&go, &l, form->action);
		add_chr_to_str(&go, &l, POST_CHAR);
		if (form->method == FM_POST) add_to_str(&go, &l, "application/x-www-form-urlencoded\n");
		else {
			add_to_str(&go, &l, "multipart/form-data; boundary=");
			add_bytes_to_str(&go, &l, bound, BL);
			add_to_str(&go, &l, "\n");
		}
		for (i = 0; i < len; i++) {
			unsigned char p[3];
			sprintf(p, "%02x", (int)data[i]);
			add_to_str(&go, &l, p);
		}
	}
	mem_free(data);
	ff:
	free_succesful_controls(&submit);
	return go;
}

unsigned char *get_link_url(struct session *ses, struct f_data_c *f, struct link *l)
{
	if (l->type == L_LINK) {
		if (!l->where) return stracpy(l->where_img);
		return stracpy(l->where);
	}
	if (l->type != L_BUTTON && l->type != L_FIELD) return NULL;
	return get_form_url(ses, f, l->form);
}

void set_frame(struct session *ses, struct f_data_c *f, int a)
{
	if (f == ses->screen) return;
	goto_url(ses, f->vs->url);
}

int enter(struct session *ses, struct f_data_c *f, int a)
{
	struct link *link;
	unsigned char *u;
	if (f->vs->current_link == -1) return 1;
	link = &f->f_data->links[f->vs->current_link];
	if (link->type == L_LINK || link->type == L_BUTTON) {
		submit:
		if ((u = get_link_url(ses, f, link))) {
			if (strlen(u) >= 4 && !casecmp(u, "MAP@", 4))
				goto_imgmap(ses, u + 4, stracpy(u + 4), stracpy(link->target));
			else goto_url_f(ses, u, link->target);
			mem_free(u);
			return 2;
		}
	} else if (link->type == L_FIELD || link->type == L_AREA) {
		if (!has_form_submit(f->f_data, link->form)) goto submit;
		down(ses, f, 0);
	} else if (link->type == L_CHECKBOX) {
		struct form_state *fs = find_form_state(f, link->form);
		if (link->form->ro) return 1;
		if (link->form->type == FC_CHECKBOX) fs->state = !fs->state;
		else {
			struct form_control *fc;
			foreach(fc, f->f_data->forms)
				if (fc->form_num == link->form->form_num && fc->type == FC_RADIO && !xstrcmp(fc->name, link->form->name)) {
					struct form_state *ffs = find_form_state(f, fc);
					if (ffs) ffs->state = 0;
				}
			fs = find_form_state(f, link->form);
			fs->state = 1;
		}
	} else if (link->type == L_SELECT) {
		if (link->form->ro) return 1;
		f->f_data->refcount++;
		add_empty_window(ses->term, (void (*)(void *))decrement_fc_refcount, f->f_data);
		do_select_submenu(ses->term, link->form->menu, ses);
	} else internal("bad link type %d", link->type);
	return 1;
}

void toggle(struct session *ses, struct f_data_c *f, int a)
{
	if (!f || !f->vs) return;
	f->vs->plain = !f->vs->plain;
	html_interpret(ses);
	draw_formatted(ses);
}

void back(struct session *ses, struct f_data_c *f, int a)
{
	go_back(ses);
}

void selected_item(struct terminal *term, void *pitem, struct session *ses)
{
	int item = (int)pitem;
	struct f_data_c *f = current_frame(ses);
	struct link *l;
	struct form_state *fs;
	if (!f) return;
	if (f->vs->current_link == -1) return;
	l = &f->f_data->links[f->vs->current_link];
	if (l->type != L_SELECT) return;
	if ((fs = find_form_state(f, l->form))) {
		struct form_control *form= l->form;
		if (item >= 0 && item < form->nvalues) {
			fs->state = item;
			if (fs->value) mem_free(fs->value);
			fs->value = stracpy(form->values[item]);
		}
		fixup_select_state(form, fs);
	}
	draw_doc(ses->term, f, 1);
	print_screen_status(ses);
	redraw_from_window(ses->win);
	/*if (!has_form_submit(f->f_data, l->form)) {
		goto_form(ses, f, l->form, l->target);
	}*/
}

int get_current_state(struct session *ses)
{
	struct f_data_c *f = current_frame(ses);
	struct link *l;
	struct form_state *fs;
	if (!f) return -1;
	if (f->vs->current_link == -1) return -1;
	l = &f->f_data->links[f->vs->current_link];
	if (l->type != L_SELECT) return -1;
	if ((fs = find_form_state(f, l->form))) return fs->state;
	return -1;
}

int textarea_adjust_viewport(struct f_data_c *fd, struct link *l)
{
	struct form_control *fc = l->form;
	struct view_state *vs = fd->vs;
	int r = 0;
	if (l->pos[0].x + fc->cols > fd->xw + vs->view_posx)
		vs->view_posx = l->pos[0].x + fc->cols - fd->xw, r = 1;
	if (l->pos[0].x < vs->view_posx)
		vs->view_posx = l->pos[0].x, r = 1;
	if (l->pos[0].y + fc->rows > fd->yw + vs->view_pos)
		vs->view_pos = l->pos[0].y + fc->rows - fd->yw, r = 1;
	if (l->pos[0].y < vs->view_pos)
		vs->view_pos = l->pos[0].y, r = 1;
	vs->orig_view_pos = vs->view_pos;
	vs->orig_view_posx = vs->view_posx;
	return r;
}

int field_op(struct session *ses, struct f_data_c *f, struct link *l, struct event *ev, int rep)
{
	struct form_control *form = l->form;
	struct form_state *fs;
	int x = 1;
	if (!form) {
		internal("link has no form control");
		return 0;
	}
	if (l->form->ro == 2) return 0;
	if (!(fs = find_form_state(f, form))) return 0;
	if (!fs->value) return 0;
	if (ev->ev == EV_KBD) {
		switch (kbd_action(KM_EDIT, ev)) {
			case ACT_LEFT: fs->state = fs->state ? fs->state - 1 : 0; break;
			case ACT_RIGHT: fs->state = (size_t)fs->state < strlen(fs->value) ? fs->state + 1 : (int)strlen(fs->value); break;
			case ACT_HOME:
				if (form->type == FC_TEXTAREA) {
					struct line_info *ln;
					if ((ln = format_text(fs->value, form->cols, form->wrap))) {
						int y;
						for (y = 0; ln[y].st; y++) if (fs->value + fs->state >= ln[y].st && fs->value + fs->state < ln[y].en + (ln[y+1].st != ln[y].en)) {
							fs->state = ln[y].st - fs->value;
							goto x;
						}
						fs->state = 0;
						x:
						mem_free(ln);
					}
				} else fs->state = 0;
				break;
			case ACT_UP:
				if (form->type == FC_TEXTAREA) {
					struct line_info *ln;
					if ((ln = format_text(fs->value, form->cols, form->wrap))) {
						int y;
						rep1:
						for (y = 0; ln[y].st; y++) if (fs->value + fs->state >= ln[y].st && fs->value + fs->state < ln[y].en + (ln[y+1].st != ln[y].en)) {
							if (!y) {
								mem_free(ln);
								goto b;
							}
							fs->state -= ln[y].st - ln[y-1].st;
							if (fs->value + fs->state > ln[y-1].en) fs->state = ln[y-1].en - fs->value;
							goto xx;
						}
						mem_free(ln);
						goto b;
						xx:
						if (rep) goto rep1;
						mem_free(ln);
					}
				} else x = 0;
				break;
			case ACT_DOWN:
				if (form->type == FC_TEXTAREA) {
					struct line_info *ln;
					if ((ln = format_text(fs->value, form->cols, form->wrap))) {
						int y;
						rep2:
						for (y = 0; ln[y].st; y++) if (fs->value + fs->state >= ln[y].st && fs->value + fs->state < ln[y].en + (ln[y+1].st != ln[y].en)) {
							if (!ln[y+1].st) {
								mem_free(ln);
								goto b;
							}
							fs->state += ln[y+1].st - ln[y].st;
							if (fs->value + fs->state > ln[y+1].en) fs->state = ln[y+1].en - fs->value;
							goto yy;
						}
						mem_free(ln);
						goto b;
						yy:
						if (rep) goto rep2;
						mem_free(ln);
					}
				} else x = 0;
				break;
			case ACT_END:
				if (form->type == FC_TEXTAREA) {
					struct line_info *ln;
					if ((ln = format_text(fs->value, form->cols, form->wrap))) {
						int y;
						for (y = 0; ln[y].st; y++) if (fs->value + fs->state >= ln[y].st && fs->value + fs->state < ln[y].en + (ln[y+1].st != ln[y].en)) {
							fs->state = ln[y].en - fs->value;
							if (fs->state && (size_t)fs->state < strlen(fs->value) && ln[y+1].st == ln[y].en) fs->state--;
							goto yyyy;
						}
						fs->state = strlen(fs->value);
						yyyy:
						mem_free(ln);
					}
				} else fs->state = strlen(fs->value);
				break;
			case ACT_COPY_CLIPBOARD:
				set_clipboard_text(fs->value);
				break;
			case ACT_CUT_CLIPBOARD:
				set_clipboard_text(fs->value);
				if (!form->ro) fs->value[0] = 0;
				fs->state = 0;
				break;
			case ACT_PASTE_CLIPBOARD: {
				char *clipboard = get_clipboard_text();
				if (!clipboard) break;
				if (!form->ro && strlen(clipboard) <= (size_t)form->maxlength) {
					unsigned char *v;
					v = mem_realloc(fs->value, strlen(clipboard) +1);
					fs->value = v;
					memmove(v , clipboard, strlen(clipboard) +1);
					fs->state = strlen(fs->value);
				}
				mem_free(clipboard);
				break;
			}
			case ACT_ENTER:
				if (form->type == FC_TEXTAREA) {
					if (!form->ro && strlen(fs->value) < (size_t)form->maxlength) {
						unsigned char *v;
						v = mem_realloc(fs->value, strlen(fs->value) + 2);
						fs->value = v;
						memmove(v + fs->state + 1, v + fs->state, strlen(v + fs->state) + 1);
						v[fs->state++] = '\n';
					}
				}
				else x = 0;
				break;
			case ACT_BACKSPACE:
				if (!form->ro && fs->state) memmove(fs->value + fs->state - 1, fs->value + fs->state, strlen(fs->value + fs->state) + 1), fs->state--;
				break;
			case ACT_DELETE:
				if (!form->ro && (size_t)fs->state < strlen(fs->value)) memmove(fs->value + fs->state, fs->value + fs->state + 1, strlen(fs->value + fs->state));
				break;
			case ACT_KILL_TO_BOL:
				if (!form->ro) memmove(fs->value, fs->value + fs->state, strlen(fs->value + fs->state) + 1);
				fs->state = 0;
				break;
		    	case ACT_KILL_TO_EOL:
				fs->value[fs->state] = 0;
				break;
			default:
				if (!ev->y && (ev->x >= 32 && ev->x < 256 && cp2u(ev->x, ses->term->spec->charset) != -1)) {
					if (!form->ro && strlen(fs->value) < (size_t)form->maxlength) {
						unsigned char *v;
						v = mem_realloc(fs->value, strlen(fs->value) + 2);
						fs->value = v;
						memmove(v + fs->state + 1, v + fs->state, strlen(v + fs->state) + 1);
						v[fs->state++] = ev->x;
					}
				} else {
					b:
					x = 0;
				}
		}
	} else x = 0;
	if (x) {
		if (ev->x != KBD_UP && ev->x != KBD_DOWN && form->type == FC_TEXTAREA) textarea_adjust_viewport(f, l);
		draw_form_entry(ses->term, f, l);
		redraw_from_window(ses->win);
	}
	return x;
}

void set_textarea(struct session *ses, struct f_data_c *f, int kbd)
{
	if (f->vs->current_link != -1 && f->f_data->links[f->vs->current_link].type == L_AREA) {
		struct event ev = { EV_KBD, 0, 0, 0 };
		ev.x = kbd;
		field_op(ses, f, &f->f_data->links[f->vs->current_link], &ev, 1);
	}
}

void search_for_back(struct session *ses, unsigned char *str)
{
	struct f_data_c *f = current_frame(ses);
	if (!f || !str || !str[0]) return;
	if (ses->search_word) mem_free(ses->search_word);
	ses->search_word = stracpy(str);
	charset_upcase_string(&ses->search_word, ses->term->spec->charset);
	if (ses->last_search_word) mem_free(ses->last_search_word);
	ses->last_search_word = stracpy(ses->search_word);
	ses->search_direction = -1;
	find_next(ses, f, 1);
}

void search_for(struct session *ses, unsigned char *str)
{
	struct f_data_c *f = current_frame(ses);
	if (!f || !str || !str[0]) return;
	if (ses->search_word) mem_free(ses->search_word);
	ses->search_word = stracpy(str);
	charset_upcase_string(&ses->search_word, ses->term->spec->charset);
	if (ses->last_search_word) mem_free(ses->last_search_word);
	ses->last_search_word = stracpy(ses->search_word);
	ses->search_direction = 1;
	find_next(ses, f, 1);
}

#define HASH_SIZE	4096

#define HASH(p) (((p.y << 6) + p.x) & (HASH_SIZE - 1))

int point_intersect(struct point *p1, int l1, struct point *p2, int l2)
{
	int i, j;
	static char hash[HASH_SIZE];
	static char init = 0;
	if (!init) memset(hash, 0, HASH_SIZE), init = 1;
	for (i = 0; i < l1; i++) hash[HASH(p1[i])] = 1;
	for (j = 0; j < l2; j++) if (hash[HASH(p2[j])]) {
		for (i = 0; i < l1; i++) if (p1[i].x == p2[j].x && p1[i].y == p2[j].y) {
			for (i = 0; i < l1; i++) hash[HASH(p1[i])] = 0;
			return 1;
		}
	}
	for (i = 0; i < l1; i++) hash[HASH(p1[i])] = 0;
	return 0;
}

int find_next_link_in_search(struct f_data_c *f, int d)
{
	struct point *pt;
	int len;
	struct link *link;
	if (d == -2 || d == 2) {
		d /= 2;
		find_link(f, d, 0);
		if (f->vs->current_link == -1) return 1;
	} else nx:if (f->vs->current_link == -1 || !(next_in_view(f, f->vs->current_link + d, d, in_view, NULL))) {
		find_link(f, d, 0);
		return 1;
	}
	link = &f->f_data->links[f->vs->current_link];
	get_searched(f, &pt, &len);
	if (point_intersect(pt, len, link->pos, link->n)) {
		mem_free(pt);
		return 0;
	}
	mem_free(pt);
	goto nx;
}

void find_next(struct session *ses, struct f_data_c *f, int a)
{
	int min, max;
	int c = 0;
	int p = f->vs->view_pos;
	if (!a && ses->search_word) {
		if (!(find_next_link_in_search(f, ses->search_direction))) return;
		p += ses->search_direction * f->yw;
	}
	if (!ses->search_word) {
		if (!ses->last_search_word) {
			msg_box(ses->term, NULL, TEXT_(T_SEARCH), AL_CENTER, TEXT_(T_NO_PREVIOUS_SEARCH), NULL, 1, TEXT_(T_CANCEL), NULL, B_ENTER | B_ESC);
			return;
		}
		ses->search_word = stracpy(ses->last_search_word);
	}
	get_search_data(f->f_data);
	do {
		if (is_in_range(f->f_data, p, f->yw, ses->search_word, &min, &max)) {
			f->vs->view_pos = p;
			if (max >= min) {
				if (max > f->vs->view_posx + f->xw) f->vs->view_posx = max - f->xw;
				if (min < f->vs->view_posx) f->vs->view_posx = min;
			}
			f->vs->orig_view_pos = f->vs->view_pos;
			f->vs->orig_view_posx = f->vs->view_posx;
			set_link(f);
			find_next_link_in_search(f, ses->search_direction * 2);
			/*draw_doc(ses->term, f, 1);
			print_screen_status(ses);
			redraw_from_window(ses->win);*/
			return;
		}
		if ((p += ses->search_direction * f->yw) > f->f_data->y) p = 0;
		if (p < 0) {
			p = 0;
			while (p < f->f_data->y) p += f->yw ? f->yw : 1;
			p -= f->yw;
		}
	} while ((c += f->yw ? f->yw : 1) < f->f_data->y + f->yw);
	/*draw_doc(ses->term, f, 1);
	print_screen_status(ses);
	redraw_from_window(ses->win);*/
	msg_box(ses->term, NULL, TEXT_(T_SEARCH), AL_CENTER, TEXT_(T_SEARCH_STRING_NOT_FOUND), NULL, 1, TEXT_(T_CANCEL), NULL, B_ENTER | B_ESC);
}

void find_next_back(struct session *ses, struct f_data_c *f, int a)
{
	ses->search_direction = - ses->search_direction;
	find_next(ses, f, a);
	ses->search_direction = - ses->search_direction;
}

void rep_ev(struct session *ses, struct f_data_c *fd, void (*f)(struct session *, struct f_data_c *, int), int a)
{
	int i = ses->kbdprefix.rep ? ses->kbdprefix.rep_num : 1;
	while (i--) f(ses, fd, a);
}

struct link *choose_mouse_link(struct f_data_c *f, struct event *ev)
{
	struct link *l1 = f->f_data->links + f->f_data->nlinks;
	struct link *l2 = f->f_data->links;
	struct link *l;
	int i;
	if (!f->f_data->nlinks) return NULL;
	if (ev->x < 0 || ev->y < 0 || ev->x >= f->xw || ev->y >= f->yw) return NULL;
	for (i = f->vs->view_pos; i < f->f_data->y && i < f->vs->view_pos + f->yw; i++) {
		if (f->f_data->lines1[i] && f->f_data->lines1[i] < l1) l1 = f->f_data->lines1[i];
		if (f->f_data->lines2[i] && f->f_data->lines2[i] > l2) l2 = f->f_data->lines2[i];
	}
	for (l = l1; l <= l2; l++) {
		int i;
		for (i = 0; i < l->n; i++) if (l->pos[i].x - f->vs->view_posx == ev->x && l->pos[i].y - f->vs->view_pos == ev->y) return l;
	}
	return NULL;
}

void goto_link_number(struct session *ses, unsigned char *num)
{
	int n = atoi(num);
	struct f_data_c *f = current_frame(ses);
	struct link *link;
	if (!f || !f->vs) return;
	if (n < 0 || n > f->f_data->nlinks) return;
	f->vs->current_link = n - 1;
	f->vs->orig_link = f->vs->current_link;
	link = &f->f_data->links[f->vs->current_link];
	check_vs(f);
	f->vs->orig_view_pos = f->vs->view_pos;
	f->vs->orig_view_posx = f->vs->view_posx;
	if (link->type != L_AREA && link->type != L_FIELD) enter(ses, f, 0);
}

void frm_download(struct session *, struct f_data_c *);
void send_image(struct terminal *term, void *xxx, struct session *ses);
    
int frame_ev(struct session *ses, struct f_data_c *fd, struct event *ev)
{
	int x = 1;
	if (fd->vs->current_link >= 0 && (fd->f_data->links[fd->vs->current_link].type == L_FIELD || fd->f_data->links[fd->vs->current_link].type == L_AREA)) if (field_op(ses, fd, &fd->f_data->links[fd->vs->current_link], ev, 0)) return 1;
	if (ev->ev == EV_KBD && ev->x >= '0'+!ses->kbdprefix.rep && ev->x <= '9' && (!fd->f_data->opt.num_links || ev->y)) {
		if (!ses->kbdprefix.rep) ses->kbdprefix.rep_num = 0;
		if ((ses->kbdprefix.rep_num = ses->kbdprefix.rep_num * 10 + ev->x - '0') > 65536) ses->kbdprefix.rep_num = 65536;
		ses->kbdprefix.rep = 1;
		return 1;
	}
	if (ev->ev == EV_KBD) {
		switch (kbd_action(KM_MAIN, ev)) {
			case ACT_PAGE_DOWN: rep_ev(ses, fd, page_down, 0); break;
			case ACT_PAGE_UP: rep_ev(ses, fd, page_up, 0); break;
			case ACT_DOWN: rep_ev(ses, fd, down, 0); break;
			case ACT_UP: rep_ev(ses, fd, up, 0); break;
			case ACT_COPY_CLIPBOARD: {
				char *current_link = print_current_link(ses);
				if (current_link) {
					set_clipboard_text( current_link );
					mem_free(current_link);
				}
				break;
			}
			case ACT_SCROLL_UP: rep_ev(ses, fd, scroll, -1 - !ses->kbdprefix.rep); break;
			case ACT_SCROLL_DOWN: rep_ev(ses, fd, scroll, 1 + !ses->kbdprefix.rep); break;
			case ACT_SCROLL_LEFT: rep_ev(ses, fd, hscroll, -1 - 7 * !ses->kbdprefix.rep); break;
			case ACT_SCROLL_RIGHT: rep_ev(ses, fd, hscroll, 1 + 7 * !ses->kbdprefix.rep); break;
			case ACT_HOME: rep_ev(ses, fd, home, 0); break;
			case ACT_END:  rep_ev(ses, fd, x_end, 0); break;
			case ACT_ENTER: x = enter(ses, fd, 0); break;
			case ACT_DOWNLOAD: if (!anonymous) frm_download(ses, fd); break;
			case ACT_SEARCH: search_dlg(ses, fd, 0); break;
			case ACT_SEARCH_BACK: search_back_dlg(ses, fd, 0); break;
			case ACT_FIND_NEXT: find_next(ses, fd, 0); break;
			case ACT_FIND_NEXT_BACK: find_next_back(ses, fd, 0); break;
			case ACT_ZOOM_FRAME: set_frame(ses, fd, 0), x = 2; break;
			case ACT_VIEW_IMAGE: send_image(ses->term, NULL, ses); break;
			default:
				if (ev->x >= '1' && ev->x <= '9' && !ev->y) {
					struct f_data *f_data = fd->f_data;
					int nl, lnl;
					unsigned char d[2];
					d[0] = ev->x;
					d[1] = 0;
					nl = f_data->nlinks, lnl = 1;
					while (nl) nl /= 10, lnl++;
					if (lnl > 1) input_field(ses->term, NULL, TEXT_(T_GO_TO_LINK), TEXT_(T_ENTER_LINK_NUMBER), TEXT_(T_OK), TEXT_(T_CANCEL), ses, NULL, lnl, d, 1, f_data->nlinks, check_number, (void (*)(void *, unsigned char *)) goto_link_number, NULL);
				}
				/*else if (ev->x == 'x') {
					struct node *node;
					static int n = -1;
					int i;
					fd->xl = -1234;
					draw_doc(ses->term, fd, 1);
					clear_link(ses->term, fd);
					n++;
					i = n;
					foreachback(node, fd->f_data->nodes) {
						if (!i--) {
							int x, y;
							for (y = 0; y < node->yw; y++) for (x = 0; x < node->xw && x < 1000; x++) {
								int rx = x + node->x + fd->xp - fd->vs->view_posx;
								int ry = y + node->y + fd->yp - fd->vs->view_pos;
								if (rx >= 0 && ry >= 0 && rx < ses->term->x && ry < ses->term->y) {
									set_color(ses->term, rx, ry, 0x3800);
								}
							}
							break;
						}
					}
					if (i >= 0) n = -1;
					x = 0;
				}*/
				else x = 0;
		}
	} else if (ev->ev == EV_MOUSE) {
		struct link *l = choose_mouse_link(fd, ev);
		if (l) {
			x = 1;
			fd->vs->current_link = l - fd->f_data->links;
			fd->vs->orig_link = fd->vs->current_link;
			if (l->type == L_LINK || l->type == L_BUTTON || l->type == L_CHECKBOX || l->type == L_SELECT) if ((ev->b & BM_ACT) == B_UP) {
				draw_doc(ses->term, fd, 1);
				print_screen_status(ses);
				redraw_from_window(ses->win);
				if ((ev->b & BM_BUTT) < B_MIDDLE) x = enter(ses, fd, 0);
				else link_menu(ses->term, NULL, ses);
			}
		}
	} else x = 0;
	ses->kbdprefix.rep = 0;
	return x;
}

struct f_data_c *current_frame(struct session *ses)
{
	struct f_data_c *fd = NULL;
	int i;
	if (ses->history.next == &ses->history) return NULL;
	i = cur_loc(ses)->vs.current_link;
	foreach(fd, ses->scrn_frames) {
		if (fd->f_data && fd->f_data->frame) continue;
		if (!i--) return fd;
	}
	if (!ses->screen || !ses->screen->f_data || ses->screen->f_data->frame) return NULL;
	return ses->screen;
}

int send_to_frame(struct session *ses, struct event *ev)
{
	int r;
	struct f_data_c *fd;
	fd = current_frame(ses);
	if (!fd) {
		/*internal("document not formatted");*/
		return 0;
	}
	r = frame_ev(ses, fd, ev);
	if (r == 1) {
		draw_doc(ses->term, fd, 1);
		print_screen_status(ses);
		redraw_from_window(ses->win);
	}
	return r;
}

void next_frame(struct session *ses, int p)
{
	int n;
	struct view_state *vs;
	struct f_data_c *fd;
	if (list_empty(ses->history) || (ses->screen && ses->screen->f_data && !ses->screen->f_data->frame)) return;
	vs = &cur_loc(ses)->vs;
	n = 0;
	foreach(fd, ses->scrn_frames) if (!(fd->f_data && fd->f_data->frame)) n++;
	vs->current_link += p;
	if (!n) n = 1;
	while (vs->current_link < 0) vs->current_link += n;
	vs->current_link %= n;
}

void do_for_frame(struct session *ses, void (*f)(struct session *, struct f_data_c *, int), int a)
{
	struct f_data_c *fd = current_frame(ses);
	if (!fd) {
		/*internal("document not formatted");*/
		return;
	}
	f(ses, fd, a);
	draw_doc(ses->term, fd, 1);
	print_screen_status(ses);
	redraw_from_window(ses->win);
}

void do_mouse_event(struct session *ses, struct event *ev)
{
	struct event evv;
	struct f_data_c *fdd, *fd = current_frame(ses);	/* !!! FXIME: frames */
	struct document_options *o;
	if (!fd) return;
	o = &fd->f_data->opt;
	if (ev->x >= o->xp && ev->x < o->xp + o->xw &&
	    ev->y >= o->yp && ev->y < o->yp + o->yw) goto ok;
	r:
	next_frame(ses, 1);
	fdd = current_frame(ses);
	o = &fdd->f_data->opt;
	if (ev->x >= o->xp && ev->x < o->xp + o->xw &&
	    ev->y >= o->yp && ev->y < o->yp + o->yw) {
		draw_formatted(ses);
		fd = fdd;
		goto ok;
	}
	if (fdd != fd) goto r;
	return;
	ok:
	memcpy(&evv, ev, sizeof(struct event));
	evv.x -= fd->xp;
	evv.y -= fd->yp;
	send_to_frame(ses, &evv);
}

void send_event(struct session *ses, struct event *ev)
{
	if (ev->ev == EV_KBD) {
		if (send_to_frame(ses, ev)) return;
		switch (kbd_action(KM_MAIN, ev)) {
			case ACT_MENU:
				activate_bfu_technology(ses, -1);
				goto x;
			case ACT_FILE_MENU:
				activate_bfu_technology(ses, 0);
				goto x;
			case ACT_NEXT_FRAME:
				next_frame(ses, 1);
				draw_formatted(ses);
				/*draw_frames(ses);
				  print_screen_status(ses);
				  redraw_from_window(ses->win);*/
				goto x;
			case ACT_PREVIOUS_FRAME:
				next_frame(ses, -1);
				draw_formatted(ses);
				goto x;
			case ACT_BACK:
				back(ses, NULL, 0);
				goto x;
			case ACT_RELOAD:
				reload(ses, -1);
				goto x;
			case ACT_GOTO_URL:
		  		quak:
				dialog_goto_url(ses,"");
				goto x;
			case ACT_GOTO_URL_CURRENT: {
				unsigned char *s;
				if (list_empty(ses->history)) goto quak;
				s = stracpy(cur_loc(ses)->vs.url);
				if (strchr(s, POST_CHAR)) *strchr(s, POST_CHAR) = 0;
				dialog_goto_url(ses, s);
				mem_free(s);
				goto x;
			}
			case ACT_GOTO_URL_CURRENT_LINK: {
				unsigned char url[MAX_STR_LEN];
				if (!get_current_link_url(ses, url, sizeof url)) goto quak;
				dialog_goto_url(ses, url);
				goto x;	
			}
			case ACT_ADD_BOOKMARK:
				if (!anonymous) launch_bm_add_doc_dialog(ses->term, NULL, ses);
				goto x;
			case ACT_BOOKMARK_MANAGER:
				if (!anonymous) menu_bookmark_manager(ses->term, NULL, ses);
				goto x;
			case ACT_REALLYQUIT:
				exit_prog(ses->term, (void *)1, ses);
				goto x;
			case ACT_QUIT:
		  		quit:
				exit_prog(ses->term, (void *)(ev->x == KBD_CTRL_C), ses);
				goto x;
			case ACT_DOCUMENT_INFO:
				state_msg(ses);
				goto x;
			case ACT_HEADER_INFO:
				head_msg(ses);
				goto x;
			case ACT_TOGGLE_DISPLAY_IMAGES:
				ses->ds.images = !ses->ds.images;
				html_interpret(ses);
				draw_formatted(ses);
				goto x;
			case ACT_TOGGLE_DISPLAY_TABLES:
				ses->ds.tables = !ses->ds.tables;
				html_interpret(ses);
				draw_formatted(ses);
				goto x;
			case ACT_TOGGLE_HTML_PLAIN:
				toggle(ses, ses->screen, 0);
				goto x;
			case ACT_OPEN_NEW_WINDOW:
				open_in_new_window(ses->term, send_open_new_xterm, ses);
				goto x;
			case ACT_OPEN_LINK_IN_NEW_WINDOW:
				open_in_new_window(ses->term, send_open_in_new_xterm, ses);
				goto x;
			default:
				if (ev->x == KBD_CTRL_C) goto quit;
				if (ev->y & KBD_ALT) {
					struct window *m;
					ev->y &= ~KBD_ALT;
					activate_bfu_technology(ses, -1);
					m = ses->term->windows.next;
					m->handler(m, ev, 0);
					if (ses->term->windows.next == m) {
						delete_window(m);
					} else goto x;
					ev->y |= KBD_ALT;
				}
		}
	}
	if (ev->ev == EV_MOUSE) {
		if (ev->y == 0 && (ev->b & BM_ACT) == B_DOWN) {
			struct window *m;
			activate_bfu_technology(ses, -1);
			m = ses->term->windows.next;
			m->handler(m, ev, 0);
			goto x;
		}
		do_mouse_event(ses, ev);
	}
	return;
	x:
	ses->kbdprefix.rep = 0;
}

void send_enter(struct terminal *term, void *xxx, struct session *ses)
{
	struct event ev = { EV_KBD, KBD_ENTER, 0, 0 };
	send_event(ses, &ev);
}

void frm_download(struct session *ses, struct f_data_c *fd)
{
	struct link *link;
	if (fd->vs->current_link == -1 || fd->vs->current_link >= fd->f_data->nlinks) return;
	if (ses->dn_url) mem_free(ses->dn_url), ses->dn_url = NULL;
	link = &fd->f_data->links[fd->vs->current_link];
	if (link->type != L_LINK && link->type != L_BUTTON) return;
	if ((ses->dn_url = get_link_url(ses, fd, link))) {
		if (!casecmp(ses->dn_url, "MAP@", 4)) {
			mem_free(ses->dn_url);
			ses->dn_url = NULL;
			return;
		}
		query_file(ses, ses->dn_url, start_download, NULL);
	}
}

void send_download_image(struct terminal *term, void *xxx, struct session *ses)
{
	struct f_data_c *fd = current_frame(ses);
	if (!fd) return;
	if (fd->vs->current_link == -1) return;
	if (ses->dn_url) mem_free(ses->dn_url);
	if ((ses->dn_url = stracpy(fd->f_data->links[fd->vs->current_link].where_img)))
		query_file(ses, ses->dn_url, start_download, NULL);
}

void send_download(struct terminal *term, void *xxx, struct session *ses)
{
	struct f_data_c *fd = current_frame(ses);
	if (!fd) return;
	if (fd->vs->current_link == -1) return;
	if (ses->dn_url) mem_free(ses->dn_url);
	if ((ses->dn_url = get_link_url(ses, fd, &fd->f_data->links[fd->vs->current_link])))
		query_file(ses, ses->dn_url, start_download, NULL);
}

/* open a link in a new xterm */
void send_open_in_new_xterm(struct terminal *term, void (*open_window)(struct terminal *term, unsigned char *, unsigned char *), struct session *ses)
{
        struct f_data_c *fd = current_frame(ses);
        if (!fd) return;
        if (fd->vs->current_link == -1) return;
        if (ses->dn_url) mem_free(ses->dn_url);
        if ((ses->dn_url = get_link_url(ses, fd, &fd->f_data->links[fd->vs->current_link]))) {
		unsigned char *enc_url = encode_url(ses->dn_url);
		unsigned char *path = escape_path(path_to_exe);
		open_window(term, path, enc_url);
		mem_free(enc_url);
		mem_free(path);
	}
}

void send_open_new_xterm(struct terminal *term, void (*open_window)(struct terminal *, unsigned char *, unsigned char *), struct session *ses)
{
	int l;
	unsigned char *path;
        if (ses->dn_url) mem_free(ses->dn_url);
	ses->dn_url = init_str();
	l = 0;
        add_to_str(&ses->dn_url, &l, "-base-session ");
	add_num_to_str(&ses->dn_url, &l, ses->id);
	path = escape_path(path_to_exe);
	open_window(term, path, ses->dn_url);
	mem_free(path);
}

void open_in_new_window(struct terminal *term, void (*xxx)(struct terminal *, void (*)(struct terminal *, unsigned char *, unsigned char *), struct session *ses), struct session *ses)
{
	struct menu_item *mi;
	struct open_in_new *oin, *oi;
	if (!(oin = get_open_in_new(term->environment))) return;
	if (!oin[1].text) {
		xxx(term, oin[0].fn, ses);
		mem_free(oin);
		return;
	}
	if (!(mi = new_menu(1))) {
		mem_free(oin);
		return;
	}
	for (oi = oin; oi->text; oi++) add_to_menu(&mi, oi->text, "", oi->hk, MENU_FUNC xxx, oi->fn, 0);
	mem_free(oin);
	do_menu(term, mi, ses);
}

int can_open_in_new(struct terminal *term)
{
	struct open_in_new *oin = get_open_in_new(term->environment);
	if (!oin) return 0;
	if (!oin[1].text) {
		mem_free(oin);
		return 1;
	}
	mem_free(oin);
	return 2;
}

void save_url(struct session *ses, unsigned char *url)
{
	unsigned char *u;
	if (!(u = translate_url(url, ses->term->cwd))) {
		struct status stat = { NULL, NULL, NULL, NULL, S_BAD_URL, PRI_CANCEL, 0, NULL, NULL, NULL };
		print_error_dialog(ses, &stat, TEXT_(T_ERROR));
		return;
	}
	if (ses->dn_url) mem_free(ses->dn_url);
	ses->dn_url = u;
	query_file(ses, ses->dn_url, start_download, NULL);
}

void send_image(struct terminal *term, void *xxx, struct session *ses)
{
	unsigned char *u;
	struct f_data_c *fd = current_frame(ses);
	if (!fd) return;
	if (fd->vs->current_link == -1) return;
	if (!(u = fd->f_data->links[fd->vs->current_link].where_img)) return;
	goto_url(ses, u);
}

void save_as(struct terminal *term, void *xxx, struct session *ses)
{
	struct location *l;
	if (list_empty(ses->history)) return;
	l = cur_loc(ses);
	if (ses->dn_url) mem_free(ses->dn_url);
	if ((ses->dn_url = stracpy(l->vs.url)))
		query_file(ses, ses->dn_url, start_download, NULL);
}

void save_formatted(struct session *ses, unsigned char *file)
{
	int h;
	struct f_data_c *f;
	if (!(f = current_frame(ses)) || !f->f_data) return;
	if ((h = create_download_file(ses->term, file, 0)) < 0) return;
	if (dump_to_file(f->f_data, h)) msg_box(ses->term, NULL, TEXT_(T_SAVE_ERROR), AL_CENTER, TEXT_(T_ERROR_WRITING_TO_FILE), NULL, 1, TEXT_(T_CANCEL), NULL, B_ENTER | B_ESC);
	close(h);
}

void menu_save_formatted(struct terminal *term, void *xxx, struct session *ses)
{
	struct f_data_c *f;
	if (!(f = current_frame(ses)) || !f->f_data) return;
	query_file(ses, f->vs->url, save_formatted, NULL);
}

void link_menu(struct terminal *term, void *xxx, struct session *ses)
{
	struct f_data_c *f = current_frame(ses);
	struct link *link;
	struct menu_item *mi;
	int l = 0;
	if (!(mi = new_menu(1))) return;
	if (!f) goto x;
	if (f->vs->current_link == -1) goto no_l;
	link = &f->f_data->links[f->vs->current_link];
	if (link->type == L_LINK && link->where) {
		l = 1;
		if (strlen(link->where) >= 4 && !casecmp(link->where, "MAP@", 4)) add_to_menu(&mi, TEXT_(T_DISPLAY_USEMAP), ">", TEXT_(T_HK_DISPLAY_USEMAP), MENU_FUNC send_enter, NULL, 1);
		else {
			int c = can_open_in_new(term);
			add_to_menu(&mi, TEXT_(T_FOLLOW_LINK), "", TEXT_(T_HK_FOLLOW_LINK), MENU_FUNC send_enter, NULL, 0);
			if (c) add_to_menu(&mi, TEXT_(T_OPEN_IN_NEW_WINDOW), c - 1 ? ">" : "", TEXT_(T_HK_OPEN_IN_NEW_WINDOW), MENU_FUNC open_in_new_window, send_open_in_new_xterm, c - 1);
			if (!anonymous) add_to_menu(&mi, TEXT_(T_DOWNLOAD_LINK), "d", TEXT_(T_HK_DOWNLOAD_LINK), MENU_FUNC send_download, NULL, 0);
			/*add_to_menu(&mi, TEXT_(T_ADD_BOOKMARK), "A", TEXT_(T_HK_ADD_BOOKMARK), MENU_FUNC launch_bm_add_link_dialog, NULL, 0);*/

		}
	}
	if (link->type == L_BUTTON && link->form) {
		l = 1;
		if (link->form->type == FC_RESET) add_to_menu(&mi, TEXT_(T_RESET_FORM), "", TEXT_(T_HK_RESET_FORM), MENU_FUNC send_enter, NULL, 0);
		else if (link->form->type == FC_SUBMIT || link->form->type == FC_IMAGE) {
			int c = can_open_in_new(term);
			add_to_menu(&mi, TEXT_(T_SUBMIT_FORM), "", TEXT_(T_HK_SUBMIT_FORM), MENU_FUNC send_enter, NULL, 0);
			if (c && link->form->method == FM_GET) add_to_menu(&mi, TEXT_(T_SUBMIT_FORM_AND_OPEN_IN_NEW_WINDOW), c - 1 ? ">" : "", TEXT_(T_HK_SUBMIT_FORM_AND_OPEN_IN_NEW_WINDOW), MENU_FUNC open_in_new_window, send_open_in_new_xterm, c - 1);
			if (!anonymous) add_to_menu(&mi, TEXT_(T_SUBMIT_FORM_AND_DOWNLOAD), "d", TEXT_(T_HK_SUBMIT_FORM_AND_DOWNLOAD), MENU_FUNC send_download, NULL, 0);
		}
	}
	if (link->where_img) {
		l = 1;
		add_to_menu(&mi, TEXT_(T_VIEW_IMAGE), "", TEXT_(T_HK_VIEW_IMAGE), MENU_FUNC send_image, NULL, 0);
		if (!anonymous) add_to_menu(&mi, TEXT_(T_DOWNLOAD_IMAGE), "", TEXT_(T_HK_DOWNLOAD_IMAGE), MENU_FUNC send_download_image, NULL, 0);
	}
	x:
	if (!l) {
		no_l:
		add_to_menu(&mi, TEXT_(T_NO_LINK_SELECTED), "", M_BAR, NULL, NULL, 0);
	}
	do_menu(term, mi, ses);
}

unsigned char *print_current_titlex(struct f_data_c *fd, int w)
{
	int ml = 0, pl = 0;
	unsigned char *m, *p;
	if (!fd) return NULL;
	w -= 1;
	p = init_str();
	if (fd->yw < fd->f_data->y) {
		int pp, pe;
		if (fd->yw) {
			pp = (fd->vs->view_pos + fd->yw / 2) / fd->yw + 1;
			pe = (fd->f_data->y + fd->yw - 1) / fd->yw;
		} else pp = pe = 1;
		if (pp > pe) pp = pe;
		if (fd->vs->view_pos + fd->yw >= fd->f_data->y) pp = pe;
		if (fd->f_data->title) add_chr_to_str(&p, &pl, ' ');
		add_to_str(&p, &pl, "(p");
		add_num_to_str(&p, &pl, pp);
		add_to_str(&p, &pl, " of ");
		add_num_to_str(&p, &pl, pe);
		add_chr_to_str(&p, &pl, ')');
	}
	if (!fd->f_data->title) return p;
	m = init_str();
	add_to_str(&m, &ml, fd->f_data->title);
	if (ml + pl > w) if ((ml = w - pl) < 0) ml = 0;
	add_to_str(&m, &ml, p);
	mem_free(p);
	return m;
}

unsigned char *print_current_linkx(struct f_data_c *fd, struct terminal *term)
{
	int ll = 0;
	struct link *l;
	unsigned char *m = NULL /* shut up warning */;
	if (!fd) return NULL;
	if (fd->vs->current_link == -1 || fd->vs->current_link >= fd->f_data->nlinks || fd->f_data->frame) return NULL;
	l = &fd->f_data->links[fd->vs->current_link];
	if (l->type == L_LINK) {
		if (!l->where && l->where_img) {
			m = init_str();
			ll = 0;
			add_to_str(&m, &ll, _(TEXT_(T_IMAGE), term));
			add_to_str(&m, &ll, " ");
			add_to_str(&m, &ll, l->where_img);
			goto p;
		}
		if (strlen(l->where) >= 4 && !casecmp(l->where, "MAP@", 4)) {
			m = init_str();
			ll = 0;
			add_to_str(&m, &ll, _(TEXT_(T_USEMAP), term));
			add_to_str(&m, &ll, " ");
			add_to_str(&m, &ll, l->where + 4);
			goto p;
		}
		m = stracpy(l->where);
		goto p;
	}
	if (!l->form) return NULL;
	if (l->type == L_BUTTON) {
		if (l->form->type == FC_RESET) {
			m = stracpy(_(TEXT_(T_RESET_FORM), term));
			goto p;
		}
		if (!l->form->action) return NULL;
		m = init_str();
		ll = 0;
		if (l->form->method == FM_GET) add_to_str(&m, &ll, _(TEXT_(T_SUBMIT_FORM_TO), term));
		else add_to_str(&m, &ll, _(TEXT_(T_POST_FORM_TO), term));
		add_to_str(&m, &ll, " ");
		add_to_str(&m, &ll, l->form->action);
		goto p;
	}
	if (l->type == L_CHECKBOX || l->type == L_SELECT || l->type == L_FIELD || l->type == L_AREA) {
		m = init_str();
		ll = 0;
		if (l->form->type == FC_RADIO) add_to_str(&m, &ll, _(TEXT_(T_RADIO_BUTTON), term));
		else if (l->form->type == FC_CHECKBOX) add_to_str(&m, &ll, _(TEXT_(T_CHECKBOX), term));
		else if (l->form->type == FC_SELECT) add_to_str(&m, &ll, _(TEXT_(T_SELECT_FIELD), term));
		else if (l->form->type == FC_TEXT) add_to_str(&m, &ll, _(TEXT_(T_TEXT_FIELD), term));
		else if (l->form->type == FC_TEXTAREA) add_to_str(&m, &ll, _(TEXT_(T_TEXT_AREA), term));
		else if (l->form->type == FC_FILE) add_to_str(&m, &ll, _(TEXT_(T_FILE_UPLOAD), term));
		else if (l->form->type == FC_PASSWORD) add_to_str(&m, &ll, _(TEXT_(T_PASSWORD_FIELD), term));
		else {
			mem_free(m);
			return NULL;
		}
		if (l->form->name && l->form->name[0]) add_to_str(&m, &ll, ", "), add_to_str(&m, &ll, _(TEXT_(T_NAME), term)), add_to_str(&m, &ll, " "), add_to_str(&m, &ll, l->form->name);
		if ((l->form->type == FC_CHECKBOX || l->form->type == FC_RADIO) && l->form->default_value && l->form->default_value[0]) add_to_str(&m, &ll, ", "), add_to_str(&m, &ll, _(TEXT_(T_VALUE), term)), add_to_str(&m, &ll, " "), add_to_str(&m, &ll, l->form->default_value);
		if (l->type == L_FIELD && !has_form_submit(fd->f_data, l->form) && l->form->action) {
			add_to_str(&m, &ll, ", ");
			add_to_str(&m, &ll, _(TEXT_(T_HIT_ENTER_TO), term));
			add_to_str(&m, &ll, " ");
			if (l->form->method == FM_GET) add_to_str(&m, &ll, _(TEXT_(T_SUBMIT_TO), term));
			else add_to_str(&m, &ll, _(TEXT_(T_POST_TO), term));
			add_to_str(&m, &ll, " ");
			add_to_str(&m, &ll, l->form->action);
		}
		goto p;
	}
	p:
	return m;
}

unsigned char *print_current_link(struct session *ses)
{
	return print_current_linkx(current_frame(ses), ses->term);
}

unsigned char *print_current_title(struct session *ses)
{
	return print_current_titlex(current_frame(ses), ses->term->x);
}

void loc_msg(struct terminal *term, struct location *lo, struct f_data_c *frame)
{
	struct cache_entry *ce;
	unsigned char *s;
	int l = 0;
	unsigned char *a;
	if (!lo || !frame || !frame->f_data) {
		msg_box(term, NULL, TEXT_(T_INFO), AL_LEFT, TEXT_(T_YOU_ARE_NOWHERE), NULL, 1, TEXT_(T_OK), NULL, B_ENTER | B_ESC);
		return;
	}
	s = init_str();
	add_to_str(&s, &l, _(TEXT_(T_URL), term));
	add_to_str(&s, &l, ": ");
	if (strchr(lo->vs.url, POST_CHAR)) add_bytes_to_str(&s, &l, lo->vs.url, (unsigned char *)strchr(lo->vs.url, POST_CHAR) - (unsigned char *)lo->vs.url);
	else add_to_str(&s, &l, lo->vs.url);
	if (!get_cache_entry(lo->vs.url, &ce)) {
		ce->refcount--;
		add_to_str(&s, &l, "\n");
		add_to_str(&s, &l, _(TEXT_(T_SIZE), term));
		add_to_str(&s, &l, ": ");
		add_num_to_str(&s, &l, ce->length);
		if (ce->incomplete) {
			add_to_str(&s, &l, " (");
			add_to_str(&s, &l, _(TEXT_(T_INCOMPLETE), term));
			add_to_str(&s, &l, ")");
		}
		add_to_str(&s, &l, "\n");
		add_to_str(&s, &l, _(TEXT_(T_CODEPAGE), term));
		add_to_str(&s, &l, ": ");
		add_to_str(&s, &l, get_cp_name(frame->f_data->cp));
		if (frame->f_data->ass == 1) add_to_str(&s, &l, " ("), add_to_str(&s, &l, _(TEXT_(T_ASSUMED), term)), add_to_str(&s, &l, ")");
		if (frame->f_data->ass == 2) add_to_str(&s, &l, " ("), add_to_str(&s, &l, _(TEXT_(T_IGNORING_SERVER_SETTING), term)), add_to_str(&s, &l, ")");
		if ((a = parse_http_header(ce->head, "Server", NULL))) {
			add_to_str(&s, &l, "\n");
			add_to_str(&s, &l, _(TEXT_(T_SERVER), term));
			add_to_str(&s, &l, ": ");
			add_to_str(&s, &l, a);
			mem_free(a);
		}
		if ((a = parse_http_header(ce->head, "Date", NULL))) {
			add_to_str(&s, &l, "\n");
			add_to_str(&s, &l, _(TEXT_(T_DATE), term));
			add_to_str(&s, &l, ": ");
			add_to_str(&s, &l, a);
			mem_free(a);
		}
		if (ce->last_modified) {
			add_to_str(&s, &l, "\n");
			add_to_str(&s, &l, _(TEXT_(T_LAST_MODIFIED), term));
			add_to_str(&s, &l, ": ");
			add_to_str(&s, &l, ce->last_modified);
		}
#ifdef HAVE_SSL
		if (ce->ssl_info) {
			add_to_str(&s, &l, "\n");
			add_to_str(&s, &l, "SSL cipher: ");
			add_to_str(&s, &l, ce->ssl_info);
		}
#endif
	}
	if ((a = print_current_linkx(frame, term))) {
		add_to_str(&s, &l, "\n\n");
		add_to_str(&s, &l, _(TEXT_(T_LINK), term));
		add_to_str(&s, &l, ": ");
		add_to_str(&s, &l, a);
		mem_free(a);
	}
	msg_box(term, getml(s, NULL), TEXT_(T_INFO), AL_LEFT, s, NULL, 1, TEXT_(T_OK), NULL, B_ENTER | B_ESC);
}

void state_msg(struct session *ses)
{
	if (list_empty(ses->history)) loc_msg(ses->term, NULL, NULL);
	else loc_msg(ses->term, cur_loc(ses), current_frame(ses));
}

void head_msg(struct session *ses)
{
	struct cache_entry *ce;
	unsigned char *s, *ss;
	int len;
	if (list_empty(ses->history)) {
		msg_box(ses->term, NULL, TEXT_(T_HEADER_INFO), AL_LEFT, TEXT_(T_YOU_ARE_NOWHERE), NULL, 1, TEXT_(T_OK), NULL, B_ENTER | B_ESC);
		return;
	}
	if (!find_in_cache(cur_loc(ses)->vs.url, &ce)) {
		ce->refcount--;
		if (ce->head) ss = s = stracpy(ce->head);
		else s = ss = stracpy("");
		len = strlen(s) - 1;
		if (len > 0) {
			while ((ss = strstr(s, "\r\n"))) memmove(ss, ss + 1, strlen(ss));
			while (*s && s[strlen(s) - 1] == '\n') s[strlen(s) - 1] = 0;
		}
		msg_box(ses->term, getml(s, NULL), TEXT_(T_HEADER_INFO), AL_LEFT, s, NULL, 1, TEXT_(T_OK), NULL, B_ENTER | B_ESC);
	}
}
