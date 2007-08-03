
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>

#include <broadcom.h>


#include <stdio.h>

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//#include "libbb.h"
typedef struct sha1_ctx_t {
	uint32_t count[2];
	uint32_t hash[5];
	uint32_t wbuf[16];
} sha1_ctx_t;

#define SHA1_BLOCK_SIZE  64
#define SHA1_DIGEST_SIZE 20
#define SHA1_HASH_SIZE   SHA1_DIGEST_SIZE
#define SHA2_GOOD        0
#define SHA2_BAD         1

#define rotl32(x,n)      (((x) << n) | ((x) >> (32 - n)))

#define SHA1_MASK        (SHA1_BLOCK_SIZE - 1)

/* reverse byte order in 32-bit words   */
#define ch(x,y,z)        ((z) ^ ((x) & ((y) ^ (z))))
#define parity(x,y,z)    ((x) ^ (y) ^ (z))
#define maj(x,y,z)       (((x) & (y)) | ((z) & ((x) | (y))))

/* A normal version as set out in the FIPS. This version uses   */
/* partial loop unrolling and is optimised for the Pentium 4    */
#define rnd(f,k) \
	do { \
		t = a; a = rotl32(a,5) + f(b,c,d) + e + k + w[i]; \
		e = d; d = c; c = rotl32(b, 30); b = t; \
	} while (0)

static void sha1_compile(sha1_ctx_t *ctx)
{
	uint32_t w[80], i, a, b, c, d, e, t;

	/* note that words are compiled from the buffer into 32-bit */
	/* words in big-endian order so an order reversal is needed */
	/* here on little endian machines                           */
	for (i = 0; i < SHA1_BLOCK_SIZE / 4; ++i)
		w[i] = htonl(ctx->wbuf[i]);

	for (i = SHA1_BLOCK_SIZE / 4; i < 80; ++i)
		w[i] = rotl32(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);

	a = ctx->hash[0];
	b = ctx->hash[1];
	c = ctx->hash[2];
	d = ctx->hash[3];
	e = ctx->hash[4];

	for (i = 0; i < 20; ++i) {
		rnd(ch, 0x5a827999);
	}

	for (i = 20; i < 40; ++i) {
		rnd(parity, 0x6ed9eba1);
	}

	for (i = 40; i < 60; ++i) {
		rnd(maj, 0x8f1bbcdc);
	}

	for (i = 60; i < 80; ++i) {
		rnd(parity, 0xca62c1d6);
	}

	ctx->hash[0] += a;
	ctx->hash[1] += b;
	ctx->hash[2] += c;
	ctx->hash[3] += d;
	ctx->hash[4] += e;
}

void sha1_begin(sha1_ctx_t *ctx)
{
	ctx->count[0] = ctx->count[1] = 0;
	ctx->hash[0] = 0x67452301;
	ctx->hash[1] = 0xefcdab89;
	ctx->hash[2] = 0x98badcfe;
	ctx->hash[3] = 0x10325476;
	ctx->hash[4] = 0xc3d2e1f0;
}

/* SHA1 hash data in an array of bytes into hash buffer and call the        */
/* hash_compile function as required.                                       */
void sha1_hash(const void *data, size_t length, sha1_ctx_t *ctx)
{
	uint32_t pos = (uint32_t) (ctx->count[0] & SHA1_MASK);
	uint32_t freeb = SHA1_BLOCK_SIZE - pos;
	const unsigned char *sp = data;

	if ((ctx->count[0] += length) < length)
		++(ctx->count[1]);

	while (length >= freeb) {	/* tranfer whole blocks while possible  */
		memcpy(((unsigned char *) ctx->wbuf) + pos, sp, freeb);
		sp += freeb;
		length -= freeb;
		freeb = SHA1_BLOCK_SIZE;
		pos = 0;
		sha1_compile(ctx);
	}

	memcpy(((unsigned char *) ctx->wbuf) + pos, sp, length);
}

