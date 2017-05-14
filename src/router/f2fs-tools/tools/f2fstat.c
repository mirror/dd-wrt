#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <libgen.h>

#ifdef DEBUG
#define dbg(fmt, args...)	printf(fmt, __VA_ARGS__);
#else
#define dbg(fmt, args...)
#endif

/*
 * f2fs status
 */
#define F2FS_STATUS	"/sys/kernel/debug/f2fs/status"

#define KEY_NODE	0x00000001
#define KEY_META	0x00000010

unsigned long util;
unsigned long used_node_blks;
unsigned long used_data_blks;
//unsigned long inline_inode;

unsigned long free_segs;
unsigned long valid_segs;
unsigned long dirty_segs;
unsigned long prefree_segs;

unsigned long gc, bg_gc;
unsigned long cp;
unsigned long gc_data_blks;
unsigned long gc_node_blks;

//unsigned long extent_hit_ratio;

unsigned long dirty_node, node_kb;
unsigned long dirty_dents;
unsigned long dirty_meta, meta_kb;
unsigned long nat_caches;
unsigned long dirty_sit;

unsigned long free_nids;

unsigned long ssr_blks;
unsigned long lfs_blks;
unsigned long memory_kb;

struct options {
	int delay;
	int interval;
	char partname[32];
};

struct mm_table {
	const char *name;
	unsigned long *val;
	int flag;
};

static int compare_mm_table(const void *a, const void *b)
{
	dbg("[COMPARE] %s, %s\n", ((struct mm_table *)a)->name, ((struct mm_table *)b)->name);
	return strcmp(((struct mm_table *)a)->name, ((struct mm_table *)b)->name);
}

static inline void remove_newline(char **head)
{
again:
	if (**head == '\n') {
		*head = *head + 1;
		goto again;
	}
}

void f2fstat(struct options *opt)
{
	int fd;
	int ret;
	char keyname[32];
	char buf[4096];
	struct mm_table key = { keyname, NULL };
	struct mm_table *found;
	int f2fstat_table_cnt;
	char *head, *tail;
	int found_cnt = 0;

	static struct mm_table f2fstat_table[] = {
		{ "  - Data",		&used_data_blks,	0 },
		{ "  - Dirty",		&dirty_segs,		0 },
		{ "  - Free",		&free_segs,		0 },
		{ "  - NATs",		&nat_caches,		0 },
		{ "  - Node",		&used_node_blks,	0 },
		{ "  - Prefree",	&prefree_segs,		0 },
		{ "  - SITs",		&dirty_sit,		0 },
		{ "  - Valid",		&valid_segs,		0 },
		{ "  - dents",		&dirty_dents,		0 },
		{ "  - free_nids",	&free_nids,		0 },
		{ "  - meta",		&dirty_meta,		KEY_META },
		{ "  - nodes",		&dirty_node,		KEY_NODE },
		{ "CP calls",		&cp,			0 },
		{ "GC calls",		&gc,			0 },
		{ "LFS",		&lfs_blks,		0 },
		{ "Memory",		&memory_kb,		0 },
		{ "SSR",		&ssr_blks,		0 },
		{ "Utilization",	&util,			0 },
	};

	f2fstat_table_cnt = sizeof(f2fstat_table)/sizeof(struct mm_table);

	fd = open(F2FS_STATUS, O_RDONLY);
	if (fd < 0) {
		perror("open " F2FS_STATUS);
		exit(EXIT_FAILURE);
	}

	ret = read(fd, buf, 4096);
	if (ret < 0) {
		perror("read " F2FS_STATUS);
		exit(EXIT_FAILURE);
	}
	buf[ret] = '\0';

	head = buf;

	if (opt->partname[0] != '\0') {
		head = strstr(buf, opt->partname);
		if (head == NULL)
			exit(EXIT_FAILURE);
	}

	for (;;) {
		remove_newline(&head);
		tail = strchr(head, ':');
		if (!tail)
			break;
		*tail = '\0';
		if (strlen(head) >= sizeof(keyname)) {
			dbg("[OVER] %s\n", head);
			*tail = ':';
			tail = strchr(head, '\n');
			head = tail + 1;
			continue;
		}

		strcpy(keyname, head);

		found = bsearch(&key, f2fstat_table, f2fstat_table_cnt, sizeof(struct mm_table), compare_mm_table);
		dbg("[RESULT] %s (%s)\n", head, (found) ? "O" : "X");
		head = tail + 1;
		if (!found)
			goto nextline;

		*(found->val) = strtoul(head, &tail, 10);
		if (found->flag) {
			int npages;
			tail = strstr(head, "in");
			head = tail + 2;
			npages = strtoul(head, &tail, 10);
			switch (found->flag & (KEY_NODE | KEY_META)) {
			case KEY_NODE:
				node_kb = npages * 4;
				break;
			case KEY_META:
				meta_kb = npages * 4;
				break;
			}
		}
		if (++found_cnt == f2fstat_table_cnt)
			break;
nextline:
		tail = strchr(head, '\n');
		if (!tail)
			break;
		head =  tail + 1;
	}

	close(fd);
}

