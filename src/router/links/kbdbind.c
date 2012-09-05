#include "links.h"

static void add_default_keybindings();

struct keybinding {
	struct keybinding *next;
	struct keybinding *prev;
	int act;
	long x;
	long y;
};

static struct list_head keymaps[KM_MAX];

static void add_keybinding(int km, int act, long x, long y)
{
	struct keybinding *kb;

	foreach(kb, keymaps[km])
		if (kb->x == x && kb->y == y) {
			/* want at top of list */
			del_from_list(kb);
			goto add;
		}
	
	kb = mem_alloc(sizeof(struct keybinding));
	add:
	kb->act = act;
	kb->x = x;
	kb->y = y;
	add_to_list(keymaps[km], kb);
}

static void delete_keybinding(int km, long x, long y)
{
	struct keybinding *kb;
	foreach(kb, keymaps[km]) 
		if (kb->x == x && kb->y == y) {
			del_from_list(kb);
			mem_free(kb);
			break;
		}
}

void init_keymaps()
{
    	int i;	
	for (i = 0; i < KM_MAX; i++) init_list(keymaps[i]);
	add_default_keybindings();
}

void free_keymaps() 
{
	int i;
	for (i = 0; i < KM_MAX; i++) free_list(keymaps[i]);
}

int kbd_action(int kmap, struct event *ev)
{
	struct keybinding *kb;
	if (ev->ev == EV_KBD)
		foreach(kb, keymaps[kmap]) 
			if (ev->x == kb->x && ev->y == kb->y) return kb->act;
	return -1;
}

/*
 * Config file helpers.
 */

struct str2num {
	unsigned char *str;
	long num;
};

static long str2num(struct str2num *table, char *s)
{
	struct str2num *p;
	for (p = table; p->str; p++) 
		if (!strcmp(p->str, s)) return p->num;
	return -1;
}

static int parse_keymap(unsigned char *s)
{
	struct str2num table[] = {
		{ "main", KM_MAIN },
		{ "edit", KM_EDIT },
		{ "menu", KM_MENU },
		{ NULL, 0 }
	};
	return str2num(table, s);
}

static long parse_key(unsigned char *s)
{
	struct str2num table[] = {
		{ "Enter", KBD_ENTER },
		{ "Backspace", KBD_BS },
		{ "Tab", KBD_TAB },
		{ "Escape", KBD_ESC },
		{ "Left", KBD_LEFT },
		{ "Right", KBD_RIGHT },
		{ "Up", KBD_UP },
		{ "Down", KBD_DOWN },
		{ "Insert", KBD_INS },
		{ "Delete", KBD_DEL },
		{ "Home", KBD_HOME },
		{ "End", KBD_END },
		{ "PageUp", KBD_PAGE_UP },
		{ "PageDown", KBD_PAGE_DOWN },
		{ "F1", KBD_F1 },
		{ "F2", KBD_F2 },
		{ "F3", KBD_F3 },
		{ "F4", KBD_F4 },
		{ "F5", KBD_F5 },
		{ "F6", KBD_F6 },
		{ "F7", KBD_F7 },
		{ "F8", KBD_F8 },
		{ "F9", KBD_F9 },
		{ "F10", KBD_F10 },
		{ "F11", KBD_F11 },
		{ "F12", KBD_F12 },
		{ NULL, 0 }
	};
	return (strlen(s) == 1) ? *s : str2num(table, s);
}

static int parse_keystroke(unsigned char *s, long *x, long *y)
{
	*y = 0;
	if (!strncmp(s, "Shift-", 6)) *y |= KBD_SHIFT, s += 6;
	else if (!strncmp(s, "Ctrl-", 5)) *y |= KBD_CTRL, s += 5;
	else if (!strncmp(s, "Alt-", 4)) *y |= KBD_ALT, s += 4;
	return ((*x = parse_key(s)) < 0) ? -1 : 0;
}

