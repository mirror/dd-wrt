#include "usr/joold/netsocket.h"

#include <errno.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>

#include "log.h"
#include "modsocket.h"
#include "common/config.h"
#include "common/types.h"
#include "usr/util/cJSON.h"
#include "usr/util/file.h"
#include "usr/util/str_utils.h"

struct netsocket_config {
	/** Address where the sessions will be advertised. Lacks a default. */
	char *mcast_addr;
	/** UDP port where the sessions will be advertised. Lacks a default. */
	char *mcast_port;

	/**
	 * On IPv4, this should be one addresses from the interface where the
	 * multicast traffic is expected to be received.
	 * On IPv6, this should be the name of the interface (eg. "eth0").
	 * Defaults to NULL, which has the kernel choose an interface for us.
	 */
	char *in_interface;
	/** Just like @in_interface, except for outgoing packets. */
	char *out_interface;

	int reuseaddr;
	bool reuseaddr_set;

	int ttl;
	bool ttl_set;
};

static int sk;
/** Processed version of the configuration's hostname and service. */
static struct addrinfo *addr_candidates;
/** Candidate from @addr_candidates that we managed to bind the socket with. */
static struct addrinfo *bound_address;

static struct in_addr *get_addr4(struct addrinfo *addr)
{
	return &((struct sockaddr_in *)addr->ai_addr)->sin_addr;
}

static struct in6_addr *get_addr6(struct addrinfo *addr)
{
	return &((struct sockaddr_in6 *)addr->ai_addr)->sin6_addr;
}

bool is_multicast4(struct in_addr *addr)
{
	return (addr->s_addr & htonl(0xf0000000)) == htonl(0xe0000000);
}

bool is_multicast6(struct in6_addr *addr)
{
	return (addr->s6_addr32[0] & htonl(0xff000000)) == htonl(0xff000000);
}

static int validate_valueint(cJSON *json, char *field)
{
	if (json->numflags & VALUENUM_INT)
		return 0;

	syslog(LOG_ERR, "%s '%s' is not a valid integer.", field,
			json->valuestring);
	return -EINVAL;
}

static int json_to_config(cJSON *json, struct netsocket_config *cfg)
{
	char *missing;
	cJSON *child;
	int error;

	memset(cfg, 0, sizeof(*cfg));

	child = cJSON_GetObjectItem(json, "multicast address");
	if (!child) {
		missing = "multicast address";
		goto fail;
	}
	cfg->mcast_addr = child->valuestring;

	child = cJSON_GetObjectItem(json, "multicast port");
	if (!child) {
		missing = "multicast port";
		goto fail;
	}
	cfg->mcast_port = child->valuestring;

	child = cJSON_GetObjectItem(json, "in interface");
	cfg->in_interface = child ? child->valuestring : NULL;

	child = cJSON_GetObjectItem(json, "out interface");
	cfg->out_interface = child ? child->valuestring : NULL;

	child = cJSON_GetObjectItem(json, "reuseaddr");
	if (child) {
		error = validate_valueint(child, "reuseaddr");
		if (error)
			return error;
		cfg->reuseaddr_set = true;
		cfg->reuseaddr = child->valueint;
	}

	child = cJSON_GetObjectItem(json, "ttl");
	if (child) {
		error = validate_valueint(child, "ttl");
		if (error)
			return error;
		cfg->ttl_set = true;
		cfg->ttl = child->valueint;
	}

	return 0;

fail:
	syslog(LOG_ERR, "The field '%s' is mandatory; please include it in the file.",
			missing);
	return 1;
}

int read_json(int argc, char **argv, cJSON **out)
{
	char *file_name;
	char *file;
	cJSON *json;
	struct jool_result result;

	file_name = (argc >= 2) ? argv[1] : "netsocket.json";
	syslog(LOG_INFO, "Opening file %s...", file_name);
	result = file_to_string(file_name, &file);
	if (result.error)
		return pr_result(&result);

	json = cJSON_Parse(file);
	if (!json) {
		syslog(LOG_ERR, "JSON syntax error.");
		syslog(LOG_ERR, "The JSON parser got confused around about here:");
		syslog(LOG_ERR, "%s", cJSON_GetErrorPtr());
		free(file);
		return 1;
	}

	free(file);
	*out = json;
	return 0;
}

