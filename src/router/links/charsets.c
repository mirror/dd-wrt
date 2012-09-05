#include "links.h"

struct table_entry {
	unsigned char c;
	int u;
};

struct codepage_desc {
	unsigned char *name;
	unsigned char **aliases;
	struct table_entry *table;
};

#include "codepage.inc"
#include "uni_7b.inc"
#include "entity.inc"
#include "upcase.inc"

char strings[256][2] = {
	"\000", "\001", "\002", "\003", "\004", "\005", "\006", "\007",
	"\010", "\011", "\012", "\013", "\014", "\015", "\016", "\017",
	"\020", "\021", "\022", "\023", "\024", "\025", "\026", "\033",
	"\030", "\031", "\032", "\033", "\034", "\035", "\036", "\033",
	"\040", "\041", "\042", "\043", "\044", "\045", "\046", "\047",
	"\050", "\051", "\052", "\053", "\054", "\055", "\056", "\057",
	"\060", "\061", "\062", "\063", "\064", "\065", "\066", "\067",
	"\070", "\071", "\072", "\073", "\074", "\075", "\076", "\077",
	"\100", "\101", "\102", "\103", "\104", "\105", "\106", "\107",
	"\110", "\111", "\112", "\113", "\114", "\115", "\116", "\117",
	"\120", "\121", "\122", "\123", "\124", "\125", "\126", "\127",
	"\130", "\131", "\132", "\133", "\134", "\135", "\136", "\137",
	"\140", "\141", "\142", "\143", "\144", "\145", "\146", "\147",
	"\150", "\151", "\152", "\153", "\154", "\155", "\156", "\157",
	"\160", "\161", "\162", "\163", "\164", "\165", "\166", "\167",
	"\170", "\171", "\172", "\173", "\174", "\175", "\176", "\177",
	"\200", "\201", "\202", "\203", "\204", "\205", "\206", "\207",
	"\210", "\211", "\212", "\213", "\214", "\215", "\216", "\217",
	"\220", "\221", "\222", "\223", "\224", "\225", "\226", "\227",
	"\230", "\231", "\232", "\233", "\234", "\235", "\236", "\237",
	"\240", "\241", "\242", "\243", "\244", "\245", "\246", "\247",
	"\250", "\251", "\252", "\253", "\254", "\255", "\256", "\257",
	"\260", "\261", "\262", "\263", "\264", "\265", "\266", "\267",
	"\270", "\271", "\272", "\273", "\274", "\275", "\276", "\277",
	"\300", "\301", "\302", "\303", "\304", "\305", "\306", "\307",
	"\310", "\311", "\312", "\313", "\314", "\315", "\316", "\317",
	"\320", "\321", "\322", "\323", "\324", "\325", "\326", "\327",
	"\330", "\331", "\332", "\333", "\334", "\335", "\336", "\337",
	"\340", "\341", "\342", "\343", "\344", "\345", "\346", "\347",
	"\350", "\351", "\352", "\353", "\354", "\355", "\356", "\357",
	"\360", "\361", "\362", "\363", "\364", "\365", "\366", "\367",
	"\370", "\371", "\372", "\373", "\374", "\375", "\376", "\377",
};

void free_translation_table(struct conv_table *p)
{
	int i;
	for (i = 0; i < 256; i++) if (p[i].t) free_translation_table(p[i].u.tbl);
	mem_free(p);
}

unsigned char *no_str = NULL;

void new_translation_table(struct conv_table *p)
{
	int i;
	for (i = 0; i < 256; i++) if (p[i].t) free_translation_table(p[i].u.tbl);
	for (i = 0; i < 128; i++) p[i].t = 0, p[i].u.str = strings[i];
	for (; i < 256; i++) p[i].t = 0, p[i].u.str = no_str;
}

#define BIN_SEARCH(table, entry, entries, key, result)			\
{									\
	int _s = 0, _e = (entries) - 1;					\
	while (_s <= _e || !((result) = -1)) {				\
		int _m = (_s + _e) / 2;					\
		if ((table)[_m].entry == (key)) {			\
			(result) = _m;					\
			break;						\
		}							\
		if ((table)[_m].entry > (key)) _e = _m - 1;		\
		if ((table)[_m].entry < (key)) _s = _m + 1;		\
	}								\
}									\

