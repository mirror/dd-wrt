#include "links.h"

/*static inline struct session *get_term_session(struct terminal *term)
{
	if ((void *)term->windows.prev == &term->windows) {
		internal("terminal has no windows");
		return NULL;
	}
	return ((struct window *)term->windows.prev)->data;
}*/

void menu_about(struct terminal *term, void *d, struct session *ses)
{
	msg_box(term, NULL, TEXT_(T_ABOUT), AL_CENTER, TEXT_(T_LINKS__LYNX_LIKE), NULL, 1, TEXT_(T_OK), NULL, B_ENTER | B_ESC);
}

void menu_keys(struct terminal *term, void *d, struct session *ses)
{
	msg_box(term, NULL, TEXT_(T_KEYS), AL_LEFT, TEXT_(T_KEYS_DESC), NULL, 1, TEXT_(T_OK), NULL, B_ENTER | B_ESC);
}

void menu_copying(struct terminal *term, void *d, struct session *ses)
{
	msg_box(term, NULL, TEXT_(T_COPYING), AL_CENTER, TEXT_(T_COPYING_DESC), NULL, 1, TEXT_(T_OK), NULL, B_ENTER | B_ESC);
}

void menu_manual(struct terminal *term, void *d, struct session *ses)
{
	goto_url(ses, LINKS_MANUAL_URL);
}

void menu_for_frame(struct terminal *term, void (*f)(struct session *, struct f_data_c *, int), struct session *ses)
{
	do_for_frame(ses, f, 0);
}

void menu_goto_url(struct terminal *term, void *d, struct session *ses)
{
	dialog_goto_url(ses, "");
}

void menu_save_url_as(struct terminal *term, void *d, struct session *ses)
{
	dialog_save_url(ses);
}

void menu_go_back(struct terminal *term, void *d, struct session *ses)
{
	go_back(ses);
}

void menu_reload(struct terminal *term, void *d, struct session *ses)
{
	reload(ses, -1);
}

void really_exit_prog(struct session *ses)
{
	register_bottom_half((void (*)(void *))destroy_terminal, ses->term);
}

void dont_exit_prog(struct session *ses)
{
	ses->exit_query = 0;
}

void query_exit(struct session *ses)
{
	ses->exit_query = 1;
	msg_box(ses->term, NULL, TEXT_(T_EXIT_LINKS), AL_CENTER, (ses->term->next == ses->term->prev && are_there_downloads()) ? TEXT_(T_DO_YOU_REALLY_WANT_TO_EXIT_LINKS_AND_TERMINATE_ALL_DOWNLOADS) : TEXT_(T_DO_YOU_REALLY_WANT_TO_EXIT_LINKS), ses, 2, TEXT_(T_YES), (void (*)(void *))really_exit_prog, B_ENTER, TEXT_(T_NO), dont_exit_prog, B_ESC);
}

void exit_prog(struct terminal *term, void *d, struct session *ses)
{
	if (!ses) {
		register_bottom_half((void (*)(void *))destroy_terminal, term);
		return;
	}
	if (!ses->exit_query && (!d || (term->next == term->prev && are_there_downloads()))) {
		query_exit(ses);
		return;
	}
	really_exit_prog(ses);
}

struct refresh {
	struct terminal *term;
	struct window *win;
	struct session *ses;
	void (*fn)(struct terminal *term, void *d, struct session *ses);
	void *data;
	int timer;
};

void refresh(struct refresh *r)
{
	struct refresh rr;
	r->timer = -1;
	memcpy(&rr, r, sizeof(struct refresh));
	delete_window(r->win);
	rr.fn(rr.term, rr.data, rr.ses);
}

void end_refresh(struct refresh *r)
{
	if (r->timer != -1) kill_timer(r->timer);
	mem_free(r);
}

void refresh_abort(struct dialog_data *dlg)
{
	end_refresh(dlg->dlg->udata2);
}

void cache_inf(struct terminal *term, void *d, struct session *ses)
{
	unsigned char *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8, *a9, *a10, *a11, *a12, *a13, *a14, *a15, *a16;
	int l = 0;
	struct refresh *r;
	r = mem_alloc(sizeof(struct refresh));
	r->term = term;
	r->win = NULL;
	r->ses = ses;
	r->fn = cache_inf;
	r->data = d;
	r->timer = -1;
	l = 0;
	l = 0, a1 = init_str(); add_to_str(&a1, &l, ": "); add_num_to_str(&a1, &l, select_info(CI_FILES));add_to_str(&a1, &l, " ");
	l = 0, a2 = init_str(); add_to_str(&a2, &l, ", "); add_num_to_str(&a2, &l, select_info(CI_TIMERS));add_to_str(&a2, &l, " ");
	l = 0, a3 = init_str(); add_to_str(&a3, &l, ".\n");

	l = 0, a4 = init_str(); add_to_str(&a4, &l, ": "); add_num_to_str(&a4, &l, connect_info(CI_FILES));add_to_str(&a4, &l, " ");
	l = 0, a5 = init_str(); add_to_str(&a5, &l, ", "); add_num_to_str(&a5, &l, connect_info(CI_CONNECTING));add_to_str(&a5, &l, " ");
	l = 0, a6 = init_str(); add_to_str(&a6, &l, ", "); add_num_to_str(&a6, &l, connect_info(CI_TRANSFER));add_to_str(&a6, &l, " ");
	l = 0, a7 = init_str(); add_to_str(&a7, &l, ", "); add_num_to_str(&a7, &l, connect_info(CI_KEEP));add_to_str(&a7, &l, " ");
	l = 0, a8 = init_str(); add_to_str(&a8, &l, ".\n");

	l = 0, a9 = init_str(); add_to_str(&a9, &l, ": "); add_num_to_str(&a9, &l, cache_info(CI_BYTES));add_to_str(&a9, &l, " ");
	l = 0, a10 = init_str(); add_to_str(&a10, &l, ", "); add_num_to_str(&a10, &l, cache_info(CI_FILES));add_to_str(&a10, &l, " ");
	l = 0, a11 = init_str(); add_to_str(&a11, &l, ", "); add_num_to_str(&a11, &l, cache_info(CI_LOCKED));add_to_str(&a11, &l, " ");
	l = 0, a12 = init_str(); add_to_str(&a12, &l, ", "); add_num_to_str(&a12, &l, cache_info(CI_LOADING));add_to_str(&a12, &l, " ");
	l = 0, a13 = init_str(); add_to_str(&a13, &l, ".\n");

	l = 0, a14 = init_str(); add_to_str(&a14, &l, ": "); add_num_to_str(&a14, &l, formatted_info(CI_FILES));add_to_str(&a14, &l, " ");
	l = 0, a15 = init_str(); add_to_str(&a15, &l, ", "); add_num_to_str(&a15, &l, formatted_info(CI_LOCKED));add_to_str(&a15, &l, " ");
	l = 0, a16 = init_str(); add_to_str(&a16, &l, ".");

	msg_box(term, getml(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, NULL), TEXT_(T_RESOURCES), AL_LEFT | AL_EXTD_TEXT, TEXT_(T_RESOURCES), a1, TEXT_(T_HANDLES), a2, TEXT_(T_TIMERS), a3, TEXT_(T_CONNECTIONS), a4, TEXT_(T_cONNECTIONS), a5, TEXT_(T_CONNECTING), a6, TEXT_(T_tRANSFERRING), a7, TEXT_(T_KEEPALIVE), a8, TEXT_(T_MEMORY_CACHE), a9, TEXT_(T_BYTES), a10, TEXT_(T_FILES), a11, TEXT_(T_LOCKED), a12, TEXT_(T_LOADING), a13, TEXT_(T_FORMATTED_DOCUMENT_CACHE), a14, TEXT_(T_DOCUMENTS), a15, TEXT_(T_LOCKED), a16, NULL, r, 1, TEXT_(T_OK), NULL, B_ENTER | B_ESC);
	r->win = term->windows.next;
	((struct dialog_data *)r->win->data)->dlg->abort = refresh_abort;
	r->timer = install_timer(RESOURCE_INFO_REFRESH, (void (*)(void *))refresh, r);
}

#ifdef DEBUG

