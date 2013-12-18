#ifndef USE_MODULES
int _cfb_init_mcrypt( int td, void* buf, void *key, int lenofkey, void *IV);
int _mcrypt_cfb(int td, void* buf,void *plaintext, int len);
int _mdecrypt_cfb(int td, void* buf, void *plaintext, int len);
int _cfb_is_block_mode();
int _cfb_has_iv();
int _cfb_is_block_algorithm_mode();
char *_mcrypt_cfb_get_modes_name();
int _mcrypt_cfb_mode_get_size ();
int _mcrypt_cfb_get_iv_size(int td);
word32 _mcrypt_cfb_mode_version();
#endif
