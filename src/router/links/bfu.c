#include "links.h"

struct memory_list *getml(void *p, ...)
{
	struct memory_list *ml;
	va_list ap;
	int n = 0;
	void *q = p;
	va_start(ap, p);
	while (q) {
		if (n == MAXINT) overalloc();
		n++, q = va_arg(ap, void *);
	}
	if ((unsigned)n > (MAXINT - sizeof(struct memory_list)) / sizeof(void *)) overalloc();
	ml = mem_alloc(sizeof(struct memory_list) + n * sizeof(void *));
	ml->n = n;
	n = 0;
	q = p;
	va_end(ap);
	va_start(ap, p);
	while (q) ml->p[n++] = q, q = va_arg(ap, void *);
	va_end(ap);
	return ml;
}

void add_to_ml(struct memory_list **ml, ...)
{
	struct memory_list *nml;
	va_list ap;
	int n = 0;
	void *q;
	if (!*ml) {
		*ml = mem_alloc(sizeof(struct memory_list));
		(*ml)->n = 0;
	}
	va_start(ap, ml);
	while ((q = va_arg(ap, void *))) {
		if (n == MAXINT) overalloc();
		n++;
	}
	if ((unsigned)n + (unsigned)((*ml)->n) > (MAXINT - sizeof(struct memory_list)) / sizeof(void *)) overalloc();
	nml = mem_realloc(*ml, sizeof(struct memory_list) + (n + (*ml)->n) * sizeof(void *));
	va_end(ap);
	va_start(ap, ml);
	while ((q = va_arg(ap, void *))) nml->p[nml->n++] = q;
	*ml = nml;
	va_end(ap);
}

void freeml(struct memory_list *ml)
{
	int i;
	if (!ml) return;
	for (i = 0; i < ml->n; i++) mem_free(ml->p[i]);
	mem_free(ml);
}

unsigned char m_bar = 0;

void menu_func(struct window *, struct event *, int);
void mainmenu_func(struct window *, struct event *, int);
void dialog_func(struct window *, struct event *, int);

void do_menu_selected(struct terminal *term, struct menu_item *items, void *data, int selected)
{
	struct menu *menu;
	menu = mem_alloc(sizeof(struct menu));
	menu->selected = selected;
	menu->view = 0;
	menu->items = items;
	menu->data = data;
	add_window(term, menu_func, menu);
}

void do_menu(struct terminal *term, struct menu_item *items, void *data)
{
	do_menu_selected(term, items, data, 0);
}

void select_menu(struct terminal *term, struct menu *menu)
{
	/*int x = menu->x + 4;
	int y = menu->y + menu->selected - menu->view + 2;*/
	struct menu_item *it = &menu->items[menu->selected];
	void (*func)(struct terminal *, void *, void *) = it->func;
	void *data1 = it->data;
	void *data2 = menu->data;
	if (menu->selected < 0 || menu->selected >= menu->ni || it->hotkey == M_BAR) return;
	if (!it->in_m) {
		struct window *win, *win1;
		for (win = term->windows.next; (void *)win != &term->windows && (win->handler == menu_func || win->handler == mainmenu_func); win1 = win->next, delete_window(win), win = win1) ;
	}
	func(term, data1, data2);
}

void count_menu_size(struct terminal *term, struct menu *menu)
{
	int sx = term->x;
	int sy = term->y;
	int mx = 4;
	int my;
	for (my = 0; menu->items[my].text; my++) {
		int s = strlen(_(menu->items[my].text, term)) + strlen(_(menu->items[my].rtext, term)) + MENU_HOTKEY_SPACE * (_(menu->items[my].rtext, term)[0] != 0) + 4;
		if (s > mx) mx = s;
	}
	menu->ni = my;
	my += 2;
	if (mx > sx) mx = sx;
	if (my > sy) my = sy;
	menu->xw = mx;
	menu->yw = my;
	if ((menu->x = menu->xp) < 0) menu->x = 0;
	if ((menu->y = menu->yp) < 0) menu->y = 0;
	if (menu->x + mx > sx) menu->x = sx - mx;
	if (menu->y + my > sy) menu->y = sy - my;
}

void scroll_menu(struct menu *menu, int d)
{
	int c = 0;
	int w = menu->yw - 2;
	int scr_i = SCROLL_ITEMS > (w-1)/2 ? (w-1)/2 : SCROLL_ITEMS;
	if (scr_i < 0) scr_i = 0;
	if (w < 0) w = 0;
	menu->selected += d;
	while (1) {
		if (c++ > menu->ni) {
			menu->selected = -1;
			menu->view = 0;
			return;
		}
		if (menu->selected < 0) menu->selected = 0;
		if (menu->selected >= menu->ni) menu->selected = menu->ni - 1;
		/*if (menu->selected < 0) menu->selected = menu->ni - 1;
		if (menu->selected >= menu->ni) menu->selected = 0;*/
		if (menu->ni && menu->items[menu->selected].hotkey != M_BAR) break;
		menu->selected += d;
	}
	/*debug("1:%d %d %d %d", menu->ni, w, menu->view, menu->selected);*/
	if (menu->selected < menu->view + scr_i) menu->view = menu->selected - scr_i;
	if (menu->selected >= menu->view + w - scr_i - 1) menu->view = menu->selected - w + scr_i + 1;
	if (menu->view > menu->ni - w) menu->view = menu->ni - w;
	if (menu->view < 0) menu->view = 0;
	/*debug("2:%d %d %d %d", menu->ni, w, menu->view, menu->selected);*/
}

void display_menu(struct terminal *term, struct menu *menu)
{
	int p, s;
	fill_area(term, menu->x+1, menu->y+1, menu->xw-2, menu->yw-2, COLOR_MENU);
	draw_frame(term, menu->x, menu->y, menu->xw, menu->yw, COLOR_MENU_FRAME, 1);
	for (p = menu->view, s = menu->y + 1; p < menu->ni && p < menu->view + menu->yw - 2; p++, s++) {
		int x;
		int h = 0;
		unsigned char c;
		unsigned char *tmptext = _(menu->items[p].text, term);
		int co = p == menu->selected ? h = 1, COLOR_MENU_SELECTED : COLOR_MENU;
		if (h) {
			set_cursor(term, menu->x+1, s, term->x - 1, term->y - 1);
			/*set_window_ptr(menu->win, menu->x+3, s+1);*/
			set_window_ptr(menu->win, menu->x+menu->xw, s);
			fill_area(term, menu->x+1, s, menu->xw-2, 1, co);
		}
		if (menu->items[p].hotkey != M_BAR || (tmptext && tmptext[0])) {
			int l = strlen(_(menu->items[p].rtext, term));
			for (x = l - 1; x >= 0 && menu->xw - 4 >= l - x && (c = _(menu->items[p].rtext, term)[x]); x--)
				set_char(term, menu->x + menu->xw - 2 - l + x, s, c | co);
			for (x = 0; x < menu->xw - 4 && (c = tmptext[x]); x++)
				set_char(term, menu->x + x + 2, s, !h && strchr(_(menu->items[p].hotkey, term), upcase(c)) ? h = 1, COLOR_MENU_HOTKEY | c : co | c);
		} else {
			set_char(term, menu->x, s, COLOR_MENU_FRAME | ATTR_FRAME | 0xc3);
			fill_area(term, menu->x+1, s, menu->xw-2, 1, COLOR_MENU_FRAME | ATTR_FRAME | 0xc4);
			set_char(term, menu->x+menu->xw-1, s, COLOR_MENU_FRAME | ATTR_FRAME | 0xb4);
		}
	}
	redraw_from_window(menu->win);
}