void list_cache(struct terminal *term, void *d, struct session *ses)
{
	unsigned char *a;
	int l = 0;
	struct refresh *r;
	struct cache_entry *ce, *cache;
	a = init_str();
	r = mem_alloc(sizeof(struct refresh));
	r->term = term;
	r->win = NULL;
	r->ses = ses;
	r->fn = list_cache;
	r->data = d;
	r->timer = -1;
	cache = (struct cache_entry *)cache_info(CI_LIST);
	add_to_str(&a, &l, ":");
	foreach(ce, *cache) {
		add_to_str(&a, &l, "\n");
		add_to_str(&a, &l, ce->url);
	}
	msg_box(term, getml(a, NULL), TEXT_(T_CACHE_INFO), AL_LEFT | AL_EXTD_TEXT, TEXT_(T_CACHE_CONTENT), a, NULL, r, 1, TEXT_(T_OK), end_refresh, B_ENTER | B_ESC);
	r->win = term->windows.next;
	r->timer = install_timer(RESOURCE_INFO_REFRESH, (void (*)(void *))refresh, r);
	/* !!! the refresh here is buggy */
}

#endif

#ifdef LEAK_DEBUG

void memory_cld(struct terminal *term, void *d)
{
	last_mem_amount = mem_amount;
}

#define MSG_BUF	2000
#define MSG_W	100

void memory_info(struct terminal *term, void *d, struct session *ses)
{
	char message[MSG_BUF];
	char *p;
	struct refresh *r;
	r = mem_alloc(sizeof(struct refresh));
	r->term = term;
	r->win = NULL;
	r->ses = ses;
	r->fn = memory_info;
	r->data = d;
	r->timer = -1;
	p = message;
	sprintf(p, "%ld %s", mem_amount, _(TEXT_(T_MEMORY_ALLOCATED), term)), p += strlen(p);
	if (last_mem_amount != -1) sprintf(p, ", %s %ld, %s %ld", _(TEXT_(T_LAST), term), last_mem_amount, _(TEXT_(T_DIFFERENCE), term), mem_amount - last_mem_amount), p += strlen(p);
	sprintf(p, "."), p += strlen(p);
#if 0 && defined(MAX_LIST_SIZE)
	if (last_mem_amount != -1) {
		long i, j;
		int l = 0;
		for (i = 0; i < MAX_LIST_SIZE; i++) if (memory_list[i].p && memory_list[i].p != last_memory_list[i].p) {
			for (j = 0; j < MAX_LIST_SIZE; j++) if (last_memory_list[j].p == memory_list[i].p) goto b;
			if (!l) sprintf(p, "\n%s: ", _(TEXT_(T_NEW_ADDRESSES), term)), p += strlen(p), l = 1;
			else sprintf(p, ", "), p += strlen(p);
			sprintf(p, "#%p of %d at %s:%d", memory_list[i].p, (int)memory_list[i].size, memory_list[i].file, memory_list[i].line), p += strlen(p);
			if (p - message >= MSG_BUF - MSG_W) {
				sprintf(p, ".."), p += strlen(p);
				break;
			}
			b:;
		}
		if (!l) sprintf(p, "\n%s", _(TEXT_(T_NO_NEW_ADDRESSES), term)), p += strlen(p);
		sprintf(p, "."), p += strlen(p);
	}
#endif
	p = stracpy(message);
	msg_box(term, getml(p, NULL), TEXT_(T_MEMORY_INFO), AL_CENTER, p, r, 1, TEXT_(T_OK), NULL, B_ENTER | B_ESC);
	r->win = term->windows.next;
	((struct dialog_data *)r->win->data)->dlg->abort = refresh_abort;
	r->timer = install_timer(RESOURCE_INFO_REFRESH, (void (*)(void *))refresh, r);
}

#endif

void flush_caches(struct terminal *term, void *d, void *e)
{
	shrink_memory(1);
}

void go_backwards(struct terminal *term, void *psteps, struct session *ses)
{
	int steps = (int) psteps;

	/*if (ses->tq_goto_position)
		--steps;
	if (ses->search_word)
		mem_free(ses->search_word), ses->search_word = NULL;*/
	abort_loading(ses);

	while (steps > 1) {
		struct location *loc = ses->history.next;
		if ((void *) loc == &ses->history) return;
		loc = loc->next;
		if ((void *) loc == &ses->history) return;
		destroy_location(loc);

		--steps;
	}

	if (steps)
		go_back(ses);
}

struct menu_item no_hist_menu[] = {
	{ TEXT_(T_NO_HISTORY), "", M_BAR, NULL, NULL, 0, 0 },
	{ NULL, NULL, 0, NULL, NULL, 0, 0 },
};

void history_menu(struct terminal *term, void *ddd, struct session *ses)
{
	struct location *l;
	struct menu_item *mi = NULL;
	int n = 0;
	foreach(l, ses->history) {
		if (n/* || ses->tq_goto_position*/) {
			unsigned char *url;
			if (!mi && !(mi = new_menu(3))) return;
			url = stracpy(l->vs.url);
			if (strchr(url, POST_CHAR)) *strchr(url, POST_CHAR) = 0;
			add_to_menu(&mi, url, "", "", MENU_FUNC go_backwards, (void *) n, 0);
		}
		n++;
	}
	if (n <= 1) do_menu(term, no_hist_menu, ses);
	else do_menu(term, mi, ses);
}

struct menu_item no_downloads_menu[] = {
	{ TEXT_(T_NO_DOWNLOADS), "", M_BAR, NULL, NULL, 0, 0 },
	{ NULL, NULL, 0, NULL, NULL, 0, 0 },
};

void downloads_menu(struct terminal *term, void *ddd, struct session *ses)
{
	struct download *d;
	struct menu_item *mi = NULL;
	int n = 0;
	foreachback(d, downloads) {
		unsigned char *u;
		if (!mi) if (!(mi = new_menu(3))) return;
		u = stracpy(d->url);
		if (strchr(u, POST_CHAR)) *strchr(u, POST_CHAR) = 0;
		add_to_menu(&mi, u, "", "", MENU_FUNC display_download, d, 0);
		n++;
	}
	if (!n) do_menu(term, no_downloads_menu, ses);
	else do_menu(term, mi, ses);
}

void menu_doc_info(struct terminal *term, void *ddd, struct session *ses)
{
	state_msg(ses);
}

void menu_head_info(struct terminal *term, void *ddd, struct session *ses)
{
	head_msg(ses);
}

void menu_toggle(struct terminal *term, void *ddd, struct session *ses)
{
	toggle(ses, ses->screen, 0);
}

void display_codepage(struct terminal *term, void *pcp, struct session *ses)
{
	int cp = (int)pcp;
	struct term_spec *t = new_term_spec(term->term);
	if (t) t->charset = cp;
	cls_redraw_all_terminals();
}

void assumed_codepage(struct terminal *term, void *pcp, struct session *ses)
{
	int cp = (int)pcp;
	ses->ds.assume_cp = cp;
	redraw_terminal_cls(term);
}

void charset_list(struct terminal *term, void *xxx, struct session *ses)
{
	int i, sel;
	unsigned char *n;
	struct menu_item *mi;
	if (!(mi = new_menu(1))) return;
	for (i = 0; (n = get_cp_name(i)); i++) {
		if (is_cp_special(i)) continue;
		add_to_menu(&mi, get_cp_name(i), "", "", MENU_FUNC display_codepage, (void *)i, 0);
	}
	sel = ses->term->spec->charset;
	if (sel < 0) sel = 0;
	do_menu_selected(term, mi, ses, sel);
}

void set_val(struct terminal *term, void *ip, int *d)
{
	*d = (int)ip;
}

void charset_sel_list(struct terminal *term, struct session *ses, int *ptr)
{
	int i, sel;
	unsigned char *n;
	struct menu_item *mi;
	if (!(mi = new_menu(1))) return;
	for (i = 0; (n = get_cp_name(i)); i++) {
		add_to_menu(&mi, get_cp_name(i), "", "", MENU_FUNC set_val, (void *)i, 0);
	}
	sel = *ptr;
	if (sel < 0) sel = 0;
	do_menu_selected(term, mi, ptr, sel);
}

void terminal_options_ok(void *p)
{
	cls_redraw_all_terminals();
}

unsigned char *td_labels[] = { TEXT_(T_NO_FRAMES), TEXT_(T_VT_100_FRAMES), TEXT_(T_LINUX_OR_OS2_FRAMES), TEXT_(T_KOI8R_FRAMES), TEXT_(T_FREEBSD_FRAMES), TEXT_(T_USE_11M), TEXT_(T_RESTRICT_FRAMES_IN_CP850_852), TEXT_(T_BLOCK_CURSOR), TEXT_(T_COLOR), NULL };

