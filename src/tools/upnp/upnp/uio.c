
/*
    Copyright 2004, Broadcom Corporation      
    All Rights Reserved.      
          
    THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY      
    KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM      
    SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS      
    FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.      
*/


#include "upnp_dbg.h"
#include "upnp_osl.h"
#include "upnp.h"

#define UIO_FILE     (1<<0)
#define UIO_STRING   (1<<2)
#define UIO_DYNAMIC  (1<<3)

/*
  
  Define UIO_TRACE to track the creation and deletion of uio
  structures when you suspect a leak.  When this symbol is defined,
  each UFILE structure will be tracked with a unique identifier.  That
  identifier will br printed when the structure is allocated and when
  it is freed.  If you see the identifiers grow much beyond 2 or 3,
  then you probably are leaking UFILE structures.

  #define UIO_TRACE 1
*/

#ifdef UIO_TRACE 
fd_set uio_fds;
#endif

struct _ufile {
#ifdef UIO_TRACE 
    int identifier;
#endif
    int flags;
    int bytes_written;
    union {
	FILE *fp;
	struct {
	    char *buffer;
	    int avail;
	} str;
    } io;
};

UFILE *uio_new()
{
    UFILE *up;
#ifdef UIO_TRACE
    int i;
#endif

    up = (UFILE *) malloc(sizeof(UFILE));
    if (up) {
	memset(up, 0, sizeof(UFILE));
#ifdef UIO_TRACE
	for (i = 0; i < 64; i++) {
	    if (!FD_ISSET(i, &uio_fds)) {
		up->index = i;
		FD_SET(up->index, &uio_fds);
		printf("%s %d\n", __FUNCTION__, up->index); 
		break;
	    }
	}
#endif
    }
    return up;
}

void uio_delete(UFILE *up)
{
#ifdef UIO_TRACE
    printf("%s %d\n", __FUNCTION__, up->index); 
    FD_CLR(up->index, &uio_fds);
#endif

    free(up);
}


UFILE *udopen(int fd)
{
    UFILE *up = NULL;
    FILE *fp;

    fp = fdopen(fd, "w");
    if (fp != NULL) {
	up = uio_new();
	if (up) {
	    up->flags |= UIO_FILE;
	    up->io.fp = fp;
	} else {
	    fclose(fp);
	}
    }
	       
    return up;
}


UFILE *ufopen(const char *fname)
{
    UFILE *up = NULL;
    FILE *fp;

    fp = fopen(fname, "w");
    if (fp != NULL) {
	up = uio_new();
	if (up) {
	    up->flags |= UIO_FILE;
	    up->io.fp = fp;
	} 
    }
	       
    return up;
}


UFILE *usopen(char *str, int maxlen)
{
    UFILE *up;

    up = uio_new();
    if (up) {
	up->flags |= UIO_STRING;
	if (str) {
	    up->io.str.buffer = str;
	    up->io.str.avail = maxlen;
	} else {
	    if (maxlen <= 0)
		maxlen = 1024;
	    up->flags |= UIO_DYNAMIC;
	    up->io.str.buffer = malloc(maxlen);
	    up->io.str.avail = maxlen;
	}
    } 
	       
    return up;
}


int uprintf(UFILE *up, char *fmt, ...)
{
    va_list ap;
    int len, n = 0;
    char *buf;

    va_start(ap, fmt);
    if (up->flags & UIO_STRING) {
	while (TRUE) {
	    buf = &up->io.str.buffer[up->bytes_written];
	    len = up->io.str.avail - up->bytes_written;
	    n = vsnprintf(buf, len, fmt, ap);
	    if (n < 0 || n > len) {
		/* we tried to overwrite the end of the buffer */
		if (up->flags & UIO_DYNAMIC) {
		    buf = realloc(up->io.str.buffer, up->io.str.avail + 256);
#ifdef UIO_TRACE
		    printf("realloc was %d, now %d\n", up->io.str.avail, up->io.str.avail + 256);
#endif
		    if (buf) {
			up->io.str.buffer = buf;
			up->io.str.avail += 256;
			continue;
		    }
		}
	    } else {
		up->bytes_written += n;
	    }
	    break;
	} /* end while */
    } else if (up->flags & UIO_FILE) {
	n = vfprintf(up->io.fp, fmt, ap);
	if (n > 0) {
	    up->bytes_written += n;
	}
    }
    va_end(ap);

    return n;
}


void uclose(UFILE *up)
{
    if (up->flags & UIO_FILE) {
	fclose(up->io.fp);
    } else if (up->flags & UIO_DYNAMIC) {
	free(up->io.str.buffer);
    }
    uio_delete(up);
}


int utell(UFILE *up)
{
    return up->bytes_written;
}


char *ubuffer(UFILE *up)
{
    char *buf = NULL;

    if (up->flags & UIO_STRING) {
	buf = up->io.str.buffer;
    } else {
	UPNP_ERROR(("Called %s on UFILE that does not use buffers.\n", __FUNCTION__));
    }
    return buf;
}


int ufileno(UFILE *up)
{
    int fd = 0;

    if (up->flags & UIO_FILE) {
	fd = fileno(up->io.fp);
    }  else {
	UPNP_ERROR(("Called %s on UFILE that does not use file descriptors.\n", __FUNCTION__));
    }
    return fd;
}


void uflush(UFILE *up)
{
    if (up->flags & UIO_FILE) {
	fflush(up->io.fp);
    } else if (up->flags & UIO_STRING) {
	up->bytes_written = 0;
    }
}
