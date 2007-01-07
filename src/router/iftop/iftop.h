/*
 * iftop.h:
 *
 */

#ifndef __IFTOP_H_ /* include guard */
#define __IFTOP_H_

#include "config.h"

/* 40 / 2  */
#define HISTORY_LENGTH  20
#define RESOLUTION 2

typedef struct {
    long recv[HISTORY_LENGTH];
    long sent[HISTORY_LENGTH];
    double long total_sent;
    double long total_recv;
    int last_write;
} history_type;

void tick(int print);

void *xmalloc(size_t n);
void *xcalloc(size_t n, size_t m);
void *xrealloc(void *w, size_t n);
char *xstrdup(const char *s);
void xfree(void *v);

/* ui.c */
void analyse_data(void);
void ui_init(void);

/* options.c */
void options_read(int argc, char **argv);


#endif /* __IFTOP_H_ */