static int try_address(struct netsocket_config *config)
{
	sk = socket(bound_address->ai_family, bound_address->ai_socktype,
			bound_address->ai_protocol);
	if (sk < 0) {
		pr_perror("socket() failed", errno);
		return 1;
	}

	/* (Do not reorder this. SO_REUSEADDR needs to happen before bind().) */
	if (config->reuseaddr_set) {
		syslog(LOG_INFO, "Setting SO_REUSEADDR as %d...",
				config->reuseaddr);
		/* http://stackoverflow.com/questions/14388706 */
		if (setsockopt(sk, SOL_SOCKET, SO_REUSEADDR, &config->reuseaddr,
				sizeof(config->reuseaddr))) {
			pr_perror("setsockopt(SO_REUSEADDR) failed", errno);
			return 1;
		}
	}

	if (bind(sk, bound_address->ai_addr, bound_address->ai_addrlen)) {
		pr_perror("bind() failed", errno);
		return 1;
	}

	syslog(LOG_INFO, "The socket to the network was created.");
	return 0;
}

static int create_socket(struct netsocket_config *config)
{
	struct addrinfo hints = { 0 };
	int err;

	syslog(LOG_INFO, "Getting address info of %s#%s...",
			config->mcast_addr,
			config->mcast_port);

	hints.ai_socktype = SOCK_DGRAM;
	err = getaddrinfo(config->mcast_addr, config->mcast_port, &hints,
			&addr_candidates);
	if (err) {
		syslog(LOG_ERR, "getaddrinfo() failed: %s", gai_strerror(err));
		return err;
	}

	bound_address = addr_candidates;
	while (bound_address) {
		syslog(LOG_INFO, "Trying an address candidate...");
		err = try_address(config);
		if (!err)
			return 0;
		bound_address = bound_address->ai_next;
	}

	syslog(LOG_ERR, "None of the candidates yielded a valid socket.");
	freeaddrinfo(addr_candidates);
	return 1;
}

static int mcast4opt_add_membership(struct netsocket_config *cfg)
{
	struct ip_mreq mreq;
	struct jool_result result;

	mreq.imr_multiaddr = *get_addr4(bound_address);
	if (cfg->in_interface) {
		result = str_to_addr4(cfg->in_interface, &mreq.imr_interface);
		if (result.error) {
			pr_result(&result);
			return 1;
		}
	} else {
		mreq.imr_interface.s_addr = htonl(INADDR_ANY);
	}

	if (setsockopt(sk, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq,
			sizeof(mreq))) {
		pr_perror("-> setsockopt(IP_ADD_MEMBERSHIP) failed", errno);
		return 1;
	}

	syslog(LOG_INFO, "-> We're now registered to the multicast group.");
	return 0;
}

static int mcast4opt_disable_loopback(void)
{
	int loop = 0;

	if (setsockopt(sk, IPPROTO_IP, IP_MULTICAST_LOOP, &loop,
			sizeof(loop))) {
		pr_perror("-> setsockopt(IP_MULTICAST_LOOP) failed", errno);
		return 1;
	}

	syslog(LOG_INFO, "-> Multicast loopback disabled.");
	return 0;
}

static int mcast4opt_set_ttl(struct netsocket_config *cfg)
{
	if (!cfg->ttl_set)
		return 0;

	if (setsockopt(sk, IPPROTO_IP, IP_MULTICAST_TTL, &cfg->ttl,
			sizeof(cfg->ttl))) {
		pr_perror("-> setsockopt(IP_MULTICAST_TTL) failed", errno);
		return 1;
	}

	syslog(LOG_INFO, "-> Tweaked the TTL of multicasts.");
	return 0;
}

static int mcast4opt_set_out_interface(struct netsocket_config *cfg)
{
	struct in_addr addr;
	struct jool_result result;

	if (!cfg->out_interface)
		return 0;

	result = str_to_addr4(cfg->out_interface, &addr);
	if (result.error) {
		pr_result(&result);
		return 1;
	}

	if (setsockopt(sk, IPPROTO_IP, IP_MULTICAST_IF, &addr, sizeof(addr))) {
		pr_perror("-> setsockopt(IP_MULTICAST_IF) failed", errno);
		return 1;
	}

	syslog(LOG_INFO, "-> The outgoing interface was overridden.");
	return 0;
}

static int handle_mcast4_opts(struct netsocket_config *cfg)
{
	int error;

	error = mcast4opt_add_membership(cfg);
	if (error)
		return error;

	error = mcast4opt_disable_loopback();
	if (error)
		return error;

	error = mcast4opt_set_ttl(cfg);
	if (error)
		return error;

	return mcast4opt_set_out_interface(cfg);
}

