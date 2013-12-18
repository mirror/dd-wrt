/* panama_x.c */

/* $Id: panama.c,v 1.21 2003/01/19 17:48:27 nmav Exp $ */

/* daemen.j@protonworld.com */
/**************************************************************************+
*
*  PANAMA high-performance reference C-code, based on the description in 
*  the paper 'Fast Hashing and Stream Encryption with PANAMA', presented 
*  at the Fast Software Encryption Workshop, Paris, 1998, see "Fast 
*  Software Encryption - 5th International Workshop, FSE'98", edited by 
*  Serge Vaudenay, LNCS-1372, Springer-Verlag, 1998, pp 60-74, also 
*  available on-line at http://standard.pictel.com/ftp/research/security
*
*  Algorithm design by Joan Daemen and Craig Clapp
*
*  panama_x.c  -  Core routines for the Panama stream/hash module, this 
*                 exportable version excludes an encryption routine.
*
*
*  History:
*
*  29-Oct-98  Craig Clapp  Implemention for Dr. Dobbs, Dec. 1998 issue, 
*                          based on earlier performance-benchmark code.
*
*
*  Notes:  This code is supplied for the purposes of evaluating the 
*          performance of the Panama stream/hash module and as a 
*          reference implementation for generating test vectors for 
*          compatibility / interoperability verification.
*
*
+**************************************************************************/

/* modified in order to use the libmcrypt API by Nikos Mavroyanopoulos 
 * All modifications are placed under the license of libmcrypt.
 */

#include <libdefs.h>

#include <mcrypt_modules.h>
#include "panama.h"


#define _mcrypt_set_key panama_LTX__mcrypt_set_key
#define _mcrypt_encrypt panama_LTX__mcrypt_encrypt
#define _mcrypt_decrypt panama_LTX__mcrypt_decrypt
#define _mcrypt_get_size panama_LTX__mcrypt_get_size
#define _mcrypt_get_block_size panama_LTX__mcrypt_get_block_size
#define _is_block_algorithm panama_LTX__is_block_algorithm
#define _mcrypt_get_key_size panama_LTX__mcrypt_get_key_size
#define _mcrypt_get_algo_iv_size panama_LTX__mcrypt_get_algo_iv_size
#define _mcrypt_get_supported_key_sizes panama_LTX__mcrypt_get_supported_key_sizes
#define _mcrypt_get_algorithms_name panama_LTX__mcrypt_get_algorithms_name
#define _mcrypt_self_test panama_LTX__mcrypt_self_test
#define _mcrypt_algorithm_version panama_LTX__mcrypt_algorithm_version

/**************************************************************************+
*                         Panama internal routines                         *
+**************************************************************************/

/* tau, rotate  word 'a' to the left by rol_bits bit positions */

#define tau(a, rol_bits)  ROTL32(a, rol_bits)

/**************************************************************************/

/* move state between memory and local registers */

#define READ_STATE_i(i)   state_##i = state->word[i]
#define WRITE_STATE_i(i)  state->word[i] = state_##i


#define READ_STATE    \
                      \
    READ_STATE_i(0);  \
    READ_STATE_i(1);  \
    READ_STATE_i(2);  \
    READ_STATE_i(3);  \
    READ_STATE_i(4);  \
    READ_STATE_i(5);  \
    READ_STATE_i(6);  \
    READ_STATE_i(7);  \
    READ_STATE_i(8);  \
    READ_STATE_i(9);  \
    READ_STATE_i(10); \
    READ_STATE_i(11); \
    READ_STATE_i(12); \
    READ_STATE_i(13); \
    READ_STATE_i(14); \
    READ_STATE_i(15); \
    READ_STATE_i(16)


#define WRITE_STATE    \
                       \
    WRITE_STATE_i(0);  \
    WRITE_STATE_i(1);  \
    WRITE_STATE_i(2);  \
    WRITE_STATE_i(3);  \
    WRITE_STATE_i(4);  \
    WRITE_STATE_i(5);  \
    WRITE_STATE_i(6);  \
    WRITE_STATE_i(7);  \
    WRITE_STATE_i(8);  \
    WRITE_STATE_i(9);  \
    WRITE_STATE_i(10); \
    WRITE_STATE_i(11); \
    WRITE_STATE_i(12); \
    WRITE_STATE_i(13); \
    WRITE_STATE_i(14); \
    WRITE_STATE_i(15); \
    WRITE_STATE_i(16)

