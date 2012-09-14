#ifndef IPTRAF_NG_FLTEDIT_H
#define IPTRAF_NG_FLTEDIT_H

void definefilter(int *aborted);
int loadfilter(char *filename, struct filterlist *fl, int resolve);
void savefilter(char *filename, struct filterlist *fl);
void destroyfilter(struct filterlist *fl);
void editfilter(int *aborted);
void delfilter(int *aborted);

#endif	/* IPTRAF_NG_FLTEDIT_H */
