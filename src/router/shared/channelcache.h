/*
 * channelcache.h
 * 
 * Copyright (C) 2017 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <malloc.h>

struct channellist_cache {
	struct wifi_channels *list;
	char *ifname;
	char *country;
};

static struct channellist_cache *cache = NULL;
static int cachecount = 0;
static void addcache(const char *ifname, const char *country, struct wifi_channels *list)
{
	if (cache) {
		int cnt = 0;
		for (cnt = 0; cnt < cachecount; cnt++) {
			if (!strcmp(cache[cnt].ifname, ifname) && !strcmp(cache[cnt].country, country)) {
				return;
			}
			if (!strcmp(cache[cnt].ifname, ifname)) {
				free(cache[cnt].list);
				cache[cnt].list = list;
				free(cache[cnt].country);
				cache[cnt].country = strdup(country);
				return;
			}
		}
	}
	cache = realloc(cache, sizeof(struct channellist_cache) * (cachecount + 1));
	cache[cachecount].ifname = strdup(ifname);
	cache[cachecount].country = strdup(country);
	cache[cachecount].list = list;
	cachecount++;
}

static struct wifi_channels *getcache(const char *ifname, const char *country)
{
	if (cache) {
		int cnt = 0;
		for (cnt = 0; cnt < cachecount; cnt++) {
			if (!strcmp(cache[cnt].ifname, ifname) && !strcmp(cache[cnt].country, country)) {
				return cache[cnt].list;
			}
		}
	}
	return NULL;
}

static void _invalidate_channelcache(void)
{
	if (cache) {
		struct channellist_cache *tmpcache = cache;
		int c = cachecount;
		cachecount = 0;
		cache = NULL;
		int cnt = 0;
		for (cnt = 0; cnt < c; cnt++) {
			free(tmpcache[cnt].list);
			free(tmpcache[cnt].ifname);
			free(tmpcache[cnt].country);
		}
		free(tmpcache);
	}
}

#define INITVALUECACHEi(prefix)         \
	static unsigned int __set = 0; \
	static unsigned int __val = 0; \
	int dn, ret = 0;                \
	if (sscanf(prefix, "wlan%d", &dn) == 1 && (__set & (1 << dn))) {
#define INITVALUECACHE()                \
	static unsigned int __set = 0; \
	static unsigned int __val = 0; \
	int dn, ret = 0;                \
	if (sscanf(prefix, "wlan%d", &dn) == 1 && (__set & (1 << dn))) {
#define EXITVALUECACHE()                    \
	}                                   \
	else                                \
	{                                   \
		return (__val & (1 << dn)); \
	}                                   \
out_cache:;                                 \
	if (dn < 8 && dn > -1) {            \
		__set |= 1 << dn;           \
		__val |= (!!ret) << dn;     \
	}
#define RETURNVALUE(val)        \
	{                       \
		ret = val;      \
		goto out_cache; \
	}
