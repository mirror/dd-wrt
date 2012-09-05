#include "links.h"

#define format format_

struct list_head html_stack = {&html_stack, &html_stack};

static inline int atchr(unsigned char c)
{
	return /*isA(c) ||*/ (c > ' ' && c != '=' && c != '<' && c != '>');
}

int parse_element(unsigned char *e, unsigned char *eof, unsigned char **name, int *namelen, unsigned char **attr, unsigned char **end)
{
	if (eof - e < 3 || *(e++) != '<') return -1;
	if (name) *name = e;
	if (*e == '/') e++;
	if (!isA(*e)) return -1;
	goto x1;
	while (isA(*e) || *e == '=') {
		x1:
		e++;
		if (e >= eof) return -1;
	}
	/*if ((!WHITECHAR(*e) && *e != '>' && *e != '<' && *e != '/' && *e != ':')) return -1;*/
	if (name && namelen) *namelen = e - *name;
	while ((WHITECHAR(*e) || *e == '/' || *e == ':')) {
		e++;
		if (e >= eof) return -1;
	}
	if ((!atchr(*e) && *e != '>' && *e != '<')) return -1;
	if (attr) *attr = e;
	nextattr:
	while (WHITECHAR(*e)) {
		e++;
		if (e >= eof) return -1;
	}
	if ((!atchr(*e) && *e != '>' && *e != '<')) return -1;
	if (*e == '>' || *e == '<') goto en;
	while (atchr(*e)) {
		e++;
		if (e >= eof) return -1;
	}
	while (WHITECHAR(*e)) {
		e++;
		if (e >= eof) return -1;
	}
	if (*e != '=') goto endattr;
	goto x2;
	while (WHITECHAR(*e)) {
		x2:
		e++;
		if (e >= eof) return -1;
	}
	if (U(*e)) {
		unsigned char uu = *e;
		/*u:*/
		goto x3;
		while (e < eof && *e != uu && *e /*(WHITECHAR(*e) || *e > ' ')*/) {
			x3:
			e++;
			if (e >= eof) return -1;
		}
		if (*e < ' ') return -1;
		e++;
		if (e >= eof /*|| (!WHITECHAR(*e) && *e != uu && *e != '>' && *e != '<')*/) return -1;
		/*if (*e == uu) goto u;*/
	} else {
		while (!WHITECHAR(*e) && *e != '>' && *e != '<') {
			e++;
			if (e >= eof) return -1;
		}
	}
	while (WHITECHAR(*e)) {
		e++;
		if (e >= eof) return -1;
	}
	endattr:
	if (*e != '>' && *e != '<') goto nextattr;
	en:
	if (end) *end = e + (*e == '>');
	return 0;
}

#define add_chr(s, l, c)						\
do {									\
	if (!((l) & (32 - 1))) {					\
		if ((unsigned)(l) > MAXINT - 32) overalloc();		\
		(s) = mem_realloc((s), (l) + 32);			\
	}								\
	(s)[(l)++] = (c);						\
} while (0)

int get_attr_val_nl = 0;

/* parses html element attributes */
/* e is attr pointer previously get from parse_element, DON'T PASS HERE ANY OTHER VALUE!!! */
/* name is searched attribute */
/* returns allocated string containing the attribute, or NULL on unsuccess */
unsigned char *get_attr_val(unsigned char *e, unsigned char *name)
{
	unsigned char *n;
	unsigned char *a = DUMMY;
	int l = 0;
	int f;
	aa:
	while (WHITECHAR(*e)) e++;
	if (*e == '>' || *e == '<') return NULL;
	n = name;
	while (*n && upcase(*e) == upcase(*n)) e++, n++;
	f = *n;
	while (atchr(*e)) f = 1, e++;
	while (WHITECHAR(*e)) e++;
	if (*e != '=') goto ea;
	e++;
	while (WHITECHAR(*e)) e++;
	if (!U(*e)) {
		while (!WHITECHAR(*e) && *e != '>' && *e != '<') {
			if (!f) add_chr(a, l, *e);
			e++;
		}
	} else {
		char uu = *e;
		/*a:*/
		e++;
		while (*e != uu) {
			if (!*e) {
				mem_free(a);
				return NULL;
			}
			if (!f) {
				if (get_attr_val_nl == 2) goto exact;
				if (*e != 13) {
					if (*e != 9 && *e != 10) exact:add_chr(a, l, *e);
					else if (!get_attr_val_nl) add_chr(a, l, ' ');
				}
			}
			e++;
		}
		e++;
		/*if (*e == uu) {
			if (!f) add_chr(a, l, *e);
			goto a;
		}*/
	}
	ea:
	if (!f) {
		unsigned char *b;
		add_chr(a, l, 0);
		if (strchr(a, '&')) {
			unsigned char *aa = a;
			int c = d_opt->cp;
			d_opt->cp = d_opt->real_cp;
			a = convert_string(NULL, aa, strlen(aa));
			d_opt->cp = c;
			mem_free(aa);
		}
		while ((b = strchr(a, 1))) *b = ' ';
		if (get_attr_val_nl != 2) {
			for (b = a; *b == ' '; b++);
			if (b != a) memmove(a, b, strlen(b) + 1);
			for (b = a + strlen(a) - 1; b >= a && *b == ' '; b--) *b = 0;
		}
		set_mem_comment(a, name, strlen(name));
		return a;
	}
	goto aa;
}

int has_attr(unsigned char *e, unsigned char *name)
{
	char *a;
	if (!(a = get_attr_val(e, name))) return 0;
	mem_free(a);
	return 1;
}

/*
unsigned char *get_url_val(unsigned char *e, unsigned char *name)
{
	int n = 0;
	unsigned char *p, *q, *pp;
	if (!(pp = get_attr_val(e, name))) return NULL;
	p = pp; q = pp;
	while (1) {
		if (*p == '#') n = 1;
		if ((*p = *q) != ' ' || n) p++;
		if (!*q) break;
		q++;
	}
	return pp;
}
*/

unsigned char *get_url_val(unsigned char *e, unsigned char *name)
{
	unsigned char *a;
	get_attr_val_nl = 1;
	a = get_attr_val(e, name);
	get_attr_val_nl = 0;
	return a;
}

unsigned char *get_exact_attr_val(unsigned char *e, unsigned char *name)
{
	unsigned char *a;
	get_attr_val_nl = 2;
	a = get_attr_val(e, name);
	get_attr_val_nl = 0;
	if (a) {
		unsigned char *x1, *x2;
		for (x1 = x2 = a; *x1; x1++, x2++) {
			if (x1[0] == '\r') {
				*x2 = '\n';
				if (x1[1] == '\n') x1++;
			} else {
				*x2 = *x1;
			}
		}
		*x2 = 0;
	}
	return a;
}

struct {
	unsigned short int n;
	char *s;
} roman_tbl[] = {
	{ 1000,	"m" },
	{ 999,	"im" },
/*	{ 995,	"vm" },*/
	{ 990,	"xm" },
/*	{ 950,	"lm" },*/
	{ 900,	"cm" },
	{ 500,	"d" },
	{ 499,	"id" },
/*	{ 495,	"vd" },*/
	{ 490,	"xd" },
/*	{ 450,	"ld" },*/
	{ 400,	"cd" },
	{ 100,	"c" },
	{ 99,	"ic" },
/*	{ 95,	"vc" },*/
	{ 90,	"xc" },
	{ 50,	"l" },
	{ 49,	"il" },
/*	{ 45,	"vl" },*/
	{ 40,	"xl" },
	{ 10,	"x" },
	{ 9,	"ix" },
	{ 5,	"v" },
	{ 4,	"iv" },
	{ 1,	"i" },
	{ 0,	NULL },
};

void roman(char *p, unsigned n)
{
	int i = 0;
	if (n >= 4000) {
		strcpy(p, "---");
		return;
	}
	if (!n) {
		strcpy(p, "o");
		return;
	}
	p[0] = 0;
	while (n) {
		while (roman_tbl[i].n <= n) {
			n -= roman_tbl[i].n;
			strcat(p, roman_tbl[i].s);
		}
		i++;
		if (n && !roman_tbl[i].n) {
			internal("BUG in roman number convertor");
			return;
		}
	}
}

struct color_spec {
	char *name;
	int rgb;
};

struct color_spec color_specs[] = {
	{"aliceblue",		0xF0F8FF},
	{"antiquewhite",	0xFAEBD7},
	{"aqua",		0x00FFFF},
	{"aquamarine",		0x7FFFD4},
	{"azure",		0xF0FFFF},
	{"beige",		0xF5F5DC},
	{"bisque",		0xFFE4C4},
	{"black",		0x000000},
	{"blanchedalmond",	0xFFEBCD},
	{"blue",		0x0000FF},
	{"blueviolet",		0x8A2BE2},
	{"brown",		0xA52A2A},
	{"burlywood",		0xDEB887},
	{"cadetblue",		0x5F9EA0},
	{"chartreuse",		0x7FFF00},
	{"chocolate",		0xD2691E},
	{"coral",		0xFF7F50},
	{"cornflowerblue",	0x6495ED},
	{"cornsilk",		0xFFF8DC},
	{"crimson",		0xDC143C},
	{"cyan",		0x00FFFF},
	{"darkblue",		0x00008B},
	{"darkcyan",		0x008B8B},
	{"darkgoldenrod",	0xB8860B},
	{"darkgray",		0xA9A9A9},
	{"darkgreen",		0x006400},
	{"darkkhaki",		0xBDB76B},
	{"darkmagenta",		0x8B008B},
	{"darkolivegreen",	0x556B2F},
	{"darkorange",		0xFF8C00},
	{"darkorchid",		0x9932CC},
	{"darkred",		0x8B0000},
	{"darksalmon",		0xE9967A},
	{"darkseagreen",	0x8FBC8F},
	{"darkslateblue",	0x483D8B},
	{"darkslategray",	0x2F4F4F},
	{"darkturquoise",	0x00CED1},
	{"darkviolet",		0x9400D3},
	{"deeppink",		0xFF1493},
	{"deepskyblue",		0x00BFFF},
	{"dimgray",		0x696969},
	{"dodgerblue",		0x1E90FF},
	{"firebrick",		0xB22222},
	{"floralwhite",		0xFFFAF0},
	{"forestgreen",		0x228B22},
	{"fuchsia",		0xFF00FF},
	{"gainsboro",		0xDCDCDC},
	{"ghostwhite",		0xF8F8FF},
	{"gold",		0xFFD700},
	{"goldenrod",		0xDAA520},
	{"gray",		0x808080},
	{"green",		0x008000},
	{"greenyellow",		0xADFF2F},
	{"honeydew",		0xF0FFF0},
	{"hotpink",		0xFF69B4},
	{"indianred",		0xCD5C5C},
	{"indigo",		0x4B0082},
	{"ivory",		0xFFFFF0},
	{"khaki",		0xF0E68C},
	{"lavender",		0xE6E6FA},
	{"lavenderblush",	0xFFF0F5},
	{"lawngreen",		0x7CFC00},
	{"lemonchiffon",	0xFFFACD},
	{"lightblue",		0xADD8E6},
	{"lightcoral",		0xF08080},
	{"lightcyan",		0xE0FFFF},
	{"lightgoldenrodyellow",	0xFAFAD2},
	{"lightgreen",		0x90EE90},
	{"lightgrey",		0xD3D3D3},
	{"lightpink",		0xFFB6C1},
	{"lightsalmon",		0xFFA07A},
	{"lightseagreen",	0x20B2AA},
	{"lightskyblue",	0x87CEFA},
	{"lightslategray",	0x778899},
	{"lightsteelblue",	0xB0C4DE},
	{"lightyellow",		0xFFFFE0},
	{"lime",		0x00FF00},
	{"limegreen",		0x32CD32},
	{"linen",		0xFAF0E6},
	{"magenta",		0xFF00FF},
	{"maroon",		0x800000},
	{"mediumaquamarine",	0x66CDAA},
	{"mediumblue",		0x0000CD},
	{"mediumorchid",	0xBA55D3},
	{"mediumpurple",	0x9370DB},
	{"mediumseagreen",	0x3CB371},
	{"mediumslateblue",	0x7B68EE},
	{"mediumspringgreen",	0x00FA9A},
	{"mediumturquoise",	0x48D1CC},
	{"mediumvioletred",	0xC71585},
	{"midnightblue",	0x191970},
	{"mintcream",		0xF5FFFA},
	{"mistyrose",		0xFFE4E1},
	{"moccasin",		0xFFE4B5},
	{"navajowhite",		0xFFDEAD},
	{"navy",		0x000080},
	{"oldlace",		0xFDF5E6},
	{"olive",		0x808000},
	{"olivedrab",		0x6B8E23},
	{"orange",		0xFFA500},
	{"orangered",		0xFF4500},
	{"orchid",		0xDA70D6},
	{"palegoldenrod",	0xEEE8AA},
	{"palegreen",		0x98FB98},
	{"paleturquoise",	0xAFEEEE},
	{"palevioletred",	0xDB7093},
	{"papayawhip",		0xFFEFD5},
	{"peachpuff",		0xFFDAB9},
	{"peru",		0xCD853F},
	{"pink",		0xFFC0CB},
	{"plum",		0xDDA0DD},
	{"powderblue",		0xB0E0E6},
	{"purple",		0x800080},
	{"red",			0xFF0000},
	{"rosybrown",		0xBC8F8F},
	{"royalblue",		0x4169E1},
	{"saddlebrown",		0x8B4513},
	{"salmon",		0xFA8072},
	{"sandybrown",		0xF4A460},
	{"seagreen",		0x2E8B57},
	{"seashell",		0xFFF5EE},
	{"sienna",		0xA0522D},
	{"silver",		0xC0C0C0},
	{"skyblue",		0x87CEEB},
	{"slateblue",		0x6A5ACD},
	{"slategray",		0x708090},
	{"snow",		0xFFFAFA},
	{"springgreen",		0x00FF7F},
	{"steelblue",		0x4682B4},
	{"tan",			0xD2B48C},
	{"teal",		0x008080},
	{"thistle",		0xD8BFD8},
	{"tomato",		0xFF6347},
	{"turquoise",		0x40E0D0},
	{"violet",		0xEE82EE},
	{"wheat",		0xF5DEB3},
	{"white",		0xFFFFFF},
	{"whitesmoke",		0xF5F5F5},
	{"yellow",		0xFFFF00},
	{"yellowgreen",		0x9ACD32},
};