void terminal_options(struct terminal *term, void *xxx, struct session *ses)
{
	struct dialog *d;
	struct term_spec *ts = new_term_spec(term->term);
	if (!ts) return;
	d = mem_alloc(sizeof(struct dialog) + 12 * sizeof(struct dialog_item));
	memset(d, 0, sizeof(struct dialog) + 12 * sizeof(struct dialog_item));
	d->title = TEXT_(T_TERMINAL_OPTIONS);
	d->fn = checkbox_list_fn;
	d->udata = td_labels;
	d->refresh = (void (*)(void *))terminal_options_ok;
	d->items[0].type = D_CHECKBOX;
	d->items[0].gid = 1;
	d->items[0].gnum = TERM_DUMB;
	d->items[0].dlen = sizeof(int);
	d->items[0].data = (void *)&ts->mode;
	d->items[1].type = D_CHECKBOX;
	d->items[1].gid = 1;
	d->items[1].gnum = TERM_VT100;
	d->items[1].dlen = sizeof(int);
	d->items[1].data = (void *)&ts->mode;
	d->items[2].type = D_CHECKBOX;
	d->items[2].gid = 1;
	d->items[2].gnum = TERM_LINUX;
	d->items[2].dlen = sizeof(int);
	d->items[2].data = (void *)&ts->mode;
	d->items[3].type = D_CHECKBOX;
	d->items[3].gid = 1;
	d->items[3].gnum = TERM_KOI8;
	d->items[3].dlen = sizeof(int);
	d->items[3].data = (void *)&ts->mode;
	d->items[4].type = D_CHECKBOX;
	d->items[4].gid = 1;
	d->items[4].gnum = TERM_FREEBSD;
	d->items[4].dlen = sizeof(int);
	d->items[4].data = (void *)&ts->mode;
	d->items[5].type = D_CHECKBOX;
	d->items[5].gid = 0;
	d->items[5].dlen = sizeof(int);
	d->items[5].data = (void *)&ts->m11_hack;
	d->items[6].type = D_CHECKBOX;
	d->items[6].gid = 0;
	d->items[6].dlen = sizeof(int);
	d->items[6].data = (void *)&ts->restrict_852;
	d->items[7].type = D_CHECKBOX;
	d->items[7].gid = 0;
	d->items[7].dlen = sizeof(int);
	d->items[7].data = (void *)&ts->block_cursor;
	d->items[8].type = D_CHECKBOX;
	d->items[8].gid = 0;
	d->items[8].dlen = sizeof(int);
	d->items[8].data = (void *)&ts->col;
	d->items[9].type = D_BUTTON;
	d->items[9].gid = B_ENTER;
	d->items[9].fn = ok_dialog;
	d->items[9].text = TEXT_(T_OK);
	d->items[10].type = D_BUTTON;
	d->items[10].gid = B_ESC;
	d->items[10].fn = cancel_dialog;
	d->items[10].text = TEXT_(T_CANCEL);
	d->items[11].type = D_END;
 	do_dialog(term, d, getml(d, NULL));
}

unsigned char *http_labels[] = { TEXT_(T_USE_HTTP_10), TEXT_(T_ALLOW_SERVER_BLACKLIST), TEXT_(T_BROKEN_302_REDIRECT), TEXT_(T_NO_KEEPALIVE_AFTER_POST_REQUEST), TEXT_(T_DO_NOT_SEND_ACCEPT_CHARSET) };

int dlg_http_options(struct dialog_data *dlg, struct dialog_item_data *di)
{
	struct http_bugs *bugs = (struct http_bugs *)di->cdata;
	struct dialog *d;
	d = mem_alloc(sizeof(struct dialog) + 8 * sizeof(struct dialog_item));
	memset(d, 0, sizeof(struct dialog) + 8 * sizeof(struct dialog_item));
	d->title = TEXT_(T_HTTP_BUG_WORKAROUNDS);
	d->fn = checkbox_list_fn;
	d->udata = http_labels;
	d->items[0].type = D_CHECKBOX;
	d->items[0].gid = 0;
	d->items[0].dlen = sizeof(int);
	d->items[0].data = (void *)&bugs->http10;
	d->items[1].type = D_CHECKBOX;
	d->items[1].gid = 0;
	d->items[1].dlen = sizeof(int);
	d->items[1].data = (void *)&bugs->allow_blacklist;
	d->items[2].type = D_CHECKBOX;
	d->items[2].gid = 0;
	d->items[2].dlen = sizeof(int);
	d->items[2].data = (void *)&bugs->bug_302_redirect;
	d->items[3].type = D_CHECKBOX;
	d->items[3].gid = 0;
	d->items[3].dlen = sizeof(int);
	d->items[3].data = (void *)&bugs->bug_post_no_keepalive;
	d->items[4].type = D_CHECKBOX;
	d->items[4].gid = 0;
	d->items[4].dlen = sizeof(int);
	d->items[4].data = (void *)&bugs->no_accept_charset;
	d->items[5].type = D_BUTTON;
	d->items[5].gid = B_ENTER;
	d->items[5].fn = ok_dialog;
	d->items[5].text = TEXT_(T_OK);
	d->items[6].type = D_BUTTON;
	d->items[6].gid = B_ESC;
	d->items[6].fn = cancel_dialog;
	d->items[6].text = TEXT_(T_CANCEL);
	d->items[7].type = D_END;
 	do_dialog(dlg->win->term, d, getml(d, NULL));
	return 0;
}

unsigned char *ftp_texts[] = { TEXT_(T_PASSWORD_FOR_ANONYMOUS_LOGIN), TEXT_(T_USE_PASSIVE_FTP), TEXT_(T_USE_FAST_FTP), TEXT_(T_SET_TYPE_OF_SERVICE), NULL };

void ftpopt_fn(struct dialog_data *dlg)
{
	struct terminal *term = dlg->win->term;
	int max = 0, min = 0;
	int w, rw;
	int y = 0;
	max_text_width(term, ftp_texts[0], &max);
	min_text_width(term, ftp_texts[0], &min);
	checkboxes_width(term, ftp_texts + 1, &max, max_text_width);
	checkboxes_width(term, ftp_texts + 1, &min, min_text_width);
	max_buttons_width(term, dlg->items + dlg->n - 2, 2, &max);
	min_buttons_width(term, dlg->items + dlg->n - 2, 2, &min);
	w = term->x * 9 / 10 - 2 * DIALOG_LB;
	if (w > max) w = max;
	if (w < min) w = min;
	if (w > term->x - 2 * DIALOG_LB) w = term->x - 2 * DIALOG_LB;
	if (w < 5) w = 5;
	rw = 0;
	dlg_format_text(NULL, term, ftp_texts[0], 0, &y, w, &rw, COLOR_DIALOG_TEXT, AL_LEFT);
	y += 1;
	dlg_format_checkboxes(NULL, term, dlg->items + 1, dlg->n - 3, 0, &y, w, &rw, ftp_texts + 1);
	y += 1;
	dlg_format_buttons(NULL, term, dlg->items + dlg->n - 2, 2, 0, &y, w, &rw, AL_CENTER);
	w = rw;
	dlg->xw = rw + 2 * DIALOG_LB;
	dlg->yw = y + 2 * DIALOG_TB;
	center_dlg(dlg);
	draw_dlg(dlg);
	y = dlg->y + DIALOG_TB;
	dlg_format_text(term, term, ftp_texts[0], dlg->x + DIALOG_LB, &y, w, NULL, COLOR_DIALOG_TEXT, AL_LEFT);
	dlg_format_field(term, term, dlg->items, dlg->x + DIALOG_LB, &y, w, NULL, AL_LEFT);
	y += 1;
	dlg_format_checkboxes(term, term, dlg->items + 1, dlg->n - 3, dlg->x + DIALOG_LB, &y, w, NULL, ftp_texts + 1);
	y += 1;
	dlg_format_buttons(term, term, dlg->items + dlg->n - 2, 2, dlg->x + DIALOG_LB, &y, w, &rw, AL_CENTER);
}


