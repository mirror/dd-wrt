#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/vmalloc.h>

#include "framework/unit_test.h"
#include "usr/common/str_utils.h"
#include "mod/nat64/pool4/db.c"

MODULE_LICENSE(JOOL_LICENSE);
MODULE_AUTHOR("Alberto Leiva");
MODULE_DESCRIPTION("");


static unsigned int RANGE_COUNT = 2;
module_param(RANGE_COUNT, uint, 0);
MODULE_PARM_DESC(RANGE_COUNT, "Number of addresses to insert to the dummy pool4. Min 1, max 256, default 2.");

static unsigned int TADDRS_PER_RANGE = 65536;
module_param(TADDRS_PER_RANGE, uint, 0);
MODULE_PARM_DESC(TADDRS_PER_RANGE, "Number of ports per pool4 address. Min 1, max 65536, default 65536. "
		"If RANGE_COUNT times TADDRS_PER_RANGE is higher than 512, "
		"then RANGE_COUNT times TADDRS_PER_RANGE has to be a multiple of 512.");

static unsigned int MAX_ITERATIONS = 0;
module_param(MAX_ITERATIONS, uint, 0);
MODULE_PARM_DESC(MAX_ITERATIONS, "Iteration limit. Same as in Jool's pool4. "
		"Zero stands for infinity. Default is zero.");


/**
 * This is just RANGE_COUNT times TADDRS_PER_RANGE.
 * Stored because it's needed often.
 * Because of the way the print function groups values together, this needs to
 * be smaller than 512 or a multiple of 512.
 */
static unsigned int TADDR_COUNT;

/**
 * A legitimate instance of pool4, used instead of simulated to make sure we're
 * testing the real thing.
 */
static struct pool4 *pool;

/**
 * This is an array used during the test to store results.
 * For every number of transport addresses already taken, stores the numer of
 * times we had to iterate to find an available port.
 *
 * For example, if iterations[123] = 5, then when 123 ports had already been
 * borrowed by previous tests, we needed to iterate 5 times to reserve the 124th
 * one.
 */
static unsigned int *iterations;
/**
 * An array linked to @iterations.
 * For every number of transport addresses already taken, stores whether there
 * was an error when we tried to find the next available port.
 *
 * For example, if errors[123] = true, the test could not find an availabe port
 * for some reason.
 *
 * The reason why I don't want to join @iterations and @errors into a structure
 * and array that is because I don't want the compiler to add slop. These arrays
 * can be really big when testing extreme cases.
 */
static bool *errors;

/**
 * An array that simulates the BIB.
 * For every port in pool4, stores whether it has been reserved or not.
 *
 * For example, if pool4 has 192.0.2.1:10-19 and 192.0.2.2:40-49, then
 * `taken[5] = true` means that 192.0.2.1:15 has been reserved, and
 * `taken[15] = false` means that 192.0.2.2:45 has not been reserved.
 *
 * I'd like to use a legitimate BIB for this, but right now it would be somewhat
 * complicated.
 */
static bool *taken;

static int init(void)
{
	struct pool4_entry entry;
	unsigned int i;
	int error;

	TADDR_COUNT = RANGE_COUNT * TADDRS_PER_RANGE;

	pr_info("RANGE_COUNT: %u\n", RANGE_COUNT);
	pr_info("TADDRS_PER_RANGE: %u\n", TADDRS_PER_RANGE);
	pr_info("TADDR_COUNT (total ports): %u\n", TADDR_COUNT);

	if (RANGE_COUNT < 1 || 256 < RANGE_COUNT) {
		pr_err("Error: RANGE_COUNT is out of range (1-256).");
		return -EINVAL;
	}
	if (TADDRS_PER_RANGE < 1 || 65536 < TADDRS_PER_RANGE) {
		pr_err("Error: TADDRS_PER_RANGE is out of range (1-65536).");
		return -EINVAL;
	}
	if (TADDR_COUNT > 512 && (TADDR_COUNT & 0x1FF) != 0) {
		pr_err("Error: TADDR_COUNT > 512, therefore it needs to be a multiple of 512.\n");
		return -EINVAL;
	}

	/* Initialize arrays */
	error = -ENOMEM;
	iterations = vzalloc(TADDR_COUNT * sizeof(unsigned int) + 1);
	if (!iterations)
		goto just_quit;
	errors = vzalloc(TADDR_COUNT * sizeof(bool) + 1);
	if (!errors)
		goto destroy_iterations_onwards;
	taken = vzalloc(TADDR_COUNT * sizeof(bool));
	if (!taken)
		goto destroy_errors_onwards;

	/* Initialize F() */
	error = rfc6056_setup();
	if (error)
		goto destroy_taken_onwards;
	/* Initialize pool4 */
	pool = pool4db_alloc();
	if (!pool) {
		error = -ENOMEM;
		goto destroy_rfc6056_onwards;
	}

	/* Add the testing addresses to pool4 */
	entry.mark = 0;
	entry.iterations = MAX_ITERATIONS;
	entry.flags = (MAX_ITERATIONS == 0)
			? ITERATIONS_INFINITE
			: ITERATIONS_SET;
	/*
	 * The reason why I'm using ICMP instead of TCP is because TCP and UDP
	 * deny port zero, and that's annoying to consider when reading results.
	 */
	entry.proto = L4PROTO_ICMP;
	entry.range.prefix.len = 32;
	entry.range.ports.min = 0;
	entry.range.ports.max = TADDRS_PER_RANGE - 1;
	for (i = 0; i < RANGE_COUNT; i++) {
		entry.range.prefix.addr.s_addr = cpu_to_be32(0xc0000200 + i);
		error = pool4db_add(pool, &entry);
		if (error)
			goto destroy_pool4_onwards;
	}

	return 0;

destroy_pool4_onwards:
	pool4db_put(pool);
	/* Fall through */
destroy_rfc6056_onwards:
	rfc6056_teardown();
	/* Fall through */
destroy_taken_onwards:
	vfree(taken);
	/* Fall through */
destroy_errors_onwards:
	vfree(errors);
	/* Fall through */
destroy_iterations_onwards:
	vfree(iterations);
	/* Fall thorugh */
just_quit:
	return error;
}