#define endof(T) ((T)+sizeof(T)/sizeof(*(T)))

int decode_color(unsigned char *str, struct rgb *col)
{
	unsigned long ch;
	if (*str != '#') {
		struct color_spec *cs;
		for (cs = color_specs; cs < endof(color_specs); cs++)
			if (!strcasecmp(cs->name, str)) {
				ch = cs->rgb;
				goto found;
			}
		str--;
	}
	str++;
	if (strlen(str) == 6) {
		char *end;
		ch = strtoul(str, &end, 16);
		if (!*end && ch < 0x1000000) {
found:
			col->r = ch / 0x10000;
			col->g = ch / 0x100 % 0x100;
			col->b = ch % 0x100;
			return 0;
		}
	}
	return -1;
}

int get_color(unsigned char *a, unsigned char *c, struct rgb *rgb)
{
	char *at;
	int r = -1;
	if (d_opt->col >= 1) if ((at = get_attr_val(a, c))) {
		r = decode_color(at, rgb);
		mem_free(at);
	}
	return r;
}

int get_bgcolor(unsigned char *a, struct rgb *rgb)
{
	if (d_opt->col < 2) return -1;
	return get_color(a, "bgcolor", rgb);
}

unsigned char *get_target(unsigned char *a)
{
	unsigned char *v = get_attr_val(a, "target");
	if (v) {
		if (!strcasecmp(v, "_self")) {
			mem_free(v);
			v = stracpy(d_opt->framename);
		}
	}
	return v;
}

void kill_html_stack_item(struct html_element *e)
{
	if (e->dontkill == 2) {
		internal("trying to kill unkillable element");
		return;
	}
	if (!e || (void *)e == &html_stack) {
		internal("trying to free bad html element");
		return;
	}
	if (e->attr.link) mem_free(e->attr.link);
	if (e->attr.target) mem_free(e->attr.target);
	if (e->attr.image) mem_free(e->attr.image);
	if (e->attr.href_base) mem_free(e->attr.href_base);
	if (e->attr.target_base) mem_free(e->attr.target_base);
	if (e->attr.select) mem_free(e->attr.select);
	del_from_list(e);
	mem_free(e);
	/*if ((void *)(html_stack.next) == &html_stack || !html_stack.next) {
		debug("killing last element");
	}*/
}

static inline void kill_elem(char *e)
{
	if ((size_t)html_top.namelen == strlen(e) && !casecmp(html_top.name, e, html_top.namelen))
		kill_html_stack_item(&html_top);
}

#ifdef DEBUG
void debug_stack()
{
	struct html_element *e;
	printf("HTML stack debug: \n");
	foreachback(e, html_stack) {
		int i;
		printf("\"");
		for (i = 0; i < e->namelen; i++) printf("%c", e->name[i]);
		printf("\"\n");
	}
	printf("%c", 7);
	fflush(stdout);
	sleep(1);
}
#endif

void html_stack_dup()
{
	struct html_element *e;
	struct html_element *ep;
	if ((void *)(ep = html_stack.next) == &html_stack || !html_stack.next) {
	   	internal("html stack empty");
		return;
	}
	e = mem_alloc(sizeof(struct html_element));
	memcpy(e, ep, sizeof(struct html_element));
	e->attr.link = stracpy(ep->attr.link);
	e->attr.target = stracpy(ep->attr.target);
	e->attr.image = stracpy(ep->attr.image);
	e->attr.href_base = stracpy(ep->attr.href_base);
	e->attr.target_base = stracpy(ep->attr.target_base);
	e->attr.select = stracpy(ep->attr.select);
	/*if (e->name) {
		if (e->attr.link) set_mem_comment(e->attr.link, e->name, e->namelen);
		if (e->attr.target) set_mem_comment(e->attr.target, e->name, e->namelen);
		if (e->attr.image) set_mem_comment(e->attr.image, e->name, e->namelen);
		if (e->attr.href_base) set_mem_comment(e->attr.href_base, e->name, e->namelen);
		if (e->attr.target_base) set_mem_comment(e->attr.target_base, e->name, e->namelen);
		if (e->attr.select) set_mem_comment(e->attr.select, e->name, e->namelen);
	}*/
	e->name = e->options = NULL;
	e->namelen = 0;
	e->dontkill = 0;
	add_to_list(html_stack, e);
}

void *ff;
int (*put_chars_f)(void *, unsigned char *, int);
void (*line_break_f)(void *);
void *(*special_f)(void *, int, ...);

unsigned char *eoff;
unsigned char *eofff;
unsigned char *startf;

int line_breax;
int pos;
int putsp;

int was_br;
int table_level;
int empty_format;

void ln_break(int n, void (*line_break)(void *), void *f)
{
	if (!n || html_top.invisible) return;
	while (n > line_breax) line_breax++, line_break(f);
	pos = 0;
	putsp = -1;
}

void put_chrs(unsigned char *start, int len, int (*put_chars)(void *, unsigned char *, int), void *f)
{
	if (par_format.align == AL_NO) putsp = 0;
	if (!len || html_top.invisible) return;
	if (putsp == 1) pos += put_chars(f, " ", 1), putsp = -1;
	if (putsp == -1) {
		if (start[0] == ' ') start++, len--;
		putsp = 0;
	}
	if (!len) {
		putsp = -1;
		if (par_format.align == AL_NO) putsp = 0;
		return;
	}
	if (start[len - 1] == ' ') putsp = -1;
	if (par_format.align == AL_NO) putsp = 0;
	was_br = 0;
	pos += put_chars(f, start, len);
	line_breax = 0;
}

void kill_until(int ls, ...)
{
	int l;
	struct html_element *e = &html_top;
	if (ls) e = e->next;
	while ((void *)e != &html_stack) {
		int sk = 0;
		va_list arg;
		va_start(arg, ls);
		while (1) {
			char *s = va_arg(arg, char *);
			if (!s) break;
			if (!*s) sk++;
			else if ((size_t)e->namelen == strlen(s) && !casecmp(e->name, s, strlen(s))) {
				if (!sk) {
					if (e->dontkill) break;
					va_end(arg);
					goto killll;
				} else if (sk == 1) {
					va_end(arg);
					goto killl;
				} else break;
			}
		}
		va_end(arg);
		if (e->dontkill || (e->namelen == 5 && !casecmp(e->name, "TABLE", 5))) break;
		if (e->namelen == 2 && upcase(e->name[0]) == 'T' && (upcase(e->name[1]) == 'D' || upcase(e->name[1]) == 'H' || upcase(e->name[1]) == 'R')) break;
		e = e->next;
	}
	return;
	killl:
	e = e->prev;
	killll:
	l = 0;
	while ((void *)e != &html_stack) {
		if (ls && e == html_stack.next) break;
		if (e->linebreak > l) l = e->linebreak;
		e = e->prev;
		kill_html_stack_item(e->next);
	}
	ln_break(l, line_break_f, ff);
}

int get_num(unsigned char *a, unsigned char *n)
{
	char *al;
	if ((al = get_attr_val(a, n))) {
		char *end;
		unsigned long s = strtoul(al, &end, 10);
		if (!*al || *end || s > 10000) s = -1;
		mem_free(al);
		return s;
	}
	return -1;
}

int parse_width(unsigned char *w, int trunc)
{
	unsigned char *end;
	int p = 0;
	long s;
	int l;
	while (WHITECHAR(*w)) w++;
	for (l = 0; w[l] && w[l] != ','; l++) ;
	while (l && WHITECHAR(w[l - 1])) l--;
	if (!l) return -1;
	if (w[l - 1] == '%') l--, p = 1;
	while (l && WHITECHAR(w[l - 1])) l--;
	if (!l) return -1;
	s = strtoul((char *)w, (char **)(void *)&end, 10);
	if (end - w < l || s < 0 || s > 10000) return -1;
	if (p) {
		if (trunc) s = s * (par_format.width - par_format.leftmargin - par_format.rightmargin) / 100;
		else return -1;
	} else s = (s + (HTML_CHAR_WIDTH - 1) / 2) / HTML_CHAR_WIDTH;
	if (trunc && s > par_format.width - par_format.leftmargin - par_format.rightmargin) s = par_format.width - par_format.leftmargin - par_format.rightmargin;
	if (s < 0) s = 0;
	return s;
}

int get_width(unsigned char *a, unsigned char *n, int trunc)
{
	int r;
	unsigned char *w;
	if (!(w = get_attr_val(a, n))) return -1;
	r = parse_width(w, trunc);
	mem_free(w);
	return r;
}

/*int form_num;
struct form form = { 0, NULL, 0 };
int g_ctrl_num;*/

struct form form = { NULL, NULL, 0, 0 };

unsigned char *last_form_tag;
unsigned char *last_form_attr;
unsigned char *last_input_tag;

