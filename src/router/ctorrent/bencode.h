#ifndef BENCODE_H
#define BENCODE_H

#include <sys/types.h>
#include <stdio.h>

#define KEY_SP '|'	//the keyname list's delimiters
#define KEYNAME_SIZ 32
#define KEYNAME_LISTSIZ 256

#define MAX_INT_SIZ 64

#define QUERY_STR 0
#define QUERY_INT 1
#define QUERY_POS 2

size_t buf_int(const char *b,size_t len,char beginchar,char endchar,size_t *pi);
size_t buf_str(const char *b,size_t len,const char **pstr,size_t* slen);
size_t decode_int(const char *b,size_t len);
size_t decode_str(const char *b,size_t len);
size_t decode_dict(const char *b,size_t len,const char *keylist);
size_t decode_list(const char *b,size_t len,const char *keylist);
size_t decode_rev(const char *b,size_t len,const char *keylist);
size_t decode_query(const char *b,size_t len,const char *keylist,const char **ps,size_t *pi,int method);
size_t decode_list2path(const char *b, size_t n, char *pathname);
size_t bencode_buf(const char *str,size_t len,FILE *fp);
size_t bencode_str(const char *str, FILE *fp);
size_t bencode_int(const int integer, FILE *fp);
size_t bencode_begin_dict(FILE *fp);
size_t bencode_begin_list(FILE *fp);
size_t bencode_end_dict_list(FILE *fp);
size_t bencode_path2list(const char *pathname, FILE *fp);

#endif
