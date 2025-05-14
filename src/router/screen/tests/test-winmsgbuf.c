/* Copyright (c) 2013
 *      Mike Gerwitz (mtg@gnu.org)
 *
 * This file is part of GNU screen.
 *
 * GNU screen is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program (see the file COPYING); if not, see
 * <http://www.gnu.org/licenses>.
 *
 ****************************************************************
 */

#include "../winmsgbuf.h"
#include "signature.h"
#include "macros.h"

SIGNATURE_CHECK(wmb_create, WinMsgBuf *, ());
SIGNATURE_CHECK(wmb_expand, size_t, (WinMsgBuf *, size_t));
SIGNATURE_CHECK(wmb_rendadd, void, (WinMsgBuf *, uint64_t, int));
SIGNATURE_CHECK(wmb_size, size_t, (const WinMsgBuf *));
SIGNATURE_CHECK(wmb_contents, const char *, (const WinMsgBuf *));
SIGNATURE_CHECK(wmb_reset, void, (WinMsgBuf *));
SIGNATURE_CHECK(wmb_free, void, (WinMsgBuf *));

SIGNATURE_CHECK(wmbc_create, WinMsgBufContext *, (WinMsgBuf *));
SIGNATURE_CHECK(wmbc_rewind, void, (WinMsgBufContext *));
SIGNATURE_CHECK(wmbc_fastfw0, void, (WinMsgBufContext *));
SIGNATURE_CHECK(wmbc_fastfw_end, void, (WinMsgBufContext *));
SIGNATURE_CHECK(wmbc_putchar, void, (WinMsgBufContext *, char));
SIGNATURE_CHECK(wmbc_strncpy, const char *, (WinMsgBufContext *, const char *, size_t));
SIGNATURE_CHECK(wmbc_strcpy, const char *, (WinMsgBufContext *, const char *));
SIGNATURE_CHECK(wmbc_printf, int, (WinMsgBufContext *, const char *, ...));
SIGNATURE_CHECK(wmbc_offset, size_t, (WinMsgBufContext *));
SIGNATURE_CHECK(wmbc_bytesleft, size_t, (WinMsgBufContext *));
SIGNATURE_CHECK(wmbc_mergewmb, const char *, (WinMsgBufContext *, WinMsgBuf *));
SIGNATURE_CHECK(wmbc_finish, const char *, (WinMsgBufContext *));
SIGNATURE_CHECK(wmbc_free, void, (WinMsgBufContext *));

