/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2015 Derick Rethans
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "timelib.h"

#ifdef HAVE_SYSTEM_TZDATA
#include <sys/mman.h>
#include <sys/stat.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>

#include "php_scandir.h"
#endif

#include <stdio.h>

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#else
#include <strings.h>
#endif

#ifndef HAVE_SYSTEM_TZDATA
#define TIMELIB_SUPPORTS_V2DATA
#include "timezonedb.h"
#endif

#include <ctype.h>

#if (defined(__APPLE__) || defined(__APPLE_CC__)) && (defined(__BIG_ENDIAN__) || defined(__LITTLE_ENDIAN__))
# if defined(__LITTLE_ENDIAN__)
#  undef WORDS_BIGENDIAN
# else
#  if defined(__BIG_ENDIAN__)
#   define WORDS_BIGENDIAN
#  endif
# endif
#endif

#ifdef WORDS_BIGENDIAN
#define timelib_conv_int(l) (l)
#else
#define timelib_conv_int(l) ((l & 0x000000ff) << 24) + ((l & 0x0000ff00) << 8) + ((l & 0x00ff0000) >> 8) + ((l & 0xff000000) >> 24)
#endif

static int read_preamble(const unsigned char **tzf, timelib_tzinfo *tz)
{
	uint32_t version;

	if (memcmp(*tzf, "TZif", 4) == 0) {
		*tzf += 20;
		return 0;
	}

	/* read ID */
	version = (*tzf)[3] - '0';
	*tzf += 4;

	/* read BC flag */
	tz->bc = (**tzf == '\1');
	*tzf += 1;

	/* read country code */
	memcpy(tz->location.country_code, *tzf, 2);
	tz->location.country_code[2] = '\0';
	*tzf += 2;

	/* skip rest of preamble */
	*tzf += 13;

	return version;
}

static void read_header(const unsigned char **tzf, timelib_tzinfo *tz)
{
	uint32_t buffer[6];

	memcpy(&buffer, *tzf, sizeof(buffer));
	tz->bit32.ttisgmtcnt = timelib_conv_int(buffer[0]);
	tz->bit32.ttisstdcnt = timelib_conv_int(buffer[1]);
	tz->bit32.leapcnt    = timelib_conv_int(buffer[2]);
	tz->bit32.timecnt    = timelib_conv_int(buffer[3]);
	tz->bit32.typecnt    = timelib_conv_int(buffer[4]);
	tz->bit32.charcnt    = timelib_conv_int(buffer[5]);
	*tzf += sizeof(buffer);
}

static void skip_64bit_transistions(const unsigned char **tzf, timelib_tzinfo *tz)
{
	if (tz->bit64.timecnt) {
		*tzf += (sizeof(int64_t) * tz->bit64.timecnt);
		*tzf += (sizeof(unsigned char) * tz->bit64.timecnt);
	}
}

static void read_transistions(const unsigned char **tzf, timelib_tzinfo *tz)
{
	int32_t *buffer = NULL;
	uint32_t i;
	unsigned char *cbuffer = NULL;

	if (tz->bit32.timecnt) {
		buffer = (int32_t*) timelib_malloc(tz->bit32.timecnt * sizeof(int32_t));
		if (!buffer) {
			return;
		}
		memcpy(buffer, *tzf, sizeof(int32_t) * tz->bit32.timecnt);
		*tzf += (sizeof(int32_t) * tz->bit32.timecnt);
		for (i = 0; i < tz->bit32.timecnt; i++) {
			buffer[i] = timelib_conv_int(buffer[i]);
		}

		cbuffer = (unsigned char*) timelib_malloc(tz->bit32.timecnt * sizeof(unsigned char));
		if (!cbuffer) {
			timelib_free(buffer);
			return;
		}
		memcpy(cbuffer, *tzf, sizeof(unsigned char) * tz->bit32.timecnt);
		*tzf += sizeof(unsigned char) * tz->bit32.timecnt;
	}

	tz->trans = buffer;
	tz->trans_idx = cbuffer;
}

