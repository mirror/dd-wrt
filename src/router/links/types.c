#include "links.h"

struct list_head mailto_prog = { &mailto_prog, &mailto_prog };
struct list_head telnet_prog = { &telnet_prog, &telnet_prog };
struct list_head tn3270_prog = { &tn3270_prog, &tn3270_prog };
struct list_head mms_prog = { &mms_prog, &mms_prog };

struct list_head assoc = { &assoc, &assoc };

tcount get_assoc_cnt()
{
	static tcount assoc_cnt = 0;
	if (!++assoc_cnt) assoc_cnt = 1;
	return assoc_cnt;
}

struct list_head extensions = { &extensions, &extensions };

void delete_association(struct assoc *del)
{
	del_from_list(del);
	mem_free(del->label);
	mem_free(del->ct);
	mem_free(del->prog);
	mem_free(del);
}

void delete_extension(struct extension *del)
{
	del_from_list(del);
	mem_free(del->ext);
	mem_free(del->ct);
	mem_free(del);
}

int is_in_list(unsigned char *list, unsigned char *str, int l)
{
	unsigned char *l2, *l3;
	if (!l) return 0;
	rep:
	while (*list && *list <= ' ') list++;
	if (!*list) return 0;
	for (l2 = list; *l2 && *l2 != ','; l2++) ;
	for (l3 = l2 - 1; l3 >= list && *l3 <= ' '; l3--) ;
	l3++;
	if (l3 - list == l && !casecmp(str, list, l)) return 1;
	list = l2;
	if (*list == ',') list++;
	goto rep;
}

unsigned char *get_content_type(unsigned char *head, unsigned char *url)
{
	struct extension *e;
	struct assoc *a;
	unsigned char *ct, *ext, *exxt;
	int extl, el;
	if (head && (ct = parse_http_header(head, "Content-Type", NULL))) {
		unsigned char *s;
		if ((s = strchr(ct, ';'))) *s = 0;
		while (*ct && ct[strlen(ct) - 1] <= ' ') ct[strlen(ct) - 1] = 0;
		return ct;
	}
	ext = NULL, extl = 0;
	for (ct = url; *ct && !end_of_dir(*ct); ct++)
		if (*ct == '.') ext = ct + 1;
		else if (dir_sep(*ct)) ext = NULL;
	if (ext) while (ext[extl] && !dir_sep(ext[extl]) && !end_of_dir(ext[extl])) extl++;
	if ((extl == 3 && !casecmp(ext, "htm", 3)) ||
	    (extl == 4 && !casecmp(ext, "html", 4))) return stracpy("text/html");
	foreach(e, extensions) if (is_in_list(e->ext, ext, extl)) return stracpy(e->ct);
	exxt = init_str(); el = 0;
	add_to_str(&exxt, &el, "application/x-");
	add_bytes_to_str(&exxt, &el, ext, extl);
	foreach(a, assoc) if (is_in_list(a->ct, exxt, el)) return exxt;
	mem_free(exxt);
	return !force_html ? stracpy("text/plain") : stracpy("text/html");
}

struct assoc *get_type_assoc(struct terminal *term, unsigned char *type)
{
	struct assoc *a;
	foreach(a, assoc) if (a->system == SYSTEM_ID && (term->environment & ENV_XWIN ? a->xwin : a->cons) && is_in_list(a->ct, type, strlen(type))) return a;
	return NULL;
}

void free_types()
{
	struct assoc *a;
	struct extension *e;
	struct protocol_program *p;
	foreach(a, assoc) {
		mem_free(a->ct);
		mem_free(a->prog);
		mem_free(a->label);
	}
	free_list(assoc);
	foreach(e, extensions) {
		mem_free(e->ext);
		mem_free(e->ct);
	}
	free_list(extensions);
	foreach(p, mailto_prog) mem_free(p->prog);
	free_list(mailto_prog);
	foreach(p, telnet_prog) mem_free(p->prog);
	free_list(telnet_prog);
	foreach(p, tn3270_prog) mem_free(p->prog);
	free_list(tn3270_prog);
	foreach(p, mms_prog) mem_free(p->prog);
	free_list(mms_prog);
}

