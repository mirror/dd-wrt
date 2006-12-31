/* file.h - (source) file IO */
 
/* Written 1995,1996 by Werner Almesberger, EPFL-LRC */
 

#ifndef FILE_H
#define FILE_H

extern int pc;

void open_files(const char *prefix);
void to_h(const char *fmt,...);
void to_c(const char *fmt,...);
void to_test(const char *fmt,...);
void to_dump(const char *fmt,...);
void close_files(void);

void begin_code(void);
void code(const char *fmt,...);
int end_code(void);
void patch(int old_pc,int value);

#endif