/**************************************************************************/

/* gamma, shift-invariant transformation a[i] XOR (a[i+1] OR NOT a[i+2]) */

#define gamma_in_(i)   state_##i
#define gamma_out_(i)  gamma_##i

#define GAMMA_i(i, i_plus_1, i_plus_2)  \
                                        \
    gamma_out_(i) = gamma_in_(i) ^ (gamma_in_(i_plus_1) | ~gamma_in_(i_plus_2))


#define GAMMA            \
                         \
    GAMMA_i( 0,  1,  2); \
    GAMMA_i( 1,  2,  3); \
    GAMMA_i( 2,  3,  4); \
    GAMMA_i( 3,  4,  5); \
    GAMMA_i( 4,  5,  6); \
    GAMMA_i( 5,  6,  7); \
    GAMMA_i( 6,  7,  8); \
    GAMMA_i( 7,  8,  9); \
    GAMMA_i( 8,  9, 10); \
    GAMMA_i( 9, 10, 11); \
    GAMMA_i(10, 11, 12); \
    GAMMA_i(11, 12, 13); \
    GAMMA_i(12, 13, 14); \
    GAMMA_i(13, 14, 15); \
    GAMMA_i(14, 15, 16); \
    GAMMA_i(15, 16,  0); \
    GAMMA_i(16,  0,  1)

/**************************************************************************/

/* pi, permute and cyclicly rotate the state words */

#define pi_in_(i)   gamma_##i
#define pi_out_(i)  pi_##i

#define PI_i(i, j, k)  pi_out_(i) = tau(pi_in_(j), k)


#define PI                  \
                            \
    pi_out_(0) = pi_in_(0); \
    PI_i( 1,  7,  1);       \
    PI_i( 2, 14,  3);       \
    PI_i( 3,  4,  6);       \
    PI_i( 4, 11, 10);       \
    PI_i( 5,  1, 15);       \
    PI_i( 6,  8, 21);       \
    PI_i( 7, 15, 28);       \
    PI_i( 8,  5,  4);       \
    PI_i( 9, 12, 13);       \
    PI_i(10,  2, 23);       \
    PI_i(11,  9,  2);       \
    PI_i(12, 16, 14);       \
    PI_i(13,  6, 27);       \
    PI_i(14, 13,  9);       \
    PI_i(15,  3, 24);       \
    PI_i(16, 10,  8)

/**************************************************************************/

/* theta, shift-invariant transformation a[i] XOR a[i+1] XOR a[i+4] */

#define theta_in_(i)   pi_##i
#define theta_out_(i)  theta_##i

#define THETA_i(i, i_plus_1, i_plus_4)  \
                                        \
    theta_out_(i) = theta_in_(i) ^ theta_in_(i_plus_1) ^ theta_in_(i_plus_4)


#define THETA            \
                         \
    THETA_i( 0,  1,  4); \
    THETA_i( 1,  2,  5); \
    THETA_i( 2,  3,  6); \
    THETA_i( 3,  4,  7); \
    THETA_i( 4,  5,  8); \
    THETA_i( 5,  6,  9); \
    THETA_i( 6,  7, 10); \
    THETA_i( 7,  8, 11); \
    THETA_i( 8,  9, 12); \
    THETA_i( 9, 10, 13); \
    THETA_i(10, 11, 14); \
    THETA_i(11, 12, 15); \
    THETA_i(12, 13, 16); \
    THETA_i(13, 14,  0); \
    THETA_i(14, 15,  1); \
    THETA_i(15, 16,  2); \
    THETA_i(16,  0,  3)

/**************************************************************************/

/* sigma, merge two buffer stages with current state */

#define sigma_in_(i)   theta_##i
#define sigma_out_(i)  state_##i