static int parse_act(unsigned char *s)
{
	int i;
	unsigned char *table[] = {
		"add-bookmark",
		"auto-complete",
		"back",
		"backspace",
		"bookmark-manager",
		"copy-clipboard",
		"cut-clipboard",
		"delete",
		"document-info",
		"down",
		"download",
		"end",
		"enter",
		"file-menu",
		"find-next",
		"find-next-back",
		"goto-url",
		"goto-url-current",
		"goto-url-current-link",
		"header-info",
		"home",
		"kill-to-bol",
		"kill-to-eol",
		"left",
		"menu",
		"next-frame",
		"open-new-window",
		"open-link-in-new-window",
		"page-down",
		"page-up",
		"paste-clipboard",
		"previous-frame",
		"really-quit",
		"quit",
		"reload",
		"right",
		"scroll-down",
		"scroll-left",
		"scroll-right",
		"scroll-up",
		"search",
		"search-back",
		"toggle-display-images",
		"toggle-display-tables",
		"toggle-html-plain",
		"up",
		"view-image",
		"zoom-frame",
		NULL
	};
	for (i = 0; table[i]; i++) if (!strcmp(table[i], s)) return i;
	return -1;
}

/*
 * Config file readers.
 */

/* bind KEYMAP KEYSTROKE ACTION */
unsigned char *bind_rd(struct option *o, unsigned char *line)
{
	unsigned char *err = NULL;
	unsigned char *ckmap;
	unsigned char *ckey;
	unsigned char *cact;
	int kmap;
	long x, y;
	int act;

	ckmap = get_token(&line);
	ckey = get_token(&line);
	cact = get_token(&line);

	if (!ckmap || !ckey || !cact)
		err = "Missing arguments"; 
	else if ((kmap = parse_keymap(ckmap)) < 0)
		err = "Unrecognised keymap";
	else if (parse_keystroke(ckey, &x, &y) < 0)
		err = "Error parsing keystroke";
	else if ((act = parse_act(cact)) < 0)
		err = "Unrecognised action"; 
	else 
		add_keybinding(kmap, act, x, y);

	if (cact) mem_free(cact);
	if (ckey) mem_free(ckey);
	if (ckmap) mem_free(ckmap);
	return err;
}

/* unbind KEYMAP KEYSTROKE */
unsigned char *unbind_rd(struct option *o, unsigned char *line)
{
	unsigned char *err = NULL;
	unsigned char *ckmap;
	unsigned char *ckey;
	int kmap;
	long x, y;
	
	ckmap = get_token(&line);
	ckey = get_token(&line);
	if (!ckmap)
		err = "Missing arguments"; 
	else if ((kmap = parse_keymap(ckmap)) < 0)
		err = "Unrecognised keymap";
	else if (parse_keystroke(ckey, &x, &y) < 0)
		err = "Error parsing keystroke";
	else 
		delete_keybinding(kmap, x, y);

	if (ckey) mem_free(ckey);
	if (ckmap) mem_free(ckmap);
	return err;
}

/*
 * Default keybindings.
 */

struct default_kb {
	int act;
	long x;
	long y;
};

static struct default_kb default_main_keymap[] = {
	{ ACT_PAGE_DOWN, KBD_PAGE_DOWN, 0 },
	{ ACT_PAGE_DOWN, ' ', 0 },
	{ ACT_PAGE_DOWN, 'F', KBD_CTRL },
	{ ACT_PAGE_UP, KBD_PAGE_UP, 0 },
	{ ACT_PAGE_UP, 'b', 0 },
	{ ACT_PAGE_UP, 'B', 0 },
	{ ACT_PAGE_UP, 'B', KBD_CTRL },
	{ ACT_DOWN, KBD_DOWN, 0 },
	{ ACT_UP, KBD_UP, 0 },
	{ ACT_COPY_CLIPBOARD, KBD_INS, KBD_CTRL },
	{ ACT_COPY_CLIPBOARD, 'C', KBD_CTRL },
	{ ACT_SCROLL_UP, KBD_INS, 0 },
	{ ACT_SCROLL_UP, 'P', KBD_CTRL },
	{ ACT_SCROLL_DOWN, KBD_DEL, 0 },
	{ ACT_SCROLL_DOWN, 'N', KBD_CTRL },
	{ ACT_SCROLL_LEFT, '[', 0 },
	{ ACT_SCROLL_RIGHT, ']', 0 },
	{ ACT_HOME, KBD_HOME, 0 },
	{ ACT_HOME, 'A', KBD_CTRL },
	{ ACT_END, KBD_END, 0 },
	{ ACT_END, 'E', KBD_CTRL },
	{ ACT_ENTER, KBD_RIGHT, 0 },
	{ ACT_ENTER, KBD_ENTER, 0 },
	{ ACT_BACK, KBD_LEFT, 0 },
	{ ACT_DOWNLOAD, 'd', 0 },
	{ ACT_DOWNLOAD, 'D', 0 },
	{ ACT_SEARCH, '/', 0 },
	{ ACT_SEARCH_BACK, '?', 0 },
	{ ACT_FIND_NEXT, 'n', 0 },
	{ ACT_FIND_NEXT_BACK, 'N', 0 },
	{ ACT_ZOOM_FRAME, 'f', 0 },
	{ ACT_ZOOM_FRAME, 'F', 0 },
	{ ACT_RELOAD, 'R', KBD_CTRL },
	{ ACT_GOTO_URL, 'g', 0 },
	{ ACT_GOTO_URL_CURRENT, 'G', 0 },
	{ ACT_ADD_BOOKMARK, 'a', 0 },
	{ ACT_ADD_BOOKMARK, 'A', 0 },
	{ ACT_BOOKMARK_MANAGER, 's', 0 },
	{ ACT_BOOKMARK_MANAGER, 'S', 0 },
	{ ACT_QUIT, 'q', 0 },
	{ ACT_REALLYQUIT, 'Q', 0 },
	{ ACT_DOCUMENT_INFO, '=', 0 },
	{ ACT_HEADER_INFO, '|', 0 },
	{ ACT_TOGGLE_HTML_PLAIN, '\\', 0 },
	{ ACT_TOGGLE_DISPLAY_IMAGES, '*', 0 },
	{ ACT_NEXT_FRAME, KBD_TAB, 0 },
	{ ACT_MENU, KBD_ESC, 0 },
	{ ACT_MENU, KBD_F9, 0 },
	{ ACT_FILE_MENU, KBD_F10, 0 },
	{ 0, 0, 0 }
};