void put_link_line(unsigned char *prefix, unsigned char *linkname, unsigned char *link, unsigned char *target)
{
	html_stack_dup();
	ln_break(1, line_break_f, ff);
	if (format.link) mem_free(format.link), format.link = NULL;
	if (format.target) mem_free(format.target), format.target = NULL;
	format.form = NULL;
	put_chrs(prefix, strlen(prefix), put_chars_f, ff);
	format.link = join_urls(format.href_base, link);
	format.target = stracpy(target);
	memcpy(&format.fg, &format.clink, sizeof(struct rgb));
	put_chrs(linkname, strlen(linkname), put_chars_f, ff);
	ln_break(1, line_break_f, ff);
	kill_html_stack_item(&html_top);
}

void html_span(unsigned char *a) { }
void html_bold(unsigned char *a) { format.attr |= AT_BOLD; }
void html_italic(unsigned char *a) { format.attr |= AT_ITALIC; }
void html_underline(unsigned char *a) { format.attr |= AT_UNDERLINE; }
void html_fixed(unsigned char *a) { format.attr |= AT_FIXED; }

void html_a(unsigned char *a)
{
	char *al;
	if ((al = get_url_val(a, "href"))) {
		char *all = al;
		while (all[0] == ' ') all++;
		while (all[0] && all[strlen(all) - 1] == ' ') all[strlen(all) - 1] = 0;
		if (format.link) mem_free(format.link);
		format.link = join_urls(format.href_base, all);
		mem_free(al);
		if ((al = get_target(a))) {
			if (format.target) mem_free(format.target);
			format.target = al;
		} else {
			if (format.target) mem_free(format.target);
			format.target = stracpy(format.target_base);
		}
		/*format.attr ^= AT_BOLD;*/
		memcpy(&format.fg, &format.clink, sizeof(struct rgb));
	} else kill_html_stack_item(&html_top);
	if ((al = get_attr_val(a, "name"))) {
		special_f(ff, SP_TAG, al);
		mem_free(al);
	}
}

void html_font(unsigned char *a)
{
	char *al;
	if ((al = get_attr_val(a, "size"))) {
		int p = 0;
		unsigned long s;
		char *nn = al;
		char *end;
		if (*al == '+') p = 1, nn++;
		if (*al == '-') p = -1, nn++;
		s = strtoul(nn, &end, 10);
		if (*nn && !*end) {
			if (s > 7) s = 7;
			if (!p) format.fontsize = s;
			else format.fontsize += p * s;
			if (format.fontsize < 1) format.fontsize = 1;
			if (format.fontsize > 7) format.fontsize = 7;
		}
		mem_free(al);
	}
	get_color(a, "color", &format.fg);
}

void html_img(unsigned char *a)
{
	unsigned char *al;
	int ismap, usemap = 0;
	/*put_chrs(" ", 1, put_chars_f, ff);*/
	if ((al = get_attr_val(a, "usemap"))) {
		unsigned char *u;
		usemap = 1;
		html_stack_dup();
		if (format.link) mem_free(format.link);
		if (format.form) format.form = NULL;
		u = join_urls(format.href_base, al);
		format.link = mem_alloc(strlen(u) + 5);
		strcpy(format.link, "MAP@");
		strcat(format.link, u);
		format.attr |= AT_BOLD;
		mem_free(u);
		mem_free(al);
	}
	ismap = format.link && has_attr(a, "ismap") && !usemap;
	if ((!(al = get_attr_val(a, "alt")) && !(al = get_attr_val(a, "title"))) || !*al) {
		if (al) mem_free(al);
		if (!d_opt->images && !format.link) return;
		if (usemap) al = stracpy("[USEMAP]");
		else if (ismap) al = stracpy("[ISMAP]");
		else al = stracpy("[IMG]");
	}
	if (format.image) mem_free(format.image), format.image = NULL;
	if (al) {
		unsigned char *s;
		if ((s = get_url_val(a, "src")) || (s = get_attr_val(a, "dynsrc"))) {
			format.image = join_urls(format.href_base, s);
			mem_free(s);
		}
		if (ismap) {
			unsigned char *h;
			html_stack_dup();
			h = stracpy(format.link);
			add_to_strn(&h, "?0,0");
			mem_free(format.link);
			format.link = h;
		}
		put_chrs(al, strlen(al), put_chars_f, ff);
		if (ismap) kill_html_stack_item(&html_top);
	}
	if (format.image) mem_free(format.image), format.image = NULL;
	mem_free(al);
	if (usemap) kill_html_stack_item(&html_top);
	/*put_chrs(" ", 1, put_chars_f, ff);*/
}

void html_body(unsigned char *a)
{
	get_color(a, "text", &format.fg);
	get_color(a, "link", &format.clink);
	get_color(a, "vlink", &format.vlink);
	get_bgcolor(a, &format.bg);
	get_bgcolor(a, &par_format.bgcolor);
}

void html_skip(unsigned char *a) { html_top.invisible = html_top.dontkill = 1; }

void html_title(unsigned char *a) { html_top.invisible = html_top.dontkill = 1; }

void html_center(unsigned char *a)
{
	par_format.align = AL_CENTER;
	if (!table_level) par_format.leftmargin = par_format.rightmargin = 0;
}

void html_linebrk(unsigned char *a)
{
	char *al;
	if ((al = get_attr_val(a, "align"))) {
		if (!strcasecmp(al, "left")) par_format.align = AL_LEFT;
		if (!strcasecmp(al, "right")) par_format.align = AL_RIGHT;
		if (!strcasecmp(al, "center")) {
			par_format.align = AL_CENTER;
			if (!table_level) par_format.leftmargin = par_format.rightmargin = 0;
		}
		if (!strcasecmp(al, "justify")) par_format.align = AL_BLOCK;
		mem_free(al);
	}
}

void html_br(unsigned char *a)
{
	html_linebrk(a);
	if (was_br) ln_break(2, line_break_f, ff);
	was_br = 1;
}

void html_form(unsigned char *a)
{
	was_br = 1;
}

void html_p(unsigned char *a)
{
	if (par_format.leftmargin < margin) par_format.leftmargin = margin;
	if (par_format.rightmargin < margin) par_format.rightmargin = margin;
	/*par_format.align = AL_LEFT;*/
	html_linebrk(a);
}

void html_address(unsigned char *a)
{
	par_format.leftmargin += 1;
	par_format.align = AL_LEFT;
}

void html_blockquote(unsigned char *a)
{
	par_format.leftmargin += 2;
	par_format.align = AL_LEFT;
}

void html_h(int h, char *a)
{
	par_format.align = AL_LEFT;
	html_linebrk(a);
	switch (par_format.align) {
		case AL_LEFT:
			par_format.leftmargin = (h - 2) * 2;
			par_format.rightmargin = 0;
			break;
		case AL_RIGHT:
			par_format.leftmargin = 0;
			par_format.rightmargin = (h - 2) * 2;
			break;
		case AL_CENTER:
			par_format.leftmargin = par_format.rightmargin = 0;
			break;
		case AL_BLOCK:
			par_format.leftmargin = par_format.rightmargin = (h - 2) * 2;
			break;
	}
}

void html_h2(unsigned char *a) { html_h(2, a); }
void html_h3(unsigned char *a) { html_h(3, a); }
void html_h4(unsigned char *a) { html_h(4, a); }
void html_h5(unsigned char *a) { html_h(5, a); }
void html_h6(unsigned char *a) { html_h(6, a); }

void html_pre(unsigned char *a)
{
	par_format.align = AL_NO;
	par_format.leftmargin = par_format.leftmargin > 1;
	par_format.rightmargin = 0;
}

void html_hr(unsigned char *a)
{
	int i/* = par_format.width - 10*/;
	unsigned char r = 205;
	int q = get_num(a, "size");
	if (q >= 0 && q < 2) r = 196;
	html_stack_dup();
	par_format.align = AL_CENTER;
	if (format.link) mem_free(format.link), format.link = NULL;
	format.form = NULL;
	html_linebrk(a);
	if (par_format.align == AL_BLOCK) par_format.align = AL_CENTER;
	par_format.leftmargin = margin;
	par_format.rightmargin = margin;
	if ((i = get_width(a, "width", 1)) == -1) i = par_format.width - 2 * margin - 4;
	format.attr = AT_GRAPHICS;
	special_f(ff, SP_NOWRAP, 1);
	while (i-- > 0) put_chrs(&r, 1, put_chars_f, ff);
	special_f(ff, SP_NOWRAP, 0);
	ln_break(2, line_break_f, ff);
	kill_html_stack_item(&html_top);
}

void html_table(unsigned char *a)
{
	par_format.leftmargin = margin;
	par_format.rightmargin = margin;
	par_format.align = AL_LEFT;
	html_linebrk(a);
	format.attr = 0;
}

void html_tr(unsigned char *a)
{
	html_linebrk(a);
}

void html_th(unsigned char *a)
{
	/*html_linebrk(a);*/
	kill_until(1, "TD", "TH", "", "TR", "TABLE", NULL);
	format.attr |= AT_BOLD;
	put_chrs(" ", 1, put_chars_f, ff);
}

void html_td(unsigned char *a)
{
	/*html_linebrk(a);*/
	kill_until(1, "TD", "TH", "", "TR", "TABLE", NULL);
	format.attr &= ~AT_BOLD;
	put_chrs(" ", 1, put_chars_f, ff);
}

void html_base(unsigned char *a)
{
	char *al;
	if ((al = get_url_val(a, "href"))) {
		if (format.href_base) mem_free(format.href_base);
		format.href_base = join_urls(((struct html_element *)html_stack.prev)->attr.href_base, al);
		mem_free(al);
	}
	if ((al = get_target(a))) {
		if (format.target_base) mem_free(format.target_base);
		format.target_base = al;
	}
}

void html_ul(unsigned char *a)
{
	char *al;
	/*debug_stack();*/
	par_format.list_level++;
	par_format.list_number = 0;
	par_format.flags = P_STAR;
	if ((al = get_attr_val(a, "type"))) {
		if (!strcasecmp(al, "disc") || !strcasecmp(al, "circle")) par_format.flags = P_O;
		if (!strcasecmp(al, "square")) par_format.flags = P_PLUS;
		mem_free(al);
	}
	if ((par_format.leftmargin += 2 + (par_format.list_level > 1)) > par_format.width * 2 / 3 && !table_level)
		par_format.leftmargin = par_format.width * 2 / 3;
	par_format.align = AL_LEFT;
	html_top.dontkill = 1;
}

void html_ol(unsigned char *a)
{
	char *al;
	int st;
	par_format.list_level++;
	st = get_num(a, "start");
	if (st == -1) st = 1;
	par_format.list_number = st;
	par_format.flags = P_NUMBER;
	if ((al = get_attr_val(a, "type"))) {
		if (!strcmp(al, "1")) par_format.flags = P_NUMBER;
		if (!strcmp(al, "a")) par_format.flags = P_alpha;
		if (!strcmp(al, "A")) par_format.flags = P_ALPHA;
		if (!strcmp(al, "r")) par_format.flags = P_roman;
		if (!strcmp(al, "R")) par_format.flags = P_ROMAN;
		if (!strcmp(al, "i")) par_format.flags = P_roman;
		if (!strcmp(al, "I")) par_format.flags = P_ROMAN;
		mem_free(al);
	}
	if ((par_format.leftmargin += (par_format.list_level > 1)) > par_format.width * 2 / 3 && !table_level)
		par_format.leftmargin = par_format.width * 2 / 3;
	par_format.align = AL_LEFT;
	html_top.dontkill = 1;
}

