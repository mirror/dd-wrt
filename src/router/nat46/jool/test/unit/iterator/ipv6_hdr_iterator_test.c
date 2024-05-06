#include <linux/module.h>
#include <linux/printk.h>

#include "framework/unit_test.h"
#include "mod/common/ipv6_hdr_iterator.c"

MODULE_LICENSE(JOOL_LICENSE);
MODULE_AUTHOR("Alberto Leiva Popper");
MODULE_DESCRIPTION("Header iterator test.");

static const __u8 HDR6_LEN = sizeof(struct ipv6hdr);
static const __u8 FRAG_HDR_LEN = sizeof(struct frag_hdr);
static const __u8 ROUTE_HDR_LEN = 40;
static const __u8 OPT_HDR_LEN = 32; /* Needs to be a multiple of 8. */

static struct ipv6hdr *kmalloc_packet(__u16 payload_len, __u8 nexthdr)
{
	struct ipv6hdr *result = kmalloc(HDR6_LEN + payload_len, GFP_ATOMIC);
	if (!result) {
		log_err("Unable to allocate a test header.");
		return NULL;
	}

	result->nexthdr = nexthdr;
	result->payload_len = cpu_to_be16(payload_len);
	return result;
}

static struct frag_hdr *add_frag_hdr(void *previous_hdr,
		__u16 previous_hdr_len, __u8 nexthdr)
{
	struct frag_hdr *hdr = previous_hdr + previous_hdr_len;
	hdr->nexthdr = nexthdr;
	return hdr;
}

static struct ipv6_opt_hdr *add_route_hdr(void *previous_hdr,
		__u16 previous_hdr_len, __u8 nexthdr)
{
	struct ipv6_opt_hdr *hdr = previous_hdr + previous_hdr_len;
	hdr->nexthdr = NEXTHDR_UDP;
	hdr->hdrlen = (ROUTE_HDR_LEN / 8) - 1;
	return hdr;
}

/**
 * Sometimes this function is also used to obtain ESP headers.
 * I'm aware they don't work like this; for the purposes of the tests that
 * doesn't matter.
 */
static struct ipv6_opt_hdr *add_opt_hdr(void *previous_hdr,
		__u16 previous_hdr_len, __u8 nexthdr)
{
	struct ipv6_opt_hdr *hdr = previous_hdr + previous_hdr_len;
	hdr->nexthdr = nexthdr;
	hdr->hdrlen = (OPT_HDR_LEN / 8) - 1;
	return hdr;
}

static unsigned char *add_payload(void *previous_hdr, __u16 previous_hdr_len)
{
	return previous_hdr + previous_hdr_len;
}

static bool test_next_no_subheaders(void)
{
	bool success = true;
	struct hdr_iterator iterator;

	/* Init */
	struct ipv6hdr *hdr6;
	unsigned char *payload;

	hdr6 = kmalloc_packet(4, NEXTHDR_UDP);
	if (!hdr6)
		return false;
	payload = add_payload(hdr6, HDR6_LEN);

	/* Test */
	hdr_iterator_init(&iterator, hdr6);
	success &= ASSERT_PTR(payload, iterator.data, "Payload1:data");
	success &= ASSERT_UINT(NEXTHDR_UDP, iterator.hdr_type, "Payload1:type");
	if (!success)
		goto end;

	success &= ASSERT_INT(0, hdr_iterator_next(&iterator), "Result1");
	success &= ASSERT_PTR(payload, iterator.data, "Payload2:data");
	success &= ASSERT_UINT(NEXTHDR_UDP, iterator.hdr_type, "Payload2:type");
	if (!success)
		goto end;

	success &= ASSERT_INT(0, hdr_iterator_next(&iterator), "Result2");
	success &= ASSERT_INT(0, hdr_iterator_next(&iterator), "Result3");
	success &= ASSERT_INT(0, hdr_iterator_next(&iterator), "Result4");
	success &= ASSERT_PTR(payload, iterator.data, "Payload3:data");
	success &= ASSERT_UINT(NEXTHDR_UDP, iterator.hdr_type, "Payload3:type");
	/* Fall through. */

end:
	kfree(hdr6);
	return success;
}

