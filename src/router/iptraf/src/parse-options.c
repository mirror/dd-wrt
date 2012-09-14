/* For terms of usage/redistribution/modification see the LICENSE file */
/* For authors and contributors see the AUTHORS file */

#include "iptraf-ng-compat.h"
#include "parse-options.h"

static int parse_opt_size(const struct options *opt)
{
	unsigned size = 1;

	for (; opt->type != OPTION_END; opt++)
		size++;

	return size;
}

#define USAGE_OPTS_WIDTH 24
#define USAGE_GAP         2

void __noreturn parse_usage_and_die(const char *const *usage,
				  const struct options *opt)
{
	fprintf(stderr, "usage: %s\n", *usage++);

	while (*usage && **usage)
		fprintf(stderr, "   or: %s\n", *usage++);

	if (opt->type != OPTION_GROUP)
		fputc('\n', stderr);

	for (; opt->type != OPTION_END; opt++) {
		size_t pos;
		int pad;

		if (opt->type == OPTION_GROUP) {
			fputc('\n', stderr);
			if (*opt->help)
				fprintf(stderr, "%s\n", opt->help);
			continue;
		}

		pos = fprintf(stderr, "    ");
		if (opt->short_name)
			pos += fprintf(stderr, "-%c", opt->short_name);

		if (opt->short_name && opt->long_name)
			pos += fprintf(stderr, ", ");

		if (opt->long_name)
			pos += fprintf(stderr, "--%s", opt->long_name);

		if (opt->argh)
			pos += fprintf(stderr, " <%s>", opt->argh);

		if (pos <= USAGE_OPTS_WIDTH)
			pad = USAGE_OPTS_WIDTH - pos;
		else {
			fputc('\n', stderr);
			pad = USAGE_OPTS_WIDTH;
		}
		fprintf(stderr, "%*s%s\n", pad + USAGE_GAP, "", opt->help);
	}
	fputc('\n', stderr);
	exit(1);
}

static int get_value(const struct options *opt)
{
	char *s = NULL;

	switch (opt->type) {
	case OPTION_BOOL:
		*(int *) opt->value += 1;
		break;
	case OPTION_INTEGER:
		*(int *) opt->value = strtol(optarg, (char **) &s, 10);
		if (*s) {
			error("invalid number -- %s", s);
			return -1;
		}
		break;
	case OPTION_STRING:
		if (optarg)
			*(char **) opt->value = (char *) optarg;
		break;
	case OPTION_GROUP:
	case OPTION_END:
		break;
	}

	return 0;
}

void parse_opts(int argc, char **argv, const struct options *opt,
		const char *const usage[])
{
	int size = parse_opt_size(opt);

	int nr = 0, alloc = 0;

	char *shortopts = NULL;
	struct option *longopts = xmallocz(sizeof(longopts[0]) * (size + 2));
	const struct options *curopt = opt;
	struct option *curlongopts = longopts;

	for (; curopt->type != OPTION_END; curopt++, curlongopts++) {
		curlongopts->name = curopt->long_name;

		switch (curopt->type) {
		case OPTION_BOOL:
			curlongopts->has_arg = no_argument;
			if (curopt->short_name) {
				ALLOC_GROW(shortopts, nr + 1, alloc);
				shortopts[nr++] = curopt->short_name;
			}
			break;
		case OPTION_INTEGER:
		case OPTION_STRING:
			curlongopts->has_arg = required_argument;
			if (curopt->short_name) {
				ALLOC_GROW(shortopts, nr + 2, alloc);
				shortopts[nr++] = curopt->short_name;
				shortopts[nr++] = ':';
			}
			break;
		case OPTION_GROUP:
		case OPTION_END:
			break;
		}

		curlongopts->flag = 0;
		curlongopts->val = curopt->short_name;
	}

	while (1) {
		curopt = opt;
		int c = getopt_long(argc, argv, shortopts, longopts, NULL);

		if (c == -1)
			break;

		if (c == '?') {
			free(longopts);
			free(shortopts);
			parse_usage_and_die(usage, opt);
		}

		for (; curopt->type != OPTION_END; curopt++) {
			if (curopt->short_name != c)
				continue;

			/* for now it fails only when string is badly converted */
			if (get_value(curopt) < 0)
				parse_usage_and_die(usage, opt);
		}
	}

	free(longopts);
	free(shortopts);
}
