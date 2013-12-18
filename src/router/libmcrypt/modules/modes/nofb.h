#ifndef USE_MODULES
int _nofb_init_mcrypt( int td, void* buf, void *key, int lenofkey, void *IV);
int _mcrypt_nofb(int td, void* buf,void *plaintext, int len);
int _mdecrypt_nofb(int td, void* buf, void *plaintext, int len);
int _nofb_is_block_mode();
int _nofb_has_iv();
int _nofb_is_block_algorithm_mode();
char *_mcrypt_nofb_get_modes_name();
int _mcrypt_nofb_mode_get_size ();
int _mcrypt_nofb_get_iv_size(int td);
word32 _mcrypt_nofb_mode_version();
#endif