void *sha1_end(void *resbuf, sha1_ctx_t *ctx)
{
	/* SHA1 Final padding and digest calculation  */
#if BB_BIG_ENDIAN
	static uint32_t mask[4] = { 0x00000000, 0xff000000, 0xffff0000, 0xffffff00 };
	static uint32_t bits[4] = { 0x80000000, 0x00800000, 0x00008000, 0x00000080 };
#else
	static uint32_t mask[4] = { 0x00000000, 0x000000ff, 0x0000ffff, 0x00ffffff };
	static uint32_t bits[4] = { 0x00000080, 0x00008000, 0x00800000, 0x80000000 };
#endif

	uint8_t *hval = resbuf;
	uint32_t i, cnt = (uint32_t) (ctx->count[0] & SHA1_MASK);

	/* mask out the rest of any partial 32-bit word and then set    */
	/* the next byte to 0x80. On big-endian machines any bytes in   */
	/* the buffer will be at the top end of 32 bit words, on little */
	/* endian machines they will be at the bottom. Hence the AND    */
	/* and OR masks above are reversed for little endian systems    */
	ctx->wbuf[cnt >> 2] =
		(ctx->wbuf[cnt >> 2] & mask[cnt & 3]) | bits[cnt & 3];

	/* we need 9 or more empty positions, one for the padding byte  */
	/* (above) and eight for the length count.  If there is not     */
	/* enough space pad and empty the buffer                        */
	if (cnt > SHA1_BLOCK_SIZE - 9) {
		if (cnt < 60)
			ctx->wbuf[15] = 0;
		sha1_compile(ctx);
		cnt = 0;
	} else				/* compute a word index for the empty buffer positions  */
		cnt = (cnt >> 2) + 1;

	while (cnt < 14)	/* and zero pad all but last two positions      */
		ctx->wbuf[cnt++] = 0;

	/* assemble the eight byte counter in the buffer in big-endian  */
	/* format					                */

	ctx->wbuf[14] = htonl((ctx->count[1] << 3) | (ctx->count[0] >> 29));
	ctx->wbuf[15] = htonl(ctx->count[0] << 3);

	sha1_compile(ctx);

	/* extract the hash value as bytes in case the hash buffer is   */
	/* misaligned for 32-bit words                                  */

	for (i = 0; i < SHA1_DIGEST_SIZE; ++i)
		hval[i] = (unsigned char) (ctx->hash[i >> 2] >> 8 * (~i & 3));

	return resbuf;
}




void
base64_encode (const unsigned char *in, size_t inlen,
	       unsigned char *out, size_t outlen)
{
  static const char b64str[64] =
    "./ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

  while (inlen && outlen)
    {
      *out++ = b64str[(in[0] >> 2) & 0x3f];
      if (!--outlen)
	break;
      *out++ = b64str[((in[0] << 4) + (--inlen ? in[1] >> 4 : 0)) & 0x3f];
      if (!--outlen)
	break;
      *out++ =
	(inlen
	 ? b64str[((in[1] << 2)
		   + (--inlen ? in[2] >> 6 : 0))
		  & 0x3f]
	 : '=');
      if (!--outlen)
	break;
      *out++ = inlen ? b64str[in[2] & 0x3f] : '=';
      if (!--outlen)
	break;
      if (inlen)
	inlen--;
      if (inlen)
	in += 3;
    }

  if (outlen)
    *out = '\0';
}

char *request_freedns(char *user,char *password)
{
unsigned char final[32];
unsigned char out[64];
char un[128];
sprintf(un,"%s|%s",user,password);
sha1_ctx_t context;
sha1_begin(&context);
sha1_hash(un,strlen(un),&context);
sha1_end(final,&context);
char request[128]={0};
char request2[128]={0};
int i;
for (i=0;i<20;i++)
sprintf(request,"%s%x",request,final[i]);
system("rm -f /tmp/.hash");
sprintf(request2,"wget \"http://freedns.afraid.org/api/?action=getdyndns&sha=%s\" -O /tmp/.hash",request);
system(request2);
FILE *in=fopen("/tmp/.hash","rb");
if (in==NULL)
    return NULL;
while(getc(in)!='?' && feof(in)==0);
i=0;
char *hash=malloc(64);
while(feof(in)==0)
    hash[i++]=getc(in);
fclose(in);
hash[i++]=0;
return hash;
}

