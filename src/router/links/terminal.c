#include "links.h"

int hard_write(int fd, unsigned char *p, int l)
{
	int w = 1;
	int t = 0;
	while (l > 0 && w) {
		if ((w = write(fd, p, l)) < 0) {
			if (errno == EINTR) continue;
			return -1;
		}
		t += w;
		p += w;
		l -= w;
	}
	return t;
}

int hard_read(int fd, unsigned char *p, int l)
{
	int r = 1;
	int t = 0;
	while (l > 0 && r) {
		if ((r = read(fd, p, l)) < 0) {
			if (errno == EINTR) continue;
			return -1;
		}
		/*{int ww;for(ww=0;ww<r;ww++)fprintf(stderr," %02x",(int)p[ww]);fflush(stderr);}*/
		t += r;
		p += r;
		l -= r;
	}
	return t;
}

unsigned char *get_cwd()
{
	int bufsize = 128;
	unsigned char *buf;
	while (1) {
		buf = mem_alloc(bufsize);
		if (getcwd(buf, bufsize)) return buf;
		mem_free(buf);
		if (errno == EINTR) continue;
		if (errno != ERANGE) return NULL;
		if ((unsigned)bufsize > MAXINT - 128) overalloc();
		bufsize += 128;
	}
	return NULL;
}

void set_cwd(unsigned char *path)
{
	if (path) while (chdir(path) && errno == EINTR) ;
}

struct list_head terminals = {&terminals, &terminals};

void alloc_term_screen(struct terminal *term, int x, int y)
{
	unsigned *s, *t;
	if (x && (unsigned)x * (unsigned)y / (unsigned)x != (unsigned)y) overalloc();
	if ((unsigned)x * (unsigned)y > MAXINT / sizeof(unsigned)) overalloc();
	s = mem_realloc(term->screen, x * y * sizeof(unsigned));
	t = mem_realloc(term->last_screen, x * y * sizeof(unsigned));
	memset(t, -1, x * y * sizeof(unsigned));
	term->x = x;
	term->y = y;
	term->last_screen = t;
	memset(s, 0, x * y * sizeof(unsigned));
	term->screen = s;
	term->dirty = 1;
}

void in_term(struct terminal *);
void destroy_terminal(struct terminal *);
void check_if_no_terminal();

void clear_terminal(struct terminal *term)
{
	fill_area(term, 0, 0, term->x, term->y, ' ');
	set_cursor(term, 0, 0, 0, 0);
}

void redraw_terminal_ev(struct terminal *term, int e)
{
	struct window *win;
	struct event ev = {0, 0, 0, 0};
	ev.ev = e;
	ev.x = term->x;
	ev.y = term->y;
	clear_terminal(term);
	term->redrawing = 2;
	foreachback(win, term->windows) win->handler(win, &ev, 0);
	term->redrawing = 0;
}

void redraw_terminal(struct terminal *term)
{
	redraw_terminal_ev(term, EV_REDRAW);
}

void redraw_terminal_all(struct terminal *term)
{
	redraw_terminal_ev(term, EV_RESIZE);
}

void erase_screen(struct terminal *term)
{
	if (!term->master || !is_blocked()) {
		if (term->master) want_draw();
		hard_write(term->fdout, "\033[2J\033[1;1H", 10);
		if (term->master) done_draw();
	}
}

void redraw_terminal_cls(struct terminal *term)
{
	erase_screen(term);
	alloc_term_screen(term, term->x, term->y);
	redraw_terminal_all(term);
}

void cls_redraw_all_terminals()
{
	struct terminal *term;
	foreach(term, terminals) redraw_terminal_cls(term);
}

void redraw_from_window(struct window *win)
{
	struct terminal *term = win->term;
	struct window *end = (void *)&term->windows;
	struct event ev = {EV_REDRAW, 0, 0, 0};
	ev.x = term->x;
	ev.y = term->y;
	if (term->redrawing) return;
	term->redrawing = 1;
	for (win = win->prev; win != end; win = win->prev) {
		win->handler(win, &ev, 0);
	}
	term->redrawing = 0;
}

