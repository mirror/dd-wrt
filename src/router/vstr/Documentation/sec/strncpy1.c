#define SZ 4
char s1[SZ];

strncpy(s1, "abcd",   SZ); /* 1 */
strncpy(s1, "abc",    SZ); /* 2 */
strncpy(s1, "a",      SZ); /* 3 */