static void skip_64bit_types(const unsigned char **tzf, timelib_tzinfo *tz)
{
	*tzf += sizeof(unsigned char) * 6 * tz->bit64.typecnt;
	*tzf += sizeof(char) * tz->bit64.charcnt;
	if (tz->bit64.leapcnt) {
		*tzf += sizeof(int64_t) * tz->bit64.leapcnt * 2;
	}
	if (tz->bit64.ttisstdcnt) {
		*tzf += sizeof(unsigned char) * tz->bit64.ttisstdcnt;
	}
	if (tz->bit64.ttisgmtcnt) {
		*tzf += sizeof(unsigned char) * tz->bit64.ttisgmtcnt;
	}
}

static void read_types(const unsigned char **tzf, timelib_tzinfo *tz)
{
	unsigned char *buffer;
	int32_t *leap_buffer;
	unsigned int i, j;

	buffer = (unsigned char*) timelib_malloc(tz->bit32.typecnt * sizeof(unsigned char) * 6);
	if (!buffer) {
		return;
	}
	memcpy(buffer, *tzf, sizeof(unsigned char) * 6 * tz->bit32.typecnt);
	*tzf += sizeof(unsigned char) * 6 * tz->bit32.typecnt;

	tz->type = (ttinfo*) timelib_malloc(tz->bit32.typecnt * sizeof(struct ttinfo));
	if (!tz->type) {
		timelib_free(buffer);
		return;
	}

	for (i = 0; i < tz->bit32.typecnt; i++) {
		j = i * 6;
		tz->type[i].offset = (buffer[j] * 16777216) + (buffer[j + 1] * 65536) + (buffer[j + 2] * 256) + buffer[j + 3];
		tz->type[i].isdst = buffer[j + 4];
		tz->type[i].abbr_idx = buffer[j + 5];
	}
	timelib_free(buffer);

	tz->timezone_abbr = (char*) timelib_malloc(tz->bit32.charcnt);
	if (!tz->timezone_abbr) {
		return;
	}
	memcpy(tz->timezone_abbr, *tzf, sizeof(char) * tz->bit32.charcnt);
	*tzf += sizeof(char) * tz->bit32.charcnt;

	if (tz->bit32.leapcnt) {
		leap_buffer = (int32_t *) timelib_malloc(tz->bit32.leapcnt * 2 * sizeof(int32_t));
		if (!leap_buffer) {
			return;
		}
		memcpy(leap_buffer, *tzf, sizeof(int32_t) * tz->bit32.leapcnt * 2);
		*tzf += sizeof(int32_t) * tz->bit32.leapcnt * 2;

		tz->leap_times = (tlinfo*) timelib_malloc(tz->bit32.leapcnt * sizeof(tlinfo));
		if (!tz->leap_times) {
			timelib_free(leap_buffer);
			return;
		}
		for (i = 0; i < tz->bit32.leapcnt; i++) {
			tz->leap_times[i].trans = timelib_conv_int(leap_buffer[i * 2]);
			tz->leap_times[i].offset = timelib_conv_int(leap_buffer[i * 2 + 1]);
		}
		timelib_free(leap_buffer);
	}

	if (tz->bit32.ttisstdcnt) {
		buffer = (unsigned char*) timelib_malloc(tz->bit32.ttisstdcnt * sizeof(unsigned char));
		if (!buffer) {
			return;
		}
		memcpy(buffer, *tzf, sizeof(unsigned char) * tz->bit32.ttisstdcnt);
		*tzf += sizeof(unsigned char) * tz->bit32.ttisstdcnt;

		for (i = 0; i < tz->bit32.ttisstdcnt; i++) {
			tz->type[i].isstdcnt = buffer[i];
		}
		timelib_free(buffer);
	}

	if (tz->bit32.ttisgmtcnt) {
		buffer = (unsigned char*) timelib_malloc(tz->bit32.ttisgmtcnt * sizeof(unsigned char));
		if (!buffer) {
			return;
		}
		memcpy(buffer, *tzf, sizeof(unsigned char) * tz->bit32.ttisgmtcnt);
		*tzf += sizeof(unsigned char) * tz->bit32.ttisgmtcnt;

		for (i = 0; i < tz->bit32.ttisgmtcnt; i++) {
			tz->type[i].isgmtcnt = buffer[i];
		}
		timelib_free(buffer);
	}
}

static void skip_posix_string(const unsigned char **tzf, timelib_tzinfo *tz)
{
	int n_count = 0;

	do {
		if (*tzf[0] == '\n') {
			n_count++;
		}
		(*tzf)++;
	} while (n_count < 2);
}