#define SIGMA_L_i(i)  sigma_out_(i) = sigma_in_(i) ^ L->word[i-1]
#define SIGMA_B_i(i)  sigma_out_(i) = sigma_in_(i) ^ b->word[i-9]


#define SIGMA      \
                   \
    sigma_out_(0) = sigma_in_(0) ^ 0x00000001L; \
                   \
    SIGMA_L_i(1);  \
    SIGMA_L_i(2);  \
    SIGMA_L_i(3);  \
    SIGMA_L_i(4);  \
    SIGMA_L_i(5);  \
    SIGMA_L_i(6);  \
    SIGMA_L_i(7);  \
    SIGMA_L_i(8);  \
                   \
    SIGMA_B_i(9);  \
    SIGMA_B_i(10); \
    SIGMA_B_i(11); \
    SIGMA_B_i(12); \
    SIGMA_B_i(13); \
    SIGMA_B_i(14); \
    SIGMA_B_i(15); \
    SIGMA_B_i(16)

/**************************************************************************/

/* lambda, update the 256-bit wide by 32-stage LFSR buffer */

#define LAMBDA_25_i(i)  \
  ptap_25->word[i] = ptap_25->word[i] ^ ptap_0->word[(i+2) & (PAN_STAGE_SIZE-1)]

#define LAMBDA_0_i(i, source)  ptap_0->word[i] = source ^ ptap_0->word[i]


#define LAMBDA_25_UPDATE \
                         \
    LAMBDA_25_i(0);      \
    LAMBDA_25_i(1);      \
    LAMBDA_25_i(2);      \
    LAMBDA_25_i(3);      \
    LAMBDA_25_i(4);      \
    LAMBDA_25_i(5);      \
    LAMBDA_25_i(6);      \
    LAMBDA_25_i(7)

#define LAMBDA_0_PULL       \
                            \
    LAMBDA_0_i(0, state_1); \
    LAMBDA_0_i(1, state_2); \
    LAMBDA_0_i(2, state_3); \
    LAMBDA_0_i(3, state_4); \
    LAMBDA_0_i(4, state_5); \
    LAMBDA_0_i(5, state_6); \
    LAMBDA_0_i(6, state_7); \
    LAMBDA_0_i(7, state_8)

#define LAMBDA_0_PUSH          \
                               \
    LAMBDA_0_i(0, L->word[0]); \
    LAMBDA_0_i(1, L->word[1]); \
    LAMBDA_0_i(2, L->word[2]); \
    LAMBDA_0_i(3, L->word[3]); \
    LAMBDA_0_i(4, L->word[4]); \
    LAMBDA_0_i(5, L->word[5]); \
    LAMBDA_0_i(6, L->word[6]); \
    LAMBDA_0_i(7, L->word[7])

/* avoid temporary register for tap 31 by finishing updating tap 25 before updating tap 0 */
#define LAMBDA_PULL   \
    LAMBDA_25_UPDATE; \
    LAMBDA_0_PULL

#define LAMBDA_PUSH   \
    LAMBDA_25_UPDATE; \
    LAMBDA_0_PUSH

/**************************************************************************/

#define regs(i)  state_##i, gamma_##i, pi_##i, theta_##i

/**************************************************************************/




/**************************************************************************+
*                         Panama external routines                         *
+**************************************************************************/


/**************************************************************************+
*
*  pan_pull() - Performs multiple iterations of the Panama 'Pull' operation.
*               The input and output arrays are treated as integer multiples 
*               of Panama's natural 256-bit block size.
*
*               Input and output arrays may be disjoint or coincident but 
*               may not be overlapped if offset from one another.
*
*               If 'In' is a NULL pointer then output is taken direct from 
*               the state machine (used for hash output). If 'Out' is a NULL 
*               pointer then a dummy 'Pull' is performed. Otherwise 'In' is 
*               XOR combined with the state machine to produce 'Out' 
*               (used for stream encryption / decryption).
*
+**************************************************************************/