unsigned char *ct_msg[] = {
	TEXT_(T_LABEL),
	TEXT_(T_CONTENT_TYPES),
	TEXT_(T_PROGRAM__IS_REPLACED_WITH_FILE_NAME),
#ifdef ASSOC_BLOCK
	TEXT_(T_BLOCK_TERMINAL_WHILE_PROGRAM_RUNNING),
#endif
#ifdef ASSOC_CONS_XWIN
	TEXT_(T_RUN_ON_TERMINAL),
	TEXT_(T_RUN_IN_XWINDOW),
#endif
	TEXT_(T_ASK_BEFORE_OPENING),
};

void add_ct_fn(struct dialog_data *dlg)
{
	struct terminal *term = dlg->win->term;
	int max = 0, min = 0;
	int w, rw;
	int y = -1;
	int p = 1;
#ifdef ASSOC_BLOCK
	p++;
#endif
#ifdef ASSOC_CONS_XWIN
	p += 2;
#endif
	max_text_width(term, ct_msg[0], &max);
	min_text_width(term, ct_msg[0], &min);
	max_text_width(term, ct_msg[1], &max);
	min_text_width(term, ct_msg[1], &min);
	max_text_width(term, ct_msg[2], &max);
	min_text_width(term, ct_msg[2], &min);
	max_group_width(term, ct_msg + 3, dlg->items + 3, p, &max);
	min_group_width(term, ct_msg + 3, dlg->items + 3, p, &min);
	max_buttons_width(term, dlg->items + 3 + p, 2, &max);
	min_buttons_width(term, dlg->items + 3 + p, 2, &min);
	w = term->x * 9 / 10 - 2 * DIALOG_LB;
	if (w > max) w = max;
	if (w < min) w = min;
	if (w > term->x - 2 * DIALOG_LB) w = term->x - 2 * DIALOG_LB;
	if (w < 1) w = 1;
	rw = 0;
	dlg_format_text(NULL, term, _(ct_msg[0], term), 0, &y, w, &rw, COLOR_DIALOG_TEXT, AL_LEFT);
	y += 2;
	dlg_format_text(NULL, term, _(ct_msg[1], term), 0, &y, w, &rw, COLOR_DIALOG_TEXT, AL_LEFT);
	y += 2;
	dlg_format_text(NULL, term, _(ct_msg[2], term), 0, &y, w, &rw, COLOR_DIALOG_TEXT, AL_LEFT);
	y += 2;
	dlg_format_group(NULL, term, ct_msg + 3, dlg->items + 3, p, 0, &y, w, &rw);
	y++;
	dlg_format_buttons(NULL, term, dlg->items + 3 + p, 2, 0, &y, w, &rw, AL_CENTER);
	w = rw;
	dlg->xw = w + 2 * DIALOG_LB;
	dlg->yw = y + 2 * DIALOG_TB;
	center_dlg(dlg);
	draw_dlg(dlg);
	y = dlg->y + DIALOG_TB;
	dlg_format_text(term, term, ct_msg[0], dlg->x + DIALOG_LB, &y, w, NULL, COLOR_DIALOG_TEXT, AL_LEFT);
	dlg_format_field(term, term, &dlg->items[0], dlg->x + DIALOG_LB, &y, w, NULL, AL_LEFT);
	y++;
	dlg_format_text(term, term, ct_msg[1], dlg->x + DIALOG_LB, &y, w, NULL, COLOR_DIALOG_TEXT, AL_LEFT);
	dlg_format_field(term, term, &dlg->items[1], dlg->x + DIALOG_LB, &y, w, NULL, AL_LEFT);
	y++;
	dlg_format_text(term, term, ct_msg[2], dlg->x + DIALOG_LB, &y, w, NULL, COLOR_DIALOG_TEXT, AL_LEFT);
	dlg_format_field(term, term, &dlg->items[2], dlg->x + DIALOG_LB, &y, w, NULL, AL_LEFT);
	y++;
	dlg_format_group(term, term, ct_msg + 3, &dlg->items[3], p, dlg->x + DIALOG_LB, &y, w, NULL);
	y++;
	dlg_format_buttons(term, term, &dlg->items[3 + p], 2, dlg->x + DIALOG_LB, &y, w, NULL, AL_CENTER);
}