int strange_chars[32] = {
0x20ac, 0x0000, 0x002a, 0x0000, 0x201e, 0x2026, 0x2020, 0x2021,
0x005e, 0x2030, 0x0160, 0x003c, 0x0152, 0x0000, 0x0000, 0x0000,
0x0000, 0x0060, 0x0027, 0x0022, 0x0022, 0x002a, 0x2013, 0x2014,
0x007e, 0x2122, 0x0161, 0x003e, 0x0153, 0x0000, 0x0000, 0x0000,
};

static inline unsigned char *u2cp(int u, int to, int fallback)
{
	int j, s;
	again:
	if (u < 128) return strings[u];
	if (u == 0xa0) return "\001";
	if (u == 0xad) return "";
	if (u < 0xa0) {
		u = strange_chars[u - 0x80];
		if (!u) return NULL;
		goto again;
	}
	for (j = 0; codepages[to].table[j].c; j++)
		if (codepages[to].table[j].u == u)
			return strings[codepages[to].table[j].c];
	if (!fallback) return NULL;
	BIN_SEARCH(unicode_7b, x, N_UNICODE_7B, u, s);
	if (s != -1) return unicode_7b[s].s;
	return NULL;
}

int cp2u(unsigned char ch, int from)
{
	struct table_entry *e;
	if (from < 0 || ch < 0x80) return ch;
	for (e = codepages[from].table; e->c; e++) if (e->c == ch) return e->u;
	return -1;
}

unsigned char utf_buffer[7];

unsigned char *encode_utf_8(int u)
{
	memset(utf_buffer, 0, 7);
	if (u < 0x80) utf_buffer[0] = u;
	else if (u < 0x800)
		utf_buffer[0] = 0xc0 | ((u >> 6) & 0x1f),
		utf_buffer[1] = 0x80 | (u & 0x3f);
	else if (u < 0x10000)
		utf_buffer[0] = 0xe0 | ((u >> 12) & 0x0f),
		utf_buffer[1] = 0x80 | ((u >> 6) & 0x3f),
		utf_buffer[2] = 0x80 | (u & 0x3f);
	else if (u < 0x200000)
		utf_buffer[0] = 0xf0 | ((u >> 18) & 0x0f),
		utf_buffer[1] = 0x80 | ((u >> 12) & 0x3f),
		utf_buffer[2] = 0x80 | ((u >> 6) & 0x3f),
		utf_buffer[3] = 0x80 | (u & 0x3f);
	else if (u < 0x4000000)
		utf_buffer[0] = 0xf8 | ((u >> 24) & 0x0f),
		utf_buffer[1] = 0x80 | ((u >> 18) & 0x3f),
		utf_buffer[2] = 0x80 | ((u >> 12) & 0x3f),
		utf_buffer[3] = 0x80 | ((u >> 6) & 0x3f),
		utf_buffer[4] = 0x80 | (u & 0x3f);
	else	utf_buffer[0] = 0xfc | ((u >> 30) & 0x01),
		utf_buffer[1] = 0x80 | ((u >> 24) & 0x3f),
		utf_buffer[2] = 0x80 | ((u >> 18) & 0x3f),
		utf_buffer[3] = 0x80 | ((u >> 12) & 0x3f),
		utf_buffer[4] = 0x80 | ((u >> 6) & 0x3f),
		utf_buffer[5] = 0x80 | (u & 0x3f);
	return utf_buffer;
}

void add_utf_8(struct conv_table *ct, int u, unsigned char *str)
{
	unsigned char *p = encode_utf_8(u);
	while (p[1]) {
		if (ct[*p].t) ct = ct[*p].u.tbl;
		else {
			struct conv_table *nct;
			if (ct[*p].u.str != no_str) {
				internal("bad utf encoding #1");
				return;
			}
			nct = mem_alloc(sizeof(struct conv_table) * 256);
			memset(nct, 0, sizeof(struct conv_table) * 256);
			new_translation_table(nct);
			ct[*p].t = 1;
			ct[*p].u.tbl = nct;
			ct = nct;
		}
		p++;
	}
	if (ct[*p].t) {
		internal("bad utf encoding #2");
		return;
	}
	if (ct[*p].u.str == no_str) ct[*p].u.str = str;
}

struct conv_table utf_table[256];
int utf_table_init = 1;

void free_utf_table()
{
	int i;
	for (i = 128; i < 256; i++) mem_free(utf_table[i].u.str);
}

