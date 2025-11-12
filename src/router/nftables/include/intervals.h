#ifndef NFTABLES_INTERVALS_H
#define NFTABLES_INTERVALS_H

int set_automerge(struct list_head *msgs, struct cmd *cmd, struct set *set,
		  struct expr *init, unsigned int debug_mask);
int set_delete(struct list_head *msgs, struct cmd *cmd, struct set *set,
	       struct expr *init, unsigned int debug_mask);
int set_overlap(struct list_head *msgs, struct set *set, struct expr *init);
int set_to_intervals(const struct set *set, struct expr *init, bool add);
int setelem_to_interval(const struct set *set, struct expr *elem,
			struct expr *next_elem, struct list_head *interval_list);

#endif
