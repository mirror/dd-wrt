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
#ifdef HAVE_WIL6210

#define INITVALUECACHEi(prefix)                                  \
	static int devs[8] = { -1, -1, -1, -1, -1, -1, -1, -1 }; \
	int dn, ret = 0;                                         \
	if (!strncmp(prefix, "giwifi", 6))                       \
		dn = 2;                                          \
	else                                                     \
		sscanf(prefix, "wlan%d", &dn);                   \
	if (dn > -1 && (dn > 7 || devs[dn] == -1)) {
#define INITVALUECACHE()                                         \
	static int devs[8] = { -1, -1, -1, -1, -1, -1, -1, -1 }; \
	int dn, ret = 0;                                         \
	if (!strncmp(prefix, "giwifi", 6))                       \
		dn = 2;                                          \
	else                                                     \
		sscanf(prefix, "wlan%d", &dn);                   \
	if (dn > -1 && (dn > 7 || devs[dn] == -1)) {
#else

#define INITVALUECACHEi(prefix)                                  \
	static int devs[8] = { -1, -1, -1, -1, -1, -1, -1, -1 }; \
	int dn, ret = 0;                                         \
	sscanf(prefix, "wlan%d", &dn);                           \
	if (dn > -1 && (dn > 7 || devs[dn] == -1)) {
#define INITVALUECACHE()                                         \
	static int devs[8] = { -1, -1, -1, -1, -1, -1, -1, -1 }; \
	int dn, ret = 0;                                         \
	sscanf(prefix, "wlan%d", &dn);                           \
	if (dn > -1 && (dn > 7 || devs[dn] == -1)) {
#endif
#define EXITVALUECACHE()         \
	}                        \
	else                     \
	{                        \
		return devs[dn]; \
	}                        \
out_cache:;                      \
	if (dn < 8 && dn > -1)   \
		devs[dn] = ret;

#define RETURNVALUE(val)        \
	{                       \
		ret = val;      \
		goto out_cache; \
	}
