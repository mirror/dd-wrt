/* broken example ... common mistake */
char *s1;
...
strcpy(s1, s1 + 1); /* chop the first character from the C string */
