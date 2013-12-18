#ifndef USE_MODULES
int _cbc_init_mcrypt( int td, void* buf, void *key, int lenofkey, void *IV);
int _mcrypt_cbc(int td, void* buf,void *plaintext, int len);
int _mdecrypt_cbc(int td, void* buf, void *plaintext, int len);
int _cbc_is_block_mode();
int _cbc_has_iv();
int _cbc_is_block_algorithm_mode();
char *_mcrypt_cbc_get_modes_name();
int _mcrypt_cbc_mode_get_size ();
int _mcrypt_cbc_get_iv_size(int td);
word32 _mcrypt_cbc_mode_version();
#endif