static void read_location(const unsigned char **tzf, timelib_tzinfo *tz)
{
	uint32_t buffer[3];
	uint32_t comments_len;

	memcpy(&buffer, *tzf, sizeof(buffer));
	tz->location.latitude = timelib_conv_int(buffer[0]);
	tz->location.latitude = (tz->location.latitude / 100000) - 90;
	tz->location.longitude = timelib_conv_int(buffer[1]);
	tz->location.longitude = (tz->location.longitude / 100000) - 180;
	comments_len = timelib_conv_int(buffer[2]);
	*tzf += sizeof(buffer);

	tz->location.comments = timelib_malloc(comments_len + 1);
	memcpy(tz->location.comments, *tzf, comments_len);
	tz->location.comments[comments_len] = '\0';
	*tzf += comments_len;
}

void timelib_dump_tzinfo(timelib_tzinfo *tz)
{
	uint32_t i;

	printf("Country Code:      %s\n", tz->location.country_code);
	printf("Geo Location:      %f,%f\n", tz->location.latitude, tz->location.longitude);
	printf("Comments:\n%s\n",          tz->location.comments);
	printf("BC:                %s\n",  tz->bc ? "" : "yes");
	printf("UTC/Local count:   " TIMELIB_ULONG_FMT "\n", (timelib_ulong) tz->bit32.ttisgmtcnt);
	printf("Std/Wall count:    " TIMELIB_ULONG_FMT "\n", (timelib_ulong) tz->bit32.ttisstdcnt);
	printf("Leap.sec. count:   " TIMELIB_ULONG_FMT "\n", (timelib_ulong) tz->bit32.leapcnt);
	printf("Trans. count:      " TIMELIB_ULONG_FMT "\n", (timelib_ulong) tz->bit32.timecnt);
	printf("Local types count: " TIMELIB_ULONG_FMT "\n", (timelib_ulong) tz->bit32.typecnt);
	printf("Zone Abbr. count:  " TIMELIB_ULONG_FMT "\n", (timelib_ulong) tz->bit32.charcnt);

	printf ("%8s (%12s) = %3d [%5ld %1d %3d '%s' (%d,%d)]\n",
		"", "", 0,
		(long int) tz->type[0].offset,
		tz->type[0].isdst,
		tz->type[0].abbr_idx,
		&tz->timezone_abbr[tz->type[0].abbr_idx],
		tz->type[0].isstdcnt,
		tz->type[0].isgmtcnt
		);
	for (i = 0; i < tz->bit32.timecnt; i++) {
		printf ("%08X (%12d) = %3d [%5ld %1d %3d '%s' (%d,%d)]\n",
			tz->trans[i], tz->trans[i], tz->trans_idx[i],
			(long int) tz->type[tz->trans_idx[i]].offset,
			tz->type[tz->trans_idx[i]].isdst,
			tz->type[tz->trans_idx[i]].abbr_idx,
			&tz->timezone_abbr[tz->type[tz->trans_idx[i]].abbr_idx],
			tz->type[tz->trans_idx[i]].isstdcnt,
			tz->type[tz->trans_idx[i]].isgmtcnt
			);
	}
	for (i = 0; i < tz->bit32.leapcnt; i++) {
		printf ("%08X (%12ld) = %d\n",
			tz->leap_times[i].trans,
			(long) tz->leap_times[i].trans,
			tz->leap_times[i].offset);
	}
}

#ifdef HAVE_SYSTEM_TZDATA

#ifdef HAVE_SYSTEM_TZDATA_PREFIX
#define ZONEINFO_PREFIX HAVE_SYSTEM_TZDATA_PREFIX
#else
#define ZONEINFO_PREFIX "/usr/share/zoneinfo"
#endif

/* System timezone database pointer. */
static const timelib_tzdb *timezonedb_system;

/* Hash table entry for the cache of the zone.tab mapping table. */
struct location_info {
        char code[2];
        double latitude, longitude;
        char name[64];
        char *comment;
        struct location_info *next;
};

/* Cache of zone.tab. */
static struct location_info **system_location_table;

/* Size of the zone.tab hash table; a random-ish prime big enough to
 * prevent too many collisions. */
