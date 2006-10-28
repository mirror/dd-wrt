#ifndef HTTPENCODE_H
#define HTTPENCODE_H

#define REQ_URL_P1_FMT "GET %s?info_hash=%s&peer_id=%s&port=%d"
#define REQ_URL_P2_FMT "%s&uploaded=%d&downloaded=%d&left=%d&event=%s&compact=1 HTTP/1.0"
#define REQ_URL_P3_FMT "%s&uploaded=%d&downloaded=%d&left=%d&compact=1 HTTP/1.0"

char* Http_url_encode(char *s,char *b,size_t n);
int Http_url_analyse(char *url,char *host,int *port,char *path);
size_t Http_split(char *b,size_t n,char **pd,size_t *dlen);
int Http_reponse_code(char *b,size_t n);
int Http_get_header(char *b,int n,char *header,char *v);

#endif