void redraw_below_window(struct window *win)
{
	int tr;
	struct terminal *term = win->term;
	struct window *end = win;
	struct event ev = {EV_REDRAW, 0, 0, 0};
	ev.x = term->x;
	ev.y = term->y;
	if (term->redrawing >= 2) return;
	tr = term->redrawing;
	win->term->redrawing = 2;
	for (win = term->windows.prev; win != end; win = win->prev) {
		win->handler(win, &ev, 0);
	}
	term->redrawing = tr;
}

void add_window_at_pos(struct terminal *term, void (*handler)(struct window *, struct event *, int), void *data, struct window *at)
{
	struct event ev = {EV_INIT, 0, 0, 0};
	struct window *win;
	ev.x = term->x;
	ev.y = term->y;
	win = mem_alloc(sizeof (struct window));
	win->handler = handler;
	win->data = data;
	win->term = term;
	win->xp = win->yp = 0;
	add_at_pos(at, win);
	win->handler(win, &ev, 0);
}

void add_window(struct terminal *term, void (*handler)(struct window *, struct event *, int), void *data)
{
	add_window_at_pos(term, handler, data, (struct window *)(void *)&term->windows);
}

void delete_window(struct window *win)
{
	struct event ev = {EV_ABORT, 0, 0, 0};
	win->handler(win, &ev, 1);
	del_from_list(win);
	if (win->data) mem_free(win->data);
	redraw_terminal(win->term);
	mem_free(win);
}

void delete_window_ev(struct window *win, struct event *ev)
{
	struct window *w = win->next;
	if ((void *)w == &win->term->windows) w = NULL;
	delete_window(win);
	if (ev && w && w->next != w) w->handler(w, ev, 1);
}

void set_window_ptr(struct window *win, int x, int y)
{
	win->xp = x;
	win->yp = y;
}

void get_parent_ptr(struct window *win, int *x, int *y)
{
	if ((void *)win->next != &win->term->windows) {
		*x = win->next->xp;
		*y = win->next->yp;
	} else {
		*x = *y = 0;
	}
}

struct window *get_root_window(struct terminal *term)
{
	if (list_empty(term->windows)) {
		internal("terminal has no windows");
		return NULL;
	}
	return (struct window *)term->windows.prev;
}

struct ewd {
	void (*fn)(void *);
	void *data;
	int b;
};

void empty_window_handler(struct window *win, struct event *ev, int fwd)
{
	struct window *n;
	struct ewd *ewd = win->data;
	int x, y;
	void (*fn)(void *) = ewd->fn;
	void *data = ewd->data;
	if (ewd->b) return;
	switch ((int)ev->ev) {
		case EV_INIT:
		case EV_RESIZE:
		case EV_REDRAW:
			get_parent_ptr(win, &x, &y);
			set_window_ptr(win, x, y);
			return;
		case EV_ABORT:
			fn(data);
			return;
	}
	ewd->b = 1;
	n = win->next;
	delete_window(win);
	fn(data);
	if (n->next != n) n->handler(n, ev, fwd);
}

void add_empty_window(struct terminal *term, void (*fn)(void *), void *data)
{
	struct ewd *ewd;
	ewd = mem_alloc(sizeof(struct ewd));
	ewd->fn = fn;
	ewd->data = data;
	ewd->b = 0;
	add_window(term, empty_window_handler, ewd);
}

void free_term_specs()
{
	free_list(term_specs);
}

struct list_head term_specs = {&term_specs, &term_specs};

struct term_spec dumb_term = { NULL, NULL, "", 0, 1, 0, 0, 0, 0 };

struct term_spec *get_term_spec(unsigned char *term)
{
	struct term_spec *t;
	foreach(t, term_specs) if (!strcasecmp(t->term, term)) return t;
	return &dumb_term;
}

struct term_spec *new_term_spec(unsigned char *term)
{
	struct term_spec *t;
	foreach(t, term_specs) if (!strcasecmp(t->term, term)) return t;
	t = mem_alloc(sizeof(struct term_spec));
	memcpy(t, &dumb_term, sizeof(struct term_spec));
	if (strlen(term) < MAX_TERM_LEN) strcpy(t->term, term);
	else memcpy(t->term, term, MAX_TERM_LEN - 1), t->term[MAX_TERM_LEN - 1] = 0;
	add_to_list(term_specs, t);
	sync_term_specs();
	return t;
}

