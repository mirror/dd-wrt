/* GPL v2, lend from the linux kernel */
#ifndef _LINUX_HEAD_H
#define _LINUX_HEAD_H

#include <stddef.h>

struct dd_list_head {
	struct dd_list_head *next, *prev;
};
#endif
