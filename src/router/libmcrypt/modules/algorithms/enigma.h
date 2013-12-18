#define E_ECHO 010
#define ROTORSZ 256
#define MASK 0377

typedef struct crypt_key {
	char t1[ROTORSZ];
	char t2[ROTORSZ];
	char t3[ROTORSZ];
	char deck[ROTORSZ];
	char cbuf[13];
	int n1, n2, nr1, nr2;
} CRYPT_KEY;