static int mcast6opt_add_membership(struct netsocket_config *cfg)
{
	struct ipv6_mreq mreq;

	mreq.ipv6mr_multiaddr = *get_addr6(bound_address);
	if (cfg->in_interface) {
		mreq.ipv6mr_interface = if_nametoindex(cfg->in_interface);
		if (!mreq.ipv6mr_interface) {
			pr_perror("The incoming interface name is invalid",
					errno);
			return 1;
		}
	} else {
		mreq.ipv6mr_interface = 0; /* Any interface. */
	}

	if (setsockopt(sk, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &mreq,
			sizeof(mreq))) {
		pr_perror("setsockopt(IPV6_ADD_MEMBERSHIP) failed", errno);
		return 1;
	}

	syslog(LOG_INFO, "We're now registered to the multicast group.");
	return 0;
}

static int mcast6opt_disable_loopback(void)
{
	int loop = 0;

	if (setsockopt(sk, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &loop,
			sizeof(loop))) {
		pr_perror("setsockopt(IP_MULTICAST_LOOP) failed", errno);
		return 1;
	}

	syslog(LOG_INFO, "Multicast loopback disabled.");
	return 0;
}

static int mcast6opt_set_ttl(struct netsocket_config *cfg)
{
	if (!cfg->ttl_set)
		return 0;

	if (setsockopt(sk, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &cfg->ttl,
			sizeof(cfg->ttl))) {
		pr_perror("setsockopt(IPV6_MULTICAST_HOPS) failed", errno);
		return 1;
	}

	syslog(LOG_INFO, "Tweaked the TTL of multicasts.");
	return 0;
}

static int mcast6opt_set_out_interface(struct netsocket_config *cfg)
{
	unsigned int interface;

	if (!cfg->out_interface)
		return 0;

	interface = if_nametoindex(cfg->out_interface);
	if (!interface) {
		pr_perror("The outgoing interface name is invalid", errno);
		return 1;
	}

	if (setsockopt(sk, IPPROTO_IPV6, IPV6_MULTICAST_IF, &interface,
			sizeof(interface))) {
		pr_perror("setsockopt(IP_MULTICAST_IF) failed", errno);
		return 1;
	}

	syslog(LOG_INFO, "The outgoing interface was overridden.");
	return 0;
}

static int handle_mcast6_opts(struct netsocket_config *cfg)
{
	int error;

	error = mcast6opt_add_membership(cfg);
	if (error)
		return error;

	error = mcast6opt_disable_loopback();
	if (error)
		return error;

	error = mcast6opt_set_ttl(cfg);
	if (error)
		return error;

	return mcast6opt_set_out_interface(cfg);
}

static int adjust_mcast_opts(struct netsocket_config *cfg)
{
	syslog(LOG_INFO, "Configuring multicast options on the socket...");

	switch (bound_address->ai_family) {
	case AF_INET:
		return handle_mcast4_opts(cfg);
	case AF_INET6:
		return handle_mcast6_opts(cfg);
	}

	syslog(LOG_INFO, "I don't know how to tweak multicast socket options for address family %d.",
			bound_address->ai_family);
	return 1;
}

int netsocket_setup(int argc, char **argv)
{
	cJSON *json;
	struct netsocket_config cfg;
	int error;

	error = read_json(argc, argv, &json);
	if (error)
		return error;

	error = json_to_config(json, &cfg);
	if (error)
		goto end;

	error = create_socket(&cfg);
	if (error)
		goto end;

	error = adjust_mcast_opts(&cfg);
	if (error) {
		close(sk);
		freeaddrinfo(addr_candidates);
		goto end;
	}
	/* Fall through. */

end:
	cJSON_Delete(json);
	return error;
}

void netsocket_teardown(void)
{
	close(sk);
	freeaddrinfo(addr_candidates);
}

void *netsocket_listen(void *arg)
{
	char buffer[JOOLD_MAX_PAYLOAD];
	int bytes;

	syslog(LOG_INFO, "Listening...");

	do {
		bytes = recv(sk, buffer, sizeof(buffer), 0);
		if (bytes < 0) {
			pr_perror("Error receiving packet from the network",
					errno);
			continue;
		}

		syslog(LOG_DEBUG, "Received %d bytes from the network.", bytes);
		modsocket_send(buffer, bytes);
	} while (true);

	return NULL;
}

void netsocket_send(void *buffer, size_t size)
{
	int bytes;

	syslog(LOG_DEBUG, "Sending %zu bytes to the network...", size);
	bytes = sendto(sk, buffer, size, 0,
			bound_address->ai_addr,
			bound_address->ai_addrlen);
	if (bytes < 0)
		pr_perror("Could not send a packet to the network", errno);
	else
		syslog(LOG_DEBUG, "Sent %d bytes to the network.\n", bytes);
}
