// I retain copyright in this code but I encourage its free use provided
// that I don't carry any responsibility for the results. I am especially 
// happy to see it used in free and open source software. If you do use 
// it I would appreciate an acknowledgement of its origin in the code or
// the product that results and I would also appreciate knowing a little
// about the use to which it is being put. I am grateful to Frank Yellin
// for some ideas that are used in this implementation.
//
// Dr B. R. Gladman <brg@gladman.uk.net> 6th April 2001.
//
// This is an implementation of the AES encryption algorithm (Rijndael)
// designed by Joan Daemen and Vincent Rijmen. This version is designed
// to provide both fixed and dynamic block and key lengths and can also 
// run with either big or little endian internal byte order (see aes.h). 
// It inputs block and key lengths in bytes with the legal values being 
// 16, 24 and 32.

/*
 * Modified by Jari Ruusu,  May 1 2001
 *  - Fixed some compile warnings, code was ok but gcc warned anyway.
 *  - Changed basic types: byte -> unsigned char, word -> u_int32_t
 *  - Major name space cleanup: Names visible to outside now begin
 *    with "aes_" or "AES_". A lot of stuff moved from aes.h to aes.c
 *  - Removed C++ and DLL support as part of name space cleanup.
 *  - Eliminated unnecessary recomputation of tables. (actual bug fix)
 *  - Merged precomputed constant tables to aes.c file.
 *  - Removed data alignment restrictions for portability reasons.
 *  - Made block and key lengths accept bit count (128/192/256)
 *    as well byte count (16/24/32).
 *  - Removed all error checks. This change also eliminated the need
 *    to preinitialize the context struct to zero.
 *  - Removed some totally unused constants.
 */

/*
 * Modified by Jari Ruusu,  June 9 2003
 *  - Removed all code not necessary for small size
 *    optimized encryption using 256 bit keys.
 */

#include "aes.h"

#if AES_BLOCK_SIZE != 16
#error an illegal block size has been specified
#endif  

// upr(x,n): rotates bytes within words by n positions, moving bytes 
// to higher index positions with wrap around into low positions
// bval(x,n): extracts a byte from a word

#define upr(x,n)        (((x) << 8 * (n)) | ((x) >> (32 - 8 * (n))))
#define bval(x,n)       ((unsigned char)((x) >> 8 * (n)))
#define bytes2word(b0, b1, b2, b3)  \
        ((u_int32_t)(b3) << 24 | (u_int32_t)(b2) << 16 | (u_int32_t)(b1) << 8 | (b0))

#if defined(i386) || defined(_I386) || defined(__i386__) || defined(__i386)
/* little endian processor without data alignment restrictions */
#define word_in(x)      *(u_int32_t*)(x)
#define word_out(x,v)   *(u_int32_t*)(x) = (v)
#else
/* slower but generic big endian or with data alignment restrictions */
#define word_in(x)      ((u_int32_t)(((unsigned char *)(x))[0])|((u_int32_t)(((unsigned char *)(x))[1])<<8)|((u_int32_t)(((unsigned char *)(x))[2])<<16)|((u_int32_t)(((unsigned char *)(x))[3])<<24))
#define word_out(x,v)   ((unsigned char *)(x))[0]=(v),((unsigned char *)(x))[1]=((v)>>8),((unsigned char *)(x))[2]=((v)>>16),((unsigned char *)(x))[3]=((v)>>24)
#endif

// the finite field modular polynomial and elements

#define ff_poly 0x011b
#define ff_hi   0x80

static int tab_gen = 0;
static unsigned char  s_box[256];            // the S box
static u_int32_t  rcon_tab[AES_RC_LENGTH];   // table of round constants
static u_int32_t  ft_tab[4][256];
static u_int32_t  fl_tab[4][256];

// Generate the tables for the dynamic table option

// It will generally be sensible to use tables to compute finite 
// field multiplies and inverses but where memory is scarse this 
// code might sometimes be better.

// return 2 ^ (n - 1) where n is the bit number of the highest bit
// set in x with x in the range 1 < x < 0x00000200.   This form is
// used so that locals within FFinv can be bytes rather than words

static unsigned char hibit(const u_int32_t x)
{   unsigned char r = (unsigned char)((x >> 1) | (x >> 2));
    
    r |= (r >> 2);
    r |= (r >> 4);
    return (r + 1) >> 1;
}

// return the inverse of the finite field element x