void menu_func(struct window *win, struct event *ev, int fwd)
{
	int s = 0;
	struct menu *menu = win->data;
	struct window *w1;
	menu->win = win;
	switch ((int)ev->ev) {
		case EV_INIT:
		case EV_RESIZE:
		case EV_REDRAW:
			get_parent_ptr(win, &menu->xp, &menu->yp);
			count_menu_size(win->term, menu);
			menu->selected--;
			scroll_menu(menu, 1);
		/*case EV_REDRAW:*/
			display_menu(win->term, menu);
			break;
		case EV_MOUSE:
			if (ev->x < menu->x || ev->x >= menu->x+menu->xw || ev->y < menu->y || ev->y >= menu->y+menu->yw) {
				if ((ev->b & BM_ACT) == B_DOWN) del:delete_window_ev(win, ev);
				else for (w1 = win; (void *)w1 != &win->term->windows; w1 = w1->next) {
					struct menu *m1;
					if (w1->handler == mainmenu_func) {
						if (!ev->y) goto del;
						break;
					}
					if (w1->handler != menu_func) break;
					m1 = w1->data;
					if (ev->x > m1->x && ev->x < m1->x+m1->xw-1 && ev->y > m1->y && ev->y < m1->y+m1->yw-1) goto del;
				}
			} else {
				if (!(ev->x < menu->x || ev->x >= menu->x+menu->xw || ev->y < menu->y+1 || ev->y >= menu->y+menu->yw-1)) {
					int s = ev->y - menu->y-1 + menu->view;
					if (s >= 0 && s < menu->ni && menu->items[s].hotkey != M_BAR) {
						menu->selected = s;
						scroll_menu(menu, 0);
						display_menu(win->term, menu);
						if ((ev->b & BM_ACT) == B_UP/* || menu->items[s].in_m*/) select_menu(win->term, menu);
					}
				}
			}
			break;
		case EV_KBD:
			switch (kbd_action(KM_MENU, ev)) {
				case ACT_LEFT:
				case ACT_RIGHT:
					if ((void *)win->next != &win->term->windows && win->next->handler == mainmenu_func) goto mm;
					/*for (w1 = win; (void *)w1 != &win->term->windows; w1 = w1->next) {
						if (w1->handler == mainmenu_func) goto mm;
						if (w1->handler != menu_func) break;
					}*/
					if (kbd_action(KM_MENU, ev) == ACT_RIGHT) goto enter;
					delete_window(win);
					goto break2;
				case ACT_UP: scroll_menu(menu, -1); break;
				case ACT_DOWN: scroll_menu(menu, 1); break;
				case ACT_HOME: menu->selected = -1, scroll_menu(menu, 1); break;
				case ACT_END: menu->selected = menu->ni, scroll_menu(menu, -1); break;
				case ACT_PAGE_UP:
					if ((menu->selected -= menu->yw - 3) < -1) menu->selected = -1;
					if ((menu->view -= menu->yw - 2) < 0) menu->view = 0;
					scroll_menu(menu, -1);
					break;
				case ACT_PAGE_DOWN:
					if ((menu->selected += menu->yw - 3) > menu->ni) menu->selected = menu->ni;
					if ((menu->view += menu->yw - 2) >= menu->ni - menu->yw + 2) menu->view = menu->ni - menu->yw + 2;
					scroll_menu(menu, 1);
					break;
				default:
					if ((ev->x >= KBD_F1 && ev->x <= KBD_F12) || ev->y == KBD_ALT) {
						mm:
						delete_window_ev(win, ev);
						goto break2;
					}
					if (ev->x == KBD_ESC) {
						delete_window_ev(win, (void *)win->next != &win->term->windows && win->next->handler == mainmenu_func ? ev : NULL);
						goto break2;
					}
					if (ev->x > ' ' && ev->x < 256) {
						int i;
						for (i = 0; i < menu->ni; i++)
							if (strchr(_(menu->items[i].hotkey, win->term), upcase(ev->x))) {
								menu->selected = i;
								scroll_menu(menu, 0);
								s = 1;
							}
					}
					break;
			}
			display_menu(win->term, menu);
			if (s || ev->x == KBD_ENTER || ev->x == ' ') {
				enter:
				select_menu(win->term, menu);
			}
			break2:
			break;
		case EV_ABORT:
			if (menu->items->free_i) {
				int i;
				for (i = 0; menu->items[i].text; i++) {
					if (menu->items[i].free_i & 2) mem_free(menu->items[i].text);
					if (menu->items[i].free_i & 4) mem_free(menu->items[i].rtext);
				}
				mem_free(menu->items);
			}
			break;
	}
}

void do_mainmenu(struct terminal *term, struct menu_item *items, void *data, int sel)
{
	struct mainmenu *menu;
	menu = mem_alloc(sizeof(struct mainmenu));
	menu->selected = sel == -1 ? 0 : sel;
	menu->items = items;
	menu->data = data;
	add_window(term, mainmenu_func, menu);
	if (sel != -1) {
		struct event ev = {EV_KBD, KBD_ENTER, 0, 0};
		struct window *win = term->windows.next;
		win->handler(win, &ev, 0);
	}
}

void display_mainmenu(struct terminal *term, struct mainmenu *menu)
{
	int i;
	int p = 2;
	fill_area(term, 0, 0, term->x, 1, COLOR_MAINMENU | ' ');
	for (i = 0; menu->items[i].text; i++) {
		int s = 0;
		int j;
		unsigned char c;
		unsigned char *tmptext = _(menu->items[i].text, term);
		int co = i == menu->selected ? s = 1, COLOR_MAINMENU_SELECTED : COLOR_MAINMENU;
		if (s) {
			fill_area(term, p, 0, 2, 1, co);
			fill_area(term, p+strlen(tmptext)+2, 0, 2, 1, co);
			menu->sp = p;
			set_cursor(term, p, 0, term->x - 1, term->y - 1);
			set_window_ptr(menu->win, p, 1);
		}
		p += 2;
		for (j = 0; (c = tmptext[j]); j++, p++)
			set_char(term, p, 0, (!s && strchr(_(menu->items[i].hotkey, term), upcase(c)) ? s = 1, COLOR_MAINMENU_HOTKEY : co) | c);
		p += 2;
	}
	menu->ni = i;
	redraw_from_window(menu->win);
}

void select_mainmenu(struct terminal *term, struct mainmenu *menu)
{
	struct menu_item *it = &menu->items[menu->selected];
	if (menu->selected < 0 || menu->selected >= menu->ni || it->hotkey == M_BAR) return;
	if (!it->in_m) {
		struct window *win, *win1;
		for (win = term->windows.next; (void *)win != &term->windows && (win->handler == menu_func || win->handler == mainmenu_func); win1 = win->next, delete_window(win), win = win1) ;
	}
	it->func(term, it->data, menu->data);
}

void mainmenu_func(struct window *win, struct event *ev, int fwd)
{
	int s = 0;
	struct mainmenu *menu = win->data;
	menu->win = win;
	switch ((int)ev->ev) {
		case EV_INIT:
		case EV_RESIZE:
		case EV_REDRAW:
			display_mainmenu(win->term, menu);
			break;
		case EV_MOUSE:
			if ((ev->b & BM_ACT) == B_DOWN && ev->y) delete_window_ev(win, ev);
			else if (!ev->y) {
				int i;
				int p = 2;
				for (i = 0; i < menu->ni; i++) {
					int o = p;
					unsigned char *tmptext = _(menu->items[i].text, win->term);
					p += strlen(tmptext) + 4;
					if (ev->x >= o && ev->x < p) {
						menu->selected = i;
						display_mainmenu(win->term, menu);
						if ((ev->b & BM_ACT) == B_UP || menu->items[s].in_m) select_mainmenu(win->term, menu);
						break;
					}
				}
			}
			break;
		case EV_KBD:
			if (ev->x == ' ' || ev->x == KBD_ENTER || ev->x == KBD_DOWN || ev->x == KBD_UP || ev->x == KBD_PAGE_DOWN || ev->x == KBD_PAGE_UP) {
				select_mainmenu(win->term, menu);
				break;
			}
			if (ev->x == KBD_LEFT) {
				if (!menu->selected--) menu->selected = menu->ni - 1;
				s = 1;
			}
			if (ev->x == KBD_RIGHT) {
				if (++menu->selected >= menu->ni) menu->selected = 0;
				s = 1;
			}
			if (ev->x == KBD_HOME) {
				menu->selected = 0;
				s = 1;
			}
			if (ev->x == KBD_END) {
				menu->selected = menu->ni - 1;
				s = 1;
			}
			if ((ev->x == KBD_LEFT || ev->x == KBD_RIGHT) && fwd) {
				display_mainmenu(win->term, menu);
				select_mainmenu(win->term, menu);
				break;
			}
			if (ev->x > ' ' && ev->x < 256) {
				int i;
				s = 1;
				for (i = 0; i < menu->ni; i++)
					if (strchr(_(menu->items[i].hotkey, win->term), upcase(ev->x))) {
						menu->selected = i;
						s = 2;
					}
			} else if (!s) {
				delete_window_ev(win, (ev->x >= KBD_F1 && ev->x <= KBD_F12) || ev->y & KBD_ALT ? ev : NULL);
				break;
			}
			display_mainmenu(win->term, menu);
			if (s == 2) select_mainmenu(win->term, menu);
			break;
		case EV_ABORT:
			break;
	}
}