int dlg_ftp_options(struct dialog_data *dlg, struct dialog_item_data *di)
{
	int a;
	struct ftp_options *ftp_options = (struct ftp_options *)di->cdata;
	struct dialog *d;
	d = mem_alloc(sizeof(struct dialog) + 7 * sizeof(struct dialog_item));
	memset(d, 0, sizeof(struct dialog) + 7 * sizeof(struct dialog_item));
	d->title = TEXT_(T_FTP_OPTIONS);
	d->fn = ftpopt_fn;
	d->items[0].type = D_FIELD;
	d->items[0].dlen = MAX_STR_LEN;
	d->items[0].data = ftp_options->anon_pass;
	d->items[1].type = D_CHECKBOX;
	d->items[1].dlen = sizeof(int);
	d->items[1].data = (void*)&ftp_options->passive_ftp;
	d->items[1].gid = 0;
	d->items[2].type = D_CHECKBOX;
	d->items[2].dlen = sizeof(int);
	d->items[2].data = (void*)&ftp_options->fast_ftp;
#ifdef HAVE_IPTOS
	d->items[3].type = D_CHECKBOX;
	d->items[3].dlen = sizeof(int);
	d->items[3].data = (void*)&ftp_options->set_tos;
	a = 4;
#else
	a = 3;
#endif
	d->items[a].type = D_BUTTON;
	d->items[a].gid = B_ENTER;
	d->items[a].fn = ok_dialog;
	d->items[a].text = TEXT_(T_OK);
	a++;
	d->items[a].type = D_BUTTON;
	d->items[a].gid = B_ESC;
	d->items[a].fn = cancel_dialog;
	d->items[a].text = TEXT_(T_CANCEL);
	a++;
	d->items[a].type = D_END;
 	do_dialog(dlg->win->term, d, getml(d, NULL));
	return 0;
}

unsigned char max_c_str[3];
unsigned char max_cth_str[3];
unsigned char max_t_str[3];
unsigned char time_str[5];
unsigned char unrtime_str[5];

void refresh_net(void *xxx)
{
	/*abort_all_connections();*/
	max_connections = atoi(max_c_str);
	max_connections_to_host = atoi(max_cth_str);
	max_tries = atoi(max_t_str);
	receive_timeout = atoi(time_str);
	unrestartable_receive_timeout = atoi(unrtime_str);
	register_bottom_half(check_queue, NULL);
}

unsigned char *net_msg[] = {
	TEXT_(T_HTTP_PROXY__HOST_PORT),
	TEXT_(T_FTP_PROXY__HOST_PORT),
	TEXT_(T_MAX_CONNECTIONS),
	TEXT_(T_MAX_CONNECTIONS_TO_ONE_HOST),
	TEXT_(T_RETRIES),
	TEXT_(T_RECEIVE_TIMEOUT_SEC),
	TEXT_(T_TIMEOUT_WHEN_UNRESTARTABLE),
	TEXT_(T_ASYNC_DNS_LOOKUP),
	TEXT_(T_SET_TIME_OF_DOWNLOADED_FILES),
	"",
	"",
};

void netopt_fn(struct dialog_data *dlg)
{
	struct terminal *term = dlg->win->term;
	int max = 0, min = 0;
	int w, rw;
	int y = -1;
	max_text_width(term, net_msg[0], &max);
	min_text_width(term, net_msg[0], &min);
	max_text_width(term, net_msg[1], &max);
	min_text_width(term, net_msg[1], &min);
	max_group_width(term, net_msg + 2, dlg->items + 2, 9, &max);
	min_group_width(term, net_msg + 2, dlg->items + 2, 9, &min);
	max_buttons_width(term, dlg->items + 11, 2, &max);
	min_buttons_width(term, dlg->items + 11, 2, &min);
	w = dlg->win->term->x * 9 / 10 - 2 * DIALOG_LB;
	if (w > max) w = max;
	if (w < min) w = min;
	if (w > dlg->win->term->x - 2 * DIALOG_LB) w = dlg->win->term->x - 2 * DIALOG_LB;
	if (w < 1) w = 1;
	rw = 0;
	dlg_format_text(NULL, term, net_msg[0], 0, &y, w, &rw, COLOR_DIALOG_TEXT, AL_LEFT);
	y += 2;
	dlg_format_text(NULL, term, net_msg[1], 0, &y, w, &rw, COLOR_DIALOG_TEXT, AL_LEFT);
	y += 2;
	dlg_format_group(NULL, term, net_msg + 2, dlg->items + 2, 9, 0, &y, w, &rw);
	y++;
	dlg_format_buttons(NULL, term, dlg->items + 11, 2, 0, &y, w, &rw, AL_CENTER);
	w = rw;
	dlg->xw = w + 2 * DIALOG_LB;
	dlg->yw = y + 2 * DIALOG_TB;
	center_dlg(dlg);
	draw_dlg(dlg);
	y = dlg->y + DIALOG_TB;
	dlg_format_text(term, term, net_msg[0], dlg->x + DIALOG_LB, &y, w, NULL, COLOR_DIALOG_TEXT, AL_LEFT);
	dlg_format_field(term, term, &dlg->items[0], dlg->x + DIALOG_LB, &y, w, NULL, AL_LEFT);
	y++;
	dlg_format_text(term, term, net_msg[1], dlg->x + DIALOG_LB, &y, w, NULL, COLOR_DIALOG_TEXT, AL_LEFT);
	dlg_format_field(term, term, &dlg->items[1], dlg->x + DIALOG_LB, &y, w, NULL, AL_LEFT);
	y++;
	dlg_format_group(term, term, net_msg + 2, &dlg->items[2], 9, dlg->x + DIALOG_LB, &y, w, NULL);
	y++;
	dlg_format_buttons(term, term, &dlg->items[11], 2, dlg->x + DIALOG_LB, &y, w, NULL, AL_CENTER);
}

void net_options(struct terminal *term, void *xxx, void *yyy)
{
	struct dialog *d;
	snprint(max_c_str, 3, max_connections);
	snprint(max_cth_str, 3, max_connections_to_host);
	snprint(max_t_str, 3, max_tries);
	snprint(time_str, 5, receive_timeout);
	snprint(unrtime_str, 5, unrestartable_receive_timeout);
	d = mem_alloc(sizeof(struct dialog) + 14 * sizeof(struct dialog_item));
	memset(d, 0, sizeof(struct dialog) + 14 * sizeof(struct dialog_item));
	d->title = TEXT_(T_NETWORK_OPTIONS);
	d->fn = netopt_fn;
	d->refresh = (void (*)(void *))refresh_net;
	d->items[0].type = D_FIELD;
	d->items[0].dlen = MAX_STR_LEN;
	d->items[0].data = http_proxy;
	d->items[1].type = D_FIELD;
	d->items[1].dlen = MAX_STR_LEN;
	d->items[1].data = ftp_proxy;
	d->items[2].type = D_FIELD;
	d->items[2].data = max_c_str;
	d->items[2].dlen = 3;
	d->items[2].fn = check_number;
	d->items[2].gid = 1;
	d->items[2].gnum = 99;
	d->items[3].type = D_FIELD;
	d->items[3].data = max_cth_str;
	d->items[3].dlen = 3;
	d->items[3].fn = check_number;
	d->items[3].gid = 1;
	d->items[3].gnum = 99;
	d->items[4].type = D_FIELD;
	d->items[4].data = max_t_str;
	d->items[4].dlen = 3;
	d->items[4].fn = check_number;
	d->items[4].gid = 0;
	d->items[4].gnum = 16;
	d->items[5].type = D_FIELD;
	d->items[5].data = time_str;
	d->items[5].dlen = 5;
	d->items[5].fn = check_number;
	d->items[5].gid = 1;
	d->items[5].gnum = 1800;
	d->items[6].type = D_FIELD;
	d->items[6].data = unrtime_str;
	d->items[6].dlen = 5;
	d->items[6].fn = check_number;
	d->items[6].gid = 1;
	d->items[6].gnum = 1800;
	d->items[7].type = D_CHECKBOX;
	d->items[7].data = (unsigned char *)&async_lookup;
	d->items[7].dlen = sizeof(int);
	d->items[8].type = D_CHECKBOX;
	d->items[8].data = (unsigned char *)&download_utime;
	d->items[8].dlen = sizeof(int);
	d->items[9].type = D_BUTTON;
	d->items[9].gid = 0;
	d->items[9].fn = dlg_http_options;
	d->items[9].text = TEXT_(T_HTTP_OPTIONS);
	d->items[9].data = (unsigned char *)&http_bugs;
	d->items[9].dlen = sizeof(struct http_bugs);
	d->items[10].type = D_BUTTON;
	d->items[10].gid = 0;
	d->items[10].fn = dlg_ftp_options;
	d->items[10].text = TEXT_(T_FTP_OPTIONS);
	d->items[10].data = (unsigned char *)&ftp_options;
	d->items[10].dlen = sizeof(struct ftp_options);
	d->items[11].type = D_BUTTON;
	d->items[11].gid = B_ENTER;
	d->items[11].fn = ok_dialog;
	d->items[11].text = TEXT_(T_OK);
	d->items[12].type = D_BUTTON;
	d->items[12].gid = B_ESC;
	d->items[12].fn = cancel_dialog;
	d->items[12].text = TEXT_(T_CANCEL);
	d->items[13].type = D_END;
	do_dialog(term, d, getml(d, NULL));
}

