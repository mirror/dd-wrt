/*
 * Copyright 2015 Steven Barth <steven at midlink.org>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <net/if.h>
#include <unistd.h>

#include <libubox/uloop.h>
#include <libubox/blobmsg.h>

#include "omcproxy.h"
#include "proxy.h"

int log_level = LOG_WARNING;


enum {
	PROXY_ATTR_SOURCE,
	PROXY_ATTR_SCOPE,
	PROXY_ATTR_DEST,
	PROXY_ATTR_MAX,
};

static struct blobmsg_policy proxy_policy[PROXY_ATTR_MAX] = {
	[PROXY_ATTR_SOURCE] = { .name = "source", .type = BLOBMSG_TYPE_STRING },
	[PROXY_ATTR_SCOPE] = { .name = "scope", .type = BLOBMSG_TYPE_STRING },
	[PROXY_ATTR_DEST] = { .name = "dest", .type = BLOBMSG_TYPE_ARRAY },
};

static int handle_proxy_set(void *data, size_t len)
{
	struct blob_attr *tb[PROXY_ATTR_MAX], *c;
	blobmsg_parse(proxy_policy, PROXY_ATTR_MAX, tb, data, len);

	const char *name = ((c = tb[PROXY_ATTR_SOURCE])) ? blobmsg_get_string(c) : NULL;
	int uplink = 0;
	int downlinks[32] = {0};
	size_t downlinks_cnt = 0;
	enum proxy_flags flags = 0;

	if (!name)
		return -EINVAL;

	if (!(uplink = if_nametoindex(name))) {
		L_WARN("%s(%s): %s", __FUNCTION__, name, strerror(errno));
		return -errno;
	}

	if ((c = tb[PROXY_ATTR_SCOPE])) {
		const char *scope = blobmsg_get_string(c);
		if (!strcmp(scope, "global"))
			flags = PROXY_GLOBAL;
		else if (!strcmp(scope, "organization"))
			flags = PROXY_ORGLOCAL;
		else if (!strcmp(scope, "site"))
			flags = PROXY_SITELOCAL;
		else if (!strcmp(scope, "admin"))
			flags = PROXY_ADMINLOCAL;
		else if (!strcmp(scope, "realm"))
			flags = PROXY_REALMLOCAL;

		if (!flags) {
			L_WARN("%s(%s): invalid scope (%s)", __FUNCTION__, name, scope);
			return -EINVAL;
		}
	}

	if ((c = tb[PROXY_ATTR_DEST])) {
		struct blob_attr *d;
		unsigned rem;
		blobmsg_for_each_attr(d, c, rem) {
			if (downlinks_cnt >= 32) {
				L_WARN("%s(%s): maximum number of destinations exceeded", __FUNCTION__, name);
				return -EINVAL;
			}

			const char *n = blobmsg_type(d) == BLOBMSG_TYPE_STRING ? blobmsg_get_string(d) : "";
			if (!(downlinks[downlinks_cnt++] = if_nametoindex(n))) {
				L_WARN("%s(%s): %s (%s)", __FUNCTION__, name, strerror(errno), blobmsg_get_string(d));
				return -errno;
			}
		}
	}

	return proxy_set(uplink, downlinks, downlinks_cnt, flags);
}

static void handle_signal(__unused int signal)
{
	uloop_end();
}

static void usage(const char *arg) {
	fprintf(stderr, "Usage: %s [options] <proxy1> [<proxy2>] [...]\n"
			"\nProxy examples:\n"
			"eth1,eth2\n"
			"eth1,eth2,eth3,scope=organization\n"
			"\nProxy options (each option may only occur once):\n"
			"	<interface>			interfaces to proxy (first is uplink)\n"
			"	scope=<scope>			minimum multicast scope to proxy\n"
			"		[global,organization,site,admin,realm] (default: global)\n"
			"\nOptions:\n"
			"	-v				verbose logging\n"
			"	-h				show this help\n",
	arg);
}

int main(int argc, char **argv) {
	signal(SIGINT, handle_signal);
	signal(SIGTERM, handle_signal);
	signal(SIGHUP, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	openlog("omcproxy", LOG_PERROR, LOG_DAEMON);

	if (getuid()) {
		L_ERR("must be run as root!");
		return 2;
	}

	uloop_init();
	bool start = true;

	for (ssize_t i = 1; i < argc; ++i) {
		const char *source = NULL;
		const char *scope = NULL;
		struct blob_buf b = {NULL, NULL, 0, NULL};

		if (!strcmp(argv[i], "-h")) {
			usage(argv[0]);
			return 1;
		} else if (!strncmp(argv[i], "-v", 2)) {
			if ((log_level = atoi(&argv[i][2])) <= 0)
				log_level = 7;
			continue;
		}


		blob_buf_init(&b, 0);

		void *k = blobmsg_open_array(&b, "dest");
		for (char *c = strtok(argv[i], ","); c; c = strtok(NULL, ",")) {
			if (!strncmp(c, "scope=", 6)) {
				scope = &c[6];
			} else if (!source) {
				source = c;
			} else {
				blobmsg_add_string(&b, NULL, c);
			}
		}
		blobmsg_close_array(&b, k);

		if (source)
			blobmsg_add_string(&b, "source", source);

		if (scope)
			blobmsg_add_string(&b, "scope", scope);

		if (handle_proxy_set(blob_data(b.head), blob_len(b.head))) {
			fprintf(stderr, "failed to setup proxy: %s\n", argv[i]);
			start = false;
		}

		blob_buf_free(&b);
	}

	if (argc < 2) {
		usage(argv[0]);
		start = false;
	}

	if (start)
		uloop_run();

	proxy_update(true);
	proxy_flush();

	uloop_done();
	return 0;
}
