typedef struct wake_key {
	word32 t[257];
	word32 r[4];
	int counter;
	word32 tmp; /* used as r1 or r2 */
	int started;
	word32 iv[8];
	int ivsize;
} WAKE_KEY;