static void pan_pull(word32 * restrict In,	/* input array                   */
	      word32 * restrict Out,	/* output array                  */
	      word32 pan_blocks,	/* number of blocks to be Pulled */
	      PAN_BUFFER * restrict buffer,	/* LFSR buffer                   */
	      PAN_STATE * restrict state)
{				/* 17-word finite-state machine  */
	int i;

	word32 regs(0), regs(1), regs(2), regs(3), regs(4);
	word32 regs(5), regs(6), regs(7), regs(8), regs(9);
	word32 regs(10), regs(11), regs(12), regs(13), regs(14);
	word32 regs(15), regs(16);

	word32 tap_0;
	PAN_STAGE *restrict ptap_0, *restrict ptap_25;
	PAN_STAGE *restrict L, *restrict b;

	/* configure routine according to which PULL mode is intended */
	static word32 null_in[PAN_STAGE_SIZE] = { 0, 0, 0, 0, 0, 0, 0, 0 };
	word32 dummy_out[PAN_STAGE_SIZE];
	word32 in_step, out_step;

	in_step = out_step = PAN_STAGE_SIZE;

	if (In == NULL || Out == NULL) {
		In = null_in;
		in_step = 0;
	}

	if (Out == NULL) {
		Out = dummy_out;
		out_step = 0;
	}

	/* copy buffer pointers and state to registers */
	tap_0 = buffer->tap_0;
	READ_STATE;

	/* rho, cascade of state update operations */

	for (i = 0; i < pan_blocks; i++) {
		/* apply state output to crypto buffer */
		Out[0] = In[0] ^ gamma_in_(9);
		Out[1] = In[1] ^ gamma_in_(10);
		Out[2] = In[2] ^ gamma_in_(11);
		Out[3] = In[3] ^ gamma_in_(12);
		Out[4] = In[4] ^ gamma_in_(13);
		Out[5] = In[5] ^ gamma_in_(14);
		Out[6] = In[6] ^ gamma_in_(15);
		Out[7] = In[7] ^ gamma_in_(16);

		Out += out_step;
		In += in_step;

		GAMMA;		/* perform non-linearity stage */

		PI;		/* perform bit-dispersion stage */

		THETA;		/* perform diffusion stage */

		/* calculate pointers to taps 4 and 16 for sigma based on current position of tap 0 */
		L = &buffer->stage[(tap_0 + 4) & (PAN_STAGES - 1)];
		b = &buffer->stage[(tap_0 + 16) & (PAN_STAGES - 1)];

		/* move tap_0 left by one stage, equivalent to shifting LFSR one stage right */
		tap_0 = (tap_0 - 1) & (PAN_STAGES - 1);

		/* set tap pointers for use by lambda */
		ptap_0 = &buffer->stage[tap_0];
		ptap_25 = &buffer->stage[(tap_0 + 25) & (PAN_STAGES - 1)];

		LAMBDA_PULL;	/* update the LFSR buffer */

		/* postpone sigma until after lambda in order to avoid extra temporaries for feedback path */
		/* note that sigma gets to use the old positions of taps 4 and 16 */

		SIGMA;		/* perform buffer injection stage */
	}

	/* write buffer pointer and state back to memory */
	buffer->tap_0 = tap_0;
	WRITE_STATE;
}


/**************************************************************************+
*
*  pan_push() - Performs multiple iterations of the Panama 'Push' operation.
*               The input array is treated as an integer multiple of the 
*               256-bit blocks which are Panama's natural input size.
*
+**************************************************************************/

