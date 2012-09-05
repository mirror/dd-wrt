#include "links.h"

struct list_head downloads = {&downloads, &downloads};

int are_there_downloads()
{
	int d = 0;
	struct download *down;
	foreach(down, downloads) if (!down->prog) d = 1;
	return d;
}

struct list_head sessions = {&sessions, &sessions};

int session_id = 1;

struct strerror_val {
	struct strerror_val *next;
	struct strerror_val *prev;
	unsigned char msg[1];
};

struct list_head strerror_buf = { &strerror_buf, &strerror_buf };

void free_strerror_buf()
{
	free_list(strerror_buf);
}

int get_error_from_errno(int errn)
{
	if (errn > 0 && (errn < -S_OK || errn > -S_MAX))
		return -errn;
#ifdef BEOS
	if (-errn > 0 && (-errn < -S_OK || -errn > -S_MAX))
		return errn;
#endif
	return S_UNKNOWN_ERROR;
}

unsigned char *get_err_msg(int state)
{
	unsigned char *e;
	struct strerror_val *s;
	if ((state >= S_MAX && state <= S_OK) || state >= S_WAIT) {
		int i;
		for (i = 0; msg_dsc[i].msg; i++)
			if (msg_dsc[i].n == state) return msg_dsc[i].msg;
		unk:
		return TEXT_(T_UNKNOWN_ERROR);
	}
#ifdef BEOS
	if ((e = strerror(state)) && *e && !strstr(e, "No Error")) goto have_error;
#endif
	if ((e = strerror(-state)) && *e) goto have_error;
	goto unk;
have_error:
	foreach(s, strerror_buf) if (!strcmp(s->msg, e)) return s->msg;
	s = mem_alloc(sizeof(struct strerror_val) + strlen(e));
	strcpy(s->msg, e);
	add_to_list(strerror_buf, s);
	return s->msg;
}

void add_xnum_to_str(unsigned char **s, int *l, off_t n)
{
	unsigned char suff = 0;
	int d = -1;
	if (n >= 1000000000) suff = 'G', d = (n / 100000000) % 10, n /= 1000000000;
	else if (n >= 1000000) suff = 'M', d = (n / 100000) % 10, n /= 1000000;
	else if (n >= 1000) suff = 'k', d = (n / 100) % 10, n /= 1000;
	add_num_to_str(s, l, n);
	if (n < 10 && d != -1) add_chr_to_str(s, l, '.'), add_num_to_str(s, l, d);
	add_chr_to_str(s, l, ' ');
	if (suff) add_chr_to_str(s, l, suff);
	add_chr_to_str(s, l, 'B');
}

void add_time_to_str(unsigned char **s, int *l, ttime t)
{
	unsigned char q[64];
	if (t < 0) t = 0;
	t &= 0xffffffff;
	if (t >= 86400) sprintf(q, "%dd ", (int)(t / 86400)), add_to_str(s, l, q);
	if (t >= 3600) t %= 86400, sprintf(q, "%d:%02d", (int)(t / 3600), (int)(t / 60 % 60)), add_to_str(s, l, q);
	else sprintf(q, "%d", (int)(t / 60)), add_to_str(s, l, q);
	sprintf(q, ":%02d", (int)(t % 60)), add_to_str(s, l, q);
}

unsigned char *get_stat_msg(struct status *stat, struct terminal *term)
{
	if (stat->state == S_TRANS && stat->prg->elapsed / 100) {
		unsigned char *m = init_str();
		int l = 0;
		add_to_str(&m, &l, _(TEXT_(T_RECEIVED), term));
		add_to_str(&m, &l, " ");
		add_xnum_to_str(&m, &l, stat->prg->pos);
		if (stat->prg->size >= 0)
			add_to_str(&m, &l, " "), add_to_str(&m, &l, _(TEXT_(T_OF), term)), add_to_str(&m, &l, " "), add_xnum_to_str(&m, &l, stat->prg->size);
		add_to_str(&m, &l, ", ");
		if (stat->prg->elapsed >= CURRENT_SPD_AFTER * SPD_DISP_TIME)
			add_to_str(&m, &l, _(TEXT_(T_AVG), term)), add_to_str(&m, &l, " ");
		add_xnum_to_str(&m, &l, (longlong)stat->prg->loaded * 10 / (stat->prg->elapsed / 100));
		add_to_str(&m, &l, "/s");
		if (stat->prg->elapsed >= CURRENT_SPD_AFTER * SPD_DISP_TIME) 
			add_to_str(&m, &l, ", "), add_to_str(&m, &l, _(TEXT_(T_CUR), term)), add_to_str(&m, &l, " "),
			add_xnum_to_str(&m, &l, stat->prg->cur_loaded / (CURRENT_SPD_SEC * SPD_DISP_TIME / 1000)),
			add_to_str(&m, &l, "/s");
		return m;
	}
	return stracpy(_(get_err_msg(stat->state), term));
}

void print_screen_status(struct session *ses)
{
	struct terminal *term = ses->term;
	struct status *stat = NULL;
	unsigned char *m;
	fill_area(term, 0, 0, term->x, 1, COLOR_TITLE_BG);
	fill_area(term, 0, term->y - 1, term->x, 1, COLOR_STATUS_BG);
	if (ses->wtd) stat = &ses->loading;
	else if (!list_empty(ses->history)) stat = &cur_loc(ses)->stat;
	if (stat && stat->state == S_OK) {
		struct file_to_load *ftl;
		foreach(ftl, ses->more_files) {
			if (ftl->req_sent && ftl->stat.state >= 0) {
				stat = &ftl->stat;
				break;
			}
		}
	}
	if (stat) {
		if (stat->state == S_OK) if ((m = print_current_link(ses))) goto p;
		if ((m = get_stat_msg(stat, term))) {
			p:
			print_text(term, 0, term->y - 1, strlen(m), m, COLOR_STATUS);
			mem_free(m);
		}
		if ((m = print_current_title(ses))) {
			int p = term->x - 1 - strlen(m);
			if (p < 0) p = 0;
			print_text(term, p, 0, strlen(m), m, COLOR_TITLE);
			/*set_window_title(0,m);*/
			/*set_terminal_title(term, m);*/
			mem_free(m);
		}
		m = stracpy("Links");
		if (ses->screen && ses->screen->f_data && ses->screen->f_data->title && ses->screen->f_data->title[0]) add_to_strn(&m, " - "), add_to_strn(&m, ses->screen->f_data->title);
		set_terminal_title(term, m);
		/* mem_free(m); -- set_terminal_title frees it */
	}
	redraw_from_window(ses->win);
}

void print_error_dialog(struct session *ses, struct status *stat, unsigned char *title)
{
	unsigned char *t = get_err_msg(stat->state);
	if (!t) return;
	msg_box(ses->term, NULL, title, AL_CENTER, t, ses, 1, TEXT_(T_CANCEL), NULL, B_ENTER | B_ESC/*, _("Retry"), NULL, 0 !!! FIXME: retry */);
}

void free_wtd(struct session *ses)
{
	if (!ses->wtd) {
		internal("no WTD");
		return;
	}
	if (ses->goto_position) mem_free(ses->goto_position), ses->goto_position = NULL;
	mem_free(ses->loading_url);
	ses->loading_url = NULL;
	ses->wtd = WTD_NO;
}

void abort_files_load(struct session *ses)
{
	struct file_to_load *ftl;
	int q;
	do {
		q = 0;
		foreach(ftl, ses->more_files) {
			if (ftl->stat.state >= 0 && ftl->req_sent) {
				q = 1;
				change_connection(&ftl->stat, NULL, PRI_CANCEL);
			}
		}
	} while (q);
}

void free_files(struct session *ses)
{
	struct file_to_load *ftl;
	abort_files_load(ses);
	foreach(ftl, ses->more_files) {
		if (ftl->ce) ftl->ce->refcount--;
		mem_free(ftl->url);
	}
	free_list(ses->more_files);
}

void destroy_location(struct location *loc)
{
	struct frame *frame;
	del_from_list(loc);
	foreach(frame, loc->frames) {
		destroy_vs(&frame->vs);
		mem_free(frame->name);
	}
	free_list(loc->frames);
	destroy_vs(&loc->vs);
	mem_free(loc);
}

