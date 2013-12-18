/* First example: Encrypts stdin to stdout using TWOFISH with 128 bit key and CFB */

#include <mcrypt.h>
#include <stdio.h>
#include <stdlib.h>
/* #include <mhash.h> */

main() {

  MCRYPT td;
  int i;
  char *key;
  char password[20];
  char block_buffer;
  char *IV;
  int keysize=19; /* 128 bits */

  key=calloc(1, keysize);
  strcpy(password, "A_large_key");

/* Generate the key using the password */
/*  mhash_keygen( KEYGEN_MCRYPT, MHASH_MD5, key, keysize, NULL, 0, password, strlen(password));
 */
  memmove( key, password, strlen(password));

  td = mcrypt_module_open("twofish", NULL, "cfb", NULL);
  if (td==MCRYPT_FAILED) {
     return 1;
  }
  IV = malloc(mcrypt_enc_get_iv_size(td));

/* Put random data in IV. Note these are not real random data, 
 * consider using /dev/random or /dev/urandom.
 */

  /*  srand(time(0)); */
  for (i=0; i< mcrypt_enc_get_iv_size( td); i++) {
    IV[i]=rand();
  }

  i=mcrypt_generic_init( td, key, keysize, IV);
  if (i<0) {
     mcrypt_perror(i);
     return 1;
  }

  /* Encryption in CFB is performed in bytes */
  while ( fread (&block_buffer, 1, 1, stdin) == 1 ) {
      mcrypt_generic (td, &block_buffer, 1);

/* Comment above and uncomment this to decrypt */
/*    mdecrypt_generic (td, &block_buffer, 1);  */

      fwrite ( &block_buffer, 1, 1, stdout);
  }
  mcrypt_generic_deinit(td);

  mcrypt_module_close(td);

  return 0;

}

#if 0
/* Second Example: encrypts using CBC and SAFER+ with 192 bits key */

#include <mcrypt.h>
#include <stdio.h>
#include <stdlib.h>

main() {

  MCRYPT td;
  int i;
  char *key; /* created using mcrypt_gen_key */
  char *block_buffer;
  char *IV;
  int blocksize;
  int keysize = 24; /* 192 bits == 24 bytes */


  key = calloc(1, keysize);
  strcpy(key, "A_large_and_random_key"); 

  td = mcrypt_module_open("saferplus", NULL, "cbc", NULL);

  blocksize = mcrypt_enc_get_block_size(td);
  block_buffer = malloc(blocksize);
/* but unfortunately this does not fill all the key so the rest bytes are
 * padded with zeros. Try to use large keys or convert them with mcrypt_gen_key().
 */

  IV=malloc(mcrypt_enc_get_iv_size(td));

/* Put random data in IV. Note these are not real random data, 
 * consider using /dev/random or /dev/urandom.
 */

/* srand(time(0)); */
  for (i=0; i < mcrypt_enc_get_iv_size(td); i++) {
    IV[i]=rand();
  }

  mcrypt_generic_init ( td key, keysize, IV);

  /* Encryption in CBC is performed in blocks */
  while ( fread (block_buffer, 1, blocksize, stdin) == blocksize ) {
      mcrypt_generic (td, block_buffer, blocksize);
/*      mdecrypt_generic (td, block_buffer, blocksize); */
      fwrite ( block_buffer, 1, blocksize, stdout);
  }
  mcrypt_generic_end (td);

  return 0;

}

#endif
