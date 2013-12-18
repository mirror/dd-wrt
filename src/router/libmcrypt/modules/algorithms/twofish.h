#define Q_TABLES
#define M_TABLE
#define MK_TABLE
#define ONE_STEP

typedef struct twofish_instance {
	word32  k_len;
	word32  l_key[40];
	word32  s_key[4];
#ifdef  Q_TABLES
	word32 qt_gen;
	byte q_tab[2][256];
#endif
#ifdef  M_TABLE
	word32 mt_gen;
	word32 m_tab[4][256];
#endif
#ifdef  ONE_STEP
	word32 mk_tab[4][256];
#else
	byte sb[4][256];
#endif
} TWI;