void sync_term_specs()
{
	struct terminal *term;
	foreach (term, terminals) term->spec = get_term_spec(term->term);
}

struct terminal *init_term(int fdin, int fdout, void (*root_window)(struct window *, struct event *, int))
{
	struct terminal *term;
	struct window *win;
	term = mem_alloc(sizeof (struct terminal));
	memset(term, 0, sizeof(struct terminal));
	term->fdin = fdin;
	term->fdout = fdout;
	term->master = term->fdout == get_output_handle();
	/*term->x = 0;
	term->y = 0;
	term->cx = 0;
	term->cy = 0;*/
	term->lcx = -1;
	term->lcy = -1;
	term->dirty = 1;
	term->redrawing = 0;
	term->blocked = -1;
	term->screen = DUMMY;
	term->last_screen = DUMMY;
	term->spec = &dumb_term;
	term->term[0] = 0;
	term->cwd[0] = 0;
	term->input_queue = DUMMY;
	term->qlen = 0;
	init_list(term->windows);
	win = mem_alloc(sizeof (struct window));
	win->handler = root_window;
	win->data = NULL;
	win->term = term;
	add_to_list(term->windows, win);
	/*alloc_term_screen(term, 80, 25);*/
	add_to_list(terminals, term);
	set_handlers(fdin, (void (*)(void *))in_term, NULL, (void (*)(void *))destroy_terminal, term);
	return term;
}

void in_term(struct terminal *term)
{
	struct event *ev;
	int r;
	unsigned char *iq;
	if ((unsigned)term->qlen + ALLOC_GR > MAXINT) overalloc();
	iq = mem_realloc(term->input_queue, term->qlen + ALLOC_GR);
	term->input_queue = iq;
	if ((r = read(term->fdin, iq + term->qlen, ALLOC_GR)) <= 0) {
		if (r == -1 && errno != ECONNRESET) error("ERROR: error %d on terminal: could not read event", errno);
		destroy_terminal(term);
		return;
	}
	term->qlen += r;
	test_queue:
	if ((size_t)term->qlen < sizeof(struct event)) return;
	ev = (struct event *)iq;
	r = sizeof(struct event);
	if (ev->ev != EV_INIT && ev->ev != EV_RESIZE && ev->ev != EV_REDRAW && ev->ev != EV_KBD && ev->ev != EV_MOUSE && ev->ev != EV_ABORT) {
		error("ERROR: error on terminal: bad event %d", ev->ev);
		goto mm;
	}
	if (ev->ev == EV_INIT) {
		int init_len;
		if ((size_t)term->qlen < sizeof(struct event) + MAX_TERM_LEN + MAX_CWD_LEN + 2 * sizeof(int)) return;
		init_len = *(int *)(iq + sizeof(struct event) + MAX_TERM_LEN + MAX_CWD_LEN + sizeof(int));
		if ((size_t)term->qlen < sizeof(struct event) + MAX_TERM_LEN + MAX_CWD_LEN + 2 * sizeof(int) + init_len) return;
		memcpy(term->term, iq + sizeof(struct event), MAX_TERM_LEN);
		term->term[MAX_TERM_LEN - 1] = 0;
		memcpy(term->cwd, iq + sizeof(struct event) + MAX_TERM_LEN, MAX_CWD_LEN);
		term->cwd[MAX_CWD_LEN - 1] = 0;
		term->environment = *(int *)(iq + sizeof(struct event) + MAX_TERM_LEN + MAX_CWD_LEN);
		ev->b = (long)(iq + sizeof(struct event) + MAX_TERM_LEN + MAX_CWD_LEN + sizeof(int));
		r = sizeof(struct event) + MAX_TERM_LEN + MAX_CWD_LEN + 2 * sizeof(int) + init_len;
		sync_term_specs();
	}
	if (ev->ev == EV_REDRAW || ev->ev == EV_RESIZE || ev->ev == EV_INIT) {
		struct window *win;
		send_redraw:
		if (ev->x < 0 || ev->y < 0) {
			error("ERROR: bad terminal size: %d, %d", (int)ev->x, (int)ev->y);
			goto mm;
		}
		alloc_term_screen(term, ev->x, ev->y);
		clear_terminal(term);
		erase_screen(term);
		term->redrawing = 1;
		foreachback(win, term->windows) win->handler(win, ev, 0);
		term->redrawing = 0;
	}
	if (ev->ev == EV_KBD || ev->ev == EV_MOUSE) {
		if (ev->ev == EV_KBD && upcase(ev->x) == 'L' && ev->y == KBD_CTRL) {
			ev->ev = EV_REDRAW;
			ev->x = term->x;
			ev->y = term->y;
			goto send_redraw;
		}
		else if (ev->ev == EV_KBD && ev->x == KBD_CTRL_C) ((struct window *)term->windows.prev)->handler(term->windows.prev, ev, 0);
		else ((struct window *)term->windows.next)->handler(term->windows.next, ev, 0);
	}
	if (ev->ev == EV_ABORT) {
		destroy_terminal(term);
		return;
	}
	/*redraw_screen(term);*/
	mm:
	if (term->qlen == r) term->qlen = 0;
	else memmove(iq, iq + r, term->qlen -= r);
	goto test_queue;
}