struct menu_item *new_menu(int free_i)
{
	struct menu_item *mi;
	mi = mem_alloc(sizeof(struct menu_item));
	memset(mi, 0, sizeof(struct menu_item));
	mi->free_i = free_i;
	return mi;
}

void add_to_menu(struct menu_item **mi, unsigned char *text, unsigned char *rtext, unsigned char *hotkey, void (*func)(struct terminal *, void *, void *), void *data, int in_m)
{
	struct menu_item *mii;
	int n;
	for (n = 0; (*mi)[n].text; n++) if (n == MAXINT) overalloc();
	if (((unsigned)n + 2) > MAXINT / sizeof(struct menu_item)) overalloc();
	mii = mem_realloc(*mi, (n + 2) * sizeof(struct menu_item));
	*mi = mii;
	memcpy(mii + n + 1, mii + n, sizeof(struct menu_item));
	mii[n].text = text;
	mii[n].rtext = rtext;
	mii[n].hotkey = hotkey;
	mii[n].func = func;
	mii[n].data = data;
	mii[n].in_m = in_m;
}

void do_dialog(struct terminal *term, struct dialog *dlg, struct memory_list *ml)
{
	struct dialog_data *dd;
	struct dialog_item *d;
	int n = 0;
	for (d = dlg->items; d->type != D_END; d++) {
		if (n == MAXINT) overalloc();
		n++;
	}
	if ((unsigned)n > (MAXINT - sizeof(struct dialog_data)) / sizeof(struct dialog_item_data)) overalloc();
	dd = mem_alloc(sizeof(struct dialog_data) + sizeof(struct dialog_item_data) * n);
	dd->dlg = dlg;
	dd->n = n;
	dd->ml = ml;
	add_window(term, dialog_func, dd);
}

void display_dlg_item(struct dialog_data *dlg, struct dialog_item_data *di, int sel)
{
	struct terminal *term = dlg->win->term;
	switch(di->item->type) {
		int co;
		unsigned char *text;
		case D_CHECKBOX:
			if (di->checked) print_text(term, di->x, di->y, 3, "[X]", COLOR_DIALOG_CHECKBOX);
			else print_text(term, di->x, di->y, 3, "[ ]", COLOR_DIALOG_CHECKBOX);
			if (sel) {
				set_cursor(term, di->x + 1, di->y, di->x + 1, di->y);
				set_window_ptr(dlg->win, di->x, di->y);
			}
			break;
		case D_FIELD_PASS:
		case D_FIELD:
			if (di->vpos + di->l <= di->cpos) di->vpos = di->cpos - di->l + 1;
			if (di->vpos > di->cpos) di->vpos = di->cpos;
			if (di->vpos < 0) di->vpos = 0;
			fill_area(term, di->x, di->y, di->l, 1, COLOR_DIALOG_FIELD);
			if (di->item->type == D_FIELD) {
				print_text(term, di->x, di->y, strlen(di->cdata + di->vpos) <= (size_t)di->l ? (int)strlen(di->cdata + di->vpos) : di->l, di->cdata + di->vpos, COLOR_DIALOG_FIELD_TEXT);
			} else {
				fill_area(term, di->x, di->y, strlen(di->cdata + di->vpos) <= (size_t)di->l ? (int)strlen(di->cdata + di->vpos) : di->l, 1, COLOR_DIALOG_FIELD_TEXT | '*');
			}
			if (sel) {
				set_cursor(term, di->x + di->cpos - di->vpos, di->y, di->x + di->cpos - di->vpos, di->y);
				set_window_ptr(dlg->win, di->x, di->y);
			}
			break;
		case D_BUTTON:
			co = sel ? COLOR_DIALOG_BUTTON_SELECTED : COLOR_DIALOG_BUTTON;
			text = _(di->item->text, term);
			print_text(term, di->x, di->y, 2, "[ ", co);
			print_text(term, di->x + 2, di->y, strlen(text), text, co);
			print_text(term, di->x + 2 + strlen(text), di->y, 2, " ]", co);
			if (sel) {
				set_cursor(term, di->x + 2, di->y, di->x + 2, di->y);
				set_window_ptr(dlg->win, di->x, di->y);
			}
			break;
		case D_BOX:
			/* Draw a hierarchy box */
			show_dlg_item_box(dlg, di);
			break;
		default:
			internal("Tried to draw unknown");
	}
}

void dlg_select_item(struct dialog_data *dlg, struct dialog_item_data *di)
{
	if (di->item->type == D_CHECKBOX) {
		if (!di->item->gid) di -> checked = *(int *)di->cdata = !*(int *)di->cdata;
		else {
			int i;
			for (i = 0; i < dlg->n; i++) {
				if (dlg->items[i].item->type == D_CHECKBOX && dlg->items[i].item->gid == di->item->gid) {
					*(int *)dlg->items[i].cdata = di->item->gnum;
					dlg->items[i].checked = 0;
					display_dlg_item(dlg, &dlg->items[i], 0);
				}
			}
			di->checked = 1;
		}
		display_dlg_item(dlg, di, 1);
	}
	else if (di->item->type == D_BUTTON) di->item->fn(dlg, di);
}

void dlg_set_history(struct dialog_item_data *di)
{
	unsigned char *s = "";
	int l;
	if ((void *)di->cur_hist != &di->history) s = di->cur_hist->d;
	if ((l = strlen(s)) > di->item->dlen) l = di->item->dlen - 1;
	memcpy(di->cdata, s, l);
	di->cdata[l] = 0;
	di->cpos = l;
	di->vpos = 0;
}

int dlg_mouse(struct dialog_data *dlg, struct dialog_item_data *di, struct event *ev)
{
	switch (di->item->type) {
		case D_BUTTON:
			if (ev->y != di->y || ev->x < di->x || (size_t)ev->x >= di->x + strlen(_(di->item->text, dlg->win->term)) + 4) return 0;
			display_dlg_item(dlg, &dlg->items[dlg->selected], 0);
			dlg->selected = di - dlg->items;
			display_dlg_item(dlg, di, 1);
			if ((ev->b & BM_ACT) == B_UP) dlg_select_item(dlg, di);
			return 1;
		case D_FIELD_PASS:
		case D_FIELD:
			if (ev->y != di->y || ev->x < di->x || ev->x >= di->x + di->l) return 0;
			if ((size_t)(di->cpos = di->vpos + ev->x - di->x) > strlen(di->cdata)) di->cpos = strlen(di->cdata);
			display_dlg_item(dlg, &dlg->items[dlg->selected], 0);
			dlg->selected = di - dlg->items;
			display_dlg_item(dlg, di, 1);
			return 1;
		case D_CHECKBOX:
			if (ev->y != di->y || ev->x < di->x || ev->x >= di->x + 3) return 0;
			display_dlg_item(dlg, &dlg->items[dlg->selected], 0);
			dlg->selected = di - dlg->items;
			display_dlg_item(dlg, di, 1);
			if ((ev->b & BM_ACT) == B_UP) dlg_select_item(dlg, di);
			return 1;
		case D_BOX:
			if ((ev->b & BM_ACT) == B_UP) {
				if ( (ev->y >= di->y)  && (ev->x >= di->x && ev->x <= di->l + di->x) ) {
					/* Clicked in the box. */
					int offset;
					offset = ev->y - di->y;
					box_sel_set_visible(di, offset);
					display_dlg_item(dlg, di, 1);
					return 1;
				}
			} /*else if ((ev->b & BM_ACT) == B_DRAG) {
				debug("drag");
			}*/
	}
	return 0;
}