static int compute_slot(struct ipv4_transport_addr *addr)
{
	return (be32_to_cpu(addr->l3.s_addr) & 0xFF) * TADDRS_PER_RANGE
			+ addr->l4;
}

static void claim_slot_anyway(const unsigned int slot)
{
	unsigned int s;

	for (s = slot; s < TADDR_COUNT; s++) {
		if (!taken[s]) {
			taken[s] = true;
			return;
		}
	}

	for (s = 0; s < slot; s++) {
		if (!taken[s]) {
			taken[s] = true;
			return;
		}
	}

	/* pool4 is exhausted; do nothing. */
}

/**
 * Simulates the reception of a packet that will need a new transport address
 * from pool4, from the eyes of pool4. (So no actual packet created; just a
 * bunch of function calls.)
 */
static void new_connection(unsigned int request)
{
	struct tuple tuple6; /* Represents a connection. */
	struct mask_domain *masks; /* Mask candidates for the tuple6 conn. */
	struct ipv4_transport_addr addr; /* Currently iterating address. */
	bool consecutive; /* Dummy; not used. */
	unsigned int attempts = 0; /* Iterations performed so far. */
	unsigned int slot; /* Currently relevant @taken array slot. */

	/* Create a completely random connection */
	get_random_bytes(&tuple6.src, sizeof(tuple6.src));
	get_random_bytes(&tuple6.dst, sizeof(tuple6.dst));
	tuple6.l3_proto = L3PROTO_IPV6;
	tuple6.l4_proto = L4PROTO_ICMP;

	/*
	 * Ask pool4 for the transport address candidates that can mask that
	 * connection.
	 */
	masks = mask_domain_find(pool, &tuple6, 11, 0);
	if (!masks) {
		iterations[request] = 0;
		errors[request] = true;
		return;
	}

	/*
	 * Iterate until we find an available port.
	 * Store the results in the global arrays.
	 */
	while (iterations[request] == 0) {
		if (mask_domain_next(masks, &addr, &consecutive)) {
			iterations[request] = attempts;
			errors[request] = true;
			/*
			 * Needed for the next iteration if mdn() failed because
			 * of MAX_ITERATIONS.
			 */
			claim_slot_anyway(compute_slot(&addr));
			break;
		}
		attempts++;
		slot = compute_slot(&addr);
		if (!taken[slot]) {
			taken[slot] = true;
			iterations[request] = attempts;
			errors[request] = false;
		}
	}

	/* Destroy the masks object. (It belongs to the heap.) */
	mask_domain_put(masks);
}

/**
 * Reserves ports over and over until pool4 is exhausted.
 * Stores iteration costs in the global arrays.
 */
static void test(void)
{
	unsigned int request;
	for (request = 0; request < (TADDR_COUNT + 1); request++)
		new_connection(request);
}

/*
 * The first column is just a dumb counter.
 * The second column is average iterations. (Because the iterations are grouped
 * into at most 512 groups.)
 * The third column is maximum iterations.
 * The fourth column is error count.
 */
static void print_results(void)
{
	unsigned int i, j;
	unsigned int slot;
	unsigned int maximum, total, error_count;
	const unsigned int TADDRS_PER_GROUP = TADDR_COUNT / 512;
	const unsigned int GROUPS = TADDR_COUNT / TADDRS_PER_GROUP;

	/* If pool4 is small, just print all the values normally. */
	if (TADDR_COUNT <= 512) {
		for (i = 0; i < (TADDR_COUNT + 1); i++)
			pr_info("%u %u %u %u\n", i, iterations[i],
					iterations[i], errors[i]);
		return;
	}

	/*
	 * Otherwise, group the values into 512 sets, and just print the
	 * largest value of each set.
	 *
	 * Why? Because
	 *
	 * 1) If we print too much, syslog will get overwhelmed and start
	 *    dropping messages, and
	 * 2) Because I feel that we only really care about the worst case
	 *    scenarios.
	 */
	for (i = 0; i < GROUPS; i++) {
		maximum = 0;
		total = 0;
		error_count = 0;

		for (j = 0; j < TADDRS_PER_GROUP; j++) {
			slot = TADDRS_PER_GROUP * i + j;
			maximum = max(maximum, iterations[slot]);
			total += iterations[slot];
			if (errors[slot])
				error_count++;
		}

		pr_info("%u %u %u %u\n", i, total / TADDRS_PER_GROUP, maximum,
				error_count);
	}

	/*
	 * Last one: should fail because pool4 was exhausted.
	 * Just printing to confirm that everything's normal.
	 */
	pr_info("512 %u %u %u\n", iterations[TADDR_COUNT],
			iterations[TADDR_COUNT], errors[TADDR_COUNT]);
}

static void cleanup(void)
{
	pool4db_put(pool);
	rfc6056_teardown();
	vfree(taken);
	vfree(errors);
	vfree(iterations);
}

/**
 * This is what happens when the user execs `sudo insmod iterations.ko`.
 */
int init_module(void)
{
	int error = init();
	if (error)
		return error;
	test();
	print_results();
	cleanup();
	return 0;
}

/**
 * This is what happens when the user execs `sudo rmmod iterations`.
 */
void cleanup_module(void)
{
	/* No code. */
}