static inline int getcompcode(int c)
{
	return (c<<1 | (c&4)>>2) & 7;
}

unsigned char frame_dumb[48] =	"   ||||++||++++++--|-+||++--|-+----++++++++     ";
unsigned char frame_vt100[48] =	"aaaxuuukkuxkjjjkmvwtqnttmlvwtqnvvwwmmllnnjla    ";
unsigned char frame_koi[48] = {
	144,145,146,129,135,178,180,167,
	166,181,161,168,174,173,172,131,
	132,137,136,134,128,138,175,176,
	171,165,187,184,177,160,190,185,
	186,182,183,170,169,162,164,189,
	188,133,130,141,140,142,143,139,
};
unsigned char frame_freebsd[48] = {
	130,138,128,153,150,150,150,140,
	140,150,153,140,139,139,139,140,
	142,151,152,149,146,143,149,149,
	142,141,151,152,149,146,143,151,
	151,152,152,142,142,141,141,143,
	143,139,141,128,128,128,128,128,
};
unsigned char frame_restrict[48] = {
	0, 0, 0, 0, 0, 179, 186, 186,
	205, 0, 0, 0, 0, 186, 205, 0,
	0, 0, 0, 0, 0, 0, 179, 186,
	0, 0, 0, 0, 0, 0, 0, 205,
	196, 205, 196, 186, 205, 205, 186, 186,
	179, 0, 0, 0, 0, 0, 0, 0,
};

#define PRINT_CHAR(p)									\
{											\
	unsigned ch = term->screen[p];							\
	unsigned char c = ch & 0xff;							\
	unsigned char A = ch >> 8 & 0x7f;						\
	if (s->mode == TERM_LINUX) {							\
		if (s->m11_hack) {							\
			if ((int)(ch >> 15) != mode) {					\
				if (!(mode = ch >> 15)) add_to_str(&a, &l, "\033[10m");	\
				else add_to_str(&a, &l, "\033[11m");			\
			}								\
		}									\
		if (s->restrict_852 && (ch >> 15) && c >= 176 && c < 224) {		\
			if (frame_restrict[c - 176]) c = frame_restrict[c - 176];	\
		}									\
	} else if (s->mode == TERM_VT100) {						\
		if ((int)(ch >> 15) != mode) {						\
			if (!(mode = ch >> 15)) add_to_str(&a, &l, "\x0f");		\
			else add_to_str(&a, &l, "\x0e");				\
		}									\
		if (mode && c >= 176 && c < 224) c = frame_vt100[c - 176];		\
	} else if (s->mode == TERM_KOI8 && (ch >> 15) && c >= 176 && c < 224) { c = frame_koi[c - 176];\
	} else if (s->mode == TERM_FREEBSD && (ch >> 15) && c >= 176 && c < 224) { c = frame_freebsd[c - 176];\
	} else if (s->mode == TERM_DUMB && (ch >> 15) && c >= 176 && c < 224) c = frame_dumb[c - 176];\
	if (!(A & 0100) && (A >> 3) == (A & 7)) A = (A & 070) | 7 * !(A & 020);		\
	if (A != attrib) {								\
		attrib = A;								\
		add_to_str(&a, &l, "\033[0");						\
		if (s->col) {								\
			unsigned char m[4];						\
			m[0] = ';'; m[1] = '3'; m[3] = 0;				\
			m[2] = (attrib & 7) + '0';					\
			add_to_str(&a, &l, m);						\
			m[1] = '4';							\
			m[2] = (attrib >> 3 & 7) + '0';					\
			add_to_str(&a, &l, m);						\
		} else if (getcompcode(attrib & 7) < getcompcode(attrib >> 3 & 7))	\
			add_to_str(&a, &l, ";7");					\
		if (attrib & 0100) add_to_str(&a, &l, ";1");				\
		add_to_str(&a, &l, "m");						\
	}										\
	if (c >= ' ' && c != 127/* && c != 155*/) add_chr_to_str(&a, &l, c);		\
	else if (!c || c == 1) add_chr_to_str(&a, &l, ' ');				\
	else add_chr_to_str(&a, &l, '.');						\
	cx++;										\
}											\