static struct default_kb default_edit_keymap[] = {
	{ ACT_LEFT, KBD_LEFT, 0 },
	{ ACT_RIGHT, KBD_RIGHT, 0 },
	{ ACT_HOME, KBD_HOME, 0 },
	{ ACT_HOME, 'A', KBD_CTRL },
	{ ACT_UP, KBD_UP, 0 },
	{ ACT_DOWN, KBD_DOWN, 0 },
	{ ACT_END, KBD_END, 0 },
	{ ACT_END, 'E', KBD_CTRL },
	{ ACT_COPY_CLIPBOARD, KBD_INS, KBD_CTRL },
	{ ACT_COPY_CLIPBOARD, 'C', KBD_CTRL },
	{ ACT_CUT_CLIPBOARD, 'X', KBD_CTRL },
	{ ACT_PASTE_CLIPBOARD, 'V', KBD_CTRL },
	{ ACT_ENTER, KBD_ENTER, 0 },
	{ ACT_BACKSPACE, KBD_BS, 0 },
	{ ACT_BACKSPACE, 'H', KBD_CTRL },
	{ ACT_DELETE, KBD_DEL, 0 },
	{ ACT_DELETE, 'D', KBD_CTRL },
	{ ACT_KILL_TO_BOL, 'U', KBD_CTRL },
	{ ACT_KILL_TO_EOL, 'K', KBD_CTRL },
    	{ ACT_AUTO_COMPLETE, 'W', KBD_CTRL },
	{ 0, 0, 0 }
};

static struct default_kb default_menu_keymap[] = {
	{ ACT_LEFT, KBD_LEFT, 0 },
	{ ACT_RIGHT, KBD_RIGHT, 0 },
	{ ACT_HOME, KBD_HOME, 0 },
	{ ACT_HOME, 'A', KBD_CTRL },
	{ ACT_UP, KBD_UP, 0 },
	{ ACT_DOWN, KBD_DOWN, 0 },
	{ ACT_END, KBD_END, 0 },
	{ ACT_END, 'E', KBD_CTRL },
	{ ACT_ENTER, KBD_ENTER, 0 },
	{ ACT_PAGE_DOWN, KBD_PAGE_DOWN, 0 },
	{ ACT_PAGE_DOWN, 'F', KBD_CTRL },
	{ ACT_PAGE_UP, KBD_PAGE_UP, 0 },
	{ ACT_PAGE_UP, 'B', KBD_CTRL },
	{ 0, 0, 0}
};

static void add_default_keybindings()
{
	struct default_kb *kb;
	for (kb = default_main_keymap; kb->x; kb++) add_keybinding(KM_MAIN, kb->act, kb->x, kb->y);
    	for (kb = default_edit_keymap; kb->x; kb++) add_keybinding(KM_EDIT, kb->act, kb->x, kb->y);
    	for (kb = default_menu_keymap; kb->x; kb++) add_keybinding(KM_MENU, kb->act, kb->x, kb->y);
}