static unsigned char FFinv(const unsigned char x)
{   unsigned char    p1 = x, p2 = 0x1b, n1 = hibit(x), n2 = 0x80, v1 = 1, v2 = 0;

    if(x < 2) return x;

    for(;;)
    {
        if(!n1) return v1;

        while(n2 >= n1)
        {   
            n2 /= n1; p2 ^= p1 * n2; v2 ^= v1 * n2; n2 = hibit(p2);
        }
        
        if(!n2) return v2;

        while(n1 >= n2)
        {   
            n1 /= n2; p1 ^= p2 * n1; v1 ^= v2 * n1; n1 = hibit(p1);
        }
    }
}

// define the finite field multiplies required for Rijndael

#define FFmul02(x)  ((((x) & 0x7f) << 1) ^ ((x) & 0x80 ? 0x1b : 0))
#define FFmul03(x)  ((x) ^ FFmul02(x))

// The forward and inverse affine transformations used in the S-box

#define fwd_affine(x) \
    (w = (u_int32_t)x, w ^= (w<<1)^(w<<2)^(w<<3)^(w<<4), 0x63^(unsigned char)(w^(w>>8)))

static void gen_tabs(void)
{   u_int32_t  i, w;

    for(i = 0, w = 1; i < AES_RC_LENGTH; ++i)
    {
        rcon_tab[i] = bytes2word(w, 0, 0, 0);
        w = (w << 1) ^ (w & ff_hi ? ff_poly : 0);
    }

    for(i = 0; i < 256; ++i)
    {   unsigned char    b;

        s_box[i] = b = fwd_affine(FFinv((unsigned char)i));

        w = bytes2word(b, 0, 0, 0);
        fl_tab[0][i] = w;
        fl_tab[1][i] = upr(w,1);
        fl_tab[2][i] = upr(w,2);
        fl_tab[3][i] = upr(w,3);
        w = bytes2word(FFmul02(b), b, b, FFmul03(b));
        ft_tab[0][i] = w;
        ft_tab[1][i] = upr(w,1);
        ft_tab[2][i] = upr(w,2);
        ft_tab[3][i] = upr(w,3);
    }
}

#define four_tables(x,tab,vf,rf,c) \
 (  tab[0][bval(vf(x,0,c),rf(0,c))] \
  ^ tab[1][bval(vf(x,1,c),rf(1,c))] \
  ^ tab[2][bval(vf(x,2,c),rf(2,c))] \
  ^ tab[3][bval(vf(x,3,c),rf(3,c))])

#define vf1(x,r,c)  (x)
#define rf1(r,c)    (r)
#define rf2(r,c)    ((r-c)&3)

#define ls_box(x,c)     four_tables(x,fl_tab,vf1,rf2,c)

#define nc   (AES_BLOCK_SIZE / 4)

// Initialise the key schedule from the user supplied key.
// The key length is now specified in bytes, 32.
// This corresponds to bit length of 256 bits, and
// to Nk value of 8 respectively.

void __loDev_aes_set_key(aes_context *cx, const unsigned char in_key[], int n_bytes __attribute__((__unused__)), const int f __attribute__((__unused__)))
{   u_int32_t    *kf, *kt, rci;

    if(!tab_gen) { gen_tabs(); tab_gen = 1; }

    cx->aes_Nkey = 8;
    cx->aes_Nrnd = (cx->aes_Nkey > nc ? cx->aes_Nkey : nc) + 6; 

    cx->aes_e_key[0] = word_in(in_key     );
    cx->aes_e_key[1] = word_in(in_key +  4);
    cx->aes_e_key[2] = word_in(in_key +  8);
    cx->aes_e_key[3] = word_in(in_key + 12);

    kf = cx->aes_e_key; 
    kt = kf + nc * (cx->aes_Nrnd + 1) - cx->aes_Nkey; 
    rci = 0;

    switch(cx->aes_Nkey)
    {
    case 8: cx->aes_e_key[4] = word_in(in_key + 16);
            cx->aes_e_key[5] = word_in(in_key + 20);
            cx->aes_e_key[6] = word_in(in_key + 24);
            cx->aes_e_key[7] = word_in(in_key + 28);
            do
            {   kf[ 8] = kf[0] ^ ls_box(kf[7],3) ^ rcon_tab[rci++];
                kf[ 9] = kf[1] ^ kf[ 8];
                kf[10] = kf[2] ^ kf[ 9];
                kf[11] = kf[3] ^ kf[10];
                kf[12] = kf[4] ^ ls_box(kf[11],0);
                kf[13] = kf[5] ^ kf[12];
                kf[14] = kf[6] ^ kf[13];
                kf[15] = kf[7] ^ kf[14];
                kf += 8;
            }
            while (kf < kt);
            break;
    }
}

