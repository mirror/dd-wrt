/*
 * mac80211info.c 
 * Copyright (C) 2010 Christian Scheele <chris@dd-wrt.com>
 * Copyright (C) 2010 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

/* 
 * mostly copied from crda:
 * Copyright (c) 2008, Luis R. Rodriguez <mcgrof@gmail.com>
 * Copyright (c) 2008, Johannes Berg <johannes@sipsolutions.net>
 * Copyright (c) 2008, Michael Green <Michael.Green@Atheros.com>
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "mac80211regulatory.h"

static void *crda_get_file_ptr(__u8 *db, int dblen, int structlen, __be32 ptr)
{
	__u32 p = ntohl(ptr);

	if (p > dblen - structlen) {
		fprintf(stderr, "Invalid database file, bad pointer!\n");
	}

	return (void *)(db + p);
}

static void reg_rule2rd(__u8 *db, int dblen, __be32 ruleptr, struct ieee80211_reg_rule *rd_reg_rule)
{
	struct regdb_file_reg_rule *rule;
	struct regdb_file_freq_range *freq;
	struct regdb_file_power_rule *power;

	struct ieee80211_freq_range *rd_freq_range = &rd_reg_rule->freq_range;
	struct ieee80211_power_rule *rd_power_rule = &rd_reg_rule->power_rule;

	rule = crda_get_file_ptr(db, dblen, sizeof(*rule), ruleptr);
	freq = crda_get_file_ptr(db, dblen, sizeof(*freq), rule->freq_range_ptr);
	power = crda_get_file_ptr(db, dblen, sizeof(*power), rule->power_rule_ptr);

	rd_freq_range->start_freq_khz = ntohl(freq->start_freq);
	rd_freq_range->end_freq_khz = ntohl(freq->end_freq);
	rd_freq_range->max_bandwidth_khz = ntohl(freq->max_bandwidth);

	rd_power_rule->max_antenna_gain = ntohl(power->max_antenna_gain);
	rd_power_rule->max_eirp = ntohl(power->max_eirp);

	rd_reg_rule->flags = ntohl(rule->flags);
}

/* Converts a file regdomain to ieee80211_regdomain, easier to manage */
struct ieee80211_regdomain *country2rd(__u8 *db, int dblen, struct regdb_file_reg_country *country)
{
	struct regdb_file_reg_rules_collection *rcoll;
	struct ieee80211_regdomain *rd;
	int i, num_rules, size_of_rd;

	rcoll = crda_get_file_ptr(db, dblen, sizeof(*rcoll), country->reg_collection_ptr);
	num_rules = ntohl(rcoll->reg_rule_num);
	/* re-get pointer with sanity checking for num_rules */
	rcoll = crda_get_file_ptr(db, dblen, sizeof(*rcoll) + num_rules * sizeof(__be32), country->reg_collection_ptr);

	size_of_rd = sizeof(struct ieee80211_regdomain) + num_rules * sizeof(struct ieee80211_reg_rule);

	rd = malloc(size_of_rd);
	if (!rd)
		return NULL;

	bzero(rd, size_of_rd);

	rd->alpha2[0] = country->alpha2[0];
	rd->alpha2[1] = country->alpha2[1];
	rd->n_reg_rules = num_rules;

	for (i = 0; i < num_rules; i++) {
		reg_rule2rd(db, dblen, rcoll->reg_rule_ptrs[i], &rd->reg_rules[i]);
	}

	return rd;
}

struct ieee80211_regdomain *mac80211_get_regdomain(const char *varcountry)
{
	int fd = -1;
	struct stat stat;
	__u8 *db;
	struct regdb_file_header *header;
	struct regdb_file_reg_country *countries;
	int dblen = 0, siglen = 0, num_countries = 0, i, j;
	char alpha2[2];
	int found_country = 0;

	struct regdb_file_reg_rules_collection *rcoll;
	struct regdb_file_reg_country *country;
	int num_rules;
	struct ieee80211_regdomain *rd = NULL;

	const char *regdb_paths[] = { "/lib/crda/regulatory.bin", NULL };
	const char **regdb = regdb_paths;

	if (!varcountry) {
		fprintf(stderr, "COUNTRY environment variable not set.\n");
		return rd;
	}

	if (!is_valid_regdom(varcountry)) {
		fprintf(stderr, "COUNTRY environment variable must be an "
				"ISO ISO 3166-1-alpha-2 (uppercase) or 00\n");
		return rd;
	}
#ifdef HAVE_IDEXX
#ifdef HAVE_IDEXX_WORLD
	memcpy(alpha2, "00", 2);
#else
	memcpy(alpha2, "US", 2);
#endif
#else
	memcpy(alpha2, varcountry, 2);
#endif

	while (*regdb != NULL) {
		fd = open(*regdb, O_RDONLY);
		if (fd >= 0)
			break;
		regdb++;
	}
	if (fd < 0) {
		perror("failed to open db file");
		return rd;
	}

	if (fstat(fd, &stat)) {
		perror("failed to fstat db file");
		return rd;
	}

	dblen = stat.st_size;

	db = mmap(NULL, dblen, PROT_READ, MAP_PRIVATE, fd, 0);
	if (db == MAP_FAILED) {
		perror("failed to mmap db file");
		if (fd)
			close(fd);
		return rd;
	}

	/* db file starts with a struct regdb_file_header */
	header = crda_get_file_ptr(db, dblen, sizeof(*header), 0);

	if (ntohl(header->magic) != REGDB_MAGIC) {
		fprintf(stderr, "Invalid database magic\n");
		goto out;
	}

	if (ntohl(header->version) != REGDB_VERSION) {
		fprintf(stderr, "Invalid database version\n");
		goto out;
	}

	siglen = ntohl(header->signature_length);
	/* adjust dblen so later sanity checks don't run into the signature */
	dblen -= siglen;

	if (dblen <= (int)sizeof(*header)) {
		fprintf(stderr, "Invalid signature length %d\n", siglen);
		goto out;
	}

	num_countries = ntohl(header->reg_country_num);
	countries = crda_get_file_ptr(db, dblen, sizeof(struct regdb_file_reg_country) * num_countries, header->reg_country_ptr);

	for (i = 0; i < num_countries; i++) {
		country = countries + i;
		if (memcmp(country->alpha2, alpha2, 2) == 0) {
			found_country = 1;
			break;
		}
	}

	if (!found_country) {
		fprintf(stderr, "No country match in regulatory database.\n");
		goto out;
	}
	rd = country2rd(db, dblen, country);
out:
	if (munmap(db, dblen + siglen) == -1) {
		fprintf(stderr, "mac80211regulatory failed to munmap crda database\n");
	}
	if (fd)
		close(fd);
	return rd;
}