void html_li(unsigned char *a)
{
	/*kill_until(0, "", "UL", "OL", NULL);*/
	if (!par_format.list_number) {
		char x[7] = "*&nbsp;";
		if ((par_format.flags & P_LISTMASK) == P_O) x[0] = 'o';
		if ((par_format.flags & P_LISTMASK) == P_PLUS) x[0] = '+';
		put_chrs(x, 7, put_chars_f, ff);
		par_format.leftmargin += 2;
		par_format.align = AL_LEFT;
		putsp = -1;
	} else {
		char c = 0;
		char n[32];
		int t = par_format.flags & P_LISTMASK;
		int s = get_num(a, "value");
		if (s != -1) par_format.list_number = s;
		if ((t != P_roman && t != P_ROMAN && par_format.list_number < 10) || t == P_alpha || t == P_ALPHA) put_chrs("&nbsp;", 6, put_chars_f, ff), c = 1;
		if (t == P_ALPHA || t == P_alpha) {
			n[0] = par_format.list_number ? (par_format.list_number - 1) % 26 + (t == P_ALPHA ? 'A' : 'a') : 0;
			n[1] = 0;
		} else if (t == P_ROMAN || t == P_roman) {
			roman(n, par_format.list_number);
			if (t == P_ROMAN) {
				char *x;
				for (x = n; *x; x++) *x = upcase(*x);
			}
		} else sprintf(n, "%d", par_format.list_number);
		put_chrs(n, strlen(n), put_chars_f, ff);
		put_chrs(".&nbsp;", 7, put_chars_f, ff);
		par_format.leftmargin += strlen(n) + c + 2;
		par_format.align = AL_LEFT;
		html_top.next->parattr.list_number = par_format.list_number + 1;
		par_format.list_number = 0;
		putsp = -1;
	}
	line_breax = 2;
}

void html_dl(unsigned char *a)
{
	par_format.flags &= ~P_COMPACT;
	if (has_attr(a, "compact")) par_format.flags |= P_COMPACT;
	if (par_format.list_level) par_format.leftmargin += 5;
	par_format.list_level++;
	par_format.list_number = 0;
	par_format.align = AL_LEFT;
	par_format.dd_margin = par_format.leftmargin;
	html_top.dontkill = 1;
	if (!(par_format.flags & P_COMPACT)) {
		ln_break(2, line_break_f, ff);
		html_top.linebreak = 2;
	}
}

void html_dt(unsigned char *a)
{
	kill_until(0, "", "DL", NULL);
	par_format.align = AL_LEFT;
	par_format.leftmargin = par_format.dd_margin;
	if (!(par_format.flags & P_COMPACT) && !has_attr(a, "compact"))
		ln_break(2, line_break_f, ff);
}

void html_dd(unsigned char *a)
{
	kill_until(0, "", "DL", NULL);
	if ((par_format.leftmargin = par_format.dd_margin + (table_level ? 3 : 8)) > par_format.width * 2 / 3 && !table_level)
		par_format.leftmargin = par_format.width * 2 / 3;
	par_format.align = AL_LEFT;
}

void get_html_form(unsigned char *a, struct form *form)
{
	unsigned char *al;
	unsigned char *ch;
	form->method = FM_GET;
	if ((al = get_attr_val(a, "method"))) {
		if (!strcasecmp(al, "post")) {
			char *ax;
			form->method = FM_POST;
			if ((ax = get_attr_val(a, "enctype"))) {
				if (!strcasecmp(ax, "multipart/form-data"))
					form->method = FM_POST_MP;
				mem_free(ax);
			}
		}
		mem_free(al);
	}
	if ((al = get_url_val(a, "action"))) {
		char *all = al;
		while (all[0] == ' ') all++;
		while (all[0] && all[strlen(all) - 1] == ' ') all[strlen(all) - 1] = 0;
		form->action = join_urls(format.href_base, all);
		mem_free(al);
	} else {
		if ((ch = strchr(form->action = stracpy(format.href_base), POST_CHAR))) *ch = 0;
		if (form->method == FM_GET && (ch = strchr(form->action, '?'))) *ch = 0;
	}
	if ((al = get_target(a))) {
		form->target = al;
	} else {
		form->target = stracpy(format.target_base);
	}
	form->num = a - startf;
}

void find_form_for_input(unsigned char *i, int when_unused)
{
	unsigned char *s, *ss, *name, *attr, *lf, *la;
	int namelen;
	if (form.action) mem_free(form.action);
	if (form.target) mem_free(form.target);
	memset(&form, 0, sizeof(form));
	if (!when_unused && !special_f(ff, SP_USED, NULL)) return;
	if (last_form_tag && last_input_tag && i <= last_input_tag && i > last_form_tag) {
		get_html_form(last_form_attr, &form);
		return;
	}
	if (last_form_tag && last_input_tag && i > last_input_tag) {
		if (parse_element(last_form_tag, i, &name, &namelen, &la, &s))
			internal("couldn't parse already parsed tag");
		lf = last_form_tag;
		s = last_input_tag;
	} else {
		lf = NULL, la = NULL;
		s = startf;
	}
	se:
	while (s < i && *s != '<') sp:s++;
	if (s >= i) goto end_parse;
	if (s + 2 <= eofff && (s[1] == '!' || s[1] == '?')) {
		s = skip_comment(s, i);
		goto se;
	}
	ss = s;
	if (parse_element(s, i, &name, &namelen, &attr, &s)) goto sp;
	if (namelen != 4 || casecmp(name, "FORM", 4)) goto se;
	lf = ss;
	la = attr;
	goto se;

	end_parse:
	if (lf) {
		last_form_tag = lf;
		last_form_attr = la;
		last_input_tag = i;
		get_html_form(la, &form);
	} else {
		memset(&form, 0, sizeof(struct form));
	}
}

void html_button(unsigned char *a)
{
	char *al;
	struct form_control *fc;
	find_form_for_input(a, 0);
	fc = mem_alloc(sizeof(struct form_control));
	memset(fc, 0, sizeof(struct form_control));
	if (!(al = get_attr_val(a, "type"))) {
		fc->type = FC_SUBMIT;
		goto xxx;
	}
	if (!strcasecmp(al, "submit")) fc->type = FC_SUBMIT;
	else if (!strcasecmp(al, "reset")) fc->type = FC_RESET;
	else if (!strcasecmp(al, "button")) {
		mem_free(al);
		put_chrs(" [&nbsp;", 8, put_chars_f, ff);
		if ((al = get_attr_val(a, "value"))) {
			put_chrs(al, strlen(al), put_chars_f, ff);
			mem_free(al);
		} else put_chrs("BUTTON", 6, put_chars_f, ff);
		put_chrs("&nbsp;] ", 8, put_chars_f, ff);
		mem_free(fc);
		return;
	} else {
		mem_free(al);
		mem_free(fc);
		return;
	}
	mem_free(al);
	xxx:
	fc->form_num = last_form_tag - startf;
	fc->ctrl_num = a - last_form_tag;
	fc->position = a - startf;
	fc->method = form.method;
	fc->action = stracpy(form.action);
	fc->name = get_attr_val(a, "name");
	fc->default_value = get_exact_attr_val(a, "value");
	fc->ro = has_attr(a, "disabled") ? 2 : has_attr(a, "readonly") ? 1 : 0;
	if (fc->type == FC_IMAGE) fc->alt = get_attr_val(a, "alt");
	if (fc->type == FC_SUBMIT && !fc->default_value) fc->default_value = stracpy("Submit");
	if (fc->type == FC_RESET && !fc->default_value) fc->default_value = stracpy("Reset");
	if (!fc->default_value) fc->default_value = stracpy("");
	special_f(ff, SP_CONTROL, fc);
	format.form = fc;
	format.attr |= AT_BOLD;
	/*put_chrs("[&nbsp;", 7, put_chars_f, ff);
	if (fc->default_value) put_chrs(fc->default_value, strlen(fc->default_value), put_chars_f, ff);
	put_chrs("&nbsp;]", 7, put_chars_f, ff);
	put_chrs(" ", 1, put_chars_f, ff);*/
}

void html_input(unsigned char *a)
{
	int i;
	unsigned char *al;
	struct form_control *fc;
	find_form_for_input(a, 0);
	fc = mem_alloc(sizeof(struct form_control));
	memset(fc, 0, sizeof(struct form_control));
	if (!(al = get_attr_val(a, "type"))) {
		fc->type = FC_TEXT;
		goto xxx;
	}
	if (!strcasecmp(al, "text")) fc->type = FC_TEXT;
	else if (!strcasecmp(al, "password")) fc->type = FC_PASSWORD;
	else if (!strcasecmp(al, "checkbox")) fc->type = FC_CHECKBOX;
	else if (!strcasecmp(al, "radio")) fc->type = FC_RADIO;
	else if (!strcasecmp(al, "submit")) fc->type = FC_SUBMIT;
	else if (!strcasecmp(al, "reset")) fc->type = FC_RESET;
	else if (!strcasecmp(al, "file")) fc->type = FC_FILE;
	else if (!strcasecmp(al, "hidden")) fc->type = FC_HIDDEN;
	else if (!strcasecmp(al, "image")) fc->type = FC_IMAGE;
	else if (!strcasecmp(al, "button")) {
		mem_free(al);
		put_chrs(" [&nbsp;", 8, put_chars_f, ff);
		if ((al = get_attr_val(a, "value"))) {
			put_chrs(al, strlen(al), put_chars_f, ff);
			mem_free(al);
		} else put_chrs("BUTTON", 6, put_chars_f, ff);
		put_chrs("&nbsp;] ", 8, put_chars_f, ff);
		mem_free(fc);
		return;
	} else fc->type = FC_TEXT;
	mem_free(al);
	xxx:
	fc->form_num = last_form_tag - startf;
	fc->ctrl_num = a - last_form_tag;
	fc->position = a - startf;
	fc->method = form.method;
	fc->action = stracpy(form.action);
	fc->target = stracpy(form.target);
	fc->name = get_attr_val(a, "name");
	if (fc->type == FC_TEXT || fc->type == FC_PASSWORD) fc->default_value = get_attr_val(a, "value");
	else if (fc->type != FC_FILE) fc->default_value = get_exact_attr_val(a, "value");
	if (fc->type == FC_CHECKBOX && !fc->default_value) fc->default_value = stracpy("on");
	if ((fc->size = get_num(a, "size")) == -1) fc->size = HTML_DEFAULT_INPUT_SIZE;
	fc->size++;
	if (fc->size > d_opt->xw) fc->size = d_opt->xw;
	if ((fc->maxlength = get_num(a, "maxlength")) == -1) fc->maxlength = MAXINT / 4;
	if (fc->type == FC_CHECKBOX || fc->type == FC_RADIO) fc->default_state = has_attr(a, "checked");
	fc->ro = has_attr(a, "disabled") ? 2 : has_attr(a, "readonly") ? 1 : 0;
	if (fc->type == FC_IMAGE) {
		fc->alt = get_attr_val(a, "alt");
		if (!fc->alt) fc->alt = get_attr_val(a, "title");
		if (!fc->alt) fc->alt = get_attr_val(a, "name");
	}
	if (fc->type == FC_SUBMIT && !fc->default_value) fc->default_value = stracpy("Submit");
	if (fc->type == FC_RESET && !fc->default_value) fc->default_value = stracpy("Reset");
	if (!fc->default_value) fc->default_value = stracpy("");
	if (fc->type == FC_HIDDEN) goto hid;
	put_chrs(" ", 1, put_chars_f, ff);
	html_stack_dup();
	format.form = fc;
	switch (fc->type) {
		case FC_TEXT:
		case FC_PASSWORD:
		case FC_FILE:
			format.attr |= AT_BOLD;
			for (i = 0; i < fc->size; i++) put_chrs("_", 1, put_chars_f, ff);
			break;
		case FC_CHECKBOX:
		case FC_RADIO:
			format.attr |= AT_BOLD;
			put_chrs("[&nbsp;]", 8, put_chars_f, ff);
			break;
		case FC_IMAGE:
			if (format.image) mem_free(format.image), format.image = NULL;
			if ((al = get_url_val(a, "src")) || (al = get_url_val(a, "dynsrc"))) {
				format.image = join_urls(format.href_base, al);
				mem_free(al);
			}
			format.attr |= AT_BOLD;
			put_chrs("[&nbsp;", 7, put_chars_f, ff);
			if (fc->alt) put_chrs(fc->alt, strlen(fc->alt), put_chars_f, ff);
			else put_chrs("Submit", 6, put_chars_f, ff);
			put_chrs("&nbsp;]", 7, put_chars_f, ff);
			break;
		case FC_SUBMIT:
		case FC_RESET:
			format.attr |= AT_BOLD;
			put_chrs("[&nbsp;", 7, put_chars_f, ff);
			if (fc->default_value) put_chrs(fc->default_value, strlen(fc->default_value), put_chars_f, ff);
			put_chrs("&nbsp;]", 7, put_chars_f, ff);
			break;
		default:
			internal("bad control type");
	}
	kill_html_stack_item(&html_top);
	put_chrs(" ", 1, put_chars_f, ff);
			
	hid:
	special_f(ff, SP_CONTROL, fc);
}