unsigned char *prg_msg[] = {
	TEXT_(T_MAILTO_PROG),
	TEXT_(T_TELNET_PROG),
	TEXT_(T_TN3270_PROG),
	TEXT_(T_MMS_PROG),
	""
};

void netprog_fn(struct dialog_data *dlg)
{
	struct terminal *term = dlg->win->term;
	int max = 0, min = 0;
	int w, rw;
	int y = -1;
	max_text_width(term, prg_msg[0], &max);
	min_text_width(term, prg_msg[0], &min);
	max_text_width(term, prg_msg[1], &max);
	min_text_width(term, prg_msg[1], &min);
	max_text_width(term, prg_msg[2], &max);
	min_text_width(term, prg_msg[2], &min);
	max_text_width(term, prg_msg[3], &max);
	min_text_width(term, prg_msg[3], &min);
	max_buttons_width(term, dlg->items + 4, 2, &max);
	min_buttons_width(term, dlg->items + 4, 2, &min);
	w = dlg->win->term->x * 9 / 10 - 2 * DIALOG_LB;
	if (w > max) w = max;
	if (w < min) w = min;
	if (w > dlg->win->term->x - 2 * DIALOG_LB) w = dlg->win->term->x - 2 * DIALOG_LB;
	if (w < 1) w = 1;
	rw = 0;
	dlg_format_text(NULL, term, prg_msg[0], 0, &y, w, &rw, COLOR_DIALOG_TEXT, AL_LEFT);
	y += 2;
	dlg_format_text(NULL, term, prg_msg[1], 0, &y, w, &rw, COLOR_DIALOG_TEXT, AL_LEFT);
	y += 2;
	dlg_format_text(NULL, term, prg_msg[2], 0, &y, w, &rw, COLOR_DIALOG_TEXT, AL_LEFT);
	y += 2;
	dlg_format_text(NULL, term, prg_msg[3], 0, &y, w, &rw, COLOR_DIALOG_TEXT, AL_LEFT);
	y += 2;
	dlg_format_buttons(NULL, term, dlg->items + 4, 2, 0, &y, w, &rw, AL_CENTER);
	w = rw;
	dlg->xw = w + 2 * DIALOG_LB;
	dlg->yw = y + 2 * DIALOG_TB;
	center_dlg(dlg);
	draw_dlg(dlg);
	y = dlg->y + DIALOG_TB;
	dlg_format_text(term, term, prg_msg[0], dlg->x + DIALOG_LB, &y, w, NULL, COLOR_DIALOG_TEXT, AL_LEFT);
	dlg_format_field(term, term, &dlg->items[0], dlg->x + DIALOG_LB, &y, w, NULL, AL_LEFT);
	y++;
	dlg_format_text(term, term, prg_msg[1], dlg->x + DIALOG_LB, &y, w, NULL, COLOR_DIALOG_TEXT, AL_LEFT);
	dlg_format_field(term, term, &dlg->items[1], dlg->x + DIALOG_LB, &y, w, NULL, AL_LEFT);
	y++;
	dlg_format_text(term, term, prg_msg[2], dlg->x + DIALOG_LB, &y, w, NULL, COLOR_DIALOG_TEXT, AL_LEFT);
	dlg_format_field(term, term, &dlg->items[2], dlg->x + DIALOG_LB, &y, w, NULL, AL_LEFT);
	y++;
	dlg_format_text(term, term, prg_msg[3], dlg->x + DIALOG_LB, &y, w, NULL, COLOR_DIALOG_TEXT, AL_LEFT);
	dlg_format_field(term, term, &dlg->items[3], dlg->x + DIALOG_LB, &y, w, NULL, AL_LEFT);
	y++;
	dlg_format_buttons(term, term, &dlg->items[4], 2, dlg->x + DIALOG_LB, &y, w, NULL, AL_CENTER);
}

void net_programs(struct terminal *term, void *xxx, void *yyy)
{
	struct dialog *d;
	d = mem_alloc(sizeof(struct dialog) + 7 * sizeof(struct dialog_item));
	memset(d, 0, sizeof(struct dialog) + 7 * sizeof(struct dialog_item));
	d->title = TEXT_(T_MAIL_AND_TELNET_PROGRAMS);
	d->fn = netprog_fn;
	d->items[0].type = D_FIELD;
	d->items[0].dlen = MAX_STR_LEN;
	d->items[0].data = get_prog(&mailto_prog);
	d->items[1].type = D_FIELD;
	d->items[1].dlen = MAX_STR_LEN;
	d->items[1].data = get_prog(&telnet_prog);
	d->items[2].type = D_FIELD;
	d->items[2].dlen = MAX_STR_LEN;
	d->items[2].data = get_prog(&tn3270_prog);
	d->items[3].type = D_FIELD;
	d->items[3].dlen = MAX_STR_LEN;
	d->items[3].data = get_prog(&mms_prog);
	d->items[4].type = D_BUTTON;
	d->items[4].gid = B_ENTER;
	d->items[4].fn = ok_dialog;
	d->items[4].text = TEXT_(T_OK);
	d->items[5].type = D_BUTTON;
	d->items[5].gid = B_ESC;
	d->items[5].fn = cancel_dialog;
	d->items[5].text = TEXT_(T_CANCEL);
	d->items[6].type = D_END;
	do_dialog(term, d, getml(d, NULL));
}

/*void net_opt_ask(struct terminal *term, void *xxx, void *yyy)
{
	if (list_empty(downloads)) {
		net_options(term, xxx, yyy);
		return;
	}
	msg_box(term, NULL, _("Network options"), AL_CENTER, _("Warning: configuring network will terminate all running downloads. Do you really want to configure network?"), term, 2, _("Yes"), (void (*)(void *))net_options, B_ENTER, _("No"), NULL, B_ESC);
}*/

unsigned char mc_str[8];
unsigned char doc_str[4];

void cache_refresh(void *xxx)
{
	memory_cache_size = atoi(mc_str) * 1024;
	max_format_cache_entries = atoi(doc_str);
	count_format_cache();
	shrink_memory(0);
}

unsigned char *cache_texts[] = { TEXT_(T_MEMORY_CACHE_SIZE__KB), TEXT_(T_NUMBER_OF_FORMATTED_DOCUMENTS) };

void cache_opt(struct terminal *term, void *xxx, void *yyy)
{
	struct dialog *d;
	snprint(mc_str, 8, memory_cache_size / 1024);
	snprint(doc_str, 4, max_format_cache_entries);
	d = mem_alloc(sizeof(struct dialog) + 5 * sizeof(struct dialog_item));
	memset(d, 0, sizeof(struct dialog) + 5 * sizeof(struct dialog_item));
	d->title = TEXT_(T_CACHE_OPTIONS);
	d->fn = group_fn;
	d->udata = cache_texts;
	d->refresh = (void (*)(void *))cache_refresh;
	d->items[0].type = D_FIELD;
	d->items[0].dlen = 8;
	d->items[0].data = mc_str;
	d->items[0].fn = check_number;
	d->items[0].gid = 0;
	d->items[0].gnum = MAXINT / 1024;
	d->items[1].type = D_FIELD;
	d->items[1].dlen = 4;
	d->items[1].data = doc_str;
	d->items[1].fn = check_number;
	d->items[1].gid = 0;
	d->items[1].gnum = 256;
	d->items[2].type = D_BUTTON;
	d->items[2].gid = B_ENTER;
	d->items[2].fn = ok_dialog;
	d->items[2].text = TEXT_(T_OK);
	d->items[3].type = D_BUTTON;
	d->items[3].gid = B_ESC;
	d->items[3].fn = cancel_dialog;
	d->items[3].text = TEXT_(T_CANCEL);
	d->items[4].type = D_END;
	do_dialog(term, d, getml(d, NULL));
}

void menu_shell(struct terminal *term, void *xxx, void *yyy)
{
	unsigned char *sh;
	if (!(sh = GETSHELL)) sh = DEFAULT_SHELL;
	exec_on_terminal(term, sh, "", 1);
}

void menu_kill_background_connections(struct terminal *term, void *xxx, void *yyy)
{
	abort_background_connections();
}

void menu_save_html_options(struct terminal *term, void *xxx, struct session *ses)
{
	memcpy(&dds, &ses->ds, sizeof(struct document_setup));
	write_html_config(term);
}

unsigned char marg_str[2];

