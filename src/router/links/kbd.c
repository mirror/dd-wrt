#include "links.h"

#define OUT_BUF_SIZE	16384
#define IN_BUF_SIZE	16

#define USE_TWIN_MOUSE	1
#define TW_BUTT_LEFT	1
#define TW_BUTT_MIDDLE	2
#define TW_BUTT_RIGHT	4

struct itrm {
	int std_in;
	int std_out;
	int sock_in;
	int sock_out;
	int ctl_in;
	int blocked;
	struct termios t;
	int flags;
	unsigned char kqueue[IN_BUF_SIZE];
	int qlen;
	int tm;
	unsigned char *ev_queue;
	int eqlen;
	void *mouse_h;
	unsigned char *orig_title;
};

void free_trm(struct itrm *);
void in_kbd(struct itrm *);
void in_sock(struct itrm *);

struct itrm *ditrm = NULL;

int is_blocked()
{
	return ditrm && ditrm->blocked;
}

void free_all_itrms()
{
	if (ditrm) free_trm(ditrm);
}

void write_ev_queue(struct itrm *itrm)
{
	int l;
	if (!itrm->eqlen) internal("event queue empty");
	if ((l = write(itrm->sock_out, itrm->ev_queue, itrm->eqlen > 128 ? 128 : itrm->eqlen)) == -1) {
		free_trm(itrm);
		return;
	}
	memmove(itrm->ev_queue, itrm->ev_queue + l, itrm->eqlen -= l);
	if (!itrm->eqlen) set_handlers(itrm->sock_out, get_handler(itrm->sock_out, H_READ), NULL, get_handler(itrm->sock_out, H_ERROR), get_handler(itrm->sock_out, H_DATA));
}

void queue_event(struct itrm *itrm, unsigned char *data, int len);

void mouse_queue_event(struct itrm *itrm, unsigned char *data, int len)
{
	if (itrm->blocked) return;
	queue_event(itrm, data, len);
}

void queue_event(struct itrm *itrm, unsigned char *data, int len)
{
	int w = 0;
	if (!len) return;
	if (!itrm->eqlen && can_write(itrm->sock_out) && (w = write(itrm->sock_out, data, len)) <= 0) {
		/*free_trm(itrm);*/
		register_bottom_half((void (*)(void *))free_trm, itrm);
		return;
	}
	if (w < len) {
		if ((unsigned)itrm->eqlen + (unsigned)(len - w) > MAXINT) overalloc();
		itrm->ev_queue = mem_realloc(itrm->ev_queue, itrm->eqlen + len - w);
		memcpy(itrm->ev_queue + itrm->eqlen, data + w, len - w);
		itrm->eqlen += len - w;
		set_handlers(itrm->sock_out, get_handler(itrm->sock_out, H_READ), (void (*)(void *))write_ev_queue, (void (*)(void *))free_trm, itrm);
	}
}

void kbd_ctrl_c()
{
	struct event ev = { EV_KBD, KBD_CTRL_C, 0, 0 };
	if (ditrm) queue_event(ditrm, (unsigned char *)&ev, sizeof(struct event));
}

/*
unsigned char *init_seq = "\033[?1000h\033[?47h\0337";
unsigned char *term_seq = "\033[2J\033[?1000l\033[?47l\0338\b \b";
*/

unsigned char init_seq[] = "\033)0\0337";
unsigned char init_seq_x_mouse[] = "\033[?1000h";
unsigned char init_seq_tw_mouse[] = "\033[?9h";
unsigned char term_seq[] = "\033[2J\0338\r \b";
unsigned char term_seq_x_mouse[] = "\033[?1000l\015       \015";
unsigned char term_seq_tw_mouse[] = "\033[?9l";

/*unsigned char *term_seq = "\033[2J\033[?1000l\0338\b \b";*/

void send_init_sequence(int h, int flags)
{
	want_draw();
	hard_write(h, init_seq, strlen(init_seq));
	if (flags & USE_TWIN_MOUSE) {
		hard_write(h, init_seq_tw_mouse, strlen(init_seq_tw_mouse));
	} else {
		hard_write(h, init_seq_x_mouse, strlen(init_seq_x_mouse));
	}
	done_draw();
}

void send_term_sequence(int h,int flags)
{
	want_draw();
	hard_write(h, term_seq, strlen(term_seq));
	if (flags & USE_TWIN_MOUSE) {
		hard_write(h, term_seq_tw_mouse, strlen(term_seq_tw_mouse));
	} else {
		hard_write(h, term_seq_x_mouse, strlen(term_seq_x_mouse));
	}
	done_draw();
}