void update_assoc(struct assoc *new)
{
	struct assoc *repl;
	if (!new->label[0] || !new->ct[0] || !new->prog[0]) return;
	if (new->cnt) {
		foreach(repl, assoc) if (repl->cnt == new->cnt) {
			mem_free(repl->label);
			mem_free(repl->ct);
			mem_free(repl->prog);
			goto replace;
		}
		return;
	}
	foreach(repl, assoc) if (!strcmp(repl->label, new->label) && !strcmp(repl->ct, new->ct) && !strcmp(repl->prog, new->prog) && repl->block == new->block && repl->cons == new->cons && repl->xwin == new->xwin && repl->ask == new->ask && repl->system == new->system) {
		del_from_list(repl);
		add_to_list(assoc, repl);
		return;
	}
	new->cnt = get_assoc_cnt();
	repl = mem_alloc(sizeof(struct assoc));
	add_to_list(assoc, repl);
	replace:
	repl->label = stracpy(new->label);
	repl->ct = stracpy(new->ct);
	repl->prog = stracpy(new->prog);
	repl->block = new->block;
	repl->cons = new->cons;
	repl->xwin = new->xwin;
	repl->ask = new->ask;
	repl->system = new->system;
	repl->cnt = new->cnt;
}

void really_del_ct(void *fcp)
{
	tcount fc = (int)fcp;
	struct assoc *del;
	foreach(del, assoc) if (del->cnt == fc) goto ok;
	return;
	ok:
	delete_association(del);
}

void menu_del_ct(struct terminal *term, void *fcp, void *xxx2)
{
	unsigned char *str;
	int l;
	tcount fc = (int)fcp;
	struct assoc *del;
	foreach(del, assoc) if (del->cnt == fc) goto ok;
	return;
	ok:
	str = init_str(), l = 0;
	add_to_str(&str, &l, del->ct);
	add_to_str(&str, &l, " -> ");
	add_to_str(&str, &l, del->prog);
	msg_box(term, getml(str, NULL), TEXT_(T_DELETE_ASSOCIATION), AL_CENTER | AL_EXTD_TEXT, TEXT_(T_DELETE_ASSOCIATION), ": ", str, "?", NULL, fcp, 2, TEXT_(T_YES), really_del_ct, B_ENTER, TEXT_(T_NO), NULL, B_ESC);
}

