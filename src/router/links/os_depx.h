#ifndef MAXINT
#ifdef INT_MAX
#define MAXINT INT_MAX
#else
#define MAXINT ((int)((unsigned int)-1 >> 1))
#endif
#endif

#ifndef MAXLONG
#ifdef LONG_MAX
#define MAXLONG LONG_MAX
#else
#define MAXLONG ((long)((unsigned long)-1L >> 1))
#endif
#endif

#ifndef SA_RESTART
#define SA_RESTART	0
#endif

#ifdef __EMX__
#define strcasecmp stricmp
#define strncasecmp strnicmp
#define read _read
#define write _write
#define getcwd _getcwd2
#define chdir _chdir2
#endif

#ifdef BEOS
#define socket be_socket
#define connect be_connect
#define getpeername be_getpeername
#define getsockname be_getsockname
#define listen be_listen
#define accept be_accept
#define bind be_bind
#define pipe be_pipe
#define read be_read
#define write be_write
#define close be_close
#define select be_select
#define getsockopt be_getsockopt
#ifndef PF_INET
#define PF_INET AF_INET
#endif
#ifndef SO_ERROR
#define SO_ERROR	10001
#endif
#endif

#if defined(O_SIZE) && defined(__EMX__)
#define HAVE_OPEN_PREALLOC
#endif

#if defined(__WATCOMC__) && defined(_WCRTLINK)
#define LIBC_CALLBACK	_WCRTLINK
#else
#define LIBC_CALLBACK
#endif

#if defined(__WATCOMC__) && defined(__LINUX__)
#define SIGNAL_HANDLER __declspec(__cdecl)
#else
#define SIGNAL_HANDLER
#endif

#if defined(HAVE_HERROR) && defined(__GNUC__) && defined(__hpux)
#undef HAVE_HERROR
#endif

#ifdef HAVE_MAXINT_CONVERSION_BUG
#undef MAXINT
#define MAXINT 0x7FFFF000
#endif

