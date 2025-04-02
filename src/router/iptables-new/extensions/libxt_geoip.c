/*
 *	"geoip" match extension for iptables
 *	Copyright © Samuel Jean <peejix [at] people netfilter org>, 2004 - 2008
 *	Copyright © Nicolas Bouliane <acidfu [at] people netfilter org>, 2004 - 2008
 *	Jan Engelhardt, 2008-2011
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License; either
 *	version 2 of the License, or any later version, as published by the
 *	Free Software Foundation.
 */
#include <sys/stat.h>
#include <sys/types.h>
#include <ctype.h>
#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <xtables.h>
#include "xt_geoip.h"
#include "compat_user.h"
#define GEOIP_DB_DIR "/usr/share/xt_geoip"

static void geoip_help(void)
{
	printf (
	"geoip match options:\n"
	"[!] --src-cc, --source-country country[,country...]\n"
	"	Match packet coming from (one of) the specified country(ies)\n"
	"[!] --dst-cc, --destination-country country[,country...]\n"
	"	Match packet going to (one of) the specified country(ies)\n"
	"\n"
	"NOTE: The country is inputed by its ISO 3166 code.\n"
	"\n"
	);
}

static struct option geoip_opts[] = {
	{.name = "dst-cc",              .has_arg = true, .val = '2'},
	{.name = "destination-country", .has_arg = true, .val = '2'},
	{.name = "src-cc",              .has_arg = true, .val = '1'},
	{.name = "source-country",      .has_arg = true, .val = '1'},
	{NULL},
};

#if __BYTE_ORDER == __LITTLE_ENDIAN
static void geoip_swap_le32(uint32_t *buf)
{
	unsigned char *p = (void *)buf;
	uint32_t n = p[0] + (p[1] << 8) + (p[2] << 16) + (p[3] << 24);
	p[0] = (n >> 24) & 0xff;
	p[1] = (n >> 16) & 0xff;
	p[2] = (n >> 8) & 0xff;
	p[3] = n & 0xff;
}

static void geoip_swap_in6(struct in6_addr *in6)
{
	geoip_swap_le32(&in6->s6_addr32[0]);
	geoip_swap_le32(&in6->s6_addr32[1]);
	geoip_swap_le32(&in6->s6_addr32[2]);
	geoip_swap_le32(&in6->s6_addr32[3]);
}
#endif

static void transform(unsigned char *src, unsigned char *dst, size_t len, int addrlen)
{
	size_t a, b;
	size_t cnt = 0;
	for (b = 0; b < addrlen; b++) {
		for (a = 0; a < len; a += (addrlen * 2)) {
			dst[a + b] = src[cnt++];
			dst[a + 4 + b] = src[cnt++];
		}
	}
}

static void *
geoip_get_subnets(const char *code, uint32_t *count, uint8_t nfproto)
{
	void *subnets, *src;
	struct stat sb;
	char buf[256];
#if __BYTE_ORDER == __LITTLE_ENDIAN
	unsigned int n;
#endif

	/* Use simple integer vector files */
	if (nfproto == NFPROTO_IPV6)
		snprintf(buf, sizeof(buf), GEOIP_DB_DIR "/%s.iv6", code);
	else
		snprintf(buf, sizeof(buf), GEOIP_DB_DIR "/%s.iv4", code);

	int fd = open(buf, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "Could not open %s: %s\n", buf, strerror(errno));
		xtables_error(OTHER_PROBLEM, "Could not read geoip database");
	}

	fstat(fd, &sb);
	*count = sb.st_size;
	switch (nfproto) {
	case NFPROTO_IPV6:
		if (sb.st_size % sizeof(struct geoip_subnet6) != 0)
			xtables_error(OTHER_PROBLEM,
				"Database file %s seems to be corrupted", buf);
		*count /= sizeof(struct geoip_subnet6);
		break;
	case NFPROTO_IPV4:
		if (sb.st_size % sizeof(struct geoip_subnet4) != 0)
			xtables_error(OTHER_PROBLEM,
				"Database file %s seems to be corrupted", buf);
		*count /= sizeof(struct geoip_subnet4);
		break;
	}
	src = malloc(sb.st_size);
	subnets = malloc(sb.st_size);
	if (subnets == NULL)
		xtables_error(OTHER_PROBLEM, "geoip: insufficient memory");
	read(fd, src, sb.st_size);
	close(fd);
	transform((unsigned char *)src, (unsigned char *)subnets, sb.st_size, nfproto == NFPROTO_IPV6 ? 16 : 4);
	free(src);
