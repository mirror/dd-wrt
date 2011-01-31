/* file.c - (source) file IO */
 
/* Written 1995,1996 by Werner Almesberger, EPFL-LRC */

#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>

#include "common.h"
#include "file.h"


#define DEFAULT_BUF_SIZE 256


typedef struct {
    const char *data;
    const char *extra;
} CODE_ITEM;


int pc;


static FILE *h_file,*c_file,*test_file,*dump_file;
static CODE_ITEM *buf;
static int buf_size;


void to_h(const char *fmt,...)
{
    va_list ap;

    va_start(ap,fmt);
    if (vfprintf(h_file,fmt,ap) == EOF) {
	perror("write");
	exit(1);
    }
    va_end(ap);
}


void to_c(const char *fmt,...)
{
    va_list ap;

    va_start(ap,fmt);
    if (vfprintf(c_file,fmt,ap) == EOF) {
	perror("write");
	exit(1);
    }
    va_end(ap);
}


void to_test(const char *fmt,...)
{
    va_list ap;

    va_start(ap,fmt);
    if (vfprintf(test_file,fmt,ap) == EOF) {
	perror("write");
	exit(1);
    }
    va_end(ap);
}


void to_dump(const char *fmt,...)
{
    va_list ap;

    va_start(ap,fmt);
    if (vfprintf(dump_file,fmt,ap) == EOF) {
	perror("write");
	exit(1);
    }
    va_end(ap);
}


void open_files(const char *prefix)
{
    char name[100]; /* maximum name */

    sprintf(name,"%s.out.h",prefix);
    h_file = fopen(name,"w");
    if (!h_file) {
	perror("creat");
	exit(1);
    }
    sprintf(name,"%s.out.c",prefix);
    c_file = fopen(name,"w");
    if (!c_file) {
	perror("creat");
	exit(1);
    }
    sprintf(name,"%s.test.c",prefix);
    test_file = fopen(name,"w");
    if (!test_file) {
	perror("creat");
	exit(1);
    }
    if (dump) {
	sprintf(name,"%s.dump.c",prefix);
	dump_file = fopen(name,"w");
	if (!dump_file) {
	    perror("creat");
	    exit(1);
	}
    }
}


void close_files(void)
{
    if (fclose(h_file) == EOF) {
	perror("fclose");
	exit(1);
    }
    if (fclose(c_file) == EOF) {
	perror("fclose");
	exit(1);
    }
    if (fclose(test_file) == EOF) {
	perror("fclose");
	exit(1);
    }
    if (dump)
	if (fclose(dump_file) == EOF) {
	    perror("fclose");
	    exit(1);
	}
}


void begin_code(void)
{
    pc = 0;
    buf_size = DEFAULT_BUF_SIZE;
    buf = alloc(sizeof(CODE_ITEM)*buf_size);
}


static void put_item(const char *item)
{
    if (pc >= buf_size) {
	buf_size *= 2;
	buf = realloc(buf,sizeof(CODE_ITEM)*buf_size);
	if (!buf) die("out of memory");
    }
    buf[pc].data = item;
    buf[pc++].extra = NULL;
}


static void append_last(const char *str)
{
    buf[pc-1].extra = str;
}


static char *itos(int val)
{
    char buffer[21]; /* enough for 64 bits ... */

    sprintf(buffer,"%d",val);
    return stralloc(buffer);
}


void code(const char *fmt,...)
{
    va_list ap;

    va_start(ap,fmt);
    while (*fmt)
	if (*fmt++ == '%') {
	    switch (*fmt++) {
		case 'd':
		    put_item(itos(va_arg(ap,int)));
		    break;
		case 's':
		    put_item(va_arg(ap,const char *));
		    break;
		default:
		    die("invalid format character %c",fmt[-1]);
	    }
	}
	else {
	    const char *here;

	    here = strchr(--fmt,'%');
	    if (!here) append_last(fmt);
	    else {
		char buffer[200]; /* ugly */

		if (here[1] != 's') die("bad format in extension");
		strncpy(buffer,fmt,(size_t) (here-fmt));
		strcpy(buffer+(here-fmt),va_arg(ap,const char *));
		strcat(buffer,here+2);
		append_last(stralloc(buffer));
	    }
	    break;
	}
    va_end(ap);
}


int end_code(void)
{
    int indent,i;

    indent = 1;
    for (i = 0; i < pc; i++) {
	if (indent) {
	    to_c("    ");
	    indent = 0;
	}
	to_c(buf[i].data);
	if (buf[i].extra && *buf[i].extra == '\n') to_c(",");
	else to_c(", ");
	if (buf[i].extra) {
	    to_c(buf[i].extra);
	    indent = !!strchr(buf[i].extra,'\n');
	    if (indent) to_c("/*%4d*/",i+1);
	}
    }
    return pc;
}


void patch(int old_pc,int value)
{
    /* may leak memory */
    buf[old_pc].data = itos(value);
}