void resize_terminal()
{
	struct event ev = { EV_RESIZE, 0, 0, 0 };
	int x, y;
	if (get_terminal_size(ditrm->std_out, &x, &y)) return;
	ev.x = x;
	ev.y = y;
	queue_event(ditrm, (unsigned char *)&ev, sizeof(struct event));
}

int ttcgetattr(int fd, struct termios *t)
{
	int r;
#ifdef SIGTTOU
	interruptible_signal(SIGTTOU, 1);
#endif
#ifdef SIGTTIN
	interruptible_signal(SIGTTIN, 1);
#endif
	r = tcgetattr(fd, t);
#ifdef SIGTTOU
	interruptible_signal(SIGTTOU, 0);
#endif
#ifdef SIGTTIN
	interruptible_signal(SIGTTIN, 0);
#endif
	return r;
}

int ttcsetattr(int fd, int a, struct termios *t)
{
	int r;
#ifdef SIGTTOU
	interruptible_signal(SIGTTOU, 1);
#endif
#ifdef SIGTTIN
	interruptible_signal(SIGTTIN, 1);
#endif
	r = tcsetattr(fd, a, t);
#ifdef SIGTTOU
	interruptible_signal(SIGTTOU, 0);
#endif
#ifdef SIGTTIN
	interruptible_signal(SIGTTIN, 0);
#endif
	return r;
}

int setraw(int fd, struct termios *p)
{
	struct termios t;
	memset(&t, 0, sizeof(struct termios));
	if (ttcgetattr(fd, &t)) {
		/*fprintf(stderr, "getattr result %s\n", strerror(errno));*/
		/* If the terminal was destroyed (the user logged off),
		 * we fake success here so that we can destroy the terminal
		 * later.
		 *
		 * Linux returns EIO
		 * FreeBSD returns ENXIO
		 */
		if (errno == EIO || errno == ENXIO) return 0;
		return -1;
	}
	if (p) memcpy(p, &t, sizeof(struct termios));
	os_cfmakeraw(&t);
	t.c_lflag |= ISIG;
#ifdef TOSTOP
	t.c_lflag |= TOSTOP;
#endif
	t.c_oflag |= OPOST;
	if (ttcsetattr(fd, TCSANOW, &t)) {
		/*fprintf(stderr, "setattr result %s\n", strerror(errno));*/
		return -1;
	}
	return 0;
}

void handle_trm(int std_in, int std_out, int sock_in, int sock_out, int ctl_in, void *init_string, int init_len)
{
	int x, y;
	struct itrm *itrm;
	struct event ev = { EV_INIT, 80, 24, 0 };
	unsigned char *ts;
	int xwin;
	if (get_terminal_size(ctl_in, &x, &y)) {
		error("ERROR: could not get terminal size");
		return;
	}
	itrm = mem_alloc(sizeof(struct itrm));
	ditrm = itrm;
	itrm->std_in = std_in;
	itrm->std_out = std_out;
	itrm->sock_in = sock_in;
	itrm->sock_out = sock_out;
	itrm->ctl_in = ctl_in;
	itrm->blocked = 0;
	itrm->qlen = 0;
	itrm->tm = -1;
	itrm->ev_queue = DUMMY;
	itrm->eqlen = 0;
	if (ctl_in >= 0) setraw(ctl_in, &itrm->t);
	set_handlers(std_in, (void (*)(void *))in_kbd, NULL, (void (*)(void *))free_trm, itrm);
	if (sock_in != std_out) set_handlers(sock_in, (void (*)(void *))in_sock, NULL, (void (*)(void *))free_trm, itrm);
	ev.x = x;
	ev.y = y;
	handle_terminal_resize(ctl_in, resize_terminal);
	queue_event(itrm, (unsigned char *)&ev, sizeof(struct event));
	xwin = is_xterm() * ENV_XWIN + can_twterm() * ENV_TWIN + (!!getenv("STY")) * ENV_SCREEN + get_system_env();
	itrm->flags = 0;
	if (!(ts = getenv("TERM"))) ts = "";
	if ((xwin & ENV_TWIN) && !strcmp(ts,"linux")) itrm->flags |= USE_TWIN_MOUSE;
	if (strlen(ts) >= MAX_TERM_LEN) queue_event(itrm, ts, MAX_TERM_LEN);
	else {
		unsigned char *mm;
		int ll = MAX_TERM_LEN - strlen(ts);
		queue_event(itrm, ts, strlen(ts));
		mm = mem_alloc(ll);
		memset(mm, 0, ll);
		queue_event(itrm, mm, ll);
		mem_free(mm);
	}
	if (!(ts = get_cwd())) ts = stracpy("");
	if (strlen(ts) >= MAX_CWD_LEN) queue_event(itrm, ts, MAX_CWD_LEN);
	else {
		unsigned char *mm;
		int ll = MAX_CWD_LEN - strlen(ts);
		queue_event(itrm, ts, strlen(ts));
		mm = mem_alloc(ll);
		memset(mm, 0, ll);
		queue_event(itrm, mm, ll);
		mem_free(mm);
	}
	mem_free(ts);
	queue_event(itrm, (unsigned char *)&xwin, sizeof(int));
	queue_event(itrm, (unsigned char *)&init_len, sizeof(int));
	queue_event(itrm, (unsigned char *)init_string, init_len);
	itrm->mouse_h = handle_mouse(0, (void (*)(void *, unsigned char *, int))mouse_queue_event, itrm);
	itrm->orig_title = get_window_title();
	set_window_title("Links");
	send_init_sequence(std_out, itrm->flags);
}

