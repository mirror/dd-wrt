#include <libiptc/libip4tc.c>
void getpacketcounts(char *table, char *chain, unsigned long long *counts, int len)
{
	int c = 0;
	const char *this;
	if (!len)
		return;
	iptc_handle_t handle = iptc_init(table);

	for (this = iptc_first_chain(&handle); this; this = iptc_next_chain(&handle)) {
		const struct ipt_entry *i;
		if (strcmp(chain, this))
			continue;
		i = iptc_first_rule(this, &handle);
		while (i) {
			const char *target = iptc_get_target(i, &handle);
			if (target && !strcmp(target, "RETURN"))
				goto skip;
			counts[c++] = i->counters.pcnt;
			if (c == len) {
				iptc_free(&handle);
				return;
			}
skip:;
			i = iptc_next_rule(i, &handle);
		}
	}
	iptc_free(&handle);
}

unsigned long long getpackettotal(char *table, char *chain)
{
	int c = 0;
	unsigned long long count = 0;
	const char *this;
	iptc_handle_t handle = iptc_init(table);

	for (this = iptc_first_chain(&handle); this; this = iptc_next_chain(&handle)) {
		const struct ipt_entry *i;
		if (strcmp(chain, this))
			continue;
		i = iptc_first_rule(this, &handle);
		while (i) {
			const char *target = iptc_get_target(i, &handle);
			if (target && !strcmp(target, "RETURN"))
				goto skip;
			count += i->counters.pcnt;
skip:;
			i = iptc_next_rule(i, &handle);
		}
	}
	iptc_free(&handle);
	return count;
}
