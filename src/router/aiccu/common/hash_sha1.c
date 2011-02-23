/*
 * sha1.c
 *
 * Originally witten by Steve Reid <steve@edmweb.com>
 * 
 * Modified by Aaron D. Gifford <agifford@infowest.com>
 *
 * NO COPYRIGHT - THIS IS 100% IN THE PUBLIC DOMAIN
 *
 * The original unmodified version is available at:
 *    ftp://ftp.funet.fi/pub/crypt/hash/sha/sha1.c
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR(S) OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "hash_sha1.h"
#include <stdlib.h>
#include <string.h>

#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */

#if BYTE_ORDER == LITTLE_ENDIAN
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&(sha1_quadbyte)0xFF00FF00) \
	|(rol(block->l[i],8)&(sha1_quadbyte)0x00FF00FF))
#else
#define blk0(i) block->l[i]
#endif

#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
	^block->l[(i+2)&15]^block->l[i&15],1))

/* (SHA_R0+SHA_R1), SHA_R2, SHA_R3, SHA_R4 are the different operations used in SHA1 */
#define SHA_R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define SHA_R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define SHA_R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define SHA_R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define SHA_R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);

typedef union _BYTE64QUAD16 {
	sha1_byte c[64];
	sha1_quadbyte l[16];
} BYTE64QUAD16;

/* Hash a single 512-bit block. This is the core of the algorithm. */
void SHA1_Transform(sha1_quadbyte state[5], const sha1_byte buffer[64]);
void SHA1_Transform(sha1_quadbyte state[5], const sha1_byte buffer[64])
{
	sha1_quadbyte	a, b, c, d, e;
	BYTE64QUAD16	*block;

	block = (BYTE64QUAD16*)buffer;
	/* Copy context->state[] to working vars */
	a = state[0];
	b = state[1];
	c = state[2];
	d = state[3];
	e = state[4];
	/* 4 rounds of 20 operations each. Loop unrolled. */
	SHA_R0(a,b,c,d,e, 0); SHA_R0(e,a,b,c,d, 1); SHA_R0(d,e,a,b,c, 2); SHA_R0(c,d,e,a,b, 3);
	SHA_R0(b,c,d,e,a, 4); SHA_R0(a,b,c,d,e, 5); SHA_R0(e,a,b,c,d, 6); SHA_R0(d,e,a,b,c, 7);
	SHA_R0(c,d,e,a,b, 8); SHA_R0(b,c,d,e,a, 9); SHA_R0(a,b,c,d,e,10); SHA_R0(e,a,b,c,d,11);
	SHA_R0(d,e,a,b,c,12); SHA_R0(c,d,e,a,b,13); SHA_R0(b,c,d,e,a,14); SHA_R0(a,b,c,d,e,15);
	SHA_R1(e,a,b,c,d,16); SHA_R1(d,e,a,b,c,17); SHA_R1(c,d,e,a,b,18); SHA_R1(b,c,d,e,a,19);
	SHA_R2(a,b,c,d,e,20); SHA_R2(e,a,b,c,d,21); SHA_R2(d,e,a,b,c,22); SHA_R2(c,d,e,a,b,23);
	SHA_R2(b,c,d,e,a,24); SHA_R2(a,b,c,d,e,25); SHA_R2(e,a,b,c,d,26); SHA_R2(d,e,a,b,c,27);
	SHA_R2(c,d,e,a,b,28); SHA_R2(b,c,d,e,a,29); SHA_R2(a,b,c,d,e,30); SHA_R2(e,a,b,c,d,31);
	SHA_R2(d,e,a,b,c,32); SHA_R2(c,d,e,a,b,33); SHA_R2(b,c,d,e,a,34); SHA_R2(a,b,c,d,e,35);
	SHA_R2(e,a,b,c,d,36); SHA_R2(d,e,a,b,c,37); SHA_R2(c,d,e,a,b,38); SHA_R2(b,c,d,e,a,39);
	SHA_R3(a,b,c,d,e,40); SHA_R3(e,a,b,c,d,41); SHA_R3(d,e,a,b,c,42); SHA_R3(c,d,e,a,b,43);
	SHA_R3(b,c,d,e,a,44); SHA_R3(a,b,c,d,e,45); SHA_R3(e,a,b,c,d,46); SHA_R3(d,e,a,b,c,47);
	SHA_R3(c,d,e,a,b,48); SHA_R3(b,c,d,e,a,49); SHA_R3(a,b,c,d,e,50); SHA_R3(e,a,b,c,d,51);
	SHA_R3(d,e,a,b,c,52); SHA_R3(c,d,e,a,b,53); SHA_R3(b,c,d,e,a,54); SHA_R3(a,b,c,d,e,55);
	SHA_R3(e,a,b,c,d,56); SHA_R3(d,e,a,b,c,57); SHA_R3(c,d,e,a,b,58); SHA_R3(b,c,d,e,a,59);
	SHA_R4(a,b,c,d,e,60); SHA_R4(e,a,b,c,d,61); SHA_R4(d,e,a,b,c,62); SHA_R4(c,d,e,a,b,63);
	SHA_R4(b,c,d,e,a,64); SHA_R4(a,b,c,d,e,65); SHA_R4(e,a,b,c,d,66); SHA_R4(d,e,a,b,c,67);
	SHA_R4(c,d,e,a,b,68); SHA_R4(b,c,d,e,a,69); SHA_R4(a,b,c,d,e,70); SHA_R4(e,a,b,c,d,71);
	SHA_R4(d,e,a,b,c,72); SHA_R4(c,d,e,a,b,73); SHA_R4(b,c,d,e,a,74); SHA_R4(a,b,c,d,e,75);
	SHA_R4(e,a,b,c,d,76); SHA_R4(d,e,a,b,c,77); SHA_R4(c,d,e,a,b,78); SHA_R4(b,c,d,e,a,79);
	/* Add the working vars back into context.state[] */
	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
	state[4] += e;
	/* Wipe variables */
	a = b = c = d = e = 0;
}