static bool test_next_subheaders(void)
{
	bool success = true;
	struct hdr_iterator iterator;

	/* Init */
	struct ipv6hdr *hdr6;
	struct frag_hdr *hdr_frag;
	struct ipv6_opt_hdr *hdr_hop;
	struct ipv6_opt_hdr *hdr_route;
	unsigned char *payload;

	hdr6 = kmalloc_packet(FRAG_HDR_LEN + OPT_HDR_LEN + ROUTE_HDR_LEN + 4,
			NEXTHDR_FRAGMENT);
	if (!hdr6)
		return false;
	hdr_frag = add_frag_hdr(hdr6, HDR6_LEN, NEXTHDR_HOP);
	hdr_hop = add_opt_hdr(hdr_frag, FRAG_HDR_LEN, NEXTHDR_ROUTING);
	hdr_route = add_route_hdr(hdr_hop, OPT_HDR_LEN, NEXTHDR_UDP);
	payload = add_payload(hdr_route, ROUTE_HDR_LEN);

	/* Test */
	hdr_iterator_init(&iterator, hdr6);
	success &= ASSERT_PTR(hdr_frag, iterator.data, "Frag:data");
	success &= ASSERT_UINT(NEXTHDR_FRAGMENT, iterator.hdr_type,
			"Frag:type");
	if (!success)
		goto end;

	success &= ASSERT_INT(EAGAIN, hdr_iterator_next(&iterator), "Next 1");
	success &= ASSERT_PTR(hdr_hop, iterator.data, "Hop:data");
	success &= ASSERT_UINT(NEXTHDR_HOP, iterator.hdr_type, "Hop:type");
	if (!success)
		goto end;

	success &= ASSERT_INT(EAGAIN, hdr_iterator_next(&iterator), "Next 2");
	success &= ASSERT_PTR(hdr_route, iterator.data, "Routing:data");
	success &= ASSERT_UINT(NEXTHDR_ROUTING, iterator.hdr_type,
			"Routing:type");
	if (!success)
		goto end;

	success &= ASSERT_INT(EAGAIN, hdr_iterator_next(&iterator), "Next 3");
	success &= ASSERT_PTR(payload, iterator.data, "Payload1:data");
	success &= ASSERT_UINT(NEXTHDR_UDP, iterator.hdr_type, "Payload1:type");
	if (!success)
		goto end;

	success &= ASSERT_INT(0, hdr_iterator_next(&iterator), "Next 4");
	success &= ASSERT_PTR(payload, iterator.data, "Payload2:data");
	success &= ASSERT_UINT(NEXTHDR_UDP, iterator.hdr_type, "Payload2:type");
	/* Fall through. */

end:
	kfree(hdr6);
	return success;
}

static bool test_next_unsupported(void)
{
	bool success = true;
	struct hdr_iterator iterator;

	/* Init */
	struct ipv6hdr *hdr6;
	struct frag_hdr *hdr_frag;
	struct ipv6_opt_hdr *hdr_esp;
	struct ipv6_opt_hdr *hdr_route;
	unsigned char *payload;

	hdr6 = kmalloc_packet(FRAG_HDR_LEN + OPT_HDR_LEN + ROUTE_HDR_LEN + 4,
			NEXTHDR_FRAGMENT);
	if (!hdr6)
		return false;
	hdr_frag = add_frag_hdr(hdr6, HDR6_LEN, NEXTHDR_ESP);
	hdr_esp = add_opt_hdr(hdr_frag, FRAG_HDR_LEN, FRAG_HDR_LEN);
	hdr_route = add_route_hdr(hdr_esp, OPT_HDR_LEN, NEXTHDR_UDP);
	payload = add_payload(hdr_route, ROUTE_HDR_LEN);

	/* Test */
	hdr_iterator_init(&iterator, hdr6);
	success &= ASSERT_PTR(hdr_frag, iterator.data, "Frag:pointer");
	success &= ASSERT_UINT(NEXTHDR_FRAGMENT, iterator.hdr_type,
			"Frag:type");
	if (!success)
		goto end;

	success &= ASSERT_INT(EAGAIN, hdr_iterator_next(&iterator), "Next 1");
	success &= ASSERT_PTR(hdr_esp, iterator.data, "ESP1:pointer");
	success &= ASSERT_UINT(NEXTHDR_ESP, iterator.hdr_type, "ESP1:type");
	if (!success)
		goto end;

	success &= ASSERT_INT(0, hdr_iterator_next(&iterator), "Next 2");
	success &= ASSERT_PTR(hdr_esp, iterator.data, "ESP2:pointer");
	success &= ASSERT_UINT(NEXTHDR_ESP, iterator.hdr_type, "ESP2:type");
	/* Fall through. */

end:
	kfree(hdr6);
	return success;
}

