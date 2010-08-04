#ifndef INFLATE_MM_H
#define INFLATE_MM_H

#ifdef STATIC
/* Code active when included from pre-boot environment: */
#define INIT
#else
/* Compile for initramfs/initrd code only */
#define INIT __init
#endif

#endif
