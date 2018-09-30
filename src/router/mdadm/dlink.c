
/* doubly linked lists */
/* This is free software. No strings attached. No copyright claimed */

#include	<unistd.h>
#include	<stdlib.h>
#include	<string.h>
#ifdef __dietlibc__
char *strncpy(char *dest, const char *src, size_t n) __THROW;
#endif
void *xcalloc(size_t num, size_t size);
#include	"dlink.h"

void *dl_head()
{
    void *h;
    h = dl_alloc(0);
    dl_next(h) = h;
    dl_prev(h) = h;
    return h;
}

void dl_free(void *v)
{
    struct __dl_head *vv  = v;
    free(vv-1);
}

void dl_init(void *v)
{
    dl_next(v) = v;
    dl_prev(v) = v;
}

void dl_insert(void *head, void *val)
{
    dl_next(val) = dl_next(head);
    dl_prev(val) = head;
    dl_next(dl_prev(val)) = val;
    dl_prev(dl_next(val)) = val;
}

void dl_add(void *head, void *val)
{
    dl_prev(val) = dl_prev(head);
    dl_next(val) = head;
    dl_next(dl_prev(val)) = val;
    dl_prev(dl_next(val)) = val;
}

void dl_del(void *val)
{
    if (dl_prev(val) == 0 || dl_next(val) == 0)
	return;
    dl_prev(dl_next(val)) = dl_prev(val);
    dl_next(dl_prev(val)) = dl_next(val);
    dl_prev(val) = dl_next(val) = 0;
}

char *dl_strndup(char *s, int l)
{
    char *n;
    if (s == NULL)
	return NULL;
    n = dl_newv(char, l+1);
    strncpy(n, s, l);
    n[l] = 0;
    return n;
}

char *dl_strdup(char *s)
{
    return dl_strndup(s, (int)strlen(s));
}
