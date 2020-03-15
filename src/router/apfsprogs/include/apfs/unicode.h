/*
 * Copyright (C) 2019 Ernesto A. Fernández <ernesto.mnd.fernandez@gmail.com>
 */

#ifndef _UNICODE_H
#define _UNICODE_H

#include <apfs/types.h>

/*
 * This structure helps normalize_next() to retrieve one normalized
 * (and case-folded) UTF-32 character at a time from a UTF-8 string.
 */
struct unicursor {
	const char *utf8curr;	/* Start of UTF-8 to decompose and reorder */
	int length;		/* Length of normalization until next starter */
	int last_pos;           /* Offset in substring of last char returned */
	u8 last_ccc;		/* CCC of the last character returned */
};

extern void init_unicursor(struct unicursor *cursor, const char *utf8str);
extern unicode_t normalize_next(struct unicursor *cursor, bool case_fold);

#endif	/* _UNICODE_H */
