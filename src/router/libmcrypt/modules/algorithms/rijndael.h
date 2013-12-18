
typedef struct rijndael_instance {
	int Nk,Nb,Nr;
	byte fi[24],ri[24];
	word32 fkey[120];
	word32 rkey[120];
} RI;

/* void _mcrypt_rijndael_128_set_key(RI* rinst, int nb,int nk,byte *key); */
  /* blocksize=32*nb bits. Key=32*nk bits */
  /* currently nb,bk = 4, 6 or 8          */
  /* key comes as 4*Nk bytes              */
  /* Key Scheduler. Create expanded encryption key */