struct conv_table *get_translation_table_to_utf_8(int from)
{
	int i;
	static int lfr = -1;
	if (from == -1) return NULL;
	if (from == lfr) return utf_table;
	if (utf_table_init) memset(utf_table, 0, sizeof(struct conv_table) * 256), utf_table_init = 0;
	else free_utf_table();
	for (i = 0; i < 128; i++) utf_table[i].u.str = strings[i];
	if (codepages[from].table == table_utf_8) {
		for (i = 128; i < 256; i++) utf_table[i].u.str = stracpy(strings[i]);
		return utf_table;
	}
	for (i = 128; i < 256; i++) utf_table[i].u.str = NULL;
	for (i = 0; codepages[from].table[i].c; i++) {
		int u = codepages[from].table[i].u;
		if (!utf_table[codepages[from].table[i].c].u.str)
			utf_table[codepages[from].table[i].c].u.str = stracpy(encode_utf_8(u));
	}
	for (i = 128; i < 256; i++)
		if (!utf_table[i].u.str) utf_table[i].u.str = stracpy(no_str);
	return utf_table;
}

struct conv_table table[256];
static int first = 1;

void free_conv_table()
{
	if (!utf_table_init) free_utf_table();
	if (first) memset(table, 0, sizeof(struct conv_table) * 256), first = 0;
	new_translation_table(table);
	if (no_str) mem_free(no_str), no_str = NULL;
}

struct conv_table *get_translation_table(int from, int to)
{
	int i;
	static int lfr = -1;
	static int lto = -1;
	if (!no_str) no_str = stracpy("*");
	if (first) memset(table, 0, sizeof(struct conv_table) * 256), first = 0;
	if (/*from == to ||*/ from == -1 || to == -1) return NULL;
	if (codepages[to].table == table_utf_8) return get_translation_table_to_utf_8(from);
	if (from == lfr && to == lto) return table;
	lfr = from; lto = to;
	new_translation_table(table);
	if (codepages[from].table == table_utf_8) {
		int j;
		for (j = 0; codepages[to].table[j].c; j++) add_utf_8(table, codepages[to].table[j].u, codepages[to].table[j].u == 0xa0 ? "\001" : codepages[to].table[j].u == 0xad ? "" : strings[codepages[to].table[j].c]);
		for (i = 0; unicode_7b[i].x != -1; i++) if (unicode_7b[i].x >= 0x80) add_utf_8(table, unicode_7b[i].x, unicode_7b[i].s);
	} else for (i = 128; i < 256; i++) {
		int j;
		char *u;
		for (j = 0; codepages[from].table[j].c; j++) {
			if (codepages[from].table[j].c == i) goto f;
		}
		continue;
		f:
		u = u2cp(codepages[from].table[j].u, to, 1);
		if (u) table[i].u.str = u;
	}
	return table;
}

static inline int xxstrcmp(unsigned char *s1, unsigned char *s2, int l2)
{
	while (l2) {
		if (*s1 > *s2) return 1;
		if (!*s1 || *s1 < *s2) return -1;
		s1++, s2++, l2--;
	}
	return !!*s1;
}

int get_entity_number(unsigned char *st, int l)
{
	int n = 0;
	if (upcase(st[0]) == 'X') {
		st++, l--;
		if (!l) return -1;
		do {
			char c = upcase(*(st++));
			if (c >= '0' && c <= '9') n = n * 16 + c - '0';
			else if (c >= 'A' && c <= 'F') n = n * 16 + c - 'A' + 10;
			else return -1;
			if (n >= 0x10000) return -1;
		} while (--l);
	} else {
		if (!l) return -1;
		do {
			char c = *(st++);
			if (c >= '0' && c <= '9') n = n * 10 + c - '0';
			else return -1;
			if (n >= 0x10000) return -1;
		} while (--l);
	}
	return n;
}

unsigned char *get_entity_string(unsigned char *st, int l, int encoding)
{
	int n;
	if (l <= 0) return NULL;
	if (st[0] == '#') {
		if (l == 1) return NULL;
		if ((n = get_entity_number(st + 1, l - 1)) == -1) return NULL;
		if (n < 32 && get_attr_val_nl != 2) n = 32;
	} else {
		int s = 0, e = N_ENTITIES - 1;
		while (s <= e) {
			int c;
			int m = (s + e) / 2;
			c = xxstrcmp(entities[m].s, st, l);
			if (!c) {
				n = entities[m].c;
				goto f;
			}
			if (c > 0) e = m - 1;
			else s = m + 1;
		}
		return NULL;
		f:;
	}

	return u2cp(n, encoding, 1);
}

