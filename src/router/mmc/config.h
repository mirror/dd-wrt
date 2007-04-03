#define LOG_LEVEL 3
#define USE_CMD18
#define USE_CMD25



#ifdef ONE
#define SD_DI 0x04
#define SD_DO 0x10
#define SD_CLK 0x08
#define SD_CS 0x80
#elif TWO
#define SD_DI 0x20
#define SD_DO 0x10
#define SD_CLK 0x08
#define SD_CS 0x80
#elif THREE
#define SD_DI 0x40
#define SD_DO 0x20
#define SD_CLK 0x08
#define SD_CS 0x80
#elif FOUR
#define SD_DI 0x20
#define SD_DO 0x40
#define SD_CLK 0x08
#define SD_CS 0x80
#endif

#if SD_DO == 0x10
	#define SHIFT_DO 3
#else
#if SD_DO == 0x40
	#define SHIFT_DO 1

#else
#if SD_DO == 0x20
	#define SHIFT_DO 2
#endif
#endif
#endif
#if SD_DI == 0x04
	#define SHIFT_DI 5
#else
#if SD_DI == 0x20
	#define SHIFT_DI 2

#else
#if SD_DI == 0x40
	#define SHIFT_DI 1
#endif
#endif
#endif
