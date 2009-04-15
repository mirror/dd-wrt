#ifndef OPENBSD_STRINGFUNCS_H
#define OPENBSD_STRINGFUNCS_H

//Use safer openbsd string functions: 
//strlcpy instead of strcpy
extern size_t strlcpy(char *dst, const char *src, size_t siz);

//strlcat instead of strcat
extern size_t strlcat(char *dst, const char *src, size_t siz);

#endif
