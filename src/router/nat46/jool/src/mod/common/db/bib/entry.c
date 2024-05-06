#include "entry.h"
#include "mod/common/address.h"

bool session_equals(const struct session_entry *s1,
		const struct session_entry *s2)
{
	return taddr6_equals(&s1->src6, &s2->src6)
			&& taddr6_equals(&s1->dst6, &s2->dst6)
			&& taddr4_equals(&s1->src4, &s2->src4)
			&& taddr4_equals(&s1->dst4, &s2->dst4)
			&& (s1->proto == s2->proto);
}