// y = output word, x = input word, r = row, c = column
// for r = 0, 1, 2 and 3 = column accessed for row r

#define s(x,c) x[c]

// I am grateful to Frank Yellin for the following constructions
// which, given the column (c) of the output state variable that
// is being computed, return the input state variables which are
// needed for each row (r) of the state

// For the fixed block size options, compilers reduce these two 
// expressions to fixed variable references. For variable block 
// size code conditional clauses will sometimes be returned

#define fwd_var(x,r,c) \
 ( r==0 ?			\
    ( c==0 ? s(x,0) \
    : c==1 ? s(x,1) \
    : c==2 ? s(x,2) \
    : c==3 ? s(x,3) \
    : c==4 ? s(x,4) \
    : c==5 ? s(x,5) \
    : c==6 ? s(x,6) \
    : s(x,7))		\
 : r==1 ?			\
    ( c==0 ? s(x,1) \
    : c==1 ? s(x,2) \
    : c==2 ? s(x,3) \
    : c==3 ? nc==4 ? s(x,0) : s(x,4) \
    : c==4 ? s(x,5) \
    : c==5 ? nc==8 ? s(x,6) : s(x,0) \
    : c==6 ? s(x,7) \
    : s(x,0))		\
 : r==2 ?			\
    ( c==0 ? nc==8 ? s(x,3) : s(x,2) \
    : c==1 ? nc==8 ? s(x,4) : s(x,3) \
    : c==2 ? nc==4 ? s(x,0) : nc==8 ? s(x,5) : s(x,4) \
    : c==3 ? nc==4 ? s(x,1) : nc==8 ? s(x,6) : s(x,5) \
    : c==4 ? nc==8 ? s(x,7) : s(x,0) \
    : c==5 ? nc==8 ? s(x,0) : s(x,1) \
    : c==6 ? s(x,1) \
    : s(x,2))		\
 :					\
    ( c==0 ? nc==8 ? s(x,4) : s(x,3) \
    : c==1 ? nc==4 ? s(x,0) : nc==8 ? s(x,5) : s(x,4) \
    : c==2 ? nc==4 ? s(x,1) : nc==8 ? s(x,6) : s(x,5) \
    : c==3 ? nc==4 ? s(x,2) : nc==8 ? s(x,7) : s(x,0) \
    : c==4 ? nc==8 ? s(x,0) : s(x,1) \
    : c==5 ? nc==8 ? s(x,1) : s(x,2) \
    : c==6 ? s(x,2) \
    : s(x,3)))

#define si(y,x,k,c) s(y,c) = word_in(x + 4 * c) ^ k[c]
#define so(y,x,c)   word_out(y + 4 * c, s(x,c))

#define fwd_rnd(y,x,k,c)    s(y,c)= (k)[c] ^ four_tables(x,ft_tab,fwd_var,rf1,c)
#define fwd_lrnd(y,x,k,c)   s(y,c)= (k)[c] ^ four_tables(x,fl_tab,fwd_var,rf1,c)

#define locals(y,x)     x[4]={},y[4]={}

#define l_copy(y, x)    s(y,0) = s(x,0); s(y,1) = s(x,1); \
                        s(y,2) = s(x,2); s(y,3) = s(x,3);
#define state_in(y,x,k) si(y,x,k,0); si(y,x,k,1); si(y,x,k,2); si(y,x,k,3)
#define state_out(y,x)  so(y,x,0); so(y,x,1); so(y,x,2); so(y,x,3)
#define round(rm,y,x,k) rm(y,x,k,0); rm(y,x,k,1); rm(y,x,k,2); rm(y,x,k,3)

void __loDev_aes_encrypt(const aes_context *cx, const unsigned char in_blk[], unsigned char out_blk[])
{   u_int32_t        locals(b0, b1);
    const u_int32_t  *kp = cx->aes_e_key;

    state_in(b0, in_blk, kp); kp += nc;

    {   u_int32_t    rnd;

        for(rnd = 0; rnd < cx->aes_Nrnd - 1; ++rnd)
        {
            round(fwd_rnd, b1, b0, kp); 
            l_copy(b0, b1); kp += nc;
        }

        round(fwd_lrnd, b0, b1, kp);
    }

    state_out(out_blk, b0);
}
