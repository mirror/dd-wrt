void definefilter(int *aborted);
int loadfilter(char *filename, struct filterlist *fl, int resolve);
void savefilter(char *filename, struct filterlist *fl);
void destroyfilter(struct filterlist *fl);
void editfilter(int *aborted);
void delfilter(int *aborted);
