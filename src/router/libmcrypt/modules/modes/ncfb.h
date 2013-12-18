#ifndef USE_MODULES
int _ncfb_init_mcrypt( int td, void* buf, void *key, int lenofkey, void *IV);
int _mcrypt_ncfb(int td, void* buf,void *plaintext, int len);
int _mdecrypt_ncfb(int td, void* buf, void *plaintext, int len);
int _ncfb_is_block_mode();
int _ncfb_has_iv();
int _ncfb_is_block_algorithm_mode();
char *_mcrypt_ncfb_get_modes_name();
int _mcrypt_ncfb_mode_get_size ();
int _mcrypt_ncfb_get_iv_size(int td);
word32 _mcrypt_ncfb_mode_version();
#endif