void menu_add_ct(struct terminal *term, void *fcp, void *xxx2)
{
	int p;
	tcount fc = (int)fcp;
	struct assoc *new, *from;
	unsigned char *label;
	unsigned char *ct;
	unsigned char *prog;
	struct dialog *d;
	if (fc) {
		foreach(from, assoc) if (from->cnt == fc) goto ok;
		return;
	}
	from = NULL;
	ok:
	d = mem_alloc(sizeof(struct dialog) + 10 * sizeof(struct dialog_item) + sizeof(struct assoc) + 3 * MAX_STR_LEN);
	memset(d, 0, sizeof(struct dialog) + 10 * sizeof(struct dialog_item) + sizeof(struct assoc) + 3 * MAX_STR_LEN);
	new = (struct assoc *)&d->items[10];
	new->label = label = (unsigned char *)(new + 1);
	new->ct = ct = label + MAX_STR_LEN;
	new->prog = prog = ct + MAX_STR_LEN;
	if (from) {
		safe_strncpy(label, from->label, MAX_STR_LEN);
		safe_strncpy(ct, from->ct, MAX_STR_LEN);
		safe_strncpy(prog, from->prog, MAX_STR_LEN);
		new->block = from->block;
		new->cons = from->cons;
		new->xwin = from->xwin;
		new->ask = from->ask;
		new->system = from->system;
		new->cnt = from->cnt;
	} else {
		new->block = new->xwin = new->cons = 1;
		new->ask = 1;
		new->system = SYSTEM_ID;
	}
	d->title = TEXT_(T_ASSOCIATION);
	d->fn = add_ct_fn;
	d->refresh = (void (*)(void *))update_assoc;
	d->refresh_data = new;
	d->items[0].type = D_FIELD;
	d->items[0].dlen = MAX_STR_LEN;
	d->items[0].data = label;
	d->items[0].fn = check_nonempty;
	d->items[1].type = D_FIELD;
	d->items[1].dlen = MAX_STR_LEN;
	d->items[1].data = ct;
	d->items[1].fn = check_nonempty;
	d->items[2].type = D_FIELD;
	d->items[2].dlen = MAX_STR_LEN;
	d->items[2].data = prog;
	d->items[2].fn = check_nonempty;
	p = 3;
#ifdef ASSOC_BLOCK
	d->items[p].type = D_CHECKBOX;
	d->items[p].data = (unsigned char *)&new->block;
	d->items[p++].dlen = sizeof(int);
#endif
#ifdef ASSOC_CONS_XWIN
	d->items[p].type = D_CHECKBOX;
	d->items[p].data = (unsigned char *)&new->cons;
	d->items[p++].dlen = sizeof(int);
	d->items[p].type = D_CHECKBOX;
	d->items[p].data = (unsigned char *)&new->xwin;
	d->items[p++].dlen = sizeof(int);
#endif
	d->items[p].type = D_CHECKBOX;
	d->items[p].data = (unsigned char *)&new->ask;
	d->items[p++].dlen = sizeof(int);
	d->items[p].type = D_BUTTON;
	d->items[p].gid = B_ENTER;
	d->items[p].fn = ok_dialog;
	d->items[p++].text = TEXT_(T_OK);
	d->items[p].type = D_BUTTON;
	d->items[p].gid = B_ESC;
	d->items[p].text = TEXT_(T_CANCEL);
	d->items[p++].fn = cancel_dialog;
	d->items[p++].type = D_END;
	do_dialog(term, d, getml(d, NULL));
}

struct menu_item mi_no_assoc[] = {
	{ TEXT_(T_NO_ASSOCIATIONS), "", M_BAR, NULL, NULL, 0, 0 },
	{ NULL, NULL, 0, NULL, NULL, 0, 0 },
};

void menu_list_assoc(struct terminal *term, void *fn, void *xxx)
{
	struct assoc *a;
	struct menu_item *mi = NULL;
	int n = 0;
	foreachback(a, assoc) if (a->system == SYSTEM_ID) {
		if (!mi && !(mi = new_menu(7))) return;
		add_to_menu(&mi, stracpy(a->label), stracpy(a->ct), "", MENU_FUNC fn, (void *)a->cnt, 0), n++;
	}
	if (!mi) do_menu(term, mi_no_assoc, xxx);
	else do_menu(term, mi, xxx);
}

unsigned char *ext_msg[] = {
	TEXT_(T_EXTENSION_S),
	TEXT_(T_CONTENT_TYPE),
};