void redraw_dialog(struct dialog_data *dlg)
{
	int i;
	struct terminal *term = dlg->win->term;
	draw_frame(term, dlg->x + DIALOG_LEFT_BORDER, dlg->y + DIALOG_TOP_BORDER, dlg->xw - 2 * DIALOG_LEFT_BORDER, dlg->yw - 2 * DIALOG_TOP_BORDER, COLOR_DIALOG_FRAME, DIALOG_FRAME);
	i = strlen(_(dlg->dlg->title, term));
	print_text(term, (dlg->xw - i) / 2 + dlg->x - 1, dlg->y + DIALOG_TOP_BORDER, 1, " ", COLOR_DIALOG_TITLE);
	print_text(term, (dlg->xw - i) / 2 + dlg->x, dlg->y + DIALOG_TOP_BORDER, i, _(dlg->dlg->title, term), COLOR_DIALOG_TITLE);
	print_text(term, (dlg->xw - i) / 2 + dlg->x + i, dlg->y + DIALOG_TOP_BORDER, 1, " ", COLOR_DIALOG_TITLE);
	for (i = 0; i < dlg->n; i++) display_dlg_item(dlg, &dlg->items[i], i == dlg->selected);
	redraw_from_window(dlg->win);
}

void tab_compl(struct terminal *term, unsigned char *item, struct window *win)
{
	struct event ev = {EV_REDRAW, 0, 0, 0};
	struct dialog_item_data *di = &((struct dialog_data*)win->data)->items[((struct dialog_data*)win->data)->selected];
	int l = strlen(item);
	if (l >= di->item->dlen) l = di->item->dlen - 1;
	memcpy(di->cdata, item, l);
	di->cdata[l] = 0;
	di->cpos = l;
	di->vpos = 0;
	ev.x = term->x;
	ev.y = term->y;
	dialog_func(win, &ev, 0);
}

void do_tab_compl(struct terminal *term, struct list_head *history, struct window *win)
{
	unsigned char *cdata = ((struct dialog_data*)win->data)->items[((struct dialog_data*)win->data)->selected].cdata;
	int l = strlen(cdata), n = 0;
	struct history_item *hi;
	struct menu_item *items = DUMMY;
	foreach(hi, *history) if (!strncmp(cdata, hi->d, l)) {
		if (!(n & (ALLOC_GR - 1))) {
			if ((unsigned)n > MAXINT / sizeof(struct menu_item) - ALLOC_GR - 1) overalloc();
			items = mem_realloc(items, (n + ALLOC_GR + 1) * sizeof(struct menu_item));
		}
		items[n].text = hi->d;
		items[n].rtext = "";
		items[n].hotkey = "";
		items[n].func = (void(*)(struct terminal *, void *, void *))tab_compl;
		items[n].rtext = "";
		items[n].data = hi->d;
		items[n].in_m = 0;
		items[n].free_i = 1;
		if (n == MAXINT) overalloc();
		n++;
	}
	if (n == 1) {
		tab_compl(term, items->data, win);
		mem_free(items);
		return;
	}
	if (n) {
		memset(&items[n], 0, sizeof(struct menu_item));
		do_menu_selected(term, items, win, n - 1);
	}
}