#define LOCINFO_HASH_SIZE (1021)

/* Compute a case insensitive hash of str */
static uint32_t tz_hash(const char *str)
{
    const unsigned char *p = (const unsigned char *)str;
    uint32_t hash = 5381;
    int c;

    while ((c = tolower(*p++)) != '\0') {
        hash = (hash << 5) ^ hash ^ c;
    }

    return hash % LOCINFO_HASH_SIZE;
}

/* Parse an ISO-6709 date as used in zone.tab. Returns end of the
 * parsed string on success, or NULL on parse error.  On success,
 * writes the parsed number to *result. */
static char *parse_iso6709(char *p, double *result)
{
    double v, sign;
    char *pend;
    size_t len;

    if (*p == '+')
        sign = 1.0;
    else if (*p == '-')
        sign = -1.0;
    else
        return NULL;

    p++;
    for (pend = p; *pend >= '0' && *pend <= '9'; pend++)
        ;;

    /* Annoying encoding used by zone.tab has no decimal point, so use
     * the length to determine the format:
     * 
     * 4 = DDMM
     * 5 = DDDMM
     * 6 = DDMMSS
     * 7 = DDDMMSS
     */
    len = pend - p;
    if (len < 4 || len > 7) {
        return NULL;
    }

    /* p => [D]DD */
    v = (p[0] - '0') * 10.0 + (p[1] - '0');
    p += 2;
    if (len == 5 || len == 7)
        v = v * 10.0 + (*p++ - '0');
    /* p => MM[SS] */
    v += (10.0 * (p[0] - '0')
          + p[1] - '0') / 60.0;
    p += 2;
    /* p => [SS] */
    if (len > 5) {
        v += (10.0 * (p[0] - '0')
              + p[1] - '0') / 3600.0;
        p += 2;
    }

    /* Round to five decimal place, not because it's a good idea,
     * but, because the builtin data uses rounded data, so, match
     * that. */
    *result = trunc(v * sign * 100000.0) / 100000.0;

    return p;
}

/* This function parses the zone.tab file to build up the mapping of
 * timezone to country code and geographic location, and returns a
 * hash table.  The hash table is indexed by the function:
 *
 *   tz_hash(timezone-name)
 */
static struct location_info **create_location_table(void)
{
    struct location_info **li, *i;
    char zone_tab[PATH_MAX];
    char line[512];
    FILE *fp;

    strncpy(zone_tab, ZONEINFO_PREFIX "/zone.tab", sizeof zone_tab);

    fp = fopen(zone_tab, "r");
    if (!fp) {
        return NULL;
    }

    li = calloc(LOCINFO_HASH_SIZE, sizeof *li);

    while (fgets(line, sizeof line, fp)) {
        char *p = line, *code, *name, *comment;
        uint32_t hash;
        double latitude, longitude;

        while (isspace(*p))
            p++;

        if (*p == '#' || *p == '\0' || *p == '\n')
            continue;
        
        if (!isalpha(p[0]) || !isalpha(p[1]) || p[2] != '\t')
            continue;
        
        /* code => AA */
        code = p;
        p[2] = 0;
        p += 3;

        /* coords => [+-][D]DDMM[SS][+-][D]DDMM[SS] */
        p = parse_iso6709(p, &latitude);
        if (!p) {
            continue;
        }
        p = parse_iso6709(p, &longitude);
        if (!p) {
            continue;
        }

        if (!p || *p != '\t') {
            continue;
        }

        /* name = string */
        name = ++p;
        while (*p != '\t' && *p && *p != '\n')
            p++;

        *p++ = '\0';

        /* comment = string */
        comment = p;
        while (*p != '\t' && *p && *p != '\n')
            p++;

        if (*p == '\n' || *p == '\t')
            *p = '\0';
        
        hash = tz_hash(name);
        i = malloc(sizeof *i);
        memcpy(i->code, code, 2);
        strncpy(i->name, name, sizeof i->name);
        i->comment = strdup(comment);
        i->longitude = longitude;
        i->latitude = latitude;
        i->next = li[hash];
        li[hash] = i;
        /* printf("%s [%u, %f, %f]\n", name, hash, latitude, longitude); */
    }

    fclose(fp);

    return li;
}

/* Return location info from hash table, using given timezone name.
 * Returns NULL if the name could not be found. */
