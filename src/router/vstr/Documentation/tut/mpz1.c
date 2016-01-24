int d;
char *ret = NULL;
/* ... */
gmp_asprintf(&ret, "%s is an num %'d.\n", "int", d);
