/*
 *
 *   Authors:
 *    Lars Fenneberg		<lf@elemental.net>
 *
 *   This software is Copyright 1996,1997 by the above mentioned author(s),
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <reubenhwk@gmail.com>.
 *
 */

#include "config.h"
#include "includes.h"
#include "radvd.h"

#ifdef UNIT_TEST
#include "test/util.c"
#endif

struct safe_buffer *new_safe_buffer(void)
{
	struct safe_buffer *sb = malloc(sizeof(struct safe_buffer));
	*sb = SAFE_BUFFER_INIT;
	sb->should_free = 1;
	return sb;
}

void safe_buffer_free(struct safe_buffer *sb)
{
	if (sb->buffer) {
		free(sb->buffer);
	}

	if (sb->should_free) {
		free(sb);
	}
}

/**
 * Requests that the safe_buffer capacity be least n bytes in size.
 *
 * If n is greater than the current capacity, the function causes the container
 * to reallocate its storage increasing its capacity to n (or greater).
 *
 * In all other cases, the function call does not cause a reallocation and the
 * capacity is not affected.
 *
 * @param sb safe_buffer to enlarge
 * @param b Minimum capacity for the safe_buffer.
 */
void safe_buffer_resize(struct safe_buffer *sb, size_t n)
{
	const int blocksize = 1 << 6; // MUST BE POWER OF 2.
	if (sb->allocated < n) {
		if (n % blocksize > 0) {
			n |= (blocksize - 1); // Set all the low bits
			n++;
		}
		if (n > 64 * 1024) {
			flog(LOG_ERR, "Requested buffer too large for any possible IPv6 ND, even with jumbogram.  Exiting.");
			exit(1);
		}
		sb->allocated = n;
		sb->buffer = realloc(sb->buffer, sb->allocated);
	}
}

size_t safe_buffer_pad(struct safe_buffer *sb, size_t count)
{
	safe_buffer_resize(sb, sb->used + count);
	memset(&sb->buffer[sb->used], (uint8_t)0, count);
	sb->used += count;
	return count;
}

size_t safe_buffer_append(struct safe_buffer *sb, void const *v, size_t count)
{
	if (sb) {
		unsigned const char *m = (unsigned const char *)v;
		safe_buffer_resize(sb, sb->used + count);
		memcpy(&sb->buffer[sb->used], m, count);
		sb->used += count;
	}

	return count;
}

/**
 * Create a new safe_buffer_list
 *
 * @return new safe_buffer_list, with a safe_buffer on the heap.
 */
struct safe_buffer_list *new_safe_buffer_list(void)
{
	struct safe_buffer_list *sbl = malloc(sizeof(struct safe_buffer_list));
	sbl->sb = new_safe_buffer();
	sbl->next = NULL;
	return sbl;
}

/**
 * Ensure list tail has an empty buffer ready to accept data.
 *
 * If the present element is empty of data, just return it.
 * Otherwise return a new safe_buffer_list ready to accept data.
 *
 * @param sbl safe_buffer_list.
 * @return new tail of list.
 */
struct safe_buffer_list *safe_buffer_list_append(struct safe_buffer_list *sbl)
{
	// Only allocate a new entry if this one has bytes in it.
	if (sbl->sb && sbl->sb->used > 0) {
		struct safe_buffer_list *next = new_safe_buffer_list();
		sbl->next = next;
		sbl = next;
	}
	return sbl;
}

/**
 * Convert an entire safe_buffer_list to a single safe_buffer.
 *
 * @param sbl safe_buffer_list source.
 * @param sb  safe_buffer destination.
 */
void safe_buffer_list_to_safe_buffer(struct safe_buffer_list *sbl, struct safe_buffer *sb)
{
	struct safe_buffer_list *cur;
	for (cur = sbl; cur; cur = cur->next) {
		if (cur->sb)
			safe_buffer_append(sb, cur->sb->buffer, cur->sb->used);
	}
}

/**
 * Free all memory used by a safe_buffer_list
 *
 * @param sbl safe_buffer_list to free.
 */