void html_refresh(struct session *ses)
{
	ses->ds.margin = atoi(marg_str);
	html_interpret(ses);
	draw_formatted(ses);
	load_frames(ses, ses->screen);
	process_file_requests(ses);
	print_screen_status(ses);
}

unsigned char *html_texts[] = { TEXT_(T_DISPLAY_TABLES), TEXT_(T_DISPLAY_FRAMES), TEXT_(T_DISPLAY_LINKS_TO_IMAGES), TEXT_(T_LINK_ORDER_BY_COLUMNS), TEXT_(T_NUMBERED_LINKS), TEXT_(T_TEXT_MARGIN), "", TEXT_(T_IGNORE_CHARSET_INFO_SENT_BY_SERVER) };

int dlg_assume_cp(struct dialog_data *dlg, struct dialog_item_data *di)
{
	charset_sel_list(dlg->win->term, dlg->dlg->udata2, (int *)di->cdata);
	return 0;
}

void menu_html_options(struct terminal *term, void *xxx, struct session *ses)
{
	struct dialog *d;
	snprint(marg_str, 2, ses->ds.margin);
	d = mem_alloc(sizeof(struct dialog) + 11 * sizeof(struct dialog_item));
	memset(d, 0, sizeof(struct dialog) + 11 * sizeof(struct dialog_item));
	d->title = TEXT_(T_HTML_OPTIONS);
	d->fn = group_fn;
	d->udata = html_texts;
	d->udata2 = ses;
	d->refresh = (void (*)(void *))html_refresh;
	d->refresh_data = ses;
	d->items[0].type = D_CHECKBOX;
	d->items[0].data = (unsigned char *) &ses->ds.tables;
	d->items[0].dlen = sizeof(int);
	d->items[1].type = D_CHECKBOX;
	d->items[1].data = (unsigned char *) &ses->ds.frames;
	d->items[1].dlen = sizeof(int);
	d->items[2].type = D_CHECKBOX;
	d->items[2].data = (unsigned char *) &ses->ds.images;
	d->items[2].dlen = sizeof(int);
	d->items[3].type = D_CHECKBOX;
	d->items[3].data = (unsigned char *) &ses->ds.table_order;
	d->items[3].dlen = sizeof(int);
	d->items[4].type = D_CHECKBOX;
	d->items[4].data = (unsigned char *) &ses->ds.num_links;
	d->items[4].dlen = sizeof(int);
	d->items[5].type = D_FIELD;
	d->items[5].dlen = 2;
	d->items[5].data = marg_str;
	d->items[5].fn = check_number;
	d->items[5].gid = 0;
	d->items[5].gnum = 9;
	d->items[6].type = D_BUTTON;
	d->items[6].gid = 0;
	d->items[6].fn = dlg_assume_cp;
	d->items[6].text = TEXT_(T_DEFAULT_CODEPAGE);
	d->items[6].data = (unsigned char *) &ses->ds.assume_cp;
	d->items[6].dlen = sizeof(int);
	d->items[7].type = D_CHECKBOX;
	d->items[7].data = (unsigned char *) &ses->ds.hard_assume;
	d->items[7].dlen = sizeof(int);
	d->items[8].type = D_BUTTON;
	d->items[8].gid = B_ENTER;
	d->items[8].fn = ok_dialog;
	d->items[8].text = TEXT_(T_OK);
	d->items[9].type = D_BUTTON;
	d->items[9].gid = B_ESC;
	d->items[9].fn = cancel_dialog;
	d->items[9].text = TEXT_(T_CANCEL);
	d->items[10].type = D_END;
	do_dialog(term, d, getml(d, NULL));
}

void menu_set_language(struct terminal *term, void *pcp, struct session *ses)
{
	set_language((int)pcp);
	cls_redraw_all_terminals();
}

void menu_language_list(struct terminal *term, void *xxx, struct session *ses)
{
	int i, sel;
	unsigned char *n;
	struct menu_item *mi;
	if (!(mi = new_menu(1))) return;
	for (i = 0; i < n_languages(); i++) {
		n = language_name(i);
		add_to_menu(&mi, n, "", "", MENU_FUNC menu_set_language, (void *)i, 0);
	}
	sel = current_language;
	do_menu_selected(term, mi, ses, sel);
}

unsigned char *resize_texts[] = { TEXT_(T_COLUMNS), TEXT_(T_ROWS) };

unsigned char x_str[4];
unsigned char y_str[4];

void do_resize_terminal(struct terminal *term)
{
	unsigned char str[8];
	strcpy(str, x_str);
	strcat(str, ",");
	strcat(str, y_str);
	do_terminal_function(term, TERM_FN_RESIZE, str);
}

void dlg_resize_terminal(struct terminal *term, void *xxx, struct session *ses)
{
	struct dialog *d;
	int x = term->x > 999 ? 999 : term->x;
	int y = term->y > 999 ? 999 : term->y;
	sprintf(x_str, "%d", x);
	sprintf(y_str, "%d", y);
	d = mem_alloc(sizeof(struct dialog) + 5 * sizeof(struct dialog_item));
	memset(d, 0, sizeof(struct dialog) + 5 * sizeof(struct dialog_item));
	d->title = TEXT_(T_RESIZE_TERMINAL);
	d->fn = group_fn;
	d->udata = resize_texts;
	d->refresh = (void (*)(void *))do_resize_terminal;
	d->refresh_data = term;
	d->items[0].type = D_FIELD;
	d->items[0].dlen = 4;
	d->items[0].data = x_str;
	d->items[0].fn = check_number;
	d->items[0].gid = 1;
	d->items[0].gnum = 999;
	d->items[1].type = D_FIELD;
	d->items[1].dlen = 4;
	d->items[1].data = y_str;
	d->items[1].fn = check_number;
	d->items[1].gid = 1;
	d->items[1].gnum = 999;
	d->items[2].type = D_BUTTON;
	d->items[2].gid = B_ENTER;
	d->items[2].fn = ok_dialog;
	d->items[2].text = TEXT_(T_OK);
	d->items[3].type = D_BUTTON;
	d->items[3].gid = B_ESC;
	d->items[3].fn = cancel_dialog;
	d->items[3].text = TEXT_(T_CANCEL);
	d->items[4].type = D_END;
	do_dialog(term, d, getml(d, NULL));

}

struct menu_item file_menu11[] = {
	{ TEXT_(T_GOTO_URL), "g", TEXT_(T_HK_GOTO_URL), MENU_FUNC menu_goto_url, (void *)0, 0, 0 },
	{ TEXT_(T_GO_BACK), "<-", TEXT_(T_HK_GO_BACK), MENU_FUNC menu_go_back, (void *)0, 0, 0 },
	{ TEXT_(T_HISTORY), ">", TEXT_(T_HK_HISTORY), MENU_FUNC history_menu, (void *)0, 1, 0 },
	{ TEXT_(T_RELOAD), "Ctrl-R", TEXT_(T_HK_RELOAD), MENU_FUNC menu_reload, (void *)0, 0, 0 },
};

struct menu_item file_menu12[] = {
	{ TEXT_(T_BOOKMARKS), "s", TEXT_(T_HK_BOOKMARKS), MENU_FUNC menu_bookmark_manager, (void *)0, 0, 0 },
	{ TEXT_(T_ADD_BOOKMARK), "a", TEXT_(T_HK_ADD_BOOKMARK), MENU_FUNC launch_bm_add_doc_dialog, (void *)0, 0, 0 },
};

struct menu_item file_menu21[] = {
	{ "", "", M_BAR, NULL, NULL, 0, 0 },
	{ TEXT_(T_SAVE_AS), "", TEXT_(T_HK_SAVE_AS), MENU_FUNC save_as, (void *)0, 0, 0 },
	{ TEXT_(T_SAVE_URL_AS), "", TEXT_(T_HK_SAVE_URL_AS), MENU_FUNC menu_save_url_as, (void *)0, 0, 0 },
	{ TEXT_(T_SAVE_FORMATTED_DOCUMENT), "", TEXT_(T_HK_SAVE_FORMATTED_DOCUMENT), MENU_FUNC menu_save_formatted, (void *)0, 0, 0 },
};