void unblock_itrm_x(void *h)
{
	close_handle(h);
	if (!ditrm) return;
	unblock_itrm(0);
}

int unblock_itrm(int fd)
{
	struct itrm *itrm = ditrm;
	if (!itrm) return -1;
	if (itrm->ctl_in >= 0 && setraw(itrm->ctl_in, NULL)) return -1;
	if (itrm->blocked != fd + 1) return -2;
	itrm->blocked = 0;
	send_init_sequence(itrm->std_out, itrm->flags);
	set_handlers(itrm->std_in, (void (*)(void *))in_kbd, NULL, (void (*)(void *))free_trm, itrm);
	handle_terminal_resize(itrm->ctl_in, resize_terminal);
	unblock_stdin();
	resize_terminal();
	return 0;
}

void block_itrm(int fd)
{
	struct itrm *itrm = ditrm;
	if (!itrm) return;
	if (itrm->blocked) return;
	itrm->blocked = fd + 1;
	block_stdin();
	unhandle_terminal_resize(itrm->ctl_in);
	send_term_sequence(itrm->std_out,itrm->flags);
	ttcsetattr(itrm->ctl_in, TCSANOW, &itrm->t);
	set_handlers(itrm->std_in, NULL, NULL, (void (*)(void *))free_trm, itrm);
}

void free_trm(struct itrm *itrm)
{
	if (!itrm) return;
	set_window_title(itrm->orig_title);
	if (itrm->orig_title) mem_free(itrm->orig_title), itrm->orig_title = NULL;
	unhandle_terminal_resize(itrm->ctl_in);
	send_term_sequence(itrm->std_out,itrm->flags);
	ttcsetattr(itrm->ctl_in, TCSANOW, &itrm->t);
	if (itrm->mouse_h) unhandle_mouse(itrm->mouse_h);
	set_handlers(itrm->std_in, NULL, NULL, NULL, NULL);
	set_handlers(itrm->sock_in, NULL, NULL, NULL, NULL);
	set_handlers(itrm->std_out, NULL, NULL, NULL, NULL);
	set_handlers(itrm->sock_out, NULL, NULL, NULL, NULL);
	if (itrm->tm != -1) kill_timer(itrm->tm);
	mem_free(itrm->ev_queue);
	mem_free(itrm);
	if (itrm == ditrm) ditrm = NULL;
}

void fatal_tty_exit(void)
{
	if (ditrm) ttcsetattr(ditrm->ctl_in, TCSANOW, &ditrm->t);
}

void resize_terminal_x(unsigned char *text)
{
	int x, y;
	unsigned char *p;
	if (!(p = strchr(text, ','))) return;
	*p++ = 0;
	x = atoi(text);
	y = atoi(p);
	resize_window(x, y);
	resize_terminal();
}

void dispatch_special(unsigned char *text)
{
	switch (text[0]) {
		case TERM_FN_TITLE:
			set_window_title(text + 1);
			break;
		case TERM_FN_RESIZE:
			resize_terminal_x(text + 1);
			break;
	}
}

