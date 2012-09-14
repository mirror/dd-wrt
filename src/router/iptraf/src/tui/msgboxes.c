/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

/*
 * msgboxes.c - message and error box display functions
 */

#include "iptraf-ng-compat.h"

#include "winops.h"

int ERR_BORDER_ATTR;
int ERR_TEXT_ATTR;
int ERR_PROMPT_ATTR;

int INFO_BORDER_ATTR;
int INFO_TEXT_ATTR;
int INFO_PROMPT_ATTR;

void tx_init_error_attrs(int border, int text, int prompt)
{
	ERR_BORDER_ATTR = border;
	ERR_TEXT_ATTR = text;
	ERR_PROMPT_ATTR = prompt;
}

void tx_init_info_attrs(int border, int text, int prompt)
{
	INFO_BORDER_ATTR = border;
	INFO_TEXT_ATTR = text;
	INFO_PROMPT_ATTR = prompt;
}

void tui_error_va(const char *prompt, const char *err, va_list vararg)
{
	WINDOW *win = newwin(4, 70, (LINES - 4) / 2, (COLS - 70) / 2);
	PANEL *panel = new_panel(win);

	wattrset(win, ERR_BORDER_ATTR);
	tx_colorwin(win);
	tx_box(win, ACS_VLINE, ACS_HLINE);
	wmove(win, 2, 2);
	wattrset(win, ERR_PROMPT_ATTR);
	wprintw(win, "%s", prompt);

	wattrset(win, ERR_TEXT_ATTR);
	wmove(win, 1, 2);

	vw_printw(win, err, vararg);

	update_panels();
	doupdate();

	int response;

	do {
		response = wgetch(win);
		if (response == 12)
			tx_refresh_screen();
	} while (response == 12);

	del_panel(panel);
	delwin(win);
	update_panels();
	doupdate();
}

void tui_error(const char *prompt, const char *err, ...)
{
	va_list params;

	va_start(params, err);
	tui_error_va(prompt, err, params);
	va_end(params);
}

void tx_infobox(char *text, char *prompt)
{
	WINDOW *win;
	PANEL *panel;
	int ch;

	win = newwin(4, 50, (LINES - 4) / 2, (COLS - 50) / 2);
	panel = new_panel(win);
	wattrset(win, INFO_BORDER_ATTR);
	tx_colorwin(win);
	tx_box(win, ACS_VLINE, ACS_HLINE);
	wattrset(win, INFO_TEXT_ATTR);
	mvwprintw(win, 1, 2, text);
	wattrset(win, INFO_PROMPT_ATTR);
	mvwprintw(win, 2, 2, prompt);
	update_panels();
	doupdate();

	do {
		ch = wgetch(win);
		if (ch == 12)
			tx_refresh_screen();
	} while (ch == 12);

	del_panel(panel);
	delwin(win);

	update_panels();
	doupdate();
}
