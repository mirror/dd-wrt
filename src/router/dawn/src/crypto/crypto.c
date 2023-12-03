// based on:
// https://github.com/vedantk/gcrypt-example/blob/master/gcry.cc

#include <gcrypt.h>

#include "utils.h"
#include "memory_utils.h"
#include "crypto.h"

#define GCRY_CIPHER GCRY_CIPHER_AES128   // Pick the cipher here
#define GCRY_C_MODE GCRY_CIPHER_MODE_ECB // Pick the cipher mode here

gcry_error_t gcry_error_handle;
gcry_cipher_hd_t gcry_cipher_hd;

void gcrypt_init() {
    if (!gcry_check_version(GCRYPT_VERSION)) {
        dawnlog_error("gcrypt: library version mismatch");
    }
    gcry_error_t err = 0;
    err = gcry_control(GCRYCTL_SUSPEND_SECMEM_WARN);
    err |= gcry_control(GCRYCTL_INIT_SECMEM, 16384, 0);
    err |= gcry_control(GCRYCTL_RESUME_SECMEM_WARN);
    err |= gcry_control(GCRYCTL_INITIALIZATION_FINISHED, 0);

    if (err) {
        dawnlog_error("gcrypt: failed initialization");
    }
}

void gcrypt_set_key_and_iv(const char *key, const char *iv) {
    size_t keylen = gcry_cipher_get_algo_keylen(GCRY_CIPHER);
    size_t blklen = gcry_cipher_get_algo_blklen(GCRY_CIPHER);

    gcry_error_handle = gcry_cipher_open(
            &gcry_cipher_hd, // gcry_cipher_hd_t *
            GCRY_CIPHER,   // int
            GCRY_C_MODE,   // int
            0);
    if (gcry_error_handle) {
        dawnlog_error("gcry_cipher_open failed:  %s/%s\n",
                gcry_strsource(gcry_error_handle),
                gcry_strerror(gcry_error_handle));
        return;
    }

    gcry_error_handle = gcry_cipher_setkey(gcry_cipher_hd, key, keylen);
    if (gcry_error_handle) {
        dawnlog_error("gcry_cipher_setkey failed:  %s/%s\n",
                gcry_strsource(gcry_error_handle),
                gcry_strerror(gcry_error_handle));
        return;
    }

    gcry_error_handle = gcry_cipher_setiv(gcry_cipher_hd, iv, blklen);
    if (gcry_error_handle) {
        dawnlog_error("gcry_cipher_setiv failed:  %s/%s\n",
                gcry_strsource(gcry_error_handle),
                gcry_strerror(gcry_error_handle));
        return;
    }
}

// free out buffer after using!
char *gcrypt_encrypt_msg(char *msg, size_t msg_length, int *out_length) {
    if (0U != (msg_length & 0xfU))
        msg_length += 0x10U - (msg_length & 0xfU);

    char *out = dawn_malloc(msg_length);
    if (!out){
        dawnlog_error("gcry_cipher_encrypt error: not enough memory\n");
        return NULL;
    }
    gcry_error_handle = gcry_cipher_encrypt(gcry_cipher_hd, out, msg_length, msg, msg_length);
    if (gcry_error_handle) {
        dawnlog_error("gcry_cipher_encrypt failed:  %s/%s\n",
                gcry_strsource(gcry_error_handle),
                gcry_strerror(gcry_error_handle));
        return NULL;
    }
    *out_length = msg_length;
    return out;
}

// free out buffer after using!
char *gcrypt_decrypt_msg(char *msg, size_t msg_length) {
    if (0U != (msg_length & 0xfU))
        msg_length += 0x10U - (msg_length & 0xfU);

    char *out_buffer = dawn_malloc(msg_length);
    if (!out_buffer){
        dawnlog_error("gcry_cipher_decrypt error: not enough memory\n");
        return NULL;
    }
    gcry_error_handle = gcry_cipher_decrypt(gcry_cipher_hd, out_buffer, msg_length, msg, msg_length);
    if (gcry_error_handle) {
        dawnlog_error("gcry_cipher_decrypt failed:  %s/%s\n",
                gcry_strsource(gcry_error_handle),
                gcry_strerror(gcry_error_handle));
        dawn_free(out_buffer);
        out_buffer = NULL;
        return NULL;
    }
    char *out = dawn_malloc(strlen(out_buffer) + 1);
    if (!out){
        dawn_free(out_buffer);
        out_buffer = NULL;
        dawnlog_error("gcry_cipher_decrypt error: not enough memory\n");
        return NULL;
    }
    strcpy(out, out_buffer);
    dawn_free(out_buffer);
    out_buffer = NULL;
    return out;
}