#if __BYTE_ORDER == __LITTLE_ENDIAN
	for (n = 0; n < *count; ++n) {
		switch (nfproto) {
		case NFPROTO_IPV6: {
			struct geoip_subnet6 *gs6 = &(((struct geoip_subnet6 *)subnets)[n]);
			geoip_swap_in6(&gs6->begin);
			geoip_swap_in6(&gs6->end);
			break;
		}
		case NFPROTO_IPV4: {
			struct geoip_subnet4 *gs4 = &(((struct geoip_subnet4 *)subnets)[n]);
			geoip_swap_le32(&gs4->begin);
			geoip_swap_le32(&gs4->end);
			break;
		}
		}
	}
#endif
	return subnets;
}

static struct geoip_country_user *geoip_load_cc(const char *code,
    unsigned short cc, uint8_t nfproto)
{
	struct geoip_country_user *ginfo;
	ginfo = malloc(sizeof(struct geoip_country_user));

	if (!ginfo)
		return NULL;

	ginfo->subnets = (unsigned long)geoip_get_subnets(code,
	                 &ginfo->count, nfproto);
	ginfo->cc = cc;

	return ginfo;
}

static u_int16_t
check_geoip_cc(char *cc, u_int16_t cc_used[], u_int8_t count)
{
	u_int8_t i;
	u_int16_t cc_int16;

	if (strlen(cc) != 2) /* Country must be 2 chars long according
													 to the ISO 3166 standard */
		xtables_error(PARAMETER_PROBLEM,
			"geoip: invalid country code '%s'", cc);

	// Verification will fail if chars aren't uppercased.
	// Make sure they are..
	for (i = 0; i < 2; i++)
		if (isalnum(cc[i]) != 0)
			cc[i] = toupper(cc[i]);
		else
			xtables_error(PARAMETER_PROBLEM,
				"geoip: invalid country code '%s'", cc);

	/* Convert chars into a single 16-bit integer.
	 * FIXME:	This assumes that a country code is
	 *			 exactly 2 chars long. If this is
	 *			 going to change someday, this whole
	 *			 match will need to be rewritten, anyway.
	 *											 - SJ  */
	cc_int16 = (cc[0] << 8) | cc[1];

	// Check for presence of value in cc_used
	for (i = 0; i < count; i++)
		if (cc_int16 == cc_used[i])
			return 0; // Present, skip it!

	return cc_int16;
}

static unsigned int parse_geoip_cc(const char *ccstr, uint16_t *cc,
    union geoip_country_group *mem, uint8_t nfproto)
{
	char *buffer, *cp, *next;
	u_int8_t i, count = 0;
	u_int16_t cctmp;

	buffer = strdup(ccstr);
	if (!buffer)
		xtables_error(OTHER_PROBLEM,
			"geoip: insufficient memory available");

	for (cp = buffer, i = 0; cp && i < XT_GEOIP_MAX; cp = next, i++)
	{
		next = strchr(cp, ',');
		if (next) *next++ = '\0';

		cctmp = check_geoip_cc(cp, cc, count);
		if (cctmp != 0) {
			if ((mem[count++].user =
			    (unsigned long)geoip_load_cc(cp, cctmp, nfproto)) == 0)
				xtables_error(OTHER_PROBLEM,
					"geoip: insufficient memory available");
			cc[count-1] = cctmp;
		}
	}

	if (cp)
		xtables_error(PARAMETER_PROBLEM,
			"geoip: too many countries specified");
	free(buffer);

