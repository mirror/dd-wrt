#include <sys/types.h>

#include "./def.h"
#include "bencode.h"

#ifndef WINDOWS
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <limits.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static const char* next_key(const char *keylist)
{
  for(;*keylist && *keylist != KEY_SP; keylist++);
  if(*keylist) keylist++;
  return keylist;
}

static size_t compare_key(const char *key,size_t keylen,const char *keylist)
{
  for(;keylen && *keylist && *key==*keylist;keylen--,key++,keylist++) ;
  if(!keylen) if(*keylist && *keylist!=KEY_SP) return 1;
  return keylen;
}

size_t buf_int(const char *b,size_t len,char beginchar,char endchar,size_t *pi)
{
  const char *p = b;
  const char *psave;

  if(2 > len) return 0;	/* buffer too small */

  if( beginchar ){ 
    if(*p != beginchar) return 0; 
    p++; len--;
  }

  for(psave = p; len && isdigit(*p); p++,len--) ;

  if(!len || MAX_INT_SIZ < (p - psave) || *p != endchar) return 0;

  if( pi ){
    if( beginchar ) *pi = (size_t)strtol(b + 1,(char**) 0,10);
    else  *pi=(size_t)strtol(b,(char**) 0,10);
  }
  return (size_t)( p - b + 1 );
}

size_t buf_str(const char *b,size_t len,const char **pstr,size_t* slen)
{
  size_t rl,sl;

  rl = buf_int(b,len,0,':',&sl);

  if( !rl ) return 0;

  if(len < rl + sl) return 0;
  if(pstr) *pstr = b + rl;
  if(slen) *slen = sl;

  return( rl + sl );
}

size_t decode_int(const char *b,size_t len)
{
  return(buf_int(b,len,'i','e',(size_t*) 0));
}

size_t decode_str(const char *b,size_t len)
{
  return (buf_str(b,len,(const char**) 0,(size_t*) 0));
}

size_t decode_dict(const char *b,size_t len,const char *keylist)
{
  size_t rl,dl,nl;
  const char *pkey;
  dl = 0;
  if(2 > len || *b != 'd') return 0;

  dl++; len--;
  for(;len && *(b + dl) != 'e';){
    rl = buf_str(b + dl,len,&pkey,&nl);

    if( !rl || KEYNAME_SIZ < nl) return 0;
    dl += rl;
    len -= rl;

    if(keylist && compare_key(pkey,nl,keylist) == 0){
      pkey = next_key(keylist);
      if(! *pkey ) return dl;
      rl = decode_dict(b + dl,len, pkey);
      if( !rl ) return 0;
      return dl + rl;
    }

    rl = decode_rev(b + dl,len,(const char*) 0);
    if( !rl ) return 0;

    dl += rl;len -= rl;
  }
  if( !len || keylist) return 0;
  return dl + 1;	/* add the last char 'e' */
}

size_t decode_list(const char *b,size_t len,const char *keylist)
{
  size_t ll,rl;
  ll = 0;
  if(2 > len || *b != 'l') return 0;
  len--; ll++;
  for(;len && *(b + ll) != 'e';){
    rl = decode_rev(b + ll,len,keylist);
    if( !rl ) return 0;

    ll += rl; len -= rl;
  }
  if( !len ) return 0;
  return ll + 1;  /* add last char 'e' */
}

size_t decode_rev(const char *b,size_t len,const char *keylist)
{
  if( !b ) return 0;
  switch( *b ){
  case 'i': return decode_int(b,len);
  case 'd': return decode_dict(b,len,keylist);
  case 'l': return decode_list(b,len,keylist);
  default: return decode_str(b,len);
  }
}

size_t decode_query(const char *b,size_t len,const char *keylist,const char **ps,size_t *pi,int method)
{
  size_t pos;
  char kl[KEYNAME_LISTSIZ];
  strcpy(kl,keylist);
  pos = decode_rev(b, len, kl);
  if( !pos ) return 0;
  switch(method){
  case QUERY_STR: return(buf_str(b + pos,len - pos, ps, pi));
  case QUERY_INT: return(buf_int(b + pos,len - pos, 'i', 'e', pi));
  case QUERY_POS:
	  if(pi) *pi = decode_rev(b + pos, len - pos, (const char*) 0);
	  return pos;
  default: return 0;
  }
}

size_t bencode_buf(const char *buf,size_t len,FILE *fp)
{
  char slen[MAX_INT_SIZ];
  char *b;
  if( MAX_INT_SIZ <= snprintf(slen, MAX_INT_SIZ, "%u:", len) ) return 0;
  if( fwrite( slen, strlen(slen), 1, fp) != 1) return 0;
  b = new char[len + strlen(slen)];
#ifndef WINDOWS
  if( !b ) return 0;
#endif
  if( fwrite(buf, len, 1, fp) != 1 ){ delete []b; return 0;}
  delete []b;
  return 1;
}

size_t bencode_str(const char *str, FILE *fp)
{
  return bencode_buf(str, strlen(str), fp);
}

size_t bencode_int(const int integer, FILE *fp)
{
  char buf[MAX_INT_SIZ];
  if( EOF == fputc('i', fp)) return 0;
  if( MAX_INT_SIZ <= snprintf(buf, MAX_INT_SIZ, "%u", integer) )
    return 0;
  if( fwrite(buf, strlen(buf), 1, fp) != 1 ) return 0;
  return ( EOF == fputc('e', fp)) ? 0: 1;
}

size_t bencode_begin_dict(FILE *fp)
{
  return (EOF == fputc('d',fp)) ? 0 : 1;
}

size_t bencode_begin_list(FILE *fp)
{
  return (EOF == fputc('l',fp)) ? 0 : 1;
}

size_t bencode_end_dict_list(FILE *fp)
{
  return (EOF == fputc('e',fp)) ? 0 : 1;
}

size_t bencode_path2list(const char *pathname, FILE *fp)
{
  char *pn;
  const char *p = pathname;
  
  if( bencode_begin_list(fp) != 1 ) return 0;
  
  for(; *p;){
    pn = strchr(p, PATH_SP);
    if( pn ){
      if( bencode_buf(p, pn - p, fp) != 1) return 0;
      p = pn + 1;
    }else{
      if( bencode_str(p, fp) != 1) return 0;
      break;
    }
  }
  
  return bencode_end_dict_list(fp);
}

size_t decode_list2path(const char *b, size_t n, char *pathname)
{
  const char *pb = b;
  const char *s = (char *) 0;
  size_t r,q;

  if( 'l' != *pb ) return 0;
  pb++;
  n--;
  if( !n ) return 0;
  for(; n;){
    if(!(r = buf_str(pb, n, &s, &q)) ) return 0;
    memcpy(pathname, s, q);
    pathname += q;
    pb += r; n -= r; 
    if( 'e' != *pb ){*pathname = PATH_SP, pathname++;} else break;
  }
  *pathname = '\0';
  return (pb - b + 1);
}
