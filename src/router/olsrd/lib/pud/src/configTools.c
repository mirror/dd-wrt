/*
 * The olsr.org Optimized Link-State Routing daemon (olsrd)
 *
 * (c) by the OLSR project
 *
 * See our Git repository to find out who worked on this file
 * and thus is a copyright holder on it.
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of olsr.org, olsrd nor the names of its
 *   contributors may be used to endorse or promote products derived
 *   from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * Visit http://www.olsr.org for more information.
 *
 * If you find this software useful feel free to make a donation
 * to the project. For more information see the website or contact
 * the copyright holders.
 *
 */

#include "configTools.h"

/* Plugin includes */
#include "pud.h"
#include "netTools.h"

/* OLSR includes */

/* System includes */
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <string.h>

/**
 Read a boolean from a string

 @param parameterName
 The name of the parameter, used when reporting errors
 @param str
 The string to convert to a boolean
 @param dst
 A pointer to the location where to store the boolean upon successful conversion
 Not touched when errors are reported.

 @return
 - true on success
 - false otherwise
 */
bool readBool(const char * parameterName, const char * str, bool * dst) {
	bool retVal = true;
	char * endPtr = NULL;
	unsigned long value;
	char * strDup = strdup(str);
	char * c;

	assert(parameterName != NULL);
	assert(str != NULL);
	assert(dst != NULL);

	/* convert to lowercase */
	for (c = strDup; *c != '\0'; c++) {
		if (isalpha(*c) && !islower(*c)) {
			*c = tolower(*c);
		}
	}

	/* try true/yes/on */
	if (!strcmp("true", strDup) || !strcmp("yes", strDup) || !strcmp("on", strDup)) {
		*dst = true;
		goto out;
	}

	/* try false/no/off */
	if (!strcmp("false", strDup) || !strcmp("no", strDup) || !strcmp("off", strDup)) {
		*dst = false;
		goto out;
	}

	/* try a number */
	errno = 0;
	value = strtoul(str, &endPtr, 10);

	if (!((endPtr != str) && (*str != '\0') && (*endPtr == '\0')) || (errno == ERANGE)) {
		/* invalid conversion */
		pudError(false, "Value of parameter %s (%s) could not be converted to a number", parameterName, str);
		retVal = false;
		goto out;
	}

	if (value > 1) {
		pudError(false, "Value of parameter %s (%lu) is outside of valid range 0-1", parameterName, value);
		retVal = false;
		goto out;
	}

	/* 0 = false, 1 = true */
	*dst = (value == 1);

	out: free(strDup);
	return retVal;
}

/**
 Read an unsigned char number from a string

 @param parameterName
 The name of the parameter, used when reporting errors
 @param str
 The string to convert to a number
 @param dst
 A pointer to the location where to store the number upon successful conversion
 Not touched when errors are reported.

 @return
 - true on success
 - false otherwise
 */
bool readUC(const char * parameterName, const char * str, unsigned char * dst) {
	char * endPtr = NULL;
	unsigned long value;

	assert(parameterName != NULL);
	assert(str != NULL);
	assert(dst != NULL);

	errno = 0;
	value = strtoul(str, &endPtr, 10);

	if (!((endPtr != str) && (*str != '\0') && (*endPtr == '\0')) || (errno == ERANGE)) {
		/* invalid conversion */
		pudError(false, "Value of parameter %s (%s) could not be converted to a number", parameterName, str);
		return false;
	}

	if (value > 255) {
		pudError(false, "Value of parameter %s (%lu) is outside of valid range 0-255", parameterName, value);
		return false;
	}

	*dst = value;
	return true;
}

/**
 Read an unsigned short number from a string

 @param parameterName
 The name of the parameter, used when reporting errors
 @param str
 The string to convert to a number
 @param dst
 A pointer to the location where to store the number upon successful conversion
 Not touched when errors are reported.

 @return
 - true on success
 - false otherwise
 */
bool readUS(const char * parameterName, const char * str, unsigned short * dst) {
	char * endPtr = NULL;
	unsigned long value;

	assert(parameterName != NULL);
	assert(str != NULL);
	assert(dst != NULL);

	errno = 0;
	value = strtoul(str, &endPtr, 10);

	if (!((endPtr != str) && (*str != '\0') && (*endPtr == '\0')) || (errno == ERANGE)) {
		/* invalid conversion */
		pudError(false, "Value of parameter %s (%s) could not be converted to a number", parameterName, str);
		return false;
	}

	if (value > 65535) {
		pudError(false, "Value of parameter %s (%lu) is outside of valid range 0-65535", parameterName, value);
		return false;
	}

	*dst = value;
	return true;
}

