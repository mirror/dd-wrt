/* Simple .conf file parser for smcroute
 *
 * Copyright (C) 2011-2021  Joachim Wiberg <troglobit@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <errno.h>
#include <glob.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "log.h"
#include "conf.h"
#include "iface.h"
#include "script.h"
#include "mcgroup.h"
#include "util.h"

#define MAX_LINE_LEN 512

#define DEBUG(fmt, args...) do {					\
	if (conf)							\
		smclog(LOG_DEBUG, "%s line %d: " fmt, conf->file,	\
		       conf->lineno, ##args);				\
	else								\
		smclog(LOG_DEBUG, "ipc: " fmt, ##args);			\
	} while (0)
#define INFO(fmt, args...) do {						\
	if (conf)							\
		smclog(LOG_INFO, "%s line %d: " fmt, conf->file,	\
			conf->lineno, ##args);				\
	else								\
		smclog(LOG_INFO, "ipc: " fmt, ##args);			\
	} while (0)

#define WARN(fmt, args...) do {						\
	if (conf)							\
		smclog(LOG_WARNING, "%s line %d: " fmt, conf->file,	\
		       conf->lineno, ##args);				\
	else								\
		smclog(LOG_WARNING, "ipc: " fmt, ##args);		\
	if (conf_vrfy)							\
		rc++;							\
	} while (0)

/* Used only for verifying .conf files */
static int conf_vrfy_vif;

static char *pop_token(char **line)
{
	char *end, *token;

	if (!line)
		return NULL;

	token = *line;
	if (!token)
		return NULL;

	/* Find start of token, skip whitespace. */
	while (*token && isspace((int)*token))
		token++;

	/* Find end of token. */
	end = token;
	while (*end && !isspace((int)*end))
		end++;
	if (end == token) {
		*line = NULL;
		return NULL;
	}

	*end = 0;		/* Terminate token. */
	*line = end + 1;

	return token;
}

static int match(char *keyword, char *token)
{
	size_t len;

	if (!keyword || !token)
		return 0;

	len = strlen(keyword);
	return !strncmp(keyword, token, len);
}

int conf_mgroup(struct conf *conf, int cmd, char *iif, char *source, char *group)
{
	inet_addr_t src = { 0 }, grp = { 0 };
	int src_len = 0;
	int grp_len = 0;
	int len_max;
	int family;
	int rc = 0;

	if (!iif || !group) {
		errno = EINVAL;
		return 1;
	}

	grp_len = is_range(group);
	if (inet_str2addr(group, &grp) || !is_multicast(&grp)) {
		WARN("join: Invalid multicast group: %s", group);
		goto done;
	}
	family = grp.ss_family;

#ifdef HAVE_IPV6_MULTICAST_HOST
	if (family == AF_INET6)
		len_max = 128;
	else
#endif
	len_max = 32;
	if (grp_len < 0 || grp_len > len_max) {
		WARN("join: Invalid group prefix length (0-%d): %d", len_max, grp_len);
		goto done;
	}
	if (!grp_len)
		grp_len = len_max;

	if (source) {
		src_len = is_range(source);
		if (src_len < 0 || src_len > len_max) {
			WARN("join: Invalid source prefix length (0-%d): %d", len_max, src_len);
			goto done;
		}

		if (inet_str2addr(source, &src)) {
			WARN("join: Invalid multicast source: %s", source);
			goto done;
		}
	} else
		inet_anyaddr(family, &src);
	if (!src_len)
		src_len = len_max;

	if (!conf_vrfy)
		rc += mcgroup_action(cmd, iif, &src, src_len, &grp, grp_len);
done:
	return rc;
}