const struct location_info *find_zone_info(struct location_info **li, 
                                           const char *name)
{
    uint32_t hash = tz_hash(name);
    const struct location_info *l;

    if (!li) {
        return NULL;
    }

    for (l = li[hash]; l; l = l->next) {
        if (strcasecmp(l->name, name) == 0)
            return l;
    }

    return NULL;
}    

/* Filter out some non-tzdata files and the posix/right databases, if
 * present. */
static int index_filter(const struct dirent *ent)
{
	return strcmp(ent->d_name, ".") != 0
		&& strcmp(ent->d_name, "..") != 0
		&& strcmp(ent->d_name, "posix") != 0
		&& strcmp(ent->d_name, "posixrules") != 0
		&& strcmp(ent->d_name, "right") != 0
		&& strstr(ent->d_name, ".list") == NULL
		&& strstr(ent->d_name, ".tab") == NULL;
}

static int sysdbcmp(const void *first, const void *second)
{
        const timelib_tzdb_index_entry *alpha = first, *beta = second;

        return strcasecmp(alpha->id, beta->id);
}


/* Create the zone identifier index by trawling the filesystem. */
static void create_zone_index(timelib_tzdb *db)
{
	size_t dirstack_size,  dirstack_top;
	size_t index_size, index_next;
	timelib_tzdb_index_entry *db_index;
	char **dirstack;

	/* LIFO stack to hold directory entries to scan; each slot is a
	 * directory name relative to the zoneinfo prefix. */
	dirstack_size = 32;
	dirstack = malloc(dirstack_size * sizeof *dirstack);
	dirstack_top = 1;
	dirstack[0] = strdup("");
	
	/* Index array. */
	index_size = 64;
	db_index = malloc(index_size * sizeof *db_index);
	index_next = 0;

	do {
		struct dirent **ents;
		char name[PATH_MAX], *top;
		int count;

		/* Pop the top stack entry, and iterate through its contents. */
		top = dirstack[--dirstack_top];
		snprintf(name, sizeof name, ZONEINFO_PREFIX "/%s", top);

		count = php_scandir(name, &ents, index_filter, php_alphasort);

		while (count > 0) {
			struct stat st;
			const char *leaf = ents[count - 1]->d_name;

			snprintf(name, sizeof name, ZONEINFO_PREFIX "/%s/%s", 
				 top, leaf);
			
			if (strlen(name) && stat(name, &st) == 0) {
				/* Name, relative to the zoneinfo prefix. */
				const char *root = top;

				if (root[0] == '/') root++;

				snprintf(name, sizeof name, "%s%s%s", root, 
					 *root ? "/": "", leaf);

				if (S_ISDIR(st.st_mode)) {
					if (dirstack_top == dirstack_size) {
						dirstack_size *= 2;
						dirstack = realloc(dirstack, 
								   dirstack_size * sizeof *dirstack);
					}
					dirstack[dirstack_top++] = strdup(name);
				}
				else {
					if (index_next == index_size) {
						index_size *= 2;
						db_index = realloc(db_index,
								   index_size * sizeof *db_index);
					}

					db_index[index_next++].id = strdup(name);
				}
			}

			free(ents[--count]);
		}
		
		if (count != -1) free(ents);
		free(top);
	} while (dirstack_top);

        qsort(db_index, index_next, sizeof *db_index, sysdbcmp);

	db->index = db_index;
	db->index_size = index_next;

	free(dirstack);
}

#define FAKE_HEADER "1234\0??\1??"
#define FAKE_UTC_POS (7 - 4)

/* Create a fake data segment for database 'sysdb'. */
static void fake_data_segment(timelib_tzdb *sysdb,
                              struct location_info **info)
{
        size_t n;
        char *data, *p;
        
        data = malloc(3 * sysdb->index_size + 7);

        p = mempcpy(data, FAKE_HEADER, sizeof(FAKE_HEADER) - 1);

