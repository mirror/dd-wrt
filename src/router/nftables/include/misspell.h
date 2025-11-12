#ifndef _MISSPELL_H_
#define _MISSPELL_H_

struct string_misspell_state {
	unsigned int	min_distance;
	void		*obj;
};

void string_misspell_init(struct string_misspell_state *st);
int string_misspell_update(const char *a, const char *b,
			   void *obj, struct string_misspell_state *st);

#endif