int conf_mroute(struct conf *conf, int cmd, char *iif, char *source, char *group, char *oif[], int num)
{
	struct ifmatch state_in, state_out;
	struct mroute mroute = { 0 };
	struct iface *iface_in, *iface;
	int len_max;
	int family;
	int rc = 0;
	int vif;

	if (!iif || !group) {
		errno = EINVAL;
		return 1;
	}

	mroute.len = is_range(group);
	if (inet_str2addr(group, &mroute.group) || !is_multicast(&mroute.group)) {
		WARN("mroute: Invalid multicast group: %s", group);
		goto done;
	}
	family = mroute.group.ss_family;

#ifdef HAVE_IPV6_MULTICAST_HOST
	if (family == AF_INET6)
		len_max = 128;
	else
#endif
		len_max = 32;
	if (mroute.len < 0 || mroute.len > len_max) {
		WARN("mroute: Invalid multicast group prefix length, %d", mroute.len);
		goto done;
	}
	if (!mroute.len)
		mroute.len = len_max;

	if (source) {
		mroute.src_len = is_range(source);
		if (mroute.src_len < 0 || mroute.src_len > len_max) {
			WARN("mroute: invalid prefix length: %d", mroute.src_len);
			goto done;
		}

		if (inet_str2addr(source, &mroute.source)) {
			WARN("mroute: Invalid source address: %s", source);
			goto done;
		}
	} else {
		inet_anyaddr(family, &mroute.source);
		mroute.src_len = 0;
	}
	if (!mroute.src_len)
		mroute.src_len = len_max;

	iface_match_init(&state_in);
	DEBUG("mroute: checking for input iface %s ...", iif);
	while (iface_match_vif_by_name(iif, &state_in, &iface_in) != NO_VIF) {
		char src[INET_ADDRSTR_LEN], grp[INET_ADDRSTR_LEN];

		vif = iface_get_vif(family, iface_in);
		DEBUG("mroute: input iface %s has vif %d", iif, vif);
		mroute.inbound = vif;

		for (int i = 0; i < num; i++) {
			iface_match_init(&state_out);

			DEBUG("mroute: checking for %s ...", oif[i]);
			while (iface_match_vif_by_name(oif[i], &state_out, &iface) != NO_VIF) {
				vif = iface_get_vif(family, iface);
				if (vif == NO_VIF)
					continue;

				if (vif == mroute.inbound && cmd) {
					/* In case of wildcard match in==out is normal, so don't complain */
					if (!ifname_is_wildcard(iif) && !ifname_is_wildcard(oif[i]))
						INFO("mroute: Same outbound interface (%s) as inbound (%s) may cause routing loops.",
						     oif[i], iface_in->ifname);
				}

				/* Use configured TTL threshold for the output phyint */
				mroute.ttl[vif] = iface->threshold;
			}
			if (!state_out.match_count)
				WARN("mroute: outbound %s is not a known phyint, skipping", oif[i]);
		}

		if (conf_vrfy)
			continue;

		if (cmd) {
			smclog(LOG_DEBUG, "mroute: adding route from %s (%s/%u,%s/%u)", iface_in->ifname,
			       inet_addr2str(&mroute.source, src, sizeof(src)), mroute.src_len,
			       inet_addr2str(&mroute.group, grp, sizeof(grp)), mroute.len);
			rc += mroute_add_route(&mroute);
		} else {
			smclog(LOG_DEBUG, "mroute: deleting route from %s (%s/%u,%s/%u)", iface_in->ifname,
			       inet_addr2str(&mroute.source, src, sizeof(src)), mroute.src_len,
			       inet_addr2str(&mroute.group, grp, sizeof(grp)), mroute.len);
			rc += mroute_del_route(&mroute);
		}
	}

	if (!state_in.match_count) {
		WARN("mroute: inbound %s is not a known phyint", iif);
		rc++;
	}

done:
	return rc;
}

static int conf_phyint(struct conf *conf, int enable, char *iif, int mrdisc, int threshold)
{
	(void)conf;

	if (conf_vrfy) {
		struct iface *iface;
		struct ifmatch ifm;

		iface_match_init(&ifm);
		iface = iface_match_by_name(iif, 1, &ifm);
		if (!iface)
			return 1;

		iface->vif = conf_vrfy_vif;
		iface->mif = conf_vrfy_vif++;

		return 0;
	}

	if (enable)
		return mroute_add_vif(iif, mrdisc, threshold);

	return mroute_del_vif(iif);
}

/*
 * This function parses the given configuration file according to the
 * below format rules.  Joins multicast groups and creates multicast
 * routes accordingly in the kernel.  Whitespace is ignored.
 *
 * Format:
 *    phyint IFNAME <enable|disable> [ttl-threshold <1-255>]
 *    mgroup   from IFNAME [source ADDRESS] group MCGROUP
 *    mroute   from IFNAME source ADDRESS   group MCGROUP to IFNAME [IFNAME ...]
 *    include FILEPATTERN
 */
