#include "usr/util/str_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <regex.h>
#include <netinet/in.h>

/* The maximum network length for IPv4. */
static const __u8 IPV4_MAX_PREFIX = 32;
/* The maximum network length for IPv6. */
static const __u8 IPV6_MAX_PREFIX = 128;

static struct jool_result validate_uint(const char *str)
{
	regex_t uint_regex;
	int error;

	if (!str) {
		return result_from_error(
			-EINVAL,
			"Programming error: 'str' is NULL."
		);
	}

	/* It seems this RE implementation doesn't understand '+'. */
	if (regcomp(&uint_regex, "^[0-9][0-9]*", 0)) {
		fprintf(stderr, "Warning: Integer regex didn't compile.\n");
		fprintf(stderr, "(I will be unable to validate integer inputs.)\n");
		regfree(&uint_regex);
		/*
		 * Don't punish the user over our incompetence.
		 * If the number is valid, this will not bother the user.
		 * Otherwise strtoull() will just read a random value, but then
		 * the user is at fault.
		 */
		return result_success();
	}

	error = regexec(&uint_regex, str, 0, NULL, 0);
	regfree(&uint_regex);
	if (error) {
		return result_from_error(
			error,
			"'%s' is not an unsigned integer. (error code %d)",
			str,
			error
		);
	}

	return result_success();
}

static struct jool_result str_to_ull(const char *str, char **endptr,
		const unsigned long long int min,
		const unsigned long long int max,
		unsigned long long int *out)
{
	unsigned long long int parsed;
	struct jool_result result;

	result = validate_uint(str);
	if (result.error)
		return result;

	errno = 0;
	parsed = strtoull(str, endptr, 10);
	if (errno) {
		return result_from_error(
			errno,
			"Parsing of '%s' threw error code %d.", str, errno
		);
	}

	if (parsed < min || max < parsed) {
		return result_from_error(
			-EINVAL,
			"'%s' is out of bounds (%llu-%llu).", str, min, max
		);
	}

	*out = parsed;
	return result_success();
}

struct jool_result str_to_bool(const char *str, bool *out)
{
	if (strcasecmp(str, "true") == 0
			|| strcasecmp(str, "1") == 0
			|| strcasecmp(str, "yes") == 0
			|| strcasecmp(str, "on") == 0) {
		*out = true;
		return result_success();
	}

	if (strcasecmp(str, "false") == 0
			|| strcasecmp(str, "0") == 0
			|| strcasecmp(str, "no") == 0
			|| strcasecmp(str, "off") == 0) {
		*out = false;
		return result_success();
	}

	return result_from_error(
		-EINVAL,
		"Cannot parse '%s' as a bool (true|false|1|0|yes|no|on|off).",
		str
	);
}

struct jool_result str_to_u8(const char *str, __u8 *u8_out, __u8 max)
{
	unsigned long long int out;
	struct jool_result result;

	result = str_to_ull(str, NULL, 0, max, &out);
	if (result.error)
		return result;

	*u8_out = out;
	return result_success();
}

struct jool_result str_to_u16(const char *str, __u16 *u16_out)
{
	unsigned long long int out;
	struct jool_result result;

	result = str_to_ull(str, NULL, 0, MAX_U16, &out);

	*u16_out = out;
	return result;
}

struct jool_result str_to_u32(const char *str, __u32 *u32_out)
{
	unsigned long long int out;
	struct jool_result result;

	result = str_to_ull(str, NULL, 0, MAX_U32, &out);

	*u32_out = out;
	return result;
}

struct jool_result str_to_timeout(const char *str, __u32 *out)
{
	unsigned long long int seconds = 0;
	unsigned long long int milliseconds;
	char *tail;

	errno = 0;
	seconds = strtoull(str, &tail, 10);
	if (errno)
		goto parse_failure;

	if (*tail == ':') {
		errno = 0;
		seconds = 60 * seconds + strtoull(tail + 1, &tail, 10);
		if (errno)
			goto parse_failure;

		if (*tail == ':') {
			errno = 0;
			seconds = 60 * seconds + strtoull(tail + 1, &tail, 10);
			if (errno)
				goto parse_failure;
		}
	}

	milliseconds = 1000 * seconds;

	if (*tail == '.') {
		if (strlen(tail + 1) < 3)
			goto msec_length;

		errno = 0;
		milliseconds += strtoull(tail + 1, &tail, 10);
		if (errno)
			goto parse_failure;
	}

	if (*tail != '\0')
		goto postfix;

