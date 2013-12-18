#ifndef USE_MODULES
int _stream_init_mcrypt( int td, void* buf, void *key, int lenofkey, void *IV);
int _mcrypt_stream(int td, void* buf,void *plaintext, int len);
int _mdecrypt_stream(int td, void* buf, void *plaintext, int len);
int _stream_is_block_mode();
int _stream_has_iv();
int _stream_is_block_algorithm_mode();
char *_mcrypt_stream_get_modes_name();
int _mcrypt_stream_mode_get_size ();
int _mcrypt_stream_get_iv_size(int td);
word32 _mcrypt_stream_mode_version();
#endif
