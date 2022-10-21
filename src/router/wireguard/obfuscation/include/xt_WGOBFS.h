#ifndef _XT_WGOBFS_H
#define _XT_WGOBFS_H

#define XT_WGOBFS_MAX_KEY_SIZE 32
#define XT_CHACHA_KEY_SIZE 32
#define XT_MODE_OBFS   0
#define XT_MODE_UNOBFS 1

struct xt_wg_obfs_info {
    unsigned char mode;
    char key[XT_WGOBFS_MAX_KEY_SIZE + 1];
    unsigned char chacha_key[XT_CHACHA_KEY_SIZE];  /* 256 bits chacha key */
};

#endif
