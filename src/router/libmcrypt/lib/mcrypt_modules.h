#define rotl32(x,n)   (((x) << ((word32)(n))) | ((x) >> (32 - (word32)(n))))
#define rotr32(x,n)   (((x) >> ((word32)(n))) | ((x) << (32 - (word32)(n))))
#define rotl16(x,n)   (((x) << ((word16)(n))) | ((x) >> (16 - (word16)(n))))
#define rotr16(x,n)   (((x) >> ((word16)(n))) | ((x) << (16 - (word16)(n))))

/* Use hardware rotations.. when available */
#ifdef swap32
# define byteswap32(x) swap32(x)
#else
# ifdef swap_32
#  define byteswap32(x) swap_32(x)
# else
#  ifdef bswap_32
#   define byteswap32(x) bswap_32(x)
#  else
#   define byteswap32(x)	((rotl32(x, 8) & 0x00ff00ff) | (rotr32(x, 8) & 0xff00ff00))
#  endif
# endif
#endif

#ifdef swap16
# define byteswap16(x) swap16(x)
#else
# ifdef swap_16
#  define byteswap16(x) swap_16(x)
# else
#  ifdef bswap_16
#   define byteswap16(x) bswap_16(x)
#  else
#   define byteswap16(x)	((rotl16(x, 8) & 0x00ff) | (rotr16(x, 8) & 0xff00))
#  endif
# endif
#endif

inline static
void memxor(unsigned char *o1, unsigned char *o2, int length)
{
	int i;

	for (i = 0; i < length; i++) {
		o1[i] ^= o2[i];
	}
	return;
}


#define Bzero(x, y) memset(x, 0, y)