int conf_parse(struct conf *conf, int do_vifs)
{
	char *linebuf, *line;
	int rc = 0;
	FILE *fp;

	fp = fopen(conf->file, "r");
	if (!fp) {
		if (errno == ENOENT)
			smclog(LOG_NOTICE, "Configuration file %s does not exist", conf->file);
		else
			smclog(LOG_WARNING, "Failed opening %s: %s", conf->file, strerror(errno));

		if (!conf_vrfy)
			smclog(LOG_NOTICE, "Continuing anyway, waiting for client to connect.");

		return 1;
	}

	linebuf = malloc(MAX_LINE_LEN * sizeof(char));
	if (!linebuf) {
		int tmp = errno;

		fclose(fp);
		errno = tmp;
		smclog(LOG_ERR, "Failed allocating memory to read .conf file: %s", strerror(errno));
		exit(EX_OSERR);
	}

	conf->lineno = 0;
next:
	while ((line = fgets(linebuf, MAX_LINE_LEN, fp))) {
		int   mrdisc = 0, threshold = DEFAULT_THRESHOLD;
		int   op = 0, num = 0, enable = do_vifs;
		char *oif[MAX_MC_VIFS];
		char *include = NULL;
		char *source = NULL;
		char *group  = NULL;
		char *iif = NULL;
		char *ttl = NULL;
		char *token;
		glob_t gl;
		size_t i;

		/* Strip any line end character(s) */
		chomp(line);
		conf->lineno++;

		DEBUG("%s", line);
		while ((token = pop_token(&line))) {
			/* Strip comments. */
			if (match("#", token))
				break;

			if (!op) {
				if (match("mgroup", token)) {
					op = MGROUP;
				} else if (match("ssmgroup", token)) {
					op = MGROUP; /* Compat */
				} else if (match("mroute", token)) {
					op = MROUTE;
				} else if (match("phyint", token)) {
					op = PHYINT;
					iif = pop_token(&line);
					if (!iif) {
						WARN("phyint missing interface pattern");
						goto next;
					}
				} else if (match("include", token)) {
					op = INCLUDE;
					include = pop_token(&line);
					smclog(LOG_DEBUG, "Found include --> %s", include);
					break;
				} else {
					WARN("Unknown command %s, skipping.", token);
					goto next;
				}
			}

			if (match("from", token)) {
				iif = pop_token(&line);
			} else if (match("source", token)) {
				source = pop_token(&line);
			} else if (match("group", token)) {
				group = pop_token(&line);
			} else if (match("to", token)) {
				while ((oif[num] = pop_token(&line)))
					num++;
			} else if (match("enable", token)) {
				enable = 1;
			} else if (match("disable", token)) {
				enable = 0;
			} else if (match("mrdisc", token)) {
				mrdisc = 1;
			} else if (match("ttl-threshold", token)) {
				ttl = pop_token(&line);
			}
		}

		if (iif && !iface_exist(iif)) {
			switch (op) {
			case MGROUP:
				WARN("mgroup from %s matches no valid phyint, skipping ...", iif);
				break;

			case MROUTE:
				WARN("mroute from %s matches no valid phyint, skipping ...", iif);
				break;

			default:
				WARN("phyint %s does not exist (yet?) on this system", iif);
				break;
			}
			continue;
		}

		if (ttl) {
			int val = atoi(ttl);

			if (val < 1 || val > 255)
				WARN("phyint %s ttl %s out of range (1-255)", iif ? iif : "", ttl);
			else
				threshold = val;
		}

		switch (op) {
		case EMPTY:
			break;

		case MGROUP:
			rc += conf_mgroup(conf, 1, iif, source, group);
			break;

		case MROUTE:
			rc += conf_mroute(conf, 1, iif, source, group, oif, num);
			break;

		case PHYINT:
			rc += conf_phyint(conf, enable, iif, mrdisc, threshold);
			break;

		case INCLUDE:
			glob(include, 0, NULL, &gl);
			for (i = 0; i < gl.gl_pathc; i++) {
				struct conf inc = { .file = gl.gl_pathv[i] };

				smclog(LOG_DEBUG, "Glob expansion to %s ...", gl.gl_pathv[i]);
				if (conf_parse(&inc, do_vifs))
					smclog(LOG_WARNING, "Failed reading %s: %s",
					       gl.gl_pathv[i], strerror(errno));
			}
			globfree(&gl);
			break;

		default:
			WARN("Unknown token %d", op);
			break;
		}
	}

	free(linebuf);
	fclose(fp);

	if (rc) {
		errno = EOPNOTSUPP;
		return 1;
	}

	return 0;
}

/* Parse .conf file and setup routes */
int conf_read(char *file, int do_vifs)
{
	struct conf conf = { .file = file };

	if (conf_parse(&conf, do_vifs)) {
		if (errno == EOPNOTSUPP)
			smclog(LOG_WARNING, "Parse error in %s", file);
		return EX_CONFIG;
	}

	if (conf_vrfy)
		return EX_OK;

	return script_exec(NULL);
}

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "linux"
 * End:
 */
