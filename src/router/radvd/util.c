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

struct safe_buffer * new_safe_buffer(void)
{
	struct safe_buffer * sb = malloc(sizeof(struct safe_buffer));
	*sb = SAFE_BUFFER_INIT;
	sb->should_free = 1;
	return sb;
}

void safe_buffer_free(struct safe_buffer * sb)
{
	if (sb->buffer) {
		free(sb->buffer);
	}

	if (sb->should_free) {
		free(sb);
	}
}

size_t safe_buffer_pad(struct safe_buffer * sb, size_t count)
{
	size_t rc = 0;
	unsigned char zero = 0;

	while (count--) {
		rc += safe_buffer_append(sb, &zero, 1);
	}

	return rc;
}

size_t safe_buffer_append(struct safe_buffer * sb, void const * v, size_t count)
{
	if (sb) {
		unsigned const char * m = (unsigned const char *)v;
		if (sb->allocated <= sb->used + count) {
			sb->allocated = sb->used + count + MSG_SIZE_SEND;
			sb->buffer = realloc(sb->buffer, sb->allocated);
		}
		memcpy(&sb->buffer[sb->used], m, count);
		sb->used += count;

		if (sb->used >= MSG_SIZE_SEND) {
			flog(LOG_ERR, "Too many prefixes, routes, rdnss or dnssl to fit in buffer.  Exiting.");
			exit(1);
		}
	}

	return count;
}

__attribute__ ((format(printf, 1, 2)))
char * strdupf(char const * format, ...)
{
	va_list va;
	va_start(va, format);
	char * strp = 0;
	int rc = vasprintf(&strp, format, va);
	if (rc == -1 || !strp) {
		flog(LOG_ERR, "vasprintf failed: %s", strerror(errno));
		exit(-1);
	}
	va_end(va);

	return strp;
}

double rand_between(double lower, double upper)
{
	return ((upper - lower) / (RAND_MAX + 1.0) * rand() + lower);
}

/* This assumes that str is not null and str_size > 0 */
void addrtostr(struct in6_addr *addr, char *str, size_t str_size)
{
	const char *res;

	res = inet_ntop(AF_INET6, (void *)addr, str, str_size);

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
		if (!memcmp(&rdnss->AdvRDNSSAddr1, addr, sizeof(struct in6_addr))
		    || !memcmp(&rdnss->AdvRDNSSAddr2, addr, sizeof(struct in6_addr))
		    || !memcmp(&rdnss->AdvRDNSSAddr3, addr, sizeof(struct in6_addr)))
			return 1;	/* rdnss address found in the list */
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
				return 1;	/* suffix found in the list */
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
