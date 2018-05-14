#ifndef HASHMAP_H
# define HASHMAP_H

# include <stdlib.h>
# include <stdint.h>

struct hashmap {
	uint32_t size;
	uint32_t(*hash)(const void *key);
	void(*free)(void*);
	struct hashmap_entry *first;
	struct hashmap_entry *last;
	struct hashmap_entry {
		void *data;
		const void *key;
		struct hashmap_entry *next;
		struct hashmap_entry *list_next;
		struct hashmap_entry *list_prev;
	} *entries[0];
};

struct hashmap *hashmap_create(uint32_t(*hash_fct)(const void*),
			       void(*free_fct)(void*), size_t size);
void hashmap_add(struct hashmap *h, void *data, const void *key);
void *hashmap_lookup(struct hashmap *h, const void *key);
void *hashmap_iter_in_order(struct hashmap *h, struct hashmap_entry **it);
void hashmap_del(struct hashmap *h, struct hashmap_entry *e);
void hashmap_free(struct hashmap *h);

uint32_t djb2_hash(const void *str);

#endif /* !HASHMAP_H */