	*out = milliseconds;
	return result_success();

parse_failure:
	return result_from_error(
		errno,
		"Parsing of '%s' threw error code %d.", str, errno
	);

msec_length:
	return result_from_error(
		-EINVAL,
		"The millisecond portion of '%s' must length at least 3 digits.",
		str
	);

postfix:
	return result_from_error(
		-EINVAL,
		"'%s' does not seem to follow the '[HH:[MM:]]SS[.mmm]' format.",
		str
	);
}

struct jool_result str_to_port_range(char *str, struct port_range *range)
{
	unsigned long long int tmp;
	char *endptr = NULL;
	struct jool_result result;

	result = str_to_ull(str, &endptr, 0, 65535, &tmp);
	if (result.error)
		return result;
	range->min = tmp;

	if (*endptr != '-') {
		range->max = range->min;
		return result_success();
	}

	result = str_to_ull(endptr + 1, NULL, 0, 65535, &tmp);
	if (result.error)
		return result;

	range->max = tmp;
	return result;
}

struct jool_result str_to_addr4(const char *str, struct in_addr *addr)
{
	if (!inet_pton(AF_INET, str, addr)) {
		return result_from_error(
			-EINVAL,
			"Cannot parse '%s' as an IPv4 address.", str
		);
	}
	return result_success();
}

struct jool_result str_to_addr6(const char *str, struct in6_addr *addr)
{
	if (!inet_pton(AF_INET6, str, addr)) {
		return result_from_error(
			-EINVAL,
			"Cannot parse '%s' as an IPv6 address.", str
		);
	}
	return result_success();
}

#undef STR_MAX_LEN
/* [addr + null chara] + # + port */
#define STR_MAX_LEN (INET_ADDRSTRLEN + 1 + 5)
struct jool_result str_to_addr4_port(const char *str,
		struct ipv4_transport_addr *addr_out)
{
	const char *FORMAT = "<IPv4 address>#<port> (eg. 203.0.113.8#80)";
	/* strtok corrupts the string, so we'll be using this copy instead. */
	char str_copy[STR_MAX_LEN];
	char *token;
	struct jool_result result;

	if (strlen(str) + 1 > STR_MAX_LEN) {
		return result_from_error(
			-EINVAL,
			"'%s' is too long for this poor, limited parser...", str
		);
	}
	strcpy(str_copy, str);

	token = strtok(str_copy, "#");
	if (!token) {
		return result_from_error(
			-EINVAL,
			"Cannot parse '%s' as a %s.", str, FORMAT
		);
	}

	result = str_to_addr4(token, &addr_out->l3);
	if (result.error)
		return result;

	token = strtok(NULL, "#");
	if (!token) {
		return result_from_error(
			-EINVAL,
			"'%s' does not seem to contain a port (format: %s).",
			str, FORMAT
		);
	}
	return str_to_u16(token, &addr_out->l4);
}

#undef STR_MAX_LEN
/* [addr + null chara] + # + port */
#define STR_MAX_LEN (INET6_ADDRSTRLEN + 1 + 5)
struct jool_result str_to_addr6_port(const char *str,
		struct ipv6_transport_addr *addr_out)
{
	const char *FORMAT = "<IPv6 address>#<port> (eg. 2001:db8::1#96)";
	/* strtok corrupts the string, so we'll be using this copy instead. */
	char str_copy[STR_MAX_LEN];
	char *token;
	struct jool_result result;

	if (strlen(str) + 1 > STR_MAX_LEN) {
		return result_from_error(
			-EINVAL,
			"'%s' is too long for this poor, limited parser...", str
		);
	}
	strcpy(str_copy, str);

	token = strtok(str_copy, "#");
	if (!token) {
		return result_from_error(
			-EINVAL,
			"Cannot parse '%s' as a %s.", str, FORMAT
		);
	}

	result = str_to_addr6(token, &addr_out->l3);
	if (result.error)
		return result;

	token = strtok(NULL, "#");
	if (!token) {
		return result_from_error(
			-EINVAL,
			"'%s' does not seem to contain a port (format: %s).",
			str, FORMAT
		);
	}
	return str_to_u16(token, &addr_out->l4);
}

#undef STR_MAX_LEN
/* [addr + null chara] + / + mask */
#define STR_MAX_LEN (INET_ADDRSTRLEN + 1 + 2)
struct jool_result str_to_prefix4(const char *str,
		struct ipv4_prefix *prefix_out)
{
	const char *FORMAT = "<IPv4 address>[/<mask>] (eg. 192.0.2.0/24)";
	/* strtok corrupts the string, so we'll be using this copy instead. */
	char str_copy[STR_MAX_LEN];
	char *token;
	struct jool_result result;

