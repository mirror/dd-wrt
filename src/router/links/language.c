#include "links.h"

struct translation {
	int code;
	unsigned char *name;
};

struct translation_desc {
	struct translation *t;
};

unsigned char dummyarray[T__N_TEXTS];

#include "language.inc"

unsigned char **translation_array[N_LANGUAGES][N_CODEPAGES];

int current_language;
int current_lang_charset;

void init_trans()
{
	int i, j;
	for (i = 0; i < N_LANGUAGES; i++)
		for (j = 0; j < N_CODEPAGES; j++)
			translation_array[i][j] = NULL;
	current_language = 0;
	current_lang_charset = 0;
}

void shutdown_trans()
{
	int i, j, k;
	for (i = 0; i < N_LANGUAGES; i++)
		for (j = 0; j < N_CODEPAGES; j++) if (translation_array[i][j]) {
			for (k = 0; k < T__N_TEXTS; k++) if (translation_array[i][j][k])
				mem_free(translation_array[i][j][k]);
			mem_free(translation_array[i][j]);
		}
}

static inline int is_direct_text(unsigned char *text)
{
/* Do not compare to dummyarray directly - thwart some misoptimizations */
	unsigned char * volatile dm = dummyarray;
	return !(text >= dm && text < dm + T__N_TEXTS);
}

unsigned char *get_text_translation(unsigned char *text, struct terminal *term)
{
	unsigned char **current_tra;
	struct conv_table *conv_table;
	unsigned char *trn;
	int ch = term ? term->spec->charset : 0;
	if (is_direct_text(text)) return text;
	if ((current_tra = translation_array[current_language][ch])) {
		unsigned char *tt;
		if ((trn = current_tra[text - dummyarray])) return trn;
		tr:
		if (!(tt = translations[current_language].t[text - dummyarray].name)) {
			trn = stracpy(translation_english[text - dummyarray].name);
		} else {
			conv_table = get_translation_table(current_lang_charset, ch);
			trn = convert_string(conv_table, tt, strlen(tt));
		}
		current_tra[text - dummyarray] = trn;
	} else {
		if (current_lang_charset && ch != current_lang_charset) {
			current_tra = translation_array[current_language][ch] = mem_alloc(sizeof (unsigned char **) * T__N_TEXTS);
			memset(current_tra, 0, sizeof (unsigned char **) * T__N_TEXTS);
			goto tr;
		}
		if (!(trn = translations[current_language].t[text - dummyarray].name)) {
			trn = translations[current_language].t[text - dummyarray].name = translation_english[text - dummyarray].name;	/* modifying translation structure */
		}
	}
	return trn;
}

unsigned char *get_english_translation(unsigned char *text)
{
	if (is_direct_text(text)) return text;
	return translation_english[text - dummyarray].name;
}

int n_languages()
{
	return N_LANGUAGES;
}

unsigned char *language_name(int l)
{
	return translations[l].t[T__LANGUAGE].name;
}

void set_language(int l)
{
	int i;
	unsigned char *cp;
	for (i = 0; i < T__N_TEXTS; i++) if (translations[l].t[i].code != i) {
		internal("Bad table for language %s. Run script synclang.", translations[l].t[T__LANGUAGE].name);
		return;
	}
	current_language = l;
	cp = translations[l].t[T__CHAR_SET].name;
	i = get_cp_index(cp);
	if (i == -1) {
		internal("Unknown charset for language %s.", translations[l].t[T__LANGUAGE].name);
		i = 0;
	}
	current_lang_charset = i;
}