struct menu_item file_menu22[] = {
	{ "", "", M_BAR, NULL, NULL, 0, 0 },
	{ TEXT_(T_KILL_BACKGROUND_CONNECTIONS), "", TEXT_(T_HK_KILL_BACKGROUND_CONNECTIONS), MENU_FUNC menu_kill_background_connections, (void *)0, 0, 0 },
	{ TEXT_(T_FLUSH_ALL_CACHES), "", TEXT_(T_HK_FLUSH_ALL_CACHES), MENU_FUNC flush_caches, (void *)0, 0, 0 },
	{ TEXT_(T_RESOURCE_INFO), "", TEXT_(T_HK_RESOURCE_INFO), MENU_FUNC cache_inf, (void *)0, 0, 0 },
#if 0
	{ TEXT_(T_CACHE_INFO), "", TEXT_(T_HK_CACHE_INFO), MENU_FUNC list_cache, (void *)0, 0, 0 },
#endif
#ifdef LEAK_DEBUG
	{ TEXT_(T_MEMORY_INFO), "", TEXT_(T_HK_MEMORY_INFO), MENU_FUNC memory_info, (void *)0, 0, 0 },
#endif
	{ "", "", M_BAR, NULL, NULL, 0, 0 },
};

struct menu_item file_menu3[] = {
	{ "", "", M_BAR, NULL, NULL, 0, 0 },
	{ TEXT_(T_EXIT), "q", TEXT_(T_HK_EXIT), MENU_FUNC exit_prog, (void *)0, 0, 0 },
	{ NULL, NULL, 0, NULL, NULL, 0, 0 },
};

void do_file_menu(struct terminal *term, void *xxx, struct session *ses)
{
	int x;
	int o;
	struct menu_item *file_menu, *e, *f;
	file_menu = mem_alloc(sizeof(file_menu11) + sizeof(file_menu12) + sizeof(file_menu21) + sizeof(file_menu22) + sizeof(file_menu3) + 3 * sizeof(struct menu_item));
	e = file_menu;
	memcpy(e, file_menu11, sizeof(file_menu11));
	e += sizeof(file_menu11) / sizeof(struct menu_item);
	if (!anonymous) {
		memcpy(e, file_menu12, sizeof(file_menu12));
		e += sizeof(file_menu12) / sizeof(struct menu_item);
	}
	if ((o = can_open_in_new(term))) {
		e->text = TEXT_(T_NEW_WINDOW);
		e->rtext = o - 1 ? ">" : "";
		e->hotkey = TEXT_(T_HK_NEW_WINDOW);
		e->func = MENU_FUNC open_in_new_window;
		e->data = send_open_new_xterm;
		e->in_m = o - 1;
		e->free_i = 0;
		e++;
	}
	if (!anonymous) {
		memcpy(e, file_menu21, sizeof(file_menu21));
		e += sizeof(file_menu21) / sizeof(struct menu_item);
	}
	memcpy(e, file_menu22, sizeof(file_menu22));
	e += sizeof(file_menu22) / sizeof(struct menu_item);
	/*"", "", M_BAR, NULL, NULL, 0, 0,
	TEXT_(T_OS_SHELL), "", TEXT_(T_HK_OS_SHELL), MENU_FUNC menu_shell, NULL, 0, 0,*/
	x = 1;
	if (!anonymous && can_open_os_shell(term->environment)) {
		e->text = TEXT_(T_OS_SHELL);
		e->rtext = "";
		e->hotkey = TEXT_(T_HK_OS_SHELL);
		e->func = MENU_FUNC menu_shell;
		e->data = NULL;
		e->in_m = 0;
		e->free_i = 0;
		e++;
		x = 0;
	}
	if (can_resize_window(term->environment)) {
		e->text = TEXT_(T_RESIZE_TERMINAL);
		e->rtext = "";
		e->hotkey = TEXT_(T_HK_RESIZE_TERMINAL);
		e->func = MENU_FUNC dlg_resize_terminal;
		e->data = NULL;
		e->in_m = 0;
		e->free_i = 0;
		e++;
		x = 0;
	}
	memcpy(e, file_menu3 + x, sizeof(file_menu3) - x * sizeof(struct menu_item));
	e += sizeof(file_menu3) / sizeof(struct menu_item);
	for (f = file_menu; f < e; f++) f->free_i = 1;
	do_menu(term, file_menu, ses);
}

struct menu_item view_menu[] = {
	{ TEXT_(T_SEARCH), "/", TEXT_(T_HK_SEARCH), MENU_FUNC menu_for_frame, (void *)search_dlg, 0, 0 },
	{ TEXT_(T_SEARCH_BACK), "?", TEXT_(T_HK_SEARCH_BACK), MENU_FUNC menu_for_frame, (void *)search_back_dlg, 0, 0 },
	{ TEXT_(T_FIND_NEXT), "n", TEXT_(T_HK_FIND_NEXT), MENU_FUNC menu_for_frame, (void *)find_next, 0, 0 },
	{ TEXT_(T_FIND_PREVIOUS), "N", TEXT_(T_HK_FIND_PREVIOUS), MENU_FUNC menu_for_frame, (void *)find_next_back, 0, 0 },
	{ "", "", M_BAR, NULL, NULL, 0, 0 },
	{ TEXT_(T_TOGGLE_HTML_PLAIN), "\\", TEXT_(T_HK_TOGGLE_HTML_PLAIN), MENU_FUNC menu_toggle, NULL, 0, 0 },
	{ TEXT_(T_DOCUMENT_INFO), "=", TEXT_(T_HK_DOCUMENT_INFO), MENU_FUNC menu_doc_info, NULL, 0, 0 },
	{ TEXT_(T_HEADER_INFO), "|", TEXT_(T_HK_HEADER_INFO), MENU_FUNC menu_head_info, NULL, 0, 0 },
	{ TEXT_(T_FRAME_AT_FULL_SCREEN), "f", TEXT_(T_HK_FRAME_AT_FULL_SCREEN), MENU_FUNC menu_for_frame, (void *)set_frame, 0, 0 },
	{ "", "", M_BAR, NULL, NULL, 0, 0 },
	{ TEXT_(T_HTML_OPTIONS), "", TEXT_(T_HK_HTML_OPTIONS), MENU_FUNC menu_html_options, (void *)0, 0, 0 },
	{ TEXT_(T_SAVE_HTML_OPTIONS), "", TEXT_(T_HK_SAVE_HTML_OPTIONS), MENU_FUNC menu_save_html_options, (void *)0, 0, 0 },
	{ NULL, NULL, 0, NULL, NULL, 0, 0 },
};

struct menu_item view_menu_anon[] = {
	{ TEXT_(T_SEARCH), "/", TEXT_(T_HK_SEARCH), MENU_FUNC menu_for_frame, (void *)search_dlg, 0, 0 },
	{ TEXT_(T_SEARCH_BACK), "?", TEXT_(T_HK_SEARCH_BACK), MENU_FUNC menu_for_frame, (void *)search_back_dlg, 0, 0 },
	{ TEXT_(T_FIND_NEXT), "n", TEXT_(T_HK_FIND_NEXT), MENU_FUNC menu_for_frame, (void *)find_next, 0, 0 },
	{ TEXT_(T_FIND_PREVIOUS), "N", TEXT_(T_HK_FIND_PREVIOUS), MENU_FUNC menu_for_frame, (void *)find_next_back, 0, 0 },
	{ "", "", M_BAR, NULL, NULL, 0, 0 },
	{ TEXT_(T_TOGGLE_HTML_PLAIN), "\\", TEXT_(T_HK_TOGGLE_HTML_PLAIN), MENU_FUNC menu_toggle, NULL, 0, 0 },
	{ TEXT_(T_DOCUMENT_INFO), "=", TEXT_(T_HK_DOCUMENT_INFO), MENU_FUNC menu_doc_info, NULL, 0, 0 },
	{ TEXT_(T_FRAME_AT_FULL_SCREEN), "f", TEXT_(T_HK_FRAME_AT_FULL_SCREEN), MENU_FUNC menu_for_frame, (void *)set_frame, 0, 0 },
	{ "", "", M_BAR, NULL, NULL, 0, 0 },
	{ TEXT_(T_HTML_OPTIONS), "", TEXT_(T_HK_HTML_OPTIONS), MENU_FUNC menu_html_options, (void *)0, 0, 0 },
	{ NULL, NULL, 0, NULL, NULL, 0, 0 },
};

struct menu_item help_menu[] = {
	{ TEXT_(T_ABOUT), "", TEXT_(T_HK_ABOUT), MENU_FUNC menu_about, (void *)0, 0, 0 },
	{ TEXT_(T_KEYS), "", TEXT_(T_HK_KEYS), MENU_FUNC menu_keys, (void *)0, 0, 0 },
	{ TEXT_(T_MANUAL), "", TEXT_(T_HK_MANUAL), MENU_FUNC menu_manual, (void *)0, 0, 0 },
	{ TEXT_(T_COPYING), "", TEXT_(T_HK_COPYING), MENU_FUNC menu_copying, (void *)0, 0, 0 },
	{ NULL, NULL, 0, NULL, NULL, 0, 0 },
};