int main(void)
{
	{
		WinMsgBuf *wmb = wmb_create();

		/* we should start off with a null-terminated buffer */
		ASSERT(wmb_size(wmb) > 0);
		ASSERT(*wmb_contents(wmb) == '\0');
		/* TODO: rendition state */

		/* buffer shall be expandable to accomodate a minimum number of bytes */
		size_t old = wmb_size(wmb);
		size_t want = old + 3;
		ASSERT_REALLOC(>= want, ASSERT(wmb_expand(wmb, want) >= want));

		/* buffer will not expand if unneeded */
		size_t new = wmb_size(wmb);
		ASSERT_NOALLOC(ASSERT(wmb_expand(wmb, want) == new));
		ASSERT_NOALLOC(ASSERT(wmb_expand(wmb, want - 1) == new));
		ASSERT_NOALLOC(ASSERT(wmb_expand(wmb, 0) == new););

		/* if buffer expansion fails, the original size shall be retained */
		ASSERT_GCC(FAILLOC(wmb_expand(wmb, new + 5)) == new);

		/* resetting should put us back to our starting state, but should do
		 * nothing with the buffer size */
		wmb_reset(wmb);
		ASSERT(*wmb_contents(wmb) == '\0');
		ASSERT(wmb_size(wmb) == new);
		/* TODO: rendition state */

		wmb_free(wmb);
	}

	/* wmb_create should return NULL on allocation failure */
	{
		ASSERT_GCC(FAILLOC(wmb_create()) == NULL);
	}

	/* scenerio: writing to single buffer via separate contexts---while
	 * maintaining separate pointers between them---and retrieving a final
	 * result */
	{
		WinMsgBuf *wmb = wmb_create();
		WinMsgBufContext *wmbc = wmbc_create(wmb);
		WinMsgBufContext *wmbc2 = wmbc_create(wmb);

		/* the offset at this point should be 0 (beginning of buffer) */
		ASSERT(wmbc_offset(wmbc) == 0);
		ASSERT(wmbc_offset(wmbc2) == 0);
		ASSERT(wmbc_bytesleft(wmbc) == wmb_size(wmb));
		ASSERT(wmbc_bytesleft(wmbc2) == wmb_size(wmb));

		/* putting a character should increase the offset and decrease the
		 * number of bytes remaining */
		char c = 'c';
		wmbc_putchar(wmbc, c);
		ASSERT(wmbc_offset(wmbc) == 1);
		ASSERT(wmbc_bytesleft(wmbc) == wmb_size(wmb) - 1);

		/* multiple contexts should move independently of one-another */
		ASSERT(wmbc_offset(wmbc2) == 0);
		ASSERT(wmbc_bytesleft(wmbc2) == wmb_size(wmb));

		/* the contents of the buffer should reflect the change */
		ASSERT(*wmb_contents(wmb) == c);
		ASSERT(*wmbc_finish(wmbc) == c);

		/* the second context is still at the first position, so it should
		 * overwrite the first character */
		char c2 = 'd';
		wmbc_putchar(wmbc2, c2);
		ASSERT(wmbc_offset(wmbc2) == 1);
		ASSERT(*wmb_contents(wmb) == c2);
		ASSERT(*wmbc_finish(wmbc) == c2);
		ASSERT(*wmbc_finish(wmbc2) == c2);

		/* wmbc_finish should terminate the string; we will add a character at
		 * the second index to ensure that it is added */
		char cx = 'x';
		wmbc_putchar(wmbc2, cx);
		ASSERT(wmbc_offset(wmbc2) == 2);
		ASSERT(wmb_contents(wmb)[1] == cx);
		ASSERT(wmbc_finish(wmbc)[1] == '\0');
		ASSERT(wmb_contents(wmb)[1] == '\0');

		/* furthermore, finishing should not adjust the offset, so that we can
		 * continue where we left off */
		ASSERT(wmbc_offset(wmbc) == 1);
		wmbc_putchar(wmbc, cx);
		ASSERT(wmb_contents(wmb)[1] == cx);

		wmbc_free(wmbc2);
		wmbc_free(wmbc);
		wmb_free(wmb);
	}

	/* scenerio: write bytes and move around the buffer */
	{
		WinMsgBuf *wmb = wmb_create();
		WinMsgBufContext *wmbc = wmbc_create(wmb);

		/* initialize buffer to contain the string "foo" */
		wmbc_putchar(wmbc, 'f');
		wmbc_putchar(wmbc, 'o');
		wmbc_putchar(wmbc, 'o');
		wmbc_putchar(wmbc, '\0');

		/* before continuing, just make sure we are where we expect to be (test
		 * sanity check) */
		ASSERT(wmbc_offset(wmbc) == 4);

		/* rewind to the beginning */
		wmbc_rewind(wmbc);
		ASSERT(wmbc_offset(wmbc) == 0);

		/* we should be able to now affect the first character */
		char c = 'm';
		ASSERT(*wmb_contents(wmb) == 'f');  /* sanity check */
		wmbc_putchar(wmbc, c);
		ASSERT(*wmb_contents(wmb) == c);

		/* fast-forward to the null byte (which should bring our pointer to a
		 * single byte before our position after having written "foo") */
		ASSERT(wmbc_offset(wmbc) == 1);  /* sanity check */
		wmbc_fastfw0(wmbc);
		ASSERT(wmbc_offset(wmbc) == 3);

		/* that should allow us to continue where we left off, similar to how
		 * wmbc_finish allows us to continue */
		ASSERT(wmb_contents(wmb)[3] != c);  /* test case sanity check */
		wmbc_putchar(wmbc, c);
		ASSERT(wmb_contents(wmb)[3] == c);

		/* we should also be able to fast-forward to the very end of the buffer
		 * (that is, the next write would trigger an expansion); this is of
		 * limited use externally */
		wmbc_fastfw_end(wmbc);
		ASSERT(wmbc_offset(wmbc) == wmb_size(wmb));
		ASSERT(wmbc_bytesleft(wmbc) == 0);

		wmbc_free(wmbc);
		wmb_free(wmb);
	}

	/* scenerio: write past end of buffer to trigger buffer expansion rather
	 * than overflow */
	{
		WinMsgBuf *wmb = wmb_create();
		WinMsgBufContext *wmbc = wmbc_create(wmb);
		size_t szstart = wmb_size(wmb);

		/* fast-forward to end of buffer and write a character, which shall
		 * trigger expansion */
		char c = 'c';
		wmbc_fastfw_end(wmbc);
		wmbc_putchar(wmbc, c);
		ASSERT(wmb_size(wmb) > szstart);
		ASSERT(wmb_contents(wmb)[wmbc_offset(wmbc) - 1] == c);

		/* if expansion fails, then the character will not be written and shall
		 * be silently ignored; this has the effect of truncating the string
		 * (and should only ever happen in terrible conditions) */
		size_t sznew = wmb_size(wmb);
		char cfail = wmb_contents(wmb)[sznew - 1] + 1;
		wmbc_fastfw_end(wmbc);
		ASSERT(wmbc_offset(wmbc) > szstart);      /* sanity check */
		FAILLOC_VOID(wmbc_putchar(wmbc, cfail));  /* fail expansion */
		ASSERT_GCC(wmb_size(wmb) == sznew);
		/* should not have written char at last position */
		ASSERT_GCC(wmb_contents(wmb)[sznew - 1] != cfail);
		ASSERT_GCC(wmbc_offset(wmbc) == sznew);

		wmbc_free(wmbc);
		wmb_free(wmb);
	}

	/* scenerio: copy strings into buffer, also attempting to expand buffer */
	{
		WinMsgBuf *wmb = wmb_create();
		WinMsgBufContext *wmbc = wmbc_create(wmb);

		/* attempt complete string copy (just like strcpy, a pointer is returned
		 * to the first copied character in the destination) */
		const char str[] = "test string";
		ASSERT(*wmbc_strcpy(wmbc, str) == str[0]);
		ASSERT(wmbc_offset(wmbc) == strlen(str));
		ASSERT(STREQ(wmbc_finish(wmbc), str));

		/* how about a partial string copy (which does not contain a null byte)? */
		size_t n = 4;
		wmbc_rewind(wmbc);
		ASSERT(*wmbc_strncpy(wmbc, str, n) == str[0]);
		ASSERT(wmbc_offset(wmbc) == n);
		ASSERT(STREQ(wmbc_finish(wmbc), "test"));  /* finish adds null byte */

		/* if we cannot fit any more contents, then the buffer should be
		 * expanded in both instances (look at that---a safe strcpy) */
		size_t szfirst = wmb_size(wmb);
		wmbc_fastfw_end(wmbc);
		/* first try strcpy */
		wmbc_strcpy(wmbc, str);
		size_t szsecond = wmb_size(wmb);
		ASSERT(szsecond > szfirst);
		ASSERT(wmbc_offset(wmbc) == szfirst + strlen(str));
		/* then strncpy */
		wmbc_fastfw_end(wmbc);
		wmbc_strncpy(wmbc, str, n);
		ASSERT(wmb_size(wmb) > szsecond);
		ASSERT(wmbc_offset(wmbc) == szsecond + n);

		/* if the buffer cannot be expanded, nothing shall be copied (similar to
		 * putchar); importantly, we cannot return a pointer to the buffer
		 * position that would represent the first character copied, because it
		 * is outside the bounds of the buffer */
		size_t szmax = wmb_size(wmb);
		char last = wmb_contents(wmb)[szmax - 1];
		wmbc_fastfw_end(wmbc);
		/* strcpy */
		ASSERT(FAILLOC(wmbc_strcpy(wmbc, str)) == NULL);
		ASSERT_GCC(wmb_size(wmb) == szmax);
		ASSERT_GCC(wmb_contents(wmb)[szmax - 1] == last);
		/* strncpy */
		ASSERT(FAILLOC(wmbc_strncpy(wmbc, str, n)) == NULL);
		ASSERT_GCC(wmb_size(wmb) == szmax);
		ASSERT_GCC(wmb_contents(wmb)[szmax - 1] == last);

		wmbc_free(wmbc);
		wmb_free(wmb);
	}

	/* scenerio: formatted strings into buffer, also triggering expansion */
	{
		WinMsgBuf *wmb = wmb_create();
		WinMsgBufContext *wmbc = wmbc_create(wmb);

		/* we will not be testing whether sprintf-style formatting works (since
		 * that would be testing vsnprintf), but make sure that it actually does
		 * something */
		char *expect = "test: foo";
		ASSERT(wmbc_printf(wmbc, "test: %s", "foo") == (int)strlen(expect));
		ASSERT(STREQ(wmbc_finish(wmbc), expect));
		ASSERT(wmbc_offset(wmbc) == strlen(expect));

		/* even though sprintf produces a null-terminated string, we do not make
		 * use of the null byte, so we should be able to treat multiple calls as
		 * concatenation */
		char *add = "bar";
		size_t szadd = strlen(add);
		char *expect2 = malloc(strlen(expect) + szadd + 1);
		strcpy(expect2, expect);
		strcat(expect2, add);
		ASSERT(wmbc_printf(wmbc, "%s", add) == (int)szadd);
		ASSERT(STREQ(wmbc_finish(wmbc), expect2));
		ASSERT(wmbc_offset(wmbc) == strlen(expect2));

		/* should trigger expansion if we do not have enough room */
		size_t szfirst = wmb_size(wmb);
		wmbc_fastfw_end(wmbc);
		ASSERT(wmbc_offset(wmbc) == szfirst);  /* sanity check */
		ASSERT(wmbc_printf(wmbc, "%s", add) == (int)szadd);
		ASSERT(wmb_size(wmb) > szfirst);
		ASSERT(STREQ(&wmbc_finish(wmbc)[szfirst], add));
		ASSERT(wmbc_offset(wmbc) == szfirst + szadd);

		/* if the buffer cannot be expanded, then we should write only the
		 * number of bytes that could fit */
		size_t szsecond = wmb_size(wmb);
		wmbc_fastfw_end(wmbc);
		wmbc->p--;  /* :x */
		ASSERT_GCC(FAILLOC(wmbc_printf(wmbc, "%s", add)) == 1);
		ASSERT_GCC(wmbc_offset(wmbc) == wmb_size(wmb));
		ASSERT_GCC(wmb_size(wmb) == szsecond);  /* sanity check */

		wmbc_free(wmbc);
		wmb_free(wmb);
		free(expect2);
	}

	/* scenerio: merging the contents of two separate buffers, also
	 * triggering expansion */
	{
		WinMsgBuf *wmb1 = wmb_create();
		WinMsgBuf *wmb2 = wmb_create();
		WinMsgBufContext *wmbc1 = wmbc_create(wmb1);
		WinMsgBufContext *wmbc2 = wmbc_create(wmb2);

		/* populate both buffers; by this point, we know that everything is
		 * working properly, so no need to assert on these */
		char *str2 = "bar";
		wmbc_strcpy(wmbc1, "foo");
		wmbc_strcpy(wmbc2, str2);

		/* if we merge the second into the first, we'd expect the equivalent
		 * of string concatenation (note that the buffer is expected to be
		 * null-terminated!) */
		const char *expected = "foobar";
		/* technically we don't need to finish here because we wrote a null
		 * byte, but this is depending on implementation details; for
		 * safety, we should always ensure we have a null byte */
		wmbc_finish(wmbc2);
		ASSERT(*wmbc_mergewmb(wmbc1, wmb2) == *str2);
		ASSERT(STREQ(wmbc_finish(wmbc1), expected));

		/* we futher expect to have advanced the pointer by the length of
		 * the merged buffer, sans the terminating null byte */
		ASSERT(wmbc_offset(wmbc1) == strlen(expected));
		/* TODO: rendition state */

		/* if we now attempt to merge past the end of the buffer, we would
		 * expect that expansion will take place */
		size_t szfirst = wmb_size(wmb1);
		wmbc_fastfw_end(wmbc1);
		ASSERT(wmbc_offset(wmbc1) == szfirst);  /* sanity check */
		/* this line did actually catch a bug due to realloc */
		ASSERT(*wmbc_mergewmb(wmbc1, wmb2) == *str2);
		ASSERT(wmb_size(wmb1) > szfirst);
		ASSERT(STREQ(&wmbc_finish(wmbc1)[szfirst], str2));

		/* if the buffer cannot be expanded, then we should simply not copy
		 * the buffer contents */
		size_t szsecond = wmb_size(wmb1);
		wmbc_fastfw_end(wmbc1);
		ASSERT(wmbc_offset(wmbc1) == szsecond);  /* sanity check */
		ASSERT_GCC(FAILLOC(wmbc_mergewmb(wmbc1, wmb2) == NULL));
		ASSERT_GCC(wmb_size(wmb1) == szsecond);

		wmbc_free(wmbc2);
		wmbc_free(wmbc1);
		wmb_free(wmb2);
		wmb_free(wmb1);
	}

	/* context creation issues */
	{
		WinMsgBuf *wmb = wmb_create();

		/* wmbc_create() should return NULL on allocation failure */
		ASSERT_GCC(FAILLOC(wmbc_create(wmb)) == NULL);

		/* it should also return NULL if no buffer is provided (this could
		 * happen on an unchecked (*gasp*) wmb_create() failure */
		ASSERT(wmbc_create(NULL) == NULL);

		wmb_free(wmb);
	}

	return 0;
}