unsigned char buf[OUT_BUF_SIZE];

#define RD(xx) { unsigned char cc; if (p < c) cc = buf[p++]; else if ((hard_read(itrm->sock_in, &cc, 1)) <= 0) goto fr; xx = cc; }

void in_sock(struct itrm *itrm)
{
	unsigned char *path, *delete;
	int pl, dl;
	unsigned char ch;
	int fg;
	int c, i, p;
	if ((c = read(itrm->sock_in, buf, OUT_BUF_SIZE)) <= 0) {
		fr:
		free_trm(itrm);
		terminate = 1;
		return;
	}
	qwerty:
	for (i = 0; i < c; i++) if (!buf[i]) goto ex;
	if (!is_blocked()) {
		want_draw();
		hard_write(itrm->std_out, buf, c);
		done_draw();
	}
	return;
	ex:
	if (!is_blocked()) {
		want_draw();
		hard_write(itrm->std_out, buf, i);
		done_draw();
	}
	i++;
	memmove(buf, buf + i, OUT_BUF_SIZE - i);
	c -= i;
	p = 0;
	/*fg = RD;*/
	RD(fg);
	path = init_str();
	delete = init_str();
	pl = dl = 0;
	while (1) {
		RD(ch);
		if (!ch) break;
		add_chr_to_str(&path, &pl, ch);
	}
	while (1) {
		RD(ch);
		if (!ch) break;
		add_chr_to_str(&delete, &dl, ch);
	}
	if (!*path) {
		dispatch_special(delete);
	} else {
		int blockh;
		unsigned char *param;
		if (is_blocked() && fg) {
			if (*delete) unlink(delete);
			goto to_je_ale_hnus;
		}
		param = mem_alloc(strlen(path) + strlen(delete) + 3);
		param[0] = fg;
		strcpy(param + 1, path);
		strcpy(param + 1 + strlen(path) + 1, delete);
		if (fg == 1) block_itrm(0);
		if ((blockh = start_thread((void (*)(void *, int))exec_thread, param, strlen(path) + strlen(delete) + 3)) == -1) {
			if (fg == 1) unblock_itrm(0);
			mem_free(param);
			goto to_je_ale_hnus;
		}
		mem_free(param);
		if (fg == 1) {
			set_handlers(blockh, (void (*)(void *))unblock_itrm_x, NULL, (void (*)(void *))unblock_itrm_x, (void *)blockh);
		} else {
			set_handlers(blockh, close_handle, NULL, close_handle, (void *)blockh);
		}
	}
	to_je_ale_hnus:
	mem_free(path);
	mem_free(delete);
	memmove(buf, buf + p, OUT_BUF_SIZE - p);
	c -= p;
	goto qwerty;
}

int process_queue(struct itrm *);

int get_esc_code(unsigned char *str, int len, unsigned char *code, int *num, int *el)
{
	int pos;
	*num = 0;
	for (pos = 2; pos < len; pos++) {
		if (str[pos] < '0' || str[pos] > '9' || pos > 7) {
			*el = pos + 1;
			*code = str[pos];
			return 0;
		}
		*num = *num * 10 + str[pos] - '0';
	}
	return -1;
}

void kbd_timeout(struct itrm *itrm)
{
	struct event ev = {EV_KBD, KBD_ESC, 0, 0};
	unsigned char code;
	int num;
	int len;
	itrm->tm = -1;
	if (can_read(itrm->std_in)) {
		in_kbd(itrm);
		return;
	}
	if (!itrm->qlen) {
		internal("timeout on empty queue");
		return;
	}
	queue_event(itrm, (unsigned char *)&ev, sizeof(struct event));
	if (get_esc_code(itrm->kqueue, itrm->qlen, &code, &num, &len)) len = 1;
	itrm->qlen -= len;
	memmove(itrm->kqueue, itrm->kqueue + len, itrm->qlen);
	while (process_queue(itrm)) ;
}

struct key {
	int x, y;
};