/**
 Read an unsigned long long number from a string

 @param parameterName
 The name of the parameter, used when reporting errors
 @param str
 The string to convert to a number
 @param dst
 A pointer to the location where to store the number upon successful conversion
 Not touched when errors are reported.
 @param base
 The base of the number conversion: 10 for decimal, 16 for hexadecimal

 @return
 - true on success
 - false otherwise
 */
bool readULL(const char * parameterName, const char * str, unsigned long long * dst, int base) {
	char * endPtr = NULL;
	unsigned long long value;

	assert(parameterName != NULL);
	assert(str != NULL);
	assert(dst != NULL);
	assert(base > 1);

	errno = 0;
	value = strtoull(str, &endPtr, base);

	if (!((endPtr != str) && (*str != '\0') && (*endPtr == '\0')) || (errno == ERANGE)) {
		/* invalid conversion */
		pudError(false, "Value of parameter %s (%s) could not be converted to a number (base %d)", parameterName, str, base);
		return false;
	}

	*dst = value;
	return true;
}

/**
 Read a double number from a string

 @param parameterName
 The name of the parameter, used when reporting errors
 @param str
 The string to convert to a number
 @param dst
 A pointer to the location where to store the number upon successful conversion
 Not touched when errors are reported.

 @return
 - true on success
 - false otherwise
 */
 bool readDouble(const char * parameterName, const char * str, double * dst) {
	char * endPtr = NULL;
	double value;

	assert(parameterName != NULL);
	assert(str != NULL);
	assert(dst != NULL);

	errno = 0;
	value = strtod(str, &endPtr);

	if (!((endPtr != str) && (*str != '\0') && (*endPtr == '\0')) || (errno == ERANGE)) {
		/* invalid conversion */
		pudError(false, "Value of parameter %s (%s) could not be converted to a number", parameterName, str);
		return false;
	}

	*dst = value;
	return true;
}

/**
 Read an (olsr_sockaddr) IP address from a string:
 First tries to parse the value as an IPv4 address, and if not successful tries to parse it as an IPv6 address.
 When the address wasn't set yet, the default port (portDefault) is set.

 @param parameterName
 The name of the parameter, used when reporting errors
 @param str
 The string to convert to an (olsr_sockaddr) IP address
 @param portDefault
 The default for the port (in host byte order, stored in network byte order)
 @param dst
 A pointer to the location where to store the (olsr_sockadd) IP address upon successful conversion.
 Not touched when errors are reported.
 @param dstSet
 A pointer to the location where to store the flag that signals whether the IP address is set.
 Not touched when errors are reported.

 @return
 - true on success
 - false otherwise
 */
 bool readIPAddress(const char * parameterName, const char * str, in_port_t portDefault,
		union olsr_sockaddr * dst, bool * dstSet) {
	union olsr_sockaddr ip;
	int conversion;
	in_port_t port;

	assert(parameterName != NULL);
	assert(str != NULL);
	assert(dst != NULL);
	assert(dstSet != NULL);

	/* try IPv4 first */
	memset(&ip, 0, sizeof(ip));
	ip.in.sa_family = AF_INET;
	conversion = inet_pton(ip.in.sa_family, str, &ip.in4.sin_addr);

	/* now try IPv6 if IPv4 conversion was not successful */
	if (conversion != 1) {
		memset(&ip, 0, sizeof(ip));
		ip.in.sa_family = AF_INET6;
		conversion = inet_pton(ip.in.sa_family, str, &ip.in6.sin6_addr);
	}

	if (conversion != 1) {
		pudError((conversion == -1) ? true : false,
		"Value of parameter %s (%s) is not an IP address", parameterName, str);
		return false;
	}

	if (!*dstSet) {
	  port = htons(portDefault);
	} else {
	  port = getOlsrSockaddrPort(dst, portDefault);
	}

	dst->in.sa_family = ip.in.sa_family;
	setOlsrSockaddrPort(dst, port);
	setOlsrSockaddrAddr(dst, &ip);
	*dstSet = true;
	return true;
}
