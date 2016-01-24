/* broken example ... fairly common mistake */
char *s1;
int X;
...
sprintf(s1, "%s:%d", s1, X); /* append a number */