static void pan_push(word32 * restrict In,	/* input array                   */
	      word32 pan_blocks,	/* number of blocks to be Pushed */
	      PAN_BUFFER * restrict buffer,	/* LFSR buffer                   */
	      PAN_STATE * restrict state)
{				/* 17-word finite-state machine  */
	int i;

	word32 regs(0), regs(1), regs(2), regs(3), regs(4);
	word32 regs(5), regs(6), regs(7), regs(8), regs(9);
	word32 regs(10), regs(11), regs(12), regs(13), regs(14);
	word32 regs(15), regs(16);

	word32 tap_0;
	PAN_STAGE *restrict ptap_0, *restrict ptap_25;
	PAN_STAGE *restrict L, *restrict b;

	/* copy buffer pointers and state to registers */
	tap_0 = buffer->tap_0;
	READ_STATE;

/*	assert((word32 *) ((PAN_STAGE *) In) == In); */
	L = (PAN_STAGE *) In;	/* we assume pointer to input buffer is compatible with pointer to PAN_STAGE */

#ifdef WORDS_BIGENDIAN
	if (L != NULL)
		for (i = 0; i < PAN_STAGE_SIZE; i++) {
			L->word[i] = byteswap32(L->word[i]);
		}
#endif

	/* rho, cascade of state update operations */

	for (i = 0; i < pan_blocks; i++) {
		GAMMA;		/* perform non-linearity stage */

		PI;		/* perform bit-dispersion stage */

		THETA;		/* perform diffusion stage */


		/* calculate pointer to tap 16 for sigma based on current position of tap 0 */
		b = &buffer->stage[(tap_0 + 16) & (PAN_STAGES - 1)];

		/* move tap_0 left by one stage, equivalent to shifting LFSR one stage right */
		tap_0 = (tap_0 - 1) & (PAN_STAGES - 1);

		/* set tap pointers for use by lambda */
		ptap_0 = &buffer->stage[tap_0];
		ptap_25 = &buffer->stage[(tap_0 + 25) & (PAN_STAGES - 1)];

		LAMBDA_PUSH;	/* update the LFSR buffer */

		/* postpone sigma until after lambda in order to avoid extra temporaries for feedback path */
		/* note that sigma gets to use the old positions of taps 4 and 16 */

		SIGMA;		/* perform buffer injection stage */

		L++;		/* In += PAN_STAGE_SIZE; */
	}

	/* write buffer pointer and state back to memory */
	buffer->tap_0 = tap_0;
	WRITE_STATE;

}


/**************************************************************************+
*
*  pan_reset() - Initializes an LFSR buffer and Panama state machine to 
*                all zeros, ready for a new hash to be accumulated or to 
*                re-synchronize or start up an encryption key-stream.
*
+**************************************************************************/

static void pan_reset(PAN_BUFFER * buffer, PAN_STATE * state)
{
	int i, j;

	buffer->tap_0 = 0;

	for (j = 0; j < PAN_STAGES; j++) {
		for (i = 0; i < PAN_STAGE_SIZE; i++) {
			buffer->stage[j].word[i] = 0L;
		}
	}

	for (i = 0; i < PAN_STATE_SIZE; i++) {
		state->word[i] = 0L;
	}
}


/**************************************************************************+
*
*  pan_crypt() - Performs stream encryption or decryption.
*
+**************************************************************************/

WIN32DLL_DEFINE
    int _mcrypt_set_key(PANAMA_KEY * pan_key, char *in_key, int keysize,
			char *init_vec, int vecsize)
{
	byte key[32];
	int keyblocks = (8 * keysize) / (PAN_STAGE_SIZE * WORDLENGTH);
	int vecblocks = (8 * vecsize) / (PAN_STAGE_SIZE * WORDLENGTH);
	int i;

	pan_key->keymat = (void*) pan_key->wkeymat;

/* initialize the Panama state machine for a fresh crypting operation */
	pan_reset(&pan_key->buffer, &pan_key->state);
	pan_push((void *) in_key, keyblocks, &pan_key->buffer,
		 &pan_key->state);
	if (init_vec != NULL)
		pan_push((void *) init_vec, vecblocks, &pan_key->buffer,
			 &pan_key->state);

	pan_pull(NULL, NULL, 32, &pan_key->buffer, &pan_key->state);

	pan_pull(NULL, pan_key->wkeymat, 1, &pan_key->buffer,
		 &pan_key->state);
	pan_key->keymat_pointer = 0;

#ifdef WORDS_BIGENDIAN
	for (i = 0; i < 8; i++) {

		pan_key->wkeymat[i] =
		    byteswap32( pan_key->wkeymat[i]);
	}
#endif

	return 0;
}