void redraw_all_terminals()
{
	struct terminal *term;
	foreach(term, terminals) redraw_screen(term);
}

void redraw_screen(struct terminal *term)
{
	int x, y, p = 0;
	int cx = term->lcx, cy = term->lcy;
	unsigned char *a;
	int attrib = -1;
	int mode = -1;
	int l = 0;
	struct term_spec *s;
	if (!term->dirty || (term->master && is_blocked())) return;
	a = init_str();
	s = term->spec;
	for (y = 0; y < term->y; y++)
		for (x = 0; x < term->x; x++, p++) {
			if (y == term->y - 1 && x == term->x - 1) break;
			if (term->screen[p] == term->last_screen[p]) continue;
			if ((term->screen[p] & 0x3800) == (term->last_screen[p] & 0x3800) && ((term->screen[p] & 0xff) == 0 || (term->screen[p] & 0xff) == 1 || (term->screen[p] & 0xff) == ' ') && ((term->last_screen[p] & 0xff) == 0 || (term->last_screen[p] & 0xff) == 1 || (term->last_screen[p] & 0xff) == ' ') && (x != term->cx || y != term->cy)) continue;
			term->last_screen[p] = term->screen[p];
			if (cx == x && cy == y) goto pc;/*PRINT_CHAR(p)*/
			else if (cy == y && x - cx < 10 && x - cx > 0) {
				int i;
				for (i = x - cx; i >= 0; i--) PRINT_CHAR(p - i);
			} else {
				add_to_str(&a, &l, "\033[");
				add_num_to_str(&a, &l, y + 1);
				add_to_str(&a, &l, ";");
				add_num_to_str(&a, &l, x + 1);
				add_to_str(&a, &l, "H");
				cx = x; cy = y;
				pc:
				PRINT_CHAR(p);
			}
		}
	if (l) {
		if (s->col) add_to_str(&a, &l, "\033[37;40m");
		add_to_str(&a, &l, "\033[0m");
		if (s->mode == TERM_LINUX && s->m11_hack) add_to_str(&a, &l, "\033[10m");
		if (s->mode == TERM_VT100) add_to_str(&a, &l, "\x0f");
	}
	term->lcx = cx;
	term->lcy = cy;
	if (term->cx != term->lcx || term->cy != term->lcy) {
		term->lcx = term->cx;
		term->lcy = term->cy;
		add_to_str(&a, &l, "\033[");
		add_num_to_str(&a, &l, term->cy + 1);
		add_to_str(&a, &l, ";");
		add_num_to_str(&a, &l, term->cx + 1);
		add_to_str(&a, &l, "H");
	}
	if (l && term->master) want_draw();
	hard_write(term->fdout, a, l);
	if (l && term->master) done_draw();
	mem_free(a);
	term->dirty = 0;
}

void destroy_terminal(struct terminal *term)
{
	while ((term->windows.next) != &term->windows) delete_window(term->windows.next);
	/*if (term->cwd) mem_free(term->cwd);*/
	if (term->title) mem_free(term->title);
	mem_free(term->screen);
	mem_free(term->last_screen);
	set_handlers(term->fdin, NULL, NULL, NULL, NULL);
	mem_free(term->input_queue);
	if (term->blocked != -1) {
		close(term->blocked);
		set_handlers(term->blocked, NULL, NULL, NULL, NULL);
	}
	del_from_list(term);
	close(term->fdin);
	if (term->fdout != 1) {
		if (term->fdout != term->fdin) close(term->fdout);
	} else {
		unhandle_terminal_signals(term);
		free_all_itrms();
#ifndef NO_FORK_ON_EXIT
		if (!list_empty(terminals)) {
			if (fork() > 0) _exit(0);
		}
#endif
	}
	mem_free(term);
	check_if_no_terminal();
}