void safe_buffer_list_free(struct safe_buffer_list *sbl)
{
	struct safe_buffer_list *next;
	for (struct safe_buffer_list *current = sbl; current; current = next) {
		if (current->sb)
			safe_buffer_free(current->sb);
		next = current->next;
		free(current);
	}
}

__attribute__((format(printf, 1, 2))) char *strdupf(char const *format, ...)
{
	va_list va;
	va_start(va, format);
	char *strp = 0;
	int rc = vasprintf(&strp, format, va);
	if (rc == -1 || !strp) {
		flog(LOG_ERR, "vasprintf failed: %s", strerror(errno));
		exit(-1);
	}
	va_end(va);

	return strp;
}

double rand_between(double lower, double upper) { return ((upper - lower) / (RAND_MAX + 1.0) * rand() + lower); }

/* This assumes that str is not null and str_size > 0 */
void addrtostr(struct in6_addr const *addr, char *str, size_t str_size)
{
	const char *res;

	res = inet_ntop(AF_INET6, (void const *)addr, str, str_size);

	if (res == NULL) {
		flog(LOG_ERR, "addrtostr: inet_ntop: %s", strerror(errno));
		strncpy(str, "[invalid address]", str_size);
		str[str_size - 1] = '\0';
	}
}

/* Check if an in6_addr exists in the rdnss list */
int check_rdnss_presence(struct AdvRDNSS *rdnss, struct in6_addr *addr)
{
	while (rdnss) {
		if (!memcmp(&rdnss->AdvRDNSSAddr1, addr, sizeof(struct in6_addr)) ||
		    !memcmp(&rdnss->AdvRDNSSAddr2, addr, sizeof(struct in6_addr)) ||
		    !memcmp(&rdnss->AdvRDNSSAddr3, addr, sizeof(struct in6_addr)))
			return 1; /* rdnss address found in the list */
		rdnss = rdnss->next;
	}
	return 0;
}

/* Check if a suffix exists in the dnssl list */
int check_dnssl_presence(struct AdvDNSSL *dnssl, const char *suffix)
{
	while (dnssl) {
		for (int i = 0; i < dnssl->AdvDNSSLNumber; ++i) {
			if (0 == strcmp(dnssl->AdvDNSSLSuffixes[i], suffix))
				return 1; /* suffix found in the list */
		}
		dnssl = dnssl->next;
	}
	return 0;
}

/* Like read(), but retries in case of partial read */
ssize_t readn(int fd, void *buf, size_t count)
{
	size_t n = 0;
	while (count > 0) {
		int r = read(fd, buf, count);
		if (r < 0) {
			if (errno == EINTR)
				continue;
			return r;
		}
		if (r == 0)
			return n;
		buf = (char *)buf + r;
		count -= r;
		n += r;
	}
	return n;
}

/* Like write(), but retries in case of partial write */
ssize_t writen(int fd, const void *buf, size_t count)
{
	size_t n = 0;
	while (count > 0) {
		int r = write(fd, buf, count);
		if (r < 0) {
			if (errno == EINTR)
				continue;
			return r;
		}
		if (r == 0)
			return n;
		buf = (const char *)buf + r;
		count -= r;
		n += r;
	}
	return n;
}

int countbits(int b)
{
	int count;

	for (count = 0; b != 0; count++) {
		b &= b - 1; // this clears the LSB-most set bit
	}

	return (count);
}

int count_mask(struct sockaddr_in6 *m)
{
	struct in6_addr *in6 = &m->sin6_addr;
	int i;
	int count = 0;

	for (i = 0; i < 16; ++i) {
		count += countbits(in6->s6_addr[i]);
	}
	return count;
}

struct in6_addr get_prefix6(struct in6_addr const *addr, struct in6_addr const *mask)
{
	struct in6_addr prefix = *addr;
	int i = 0;

	for (; i < 16; ++i) {
		prefix.s6_addr[i] &= mask->s6_addr[i];
	}

	return prefix;
}