struct menu_item assoc_menu[] = {
	{ TEXT_(T_ADD), "", TEXT_(T_HK_ADD), MENU_FUNC menu_add_ct, NULL, 0, 0 },
	{ TEXT_(T_MODIFY), ">", TEXT_(T_HK_MODIFY), MENU_FUNC menu_list_assoc, menu_add_ct, 1, 0 },
	{ TEXT_(T_DELETE), ">", TEXT_(T_HK_DELETE), MENU_FUNC menu_list_assoc, menu_del_ct, 1, 0 },
	{ NULL, NULL, 0, NULL, NULL, 0, 0 },
};

struct menu_item ext_menu[] = {
	{ TEXT_(T_ADD), "", TEXT_(T_HK_ADD), MENU_FUNC menu_add_ext, NULL, 0, 0 },
	{ TEXT_(T_MODIFY), ">", TEXT_(T_HK_MODIFY), MENU_FUNC menu_list_ext, menu_add_ext, 1, 0 },
	{ TEXT_(T_DELETE), ">", TEXT_(T_HK_DELETE), MENU_FUNC menu_list_ext, menu_del_ext, 1, 0 },
	{ NULL, NULL, 0, NULL, NULL, 0, 0 },
};

struct menu_item setup_menu[] = {
	{ TEXT_(T_LANGUAGE), ">", TEXT_(T_HK_LANGUAGE), MENU_FUNC menu_language_list, NULL, 1, 0 },
	{ TEXT_(T_CHARACTER_SET), ">", TEXT_(T_HK_CHARACTER_SET), MENU_FUNC charset_list, (void *)1, 1, 0 },
	{ TEXT_(T_TERMINAL_OPTIONS), "", TEXT_(T_HK_TERMINAL_OPTIONS), MENU_FUNC terminal_options, NULL, 0, 0 },
	{ TEXT_(T_NETWORK_OPTIONS), "", TEXT_(T_HK_NETWORK_OPTIONS), MENU_FUNC net_options, NULL, 0, 0 },
	{ TEXT_(T_CACHE), "", TEXT_(T_HK_CACHE), MENU_FUNC cache_opt, NULL, 0, 0 },
	{ TEXT_(T_MAIL_AND_TELNEL), "", TEXT_(T_HK_MAIL_AND_TELNEL), MENU_FUNC net_programs, NULL, 0, 0 },
	{ TEXT_(T_ASSOCIATIONS), ">", TEXT_(T_HK_ASSOCIATIONS), MENU_FUNC do_menu, assoc_menu, 1, 0 },
	{ TEXT_(T_FILE_EXTENSIONS), ">", TEXT_(T_HK_FILE_EXTENSIONS), MENU_FUNC do_menu, ext_menu, 1, 0 },
	{ "", "", M_BAR, NULL, NULL, 0, 0 },
	{ TEXT_(T_SAVE_OPTIONS), "", TEXT_(T_HK_SAVE_OPTIONS), MENU_FUNC write_config, NULL, 0, 0 },
	{ NULL, NULL, 0, NULL, NULL, 0, 0 },
};

struct menu_item setup_menu_anon[] = {
	{ TEXT_(T_LANGUAGE), ">", TEXT_(T_HK_LANGUAGE), MENU_FUNC menu_language_list, NULL, 1, 0 },
	{ TEXT_(T_CHARACTER_SET), ">", TEXT_(T_HK_CHARACTER_SET), MENU_FUNC charset_list, (void *)1, 1, 0 },
	{ TEXT_(T_TERMINAL_OPTIONS), "", TEXT_(T_HK_TERMINAL_OPTIONS), MENU_FUNC terminal_options, NULL, 0, 0 },
	/*{ TEXT_(T_NETWORK_OPTIONS), "", TEXT_(T_HK_NETWORK_OPTIONS), MENU_FUNC net_options, NULL, 0, 0,}*/
	{ NULL, NULL, 0, NULL, NULL, 0, 0 },
};

void do_view_menu(struct terminal *term, void *xxx, struct session *ses)
{
	if (!anonymous) do_menu(term, view_menu, ses);
	else do_menu(term, view_menu_anon, ses);
}

void do_setup_menu(struct terminal *term, void *xxx, struct session *ses)
{
	if (!anonymous) do_menu(term, setup_menu, ses);
	else do_menu(term, setup_menu_anon, ses);
}

struct menu_item main_menu[] = {
	{ TEXT_(T_FILE), "", TEXT_(T_HK_FILE), MENU_FUNC do_file_menu, NULL, 1, 1 },
	{ TEXT_(T_VIEW), "", TEXT_(T_HK_VIEW), MENU_FUNC do_view_menu, NULL, 1, 1 },
	{ TEXT_(T_LINK), "", TEXT_(T_HK_LINK), MENU_FUNC link_menu, NULL, 1, 1 },
	{ TEXT_(T_DOWNLOADS), "", TEXT_(T_HK_DOWNLOADS), MENU_FUNC downloads_menu, NULL, 1, 1 },
	{ TEXT_(T_SETUP), "", TEXT_(T_HK_SETUP), MENU_FUNC do_setup_menu, NULL, 1, 1 },
	{ TEXT_(T_HELP), "", TEXT_(T_HK_HELP), MENU_FUNC do_menu, help_menu, 1, 1 },
	{ NULL, NULL, 0, NULL, NULL, 0, 0 },
};

void activate_bfu_technology(struct session *ses, int item)
{
	struct terminal *term = ses->term;
	do_mainmenu(term, main_menu, ses, item);
}

struct history goto_url_history = { 0, { &goto_url_history.items, &goto_url_history.items } };

void dialog_goto_url(struct session *ses, char *url)
{
	input_field(ses->term, NULL, TEXT_(T_GOTO_URL), TEXT_(T_ENTER_URL), TEXT_(T_OK), TEXT_(T_CANCEL), ses, &goto_url_history, MAX_INPUT_URL_LEN, url, 0, 0, NULL, (void (*)(void *, unsigned char *)) goto_url, NULL);
}

void dialog_save_url(struct session *ses)
{
	input_field(ses->term, NULL, TEXT_(T_SAVE_URL), TEXT_(T_ENTER_URL), TEXT_(T_OK), TEXT_(T_CANCEL), ses, &goto_url_history, MAX_INPUT_URL_LEN, "", 0, 0, NULL, (void (*)(void *, unsigned char *)) save_url, NULL);
}

struct history file_history = { 0, { &file_history.items, &file_history.items } };

void query_file(struct session *ses, unsigned char *url, void (*std)(struct session *, unsigned char *), void (*cancel)(struct session *))
{
	unsigned char *file, *def;
	int dfl = 0;
	int l;
	get_filename_from_url(url, &file, &l);
	def = init_str();
	add_to_str(&def, &dfl, download_dir);
	if (*def && !dir_sep(def[strlen(def) - 1])) add_chr_to_str(&def, &dfl, '/');
	add_bytes_to_str(&def, &dfl, file, l);
	input_field(ses->term, NULL, TEXT_(T_DOWNLOAD), TEXT_(T_SAVE_TO_FILE), TEXT_(T_OK), TEXT_(T_CANCEL), ses, &file_history, MAX_INPUT_URL_LEN, def, 0, 0, NULL, (void (*)(void *, unsigned char *))std, (void (*)(void *))cancel);
	mem_free(def);
}

struct history search_history = { 0, { &search_history.items, &search_history.items } };

void search_back_dlg(struct session *ses, struct f_data_c *f, int a)
{
	input_field(ses->term, NULL, TEXT_(T_SEARCH_BACK), TEXT_(T_SEARCH_FOR_TEXT), TEXT_(T_OK), TEXT_(T_CANCEL), ses, &search_history, MAX_INPUT_URL_LEN, "", 0, 0, NULL, (void (*)(void *, unsigned char *)) search_for_back, NULL);
}

void search_dlg(struct session *ses, struct f_data_c *f, int a)
{
	input_field(ses->term, NULL, TEXT_(T_SEARCH), TEXT_(T_SEARCH_FOR_TEXT), TEXT_(T_OK), TEXT_(T_CANCEL), ses, &search_history, MAX_INPUT_URL_LEN, "", 0, 0, NULL, (void (*)(void *, unsigned char *)) search_for, NULL);
}

void free_history_lists()
{
	free_list(goto_url_history.items);
	free_list(file_history.items);
	free_list(search_history.items);
}