void html_select(unsigned char *a)
{
	char *al;
	if (!(al = get_attr_val(a, "name"))) return;
	html_top.dontkill = 1;
	if (format.select) mem_free(format.select);
	format.select = al;
	format.select_disabled = 2 * has_attr(a, "disabled");
}

void html_option(unsigned char *a)
{
	struct form_control *fc;
	unsigned char *val;
	find_form_for_input(a, 0);
	if (!format.select) return;
	fc = mem_alloc(sizeof(struct form_control));
	memset(fc, 0, sizeof(struct form_control));
	if (!(val = get_exact_attr_val(a, "value"))) {
		unsigned char *p, *r;
		unsigned char *name;
		int namelen;
		int l = 0;
		for (p = a - 1; *p != '<'; p--) ;
		val = init_str();
		if (parse_element(p, eoff, NULL, NULL, NULL, &p)) {
			internal("parse element failed");
			goto x;
		}
		rrrr:
		while (p < eoff && WHITECHAR(*p)) p++;
		while (p < eoff && !WHITECHAR(*p) && *p != '<') {
			pppp:
			add_chr_to_str(&val, &l, *p), p++;
		}
		r = p;
		while (r < eoff && WHITECHAR(*r)) r++;
		if (r >= eoff) goto x;
		if (r - 2 <= eoff && (r[1] == '!' || r[1] == '?')) {
			p = skip_comment(r, eoff);
			goto rrrr;
		}
		if (parse_element(r, eoff, &name, &namelen, NULL, &p)) goto pppp;
		if (!((namelen == 6 && !casecmp(name, "OPTION", 6)) ||
		    (namelen == 7 && !casecmp(name, "/OPTION", 7)) ||
		    (namelen == 6 && !casecmp(name, "SELECT", 6)) ||
		    (namelen == 7 && !casecmp(name, "/SELECT", 7)) ||
		    (namelen == 8 && !casecmp(name, "OPTGROUP", 8)) ||
		    (namelen == 9 && !casecmp(name, "/OPTGROUP", 9)))) goto rrrr;
	}
	x:
	fc->form_num = last_form_tag - startf;
	fc->ctrl_num = a - last_form_tag;
	fc->position = a - startf;
	fc->method = form.method;
	fc->action = stracpy(form.action);
	fc->type = FC_CHECKBOX;
	fc->name = stracpy(format.select);
	fc->default_value = val;
	fc->default_state = has_attr(a, "selected");
	fc->ro = format.select_disabled;
	if (has_attr(a, "disabled")) fc->ro = 2;
	put_chrs(" ", 1, put_chars_f, ff);
	html_stack_dup();
	format.form = fc;
	format.attr |= AT_BOLD;
	put_chrs("[ ]", 3, put_chars_f, ff);
	kill_html_stack_item(&html_top);
	put_chrs(" ", 1, put_chars_f, ff);
	special_f(ff, SP_CONTROL, fc);
}

void clr_spaces(unsigned char *name)
{
	unsigned char *n1, *n2;
	for (n1 = name; *n1; n1++)
		if (WHITECHAR(*n1) || *n1 == 1) *n1 = ' ';
	/*for (nm = name; *nm; nm++)
		while (nm[0] == ' ' && (nm == name || nm[1] == ' ' || !nm[1]))
			memmove(nm, nm + 1, strlen(nm));*/
	if (!strchr(name, ' ')) return;
	for (n1 = name, n2 = name; *n1; n1++)
		if (!(n1[0] == ' ' && (n2 == name || n1[1] == ' ' || !n1[1])))
			*n2++ = *n1;
	*n2 = 0;
}

int menu_stack_size;
struct menu_item **menu_stack;

void new_menu_item(unsigned char *name, int data, int fullname)
	/* name == NULL - up;	data == -1 - down */
{
	struct menu_item *top, *item, *nmenu = NULL; /* no uninitialized warnings */
	if (name) {
		clr_spaces(name);
		if (!name[0]) mem_free(name), name = stracpy(" ");
		if (name[0] == 1) name[0] = ' ';
	}
	if (name && data == -1) {
		nmenu = mem_alloc(sizeof(struct menu_item));
		memset(nmenu, 0, sizeof(struct menu_item));
		/*nmenu->text = "";*/
	}
	if (menu_stack_size && name) {
		top = item = menu_stack[menu_stack_size - 1];
		while (item->text) item++;
		if ((size_t)((char *)(item + 2) - (char *)top) > MAXINT) overalloc();
		top = mem_realloc(top, (char *)(item + 2) - (char *)top);
		item = item - menu_stack[menu_stack_size - 1] + top;
		menu_stack[menu_stack_size - 1] = top;
		if (menu_stack_size >= 2) {
			struct menu_item *below = menu_stack[menu_stack_size - 2];
			while (below->text) below++;
			below[-1].data = top;
		}
		item->text = name;
		item->rtext = data == -1 ? ">" : "";
		item->hotkey = fullname ? "\000\001" : "\000\000"; /* dirty */
		item->func = data == -1 ? MENU_FUNC do_select_submenu : MENU_FUNC selected_item;
		item->data = data == -1 ? nmenu : (void *)data;
		item->in_m = data == -1 ? 1 : 0;
		item->free_i = 0;
		item++;
		memset(item, 0, sizeof(struct menu_item));
		/*item->text = "";*/
	} else if (name) mem_free(name);
	if (name && data == -1) {
		if ((unsigned)menu_stack_size > MAXINT / sizeof(struct menu_item *) - 1) overalloc();
		menu_stack = mem_realloc(menu_stack, (menu_stack_size + 1) * sizeof(struct menu_item *));
		menu_stack[menu_stack_size++] = nmenu;
	}
	if (!name) menu_stack_size--;
}

void init_menu()
{
	menu_stack_size = 0;
	menu_stack = DUMMY;
	new_menu_item(stracpy(""), -1, 0);
}

void free_menu(struct menu_item *m) /* Grrr. Recursion */
{
	struct menu_item *mm;
	for (mm = m; mm->text; mm++) {
		mem_free(mm->text);
		if (mm->func == MENU_FUNC do_select_submenu) free_menu(mm->data);
	}
	mem_free(m);
}

struct menu_item *detach_menu()
{
	struct menu_item *i = NULL;
	if (menu_stack_size) i = menu_stack[0];
	if (menu_stack) mem_free(menu_stack);
	return i;
}

void destroy_menu()
{
	if (menu_stack && menu_stack != DUMMY) free_menu(menu_stack[0]);
	detach_menu();
}

void menu_labels(struct menu_item *m, unsigned char *base, unsigned char **lbls)
{
	unsigned char *bs;
	for (; m->text; m++) {
		if (m->func == MENU_FUNC do_select_submenu) {
			if ((bs = stracpy(base))) {
				add_to_strn(&bs, m->text);
				add_to_strn(&bs, " ");
				menu_labels(m->data, bs, lbls);
				mem_free(bs);
			}
		} else {
			if ((bs = stracpy(m->hotkey[1] ? (unsigned char *)"" : base))) add_to_strn(&bs, m->text);
			lbls[(int)m->data] = bs;
		}
	}
}

int menu_contains(struct menu_item *m, int f)
{
	if (m->func != MENU_FUNC do_select_submenu) return (int)m->data == f;
	for (m = m->data; m->text; m++) if (menu_contains(m, f)) return 1;
	return 0;
}

void do_select_submenu(struct terminal *term, struct menu_item *menu, struct session *ses)
{
	struct menu_item *m;
	int def = get_current_state(ses);
	int sel = 0;
	if (def < 0) def = 0;
	for (m = menu; m->text; m++, sel++) if (menu_contains(m, def)) goto f;
	sel = 0;
	f:
	do_menu_selected(term, menu, ses, sel);
}