void dialog_func(struct window *win, struct event *ev, int fwd)
{
	int i;
	struct terminal *term = win->term;
	struct dialog_data *dlg = win->data;
	struct dialog_item_data *di;

	dlg->win = win;

	/* Use nonstandard event handlers */
	if (dlg->dlg->handle_event && ((dlg->dlg->handle_event)(dlg, ev) == EVENT_PROCESSED) ) {
		return;
	}
	
	switch ((int)ev->ev) {
		case EV_INIT:
			for (i = 0; i < dlg->n; i++) {
				/* volatile because of a compiler bug */
				struct dialog_item_data * volatile di = &dlg->items[i];
				memset(di, 0, sizeof(struct dialog_item_data));
				di->item = &dlg->dlg->items[i];
				di->cdata = mem_alloc(di->item->dlen);
				memcpy(di->cdata, di->item->data, di->item->dlen);
				if (di->item->type == D_CHECKBOX) {
					if (di->item->gid) {
						if (*(int *)di->cdata == di->item->gnum) di->checked = 1;
					} else if (*(int *)di->cdata) di->checked = 1;
				} 
				if (di->item->type == D_BOX) {
					/* Freed in bookmark_dialog_abort_handler() */
					di->cdata = mem_alloc( sizeof(struct dlg_data_item_data_box) );
					((struct dlg_data_item_data_box*)di->cdata)->sel = -1;
					((struct dlg_data_item_data_box*)di->cdata)->box_top = 0;
					((struct dlg_data_item_data_box*)di->cdata)->list_len = -1;

					init_list(((struct dlg_data_item_data_box*)di->cdata)->items);
				}
				init_list(di->history);
				di->cur_hist = (struct history_item *)&di->history;
				if (di->item->type == D_FIELD || di->item->type == D_FIELD_PASS) {
					if (di->item->history) {
						struct history_item *j;
						/*int l = di->item->dlen;*/
						foreach(j, di->item->history->items) {
							struct history_item *hi;
							hi = mem_alloc(sizeof(struct history_item) + strlen(j->d) + 1);
							strcpy(hi->d, j->d);
							add_to_list(di->history, hi);
						}
					}
					di->cpos = strlen(di->cdata);
				}
			}
			dlg->selected = 0;
		case EV_RESIZE:
		case EV_REDRAW:
			dlg->dlg->fn(dlg);
			redraw_dialog(dlg);
			break;
		case EV_MOUSE:
			for (i = 0; i < dlg->n; i++) if (dlg_mouse(dlg, &dlg->items[i], ev)) break;
			break;
		case EV_KBD:
			di = &dlg->items[dlg->selected];
			if (di->item->type == D_FIELD || di->item->type == D_FIELD_PASS) {
				switch (kbd_action(KM_EDIT, ev)) {
					case ACT_UP:
						if ((void *)di->cur_hist->prev != &di->history) {
							di->cur_hist = di->cur_hist->prev;
							dlg_set_history(di);
							goto dsp_f;
						}
						break;
					case ACT_DOWN:
						if ((void *)di->cur_hist != &di->history) {
							di->cur_hist = di->cur_hist->next;
							dlg_set_history(di);
							goto dsp_f;
						}
						break;
					case ACT_RIGHT:
						if ((size_t)di->cpos < strlen(di->cdata)) di->cpos++;
						goto dsp_f;
					case ACT_LEFT:
						if (di->cpos > 0) di->cpos--;
						goto dsp_f;
					case ACT_HOME:
						di->cpos = 0;
						goto dsp_f;
					case ACT_END:
						di->cpos = strlen(di->cdata);
						goto dsp_f;
					case ACT_BACKSPACE:
						if (di->cpos) {
							memmove(di->cdata + di->cpos - 1, di->cdata + di->cpos, strlen(di->cdata) - di->cpos + 1);
							di->cpos--;
						}
						goto dsp_f;
					case ACT_DELETE:
						if ((size_t)di->cpos < strlen(di->cdata))
							memmove(di->cdata + di->cpos, di->cdata + di->cpos + 1, strlen(di->cdata) - di->cpos + 1);
						goto dsp_f;
					case ACT_KILL_TO_BOL:
						memmove(di->cdata, di->cdata + di->cpos, strlen(di->cdata + di->cpos) + 1);
						di->cpos = 0;
						goto dsp_f;
					case ACT_KILL_TO_EOL:
						di->cdata[di->cpos] = 0;
						goto dsp_f;
				    	case ACT_COPY_CLIPBOARD:
 						/* Copy to clipboard */
						set_clipboard_text(di->cdata);
						break;	/* We don't need to redraw */
					case ACT_CUT_CLIPBOARD:
 						/* Cut to clipboard */						
						set_clipboard_text(di->cdata);
						di->cdata[0] = 0;
						di->cpos = 0;
						goto dsp_f;
					case ACT_PASTE_CLIPBOARD: {
						/* Paste from clipboard */
						unsigned char * clipboard = get_clipboard_text();
						if (clipboard) {
							safe_strncpy(di->cdata, clipboard, di->item->dlen);
							di->cpos = strlen(di->cdata);
							mem_free(clipboard);
						}
						goto dsp_f;
					}
					case ACT_AUTO_COMPLETE: 
						do_tab_compl(term, &di->history, win);
						goto dsp_f;
					default:
						if (ev->x >= ' ' && ev->x < 0x100 && !ev->y) {
							if (strlen(di->cdata) + 1 < (size_t)di->item->dlen) {
								memmove(di->cdata + di->cpos + 1, di->cdata + di->cpos, strlen(di->cdata) - di->cpos + 1);
								di->cdata[di->cpos++] = ev->x;
							}
							goto dsp_f;
						}
				}
				goto gh;
				dsp_f:
				display_dlg_item(dlg, di, 1);
				redraw_from_window(dlg->win);
				break;
			}
			if ((ev->x == KBD_ENTER && di->item->type == D_BUTTON) || ev->x == ' ') {
				dlg_select_item(dlg, di);
				break;
			}
			gh:
			if (ev->x > ' ' && ev->x < 0x100) for (i = 0; i < dlg->n; i++)
				if (dlg->dlg->items[i].type == D_BUTTON && upcase(_(dlg->dlg->items[i].text, term)[0]) == upcase(ev->x)) {
					sel:
					if (dlg->selected != i) {
						display_dlg_item(dlg, &dlg->items[dlg->selected], 0);
						display_dlg_item(dlg, &dlg->items[i], 1);
						dlg->selected = i;
					}
					dlg_select_item(dlg, &dlg->items[i]);
					goto bla;
			}
			if (ev->x == KBD_ENTER) for (i = 0; i < dlg->n; i++)
				if (dlg->dlg->items[i].type == D_BUTTON && dlg->dlg->items[i].gid & B_ENTER) goto sel;
			if (ev->x == KBD_ESC) for (i = 0; i < dlg->n; i++)
				if (dlg->dlg->items[i].type == D_BUTTON && dlg->dlg->items[i].gid & B_ESC) goto sel;
			if ((ev->x == KBD_TAB && !ev->y) || ev->x == KBD_DOWN || ev->x == KBD_RIGHT) {
				display_dlg_item(dlg, &dlg->items[dlg->selected], 0);
				if ((++dlg->selected) >= dlg->n) dlg->selected = 0;
				display_dlg_item(dlg, &dlg->items[dlg->selected], 1);
				redraw_from_window(dlg->win);
				break;
			}
			if ((ev->x == KBD_TAB && ev->y) || ev->x == KBD_UP || ev->x == KBD_LEFT) {
				display_dlg_item(dlg, &dlg->items[dlg->selected], 0);
				if ((--dlg->selected) < 0) dlg->selected = dlg->n - 1;
				display_dlg_item(dlg, &dlg->items[dlg->selected], 1);
				redraw_from_window(dlg->win);
				break;
			}
			break;
		case EV_ABORT:
			/* Moved this line up so that the dlg would have access to its 
				member vars before they get freed. */ 
			if (dlg->dlg->abort) dlg->dlg->abort(dlg);
			for (i = 0; i < dlg->n; i++) {
				struct dialog_item_data *di = &dlg->items[i];
				if (di->cdata) mem_free(di->cdata);
				free_list(di->history);
			}
			freeml(dlg->ml);
	}
	bla:;
}

int check_number(struct dialog_data *dlg, struct dialog_item_data *di)
{
	unsigned char *end;
	long l = strtol(di->cdata, (char **)(void *)&end, 10);
	if (!*di->cdata || *end) {
		msg_box(dlg->win->term, NULL, TEXT_(T_BAD_NUMBER), AL_CENTER, TEXT_(T_NUMBER_EXPECTED), NULL, 1, TEXT_(T_CANCEL), NULL, B_ENTER | B_ESC);
		return 1;
	}
	if (l < di->item->gid || l > di->item->gnum) {
		msg_box(dlg->win->term, NULL, TEXT_(T_BAD_NUMBER), AL_CENTER, TEXT_(T_NUMBER_OUT_OF_RANGE), NULL, 1, TEXT_(T_CANCEL), NULL, B_ENTER | B_ESC);
		return 1;
	}
	return 0;
}

int check_nonempty(struct dialog_data *dlg, struct dialog_item_data *di)
{
	unsigned char *p;
	for (p = di->cdata; *p; p++) if (*p > ' ') return 0;
	msg_box(dlg->win->term, NULL, TEXT_(T_BAD_STRING), AL_CENTER, TEXT_(T_EMPTY_STRING_NOT_ALLOWED), NULL, 1, TEXT_(T_CANCEL), NULL, B_ENTER | B_ESC);
	return 1;
}

int cancel_dialog(struct dialog_data *dlg, struct dialog_item_data *di)
{
	/*struct dialog *dl = dlg->dlg;*/
	delete_window(dlg->win);
	/*mem_free(dl);*/
	return 0;
}

int check_dialog(struct dialog_data *dlg)
{
	int i;
	for (i = 0; i < dlg->n; i++)
		if (dlg->dlg->items[i].type == D_CHECKBOX || dlg->dlg->items[i].type == D_FIELD || dlg->dlg->items[i].type == D_FIELD_PASS)
			if (dlg->dlg->items[i].fn && dlg->dlg->items[i].fn(dlg, &dlg->items[i])) {
				dlg->selected = i;
				redraw_dialog(dlg);
				return 1;
			}
	return 0;
}

void get_dialog_data(struct dialog_data *dlg)
{
	int i;
	for (i = 0; i < dlg->n; i++) {
		/* volatile because of a compiler bug */
		void * volatile p1 = dlg->dlg->items[i].data;
		void * volatile p2 = dlg->items[i].cdata;
		volatile int l = dlg->dlg->items[i].dlen;
		memcpy(p1, p2, l);
	}
}

int ok_dialog(struct dialog_data *dlg, struct dialog_item_data *di)
{
	void (*fn)(void *) = dlg->dlg->refresh;
	void *data = dlg->dlg->refresh_data;
	if (check_dialog(dlg)) return 1;
	get_dialog_data(dlg);
	if (fn) fn(data);
	return cancel_dialog(dlg, di);
}

void center_dlg(struct dialog_data *dlg)
{
	dlg->x = (dlg->win->term->x - dlg->xw) / 2;
	dlg->y = (dlg->win->term->y - dlg->yw) / 2;
}

void draw_dlg(struct dialog_data *dlg)
{
	fill_area(dlg->win->term, dlg->x, dlg->y, dlg->xw, dlg->yw, COLOR_DIALOG);
}

