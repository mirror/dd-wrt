#include "rbtree.h"
#include <linux/module.h>
#include "mod/common/log.h"

void treeslot_init(struct tree_slot *slot,
		struct rb_root *root,
		struct rb_node *entry)
{
	slot->tree = root;
	slot->entry = entry;
	slot->parent = NULL;
	slot->rb_link = &root->rb_node;
}

void treeslot_commit(struct tree_slot *slot)
{
	rb_link_node(slot->entry, slot->parent, slot->rb_link);
	rb_insert_color(slot->entry, slot->tree);
}