int do_html_select(unsigned char *attr, unsigned char *html, unsigned char *eof, unsigned char **end, void *f)
{
	struct form_control *fc;
	unsigned char *t_name, *t_attr, *en;
	int t_namelen;
	unsigned char *lbl;
	int lbl_l;
	unsigned char *vlbl;
	int vlbl_l;
	int nnmi = 0;
	struct conv_table *ct = special_f(f, SP_TABLE, NULL);
	unsigned char **val, **lbls;
	int order, preselect, group;
	int i, mw;
	if (has_attr(attr, "multiple")) return 1;
	find_form_for_input(attr, 0);
	lbl = NULL;
	lbl_l = 0;
	vlbl = NULL;
	vlbl_l = 0;
	val = DUMMY;
	order = 0, group = 0, preselect = -1;
	init_menu();
        se:
        en = html;
        see:
        html = en;
	while (html < eof && *html != '<') html++;
	if (html >= eof) {
		int i;
		abort:
		*end = html;
		if (lbl) mem_free(lbl);
		if (vlbl) mem_free(vlbl);
		for (i = 0; i < order; i++) if (val[i]) mem_free(val[i]);
		mem_free(val);
		destroy_menu();
		*end = en;
		return 0;
	}
	if (lbl) {
		unsigned char *q, *s = en;
		int l = html - en;
		while (l && WHITECHAR(s[0])) s++, l--;
		while (l && WHITECHAR(s[l-1])) l--;
		q = convert_string(ct, s, l);
		if (q) add_to_str(&lbl, &lbl_l, q), mem_free(q);
		add_bytes_to_str(&vlbl, &vlbl_l, s, l);
	}
	if (html + 2 <= eof && (html[1] == '!' || html[1] == '?')) {
		html = skip_comment(html, eof);
		goto se;
	}
	if (parse_element(html, eof, &t_name, &t_namelen, &t_attr, &en)) {
		html++;
		goto se;
	}
	if (t_namelen == 7 && !casecmp(t_name, "/SELECT", 7)) {
		if (lbl) {
			if (!val[order - 1]) val[order - 1] = stracpy(vlbl);
			if (!nnmi) new_menu_item(lbl, order - 1, 1), lbl = NULL;
			else mem_free(lbl), lbl = NULL;
			mem_free(vlbl), vlbl = NULL;
		}
		goto end_parse;
	}
	if (t_namelen == 7 && !casecmp(t_name, "/OPTION", 7)) {
		if (lbl) {
			if (!val[order - 1]) val[order - 1] = stracpy(vlbl);
			if (!nnmi) new_menu_item(lbl, order - 1, 1), lbl = NULL;
			else mem_free(lbl), lbl = NULL;
			mem_free(vlbl), vlbl = NULL;
		}
		goto see;
	}
	if (t_namelen == 6 && !casecmp(t_name, "OPTION", 6)) {
		unsigned char *v, *vx;
		if (lbl) {
			if (!val[order - 1]) val[order - 1] = stracpy(vlbl);
			if (!nnmi) new_menu_item(lbl, order - 1, 1), lbl = NULL;
			else mem_free(lbl), lbl = NULL;
			mem_free(vlbl), vlbl = NULL;
		}
		if (has_attr(t_attr, "disabled")) goto see;
		if (preselect == -1 && has_attr(t_attr, "selected")) preselect = order;
		v = get_exact_attr_val(t_attr, "value");
		if (!(order & (ALLOC_GR - 1))) {
			if ((unsigned)order > MAXINT / sizeof(unsigned char *) - ALLOC_GR) overalloc();
			if ((unsigned)order > MAXINT / sizeof(char *) - ALLOC_GR) overalloc();
			val = mem_realloc(val, (order + ALLOC_GR) * sizeof(unsigned char *));
		}
		val[order++] = v;
		if ((vx = get_attr_val(t_attr, "label"))) {
			new_menu_item(convert_string(ct, vx, strlen(vx)), order - 1, 0);
			mem_free(vx);
		}
		if (!v || !vx) {
			lbl = init_str(), lbl_l = 0;
			vlbl = init_str(), vlbl_l = 0;
			nnmi = !!vx;
		}
		goto see;
	}
	if ((t_namelen == 8 && !casecmp(t_name, "OPTGROUP", 8)) || (t_namelen == 9 && !casecmp(t_name, "/OPTGROUP", 9))) {
		if (lbl) {
			if (!val[order - 1]) val[order - 1] = stracpy(vlbl);
			if (!nnmi) new_menu_item(lbl, order - 1, 1), lbl = NULL;
			else mem_free(lbl), lbl = NULL;
			mem_free(vlbl), vlbl = NULL;
		}
		if (group) new_menu_item(NULL, -1, 0), group = 0;
	}
	if (t_namelen == 8 && !casecmp(t_name, "OPTGROUP", 8)) {
		char *la;
		if (!(la = get_attr_val(t_attr, "label"))) la = stracpy("");
		new_menu_item(convert_string(ct, la, strlen(la)), -1, 0);
		mem_free(la);
		group = 1;
	}
	goto see;

	end_parse:
	*end = en;
	if (!order) goto abort;
	fc = mem_alloc(sizeof(struct form_control));
	memset(fc, 0, sizeof(struct form_control));
	lbls = mem_alloc(order * sizeof(char *));
	memset(lbls, 0, order * sizeof(char *));
	fc->form_num = last_form_tag - startf;
	fc->ctrl_num = attr - last_form_tag;
	fc->position = attr - startf;
	fc->method = form.method;
	fc->action = stracpy(form.action);
	fc->name = get_attr_val(attr, "name");
	fc->type = FC_SELECT;
	fc->default_state = preselect < 0 ? 0 : preselect;
	fc->default_value = order ? stracpy(val[fc->default_state]) : stracpy("");
	fc->ro = has_attr(attr, "disabled") ? 2 : has_attr(attr, "readonly") ? 1 : 0;
	fc->nvalues = order;
	fc->values = val;
	fc->menu = detach_menu();
	fc->labels = lbls;
	menu_labels(fc->menu, "", lbls);
	put_chrs("[", 1, put_chars_f, f);
	html_stack_dup();
	format.form = fc;
	format.attr |= AT_BOLD;
	mw = 0;
	for (i = 0; i < order; i++) if (lbls[i] && strlen(lbls[i]) > (size_t)mw) mw = strlen(lbls[i]);
	for (i = 0; i < mw; i++) put_chrs("_", 1, put_chars_f, f);
	kill_html_stack_item(&html_top);
	put_chrs("]", 1, put_chars_f, f);
	special_f(ff, SP_CONTROL, fc);
	return 0;
}

void html_textarea(unsigned char *a)
{
	internal("This should be never called");
}

void do_html_textarea(unsigned char *attr, unsigned char *html, unsigned char *eof, unsigned char **end, void *f)
{
	struct form_control *fc;
	unsigned char *p, *t_name, *w;
	int t_namelen;
	int cols, rows;
	int i;
	find_form_for_input(attr, 1);
	while (html < eof && (*html == '\n' || *html == '\r')) html++;
	p = html;
	while (p < eof && *p != '<') {
		pp:
		p++;
	}
	if (p >= eof) {
		*end = eof;
		return;
	}
	if (parse_element(p, eof, &t_name, &t_namelen, NULL, end)) goto pp;
	if (t_namelen != 9 || casecmp(t_name, "/TEXTAREA", 9)) goto pp;
	if (!last_form_tag) return;
	fc = mem_alloc(sizeof(struct form_control));
	memset(fc, 0, sizeof(struct form_control));
	fc->form_num = last_form_tag - startf;
	fc->ctrl_num = attr - last_form_tag;
	fc->position = attr - startf;
	fc->method = form.method;
	fc->action = stracpy(form.action);
	fc->name = get_attr_val(attr, "name");
	fc->type = FC_TEXTAREA;;
	fc->ro = has_attr(attr, "disabled") ? 2 : has_attr(attr, "readonly") ? 1 : 0;
	fc->default_value = memacpy(html, p - html);
	if ((cols = get_num(attr, "cols")) <= 5) cols = HTML_DEFAULT_TEXTAREA_WIDTH;
	cols++;
	if ((rows = get_num(attr, "rows")) <= 0) rows = HTML_DEFAULT_TEXTAREA_HEIGHT;
	if (cols > d_opt->xw) cols = d_opt->xw;
	if (rows > d_opt->yw) rows = d_opt->yw;
	fc->cols = cols;
	fc->rows = rows;
	fc->wrap = 1;
	if ((w = get_attr_val(attr, "wrap"))) {
		if (!strcasecmp(w, "hard") || !strcasecmp(w, "physical")) fc->wrap = 2;
		else if (!strcasecmp(w, "off")) fc->wrap = 0;
		mem_free(w);
	}
	if ((fc->maxlength = get_num(attr, "maxlength")) == -1) fc->maxlength = MAXINT / 4;
	if (rows > 1) ln_break(1, line_break_f, f);
	else put_chrs(" ", 1, put_chars_f, f);
	html_stack_dup();
	format.form = fc;
	format.attr |= AT_BOLD;
	for (i = 0; i < rows; i++) {
		int j;
		for (j = 0; j < cols; j++) put_chrs("_", 1, put_chars_f, f);
		if (i < rows - 1) ln_break(1, line_break_f, f);
	}
	kill_html_stack_item(&html_top);
	if (rows > 1) ln_break(1, line_break_f, f);
	else put_chrs(" ", 1, put_chars_f, f);
	special_f(f, SP_CONTROL, fc);
}

void html_iframe(unsigned char *a)
{
	unsigned char *name, *url;
	if (!(url = get_url_val(a, "src"))) return;
	if (!(name = get_attr_val(a, "name"))) name = stracpy("");
	if (*name) put_link_line("IFrame: ", name, url, d_opt->framename);
	else put_link_line("", "IFrame", url, d_opt->framename);
	mem_free(name);
	mem_free(url);
}

void html_noframes(unsigned char *a)
{
	if (d_opt->frames) html_skip(a);
}

void html_frame(unsigned char *a)
{
	unsigned char *name, *u2, *url;
	if (!(u2 = get_url_val(a, "src"))) {
		url = stracpy("");
	} else {
		url = join_urls(format.href_base, u2);
		mem_free(u2);
	}
	if (!url) return;
	name = get_attr_val (a, "name");
	if (!name)
		name = stracpy(url);
	else if (!name[0]) { /* When name doesn't have a value */
		mem_free(name);
		name = stracpy(url);
	}
	if (!d_opt->frames || !html_top.frameset) put_link_line("Frame: ", name, url, "");
	else {
		struct frame_param fp;
		fp.name = name;
		fp.url = url;
		fp.parent = html_top.frameset;
		if (special_f(ff, SP_USED, NULL)) special_f(ff, SP_FRAME, &fp);
	}
	mem_free(name);
	mem_free(url);
}

void parse_frame_widths(unsigned char *a, int ww, int www, int **op, int *olp)
{
	unsigned char *aa;
	int q, qq, i, d, nn;
	unsigned long n;
	int *oo, *o;
	int ol;
	ol = 0;
	o = DUMMY;
	new_ch:
	while (WHITECHAR(*a)) a++;
	n = strtoul(a, (char **)(void *)&a, 10);
	if (n > 10000) n = 10000;
	q = n;
	if (*a == '%') q = q * ww / 100;
	else if (*a != '*') q = (q + (www - 1) / 2) / (www ? www : 1);
	else if (!(q = -q)) q = -1;
	if ((unsigned)ol > MAXINT / sizeof(int) - 1) overalloc();
	o = mem_realloc(o, (ol + 1) * sizeof(int));
	o[ol++] = q;
	if ((aa = strchr(a, ','))) {
		a = aa + 1;
		goto new_ch;
	}
	*op = o;
	*olp = ol;
	q = 2 * ol - 1;
	for (i = 0; i < ol; i++) if (o[i] > 0) q += o[i] - 1;
	if (q >= ww) {
		distribute:
		for (i = 0; i < ol; i++) if (o[i] < 1) o[i] = 1;
		q -= ww;
		d = 0;
		for (i = 0; i < ol; i++) d += o[i];
		qq = q;
		for (i = 0; i < ol; i++) {
			q -= o[i] - o[i] * (d - qq) / (d ? d : 1);
			do_not_optimize_here(&d);
				/* SIGH! gcc 2.7.2.* has an optimizer bug! */
			o[i] = o[i] * (d - qq) / (d ? d : 1);
		}
		while (q) {
			nn = 0;
			for (i = 0; i < ol; i++) {
				if (q < 0) o[i]++, q++, nn = 1;
				if (q > 0 && o[i] > 1) o[i]--, q--, nn = 1;
				if (!q) break;
			}
			if (!nn) break;
		}
	} else {
		int nn = 0;
		for (i = 0; i < ol; i++) if (o[i] < 0) nn = 1;
		if (!nn) goto distribute;
		if ((unsigned)ol > MAXINT / sizeof(int)) overalloc();
		oo = mem_alloc(ol * sizeof(int));
		memcpy(oo, o, ol * sizeof(int));
		for (i = 0; i < ol; i++) if (o[i] < 1) o[i] = 1;
		q = ww - q;
		d = 0;
		for (i = 0; i < ol; i++) if (oo[i] < 0) d += -oo[i];
		qq = q;
		for (i = 0; i < ol; i++) if (oo[i] < 0) {
			o[i] += (-oo[i] * qq / (d ? d : 1));
			q -= (-oo[i] * qq / (d ? d : 1));
		}
		if (q < 0) {
			q = 0;
			/*internal("parse_frame_widths: q < 0"); may happen when page contains too big values */
		}
		for (i = 0; i < ol; i++) if (oo[i] < 0) {
			if (q) o[i]++, q--;
		}
		if (q > 0) {
			q = 0;
			/*internal("parse_frame_widths: q > 0"); may happen when page contains too big values */
		}
		mem_free(oo);
	}
	for (i = 0; i < ol; i++) if (!o[i]) {
		int j;
		int m = 0;
		int mj = 0;
		for (j = 0; j < ol; j++) if (o[j] > m) m = o[j], mj = j;
		if (m) o[i] = 1, o[mj]--;
	}
}

