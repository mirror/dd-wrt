#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <netlink/netlink.h>
#include <netlink/helpers.h>
#include <netlink/route/addr.h>
#include <netlink/route/link.h>

static void
print_usage(void)
{
	printf(
	"Usage:\n"                          \
	"  addr list [{brief|full|stats}] [filter]\n" \
	"  addr set DEV {add|del} filter [label]\n" \
	"\n" \
	"  filter := local ADDR[/PREFIX]\n" \
	"  label := label DEV:NAME\n" );
	exit(1);
}

static struct nl_cache ac = RTNL_INIT_ADDR_CACHE();
static struct nl_cache lc = RTNL_INIT_LINK_CACHE();

static void
get_filter(struct rtnl_addr *l, int* next_arg, const int argc, char** argv, int mandatory)
{
    /* filter maps to exactly 2 arguments */
	if (argc - *next_arg >= 2) {
		if (!strcasecmp(argv[*next_arg], "local")) {
		    ++(*next_arg);
			rtnl_addr_set_local_str(l, argv[*next_arg]);
		    ++(*next_arg);
		} else {
			fprintf(stderr, "What is '%s'?\n", argv[*next_arg]);
			exit(1);
		}
	} else if (mandatory) {
		fprintf(stderr, "Missing or incomplete mandatory filter.\n");
		exit(1);
	}
}

static void
get_label(struct rtnl_addr *l, int* next_arg, const int argc, char** argv, char* devname)
{
    size_t devname_len;
    /* filter maps to exactly 2 arguments */
	if (argc - *next_arg >= 2) {
		if (!strcasecmp(argv[*next_arg], "label")) {
		    ++(*next_arg);
		    devname_len = strlen(devname);
		    if (strncmp(devname, argv[*next_arg], devname_len) == 0 &&
		        argv[*next_arg][devname_len] == ':') {
        		l->a_mask |= ADDR_HAS_LABEL;
        		strncpy(l->a_label, argv[*next_arg], IFNAMSIZ*2);
        		l->a_label[IFNAMSIZ*2] = '\0';
		    } else {
		        fprintf(stderr, "Invalid label %s\n", argv[*next_arg]);
                exit(1);
		    }
		    ++(*next_arg);
		} else {
			fprintf(stderr, "What is '%s'?\n", argv[*next_arg]);
			exit(1);
		}
	}
}

int
main(int argc, char *argv[])
{
	struct nl_handle h = NL_INIT_HANDLE();
	struct rtnl_addr a = RTNL_INIT_ADDR();
	struct nl_dump_params params = {
		.dp_type = NL_DUMP_BRIEF,
	};
	int next_arg = 1;

	nl_use_default_verbose_handlers(&h);

	if (argc < 2)
		print_usage();

	if (nl_connect(&h, NETLINK_ROUTE) < 0)
		fprintf(stderr, "%s\n", nl_geterror());

	if (nl_cache_update(&h, &ac) < 0)
		fprintf(stderr, "%s\n", nl_geterror());

	if (nl_cache_update(&h, &lc) < 0)
		fprintf(stderr, "%s\n", nl_geterror());

	nl_cache_provide(&ac);
	nl_cache_provide(&lc);

	if (!strcasecmp(argv[next_arg], "list")) {
	    ++next_arg;
	    /* 1 optional argument */
		if (argc >= (next_arg + 1)) {
			if (!strcasecmp(argv[next_arg], "brief")) {
				++next_arg;
				params.dp_type = NL_DUMP_BRIEF;
			} else if (!strcasecmp(argv[next_arg], "full")) {
				++next_arg;
				params.dp_type = NL_DUMP_FULL;
			} else if (!strcasecmp(argv[next_arg], "stats")) {
				++next_arg;
				params.dp_type = NL_DUMP_STATS;
			}
		}
		get_filter(&a, &next_arg, argc, argv, 0);

		nl_cache_dump_filter(&ac, stdout, &params, (struct nl_common *) &a);

	} else if (!strcasecmp(argv[next_arg], "set")) {
		int i;
		++next_arg;
		/* 3 mandatory arguments */
		if (argc >= (next_arg + 3)) {
			char* devname = argv[next_arg];
			i = rtnl_link_name2i(&lc, devname);
			if (RTNL_LINK_NOT_FOUND == i) {
				fprintf(stderr, "Link %s was not found\n", argv[next_arg]);
				exit(1);
			}
			++next_arg;

			if (!strcasecmp(argv[next_arg], "add")) {
			    ++next_arg;
				get_filter(&a, &next_arg, argc, argv, 1);
				a.a_ifindex = i;
				a.a_mask |= ADDR_HAS_IFINDEX;
				get_label(&a, &next_arg, argc, argv, devname);
				if (rtnl_addr_add(&h, &a) < 0) {
					fprintf(stderr, "%s\n", nl_geterror());
					exit(1);
				}
			} else if (!strcasecmp(argv[next_arg], "del")) {
				++next_arg;
				get_filter(&a, &next_arg, argc, argv, 1);
				a.a_ifindex = i;
				a.a_mask |= ADDR_HAS_IFINDEX;
				if (rtnl_addr_delete(&h, &a) < 0) {
					fprintf(stderr, "%s\n", nl_geterror());
					exit(1);
				}
			} else print_usage();
		} else print_usage();
	} else print_usage();

	nl_cache_destroy(&ac);
	nl_close(&h);

	return 0;
}