void usage(void)
{
	printf("Usage: f2fstat [option]\n"
			"    -d    delay (secs)\n"
			"    -i    interval of head info\n"
			"    -p    partition name (e.g. /dev/sda3)\n");
	exit(EXIT_FAILURE);
}

void parse_option(int argc, char *argv[], struct options *opt)
{
	int option;
	const char *option_string = "d:i:p:h";

	while ((option = getopt(argc, argv, option_string)) != EOF) {
		switch (option) {
		case 'd':
			opt->delay = atoi(optarg);
			break;
		case 'i':
			opt->interval = atoi(optarg);
			break;
		case 'p':
			strcpy(opt->partname, basename(optarg));
			break;
		default:
			usage();
			break;
		}
	}
}

void __make_head(char *head, int index, int i, int len)
{
	char name_h[5][20] = {"main segments", "page/slab caches", "cp/gc", "blks", "memory"};
	int half = (len - strlen(name_h[i])) / 2;

	*(head + index) = '|';
	index++;
	memset(head + index, '-', half);
	index += half;
	strcpy(head + index, name_h[i]);
	index += strlen(name_h[i]);
	memset(head + index, '-', half);
}

void print_head(char *res)
{
	char *ptr, *ptr_buf;
	char buf[1024], head[1024];
	char name[20][10] = {"util", "node", "data", "free", "valid", "dirty", "prefree", "node", "dent", "meta",
		"sit", "nat", "fnid", "cp", "gc", "ssr", "lfs", "total", "node", "meta"};
	int i, len, prev_index = 0;

	ptr_buf = buf;
	memset(buf, ' ', 1024);
	memset(head, ' ', 1024);

	for (i = 0; i < 20; i++) {
		ptr = (i == 0) ? strtok(res, " ") : strtok(NULL, " ");
		strncpy(ptr_buf, name[i], strlen(name[i]));
		if (i == 1) {
			prev_index = ptr_buf - buf - 1;
		} else if (i == 7) {
			len = (ptr_buf - buf) - 1 - prev_index;
			__make_head(head, prev_index, 0, len);
			prev_index = ptr_buf - buf - 1;
		} else if (i == 13) {
			len = (ptr_buf - buf) - 1 - prev_index;
			__make_head(head, prev_index, 1, len);
			prev_index = ptr_buf - buf - 1;
		} else if (i == 15) {
			len = (ptr_buf - buf) - 1 - prev_index;
			__make_head(head, prev_index, 2, len);
			prev_index = ptr_buf - buf - 1;
		} else if (i == 17) {
			len = (ptr_buf - buf) - 1 - prev_index;
			__make_head(head, prev_index, 3, len);
			prev_index = ptr_buf - buf - 1;
		}

		len = strlen(ptr);
		ptr_buf += (len > strlen(name[i]) ? len : strlen(name[i])) + 1;
	}

	len = (ptr_buf - buf) - 1 - prev_index;
	__make_head(head, prev_index, 4, len);

	*ptr_buf = 0;
	*(head + (ptr_buf - buf - 1)) = '|';
	*(head + (ptr_buf - buf)) = 0;
	fprintf(stderr, "%s\n%s\n", head, buf);
}

int main(int argc, char *argv[])
{
	char format[] = "%4ld %4ld %4ld %4ld %5ld %5ld %7ld %4ld %4ld %4ld %3ld %3ld %4ld %2ld %2ld %3ld %3ld %5ld %4ld %4ld";
	char buf[1024], tmp[1024];
	int head_interval;
	struct options opt = {
		.delay = 1,
		.interval = 20,
		.partname = { 0, },
	};

	parse_option(argc, argv, &opt);
	head_interval = opt.interval;

	while (1) {
		memset(buf, 0, 1024);
		f2fstat(&opt);
		sprintf(buf, format, util, used_node_blks, used_data_blks,
			free_segs, valid_segs, dirty_segs, prefree_segs,
			dirty_node, dirty_dents, dirty_meta, dirty_sit, nat_caches, free_nids,
			cp, gc, ssr_blks, lfs_blks, memory_kb, node_kb, meta_kb);

		strcpy(tmp, buf);
		if (head_interval == opt.interval)
			print_head(tmp);
		if (head_interval-- == 0)
			head_interval = opt.interval;

		fprintf(stderr, "%s\n", buf);

		sleep(opt.delay);
	}

	return 0;
}