void html_frameset(unsigned char *a)
{
	int x, y;
	struct frameset_param fp;
	unsigned char *c, *d;
	if (!d_opt->frames || !special_f(ff, SP_USED, NULL)) return;
	if (!(c = get_attr_val(a, "cols"))) c = stracpy("100%");
	if (!(d = get_attr_val(a, "rows"))) d = stracpy("100%");
	if (!html_top.frameset) {
		x = d_opt->xw;
		y = d_opt->yw;
	} else {
		struct frameset_desc *f = html_top.frameset;
		if (f->yp >= f->y) goto free_cd;
		x = f->f[f->xp + f->yp * f->x].xw;
		y = f->f[f->xp + f->yp * f->x].yw;
	}
	parse_frame_widths(c, x, HTML_FRAME_CHAR_WIDTH, &fp.xw, &fp.x);
	parse_frame_widths(d, y, HTML_FRAME_CHAR_HEIGHT, &fp.yw, &fp.y);
	fp.parent = html_top.frameset;
	if (fp.x && fp.y) html_top.frameset = special_f(ff, SP_FRAMESET, &fp);
	mem_free(fp.xw);
	mem_free(fp.yw);
	free_cd:
	mem_free(c);
	mem_free(d);
}

/*void html_frameset(unsigned char *a)
{
	int w;
	int horiz = 0;
	struct frameset_param *fp;
	unsigned char *c, *d;
	if (!d_opt->frames || !special_f(ff, SP_USED, NULL)) return;
	if (!(c = get_attr_val(a, "cols"))) {
		horiz = 1;
		if (!(c = get_attr_val(a, "rows"))) return;
	}
	fp = mem_alloc(sizeof(struct frameset_param));
	fp->n = 0;
	fp->horiz = horiz;
	par_format.leftmargin = par_format.rightmargin = 0;
	d = c;
	while (1) {
		while (WHITECHAR(*d)) d++;
		if (!*d) break;
		if (*d == ',') {
			d++;
			continue;
		}
		if ((w = parse_width(d, 1)) != -1) {
			if ((unsigned)fp->n > (MAXINT - sizeof(struct frameset_param)) / sizeof(int) - 1) overalloc();
			fp = mem_realloc(fp, sizeof(struct frameset_param) + (fp->n + 1) * sizeof(int));
			fp->width[fp->n++] = w;
		}
		if (!(d = strchr(d, ','))) break;
		d++;
	}
	fp->parent = html_top.frameset;
	if (fp->n) html_top.frameset = special_f(ff, SP_FRAMESET, fp);
	mem_free(fp);
	f:
	mem_free(c);
}*/

void html_link(unsigned char *a)
{
	unsigned char *name, *url, *title;
	if ((name = get_attr_val(a, "type"))) {
		if (strcasecmp(name, "text/html")) {
			mem_free(name);
			return;
		}
		mem_free(name);
	}
	if (!(url = get_url_val(a, "href"))) return;
	if (!(name = get_attr_val(a, "rel")))
		if (!(name = get_attr_val(a, "rev")))
			if (!(name = get_attr_val(a, "ref")))
				name = stracpy(url);
	if (!strcasecmp(name, "stylesheet") ||
	    !strcasecmp(name, "alternate stylesheet") ||
	    !strcasecmp(name, "made") ||
	    !strcasecmp(name, "icon") ||
	    !strcasecmp(name, "shortcut icon") ||
	    !strcasecmp(name, "apple-touch-icon") ||
	    !strcasecmp(name, "meta") ||
	    !strcasecmp(name, "pingback") ||
	    !strcasecmp(name, "File-List") ||
	    !casecmp(name, "schema", 6)) goto skip;
	if ((title = get_attr_val(a, "title"))) {
		add_to_strn(&name, ": ");
		add_to_strn(&name, title);
		mem_free(title);
	}
	put_link_line("Link: ", name, url, format.target_base);
	skip:
	mem_free(name);
	mem_free(url);
}

struct element_info {
	char *name;
	void (*func)(unsigned char *);
	int linebreak;
	int nopair;
};

struct element_info elements[] = {
	{"SPAN",	html_span,	0, 0},
	{"B",		html_bold,	0, 0},
	{"STRONG",	html_bold,	0, 0},
	{"DFN",		html_bold,	0, 0},
	{"I",		html_italic,	0, 0},
	{"Q",		html_italic,	0, 0},
	{"CITE",	html_italic,	0, 0},
	{"EM",		html_italic,	0, 0},
	{"ABBR",	html_italic,	0, 0},
	{"U",		html_underline,	0, 0},
	{"S",		html_underline,	0, 0},
	{"STRIKE",	html_underline,	0, 0},
	{"FIXED",	html_fixed,	0, 0},
	{"CODE",	html_fixed,	0, 0},
	{"FONT",	html_font,	0, 0},
	{"A",		html_a,		0, 2},
	{"IMG",		html_img,	0, 1},

	{"BASE",	html_base,	0, 1},
	{"BASEFONT",	html_font,	0, 1},

	{"BODY",	html_body,	0, 0},

/*	{"HEAD",	html_skip,	0, 0},*/
	{"TITLE",	html_title,	0, 0},
	{"SCRIPT",	html_skip,	0, 0},
	{"STYLE",	html_skip,	0, 0},

	{"BR",		html_br,	1, 1},
	{"DIV",		html_linebrk,	1, 0},
	{"CENTER",	html_center,	1, 0},
	{"CAPTION",	html_center,	1, 0},
	{"P",		html_p,		2, 2},
	{"HR",		html_hr,	2, 1},
	{"H1",		html_center,	2, 2},
	{"H2",		html_h2,	2, 2},
	{"H3",		html_h3,	2, 2},
	{"H4",		html_h4,	2, 2},
	{"H5",		html_h5,	2, 2},
	{"H6",		html_h6,	2, 2},
	{"BLOCKQUOTE",	html_blockquote,2, 0},
	{"ADDRESS",	html_address,	2, 0},
	{"PRE",		html_pre,	2, 0},
	{"LISTING",	html_pre,	2, 0},

	{"UL",		html_ul,	1, 0},
	{"DIR",		html_ul,	1, 0},
	{"MENU",	html_ul,	1, 0},
	{"OL",		html_ol,	1, 0},
	{"LI",		html_li,	1, 3},
	{"DL",		html_dl,	1, 0},
	{"DT",		html_dt,	1, 1},
	{"DD",		html_dd,	1, 1},

	{"TABLE",	html_table,	2, 0},
	{"TR",		html_tr,	1, 0},
	{"TD",		html_td,	0, 0},
	{"TH",		html_th,	0, 0},

	{"FORM",	html_form,	1, 0},
	{"INPUT",	html_input,	0, 1},
	{"TEXTAREA",	html_textarea,	0, 1},
	{"SELECT",	html_select,	0, 0},
	{"OPTION",	html_option,	1, 1},
	{"BUTTON",	html_button,	0, 0},

	{"LINK",	html_link,	1, 1},
	{"IFRAME",	html_iframe,	1, 1},
	{"FRAME",	html_frame,	1, 1},
	{"FRAMESET",	html_frameset,	1, 0},
	{"NOFRAMES",	html_noframes,	0, 0},

	{NULL,		NULL, 0, 0},
};

unsigned char *skip_comment(unsigned char *html, unsigned char *eof)
{
	int comm = html + 4 <= eof && html[2] == '-' && html[3] == '-';
	html += comm ? 4 : 2;
	while (html < eof) {
		if (!comm && html[0] == '>') return html + 1;
		if (comm && html + 2 <= eof && html[0] == '-' && html[1] == '-') {
			html += 2;
			while (html < eof && *html == '-') html++;
			while (html < eof && WHITECHAR(*html)) html++;
			if (html >= eof) return eof;
			if (*html == '>') return html + 1;
			continue;
		}
		html++;
	}
	return eof;
}

void process_head(unsigned char *head)
{
	unsigned char *r, *p;
	if ((r = parse_http_header(head, "Refresh", NULL))) {
		if ((p = parse_header_param(r, "URL")) || (p = parse_header_param(r, ""))) {
			put_link_line("Refresh: ", p, p, d_opt->framename);
			mem_free(p);
		}
		mem_free(r);
	}
}

int qd(unsigned char *html, unsigned char *eof, int *len)
{
	int l;
	*len = 1;
	if (html >= eof) {
		internal("qd: out of data, html == %p, eof == %p", html, eof);
		return -1;
	}
	if (html[0] != '&' || d_opt->plain) return html[0];
	if (html + 1 >= eof) return -1;
	if (html[1] != '#') return -1;
	for (l = 2; l < 10 && html + l < eof; l++) if (html[l] == ';') {
		int n = get_entity_number(html + 2, l - 2);
		if (n >= 0) {
			*len = l + 1;
			return n;
		}
		break;
	}
	return -1;
}