WIN32DLL_DEFINE void _mcrypt_encrypt(PANAMA_KEY * pan_key,	/* the key from pan_init */
				     byte * buf,	/* input array                         */
				     int length)
{				/* length to be encrypted, in bits     */
	int i;
#ifdef WORDS_BIGENDIAN
	int j;
#endif

/* initialize the Panama state machine for a fresh crypting operation */
	for (i = 0; i < length; i++) {

		if (pan_key->keymat_pointer == 32) {
			pan_pull(NULL, (void *) pan_key->wkeymat, 1,
				 &pan_key->buffer, &pan_key->state);
			pan_key->keymat_pointer = 0;
#ifdef WORDS_BIGENDIAN
			for (j = 0; j < 8; j++) {
				pan_key->wkeymat[j] =
				    byteswap32( pan_key->wkeymat[j]);
			}
#endif
		}
		buf[i] ^= pan_key->keymat[pan_key->keymat_pointer];
		pan_key->keymat_pointer++;
	}
}

WIN32DLL_DEFINE void _mcrypt_decrypt(PANAMA_KEY * pan_key,	/* the key from pan_init */
				     byte * buf,	/* input array                         */
				     int length)
{				/* length to be encrypted, in bits     */
	_mcrypt_encrypt(pan_key, buf, length);
}

/**************************************************************************/


WIN32DLL_DEFINE int _mcrypt_get_size()
{
	return sizeof(PANAMA_KEY);
}
WIN32DLL_DEFINE int _mcrypt_get_block_size()
{
	return 1;
}
WIN32DLL_DEFINE int _mcrypt_get_algo_iv_size()
{
	return 32;
}
WIN32DLL_DEFINE int _is_block_algorithm()
{
	return 0;
}
WIN32DLL_DEFINE int _mcrypt_get_key_size()
{
	return 32;
}

static const int key_sizes[] = { 32 };
WIN32DLL_DEFINE const int *_mcrypt_get_supported_key_sizes(int *len)
{
	*len = sizeof(key_sizes)/sizeof(int);
	return key_sizes;

}

WIN32DLL_DEFINE char *_mcrypt_get_algorithms_name()
{
return "PANAMA";
}

#define CIPHER "d76e3c2243feadd2c99edfcb95c64c852ba6c59f"

WIN32DLL_DEFINE int _mcrypt_self_test()
{
	char *keyword;
	unsigned char plaintext[20];
	unsigned char ciphertext[20];
	int blocksize = 20, j;
	void *key;
	unsigned char cipher_tmp[200];

	keyword = calloc(1, _mcrypt_get_key_size());
	if (keyword == NULL)
		return -1;

	for (j = 0; j < _mcrypt_get_key_size(); j++) {
		keyword[j] = ((j * 2 + 10) % 256);
	}

	for (j = 0; j < blocksize; j++) {
		plaintext[j] = j % 256;
	}
	key = malloc(_mcrypt_get_size());
	if (key == NULL) {
		free(keyword);
		return -1;
	}

	memcpy(ciphertext, plaintext, blocksize);

	_mcrypt_set_key(key, (void *) keyword, _mcrypt_get_key_size(),
			NULL, 0);
	_mcrypt_encrypt(key, (void *) ciphertext, blocksize);

	for (j = 0; j < blocksize; j++) {
		sprintf(&((char *) cipher_tmp)[2 * j], "%.2x",
			ciphertext[j]);
	}

	if (strcmp((char *) cipher_tmp, CIPHER) != 0) {
		printf("failed compatibility\n");
		printf("Expected: %s\nGot: %s\n", CIPHER,
		       (char *) cipher_tmp);
		free(keyword);
		free(key);
		return -1;
	}

	_mcrypt_set_key(key, (void *) keyword, _mcrypt_get_key_size(),
			NULL, 0);
	free(keyword);

	_mcrypt_decrypt(key, (void *) ciphertext, blocksize);
	free(key);

	if (strcmp(ciphertext, plaintext) != 0) {
		printf("failed internally\n");
		return -1;
	}

	return 0;
}

WIN32DLL_DEFINE word32 _mcrypt_algorithm_version(void)
{
	return 20010801;
}

#ifdef WIN32
# ifdef USE_LTDL
WIN32DLL_DEFINE int main (void)
{
       /* empty main function to avoid linker error (see cygwin FAQ) */
}
# endif
#endif