void add_ext_fn(struct dialog_data *dlg)
{
	struct terminal *term = dlg->win->term;
	int max = 0, min = 0;
	int w, rw;
	int y = -1;
	max_text_width(term, ext_msg[0], &max);
	min_text_width(term, ext_msg[0], &min);
	max_text_width(term, ext_msg[1], &max);
	min_text_width(term, ext_msg[1], &min);
	max_buttons_width(term, dlg->items + 2, 2, &max);
	min_buttons_width(term, dlg->items + 2, 2, &min);
	w = term->x * 9 / 10 - 2 * DIALOG_LB;
	if (w > max) w = max;
	if (w < min) w = min;
	if (w > term->x - 2 * DIALOG_LB) w = term->x - 2 * DIALOG_LB;
	if (w < 1) w = 1;
	rw = 0;
	dlg_format_text(NULL, term, ext_msg[0], 0, &y, w, &rw, COLOR_DIALOG_TEXT, AL_LEFT);
	y += 2;
	dlg_format_text(NULL, term, ext_msg[1], 0, &y, w, &rw, COLOR_DIALOG_TEXT, AL_LEFT);
	y += 2;
	dlg_format_buttons(NULL, term, dlg->items + 2, 2, 0, &y, w, &rw, AL_CENTER);
	w = rw;
	dlg->xw = w + 2 * DIALOG_LB;
	dlg->yw = y + 2 * DIALOG_TB;
	center_dlg(dlg);
	draw_dlg(dlg);
	y = dlg->y + DIALOG_TB;
	dlg_format_text(term, term, ext_msg[0], dlg->x + DIALOG_LB, &y, w, NULL, COLOR_DIALOG_TEXT, AL_LEFT);
	dlg_format_field(term, term, &dlg->items[0], dlg->x + DIALOG_LB, &y, w, NULL, AL_LEFT);
	y++;
	dlg_format_text(term, term, ext_msg[1], dlg->x + DIALOG_LB, &y, w, NULL, COLOR_DIALOG_TEXT, AL_LEFT);
	dlg_format_field(term, term, &dlg->items[1], dlg->x + DIALOG_LB, &y, w, NULL, AL_LEFT);
	y++;
	dlg_format_buttons(term, term, &dlg->items[2], 2, dlg->x + DIALOG_LB, &y, w, NULL, AL_CENTER);
}

void update_ext(struct extension *new)
{
	struct extension *repl;
	if (!new->ext[0] || !new->ct[0]) return;
	if (new->cnt) {
		foreach(repl, extensions) if (repl->cnt == new->cnt) {
			mem_free(repl->ext);
			mem_free(repl->ct);
			goto replace;
		}
		return;
	}
	foreach(repl, extensions) if (!strcmp(repl->ext, new->ext) && !strcmp(repl->ct, new->ct)) {
		del_from_list(repl);
		add_to_list(extensions, repl);
		return;
	}
	new->cnt = get_assoc_cnt();
	repl = mem_alloc(sizeof(struct extension));
	add_to_list(extensions, repl);
	replace:
	repl->ext = stracpy(new->ext);
	repl->ct = stracpy(new->ct);
	repl->cnt = new->cnt;
}

void really_del_ext(void *fcp)
{
	tcount fc = (int)fcp;
	struct extension *del;
	foreach(del, extensions) if (del->cnt == fc) goto ok;
	return;
	ok:
	delete_extension(del);
}

void menu_del_ext(struct terminal *term, void *fcp, void *xxx2)
{
	unsigned char *str;
	int l;
	tcount fc = (int)fcp;
	struct extension *del;
	foreach(del, extensions) if (del->cnt == fc) goto ok;
	return;
	ok:
	str = init_str(), l = 0;
	add_to_str(&str, &l, del->ext);
	add_to_str(&str, &l, " -> ");
	add_to_str(&str, &l, del->ct);
	msg_box(term, getml(str, NULL), TEXT_(T_DELETE_EXTENSION), AL_CENTER | AL_EXTD_TEXT, TEXT_(T_DELETE_EXTENSION), " ", str, "?", NULL, fcp, 2, TEXT_(T_YES), really_del_ext, B_ENTER, TEXT_(T_NO), NULL, B_ESC);
}