        for (n = 0; n < sysdb->index_size; n++) {
                const struct location_info *li;
                timelib_tzdb_index_entry *ent;

                ent = (timelib_tzdb_index_entry *)&sysdb->index[n];

                /* Lookup the timezone name in the hash table. */
                if (strcmp(ent->id, "UTC") == 0) {
                        ent->pos = FAKE_UTC_POS;
                        continue;
                }

                li = find_zone_info(info, ent->id);
                if (li) {
                        /* If found, append the BC byte and the
                         * country code; set the position for this
                         * section of timezone data.  */
                        ent->pos = (p - data) - 4;
                        *p++ = '\1';
                        *p++ = li->code[0];
                        *p++ = li->code[1];
                }
                else {
                        /* If not found, the timezone data can
                         * point at the header. */
                        ent->pos = 0;
                }
        }
        
        sysdb->data = (unsigned char *)data;
}

/* Returns true if the passed-in stat structure describes a
 * probably-valid timezone file. */
static int is_valid_tzfile(const struct stat *st, int fd)
{
	if (fd) {
		char buf[20];
		if (read(fd, buf, 20)!=20) {
			return 0;
		}
		lseek(fd, SEEK_SET, 0);
		if (memcmp(buf, "TZif", 4)) {
			return 0;
		}
	}
	return S_ISREG(st->st_mode) && st->st_size > 20;
}

/* To allow timezone names to be used case-insensitively, find the
 * canonical name for this timezone, if possible. */
static const char *canonical_tzname(const char *timezone)
{
    if (timezonedb_system) {
        timelib_tzdb_index_entry *ent, lookup;

        lookup.id = (char *)timezone;

        ent = bsearch(&lookup, timezonedb_system->index,
                      timezonedb_system->index_size, sizeof lookup,
                      sysdbcmp);
        if (ent) {
            return ent->id;
        }
    }

    return timezone;
}

/* Return the mmap()ed tzfile if found, else NULL.  On success, the
 * length of the mapped data is placed in *length. */
static char *map_tzfile(const char *timezone, size_t *length)
{
	char fname[PATH_MAX];
	struct stat st;
	char *p;
	int fd;
	
	if (timezone[0] == '\0' || strstr(timezone, "..") != NULL) {
		return NULL;
	}

	snprintf(fname, sizeof fname, ZONEINFO_PREFIX "/%s", canonical_tzname(timezone));

	fd = open(fname, O_RDONLY);
	if (fd == -1) {
		return NULL;
	} else if (fstat(fd, &st) != 0 || !is_valid_tzfile(&st, fd)) {
		close(fd);
		return NULL;
	}

	*length = st.st_size;
	p = mmap(NULL, st.st_size, PROT_READ, MAP_SHARED, fd, 0);
	close(fd);
	
	return p != MAP_FAILED ? p : NULL;
}

#endif

static int inmem_seek_to_tz_position(const unsigned char **tzf, char *timezone, const timelib_tzdb *tzdb)
{
	int left = 0, right = tzdb->index_size - 1;
#ifdef HAVE_SETLOCALE
	char *cur_locale = NULL, *tmp;

	tmp = setlocale(LC_CTYPE, NULL);
	if (tmp) {
		cur_locale = timelib_strdup(tmp);
	}
	setlocale(LC_CTYPE, "C");
#endif

	do {
		int mid = ((unsigned)left + right) >> 1;
		int cmp = strcasecmp(timezone, tzdb->index[mid].id);

		if (cmp < 0) {
			right = mid - 1;
		} else if (cmp > 0) {
			left = mid + 1;
		} else { /* (cmp == 0) */
			(*tzf) = &(tzdb->data[tzdb->index[mid].pos]);
#ifdef HAVE_SETLOCALE
			setlocale(LC_CTYPE, cur_locale);
			if (cur_locale) timelib_free(cur_locale);
#endif
			return 1;
		}

	} while (left <= right);

#ifdef HAVE_SETLOCALE
	setlocale(LC_CTYPE, cur_locale);
	if (cur_locale) timelib_free(cur_locale);
#endif
	return 0;
}

static int seek_to_tz_position(const unsigned char **tzf, char *timezone,
			       char **map, size_t *maplen,
			       const timelib_tzdb *tzdb)
{
#ifdef HAVE_SYSTEM_TZDATA
	if (tzdb == timezonedb_system) {
		char *orig;

		orig = map_tzfile(timezone, maplen);
		if (orig == NULL) {
			return 0;
		}

		(*tzf) = (unsigned char *)orig;
		*map = orig;
        return 1;
	}
	else
#endif
	{
		return inmem_seek_to_tz_position(tzf, timezone, tzdb);
	}
}

