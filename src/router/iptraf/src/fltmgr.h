#ifndef IPTRAF_NG_FLTMGR_H
#define IPTRAF_NG_FLTMGR_H

/***

fltmgr.h - filter list management routine prototypes

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

void makestdfiltermenu(struct MENU *menu);
void makemainfiltermenu(struct MENU *menu);
int loadfilterlist(struct ffnode **fltfile);
void save_filterlist(struct ffnode *fltlist);
void pickafilter(struct ffnode *files, struct ffnode **fltfile, int *aborted);
void selectfilter(struct filterfileent *ffe, int *aborted);
void destroyfilterlist(struct ffnode *fltlist);
void get_filter_description(char *description, int *aborted, char *pre_edit);
void genname(unsigned long n, char *m);
unsigned long int nametoaddr(char *ascname, int *err);
void listfileerr(int code);

#endif	/* IPTRAF_NG_FLTMGR_H */