	if (strlen(str) + 1 > STR_MAX_LEN) {
		return result_from_error(
			-EINVAL,
			"'%s' is too long for this poor, limited parser...", str
		);
	}
	strcpy(str_copy, str);

	token = strtok(str_copy, "/");
	if (!token) {
		return result_from_error(
			-EINVAL,
			"Cannot parse '%s' as a %s.", str, FORMAT
		);
	}

	result = str_to_addr4(token, &prefix_out->addr);
	if (result.error)
		return result;

	token = strtok(NULL, "/");
	if (!token) {
		prefix_out->len = IPV4_MAX_PREFIX;
		return result_success();
	}
	return str_to_u8(token, &prefix_out->len, 32);
}

#undef STR_MAX_LEN
/* [addr + null chara] + / + pref len */
#define STR_MAX_LEN (INET6_ADDRSTRLEN + 1 + 3)
struct jool_result str_to_prefix6(const char *str,
		struct ipv6_prefix *prefix_out)
{
	const char *FORMAT = "<IPv6 address>[/<length>] (eg. 64:ff9b::/96)";
	/* strtok corrupts the string, so we'll be using this copy instead. */
	char str_copy[STR_MAX_LEN];
	char *token;
	struct jool_result result;

	if (strlen(str) + 1 > STR_MAX_LEN) {
		return result_from_error(
			-EINVAL,
			"'%s' is too long for this poor, limited parser...", str
		);
	}
	strcpy(str_copy, str);

	token = strtok(str_copy, "/");
	if (!token) {
		return result_from_error(
			-EINVAL,
			"Cannot parse '%s' as a %s.", str, FORMAT
		);
	}

	result = str_to_addr6(token, &prefix_out->addr);
	if (result.error)
		return result;

	token = strtok(NULL, "/");
	if (!token) {
		prefix_out->len = IPV6_MAX_PREFIX;
		return result_success();
	}
	return str_to_u8(token, &prefix_out->len, 128);
}

struct jool_result str_to_plateaus_array(const char *str, struct mtu_plateaus *plateaus)
{
	/* strtok corrupts the string, so we'll be using this copy instead. */
	char *str_copy;
	char *token;
	unsigned int len;
	struct jool_result result;

	/* Validate str and copy it to the temp buffer. */
	/* TODO (fine) strdup, damn it */
	str_copy = malloc(strlen(str) + 1);
	if (!str_copy)
		return result_from_enomem();

	strcpy(str_copy, str);

	/* Count the number of elements in the string. */
	len = 0;
	token = strtok(str_copy, ",");
	while (token) {
		len++;
		token = strtok(NULL, ",");
	}

	if (len == 0) {
		free(str_copy);
		return result_from_error(
			-EINVAL,
			"The plateaus string cannot be empty."
		);
	}
	if (len > PLATEAUS_MAX) {
		free(str_copy);
		return result_from_error(
			-EINVAL,
			"Too many plateaus. The current max is %u.",
			PLATEAUS_MAX
		);
	}

	/* Build the result. */
	plateaus->count = len;
	len = 0;
	strcpy(str_copy, str);
	token = strtok(str_copy, ",");
	while (token) {
		result = str_to_u16(token, &plateaus->values[len]);
		if (result.error) {
			free(str_copy);
			return result;
		}

		len++;
		token = strtok(NULL, ",");
	}

	free(str_copy);
	return result_success();
}

void timeout2str(unsigned int millis, char *buffer)
{
	static const unsigned int MILLIS_PER_SECOND = 1000;
	static const unsigned int MILLIS_PER_MIN = 60000;
	static const unsigned int MILLIS_PER_HOUR = 3600000;
	unsigned int hours;
	unsigned int minutes;
	unsigned int seconds;
	int offset;

	hours = millis / MILLIS_PER_HOUR;
	millis -= hours * MILLIS_PER_HOUR;

	minutes = millis / MILLIS_PER_MIN;
	millis -= minutes * MILLIS_PER_MIN;

	seconds = millis / MILLIS_PER_SECOND;
	millis -= seconds * MILLIS_PER_SECOND;

	offset = snprintf(buffer, TIMEOUT_BUFLEN, "%u:%02u:%02u", hours,
			minutes, seconds);
	if (millis) {
		snprintf(buffer + offset, TIMEOUT_BUFLEN - offset, ".%03u",
				millis);
	}
}

