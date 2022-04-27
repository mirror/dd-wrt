#ifndef __printflike
#define	__printflike(_a,_b) \
	__attribute__ ((__format__ (__printf__, _a, _b)))
#endif
#ifndef __va_list
#define	__va_list	va_list
#endif
#ifndef __packed
#define	__packed	__attribute__((__packed__))
#endif
