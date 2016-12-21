#ifndef _CVIMG_H

#define	_CVIMG_H

#define __PACK__				__attribute__ ((packed))
#define SIGNATURE_LEN			4

#ifndef _LITTLE_ENDIAN_
#define	DWORD_SWAP(v)	((((v&0xff)<<24)&0xff000000) | \
						((((v>>8)&0xff)<<16)&0xff0000) | \
						((((v>>16)&0xff)<<8)&0xff00) | \
						(((v>>24)&0xff)&0xff) )
#define WORD_SWAP(v)	((unsigned short)(((v>>8)&0xff) | ((v<<8)&0xff00)))
#else
#define DWORD_SWAP(v) (v)
#define WORD_SWAP(v) (v)
#endif

#define	FW_HEADER			((char *)"CSYS")
#define _WEB_HEADER_ "RN67"

typedef struct img_header {
	unsigned char signature[SIGNATURE_LEN];
	unsigned int startAddr __PACK__;
	unsigned int burnAddr __PACK__;
	unsigned char modTag[SIGNATURE_LEN];
	unsigned int len __PACK__;
} __PACK__ IMG_HEADER_T, *IMG_HEADER_Tp;

#endif