void
ej_show_ddns_status (webs_t wp, int argc, char_t ** argv)
{
  char buff[512];
  FILE *fp;
  char *enable = websGetVar (wp, "ddns_enable", NULL);

  if (!enable)
    enable = nvram_safe_get ("ddns_enable");	// for first time

  if (strcmp (nvram_safe_get ("ddns_enable"), enable))	// change service
    websWrite (wp, " ");

  if (nvram_match ("ddns_enable", "0"))	// only for no hidden page
    {
      websWrite (wp, "%s", live_translate ("ddnsm.all_disabled"));
      return;
    }

  /*if (!check_wan_link (0))
     {
     websWrite (wp,
     "<script type=\"text/javascript\">Capture(ddnsm.all_noip)</script>");
     return;
     } */

  if ((fp = fopen ("/tmp/ddns/ddns.log", "r")))
    {
      /* Just dump the log file onto the web page */
      while (fgets (buff, sizeof (buff), fp))
	{
	  websWrite (wp, "%s <br />", buff);
	}
      fclose (fp);
    }
  else
    {
      websWrite (wp, "%s", live_translate ("ddnsm.all_connecting"));
      return;
    }

  return;
}

int
ddns_save_value (webs_t wp)
{
  char *enable, *username, *passwd, *hostname, *dyndnstype, *wildcard,
    *custom, *conf, *url, *force;
  struct variable ddns_variables[] = {
  {argv:ARGV ("0", "1", "2", "3", "4", "5", "6",
	  "7",
	  "8")},
  {argv:ARGV ("30")},
  }, *which;
  int ret = -1;
  char _username[] = "ddns_username_X";
  char _passwd[] = "ddns_passwd_X";
  char _hostname[] = "ddns_hostname_X";
  char _dyndnstype[] = "ddns_dyndnstype_X";
  char _wildcard[] = "ddns_wildcard_X";
  char _custom[] = "ddns_custom_X";
  char _conf[] = "ddns_conf";
  char _url[] = "ddns_url";
  char _force[] = "ddns_force";

  which = &ddns_variables[0];

  enable = websGetVar (wp, "ddns_enable", NULL);
  if (!enable && !valid_choice (wp, enable, &which[0]))
    {
      error_value = 1;
      return 1;
    }
int gethash=0;
  if (atoi (enable) == 0)
    {
      // Disable
      nvram_set ("ddns_enable", enable);
      return 1;
    }
  else if (atoi (enable) == 1)
    {
      // dyndns
      snprintf (_username, sizeof (_username), "%s", "ddns_username");
      snprintf (_passwd, sizeof (_passwd), "%s", "ddns_passwd");
      snprintf (_hostname, sizeof (_hostname), "%s", "ddns_hostname");
      snprintf (_dyndnstype, sizeof (_dyndnstype), "%s", "ddns_dyndnstype");
      snprintf (_wildcard, sizeof (_wildcard), "%s", "ddns_wildcard");
    }
  else if (atoi (enable) == 2)
    {
      // afraid
      snprintf (_username, sizeof (_username), "ddns_username_%s", enable);
      snprintf (_passwd, sizeof (_passwd), "ddns_passwd_%s", enable);
      snprintf (_hostname, sizeof (_hostname), "ddns_hostname_%s", enable);
      gethash=1;
    }
  else if (atoi (enable) == 3)
    {
      // zoneedit
      snprintf (_username, sizeof (_username), "ddns_username_%s", enable);
      snprintf (_passwd, sizeof (_passwd), "ddns_passwd_%s", enable);
      snprintf (_hostname, sizeof (_hostname), "ddns_hostname_%s", enable);
    }
  else if (atoi (enable) == 4)
    {
      // no-ip
      snprintf (_username, sizeof (_username), "ddns_username_%s", enable);
      snprintf (_passwd, sizeof (_passwd), "ddns_passwd_%s", enable);
      snprintf (_hostname, sizeof (_hostname), "ddns_hostname_%s", enable);
    }
  else if (atoi (enable) == 5)
    {
      // custom
      snprintf (_username, sizeof (_username), "ddns_username_%s", enable);
      snprintf (_passwd, sizeof (_passwd), "ddns_passwd_%s", enable);
      snprintf (_hostname, sizeof (_hostname), "ddns_hostname_%s", enable);
      snprintf (_custom, sizeof (_custom), "ddns_custom_%s", enable);
      snprintf (_conf, sizeof (_conf), "%s", "ddns_conf");
      snprintf (_url, sizeof (_url), "%s", "ddns_url");
    }
  else if (atoi (enable) == 6)
    {
      // 3322 dynamic : added botho 30/07/06
      snprintf (_username, sizeof (_username), "ddns_username_%s", enable);
      snprintf (_passwd, sizeof (_passwd), "ddns_passwd_%s", enable);
      snprintf (_hostname, sizeof (_hostname), "ddns_hostname_%s", enable);
      snprintf (_dyndnstype, sizeof (_dyndnstype), "ddns_dyndnstype_%s",
		enable);
      snprintf (_wildcard, sizeof (_wildcard), "ddns_wildcard_%s", enable);
    }
  else if (atoi (enable) == 7)
    {
      // easydns
      snprintf (_username, sizeof (_username), "ddns_username_%s", enable);
      snprintf (_passwd, sizeof (_passwd), "ddns_passwd_%s", enable);
      snprintf (_hostname, sizeof (_hostname), "ddns_hostname_%s", enable);
      snprintf (_wildcard, sizeof (_wildcard), "ddns_wildcard_%s", enable);
    }
  else if (atoi (enable) == 8)
    {
      // tzo
      snprintf (_username, sizeof (_username), "ddns_username_%s", enable);
      snprintf (_passwd, sizeof (_passwd), "ddns_passwd_%s", enable);
      snprintf (_hostname, sizeof (_hostname), "ddns_hostname_%s", enable);
    }

  username = websGetVar (wp, _username, NULL);
  passwd = websGetVar (wp, _passwd, NULL);
  hostname = websGetVar (wp, _hostname, NULL);
  dyndnstype = websGetVar (wp, _dyndnstype, NULL);
  wildcard = websGetVar (wp, _wildcard, NULL);
  custom = websGetVar (wp, _custom, NULL);
  conf = websGetVar (wp, _conf, NULL);
  url = websGetVar (wp, _url, NULL);
  force = websGetVar (wp, _force, NULL);

  if (!username || !passwd || !hostname || !force)
    {
      error_value = 1;
      return 1;
    }

  nvram_set ("ddns_enable", enable);
  nvram_set (_username, username);
  if (strcmp (passwd, TMP_PASSWD))
    {
      nvram_set (_passwd, passwd);
    }
  if (gethash && !contains(hostname,','))
    {
    char hostn[128];
    char *hash=request_freedns(username,nvram_safe_get(_passwd));
    if (hash)
    {
    sprintf(hostn,"%s,%s",hostname,hash);
    nvram_set(_hostname,hostn);
    free(hash);
    }else
    {
    nvram_set(_hostname,hostname);    
    }
    }else
  nvram_set (_hostname, hostname);
  nvram_set (_dyndnstype, dyndnstype);
  nvram_set (_wildcard, wildcard);
  nvram_set (_custom, custom);
  nvram_set (_conf, conf);
  nvram_set (_url, url);
  nvram_set (_force, force);

  return ret;
}

int
ddns_update_value (webs_t wp)
{
  return 1;
}
