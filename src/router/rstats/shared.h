#ifndef __SHARED_H__
#define __SHARED_H__

#include <netinet/in.h>
#include <signal.h>


#define Y2K			946684800UL		// seconds since 1970


//version.c
extern const char *tomato_version;
extern const char *tomato_buildtime;


// misc.c
#define	WP_DISABLED		0		// order must be synced with def at misc.c
#define	WP_STATIC		1
#define WP_DHCP			2
#define	WP_HEARTBEAT	3
#define	WP_L2TP			4
#define	WP_PPPOE		5
#define	WP_PPTP			6


typedef struct {
	int count;
	struct in_addr dns[3];
} dns_list_t;

extern int get_wan_proto(void);
extern int using_dhcpc(void);
extern void notice_set(const char *path, const char *format, ...);
extern int check_wanup(void);
extern const dns_list_t *get_dns(void);
extern void set_action(int a);
extern int check_action(void);
extern int wait_action_idle(int n);
extern int wl_client(void);


// process.c
extern char *psname(int pid, char *buffer, int maxlen);
extern int pidof(const char *name);
extern int killall(const char *name, int sig);


// files.c
#define FW_CREATE	0
#define FW_APPEND	1
#define FW_NEWLINE	2

extern unsigned long f_size(const char *path);
extern int f_exists(const char *file);
extern int f_read(const char *file, void *buffer, int max);												// returns bytes read
extern int f_write(const char *file, const void *buffer, int len, unsigned flags, unsigned cmode);		//
extern int f_read_string(const char *file, char *buffer, int max);										// returns bytes read, not including term; max includes term
extern int f_write_string(const char *file, const char *buffer, unsigned flags, unsigned cmode);		//
extern int f_read_alloc(const char *path, char **buffer, int max);
extern int f_read_alloc_string(const char *path, char **buffer, int max);


// led.c
#define LED_WLAN			0
#define LED_DIAG			1
#define LED_WHITE			2
#define LED_AMBER			3
#define LED_DMZ				4
#define LED_USB				5	// SL
#define LED_MAX				5

#define	LED_OFF				0
#define	LED_ON				1
#define LED_BLINK			2	// (USB only)

//	extern int gpio_outen(unsigned long bits, unsigned long mask);
//	extern unsigned long rgpio(unsigned long bits);
//	extern void wgpio(unsigned long bits, unsigned long mask);
extern void led(int which, int mode);


// base64.c
extern const char base64_xlat[];
extern int base64_encode(unsigned char *in, char *out, int inlen);			// returns amount of out buffer used
extern int base64_decode(const char *in, unsigned char *out, int inlen);	// returns amount of out buffer used
extern int base64_encoded_len(int len);
extern int base64_decoded_len(int len);										// maximum possible, may not be actual


// strings.c
//	#define vstrsep(buf, sep, args...) _vstrsep(buf, sep, args, NULL)
//	extern int _vstrsep(char *buf, const char *sep, ...);

#endif