void destroy_all_terminals()
{
	struct terminal *term;
	while ((void *)(term = terminals.next) != &terminals) destroy_terminal(term);
}

void check_if_no_terminal()
{
	if (!list_empty(terminals)) return;
	terminate = 1;
}

void set_char(struct terminal *t, int x, int y, unsigned c)
{
	t->dirty = 1;
	if (x >= 0 && x < t->x && y >= 0 && y < t->y) t->screen[x + t->x * y] = c;
}

unsigned get_char(struct terminal *t, int x, int y)
{
	if (x >= t->x) x = t->x - 1;
	if (x < 0) x = 0;
	if (y >= t->y) y = t->y - 1;
	if (y < 0) y = 0;
	return t->screen[x + t->x * y];
}

void set_color(struct terminal *t, int x, int y, unsigned c)
{
	t->dirty = 1;
	if (x >= 0 && x < t->x && y >= 0 && y < t->y) t->screen[x + t->x * y] = (t->screen[x + t->x * y] & 0x80ff) | (c & ~0x80ff);
}

void set_only_char(struct terminal *t, int x, int y, unsigned c)
{
	t->dirty = 1;
	if (x >= 0 && x < t->x && y >= 0 && y < t->y) t->screen[x + t->x * y] = (t->screen[x + t->x * y] & ~0x80ff) | (c & 0x80ff);
}

void set_line(struct terminal *t, int x, int y, int l, chr *line)
{
	int i;
	t->dirty = 1;
	for (i = x >= 0 ? 0 : -x; i < (x+l <= t->x ? l : t->x-x); i++)
		t->screen[x+i + t->x * y] = line[i];
}

void set_line_color(struct terminal *t, int x, int y, int l, unsigned c)
{
	int i;
	t->dirty = 1;
	for (i = x >= 0 ? 0 : -x; i < (x+l <= t->x ? l : t->x-x); i++)
		t->screen[x+i + t->x * y] = (t->screen[x+i + t->x * y] & 0x80ff) | (c & ~0x80ff);
}

void fill_area(struct terminal *t, int x, int y, int xw, int yw, unsigned c)
{
	int i,j;
	t->dirty = 1;
	for (j = y >= 0 ? 0 : -y; j < yw && y+j < t->y; j++)
		for (i = x >= 0 ? 0 : -x; i < xw && x+i < t->x; i++) 
			t->screen[x+i + t->x*(y+j)] = c;
}

int p1[] = { 218, 191, 192, 217, 179, 196 };
int p2[] = { 201, 187, 200, 188, 186, 205 };

void draw_frame(struct terminal *t, int x, int y, int xw, int yw, unsigned c, int w)
{
	int *p = w > 1 ? p2 : p1;
	c |= ATTR_FRAME;
	set_char(t, x, y, c+p[0]);
	set_char(t, x+xw-1, y, c+p[1]);
	set_char(t, x, y+yw-1, c+p[2]);
	set_char(t, x+xw-1, y+yw-1, c+p[3]);
	fill_area(t, x, y+1, 1, yw-2, c+p[4]);
	fill_area(t, x+xw-1, y+1, 1, yw-2, c+p[4]);
	fill_area(t, x+1, y, xw-2, 1, c+p[5]);
	fill_area(t, x+1, y+yw-1, xw-2, 1, c+p[5]);
}

void print_text(struct terminal *t, int x, int y, int l, unsigned char *text, unsigned c)
{
	for (; l-- && *text; text++, x++) set_char(t, x, y, *text + c);
}

void set_cursor(struct terminal *term, int x, int y, int altx, int alty)
{
	term->dirty = 1;
	if (term->spec->block_cursor) x = altx, y = alty;
	if (x >= term->x) x = term->x - 1;
	if (y >= term->y) y = term->y - 1;
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	term->cx = x;
	term->cy = y;
}