struct key os2xtd[256] = {
/* 0 */
{0,0},{0,0},{' ',KBD_CTRL},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{KBD_BS,KBD_ALT},{0,0},
/* 16 */
{'Q',KBD_ALT},{'W',KBD_ALT},{'E',KBD_ALT},{'R',KBD_ALT},{'T',KBD_ALT},{'Y',KBD_ALT},{'U',KBD_ALT},{'I',KBD_ALT},
/* 24 */
{'O',KBD_ALT},{'P',KBD_ALT},{'[',KBD_ALT},{']',KBD_ALT},{KBD_ENTER,KBD_ALT},{0,0},{'A',KBD_ALT},{'S',KBD_ALT},
/* 32 */
{'D',KBD_ALT},{'F',KBD_ALT},{'G',KBD_ALT},{'H',KBD_ALT},{'J',KBD_ALT},{'K',KBD_ALT},{'L',KBD_ALT},{';',KBD_ALT},
/* 40 */
{'\'',KBD_ALT},{'`',KBD_ALT},{0,0},{'\\',KBD_ALT},{'Z',KBD_ALT},{'X',KBD_ALT},{'C',KBD_ALT},{'V',KBD_ALT},
/* 48 */
{'B',KBD_ALT},{'N',KBD_ALT},{'M',KBD_ALT},{',',KBD_ALT},{'.',KBD_ALT},{'/',KBD_ALT},{0,0},{'*',KBD_ALT},
/* 56 */
{0,0},{' ',KBD_ALT},{0,0},{KBD_F1,0},{KBD_F2,0},{KBD_F3,0},{KBD_F4,0},{KBD_F5,0},
/* 64 */
{KBD_F6,0},{KBD_F7,0},{KBD_F8,0},{KBD_F9,0},{KBD_F10,0},{0,0},{0,0},{KBD_HOME,0},
/* 72 */
{KBD_UP,0},{KBD_PAGE_UP,0},{'-',KBD_ALT},{KBD_LEFT,0},{'5',0},{KBD_RIGHT,0},{'+',KBD_ALT},{KBD_END,0},
/* 80 */
{KBD_DOWN,0},{KBD_PAGE_DOWN,0},{KBD_INS,0},{KBD_DEL,0},{0,0},{0,0},{0,0},{0,0},
/* 88 */
{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{KBD_F1,KBD_CTRL},{KBD_F2,KBD_CTRL},
/* 96 */
{KBD_F3,KBD_CTRL},{KBD_F4,KBD_CTRL},{KBD_F5,KBD_CTRL},{KBD_F6,KBD_CTRL},{KBD_F7,KBD_CTRL},{KBD_F8,KBD_CTRL},{KBD_F9,KBD_CTRL},{KBD_F10,KBD_CTRL},
/* 104 */
{KBD_F1,KBD_ALT},{KBD_F2,KBD_ALT},{KBD_F3,KBD_ALT},{KBD_F4,KBD_ALT},{KBD_F5,KBD_ALT},{KBD_F6,KBD_ALT},{KBD_F7,KBD_ALT},{KBD_F8,KBD_ALT},
/* 112 */
{KBD_F9,KBD_ALT},{KBD_F10,KBD_ALT},{0,0},{KBD_LEFT,KBD_CTRL},{KBD_RIGHT,KBD_CTRL},{KBD_END,KBD_CTRL},{KBD_PAGE_DOWN,KBD_CTRL},{KBD_HOME,KBD_CTRL},
/* 120 */
{'1',KBD_ALT},{'2',KBD_ALT},{'3',KBD_ALT},{'4',KBD_ALT},{'5',KBD_ALT},{'6',KBD_ALT},{'7',KBD_ALT},{'8',KBD_ALT},
/* 128 */
{'9',KBD_ALT},{'0',KBD_ALT},{'-',KBD_ALT},{'=',KBD_ALT},{KBD_PAGE_UP,KBD_CTRL},{KBD_F11,0},{KBD_F12,0},{0,0},
/* 136 */
{0,0},{KBD_F11,KBD_CTRL},{KBD_F12,KBD_CTRL},{KBD_F11,KBD_ALT},{KBD_F12,KBD_ALT},{KBD_UP,KBD_CTRL},{'-',KBD_CTRL},{'5',KBD_CTRL},
/* 144 */
{'+',KBD_CTRL},{KBD_DOWN,KBD_CTRL},{KBD_INS,KBD_CTRL},{KBD_DEL,KBD_CTRL},{KBD_TAB,KBD_CTRL},{0,0},{0,0},{KBD_HOME,KBD_ALT},
/* 152 */
{KBD_UP,KBD_ALT},{KBD_PAGE_UP,KBD_ALT},{0,0},{KBD_LEFT,KBD_ALT},{0,0},{KBD_RIGHT,KBD_ALT},{0,0},{KBD_END,KBD_ALT},
/* 160 */
{KBD_DOWN,KBD_ALT},{KBD_PAGE_DOWN,KBD_ALT},{KBD_INS,KBD_ALT},{KBD_DEL,KBD_ALT},{0,0},{KBD_TAB,KBD_ALT},{KBD_ENTER,KBD_ALT},{0,0},
/* 168 */
{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
/* 176 */
{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
/* 192 */
{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
/* 208 */
{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
/* 224 */
{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
/* 240 */
{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},{0,0},
/* 256 */
};

int xterm_button = -1;

int is_interix(void)
{
#ifdef INTERIX
	return 1;
#else
	unsigned char *term = getenv("TERM");
	return term && !strncmp(term, "interix", 7);
#endif
}

int is_ibm(void)
{
	unsigned char *term = getenv("TERM");
	return term && !strncmp(term, "ibm", 3);
}

int process_queue(struct itrm *itrm)
{
	struct event ev = { EV_KBD, -1, 0, 0 };
	int el = 0;
	if (!itrm->qlen) goto end;
	/*{
		int i;
		fprintf(stderr, "queue:");
		for (i = 0; i < itrm->qlen; i++) {
			fprintf(stderr, " %d", itrm->kqueue[i]);
			if (itrm->kqueue[i] >= 32) fprintf(stderr, "[%c]", itrm->kqueue[i]);
		}
		fprintf(stderr, ".\n");
	}*/
	if (itrm->kqueue[0] == '\033') {
		if (itrm->qlen < 2) goto ret;
		if (itrm->kqueue[1] == '[' || itrm->kqueue[1] == 'O') {
			unsigned char c = 0;
			int v;
			if (itrm->qlen >= 4 && itrm->kqueue[2] == '[') {
				if (itrm->kqueue[3] < 'A' || itrm->kqueue[3] > 'L') goto ret;
				ev.x = KBD_F1 + (itrm->kqueue[3] - 'A');
				el = 4;
			} else if (get_esc_code(itrm->kqueue, itrm->qlen, &c, &v, &el)) goto ret;
			else switch (c) {
				case 'L':
				case '@': ev.x = KBD_INS; break;
				case 'A': ev.x = KBD_UP; break;
				case 'B': ev.x = KBD_DOWN; break;
				case 'C': ev.x = KBD_RIGHT; break;
				case 'D': ev.x = KBD_LEFT; break;
				case 'F':
				case 'K':
				case 'e': ev.x = KBD_END; break;
				case 'H':
				case 0: ev.x = KBD_HOME; break;
				case 'V':
				case 'I': ev.x = KBD_PAGE_UP; break;
				case 'U':
					if (is_interix()) {
						ev.x = KBD_END;
						break;
					}
				case 'G': ev.x = KBD_PAGE_DOWN; break;
				case 'P':
					if (is_ibm()) {
						ev.x = KBD_DEL;
						break;
					}
					ev.x = KBD_F1; break;
				case 'Q': ev.x = KBD_F2; break;
				case 'S':
					if (is_interix()) {
						ev.x = KBD_PAGE_UP;
						break;
					}
					ev.x = KBD_F4; break;
				case 'T':
					if (is_interix()) {
						ev.x = KBD_PAGE_DOWN;
						break;
					}
					ev.x = KBD_F5; break;
				case 'W': ev.x = KBD_F8; break;
				case 'X': ev.x = KBD_F9; break;
				case 'Y': ev.x = KBD_F11; break;

				case 'q': switch (v) {
					case 139: ev.x = KBD_INS; break;
					case 146: ev.x = KBD_END; break;
					case 150: ev.x = KBD_PAGE_UP; break;
					case 154: ev.x = KBD_PAGE_DOWN; break;
					default: if (v >= 1 && v <= 48) {
							int fn = (v - 1) % 12;
							int mod = (v - 1) / 12;
							ev.x = KBD_F1 + fn;
							if (mod == 1)
								ev.y |= KBD_SHIFT;
							if (mod == 2)
								ev.y |= KBD_CTRL;
							if (mod == 3)
								ev.y |= KBD_ALT;
						} break;
					} break;
				case 'z': switch (v) {
					case 247: ev.x = KBD_INS; break;
					case 214: ev.x = KBD_HOME; break;
					case 220: ev.x = KBD_END; break;
					case 216: ev.x = KBD_PAGE_UP; break;
					case 222: ev.x = KBD_PAGE_DOWN; break;
					case 249: ev.x = KBD_DEL; break;
					} break;
				case '~': switch (v) {
					case 1: ev.x = KBD_HOME; break;
					case 2: ev.x = KBD_INS; break;
					case 3: ev.x = KBD_DEL; break;
					case 4: ev.x = KBD_END; break;
					case 5: ev.x = KBD_PAGE_UP; break;
					case 6: ev.x = KBD_PAGE_DOWN; break;
					case 7: ev.x = KBD_HOME; break;
					case 8: ev.x = KBD_END; break;
					case 17: ev.x = KBD_F6; break;
					case 18: ev.x = KBD_F7; break;
					case 19: ev.x = KBD_F8; break;
					case 20: ev.x = KBD_F9; break;
					case 21: ev.x = KBD_F10; break;
					case 23: ev.x = KBD_F11; break;
					case 24: ev.x = KBD_F12; break;
					} break;
				case 'R':
					resize_terminal();
					break ;
				case 'M': if (itrm->qlen - el < 3) goto ret;
					if (v == 5) {
						if (xterm_button == -1) xterm_button = 0; /* */
						if (itrm->qlen - el < 5) goto ret;
						ev.x = (unsigned char)(itrm->kqueue[el+1]) - ' ' - 1 + ((int)((unsigned char)(itrm->kqueue[el+2]) - ' ' - 1) << 7);
						if ( ev.x & (1 << 13)) ev.x = 0; /* ev.x |= ~0 << 14; */
						ev.y = (unsigned char)(itrm->kqueue[el+3]) - ' ' - 1 + ((int)((unsigned char)(itrm->kqueue[el+4]) - ' ' - 1) << 7);
						if ( ev.y & (1 << 13)) ev.y = 0; /* ev.y |= ~0 << 14; */
						switch ((itrm->kqueue[el] - ' ') ^ xterm_button) { /* Every event changhes only one bit */
						    case TW_BUTT_LEFT:   ev.b = B_LEFT | ( (xterm_button & TW_BUTT_LEFT) ? B_UP : B_DOWN ); break; 
						    case TW_BUTT_MIDDLE: ev.b = B_MIDDLE | ( (xterm_button & TW_BUTT_MIDDLE) ? B_UP :  B_DOWN ); break;
						    case TW_BUTT_RIGHT:  ev.b = B_RIGHT | ( (xterm_button & TW_BUTT_RIGHT) ? B_UP : B_DOWN ); break;
						    case 0: ev.b = B_DRAG;
						    /* default : Twin protocol error */
						}
						xterm_button = itrm->kqueue[el] - ' ';
						el += 5;
					} else {
						ev.x = itrm->kqueue[el+1] - ' ' - 1;
						ev.y = itrm->kqueue[el+2] - ' ' - 1;
						ev.b = B_DOWN;
						if (itrm->kqueue[el] & 4) ev.b = B_DRAG;
						if ((ev.b |= (itrm->kqueue[el] & BM_BUTT) | B_DOWN) == 3) {
							ev.b = B_UP;
							if (xterm_button != -1) ev.b |= xterm_button;
						}  
						/*if ((itrm->kqueue[el] & 4) && ev.b != B_UP) ev.b |= B_DRAG;*/
						xterm_button = -1;
						if ((ev.b & BM_ACT) == B_DOWN) xterm_button = ev.b & BM_BUTT;
						el += 3;
					}
					ev.ev = EV_MOUSE;
					break;
			}
		} else {
			el = 2;
			if (itrm->kqueue[1] == '\033') {
				if (itrm->qlen >= 3 && (itrm->kqueue[2] == '[' || itrm->kqueue[2] == 'O')) el = 1;
				ev.x = KBD_ESC;
				goto l2;
			} else if (itrm->kqueue[1] == 127) {
				ev.x = KBD_DEL;
				ev.y = 0;
				goto l2;
			} else if (itrm->kqueue[1] == 'F' && is_interix()) {
				int num;
				if (itrm->qlen < 3) goto ret;
				if (itrm->kqueue[2] >= '1' && itrm->kqueue[2] <= '9') {
					num = itrm->kqueue[2] - '1';
				} else if (itrm->kqueue[2] >= 'A' && itrm->kqueue[2] <= 'Z') {
					num = itrm->kqueue[2] - 'A' + 9;
				} else if (itrm->kqueue[2] >= 'a' && itrm->kqueue[2] <= 'k') {
					num = itrm->kqueue[2] - 'a' + 35;
				} else if (itrm->kqueue[2] >= 'm' && itrm->kqueue[2] <= 'z') {
					num = itrm->kqueue[2] - 'a' + 34;
				} else goto do_alt;
				if (num < 12) {
					ev.x = KBD_F1 + num;
				} else if (num < 24) {
					ev.x = KBD_F1 + (num - 12);
					ev.y |= KBD_SHIFT;
				} else if (num < 36) {
					ev.x = KBD_F1 + (num - 24);
					ev.y |= KBD_ALT;
				} else if (num < 48) {
					ev.x = KBD_F1 + (num - 36);
					ev.y |= KBD_CTRL;
				} else {
					ev.x = KBD_F1 + (num - 48);
					ev.y |= KBD_SHIFT | KBD_CTRL;
				}
				el = 3;
				goto l1;
			} else {
do_alt:
				ev.x = itrm->kqueue[1];
				ev.y |= KBD_ALT;
				goto l2;
			}
		}
		goto l1;
	} else if (itrm->kqueue[0] == 0) {
		if (itrm->qlen < 2) goto ret;
		if (!(ev.x = os2xtd[itrm->kqueue[1]].x)) ev.x = -1;
		ev.y = os2xtd[itrm->kqueue[1]].y;
		el = 2;
		/*printf("%02x - %02x %02x\n", (int)itrm->kqueue[1], ev.x, ev.y);*/
		goto l1;
	}
	el = 1;
	ev.x = itrm->kqueue[0];
	l2:
	/*if (ev.x == 1) ev.x = KBD_HOME;
	if (ev.x == 2) ev.x = KBD_PAGE_UP;
	if (ev.x == 4) ev.x = KBD_DEL;
	if (ev.x == 5) ev.x = KBD_END;
	if (ev.x == 6) ev.x = KBD_PAGE_DOWN;*/
	if (ev.x == 8) ev.x = KBD_BS;
	if (ev.x == 9) ev.x = KBD_TAB;
	if (ev.x == 10) ev.x = KBD_ENTER/*, ev.y = KBD_CTRL*/;
	if (ev.x == 13) ev.x = KBD_ENTER;
	if (ev.x == 127) {
		if (is_interix()) ev.x = KBD_DEL;
		else ev.x = KBD_BS;
	}
	if (ev.x >= 0 && ev.x < ' ') {
		ev.x += 'A' - 1;
		ev.y |= KBD_CTRL;
	}
	l1:
	if (itrm->qlen < el) {
		internal("event queue underflow");
		itrm->qlen = el;
	}
	if (ev.x != -1) {
		queue_event(itrm, (unsigned char *)&ev, sizeof(struct event));
		memmove(itrm->kqueue, itrm->kqueue + el, itrm->qlen -= el);
	} else {
		/*printf("%d %d\n", itrm->qlen, el);fflush(stdout);*/
		memmove(itrm->kqueue, itrm->kqueue + el, itrm->qlen -= el);
	}
	end:
	if (itrm->qlen < IN_BUF_SIZE && !itrm->blocked) set_handlers(itrm->std_in, (void (*)(void *))in_kbd, NULL, (void (*)(void *))free_trm, itrm);
	return el;
	ret:
	itrm->tm = install_timer(ESC_TIMEOUT, (void (*)(void *))kbd_timeout, itrm);
	return 0;
}

void in_kbd(struct itrm *itrm)
{
	int r;
	if (!can_read(itrm->std_in)) return;
	if (itrm->tm != -1) kill_timer(itrm->tm), itrm->tm = -1;
	if (itrm->qlen >= IN_BUF_SIZE) {
		set_handlers(itrm->std_in, NULL, NULL, (void (*)(void *))free_trm, itrm);
		while (process_queue(itrm));
		return;
	}
	if ((r = read(itrm->std_in, itrm->kqueue + itrm->qlen, IN_BUF_SIZE - itrm->qlen)) <= 0) {
		struct event ev = { EV_ABORT, 0, 0, 0 };
		set_handlers(itrm->std_in, NULL, NULL, (void (*)(void *))free_trm, itrm);
		queue_event(itrm, (unsigned char *)&ev, sizeof(struct event));
		return;
	}
	if ((itrm->qlen += r) > IN_BUF_SIZE) {
		error("ERROR: too many bytes read");
		itrm->qlen = IN_BUF_SIZE;
	}
	while (process_queue(itrm));
}
