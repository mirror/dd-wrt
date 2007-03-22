/***

fltmgr.h - filter list management routine prototypes

Copyright (c) Gerard Paul Java 1998, 2002

***/

struct filterfileent {
    char desc[35];
    char filename[40];
};

struct ffnode {
    struct filterfileent ffe;
    struct ffnode *next_entry;
    struct ffnode *prev_entry;
};

#ifndef IGNORE_FILTER_PROTOTYPES
void makestdfiltermenu(struct MENU *menu);
void makemainfiltermenu(struct MENU *menu);
int loadfilterlist(struct ffnode **fltfile);
void save_filterlist(struct ffnode *fltlist);
void pickafilter(struct ffnode *files, struct ffnode **fltfile,
                 int *aborted);
char *pickfilterbyname(struct ffnode *fltlist, char *filename);
void selectfilter(struct filterfileent *ffe, int *aborted);
void destroyfilterlist(struct ffnode *fltlist);
void get_filter_description(char *description, int *aborted,
                            char *pre_edit);
void genname(unsigned long n, char *m);
unsigned long int nametoaddr(char *ascname, int *err);
void listfileerr(int code);
int mark_filter_change();
void clear_flt_tag();
#endif