void exec_thread(unsigned char *path, int p)
{
#if defined(HAVE_SETPGID) && !defined(BEOS) && !defined(HAVE_BEGINTHREAD)
	if (path[0] == 2) setpgid(0, 0);
#endif
	exe(path + 1);
	/*close(p);*/
	if (path[1 + strlen(path + 1) + 1]) unlink(path + 1 + strlen(path + 1) + 1);
}

void close_handle(void *p)
{
	int h = (int)p;
	close(h);
	set_handlers(h, NULL, NULL, NULL, NULL);
}

void unblock_terminal(struct terminal *term)
{
	close_handle((void *)term->blocked);
	term->blocked = -1;
	set_handlers(term->fdin, (void (*)(void *))in_term, NULL, (void (*)(void *))destroy_terminal, term);
	unblock_itrm(term->fdin);
	redraw_terminal_cls(term);
}

void exec_on_terminal(struct terminal *term, unsigned char *path, unsigned char *delete, int fg)
{
	if (path && !*path) return;
	if (!path) path="";
#ifdef NO_FG_EXEC
	fg = 0;
#endif
	if (term->master) {
		if (!*path) dispatch_special(delete);
		else {
			int blockh;
			unsigned char *param;
			if (is_blocked() && fg) {
				unlink(delete);
				return;
			}
			param = mem_alloc(strlen(path) + strlen(delete) + 3);
			param[0] = fg;
			strcpy(param + 1, path);
			strcpy(param + 1 + strlen(path) + 1, delete);
			if (fg == 1) block_itrm(term->fdin);
			if ((blockh = start_thread((void (*)(void *, int))exec_thread, param, strlen(path) + strlen(delete) + 3)) == -1) {
				if (fg == 1) unblock_itrm(term->fdin);
				mem_free(param);
				return;
			}
			mem_free(param);
			if (fg == 1) {
				term->blocked = blockh;
				set_handlers(blockh, (void (*)(void *))unblock_terminal, NULL, (void (*)(void *))unblock_terminal, term);
				set_handlers(term->fdin, NULL, NULL, (void (*)(void *))destroy_terminal, term);
				/*block_itrm(term->fdin);*/
			} else {
				set_handlers(blockh, close_handle, NULL, close_handle, (void *)blockh);
			}
		}
	} else {
		unsigned char *data;
		data = mem_alloc(strlen(path) + strlen(delete) + 4);
		data[0] = 0;
		data[1] = fg;
		strcpy(data + 2, path);
		strcpy(data + 3 + strlen(path), delete);
		hard_write(term->fdout, data, strlen(path) + strlen(delete) + 4);
		mem_free(data);
		/*char x = 0;
		hard_write(term->fdout, &x, 1);
		x = fg;
		hard_write(term->fdout, &x, 1);
		hard_write(term->fdout, path, strlen(path) + 1);
		hard_write(term->fdout, delete, strlen(delete) + 1);*/
	}
}

void do_terminal_function(struct terminal *term, unsigned char code, unsigned char *data)
{
	unsigned char *x_data;
	x_data = mem_alloc(strlen(data) + 2);
	x_data[0] = code;
	strcpy(x_data + 1, data);
	exec_on_terminal(term, NULL, x_data, 0);
	mem_free(x_data);
}

void set_terminal_title(struct terminal *term, unsigned char *title)
{
	int i;
	for (i = 0; i < 10000; i++) if (!title[i]) goto s;
	title[10000] = 0;
	s:
	if (strchr(title, 1)) {
		unsigned char *a, *b;
		for (a = title, b = title; *a; a++) if (*a != 1) *b++ = *a;
		*b = 0;
	}
	if (term->title && !strcmp(title, term->title)) goto ret;
	if (term->title) mem_free(term->title);
	term->title = stracpy(title);
#ifdef SET_WINDOW_TITLE_UTF_8
	{
		struct conv_table *table;
		mem_free(title);
		table = get_translation_table(term->spec->charset, get_cp_index("utf-8"));
		title = convert_string(table, term->title, strlen(term->title));
	}
#endif
	do_terminal_function(term, TERM_FN_TITLE, title);
	ret:
	mem_free(title);
}
