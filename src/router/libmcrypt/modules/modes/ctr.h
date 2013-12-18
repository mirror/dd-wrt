#ifndef USE_MODULES
int _ctr_init_mcrypt( int td, void* buf, void *key, int lenofkey, void *IV);
int _mcrypt_ctr(int td, void* buf,void *plaintext, int len);
int _mdecrypt_ctr(int td, void* buf, void *plaintext, int len);
int _ctr_is_block_mode();
int _ctr_has_iv();
int _ctr_is_block_algorithm_mode();
char *_mcrypt_ctr_get_modes_name();
int _mcrypt_ctr_mode_get_size ();
int _mcrypt_ctr_get_iv_size(int td);
word32 _mcrypt_ctr_mode_version();
#endif