void max_text_width(struct terminal *term, unsigned char *text, int *width)
{
	text = _(text, term);
	do {
		int c = 0;
		while (*text && *text != '\n') text++, c++;
		if (c > *width) *width = c;
	} while (*(text++));
}

void min_text_width(struct terminal *term, unsigned char *text, int *width)
{
	text = _(text, term);
	do {
		int c = 0;
		while (*text && *text != '\n' && *text != ' ') text++, c++;
		if (c > *width) *width = c;
	} while (*(text++));
}

void dlg_format_text(struct terminal *term, struct terminal *t2, unsigned char *text, int x, int *y, int w, int *rw, int co, int align)
{
	text = _(text, t2);
	do {
		unsigned char *tx;
		unsigned char *tt = text;
		int s;
		int xx = x;
		do {
			while (*text && *text != '\n' && *text != ' ') {
				/*if (term) set_char(term, xx, *y, co | *text);*/
				text++, xx++;
			}
			tx = ++text;
			xx++;
			if (*(text - 1) != ' ') break;
			while (*tx && *tx != '\n' && *tx != ' ') tx++;
		} while (tx - text + xx - x <= w);
		s = (align & AL_MASK) == AL_CENTER ? (w - (xx - 1 - x)) / 2 : 0;
		if (s < 0) s = 0;
		while (tt < text - 1) {
			if (s >= w) {
				s = 0, (*y)++;
				if (rw) *rw = w;
				rw = NULL;
			}
			if (term) set_char(term, x + s, *y, co | *tt);
			s++, tt++;
		}
		if (rw && xx - 1 - x > *rw) *rw = xx - 1 - x;
		(*y)++;
	} while (*(text - 1));
}

void max_buttons_width(struct terminal *term, struct dialog_item_data *butt, int n, int *width)
{
	int w = -2;
	int i;
	for (i = 0; i < n; i++) w += strlen(_((butt++)->item->text, term)) + 6;
	if (w > *width) *width = w;
}

void min_buttons_width(struct terminal *term, struct dialog_item_data *butt, int n, int *width)
{
	int i;
	for (i = 0; i < n; i++) {
		int w = strlen(_((butt++)->item->text, term)) + 4;
		if (w > *width) *width = w;
	}
}

void dlg_format_buttons(struct terminal *term, struct terminal *t2, struct dialog_item_data *butt, int n, int x, int *y, int w, int *rw, int align)
{
	int i1 = 0;
	while (i1 < n) {
		int i2 = i1 + 1;
		int mw;
		while (i2 < n) {
			mw = 0;
			max_buttons_width(t2, butt + i1, i2 - i1 + 1, &mw);
			if (mw <= w) i2++;
			else break;
		}
		mw = 0;
		max_buttons_width(t2, butt + i1, i2 - i1, &mw);
		if (rw && mw > *rw) if ((*rw = mw) > w) *rw = w;
		if (term) {
			int i;
			int p = x + ((align & AL_MASK) == AL_CENTER ? (w - mw) / 2 : 0);
			for (i = i1; i < i2; i++) {
				butt[i].x = p;
				butt[i].y = *y;
				p += (butt[i].l = strlen(_(butt[i].item->text, t2)) + 4) + 2;
			}
		}
		*y += 2;
		i1 = i2;
	}
}

void dlg_format_checkbox(struct terminal *term, struct terminal *t2, struct dialog_item_data *chkb, int x, int *y, int w, int *rw, unsigned char *text)
{
	if (term) {
		chkb->x = x;
		chkb->y = *y;
	}
	if (rw) *rw -= 4;
	dlg_format_text(term, t2, text, x + 4, y, w - 4, rw, COLOR_DIALOG_CHECKBOX_TEXT, AL_LEFT);
	if (rw) *rw += 4;
}

void dlg_format_checkboxes(struct terminal *term, struct terminal *t2, struct dialog_item_data *chkb, int n, int x, int *y, int w, int *rw, unsigned char **texts)
{
	while (n) {
		dlg_format_checkbox(term, t2, chkb, x, y, w, rw, _(texts[0], t2));
		texts++; chkb++; n--;
	}
}

void checkboxes_width(struct terminal *term, unsigned char **texts, int *w, void (*fn)(struct terminal *, unsigned char *, int *))
{
	while (texts[0]) {
		*w -= 4;
		fn(term, _(texts[0], term), w);
		*w += 4;
		texts++;
	}
}

void dlg_format_field(struct terminal *term, struct terminal *t2, struct dialog_item_data *item, int x, int *y, int w, int *rw, int align)
{
	item->x = x;
	item->y = *y;
	item->l = w;
	/*if ((item->l = w) > item->item->dlen - 1) item->l = item->item->dlen - 1;*/
	if (rw && item->l > *rw) if ((*rw = item->l) > w) *rw = w;
	(*y)++;
}

/* Layout for generic boxes */
void dlg_format_box(struct terminal *term, struct terminal *t2, struct dialog_item_data *item, int x, int *y, int w, int *rw, int align) {
	item->x = x;
	item->y = *y;
	item->l = w;
	if (rw && item->l > *rw) if ((*rw = item->l) > w) *rw = w;
	(*y) += item->item->gid;
}

void max_group_width(struct terminal *term, unsigned char **texts, struct dialog_item_data *item, int n, int *w)
{
	int ww = 0;
	while (n--) {
		int wx = item->item->type == D_CHECKBOX ? 4 : item->item->type == D_BUTTON ? strlen(_(item->item->text, term)) + 5 : (size_t)item->item->dlen + 1;
		wx += strlen(_(texts[0], term));
		if (n) wx++;
		ww += wx;
		texts++;
		item++;
	}
	if (ww > *w) *w = ww;
}

void min_group_width(struct terminal *term, unsigned char **texts, struct dialog_item_data *item, int n, int *w)
{
	while (n--) {
		int wx = item->item->type == D_CHECKBOX ? 4 : item->item->type == D_BUTTON ? strlen(_(item->item->text, term)) + 5 : (size_t)item->item->dlen + 1;
		wx += strlen(_(texts[0], term));
		if (wx > *w) *w = wx;
		texts++;
		item++;
	}
}

void dlg_format_group(struct terminal *term, struct terminal *t2, unsigned char **texts, struct dialog_item_data *item, int n, int x, int *y, int w, int *rw)
{
	int nx = 0;
	while (n--) {
		int sl;
		int wx = item->item->type == D_CHECKBOX ? 4 : item->item->type == D_BUTTON ? strlen(_(item->item->text, t2)) + 5 : (size_t)item->item->dlen + 1;
		if (_(texts[0], t2)[0]) sl = strlen(_(texts[0], t2));
		else sl = -1;
		wx += sl;
		if (nx && nx + wx > w) {
			nx = 0;
			(*y) += 2;
		}
		if (term) {
			print_text(term, x + nx + 4 * (item->item->type == D_CHECKBOX), *y, strlen(_(texts[0], t2)), _(texts[0], t2), COLOR_DIALOG_TEXT);
			item->x = x + nx + (sl + 1) * (item->item->type != D_CHECKBOX);
			item->y = *y;
			if (item->item->type == D_FIELD || item->item->type == D_FIELD_PASS) item->l = item->item->dlen;
		}
		if (rw && nx + wx > *rw) if ((*rw = nx + wx) > w) *rw = w;
		nx += wx + 1;
		texts++;
		item++;
	}
	(*y)++;
}