unsigned char *convert_string(struct conv_table *ct, unsigned char *c, int l)
{
	unsigned char *buffer;
	int bp = 0;
	int pp = 0;
	if (!ct) {
		int i;
		for (i = 0; i < l; i++) if (c[i] == '&') goto xx;
		return memacpy(c, l);
		xx:;
	}
	buffer = mem_alloc(ALLOC_GR);
	while (pp < l) {
		unsigned char *e;
		if (c[pp] < 128 && c[pp] != '&') {
			put_c:
			buffer[bp++] = c[pp++];
			if (!(bp & (ALLOC_GR - 1))) {
				if ((unsigned)bp > MAXINT - ALLOC_GR) overalloc();
				buffer = mem_realloc(buffer, bp + ALLOC_GR);
			}
			continue;
		}
		if (c[pp] != '&') {
			struct conv_table *t;
			int i;
			if (!ct) goto put_c;
			t = ct;
			i = pp;
			decode:
			if (!t[c[i]].t) {
				e = t[c[i]].u.str;
			} else {
				t = t[c[i++]].u.tbl;
				if (i >= l) goto put_c;
				goto decode;
			}
			pp = i + 1;
		} else {
			int i = pp + 1;
			if (d_opt->plain) goto put_c;
			while (i < l && c[i] != ';' && c[i] != '&' && c[i] > ' ') i++;
			if (!(e = get_entity_string(&c[pp + 1], i - pp - 1, d_opt->cp))) goto put_c;
			pp = i + (i < l && c[i] == ';');
		}
		if (!e[0]) continue;
		if (!e[1]) {
			buffer[bp++] = e[0];
			if (!(bp & (ALLOC_GR - 1))) {
				if ((unsigned)bp > MAXINT - ALLOC_GR) overalloc();
				buffer = mem_realloc(buffer, bp + ALLOC_GR);
			}
			continue;
		}
		while (*e) {
			buffer[bp++] = *(e++);
			if (!(bp & (ALLOC_GR - 1))) {
				if ((unsigned)bp > MAXINT - ALLOC_GR) overalloc();
				buffer = mem_realloc(buffer, bp + ALLOC_GR);
			}
		}
	}
	buffer[bp] = 0;
	return buffer;
}

int get_cp_index(unsigned char *n)
{
	int i, a, p, q;
	int ii = -1, ll = 0;
	for (i = 0; codepages[i].name; i++) {
		for (a = 0; codepages[i].aliases[a]; a++) {
			for (p = 0; n[p]; p++) {
				if (upcase(n[p]) == upcase(codepages[i].aliases[a][0])) {
					for (q = 1; codepages[i].aliases[a][q]; q++) {
						if (upcase(n[p+q]) != upcase(codepages[i].aliases[a][q])) goto fail;
					}
					if (strlen(codepages[i].aliases[a]) > (size_t)ll) {
						ll = strlen(codepages[i].aliases[a]);
						ii = i;
					}
				}
				fail:;
			}
		}
	}
	return ii;
}

unsigned char *get_cp_name(int index)
{
	if (index < 0) return "none";
	return codepages[index].name;
}

unsigned char *get_cp_mime_name(int index)
{
	if (index < 0) return "none";
	if (!codepages[index].aliases) return NULL;
	return codepages[index].aliases[0];
}

int is_cp_special(int index)
{
	return codepages[index].table == table_utf_8;
}

unsigned char charset_upcase(unsigned char ch, int cp)
{
	int u, res;
	unsigned char *str;
	if (ch < 0x80) return upcase(ch);
	u = cp2u(ch, cp);
	BIN_SEARCH(unicode_upcase, lo, sizeof(unicode_upcase) / sizeof(*unicode_upcase), u, res);
	if (res == -1) return ch;
	str = u2cp(unicode_upcase[res].up, cp, 0);
	if (!str || !str[0] || str[1]) return ch;
	return str[0];
}

void charset_upcase_string(unsigned char **chp, int cp)
{
	unsigned char *ch = *chp;
	int i;
	for (i = 0; ch[i]; i++) ch[i] = charset_upcase(ch[i], cp);
}

