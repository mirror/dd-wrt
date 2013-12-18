#ifndef USE_MODULES
int _ofb_init_mcrypt( int td, void* buf, void *key, int lenofkey, void *IV);
int _mcrypt_ofb(int td, void* buf,void *plaintext, int len);
int _mdecrypt_ofb(int td, void* buf, void *plaintext, int len);
int _ofb_is_block_mode();
int _ofb_has_iv();
int _ofb_is_block_algorithm_mode();
char *_mcrypt_ofb_get_modes_name();
int _mcrypt_ofb_mode_get_size ();
int _mcrypt_ofb_get_iv_size(int td);
word32 _mcrypt_ofb_mode_version();
#endif