void checkbox_list_fn(struct dialog_data *dlg)
{
	struct terminal *term = dlg->win->term;
	int max = 0, min = 0;
	int w, rw;
	int y = 0;
	checkboxes_width(term, dlg->dlg->udata, &max, max_text_width);
	checkboxes_width(term, dlg->dlg->udata, &min, min_text_width);
	max_buttons_width(term, dlg->items + dlg->n - 2, 2, &max);
	min_buttons_width(term, dlg->items + dlg->n - 2, 2, &min);
	w = term->x * 9 / 10 - 2 * DIALOG_LB;
	if (w > max) w = max;
	if (w < min) w = min;
	if (w > term->x - 2 * DIALOG_LB) w = term->x - 2 * DIALOG_LB;
	if (w < 5) w = 5;
	rw = 0;
	dlg_format_checkboxes(NULL, term, dlg->items, dlg->n - 2, 0, &y, w, &rw, dlg->dlg->udata);
	y++;
	dlg_format_buttons(NULL, term, dlg->items + dlg->n - 2, 2, 0, &y, w, &rw, AL_CENTER);
	w = rw;
	dlg->xw = rw + 2 * DIALOG_LB;
	dlg->yw = y + 2 * DIALOG_TB;
	center_dlg(dlg);
	draw_dlg(dlg);
	y = dlg->y + DIALOG_TB + 1;
	dlg_format_checkboxes(term, term, dlg->items, dlg->n - 2, dlg->x + DIALOG_LB, &y, w, NULL, dlg->dlg->udata);
	y++;
	dlg_format_buttons(term, term, dlg->items + dlg->n - 2, 2, dlg->x + DIALOG_LB, &y, w, &rw, AL_CENTER);
}

void group_fn(struct dialog_data *dlg)
{
	struct terminal *term = dlg->win->term;
	int max = 0, min = 0;
	int w, rw;
	int y = 0;
	max_group_width(term, dlg->dlg->udata, dlg->items, dlg->n - 2, &max);
	min_group_width(term, dlg->dlg->udata, dlg->items, dlg->n - 2, &min);
	max_buttons_width(term, dlg->items + dlg->n - 2, 2, &max);
	min_buttons_width(term, dlg->items + dlg->n - 2, 2, &min);
	w = term->x * 9 / 10 - 2 * DIALOG_LB;
	if (w > max) w = max;
	if (w < min) w = min;
	if (w > term->x - 2 * DIALOG_LB) w = term->x - 2 * DIALOG_LB;
	if (w < 1) w = 1;
	rw = 0;
	dlg_format_group(NULL, term, dlg->dlg->udata, dlg->items, dlg->n - 2, 0, &y, w, &rw);
	y++;
	dlg_format_buttons(NULL, term, dlg->items + dlg->n - 2, 2, 0, &y, w, &rw, AL_CENTER);
	w = rw;
	dlg->xw = rw + 2 * DIALOG_LB;
	dlg->yw = y + 2 * DIALOG_TB;
	center_dlg(dlg);
	draw_dlg(dlg);
	y = dlg->y + DIALOG_TB + 1;
	dlg_format_group(term, term, dlg->dlg->udata, dlg->items, dlg->n - 2, dlg->x + DIALOG_LB, &y, w, NULL);
	y++;
	dlg_format_buttons(term, term, dlg->items + dlg->n - 2, 2, dlg->x + DIALOG_LB, &y, w, &rw, AL_CENTER);
}

void msg_box_fn(struct dialog_data *dlg)
{
	struct terminal *term = dlg->win->term;
	int max = 0, min = 0;
	int w, rw;
	int y = 0;
	unsigned char **ptr;
	unsigned char *text = init_str();
	int textl = 0;
	for (ptr = dlg->dlg->udata; *ptr; ptr++) add_to_str(&text, &textl, _(*ptr, term));
	max_text_width(term, text, &max);
	min_text_width(term, text, &min);
	max_buttons_width(term, dlg->items, dlg->n, &max);
	min_buttons_width(term, dlg->items, dlg->n, &min);
	w = term->x * 9 / 10 - 2 * DIALOG_LB;
	if (w > max) w = max;
	if (w < min) w = min;
	if (w > term->x - 2 * DIALOG_LB) w = term->x - 2 * DIALOG_LB;
	if (w < 1) w = 1;
	rw = 0;
	dlg_format_text(NULL, term, text, 0, &y, w, &rw, COLOR_DIALOG_TEXT, dlg->dlg->align);
	y++;
	dlg_format_buttons(NULL, term, dlg->items, dlg->n, 0, &y, w, &rw, AL_CENTER);
	w = rw;
	dlg->xw = rw + 2 * DIALOG_LB;
	dlg->yw = y + 2 * DIALOG_TB;
	center_dlg(dlg);
	draw_dlg(dlg);
	y = dlg->y + DIALOG_TB + 1;
	dlg_format_text(term, term, text, dlg->x + DIALOG_LB, &y, w, NULL, COLOR_DIALOG_TEXT, dlg->dlg->align);
	y++;
	dlg_format_buttons(term, term, dlg->items, dlg->n, dlg->x + DIALOG_LB, &y, w, NULL, AL_CENTER);
	mem_free(text);
}

int msg_box_button(struct dialog_data *dlg, struct dialog_item_data *di)
{
	void (*fn)(void *) = (void (*)(void *))di->item->udata;
	void *data = dlg->dlg->udata2;
	/*struct dialog *dl = dlg->dlg;*/
	if (fn) fn(data);
	cancel_dialog(dlg, di);
	return 0;
}

void msg_box(struct terminal *term, struct memory_list *ml, unsigned char *title, int align, /*unsigned char *text, void *data, int n,*/ ...)
{
	struct dialog *dlg;
	int i;
	int n;
	unsigned char *text;
	unsigned char **udata;
	void *udata2;
	int udatan;
	va_list ap;
	va_start(ap, align);
	udata = DUMMY;
	udatan = 0;
	do {
		text = va_arg(ap, unsigned char *);
		na_kovarne__to_je_narez:
		udatan++;
		if ((unsigned)udatan > MAXINT / sizeof(unsigned char *)) overalloc();
		udata = mem_realloc(udata, udatan * sizeof(unsigned char *));
		udata[udatan - 1] = text;
		if (text && !(align & AL_EXTD_TEXT)) {
			text = NULL;
			goto na_kovarne__to_je_narez;
		}
	} while (text);
	udata2 = va_arg(ap, void *);
	n = va_arg(ap, int);
	if ((unsigned)n > (MAXINT - sizeof(struct dialog)) / sizeof(struct dialog_item) - 1) overalloc();
	dlg = mem_alloc(sizeof(struct dialog) + (n + 1) * sizeof(struct dialog_item));
	memset(dlg, 0, sizeof(struct dialog) + (n + 1) * sizeof(struct dialog_item));
	dlg->title = title;
	dlg->fn = msg_box_fn;
	dlg->udata = udata;
	dlg->udata2 = udata2;
	dlg->align = align;
	for (i = 0; i < n; i++) {
		unsigned char *m;
		void (*fn)(void *);
		int flags;
		m = va_arg(ap, unsigned char *);
		fn = va_arg(ap, void *);
		flags = va_arg(ap, int);
		if (!m) {
			i--, n--;
			continue;
		}
		dlg->items[i].type = D_BUTTON;
		dlg->items[i].gid = flags;
		dlg->items[i].fn = msg_box_button;
		dlg->items[i].dlen = 0;
		dlg->items[i].text = m;
		dlg->items[i].udata = fn;
	}
	va_end(ap);
	dlg->items[i].type = D_END;
	add_to_ml(&ml, dlg, udata, NULL);
	do_dialog(term, dlg, ml);
}

void add_to_history(struct history *h, unsigned char *t, int check_duplicates)
{
	struct history_item *hi, *hs;
	size_t l;
	if (!h || !t || !*t) return;
	l = strlen(t);
	if (l > MAXINT - sizeof(struct history_item)) overalloc();
	hi = mem_alloc(sizeof(struct history_item) + l);
	memcpy(hi->d, t, l + 1);
	if (check_duplicates) foreach(hs, h->items) if (!strcmp(hs->d, t)) {
		struct history_item *hd = hs;
		hs = hs->prev;
		del_from_list(hd);
		mem_free(hd);
		h->n--;
	}
	add_to_list(h->items, hi);
	h->n++;
	while (h->n > MAX_HISTORY_ITEMS) {
		struct history_item *hd = h->items.prev;
		if ((void *)hd == &h->items) {
			internal("history is empty");
			h->n = 0;
			return;
		}
		del_from_list(hd);
		mem_free(hd);
		h->n--;
	}
}