void menu_add_ext(struct terminal *term, void *fcp, void *xxx2)
{
	tcount fc = (int)fcp;
	struct extension *new, *from;
	unsigned char *ext;
	unsigned char *ct;
	struct dialog *d;
	if (fc) {
		foreach(from, extensions) if (from->cnt == fc) goto ok;
		return;
	}
	from = NULL;
	ok:
	d = mem_alloc(sizeof(struct dialog) + 5 * sizeof(struct dialog_item) + sizeof(struct extension) + 2 * MAX_STR_LEN);
	memset(d, 0, sizeof(struct dialog) + 5 * sizeof(struct dialog_item) + sizeof(struct extension) + 2 * MAX_STR_LEN);
	new = (struct extension *)&d->items[5];
	new->ext = ext = (unsigned char *)(new + 1);
	new->ct = ct = ext + MAX_STR_LEN;
	if (from) {
		safe_strncpy(ext, from->ext, MAX_STR_LEN);
		safe_strncpy(ct, from->ct, MAX_STR_LEN);
		new->cnt = from->cnt;
	}
	d->title = TEXT_(T_EXTENSION);
	d->fn = add_ext_fn;
	d->refresh = (void (*)(void *))update_ext;
	d->refresh_data = new;
	d->items[0].type = D_FIELD;
	d->items[0].dlen = MAX_STR_LEN;
	d->items[0].data = ext;
	d->items[0].fn = check_nonempty;
	d->items[1].type = D_FIELD;
	d->items[1].dlen = MAX_STR_LEN;
	d->items[1].data = ct;
	d->items[1].fn = check_nonempty;
	d->items[2].type = D_BUTTON;
	d->items[2].gid = B_ENTER;
	d->items[2].fn = ok_dialog;
	d->items[2].text = TEXT_(T_OK);
	d->items[3].type = D_BUTTON;
	d->items[3].gid = B_ESC;
	d->items[3].text = TEXT_(T_CANCEL);
	d->items[3].fn = cancel_dialog;
	d->items[4].type = D_END;
	do_dialog(term, d, getml(d, NULL));
}

struct menu_item mi_no_ext[] = {
	{ TEXT_(T_NO_EXTENSIONS), "", M_BAR, NULL, NULL, 0, 0 },
	{ NULL, NULL, 0, NULL, NULL, 0, 0 },
};

void menu_list_ext(struct terminal *term, void *fn, void *xxx)
{
	struct extension *a;
	struct menu_item *mi = NULL;
	int n = 0;
	foreachback(a, extensions) {
		if (!mi && !(mi = new_menu(7))) return;
		add_to_menu(&mi, stracpy(a->ext), stracpy(a->ct), "", MENU_FUNC fn, (void *)a->cnt, 0), n++;
	}
	if (!mi) do_menu(term, mi_no_ext, xxx);
	else do_menu(term, mi, xxx);
}

void update_prog(struct list_head *l, unsigned char *p, int s)
{
	struct protocol_program *repl;
	foreach(repl, *l) if (repl->system == s) {
		mem_free(repl->prog);
		goto ss;
	}
	repl = mem_alloc(sizeof(struct protocol_program));
	add_to_list(*l, repl);
	repl->system = s;
	ss:
	repl->prog = mem_alloc(MAX_STR_LEN);
	safe_strncpy(repl->prog, p, MAX_STR_LEN);
}

unsigned char *get_prog(struct list_head *l)
{
	struct protocol_program *repl;
	foreach(repl, *l) if (repl->system == SYSTEM_ID) return repl->prog;
	update_prog(l, "", SYSTEM_ID);
	foreach(repl, *l) if (repl->system == SYSTEM_ID) return repl->prog;
	return NULL;
}

int is_html_type(unsigned char *ct)
{
	return !strcasecmp(ct, "text/html") || !strcasecmp(ct, "text/x-server-parsed-html") || !casecmp(ct, "application/xhtml", strlen("application/xhtml"));
}