static bool test_last_no_subheaders(void)
{
	bool success = true;
	struct hdr_iterator iterator;

	/* Init */
	struct ipv6hdr *hdr6;
	unsigned char *payload;

	hdr6 = kmalloc_packet(4, NEXTHDR_UDP);
	if (!hdr6)
		return false;
	payload = add_payload(hdr6, HDR6_LEN);

	/* Test */
	hdr_iterator_init(&iterator, hdr6);
	hdr_iterator_last(&iterator);
	success &= ASSERT_PTR(payload, iterator.data, "Last:data");
	success &= ASSERT_UINT(NEXTHDR_UDP, iterator.hdr_type, "Last:type");

	/* End */
	kfree(hdr6);
	return success;
}

static bool test_last_subheaders(void)
{
	bool success = true;
	struct hdr_iterator iterator;

	/* Init */
	struct ipv6hdr *hdr6;
	struct frag_hdr *hdr_frag;
	struct ipv6_opt_hdr *hdr_hop;
	struct ipv6_opt_hdr *hdr_route;
	unsigned char *payload;

	hdr6 = kmalloc_packet(FRAG_HDR_LEN + OPT_HDR_LEN + ROUTE_HDR_LEN + 4,
			NEXTHDR_FRAGMENT);
	if (!hdr6)
		return false;
	hdr_frag = add_frag_hdr(hdr6, HDR6_LEN, NEXTHDR_HOP);
	hdr_hop = add_opt_hdr(hdr_frag, FRAG_HDR_LEN, NEXTHDR_ROUTING);
	hdr_route = add_route_hdr(hdr_hop, OPT_HDR_LEN, NEXTHDR_UDP);
	payload = add_payload(hdr_route, ROUTE_HDR_LEN);

	/* Test */
	hdr_iterator_init(&iterator, hdr6);
	hdr_iterator_last(&iterator);
	success &= ASSERT_PTR(payload, iterator.data, "Last:data");
	success &= ASSERT_UINT(NEXTHDR_UDP, iterator.hdr_type, "Last:type");

	/* End */
	kfree(hdr6);
	return success;
}

static bool test_last_unsupported(void)
{
	bool success = true;
	struct hdr_iterator iterator;

	/* Init */
	struct ipv6hdr *hdr6;
	struct frag_hdr *hdr_frag;
	struct ipv6_opt_hdr *hdr_esp;
	struct ipv6_opt_hdr *hdr_route;
	unsigned char *payload;

	hdr6 = kmalloc_packet(FRAG_HDR_LEN + OPT_HDR_LEN + ROUTE_HDR_LEN + 4,
			NEXTHDR_FRAGMENT);
	if (!hdr6)
		return false;
	hdr_frag = add_frag_hdr(hdr6, HDR6_LEN, NEXTHDR_ESP);
	hdr_esp = add_opt_hdr(hdr_frag, FRAG_HDR_LEN, FRAG_HDR_LEN);
	hdr_route = add_route_hdr(hdr_esp, OPT_HDR_LEN, NEXTHDR_UDP);
	payload = add_payload(hdr_route, ROUTE_HDR_LEN);

	/* Test */
	hdr_iterator_init(&iterator, hdr6);
	hdr_iterator_last(&iterator);
	success &= ASSERT_PTR(hdr_esp, iterator.data, "Last:data");
	success &= ASSERT_UINT(NEXTHDR_ESP, iterator.hdr_type, "Last:type");

	/* End */
	kfree(hdr6);
	return success;
}

static bool test_find_no_subheaders(void)
{
	bool success = true;

	/* Init */
	struct ipv6hdr *hdr6;
	unsigned char *payload;

	hdr6 = kmalloc_packet(4, NEXTHDR_UDP);
	if (!hdr6)
		return false;
	payload = add_payload(hdr6, HDR6_LEN);

	/* Test */
	success &= ASSERT_PTR(NULL, hdr_iterator_find(hdr6, NEXTHDR_FRAGMENT),
			"Frag hdr");
	success &= ASSERT_PTR(NULL, hdr_iterator_find(hdr6, NEXTHDR_HOP),
			"Hop-by-hop hdr");
	success &= ASSERT_PTR(NULL, hdr_iterator_find(hdr6, NEXTHDR_ESP),
			"ESP hdr");
	success &= ASSERT_PTR(hdr6 + 1, hdr_iterator_find(hdr6, NEXTHDR_UDP),
			"Payload");

	/* End */
	kfree(hdr6);
	return success;
}