const timelib_tzdb *timelib_builtin_db(void)
{
#ifdef HAVE_SYSTEM_TZDATA
	if (timezonedb_system == NULL) {
		timelib_tzdb *tmp = malloc(sizeof *tmp);

		tmp->version = "0.system";
		tmp->data = NULL;
		create_zone_index(tmp);
		system_location_table = create_location_table();
		fake_data_segment(tmp, system_location_table);
		timezonedb_system = tmp;
	}

	return timezonedb_system;
#else
	return &timezonedb_builtin;
#endif
}

const timelib_tzdb_index_entry *timelib_timezone_builtin_identifiers_list(int *count)
{
#ifdef HAVE_SYSTEM_TZDATA
	*count = timezonedb_system->index_size;
	return timezonedb_system->index;
#else
	*count = sizeof(timezonedb_idx_builtin) / sizeof(*timezonedb_idx_builtin);
	return timezonedb_idx_builtin;
#endif
}

int timelib_timezone_id_is_valid(char *timezone, const timelib_tzdb *tzdb)
{
	const unsigned char *tzf;

#ifdef HAVE_SYSTEM_TZDATA
	if (tzdb == timezonedb_system) {
		char fname[PATH_MAX];
		struct stat st;

		if (timezone[0] == '\0' || strstr(timezone, "..") != NULL) {
			return 0;
		}

		if (system_location_table) {
			if (find_zone_info(system_location_table, timezone) != NULL) {
				/* found in cache */
				return 1;
			}
		}

		snprintf(fname, sizeof fname, ZONEINFO_PREFIX "/%s", canonical_tzname(timezone));

		return stat(fname, &st) == 0 && is_valid_tzfile(&st, 0);
	}
#endif

	return (inmem_seek_to_tz_position(&tzf, timezone, tzdb));
}

static void skip_64bit_preamble(const unsigned char **tzf, timelib_tzinfo *tz)
{
	*tzf += 20;
}

static void read_64bit_header(const unsigned char **tzf, timelib_tzinfo *tz)
{
	uint32_t buffer[6];

	memcpy(&buffer, *tzf, sizeof(buffer));
	tz->bit64.ttisgmtcnt = timelib_conv_int(buffer[0]);
	tz->bit64.ttisstdcnt = timelib_conv_int(buffer[1]);
	tz->bit64.leapcnt    = timelib_conv_int(buffer[2]);
	tz->bit64.timecnt    = timelib_conv_int(buffer[3]);
	tz->bit64.typecnt    = timelib_conv_int(buffer[4]);
	tz->bit64.charcnt    = timelib_conv_int(buffer[5]);
	*tzf += sizeof(buffer);
}

timelib_tzinfo *timelib_parse_tzfile(char *timezone, const timelib_tzdb *tzdb)
{
	const unsigned char *tzf;
	char *memmap = NULL;
	size_t maplen;
	timelib_tzinfo *tmp;
	int version;

	if (seek_to_tz_position(&tzf, timezone, &memmap, &maplen, tzdb)) {
		tmp = timelib_tzinfo_ctor(timezone);

		version = read_preamble(&tzf, tmp);
		read_header(&tzf, tmp);
		read_transistions(&tzf, tmp);
		read_types(&tzf, tmp);

#ifdef HAVE_SYSTEM_TZDATA
		if (memmap) {
			const struct location_info *li;

			/* TZif-style - grok the location info from the system database,
			 * if possible. */

			if ((li = find_zone_info(system_location_table, timezone)) != NULL) {
				tmp->location.comments = timelib_strdup(li->comment);
				strncpy(tmp->location.country_code, li->code, 2);
				tmp->location.longitude = li->longitude;
				tmp->location.latitude = li->latitude;
				tmp->bc = 1;
			}
			else {
				strcpy(tmp->location.country_code, "??");
				tmp->bc = 0;
				tmp->location.comments = timelib_strdup("");
			}

			/* Now done with the mmap segment - discard it. */
			munmap(memmap, maplen);
		} else
#endif
		{
			/* PHP-style - use the embedded info. */
			if (version == 2) {
				skip_64bit_preamble(&tzf, tmp);
				read_64bit_header(&tzf, tmp);
				skip_64bit_transistions(&tzf, tmp);
				skip_64bit_types(&tzf, tmp);
				skip_posix_string(&tzf, tmp);
			}
			read_location(&tzf, tmp);
 		}
	} else {
		tmp = NULL;
	}

	return tmp;
}

