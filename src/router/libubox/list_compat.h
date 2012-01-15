#ifndef __LIST_COMPAT_H
#define __LIST_COMPAT_H

#define list_entity				list_head

#define list_init_head(_list)			INIT_LIST_HEAD(_list)
#define list_add_head(_head, _list)		list_add(_list, _head)
#define list_add_after(_after, _list)		list_add(_list, _after)
#define list_add_before(_before, _list)		list_add_tail(_list, _before)
#define list_remove(_list)			list_del(_list)
#define list_is_empty(_list)			list_empty(_list)
#define list_next_element(_element, _member)	list_entry((_element)->_member.next, typeof(*(_element)), _member)


#endif
