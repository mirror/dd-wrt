#ifndef IPTRAF_NG_PARSE_OPTIONS_H
#define IPTRAF_NG_PARSE_OPTIONS_H

enum parse_opt_type {
	OPTION_BOOL,
	OPTION_GROUP,
	OPTION_STRING,
	OPTION_INTEGER,
	OPTION_END,
};

struct options {
	enum parse_opt_type type;
	int short_name;
	const char *long_name;
	void *value;
	const char *argh;
	const char *help;
};

/*
 * s - short_name
 * l - long_name
 * v - value
 * a - argh argument help
 * h - help
 */
#define OPT_END()                   { OPTION_END, 0, NULL, NULL, NULL, NULL }
#define OPT_BOOL(s, l, v, h)        { OPTION_BOOL, (s), (l), (v), NULL, (h) }
#define OPT_GROUP(h)                { OPTION_GROUP, 0, NULL, NULL, NULL, (h) }
#define OPT_INTEGER(s, l, v, h)     { OPTION_INTEGER, (s), (l), (v), "n", (h) }
#define OPT_STRING(s, l, v, a, h)   { OPTION_STRING, (s), (l), (v), (a), (h) }

#define OPT__VERBOSE(v)     OPT_BOOL('v', "verbose", (v), "be verbose")
#define OPT__HELP(v)        OPT_BOOL('h', "help", (v), "show this help message")

void parse_opts(int argc, char **argv, const struct options *opt,
		const char *const usage[]);

void parse_usage_and_die(const char *const *usage, const struct options *opt);

#endif	/* IPTRAF_NG_PARSE_OPTIONS_H */