static bool test_find_subheaders(void)
{
	bool success = true;

	/* Init */
	struct ipv6hdr *hdr6;
	struct frag_hdr *hdr_frag;
	struct ipv6_opt_hdr *hdr_hop;
	struct ipv6_opt_hdr *hdr_route;
	unsigned char *payload;

	hdr6 = kmalloc_packet(FRAG_HDR_LEN + OPT_HDR_LEN + ROUTE_HDR_LEN + 4,
			NEXTHDR_FRAGMENT);
	if (!hdr6)
		return false;
	hdr_frag = add_frag_hdr(hdr6, HDR6_LEN, NEXTHDR_HOP);
	hdr_hop = add_opt_hdr(hdr_frag, FRAG_HDR_LEN, NEXTHDR_ROUTING);
	hdr_route = add_route_hdr(hdr_hop, OPT_HDR_LEN, NEXTHDR_UDP);
	payload = add_payload(hdr_route, ROUTE_HDR_LEN);

	/* Test */
	success &= ASSERT_PTR(hdr_frag,
			hdr_iterator_find(hdr6, NEXTHDR_FRAGMENT),
			"Frag hdr");
	success &= ASSERT_PTR(hdr_hop, hdr_iterator_find(hdr6, NEXTHDR_HOP),
			"Hop-by-hop hdr");
	success &= ASSERT_PTR(NULL, hdr_iterator_find(hdr6, NEXTHDR_ESP),
			"ESP hdr");
	success &= ASSERT_PTR(hdr_route,
			hdr_iterator_find(hdr6, NEXTHDR_ROUTING),
			"Routing hdr");
	success &= ASSERT_PTR(payload, hdr_iterator_find(hdr6, NEXTHDR_UDP),
			"Payload");

	/* End */
	kfree(hdr6);
	return success;
}

static bool test_find_unsupported(void)
{
	bool success = true;

	/* Init */
	struct ipv6hdr *hdr6;
	struct frag_hdr *hdr_frag;
	struct ipv6_opt_hdr *hdr_esp;
	struct ipv6_opt_hdr *hdr_route;
	unsigned char *payload;

	hdr6 = kmalloc_packet(FRAG_HDR_LEN + OPT_HDR_LEN + ROUTE_HDR_LEN + 4,
			NEXTHDR_FRAGMENT);
	if (!hdr6)
		return false;
	hdr_frag = add_frag_hdr(hdr6, HDR6_LEN, NEXTHDR_ESP);
	hdr_esp = add_opt_hdr(hdr_frag, FRAG_HDR_LEN, FRAG_HDR_LEN);
	hdr_route = add_route_hdr(hdr_esp, OPT_HDR_LEN, NEXTHDR_UDP);
	payload = add_payload(hdr_route, ROUTE_HDR_LEN);

	/* Test */
	success &= ASSERT_PTR(hdr_frag,
			hdr_iterator_find(hdr6, NEXTHDR_FRAGMENT),
			"Frag hdr");
	success &= ASSERT_PTR(hdr_esp, hdr_iterator_find(hdr6, NEXTHDR_ESP),
			"ESP header");
	/* The ESP header is in the way. */
	success &= ASSERT_PTR(NULL, hdr_iterator_find(hdr6, NEXTHDR_ROUTING),
			"Routing header");
	/* Same, but that isn't an extension header anyway. */
	success &= ASSERT_PTR(NULL, hdr_iterator_find(hdr6, NEXTHDR_UDP),
			"Payload");

	/* End */
	kfree(hdr6);
	return success;
}

int init_module(void)
{
	struct test_group test = {
		.name = "IPv6 header iterator",
	};

	if (test_group_begin(&test))
		return -EINVAL;

	test_group_test(&test, test_next_no_subheaders, "next function, no subheaders");
	test_group_test(&test, test_next_subheaders, "next function, subheaders");
	test_group_test(&test, test_next_unsupported, "next function, unsupported hdrs");

	test_group_test(&test, test_last_no_subheaders, "last function, no subheaders");
	test_group_test(&test, test_last_subheaders, "last function, subheaders");
	test_group_test(&test, test_last_unsupported, "last function, unsupported hdrs");

	test_group_test(&test, test_find_no_subheaders, "find function, no subheaders");
	test_group_test(&test, test_find_subheaders, "find function, subheaders");
	test_group_test(&test, test_find_unsupported, "find function, unsupported hdrs");

	return test_group_end(&test);
}

void cleanup_module(void)
{
	/* No code. */
}
