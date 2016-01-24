mpz_t z;
char *ret = NULL;
/* ... */
gmp_asprintf(&ret, "%.*s is an bignum %Zd.\n", "mpz_t", z);
