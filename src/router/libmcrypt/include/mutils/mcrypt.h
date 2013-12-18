/*
 * Copyright (C) 1998,1999,2002 Nikos Mavroyanopoulos
 *
 * Encryption/decryption library. This library is free software;
 * you can redistribute it and/or modify it under the terms of the
 * GNU Library General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any
 * later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#define MCRYPT_API_VERSION 20021217

#define LIBMCRYPT_VERSION "2.5.8"

#ifdef __cplusplus
extern "C" {
#endif
/* Definitions
 */
#define MCRYPT_FAILED 0x0

struct CRYPT_STREAM;
typedef struct CRYPT_STREAM *MCRYPT;

/* generic - high level functions. 
 */
	MCRYPT mcrypt_module_open(char *algorithm,
				  char *a_directory, char *mode,
				  char *m_directory);
	int mcrypt_module_close(MCRYPT td);

	/* returns 0 if the library has not been compiled with
	 * dynamic module support.
 	 */
	int mcrypt_module_support_dynamic(void);

/* returns thread descriptor */

	int mcrypt_generic_init(const MCRYPT td, void *key, int lenofkey,
				void *IV);
	int mcrypt_generic_deinit(const MCRYPT td);
	int mcrypt_generic_end(const MCRYPT td);
	int mdecrypt_generic(MCRYPT td, void *plaintext, int len);
	int mcrypt_generic(MCRYPT td, void *plaintext, int len);

/* extra functions */

	int mcrypt_enc_set_state(MCRYPT td, void *st, int size);
	int mcrypt_enc_get_state(MCRYPT td, void *st, int *size); /* only 
	* for block algorithms and certain modes like cbc
	* ncfb etc.
	*/
	int (mcrypt_enc_self_test) (MCRYPT);
	int (mcrypt_enc_get_block_size) (MCRYPT);
	int (mcrypt_enc_get_iv_size) (MCRYPT);
	int (mcrypt_enc_get_key_size) (MCRYPT);

/* If this is a block algorithm returns 1 
 */
	int (mcrypt_enc_is_block_algorithm) (MCRYPT);

/* If the mode operates in blocks returns 1 
 */
	int (mcrypt_enc_is_block_mode) (MCRYPT);

/* If the mode is for block algorithms it returns 1 
 */
	int (mcrypt_enc_is_block_algorithm_mode) (MCRYPT td);
	int mcrypt_enc_mode_has_iv(MCRYPT td);

/* Return a const string containing the name of the algorithm/mode
 */
	char *(mcrypt_enc_get_algorithms_name) (MCRYPT td);
	char *(mcrypt_enc_get_modes_name) (MCRYPT td);

	int *mcrypt_enc_get_supported_key_sizes(MCRYPT td, int *len);


	char **mcrypt_list_algorithms(char *libdir, int *size);
	char **mcrypt_list_modes(char *libdir, int *size);

	/* Frees the memory allocated by the mcrypt_list_xxx() functions. 
	 */
	void mcrypt_free_p(char **p, int size);
	void mcrypt_free(void *ptr);

	/* If mcrypt_xxx functions return an error code, and you supply this
	 * to this function, it will print a human readable message
	 */
	void mcrypt_perror(int err);
	const char* mcrypt_strerror(int err);

	/* Self test for the specified algorithm 
	 */
	int mcrypt_module_self_test(char *algorithm, char *a_directory);

	int mcrypt_module_is_block_algorithm(char *algorithm,
					     char *a_directory);
	int mcrypt_module_is_block_algorithm_mode(char *mode,
						  char *m_directory);
	int mcrypt_module_is_block_mode(char *mode, char *m_directory);

	int mcrypt_module_get_algo_key_size(char *algorithm,
					    char *a_directory);
	int mcrypt_module_get_algo_block_size(char *algorithm,
					      char *a_directory);

	int *mcrypt_module_get_algo_supported_key_sizes(char *algorithm,
							char *a_directory,
							int *len);

	/* Checks the version of the specified module 
	 */
	int mcrypt_module_algorithm_version(char *algorithm,
					    char *a_directory);
	int mcrypt_module_mode_version(char *mode, char *a_directory);


	/* for multithreaded applications: 
	 */
	int mcrypt_mutex_register ( void (*mutex_lock)(void) , 
			void (*mutex_unlock)(void), 
			void (*set_error)(const char*), 
			const char* (*get_error)(void));

	const char *
		mcrypt_check_version( const char *);

	/* These definitions exist in order to ease the access to 
	 * mcrypt_module_init().
	 */

	/* Algorithms */
#define MCRYPT_BLOWFISH		"blowfish"
#define MCRYPT_DES 		"des"
#define MCRYPT_3DES 		"tripledes"
#define MCRYPT_3WAY 		"threeway"
#define MCRYPT_GOST 		"gost"
#define MCRYPT_SAFER_SK64 	"safer-sk64"
#define MCRYPT_SAFER_SK128 	"safer-sk128"
#define MCRYPT_CAST_128 	"cast-128"
#define MCRYPT_XTEA 		"xtea"
#define MCRYPT_RC2	 	"rc2"
#define MCRYPT_TWOFISH 		"twofish"
#define MCRYPT_CAST_256 	"cast-256"
#define MCRYPT_SAFERPLUS 	"saferplus"
#define MCRYPT_LOKI97 		"loki97"
#define MCRYPT_SERPENT 		"serpent"
#define MCRYPT_RIJNDAEL_128 	"rijndael-128"
#define MCRYPT_RIJNDAEL_192 	"rijndael-192"
#define MCRYPT_RIJNDAEL_256 	"rijndael-256"
#define MCRYPT_ENIGMA 		"enigma"
#define MCRYPT_ARCFOUR		"arcfour"
#define MCRYPT_WAKE		"wake"

	/* Modes */
#define MCRYPT_CBC		"cbc"
#define MCRYPT_ECB		"ecb"
#define MCRYPT_CFB		"cfb"
#define MCRYPT_OFB		"ofb"
#define MCRYPT_nOFB		"nofb"
#define MCRYPT_STREAM		"stream"

#ifdef __cplusplus
}
#endif