	if (count == 0)
		xtables_error(PARAMETER_PROBLEM,
			"geoip: don't know what happened");

	return count;
}

static int geoip_parse(int c, bool invert, unsigned int *flags,
    const char *arg, struct xt_geoip_match_info *info, uint8_t nfproto)
{
	switch (c) {
	case '1':
		if (*flags & (XT_GEOIP_SRC | XT_GEOIP_DST))
			xtables_error(PARAMETER_PROBLEM,
				"geoip: Only exactly one of --source-country "
				"or --destination-country must be specified!");

		*flags |= XT_GEOIP_SRC;
		if (invert)
			*flags |= XT_GEOIP_INV;

		info->count = parse_geoip_cc(arg, info->cc, info->mem,
		              nfproto);
		info->flags = *flags;
		return true;

	case '2':
		if (*flags & (XT_GEOIP_SRC | XT_GEOIP_DST))
			xtables_error(PARAMETER_PROBLEM,
				"geoip: Only exactly one of --source-country "
				"or --destination-country must be specified!");

		*flags |= XT_GEOIP_DST;
		if (invert)
			*flags |= XT_GEOIP_INV;

		info->count = parse_geoip_cc(arg, info->cc, info->mem,
		              nfproto);
		info->flags = *flags;
		return true;
	}

	return false;
}

static int geoip_parse6(int c, char **argv, int invert, unsigned int *flags,
    const void *entry, struct xt_entry_match **match)
{
	return geoip_parse(c, invert, flags, optarg,
	       (void *)(*match)->data, NFPROTO_IPV6);
}

static int geoip_parse4(int c, char **argv, int invert, unsigned int *flags,
    const void *entry, struct xt_entry_match **match)
{
	return geoip_parse(c, invert, flags, optarg,
	       (void *)(*match)->data, NFPROTO_IPV4);
}

static void
geoip_final_check(unsigned int flags)
{
	if (!flags)
		xtables_error(PARAMETER_PROBLEM,
			"geoip: missing arguments");
}

static void
geoip_save(const void *ip, const struct xt_entry_match *match)
{
	const struct xt_geoip_match_info *info = (void *)match->data;
	u_int8_t i;

	if (info->flags & XT_GEOIP_INV)
		printf(" !");

	if (info->flags & XT_GEOIP_SRC)
		printf(" --source-country ");
	else
		printf(" --destination-country ");

	for (i = 0; i < info->count; i++)
		printf("%s%c%c", i ? "," : "", COUNTRY(info->cc[i]));
	printf(" ");
}

static void
geoip_print(const void *ip, const struct xt_entry_match *match, int numeric)
{
	printf(" -m geoip");
	geoip_save(ip, match);
}

static struct xtables_match geoip_match[] = {
	{
		.family        = NFPROTO_IPV6,
		.name          = "geoip",
		.revision      = 2,
		.version       = XTABLES_VERSION,
		.size          = XT_ALIGN(sizeof(struct xt_geoip_match_info)),
		.userspacesize = offsetof(struct xt_geoip_match_info, mem),
//		.help          = geoip_help,
		.parse         = geoip_parse6,
		.final_check   = geoip_final_check,
		.print         = geoip_print,
		.save          = geoip_save,
		.extra_opts    = geoip_opts,
	},
	{
		.family        = NFPROTO_IPV4,
		.name          = "geoip",
		.revision      = 2,
		.version       = XTABLES_VERSION,
		.size          = XT_ALIGN(sizeof(struct xt_geoip_match_info)),
		.userspacesize = offsetof(struct xt_geoip_match_info, mem),
//		.help          = geoip_help,
		.parse         = geoip_parse4,
		.final_check   = geoip_final_check,
		.print         = geoip_print,
		.save          = geoip_save,
		.extra_opts    = geoip_opts,
	},
};

void _init(void)
{
	xtables_register_matches(geoip_match,
		sizeof(geoip_match) / sizeof(*geoip_match));
}
