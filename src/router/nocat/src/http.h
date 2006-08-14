# define RAW_LINE_SIZE 40
# define MAX_REQUEST_SIZE 100000L
# define REQ_TIMEOUT 250000
# define MAX_REQ_TIMEOUTS 5
# define HEADER(x) (h->header == NULL ? NULL : \
	(gchar *)g_hash_table_lookup(h->header, (x)))
# define QUERY(x)  (h->query == NULL  ? NULL : \
	(gchar *)g_hash_table_lookup(h->query, (x)))

typedef struct {
    gchar *uri;
    gchar *method;
    GHashTable *header;
    GHashTable *query;
    GHashTable *response;
    GString *buffer;
    gboolean complete;
    GIOChannel *sock;
    gchar peer_ip[16];
    gchar sock_ip[16];
    gboolean password_checked;
} http_request;

/*** Function prototypes start here ***/
GIOChannel *http_bind_socket( const char *ip, int port, int queue );
http_request *http_request_new ( GIOChannel *sock );
void http_request_free ( http_request *h );
GHashTable *parse_query_string( gchar *query );
GHashTable *http_parse_header (http_request *h, gchar *req);
GHashTable *http_parse_query (http_request *h, gchar *post);
guint http_request_read (http_request *h);
gboolean http_request_ok (http_request *h);
void http_add_header ( http_request *h, const gchar *key, gchar *val );
void http_printf_header ( http_request *h, gchar *key, gchar *fmt, ... );
GIOError http_send_header ( http_request *h, int status, const gchar *msg );
void http_send_redirect( http_request *h, gchar *dest );
void syslog_message(const char *prefix, const char *buf, size_t n);

gchar *http_fix_path (const gchar *uri, const gchar *docroot);
gchar *http_mime_type (const gchar *path);
int http_open_file (const gchar *path, int *status);
ssize_t http_sendfile ( http_request *h, int in_fd );
int http_serve_file ( http_request *h, const gchar *docroot );
int http_serve_template ( http_request *h, gchar *file, GHashTable *data );
