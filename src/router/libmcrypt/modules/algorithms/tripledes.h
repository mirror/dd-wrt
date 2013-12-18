typedef struct triple_des_key {
	char kn[3][16][8];
	word32 sp[3][8][64];

	char iperm[16][16][8];
	char fperm[16][16][8];

} TRIPLEDES_KEY;