void ses_forward(struct session *ses)
{
	struct location *l;
	size_t len;
	free_files(ses);
	if (!list_empty(ses->history)) {
		l = cur_loc(ses);
	}
	if (ses->search_word) mem_free(ses->search_word), ses->search_word = NULL;
	x:
	len = strlen(ses->loading_url);
	if (!list_empty(ses->history) && len < strlen(cur_loc(ses)->vs.url))
		len = strlen(cur_loc(ses)->vs.url);
	l = mem_alloc(sizeof(struct location) + len + 1);
	memset(l, 0, sizeof(struct location));
	memcpy(&l->stat, &ses->loading, sizeof(struct status));
	if (ses->wtd_target && *ses->wtd_target) {
		struct frame *frm;
		if (list_empty(ses->history)) {
			internal("no history");
			return;
		}
		copy_location(l, cur_loc(ses));
		add_to_list(ses->history, l);
		frm = ses_change_frame_url(ses, ses->wtd_target, ses->loading_url);
		if (!frm) {
			destroy_location(l);
			ses->wtd_target = NULL;
			goto x;
		}
		destroy_vs(&frm->vs);
		init_vs(&frm->vs, ses->loading_url);
		if (ses->goto_position) {
			if (frm->vs.goto_position) mem_free(frm->vs.goto_position);
			frm->vs.goto_position = ses->goto_position;
			ses->goto_position = NULL;
		}
		/*request_additional_loading_file(ses, ses->loading_url, &ses->loading, PRI_FRAME);*/
	} else {
		init_list(l->frames);
		init_vs(&l->vs, ses->loading_url);
		add_to_list(ses->history, l);
		if (ses->goto_position) {
			l->vs.goto_position = ses->goto_position;
			ses->goto_position = NULL;
		}
	}
}

void ses_imgmap(struct session *ses)
{
	struct cache_entry *ce;
	struct fragment *fr;
	struct memory_list *ml;
	struct menu_item *menu;
	struct f_data_c *fd;
	if (find_in_cache(ses->loading_url, &ce)) {
		internal("can't find cache entry");
		return;
	}
	ce->refcount--;
	defrag_entry(ce);
	fr = ce->frag.next;
	if ((void *)fr == &ce->frag) return;
	if (!(fd = current_frame(ses)) || !fd->f_data) return;
	d_opt = &fd->f_data->opt;
	if (get_image_map(ce->head, fr->data, fr->data + fr->length, ses->goto_position, &menu, &ml, ses->imgmap_href_base, ses->imgmap_target_base, ses->term->spec->charset, ses->ds.assume_cp, ses->ds.hard_assume))
		return;
	add_empty_window(ses->term, (void (*)(void *))freeml, ml);
	do_menu(ses->term, menu, ses);
}

void map_selected(struct terminal *term, struct link_def *ld, struct session *ses)
{
	goto_url_f(ses, ld->link, ld->target);
}

void ses_back(struct session *ses)
{
	struct location *loc;
	free_files(ses);
	loc = ses->history.next;
	if (ses->search_word) mem_free(ses->search_word), ses->search_word = NULL;
	if ((void *)loc == &ses->history) return;
	destroy_location(loc);
	loc = ses->history.next;
	if ((void *)loc == &ses->history) return;
	if (!strcmp(loc->vs.url, ses->loading_url)) return;
	destroy_location(loc);
	ses_forward(ses);
}

void end_load(struct status *, struct session *);
void doc_end_load(struct status *, struct session *);
void file_end_load(struct status *, struct file_to_load *);
void abort_loading(struct session *);
void abort_preloading(struct session *);

struct session *get_download_ses(struct download *down)
{
	struct session *ses;
	foreach(ses, sessions) if (ses == down->ses) return ses;
	if (!list_empty(sessions)) return sessions.next;
	return NULL;
}

void abort_download(struct download *down)
{
	if (down->win) delete_window(down->win);
	if (down->ask) delete_window(down->ask);
	if (down->stat.state >= 0) change_connection(&down->stat, NULL, PRI_CANCEL);
	mem_free(down->url);
	if (down->handle != -1) prealloc_truncate(down->handle, down->last_pos), close(down->handle);
	if (down->prog) {
		unlink(down->file);
		mem_free(down->prog);
	}
	mem_free(down->file);
	del_from_list(down);
	mem_free(down);
}

void kill_downloads_to_file(unsigned char *file)
{
	struct download *down;
	foreach(down, downloads) if (!strcmp(down->file, file)) down = down->prev, abort_download(down->next);
}

void undisplay_download(struct download *down)
{
	if (down->win) delete_window(down->win);
}

int dlg_abort_download(struct dialog_data *dlg, struct dialog_item_data *di)
{
	register_bottom_half((void (*)(void *))abort_download, dlg->dlg->udata);
	return 0;
}

int dlg_undisplay_download(struct dialog_data *dlg, struct dialog_item_data *di)
{
	register_bottom_half((void (*)(void *))undisplay_download, dlg->dlg->udata);
	return 0;
}

void download_abort_function(struct dialog_data *dlg)
{
	struct download *down = dlg->dlg->udata;
	down->win = NULL;
}

void download_window_function(struct dialog_data *dlg)
{
	struct download *down = dlg->dlg->udata;
	struct terminal *term = dlg->win->term;
	int max = 0, min = 0;
	int w, x, y;
	int t = 0;
	unsigned char *m, *u;
	struct status *stat = &down->stat;
	redraw_below_window(dlg->win);
	down->win = dlg->win;
	if (stat->state == S_TRANS && stat->prg->elapsed / 100) {
		int l = 0;
		m = init_str();
		t = 1;
		add_to_str(&m, &l, _(TEXT_(T_RECEIVED), term));
		add_to_str(&m, &l, " ");
		add_xnum_to_str(&m, &l, stat->prg->pos);
		if (stat->prg->size >= 0)
			add_to_str(&m, &l, " "), add_to_str(&m, &l, _(TEXT_(T_OF),term)), add_to_str(&m, &l, " "), add_xnum_to_str(&m, &l, stat->prg->size), add_to_str(&m, &l, " ");
		add_to_str(&m, &l, "\n");
		if (stat->prg->elapsed >= CURRENT_SPD_AFTER * SPD_DISP_TIME)
			add_to_str(&m, &l, _(TEXT_(T_AVERAGE_SPEED), term));
		else add_to_str(&m, &l, _(TEXT_(T_SPEED), term));
		add_to_str(&m, &l, " ");
		add_xnum_to_str(&m, &l, (longlong)stat->prg->loaded * 10 / (stat->prg->elapsed / 100));
		add_to_str(&m, &l, "/s");
		if (stat->prg->elapsed >= CURRENT_SPD_AFTER * SPD_DISP_TIME) 
			add_to_str(&m, &l, ", "), add_to_str(&m, &l, _(TEXT_(T_CURRENT_SPEED), term)), add_to_str(&m, &l, " "),
			add_xnum_to_str(&m, &l, stat->prg->cur_loaded / (CURRENT_SPD_SEC * SPD_DISP_TIME / 1000)),
			add_to_str(&m, &l, "/s");
		add_to_str(&m, &l, "\n");
		add_to_str(&m, &l, _(TEXT_(T_ELAPSED_TIME), term));
		add_to_str(&m, &l, " ");
		add_time_to_str(&m, &l, stat->prg->elapsed / 1000);
		if (stat->prg->size >= 0 && stat->prg->loaded > 0) {
			add_to_str(&m, &l, ", ");
			add_to_str(&m, &l, _(TEXT_(T_ESTIMATED_TIME), term));
			add_to_str(&m, &l, " ");
			/*add_time_to_str(&m, &l, stat->prg->elapsed / 1000 * stat->prg->size / stat->prg->loaded * 1000 - stat->prg->elapsed);*/
			add_time_to_str(&m, &l, (stat->prg->size - stat->prg->pos) / ((longlong)stat->prg->loaded * 10 / (stat->prg->elapsed / 100)));
		}
	} else m = stracpy(_(get_err_msg(stat->state), term));
	u = stracpy(down->url);
	if (strchr(u, POST_CHAR)) *strchr(u, POST_CHAR) = 0;
	max_text_width(term, u, &max);
	min_text_width(term, u, &min);
	max_text_width(term, m, &max);
	min_text_width(term, m, &min);
	max_buttons_width(term, dlg->items, dlg->n, &max);
	min_buttons_width(term, dlg->items, dlg->n, &min);
	w = dlg->win->term->x * 9 / 10 - 2 * DIALOG_LB;
	if (w < min) w = min;
	if (w > dlg->win->term->x - 2 * DIALOG_LB) w = dlg->win->term->x - 2 * DIALOG_LB;
	if (t && stat->prg->size >= 0) {
		if (w < DOWN_DLG_MIN) w = DOWN_DLG_MIN;
	} else {
		if (w > max) w = max;
	}
	if (w < 1) w = 1;
	y = 0;
	dlg_format_text(NULL, term, u, 0, &y, w, NULL, COLOR_DIALOG_TEXT, AL_LEFT);
	y++;
	if (t && stat->prg->size >= 0) y += 2;
	dlg_format_text(NULL, term, m, 0, &y, w, NULL, COLOR_DIALOG_TEXT, AL_LEFT);
	y++;
	dlg_format_buttons(NULL, term, dlg->items, dlg->n, 0, &y, w, NULL, AL_CENTER);
	dlg->xw = w + 2 * DIALOG_LB;
	dlg->yw = y + 2 * DIALOG_TB;
	center_dlg(dlg);
	draw_dlg(dlg);
	y = dlg->y + DIALOG_TB + 1;
	x = dlg->x + DIALOG_LB;
	dlg_format_text(term, term, u, x, &y, w, NULL, COLOR_DIALOG_TEXT, AL_LEFT);
	if (t && stat->prg->size >= 0) {
		unsigned char q[64];
		int p = w - 6;
		y++;
		set_only_char(term, x, y, '[');
		set_only_char(term, x + w - 5, y, ']');
		fill_area(term, x + 1, y, (int)((longlong)p * (longlong)stat->prg->pos / (longlong)stat->prg->size), 1, COLOR_DIALOG_METER);
		sprintf(q, "%3d%%", (int)((longlong)100 * (longlong)stat->prg->pos / (longlong)stat->prg->size));
		print_text(term, x + w - 4, y, strlen(q), q, COLOR_DIALOG_TEXT);
		y++;
	}
	y++;
	dlg_format_text(term, term, m, x, &y, w, NULL, COLOR_DIALOG_TEXT, AL_LEFT);
	y++;
	dlg_format_buttons(term, term, dlg->items, dlg->n, x, &y, w, NULL, AL_CENTER);
	mem_free(u);
	mem_free(m);
}