/* SHA1_Init - Initialize new context */
void SHA1_Init(SHA_CTX* context) {
	/* SHA1 initialization constants */
	context->state[0] = 0x67452301;
	context->state[1] = 0xEFCDAB89;
	context->state[2] = 0x98BADCFE;
	context->state[3] = 0x10325476;
	context->state[4] = 0xC3D2E1F0;
	context->count[0] = context->count[1] = 0;
}

/* Run your data through this. */
void SHA1_Update(SHA_CTX *context, const sha1_byte *d, unsigned int len) {
	unsigned int	i, j;

	/* Make a temporary storage as Transform destroys it */
	sha1_byte *data = (sha1_byte *)malloc(len);
	if (!data) exit(-42);
	memcpy(data, d, len);

	j = (context->count[0] >> 3) & 63;
	if ((context->count[0] += len << 3) < (len << 3)) context->count[1]++;
	context->count[1] += (len >> 29);
	if ((j + len) > 63) {
	    memcpy(&context->buffer[j], data, (i = 64-j));
	    SHA1_Transform(context->state, context->buffer);
	    for ( ; i + 63 < len; i += 64) {
	        SHA1_Transform(context->state, &data[i]);
	    }
	    j = 0;
	}
	else i = 0;
	memcpy(&context->buffer[j], &data[i], len - i);

	/* Free the temporary buffer */
	free(data);
}

/* Add padding and return the message digest. */
void SHA1_Final(sha1_byte digest[SHA1_DIGEST_LENGTH], SHA_CTX *context) {
	sha1_quadbyte	i, j;
	sha1_byte	finalcount[8];

	for (i = 0; i < 8; i++) {
	    finalcount[i] = (sha1_byte)((context->count[(i >= 4 ? 0 : 1)]
	     >> ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
	}
	SHA1_Update(context, (sha1_byte *)"\200", 1);
	while ((context->count[0] & 504) != 448) {
	    SHA1_Update(context, (sha1_byte *)"\0", 1);
	}
	/* Should cause a SHA1_Transform() */
	SHA1_Update(context, finalcount, 8);
	for (i = 0; i < SHA1_DIGEST_LENGTH; i++) {
	    digest[i] = (sha1_byte)
	     ((context->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
	}
	/* Wipe variables */
	i = j = 0;
	memset(context->buffer, 0, SHA1_BLOCK_LENGTH);
	memset(context->state, 0, SHA1_DIGEST_LENGTH);
	memset(context->count, 0, 8);
	memset(&finalcount, 0, 8);
}