static ttinfo* fetch_timezone_offset(timelib_tzinfo *tz, timelib_sll ts, timelib_sll *transition_time)
{
	uint32_t i;

	/* If there is no transition time, we pick the first one, if that doesn't
	 * exist we return NULL */
	if (!tz->bit32.timecnt || !tz->trans) {
		*transition_time = 0;
		if (tz->bit32.typecnt == 1) {
			return &(tz->type[0]);
		}
		return NULL;
	}

	/* If the TS is lower than the first transition time, then we scan over
	 * all the transition times to find the first non-DST one, or the first
	 * one in case there are only DST entries. Not sure which smartass came up
	 * with this idea in the first though :) */
	if (ts < tz->trans[0]) {
		uint32_t j;

		*transition_time = 0;
		j = 0;
		while (j < tz->bit32.timecnt && tz->type[tz->trans_idx[j]].isdst) {
			++j;
		}
		if (j == tz->bit32.timecnt) {
			j = 0;
		}
		return &(tz->type[tz->trans_idx[j]]);
	}

	/* In all other cases we loop through the available transtion times to find
	 * the correct entry */
	for (i = 0; i < tz->bit32.timecnt; i++) {
		if (ts < tz->trans[i]) {
			*transition_time = tz->trans[i - 1];
			return &(tz->type[tz->trans_idx[i - 1]]);
		}
	}
	*transition_time = tz->trans[tz->bit32.timecnt - 1];
	return &(tz->type[tz->trans_idx[tz->bit32.timecnt - 1]]);
}

static tlinfo* fetch_leaptime_offset(timelib_tzinfo *tz, timelib_sll ts)
{
	int i;

	if (!tz->bit32.leapcnt || !tz->leap_times) {
		return NULL;
	}

	for (i = tz->bit32.leapcnt - 1; i > 0; i--) {
		if (ts > tz->leap_times[i].trans) {
			return &(tz->leap_times[i]);
		}
	}
	return NULL;
}

int timelib_timestamp_is_in_dst(timelib_sll ts, timelib_tzinfo *tz)
{
	ttinfo *to;
	timelib_sll dummy;

	if ((to = fetch_timezone_offset(tz, ts, &dummy))) {
		return to->isdst;
	}
	return -1;
}

timelib_time_offset *timelib_get_time_zone_info(timelib_sll ts, timelib_tzinfo *tz)
{
	ttinfo *to;
	tlinfo *tl;
	int32_t offset = 0, leap_secs = 0;
	char *abbr;
	timelib_time_offset *tmp = timelib_time_offset_ctor();
	timelib_sll                transistion_time;

	if ((to = fetch_timezone_offset(tz, ts, &transistion_time))) {
		offset = to->offset;
		abbr = &(tz->timezone_abbr[to->abbr_idx]);
		tmp->is_dst = to->isdst;
		tmp->transistion_time = transistion_time;
	} else {
		offset = 0;
		abbr = tz->timezone_abbr;
		tmp->is_dst = 0;
		tmp->transistion_time = 0;
	}

	if ((tl = fetch_leaptime_offset(tz, ts))) {
		leap_secs = -tl->offset;
	}

	tmp->offset = offset;
	tmp->leap_secs = leap_secs;
	tmp->abbr = abbr ? timelib_strdup(abbr) : timelib_strdup("GMT");

	return tmp;
}

timelib_sll timelib_get_current_offset(timelib_time *t)
{
	timelib_time_offset *gmt_offset;
	timelib_sll retval;

	switch (t->zone_type) {
		case TIMELIB_ZONETYPE_ABBR:
		case TIMELIB_ZONETYPE_OFFSET:
			return (t->z + t->dst) * -60;

		case TIMELIB_ZONETYPE_ID:
			gmt_offset = timelib_get_time_zone_info(t->sse, t->tz_info);
			retval = gmt_offset->offset;
			timelib_time_offset_dtor(gmt_offset);
			return retval;

		default:
			return 0;
	}
}
