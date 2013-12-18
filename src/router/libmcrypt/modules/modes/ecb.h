#ifndef USE_MODULES
int _ecb_init_mcrypt( int td, void* buf, void *key, int lenofkey, void *IV);
int _mcrypt_ecb(int td, void* buf,void *plaintext, int len);
int _mdecrypt_ecb(int td, void* buf, void *plaintext, int len);
int _ecb_is_block_mode();
int _ecb_has_iv();
int _ecb_is_block_algorithm_mode();
char *_mcrypt_ecb_get_modes_name();
int _mcrypt_ecb_mode_get_size ();
int _mcrypt_ecb_get_iv_size(int td);
word32 _mcrypt_ecb_mode_version();
#endif
