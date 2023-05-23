#ifndef _AES_H
#define _AES_H

#include <apfs/types.h>

int aes_unwrap(const u8 *kek, int n, const u8 *cipher, u8 *plain);
int aes_xts_decrypt(const u8 *key1, const u8 *key2, u64 tweak, const u8 *cipher, int len, u8 *plain);

#endif /* _AES_H */
