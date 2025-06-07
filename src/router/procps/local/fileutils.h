#ifndef PROCPS_NG_FILEUTILS
#define PROCPS_NG_FILEUTILS

#include <wchar.h>

int close_stream(FILE * stream);
void close_stdout(void);
wint_t getmb(FILE *f);

#endif