int input_field_cancel(struct dialog_data *dlg, struct dialog_item_data *di)
{
	void (*fn)(void *) = di->item->udata;
	void *data = dlg->dlg->udata2;
	if (fn) fn(data);
	cancel_dialog(dlg, di);
	return 0;
}

int input_field_ok(struct dialog_data *dlg, struct dialog_item_data *di)
{
	void (*fn)(void *, unsigned char *) = di->item->udata;
	void *data = dlg->dlg->udata2;
	unsigned char *text = dlg->items->cdata;
	if (check_dialog(dlg)) return 1;
	add_to_history(dlg->dlg->items->history, text, 1);
	if (fn) fn(data, text);
	ok_dialog(dlg, di);
	return 0;
}

void input_field_fn(struct dialog_data *dlg)
{
	struct terminal *term = dlg->win->term;
	int max = 0, min = 0;
	int w, rw;
	int y = -1;
	max_text_width(term, dlg->dlg->udata, &max);
	min_text_width(term, dlg->dlg->udata, &min);
	max_buttons_width(term, dlg->items + 1, 2, &max);
	min_buttons_width(term, dlg->items + 1, 2, &min);
	if (max < dlg->dlg->items->dlen) max = dlg->dlg->items->dlen;
	w = term->x * 9 / 10 - 2 * DIALOG_LB;
	if (w > max) w = max;
	if (w < min) w = min;
	rw = 0; /* !!! FIXME: input field */
	dlg_format_text(NULL, term, dlg->dlg->udata, 0, &y, w, &rw, COLOR_DIALOG_TEXT, AL_LEFT);
	dlg_format_field(NULL, term, dlg->items, 0, &y, w, &rw, AL_LEFT);
	y++;
	dlg_format_buttons(NULL, term, dlg->items + 1, 2, 0, &y, w, &rw, AL_CENTER);
	w = rw;
	dlg->xw = rw + 2 * DIALOG_LB;
	dlg->yw = y + 2 * DIALOG_TB;
	center_dlg(dlg);
	draw_dlg(dlg);
	y = dlg->y + DIALOG_TB;
	dlg_format_text(term, term, dlg->dlg->udata, dlg->x + DIALOG_LB, &y, w, NULL, COLOR_DIALOG_TEXT, AL_LEFT);
	dlg_format_field(term, term, dlg->items, dlg->x + DIALOG_LB, &y, w, NULL, AL_LEFT);
	y++;
	dlg_format_buttons(term, term, dlg->items + 1, 2, dlg->x + DIALOG_LB, &y, w, NULL, AL_CENTER);
}

void input_field(struct terminal *term, struct memory_list *ml, unsigned char *title, unsigned char *text, unsigned char *okbutton, unsigned char *cancelbutton, void *data, struct history *history, int l, unsigned char *def, int min, int max, int (*check)(struct dialog_data *, struct dialog_item_data *), void (*fn)(void *, unsigned char *), void (*cancelfn)(void *))
{
	struct dialog *dlg;
	unsigned char *field;
	if ((unsigned)l > MAXINT - sizeof(struct dialog) + 4 * sizeof(struct dialog_item)) overalloc();
	dlg = mem_alloc(sizeof(struct dialog) + 4 * sizeof(struct dialog_item) + l);
	memset(dlg, 0, sizeof(struct dialog) + 4 * sizeof(struct dialog_item) + l);
	*(field = (unsigned char *)dlg + sizeof(struct dialog) + 4 * sizeof(struct dialog_item)) = 0;
	if (def) {
		if (strlen(def) + 1 > (size_t)l) memcpy(field, def, l - 1);
		else strcpy(field, def);
	}
	dlg->title = title;
	dlg->fn = input_field_fn;
	dlg->udata = text;
	dlg->udata2 = data;
	dlg->items[0].type = D_FIELD;
	dlg->items[0].gid = min;
	dlg->items[0].gnum = max;
	dlg->items[0].fn = check;
	dlg->items[0].history = history;
	dlg->items[0].dlen = l;
	dlg->items[0].data = field;
	dlg->items[1].type = D_BUTTON;
	dlg->items[1].gid = B_ENTER;
	dlg->items[1].fn = input_field_ok;
	dlg->items[1].dlen = 0;
	dlg->items[1].text = okbutton;
	dlg->items[1].udata = fn;
	dlg->items[2].type = D_BUTTON;
	dlg->items[2].gid = B_ESC;
	dlg->items[2].fn = input_field_cancel;
	dlg->items[2].dlen = 0;
	dlg->items[2].text = cancelbutton;
	dlg->items[2].udata = cancelfn;
	dlg->items[3].type = D_END;
	add_to_ml(&ml, dlg, NULL);
	do_dialog(term, dlg, ml);
}


/* Sets the selected item to one that is visible.*/
void box_sel_set_visible(struct dialog_item_data *box_item_data, int offset) {
    struct dlg_data_item_data_box *box;
	int sel;
	
	box = (struct dlg_data_item_data_box *)(box_item_data->item->data);
	if (offset > box_item_data->item->gid || offset < 0) {
		return;
	}
	/*debug("offset: %d", offset);*/
	sel = box->box_top + offset;
	if (sel > box->list_len) {
		box->sel = box->list_len - 1;
	} else {
		box->sel = sel;
	}
}

/* Moves the selected item [dist] thingies. If [dist] is out of the current 
	range, the selected item is moved to the extreme (ie, the top or bottom) 
*/
void box_sel_move(struct dialog_item_data *box_item_data, int dist) {
    struct dlg_data_item_data_box *box;
	int new_sel;
	int new_top;
	
	box = (struct dlg_data_item_data_box *)(box_item_data->item->data);

	new_sel = box->sel + dist; 
	new_top = box->box_top;

	/* Ensure that the selection is in range */
	if (new_sel < 0)
		new_sel = 0;
	else if (new_sel >= box->list_len)
		new_sel = box->list_len - 1;
	
	/* Ensure that the display box is over the item */
	if ( new_sel >= (new_top + box_item_data->item->gid) ) {
		/* Move it down */
		new_top = new_sel - box_item_data->item->gid + 1;
#ifdef DEBUG
		if (new_top < 0)
			debug("Newly calculated box_top is an extremely wrong value (%d). It should not be below zero.", new_top);
#endif	
	} else if ( new_sel < new_top ) {
		/* Move the display up (if necessary) */	
		new_top = new_sel;
	}
	
	box->sel = new_sel;
	box->box_top = new_top;
}


/* Displays a dialog box */
void show_dlg_item_box(struct dialog_data *dlg, struct dialog_item_data *box_item_data) {
	struct terminal *term = dlg->win->term;
	struct dlg_data_item_data_box *box;
	struct box_item *citem;	/* Item currently being shown */
	int n;	/* Index of item currently being displayed */

	box = (struct dlg_data_item_data_box *)(box_item_data->item->data);
	/* FIXME: Counting here SHOULD be unnecessary */
	n = 0;

	fill_area(term, box_item_data->x, box_item_data->y, box_item_data->l, box_item_data->item->gid, COLOR_DIALOG_FIELD);

	foreach (citem, box->items) {
		int len; /* Length of the current text field. */
		len = strlen(citem->text);
		if (len > box_item_data->l) {
			len = box_item_data->l;
		}

		/* Is the current item in the region to be displayed? */
		if ( (n >= box->box_top) && (n < (box->box_top + box_item_data->item->gid)) ) {
			print_text(term, box_item_data->x, box_item_data->y + n - box->box_top, len, citem->text, n == box->sel ? COLOR_DIALOG_BUTTON_SELECTED : COLOR_DIALOG_FIELD_TEXT);

		}
		n++;
	}

	box->list_len = n;
}