void display_download(struct terminal *term, struct download *down, struct session *ses)
{
	struct dialog *dlg;
	struct download *dd;
	foreach(dd, downloads) if (dd == down) goto found;
	return;
	found:
	dlg = mem_alloc(sizeof(struct dialog) + 3 * sizeof(struct dialog_item));
	memset(dlg, 0, sizeof(struct dialog) + 3 * sizeof(struct dialog_item));
	undisplay_download(down);
	down->ses = ses;
	dlg->title = TEXT_(T_DOWNLOAD);
	dlg->fn = download_window_function;
	dlg->abort = download_abort_function;
	dlg->udata = down;
	dlg->align = AL_CENTER;
	dlg->items[0].type = D_BUTTON;
	dlg->items[0].gid = B_ENTER | B_ESC;
	dlg->items[0].fn = dlg_undisplay_download;
	dlg->items[0].text = TEXT_(T_BACKGROUND);
	dlg->items[1].type = D_BUTTON;
	dlg->items[1].gid = 0;
	dlg->items[1].fn = dlg_abort_download;
	dlg->items[1].text = TEXT_(T_ABORT);
	dlg->items[2].type = D_END;
	do_dialog(term, dlg, getml(dlg, NULL));
}

time_t parse_http_date(const char *date)	/* this functions is bad !!! */
{
	const char *months[12] =
		{"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		 "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

	time_t t = 0;
	/* Mon, 03 Jan 2000 21:29:33 GMT */
	struct tm tm;
	if (!date || strlen(date) < 28) return 0;
	date += 5;
	tm.tm_mday = (date[0] - '0') * 10 + date[1] - '0';
	date += 3;
	for (tm.tm_mon = 0; tm.tm_mon < 12; tm.tm_mon++)
		if (!strncmp(date, months[tm.tm_mon], 3)) break;
	date += 4;
	tm.tm_year = (date[0] - '0') * 1000 + (date[1] - '0') * 100 + (date[2] - '0') * 10 + date[3] - '0' - 1900;
	date += 5;
	tm.tm_hour = (date[0] - '0') * 10 + date[1] - '0';
	date += 3;
	tm.tm_min = (date[0] - '0') * 10 + date[1] - '0';
	date += 3;
	tm.tm_sec = (date[0] - '0') * 10 + date[1] - '0';
	t = mktime(&tm);
	if (t == (time_t) - 1) return 0;
	else return t;
}

void download_data(struct status *stat, struct download *down)
{
	struct cache_entry *ce;
	struct fragment *frag;
	if (stat->state >= S_WAIT && stat->state < S_TRANS) goto end_store;
	if (!(ce = stat->ce)) goto end_store;
	if (ce->last_modified)
	down->remotetime = parse_http_date(ce->last_modified);
/*	  fprintf(stderr,"\nFEFE date %s\n",ce->last_modified); */
	if (ce->redirect && down->redirect_cnt++ < MAX_REDIRECTS) {
		unsigned char *u, *p, *pos;
		if (stat->state >= 0) change_connection(&down->stat, NULL, PRI_CANCEL);
		u = join_urls(down->url, ce->redirect);
		if (!u) goto x;
		if ((pos = extract_position(u))) mem_free(pos);
		if (!http_bugs.bug_302_redirect) if (!ce->redirect_get && (p = strchr(down->url, POST_CHAR))) add_to_strn(&u, p);
		mem_free(down->url);
		down->url = u;
		down->stat.state = S_WAIT_REDIR;
		if (down->win) {
			struct event ev = { EV_REDRAW, 0, 0, 0 };
			ev.x = down->win->term->x;
			ev.y = down->win->term->y;
			down->win->handler(down->win, &ev, 0);
		}
		/*if (!strchr(down->url, POST_CHAR)) {*/
			load_url(down->url, &down->stat, PRI_DOWNLOAD, down->redirect_cnt < MAX_CACHED_REDIRECTS ? NC_CACHE : NC_RELOAD);
			return;
		/*} else {
			unsigned char *msg = init_str();
			int l = 0;
			add_bytes_to_str(&msg, &l, down->url, (unsigned char *)strchr(down->url, POST_CHAR) - down->url);
			msg_box(get_download_ses(down)->term, getml(msg, NULL), TEXT_(T_WARNING), AL_CENTER | AL_EXTD_TEXT, TEXT_(T_DO_YOU_WANT_TO_FOLLOW_REDIRECT_AND_POST_FORM_DATA_TO_URL), "", msg, "?", NULL, down, 3, TEXT_(T_YES), down_post_yes, B_ENTER, TEXT_(T_NO), down_post_no, 0, TEXT_(T_CANCEL), down_post_cancel, B_ESC);
		}*/
	}
	x:
	foreach(frag, ce->frag) while (frag->offset <= down->last_pos && frag->offset + frag->length > down->last_pos) {
		int w;
#ifdef HAVE_OPEN_PREALLOC
		if (!down->last_pos && (!down->stat.prg || down->stat.prg->size > 0) && can_prealloc(down->file)) {
			struct stat st;
			if (fstat(down->handle, &st) || !S_ISREG(st.st_mode)) goto skip_prealloc;
			close(down->handle);
			down->handle = open_prealloc(down->file, O_CREAT|O_WRONLY|O_TRUNC, 0666, down->stat.prg ? down->stat.prg->size : ce->length);
			if (down->handle == -1) goto error;
			set_bin(down->handle);
			skip_prealloc:;
		}
#endif
		w = write(down->handle, frag->data + (down->last_pos - frag->offset), frag->length - (down->last_pos - frag->offset));
		if (w == -1) {
#ifdef HAVE_OPEN_PREALLOC
			error:
#endif
			if (!list_empty(sessions)) {
				unsigned char *emsg = stracpy(strerror(errno));
				unsigned char *msg = stracpy(down->file);
				msg_box(get_download_ses(down)->term, getml(msg, emsg, NULL), TEXT_(T_DOWNLOAD_ERROR), AL_CENTER | AL_EXTD_TEXT, TEXT_(T_COULD_NOT_WRITE_TO_FILE), " ", msg, ": ", emsg, NULL, NULL, 1, TEXT_(T_CANCEL), NULL, B_ENTER | B_ESC);
			}
			detach_connection(stat, down->last_pos);
			abort_download(down);
			return;
		}
		down->last_pos += w;
	}
	detach_connection(stat, down->last_pos);
	end_store:;
	if (stat->state < 0) {
		if (stat->state != S_OK) {
			unsigned char *t = get_err_msg(stat->state);
			if (t) {
				unsigned char *tt = stracpy(down->url);
				if (strchr(tt, POST_CHAR)) *strchr(tt, POST_CHAR) = 0;
				msg_box(get_download_ses(down)->term, getml(tt, NULL), TEXT_(T_DOWNLOAD_ERROR), AL_CENTER | AL_EXTD_TEXT, TEXT_(T_ERROR_DOWNLOADING), " ", tt, ":\n\n", t, NULL, get_download_ses(down), 1, TEXT_(T_CANCEL), NULL, B_ENTER | B_ESC/*, TEXT_(T_RETRY), NULL, 0 !!! FIXME: retry */);
			}
		} else {
			if (down->prog) {
				prealloc_truncate(down->handle, down->last_pos);
				close(down->handle), down->handle = -1;
				exec_on_terminal(get_download_ses(down)->term, down->prog, down->file, !!down->prog_flags);
				mem_free(down->prog), down->prog = NULL;
			} else if (down->remotetime && download_utime) {
				struct utimbuf foo;
				foo.actime = foo.modtime = down->remotetime;
				utime(down->file, &foo);
			}
		}
		abort_download(down);
		return;
	}
	if (down->win) {
		struct event ev = { EV_REDRAW, 0, 0, 0 };
		ev.x = down->win->term->x;
		ev.y = down->win->term->y;
		down->win->handler(down->win, &ev, 0);
	}
}

int create_download_file(struct terminal *term, unsigned char *fi, int safe)
{
	unsigned char *file = fi;
	unsigned char *wd = NULL;
	int h;
	int i;
#ifdef NO_FILE_SECURITY
	int sf = 0;
#else
	int sf = safe;
#endif
	if (!safe) {
		wd = get_cwd();
		set_cwd(term->cwd);
		if (file[0] == '~' && dir_sep(file[1])) {
			unsigned char *h = getenv("HOME");
			if (h) {
				int fl = 0;
				file = init_str();
				add_to_str(&file, &fl, h);
				add_to_str(&file, &fl, fi + 1);
			}
		}
	}
	h = open(file, O_CREAT|O_WRONLY|O_TRUNC|(safe?O_EXCL:0), sf ? 0600 : 0666);
	if (h == -1) {
		unsigned char *msg, *msge;
		int errn = errno;
		if (errn == EEXIST && safe) {
			h = -2;
			goto x;
		}
		msg = stracpy(file);
		msge = stracpy(strerror(errn));
		msg_box(term, getml(msg, msge, NULL), TEXT_(T_DOWNLOAD_ERROR), AL_CENTER | AL_EXTD_TEXT, TEXT_(T_COULD_NOT_CREATE_FILE), " ", msg, ": ", msge, NULL, NULL, 1, TEXT_(T_CANCEL), NULL, B_ENTER | B_ESC);
		goto x;
	}
	set_bin(h);
	if (safe) goto x;
	if (strlen(file) >= MAX_STR_LEN) memcpy(download_dir, file, MAX_STR_LEN - 1), download_dir[MAX_STR_LEN - 1] = 0;
	else strcpy(download_dir, file);
	for (i = strlen(download_dir) - 1; i >= 0; i--) if (dir_sep(download_dir[i])) {
		download_dir[i + 1] = 0;
		goto x;
	}
	download_dir[0] = 0;
	x:
	if (file != fi) mem_free(file);
	if (wd) set_cwd(wd), mem_free(wd);
	return h;
}

unsigned char *get_temp_name(unsigned char *url)
{
	int l, nl;
	unsigned char *name, *fn, *fnn, *fnnn, *s;
	unsigned char *nm;
	unsigned char *directory = NULL;
#ifdef WIN32
	directory = getenv("TMP");
	if (!directory) directory = getenv("TEMP");
#endif
	nm = tempnam(directory, "links");
	if (!nm) return NULL;
#ifdef OS2
	if (strlen(nm) > 4 && !strcasecmp(nm + strlen(nm) - 4, ".tmp")) nm[strlen(nm) - 4] = 0;
#endif
	name = init_str();
	nl = 0;
	add_to_str(&name, &nl, nm);
	free(nm);
	get_filename_from_url(url, &fn, &l);
	fnnn = NULL;
	for (fnn = fn; fnn < fn + l; fnn++) if (*fnn == '.') fnnn = fnn;
	if (fnnn) {
		s = memacpy(fnnn, l - (fnnn - fn));
		check_shell_security(&s);
		add_to_str(&name, &nl, s);
		mem_free(s);
	}
	return name;
}

unsigned char *subst_file(unsigned char *prog, unsigned char *file, int cyg_subst)
{
	unsigned char *orig_prog = prog;
	unsigned char *nn;
	unsigned char *n = init_str();
	int l = 0;
	while (*prog) {
		int p;
		for (p = 0; prog[p] && prog[p] != '%'; p++)
			;
		add_bytes_to_str(&n, &l, prog, p);
		prog += p;
		if (*prog == '%') {
			if (cyg_subst) {
				unsigned char *conv = os_conv_to_external_path(file, orig_prog);
				add_to_str(&n, &l, conv);
				mem_free(conv);
			} else {
				add_to_str(&n, &l, file);
			}
			prog++;
		}
	}
	nn = os_fixup_external_program(n);
	mem_free(n);
	return nn;
}

void start_download(struct session *ses, unsigned char *file)
{
	struct download *down;
	int h;
	unsigned char *url = ses->dn_url;
	unsigned char *pos;
	if (!url) return;
	if ((pos = extract_position(url))) mem_free(pos);
	kill_downloads_to_file(file);
	if ((h = create_download_file(ses->term, file, 0)) < 0) return;
	down = mem_alloc(sizeof(struct download));
	memset(down, 0, sizeof(struct download));
	down->url = stracpy(url);
	down->stat.end = (void (*)(struct status *, void *))download_data;
	down->stat.data = down;
	down->last_pos = 0;
	down->file = stracpy(file);
	down->handle = h;
	down->ses = ses;
	down->remotetime = 0;
	add_to_list(downloads, down);
	load_url(url, &down->stat, PRI_DOWNLOAD, NC_CACHE);
	display_download(ses->term, down, ses);
}

void tp_cancel(struct session *);

void tp_free(struct session *);

void continue_download(struct session *ses, unsigned char *file)
{
	struct download *down;
	int h;
	int namecount = 0;
	unsigned char *url = ses->tq_url;
	if (!url) return;
	if (ses->tq_prog) {
		new_name:
		if (!(file = get_temp_name(url))) {
			tp_cancel(ses);
			return;
		}
	}
	kill_downloads_to_file(file);
	if (!ses->tq_prog) kill_downloads_to_file(file);
	if ((h = create_download_file(ses->term, file, !!ses->tq_prog)) < 0) {
		if (h == -2 && ses->tq_prog) {
			if (++namecount < DOWNLOAD_NAME_TRIES) {
				mem_free(file);
				goto new_name;
			}
			msg_box(ses->term, NULL, TEXT_(T_DOWNLOAD_ERROR), AL_CENTER | AL_EXTD_TEXT, TEXT_(T_COULD_NOT_CREATE_TEMPORARY_FILE), NULL, NULL, 1, TEXT_(T_CANCEL), NULL, B_ENTER | B_ESC);
		}
		if (ses->tq_prog) mem_free(file);
		tp_cancel(ses);
		return;
	}
	down = mem_alloc(sizeof(struct download));
	memset(down, 0, sizeof(struct download));
	down->url = stracpy(url);
	down->stat.end = (void (*)(struct status *, void *))download_data;
	down->stat.data = down;
	down->last_pos = 0;
	down->file = stracpy(file);
	down->handle = h;
	down->ses = ses;
	if (ses->tq_prog) {
		down->prog = subst_file(ses->tq_prog, file, 1);
		mem_free(file);
		mem_free(ses->tq_prog);
		ses->tq_prog = NULL;
	}
	down->prog_flags = ses->tq_prog_flags;
	add_to_list(downloads, down);
	change_connection(&ses->tq, &down->stat, PRI_DOWNLOAD);
	tp_free(ses);
	display_download(ses->term, down, ses);
}

void tp_free(struct session *ses)
{
	ses->tq_ce->refcount--;
	mem_free(ses->tq_url);
	ses->tq_url = NULL;
	if (ses->tq_goto_position) mem_free(ses->tq_goto_position), ses->tq_goto_position = NULL;
	ses->tq_ce = NULL;
}

void tp_cancel(struct session *ses)
{
	change_connection(&ses->tq, NULL, PRI_CANCEL);
	tp_free(ses);
}

void tp_save(struct session *ses)
{
	if (ses->tq_prog) mem_free(ses->tq_prog), ses->tq_prog = NULL;
	query_file(ses, ses->tq_url, continue_download, tp_cancel);
}

void tp_open(struct session *ses)
{
	continue_download(ses, "");
}

void display_timer(struct session *);

void tp_display(struct session *ses)	/* !!! FIXME: frames */
{
	struct location *l;
	l = mem_alloc(sizeof(struct location) + strlen(ses->tq_url));
	memset(l, 0, sizeof(struct location));
	init_list(l->frames);
	memcpy(&l->stat, &ses->tq, sizeof(struct status));
	init_vs(&l->vs, ses->tq_url);
	if (ses->tq_goto_position) {
		l->vs.goto_position = ses->tq_goto_position;
		ses->tq_goto_position = NULL;
	}
	add_to_list(ses->history, l);
	cur_loc(ses)->stat.end = (void (*)(struct status *, void *))doc_end_load;
	cur_loc(ses)->stat.data = ses;
	if (ses->tq.state >= 0) change_connection(&ses->tq, &cur_loc(ses)->stat, PRI_MAIN);
	else cur_loc(ses)->stat.state = ses->tq.state;
	cur_loc(ses)->vs.plain = 1;
	display_timer(ses);
	tp_free(ses);
}

void type_query(struct session *ses, struct cache_entry *ce, unsigned char *ct, struct assoc *a)
{
	unsigned char *m1;
	unsigned char *m2;
	if (ses->tq_prog) mem_free(ses->tq_prog), ses->tq_prog = NULL;
	if (a) ses->tq_prog = stracpy(a->prog), ses->tq_prog_flags = a->block;
	if (a && !a->ask) {
		tp_open(ses);
		return;
	}
	m1 = stracpy(ct);
	if (!a) {
		if (!anonymous) msg_box(ses->term, getml(m1, NULL), TEXT_(T_UNKNOWN_TYPE), AL_CENTER | AL_EXTD_TEXT, TEXT_(T_CONTEN_TYPE_IS), " ", m1, ".\n", TEXT_(T_DO_YOU_WANT_TO_SAVE_OR_DISLPAY_THIS_FILE), NULL, ses, 3, TEXT_(T_SAVE), tp_save, B_ENTER, TEXT_(T_DISPLAY), tp_display, 0, TEXT_(T_CANCEL), tp_cancel, B_ESC);
		else msg_box(ses->term, getml(m1, NULL), TEXT_(T_UNKNOWN_TYPE), AL_CENTER | AL_EXTD_TEXT, TEXT_(T_CONTEN_TYPE_IS), " ", m1, ".\n", TEXT_(T_DO_YOU_WANT_TO_SAVE_OR_DISLPAY_THIS_FILE), NULL, ses, 2, TEXT_(T_DISPLAY), tp_display, B_ENTER, TEXT_(T_CANCEL), tp_cancel, B_ESC);
	} else {
		m2 = stracpy(a->label ? a->label : (unsigned char *)"");
		if (!anonymous) msg_box(ses->term, getml(m1, m2, NULL), TEXT_(T_WHAT_TO_DO), AL_CENTER | AL_EXTD_TEXT, TEXT_(T_CONTEN_TYPE_IS), " ", m1, ".\n", TEXT_(T_DO_YOU_WANT_TO_OPEN_FILE_WITH), " ", m2, ", ", TEXT_(T_SAVE_IT_OR_DISPLAY_IT), NULL, ses, 4, TEXT_(T_OPEN), tp_open, B_ENTER, TEXT_(T_SAVE), tp_save, 0, TEXT_(T_DISPLAY), tp_display, 0, TEXT_(T_CANCEL), tp_cancel, B_ESC);
		else msg_box(ses->term, getml(m1, m2, NULL), TEXT_(T_WHAT_TO_DO), AL_CENTER | AL_EXTD_TEXT, TEXT_(T_CONTEN_TYPE_IS), " ", m1, ".\n", TEXT_(T_DO_YOU_WANT_TO_OPEN_FILE_WITH), " ", m2, ", ", TEXT_(T_SAVE_IT_OR_DISPLAY_IT), NULL, ses, 3, TEXT_(T_OPEN), tp_open, B_ENTER, TEXT_(T_DISPLAY), tp_display, 0, TEXT_(T_CANCEL), tp_cancel, B_ESC);
	}
}

int ses_chktype(struct session *ses, struct status **stat, struct cache_entry *ce)
{
	struct assoc *a;
	unsigned char *ct;
	int r = 0;
	if (!(ct = get_content_type(ce->head, ce->url))) goto f;
	if (is_html_type(ct)) goto ff;
	r = 1;
	if (!strcasecmp(ct, "text/plain") || !strcasecmp(ct, "file/txt")) goto ff;
	if (!(a = get_type_assoc(ses->term, ct)) && strlen(ct) >= 4 && !casecmp(ct, "text", 4)) goto ff;
	if (ses->tq_url) internal("type query to %s already in prgress", ses->tq_url);
	ses->tq_url = stracpy(ses->loading_url);
	change_connection(&ses->loading, *stat = &ses->tq, PRI_MAIN);
	(ses->tq_ce = ce)->refcount++;
	if (ses->tq_goto_position) mem_free(ses->tq_goto_position);
	ses->tq_goto_position = stracpy(ses->goto_position);
	type_query(ses, ce, ct, a);
	mem_free(ct);
	return 1;

	ff:
	mem_free(ct);
	f:
	if (ses->wtd_target && r) *ses->wtd_target = 0;
	ses_forward(ses);
	cur_loc(ses)->vs.plain = r;
	return 0;
}

struct wtd_data {
	struct session *ses;
	unsigned char *url;
	int pri;
	int cache;
	int wtd;
	unsigned char *target;
	unsigned char *pos;
	void (*fn)(struct status *, struct session *);
};

void post_yes(struct wtd_data *w)
{
	abort_preloading(w->ses);
	if (w->ses->goto_position) mem_free(w->ses->goto_position);
	w->ses->goto_position = stracpy(w->pos);
	w->ses->loading.end = (void (*)(struct status *, void *))w->fn;
	w->ses->loading.data = w->ses;
	w->ses->loading_url = stracpy(w->url);
	w->ses->wtd = w->wtd;
	w->ses->wtd_target = w->target;
	load_url(w->ses->loading_url, &w->ses->loading, w->pri, w->cache);
}

void post_no(struct wtd_data *w)
{
	*strchr(w->url, POST_CHAR) = 0;
	post_yes(w);
}

void post_cancel(struct wtd_data *w)
{
	reload(w->ses, NC_CACHE);
}

void ses_goto(struct session *ses, unsigned char *url, unsigned char *target, int pri, int cache, int wtd, unsigned char *pos, void (*fn)(struct status *, struct session *), int redir)
{
	struct wtd_data *w;
	unsigned char *m1, *m2;
	struct cache_entry *e;
	e = NULL;
	if (!find_in_cache(url, &e)) e->refcount--;
	if (!strchr(url, POST_CHAR) || (cache == NC_ALWAYS_CACHE && e && !e->incomplete)) {
		if (ses->goto_position) mem_free(ses->goto_position);
		ses->goto_position = pos;
		ses->loading.end = (void (*)(struct status *, void *))fn;
		ses->loading.data = ses;
		ses->loading_url = url;
		ses->wtd = wtd;
		ses->wtd_target = target;
		load_url(url, &ses->loading, pri, cache);
		return;
	}
	w = mem_alloc(sizeof(struct wtd_data));
	w->ses = ses;
	w->url = url;
	w->pri = pri;
	w->cache = cache;
	w->wtd = wtd;
	w->target = target;
	w->pos = pos;
	w->fn = fn;
	if (redir) m1 = TEXT_(T_DO_YOU_WANT_TO_FOLLOW_REDIRECT_AND_POST_FORM_DATA_TO_URL);
	else if (wtd == WTD_FORWARD) m1 = TEXT_(T_DO_YOU_WANT_TO_POST_FORM_DATA_TO_URL);
	else m1 = TEXT_(T_DO_YOU_WANT_TO_REPOST_FORM_DATA_TO_URL);
	m2 = memacpy(url, (unsigned char *)strchr(url, POST_CHAR) - url);
	msg_box(ses->term, getml(m2, w, w->url, w->pos, NULL), TEXT_(T_WARNING), AL_CENTER | AL_EXTD_TEXT, m1, " ", m2, "?", NULL, w, 3, TEXT_(T_YES), post_yes, B_ENTER, TEXT_(T_NO), post_no, 0, TEXT_(T_CANCEL), post_cancel, B_ESC);
}

int do_move(struct session *ses, struct status **stat)
{
	struct cache_entry *ce = NULL;
	if (!ses->loading_url) {
		internal("no ses->loading_url");
		return 0;
	}
	if (!(ce = (*stat)->ce) || (ses->wtd == WTD_IMGMAP && (*stat)->state >= 0)) {
		return 0;
	}
	if (ce->redirect && ses->redirect_cnt++ < MAX_REDIRECTS) {
		unsigned char *u, *p, *gp, *pos;
		int w = ses->wtd;
		if (ses->wtd == WTD_BACK && (void *)cur_loc(ses)->next == &ses->history)
			goto b;
		if (!(u = join_urls(ses->loading_url, ce->redirect))) goto b;
		if (!http_bugs.bug_302_redirect) if (!ce->redirect_get && (p = strchr(ses->loading_url, POST_CHAR))) add_to_strn(&u, p);
		/* ^^^^ According to RFC2068 POST must not be redirected to GET,
			but some BUGGY message boards rely on it :-( */
		gp = stracpy(ses->goto_position);
		if ((pos = extract_position(u))) {
			if (!gp) gp = pos;
			else mem_free(pos);
		}
		abort_loading(ses);
		if (!list_empty(ses->history)) *stat = &cur_loc(ses)->stat;
		else *stat = NULL;
		if (w == WTD_FORWARD || w == WTD_IMGMAP) {
			ses_goto(ses, u, ses->wtd_target, PRI_MAIN, ses->redirect_cnt < MAX_CACHED_REDIRECTS ? NC_CACHE : NC_RELOAD, w, gp, end_load, 1);
			return 2;
		}
		if (gp) mem_free(gp);
		if (w == WTD_BACK) {
			ses_goto(ses, u, NULL, PRI_MAIN, ses->redirect_cnt < MAX_CACHED_REDIRECTS ? NC_CACHE : NC_RELOAD, WTD_RELOAD, NULL, end_load, 1);
			return 2;
		}
		if (w == WTD_RELOAD) {
			ses_goto(ses, u, NULL, PRI_MAIN, ses->redirect_cnt < MAX_CACHED_REDIRECTS && ses->reloadlevel < NC_RELOAD ? ses->reloadlevel : NC_RELOAD, WTD_RELOAD, NULL, end_load, 1);
			return 2;
		}
	}
	b:
	if (ses->display_timer != -1) kill_timer(ses->display_timer), ses->display_timer = -1;
	if (ses->wtd == WTD_FORWARD) {
		if (ses_chktype(ses, stat, ce)) {
			free_wtd(ses);
			reload(ses, NC_CACHE);
			return 2;
		}
	}
	if (ses->wtd == WTD_IMGMAP) ses_imgmap(ses);
	if (ses->wtd == WTD_BACK) ses_back(ses);
	if (ses->wtd == WTD_RELOAD) ses_back(ses), ses_forward(ses);
	if ((*stat)->state >= 0) change_connection(&ses->loading, *stat = &cur_loc(ses)->stat, PRI_MAIN);
	else cur_loc(ses)->stat.state = ses->loading.state;
	free_wtd(ses);
	return 1;
}

void request_frameset(struct session *, struct frameset_desc *);

void request_frame(struct session *ses, unsigned char *name, unsigned char *uurl)
{
	struct location *loc = cur_loc(ses);
	struct frame *frm;
	unsigned char *url, *pos;
	if (list_empty(ses->history)) {
		internal("request_frame: no location");
		return;
	}
	foreach(frm, loc->frames) if (!strcasecmp(frm->name, name)) {
		struct f_data_c *fd;
		foreach(fd, ses->scrn_frames) if (fd->vs == &frm->vs && fd->f_data->frame_desc) {
			request_frameset(ses, fd->f_data->frame_desc);
			return;
		}
		url = stracpy(frm->vs.url);
		/*
		if (frm->vs.f && frm->vs.f->f_data && frm->vs.f->f_data->frame) {
			request_frameset(ses, frm->vs.f->f_data->frame_desc);
			mem_free(url);
			return;
		}*/
		goto found;
	}
	url = stracpy(uurl);
	pos = extract_position(url);
	frm = mem_alloc(sizeof(struct frame) + strlen(url) + 1);
	memset(frm, 0, sizeof(struct frame));
	if (!(frm->name = stracpy(name))) {
		mem_free(frm);
		mem_free(url);
		if (pos) mem_free(pos);
		return;
	}
	init_vs(&frm->vs, url);
	if (pos) frm->vs.goto_position = pos;
	add_to_list(loc->frames, frm);
	found:
	if (*url) request_additional_file(ses, url, PRI_FRAME);
	mem_free(url);
}

void request_frameset(struct session *ses, struct frameset_desc *fd)
{
	int i;
	static int depth = 0;
	if (++depth <= HTML_MAX_FRAME_DEPTH) {
		for (i = 0; i < fd->n; i++) {
			if (fd->f[i].subframe) request_frameset(ses, fd->f[i].subframe);
			else if (fd->f[i].name) request_frame(ses, fd->f[i].name, fd->f[i].url);
		}
	}
	depth--;
}

void load_frames(struct session *ses, struct f_data_c *fd)
{
	struct f_data *ff = fd->f_data;
	if (!ff || !ff->frame) return;
	request_frameset(ses, ff->frame_desc);
}

void display_timer(struct session *ses)
{
	ttime t = get_time();
	html_interpret(ses);
	draw_formatted(ses);
	t = (get_time() - t) * DISPLAY_TIME;
	if (t < DISPLAY_TIME_MIN) t = DISPLAY_TIME_MIN;
	ses->display_timer = install_timer(t, (void (*)(void *))display_timer, ses);
	load_frames(ses, ses->screen);
	process_file_requests(ses);
}

void end_load(struct status *stat, struct session *ses)
{
	int d;
	if (!ses->wtd) {
		internal("end_load: !ses->wtd");
		return;
	}
	d = do_move(ses, &stat);
	if (!stat) return;
	if (d == 1) {
		stat->end = (void (*)(struct status *, void *))doc_end_load;
		display_timer(ses);
	}
	if (stat->state < 0) {
		if (d != 2 && ses->wtd) {
			free_wtd(ses);
		}
		if (d == 1) doc_end_load(stat, ses);
	}
	if (stat->state < 0 && stat->state != S_OK && d != 2 && d != 1) {
		print_error_dialog(ses, stat, TEXT_(T_ERROR));
		if (!d) reload(ses, NC_CACHE);
	}
	print_screen_status(ses);
}

void doc_end_load(struct status *stat, struct session *ses)
{
	if (stat->state < 0) {
		if (ses->display_timer != -1) kill_timer(ses->display_timer), ses->display_timer = -1;
		html_interpret(ses);
		draw_formatted(ses);
		load_frames(ses, ses->screen);
		process_file_requests(ses);
		if (stat->state != S_OK) print_error_dialog(ses, stat, TEXT_(T_ERROR));
	} else if (ses->display_timer == -1) display_timer(ses);
	print_screen_status(ses);
}

void file_end_load(struct status *stat, struct file_to_load *ftl)
{
	if (ftl->stat.ce) {
		if (ftl->ce) ftl->ce->refcount--;
		(ftl->ce = ftl->stat.ce)->refcount++;
	}
	doc_end_load(stat, ftl->ses);
}

struct file_to_load *request_additional_file(struct session *ses, unsigned char *url, int pri)
{
	struct file_to_load *ftl;
	foreach(ftl, ses->more_files) {
		if (!strcmp(ftl->url, url)) {
			if (ftl->pri > pri) {
				ftl->pri = pri;
				change_connection(&ftl->stat, &ftl->stat, pri);
			}
			return NULL;
		}
	}
	ftl = mem_alloc(sizeof(struct file_to_load));
	ftl->url = stracpy(url);
	ftl->stat.end = (void (*)(struct status *, void *))file_end_load;
	ftl->stat.data = ftl;
	ftl->req_sent = 0;
	ftl->pri = pri;
	ftl->ce = NULL;
	ftl->ses = ses;
	add_to_list(ses->more_files, ftl);
	return ftl;
}

struct file_to_load *request_additional_loading_file(struct session *ses, unsigned char *url, struct status *stat, int pri)
{
	struct file_to_load *ftl;
	if (!(ftl = request_additional_file(ses, url, pri))) {
		change_connection(stat, NULL, PRI_CANCEL);
		return NULL;
	}
	ftl->req_sent = 1;
	ftl->ce = stat->ce;
	change_connection(stat, &ftl->stat, pri);
	return ftl;
}

void process_file_requests(struct session *ses)
{
	static int stop_recursion = 0;
	struct file_to_load *ftl;
	int more;
	if (stop_recursion) return;
	stop_recursion = 1;
	do {
		more = 0;
		foreach(ftl, ses->more_files) if (!ftl->req_sent) {
			ftl->req_sent = 1;
			load_url(ftl->url, &ftl->stat, ftl->pri, NC_CACHE);
			more = 1;
		}
	} while (more);
	stop_recursion = 0;
}

struct session *create_session(struct window *win)
{
	struct terminal *term = win->term;
	struct session *ses;
	ses = mem_alloc(sizeof(struct session));
	memset(ses, 0, sizeof(struct session));
	init_list(ses->history);
	init_list(ses->scrn_frames);
	init_list(ses->more_files);
	ses->term = term;
	ses->win = win;
	ses->id = session_id++;
	ses->screen = NULL;
	ses->wtd = WTD_NO;
	ses->display_timer = -1;
	ses->loading_url = NULL;
	ses->goto_position = NULL;
	memcpy(&ses->ds, &dds, sizeof(struct document_setup));
	add_to_list(sessions, ses);
	if (first_use) {
		first_use = 0;
		msg_box(term, NULL, TEXT_(T_WELCOME), AL_CENTER | AL_EXTD_TEXT, TEXT_(T_WELCOME_TO_LINKS), "\n\n", TEXT_(T_BASIC_HELP), NULL, NULL, 1, TEXT_(T_OK), NULL, B_ENTER | B_ESC);
	}
	return ses;
}

void copy_session(struct session *old, struct session *new)
{
	/*struct location *l;
	foreachback(l, old->history) {
		struct location *nl;
		struct frame *frm;
		nl = mem_alloc(sizeof(struct location) + strlen(l->vs.url) + 1);
		memcpy(nl, l, sizeof(struct location) + strlen(l->vs.url) + 1);
		init_list(nl->frames);
		foreachback(frm, l->frames) {
			struct frame *nfrm;
			nfrm = mem_alloc(sizeof(struct frame) + strlen(frm->vs.url) + 1);
			memcpy(nfrm, frm, sizeof(struct frame) + strlen(frm->vs.url) + 1);
			add_to_list(nl->frames, nfrm);
		}
		add_to_list(new->history, nl);
	}*/
	if (!list_empty(old->history)) {
		goto_url(new, cur_loc(old)->vs.url);
	}
}

void *create_session_info(int cp, unsigned char *url, int *ll)
{
	int l = strlen(url);
	int *i;
	*ll = 2 * sizeof(int) + l;
	i = mem_alloc(2 * sizeof(int) + l);
	i[0] = cp;
	i[1] = l;
	memcpy(i + 2, url, l);
	return i;
}

static inline unsigned char hx(int a)
{
	return a >= 10 ? a + 'A' - 10 : a + '0';
}

static inline int unhx(unsigned char a)
{
	if (a >= '0' && a <= '9') return a - '0';
	if (a >= 'A' && a <= 'F') return a - 'A' + 10;
	if (a >= 'a' && a <= 'f') return a - 'a' + 10;
	return -1;
}

unsigned char *encode_url(unsigned char *url)
{
	unsigned char *u = init_str();
	int l = 0;
	add_to_str(&u, &l, "+++");
	for (; *url; url++) {
		if (is_safe_in_shell(*url) && *url != '+') add_chr_to_str(&u, &l, *url);
		else add_chr_to_str(&u, &l, '+'), add_chr_to_str(&u, &l, hx(*url >> 4)), add_chr_to_str(&u, &l, hx(*url & 0xf));
	}
	return u;
}

unsigned char *decode_url(unsigned char *url)
{
	unsigned char *u;
	int l;
	if (casecmp(url, "+++", 3)) return stracpy(url);
	url += 3;
	u = init_str();
	l = 0;
	for (; *url; url++) {
		if (*url != '+' || unhx(url[1]) == -1 || unhx(url[2]) == -1) add_chr_to_str(&u, &l, *url);
		else add_chr_to_str(&u, &l, (unhx(url[1]) << 4) + unhx(url[2])), url += 2;
	}
	return u;
}

int read_session_info(int fd, struct session *ses, void *data, int len)
{
	unsigned char *h;
	int cpfrom, sz;
	struct session *s;
	if (len < 2 * (int)sizeof(int)) return -1;
	cpfrom = *(int *)data;
	sz = *((int *)data + 1);
	foreach(s, sessions) if (s->id == cpfrom) {
		copy_session(s, ses);
		break;
	}
	if (sz) {
		char *u, *uu;
		if (len < 2 * (int)sizeof(int) + sz) return 0;
		if ((unsigned)sz >= MAXINT) overalloc();
		u = mem_alloc(sz + 1);
		memcpy(u, (int *)data + 2, sz);
		u[sz] = 0;
		uu = decode_url(u);
		goto_url(ses, uu);
		mem_free(u);
		mem_free(uu);
	} else if ((h = getenv("WWW_HOME")) && *h) goto_url(ses, h);
	return 0;
}

void abort_preloading(struct session *ses)
{
	if (ses->wtd) {
		change_connection(&ses->loading, NULL, PRI_CANCEL);
		free_wtd(ses);
	}
}

void abort_loading(struct session *ses)
{
	struct location *l = cur_loc(ses);
	if ((void *)l != &ses->history) {
		if (l->stat.state >= 0)
			change_connection(&l->stat, NULL, PRI_CANCEL);
		abort_files_load(ses);
	}
	abort_preloading(ses);
}

void destroy_session(struct session *ses)
{
	struct f_data_c *fdc;
	struct download *d;
	struct location *l;
	if (!ses) return;
	foreach(d, downloads) if (d->ses == ses && d->prog) {
		d = d->prev;
		abort_download(d->next);
	}
	abort_loading(ses);
	free_files(ses);
	if (ses->screen) detach_formatted(ses->screen), mem_free(ses->screen);
	foreach(fdc, ses->scrn_frames) detach_formatted(fdc);
	free_list(ses->scrn_frames);
	while ((void *)(l = ses->history.next) != &ses->history) {
		struct frame *frm;
		while ((void *)(frm = l->frames.next) != &l->frames) {
			destroy_vs(&frm->vs);
			mem_free(frm->name);
			del_from_list(frm);
			mem_free(frm);
		}
		destroy_vs(&l->vs);
		del_from_list(l);
		mem_free(l);
	}
	if (ses->loading_url) mem_free(ses->loading_url);
	if (ses->display_timer != -1) kill_timer(ses->display_timer);
	if (ses->goto_position) mem_free(ses->goto_position);
	if (ses->imgmap_href_base) mem_free(ses->imgmap_href_base);
	if (ses->imgmap_target_base) mem_free(ses->imgmap_target_base);
	if (ses->tq_ce) ses->tq_ce->refcount--;
	if (ses->tq_url) {
		change_connection(&ses->tq, NULL, PRI_CANCEL);
		mem_free(ses->tq_url);
	}
	if (ses->tq_goto_position) mem_free(ses->tq_goto_position);
	if (ses->tq_prog) mem_free(ses->tq_prog);
	if (ses->dn_url) mem_free(ses->dn_url);
	if (ses->search_word) mem_free(ses->search_word);
	if (ses->last_search_word) mem_free(ses->last_search_word);
	del_from_list(ses);
	/*mem_free(ses);*/
}

void destroy_all_sessions()
{
	/*while (!list_empty(sessions)) destroy_session(sessions.next);*/
}

void abort_all_downloads()
{
	while (!list_empty(downloads)) abort_download(downloads.next);
}

void reload(struct session *ses, int no_cache)
{
	struct location *l;
	abort_loading(ses);
	if (no_cache == -1) no_cache = ++ses->reloadlevel;
	else ses->reloadlevel = no_cache;
	if ((void *)(l = ses->history.next) != &ses->history) {
		struct file_to_load *ftl;
		l->stat.data = ses;
		l->stat.end = (void *)doc_end_load;
		load_url(l->vs.url, &l->stat, PRI_MAIN, no_cache);
		foreach(ftl, ses->more_files) {
			if (ftl->req_sent && ftl->stat.state >= 0) continue;
			ftl->stat.data = ftl;
			ftl->stat.end = (void *)file_end_load;
			load_url(ftl->url, &ftl->stat, PRI_FRAME, no_cache);
		}
	}
}

void go_back(struct session *ses)
{
	unsigned char *url;
	ses->reloadlevel = NC_CACHE;
	if (ses->wtd) {
		if (1 || ses->wtd != WTD_BACK) {
			abort_loading(ses);
			print_screen_status(ses);
			reload(ses, NC_CACHE);
		}
		return;
	}
	if (ses->history.next == &ses->history || ses->history.next == ses->history.prev)
		return;
	abort_loading(ses);
	url = stracpy(((struct location *)ses->history.next)->next->vs.url);
	ses->redirect_cnt = 0;
	ses_goto(ses, url, NULL, PRI_MAIN, NC_ALWAYS_CACHE, WTD_BACK, NULL, end_load, 0);
}

void goto_url_w(struct session *ses, unsigned char *url, unsigned char *target, int wtd)
{
	unsigned char *u;
	unsigned char *pos;
	void (*fn)(struct session *, unsigned char *);
	if ((fn = get_external_protocol_function(url))) {
		fn(ses, url);
		return;
	}
	ses->reloadlevel = NC_CACHE;
	/*struct location *l = ses->history.next;*/
	if (!(u = translate_url(url, ses->term->cwd))) {
		struct status stat = { NULL, NULL, NULL, NULL, S_BAD_URL, PRI_CANCEL, 0, NULL, NULL, NULL };
		print_error_dialog(ses, &stat, TEXT_(T_ERROR));
		return;
	}
	pos = extract_position(u);
	if (ses->wtd == wtd) {
		if (!strcmp(ses->loading_url, u)) {
			mem_free(u);
			if (ses->goto_position) mem_free(ses->goto_position);
			ses->goto_position = pos;
			return;
		}
	}
	abort_loading(ses);
	ses->redirect_cnt = 0;
	ses_goto(ses, u, target, PRI_MAIN, NC_CACHE, wtd, pos, end_load, 0);
	/*abort_loading(ses);*/
}

void goto_url_f(struct session *ses, unsigned char *url, unsigned char *target)
{
	goto_url_w(ses, url, target, WTD_FORWARD);
}

void goto_url(struct session *ses, unsigned char *url)
{
	goto_url_w(ses, url, NULL, WTD_FORWARD);
}

void goto_imgmap(struct session *ses, unsigned char *url, unsigned char *href, unsigned char *target)
{
	if (ses->imgmap_href_base) mem_free(ses->imgmap_href_base);
	ses->imgmap_href_base = href;
	if (ses->imgmap_target_base) mem_free(ses->imgmap_target_base);
	ses->imgmap_target_base = target;
	goto_url_w(ses, url, target, WTD_IMGMAP);
}

struct frame *ses_find_frame(struct session *ses, unsigned char *name)
{
	struct location *l = cur_loc(ses);
	struct frame *frm;
	if (list_empty(ses->history)) {
		internal("ses_request_frame: history empty");
		return NULL;
	}
	foreachback(frm, l->frames) if (!strcasecmp(frm->name, name)) return frm;
	/*internal("ses_find_frame: frame not found");*/
	return NULL;
}

struct frame *ses_change_frame_url(struct session *ses, unsigned char *name, unsigned char *url)
{
	struct location *l = cur_loc(ses);
	struct frame *frm;
	if (list_empty(ses->history)) {
		internal("ses_change_frame_url: history empty");
		return NULL;
	}
	foreachback(frm, l->frames) if (!strcasecmp(frm->name, name)) {
		if (strlen(url) > strlen(frm->vs.url)) {
			struct f_data_c *fd;
			struct frame *nf = frm;
			nf = mem_realloc(frm, sizeof(struct frame) + strlen(url) + 1);
			nf->prev->next = nf->next->prev = nf;
			foreach(fd, ses->scrn_frames) {
				if (fd->vs == &frm->vs) fd->vs = &nf->vs;
			}
			frm = nf;
		}
		strcpy(frm->vs.url, url);
		return frm;
	}
	return NULL;
	
}

void win_func(struct window *win, struct event *ev, int fw)
{
	struct session *ses = win->data;
	switch ((int)ev->ev) {
		case EV_ABORT:
			if (ses) destroy_session(ses);
			break;
		case EV_INIT:
			if (!(ses = win->data = create_session(win)) ||
			    read_session_info(win->term->fdin, ses, (char *)ev->b + sizeof(int), *(int *)ev->b)) {
				register_bottom_half((void (*)(void *))destroy_terminal, win->term);
				return;
			}
			/*make_request(ses, 0);*/
		case EV_RESIZE:
			html_interpret(ses);
			draw_formatted(ses);
			load_frames(ses, ses->screen);
			process_file_requests(ses);
			print_screen_status(ses);
			break;
		case EV_REDRAW:
			draw_formatted(ses);
			print_screen_status(ses);
			break;
		case EV_KBD:
		case EV_MOUSE:
			send_event(ses, ev);
			break;
		default:
			error("ERROR: unknown event");
	}
}

/* 
  Gets the url being viewed by this session. Writes it into str.
  A maximum of str_size bytes (including null) will be written.
*/  
unsigned char *get_current_url(struct session *ses, unsigned char *str, size_t str_size) {
	unsigned char *here, *end_of_url;
	size_t url_len = 0;

	/* Not looking at anything */
	if (list_empty(ses->history))
		return NULL;

	here = cur_loc(ses)->vs.url;

	/* Find the length of the url */
	if ((end_of_url = strchr(here, POST_CHAR))) {
		url_len = (size_t)(end_of_url - (unsigned char *)here);
	} else {
		url_len = strlen(here);
	}

	/* Ensure that the url size is not greater than str_size */ 
	if (url_len >= str_size)
			url_len = str_size - 1;

	safe_strncpy(str, here, url_len + 1);

	return str;
}


/* 
  Gets the title of the page being viewed by this session. Writes it into str.
  A maximum of str_size bytes (including null) will be written.
*/  
unsigned char *get_current_title(struct session *ses, unsigned char *str, size_t str_size) {
	struct f_data_c *fd;
	fd = (struct f_data_c *)current_frame(ses);

	/* Ensure that the title is defined */
	if (!fd)
		return NULL;

	return safe_strncpy(str, fd->f_data->title, str_size);
}

/* 
  Gets the url of the link currently selected. Writes it into str.
  A maximum of str_size bytes (including null) will be written.
*/  
unsigned char *get_current_link_url(struct session *ses, unsigned char *str, size_t str_size) {
	struct f_data_c *fd;
    struct link *l;
	
	fd = (struct f_data_c *)current_frame(ses);
	/* What the hell is an 'fd'? */
	if (!fd)
		return NULL;
	
	/* Nothing selected? */
    if (fd->vs->current_link == -1) 
		return NULL;

    l = &fd->f_data->links[fd->vs->current_link];
	/* Only write a link */
    if (l->type != L_LINK) 
		return NULL;
	
	return safe_strncpy(str, l->where ? l->where : l->where_img, str_size);
}
