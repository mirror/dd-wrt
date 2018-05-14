#include "hashmap.h"
#include <string.h>

uint32_t djb2_hash(const void *str)
{
	int c;
	const char *s = str;
	uint32_t hash = 5381;

	while ((c = *s++))
		hash = ((hash << 5) + hash) + c;
	return hash;
}

struct hashmap *hashmap_create(uint32_t(*hash_fct)(const void*),
			       void(*free_fct)(void*), size_t size)
{
	struct hashmap *h = calloc(sizeof(struct hashmap) +
				      sizeof(struct hashmap_entry) * size, 1);
	h->size = size;
	h->free = free_fct;
	h->hash = hash_fct;
	h->first = h->last = NULL;
	return h;
}

void hashmap_add(struct hashmap *h, void *data, const void *key)
{
	uint32_t hash = h->hash(key) % h->size;
	struct hashmap_entry *e = malloc(sizeof(*e));

	e->data = data;
	e->key = key;
	e->next = h->entries[hash];
	h->entries[hash] = e;

	e->list_prev = NULL;
	e->list_next = h->first;
	if (h->first)
		h->first->list_prev = e;
	h->first = e;
	if (!h->last)
		h->last = e;
}

void *hashmap_lookup(struct hashmap *h, const void *key)
{
	struct hashmap_entry *iter;
	uint32_t hash = h->hash(key) % h->size;

	for (iter = h->entries[hash]; iter; iter = iter->next)
		if (!strcmp(iter->key, key))
			return iter->data;
	return NULL;
}

void *hashmap_iter_in_order(struct hashmap *h, struct hashmap_entry **it)
{
	*it = *it ? (*it)->list_next : h->first;
	return *it ? (*it)->data : NULL;
}

void hashmap_free(struct hashmap *h)
{
	for (size_t i = 0; i < h->size; ++i) {
		struct hashmap_entry *it = h->entries[i];
		while (it) {
			struct hashmap_entry *tmp = it->next;
			if (h->free)
				h->free(it->data);
			free(it);
			it = tmp;
		}
	}
	free(h);
}