void parse_html(unsigned char *html, unsigned char *eof, int (*put_chars)(void *, unsigned char *, int), void (*line_break)(void *), void *(*special)(void *, int, ...), void *f, unsigned char *head)
{
	/*unsigned char *start = html;*/
	unsigned char *lt;
	putsp = -1;
	line_breax = table_level ? 2 : 1;
	pos = 0;
	was_br = 0;
	put_chars_f = put_chars;
	line_break_f = line_break;
	special_f = special;
	ff = f;
	eoff = eof;
	if (head) process_head(head);
	set_lt:
	put_chars_f = put_chars;
	line_break_f = line_break;
	special_f = special;
	ff = f;
	eoff = eof;
	lt = html;
	while (html < eof) {
		unsigned char *name, *attr, *end;
		int namelen;
		struct element_info *ei;
		int inv;
		if (WHITECHAR(*html) && par_format.align != AL_NO) {
			unsigned char *h = html;
			/*if (putsp == -1) {
				while (html < eof && WHITECHAR(*html)) html++;
				goto set_lt;
			}
			putsp = 0;*/
			while (h < eof && WHITECHAR(*h)) h++;
			if (h + 1 < eof && h[0] == '<' && h[1] == '/') {
				if (!parse_element(h, eof, &name, &namelen, &attr, &end)) {
					put_chrs(lt, html - lt, put_chars, f);
					lt = html = h;
					if (!html_top.invisible) putsp = 1;
					goto element;
				}
			}
			html++;
			if (!(pos + (html-lt-1))) goto skip_w; /* ??? */
			if (*(html - 1) == ' ') {
				if (html < eof && !WHITECHAR(*html)) continue;	/* BIG performance win; not sure if it doesn't cause any bug */
				put_chrs(lt, html - lt, put_chars, f);
			} else {
				put_chrs(lt, html - 1 - lt, put_chars, f);
				put_chrs(" ", 1, put_chars, f);
			}
			skip_w:
			while (html < eof && WHITECHAR(*html)) html++;
			/*putsp = -1;*/
			goto set_lt;
			put_sp:
			put_chrs(" ", 1, put_chars, f);
			/*putsp = -1;*/
		}
		if (par_format.align == AL_NO && (*html < 32 || *html == '&')) {
			int l;
			int q = qd(html, eof, &l);
			putsp = 0;
			if (q == 9) {
				put_chrs(lt, html - lt, put_chars, f);
				put_chrs("        ", 8 - pos % 8, put_chars, f);
				html += l;
				goto set_lt;
			} else if (q == 13 || q == 10) {
				put_chrs(lt, html - lt, put_chars, f);
				next_break:
				html += l;
				if (q == 13 && html < eof - 1 && qd(html, eof, &l) == 10) html += l;
				ln_break(1, line_break, f);
				if (html >= eof) goto set_lt;
				q = qd(html, eof, &l);
				if (q == 13 || q == 10) {
					line_breax = 0;
					goto next_break;
				}
				goto set_lt;
			}
		}
		if (*html < ' ') {
			/*if (putsp == 1) goto put_sp;
			putsp = 0;*/
			put_chrs(lt, html - lt, put_chars, f);
			put_chrs(".", 1, put_chars, f);
			html++;
			goto set_lt;
		}
		if (html + 2 <= eof && html[0] == '<' && (html[1] == '!' || html[1] == '?') && !d_opt->plain) {
			/*if (putsp == 1) goto put_sp;
			putsp = 0;*/
			put_chrs(lt, html - lt, put_chars, f);
			html = skip_comment(html, eof);
			goto set_lt;
		}
		if (*html != '<' || d_opt->plain || parse_element(html, eof, &name, &namelen, &attr, &end)) {
			/*if (putsp == 1) goto put_sp;
			putsp = 0;*/
			html++;
			continue;
		}
		element:
		inv = *name == '/'; name += inv; namelen -= inv;
		if (!inv && putsp == 1 && !html_top.invisible) goto put_sp;
		put_chrs(lt, html - lt, put_chars, f);
		if (par_format.align != AL_NO) if (!inv && !putsp) {
			unsigned char *ee = end;
			unsigned char *nm;
			while (!parse_element(ee, eof, &nm, NULL, NULL, &ee))
				if (*nm == '/') goto ng;
			if (ee < eof && WHITECHAR(*ee)) {
				/*putsp = -1;*/
				put_chrs(" ", 1, put_chars, f);
			}
			ng:;
		}
		html = end;
		for (ei = elements; ei->name; ei++) {
			if (ei->name &&
			   (strlen(ei->name) != (size_t)namelen || casecmp(ei->name, name, namelen)))
				continue;
			if (!inv) {
				char *a;
				ln_break(ei->linebreak, line_break, f);
				if (ei->name && ((a = get_attr_val(attr, "id")))) {
					special(f, SP_TAG, a);
					mem_free(a);
				}
				if (!html_top.invisible) {
					int a = par_format.align == AL_NO;
					struct par_attrib pa = par_format;
					if (ei->func == html_table && d_opt->tables && table_level < HTML_MAX_TABLE_LEVEL) {
						format_table(attr, html, eof, &html, f);
						ln_break(2, line_break, f);
						goto set_lt;
					}
					if (ei->func == html_select) {
						if (!do_html_select(attr, html, eof, &html, f))
							goto set_lt;
					}
					if (ei->func == html_textarea) {
						do_html_textarea(attr, html, eof, &html, f);
						goto set_lt;
					}
					if (ei->nopair == 2 || ei->nopair == 3) {
						struct html_element *e;
						if (ei->nopair == 2) {
							foreach(e, html_stack) {
								if (e->dontkill) break;
								if (e->linebreak || !ei->linebreak) break;
							}
						} else foreach(e, html_stack) {
							if (e->linebreak && !ei->linebreak) break;
							if (e->dontkill) break;
							if (e->namelen == namelen && !casecmp(e->name, name, e->namelen)) break;
						}
						if (e->namelen == namelen && !casecmp(e->name, name, e->namelen)) {
							while (e->prev != (void *)&html_stack) kill_html_stack_item(e->prev);
							if (e->dontkill != 2) kill_html_stack_item(e);
						}
					}
					if (ei->nopair != 1) {
						html_stack_dup();
						html_top.name = name;
						html_top.namelen = namelen;
						html_top.options = attr;
						html_top.linebreak = ei->linebreak;
					}
					if (ei->func) ei->func(attr);
					if (ei->func != html_br) was_br = 0;
					if (a) par_format = pa;
				}
			} else {
				struct html_element *e, *ff;
				int lnb = 0;
				int xxx = 0;
				was_br = 0;
				if (ei->nopair == 1 || ei->nopair == 3) break;
				/*debug_stack();*/
				foreach(e, html_stack) {
					if (e->linebreak && !ei->linebreak && ei->name) xxx = 1;
					if (e->namelen != namelen || casecmp(e->name, name, e->namelen)) {
						if (e->dontkill) break;
						else continue;
					}
					if (xxx) {
						kill_html_stack_item(e);
						break;
					}
					for (ff = e; ff != (void *)&html_stack; ff = ff->prev)
						if (ff->linebreak > lnb) lnb = ff->linebreak;
					ln_break(lnb, line_break, f);
					while (e->prev != (void *)&html_stack) kill_html_stack_item(e->prev);
					kill_html_stack_item(e);
					break;
				}
				/*debug_stack();*/
			}
			goto set_lt;
		}
		goto set_lt; /* unreachable */
	}
	put_chrs(lt, html - lt, put_chars, f);
	ln_break(1, line_break, f);
	putsp = -1;
	pos = 0;
	/*line_breax = 1;*/
	was_br = 0;
}

int get_image_map(unsigned char *head, unsigned char *s, unsigned char *eof, unsigned char *tag, struct menu_item **menu, struct memory_list **ml, unsigned char *href_base, unsigned char *target_base, int to, int def, int hdef)
{
	unsigned char *name, *attr, *al, *label, *href, *target;
	int namelen, lblen;
	struct link_def *ld;
	struct menu_item *nm;
	int nmenu = 0;
	int i;
	unsigned char *hd = init_str();
	int hdl = 0;
	struct conv_table *ct;
	if (head) add_to_str(&hd, &hdl, head);
	scan_http_equiv(s, eof, &hd, &hdl, NULL);
	ct = get_convert_table(hd, to, def, NULL, NULL, hdef);
	mem_free(hd);
	*menu = mem_alloc(sizeof(struct menu_item));
	memset(*menu, 0, sizeof(struct menu_item));
	/*(*menu)->text = NULL;*/
	se:
	while (s < eof && *s != '<') {
		sp:
		s++;
	}
	if (s >= eof) {
		mem_free(*menu);
		return -1;
	}
	if (s + 2 <= eof && (s[1] == '!' || s[1] == '?')) {
		s = skip_comment(s, eof);
		goto se;
	}
	if (parse_element(s, eof, &name, &namelen, &attr, &s)) goto sp;
	if (namelen != 3 || casecmp(name, "MAP", 3)) goto se;
	if (tag && *tag) {
		if (!(al = get_attr_val(attr, "name"))) goto se;
		if (strcasecmp(al, tag)) {
			mem_free(al);
			goto se;
		}
		mem_free(al);
	}
	*ml = getml(NULL);
	se2:
	while (s < eof && *s != '<') {
		sp2:
		s++;
	}
	if (s >= eof) {
		freeml(*ml);
		mem_free(*menu);
		return -1;
	}
	if (s + 2 <= eof && (s[1] == '!' || s[1] == '?')) {
		s = skip_comment(s, eof);
		goto se2;
	}
	if (parse_element(s, eof, &name, &namelen, &attr, &s)) goto sp2;
	if (namelen == 1 && !casecmp(name, "A", 1)) {
		unsigned char *ss;
		label = init_str();
		lblen = 0;
		se3:
		ss = s;
		while (ss < eof && *ss != '<') ss++;
		if (ss >= eof) {
			mem_free(label);
			freeml(*ml);
			mem_free(*menu);
			return -1;
		}
		add_bytes_to_str(&label, &lblen, s, ss - s);
		s = ss;
		if (s + 2 <= eof && (s[1] == '!' || s[1] == '?')) {
			s = skip_comment(s, eof);
			goto se3;
		}
		if (parse_element(s, eof, NULL, NULL, NULL, &ss)) goto se3;
		if (!((namelen == 1 && !casecmp(name, "A", 1)) ||
		      (namelen == 2 && !casecmp(name, "/A", 2)) ||
		      (namelen == 3 && !casecmp(name, "MAP", 3)) ||
		      (namelen == 4 && !casecmp(name, "/MAP", 4)) ||
		      (namelen == 4 && !casecmp(name, "AREA", 4)) ||
		      (namelen == 5 && !casecmp(name, "/AREA", 5)))) {
				s = ss;
				goto se3;
		}
	} else if (namelen == 4 && !casecmp(name, "AREA", 4)) {
		unsigned char *l = get_attr_val(attr, "alt");
		if (l) label = convert_string(ct, l, strlen(l)), mem_free(l);
		else label = NULL;
	} else if (namelen == 4 && !casecmp(name, "/MAP", 4)) goto done;
	else goto se2;
	if (!(href = get_url_val(attr, "href"))) {
		if (label) mem_free(label);
		goto se2;
	}
	if (!(target = get_target(attr)) && !(target = stracpy(target_base)))
		target = stracpy("");
	ld = mem_alloc(sizeof(struct link_def));
	if (!(ld->link = join_urls(href_base, href))) {
		mem_free(href);
		mem_free(target);
		f:
		mem_free(ld);
		if (label) mem_free(label);
		goto se2;
	}
	mem_free(href);
	ld->target = target;
	for (i = 0; i < nmenu; i++) {
		struct link_def *ll = (*menu)[i].data;
		if (!strcmp(ll->link, ld->link) && !strcmp(ll->target, ld->target)) {
			mem_free(ld->link);
			mem_free(ld->target);
			goto f;
		}
	}
	if (label) clr_spaces(label);
	if (label && !*label) mem_free(label), label = NULL;
	if (!label) label = stracpy(ld->link);
	if ((unsigned)nmenu > MAXINT / sizeof(struct menu_item) - 2) overalloc();
	nm = mem_realloc(*menu, (nmenu + 2) * sizeof(struct menu_item));
	*menu = nm;
	memset(&nm[nmenu], 0, 2 * sizeof(struct menu_item));
	nm[nmenu].text = label;
	nm[nmenu].rtext = "";
	nm[nmenu].hotkey = "";
	nm[nmenu].func = MENU_FUNC map_selected;
	nm[nmenu].data = ld;
	nm[++nmenu].text = NULL;
	add_to_ml(ml, ld, ld->link, ld->target, label, NULL);
	goto se2;
	done:
	add_to_ml(ml, *menu, NULL);
	return 0;
}

void scan_http_equiv(unsigned char *s, unsigned char *eof, unsigned char **head, int *hdl, unsigned char **title)
{
	unsigned char *name, *attr, *he, *c;
	int namelen;
	int tlen = 0;
	if (title) *title = init_str();
	add_chr_to_str(head, hdl, '\n');
	se:
	while (s < eof && *s != '<') sp:s++;
	if (s >= eof) return;
	if (s + 2 <= eof && (s[1] == '!' || s[1] == '?')) {
		s = skip_comment(s, eof);
		goto se;
	}
	if (parse_element(s, eof, &name, &namelen, &attr, &s)) goto sp;
	ps:
	if (namelen == 5 && !casecmp(name, "/HEAD", 5)) return;
	if (title && !tlen && namelen == 5 && !casecmp(name, "TITLE", 5)) {
		unsigned char *s1;
		xse:
		s1 = s;
		while (s < eof && *s != '<') xsp:s++;
		add_bytes_to_str(title, &tlen, s1, s - s1);
		if (s >= eof) goto se;
		if (s + 2 <= eof && (s[1] == '!' || s[1] == '?')) {
			s = skip_comment(s, eof);
			goto xse;
		}
		if (parse_element(s, eof, &name, &namelen, &attr, &s)) {
			s1 = s;
			goto xsp;
		}
		clr_spaces(*title);
		goto ps;
	}
	if (namelen != 4 || casecmp(name, "META", 4)) goto se;
	if ((he = get_attr_val(attr, "charset"))) {
		add_to_str(head, hdl, "Charset: ");
		add_to_str(head, hdl, he);
		mem_free(he);
	}
	if (!(he = get_attr_val(attr, "http-equiv"))) goto se;
	c = get_attr_val(attr, "content");
	add_to_str(head, hdl, he);
	if (c) add_to_str(head, hdl, ": "), add_to_str(head, hdl, c), mem_free(c);
	mem_free(he);
	add_to_str(head, hdl, "\r\n");
	goto se;
}

