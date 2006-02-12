/* files.h */
#ifndef _FILES_H
#define _FILES_H

struct config_keyword {
	char keyword[14];
	int (*handler)(char *line, void *var);
	void *var;
	char def[30];
};

int read_config(char *file);
void write_leases(void);
void delete_leases(int dummy);
void read_leases(char *file);
void read_statics(char *file);

#endif
