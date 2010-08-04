#ifndef UNLZMA_MM_H
#define UNLZMA_MM_H

#ifdef STATIC

/* Code active when included from pre-boot environment: */
#define INIT

#elif defined(CONFIG_DECOMPRESS_LZMA_NEEDED)

/* Make it available to non initramfs/initrd code */
#define INIT
#include <linux/module.h>
#else

/* Compile for initramfs/initrd code only */
#define INIT __init
#endif

#endif
