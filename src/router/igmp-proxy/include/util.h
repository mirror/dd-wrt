
#define UNKNOWN -1
#define EMPTY 0
#define IGMPVERSION 1
#define IS_QUERIER 2
#define UPSTREAM   4

/*#define DEFAULT_VERSION 2*/
#define DEFAULT_ISQUERIER 1
#define EQUAL(s1,s2)	(strcmp((s1),(s2)) == 0)
extern int log_level;

/*
 * Log support
 */
#ifdef DEBUG
#define LOG(x) debug x
#else
#define LOG(x)
#endif
//#define LOG_INFO	1
#define	LOG_DETAIL	2
//#define LOG_DEBUG	3

typedef struct _interface_list_t {
    char ifl_name[IFNAMSIZ];
    struct sockaddr ifl_addr;
    struct _interface_list_t *ifl_next;
} interface_list_t;

unsigned short in_cksum(unsigned short *addr, int len);
interface_list_t* get_interface_list(short af, short flags, short unflags);
void free_interface_list(interface_list_t *ifl);
short get_interface_flags(char *ifname);
short set_interface_flags(char *ifname, short flags);
int get_interface_mtu(char *ifname);
int mrouter_onoff(int sockfd, int onoff);
char *next_word(char **s);
