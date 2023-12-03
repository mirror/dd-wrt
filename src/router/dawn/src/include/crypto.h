#ifndef DAWN_CRYPTO_H
#define DAWN_CRYPTO_H

#include <stddef.h>

/**
 * Initialize gcrypt.
 * Has to be called before using the other functions!
 */
void gcrypt_init();

/**
 * Set the Key and the iv.
 * @param key
 * @param iv
 */
void gcrypt_set_key_and_iv(const char *key, const char *iv);

/**
 * Function that encrypts the message.
 * Free the string after using it!
 * @param msg
 * @param msg_length
 * @param out_length
 * @return the encrypted string.
 */
char *gcrypt_encrypt_msg(char *msg, size_t msg_length, int *out_length);

/**
 * FUnction that decrypts a message.
 * Free the string after using it!
 * @param msg
 * @param msg_length
 * @return the decrypted string.
 */
char *gcrypt_decrypt_msg(char *msg, size_t msg_length);

#endif //DAWN_CRYPTO_H
